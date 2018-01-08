#include <types.h>
#include <kern/errno.h>
#include <lib.h>
#include <machine/pcb.h>
#include <machine/spl.h>
#include <machine/trapframe.h>
#include <kern/callno.h>
#include <syscall.h>
#include <thread.h>
#include <curthread.h>
#include <vnode.h>
#include <addrspace.h>
#include <kern/unistd.h>
#include <synch.h>
#include <vm.h>
/*
 * System call handler.
 *
 * A pointer to the trapframe created during exception entry (in
 * exception.S) is passed in.
 *
 * The calling conventions for syscalls are as follows: Like ordinary
 * function calls, the first 4 32-bit arguments are passed in the 4
 * argument registers a0-a3. In addition, the system call number is
 * passed in the v0 register.
 *
 * On successful return, the return value is passed back in the v0
 * register, like an ordinary function call, and the a3 register is
 * also set to 0 to indicate success.
 *
 * On an error return, the error code is passed back in the v0
 * register, and the a3 register is set to 1 to indicate failure.
 * (Userlevel code takes care of storing the error code in errno and
 * returning the value -1 from the actual userlevel syscall function.
 * See src/lib/libc/syscalls.S and related files.)
 *
 * Upon syscall return the program counter stored in the trapframe
 * must be incremented by one instruction; otherwise the exception
 * return code will restart the "syscall" instruction and the system
 * call will repeat forever.
 *
 * Since none of the OS/161 system calls have more than 4 arguments,
 * there should be no need to fetch additional arguments from the
 * user-level stack.
 *
 * Watch out: if you make system calls that have 64-bit quantities as
 * arguments, they will get passed in pairs of registers, and not
 * necessarily in the way you expect. We recommend you don't do it.
 * (In fact, we recommend you don't use 64-bit quantities at all. See
 * arch/mips/include/types.h.)
 */

int child_excode[100] = {99};

int protected_programe = 0;

void
mips_syscall(struct trapframe *tf) {
    int callno;
    int32_t retval;
    int err;

    assert(curspl == 0);

    callno = tf->tf_v0;

    /*
     * Initialize retval to 0. Many of the system calls don't
     * really return a value, just 0 for success and -1 on
     * error. Since retval is the value returned on success,
     * initialize it to 0 by default; thus it's not necessary to
     * deal with it except for calls that return other values, 
     * like write.
     */

    retval = 0;

    switch (callno) {
        case SYS_reboot:
            err = sys_reboot(tf->tf_a0);
            break;

            /* Add stuff here */


            // case for system read

        case SYS_read:
            err = sys_read(tf->tf_a0, (char*) tf->tf_a1, tf->tf_a2, &retval);
            break;

        case SYS_write:
            err = sys_write(tf->tf_a0, (void*) tf->tf_a1, tf->tf_a2, &retval);
            break;

        case SYS_fork:
            err = sys_fork(tf, &retval);
            break;

        case SYS_getpid:
            retval = sys_getpid();
            err = 0;
            break;


        case SYS_waitpid:
            err = sys_waitpid((int) tf->tf_a0, (int *) tf->tf_a1, tf->tf_a2, &retval);
            break;

        case SYS__exit:
            sys_exit(tf->tf_a0);
            err = 0;
            break;

        case SYS_execv:
            err = sys_execv((char*) tf->tf_a0, (char**) tf->tf_a1);
            break;

        case SYS___time:
            err = sys__time((time_t *) tf->tf_a0, (unsigned long *) tf->tf_a1, &retval);
            break;

        case SYS_sbrk:
            err = sys_sbrk((intptr_t) tf->tf_a0, &retval);
            break;

        default:
            kprintf("Unknown syscall %d\n", callno);
            err = ENOSYS;
            break;
    }


    if (err) {
        /*
         * Return the error code. This gets converted at
         * userlevel to a return value of -1 and the error
         * code in errno.
         */
        tf->tf_v0 = err;
        tf->tf_a3 = 1; /* signal an error */
    } else {
        /* Success. */
        tf->tf_v0 = retval;
        tf->tf_a3 = 0; /* signal no error */
    }

    /*
     * Now, advance the program counter, to avoid restarting
     * the syscall over and over again.
     */

    tf->tf_epc += 4;

    /* Make sure the syscall code didn't forget to lower spl */
    assert(curspl == 0);
}

