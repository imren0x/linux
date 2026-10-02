// SPDX-License-Identifier: GPL-2.0-only
/*
 * ION compatibility device
 *
 * Provides /dev/ion for vendor userspace built against the msm ION of
 * downstream kernels, on top of DMA-BUF heaps. Both the legacy (pre-4.12)
 * handle based ioctl ABI and the 4.12+ fd based one are served; they are told
 * apart by the size of the allocation request.
 *
 * The heaps are described like the downstream "qcom,msm-ion" node: each child
 * has the ION heap ID in "reg" and a "qcom,ion-heap-type". SYSTEM heaps
 * allocate from the "system" DMA-BUF heap, DMA heaps from the CMA heap of
 * their "memory-region". "linux,dma-heap-name" overrides the DMA-BUF heap.
 * Secure heaps are not supported and are skipped.
 */

#define pr_fmt(fmt) "ion-compat: " fmt

#include <linux/compat.h>
#include <linux/dma-buf.h>
#include <linux/dma-heap.h>
#include <linux/fcntl.h>
#include <linux/miscdevice.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/of.h>
#include <linux/of_reserved_mem.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/sort.h>
#include <linux/uaccess.h>
#include <linux/xarray.h>

#define ION_NUM_HEAP_IDS	32
#define ION_NAME_LEN		32

/* Bit 31 of the flags marks content protected (secure) allocations. */
#define ION_FLAG_SECURE		BIT(31)

/* Heap types, as in the upstream and msm ION uapi */
enum ion_heap_type {
	ION_HEAP_TYPE_SYSTEM,
	ION_HEAP_TYPE_SYSTEM_CONTIG,
	ION_HEAP_TYPE_CARVEOUT,
	ION_HEAP_TYPE_CHUNK,
	ION_HEAP_TYPE_DMA,
};

#define ION_IOC_MAGIC		'I'

/* Legacy (pre-4.12) ABI */

typedef int ion_user_handle_t;

struct ion_legacy_allocation_data {
	size_t len;
	size_t align;
	unsigned int heap_id_mask;
	unsigned int flags;
	ion_user_handle_t handle;
};

#ifdef CONFIG_COMPAT
struct compat_ion_legacy_allocation_data {
	compat_size_t len;
	compat_size_t align;
	compat_uint_t heap_id_mask;
	compat_uint_t flags;
	compat_int_t handle;
};
#endif

struct ion_fd_data {
	ion_user_handle_t handle;
	int fd;
};

struct ion_handle_data {
	ion_user_handle_t handle;
};

#define ION_IOC_LEGACY_ALLOC	_IOWR(ION_IOC_MAGIC, 0, \
				      struct ion_legacy_allocation_data)
#ifdef CONFIG_COMPAT
#define COMPAT_ION_IOC_LEGACY_ALLOC _IOWR(ION_IOC_MAGIC, 0, \
				      struct compat_ion_legacy_allocation_data)
#endif
#define ION_IOC_FREE		_IOWR(ION_IOC_MAGIC, 1, struct ion_handle_data)
#define ION_IOC_MAP		_IOWR(ION_IOC_MAGIC, 2, struct ion_fd_data)
#define ION_IOC_SHARE		_IOWR(ION_IOC_MAGIC, 4, struct ion_fd_data)
#define ION_IOC_IMPORT		_IOWR(ION_IOC_MAGIC, 5, struct ion_fd_data)
#define ION_IOC_SYNC		_IOWR(ION_IOC_MAGIC, 7, struct ion_fd_data)

/* 4.12+ ABI */

struct ion_allocation_data {
	__u64 len;
	__u32 heap_id_mask;
	__u32 flags;
	__u32 fd;
	__u32 unused;
};

struct ion_heap_data {
	char name[ION_NAME_LEN];
	__u32 type;
	__u32 heap_id;
	__u32 reserved0;
	__u32 reserved1;
	__u32 reserved2;
};

struct ion_heap_query {
	__u32 cnt;
	__u32 reserved0;
	__u64 heaps;
	__u32 reserved1;
	__u32 reserved2;
};

#define ION_IOC_ALLOC		_IOWR(ION_IOC_MAGIC, 0, \
				      struct ion_allocation_data)
#define ION_IOC_HEAP_QUERY	_IOWR(ION_IOC_MAGIC, 8, struct ion_heap_query)

struct ion_compat_heap {
	unsigned int id;
	enum ion_heap_type type;
	const char *name;		/* ION heap name, for HEAP_QUERY */
	const char *dma_heap_name;
	struct dma_heap *dma_heap;	/* looked up on first use */
};

