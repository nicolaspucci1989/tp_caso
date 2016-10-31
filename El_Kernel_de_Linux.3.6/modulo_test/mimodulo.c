#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/semaphore.h>
#include <asm/uaccess.h>


//(1) Crear una estructura para el dispositivo falso
struct fake_device {
	char data[100];
	struct semaphore sem;
} virtual_device;


//(2) Para luego registrar el dispositivo  es necesario un objeto cdev y otras variables
struct cdev *mcdev;
int major_number;
int ret;

dev_t dev_num;

#define DEVICE_NAME "nicosdevice"

//(7)llamada en device_file open
//		inode referencia al archivo en disco
//		 y contiene informacion acerca del archivo
//		struct file representa un open file abstracto 
int device_open(struct inode *inode, struct file *flip){

	//permitir que solo un proceso abra el dispositivo usando un semaforo como exclusion mutua
	if(down_interruptible(&virtual_device.sem) != 0){
		printk(KERN_ALERT "nicosdevice: no se pudo lockear el dispositivo durante el open");
		return -1;
	}

	printk(KERN_INFO "nicosdevice: dispositivo abierto");
	return 0;
}

//(8) called when user wants to get information from the device
ssize_t device_read(struct file* flip, char* bufStoreData, size_t bufCount, loff_t* curOffset) {
	//take data from kernel space(device) to user space (process)
	// copy_to_user (destination, source, sizeToTransfer)
	printk(KERN_INFO "nicosdevice: Reading from device");
	ret = copy_to_user(bufStoreData, virtual_device.data, bufCount);
	return ret;
}

//(9) called when user wants to send information to the device
ssize_t device_write(struct file* flip, const char* bufSourceData, size_t bufCount, loff_t* curOffset) {
	//setnd data from user to kernel
	//copy_from_user (dest, source, count)

	printk(KERN_INFO "nicosdevice: writing to device");
	ret = copy_from_user(virtual_device.data, bufSourceData, bufCount);
	return ret;
}

//(10) called upon user close
int device_close(struct inode *inode, struct file *flip) {
	
	//by calling up, which is opposite of down for semaphore, we release the mutex that we obtainde at device open
	//this has the effect of allowing other process to use the device now
	
	up(&virtual_device.sem);	
	printk(KERN_INFO "nicosdevice: closed device");
	return 0;
}
//(6) Decirle al kernel que funciones llamar cuando el usuario opera nuestro device file
struct file_operations fops = {
	.owner = THIS_MODULE,
	.open = device_open,
	.release = device_close,
	.write = device_write,
	.read = device_read
};

static int driver_entry(void) {
	//(3) Registrar nuestro dispositivo con el sistema: un proceso de dos pasos
	// paso(1) usar alocacion dinamica para asignar a nuestro dispositivo
	// un numero mayor-- alloc_chrdev_region(dev_t*, uint fminor, untint conunt, char* name)
	ret = alloc_chrdev_region(&dev_num, 0, 1, DEVICE_NAME);
	if(ret < 0) { // si devuelve negativo hay error
		printk(KERN_ALERT "nicosdevice: fallo al alocar numaro mayor");
		return ret; //propagar error
	}
	major_number = MAJOR(dev_num); //extrae el numero mayor y lo guarda en una variable
	printk(KERN_INFO "nicosdevice: el numero mayor es %d", major_number);
	printk(KERN_INFO "\tuse \"mknod /dev/%s c %d 0\"for device file", DEVICE_NAME, major_number); //dmesg

	//paso(2)
	mcdev = cdev_alloc(); //crear nuestra estructura cdev, inicializada 
	mcdev->ops = &fops;
	mcdev->owner = THIS_MODULE;

	//ahora que creamos cdev, tenemos que agregarlo al kernel
	//int cdev_add(struct cdev* dev, dev_t num, unsigned int count)
	ret = cdev_add(mcdev, dev_num, 1);
	if(ret < 0) { //checkeo de errores
		printk(KERN_ALERT "nicosdevice: no se puede agregar cdev al kernel");
		return ret;
	}

	//(4) Inicializar semaforo
	sema_init(&virtual_device.sem, 1); //valor inicial 1
	return 0;
}

static void driver_exit(void) {
	//(5) desregistrar todo en orden inverso
	//(a)
	cdev_del(mcdev);

	//(b)
	unregister_chrdev_region(dev_num, 1);
	printk(KERN_ALERT "nicosdevice: modulo descargado");
}

//Inform the kernel where to start and stop with our module/driver
module_init(driver_entry);
module_exit(driver_exit);