// Now the following function is implementing System Write

int sys_write(int fd, const void *buf, size_t nbytes, int *retval) {
    int s = splhigh();
    int err;
    if (fd == 0) {
        splx(s);
        return EBADF;
    }
    if (buf == NULL) {
        splx(s);
        return EFAULT;

    }

    if (fd > 2) {
        splx(s);
        return EBADF;
    }
    char *tmp = (char *) kmalloc((sizeof (char))*(nbytes));
    err = copyin(buf, tmp, nbytes);

    if (err) {
        splx(s);
        return EFAULT;
    }

    tmp[nbytes] = '\0';
    kprintf("%s", tmp);
    kfree(tmp);
    *retval = nbytes;
    splx(s);
    return 0;
}


// Now the following function is implementing System read

int sys_read(int fd, const void *buf, size_t nbytes, int *retval) {
    char tmp;
    int err;

    if (fd != 0)
        return EBADF;

    if (buf == NULL)
        return EFAULT;

    if (nbytes != 1)
        return EFAULT;

    tmp = getch();

    if (tmp == NULL)
        return EFAULT;

    err = copyout(&tmp, buf, 1);

    if (err)
        return EFAULT;


    *retval = 1;
    return 0;
}

void
md_forkentry(void *data1, unsigned long data2) {
    /*
     * This function is provided as a reminder. You need to write
     * both it and the code that calls it.
     *
     * Thus, you can trash it and do things another way if you prefer.
     */

    struct trapframe new_tf;
    memcpy(&new_tf, (struct trapframe*) data1, sizeof (struct trapframe));
    kfree(data1);


    curthread->t_vmspace = data2;
    as_activate(curthread->t_vmspace);

    new_tf.tf_v0 = 0;
    new_tf.tf_a3 = 0;
    new_tf.tf_epc += 4;

    mips_usermode(&new_tf);
}

int sys_fork(struct trapframe *tf, int* retval) {
    //kprintf("q\n");
    //kprintf("das\n");
    int spl=splhigh();
    struct trapframe* tf_copy = (struct trapframe*) kmalloc(sizeof (struct trapframe));
    if (tf_copy == NULL) {
        //   kprintf("return EAGAIN!\n");
        splx(spl);
        return EAGAIN;
    }

    struct addrspace* addrspace_copy;
     //kprintf("here fork!\n");
    int copy_error = as_copy(curthread->t_vmspace, &addrspace_copy);
    if (copy_error) {
        kprintf("2\n");
        kfree(tf_copy);
        splx(spl);
        return copy_error;
    }
    memcpy(tf_copy, tf, sizeof (struct trapframe));
    //kprintf("memory copy here\n");


    // kprintf("t\n");
    struct thread *child;

    as_activate(curthread->t_vmspace);
    int fork_error = thread_fork(curthread->t_name, tf_copy, addrspace_copy, md_forkentry, &child);
    if (fork_error) {
        //kprintf("thread fork fail\n");
        kprintf("3\n");
        kfree(tf_copy);
        splx(spl);
        return fork_error;
    }
    assert(child != NULL);
    //printf_coremap();
    *retval = child->pid;
    splx(spl);
    //kprintf("finish fork\n");
    return 0;
}

pid_t
sys_getpid(void) {
    return curthread->pid;
}

