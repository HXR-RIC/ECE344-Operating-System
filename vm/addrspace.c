#include <types.h>
#include <kern/errno.h>
#include <lib.h>
#include <thread.h>
#include <curthread.h>
#include <addrspace.h>
#include <vm.h>
#include <machine/spl.h>
#include <machine/tlb.h>
#include <vnode.h>

/*
 * Dumb MIPS-only "VM system" that is intended to only be just barely
 * enough to struggle off the ground. You should replace all of this
 * code while doing the VM assignment. In fact, starting in that
 * assignment, this file is not included in your kernel!
 */

int create_PageDic(struct addrspace *as) {
#if 1    
    as->PD = kmalloc(sizeof (struct page_dictionary)*PTE_NUMBER);

    if (as->PD == NULL) {
        //kprintf("no mem in first level page table\n");

        return ENOMEM;
    }

    int i;

    for (i = 0; i < PTE_NUMBER; i++)
        as->PD[i].dictionary_entry = NULL;

    return 0;
#endif
    //void(as);
    //return 0;
}

struct page_table *create_page_table(void) {
#if 1    
    int i;
    struct page_table *PT;
    PT = kmalloc(sizeof (struct page_table)*PTE_NUMBER);

    if (PT == NULL) {

        //kprintf("no mem in second level page table\n");
        return ENOMEM;
    }


    for (i = 0; i < PTE_NUMBER; i++) {
        PT[i].paddr = 0;
        PT[i].valid_bit = 0;
    }
    return PT;
#endif
}

struct addrspace *
as_create(void) {
#if 1
    struct addrspace *as = kmalloc(sizeof (struct addrspace));
    if (as == NULL)
        return NULL;

    as->as_vbase1 = 0;
    as->offset1 = 0;
    as->memsize1 = 0;
    as->filesize1 = 0;
    as->as_npages1 = 0;
    as->as_flag1 = 0;

    as->as_vbase2 = 0;
    as->offset2 = 0;
    as->memsize2 = 0;
    as->filesize1 = 0;
    as->as_npages2 = 0;
    as->as_flag2 = 0;

    as->v = NULL;
    as->PD = NULL;
    as->as_heap_start = 0;
    as->as_heap_end = 0;
    as->as_stackpbase = 0;

    int result;
    result = create_PageDic(as);
    if (result)
        return NULL;

    return as;
#endif
}

void
as_destroy(struct addrspace *as) {
#if 1
    int spl = splhigh();
    int i;
    VOP_DECREF(as->v);
    free_upages(as);
    for (i = 0; i < PTE_NUMBER; i++) {
        //page table is not NULL
        if (as->PD[i].dictionary_entry != NULL) {
            kfree(as->PD[i].dictionary_entry);
        }
    }
    kfree(as->PD);
    kfree(as);
    printf_coremap();
    splx(spl);
#endif
    //kfree(as);

}

void
as_activate(struct addrspace *as) {
    int i, spl;

    (void) as;

    spl = splhigh();

    for (i = 0; i < NUM_TLB; i++) {
        TLB_Write(TLBHI_INVALID(i), TLBLO_INVALID(), i);
    }

    splx(spl);
}

int
as_define_region(struct addrspace *as, vaddr_t vaddr, size_t sz,
        int readable, int writeable, int executable) {
#if 1
    int spl;
    spl = splhigh();
    size_t npages;

    /* Align the region. First, the base... */
    sz += vaddr & ~(vaddr_t) PAGE_FRAME;
    vaddr &= PAGE_FRAME;

    /* ...and now the length. */
    sz = (sz + PAGE_SIZE - 1) & PAGE_FRAME;

    npages = sz / PAGE_SIZE;

    /* We don't use these - all pages are read-write */
    (void) readable;
    (void) writeable;
    (void) executable;

    //int i = 0;enter as copy part

    //for (i; i < PTE_NUMBER; i++) {
    //    as->PD[i].dictionary_entry = create_page_table();
    //}
    //program memory layout is consist of code, data, bss, heap, stack
    //initialize the vaddr of code
    if (as->as_vbase1 == 0) {
        as->as_vbase1 = vaddr;
        as->as_npages1 = npages;
        //kprintf("code page is %d\n",as->as_npages1);
        splx(spl);
        return 0;
    }
    //initialize the vaddr of data. The vaddr of code and data in this case can be same since, we actually not manipulate text and data.
    //we only need to define the starting point of heap, and we will define the end point of heap by sbrk function
    if (as->as_vbase2 == 0) {
        as->as_vbase2 = vaddr;
        as->as_npages2 = npages;
        as->as_heap_start = vaddr + npages*PAGE_SIZE;
        as->as_heap_end = as->as_heap_start;
        //kprintf("%x\n",as->as_heap_start);
        //kprintf("%x\n",as->as_heap_end);
        splx(spl);
        return 0;
    }
    /*
     * Support for more than two regions is not available.
     */
    splx(spl);
    kprintf("vm: Warning: too many regions\n");
    return EUNIMP;
#endif

}

