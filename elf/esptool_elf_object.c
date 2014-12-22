/**********************************************************************************
 **********************************************************************************
 ***
 ***    esptool_elf_object.c
 ***    - implementation of various functions to access ELF object/executable files
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

#include <stdlib.h>
#include <string.h>

#include "infohelper.h"
#include "esptool_elf_object.h"
#include "esptool_elf_enums.h"

static ELF_Object       e_object;

static int check_elf_header(void)
{
    if(fread( (char*) &e_object.header, 1, sizeof(Elf32_Ehdr), e_object.e_file) != sizeof(Elf32_Ehdr))
    {
        info_printf(-1, "can't read ELF file header\r\n");
        return 0;
    }
        

    if(e_object.header.e_ident[EI_MAG0] == 0x7F && 
       e_object.header.e_ident[EI_MAG1] == 'E'  && 
       e_object.header.e_ident[EI_MAG2] == 'L'  && 
       e_object.header.e_ident[EI_MAG3] == 'F')
    {
        return 1;
    }
    
    return 0;
}

void get_elf_strings(void)
{
    if(e_object.header.e_shstrndx)
    {
        if(fseek(e_object.e_file, e_object.header.e_shoff+(e_object.header.e_shstrndx*e_object.header.e_shentsize), SEEK_SET) != 0)
        {
            info_printf(-1, "can't seek to stringtable section info in ELF file\r\n");
            return;
        }
        
        if(fread((char*)&e_object.section, 1, sizeof(Elf32_Shdr), e_object.e_file) != sizeof(Elf32_Shdr) )
        {
            info_printf(-1, "can't read stringtable section info from ELF file!\r\n");
            return;
        }
 
        if(e_object.section.sh_size)
        {
            info_printf(3, "loading string table from ELF file\r\n");
            e_object.strings = malloc(e_object.section.sh_size);
            
            if(e_object.strings == NULL)
            {
                info_printf(-1, "can't malloc memory for stringtable of ELF file\r\n");
                return;
            }
            
            if(fseek(e_object.e_file, e_object.section.sh_offset, SEEK_SET) != 0)
            {
                info_printf(-1, "can't seek to stringtable section info in ELF file\r\n");
                return;
            }
            
            if( fread(e_object.strings, 1, e_object.section.sh_size, e_object.e_file) != e_object.section.sh_size)
            {
                info_printf(-1, "can't read stringtable from ELF file!\r\n");
                return;
            }

        }

    }
    
}

void collect_elf_sections(void)
{
    unsigned int cnt;
    
    e_object.sections = malloc(e_object.header.e_shnum*sizeof(ELF_section));
    if(e_object.sections == NULL)
    {
        info_printf(-1, "can't malloc memory for ELF section list\r\n");
        return;
    }
    
    if(e_object.sections)
    {
        if(e_object.header.e_shnum)
        {
            info_printf(3, "building ELF section list\r\n");
            for(cnt = 1; cnt < e_object.header.e_shnum; cnt++)
            {
                if(fseek(e_object.e_file, e_object.header.e_shoff+(cnt*e_object.header.e_shentsize), SEEK_SET) != 0)
                {
                    info_printf(-1, "can't seek to ELF file section info #%i\r\n", cnt);
                    return;
                }
                
                if(fread((char*)&e_object.section, 1, sizeof(Elf32_Shdr), e_object.e_file) != sizeof(Elf32_Shdr))
                {
                    info_printf(-1, "can't read section info #%i from ELF file\r\n", cnt);
                    return;
                }
                
                if(e_object.section.sh_name)
                {
                    e_object.sections[cnt-1].name = e_object.strings+e_object.section.sh_name;
                }
                else
                {
                    e_object.sections[cnt-1].name = 0;
                }
                
                e_object.sections[cnt-1].offset = e_object.section.sh_offset;
                e_object.sections[cnt-1].address = e_object.section.sh_addr;
                e_object.sections[cnt-1].size = e_object.section.sh_size;
            }
            info_printf(3, "ELF section list created: %i entries\r\n", e_object.header.e_shnum-1);
        }
        else
        {
            info_printf(0, "no sections in ELF object\r\n");
        }
    }
    else
    {
        info_printf(-1, "can't malloc memory for ELF sections list\r\n");
    }
}

void list_elf_sections(void)
{
    unsigned int cnt;
    for(cnt = 1; cnt < e_object.header.e_shnum; cnt++)
    {
        info_printf(4, "ADDR: 0x%08X - SIZE: 0x%08X - OFFSET: 0x%08X - Name: %s\r\n", e_object.sections[cnt-1].address, 
                                                                                  e_object.sections[cnt-1].size, 
                                                                                  e_object.sections[cnt-1].offset, 
                                                                                  e_object.sections[cnt-1].name);
    }
}

void print_elf_section_info(Elf32_Half secnum)
{
    if(secnum && secnum < e_object.header.e_shnum)
    {
        if(fseek(e_object.e_file, e_object.header.e_shoff+(secnum*e_object.header.e_shentsize), SEEK_SET) != 0) 
        {
            info_printf(-1, "can't seek to ELF file section info %i\r\n", secnum);
            return;
        }
        
        if(fread((char*)&e_object.section, 1, sizeof(Elf32_Shdr), e_object.e_file) != sizeof(Elf32_Shdr))
        {
            info_printf(-1, "can't read ELF file section info %i\r\n", secnum);
            return;
        }
        
        if(e_object.section.sh_name)
        {
            info_printf(4, "section name    : %s\r\n",  e_object.strings+e_object.section.sh_name);
        }
        else
        {
            info_printf(4, "section name    : <NONE>\r\n");
        }
        info_printf(4, "sh_name         : 0x%08X\r\n", e_object.section.sh_name);
        info_printf(4, "sh_type         : 0x%08X\r\n", e_object.section.sh_type);
        info_printf(4, "sh_flags        : 0x%08X\r\n", e_object.section.sh_flags);
        info_printf(4, "sh_addr         : 0x%08X\r\n", e_object.section.sh_addr);
        info_printf(4, "sh_offset       : 0x%08X\r\n", e_object.section.sh_offset);
        info_printf(4, "sh_size         : 0x%08X\r\n", e_object.section.sh_size);
        info_printf(4, "sh_link         : 0x%08X\r\n", e_object.section.sh_link);
        info_printf(4, "sh_info         : 0x%08X\r\n", e_object.section.sh_info);
        info_printf(4, "sh_addralign    : 0x%08X\r\n", e_object.section.sh_addralign);
        info_printf(4, "sh_entsize      : 0x%08X\r\n", e_object.section.sh_entsize);
        info_printf(4, "\r\n");
    }
}

int get_elf_num_sections(void)
{
    return e_object.header.e_shnum;
}

int get_elf_secnum_by_name(char *secname)
{
    unsigned int cnt;
    
    for(cnt = 1; cnt < e_object.header.e_shnum; cnt++)
    {
        if( strcmp(e_object.sections[cnt-1].name, secname) == 0)
        {
            info_printf(3, "found section %s at index %i\r\n", secname, cnt);
            return cnt;
        }
    }
    return 0;
}

unsigned char* get_elf_section_bindata(Elf32_Half secnum, uint32_t pad_to)
{
    unsigned char *bindata;
    uint32_t pad_pos;
    
    bindata = 0;
    
    if(secnum && secnum < e_object.header.e_shnum)
    {
        if(e_object.sections[secnum-1].size)
        {
            if(e_object.sections[secnum-1].size < pad_to)
            {
                info_printf(3, "padding section #%i binary data (size 0x%08X) with %i bytes\r\n", secnum, e_object.sections[secnum-1].size, (pad_to-e_object.sections[secnum-1].size));
                bindata = malloc(pad_to);
            }
            else
            {
                bindata = malloc(e_object.sections[secnum-1].size);
            }
            
            if(bindata == NULL)
            {
                info_printf(-1, "can't malloc memory for ELF section %i binary data\r\n", secnum);
                return 0;
            }
            
            if(fseek(e_object.e_file, e_object.sections[secnum-1].offset, SEEK_SET) != 0)
            {
                info_printf(-1, "can't seek to ELF file section %i binary datai\r\n", secnum);
                free(bindata);
                return 0;
            }
            
            if(fread(bindata, 1, e_object.sections[secnum-1].size, e_object.e_file) != e_object.sections[secnum-1].size)
            {
                info_printf(-1, "can't read section %i binary data from ELF file\r\n", secnum);
                free(bindata);
                return 0;
            }
            
            
            for(pad_pos = e_object.sections[secnum-1].size; pad_pos < pad_to; pad_pos++)
            {
                bindata[pad_pos] = 0x00;
            }
            
            return bindata;
        }
    }
    
    return 0;
}


int save_elf_section_bindata( char *secname, char *fname )
{
    FILE *f;
    unsigned char *binblob;
    uint32_t secnum;
    
    secnum = get_elf_secnum_by_name(secname);
    
    binblob = get_elf_section_bindata(secnum, 0);
    
    if(binblob)
    {
        f = fopen( fname, "wb" );
        if(f == NULL)
        {
            info_printf(-1, "can't open file \"%s\" to save binary dump of ELF section \"%s\"\r\n", fname, e_object.sections[secnum-1].name);
            free(binblob);
            return 0;
        }
        
        if( fwrite(binblob, 1, e_object.sections[secnum-1].size, f) != e_object.sections[secnum-1].size)
        {
            info_printf(-1, "can't open file \"%s\" to save binary dump of ELF section \"%s\"\r\n", fname, e_object.sections[secnum-1].name);
            fclose(f);
            free(binblob);
            return 0;
        }
        
        fclose(f);
        free(binblob);
        
        info_printf(2, "saved section \"%s\" to file \"%s\"\r\n", e_object.sections[secnum-1].name, fname);
    }
    return 1;
}

Elf32_Word get_elf_section_size(Elf32_Half secnum)
{
    if(secnum && secnum < e_object.header.e_shnum)
    {
        return e_object.sections[secnum-1].size;
    }
    return 0;
}

Elf32_Word get_elf_section_addr(Elf32_Half secnum)
{
    if(secnum && secnum < e_object.header.e_shnum)
    {
        return e_object.sections[secnum-1].address;
    }
    return 0;
}

char* get_elf_section_name(Elf32_Half secnum)
{
    if(secnum && secnum < e_object.header.e_shnum)
    {
        return e_object.sections[secnum-1].name;
    }
    return 0;
}

Elf32_Word get_elf_entry(void)
{
    return e_object.header.e_entry;
}

int create_elf_object(char *filename)
{
    if(e_object.e_file)
    {
        return 0;
    }
    
    e_object.strings = 0;
    e_object.e_file = 0;
    
    e_object.e_file = fopen(filename, "rb");
    
    if(e_object.e_file)
    {
        info_printf(2, "using ELF file \"%s\"\r\n", filename);

        if( check_elf_header() )
        {
            info_printf(3, "ELF header OK\r\n\r\n");
            
            info_printf(3, "e_type      : 0x%04X\r\n", e_object.header.e_type);
            info_printf(3, "e_machine   : 0x%04X\r\n", e_object.header.e_machine);
            info_printf(3, "e_version   : 0x%08X\r\n", e_object.header.e_version);
            info_printf(3, "e_entry     : 0x%08X\r\n", e_object.header.e_entry);
            info_printf(3, "e_phoff     : 0x%08X\r\n", e_object.header.e_phoff);
            info_printf(3, "e_shoff     : 0x%08X\r\n", e_object.header.e_shoff);
            info_printf(3, "e_flags     : 0x%08X\r\n", e_object.header.e_flags);
            info_printf(3, "e_ehsize    : 0x%04X\r\n", e_object.header.e_ehsize);
            info_printf(3, "e_phentsize : 0x%04X\r\n", e_object.header.e_phentsize);
            info_printf(3, "e_phnum     : 0x%04X\r\n", e_object.header.e_phnum);
            info_printf(3, "e_shentsize : 0x%04X\r\n", e_object.header.e_shentsize);
            info_printf(3, "e_shnum     : 0x%04X\r\n", e_object.header.e_shnum);
            info_printf(3, "e_shstrndx  : 0x%04X\r\n", e_object.header.e_shstrndx);
            
            info_printf(3, "\r\n");
            
            get_elf_strings();
            collect_elf_sections();
            
            return 1;
        }
        else
        {
            info_printf(-1, "wrong header for ELF file!\r\n");
        }
    }
    else
    {
        info_printf(-1, "can't open ELF file %s\r\n", filename);
    }
    
    return 0;
}

int close_elf_object(void)
{
    if(e_object.strings)
    {
        info_printf(4, "releasing memory from ELF file stringtable\r\n");
        free(e_object.strings);
        e_object.strings = 0;
    }
    
    if(e_object.sections)
    {
        info_printf(4, "releasing memory from ELF file section info\r\n");
        free(e_object.sections);
        e_object.sections = 0;
    }

    if(e_object.e_file)
    {
        info_printf(3, "closing ELF file\r\n");
        fclose(e_object.e_file);
        e_object.e_file = 0;
    }
    else
    {
        return 0;
    }
    
    return 1;
}
