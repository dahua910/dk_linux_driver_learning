//#include <stdio.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/fs.h>
#include <linux/errno.h>
#include <linux/mm.h>
#include <linux/sched.h>
#include <linux/cdev.h>
#include <linux/init.h>
#include <asm/io.h>
#include <asm/system.h>
#include <asm/uaccess.h>

#define GLOBALMEM_SIZE 0x1000
#define GLOBALMEM_MAJOR 250

struct globalmem_dev
{
	struct cdev cdev;
	unsigned char mem[GLOBALMEM_SIZE];
};

struct globalmem_dev dk_globalmem_cdev;
static const struct file_operations globalmem_fops ={
	.owner = THIS_MODULE,
	.llseek = globalmen_llseek,
	.read = globalmem_read,
	.write = globalmem_write,
	.iotl = globalmem_ioctl,
};

//struct globalmem_dev dev;

int globalmem_init(void)
{
	printk(KERN_NOTICE "\n dk debug: start globalmem_init \n");
	int result;
	dev_t devno = MKDEV(GLOBALMEM_MAJOR,0);
	if(GLOBALMEM_MAJOR){
		result= register_chrdev_region(devno,1,"GLOBALMEM");}
	else{
		result= alloc_chrdev_region(&devno,0,1,"GLOBALMEM");}
	if(result<0){
		return result;}
	globalmem_setup_cdev();
	printk(KERN_NOTICE "\n dk debug: globalmem finish!!!\n");
	return 0;
}


static void globalmem_setup_cdev()
{	
	int err;
	int devno=MKDEV(GLOBALMEM_MAJOR,0);
	cdev_init(&dk_globalmem_cdev.cdev, &globalmem_fops);
	dk_globalmem_cdev.cdev.owner = THIS_MODULE;
	
	err=cdev_add(&globalmem_cdev.cdev,devno,1);
	if(err){printk(KERN_NOTICE "erro %d adding dk_globalmem",err);}
}


int globalmem_exit(void)
{
	cdev_del(&dev.cdev);
	unregister_chrdev_region(MKDEV(GLOBALMEM_MAJOR,0),1);
}

static ssize_t globalmem_read(struct file *filp,char __user *buf,size_t count,loff_t *ppos)
{
	unsigned long p = *ppos;
	int ret =0;
	if(p >= GLOBALMEM_SIZE){return 0;}
	if(count > GLOBALMEM_SIZE-p){count = GLOBALMEM_SIZE-p;}
}


module_init(globalmem_init);
module_exit(globalmem_exit);