struct ion_compat {
	struct device *dev;
	struct miscdevice misc;
	struct mutex lock;		/* protects the dma_heap lookups */
	struct ion_compat_heap heaps[ION_NUM_HEAP_IDS];
	unsigned int num_heaps;		/* sorted by descending ID */
};

struct ion_compat_handle {
	struct dma_buf *dmabuf;
	unsigned int refcount;
};

/* An open /dev/ion, holding its legacy handles */
struct ion_compat_client {
	struct ion_compat *ion;
	struct mutex lock;		/* protects the handles */
	struct xarray handles;
};

/* Names of the msm ION heaps, as downstream reports them */
static const char * const ion_msm_heap_names[ION_NUM_HEAP_IDS] = {
	[8] = "mm",
	[9] = "secure_heap",
	[10] = "secure_display",
	[13] = "spss",
	[14] = "secure_carveout",
	[19] = "qsecom_ta",
	[20] = "camera_preview",
	[22] = "adsp",
	[25] = "system",
	[26] = "user_contig",
	[27] = "qsecom",
	[28] = "audio",
};

static struct dma_heap *ion_compat_get_dma_heap(struct ion_compat *ion,
						struct ion_compat_heap *heap)
{
	struct dma_heap *dma_heap;

	mutex_lock(&ion->lock);
	if (!heap->dma_heap)
		heap->dma_heap = dma_heap_find(heap->dma_heap_name);
	dma_heap = heap->dma_heap;
	mutex_unlock(&ion->lock);

	return dma_heap;
}

static struct dma_buf *ion_compat_alloc(struct ion_compat *ion, size_t len,
					unsigned int heap_id_mask,
					unsigned int flags)
{
	struct dma_buf *dmabuf = ERR_PTR(-ENODEV);
	unsigned int i;

	if (flags & ION_FLAG_SECURE) {
		dev_dbg(ion->dev, "secure allocations are not supported\n");
		return ERR_PTR(-EOPNOTSUPP);
	}

	/* Like ION, try the heaps of the mask in order of descending ID. */
	for (i = 0; i < ion->num_heaps; i++) {
		struct ion_compat_heap *heap = &ion->heaps[i];
		struct dma_heap *dma_heap;

		if (!(heap_id_mask & BIT(heap->id)))
			continue;

		dma_heap = ion_compat_get_dma_heap(ion, heap);
		if (!dma_heap) {
			dev_warn_ratelimited(ion->dev,
					     "DMA-BUF heap %s of heap %u not found\n",
					     heap->dma_heap_name, heap->id);
			continue;
		}

		dmabuf = dma_heap_buffer_alloc(dma_heap, len,
					       O_RDWR | O_CLOEXEC, 0);
		if (!IS_ERR(dmabuf))
			return dmabuf;
	}

	if (PTR_ERR(dmabuf) == -ENODEV)
		dev_dbg(ion->dev, "no heap for mask 0x%x\n", heap_id_mask);

	return dmabuf;
}

static int ion_compat_export_fd(struct dma_buf *dmabuf)
{
	int fd;

	get_dma_buf(dmabuf);
	fd = dma_buf_fd(dmabuf, O_CLOEXEC);
	if (fd < 0)
		dma_buf_put(dmabuf);

	return fd;
}

/* Takes over the reference to dmabuf, on success and failure. */
static int ion_compat_handle_add(struct ion_compat_client *client,
				 struct dma_buf *dmabuf)
{
	struct ion_compat_handle *handle;
	unsigned long index;
	u32 id;
	int ret;

	mutex_lock(&client->lock);

	/* Like ION, importing a buffer twice gives the same handle. */
	xa_for_each(&client->handles, index, handle) {
		if (handle->dmabuf == dmabuf) {
			handle->refcount++;
			mutex_unlock(&client->lock);
			dma_buf_put(dmabuf);
			return index;
		}
	}

	handle = kzalloc(sizeof(*handle), GFP_KERNEL);
	if (!handle) {
		ret = -ENOMEM;
		goto err_put;
	}
	handle->dmabuf = dmabuf;
	handle->refcount = 1;

	/* Handle 0 is invalid in ION. */
	ret = xa_alloc(&client->handles, &id, handle, xa_limit_31b,
		       GFP_KERNEL);
	if (ret)
		goto err_free;

	mutex_unlock(&client->lock);
	return id;

err_free:
	kfree(handle);
err_put:
	mutex_unlock(&client->lock);
	dma_buf_put(dmabuf);
	return ret;
}

