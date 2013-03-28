/*
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file COPYING in the main directory of this archive
 * for more details.
 *
 * Copyright (C) 2009  Hangzhou C-SKY Microsystems co.,ltd.
 * Copyright (C) 2009  Hu junshan (junshan_hu@c-sky.com)
 *
 */

#include <linux/moduleloader.h>
#include <linux/elf.h>
#include <linux/mm.h>
#include <linux/vmalloc.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/string.h>
#include <linux/kernel.h>
#include <linux/spinlock.h>
#include <asm/pgtable.h>        

#if 0
#define DEBUGP printk
#else
#define DEBUGP(fmt...)
#endif

#ifdef CONFIG_MODULES

void *module_alloc(unsigned long size)
{
#ifdef MODULE_START
        struct vm_struct *area;

        size = PAGE_ALIGN(size);
        if (!size)
                return NULL;

        area = __get_vm_area(size, VM_ALLOC, MODULE_START, MODULE_END);
        if (!area)
                return NULL;

        return __vmalloc_area(area, GFP_KERNEL, PAGE_KERNEL);
#else
        if (size == 0)
                return NULL;
        return vmalloc(size);
#endif
}

/*
 * Free memory returned from module_alloc
 */
void module_free(struct module *mod, void *module_region)
{
        vfree(module_region);
        /* FIXME: If module_region == mod->init_region, trim exception
           table entries. */
}

/*
 * We don't need anything special.
 */
int module_frob_arch_sections(Elf_Ehdr *hdr, Elf_Shdr *sechdrs,
                              char *secstrings, struct module *mod)
{
        return 0;
}

int apply_relocate(Elf_Shdr *sechdrs, const char *strtab,
                   unsigned int symindex, unsigned int relsec,
                   struct module *me)
{
	unsigned int i;
        Elf32_Rel *rel = (void *)sechdrs[relsec].sh_addr;
        Elf32_Sym *sym;
        uint32_t *location;

        DEBUGP("Applying relocate section %u to %u\n", relsec,
               sechdrs[relsec].sh_info); 
        for (i = 0; i < sechdrs[relsec].sh_size / sizeof(*rel); i++) {
                /* This is where to make the change */
                location = (void *)sechdrs[sechdrs[relsec].sh_info].sh_addr
                        + rel[i].r_offset;
                /* This is the symbol it is referring to.  Note that all
                   undefined symbols have been resolved.  */
                sym = (Elf32_Sym *)sechdrs[symindex].sh_addr
                        + ELF32_R_SYM(rel[i].r_info);
		switch (ELF32_R_TYPE(rel[i].r_info)) {
        case R_CSKY_NONE:
			/* ignore */
			break;
        case R_CSKY_32:
            /* We add the value into the location given */
            *location += sym->st_value;
            break;
        case R_CSKY_PC32:
            /* Add the value, subtract its postition */
            *location += sym->st_value - (uint32_t)location;
            break;
        default:
            printk(KERN_ERR "module %s: Unknown relocation: %u\n",
                   me->name, ELF32_R_TYPE(rel[i].r_info));
            return -ENOEXEC;
        }
    }
    return 0;

}

int apply_relocate_add(Elf32_Shdr *sechdrs,
               const char *strtab,
               unsigned int symindex,
               unsigned int relsec,
               struct module *me)
{
    unsigned int i;
    Elf32_Rela *rel = (void *)sechdrs[relsec].sh_addr;
    Elf32_Sym *sym;
    uint32_t *location;

    DEBUGP("Applying relocate_add section %u to %u\n", relsec,
           sechdrs[relsec].sh_info);
    for (i = 0; i < sechdrs[relsec].sh_size / sizeof(*rel); i++) {
        /* This is where to make the change */
        location = (void *)sechdrs[sechdrs[relsec].sh_info].sh_addr
            + rel[i].r_offset;
        /* This is the symbol it is referring to.  Note that all
           undefined symbols have been resolved.  */
        sym = (Elf32_Sym *)sechdrs[symindex].sh_addr
            + ELF32_R_SYM(rel[i].r_info);

        switch (ELF32_R_TYPE(rel[i].r_info)) {
        case R_CSKY_32:
            /* We add the value into the location given */
            *location = rel[i].r_addend + sym->st_value;
            break;
        case R_CSKY_PC32:
            /* Add the value, subtract its postition */
            *location = rel[i].r_addend + sym->st_value - (uint32_t)location;
            break;
        default:
            printk(KERN_ERR "module %s: Unknown relocation: %u\n",
                   me->name, ELF32_R_TYPE(rel[i].r_info));
            return -ENOEXEC;
        }
    }
    return 0;
}

int module_finalize(const Elf_Ehdr *hdr,
            const Elf_Shdr *sechdrs,
            struct module *mod)
{

    return 0;
}

void module_arch_cleanup(struct module *mod)
{
}

#endif /* CONFIG_MODULES */
