#include <types.h>
#include <lib.h>
#include <kern/errno.h>
#include <array.h>
#include <machine/spl.h>
#include <machine/pcb.h>
#include <thread.h>
#include <curthread.h>
#include <scheduler.h>
#include <addrspace.h>
#include <vnode.h>
#include "opt-synchprobs.h"
#include <synch.h>
#include <vm.h>
#include <machine/tlb.h>
#include <uio.h>
#include <kern/stat.h>
#include <kern/unistd.h>
#include <vfs.h>
int fix_page_end = 0;
int total_page_num = 0;
struct core_map* coremap;
struct vnode* swap_file;

void vm_bootstrap(void) {
#if 1
    int spl;
    spl = splhigh();

    int result;
    result = vfs_open("lhd1raw:", O_RDWR, &swap_file);
    if (result) {
        panic("vfs_open on lhad1raw failed");
    }
    struct stat stat;
    VOP_STAT(swap_file, &stat);
    paddr_t start_point, end_point;
    int fix_page, free_page;
    int free_start;
    //we need to get the how much memory it available in the RAM
    ram_getsize(&start_point, &end_point);
    //kprintf("start_point is %d  end_point is %d \n",start_point,end_point);
    paddr_t tmp = start_point;
    //we need to get how many pages can be constructed in such large memory
    int num_pages = 0;
    size_t sz = end_point - start_point;
    //kprintf("size is %d\n",sz);
    num_pages = ROUNDUP(sz, PAGE_SIZE) / PAGE_SIZE;
    //num_pages=sz/PAGE_SIZE;
    //kprintf("npages is %d\n",num_pages);
    /* ...and now the length. */
    //sz = (sz + PAGE_SIZE - 1) & PAGE_FRAME;
    //num_pages = sz / PAGE_SIZE;
    total_page_num = num_pages;
    //set the starting point of core_map
    coremap = (struct core_map *) PADDR_TO_KVADDR(start_point);
    //now want to give some fixed pages for core-map
    //meanwhile, we need to get how many free pages
    free_start = start_point + num_pages * (sizeof (struct core_map));
    //store the fix_page number into the global variable
    sz = free_start - tmp;
    //sz = (sz + PAGE_SIZE - 1) & PAGE_FRAME;
    fix_page = ROUNDUP(sz, PAGE_SIZE) / PAGE_SIZE;
    // kprintf("fix_page is %d\n",fix_page);
    //fix_page = sz / PAGE_SIZE;
    fix_page_end = fix_page;
    //here we can get how many free pages the RAM has
    free_page = num_pages - fix_page;
    //kprintf("free %d",free_page);
    int i = 0;
    //we store core-map which is fixed
    while (i < fix_page) {
        coremap[i].state = FIX;
        coremap[i].addrspace = NULL;
        coremap[i].core_map_padd = start_point;
        coremap[i].core_map_vadd = 0; //PADDR_TO_KVADDR(start_point);
        coremap[i].npages = 1;
        //kprintf("next start_point address %x\n",start_point);
        start_point += PAGE_SIZE;
        //kprintf("next start_point address %x\n",start_point);
        i++;
    }
    //here is the free pages that can be used
    while (i < num_pages) {
        coremap[i].state = FREE;
        coremap[i].addrspace = NULL;
        coremap[i].core_map_padd = start_point;
        coremap[i].core_map_vadd = PADDR_TO_KVADDR(start_point);
        coremap[i].npages = 0;
        start_point += PAGE_SIZE;
        i++;
    }
    splx(spl);
#endif  
}