/* Returns the handle's dma-buf with a new reference. */
static struct dma_buf *ion_compat_handle_get(struct ion_compat_client *client,
					     ion_user_handle_t id)
{
	struct ion_compat_handle *handle;
	struct dma_buf *dmabuf = ERR_PTR(-EINVAL);

	if (id <= 0)
		return dmabuf;

	mutex_lock(&client->lock);
	handle = xa_load(&client->handles, id);
	if (handle) {
		dmabuf = handle->dmabuf;
		get_dma_buf(dmabuf);
	}
	mutex_unlock(&client->lock);

	return dmabuf;
}

static int ion_compat_handle_put(struct ion_compat_client *client,
				 ion_user_handle_t id)
{
	struct ion_compat_handle *handle;

	/* Handle 0 is also used to probe for the legacy ABI. */
	if (id <= 0)
		return -EINVAL;

	mutex_lock(&client->lock);
	handle = xa_load(&client->handles, id);
	if (!handle) {
		mutex_unlock(&client->lock);
		return -EINVAL;
	}

	if (--handle->refcount) {
		mutex_unlock(&client->lock);
		return 0;
	}

	xa_erase(&client->handles, id);
	mutex_unlock(&client->lock);

	dma_buf_put(handle->dmabuf);
	kfree(handle);
	return 0;
}

static int ion_compat_legacy_alloc(struct ion_compat_client *client,
				   size_t len, unsigned int heap_id_mask,
				   unsigned int flags)
{
	struct dma_buf *dmabuf;

	dmabuf = ion_compat_alloc(client->ion, len, heap_id_mask, flags);
	if (IS_ERR(dmabuf))
		return PTR_ERR(dmabuf);

	return ion_compat_handle_add(client, dmabuf);
}

static int ion_compat_share(struct ion_compat_client *client,
			    struct ion_fd_data *data)
{
	struct dma_buf *dmabuf;
	int fd;

	dmabuf = ion_compat_handle_get(client, data->handle);
	if (IS_ERR(dmabuf))
		return PTR_ERR(dmabuf);

	fd = ion_compat_export_fd(dmabuf);
	dma_buf_put(dmabuf);
	if (fd < 0)
		return fd;

	data->fd = fd;
	return 0;
}

static int ion_compat_import(struct ion_compat_client *client,
			     struct ion_fd_data *data)
{
	struct dma_buf *dmabuf;
	int id;

	dmabuf = dma_buf_get(data->fd);
	if (IS_ERR(dmabuf))
		return PTR_ERR(dmabuf);

	id = ion_compat_handle_add(client, dmabuf);
	if (id < 0)
		return id;

	data->handle = id;
	return 0;
}

static int ion_compat_sync(struct ion_fd_data *data)
{
	struct dma_buf *dmabuf;

	dmabuf = dma_buf_get(data->fd);
	if (IS_ERR(dmabuf))
		return PTR_ERR(dmabuf);

	/* Write back and invalidate the CPU caches, as ION did. */
	dma_buf_begin_cpu_access(dmabuf, DMA_BIDIRECTIONAL);
	dma_buf_end_cpu_access(dmabuf, DMA_BIDIRECTIONAL);

	dma_buf_put(dmabuf);
	return 0;
}

static int ion_compat_alloc_fd(struct ion_compat *ion,
			       struct ion_allocation_data *data)
{
	struct dma_buf *dmabuf;
	int fd;

	dmabuf = ion_compat_alloc(ion, data->len, data->heap_id_mask,
				  data->flags);
	if (IS_ERR(dmabuf))
		return PTR_ERR(dmabuf);

	fd = dma_buf_fd(dmabuf, O_CLOEXEC);
	if (fd < 0) {
		dma_buf_put(dmabuf);
		return fd;
	}

	data->fd = fd;
	return 0;
}

static int ion_compat_heap_query(struct ion_compat *ion,
				 struct ion_heap_query *query)
{
	struct ion_heap_data __user *buffer = u64_to_user_ptr(query->heaps);
	unsigned int i, cnt;

	if (query->reserved0 || query->reserved1 || query->reserved2)
		return -EINVAL;

	if (!buffer) {
		query->cnt = ion->num_heaps;
		return 0;
	}

	cnt = min(query->cnt, ion->num_heaps);
	for (i = 0; i < cnt; i++) {
		struct ion_compat_heap *heap = &ion->heaps[i];
		struct ion_heap_data hdata = {
			.type = heap->type,
			.heap_id = heap->id,
		};

		strscpy(hdata.name, heap->name, sizeof(hdata.name));
		if (copy_to_user(&buffer[i], &hdata, sizeof(hdata)))
			return -EFAULT;
	}

	query->cnt = cnt;
	return 0;
}