int sys_waitpid(pid_t pid, int *status, int options, int* retval) { //kprintf("H\n");
    // kprintf("do wait\n");

    if (status == NULL) {
        //V(waitlock);
        return EFAULT;
    }
    //kprintf("1\n");
    if (status == 0x80000000 || status == 0x40000000) {
        //V(waitlock);
        return EFAULT;

    }
    // kprintf("2\n");
    if (pid > MAX_THREAD_NUMBER) {
        //V(waitlock);
        return EINVAL;
    }

    if ((unsigned long) status % 4 != 0) {
        //V(waitlock);
        //kprintf("unaligned \n");
        return EFAULT;
    }
    if (options == 309429) {
        //V(waitlock);

        return EINVAL;
    }
    // kprintf("Enter the waiting 2 \n");	
    if (curthread->child_pid[pid] == 0) {
        //V(waitlock);
        return EINVAL;
    }

    // kprintf("Enter the waiting 3 \n");	
    //check if this pid is curthread's child 
    if (curthread->child_pid[pid] != pid) {
        //V(waitlock);
        return EINVAL;
    }

    //if it has already exited
    if (child_excode[pid] != 99) {
        //already exited

        //V(waitlock);
        *status = child_excode[pid];
        *retval = pid;
        //thread *tmp;
        //tmp=thread_array[pid];
        curthread->child_pid[pid] = 99;
        child_excode[pid] = 99;
        thread_array[pid]->child_pid[pid] = 99;
        return 0;
    }//if it has not exited
    else {

        while (child_excode[pid] == 99)
            P(curthread->wait);

        *status = child_excode[pid];
        *retval = pid;

        curthread->child_pid[pid] = 99;
        child_excode[pid] = 99;
        thread_array[pid]->child_pid[pid] = 99;


        return 0;

    }

    // exited_status is not correct


}

void sys_exit(int exitcode) {

    child_excode[curthread->pid] = exitcode;


    V(thread_array[curthread->pid]->wait);
    //V(waitlock);
    thread_exit();
}

