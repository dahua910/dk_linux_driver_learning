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
#define MEM_CLEAR 0x00

/*globalmem device struct*/
struct globalmem_dev
{
	struct cdev cdev;  /*char device struct*/
	unsigned char mem[GLOBALMEM_SIZE];  /*global mem*/
};

int globalmem_init(void);   /*init function for globalmen device*/
int globalmem_exit(void);   /*exit function for globalmen device*/
static ssize_t globalmem_read(struct file *filp,char __user *buf,size_t count,loff_t *ppos); /*read function*/
static ssize_t globalmem_write(struct file *filp,const char __user *buf,size_t count,loff_t *ppos); /*write function*/
static loff_t globalmen_llseek(struct file *filp, loff_t offset, int orig); /*llseek function,repositions the offset of the file*/
static int globalmem_ioctl(struct inode *inodep, struct file *filp,unsigned int cmd, unsigned long arg);    /*ioctl function, MEM clear only*/
static void globalmem_setup_cdev(); /*init and add cdev */


struct globalmem_dev dk_globalmem_cdev;

static const struct file_operations globalmem_fops ={
	.owner = THIS_MODULE,
	.llseek = globalmen_llseek,
	.read = globalmem_read,
	.write = globalmem_write,
	.ioctl = globalmem_ioctl,
	.open = globalmem_open,
	.release = globalmem_release,
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

	err=cdev_add(&dk_globalmem_cdev.cdev,devno,1);
	if(err){printk(KERN_NOTICE "erro %d adding dk_globalmem",err);}
}


int globalmem_exit(void)
{
	cdev_del(&dk_globalmem_cdev.cdev);
	unregister_chrdev_region(MKDEV(GLOBALMEM_MAJOR,0),1);
}

static ssize_t globalmem_read(struct file *filp,char __user *buf,size_t count,loff_t *ppos)
{
	unsigned long p = *ppos;
	int ret =0;
	if(p >= GLOBALMEM_SIZE){return 0;}
	if(count > GLOBALMEM_SIZE-p){count = GLOBALMEM_SIZE-p;}
	if(copy_to_user(buf,(void*)(dk_globalmem_cdev.mem + p),count)){ret = - EFAULT;}
	else{
	    *ppos += count;
	    ret = count;
	    printk(KERN_INFO "read %d bytes from %d\n",count, p);}
    return ret;
}

static ssize_t globalmem_write(struct file *filp,const char __user *buf,size_t count,loff_t *ppos)
{
    unsigned long p = *ppos;
	int ret =0;
	if(p >= GLOBALMEM_SIZE){return 0;}
	if(count > GLOBALMEM_SIZE-p){count = GLOBALMEM_SIZE-p;}
	if(copy_from_user((dk_globalmem_cdev.mem + p),buf,count)){ret = - EFAULT;}
	else{
	    *ppos += count;
	    ret = count;
	    printk(KERN_INFO "write %d bytes from %d\n",count, p);}
    return ret;
}


static loff_t globalmen_llseek(struct file *filp, loff_t offset, int orig)
{
    loff_t ret;
    switch(orig){
    case 0:
        if(offset < 0){
            ret = -EINVAL;
            break;}
        if((unsigned int)offset > GLOBALMEM_SIZE){
            ret = -EINVAL;
            break;}
        filp->f_pos = (unsigned int)offset;
        ret = filp->f_pos;
        break;

    case 1:
        if((filp->f_pos + offset)>GLOBALMEM_SIZE){
            ret = -EINVAL;
            break;}
        filp->f_pos += offset;
        ret = filp->f_pos;
        break;
    default:
        ret = -EINVAL;}

    return ret;
}

static int globalmem_ioctl(struct inode *inodep, struct file *filp,unsigned int cmd, unsigned long arg)
{
    switch(cmd){
    case MEM_CLEAR:
        memset(dk_globalmem_cdev.mem, 0, GLOBALMEM_SIZE);
        printk(KERN_INFO "globalmem is set to all zero\n");
        break;

    default:
        return -EINVAL;
    }
    return 0;
}


module_init(globalmem_init);
module_exit(globalmem_exit);