paddr_t
getppages(unsigned long npages) {
    //for this function, we need to check whether it has n-pages to allocate, therefore, its functionality is to check n-pages existence
    //at this time coremap is not ready, therefore we need to steal from ram
    //kprintf("here5\n");
#if 1
    int spl;
    paddr_t addr;
    spl = splhigh();
    if (coremap == NULL) {
        addr = ram_stealmem(npages);
        splx(spl);
        return addr;
    } else {
        //kprintf("here123\n");
        //now check whether it exist n-pages
        //initialize a counter to count the free pages
        int free_page_counter = 0;
        int i = fix_page_end; //core_map[fix_page_end] is actually store the free page information
        for (i; i < total_page_num; i++) {
            if (free_page_counter == npages)
                break;
            //check whether the free page is connected         
            if (coremap[i].state == FIX || coremap[i].state == DIRTY)
                free_page_counter = 0;
            //count how many page is connected
            if (coremap[i].state == FREE) {
                free_page_counter++;
            }
            if (i == total_page_num && free_page_counter < npages) {
                //kprintf("need swap function\n");
                splx(spl);
                return 0; //function : swap();
            }
            //if there exist enough free pages to allocate, then break and return
        }
        int j = 0;
        int free_alloc_start = i - free_page_counter;
        //reach at this point, there exist connected pages that can be allocated
        for (j; j < npages; j++) {
            coremap[free_alloc_start + j].addrspace = curthread->t_vmspace;
            coremap[free_alloc_start + j].state = FIX;
            coremap[free_alloc_start + j].core_map_vadd = PADDR_TO_KVADDR(coremap[free_alloc_start ].core_map_padd);
            if (j == 0) {
                coremap[free_alloc_start].npages = npages;
            }
        }
        splx(spl);
        return coremap[free_alloc_start].core_map_padd;
    }
#endif
}

//Fault handling function called by trap code 