int
as_prepare_load(struct addrspace *as) {
#if 1
    return 0;
#endif
}

int
as_complete_load(struct addrspace *as) {
    (void) as;
    return 0;
}

int
as_define_stack(struct addrspace *as, vaddr_t *stackptr) {
    //assert(as->as_stackpbase != 0);
    //int i = 0;
    //while (i < VM_STACKPAGES) {
    //since stack is increment downwards
    //   as->as_stackvbase = USERSTACK - VM_STACKPAGES*i;
    as->as_stackpbase = USERSTACK;
    *stackptr = USERSTACK;
    return 0;
}

int
as_copy(struct addrspace *old, struct addrspace **ret) {
#if 1  
    struct addrspace *new;
    kprintf("enter as copy part\n");
    new = as_create();

    if (new == NULL) {
        kprintf("1\n");
        return ENOMEM;
    }
    new->as_vbase1 = old->as_vbase1;
    new->as_npages1 = old->as_npages1;
    new->as_vbase2 = old->as_vbase2;
    new->v = old->v;
    new->as_flag1 = old->as_flag1;
    new->as_flag2 = old->as_flag2;
    new->filesize1 = old->filesize1;
    new->filesize2 = old->filesize2;
    new->memsize1 = old->memsize1;
    new->memsize2 = old->memsize2;
    new->offset1 = old->offset1;
    new->offset2 = old->offset2;
    new->as_npages2 = old->as_npages2;
    new->as_stackpbase = old->as_stackpbase;
    new->as_heap_start = old->as_heap_start;
    new->as_heap_end = old->as_heap_end;
    //int heap_num=0;
    //heap_num=(old->as_heap_end-old->as_heap_start)/PAGE_SIZE;
    //new->PD = kmalloc(sizeof (struct page_dictionary) * PTE_NUMBER);
    VOP_INCREF(old->v);
    int i, j;
    kprintf("here 1\n");
    //for (i = 0; i < PTE_NUMBER; i++)
    //    new->PD[i].dictionary_entry = old->PD[i].dictionary_entry;
    for (i = 0; i < PTE_NUMBER; i++) {
        if (old->PD[i].dictionary_entry != NULL) {
            //kprintf("enter copy page table\n");
            new->PD[i].dictionary_entry = create_page_table();
            kprintf("here 2\n");
            if (new->PD[i].dictionary_entry == NULL) {
                int j = 0;
                for (; j < i; j++) {
                    kprintf("here 3\n");
                    kfree(new->PD[j].dictionary_entry);
                }
                //kprintf("As_copy Failed coremap\n");
                //printf_coremap();
                kprintf("here 4\n");
                return ENOMEM;
            }
            //kprintf("As_copy Succeed coremap\n");
            //  printf_coremap();
            for (j = 0; j < PTE_NUMBER; j++) {
                if (old->PD[i].dictionary_entry[j].valid_bit == 1) {
                    //kprintf("Old ID is %d, old PD addr is 0x%x\n",i,old->PD[i].dictionary_entry[j].paddr);
                    //  kprintf("As_copy Succeed coremap2222\n");
                    //printf_coremap();
                    new->PD[i].dictionary_entry[j].paddr = getupages(1);
                    if (new->PD[i].dictionary_entry[j].paddr == 0)
                        return ENOMEM;
                    new->PD[i].dictionary_entry[j].valid_bit = 1;
                    //kprintf("valid bit is 1 and copy\n");
                    memmove((void *) PADDR_TO_KVADDR(new->PD[i].dictionary_entry[j].paddr),
                            (const void *) PADDR_TO_KVADDR(old->PD[i].dictionary_entry[j].paddr),
                            PAGE_SIZE);
                            kprintf("here 5\n");
                    //kprintf("copy one page table entry\n");
                } else {
                    ;
                    //kprintf("valid bit is 0 and copy\n");
                    //new->PD[i].dictionary_entry[j].paddr = old->PD[i].dictionary_entry[j].paddr;
                    //memmove((void *) PADDR_TO_KVADDR(new->PD[i].dictionary_entry[j].paddr),
                    //      (const void *) PADDR_TO_KVADDR(old->PD[i].dictionary_entry[j].paddr),
                    //    PAGE_SIZE);
                }
            }
        }
        //else
        //new->PD[i].dictionary_entry = NULL;
    }
    //kprintf("copy page table already\n");
#endif

    assert(new->as_vbase1 != 0);
    assert(new->as_vbase2 != 0);
    assert(new->as_npages2 != 0);
    assert(new->as_npages1 != 0);
    assert(new->as_stackpbase != 0);
    assert(new->as_heap_start != 0);
    assert(new->as_heap_end != 0);

    *ret = new;
    kprintf("Leaving as copy\n");
    return 0;
}
