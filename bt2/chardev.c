#include <linux/module.h> // din nghia cac macro (module_init, module_exit) 
#include <linux/fs.h> // dinh nghia cac ham cap phat, giai phong device number 
#include <linux/device.h> // chua cac ham phuc vu viec tao device file
#include <linux/slab.h> // chua cac ham kmalloc va kfree
#include <linux/cdev.h> // chua cac ham lam viec voi charater device
#include <linux/uaccess.h> // chua cac ham thuc hien trao doi giua user va kernel
#include <linux/ioctl.h> // chua cac ham phuc vu iocontrol

#include "char_driver.h" // mo ta cac thanh ghi cua char_device

#define DRIVER_AUTHOR "ABC"
#define DRIVER_DESC "A sample character device driver"
#define DRIVER_VERSION "0.2"


#define MAGICAL_NUMBER 243
#define CHAR_CLR_DATA_REG _IO(MAGICAL_NUMBER, 0)
#define CHAR_GET_STS_REGS _IOR(MAGICAL_NUMBER, 1, sts_regs_t *)
#define CHAR_SET_RD_DATA_REGS _IOW(MAGICAL_NUMBER, 2, unsigned char *)
#define CHAR_SET_WR_DATA_REGS _IOW(MAGICAL_NUMBER, 3, unsigned char *)

typedef struct {
	unsigned char read_count_h_reg; 
	unsigned char read_count_l_reg; 
	unsigned char write_count_h_reg; 
	unsigned char write_count_l_reg; 
	unsigned char device_status_reg; 
} sts_regs_t; 


typedef struct char_dev {
	unsigned char *control_regs; 
	unsigned char *status_regs;
	unsigned char *data_regs; 

} char_dev_t; 


struct _char_drv {
	dev_t dev_num; 
	struct class *dev_class; 
	struct device *dev; 
	char_dev_t *char_hw;
	struct cdev *ccdev; 
	unsigned int open_cnt;  
} char_drv;


/***** DEVICE SPECIFIC *******/

/* khoi tao thiet bi*/
int char_hw_init(char_dev_t *hw) {
	char *buf; 
	buf = kzalloc(NUM_DEV_REGS*REG_SIZE, GFP_KERNEL); 
	if (!buf) {
		return -ENOMEM; 
	}	

	hw->control_regs = buf;
	hw->status_regs = hw->control_regs + NUM_CTRL_REGS; 
	hw->data_regs = hw->status_regs + NUM_STS_REGS; 

	// khoi tao gia tri ban dau cho cac thanh ghi
	hw->control_regs[CONTROL_ACCESS_REG] = 0x03; 
	hw->status_regs[DEVICE_STATUS_REG] = 0x03; 

	return 0; 
}
/* giai phong thiet bi*/
void char_hw_exit(char_dev_t *hw) {
	kfree(hw->control_regs); 
}

/* doc tu cac thanh ghi du lieu cua thiet bi*/
int char_hw_read_data(char_dev_t *hw, int start_reg, int num_regs, char* kbuf)
{	
	int read_bytes = num_regs; 

	// kiem tra quyen doc du lieu
	if ((hw->control_regs[CONTROL_ACCESS_REG] & CTRL_READ_DATA_BIT) == DISABLE) {
		return -1; 
	}
	// kiem tra dia chi kernel co hop le khong
	if(kbuf == NULL)
		return -1; 
	// kiem tra vi tri cua cac thanh ghi can doc co hop ly khong
	if (start_reg > NUM_DATA_REGS)
		return -1; 
	// dieu chinh lai so luong thanh ghi du lieu can doc
	if (num_regs > (NUM_DATA_REGS - start_reg))
		read_bytes = NUM_DATA_REGS - start_reg;
	// ghi du lieu tu kernel buffer vao cac thanh ghi du lieu
	memcpy(kbuf, hw->data_regs + start_reg, read_bytes);

	// cap nhat so lan doc tu cac thanh ghi du lieu
	hw->status_regs[READ_COUNT_L_REG] += 1; 
	if(hw->status_regs[READ_COUNT_L_REG] == 0)
		hw->status_regs[READ_COUNT_H_REG] += 1; 
	// tra ve so byte da doc duoc tu cac thanh ghi du lieu
	return read_bytes;  
}

