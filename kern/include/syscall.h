#ifndef _SYSCALL_H_
#define _SYSCALL_H_

/*
 * Prototypes for IN-KERNEL entry points for system call implementations.
 */
#define MAX_PATH_LEN 128
#define MAX_ARG_LEN 32
int sys_reboot(int code);
int sys_write(int fd, const void *buf, size_t nbytes, int *retval);
int sys_read(int fd, const void *buf, size_t nbytes, int *retval);
pid_t sys_fork(struct trapframe *tf, int* retval);
pid_t sys_getpid(void);
int sys_waitpid(pid_t pid, int *status, int options, int* retval);
void sys_exit(int exitcode);
int sys_execv(const char *program, char **args);
time_t sys__time(time_t *seconds, unsigned long *nanoseconds, int* retval);
int sys_sbrk(intptr_t amount, int32_t* retval);
#endif /* _SYSCALL_H_ */