static long ion_compat_ioctl(struct file *file, unsigned int cmd,
			     unsigned long arg)
{
	struct ion_compat_client *client = file->private_data;
	void __user *uarg = (void __user *)arg;
	union {
		struct ion_legacy_allocation_data legacy_alloc;
		struct ion_allocation_data alloc;
		struct ion_handle_data handle;
		struct ion_fd_data fd;
		struct ion_heap_query query;
	} data;
	int ret;

	if (_IOC_TYPE(cmd) != ION_IOC_MAGIC || _IOC_SIZE(cmd) > sizeof(data))
		return -ENOTTY;

	if (copy_from_user(&data, uarg, _IOC_SIZE(cmd)))
		return -EFAULT;

	switch (cmd) {
	case ION_IOC_LEGACY_ALLOC:
		ret = ion_compat_legacy_alloc(client, data.legacy_alloc.len,
					      data.legacy_alloc.heap_id_mask,
					      data.legacy_alloc.flags);
		if (ret < 0)
			return ret;
		data.legacy_alloc.handle = ret;
		break;
	case ION_IOC_ALLOC:
		ret = ion_compat_alloc_fd(client->ion, &data.alloc);
		if (ret)
			return ret;
		break;
	case ION_IOC_FREE:
		return ion_compat_handle_put(client, data.handle.handle);
	case ION_IOC_MAP:
	case ION_IOC_SHARE:
		ret = ion_compat_share(client, &data.fd);
		if (ret)
			return ret;
		break;
	case ION_IOC_IMPORT:
		ret = ion_compat_import(client, &data.fd);
		if (ret)
			return ret;
		break;
	case ION_IOC_SYNC:
		return ion_compat_sync(&data.fd);
	case ION_IOC_HEAP_QUERY:
		ret = ion_compat_heap_query(client->ion, &data.query);
		if (ret)
			return ret;
		break;
	default:
		dev_warn_ratelimited(client->ion->dev,
				     "%s: unsupported ioctl 0x%x\n",
				     current->comm, cmd);
		return -ENOTTY;
	}

	if (copy_to_user(uarg, &data, _IOC_SIZE(cmd)))
		return -EFAULT;

	return 0;
}

#ifdef CONFIG_COMPAT
static long ion_compat_compat_ioctl(struct file *file, unsigned int cmd,
				    unsigned long arg)
{
	struct ion_compat_client *client = file->private_data;
	struct compat_ion_legacy_allocation_data data;
	void __user *uarg = compat_ptr(arg);
	int ret;

	/* Only the legacy allocation request differs in size. */
	if (cmd != COMPAT_ION_IOC_LEGACY_ALLOC)
		return ion_compat_ioctl(file, cmd, (unsigned long)uarg);

	if (copy_from_user(&data, uarg, sizeof(data)))
		return -EFAULT;

	ret = ion_compat_legacy_alloc(client, data.len, data.heap_id_mask,
				      data.flags);
	if (ret < 0)
		return ret;

	data.handle = ret;
	if (copy_to_user(uarg, &data, sizeof(data)))
		return -EFAULT;

	return 0;
}
#endif

static int ion_compat_open(struct inode *inode, struct file *file)
{
	struct ion_compat *ion = container_of(file->private_data,
					      struct ion_compat, misc);
	struct ion_compat_client *client;

	client = kzalloc(sizeof(*client), GFP_KERNEL);
	if (!client)
		return -ENOMEM;

	client->ion = ion;
	mutex_init(&client->lock);
	xa_init_flags(&client->handles, XA_FLAGS_ALLOC1);

	file->private_data = client;
	return nonseekable_open(inode, file);
}

static int ion_compat_release(struct inode *inode, struct file *file)
{
	struct ion_compat_client *client = file->private_data;
	struct ion_compat_handle *handle;
	unsigned long index;

	xa_for_each(&client->handles, index, handle) {
		dma_buf_put(handle->dmabuf);
		kfree(handle);
	}
	xa_destroy(&client->handles);
	mutex_destroy(&client->lock);
	kfree(client);

	return 0;
}

static const struct file_operations ion_compat_fops = {
	.owner = THIS_MODULE,
	.open = ion_compat_open,
	.release = ion_compat_release,
	.unlocked_ioctl = ion_compat_ioctl,
#ifdef CONFIG_COMPAT
	.compat_ioctl = ion_compat_compat_ioctl,
#endif
};

static int ion_compat_parse_heap(struct ion_compat *ion,
				 struct device_node *np,
				 struct ion_compat_heap *heap)
{
	struct device *dev = ion->dev;
	const char *type;
	u32 id;

