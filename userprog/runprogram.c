/*
 * Sample/test code for running a user program.  You can use this for
 * reference when implementing the execv() system call. Remember though
 * that execv() needs to do more than this function does.
 */

#include <types.h>
#include <kern/unistd.h>
#include <kern/errno.h>
#include <lib.h>
#include <addrspace.h>
#include <thread.h>
#include <curthread.h>
#include <vm.h>
#include <vfs.h>
#include <test.h>

/*
 * Load program "progname" and start running it in usermode.
 * Does not return except on error.nargs
 *
 * Calls vfs_open on progname and thus may destroy it.
 */
int
runprogram(char *progname,char ** args, int nargs)
{
	struct vnode *v;
	vaddr_t entrypoint, stackptr;
	int result;

	/* Open the file. */
	result = vfs_open(progname, O_RDONLY, &v);
	if (result) {
		return result;
	}

	/* We should be a new thread. */
	assert(curthread->t_vmspace == NULL);

	/* Create a new address space. */
	curthread->t_vmspace = as_create();
	if (curthread->t_vmspace==NULL) {
		vfs_close(v);
		return ENOMEM;
	}

	/* Activate it. */
	as_activate(curthread->t_vmspace);

	/* Load the executable. */
	result = load_elf(v, &entrypoint);
	if (result) {
		/* thread_exit destroys curthread->t_vmspace */
		vfs_close(v);
		return result;
	}

	/* Done with the file now. */
	vfs_close(v);

	/* Define the user stack in the address space */
	result = as_define_stack(curthread->t_vmspace, &stackptr);
	if (result) {
		/* thread_exit destroys curthread->t_vmspace */
		return result;
	}
        
	
        int i;
	userptr_t tmpptr[nargs];
	for(i=nargs-1;i>=0;i--){
	int sizeStack= 1+strlen(args[i]);
	int remainder, quotient;
	remainder = sizeStack%4;
	quotient = sizeStack/4;
	if(remainder > 0)
	   quotient=quotient +1;
	sizeStack=quotient*4;	
	stackptr = stackptr-sizeStack;
	tmpptr[i]=stackptr;
	result=copyoutstr(args[i],(userptr_t)stackptr,sizeStack,NULL);
	if(result)
		return result;	
        //kprintf("copied arguments: %s\n", args[i]);
	}

	stackptr=stackptr-4;
	char ptr123='\0';
	result=copyout(&ptr123,stackptr,4);
	if(result)
	return result;

	for(i=nargs-1;i>=0;i--){
		stackptr=stackptr-4;
		copyout(tmpptr+i,stackptr,4);	
	}
	md_usermode(nargs ,stackptr , stackptr, entrypoint);
	/* Warp to user mode. */
	//md_usermode(0 /*argc*/, NULL /*userspace addr of argv*/,
	//	    stackptr, entrypoint);
	
	/* md_usermode does not return */
	panic("md_usermode returned\n");
	return EINVAL;
}