int vm_fault(int faulttype, vaddr_t faultaddress) {
#if 0    //    Virtual page not allocated in address space, OS sends fault to process (e.g., segmentation fault) 
    //Page not in physical memory, OS allocates frame, reads from disk, maps PTE to physical frame
    struct addrspace *as;
    int spl;

    spl = splhigh();
    //Extracts the page number from the virtual address
    faultaddress &= PAGE_FRAME;

    DEBUG(DB_VM, "vm: fault: 0x%x\n", faultaddress);

    switch (faulttype) {
        case VM_FAULT_READONLY:
            /* We always create pages read-write, so we can't get this */
            panic("vm: got VM_FAULT_READONLY\n");
        case VM_FAULT_READ:
        case VM_FAULT_WRITE:
            break;
        default:
            splx(spl);
            return EINVAL;
    }

    as = curthread->t_vmspace;
    if (as == NULL) {
        return EFAULT;
    }

    /* make sure it's page-aligned */
    //assert((paddr & PAGE_FRAME) == paddr);
    return 0;
#endif

    vaddr_t vbase1, vtop1, vbase2, vtop2, stackbase, stacktop;
    paddr_t paddr;
    int i;
    u_int32_t ehi, elo;
    struct addrspace *as;
    int spl;

    spl = splhigh();

    faultaddress &= PAGE_FRAME;

    DEBUG(DB_VM, "vm: fault: 0x%x\n", faultaddress);

    switch (faulttype) {
        case VM_FAULT_READONLY:
            // We always create pages read-write, so we can't get this 
            panic("vm: got VM_FAULT_READONLY\n");
        case VM_FAULT_READ:
        case VM_FAULT_WRITE:
            break;
        default:
            splx(spl);
            return EINVAL;
    }

    as = curthread->t_vmspace;
    if (as == NULL) {
        return EFAULT;
    }

    // Assert that the address space has been set up properly. 
    assert(as->as_vbase1 != 0);
    //assert(as->as_pbase1 != 0);
    assert(as->as_npages1 != 0);
    assert(as->as_vbase2 != 0);
    //assert(as->as_pbase2 != 0);
    assert(as->as_npages2 != 0);
    //assert(as->as_stackpbase != 0);
    assert((as->as_vbase1 & PAGE_FRAME) == as->as_vbase1);
    //assert((as->as_pbase1 & PAGE_FRAME) == as->as_pbase1);
    assert((as->as_vbase2 & PAGE_FRAME) == as->as_vbase2);
    //assert((as->as_pbase2 & PAGE_FRAME) == as->as_pbase2);
    assert((as->as_stackpbase & PAGE_FRAME) == as->as_stackpbase);

    vaddr_t heapbase, heaptop;
    vbase1 = as->as_vbase1;
    vtop1 = vbase1 + as->as_npages1 * PAGE_SIZE;
    vbase2 = as->as_vbase2;
    vtop2 = vbase2 + as->as_npages2 * PAGE_SIZE;
    stackbase = USERSTACK - 1024 * PAGE_SIZE;
    stacktop = USERSTACK;
    heapbase = as->as_heap_start;
    heaptop = as->as_heap_end;

    size_t first_ten_bits = 0;
    size_t second_ten_bits = 0;

    first_ten_bits = faultaddress & First_PAGE_DIRECTORY;
    first_ten_bits = first_ten_bits >> 22;
    second_ten_bits = faultaddress & MID_PAGE_FRAME;
    second_ten_bits = second_ten_bits >> 12;
    int enter_bit = 0;
    if (as->PD[first_ten_bits].dictionary_entry) {
        //second level page table exist
        if (0)
            //if swapped out, swap it back and return the paddr
            ;
        else {
            //kprintf("lalala\n");
            if (as->PD[first_ten_bits].dictionary_entry[second_ten_bits].valid_bit == 1) {
                enter_bit = 1;
                paddr = as->PD[first_ten_bits].dictionary_entry[second_ten_bits].paddr;
            }
        }
    }
    else 
        as->PD[first_ten_bits].dictionary_entry = create_page_table();

    if (!enter_bit) {
        //kprintf("here bang!\n");
        if (faultaddress >= vbase1 && faultaddress < vtop1) {
            //kprintf("here 1!\n");
          
                paddr = getupages_via_vaddr(faultaddress);
           //kprintf("filling one page table\n");
            as->PD[first_ten_bits].dictionary_entry[second_ten_bits].paddr = paddr;
            as->PD[first_ten_bits].dictionary_entry[second_ten_bits].valid_bit = 1;
            //kprintf("call load segment\n");
            load_segment_helper(as, as->as_vbase1, as->offset1, faultaddress, as->memsize1, as->filesize1, as->as_flag1);
            splx(spl);
            return 0;
        } else if (faultaddress >= vbase2 && faultaddress < vtop2) {
           //kprintf("here 2!\n");
            
                paddr = getupages_via_vaddr(faultaddress);
            //kprintf("filling one page table\n");
            as->PD[first_ten_bits].dictionary_entry[second_ten_bits].paddr = paddr;
            as->PD[first_ten_bits].dictionary_entry[second_ten_bits].valid_bit = 1;
            load_segment_helper(as, as->as_vbase2, as->offset2, faultaddress, as->memsize2, as->filesize2, as->as_flag2);
            splx(spl);
            return 0;
        } else if (faultaddress >= stackbase && faultaddress < stacktop) {
          // kprintf("here 3!\n");
         
                paddr = getupages_via_vaddr(faultaddress);
             //   kprintf("filling one page table\n");
            as->PD[first_ten_bits].dictionary_entry[second_ten_bits].paddr = paddr;
            as->PD[first_ten_bits].dictionary_entry[second_ten_bits].valid_bit = 1;
            as->as_stackpbase -= PAGE_SIZE;
        } else if (faultaddress >= heapbase && faultaddress < heaptop) {
            //kprintf("here 4!\n");
            paddr = getupages_via_vaddr(faultaddress);
            //kprintf("filling one page table\n");
            as->PD[first_ten_bits].dictionary_entry[second_ten_bits].paddr = paddr;
            as->PD[first_ten_bits].dictionary_entry[second_ten_bits].valid_bit = 1;
        } else {
           // kprintf("here 5!\n");
            splx(spl);
            return EFAULT;
        }
    }
    // make sure it's page-aligned 
    assert((paddr & PAGE_FRAME) == paddr);

    for (i = 0; i < NUM_TLB; i++) {
        TLB_Read(&ehi, &elo, i);
        if (elo & TLBLO_VALID) {
            continue;
        }
        ehi = faultaddress;
        elo = paddr | TLBLO_DIRTY | TLBLO_VALID;
        DEBUG(DB_VM, "vm: 0x%x -> 0x%x\n", faultaddress, paddr);
        TLB_Write(ehi, elo, i);
        splx(spl);
        return 0;
    }

    kprintf("vm: Ran out of TLB entries - cannot handle page fault\n");
    splx(spl);
    return EFAULT;
}

//as->as_pbase1 = getppages(as->as_npages1);
//if (as->as_pbase1 == 0) {
//    return ENOMEM;
//}

