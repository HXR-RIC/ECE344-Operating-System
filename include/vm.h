#ifndef _VM_H_
#define _VM_H_

#include <machine/vm.h>

/*
 * VM system-related definitions.
 *
 * You'll probably want to add stuff here.
 */


/* Fault-type arguments to vm_fault() */
#define VM_FAULT_READ        0    /* A read was attempted */
#define VM_FAULT_WRITE       1    /* A write was attempted */
#define VM_FAULT_READONLY    2    /* A write to a readonly page was attempted*/
#define DUMBVM_STACKPAGES  24

/* Initialization function */
void vm_bootstrap(void);
 paddr_t getppages(unsigned long npages);
/* Fault handling function called by trap code */
int vm_fault(int faulttype, vaddr_t faultaddress);
paddr_t fault_handler(vaddr_t faultaddress, struct addrspace *as);
paddr_t getupages_via_vaddr(vaddr_t addr);
void free_upages(struct addrspace* as);
/* Allocate/free kernel heap pages (called by kmalloc/kfree) */
vaddr_t alloc_kpages(int npages);
void free_kpages(vaddr_t addr);
paddr_t getupages(int npages);
void load_segment_helper(struct addrspace *as,vaddr_t vbase, off_t offset, vaddr_t vaddr, size_t memsize, size_t filesize,int is_executable);
void printf_coremap(void);
//paddr_t alloc_userpages(vaddr_t addr, int npages);
#endif /* _VM_H_ */
