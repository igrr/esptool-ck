/**********************************************************************************
 **********************************************************************************
 ***
 ***    esptool_elf_enums.h
 ***    - various enumerations that appear in ELF formatted files
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

#ifndef ESPTOOL_ELF_ENUMS_H
#define ESPTOOL_ELF_ENUMS_H

enum {
    ET_NONE             = 0,
    ET_REL              = 1,
    ET_EXEC             = 2,
    ET_DYN              = 3,
    ET_CORE             = 4,
    ET_LOPROC           = 0xFF00,
    ET_HIPROC           = 0xFFFF
};

enum {
    EM_NONE             = 0,
    EM_M32              = 1,
    EM_SPARC            = 2,
    EM_386              = 3,
    EM_68K              = 4,
    EM_88K              = 5,
    EM_860              = 7,
    EM_MIPS             = 8
};

enum {
    EV_NONE             = 0,
    EV_CURRENT          = 1
};

enum {
    EI_MAG0             = 0,
    EI_MAG1             = 1,
    EI_MAG2             = 2,
    EI_MAG3             = 3,
    EI_CLASS            = 4,
    EI_DATA             = 5,
    EI_VERSION          = 6,
    EI_PAD              = 7,
    EI_NIDENT           = 16
};

enum {
    ELFCLASSNONE        = 0,
    ELFCLASS32          = 1,
    ELFCLASS64          = 2
};

enum {
    ELFDATANONE         = 0,
    ELFDATA2LSB         = 1,
    ELFDATA2MSB         = 2
};

enum {
    SHN_UNDEF           = 0,
    SHN_LORESERVE       = 0xFF00,
    SHN_LOPROC          = 0xFF00,
    SHN_HIPROC          = 0xFF1F,
    SHN_ABS             = 0xFFF1,
    SHN_COMMON          = 0xFFF2,
    SHN_HIRESERVE       = 0xFFFF
};


enum {
    SHT_NULL            = 0,
    SHT_PROGBITS        = 1,
    SHT_SYMTAB          = 2,
    SHT_STRTAB          = 3,
    SHT_RELA            = 4,
    SHT_HASH            = 5,
    SHT_DYNAMIC         = 6,
    SHT_NOTE            = 7,
    SHT_NOBITS          = 8,
    SHT_REL             = 9,
    SHT_SHLIB           = 10,
    SHT_DYNSYM          = 11,
    SHT_LOPROC          = 0x70000000,
    SHT_HIPROC          = 0x7FFFFFFF,
    SHT_LOUSER          = 0x80000000,
    SHT_HIUSER          = 0xFFFFFFFF
};

enum {
    SHF_WRITE           = 0x1,
    SHF_ALLOC           = 0x2,
    SHF_EXECINSTR       = 0x4,
    SHF_MASKPROC        = 0xF0000000
};


enum {
    STB_LOCAL           = 0,
    STB_GLOBAL          = 1,
    STB_WEAK            = 2,
    STB_LOPROC          = 13,
    STB_HIPROC          = 15
};

enum {
    STT_NOTYPE          = 0,
    STT_OBJECT          = 1,
    STT_FUNC            = 2,
    STT_SECTION         = 3,
    STT_FILE            = 4,
    STT_LOPROC          = 13,
    STT_HIPROC          = 15
};


enum {
    R_386_NONE          = 0,
    R_386_32            = 1,
    R_386_PC32          = 2,
    R_386_GOT32         = 3,
    R_386_PLT32         = 4,
    R_386_COPY          = 5,
    R_386_GLOB_DAT      = 6,
    R_386_JMP_SLOT      = 7,
    R_386_RELATIVE      = 8,
    R_386_GOTOFF        = 9,
    R_386_GOTPC         = 10,
};


enum {
    PT_NULL             = 0,
    PT_LOAD             = 1,
    PT_DYNAMIC          = 2,
    PT_INTERP           = 3,
    PT_NOTE             = 4,
    PT_SHLIB            = 5,
    PT_PHDR             = 6,
    PT_LOPROC           = 0x70000000,
    PT_HIPROC           = 0x7FFFFFFF,
};

enum {
    DT_NULL             = 0,
    DT_NEEDED           = 1,
    DT_PLTRELSZ         = 2,
    DT_PLTGOT           = 3,
    DT_HASH             = 4,
    DT_STRTAB           = 5,
    DT_SYMTAB           = 6,
    DT_RELA             = 7,
    DT_RELASZ           = 8,
    DT_RELAENT          = 9,
    DT_STRSZ            = 10,
    DT_SYMENT           = 11,
    DT_INIT             = 12,
    DT_FINI             = 13,
    DT_SONAME           = 14,
    DT_RPATH            = 15,
    DT_SYMBOLIC         = 16,
    DT_REL              = 17,
    DT_RELSZ            = 18,
    DT_RELENT           = 19,
    DT_PLTREL           = 20,
    DT_DEBUG            = 21,
    DT_TEXTREL          = 22,
    DT_JMPREL           = 23,
    DT_LOPROC           = 0x70000000,
    DT_HIPROC           = 0x7FFFFFFF 
};

#endif
