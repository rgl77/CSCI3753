#include <stdio.h>
#include <linux/kernel.h>
#include <sys/syscall.h>
#include <unistd.h>

int main(){
printf("Attention people, test file running! \n");
int pointer;
int a = 1;
int b = 2;
printf("Pointer value at first: %d with address %d \n",pointer,&pointer);
syscall(334,a,b,&pointer);
printf("Using copy_to_user, we've updated the address from kernel space\n");
printf("Adding %d to %d is %d and it is stored at %d  \n",a,b,pointer,&pointer);
}

