#include <linux/module.h>
#include <linux/init.h>
#include <linux/input.h>
#include <linux/blkdev.h>
#include <linux/blk-mq.h>
#include <linux/bio.h>
#include <linux/vmalloc.h>
#include <linux/kobject.h>
#include <linux/sysfs.h>

#define WAYNE_BLK_SECTORS 2048
#define WAYNE_BLK_SIZE (WAYNE_BLK_SECTORS * 512)

static struct gendisk *wayne_blk_disk;
static struct blk_mq_tag_set wayne_blk_tag;
static struct input_dev *wayne_blk_dev;
static struct kobject *wayne_blk_kobj;
static u8 *wayne_blk_data;
static int wayne_blk_press;
static int wayne_blk_reads;
static int wayne_blk_writes;
static int wayne_blk_major;

static blk_status_t wayne_blk_queue_rq(struct blk_mq_hw_ctx *hctx, const struct blk_mq_queue_data *bd)
{
    struct request *rq = bd->rq;
    struct bio *bio = rq->bio;
    sector_t sector = bio->bi_iter.bi_sector;
    unsigned long offset = sector * 512;
    unsigned long pos = offset;
    struct bio_vec bvec;
    struct bvec_iter iter;

    blk_mq_start_request(rq);

    if (offset + bio->bi_iter.bi_size > WAYNE_BLK_SIZE) {
        blk_mq_end_request(rq, BLK_STS_IOERR);
        return BLK_STS_IOERR;
    }

    if (bio_data_dir(bio) == READ)
        wayne_blk_reads++;
    else
        wayne_blk_writes++;

    bio_for_each_segment(bvec, bio, iter) {
        void *buf = kmap_local_page(bvec.bv_page) + bvec.bv_offset;
        if (bio_data_dir(bio) == READ)
            memcpy(buf, wayne_blk_data + pos, bvec.bv_len);
        else
            memcpy(wayne_blk_data + pos, buf, bvec.bv_len);
        kunmap_local(buf);
        pos += bvec.bv_len;
    }

    blk_mq_end_request(rq, BLK_STS_OK);
    return BLK_STS_OK;
}

static const struct blk_mq_ops wayne_blk_mq_ops = {
    .queue_rq = wayne_blk_queue_rq,
};

static const struct block_device_operations wayne_blk_ops = {
    .owner = THIS_MODULE,
};

static ssize_t wayne_blk_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    return sysfs_emit(buf, "Wayne Block Paw - wayne0 - press %d - reads %d - writes %d\n", wayne_blk_press, wayne_blk_reads, wayne_blk_writes);
}

static ssize_t wayne_blk_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count)
{
    wayne_blk_press++;
    input_report_key(wayne_blk_dev, KEY_B, 1);
    input_sync(wayne_blk_dev);
    input_report_key(wayne_blk_dev, KEY_B, 0);
    input_sync(wayne_blk_dev);
    pr_info("WAYNE BLOCK 39.0: aperto %d - reads %d - writes %d - KEY_B - Wayne Block Paw\n", wayne_blk_press, wayne_blk_reads, wayne_blk_writes);
    return count;
}

static struct kobj_attribute wayne_blk_attr = __ATTR(press, 0664, wayne_blk_show, wayne_blk_store);