	if (of_property_read_u32(np, "reg", &id) || id >= ION_NUM_HEAP_IDS)
		return dev_err_probe(dev, -EINVAL, "%pOF: invalid heap ID\n",
				     np);

	if (of_property_read_string(np, "qcom,ion-heap-type", &type))
		return dev_err_probe(dev, -EINVAL, "%pOF: missing heap type\n",
				     np);

	heap->id = id;
	heap->name = ion_msm_heap_names[id];

	if (!strcmp(type, "SYSTEM")) {
		heap->type = ION_HEAP_TYPE_SYSTEM;
		heap->dma_heap_name = "system";
	} else if (!strcmp(type, "DMA")) {
		struct device_node *rmem_np;
		struct reserved_mem *rmem;

		heap->type = ION_HEAP_TYPE_DMA;

		rmem_np = of_parse_phandle(np, "memory-region", 0);
		if (!rmem_np)
			return dev_err_probe(dev, -EINVAL,
					     "%pOF: missing memory-region\n",
					     np);
		/* CMA heaps are named after their reserved-memory node. */
		rmem = of_reserved_mem_lookup(rmem_np);
		of_node_put(rmem_np);
		if (!rmem) {
			dev_info(dev, "%pOF: memory-region not available\n",
				 np);
			return -ENODEV;
		}
		heap->dma_heap_name = rmem->name;
	} else {
		dev_info(dev, "%pOF: heap type %s not supported\n", np, type);
		return -ENODEV;
	}

	of_property_read_string(np, "linux,dma-heap-name",
				&heap->dma_heap_name);

	if (!heap->name)
		heap->name = heap->dma_heap_name;

	return 0;
}

static int ion_compat_heap_cmp(const void *a, const void *b)
{
	const struct ion_compat_heap *ha = a, *hb = b;

	return hb->id - ha->id;
}

static int ion_compat_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct ion_compat *ion;
	unsigned int i;
	int ret;

	ion = devm_kzalloc(dev, sizeof(*ion), GFP_KERNEL);
	if (!ion)
		return -ENOMEM;

	ion->dev = dev;
	ret = devm_mutex_init(dev, &ion->lock);
	if (ret)
		return ret;

	for_each_available_child_of_node_scoped(dev->of_node, np) {
		struct ion_compat_heap *heap = &ion->heaps[ion->num_heaps];

		if (ion->num_heaps == ION_NUM_HEAP_IDS)
			return dev_err_probe(dev, -EINVAL, "too many heaps\n");

		ret = ion_compat_parse_heap(ion, np, heap);
		if (ret == -ENODEV)
			continue;
		if (ret)
			return ret;

		for (i = 0; i < ion->num_heaps; i++)
			if (ion->heaps[i].id == heap->id)
				return dev_err_probe(dev, -EINVAL,
						     "duplicate heap ID %u\n",
						     heap->id);

		ion->num_heaps++;
	}

	sort(ion->heaps, ion->num_heaps, sizeof(ion->heaps[0]),
	     ion_compat_heap_cmp, NULL);

	for (i = 0; i < ion->num_heaps; i++)
		dev_dbg(dev, "heap %u (%s): DMA-BUF heap %s\n",
			ion->heaps[i].id, ion->heaps[i].name,
			ion->heaps[i].dma_heap_name);

	ion->misc.minor = MISC_DYNAMIC_MINOR;
	ion->misc.name = "ion";
	ion->misc.fops = &ion_compat_fops;
	ion->misc.parent = dev;

	ret = misc_register(&ion->misc);
	if (ret)
		return dev_err_probe(dev, ret, "failed to register /dev/ion\n");

	platform_set_drvdata(pdev, ion);
	return 0;
}

static void ion_compat_remove(struct platform_device *pdev)
{
	struct ion_compat *ion = platform_get_drvdata(pdev);

	misc_deregister(&ion->misc);
}

static const struct of_device_id ion_compat_of_match[] = {
	{ .compatible = "qcom,msm-ion" },
	{ }
};
MODULE_DEVICE_TABLE(of, ion_compat_of_match);

static struct platform_driver ion_compat_driver = {
	.probe = ion_compat_probe,
	.remove = ion_compat_remove,
	.driver = {
		.name = "ion-compat",
		.of_match_table = ion_compat_of_match,
	},
};
module_platform_driver(ion_compat_driver);

MODULE_DESCRIPTION("ION compatibility device on top of DMA-BUF heaps");
MODULE_LICENSE("GPL");
MODULE_IMPORT_NS("DMA_BUF");
MODULE_IMPORT_NS("DMA_BUF_HEAP");