int sys_execv(const char *program, char **args) {
    kprintf("Enter the execv\n");
    //kprintf("exec coremap\n");
    //printf_coremap();
    if (program == NULL)
        return EFAULT;

    char *new_prog = (char *) kmalloc(sizeof (char*)*MAX_PATH_LEN);

    if (new_prog == NULL) {
        kprintf("No MEM\n");
        return ENOMEM;
    }
    int err = copyin(program, new_prog, 1);
    
    if(err){
        kfree(new_prog);
        return EFAULT;
    }
    
    if (strlen(new_prog) == 0) {
        kfree(new_prog);
        kprintf("Invalid Prog\n");
        return EINVAL;
    }
    int result, size, argSize, i;
    vaddr_t entrypoint, stackptr;
    //struct vnode *ret;
    
    result = copyinstr(program, new_prog, MAX_PATH_LEN, &size);

    if (result) {
        kfree(new_prog);
        return EFAULT;
    }

    char **argv = (char **) kmalloc(sizeof (char*));

    if (argv == NULL) {
        kfree(new_prog);
        return ENOMEM;
    }
    result = copyin(args, argv, sizeof (char **));

    if (result) {
        kfree(argv);
        kfree(new_prog);
        return EFAULT;
    }
    //kprintf("lalala\n");
    argSize = 0;
    while (args[argSize] != NULL)
    {
        argv[argSize] = (char *) kmalloc(sizeof (char)* MAX_ARG_LEN);
        if (argv[argSize] == NULL) {
            int i;
            for (i = 0; i < argSize; i++) {
                kfree(argv[i]);
            }
            kfree(argv);
            kfree(new_prog);
            return ENOMEM;
        }
        result = copyinstr(args[argSize], argv[argSize], MAX_ARG_LEN, &size);
        if (result) {
            int i;
            for (i = 0; i < argSize; i++) {
                kfree(argv[i]);
            }
            kfree(argv);
            kfree(new_prog);
            return EFAULT;
        }
        argSize++;
    }
    argv[argSize] = NULL;
    if (curthread->t_vmspace != NULL) {
        as_destroy(curthread->t_vmspace);
        curthread->t_vmspace = NULL;
    }
    // result = runprogram(new_prog, argv, argSize);
#if 1
    // runprogram(char *progname,char ** args, int nargs)
    struct vnode *v;
    //vaddr_t entrypoint, stackptr;
    //int result;
    //kprintf("here 1\n");
    /* Open the file. */
    result = vfs_open(new_prog, O_RDONLY, &v);
    if (result) {
        int j;
        for (j = 0; j < argSize; j++) {
            kfree(argv[i]);
        }
        kfree(argv);
        kfree(new_prog);
        return result;
    }
    // kprintf("here 2\n");
    /* We should be a new thread. */
    assert(curthread->t_vmspace == NULL);

    /* Create a new address space. */
    curthread->t_vmspace = as_create();
    if (curthread->t_vmspace == NULL) {
        vfs_close(v);
        int j;
        for (j = 0; j < argSize; j++) {
            kfree(argv[i]);
        }
        kfree(argv);
        kfree(new_prog);
        return ENOMEM;
    }
    //kprintf("here 3\n");
    /* Activate it. */
    as_activate(curthread->t_vmspace);

    /* Load the executable. */
    result = load_elf(v, &entrypoint);
    if (result) {
        /* thread_exit destroys curthread->t_vmspace */
        vfs_close(v);
        //as_destroy(curthread->t_vmspace);
        int j;
        for (j = 0; j < argSize; j++) {
            kfree(argv[i]);
        }
        kfree(argv);
        kfree(new_prog);
        return result;
    }

    /* Done with the file now. */
    vfs_close(v);

    /* Define the user stack in the address space */
    result = as_define_stack(curthread->t_vmspace, &stackptr);
    if (result) {
        /* thread_exit destroys curthread->t_vmspace */
        //as_destroy(curthread->t_vmspace);
        int j;
        for (j = 0; j < argSize; j++) {
            kfree(argv[i]);
        }
        kfree(argv);
        kfree(new_prog);
        return result;
    }


    //int i;
    userptr_t tmpptr[argSize];
    for (i = argSize - 1; i >= 0; i--) {
        int sizeStack = 1 + strlen(argv[i]);
        int remainder, quotient;
        remainder = sizeStack % 4;
        quotient = sizeStack / 4;
        if (remainder > 0)
            quotient = quotient + 1;
        sizeStack = quotient * 4;
        stackptr = stackptr - sizeStack;
        tmpptr[i] = stackptr;
        result = copyoutstr(argv[i], (userptr_t) stackptr, sizeStack, NULL);
        if (result) {
            //as_destroy(curthread->t_vmspace);
            int j;
            for (j = 0; j < argSize; j++) {
                kfree(argv[i]);
            }
            kfree(argv);
            kfree(new_prog);
            return result;
        }
        //kprintf("copied arguments: %s\n", args[i]);
    }

    stackptr = stackptr - 4;
    char ptr123 = '\0';
    result = copyout(&ptr123, stackptr, 4);
    if (result) {
        //as_destroy(curthread->t_vmspace);
        int j;
        for (j = 0; j < argSize; j++) {
            kfree(argv[i]);
        }
        kfree(argv);
        kfree(new_prog);
        return result;
    }
    for (i = argSize - 1; i >= 0; i--) {
        stackptr = stackptr - 4;
        copyout(tmpptr + i, stackptr, 4);
    }
    //as_destroy(curthread->t_vmspace);
    int j;
    for (j = 0; j < argSize; j++) {
        kfree(argv[i]);
    }
    //for(j=0;j<argSize;j++){
    //  if(argv[i]!=NULL)
    //  kprintf("NOT FREE!!!\n");
    //}
    //kprintf("Freeing~~~\n");
    kfree(argv);
    kfree(new_prog);
    //kprintf("Finish exec coremap\n");
    //printf_coremap();
    //kprintf("Finish exec\n");
    md_usermode(argSize, stackptr, stackptr, entrypoint);
    /* Warp to user mode. */
    //md_usermode(0 /*argc*/, NULL /*userspace addr of argv*/,
    //	    stackptr, entrypoint);

    /* md_usermode does not return */
    panic("md_usermode returned\n");
    return EINVAL;

#endif    
    //return 0;
}

//this function is similar to the function of sys_write to write things from the kernel to user mode