/* ghi vao cac thanh ghi du lieu cua thiet bi*/
int char_hw_write_data(char_dev_t *hw, int start_reg, int num_regs, char* kbuf)
{
	int write_bytes = num_regs; 

	// kiem tra xem co quyen ghi du lieu khong 
	if ((hw->control_regs[CONTROL_ACCESS_REG] & CTRL_WRITE_DATA_BIT) == DISABLE) 
		return -1; 

	// kiem tra dia chi cua kernel buffer co hop le khong 
	if (kbuf == NULL) 
		return -1; 
	// kiem tra vi tri cac thanh can ghi co hop ly khong 
	if (start_reg > NUM_DATA_REGS)
		return -1; 
	// dieu chinh lai so luong thanh ghi du lieu can ghi 
	if (num_regs > (NUM_DATA_REGS - start_reg)) {
		write_bytes = NUM_DATA_REGS - start_reg; 
		hw->status_regs[DEVICE_STATUS_REG] = STS_DATAREGS_OVERFLOW_BIT; 	
	}
	// doc du lieu tu cac thanh ghi du lieu vao kernel buffer 
	memcpy(hw->data_regs + start_reg, kbuf, write_bytes); 
	// cap nhat so lan ghi vao cac thanh ghi du lieu
	hw->status_regs[WRITE_COUNT_L_REG] += 1; 
	if (hw->status_regs[WRITE_COUNT_L_REG == 0]) 
		hw->status_regs[WRITE_COUNT_H_REG] += 1; 

	// tra ve so byte da ghi vao cac thanh ghi du lieu 
	return write_bytes; 

}
int char_hw_clear_data(char_dev_t *hw) {
	if ((hw->control_regs[CONTROL_ACCESS_REG] & CTRL_WRITE_DATA_BIT) == DISABLE)
		return -1; 
	memset(hw->data_regs, 0, NUM_DATA_REGS*REG_SIZE); 
	hw->status_regs[DEVICE_STATUS_REG] &= ~STS_DATAREGS_OVERFLOW_BIT; 

	return 0; 
}
/* doc tu cac thanh ghi trang thai cua thiet bi*/
void char_hw_get_status(char_dev_t *hw, sts_regs_t*status) {
	memcpy(status, hw->status_regs, NUM_STS_REGS*REG_SIZE); 
}
/* ghi vao cac thanh ghi trang thai cua thiet bi*/
// cho phep doc tu cac thanh ghi du lieu cua thiet bi 
void char_hw_enable_read(char_dev_t *hw, unsigned char isEnable) {

	if (isEnable == ENABLE) {
		// dieu khien cho phep doc tu cac thanh ghi du lieu 
		hw->control_regs[CONTROL_ACCESS_REG] |= CTRL_READ_DATA_BIT; 
		// cap nhat trang thai "co the doc"
		hw->status_regs[DEVICE_STATUS_REG] |= STS_READ_ACCESS_BIT; 
	}
	else {
		// dieu khien khong cho phep doc tu cac thanh ghi du lieu 
		hw->control_regs[CONTROL_ACCESS_REG] &= ~CTRL_READ_DATA_BIT; 
		// cap nhat trang thai khong doc 
		hw->status_regs[DEVICE_STATUS_REG] &= ~STS_READ_ACCESS_BIT; 
	}
}

// ham cho phep ghi vao cac thanh ghi du lieu cua mot thiet bi 
void char_hw_enable_write(char_dev_t *hw, unsigned char isEnable) {
	if (isEnable == ENABLE) {
		// dieu khien cho phep ghi vao cac thanh ghi du lieu
		hw->control_regs[CONTROL_ACCESS_REG] |= CTRL_WRITE_DATA_BIT;
		// cap nhat trang thai "co the ghi"
		hw->status_regs[DEVICE_STATUS_REG] |= STS_WRITE_ACCESS_BIT;  
	}
	else {
		// dieu khien khong cho ghi vao cac thanh ghi du lieu 
		hw->control_regs[CONTROL_ACCESS_REG] &= ~CTRL_WRITE_DATA_BIT; 
		// cap nhat trang thai "khong the ghi"
		hw->status_regs[DEVICE_STATUS_REG] &= ~STS_WRITE_ACCESS_BIT; 
	}
}
/* doc tu cac thanh ghi dieu khien cua thiet bi*/
/* ghi vao cac thanh ghi dieu khien cua thiet bi*/
/* xu ly tin hieu ngat tu thiet bi */