paddr_t getupages_via_vaddr(vaddr_t addr) {
    //int spl;
    //spl = splhigh();
    
    int free_page_counter = 0;
    int i = fix_page_end; //core_map[fix_page_end] is actually store the free page information
    for (i; i < total_page_num; i++) {
        if (free_page_counter == 1)
            break;
        //check whether the free page is connected         
        if (coremap[i].state == FIX)
            free_page_counter = 0;
        //count how many page is connected
        if (coremap[i].state == FREE) {
            free_page_counter++;
        }
        if (i == total_page_num && free_page_counter < 1) {
          //  kprintf("need swap function\n");
            // splx(spl);
            return 0; //function : swap();
        }
    }
    int j = 0;
    int free_alloc_start = i - free_page_counter;
    //reach at this point, there exist connected pages that can be allocated
    coremap[free_alloc_start + j].addrspace = curthread->t_vmspace;
    coremap[free_alloc_start + j].state = DIRTY;
    coremap[free_alloc_start + j].core_map_vadd = addr;
    coremap[free_alloc_start].npages = 1;
    //splx(spl);
    return coremap[free_alloc_start].core_map_padd;
}

void
load_segment_helper(struct addrspace *as, vaddr_t vbase, off_t offset, vaddr_t vaddr,
        size_t memsize, size_t filesize,
        int is_executable) {
#if 1
    //vaddr is fault address
    //we need *as to get the information we stored in the structure
    struct uio u;
    int result;
    size_t fillamt;
    int tmp_resid=0;
    //if get one page that will cause
    //fault address + page_size > file_size -> resid =PAGE_SIZE
    //if file_size is 0 then resid=0;
    //   
    if (filesize == 0) {
        //kprintf("ELF: warning: segment filesize = 0\n");
        u.uio_resid = 0;
    }

    if (filesize > memsize) {
        //kprintf("ELF: warning: segment filesize > segment memsize\n");
        filesize = memsize;
        u.uio_resid = filesize;
    }

    if ((vaddr + PAGE_SIZE) < vbase + filesize) {
        //kprintf("ELF: warning: 1 page is not enough\n");
        u.uio_resid = PAGE_SIZE;
    }
    if ((vaddr + PAGE_SIZE) > vbase + filesize) {
        //kprintf("ELF: warning: 1 page is enough\n");
        u.uio_resid = (vbase + filesize - vaddr);
    }
   // kprintf("fault address is 0x%x resid is %d memsize is %d\n", vaddr, u.uio_resid);
    DEBUG(DB_EXEC, "ELF: Loading %lu bytes to 0x%lx\n",
            (unsigned long) filesize, (unsigned long) vaddr);

    u.uio_iovec.iov_ubase = (userptr_t) vaddr;
    u.uio_iovec.iov_len = PAGE_SIZE; // length of the memory space
    // u.uio_resid = filesize; // amount to actually read
    u.uio_offset = offset + (vaddr - vbase);
    u.uio_segflg = is_executable ? UIO_USERISPACE : UIO_USERSPACE;
    u.uio_rw = UIO_READ;
    u.uio_space = curthread->t_vmspace;
    //kprintf("offset is %d\n", u.uio_offset);
   // kprintf("here1\n");
    tmp_resid=u.uio_resid;
    result = VOP_READ(as->v, &u);
    if (result) {
        return result;
    }
    //kprintf("here2\n");
    if (u.uio_resid != 0) {
        /* short read; problem with executable? */
        //kprintf("ELF: short read on segment - file truncated?\n");
        return ENOEXEC;
    }
    //kprintf("here3\n");
    /* Fill the rest of the memory space (if any) with zeros */
    fillamt = PAGE_SIZE - tmp_resid;
    if (fillamt > 0) {
        DEBUG(DB_EXEC, "ELF: Zero-filling %lu more bytes\n",
                (unsigned long) fillamt);
        u.uio_resid += fillamt;
        result = uiomovezeros(fillamt, &u);
    }
    return result;
#endif
    //return 0;
}

