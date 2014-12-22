/**********************************************************************************
 **********************************************************************************
 ***
 ***    esptool_elf_object.h
 ***    - defines and prototypes for accessing ELF object/executable files
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

#ifndef ESPTOOL_ELF_OBJECT_H
#define ESPTOOL_ELF_OBJECT_H

#include <inttypes.h>
#include <stdio.h>

/*
** structs used to hold a local ELF cache object of the informations
** retrieved from the firmware ELF file
*/

/*
** struct holding  the information of a given section according to
** what is found in the ELF file
*/
typedef struct {
    const char         *name;
    Elf32_Word          offset;
    Elf32_Addr          address;
    Elf32_Word          size;
} ELF_section;

/*
** struct defining the local ELF cache object and whatever sections it has
*/
typedef struct {
    FILE                *e_file;
    Elf32_Ehdr          header;
    Elf32_Shdr          section;
    ELF_section         *sections;
    char                *strings;
} ELF_Object;



/*
** creates a local ELF cache object
** filename: the name of the ELF file on disk
*/
int create_elf_object(const char *filename);

/*
** cleanup and release allocated memory for the ELF cahce object
** also closes the the ELF file on disk
*/
int close_elf_object(void);

/*
** list of all section information that is in the ELF cache object
*/
void list_elf_sections(void);

/*
** print the information of a given section header in the ELF cache object
** secnum: the index position of the section header to query
*/
void print_elf_section_info( Elf32_Half secnum );

/*
** return the number of section header stored in the ELF cache object
*/
int get_elf_num_sections(void);

/*
** retrieve the index position of a section, searched for by the sections name
** secname: the neame of the section to look up
*/
int get_elf_secnum_by_name(const char *secname);

/*
** retrieve the binary data of a section
** allocates the memory required for this and  returns a pointer to that byte array
** secnum: index number of the section from which the  binary data should be retrieved
** pad_to: minimum blocksize in-powers-of-two to pad the resulting array to 
*/
unsigned char* get_elf_section_bindata(Elf32_Half secnum, uint32_t pad_to);

/*
** dump the binary data of a section into a file on disk
** secname: the name of the section to read and write out
** fname: the name of the file to write to
*/
int save_elf_section_bindata(const char *secname, const char *fname);

/*
** retrieve the size of the binary data for a section
** secnum: index position of the section
*/
Elf32_Word get_elf_section_size(Elf32_Half secnum);

/*
** retrieve the memory addres as specified in the ELF cache object of the binary data for a section
** secnum: index position of the section
*/
Elf32_Word get_elf_section_addr(Elf32_Half secnum);

/*
** retrieve the code entry point of the ELF file
*/
Elf32_Word get_elf_entry(void);

/*
** retrieve the name of a section
** secnum: index position of the section
** returns a pointer to a string representing the name
*/
const char* get_elf_section_name(Elf32_Half secnum);

#endif