/***** OS SPECIFIC *******/
/* cac ham entry points */
static int char_driver_open(struct inode* inode, struct file *filp) {
	char_drv.open_cnt++; 
	printk("Handle opened even (%d)\n", char_drv.open_cnt); 
	return 0; 
}

static int char_driver_release(struct inode* inode, struct file *filp) {
	 
	printk("Handle closed event\n"); 
	return 0; 
}

static ssize_t char_driver_read(struct file *filp, char __user *user_buf, size_t len, loff_t *off){
	char *kernel_buf = NULL; 
	int num_bytes = 0; 

	printk("Handle read event start from %lld, %zu bytes\n", *off, len); 

	kernel_buf = kzalloc(len, GFP_KERNEL); 
	if (kernel_buf == NULL) {
		return 0; 
	}
	num_bytes = char_hw_read_data(char_drv.char_hw, *off, len, kernel_buf); 
	printk("read %d bytes from HW\n", num_bytes); 

	if (num_bytes < 0)
		return -EFAULT; 
	if (copy_to_user(user_buf, kernel_buf, num_bytes))
		return -EFAULT; 

	*off += num_bytes; 
	return num_bytes; 

}

static ssize_t char_driver_write(struct file *filp, const char __user *user_buf, size_t len, loff_t *off) 
{
	char *kernel_buf = NULL; 
	int num_bytes = 0; 
	printk("Handle write event start from %lld, %zu bytes\n", *off, len); 

	kernel_buf = kzalloc(len, GFP_KERNEL); 
	if (copy_from_user(kernel_buf, user_buf, len))
		return -EFAULT; 

	num_bytes = char_hw_write_data(char_drv.char_hw, *off, len, kernel_buf); 
	printk("writes %d bytes to HW\n", num_bytes); 

	if (num_bytes < 0)
		return -EFAULT; 

	*off += num_bytes; 
	return num_bytes; 
}

static long char_driver_ioctl(struct file *filp, unsigned int cmd, unsigned long arg) {
	int ret = 0; 
	printk("Handle ioctl event (cmd: %u)\n", cmd); 
	switch(cmd) {
		case CHAR_CLR_DATA_REG: 
		{
			ret = char_hw_clear_data(char_drv.char_hw); 
			if (ret < 0) {
				printk("Can not clear data register\n"); 
			}
			else {
				printk("Data registers have been cleared\n"); 
			}

		}
		break; 
		case CHAR_SET_RD_DATA_REGS: 
		{
			unsigned char isReadEnable; 
			copy_from_user(&isReadEnable, (unsigned char *)arg, sizeof(isReadEnable)); 
			char_hw_enable_read(char_drv.char_hw, isReadEnable); 
			printk("Data registers have been %s to read\n", (isReadEnable == ENABLE)? "enabled": "disabled"); 
		}
		break; 
		case CHAR_SET_WR_DATA_REGS: 
		{
			unsigned char isWriteEnable; 
			copy_from_user(&isWriteEnable, (unsigned char*)arg, sizeof(isWriteEnable)); 
			char_hw_enable_write(char_drv.char_hw, isWriteEnable); 
			printk("Data registers have been %s to write", (isWriteEnable == ENABLE)? "enabled": "disabled"); 
		}
		break; 
		case CHAR_GET_STS_REGS: 
		{
			sts_regs_t status; 
			char_hw_get_status(char_drv.char_hw, &status); 
			copy_to_user((sts_regs_t*)arg, &status, sizeof(status)); 
			printk("Got information from status registers"); 

		}
		break; 
	}
	return ret; 
}

