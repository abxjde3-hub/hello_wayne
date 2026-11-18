#include <linux/module.h>
#include <linux/init.h>
#include <linux/kobject.h>
#include <linux/sysfs.h>
#include <linux/string.h>
#include <linux/utsname.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Wayne");
MODULE_DESCRIPTION("Projeto 5 - WAYNE SYSFS - /sys/kernel/wayne/");

static struct kobject *wayne_kobj;
static char wayne_modo[32] = "normal";
static char wayne_mensagem[128] = "OLA DO SYSFS";
static int wayne_contador;

static ssize_t modo_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    return sysfs_emit(buf, "%s\n", wayne_modo);
}

static ssize_t modo_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count)
{
    size_t len = min(count, sizeof(wayne_modo) - 1);
    memcpy(wayne_modo, buf, len);
    wayne_modo[len] = '\0';
    if (len > 0 && wayne_modo[len - 1] == '\n')
        wayne_modo[len - 1] = '\0';
    wayne_contador++;
    pr_info("WAYNE SYSFS: modo mudou para %s\n", wayne_modo);
    return count;
}

static ssize_t mensagem_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    return sysfs_emit(buf, "%s\n", wayne_mensagem);
}

static ssize_t mensagem_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count)
{
    size_t len = min(count, sizeof(wayne_mensagem) - 1);
    memcpy(wayne_mensagem, buf, len);
    wayne_mensagem[len] = '\0';
    if (len > 0 && wayne_mensagem[len - 1] == '\n')
        wayne_mensagem[len - 1] = '\0';
    wayne_contador++;
    pr_info("WAYNE SYSFS: mensagem mudou para %s\n", wayne_mensagem);
    return count;
}

static ssize_t contador_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    return sysfs_emit(buf, "%d\n", wayne_contador);
}

static ssize_t versao_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    return sysfs_emit(buf, "WAYNE SYSFS 5.0 - Kernel %s\n", init_utsname()->release);
}

static struct kobj_attribute modo_attr = __ATTR(modo, 0664, modo_show, modo_store);
static struct kobj_attribute mensagem_attr = __ATTR(mensagem, 0664, mensagem_show, mensagem_store);
static struct kobj_attribute contador_attr = __ATTR(contador, 0444, contador_show, NULL);
static struct kobj_attribute versao_attr = __ATTR(versao, 0444, versao_show, NULL);

static struct attribute *wayne_attrs[] = {
    &modo_attr.attr,
    &mensagem_attr.attr,
    &contador_attr.attr,
    &versao_attr.attr,
    NULL,
};

static struct attribute_group wayne_group = {
    .attrs = wayne_attrs,
};

static int __init wayne_sysfs_init(void)
{
    int ret;
    wayne_kobj = kobject_create_and_add("wayne", kernel_kobj);
    if (!wayne_kobj)
        return -ENOMEM;
    ret = sysfs_create_group(wayne_kobj, &wayne_group);
    if (ret) {
        kobject_put(wayne_kobj);
        return ret;
    }
    wayne_contador = 0;
    pr_info("WAYNE SYSFS carregado - /sys/kernel/wayne/ criado\n");
    return 0;
}

static void __exit wayne_sysfs_exit(void)
{
    sysfs_remove_group(wayne_kobj, &wayne_group);
    kobject_put(wayne_kobj);
    pr_info("WAYNE SYSFS removido - contador final %d\n", wayne_contador);
}

module_init(wayne_sysfs_init);
module_exit(wayne_sysfs_exit);
