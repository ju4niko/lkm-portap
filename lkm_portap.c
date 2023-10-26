#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/cdev.h>
#include <linux/slab.h>

#define DEVICE_NAME "lkm_portap"
#define CLASS_NAME "lkm_portap_class"


MODULE_LICENSE("GPL");
MODULE_AUTHOR("J.A.M.");
MODULE_DESCRIPTION("Portapapeles de texto 4k");
MODULE_VERSION("1.0");

static int majorNumber;
static struct class* lkm_portap_class = NULL;
static struct device* lkm_portap_device = NULL;
static struct cdev lkm_cdev;
static char* data_buffer = NULL; // Búfer de datos en el kernel

static int lkm_portap_open(struct inode *inodep, struct file *filep) {
    printk(KERN_INFO "lkm_portap: Dispositivo abierto\n");


    return 0;
}
static ssize_t lkm_portap_read(struct file *filep, char *buffer, size_t len, loff_t *offset) {
    if (data_buffer == NULL || len == 0) {
        //  devuelve EOF
        return 0;
    }

    int bytes_to_copy = len < PAGE_SIZE ? len : PAGE_SIZE;
    
    if (copy_to_user(buffer, data_buffer, bytes_to_copy)) {
        return -EFAULT;
    }
    
    printk(KERN_INFO "lkm_portap: Llamada a read()\n");
    
    //  borra el contenido del buffer
    memset(data_buffer, 0, PAGE_SIZE);

    return bytes_to_copy;
}

static ssize_t lkm_portap_write(struct file *filep, const char *buffer, size_t len, loff_t *offset) {
    int bytes_to_copy = len < PAGE_SIZE ? len : PAGE_SIZE;
    if (copy_from_user(data_buffer, buffer, bytes_to_copy)) {
        return -EFAULT;
    }
    printk(KERN_INFO "lkm_portap: Llamada a write()\n");
    return bytes_to_copy;
}

static int lkm_portap_release(struct inode *inodep, struct file *filep) {
    printk(KERN_INFO "lkm_portap: Dispositivo cerrado\n");

    return 0;
}

static struct file_operations fops = {
    .open = lkm_portap_open,
    .read = lkm_portap_read,
    .write = lkm_portap_write,
    .release = lkm_portap_release,
    .owner = THIS_MODULE, 
};

static int __init lkm_portap_init(void) {
    printk(KERN_INFO "lkm_portap: Módulo del kernel inicializando\n");

    if (alloc_chrdev_region(&majorNumber, 0, 1, DEVICE_NAME) < 0) {
        printk(KERN_ALERT "lkm_portap: Fallo al asignar un número principal\n");
        return -1;
    }

    printk(KERN_INFO "lkm_portap: region registrada\n");

    lkm_portap_class = class_create(THIS_MODULE, CLASS_NAME);
    if (IS_ERR(lkm_portap_class)) {
        unregister_chrdev_region(majorNumber, 1);
        printk(KERN_ALERT "lkm_portap: No se pudo registrar la clase del dispositivo\n");
        return PTR_ERR(lkm_portap_class);
    }
    printk(KERN_INFO "lkm_portap: clase creada\n");
    cdev_init(&lkm_cdev, &fops);
    lkm_cdev.owner = THIS_MODULE;

    if (cdev_add(&lkm_cdev, majorNumber, 1) < 0) {
        device_destroy(lkm_portap_class, majorNumber);
        class_destroy(lkm_portap_class);
        unregister_chrdev_region(majorNumber, 1);
        printk(KERN_ALERT "lkm_portap: No se pudo agregar el dispositivo al kernel\n");
        return -1;
    }
    printk(KERN_INFO "lkm_portap: Módulo agregado al kernel\n");

    if (data_buffer == NULL) {
        data_buffer = kmalloc(PAGE_SIZE, GFP_KERNEL);
        if (!data_buffer) {
            printk(KERN_ALERT "lkm_portap: Fallo al asignar el búfer de datos, remueva con rmmod\n");
            return -ENOMEM;
        }
    }
    printk(KERN_INFO "lkm_portap: buffer reservado para portapapeles, tamanio: %d\n",PAGE_SIZE);

    return 0;
}

static void __exit lkm_portap_exit(void) {

    // libero la memoria que reserve para el buffer
    if (data_buffer) {
        kfree(data_buffer);
        data_buffer = NULL;
    }

    device_destroy(lkm_portap_class, MKDEV(majorNumber, 0));
    cdev_del(&lkm_cdev);
    class_destroy(lkm_portap_class);
    unregister_chrdev_region(majorNumber, 1);

    if (data_buffer) {
        kfree(data_buffer);
    }

    printk(KERN_INFO "lkm_portap: Módulo del kernel descargado\n");
}

module_init(lkm_portap_init);
module_exit(lkm_portap_exit);
