#include <linux/kernel.h>
#include <linux/linkage.h>
#include <linux/uaccess.h>


asmlinkage long sys_cs3753_add(int a, int b, int *ptr)
{
   int result = a+b;
   printk(KERN_ALERT "Adding %d to %d is %d \n", a,b,result);
   copy_to_user(ptr,&result,4);
   return a+b; 
}