static int __init wayne_blk_init(void)
{
    int err;
    pr_info("WAYNE BLOCK 39.0: iniciando o block\n");

    wayne_blk_data = vzalloc(WAYNE_BLK_SIZE);
    if (!wayne_blk_data) {
        pr_info("WAYNE BLOCK 39.0: falhou vzalloc\n");
        return -ENOMEM;
    }
    pr_info("WAYNE BLOCK 39.0: passo 1 vzalloc ok\n");

    wayne_blk_major = register_blkdev(0, "wayne0");
    if (wayne_blk_major < 0) {
        pr_info("WAYNE BLOCK 39.0: falhou register_blkdev %d\n", wayne_blk_major);
        vfree(wayne_blk_data);
        return wayne_blk_major;
    }
    pr_info("WAYNE BLOCK 39.0: passo 2 register_blkdev ok major %d\n", wayne_blk_major);

    wayne_blk_dev = input_allocate_device();
    if (!wayne_blk_dev) {
        unregister_blkdev(wayne_blk_major, "wayne0");
        vfree(wayne_blk_data);
        return -ENOMEM;
    }

    wayne_blk_dev->name = "Wayne Block Paw";
    wayne_blk_dev->phys = "wayne/block0";
    wayne_blk_dev->id.bustype = BUS_VIRTUAL;
    wayne_blk_dev->id.vendor = 0x1234;
    wayne_blk_dev->id.product = 0x0039;
    wayne_blk_dev->id.version = 0x3900;

    __set_bit(EV_KEY, wayne_blk_dev->evbit);
    __set_bit(EV_SYN, wayne_blk_dev->evbit);
    __set_bit(KEY_B, wayne_blk_dev->keybit);

    err = input_register_device(wayne_blk_dev);
    if (err) {
        pr_info("WAYNE BLOCK 39.0: falhou input_register %d\n", err);
        input_free_device(wayne_blk_dev);
        unregister_blkdev(wayne_blk_major, "wayne0");
        vfree(wayne_blk_data);
        return err;
    }
    pr_info("WAYNE BLOCK 39.0: passo 3 input ok\n");

    memset(&wayne_blk_tag, 0, sizeof(wayne_blk_tag));
    wayne_blk_tag.ops = &wayne_blk_mq_ops;
    wayne_blk_tag.nr_hw_queues = 1;
    wayne_blk_tag.queue_depth = 16;
    wayne_blk_tag.numa_node = NUMA_NO_NODE;
    wayne_blk_tag.timeout = 30 * HZ;
    wayne_blk_tag.nr_maps = 1;

    err = blk_mq_alloc_tag_set(&wayne_blk_tag);
    if (err) {
        pr_info("WAYNE BLOCK 39.0: falhou alloc_tag_set %d\n", err);
        input_unregister_device(wayne_blk_dev);
        unregister_blkdev(wayne_blk_major, "wayne0");
        vfree(wayne_blk_data);
        return err;
    }
    pr_info("WAYNE BLOCK 39.0: passo 4 alloc_tag_set ok\n");

    wayne_blk_disk = blk_mq_alloc_disk(&wayne_blk_tag, NULL, NULL);
    if (IS_ERR(wayne_blk_disk)) {
        err = PTR_ERR(wayne_blk_disk);
        pr_info("WAYNE BLOCK 39.0: falhou alloc_disk %d\n", err);
        blk_mq_free_tag_set(&wayne_blk_tag);
        input_unregister_device(wayne_blk_dev);
        unregister_blkdev(wayne_blk_major, "wayne0");
        vfree(wayne_blk_data);
        return err;
    }
    pr_info("WAYNE BLOCK 39.0: passo 5 alloc_disk ok\n");

    wayne_blk_disk->major = wayne_blk_major;
    wayne_blk_disk->first_minor = 0;
    wayne_blk_disk->minors = 1;
    wayne_blk_disk->fops = &wayne_blk_ops;
    strcpy(wayne_blk_disk->disk_name, "wayne0");
    set_capacity(wayne_blk_disk, WAYNE_BLK_SECTORS);

    err = add_disk(wayne_blk_disk);
    if (err) {
        pr_info("WAYNE BLOCK 39.0: falhou add_disk %d\n", err);
        put_disk(wayne_blk_disk);
        blk_mq_free_tag_set(&wayne_blk_tag);
        input_unregister_device(wayne_blk_dev);
        unregister_blkdev(wayne_blk_major, "wayne0");
        vfree(wayne_blk_data);
        return err;
    }
    pr_info("WAYNE BLOCK 39.0: passo 6 add_disk ok\n");

    wayne_blk_press = 0;
    wayne_blk_reads = 0;
    wayne_blk_writes = 0;

    wayne_blk_kobj = kobject_create_and_add("wayne", kernel_kobj);
    if (wayne_blk_kobj) {
        err = sysfs_create_file(wayne_blk_kobj, &wayne_blk_attr.attr);
        if (err)
            pr_info("WAYNE BLOCK 39.0: sysfs falhou %d mas disco ok\n", err);
    }

    pr_info("WAYNE BLOCK 39.0: block criado - disco /dev/wayne0 - 1MB - Wayne Block Paw\n");
    return 0;
}

static void __exit wayne_blk_exit(void)
{
    if (wayne_blk_kobj) {
        sysfs_remove_file(wayne_blk_kobj, &wayne_blk_attr.attr);
        kobject_put(wayne_blk_kobj);
    }
    if (wayne_blk_disk) {
        del_gendisk(wayne_blk_disk);
        put_disk(wayne_blk_disk);
    }
    blk_mq_free_tag_set(&wayne_blk_tag);
    unregister_blkdev(wayne_blk_major, "wayne0");
    input_unregister_device(wayne_blk_dev);
    vfree(wayne_blk_data);
    pr_info("WAYNE BLOCK 39.0: block removido - press %d - reads %d - writes %d\n", wayne_blk_press, wayne_blk_reads, wayne_blk_writes);
}

module_init(wayne_blk_init);
module_exit(wayne_blk_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Wayne Lab");
MODULE_DESCRIPTION("Projeto 39 - WAYNE BLOCK - disco virtual");