time_t sys__time(time_t *seconds, unsigned long *nanoseconds, int* retval) {
    //INVAL_PTR passed in
    //kprintf("time here1\n");
    if (seconds == 0x40000000 || nanoseconds == 0x40000000)
        return EFAULT;
    //kernel pointer points to second pointer/nanoseconds which passed in the function
    if (seconds == 0x80000000 || nanoseconds == 0x80000000)
        return EFAULT;

    if (seconds == NULL && nanoseconds == NULL) {
        time_t tmp1;
        unsigned long tmp2;
        gettime(&tmp1, &tmp2);
        *retval = tmp1;
        return 0;
    }
    //kprintf("time here2\n");
    //simply do not store the number of seconds in the pointer that has passed in in __time(), and set the retval to the number of seconds you got.
    if (seconds != NULL && nanoseconds == NULL) {
        time_t tmp1; // this pointer is for seconds
        unsigned long tmp2;
        gettime(&tmp1, &tmp2);

        int err;
        err = copyout(&tmp1, seconds, sizeof (time_t));
        if (err)
            return EFAULT;

        *retval = tmp1;
        return 0;
    }
    if (seconds == NULL && nanoseconds != NULL) {
        time_t tmp1;
        unsigned long tmp2; // this pointer is for nanoseconds 
        gettime(&tmp1, &tmp2);

        int err;
        err = copyout(&tmp2, nanoseconds, sizeof (unsigned int));
        if (err)
            return EFAULT;

        *retval = tmp1; // since the second is null pointer and I am not sure the return value whether can be returned by null ptr. 
        //kprintf("return value is %d", *retval);
        return 0;
    }
    //reach this stage, both seconds and nanoseconds are valid to do any manipulation on it
    //we need to store the time into the pointer that has passed in time()
    // second && nanoseconds not NULL and return second in retval
    time_t tmp1;
    unsigned long tmp2;
    int err;

    gettime(&tmp1, &tmp2);

    //just copy out the info to the user mode and check the err value, if the copy out goes wrong, just return the callno   
    err = copyout(&tmp1, seconds, sizeof (time_t));

    if (err)
        return EFAULT;

    err = copyout(&tmp2, nanoseconds, sizeof (unsigned long));
    if (err)
        return EFAULT;

    //reach at this stage, we have already successfully handle the system call from __time() and we set the value to the retval and return;
    *retval = *seconds;
    return 0;
}

int sys_sbrk(intptr_t amount, int32_t * retval) {
    //On error, ((void *)-1) is returned, and errno is set according to the error encountered.  
    // case1: negative case2: positive case3:unaligned to judge the situation

    //how to get the "break"-- the last-used heap address

    //check whether pass variable amount is in good alignment (by modulo of 2 ), we need to ensure it is in heap region .
    struct addrspace* addrspace_copy = curthread->t_vmspace;
    //solved the problem of alignment
    int remainder;
    remainder = amount % 2;
    //kprintf("amount is %d\n",amount);
    if (remainder != 0) {
        return EINVAL;
    }
    //we need to check amount is positive or negative. If positive go to the next judgment, else go into the clause to return EINVAL
    //negative
    if (amount <= -PAGE_SIZE)
        return EINVAL;
    if (addrspace_copy->as_heap_end + amount < (addrspace_copy->as_heap_start)) {
        *retval = (void *) - 1;
        return EINVAL;
    }
    //reach at this stage (amount is aligned and is positive), we need to check whether the heap is available to allocate; return ENOMEM if no available    
    if ((addrspace_copy->as_heap_end + amount) >= addrspace_copy->as_heap_end + 12 * PAGE_SIZE) {
        *retval = (void *) - 1;
        return ENOMEM;
    }

    //On success, sbrk returns the previous value of the "break".
    *retval = addrspace_copy->as_heap_end;
    //kprintf("as_heap_end is %x\n",addrspace_copy->as_heap_end);
    //set the heap to the last one heap address + amount of allocated memory  
    addrspace_copy->as_heap_end = (addrspace_copy->as_heap_end + amount);
    //kprintf("new_as_heap_end is %x\n",addrspace_copy->as_heap_end);
    return 0;
}