paddr_t fault_handler(vaddr_t faultaddress, struct addrspace *as) {
    //handle the fault address to determine the physical address of this fault address
    paddr_t paddr;
    //first 10 bits  31-21
    size_t first_ten_bits = 0;
    size_t second_ten_bits = 0;

    first_ten_bits = faultaddress & First_PAGE_DIRECTORY;
    first_ten_bits = first_ten_bits >> 22;
    second_ten_bits = faultaddress & MID_PAGE_FRAME;
    second_ten_bits = second_ten_bits >> 12;

    //if the second level exist then we can just load the page, otherwise we need to get page for user process/thread
    if (as->PD[first_ten_bits].dictionary_entry) {
        //second level page table exist
        if (0)
            //if swapped out, swap it back and return the paddr
            ;
        else {
            //if valid bit is 1
            //return paddr = page table physical address
            //else if valid bit is 0
            if (as->PD[first_ten_bits].dictionary_entry[second_ten_bits].valid_bit == 1) {
                //kprintf("fault address is 0x%x\n", faultaddress);
                return as->PD[first_ten_bits].dictionary_entry[second_ten_bits].paddr;
            } else {//fill the page table
                //kprintf("fault address is 0x%x\n", faultaddress);
                paddr = getupages_via_vaddr(faultaddress);
                as->PD[first_ten_bits].dictionary_entry[second_ten_bits].paddr = paddr;
                as->PD[first_ten_bits].dictionary_entry[second_ten_bits].valid_bit = 1;
                return paddr;
            }
        }

    } else {//the second does not exist then we create the second table and allocate the page for this faultaddress
        as->PD[first_ten_bits].dictionary_entry = create_page_table();
        paddr = getupages_via_vaddr(faultaddress);
        as->PD[first_ten_bits].dictionary_entry[second_ten_bits].paddr = paddr;
        as->PD[first_ten_bits].dictionary_entry[second_ten_bits].valid_bit = 1;
        return paddr;
    }
}

// Allocate/free kernel heap pages (called by kmalloc/kfree) 

vaddr_t alloc_kpages(int npages) {
    paddr_t pa;
    pa = getppages(npages);
    if (pa == 0) {
        return 0;
    }
    return PADDR_TO_KVADDR(pa);

}

paddr_t getupages(int npages) {
    int spl;
    spl = splhigh();
    //now check whether it exist n-pages
    //initialize a counter to count the free pages
    int free_page_counter = 0;
    int i = fix_page_end; //core_map[fix_page_end] is actually store the free page information
    for (i; i < total_page_num; i++) {
        if (free_page_counter == npages)
            break;
        //check whether the free page is connected         
        if (coremap[i].state == FIX ||coremap[i].state==DIRTY)
            free_page_counter = 0;
        //count how many page is connected
        if (coremap[i].state == FREE) {
            free_page_counter++;
        }
        if (i == total_page_num && free_page_counter < npages) {
            //kprintf("need swap function\n");
            splx(spl);
            return 0; //function : swap();
            //}
            //if there exist enough free pages to allocate, then break and return
        }}
        int j = 0;
        int free_alloc_start = i - free_page_counter;
        //reach at this point, there exist connected pages that can be allocated
        for (j; j < npages; j++) {
            coremap[free_alloc_start + j].addrspace = curthread->t_vmspace;
            coremap[free_alloc_start + j].state = DIRTY;
            coremap[free_alloc_start + j].core_map_vadd = PADDR_TO_KVADDR(coremap[free_alloc_start + j].core_map_padd);
            if (j == 0) {
                coremap[free_alloc_start].npages = npages;
            }
        }
        splx(spl);
        return coremap[free_alloc_start].core_map_padd;
    
}

void free_kpages(vaddr_t addr) {
#if  1  //free_kpages just do the reverse functionality of alloc_kpages
    int spl;
    spl = splhigh();
    int i = fix_page_end;
    int free_counter = 0;

    while (i < total_page_num) {
        if (coremap[i].core_map_vadd == addr) {
            int tmp_npage = 0;
            tmp_npage = coremap[i].npages;
            int j = 0;
            for (j; j < tmp_npage; j++) {
                coremap[i + j].addrspace = NULL;
                coremap[i + j].state = FREE;
                if (j == 0)
                    coremap[i + j].npages = 0;
            }
            splx(spl);
            return;
        }
        i++;
    }
    splx(spl);
    return;
#endif
}

void free_upages(struct addrspace* as) {
    int spl;
    int i = 0;

    while (i < total_page_num) {
        if (coremap[i].state != FREE && coremap[i].addrspace == as) {
            coremap[i].state = FREE;
            coremap[i].npages = 0;
            coremap[i].addrspace = NULL;
            coremap[i].core_map_vadd=0;
        }
        i++;
    }
}


void printf_coremap(void){
    int i=0;
    for(i;i<total_page_num;i++){
    kprintf("[%d]:",i);
    if(coremap[i].state == FREE){
        kprintf("0 ");
    }
    else
        kprintf("1 ");
    if(i==10||i==20||i==30||i==40||i==50||i==60||i==70||i==80)
        kprintf("\n");
    
    }
    if(i==total_page_num)
        kprintf("\n");
    
}