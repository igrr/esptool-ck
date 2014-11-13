/**********************************************************************************
 **********************************************************************************
 ***
 ***    esptool_elf.h
 ***    - defines, structs and other stuff to access ELF files
 ***
 ***    Copyright (C) 2014 Christian Klippel <ck@atelier-klippel.de>
 ***
 ***    This program is free software; you can redistribute it and/or modify
 ***    it under the terms of the GNU General Public License as published by
 ***    the Free Software Foundation; either version 2 of the License, or
 ***    (at your option) any later version.
 ***
 ***    This program is distributed in the hope that it will be useful,
 ***    but WITHOUT ANY WARRANTY; without even the implied warranty of
 ***    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 ***    GNU General Public License for more details.
 ***
 ***    You should have received a copy of the GNU General Public License along
 ***    with this program; if not, write to the Free Software Foundation, Inc.,
 ***    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 ***
 **/

#ifndef ESPTOOL_ELF_H
#define ESPTOOL_ELF_H

#include <inttypes.h>

#define Elf32_Addr              uint32_t
#define Elf32_Half              uint16_t
#define Elf32_Off               uint32_t
#define Elf32_Sword             int32_t
#define Elf32_Word              uint32_t

#define ELF32_ST_BIND(i)        ((i)>>4)
#define ELF32_ST_TYPE(i)        ((i)&0x0F)
#define ELF32_ST_INFO(b,t)      (((b)<<4)+((t)&0x0F))

#define ELF32_R_SYM(i)          ((i)>>8)
#define ELF32_R_TYPE(i)         ((unsigned char)(i))
#define ELF32_R_INFO(s,t)       ((s)<<8)+(unsigned char)(t))

#define SIZE_EI_NIDENT          16

typedef struct {
    unsigned char       e_ident[SIZE_EI_NIDENT];
    Elf32_Half          e_type;
    Elf32_Half          e_machine;
    Elf32_Word          e_version;
    Elf32_Addr          e_entry;
    Elf32_Off           e_phoff;
    Elf32_Off           e_shoff;
    Elf32_Word          e_flags;
    Elf32_Half          e_ehsize;
    Elf32_Half          e_phentsize;
    Elf32_Half          e_phnum;
    Elf32_Half          e_shentsize;
    Elf32_Half          e_shnum;
    Elf32_Half          e_shstrndx;
} Elf32_Ehdr;

typedef struct {
    Elf32_Word          sh_name;
    Elf32_Word          sh_type;
    Elf32_Word          sh_flags;
    Elf32_Addr          sh_addr;    
    Elf32_Off           sh_offset;
    Elf32_Word          sh_size;
    Elf32_Word          sh_link;
    Elf32_Word          sh_info;
    Elf32_Word          sh_addralign;
    Elf32_Word          sh_entsize;
} Elf32_Shdr;

typedef struct {
    Elf32_Word          st_name;
    Elf32_Addr          st_value;
    Elf32_Word          st_size;
    unsigned char       st_info;
    unsigned char       st_other;
    Elf32_Half          st_shndx;
} Elf32_Sym;

typedef struct {
    Elf32_Addr          r_offset;
    Elf32_Word          r_info;
} Elf32_Rel;

typedef struct {
    Elf32_Addr          r_offset;
    Elf32_Word          r_info;
    Elf32_Sword         r_addend;
} Elf32_Rela;

typedef struct {
    Elf32_Word          p_type;
    Elf32_Off           p_offset;
    Elf32_Addr          p_vaddr;
    Elf32_Addr          p_paddr;
    Elf32_Word          p_filesz;
    Elf32_Word          p_memsz;
    Elf32_Word          p_flags;
    Elf32_Word          p_align;
} Elf32_Phdr;

typedef struct {
    Elf32_Sword         d_tag;
    union {
        Elf32_Word      d_val;
        Elf32_Addr      d_ptr;
    } d_un;
} Elf32_Dyn;


unsigned long elf_hash(const unsigned char *name);

#endif