static struct file_operations fops = {
	.owner   = THIS_MODULE,
	.open    = char_driver_open,
	.release = char_driver_release,
	.read    = char_driver_read,
	.write   = char_driver_write,
	.unlocked_ioctl = char_driver_ioctl,
}; 

/* ham khoi tao driver */
static int __init char_driver_init(void) 
{
	int ret = 0; 

	/* cap phat device number */
	// char_drv.dev_num = MKDEV(235, 0);
	// ret = register_chrdev_region(char_drv.dev_num, 1, "char_device");  
	ret = alloc_chrdev_region(&char_drv.dev_num, 0, 1, "char_device"); 
	if (ret < 0) {
		printk("failed to register device number statically\n"); 
		goto failed_register_devnum; 
	}
	
	printk("allocated device number (%d,%d)\n", MAJOR(char_drv.dev_num), MINOR(char_drv.dev_num)); 

	/* tao device file */
	char_drv.dev_class = class_create(THIS_MODULE, "class_char_dev"); 
	if (char_drv.dev_class == NULL) {
		printk("failed to create a device class\n"); 
		goto failed_create_class; 
	}
	char_drv.dev = device_create(char_drv.dev_class, NULL, char_drv.dev_num, NULL, "char_dev"); 
	if (IS_ERR(char_drv.dev)) {
		printk("failed to create a device\n"); 
		goto failed_create_device; 
	}

	/* cap phat bo nho cho cac cau truc du lieu cua driver vaf khoi tao*/
	char_drv.char_hw = kzalloc(sizeof(char_dev_t), GFP_KERNEL); 
	if (!char_drv.char_hw) {
		printk("failed to allocate data structure of the driver\n"); 
		ret = -ENOMEM; 
		goto failed_allocate_structure; 
	}

	/* khoi tao thiet bi vat ly */
	ret = char_hw_init(char_drv.char_hw); 
	if (ret < 0) {
		printk("failed to initialize a virtual character device\n"); 
		goto failed_init_hw; 
	}

	/* dang ki entry points voi kernel*/
	char_drv.ccdev = cdev_alloc(); 
	if (char_drv.ccdev == NULL) {
		printk("failed to allocate cdev structure\n"); 
		goto failed_allocate_cdev; 
	}
	cdev_init(char_drv.ccdev, &fops); 
	ret = cdev_add(char_drv.ccdev, char_drv.dev_num, 1); 
	if (ret < 0) {
		printk("failed to add a char device to the system\n"); 
		goto failed_allocate_cdev; 
	}

	/* dang ki ham xu ly ngat*/

	printk("Initialize chardev successfully\n");
	return 0;

failed_allocate_cdev: 
	char_hw_exit(char_drv.char_hw); 
failed_init_hw: 
	kfree(char_drv.char_hw); 
failed_allocate_structure:
	device_destroy(char_drv.dev_class, char_drv.dev_num); 
failed_create_device: 
	class_destroy(char_drv.dev_class); 
failed_create_class: 
	unregister_chrdev_region(char_drv.dev_num, 1); 
failed_register_devnum:
	return ret; 
}

/* ham ket thuc driver */
static void __exit char_driver_exit(void)
{
	/* huy dang ky xu ly ngat*/

	/* huy dang ky entry point voi kernel*/
	cdev_del(char_drv.ccdev); 
	/* giai phong thiet bi vat ly*/
	char_hw_exit(char_drv.char_hw); 
	
	/* giai phong bo nho da cap phat cau truc du lieu cua driver*/
	kfree(char_drv.char_hw); 

	/* xoa bo device file*/
	device_destroy(char_drv.dev_class, char_drv.dev_num); 
	class_destroy(char_drv.dev_class); 
	/* giai phong device number*/
	unregister_chrdev_region(char_drv.dev_num, 1); 

	printk("Exit chardev driver\n"); 
}

module_init(char_driver_init); 
module_exit(char_driver_exit); 

MODULE_LICENSE("GPL"); 
MODULE_AUTHOR(DRIVER_AUTHOR); 
MODULE_DESCRIPTION(DRIVER_DESC); 
MODULE_SUPPORTED_DEVICE("test device"); 