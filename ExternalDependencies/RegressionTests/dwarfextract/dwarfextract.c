/*
 * This file is the source for a tool that will extract types (structures,
 * typedefs, etc.) from a kernel or other binary that has been compiled
 * with debugging information.
 *
 * Created by Silicon Graphics, Inc.
 *
 * Copyright (C) 2005 Silicon Graphics, Inc. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */
/*
 * dwarfextract
 *
 *  Extracts the types out of a -g binary (esp. a DEBUG_INFO=y kernel).
 *  These types can be used by lcrash as its -t or --types ("kerntypes") file.
 *
 *  usage: dwarfextract debugkernel kerntypes
 *
 *  Based on dwarfdump
 *    cwickman  5/2005
 *
 *  note (01/2008) [dgollub]:
 *    Happy new year!
 *    Initial ELF RELA relocation implementation got introduced:
 *    dwarf_elf_relocation(Dwarf_Debg, Dwarf_Error*)    
 *
 *  note (12/2007) [dgollub]: 
 *   slurp implementation of relocation information got dropped.
 *   dwarfextract builds now standalone, no access to OPAQUE libdwarf structs.
 *   This breaks RELA relocations!
 *
 *  note (6/2005): lcrash depends upon some structures in the kerntypes
 *   file that may not be in the kernel (they may be, if you grab all modules).
 *   If you don't extract from the modules you should do a
 *   "dwarfextract alltypes -c Kerntypes" to add the missing ones from the
 *   standard Kerntypes file.
 *
 *  note: (8/2005)
 *   for lcrash's benefit, we could create typedef something like this:
 *   __linux_compile_version_id__uran_Sat_Jul_23_02_20_49_UTC_2005_t
 *   to identify the version of types file we are creating.
 *   (else it will complain "types: No version info found!")
 *
 *  note: (9/2005)
 *   a very big -C file has been known to cause a SEGV in realloc() (when
 *   a malloc'd area being enlarged was initially very big).
 *   (e.g. INITIAL_NEEDPOINTERS  of 35000 caused the failure when it was
 *    exceeded.)
 *   The reason for the failure is still unknown, but the problem went away
 *   when the initial size guesses were set higher or lower.
 *
 *  note: (9/2005)
 *   extraction from a kernel alone took about 90 seconds
 *   extraction from a community kernel and 55 modules took about 5 minutes
 *   extraction from the 2.6 kernel and 1079 modules (sles9) took 2 hours!
 */
/*#include <unistd.h> */
#include <sys/types.h> 
/* typedef __off64_t off64_t; */ /* No longer needed. */
#include <libelf.h>
#include <fcntl.h>
#include <getopt.h>
#include <libdwarf.h>
#include <dwarf.h>

/* For freebsd 10.2, need defines for bfd.h */
#define PACKAGE "dwarfextract-test"
#define PACKAGE_VERSION "1.0"
#define ATTRIBUTE_UNUSED
#define ENUM_BITFIELD(z) enum z
/* End required mess. */

#include <sys/time.h>

#ifdef OPAQUE_RELOCATION
#warning "WARNING: OPAQUE_RELOCATION implementation is used. This requires Private API access to libdwarf!"
#include "dwarf_base_types.h"
#include "dwarf_alloc.h"
#include "dwarf_opaque.h"
#endif /* OPAQUE_RELOCATION */

#include <stdio.h>
#include <errno.h>
/*#include <malloc.h>  */ /* stdlib.h has malloc now. */
#include <string.h>
#include <sys/time.h>
#include <bfd.h>
#include <assert.h>
#include <stdlib.h>
#include <unistd.h>
#include <stddef.h>
#define BUFSIZE 1024
#define NUM_SECTIONS 5
/* For testing dwarfdump, force __WORDSIZE to 32. */
#undef __WORDSIZE
#define __WORDSIZE 32

#ifndef __WORDSIZE
#define __WORDSIZE 32
#endif
#if __WORDSIZE == 32
#define ELF_HEADER Elf32_Ehdr
#define SECTION_HEADER Elf32_Shdr
#define VERSION_FIELD Elf32_Word
#else
#define ELF_HEADER Elf64_Ehdr
#define SECTION_HEADER Elf64_Shdr
#define VERSION_FIELD Elf64_Word
#endif
#define SECTION_DEBUGABBREV 1
#define SECTION_DEBUGINFO 2
#define SECTION_RELDEBUGINFO 3
#define SECTION_STRING 4
#define EV_DWARFEXTRACT 101010101

/* assumed size of a pointer - this is just used for attribute storage size,
   so it works fine on 32-bit as well */
#define PTRSIZE 8
/* number of files allowed with -c; could be raised to any size but the max.
   size of a command line is a practical limit; use -C for long lists  */
#define NUMCFILES 255
/* make space for a large number die references that need translating */
/* the below 5 are arbitrary, but set high so that an extract of all the
    types in a 2.6 kernel + modules does not cause realloc warning messages */
/* might as well be set high, as the malloc will not actually consume pages
   until they are used */
/* max number of dies referencing something else (will need a ref. attribute):*/
#define INITIAL_REFERENCE     170000
/* max number of pieces in a debugging section: */
#define INITIAL_PIECES	       1000
/* max number of structure prototypes in a debugging section: */
#define INITIAL_STRUCTPROTOS 130000
/* max number of dies that need pointers added to them: */
#define INITIAL_NEEDPOINTERS  75000
/* max number of dies that need array links added to them: */
#define INITIAL_NEEDARRAYS    10000

#define FMAX 10000 /* max. number of files to concatenate */
#define LNOTFOUND 0
#define LFOUND    1
#define LUNPTR    2
#define LUNARRAY  3
#define PTRLINK   1
#define ARRAYLINK 2
extern	int	optind;
extern	char	*optarg;
/* the avl_ routines are copied from the GNU libavl */
/* libavl - library for manipulation of binary trees.
   Copyright (C) 1998-2002 Free Software Foundation, Inc.

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
   See the GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
   02111-1307, USA.

   The author may be contacted at <blp@gnu.org> on the Internet, or
   write to Ben Pfaff, Stanford University, Computer Science Dept., 353
   Serra Mall, Stanford CA 94305, USA.
*/
/* begin avl.h stuff */
typedef int avl_comparison_func (const void *avl_a, const void *avl_b,
                                 void *avl_param);
struct libavl_allocator
{
	void *(*libavl_malloc) (struct libavl_allocator *, size_t libavl_size);
	void (*libavl_free) (struct libavl_allocator *, void *libavl_block);
};
void *avl_malloc (struct libavl_allocator *, size_t);
void avl_free (struct libavl_allocator *, void *);
struct libavl_allocator avl_allocator_default = {
	avl_malloc,
	avl_free
};
#define AVL_MAX_HEIGHT 32
struct avl_table {
	struct avl_node *avl_root;          /* Tree's root. */
	avl_comparison_func *avl_compare;   /* Comparison function. */
	void *avl_param;                    /* Extra argument to |avl_compare|. */
	struct libavl_allocator *avl_alloc; /* Memory allocator. */
	size_t avl_count;                   /* Number of items in tree. */
	unsigned long avl_generation;       /* Generation number. */
};
struct avl_node {
    struct avl_node *avl_link[2];  /* Subtrees. */
    void *avl_data;                /* Pointer to data. */
    signed char avl_balance;       /* Balance factor. */
};
struct avl_traverser {
    struct avl_table *avl_table;        /* Tree being traversed. */
    struct avl_node *avl_node;          /* Current node in tree. */
    struct avl_node *avl_stack[AVL_MAX_HEIGHT];
                                        /* All the nodes above |avl_node|. */
    size_t avl_height;                  /* Number of nodes in |avl_parent|. */
    unsigned long avl_generation;       /* Generation number. */
};
struct avl_table *avl_create (avl_comparison_func *, void *,
                              struct libavl_allocator *);
void **avl_probe (struct avl_table *, void *);
void *avl_insert (struct avl_table *, void *);
void avl_t_init (struct avl_traverser *, struct avl_table *);
void *avl_t_first (struct avl_traverser *, struct avl_table *);
void *avl_t_next (struct avl_traverser *);
void trav_refresh (struct avl_traverser *);
/* end avl.h stuff */
int	firstnewdie=1, verbose=0, debug=0, tree_level;
int	start_count_level, member_hash_total, no_input_rel_debug_info_section=0;
int	out_string_size, file_debug_offset, in_place=0, tree_total=0;
int	num_cfiles=0, current_file_number=1, Hflag=0, Sflag=0, mflag=0;
int	num_refchecks, size_refchecks, aflag=0, iflag=0;
int	num_aliases=0, cflag=0, root_cus=0, sflag=0, gflag=0;
int	num_abbrev_pieces, num_debug_pieces, num_reldebug_pieces, Tflag=0;
int	size_abbrev_pieces, size_debug_pieces, size_reldebug_pieces;
int	total_dies=0, total_duplicates=0, total_cus=0, total_newdies=0;
int	tflag=0, pflag=0, total_structs=0, duplicate_structs=0, total_types=0;
int	reldebug_idx, debug_idx, abbrev_idx, string_idx, reldebug_strlen;
int	abbrev_size, debug_size, reldebug_size, total_unnamed_structs=0;
int	lookupcnt=0, addcnt=0, tree_branches, rflag=0, oflag=0, startsec;
int	tested_dies=0, deepest_nesting=0, num_refhash, Pflag=0;
int	num_protos, size_protos, num_needptrs, size_needptrs, scan_one_file=0;
int	num_needarrs, size_needarrs, *tree_namehashp, num_namehash;
int	is_32_bit=0, is_64_bit=0, needs_old;
float	lookuptime=0.0, addtime=0.0;
char	myno_die_name[] = "NO_NAME", *groupnamep, *iname;
char	*program_name, *in_file_name, *out_file_name, debug_buffer[BUFSIZE];
char	indentation_string[100], indentation_spaces[100];
char	reldebug_string[] = {".rel.debug_info\0"};
char	**abbrev_addrp, **debug_addrp, **reldebug_addrp;
char 	*in_string_buffer, *oname;
char    **cfilep, **struct_protonamep;
Dwarf_Unsigned	*abbrev_lengthp, *debug_lengthp, *reldebug_lengthp;
/* Dwarf_Off is 8 bytes on both 32-bit and 64-bit Intel */
Dwarf_Off *struct_protop, *struct_protop2;
Dwarf_Off *needptr_refp, *needptr_refp2, *needarr_refp, *needarr_refp2;
Dwarf_Off *needarr_origp, *needarr_origp2, *tree_refhashp;
ELF_HEADER	inelfhdr, outelfhdr;
SECTION_HEADER	*base_shp, *debug_shp;
Dwarf_Unsigned	cu_offset = 0, producer_flags;
Dwarf_Ptr	producer_error;
Dwarf_P_Die	root_p_die, *ref_diep, *ref_diep2, void_pointer_die=0;
Dwarf_P_Die	common_base_type_die, *needptr_pdiep, *needarr_pdiep;
Dwarf_P_Die	*needptr_pdiep2, *needarr_pdiep2;
Dwarf_Off	*ref_refp, *ref_refp2;
Dwarf_P_Debug	newdbg;  /* global, the new "Producer" dbg */
Dwarf_Debug	*needarr_dbgp, *needarr_dbgp2;
const char *   producer_isa = "x86";
const char *   producer_version = "V2";
void *   producer_user_data = (void *)101;
/* this is the type information of each unique type that we have
   saved as a new Dwarf_P_Die */
struct typenode {
	int members;
	unsigned long long int hash;
	int size;
	int declaration; /* this is a structure declaration */
	Dwarf_Off tag;
	char *namep;
	/* the above 5 (combined) form the key to the node (sort order) */
	Dwarf_P_Die pdie;
	Dwarf_P_Die ptrto;  /* unnamed pointer case */
	Dwarf_P_Die arrto;  /* unnamed array link case */
	Dwarf_Off   offset; /* needed for a search by offset */
};
/* this is the type information of duplicates (already captured in the
   typenode tree (tree_base)) */
struct aliasnode {
	Dwarf_Off	ref;   	/* sorted by this (offset+FMAX*file) */
	struct typenode *typep; /* the type that it is a duplicate of */
};
struct  avl_table *tree_base, *alias_tree;
struct typenode **alias_typep, **alias_typep2,
	 **needptr_typep, **needarr_typep,
	 **needptr_typep2, **needarr_typep2,
	 **tree_nametypep, **tree_reftypep;
/* stuff from readelf   binutils-<rlse>/binutils    */
/*	              binutils-<rlse>/include/elf */
typedef struct elf_internal_rela {
	bfd_vma	r_offset;	/* Location at which to apply the action */
	bfd_vma	r_info;		/* Index and Type of relocation */
	bfd_vma	r_addend;	/* Constant addend used to compute value */
} Elf_Internal_Rela;
struct elf_internal_sym { /* Symbol table entry */
	bfd_vma	st_value;	/* Value of the symbol */
	bfd_vma	st_size;	/* Associated symbol size */
	unsigned long	st_name;	/* Symbol name, index in string tbl */
	unsigned char	st_info;	/* Type and binding attributes */
	unsigned char	st_other;	/* Visibilty, and target specific */
	unsigned int	st_shndx;	/* Associated section index */
};
typedef struct elf_internal_sym Elf_Internal_Sym;
typedef struct {
	unsigned long  cu_length;
	unsigned short cu_version;
	unsigned long  cu_abbrev_offset;
	unsigned char  cu_pointer_size;
} DWARF2_Internal_CompUnit;
typedef struct {
	unsigned char	r_offset[8];	/* Location at which to apply the
					   action */
	unsigned char	r_info[8];	/* index and type of relocation */
	unsigned char	r_addend[8];	/* Constant addend used to compute
					   value */
} Elf64_External_Rela;
static	bfd_vma (*byte_get) (unsigned char *, int);
static	void (*byte_put) (unsigned char *, bfd_vma, int);
struct timeval	tv;
struct timezone	tz;
struct avl_traverser	my_trav;
/* If we can support a 64 bit data type then BFD64 should be defined
   and sizeof (bfd_vma) == 8.  In this case when translating from an
   external 8 byte field to an internal field, we can assume that the
   internal field is also 8 bytes wide and so we can extract all the data.
   If, however, BFD64 is not defined, then we must assume that the
   internal data structure only has 4 byte wide fields that are the
   equivalent of the 8 byte wide external counterparts, and so we must
   truncate the data.  */
#ifdef  BFD64
#define BYTE_GET8(field)	byte_get (field, -8)
#else
#define BYTE_GET8(field)	byte_get (field, 8)
#endif
/* end of readelf stuff */

/* function prototypes */
static int producer_callback(const char *, int, Dwarf_Unsigned,
	Dwarf_Unsigned, Dwarf_Unsigned, Dwarf_Unsigned, int *, int*);
static int producer_callback_c(const char *name, int size, Dwarf_Unsigned type,
        Dwarf_Unsigned flags, Dwarf_Unsigned link, Dwarf_Unsigned info,
        Dwarf_Unsigned *sect_name_index, void *user_data,int *error);

int	process_one_file(Elf *, int intfd, int outfd);
int	is_type_we_want(Dwarf_Debug, Dwarf_Die, int *);
int	in_string_size;
int	decode_unsigned_leb128 (unsigned char *, int *);
int	get_offset(Dwarf_Debug, Dwarf_Attribute, Dwarf_Half);
int	is_a_struct(Dwarf_Die);
int	is_a_group(Dwarf_Die);
int	lookuptype(Dwarf_Debug, Dwarf_Die, struct avl_table *, int,
	  unsigned long long int, int, Dwarf_Half, char *,
	  struct typenode **, int *);
int	get_debug_sectionhead(int, SECTION_HEADER **, SECTION_HEADER **,
							int, int, int, int);
int	file_is_relocatable(int);
int	process_concat_file(Elf *, char *, int, int);
int	child_count(Dwarf_Debug, Dwarf_Die);
int	has_attr_group(Dwarf_Debug, Dwarf_Die, Dwarf_Die *);
int	is_unnamed_pointer(Dwarf_Debug, Dwarf_Die, Dwarf_Half, char *);
int	is_unnamed_array(Dwarf_Debug, Dwarf_Die, Dwarf_Half, char *);
int	get_attr(Dwarf_Die, Dwarf_Half, Dwarf_Attribute *);
int	get_child(Dwarf_Die, Dwarf_Die *);
int	get_upper_bound(Dwarf_Attribute attr);
int	lookup_type_name(char *, struct typenode **);
int	lookup_type_ref(Dwarf_Off, struct typenode **);
int	lookup_needarr_ref(Dwarf_Off, int *, int *);
int	lookup_needptr_ref(Dwarf_Off, int *, int *);
int	lookup_reference_ref(Dwarf_Off, int *, int *);
int	get_bound (Dwarf_Die);
int	get_sibling(Dwarf_Debug, Dwarf_Die, Dwarf_Die *);

#ifdef OPAQUE_RELOCATION
int	reloc_debug_info (unsigned char *, unsigned char *, int);
#endif /* OPAQUE_RELOCATION */

char	*get_strings(int infd, int adjust);
char	*get_strings_basic(int, SECTION_HEADER *, int);
char	*die_type (Dwarf_Die);
char	*name_of_die(Dwarf_Die);
char	*get_reftype(Dwarf_Die, Dwarf_Debug);
void	inserttype(int, unsigned long long int,
		int, Dwarf_Off, char *, Dwarf_P_Die, Dwarf_Off, int);
void	addtosavedtypes(char *, int, unsigned long long int, Dwarf_P_Die,
		Dwarf_Die, int, Dwarf_Half, int);
void	print_a_die(Dwarf_Debug, Dwarf_Die);
void	test_a_die(Dwarf_Debug, Dwarf_Die);
void	walk_die_and_children(Dwarf_Debug, Dwarf_Die, Dwarf_P_Die);
void	display_die_and_children(Dwarf_Debug, Dwarf_Die);
void	walk_cus(Dwarf_Debug, int);
void	usage();
void	init_refs_area();
void	init_pieces_area();
void	Cfilenames(char *);
void	get_options(int argc, char *argv[]);
void	setup_input_file(int infd);
int	open_input__files (const char *);
int	open_output_files (const char *);
void	add_to_trans_list(Dwarf_P_Die, Dwarf_Half);
void	do_reference_translations();
void	addtoreflist(Dwarf_P_Die, Dwarf_Off);
void	trace_file(Elf *, int);
void	list_attributes(Dwarf_Debug, Dwarf_Die);
void	trace_input_elf_file(int, char *);
void	print_reference(Dwarf_Debug, Dwarf_Attribute, Dwarf_Half);
void	get_type_values(Dwarf_Debug, Dwarf_Die, char **,
	unsigned long long int *, int *, int *, Dwarf_Half *, Dwarf_Off *,
	int *);
void	save_abbrev_piece(char *, Dwarf_Unsigned);
void	save_debug_piece(char *, Dwarf_Unsigned);
void	save_reldebug_piece(char *, Dwarf_Unsigned);
void	producer_errhandler(Dwarf_Error, Dwarf_Ptr);
void	ref_summary();
void	add_alias_tree(Dwarf_Die, struct typenode *, Dwarf_Off);
void	printTree(struct avl_table *);
void	concatenate (Elf *, int infd, int outfd);
void	write_output_file(int outfd);
void	*get_data (void *, int, long, size_t);
void	byte_put_little_endian (unsigned char *, bfd_vma, int);
void	byte_put_big_endian (unsigned char *, bfd_vma, int);
void	read_elf_header(ELF_HEADER *, int);
void	test_endianness(ELF_HEADER *);
void	show_structure(Dwarf_Debug, Dwarf_Die, char *, int, int);
void	show_die_offset(Dwarf_Debug, Dwarf_Die, int, int);
void	show_enum(Dwarf_Debug, Dwarf_Die, char *, int, int);
void	show_typedef(Dwarf_Debug, Dwarf_Die, char *, int, int);
void	show_union(Dwarf_Debug, Dwarf_Die, char *, int, int);
void	get_tag(Dwarf_Die, Dwarf_Half *);
void	add_to_needs_ptr_list(Dwarf_Off, Dwarf_P_Die);
void	add_to_needs_arr_list(Dwarf_Debug, Dwarf_Off, Dwarf_Off, Dwarf_P_Die);
void	add_to_proto_list(Dwarf_Off, Dwarf_Off, char *);
void	link_to_pointer(Dwarf_P_Die, Dwarf_P_Die);
void	get_refoffset(Dwarf_Attribute, Dwarf_Off *);
void	get_die(Dwarf_Debug, Dwarf_Off, Dwarf_Die *);
void	add_subranges(Dwarf_P_Die, Dwarf_Debug, Dwarf_Off);
void	sort_aliases();
void	sort_references();
void	make_types_name_ref_hash();
void	walkTree_name_ref(struct avl_table *);
void	sort_treenames();
void	sort_treerefs();
void	sort_needptrs();
void	sort_needarrs();
void	init_misc_area();
void	show_array(Dwarf_Debug, Dwarf_Die, char *, int , int);
void	set_version(char *);
int	my_params = 0;
void	printNodes(struct avl_node *, int);
Dwarf_P_Die convert_to_new(Dwarf_Debug, Dwarf_P_Debug, Dwarf_Die, int);
Dwarf_P_Die convert_subrange(Dwarf_Debug, Dwarf_P_Debug, Dwarf_Die);
Dwarf_P_Die make_pointer_die(Dwarf_P_Die);
Dwarf_P_Die make_array_die(Dwarf_P_Die);
Dwarf_P_Die *alloc_pdie_list(int);
Dwarf_Ptr _dwarf_p_get_alloc(Dwarf_P_Debug, Dwarf_Unsigned);
Dwarf_Off current_offset(Dwarf_Die);
Dwarf_Off *alloc_offset_list(int);
Dwarf_Debug *alloc_dbg_list(int);
struct typenode * NewtypeNode(int, unsigned long long int, int, Dwarf_Off,
		char *, Dwarf_P_Die, Dwarf_Off, int);
struct typenode **alloc_type_list(int);
struct typenode *avl_find(struct avl_table *, void *);
struct aliasnode *lookup_alias_ref(Dwarf_Off);
Elf	*open_as_elf(int, char *);
int	typenode_compare(const void *, const void *, void *);
int	alias_compare(const void *, const void *, void *);
bfd_vma	byte_get_little_endian (unsigned char *, int);
bfd_vma	byte_get_big_endian (unsigned char *, int);

#ifdef OPAQUE_RELOCATION
int	slurp_rela_relocs (int, unsigned long, unsigned long,
	 Elf_Internal_Rela **, unsigned long *);

int	debug_apply_rela_addends(int, SECTION_HEADER *, SECTION_HEADER *,
	  int, unsigned char *, unsigned char *, unsigned char *, int, int);
#endif /* OPAQUE_RELOCATION */

int
open_input_files (const char *in_file_name)
{
	int infd;
	if ((infd = open(in_file_name, O_RDONLY)) < 0) {
		printf ("%s: open of %s failed\n", program_name, in_file_name);
		exit(1);
	}

	return infd;
}


int
open_output_files (const char *out_file_name)
{
	int outfd;
	if ((outfd = open(out_file_name, O_RDONLY)) < 0) {
		if ((outfd = open(out_file_name, O_RDWR| O_CREAT, 0600)) < 0) {
			printf ("%s: creation of %s failed\n",
				program_name, out_file_name);
			exit(1);
		}
	} else {
		/* output file already exists - overwrite it */
		close (outfd);
		if ((outfd = open(out_file_name,
					O_RDWR|O_CREAT|O_TRUNC, 0600)) < 0) {
			printf ("%s: creation of %s failed\n",
				program_name, out_file_name);
			exit(1);
		}
	}

	return outfd;
}

int
main(int argc, char *argv[])
{
	Elf		*elf;
	int		i;
	int		outfd = -1, infd;

	for (i=0; i<100; i++) {
		indentation_spaces[i]=' ';
	}
	indentation_spaces[99]='\0';
	(void) elf_version(EV_NONE);
	if (elf_version(EV_CURRENT) == EV_NONE) {
		(void) fprintf(stderr, "dwarfextract: libelf.a out of date.\n");
		exit(1);
	}

	get_options(argc, argv);

	if (mflag) {
		set_version(in_file_name);
		exit(0);
	}

	if (Pflag) {
		gettimeofday(&tv, &tz);
		startsec = tv.tv_sec;
	}

	/*returns infd*/ 
	infd = open_input_files(in_file_name);

	/* just reading one file for -t, -T, -a or -g */
	/* if using -c and only an input file given,
	   we will update it in place */
	if (!(scan_one_file || in_place))
		outfd = open_output_files(out_file_name);

	/* convert the open fd to an elf descriptor for some routines
	   below */

	elf = open_as_elf(infd, in_file_name);

	if (scan_one_file) {
		trace_input_elf_file(infd, in_file_name);
		trace_file(elf, infd);
	} else {
		init_refs_area();
		init_misc_area();
		init_pieces_area();
		tree_base = avl_create (typenode_compare, &my_params,
					&avl_allocator_default);
		alias_tree = avl_create (alias_compare, &my_params,
					&avl_allocator_default);
		setup_input_file(infd);

		if (cflag) {
			concatenate(elf, infd, outfd);
		} else {
			process_one_file(elf, infd, outfd);
		}

		/* HACK HACK HACK AHCK */
		avl_free(&avl_allocator_default, tree_base);
		avl_free(&avl_allocator_default, alias_tree);
	}

	elf_end(elf);
	close (infd);

	if (rflag) {
		printf ("tested %d dies; deepest nesting: %d\n",
			tested_dies, deepest_nesting);
	}

	if (tflag || Tflag || gflag || aflag || Hflag || Sflag || rflag ||
		oflag) {
		exit(0);
	}

	close (outfd);
	if (Pflag) {
		gettimeofday(&tv, &tz);
		printf ("elapsed time: %ld seconds\n", tv.tv_sec-startsec);
	}
	exit(0);
}

Elf *
open_as_elf(int fd, char *file_name)
{
	Elf_Cmd		cmd;
	Elf		*elf;
	Elf32_Ehdr	*eh32;
	Elf64_Ehdr	*eh64;

	cmd = ELF_C_READ;
	elf = elf_begin(fd, cmd, (Elf *) 0);
	if (elf_kind(elf) == ELF_K_AR) {
		printf ("%s is an archive; aborting\n", file_name);
		exit(1);
	}

	eh32 = elf32_getehdr(elf);
	if (!eh32) {
		/* not a 32-bit obj */
		/* must be a 62-bit obj */
		eh64 = elf64_getehdr(elf);
		is_64_bit++;
		if (!eh64) {
			printf ("%s is neither 32 nor 64-bit\n", file_name);
			exit(1);
		} else {
			if (debug) {
				printf ("process the file %s\n",
					file_name);
			}
			producer_flags = DW_DLC_SIZE_64;
		}
	} else {
		is_32_bit++;
		/* a 32-bit obj */
		if (debug) {
			printf ("process the file %s\n", file_name);
		}
		producer_flags = DW_DLC_SIZE_32;
	}
	return elf;
}



/*
 * Given a file which we know is an elf file, process the dwarf data.
 */
int
process_one_file(Elf * elf, int infd, int outfd)
{
	int		dres = 0;
	Dwarf_Debug	dbg = 0;
	Dwarf_Error	error = 0; 
#ifdef PRODUCER_INIT_C
        Dwarf_Ptr       errarg = 0;
#endif

	dres = dwarf_elf_init(elf, DW_DLC_READ, NULL, NULL, &dbg, &error);
	if (dres == DW_DLV_NO_ENTRY) {
		printf("No DWARF information present in %s\n", in_file_name);
		return 0;
	}
	if (dres != DW_DLV_OK) {
		printf ("dwarf_elf_init failed\n");
		exit(1);
	}

        printf("Using original dwarf_producer_init\n");
	dres = dwarf_producer_init(producer_flags, 
            producer_callback_c,
	    producer_errhandler, 
            producer_error, 
            producer_user_data,
            producer_isa,
            producer_version,
            0, /* No extra args. */
            &newdbg,
            &error);

	if (dres != DW_DLV_OK) {
		printf ("dwarf_producer_init failed\n");
		exit(1);
	}
	if (debug) {
		printf ("newdbg created at %p\n", newdbg);
	}

	common_base_type_die = (Dwarf_P_Die)0;
	/*
 	 * Iterate through dwarf and extract all type info.
 	 */
	if (debug) {
		printf ("walking the CUs tree\n");
	}
	walk_cus(dbg, infd);
	if (pflag) {
		printf ("%ld types in tree\n", (long)tree_base->avl_count);
	}
	if (debug) {
		printf ("total new dies created %d\n", total_newdies);
	}

	do_reference_translations();

	dres = dwarf_finish(dbg, &error);
	if (dres != DW_DLV_OK) {
		printf ("dwarf_finish failed\n");
		exit(1);
	}

	get_strings(infd, 1); /* get strings from input file and adjust */
	write_output_file(outfd);

	if (!sflag) {
		printf ("input file:\t\t\t\t%s\n", in_file_name);
		printf ("compilation units:\t\t%d\n", total_cus);
		printf ("structures captured:\t\t%d\n", total_structs);
		printf ("output file:\t\t\t\t%s\n", out_file_name);
	}
	if (debug) {
		printf ("final space usages:\n");
		printf ("num_refchecks:    %d\n", num_refchecks);
		printf ("num_aliases:      %d\n", num_aliases);
		printf ("num_debug_pieces: %d\n", num_debug_pieces);
	}
	return 0;
}

/*
 * Given a file which we know is an elf file, add its dwarf data to what
 * we've already added to newdbg.
 */
int
process_concat_file(Elf * elf, char *filename, int fd, int filenumber)
{
	int		dres;
	Dwarf_Debug	concat_dbg;
	Dwarf_Error	error; /* a structure */

	total_cus = total_dies = total_duplicates =
	total_structs = total_unnamed_structs = duplicate_structs = 0;

	dres = dwarf_elf_init(elf, DW_DLC_READ, NULL, NULL, &concat_dbg,
					&error);
	if (dres == DW_DLV_NO_ENTRY) {
		printf("No DWARF information present in %s\n", filename);
		return 0;
	}
	if (dres != DW_DLV_OK) {
		printf ("dwarf_elf_init failed\n");
		exit(1);
	}

	if (!sflag) {
		printf ("%d/%d, concatenating file %s\n",
			filenumber+1, num_cfiles, filename);
	}

	/*
 	 * Iterate through dwarf and extract all type info.
 	 */
	if (debug) {
		printf ("walking the CUs tree for %s\n", filename);
	}
	walk_cus(concat_dbg, fd);
	if (debug) {
		printf ("total new dies created %d\n", total_newdies);
	}

	if (needs_old == 0) {
		/* leave the files open because we need the dbg
		   in place for needarr_dbgp use; unless this file
		   had none of them */
		dres = dwarf_finish(concat_dbg, &error);
	}

	if (!sflag) {
		printf ("  compilation units:\t\t%d\n", total_cus);
		printf ("  structures captured:\t\t%d\n", total_structs);
		printf ("  types in tree:\t\t%ld\n", (long)tree_base->avl_count);
	}
	return 0;
}

void
write_output_file(int outfd)
{
	int		i, j, coffset, sh_size, string_offset;
	int		sh_offset, sections_offset;
	int		abbrev_offset, debug_offset, reldebug_offset;
	Dwarf_Error	error;
	Dwarf_Signed	numsections, section, elf_section_index;
	Dwarf_Unsigned	length;
	Dwarf_Ptr	gres;
	SECTION_HEADER	*shp1;

	/* write the new ELF header */
	/* coffset = lseek(outfd, 0, SEEK_CUR); */
	if ((i = write(outfd, &outelfhdr, sizeof(outelfhdr))) !=
							sizeof(outelfhdr)) {
		printf ("cannot write elf header to %s - %s\n",
				in_place ? in_file_name : out_file_name,
				errno ? strerror(errno) : ". abort!");
		exit(1);
	}

	if (pflag) {
		printf ("transforming to disk form and ");
		printf("writing file %s\n",
			in_place ? in_file_name : out_file_name);
	}
	numsections = dwarf_transform_to_disk_form (newdbg, &error);
	if (numsections == DW_DLV_NOCOUNT) {
		printf ("dwarf_transform_to_disk_form failed\n");
		exit(1);
	}

	if (debug) {
		printf ("transformed %lld disk image pieces\n", (long long)numsections);
	}
	gres = (Dwarf_Ptr)1;
	section = 0;
	while (gres) {
		if (debug) {
			printf (
			   "calling dwarf_get_section_bytes for section %lld\n",
				(long long)section);
		}

		gres = dwarf_get_section_bytes(newdbg, section,
					&elf_section_index, &length, &error);
		if (gres == NULL) {
			if (debug) {
				printf (
			      "dwarf_get_section_bytes returned NULL at %lld\n",
					(long long)section);
			}
			if ((num_abbrev_pieces   < 1) ||
			    (num_debug_pieces    < 1) ||
			    (num_reldebug_pieces < 1)) {
				printf (
				    "did not get the expected 3 sections\n");
				exit(1);
			}
		} else {
			/* we may get each section a piece at a time; we'll
			   have to save an array of starts and lengths */
			if (debug) {
				printf ("dwarf_get_section_bytes index:%lld ",
					(long long)elf_section_index);
				printf ("length:%lld\n", (long long)length);
			}
			if (elf_section_index == SECTION_DEBUGABBREV) {
				save_abbrev_piece((char *)gres, length);
			} else if (elf_section_index == SECTION_DEBUGINFO) {
				save_debug_piece((char *)gres, length);
			} else if (elf_section_index == SECTION_RELDEBUGINFO) {
				save_reldebug_piece((char *)gres, length);
			} else {
				printf (
	   "dwarf_get_section_bytes returned unexpected section number %lld\n",
				(long long)elf_section_index);
				exit(1);
			}
		}
		section++;
	}

	/* future file location of section header table and sections */
	sh_offset = lseek(outfd, 0, SEEK_CUR);
	sections_offset = sh_offset + (NUM_SECTIONS * sizeof(SECTION_HEADER));
	/* [1]  (there is no [0] section, just [0] in section header table) */
	abbrev_offset = sections_offset;
	/* [2] */
	debug_offset = abbrev_offset + abbrev_size;
	/* [3] */
	reldebug_offset = debug_offset + debug_size;
	/* [4] */
	string_offset = reldebug_offset + reldebug_size;

	/* build the section header table */

	/*  [0]: the required empty one */
	shp1 = &base_shp[0];
	shp1->sh_name = 0;
	shp1->sh_type = 0;
	shp1->sh_flags = 0;
	shp1->sh_addr = 0;
	shp1->sh_entsize = inelfhdr.e_shentsize;
	shp1->sh_offset = 0;
	shp1->sh_size = 0;

	/*  [1]: the .debug_abbrev header */
	shp1 = &base_shp[SECTION_DEBUGABBREV];
	shp1->sh_name = abbrev_idx;
	shp1->sh_offset = abbrev_offset;
	shp1->sh_size =   abbrev_size;
	shp1->sh_type = 1;  /* copy */
	shp1->sh_flags = 0;
	shp1->sh_addr = 0;  /* copy */
	shp1->sh_entsize = inelfhdr.e_shentsize;

	/*  [2]: the .debug_info section */
	shp1 = &base_shp[SECTION_DEBUGINFO];
	shp1->sh_name = debug_idx;
	shp1->sh_offset = debug_offset;
	shp1->sh_size =   debug_size;
	shp1->sh_type = 1;  /* copy */
	shp1->sh_flags = 0;
	shp1->sh_addr = 0;  /* copy */
	shp1->sh_entsize = inelfhdr.e_shentsize;

	/*  [3]: the rel.debug_info section */
	shp1 = &base_shp[SECTION_RELDEBUGINFO];
	shp1->sh_name = reldebug_idx;
	shp1->sh_offset = reldebug_offset;
	shp1->sh_size =   reldebug_size;
	shp1->sh_type = 1;  /* copy */
	shp1->sh_flags = 0;
	shp1->sh_addr = 0;  /* copy */
	shp1->sh_entsize = inelfhdr.e_shentsize;

	/*  [4]: the strings section
	   size was already set to input file's string size
	    we just copy the one from the original input file */
	shp1 = &base_shp[SECTION_STRING];
	shp1->sh_name = string_idx;
	shp1->sh_offset = string_offset;
	shp1->sh_size = out_string_size;  /* same strings as input file */
	/* elf_repl.h */
	shp1->sh_type = SHT_STRTAB;
	shp1->sh_flags = 0;
	shp1->sh_addr = 0;  /* copy */
	shp1->sh_entsize = inelfhdr.e_shentsize;

	/* write the section header table */
	sh_size = NUM_SECTIONS * sizeof(SECTION_HEADER);
	if (debug) {
		coffset = lseek(outfd, 0, SEEK_CUR);
		printf ("writing section header at %#x of %s (size %d)\n",
			coffset, out_file_name, sh_size);
	}

	if ((i = write(outfd, base_shp, sh_size)) != sh_size) {
	        printf ("cannot write section header table\n");
	        exit(1);
	}

	if (verbose) {
		shp1 = base_shp;
		printf ("output Section header table:\n");
		for (i=0; i<NUM_SECTIONS; i++, shp1++) {
			printf ("  section %d:\n", i);
			printf ("    sh_name:%d (%s)\n", shp1->sh_name,
					in_string_buffer + shp1->sh_name);
			printf ("    sh_type:%d\n", shp1->sh_type);
			printf ("    sh_flags:%#lx\n", (long)shp1->sh_flags);
			printf ("    sh_addr:%#lx\n", (long)shp1->sh_addr);
			printf ("    sh_offset:%#lx\n", (long)shp1->sh_offset);
			printf ("    sh_size:%ld\n", (long)shp1->sh_size);
			printf ("    sh_entsize:%ld\n", (long)shp1->sh_entsize);
		}
	}

	/* [1] write the abbrev section */
	if (debug) {
		coffset = lseek(outfd, 0, SEEK_CUR);
		printf ("writing abbrev section at %#x of %s (size %d)\n",
			coffset, out_file_name, abbrev_size);
	}
	for (i=0; i<num_abbrev_pieces; i++) {
		if ((j = write(outfd,*(abbrev_addrp+i), *(abbrev_lengthp+i))) !=
						*(abbrev_lengthp+i)) {
			printf ("cannot write new abbrev section; abort\n");
			exit(1);
		}
	}

	/* [2] write the debug_info section */
	if (debug) {
		coffset = lseek(outfd, 0, SEEK_CUR);
		printf ("writing debug_info section at %#x of %s (size %d)\n",
			coffset, out_file_name, debug_size);
	}
	for (i=0; i<num_debug_pieces; i++) {
		if ((j = write(outfd, *(debug_addrp+i), *(debug_lengthp+i))) !=
						*(debug_lengthp+i)) {
			printf ("cannot write new debug section; abort\n");
			exit(1);
		}
	}

	/* [3] write the rel.debug_info section */
	if (debug) {
		coffset = lseek(outfd, 0, SEEK_CUR);
		printf (
		   "writing rel.debug_info section at %#x of %s (size %d)\n",
			coffset, out_file_name, reldebug_size);
	}
	for (i=0; i<num_reldebug_pieces; i++) {
		if ((j = write(outfd, *(reldebug_addrp+i),
			*(reldebug_lengthp+i))) != *(reldebug_lengthp+i)) {
			printf ("cannot write new reldebug section; abort\n");
			exit(1);
		}
	}

	/* [4] write the string section (just what we had on input) */
	if (debug) {
		coffset = lseek(outfd, 0, SEEK_CUR);
		printf ("writing string section at %#x of %s (size %d)\n",
			coffset, out_file_name, out_string_size);
	}
	if ((i = write(outfd, in_string_buffer, out_string_size)) !=
						out_string_size) {
		printf ("cannot write new string section; abort\n");
		exit(1);
	}
}

/*
 * save the address and length of a piece of the abbrev section
 */
void
save_abbrev_piece(char *address, Dwarf_Unsigned length)
{
	if (num_abbrev_pieces >= size_abbrev_pieces) {
		num_abbrev_pieces = num_abbrev_pieces + (num_abbrev_pieces/2);
		if (pflag) {
			printf ("bump num_abbrev_pieces to %d and realloc\n",
				num_abbrev_pieces);
		}
		if ((abbrev_addrp = (char **)realloc((void *)abbrev_addrp,
			num_abbrev_pieces * sizeof(char *))) == NULL) {
			printf ("cannot realloc space for abbrev addrs\n");
			exit(1);
		}
		if ((abbrev_lengthp =
			(Dwarf_Unsigned *)realloc((void *)abbrev_lengthp,
			num_abbrev_pieces * sizeof(Dwarf_Unsigned))) == NULL) {
			printf ("cannot realloc space for abbrev lenths\n");
			exit(1);
		}
		if (pflag) {
			printf ("reallocs complete\n");
		}
	}

	*(abbrev_addrp   + num_abbrev_pieces) = address;
	*(abbrev_lengthp + num_abbrev_pieces) = length;
	abbrev_size += length;
	num_abbrev_pieces++;
	return;
}

/*
 * save the address and length of a piece of the debug section
 */
void
save_debug_piece(char *address, Dwarf_Unsigned length)
{
	if (num_debug_pieces >= size_debug_pieces) {
		size_debug_pieces = size_debug_pieces + (size_debug_pieces/2);
		if (pflag) {
			printf ("bump size_debug_pieces to %d and realloc\n",
				size_debug_pieces);
		}
		if ((debug_addrp = (char **)realloc((void *)debug_addrp,
			size_debug_pieces * sizeof(char *))) == NULL) {
			printf ("cannot realloc space for debug addrs\n");
			exit(1);
		}
		if ((debug_lengthp =
			(Dwarf_Unsigned *)realloc((void *)debug_lengthp,
			size_debug_pieces * sizeof(Dwarf_Unsigned))) == NULL) {
			printf ("cannot realloc space for debug lengths\n");
			exit(1);
		}
		if (pflag) {
			printf ("reallocs complete\n");
		}
	}

	*(debug_addrp   + num_debug_pieces) = address;
	*(debug_lengthp + num_debug_pieces) = length;
	debug_size += length;
	num_debug_pieces++;
	return;
}

/*
 * save the address and length of a piece of the debug section
 */
void
save_reldebug_piece(char *address, Dwarf_Unsigned length)
{
	if (num_reldebug_pieces >= size_reldebug_pieces) {
		size_reldebug_pieces = size_reldebug_pieces +
						(size_reldebug_pieces/2);
		if (pflag) {
			printf ("bump size_reldebug_pieces to %d and realloc\n",
				size_reldebug_pieces);

		}
		if ((reldebug_addrp = (char **)realloc((void *)reldebug_addrp,
			size_reldebug_pieces * sizeof(char *))) == NULL) {
			printf ("cannot realloc space for reldebug addrs\n");
			exit(1);
		}
		if ((reldebug_lengthp =
			(Dwarf_Unsigned *)realloc((void *)reldebug_lengthp,
			size_reldebug_pieces * sizeof(Dwarf_Unsigned)))
								== NULL) {
			printf ("cannot realloc space for reldebug lengths\n");
			exit(1);
		}
		if (pflag) {
			printf ("reallocs complete\n");
		}
	}

	*(reldebug_addrp   + num_reldebug_pieces) = address;
	*(reldebug_lengthp + num_reldebug_pieces) = length;
	reldebug_size += length;
	num_reldebug_pieces++;
	return;
}

/*
 * save the address and length of a structure prototype
 */
void
add_to_proto_list(Dwarf_Off offset, Dwarf_Off offset2, char *namep)
{
	char	*cp;

	if (num_protos >= size_protos) {
		size_protos = size_protos + (size_protos/2);
		if (pflag) {
			printf ("bump size_protos to %d and realloc\n",
				size_protos);
		}
		if ((struct_protop = (Dwarf_Off *)
			realloc((void *)struct_protop,
				size_protos * sizeof(Dwarf_Off))) == NULL) {
			printf ("cannot realloc space for struct protos\n");
			exit(1);
		}
		if ((struct_protop2 = (Dwarf_Off *)
			realloc((void *)struct_protop2,
				size_protos * sizeof(Dwarf_Off))) == NULL) {
			printf ("cannot realloc space for 2nd struct protos\n");
			exit(1);
		}
		if ((struct_protonamep = (char **)
			realloc((void *)struct_protonamep,
				size_protos * sizeof(char **))) == NULL) {
			printf ("cannot realloc space for proto names\n");
			exit(1);
		}
		if (pflag) {
			printf ("reallocs complete\n");
		}
	}

	*(struct_protop  + num_protos) = offset; /* offset within a dbg */
	*(struct_protop2 + num_protos) = offset2; /* offset within all files */
	if ((cp = (char *)malloc((int)strlen(namep)+1)) == NULL) {
		printf ( "malloc of proto name failed\n");
		exit (1);
	}
	strcpy (cp, namep);
	*(struct_protonamep + num_protos) = cp;
	num_protos++;
	return;
}

/*
 * process all DIE's in all compilation units
 */
void
walk_cus(Dwarf_Debug dbg, int fd)
{
	int		nres = DW_DLV_OK;
	Dwarf_Unsigned	cu_header_length = 0, abbrev_offset = 0;
	Dwarf_Unsigned	next_cu_offset = 0, dres;
	Dwarf_Half	version_stamp = 0, address_size = 0;
	Dwarf_Die	cu_die = 0, child;
	Dwarf_Error	error; /* pointer to a structure */
	Dwarf_P_Attribute aresult;

	/* Loop until there are no more Compilation Units  */
	while ((nres = dwarf_next_cu_header(dbg, &cu_header_length,
		&version_stamp, &abbrev_offset, &address_size,
		&next_cu_offset, &error)) == DW_DLV_OK) {
		total_cus++;
		if (total_cus == 1) {
#ifdef OPAQUE_RELOCATION			
			if (dbg->de_debug_info == NULL) {
				printf ("de_debug_info is null; abort\n");
				exit(1);
			}

			if (file_is_relocatable(fd)) {
				unsigned char	*end;
				end = dbg->de_debug_info+dbg->de_debug_info_size;
				reloc_debug_info(dbg->de_debug_info, end, fd);
			}
#endif	/* OPAQUE_RELOCATION */

		}
		/*get "first" die of the CU (this is not a sibling)*/
		get_sibling(dbg, NULL, &cu_die);
		if (debug) {
			printf ("first CU die: %p offset:%#llx\n",
				cu_die, current_offset(cu_die));
		}
		/* convert the first CU to the root of our new tree */
		if (root_cus == 0) {
			root_cus++;
			root_p_die = convert_to_new(dbg, newdbg, cu_die, 0);
			if (debug) {
				printf ("created root_p_die\n");
			}
			/* make this the root of the new graph */
			dres = dwarf_add_die_to_debug(newdbg, root_p_die,
								&error);
			if (dres != 0) {
				printf (
				 "dwarf_add_die_to_debug failed (%lld)\n",
                                  (long long)dres);
				printf ("DW_DLV_NOCOUNT is %lld\n",
                                  (long long)DW_DLV_NOCOUNT);
				exit(1);
			}
			if (debug) {
				printf ("made new graph root\n");
			}

			/* add a producer attribute to the new root */
			aresult = dwarf_add_AT_producer(root_p_die,
				program_name, &error);
			if (aresult == (Dwarf_P_Attribute)DW_DLV_BADADDR) {
				printf ("dwarf_add_AT_producer failed\n");
				exit(1);
			}
		}

		if (pflag && current_file_number==1 && (total_cus % 10 == 0)) {
			/* this progress item is only confusion for
			   a -c or -C file */
			printf ("CU: %d\n", total_cus);
		}

		/* get the die of the CU */
		if (get_sibling(dbg, NULL, &cu_die)) {
			/* start with the first child of cu_die; create
			   everything else that we select at the top level
			   of the CU as a sibling of this first child */
			if (get_child(cu_die, &child)) {
				if (debug) {
					printf ("walking children of %s\n",
						name_of_die(cu_die));
				}

				tree_level = 0;
				/* so that all the base types below the CU
				   begin at level 1 */
				if (debug) {
					printf (
					   "walk CUs die: %p offset:%#llx\n",
					   cu_die, current_offset(cu_die));
				}
				walk_die_and_children(dbg, child, root_p_die);

			}
		}
		cu_offset = next_cu_offset;
		if (debug) {
			printf ("bump to next cu_offset\n");
		}
	}
	if (debug) {
		printf ("end of all CUs\n");
	}
}

/*
 * recursively follow the die tree
 */
void
walk_die_and_children(Dwarf_Debug dbg, Dwarf_Die die, Dwarf_P_Die parent_p_die)
{
	int		do_this_one, members, size, len;
	int		fndtype, alias_index, isptr, declaration;
	unsigned long long int	hash;
	char		*namep, *stringp;
	struct typenode *typep;
	Dwarf_Error	err;
	Dwarf_Die	child, sibling;
	Dwarf_P_Die	new_p_die;     /* pointer to a producer structure */
	Dwarf_P_Die	producer_p_die;
	Dwarf_Half	tag;
	Dwarf_Off	array_offset, typeoffset;  /* of type pointed to */
	Dwarf_Attribute attr;

	if (debug) {
		printf ("walk_die_and_children entered, tree_level:%d CU:%d\n",
			tree_level, total_cus);
	}
	/* if we don't create a p_die at this level, inherit the parent */
	new_p_die = parent_p_die;
	tree_level++; /* becomes 1 on first entry of each CU */

	while (die) {

		total_dies++;
		if (pflag && current_file_number == 1) {
			/* this progress item is only confusion for
			   a -c or -C file */
			if (total_dies % 100000 == 0) {
				printf (
				"dies: %d aliases:%d saved structures:%d\n",
				    total_dies, num_aliases, total_structs);
			}
		}
		do_this_one = is_type_we_want(dbg, die, &declaration);
		if (debug) {
			printf ("%s: is_type_we_want:%d\n",
				name_of_die(die), do_this_one);
		}
		if  (do_this_one) {
			if (debug) {
				printf ("begin die %s, offset %lld <%#llx>\n",
					name_of_die(die), 
                                        (long long)current_offset(die),
					current_offset(die));
			}
			get_type_values(dbg, die, &namep, &hash, &members,
				&size, &tag, &typeoffset, &isptr);
			/* generally we only want structures at the highest
			   level, but within subprograms you find some down a
			   couple levels */
			if ((tree_level == 1) ||
		    	    (tree_level < 5 && is_a_group(die))) {
				/* duplicate elimination only for the highest
				   level */
				/* returns LNOTFOUND if this is a new (unique)
									type */
				/* returns LFOUND if this is a normal found */
				/* returns LUNPTR if this is an unnamed pointer
					to some other type */
				/* returns LUNARRAY if this is an unnamed array
					link to some other type */
				fndtype = lookuptype(dbg, die, tree_base,
					members, hash, size, tag, namep, &typep,
					&alias_index);
			} else {
				fndtype = LNOTFOUND;
			}
			if (fndtype == LNOTFOUND) {
				/* do this one, it is unique so far */
				/* (or it is a member of a wanted type) */
				if (is_a_struct(die)) {
					total_structs++;
					if (!strcmp(namep, myno_die_name)) {
						total_unnamed_structs++;
					}
				}
				/* leave do_this_one set */
			} else if (fndtype == LFOUND) {
				/* this one is a duplicate */
				do_this_one = 0;
				if (debug) {
					printf (
				 "duplicate: %s members:%d size:%d hash:%lld\n",
						namep, members, size, 
                                                (long long)hash);
				}
				/* add this to the alias - something may
				   refer to it and we will have to redirect
				   that reference to the thing it is a
				   duplicate of */
				add_alias_tree(die, typep, typeoffset);
				total_duplicates++;
				if (is_a_struct(die)) {
					duplicate_structs++;
				}
			} else if (fndtype == LUNPTR) {
				/* this is an unnamed pointer that
				   will be added to those types that reference
				   them (at the end of the program) */
				do_this_one = 0;
			} else if (fndtype == LUNARRAY) {
				/* this is an unnamed array link that
				   will be added to those types that reference
				   them (at the end of the program) */
				do_this_one = 0;
			}
		}
		if (debug) {
			printf ("%s: after lookups, do this one:%d\n",
				name_of_die(die), do_this_one);
		}
		if  (do_this_one) {
			/* create a producer die for this and its selected
			   siblings
			   (unnamed pointers are not created until the very
			    end, when we know the dies that need them */
			new_p_die = convert_to_new(dbg, newdbg, die, isptr);
			/* we need a new location for the name; the old
			   dbg may be thrown away before we're done, and
			   the name's address is recorded in the typenode */
			len = (int)strlen(namep);
			stringp = (char *)malloc(len + 1);
			strcpy(stringp, namep);
			namep = stringp;

			if (isptr == PTRLINK) {
				/* this die links to a pointer;  add it to the
				   list of dies for which a pointer needs to
				   be created at the end */
				/* the pointer dies themselves are not
				   created yet (see LUNPTR) */
				add_to_needs_ptr_list(typeoffset, new_p_die);
			} else if (isptr == ARRAYLINK) {
				/* this is an array link;  add it to the list
				   of dies for which an array link needs to
				   be created at the end */
				/* the array link dies themselves are not
				   created yet (see LUNARRAY) */
				if (!get_attr(die, DW_AT_type, &attr)) {
					printf ("array link: no type\n");
					exit(1);
				}
				get_refoffset(attr, &array_offset);
				add_to_needs_arr_list(dbg, array_offset,
 							typeoffset, new_p_die);
			}

			/* this is a child; link it to its parent */

			if (debug) {
				printf ("link pdie %p to parent %p\n",
						new_p_die, parent_p_die);
			}

			/*
			struct timeval tv, tv2;
			gettimeofday(&tv, NULL);
			*/

			producer_p_die = dwarf_die_link(new_p_die,
					parent_p_die,NULL,NULL,NULL, &err);

			/*
			gettimeofday(&tv2, NULL);

			printf("%u.%04u\n", tv2.tv_sec - tv.tv_sec,
					tv2.tv_usec - tv.tv_usec);
			*/

			if (producer_p_die == (Dwarf_P_Die)DW_DLV_BADADDR) {
				__asm__("int $3");
				printf ("first dwarf_die_link failed\n");
				exit(1);
			}
			/* Generally we just have structures and base types
			   at level 1; but exclude all the subroutines from
			   the duplicate-elimination tree.
			   Also take all the structures at higher levels. */
			if ((tree_level == 1 && tag != DW_TAG_subprogram) ||
			    (tree_level < 5  && is_a_group(die))) {
				addtosavedtypes(namep, members, hash, new_p_die,
					die, size, tag, declaration);
				if (debug) {
					printf ("add %s to saved types\n",
						namep);
				}
			} else {
				if (debug) {
					printf (
				      "%s not added to saved types, level %d\n",
						namep, tree_level);
				}
			}

		}

		/* now do its children */
		if (do_this_one) {
			/* do children before siblings: we are doing
			   depth-first walk */
			if (get_child(die, &child)) {
				if (debug) {
					printf ("walking children of %s\n",
						name_of_die(die));
				}
				walk_die_and_children(dbg, child, new_p_die);
			}
		}

		/* check whether there are any siblings to our current,
		   top level die */
		if (get_sibling(dbg, die, &sibling)) {
			/* we do have a sibling, now in die "sibling" */
			die = sibling;
			if (debug) {
				printf (
				  "found sibling %s\n", name_of_die(sibling));
			}
		} else {
			die = NULL;
			if (debug) {
				printf ("no more siblings\n");
			}
		}
	}
	tree_level--;
	return;
}

void
add_to_needs_ptr_list(Dwarf_Off typeoffset, Dwarf_P_Die pdie)
{
	if (num_needptrs >= size_needptrs) {
		size_needptrs = size_needptrs + (size_needptrs/2);
		if (pflag) {
			printf ("bump size_needptrs to %d and realloc\n",
				size_needptrs);
		}
		if ((needptr_refp = (Dwarf_Off *)realloc((void *)needptr_refp,
			size_needptrs * sizeof(Dwarf_Off))) == NULL) {
			printf ("cannot realloc space for needptr offsets\n");
			exit(1);
		}
		if ((needptr_typep =
			(struct typenode **)realloc((void *)needptr_typep,
			size_needptrs * sizeof(struct typenode *))) == NULL) {
			printf ("cannot realloc space for needptr indices\n");
			exit(1);
		}
		if ((needptr_pdiep =
			(Dwarf_P_Die *)realloc((void *)needptr_pdiep,
			size_needptrs * sizeof(Dwarf_P_Die))) == NULL) {
			printf ("cannot realloc space for needptr dies\n");
			exit(1);
		}
		if (pflag) {
			printf ("reallocs complete\n");
		}
	}

	*(needptr_refp  + num_needptrs) = typeoffset*FMAX +
						current_file_number;
	*(needptr_pdiep + num_needptrs) = pdie;
	*(needptr_typep + num_needptrs) = 0;
	if (debug) {
		printf (
		 "adding offset %lld die %p to need_pointer list, slot %d\n",
			(long long)typeoffset, pdie, num_needptrs);
	}
	num_needptrs++;
	return;
}

void
add_to_needs_arr_list(Dwarf_Debug dbg, Dwarf_Off this_offset,
			Dwarf_Off typeoffset, Dwarf_P_Die pdie)
{
	if (num_needarrs >= size_needarrs) {
		size_needarrs = size_needarrs + (size_needarrs/2);
		if (pflag) {
			printf ("bump size_needarrs to %d and realloc\n",
				size_needarrs);
		}
		if ((needarr_refp = (Dwarf_Off *)realloc((void *)needarr_refp,
			size_needarrs * sizeof(Dwarf_Off))) == NULL) {
			printf ("cannot realloc space for needarr offsets\n");
			exit(1);
		}
		if ((needarr_typep =
			(struct typenode **)realloc((void *)needarr_typep,
			size_needarrs * sizeof(struct typenode *))) == NULL) {
			printf ("cannot realloc space for needarr indices\n");
			exit(1);
		}
		if ((needarr_pdiep =
			(Dwarf_P_Die *)realloc((void *)needarr_pdiep,
			size_needarrs * sizeof(Dwarf_P_Die))) == NULL) {
			printf ("cannot realloc space for needarr dies\n");
			exit(1);
		}
		if ((needarr_origp = (Dwarf_Off *)realloc((void *)needarr_origp,
			size_needarrs * sizeof(Dwarf_Off))) == NULL) {
			printf ("cannot realloc space for needarr origs\n");
			exit(1);
		}
		if ((needarr_dbgp =
			(Dwarf_Debug *)realloc((void *)needarr_dbgp,
			size_needarrs * sizeof(Dwarf_Debug))) == NULL) {
			printf ("cannot realloc space for needarr dbgs\n");
			exit(1);
		}
		if (pflag) {
			printf ("reallocs complete\n");
		}
	}

	*(needarr_refp  + num_needarrs) = typeoffset*FMAX + current_file_number;
	*(needarr_pdiep + num_needarrs) = pdie;
	*(needarr_typep + num_needarrs) = 0;
	*(needarr_dbgp  + num_needarrs) = dbg;
	*(needarr_origp + num_needarrs) = this_offset;
	if (debug) {
		printf (
		  "adding offset %lld die %p to need_array list, slot %d\n",
			(long long)typeoffset, pdie, num_needarrs);
	}
	needs_old++; /* remember to leave this file open */
	num_needarrs++;
	return;
}

/*
 * return a die's name, or the NO_NAME string
 */
char *
name_of_die(Dwarf_Die die)
{
	int		result;
	char		*die_name;
	Dwarf_Error	err;

	result = dwarf_diename(die, &die_name, &err);
	if (result == DW_DLV_ERROR)  {
		printf("die_name: dwarf_diename failed\n");
		exit(1);
	} else if (result == DW_DLV_NO_ENTRY) {
		return myno_die_name;
	} else {
		return die_name;
	}
}

/*
 * print info about die (Debug Information Entry)
 */
void
print_a_die(Dwarf_Debug dbg, Dwarf_Die die)
{
	int		result, members, size, isptr;
	unsigned long long int	hash;
	char		*die_name;
	Dwarf_Error	err;
	Dwarf_Off	globaloffset;
	Dwarf_Half	dietag, tag;
	Dwarf_Unsigned	bytesize, bitsize;

	get_tag(die, &dietag);

	/* -o means display the members of something name <oname> */
	if (oflag) {
		if (dietag == DW_TAG_structure_type   ||
	    	    dietag == DW_TAG_enumeration_type ||
		    dietag == DW_TAG_typedef	  ||
	    	    dietag == DW_TAG_union_type)      {
			die_name = name_of_die(die);
			if (!strcmp(die_name, myno_die_name)) {
				return; /* don't show unnamed items */
			}
			if (!strcmp(die_name, oname)) {
				if (dietag == DW_TAG_structure_type) {
					show_structure(dbg, die, "", 0, 0);
				} else if (dietag == DW_TAG_enumeration_type) {
					show_enum(dbg, die, "", 0, 0);
				} else if (dietag == DW_TAG_typedef) {
					show_typedef(dbg, die, "", 0, 0);
				} else if (dietag == DW_TAG_union_type) {
					show_union(dbg, die, "", 0, 0);
				}
			}
		}
		return;
	}

	/* -i means display info about something name <iname> */
	if (iflag) {
		if (dietag == DW_TAG_structure_type   ||
	    	    dietag == DW_TAG_enumeration_type ||
		    dietag == DW_TAG_typedef	  ||
	    	    dietag == DW_TAG_union_type)      {
			die_name = name_of_die(die);
			if (!strcmp(die_name, myno_die_name)) {
				return; /* don't show unnamed items */
			}
			if (!strcmp(die_name, iname)) {
				get_type_values(dbg, die, &die_name, &hash,
				  &members, &size, &tag, &globaloffset, &isptr);
				printf ("%s size:%d members:%d hash:%lld\n",
 					die_name, size, members, 
                                        (long long)hash);
				return;
			}
		}
		return;
	}

	/* -g means just display structures, unions, typedefs and enumerations*/
	if (gflag) {
		if (dietag == DW_TAG_structure_type   ||
		    dietag == DW_TAG_enumeration_type ||
		    dietag == DW_TAG_typedef	  ||
		    dietag == DW_TAG_union_type)      {
			printf ("%s %s\n", die_type(die), name_of_die(die));
		}
		return;
	}
	/* -a means display all types */
	if (aflag) {
		printf ("%s\n", name_of_die(die));
		return;
	}
	/* -H means display just structures, with a hash total and size */
	if (Hflag) {
		if (dietag != DW_TAG_structure_type) {
			return;
		}
		die_name = name_of_die(die);
		if (!strcmp(die_name, myno_die_name)) {
			return; /* don't show unnamed items */
		}
		get_type_values(dbg, die, &die_name, &hash, &members,
				&size, &tag, &globaloffset, &isptr);
		printf ("%s %lld %d\n", die_name, (long long)hash, size);
		return;
	}
	/* -S means display just structures, with their size */
	if (Sflag) {
		if (dietag != DW_TAG_structure_type) {
			return;
		}
		die_name = name_of_die(die);
		if (!strcmp(die_name, myno_die_name)) {
			return; /* don't show unnamed items */
		}
		result = dwarf_bytesize(die, &bytesize, &err);
		if (result == DW_DLV_ERROR) {
			printf("print_a_die: problem reading byte size\n");
			exit(1);
		} else if (result == DW_DLV_NO_ENTRY) {
			bytesize = 0;
		}
		if (bytesize) {
			printf ("%s %lld\n", die_name, (long long)bytesize);
		} else {
			printf ("%s unknown-size\n", die_name);
		}
		return;
	}

	strcpy(indentation_string, indentation_spaces);
	indentation_string[(tree_level)*2] = '\0';
	printf ("%s", indentation_string);

	printf ("type: %s", die_type(die));

	get_tag(die, &tag);
	globaloffset = current_offset(die);

	/* for cross-referencing, we'll use global offsets exclusively
	ores = dwarf_die_CU_offset(die, &cuoffset, &err);
	if (ores != DW_DLV_OK) {
		printf ("dwarf_die_CU_offset failed\n");
		exit(1);
	}
	*/

	printf ("own_offset:%llu ", (unsigned long long)globaloffset);

	die_name = name_of_die(die);
	printf ("name:%s ", die_name);

	result = dwarf_bytesize(die, &bytesize, &err);
	if (result == DW_DLV_ERROR) {
		printf("print_a_die: problem reading byte size\n");
		exit(1);
	} else if (result == DW_DLV_NO_ENTRY) {
		bytesize = 0;
	}
	if (bytesize) {
		printf ("bytesize:%llu ", (unsigned long long)bytesize);
	}

	result = dwarf_bitsize(die, &bitsize, &err);
	if (result == DW_DLV_NO_ENTRY) {
		bitsize = 0;
	} else if (result == DW_DLV_ERROR) {
		printf ("dwarf_bitsize failed\n");
		exit(1);
	}
	if (bitsize) {
		printf ("bitsize:%llu ", (unsigned long long)bitsize);
	}

	if (!Tflag) {
		get_type_values(dbg, die, &die_name, &hash, &members,
				&size, &tag, &globaloffset, &isptr);
		printf ("hash:%lld members:%d ", (long long)hash, members);
		printf ("\n");
	}

	list_attributes(dbg, die);

	return;
}

char *
die_type (Dwarf_Die die)
{
	Dwarf_Half	dietag;

	get_tag(die, &dietag);
	switch(dietag) {
		case DW_TAG_base_type:{
			return ("base type");
		}
		case DW_TAG_enumeration_type: {
			return ("enumeration ");
		}
		case DW_TAG_structure_type: {
			return ("structure ");
		}
		case DW_TAG_union_type: {
			return ("union ");
		}
		case DW_TAG_member: {
			return ("member ");
		}
		case DW_TAG_typedef: {
			return ("typedef ");
		}
		case DW_TAG_array_type: {
			return ("array ");
		}
		case DW_TAG_pointer_type: {
			return ("pointer ");
		}
		case DW_TAG_volatile_type:  {
			return ("volatile ");
		}
		case DW_TAG_const_type: {
			return ("constant ");
		}
		case DW_TAG_subroutine_type: {
			return ("subroutine ");
		}
		case DW_TAG_enumerator: {
			return ("enumerator ");
		}
		case DW_TAG_formal_parameter: {
			return ("paramater ");
		}
		case DW_TAG_subrange_type: {
			return ("subrange ");
		}
		case DW_TAG_compile_unit: {
			return ("compile unit ");
		}
		case DW_TAG_variable: {
			return ("variable ");
		}
		case DW_TAG_subprogram: {
			return ("subprogram ");
		}
		case DW_TAG_unspecified_parameters: {
			return ("unspecified parameters ");
		}
		case DW_TAG_lexical_block: {
			return ("lexical block ");
		}
		default: {
			return ("");
		}
	}
}

/*
 * -o means display the members of a structure named <oname>
 */
void
show_structure(Dwarf_Debug dbg, Dwarf_Die die, char *name, int level,
							int parent_offset)
{
	char		*die_name;
	Dwarf_Die	child_die, this_die, sibling_die;

	die_name = name_of_die(die);
	strcpy(indentation_string, indentation_spaces);
	indentation_string[level*4] = '\0';
	printf ("%s", indentation_string);
	printf ("%s%s %s\n", die_type(die), die_name, name);
	level++;

	/* printf ("\n"); this fixes some printf bug in x86_64 */
	/* walk the children: */
	if (get_child(die, &child_die)) {
		show_die_offset(dbg, child_die, level, parent_offset);
		this_die = child_die;
		while (get_sibling(dbg, this_die, &sibling_die)) {
			show_die_offset(dbg, sibling_die, level, parent_offset);
			this_die = sibling_die;
		}
	}
	return;
}

/*
 * -o means display the members of a union named <oname>
 */
void
show_union(Dwarf_Debug dbg, Dwarf_Die die, char *name, int level,
							int parent_offset)
{
	char		*die_name;
	Dwarf_Die	child_die, this_die, sibling_die;
	Dwarf_Half	tag;

	die_name = name_of_die(die);
	strcpy(indentation_string, indentation_spaces);
	indentation_string[level*4] = '\0';
	printf ("%s", indentation_string);
	printf ("%s%s %s\n", die_type(die), die_name, name);
	level++;

	/* walk the children (overlapping members of this union) */
	if (get_child(die, &child_die)) {
		die_name = name_of_die(child_die);
		if (!strcmp(die_name, myno_die_name)) {
			die_name = "";
		}
		get_tag(child_die, &tag);
		if (tag == DW_TAG_structure_type) {
			show_structure(dbg, child_die, die_name, level,
								parent_offset);
		} else if (tag == DW_TAG_enumeration_type) {
			show_enum(dbg, child_die, die_name, level,
								parent_offset);
		} else if (tag == DW_TAG_typedef) {
			show_typedef(dbg, child_die, die_name, level,
								parent_offset);
		} else if (tag == DW_TAG_union_type) {
			show_union(dbg, child_die, die_name, level,
								parent_offset);
		} else {
			show_die_offset(dbg, child_die, level, parent_offset);
		}
		this_die = child_die;
		while (get_sibling(dbg, this_die, &sibling_die)) {
			show_die_offset(dbg, sibling_die, level, parent_offset);
			this_die = sibling_die;
		}
	}
	return;
}

/*
 * -o means display the members of a typedef named <oname>
 */
void
show_typedef(Dwarf_Debug dbg, Dwarf_Die die, char *name, int level,
							int parent_offset)
{
	int		has_children=0;
	char		*die_name, *typedie_name;
	Dwarf_Die	child_die, this_die, sibling_die, type_die;

	die_name = name_of_die(die);
	if (has_attr_group(dbg, die, &type_die)) {
		has_children++;
		typedie_name = name_of_die(type_die);
		if (!strcmp(typedie_name, myno_die_name)) {
			typedie_name = "";
		}
	} else {
		typedie_name = "";
	}
	strcpy(indentation_string, indentation_spaces);
	indentation_string[level*4] = '\0';
	printf ("%s", indentation_string);
	printf ("%s%s %s %s%s %#x %d\n",
			die_type(die), die_name, name, die_type(type_die),
			typedie_name, parent_offset, parent_offset);
	if (has_children) {
		die = type_die;
		level++;

		/* walk the children: */
		if (get_child(die, &child_die)) {
			show_die_offset(dbg, child_die, level, parent_offset);
			this_die = child_die;
			while (get_sibling(dbg, this_die, &sibling_die)) {
				show_die_offset(dbg, sibling_die, level,
								parent_offset);
				this_die = sibling_die;
			}
		}
	}
	return;
}

/*
 * display the members of an enumeration named <oname>
 */
void
show_enum(Dwarf_Debug dbg, Dwarf_Die die, char *name, int level,
							int parent_offset)
{
	char		*die_name;
	Dwarf_Die	child_die, this_die, sibling_die;

	die_name = name_of_die(die);
	/* enumerations have no DW_AT_data_member_location x38 */

	strcpy(indentation_string, indentation_spaces);
	indentation_string[level*4] = '\0';
	printf ("%s", indentation_string);
	printf ("%s%s %s %#x %d\n", die_type(die),
			die_name, name, parent_offset, parent_offset);
	level++;

	/* walk the children: */
	if (get_child(die, &child_die)) {
		show_die_offset(dbg, child_die, level, parent_offset);
		this_die = child_die;
		while (get_sibling(dbg, this_die, &sibling_die)) {
			show_die_offset(dbg, sibling_die, level, parent_offset);
			this_die = sibling_die;
		}
	}
	return;
}

/*
 * display the characteristics of an array
 */
void
show_array(Dwarf_Debug dbg, Dwarf_Die die, char *name, int level,
							int parent_offset)
{
	int		bound;
	Dwarf_Die	child_die, this_die, sibling_die;


	/* arrays have subrange children with their dimensions in them*/

	strcpy(indentation_string, indentation_spaces);
	indentation_string[level*4] = '\0';
	printf ("%s", indentation_string);
	printf ("%s %s", name, die_type(die));
	level++;

	/* walk the children: */
	if (get_child(die, &child_die)) {
		this_die = child_die;
		bound = get_bound(this_die);
		if (bound) {
			printf ("[%d] ", bound+1);
		} else {
			printf ("void ");
		}
		while (get_sibling(dbg, this_die, &sibling_die)) {
			this_die = sibling_die;
			bound = get_bound(this_die);
			if (bound) {
				printf ("[%d] ", bound+1);
			} else {
				printf ("void ");
			}
		}
	} else {
		printf ("unknown array bounds");
	}
	printf ("of %s ", get_reftype(die, dbg)); /* type or name */
	printf ("%#x %d", parent_offset, parent_offset);
	printf ("\n");
	return;
}

/*
 * find a die's DW_AT_upper_bound attribute and return it
 * else return 0
 */
int
get_bound (Dwarf_Die die)
{
	char		*die_name;
	int		result;
	Dwarf_Half	dietag;
	Dwarf_Unsigned	uvalue;
	Dwarf_Error	error;
	Dwarf_Attribute	attr;
	Dwarf_Half	formtype;

	get_tag(die, &dietag);
	if (dietag == DW_TAG_subrange_type) {
		die_name = name_of_die(die);
		if (get_attr(die, DW_AT_upper_bound, &attr)) {
			result = dwarf_whatform(attr, &formtype, &error);
			if(result == DW_DLV_ERROR) {
				printf("dwarf_whatform error\n");
				exit(1);
			}
			if ((formtype == DW_FORM_sdata) ||
			    (formtype == DW_FORM_udata) ||
			    (formtype == DW_FORM_data1) ||
			    (formtype == DW_FORM_data2) ||
			    (formtype == DW_FORM_data4)) {
				result = dwarf_formudata(attr, &uvalue, &error);
				if (result == DW_DLV_ERROR) {
					printf(
					"get_bound: dwarf_formudata error\n");
					exit(1);
				}
			} else {
				/* probably DW_FORM_ref4, a run-time bound */
				uvalue = 0;
			}
		} else {
			uvalue = 0;
		}
	} else {
		printf ("unexpected child on array\n");
		uvalue = 0;
	}
	result = uvalue;
	return result;
}

/*
 * find a die's type attribute, and from it the name of the type it points to
 * (return its type if it has no name)
 */
char *
get_reftype(Dwarf_Die die, Dwarf_Debug dbg)
{
	Dwarf_Attribute	attr;
	Dwarf_Off	type_offset;
	Dwarf_Die	type_die;

	if (get_attr(die, DW_AT_type, &attr)) {
		get_refoffset(attr, &type_offset);
		get_die(dbg, type_offset, &type_die);
		if (get_attr(type_die, DW_AT_name, &attr)) {
			return (name_of_die(type_die));
		} else {
			return (die_type(type_die));
		}
	} else {
		return "";
	}
}

/*
 * display name and offset
 */
void
show_die_offset(Dwarf_Debug dbg, Dwarf_Die die, int level, int parent_offset)
{
	char		*die_name;
	Dwarf_Die	type_die;
	Dwarf_Half 	offset=0, type_tag;
	Dwarf_Attribute	attr;

	die_name = name_of_die(die);
	if (get_attr(die, DW_AT_data_member_location, &attr)) {
		offset = get_offset(dbg, attr, DW_AT_data_member_location);
	}
	offset += parent_offset;
	if (has_attr_group(dbg, die, &type_die)) {
		get_tag(type_die, &type_tag);
		if (type_tag == DW_TAG_structure_type) {
			show_structure(dbg, type_die, die_name, level, offset);
		} else if (type_tag == DW_TAG_enumeration_type) {
			show_enum(dbg, type_die, die_name, level, offset);
		} else if (type_tag == DW_TAG_typedef) {
			show_typedef(dbg, type_die, die_name, level, offset);
		} else if (type_tag == DW_TAG_union_type) {
			show_union(dbg, type_die, die_name, level, offset);
		} else if (type_tag == DW_TAG_array_type) {
			show_array(dbg, type_die, die_name, level, offset);
		} else {
			printf("unexpected type die %p\n", type_die);
			exit(1);
		}
	} else {
		strcpy(indentation_string, indentation_spaces);
		indentation_string[level*4] = '\0';
		printf ("%s", indentation_string);
		printf ("%s %#x %d\n", die_name, offset, offset);
	}
	return;
}

/*
 * test a die for circularity (does a->b->c->a ...)    -r option
 */
void
test_a_die(Dwarf_Debug dbg, Dwarf_Die die)
{
	int		count;
	Dwarf_Die	next_die;
	Dwarf_Attribute	attr;
	Dwarf_Off	globaloffset, offset;

	tested_dies++;
	if (verbose && (tested_dies % 10000 == 0)) {
		offset = current_offset(die);
		printf ("tested %d dies, offset: %lld\n", tested_dies, 
                    (long long)offset);
	}
	next_die = die;
	count = 0;
	while (get_attr(next_die, DW_AT_type, &attr)) {
		count++;
		if (count > 10) {
			printf ("circularity found\n");
			return;
		}
		get_refoffset(attr, &globaloffset);
		get_die(dbg, globaloffset, &next_die);
	}
	if (count > deepest_nesting) {
		deepest_nesting = count;
	}

	return;
}

/*
 * set program_name to point to the name of this program
 *
 * get the options and arguments
 *
 * test the options and arguments for sanity; exit on errors
 */
void
get_options(int argc, char *argv[])
{
	int	commandlineerror=0, i, byte, numargs;
	char	*cp;

	program_name = argv[0];
	if (strchr(program_name,'/')) { /* isolate the last component */
		i = strlen (program_name);
		for (program_name=program_name+i; *(program_name-1) !=
			'/'; program_name--);
	}
	while ((byte = getopt(argc, argv, "vdtTsHSgmo:apPrc:C:i:z?")) != EOF) {
		switch	(byte) {
		case 'v':
			verbose++;
	        	continue;
		case 'd':
			debug++;
	        	continue;
		case 't':
			tflag++;
			scan_one_file++;
	        	continue;
		case 'o':
			oflag++;
			oname=optarg;
			scan_one_file++;
	        	continue;
		case 'r':
			rflag++;
			scan_one_file++;
	        	continue;
		case 'T':
			Tflag++;
			scan_one_file++;
	        	continue;
		case 's':
			sflag++;
	        	continue;
		case 'g':
			gflag++;
			scan_one_file++;
	        	continue;
		case 'a':
			aflag++;
			scan_one_file++;
	        	continue;
		case 'H':
			Hflag++;
			scan_one_file++;
	        	continue;
		case 'S':
			Sflag++;
			scan_one_file++;
	        	continue;
		case 'p':
			pflag++;
	        	continue;
		case 'P':
			Pflag++;
	        	continue;
		case 'm':
			mflag++;
			scan_one_file++;
	        	continue;
		case 'c':
			cflag++;
			if ((cfilep = (char **)malloc(NUMCFILES *
						sizeof(char *))) == NULL) {
				printf ( "malloc of cfilep failed\n");
				exit (1);
			}
			if ((cp=strtok(optarg,","))) {
				cfilep[num_cfiles]=cp;
				num_cfiles++;
				while((cp=strtok(NULL,","))) {
					cfilep[num_cfiles]=cp;
					num_cfiles++;
					if (num_cfiles > NUMCFILES) {
						printf ("too many -c files\n");
						commandlineerror++;
						break;
					}
				}
			}
	        	continue;
		case 'C':
			cflag++;
			Cfilenames(optarg);
	        	continue;
		case 'i':
			iflag++;
			iname = optarg;
			scan_one_file++;
	        	continue;
		case 'z':
			usage();
			exit(0);
		case '?':
			commandlineerror++;
			continue;
		default:
			commandlineerror++;
	 	}
	}
	numargs = argc-optind;
	i = optind;
	/* numargs should be 1 for -g, -t, -T, ... or optionally when using
	   -c and the user wants to update the input file in place */
	if (scan_one_file) {
		if (numargs != 1) {
			fprintf(stderr,
		   "one file name required for -t, -T, -a, -o, -r, -g or -i\n");
			commandlineerror++;
		}
		in_file_name = *(argv+i);
	} else {
		/* we're making new dwarf information, the output can be
		   back to the input file (with -c), or to a new file */
		if (cflag && numargs == 1) {
			in_file_name = *(argv+i);
			in_place++;
		} else if (numargs == 2) {
			in_file_name = *(argv+i);
			out_file_name = *(argv+i+1);
		} else {
			commandlineerror++;
			if (cflag) {
				fprintf(stderr,
				"one or two file names required for -c\n");
			} else {
				fprintf(stderr, "two file names required\n");
			}

		}
	}
	if (commandlineerror) {
		usage();
		exit(1);
	}
}

void
Cfilenames(char *Cfilename)
{
#define BUFLEN 200
	int	buflen = BUFLEN;
	int	numlines=0, max=0, i;
	char	buffer[BUFLEN], *p, *p2, *cp, *cfiles;
	FILE	*infile;

	infile = fopen (Cfilename, "r");
	if (!infile) {
		printf ( "open error on %s\n", Cfilename);
		exit (1);
	}

	p = fgets (buffer, buflen, infile);
	while (p)
	{
		numlines++;
		if ((i = (int)strlen(p)) > max) {
			max = i;
		}
		p = fgets (buffer, buflen, infile);
	}
	fclose (infile);
	if (numlines > FMAX) {
		/* shouldn't happen; not logically allowed */
		printf ("There are %d filenames in the -C file; exceeds FMAX",
			numlines);
		exit (1);
	}
	if ((cfilep = (char **)malloc(numlines * sizeof(char *))) == NULL) {
		printf ( "malloc of cfilep failed\n");
		exit (1);
	}
	if ((cfiles = (char *)malloc(numlines * max)) == NULL) {
		printf ( "malloc of cfiles space failed\n");
		exit (1);
	}

	infile = fopen (Cfilename, "r");
	p = fgets (buffer, buflen, infile);
	numlines=0;
	cp = cfiles;
	while (p)
	{
		p2=strchr(p,'\n');
		if (p2) {
			*p2 = '\0';
		}
		strcpy (cp, p);
		cfilep[numlines] = cp;
		numlines++;
		cp += max;
		p = fgets (buffer, buflen, infile);
	}
	num_cfiles = numlines;
	fclose (infile);
	return;
}

void
usage()
{
	fprintf (stderr, "to extract type information from a file ");
	fprintf (stderr, "with several CU (compile units):\n");
	fprintf (stderr, "%s [-v][-p] in_file_name out_file_name\n",
		program_name);
	fprintf (stderr, "    -p   show progress (CUs and dies)\n");
	fprintf (stderr, "    -P   show elapsed times\n");
	fprintf (stderr, "    -v   verbose output\n");
	fprintf (stderr, "    -s   silent (no summary)\n\n");
	fprintf (stderr, "or to add more types to an existing output:\n");
	fprintf (stderr, "   %s in_file_name -c file1,file2,file3\n",
		program_name);
	fprintf (stderr,
		"or %s in_file_name -c file1,file2,file3 out_file_name\n",
		program_name);
	fprintf (stderr,
      "    -c file1,file2,... concatenate the types in files ");
	fprintf (stderr, "file1,file2,...\n    (up to %d) ", NUMCFILES);
	fprintf (stderr, "to those in in_file_name;\n");
	fprintf (stderr,
		"or %s in_file_name -C file1 out_file_name\n",
		program_name);
	fprintf (stderr, "    where file1 contains a list of file names ");
	fprintf (stderr,      "(no limit)\n");
	fprintf (stderr, "    (e.g. find . -name \"*.ko\" > file1)\n");
	fprintf (stderr, "    (you may need to use ulimit -n 2000 or ");
	fprintf (stderr, "raise nofile in\n     /etc/security/limits.conf ");
	fprintf (stderr, "to open more than 1024 files)\n");
	fprintf (stderr,
		"    can update in_file_name or write file out_file_name\n");
	fprintf (stderr, "\nor to show the types in a file:\n");
	fprintf (stderr, "%s -t|-o|-T|-g|-a|-H|-i in_file_name\n",
			program_name);
	fprintf (stderr,
	"    -t   detailed trace all the components of the file\n");
	fprintf (stderr,
	"    -o struct  show the offsets of members of structure struct\n");
	fprintf (stderr,
	"    -T   less detailed trace of all the components of the file\n");
	fprintf (stderr,
	"    -g   list all the struct/union/enum groups in the file\n");
	fprintf (stderr,
	"    -a   list all types in the file (including members)\n");
	fprintf (stderr,
	"    -H   list all structures, with a hash total and size for each\n");
	fprintf (stderr,
	"    -i name    list size, number of members and hash total for a\n");
	fprintf (stderr,
	"               specified structure, typedef, union or enum\n");
	fprintf (stderr,
	"    -S   list all structures, with their size\n");
	fprintf (stderr,
	"    -r   test the types for looping\n");
	fprintf (stderr, "\n");
	fprintf (stderr, "    -d   debug output\n");
	fprintf (stderr, "    -z   show this usage\n");
	fprintf (stderr, "\n");
	fprintf (stderr, "example:\n");
	fprintf (stderr, "%s -s vmlinux.pp42 Kerntypes.pp42\n", program_name);
	fprintf (stderr,
		"%s -s Kerntypes.pp42 -c module1.o,module2.o\n", program_name);
	fprintf (stderr, "%s -g Kerntypes.pp42\n", program_name);
	fprintf (stderr, "to set the %s magic version into a kerntypes file:\n",
		program_name);
	fprintf (stderr, " %s -m kerntypesfile\n", program_name);
}

/*
 * initialize the tables for reference translation
 */
void
init_refs_area()
{
	ref_diep = alloc_pdie_list(INITIAL_REFERENCE);
	ref_refp = alloc_offset_list(INITIAL_REFERENCE);
	num_refchecks = 0;
	size_refchecks = INITIAL_REFERENCE;
	return;
}

Dwarf_P_Die *
alloc_pdie_list(int size)
{
	Dwarf_P_Die	 *listptr;

	if ((listptr = (Dwarf_P_Die *)malloc(size * sizeof(Dwarf_P_Die)))
							== NULL) {
		printf ("cannot allocate space for %d Dwarf_P_Die pointers\n",
			size);
		exit(1);
	}
	return listptr;
}

Dwarf_Off *
alloc_offset_list(int size)
{
	Dwarf_Off	 *listptr;

	if ((listptr = (Dwarf_Off *)malloc(size * sizeof(Dwarf_Off)))
							== NULL) {
		printf ("cannot allocate space for %d Dwarf_Off offsets\n",
			size);
		exit(1);
	}
	return listptr;
}

char **
alloc_namep_list(int size)
{
	char	 **listptr;

	if ((listptr = (char **)malloc(size * sizeof(char *)))
							== NULL) {
		printf ("cannot allocate space for %d name pointers\n", size);
		exit(1);
	}
	return listptr;
}

struct typenode **
alloc_type_list(int size)
{
	struct typenode **listptr;

	if ((listptr = (struct typenode **)malloc(size *
					 sizeof(struct typenode *))) == NULL) {
		printf ("cannot allocate space for %d typenode pointers\n",
			size);
		exit(1);
	}
	return listptr;
}

Dwarf_Debug *
alloc_dbg_list(int size)
{
	Dwarf_Debug *listptr;

	if ((listptr = (Dwarf_Debug *)malloc(size *
					sizeof(Dwarf_Debug))) == NULL) {
		printf ("cannot allocate space for %d Dwarf_Debug\n",
			size);
		exit(1);
	}
	return listptr;
}

/*
 * initialize the tables for handling structure prototypes and unnamed
 * pointers and arrays
 */
void
init_misc_area()
{

	struct_protop = alloc_offset_list(INITIAL_STRUCTPROTOS);
	struct_protop2 = alloc_offset_list(INITIAL_STRUCTPROTOS);
	struct_protonamep = alloc_namep_list(INITIAL_STRUCTPROTOS);
	num_protos = 0;
	size_protos = INITIAL_STRUCTPROTOS;

	needptr_refp   = alloc_offset_list(INITIAL_NEEDPOINTERS);
	needptr_pdiep  = alloc_pdie_list(INITIAL_NEEDPOINTERS);
	needptr_typep  = alloc_type_list(INITIAL_NEEDPOINTERS);
	num_needptrs = 0;
	size_needptrs = INITIAL_NEEDPOINTERS;

	needarr_refp  = alloc_offset_list(INITIAL_NEEDARRAYS);
	needarr_pdiep = alloc_pdie_list(INITIAL_NEEDARRAYS);
	needarr_origp = alloc_offset_list(INITIAL_NEEDARRAYS);
	needarr_dbgp  = alloc_dbg_list(INITIAL_NEEDARRAYS);
	needarr_typep = alloc_type_list(INITIAL_NEEDARRAYS);
	num_needarrs = 0;
	size_needarrs = INITIAL_NEEDARRAYS;

	return;
}

/*
 * initialize the tables for remembering the pieces of segments
 */
void
init_pieces_area()
{
	num_abbrev_pieces = 0;
	num_debug_pieces = 0;
	num_reldebug_pieces = 0;

	if ((abbrev_addrp = (char **)malloc(INITIAL_PIECES *
				sizeof(char *))) == NULL) {
		printf ("cannot allocate space for abbrev addr pointers\n");
		exit(1);
	}
	if ((debug_addrp = (char **)malloc(INITIAL_PIECES *
				sizeof(char *))) == NULL) {
		printf ("cannot allocate space for debug addr pointers\n");
		exit(1);
	}
	if ((reldebug_addrp = (char **)malloc(INITIAL_PIECES *
				sizeof(char *))) == NULL) {
		printf ("cannot allocate space for reldebug addr pointers\n");
		exit(1);
	}
	if ((abbrev_lengthp = (Dwarf_Unsigned *)malloc(INITIAL_PIECES *
				sizeof(Dwarf_Unsigned))) == NULL) {
		printf ("cannot allocate space for abbrev lengths\n");
		exit(1);
	}
	if ((debug_lengthp = (Dwarf_Unsigned *)malloc(INITIAL_PIECES *
				sizeof(Dwarf_Unsigned))) == NULL) {
		printf ("cannot allocate space for debug lengths\n");
		exit(1);
	}
	if ((reldebug_lengthp = (Dwarf_Unsigned *)malloc(INITIAL_PIECES *
				sizeof(Dwarf_Unsigned))) == NULL) {
		printf ("cannot allocate space for reldebug lengths\n");
		exit(1);
	}
	size_abbrev_pieces = INITIAL_PIECES;
	size_debug_pieces = INITIAL_PIECES;
	size_reldebug_pieces = INITIAL_PIECES;

	return;
}

/*
 *  add this name and its numbers to the list of what has been found
 *
 *  unconditionally adds to the list; caller must have called
 *  lookuptype() first! (we won't check again for a duplicate)
 */
void
addtosavedtypes(char *typenamep, int member_count,
		unsigned long long int member_hash_total, Dwarf_P_Die die,
		Dwarf_Die olddie, int size, Dwarf_Half tag, int declaration)
{
	/* globaloffset is the types this thing points to */
	Dwarf_Off	offset;

	if (typenamep == 0) {
		printf ("addtosavedtypes received null typename; aborting\n");
		exit(1);
	}

#if DOTIMING
	addcnt++;
	gettimeofday(&tv, &tz);
	before = tv.tv_usec;
#endif

	/* kludge: a kernel can have some references to non-existent
	   types; remember a common base type and substitute it for
	   any such unresolvable references */
	if (tag == DW_TAG_base_type) {
		if (!strcmp(typenamep, "unsigned int")) {
			common_base_type_die = die;
		}
	}

	/* get the offset of this type */
	/* (for cross-referencing, we'll use global offsets exclusively) */
	offset = current_offset(olddie);

	inserttype(member_count, member_hash_total,
			size, tag, typenamep, die, offset, declaration);

	if (debug) {
		printf (
	  "added %s to the saved types list, offset %lld (%#llx> hash:%lld\n",
			typenamep, (long long)offset, 
                        (unsigned long long)offset, 
                        (long long)member_hash_total);
	}
	total_types++;

#if DOTIMING
	gettimeofday(&tv, &tz);
	after = tv.tv_usec;
	addtime += after-before;
	if (addcnt % 1000 == 0) {
		printf ("adds to %d: %f usec\n", addcnt, addtime);
		addtime = 0.0;
	}
#endif
	return;
}

/*
 * Helper function that allocates a new node with the given data and NULL
 * left and right pointers.
 */
inline struct typenode *
NewtypeNode(int members, unsigned long long int hash, int size, Dwarf_Off tag,
	    char *namep, Dwarf_P_Die pdie, Dwarf_Off offset, int declaration)
{
	struct typenode * node;

	node = (struct typenode *)malloc(sizeof(struct typenode));
	node->members = members;
	node->hash    = hash;
	node->size    = size;
	node->tag     = tag;
	node->namep   = namep;
	node->declaration = declaration;
	node->pdie	= pdie;
	node->offset      = offset*FMAX + current_file_number;
	node->ptrto       = NULL;
	node->arrto       = NULL;
	if (debug) {
		printf("insert name %s, die:%p type node:%p offset:%lld\n",
			namep, node->pdie, node, (long long)node->offset);
	}
	return(node);
}

/*
 * Give an AVL binary search tree and key data, insert a new node
 * with the given key in the correct place in the tree.
 * returns the new node.
 *
 * the 5 key items:
 *	members hash size tag namep form the (sort) key
 */
void
inserttype(int members, unsigned long long int hash,
	int size, Dwarf_Off tag, char *namep, Dwarf_P_Die pdie,
	Dwarf_Off offset, int declaration)
{
	struct typenode *newp;

	tree_branches = 0;

	newp = NewtypeNode(members, hash, size, tag, namep, pdie, offset,
							declaration);
	if (avl_insert(tree_base, newp)) {
		printf ("types AVL insertion FAILED\n");
		exit(1);
	}
	return;
}

/*
 * Given a binary tree, return true if a node with the target data is
 * found in the tree. Traverse down the tree, choosing the left or right
 * branch by comparing the target to each node.
 *
 * returns LNOTFOUND for not found
 * returns LFOUND    for normal found
 * returns LUNPTR    for special unnamed pointer
 * returns LUNARRAY  for special unnamed array
 */
int
lookuptype(Dwarf_Debug dbg, Dwarf_Die die, struct avl_table *node, int members,
	unsigned long long int hash, int size, Dwarf_Half tag, char *namep,
	struct typenode ** nodeaddr, int *index)
	/* get_type_values has set isptr to tell us if this is to be
	   considered a special unnamed pointer or not */
{
	int		retval;
	struct typenode *workp, worktype;

#if DOTIMING
	lookupcnt++;
	gettimeofday(&tv, &tz);
	before = tv.tv_usec;
#endif
	if (is_unnamed_pointer(dbg, die, tag, namep)) {
		/* don't save un-named pointers; we'll provide
		   them to the dies that need them during
		   do_reference_translations() */
		retval = LUNPTR;
		goto lookupexit;
	}
	if (is_unnamed_array(dbg, die, tag, namep)) {
		/* don't save un-named array links; we'll provide
		   them to the dies that need them during
		   do_reference_translations() */
		retval = LUNARRAY;
		goto lookupexit;
	}

	worktype.hash    = hash;
	worktype.members = members;
	worktype.size    = size;
	worktype.tag     = tag;
	worktype.namep   = namep;
	/* comparison is done in typenode_compare */
	/* on hash, members, size, tag, name */
	workp = avl_find(tree_base, (void *)&worktype);
	if (!workp) {
		retval = LNOTFOUND;
	} else {
		*nodeaddr = workp;  /* provide data to caller */
		retval = LFOUND;
	}
lookupexit:
#if DOTIMING
	gettimeofday(&tv, &tz);
	after = tv.tv_usec;
	lookuptime += after-before;
	if (lookupcnt % 1000 == 0) {
		printf ("lookups to %d: %f usec\n", lookupcnt, lookuptime);
		lookuptime = 0.0;
	}
#endif
	return retval;
}

/*
 * return 1 if this die is an unnamed pointer
 * else return  0
 */
int
is_unnamed_pointer(Dwarf_Debug dbg, Dwarf_Die die, Dwarf_Half tag, char *namep)
{
	Dwarf_Off	type_offset;
	Dwarf_Die	type_die;
	Dwarf_Attribute	attr;
	Dwarf_Half	type_tag;

	/* assuming get_type_values() has created namep and assigned
	   myno_die_name to an unnamed die */
	if (tag == DW_TAG_pointer_type) {
		if (!strcmp(namep, myno_die_name)) {
			/* if this is pointer to a pointer or a pointer
			   to an array link do NOT consider it an unnamed
			   pointer */
			if (!get_attr(die, DW_AT_type, &attr)) {
				/* no type attribute is possible if this is
				   a void * */
				return 1;
			} else {
				get_refoffset(attr, &type_offset);
				get_die(dbg, type_offset, &type_die);
				get_tag(type_die, &type_tag);
				if (type_tag == DW_TAG_pointer_type ||
				    type_tag == DW_TAG_array_type) {
					/* this is pointer to pointer or
					   pointer to array link */
					return 0;
				}
			}
			return 1;
		}
	}
	return 0;
}

/*
 * return 1 if this die is an unnamed array link
 * else return  0
 */
int
is_unnamed_array(Dwarf_Debug dbg, Dwarf_Die die, Dwarf_Half tag, char *namep)
{
	Dwarf_Off	type_offset;
	Dwarf_Die	type_die;
	Dwarf_Attribute	attr;
	Dwarf_Half	type_tag;

	/* assuming get_type_values() has created namep and assigned
	   myno_die_name to an unnamed die */
	if (tag == DW_TAG_array_type) {
		if (!strcmp(namep, myno_die_name)) {
			/* if this is array link to a pointer or an array
			   link to an array link do NOT consider it an unnamed
			   array link */
			if (get_attr(die, DW_AT_type, &attr)) {
				get_refoffset(attr, &type_offset);
				get_die(dbg, type_offset, &type_die);
				get_tag(type_die, &type_tag);
				if (type_tag == DW_TAG_pointer_type ||
			    	type_tag == DW_TAG_array_type) {
					/* this is array link to pointer or
				   	   array link to array link */
					return 0;
				}
				return 1;
			}
		}
	}
	return 0;
}

/*
 * when a duplicate type is rejected, save its reference (offset) and
 * an index to the type that we did save, and if its a pointer, save
 * the offset of the thing it points to
 */
void
add_alias_tree(Dwarf_Die die, struct typenode *typep, Dwarf_Off typeref)
{
	struct aliasnode *node;
	Dwarf_Off	ref;

	node = (struct aliasnode *)malloc(sizeof(struct aliasnode));

	/* get the offset of this type */
	/* (for cross-referencing, we use global offsets exclusively) */
	ref         = current_offset(die);
	node->ref   = ref*FMAX + current_file_number;
	node->typep = typep;
	if (avl_insert(alias_tree, node)) {
		printf ("alias AVL insertion FAILED\n");
		exit(1);
	}

	num_aliases++;
	return;
}

void
ref_summary()
{
	int		i;
	Dwarf_Off	*rp;
	Dwarf_P_Die	*dp;

	printf ("/nreferences summary:\n");
	for (i=0, dp=ref_diep, rp=ref_refp;
		i < num_refchecks; i++, dp++, rp++) {
		printf ("die:%p ref:%#llx\n", *dp, *rp);
	}
}

/*
 * triggered by calling  dwarf_transform_to_disk_form
 */
static int
producer_callback_common(const char *name, int size, Dwarf_Unsigned type,
	Dwarf_Unsigned flags, Dwarf_Unsigned link, Dwarf_Unsigned info,
	int *sect_name_index, int *error);
static int
producer_callback_c(const char *name, int size, Dwarf_Unsigned type,
	Dwarf_Unsigned flags, Dwarf_Unsigned link, Dwarf_Unsigned info,
	Dwarf_Unsigned *sect_name_index, void *user_data,int *error)
{
       int name_index = 0;
       int res;
       if( user_data != (void *)101) {
           unsigned long long ud = (unsigned long long)user_data;
           printf("Error: bad data passed through user_data pointer, got 0x%llx\n",ud);
           exit(1);
       }
       /* This horrible casting is so we can use common code
          here.  Our name counts are small, so there will
          be no overflow of an int. */
       name_index = (int)(*sect_name_index);
       res  = producer_callback_common(name,size,type,
           flags,link,info,&name_index,error);
       *sect_name_index = (Dwarf_Unsigned)name_index;
       return res;
}
static int
producer_callback_common(const char *name, int size, Dwarf_Unsigned type,
	Dwarf_Unsigned flags, Dwarf_Unsigned link, Dwarf_Unsigned info,
	int *sect_name_index, int *error)
{
	/*
	.debug_abbrev		will be section [1]
	.debug_info		will be section [2]
	.rel.debug_info		will be section [3]
	*/
	int 	section_number=0;

	if (!strcmp(name, ".debug_info")) {
		section_number = SECTION_DEBUGINFO;
	} else if (!strcmp(name, ".debug_abbrev")) {
		section_number = SECTION_DEBUGABBREV;
	} else if (!strcmp(name, ".rel.debug_info")
			|| !strcmp(name, ".rela.debug_info")) {
		section_number = SECTION_RELDEBUGINFO;
	} else {
		printf (
	    "WARNING: producer_callback received unexpected section name %s\n",
			name);
	}

	/* values for .e.g.  e_flags */
	if (debug) {
		printf("producer_callback: called for %s\n", name);
		printf(
		   "  size:%d type:%lld flags:%#llx link:%#llx info:%#llx\n",
			size, (long long)type, 
                        (unsigned long long)flags, 
                        (unsigned long long)link, 
                        (unsigned long long)info);
		printf("  not setting sect_name_index\n");
		printf("  returning section number %d\n", section_number);
	}
	return section_number;
}

void
producer_errhandler(Dwarf_Error errnum, Dwarf_Ptr errp)
{
	printf ("producer_errhandler entered\n");
	exit(1);
}

/*
 * read the elf header
 */
void
read_elf_header(ELF_HEADER *elfp, int fd)
{
	int		i;

	if ((i = lseek(fd, 0, 0)) < 0) {
		printf ("cannot seek to 0 fd %d\n", fd);
		exit(1);
	}
	if ((i = read(fd, elfp, sizeof(ELF_HEADER))) != sizeof(ELF_HEADER)) {
		printf ("cannot read input elf header; abort\n");
		exit(1);
	}

	switch (elfp->e_ident[EI_CLASS]) {
		case ELFCLASS32:
			if (__WORDSIZE != 32) {
				printf ("This is intended for 64bit, but the file is ELFCLASS32.\n");
				exit(1);
			}
			break;
		case ELFCLASS64:
			if (__WORDSIZE == 32) {
				printf ("This is intended for 32bit, but the file is ELFCLASS64.\n");
				exit(1);
			}
			break;
		default:
			printf("Unkown class: %i", elfp->e_ident[EI_CLASS]);
			exit(1);
			break;

	}
}

/*
 * set the dwarfextract magic e_version into a specified kerntypes file
 */
void
set_version(char *filename)
{
	int	i, fd, offset;
	Elf	*elf;
	VERSION_FIELD version_field;

	if ((fd = open(filename, O_RDWR)) < 0) {
		printf ("%s: open of %s failed\n", program_name, filename);
		exit(1);
	}
	elf = open_as_elf(fd, filename); /* verify that it's elf */
	read_elf_header(&inelfhdr, fd);
	test_endianness(&inelfhdr);
	version_field = EV_DWARFEXTRACT;
	byte_put((unsigned char *)&inelfhdr.e_version, version_field,
			sizeof(version_field));
	offset = lseek(fd, 0, SEEK_SET);
	if ((i = write(fd, &inelfhdr, sizeof(inelfhdr))) != sizeof(inelfhdr)) {
		printf ("cannot write elf header to %s; abort\n", filename);
		exit(1);
	}
	printf ("wrote special %s version %d to e_version of %s\n",
		program_name, version_field, filename);
	return;
}

/*
 * read the header section table from the input file
 */
void
setup_input_file(int infd)
{
	int		i;
	int		in_section_header_size, fnd;
	char		*strp;
	SECTION_HEADER	*shp1;

	read_elf_header(&inelfhdr, infd);

	if (verbose) {
		printf ("input ELF header:\n");
		printf ("  e_ident: %s\n", inelfhdr.e_ident);
		printf ("  e_machine: %#x\n", inelfhdr.e_machine);
		printf ("  e_phoff: %ld\n", (long)inelfhdr.e_phoff);
		printf ("  e_shoff: %ld\n", (long)inelfhdr.e_shoff);
		printf ("  e_flags: %#x\n", inelfhdr.e_flags);
		printf ("  e_ehsize: %d\n", inelfhdr.e_ehsize);
		printf ("  e_phentsize: %d\n", inelfhdr.e_phentsize);
		printf ("  e_phnum: %d\n", inelfhdr.e_phnum);
		printf ("  e_shentsize: %d\n", inelfhdr.e_shentsize);
		printf ("  e_shnum: %d\n", inelfhdr.e_shnum);
		printf ("  e_shstrndx: %d\n", inelfhdr.e_shstrndx);
	}

	/* get the section header table */
	base_shp =
	  (SECTION_HEADER *)malloc(inelfhdr.e_shnum * inelfhdr.e_shentsize);
	if ((i = lseek(infd, inelfhdr.e_shoff, 0)) < 0) {
		fprintf(stderr,"%s: cannot seek to %ld\n",
			program_name, (long)inelfhdr.e_shoff);
		exit(1);
	}
	in_section_header_size = inelfhdr.e_shentsize*inelfhdr.e_shnum;
	if ((i = read(infd, base_shp, in_section_header_size)) !=
					in_section_header_size) {
		printf ("cannot read section header table; abort\n");
		exit(1);
	}

	strp = get_strings(infd, 0);

	if (verbose) {
		printf ("input Section header table:\n");
		for (i=0, shp1=base_shp; i<inelfhdr.e_shnum; i++, shp1++) {
			printf ("  section %d:\n", i);
			printf ("    sh_name: %d (%s)\n",
				shp1->sh_name, strp+shp1->sh_name);
			printf ("    sh_type: %d\n", shp1->sh_type);
			printf ("    sh_flags: %#lx\n", (long)shp1->sh_flags);
			printf ("    sh_addr: %#lx\n", (long)shp1->sh_addr);
			printf ("    sh_offset: %#lx\n", (long)shp1->sh_offset);
			printf ("    sh_size: %ld\n", (long)shp1->sh_size);
			printf ("    sh_entsize: %ld\n", (long)shp1->sh_entsize);
		}
	}

	/* find the .debug_abbrev and .debug_info sections */
	fnd = 0;
	for (i=0, shp1=base_shp; i<inelfhdr.e_shnum; i++, shp1++) {
		if (!strcmp(strp+shp1->sh_name, ".debug_info")) {
			debug_idx = shp1->sh_name;
		}
		if (!strcmp(strp+shp1->sh_name, ".debug_abbrev")) {
			abbrev_idx = shp1->sh_name;

		}
		if (!strcmp(strp+shp1->sh_name, ".rel.debug_info")
				|| !strcmp(strp+shp1->sh_name, ".rela.debug_info")) {
			reldebug_idx = shp1->sh_name;
			fnd++;
		}


	}

	free(strp);

	if (fnd == 0) {
		no_input_rel_debug_info_section++;
	}

	/* make the new ELF header from the old one, then tailor */
	outelfhdr = inelfhdr;
	outelfhdr.e_version = EV_DWARFEXTRACT;
	outelfhdr.e_entry = 0;
	outelfhdr.e_phoff = 0;
	outelfhdr.e_phnum = 0;
	outelfhdr.e_shoff = sizeof(outelfhdr);
	outelfhdr.e_shentsize = sizeof(SECTION_HEADER);
	outelfhdr.e_shnum = NUM_SECTIONS;
	outelfhdr.e_shstrndx = SECTION_STRING;  /* string section index */
		/* empty[0], abbrev[1], debug[2], rel.debug[3] strings[4] */

	if (verbose) {
		printf ("output ELF header:\n");
		printf ("  e_ident: %s\n", outelfhdr.e_ident);
		printf ("  e_machine: %#x\n", outelfhdr.e_machine);
		printf ("  e_phoff: %ld\n", (long)outelfhdr.e_phoff);
		printf ("  e_shoff: %ld\n", (long)outelfhdr.e_shoff);
		printf ("  e_flags: %#x\n", outelfhdr.e_flags);
		printf ("  e_ehsize: %d\n", outelfhdr.e_ehsize);
		printf ("  e_phentsize: %d\n", outelfhdr.e_phentsize);
		printf ("  e_phnum: %d\n", outelfhdr.e_phnum);
		printf ("  e_shentsize: %d\n", outelfhdr.e_shentsize);
		printf ("  e_shnum: %d\n", outelfhdr.e_shnum);
		printf ("  e_shstrndx: %d\n", outelfhdr.e_shstrndx);
	}

	return;
}

/*
 * open the input file and (if -t) display its elf setup
 */
void
trace_input_elf_file(int infd, char *filename)
{

	int		i, in_section_header_size;
	char		*strp;
	SECTION_HEADER	*shp1;

	read_elf_header(&inelfhdr, infd);

	if (tflag) {
		printf ("input ELF header:\n");
		printf ("  e_ident: %s\n", inelfhdr.e_ident);
		printf ("  e_typ: %d\n", inelfhdr.e_type);
		printf ("  e_machine: %#x\n", inelfhdr.e_machine);
		printf ("  e_phoff: %ld\n", (long)inelfhdr.e_phoff);
		printf ("  e_shoff: %ld\n", (long)inelfhdr.e_shoff);
		printf ("  e_flags: %#x\n", inelfhdr.e_flags);
		printf ("  e_ehsize: %d\n", inelfhdr.e_ehsize);
		printf ("  e_phentsize: %d\n", inelfhdr.e_phentsize);
		printf ("  e_phnum: %d\n", inelfhdr.e_phnum);
		printf ("  e_shentsize: %d\n", inelfhdr.e_shentsize);
		printf ("  e_shnum: %d\n", inelfhdr.e_shnum);
		printf ("  e_shstrndx: %d\n", inelfhdr.e_shstrndx);
	}

	/* get the section header table */
	base_shp =
	  (SECTION_HEADER *)malloc(inelfhdr.e_shnum * inelfhdr.e_shentsize);
	if ((i = lseek(infd, inelfhdr.e_shoff, 0)) < 0) {
		fprintf(stderr,"%s: cannot seek to %ld\n",
			program_name, (long)inelfhdr.e_shoff);
		exit(1);
	}
	in_section_header_size = inelfhdr.e_shentsize*inelfhdr.e_shnum;
	if ((i = read(infd, base_shp, in_section_header_size)) !=
					in_section_header_size) {
		printf ("cannot read section header table; abort\n");
		exit(1);
	}

	/* get the string section */
	strp = get_strings(infd, 0);

	if (tflag) {
		printf ("input Section header table:\n");
		for (i=0, shp1=base_shp; i<inelfhdr.e_shnum; i++, shp1++) {
			printf ("  section %d:\n", i);
			printf ("    sh_name: %d (%s)\n",
				shp1->sh_name, strp+shp1->sh_name);
			printf ("    sh_type: %d\n", shp1->sh_type);
			printf ("    sh_flags: %#lx\n", (unsigned long)shp1->sh_flags);
			printf ("    sh_addr: %#lx\n", (unsigned long)shp1->sh_addr);
			printf ("    sh_offset: %#lx\n", (unsigned long)shp1->sh_offset);
			printf ("    sh_size: %ld\n", (long)shp1->sh_size);
			printf ("    sh_entsize: %ld\n", (long)shp1->sh_entsize);
		}
	}

	/* find the .debug_abbrev and .debug_info sections */
	debug_idx = -1;
	abbrev_idx = -1;
	for (i=0, shp1=base_shp; i<inelfhdr.e_shnum; i++, shp1++) {
		if (!strcmp(strp+shp1->sh_name, ".debug_info")) {
			debug_idx = shp1->sh_name;
		}
		if (!strcmp(strp+shp1->sh_name, ".debug_abbrev")) {
			abbrev_idx = shp1->sh_name;
		}
		if (!strcmp(strp+shp1->sh_name, ".rel.debug_info")
				|| !strcmp(strp+shp1->sh_name, ".rela.debug_info")) {
			reldebug_idx = shp1->sh_name;
		}
	}
	if (debug_idx == -1) {
		printf ("no debug info found in %s; aborted\n",
			in_file_name);
		printf ("(could this be a 2.4 kernel and gcc version 3.2?)\n");
		printf ("(for 2.6 we switch from stabs to dwarf debugging)\n");
		exit(1);
	}

	if (abbrev_idx == -1) {
		printf ("no abbrev info found in %s; aborted\n",
			in_file_name);
		exit(1);
	}

	return;
}

/* we expect to convert these attributes, by experience
by class:

reference:
attribute:0x1    DW_AT_sibling
attribute:0x49   DW_AT_type

string:
attribute:0x3    DW_AT_name

constant:
attribute:0x1c   DW_AT_const_value
attribute:0xb    DW_AT_byte_size
attribute:0xb    DW_AT_bit_size
attribute:0xb    DW_AT_offset_size
attribute:0x3a   DW_AT_decl_file
attribute:0x3b   DW_AT_decl_line
attribute:0x3e   DW_AT_encoding

location expression:
attribute:0x38   DW_AT_data_member_location
attribute:0x40   DW_AT_frame_base
attribute:0x2    DW_AT_location

flag:
attribute:0x3c   DW_AT_declaration	subprograms and unions
attribute:0x27   DW_AT_prototyped	only in subprogram
attribute:0x3f   DW_AT_external		only in subprogram
attribute:0x2d   DW_TAG_packed_type	packed structures (never used?)

address_b:
attribute:0x11   DW_AT_low_pc		only in subprogram
attribute:0x12   DW_AT_high_pc		only in subprogram
*/

/*
 * make a new P_Die from the old Die
 * (if "isptr" is set, this is a known pointer to an unnamed pointer
 *  die, so don't add this reference to the list to be translated;
 *  these pointers will be constructed at the end)
 */
Dwarf_P_Die
convert_to_new(Dwarf_Debug olddbg, Dwarf_P_Debug dbg, Dwarf_Die olddie,
		int isptr)
{
	int			errv, i, is_a_declaration=0;
	int			result, soffset, len;
	char			*stringp, *stringp2;
	Dwarf_Half		tag, formtype, attr_val;
	Dwarf_Tag		newtag;
	Dwarf_Error		error;
	Dwarf_Bool		tf;
	Dwarf_Off		globaloffset;
	Dwarf_Unsigned		sym_index, exprres, val1, val2, value;
	Dwarf_Signed		atcnt, svalue;
	Dwarf_Attribute		*atlist;  /* this is a pointer to a pointer */
	Dwarf_Attribute		old_attr; /* a pointer */
	Dwarf_Addr		pc_value;
	Dwarf_P_Attribute	aresult, new_attr; /* a pointer */
	Dwarf_P_Die		newdie;   /* this is a pointer */
	Dwarf_P_Expr		loc_expr;

	get_tag(olddie, &tag);
	newtag = tag;

	result = dwarf_hasattr(olddie, DW_AT_declaration, &tf, &error);
	/* caution: do not test error, it is not changed */
	if (result != DW_DLV_OK) {
		printf ("convert_to_new: dwarf_hasattr failed\n");
		exit(1);
	}
	if (tf && tag == DW_TAG_structure_type) {
		is_a_declaration=1;
	}

	errv = dwarf_attrlist(olddie, &atlist, &atcnt, &error);
	if ((errv != DW_DLV_OK) && (errv != DW_DLV_NO_ENTRY)) {
		printf ("cannot get attr list in convert_to_new\n");
		exit(1);
	}

	/* we have the tag and all the attributes, so construct the
	   new producer Dwarf_P_Die */
	/* caution: error is not changed */
	newdie = dwarf_new_die(dbg, newtag, NULL,NULL,NULL,NULL, &error);
	if (newdie == (Dwarf_P_Die)DW_DLV_BADADDR) {
		printf ("dwarf_new_die failed in convert_to_new\n");
		exit(1);
	}
	if (debug) {
		printf ("converted die named %s (at %p) to new die %p\n",
			name_of_die(olddie), olddie, newdie);
	}

	/* test result of the dwarf_attrlist() call */
	if (errv == DW_DLV_NO_ENTRY) {
		/* there were no entries; no list was allocated */
		return newdie;
	}

	/* add all the attributes */
	/*  (example from dwarf_add_AT_name() */
	for (i=0; i<atcnt; i++) {
		old_attr = atlist[i];
		errv = dwarf_whatattr(old_attr, &attr_val, &error);
		if (errv != DW_DLV_OK) {
			printf ("convert_to_new: dwarf_whatattr error\n");
			exit(1);
		}

		switch (attr_val) {
		/* class FLAG */
		case DW_AT_declaration:
		case DW_AT_prototyped:
		case DW_AT_external:
		case DW_TAG_packed_type:
		{
			errv = dwarf_formflag(old_attr, &tf, &error);
			/* get the true/false flag from the old attr */
			new_attr =
			  dwarf_add_AT_flag(newdbg, newdie, attr_val, tf,
					  &error);
			if (new_attr == (Dwarf_P_Attribute)DW_DLV_BADADDR) {
				printf (
				  "convert_to_new: dwarf_add_AT_flag failed\n");
				exit(1);
			}
			break;
		}
		/* class REFERENCE */
		case DW_AT_sibling:
			/* There is no apparent need for the DW_AT_sibling
			   attribute;  subprograms use it, but we're not
			   interested in their type;  and structures use
			   it to reference the following structure.  But
			   as the next structure described in the code is a
			   natural sibling (by position), this would seem
			   to be unnecessary.  We'll try to just ignore
			   this attribute for now, and not create one
			   in the new die. */

			/* note!! transform_to_disk_form adds this attribute
			   to those dies with children */
			break;
		case DW_AT_type:
		{
			/* sanity check */
			errv = dwarf_whatform(old_attr, &formtype, &error);
			if (formtype != DW_FORM_ref1 &&
			    formtype != DW_FORM_ref2 &&
			    formtype != DW_FORM_ref4 &&
			    formtype != DW_FORM_ref8) {
				printf ("attribute is form %#x, not REF\n",
					formtype);
				exit(1);
			}

			/* get the offset of the thing this die's  attribute
			   is pointing to */
			/* (for cross-referencing, we'll use global
			    offsets exclusively) */
			get_refoffset(old_attr, &globaloffset);
			/* save the die that has a reference, and the
			   (untranslated) reference */
			/* (but not for unnamed pointers and array links) */
			if (!isptr) {
				addtoreflist(newdie, globaloffset);
				if (debug) {
				 printf(
				   "die:%s made reference to %lld <%#llx>\n",
					name_of_die(olddie), 
                                        (long long)globaloffset,
					 (unsigned long long)globaloffset);
				}
			}
			break;
		}
		/* class STRING */
		case DW_AT_name:
		{
			errv = dwarf_formstring(old_attr, &stringp, &error);
			/* get the address of the string from the old attr */

			if (firstnewdie && !cflag) {
				/*  on the first die (the cu), we make
				    this string the input file name; but
				    if we are concatenating other files
				    (using -c) keep the old file name */
				stringp = in_file_name;
			}
			len = (int)strlen(stringp);
			if (is_a_declaration) {
			  /* make the name of a declaration throw-away; we do
			     not want structure prototypes (length 0) to be
 			     used for the real structure */
				if (len < 5 || strcmp(stringp+len-5, "_Decl")){
					/* don't append _Decl again */
					stringp2 = (char *)malloc(len + 6);
					strcpy(stringp2, stringp);
					strcat(stringp2, "_Decl");
				} else {
					stringp2 = (char *)malloc(len + 1);
					strcpy(stringp2, stringp);
				}
			} else {
				/* allocate a new space for the name so that
				   we can close the old dbg space */
				stringp2 = (char *)malloc(len + 1);
				strcpy(stringp2, stringp);
			}
			new_attr = dwarf_add_AT_string(newdbg, newdie, attr_val,
					stringp2, &error);

			free(stringp2);

			if (new_attr == (Dwarf_P_Attribute)DW_DLV_BADADDR) {
				printf (
				"convert_to_new: dwarf_add_AT_string failed\n");
				exit(1);
			}
			break;
		}
		/* class CONSTANT */
		case DW_AT_byte_size:
		case DW_AT_decl_file:
		case DW_AT_decl_line:
		case DW_AT_encoding:
		case DW_AT_const_value:
		case DW_AT_bit_size:
		case DW_AT_bit_offset:
		{
			result = dwarf_whatform(old_attr, &formtype, &error);
			if(result == DW_DLV_ERROR) {
				printf("dwarf_whatform error\n");
				exit(1);
       			}
			if (formtype == DW_FORM_sdata) {
				errv = dwarf_formsdata(old_attr,&svalue,&error);
				if (errv == DW_DLV_ERROR) {
					printf("dwarf_formsdata error\n");
					exit(1);
				}
				value = svalue;
			} else {
				errv = dwarf_formudata(old_attr,&value,&error);
				if (errv == DW_DLV_ERROR) {
					printf (
			    "convert_to_new constant: dwarf_formudata error\n");
					exit(1);
				}
			}
			/* seems that this should be a signed constant for the
			   sdata case, but it's not allowed for these
			   attributes; so do unsigned for both cases */
			/* add the address of the string from the old attr */
			new_attr = dwarf_add_AT_unsigned_const(newdbg, newdie,
				attr_val, value, &error);
			if (new_attr == (Dwarf_P_Attribute)DW_DLV_BADADDR) {
				printf ("dwarf_add_AT_unsigned_const failed\n");
				exit(1);
			}
			break;
		}
		/* class LOCATION EXPRESSION */
		case DW_AT_data_member_location:
			 /* needed on structure members */
		case DW_AT_location:  /* used on unions */
		case DW_AT_frame_base:
		{
			/* extract the offset from the old attribute */
			/* (assuming here that it is a member offset) */
			/* (treat AT_location the same way) */
			soffset = get_offset(olddbg, old_attr, attr_val);
			val1 = soffset;
			val2 = 0;

			/* create a new, empty attribute */
			loc_expr = dwarf_new_expr(dbg, &error);
			if (loc_expr == NULL) {
				printf ("dwarf_new_expr failed\n");
				exit(1);
			}

			/* add the expression to the new attribute */
			/* DW_OP_const4s    constant 4-byte unsigned */
			exprres = dwarf_add_expr_gen(loc_expr, DW_OP_constu,
				val1, val2, &error);
			if (exprres == DW_DLV_NOCOUNT) {
				printf ("dwarf_add_expr_gen failed\n");
				exit(1);
			}

			/* add the attribute to the die */
			new_attr = dwarf_add_AT_location_expr(newdbg,
					newdie, attr_val, loc_expr, &error);
			if (new_attr == (Dwarf_P_Attribute)DW_DLV_BADADDR) {
				printf (
				"dwarf_add_AT_location_expression failed\n");
				exit(1);
			}
			break;
		}
		/* class  ADDRESS */
		case DW_AT_low_pc:
		case DW_AT_high_pc:
		{
			errv = dwarf_formaddr(old_attr, &pc_value, &error);
			/* get the address of the string from the old attr */
			sym_index = 0; /* just use 0, don't seem to need it */
			new_attr = dwarf_add_AT_targ_address(newdbg, newdie,
					attr_val, pc_value, sym_index, &error);
			if (new_attr == (Dwarf_P_Attribute)DW_DLV_BADADDR) {
				printf ( "dwarf_add_AT_targ_address failed\n");
				exit(1);
			}
			break;
		}
		case DW_AT_upper_bound:
		{
			errv = dwarf_formudata(old_attr, &value, &error);
			if (errv == DW_DLV_ERROR) {
				printf(
			"convert_to_new upper bound: dwarf_formudata error\n");
				exit(1);
			}
			svalue = value;
			new_attr = dwarf_add_AT_signed_const(newdbg, newdie,
				attr_val, svalue, &error);
			if (new_attr == (Dwarf_P_Attribute)DW_DLV_BADADDR) {
				printf ( "dwarf_add_AT_signed_const failed\n");
				exit(1);
			}
			break;
		}
		case DW_AT_language:
		{
			errv = dwarf_srclang(olddie, &value, &error);
			if (errv != DW_DLV_OK) {
				printf("dwarf_srclang error\n");
				exit(1);
			}
			new_attr = dwarf_add_AT_unsigned_const(newdbg, newdie,
				attr_val, value, &error);
			if (new_attr == (Dwarf_P_Attribute)DW_DLV_BADADDR) {
				printf ("dwarf_add_AT_unsigned_const failed\n");
				exit(1);
			}
			break;
		}
		default:
		{
			if (debug) {
				printf ("die:%s: ", name_of_die(olddie));
			}
			if (attr_val == DW_AT_stmt_list) {
				if (debug)
					printf ("ignoring DW_AT_stmt_list\n");
			} else if (attr_val == DW_AT_const_value) {
				if (debug)
					printf ("ignoring DW_AT_const_value\n");
			} else if (attr_val == DW_AT_producer) {
				if (debug)
					printf ("ignoring DW_AT_producer\n");
			} else if (attr_val == DW_AT_comp_dir) {
				if (debug)
					printf ("ignoring DW_AT_comp_dir\n");
			} else if (attr_val == DW_AT_macro_info) {
				if (debug)
					printf ("ignoring DW_AT_macro_info\n");
			} else if (attr_val == DW_AT_ranges) {
				if (debug)
					printf ("ignoring DW_AT_ranges\n");
			} else if (attr_val == DW_AT_artificial) {
				if (debug)
					printf ("ignoring DW_AT_artificial\n");
			} else if (attr_val == DW_AT_abstract_origin) {
				/* this one is common in the kernel */
				if (debug)
				   printf ("ignoring DW_AT_abstract_origin\n");
			} else if (attr_val == DW_AT_inline) {
				/* this one is common in the kernel */
				if (debug)
					printf ("ignoring DW_AT_inline\n");
			} else if (attr_val ==  DW_AT_entry_pc) {
				if (debug)
					printf ("ignoring DW_AT_entry_pc\n");
			} else if (attr_val ==  DW_AT_specification) {
				if (debug)
					printf ("ignoring DW_AT_specification\n");
			} else if (attr_val ==  DW_AT_MIPS_linkage_name) {
				if (debug)
					printf ("ignoring DW_AT_MIPS_linkage_name\n");
			} else if (attr_val ==  DW_AT_GNU_all_tail_call_sites ) {
				if (debug)
					printf ("ignoring DW_AT_GNU_all_tail_call_sites \n");
			} else {
				printf ("%s: attribute %#x not converted\n",
					name_of_die(olddie), attr_val);
				printf ("aborting\n");
				exit(1);
			}
		}
		} /* end of switch */
	}

	/* now remove the temporary list describing the old one */
	for (i=0; i<atcnt; i++) {
		dwarf_dealloc(olddbg, atlist[i], DW_DLA_ATTR);
	}
	dwarf_dealloc(olddbg, atlist, DW_DLA_LIST);

	/* kludge: you might see a typedef in the kernel that has
	   no type attribute.  If that is the case, add a reference
	   to a common base type (just to keep lcrash from trapping) */
	if (tag == DW_TAG_typedef) {
		result = dwarf_hasattr(olddie, DW_AT_type, &tf, &error);
		if (result != DW_DLV_OK) {
			printf ("dwarf_hasattr failed\n");
			exit(1);
		}
		if (!tf) {
			if (common_base_type_die == (Dwarf_P_Die)0) {
				/* we haven't see long int defined yet */
				printf (
				"convert_to_new: kludge before common\n");
				exit(1);
			}
			/* this one doesn't have a type reference */
			aresult = dwarf_add_AT_reference(dbg, newdie,
					DW_AT_type, common_base_type_die,
					&error);
			if (aresult == (Dwarf_P_Attribute)DW_DLV_BADADDR) {
				printf (
				"convert_to_new: add AT_ref failed\n");
				exit(1);
			}
			if (debug) {
				printf ("kludge: type given to typedef %s\n",
					name_of_die(olddie));
			}
		}
	}

	total_newdies++;
	firstnewdie = 0;
	return newdie;
}

/*
 * add a die to the list of dies that will need an attribute added
 * to them later (when the die they point at is known);
 */
void
addtoreflist(Dwarf_P_Die die, Dwarf_Off ref)
{
	if (num_refchecks >= size_refchecks) {
		size_refchecks = size_refchecks + (size_refchecks/2);
		if (pflag) {
			printf ("bump size_refchecks to %d and realloc\n",
				size_refchecks);
		}
		if ((ref_refp = (Dwarf_Off *)realloc((void *)ref_refp,
			size_refchecks * sizeof(Dwarf_Off))) == NULL) {
			printf ("cannot realloc space for ref offsets\n");
			exit(1);
		}
		if ((ref_diep =
			(Dwarf_P_Die *)realloc((void *)ref_diep,
			size_refchecks * sizeof(Dwarf_P_Die))) == NULL) {
			printf ("cannot realloc space for ref dies\n");
			exit(1);
		}
		if (pflag) {
			printf ("reallocs complete\n");
		}
	}

	*(ref_diep + num_refchecks) = die;
	*(ref_refp + num_refchecks) = ref*FMAX + current_file_number;
	if (debug) {
		printf(
		   "adding die %p reference to %lld to be resolved\n",
			die, (long long)ref);
	}

	num_refchecks++;
	return;
}

/*
 * now that we've built our new tree, walk thru the lists created by
 * addtoreflist and create an attribute for each die that had a
 * reference to another (these should all be DW_AT_type attributes)
 */
void
do_reference_translations()
{
	int			i, j, j2, fnd;
	char			*namep;
	Dwarf_Error		error;
	Dwarf_P_Die		dp1, dp2;
	Dwarf_Off		rp1, rp1b, rp2, offset; /* unsigned long long */
	Dwarf_P_Attribute	aresult;
	struct typenode		*typep;
	struct aliasnode	*aliasp;

	if (Pflag) {
		gettimeofday(&tv, &tz);
		printf ("elapsed time: %ld seconds\n", (long)tv.tv_sec-startsec);
	}
	if (verbose || pflag) {
		if (num_refchecks > INITIAL_REFERENCE) {
			printf ("NOTE: increase INITIAL_REFERENCE above %d\n",
				num_refchecks);
		}
		if (num_protos > INITIAL_STRUCTPROTOS) {
		       printf ("NOTE: increase INITIAL_STRUCTPROTOS above %d\n",
				num_protos);
		}
		if (num_needptrs > INITIAL_NEEDPOINTERS) {
		       printf ("NOTE: increase INITIAL_NEEDPOINTERS above %d\n",
				num_needptrs);
		}
		if (num_needarrs > INITIAL_NEEDARRAYS) {
			printf ("NOTE: increase INITIAL_NEEDARRAYS above %d\n",
				num_needarrs);
		}
	}
	if (debug) {
		printf ("starting reference translations\n");
	}

	if (pflag) {
		printf ("sorting lists of %d, %d and %d\n",
			num_refchecks, num_needptrs, num_needarrs);
	}
	sort_references();
	/* and make identical lists for the proto translations */
	ref_diep2 = alloc_pdie_list(num_refchecks);
	ref_refp2 = alloc_offset_list(num_refchecks);
	for (j=0; j<num_refchecks; j++) {
		*(ref_refp2 + j) = *(ref_refp + j);
		*(ref_diep2 + j) = *(ref_diep + j);
	}
	sort_needptrs();
	/* and make identical lists for the proto translations */
	needptr_refp2  = alloc_offset_list(num_needptrs);
	needptr_pdiep2 = alloc_pdie_list(num_needptrs);
	needptr_typep2 = alloc_type_list(num_needptrs);
	for (j=0; j<num_needptrs; j++) {
		*(needptr_refp2  + j) = *(needptr_refp  + j);
		*(needptr_pdiep2 + j) = *(needptr_pdiep + j);
		*(needptr_typep2 + j) = *(needptr_typep + j);
	}
	sort_needarrs();
	/* and make identical lists for the proto translations */
	needarr_refp2  = alloc_offset_list(num_needarrs);
	needarr_pdiep2 = alloc_pdie_list(num_needarrs);
	needarr_typep2 = alloc_type_list(num_needarrs);
	needarr_origp2 = alloc_offset_list(num_needarrs);
	needarr_dbgp2  = alloc_dbg_list(num_needarrs);
	for (j=0; j<num_needarrs; j++) {
		*(needarr_refp2  + j) = *(needarr_refp  + j);
		*(needarr_pdiep2 + j) = *(needarr_pdiep + j);
		*(needarr_typep2 + j) = *(needarr_typep + j);
		*(needarr_dbgp2  + j) = *(needarr_dbgp  + j);
		*(needarr_origp2 + j) = *(needarr_origp + j);
	}

	/* need a quicker way to look up saved names than walking the tree,
	   so make sorted arrays of the hashed names and offset */
	make_types_name_ref_hash();

	if (pflag) {
		printf ("translating references to %d prototype structures\n",
			num_protos);
	}
	/* The protos is a list of all the structure prototypes (by their
	   offsets).  All references to these structures should be translated
	   to the real structure */
	for (i=0; i<num_protos; i++) {
		rp1 = *(struct_protop  + i);   /* offset within one dbg */
		rp1b = *(struct_protop2 + i);  /* offset within all files */
		namep = *(struct_protonamep + i); /* die name */
		typep = NULL;
		/* use the list set up by make_types_name_ref_hash */
		/* will find the non-prototype item if name is ambiguous */
		if (lookup_type_name(namep, &typep)) {
			/* this prototype structure was later defined and
			   recorded in the types tree */
			rp2 = typep->offset;
			/* change all references to a prototype (rp1b) to
			   point to the real full type (rp2) */
			if (lookup_reference_ref(rp1b, &j, &j2)) {
				for (; j<=j2; j++) {
					*(ref_refp2 + j) = rp2;
				}
			}
			/* and if anything was aliased to the prototype,
			   point it to the full type instead */
			aliasp = lookup_alias_ref(rp1b);
			if (aliasp){
				/* modify the alias to the real type */
				aliasp->typep = typep;
			}
			/* and if any unnamed pointers pointed to it,
		   	   point them to the full type as well */
			if (lookup_needptr_ref(rp1b, &j, &j2)) {
				/* modify the alternate list */
				for (; j<=j2; j++) {
					*(needptr_refp2 + j) = rp2;
				}
			}
			/* and if any unnamed array links pointed to it,
		   	   point them to the full type as well */
			if (lookup_needarr_ref(rp1b, &j, &j2)) {
				/* modify the alternate list */
				for (; j<=j2; j++) {
					*(needarr_refp2 + j) = rp2;
				}
			}
		}
	}
	if (Pflag) {
		gettimeofday(&tv, &tz);
		printf ("elapsed time: %ld seconds\n", (long)tv.tv_sec-startsec);
	}
	if (pflag) {
		printf ("re-sorting list of %d\n", num_refchecks);
	}
	/* the updated alternate lists become the future lists */
	free (ref_refp);
	free (ref_diep);
	ref_refp = ref_refp2;
	ref_diep = ref_diep2;
	sort_references(); /* sort for future lookups */

	free (needptr_refp);
	free (needptr_pdiep);
	free (needptr_typep);
	needptr_refp = needptr_refp2;
	needptr_pdiep = needptr_pdiep2;
	needptr_typep = needptr_typep2;
	/* no need to sort - walked sequentially from here on */

	free (needarr_refp);
	free (needarr_pdiep);
	free (needarr_typep);
	free (needarr_dbgp);
	free (needarr_origp);
	needarr_refp = needarr_refp2;
	needarr_pdiep = needarr_pdiep2;
	needarr_typep = needarr_typep2;
	needarr_dbgp = needarr_dbgp2;
	needarr_origp = needarr_origp2;
	/* no need to sort - walked sequentially from here on */

	/* fill in the typenode pointer in the list of dies needing
	   a pointer created */
	if (pflag) {
		printf ("resolving %d dies needing pointers\n", num_needptrs);
	}
	for (i=0; i<num_needptrs; i++) {
		fnd = 0;
		if (*(needptr_typep + i) == 0) {
			offset = *(needptr_refp + i);
			if (offset < FMAX) {
				/* a void  pointer has no type attribute,
				   so the offset is 0*FMAX + file number */
				/* leave *(needptr_typep + i) NULL */
			} else {
				if (lookup_type_ref(offset, &typep)) {
					fnd = 1;
					*(needptr_typep + i) = typep;
				} else {
					aliasp = lookup_alias_ref(offset);
					if (aliasp){
						*(needptr_typep + i) =
							aliasp->typep;
						typep = aliasp->typep;
						fnd = 1;
					} else {
						printf (
		  "needptr slot %d: offset %#llx cannot be resolved; ABORT\n\n",
								i, offset);
						exit(1);
					}
				}
				/* propagate the found one to the rest of
				   the array */
				if (fnd) {
					for (j=i+1; j<num_needptrs; j++) {
						if (*(needptr_refp + j) ==
								offset) {
							*(needptr_typep + j) =
									typep;
						}
					}
				}
			}
		}
	}
	/* make the new dies for the unnamed pointers and connect
	   the referencing dies to them */
	for (i=0; i<num_needptrs; i++) {
		/* this is the type that this die needs a pointer to: */
		typep = *(needptr_typep + i);
		if (typep == NULL) {
			/* a void *;  the pointer we make will not
			   have a type attribute */
			/* make all the void *'s point to the same p_die */
			if (void_pointer_die == 0) {
				dp2 = make_pointer_die(0);
				void_pointer_die = dp2;
			}
			dp1   = *(needptr_pdiep + i);
			/* link this die to the void pointer die */
			link_to_pointer(dp1, void_pointer_die);
		} else {
			/* link this die to the single pointer to this type */
			if (typep->ptrto == NULL) {
				dp2 = make_pointer_die(typep->pdie);
				typep->ptrto = dp2;
			}
			dp1   = *(needptr_pdiep + i);
			/* link this die to the pointer die */
			link_to_pointer(dp1, typep->ptrto);
		}
	}

	/* fill in the typenode pointer in the list of dies needing
	   an array link created */
	if (pflag) {
		printf ("resolving %d dies needing array links\n",num_needarrs);
	}
	for (i=0; i<num_needarrs; i++) {
		fnd = 0;
		if (*(needarr_typep + i) == 0) {
			offset = *(needarr_refp + i);
			if (lookup_type_ref(offset, &typep)) {
				fnd = 1;
				*(needarr_typep + i) = typep;
			} else {
				aliasp = lookup_alias_ref(offset);
				if (aliasp){
					*(needarr_typep + i) = aliasp->typep;
					typep = aliasp->typep;
					fnd = 1;
				} else {
					printf (
			   "needarr: offset %lld cannot be resolved; ABORT\n\n",
					(long long)offset);
					exit(1);
				}
			}
			/* propagate the found one to the rest of the array */
			if (fnd) {
				for (j=i+1; j<num_needarrs; j++) {
					if (*(needarr_refp + j) == offset) {
						*(needarr_typep + j) =
							typep;
						fnd = 1;
						break;
					}
				}
			}
		}
	}
	/* make the new dies for the unnamed array links and connect
	   the referencing dies to them */
	for (i=0; i<num_needarrs; i++) {
		typep = *(needarr_typep + i);
		dp2 = make_array_die(typep->pdie);
		add_subranges(dp2, *(needarr_dbgp+i), *(needarr_origp+i));
		dp1   = *(needarr_pdiep + i);
		link_to_pointer(dp1, dp2);
	}

	if (Pflag) {
		gettimeofday(&tv, &tz);
		printf ("elapsed time: %ld seconds\n", (long)(tv.tv_sec-startsec));
	}
	/* we've saved pointers to all the dies needing a reference, */
	/* along with the global reference number (offset) */
	if (pflag) {
		printf ("translating %d type references\n", num_refchecks);
	}
	for (i=0;  i<num_refchecks; i++) {
		/*  the die, and what it needs to reference: */
		dp1 = *(ref_diep + i);
		rp1 = *(ref_refp + i);
		if (pflag) {
			if (i > 0 && i % 10000 == 0) {
				printf ("%d translated\n", i);
			}
		}
		if (debug) {
			printf ("translating offset %lld <%#llx>\n", 
                          (long long)rp1, (unsigned long long)rp1);
		}

		/* walk our tree of saved types for a match of what
		  	   this die wants to reference */
		if (debug) {
			printf ("checking the tree\n");
		}
		fnd = 0;
		typep = NULL;
		if (lookup_type_ref(rp1, &typep)) {
			fnd = 1;
			dp2 = typep->pdie;
			if (debug) {
				printf ("tree yielded pdie:%p\n", dp2);
			}
		} else {
			if (debug) {
				printf ("checking the aliases\n");
			}
			/* not in saved types, check the rejected (duplicate)
			   types list */
			aliasp = lookup_alias_ref(rp1);
			if (aliasp){
				typep = aliasp->typep;
				dp2 = typep->pdie;
				if (debug) {
					printf ("aliases yielded pdie:%p\n",
						dp2);
				}
				fnd = 1;
			}
		}

		if (fnd == 1) {
			aresult = dwarf_add_AT_reference(newdbg, dp1,
					DW_AT_type, dp2, &error);
			if (aresult == (Dwarf_P_Attribute)DW_DLV_BADADDR) {
				printf (
			    "do_reference_translations: add failed; ABORT\n\n");
				exit(1);
			}
		} else {
			/* on this warning, better check the types
			   we choose in is_type_we_want */
			printf (
 		      "cannot translate reference to %#llx (%lld file %lld); ",
				(unsigned long long)rp1, 
                                (unsigned long long)(rp1/FMAX), 
                                (unsigned long long)(rp1 % FMAX));
			printf ("making it reference long int\n");
			aresult = dwarf_add_AT_reference(newdbg, dp1,
				DW_AT_type, common_base_type_die,
				&error);
			if (aresult == (Dwarf_P_Attribute)DW_DLV_BADADDR) {
				printf (
			    "do_reference_translations: add failed; ABORT\n\n");
				exit(1);
			}
		}
	}
	return;
}

/*
 * construct DW_TAG_subrange P_Dies as children of the P_Die we just created
 *  use the old ones at dbg/offset as templates
 */
void
add_subranges(Dwarf_P_Die pdie, Dwarf_Debug dbg, Dwarf_Off offset)
{
	Dwarf_Die	olddie, child_die, sibling_die;
	Dwarf_Half	dietag;
	Dwarf_P_Die	newpdie, dumdie;
	Dwarf_Error	error;

	get_die(dbg, offset, &olddie);
	if (get_child(olddie, &child_die)) {
		get_tag(child_die, &dietag);
		if (dietag == DW_TAG_subrange_type) {
			newpdie = convert_subrange(dbg, newdbg, child_die);
			dumdie = dwarf_die_link(newpdie, pdie, NULL, NULL, NULL,
								&error);
		}
		while (get_sibling(dbg, child_die, &sibling_die)) {
			child_die = sibling_die;
			get_tag(child_die, &dietag);
			if (dietag == DW_TAG_subrange_type) {
				newpdie = convert_subrange(dbg, newdbg,
							child_die);
				dumdie = dwarf_die_link(newpdie, pdie, NULL,
							NULL, NULL, &error);
			}
		}
	}
	return;
}

/*
 * make a new subrange P_Die from the old Die
 *  convert only the bound and reference attributes (should be all the
 *  attributes it has)
 */
Dwarf_P_Die
convert_subrange(Dwarf_Debug olddbg, Dwarf_P_Debug dbg, Dwarf_Die olddie)
{
	int			errv, i;
	Dwarf_Half		attr_val;
	Dwarf_Error		error;
	Dwarf_Signed		atcnt;
	Dwarf_Signed		svalue;
	Dwarf_Unsigned		uvalue;
	Dwarf_Attribute		*atlist, old_attr;
	Dwarf_P_Attribute	aresult, new_attr;
	Dwarf_P_Die		newdie;

	errv = dwarf_attrlist(olddie, &atlist, &atcnt, &error);
	if ((errv != DW_DLV_OK) && (errv != DW_DLV_NO_ENTRY)) {
		printf ("convert_subrange: cannot get attr list\n");
		exit(1);
	}
	newdie = dwarf_new_die(dbg, DW_TAG_subrange_type, NULL, NULL, NULL,
								NULL, &error);
	if (newdie == (Dwarf_P_Die)DW_DLV_BADADDR) {
		printf ("convert_subrange: dwarf_new_die failed\n");
		exit(1);
	}
	if (debug) {
		printf (
		"subrange: converted die named %s (at %p) to new die %p\n",
			name_of_die(olddie), olddie, newdie);
	}
	if (errv == DW_DLV_NO_ENTRY) {
		/* there were no entries; no list was allocated */
		if (debug) {
			printf ("warning: no attributes on subrange at %lld\n",
				(long long)current_offset(olddie));
		}
		return newdie;
	}

	/* add the attributes */

	/* curiously, we must create the bound before the type or we
	   get a bad type */
	svalue = -99;
	for (i=0; i<atcnt; i++) {
		old_attr = atlist[i];
		errv = dwarf_whatattr(old_attr, &attr_val, &error);
		if (errv != DW_DLV_OK) {
			printf ("convert_to_new: dwarf_whatattr error\n");
			exit(1);
		}
		switch (attr_val) {
		case DW_AT_type:
		{
			break;
		}
		case DW_AT_upper_bound:
		{
			errv = dwarf_formudata(old_attr, &uvalue, &error);
			if (errv == DW_DLV_ERROR) {
				printf(
				  "convert_subrange: dwarf_formudata error\n");
				exit(1);
			}
			svalue = uvalue;
			break;
		}
		default:
		{
			if (debug) {
				printf ("die:%s: ", name_of_die(olddie));
				printf ("ignoring attribute %d\n", attr_val);
			}
		}
		} /* end of switch */
	}
	if (svalue != -99) {
		new_attr = dwarf_add_AT_signed_const(newdbg, newdie,
				DW_AT_upper_bound, svalue, &error);
		if (new_attr == (Dwarf_P_Attribute)DW_DLV_BADADDR) {
			printf ("convert_subrange: add val failed\n");
			exit(1);
		}
	}
	/* owner die, ... other die -- make it point to a common base
	   type (unsigned int) */
	aresult = dwarf_add_AT_reference(newdbg, newdie,
				DW_AT_type, common_base_type_die, &error);
	if (aresult == (Dwarf_P_Attribute)DW_DLV_BADADDR) {
		printf ("convert_subrange: add ref failed\n");
		exit(1);
	}

	/* now remove the temporary list describing the old one */
	for (i=0; i<atcnt; i++) {
		dwarf_dealloc(olddbg, atlist[i], DW_DLA_ATTR);
	}
	dwarf_dealloc(olddbg, atlist, DW_DLA_LIST);
	total_newdies++;
	return newdie;
}

/*
 * make a Dwarf_P_Die which is an unnamed pointer to a type represented
 * by typedie
 */
Dwarf_P_Die
make_pointer_die(Dwarf_P_Die typedie)
{
	Dwarf_P_Die		newdie, producer_p_die;
	Dwarf_Error		error;
	Dwarf_P_Attribute	new_attr, aresult;

	newdie = dwarf_new_die(newdbg, DW_TAG_pointer_type,NULL,NULL,NULL,
								NULL, &error);
	if (newdie == (Dwarf_P_Die)DW_DLV_BADADDR) {
		printf ("make_pointer_die: dwarf_new_die failed\n");
		exit(1);
	}
	producer_p_die = dwarf_die_link(newdie, root_p_die, NULL, NULL, NULL,
								&error);
	if (producer_p_die == (Dwarf_P_Die)DW_DLV_BADADDR) {
		printf ("make_pointer_die: dwarf_die_link failed\n");
		exit(1);
	}

	/* the new die has just 2 attributes: AT_bytesize of 8
	   and the offset to the typedie (AT_type) */
	   /* don't add add a reference attribute to a void *  pointer */
	new_attr = dwarf_add_AT_unsigned_const(newdbg, newdie,
					DW_AT_byte_size, PTRSIZE, &error);
	if (new_attr == (Dwarf_P_Attribute)DW_DLV_BADADDR) {
			printf ("make_pointer_die: add val failed\n");
			exit(1);
	}

	/* curious: if you move this dwarf_add_AT_reference ahead of the
	   one above, it fails; add the constant before the type */
	if (typedie) {
		aresult = dwarf_add_AT_reference(newdbg, newdie, DW_AT_type,
							typedie, &error);
		if (aresult == (Dwarf_P_Attribute)DW_DLV_BADADDR) {
			printf (
			  "make_pointer_die: dwarf_add_AT_reference failed\n");
			exit(1);
		}
	}
	return newdie;
}

/*
 * make a Dwarf_P_Die which is an unnamed array link to a type represented
 * by typedie
 */
Dwarf_P_Die
make_array_die(Dwarf_P_Die typedie)
{
	Dwarf_P_Die		newdie, producer_p_die;
	Dwarf_Error		error;
	Dwarf_P_Attribute	new_attr, aresult;

	newdie = dwarf_new_die(newdbg, DW_TAG_array_type,NULL,NULL,NULL,
								NULL, &error);
	if (newdie == (Dwarf_P_Die)DW_DLV_BADADDR) {
		printf ( "make_array_die: dwarf_new_die failed\n");
		exit(1);
	}
	producer_p_die = dwarf_die_link(newdie, root_p_die, NULL, NULL, NULL,
								&error);
	if (producer_p_die == (Dwarf_P_Die)DW_DLV_BADADDR) {
		printf ("make_array_die: dwarf_die_link failed\n");
		exit(1);
	}

	/* the new die has just 2 attributes: AT_bytesize of 8
	   and the offset to the typedie (AT_type) */
	aresult = dwarf_add_AT_reference(newdbg, newdie, DW_AT_type,
							typedie, &error);
	if (aresult == (Dwarf_P_Attribute)DW_DLV_BADADDR) {
		printf ("make_array_die: dwarf_add_AT_reference failed\n");
		exit(1);
	}
	new_attr = dwarf_add_AT_unsigned_const(newdbg, newdie,
					DW_AT_byte_size, PTRSIZE, &error);
	if (new_attr == (Dwarf_P_Attribute)DW_DLV_BADADDR) {
		printf ("make_array_die: dwarf_add_AT_unsigned_const failed\n");
		exit(1);
	}
	return newdie;
}

/*
 * make an attribute in "from" to reference "to"
 */
void
link_to_pointer(Dwarf_P_Die from, Dwarf_P_Die to)
{
	Dwarf_Error		error;
	Dwarf_P_Attribute	aresult;

	aresult = dwarf_add_AT_reference(newdbg, from, DW_AT_type, to,
								&error);
	if (aresult == (Dwarf_P_Attribute)DW_DLV_BADADDR) {
		printf ("link_to_pointer: dwarf_add_AT_reference failed\n");
		exit(1);
	}
	return;
}

/*
 * Given a file which we know is an elf file, display the dwarf data.
 */
void
trace_file(Elf * elf, int fd)
{
	int			nres = DW_DLV_OK;
	Dwarf_Debug		dbg;
	Dwarf_Error		error; /* a structure */
	Dwarf_Unsigned		cu_header_length=0, abbrev_offset=0;
	Dwarf_Half		version_stamp=0, address_size=0;
	Dwarf_Die		cu_die = 0;
	Dwarf_Unsigned		next_cu_offset = 0, dres;

	dres = dwarf_elf_init(elf, DW_DLC_READ, NULL, NULL, &dbg, &error);
	if (dres == DW_DLV_NO_ENTRY) {
		printf("No DWARF information present in %s\n", in_file_name);
		return;
	}
	if (dres != DW_DLV_OK) {
		printf ("dwarf_elf_init failed\n");
		exit(1);
	}

	/*
 	 * Iterate through dwarf and display all type info.
 	 */
	if (debug) {
		printf ("walking the CUs tree\n");
	}

	/* Loop until it fails.  */
	while ((nres =
		dwarf_next_cu_header(dbg, &cu_header_length, &version_stamp,
		&abbrev_offset, &address_size, &next_cu_offset, &error))
	   					== DW_DLV_OK) {
		total_cus++;
		if (total_cus == 1) {

#ifdef OPAQUE_RELOCATION			
			if (dbg->de_debug_info == NULL) {
				printf ("de_debug_info is null; abort\n");
				exit(1);
			}

			if (file_is_relocatable(fd)) {
				if (verbose) {
					printf ("file is relocatable\n");
				}
				unsigned char *end =
				  dbg->de_debug_info+dbg->de_debug_info_size;
				reloc_debug_info (dbg->de_debug_info, end, fd);
			}
#endif /* OPAQUE_RELOCATION */

		}
		if (tflag || Tflag) {
			printf ("Compile Unit %d\n", total_cus);
			printf ("  CU: abbrev offset:  %lld\n", (long long)abbrev_offset);
			printf ("  CU: address size:   %d\n", address_size);
			printf ("  CU: next cu offset: %lld\n", (long long)next_cu_offset);
		}
		if (debug) {
			printf("  CU: header length:  %lld\n",
				(long long)cu_header_length);
			printf("  CU: version stamp:  %d\n", version_stamp);
		}

		tree_level = -1;

		/* get first die of the cu */
		get_sibling(dbg, NULL, &cu_die);
		if (tflag || Tflag) {
			printf ("  CU: name:	   %s\n",
				name_of_die(cu_die));
		}

		/* process a single compilation unit in .debug_info. */
		if (get_sibling(dbg, NULL, &cu_die)) {
			if (debug) {
				printf (
				  "call display_die_and_children (next CU)\n");
			}
			display_die_and_children(dbg, cu_die);
		}
		cu_offset = next_cu_offset;
		if (debug) {
			printf ("bump to next cu_offset\n");
		}
	}
	if (debug) {
		printf ("end of all CUs\n");
	}

	dres = dwarf_finish(dbg, &error);
	if (dres != DW_DLV_OK) {
		printf ("dwarf_finish failed\n");
		exit(1);
	}

	return;
}

/*
 * recursively follow the entire die tree, just for tracing purposes
 */
void
display_die_and_children(Dwarf_Debug dbg, Dwarf_Die in_die)
{
	Dwarf_Die	child, sibling;

	tree_level++;
	for (;;) {   /* do this die and all its siblings */
		if (rflag) {
			test_a_die(dbg, in_die);
		} else {
			if (debug) {
				printf ("call print_a_die for %s\n",
					name_of_die(in_die));
			}
			print_a_die(dbg, in_die);
		}

		if (get_child(in_die, &child)) {
			/* child first: we are doing depth-first walk */
			display_die_and_children(dbg, child);
		}

		/* check whether there are any siblings */
		if (get_sibling(dbg, in_die, &sibling)) {
			/* we have a sibling, loop again to process it. */
			in_die = sibling; /* sibling becomes the current */
		} else {
			/* We are done, no more siblings at this level. */
			break;
		}
	}				/* end for loop on siblings */
	tree_level--;

	return;
}

/*
 * return 0 or 1 for not/is a die that we want to convert to our new
 * p_die tree
 *
 * side-effect: detect structure declarations; add them to the struct_protop
 * list and return 1 in the callers "declaration" field
 */
int
is_type_we_want(Dwarf_Debug dbg, Dwarf_Die die, int *declaration)
{
	int		result;
	char		*die_name;
	Dwarf_Error	dwarf_err;
	Dwarf_Half	tag;
	Dwarf_Bool	tf;
	Dwarf_Off	offset;

	*declaration = 0;
	get_tag(die, &tag);
	switch (tag) {
	case DW_TAG_structure_type:
	{
		result = dwarf_hasattr(die, DW_AT_declaration, &tf, &dwarf_err);
		if (result != DW_DLV_OK) {
			printf ("is_type_we_want: dwarf_hasattr failed\n");
			exit(1);
		} else {
			if (tf) {
			/* This is just an empty structure prototype so save
			   its offset and name.
			   At the end of the run go back and examine these.
			   If it is not defined as an actual type, we let
			   everything pointing to it continue to do so.
 			   If the real structure is defined, change all
			   references to this offset to the full type's. */
				offset = current_offset(die);
				die_name = name_of_die(die);
				add_to_proto_list(offset,
					offset*FMAX + current_file_number,
					die_name);
				*declaration = 1;
				/* Fall through and select and create this one
				   even tho' we may end up with no others
				   pointing to it.  We won't know until later */
			}
		}
	}
	case DW_TAG_base_type:
	case DW_TAG_enumeration_type:
	case DW_TAG_member:
	case DW_TAG_typedef:
	case DW_TAG_enumerator:
	case DW_TAG_union_type:
	case DW_TAG_pointer_type:
	case DW_TAG_array_type:
	case DW_TAG_const_type:
	case DW_TAG_volatile_type:
	case DW_TAG_subroutine_type:
	/* need to keep and convert array of pointer */
	case DW_TAG_subrange_type:
	/* keep the subprogram to maintain parent/child relationships */
	case DW_TAG_subprogram:

	/* we don't need these:
	if they are referenced we will get a translation error message
	case DW_TAG_variable:
	case DW_TAG_lexical_block:
	*/
		return 1;
	default:
	{
		if (debug) {
			printf (
		   "is_type_we_want: die %p named %s, type %d rejected\n",
				die, name_of_die(die), tag);
		}
		return 0;
	}
	}
}

int
is_a_struct(Dwarf_Die die)
{
	Dwarf_Half	tag;

	get_tag(die, &tag);
	if (tag == DW_TAG_structure_type) {
		return 1;
	} else {
		return 0;
	}
}

int
is_a_group(Dwarf_Die die)
{
	Dwarf_Half	tag;

	/* typedef added 9/2005 */
	get_tag(die, &tag);
	if (tag == DW_TAG_structure_type   ||
	    tag == DW_TAG_enumeration_type ||
	    tag == DW_TAG_typedef	   ||
	    tag == DW_TAG_union_type)      {
		return 1;
	} else {
		return 0;
	}
}

/*
 * get the type (tag) of a die
 */
void
get_tag(Dwarf_Die die, Dwarf_Half *tag)
{
	int		result;
	Dwarf_Error	dwarf_err;

	result = dwarf_tag(die, tag, &dwarf_err);
	if (result != DW_DLV_OK) {
		printf ("get_tag: dwarf_tag failed\n");
		exit(1);
	}
	return;
}

/*
 * test the die for a type attribute that directly references a structure,
 * union, typdef, enumeration or array
 *
 * if it does, return 1 and the address of that die
 * else return 0
 */
int
has_attr_group(Dwarf_Debug dbg, Dwarf_Die die, Dwarf_Die *type_die)
{
	Dwarf_Attribute	attr;
	Dwarf_Half	tag;
	Dwarf_Off	offset;

	if (get_attr(die, DW_AT_type, &attr)) {
		/* it has a type attribute */
		get_refoffset(attr, &offset);
		/* get the die of the type (and pass to caller) */
		get_die(dbg, offset, type_die);
		get_tag(*type_die, &tag);
		if (tag == DW_TAG_structure_type   ||
		    tag == DW_TAG_enumeration_type ||
		    tag == DW_TAG_typedef	  ||
		    tag == DW_TAG_array_type	   ||
		    tag == DW_TAG_union_type)      {
			return 1;
		}
	}
	return 0;
}

void
list_attributes(Dwarf_Debug dbg, Dwarf_Die die)
{
	int		errv, i;
	Dwarf_Signed	atcnt;
	Dwarf_Attribute	*atlist, attr;
	Dwarf_Error	error;
	Dwarf_Half	attr_val;

	errv = dwarf_attrlist(die, &atlist, &atcnt, &error);
	if (errv == DW_DLV_NO_ENTRY) {
		/* there were no entries; no list was allocated */
		return;
	}
	if (errv != DW_DLV_OK) {
		printf ("cannot get attr list in list_attributes\n");
		exit(1);
	}

	/* with -T just put the type attribute on the same line as the
 	   basic information about the die */
	if (!Tflag && atcnt > 0) {
		/* assume it's still set up by print_a_die */
		printf ("  %sattributes: ", indentation_string);
	}
	/* walk thru the attributes */
	for (i=0; i<atcnt; i++) {
		attr = atlist[i];
		errv = dwarf_whatattr(attr, &attr_val, &error);
		if (errv != DW_DLV_OK) {
			printf ("dwarf_whatattr error\n");
			exit(1);
		}
		print_reference(dbg, attr, attr_val);
	}
	if (atcnt > 0) {
		printf ("\n");
	}

	/* now remove the temporary list describing the old one */
	for (i=0; i<atcnt; i++) {
		dwarf_dealloc(dbg, atlist[i], DW_DLA_ATTR);
	}
	dwarf_dealloc(dbg, atlist, DW_DLA_LIST);

	return;
}

/*
 * print any details of this attribute
 */
void
print_reference(Dwarf_Debug dbg, Dwarf_Attribute attr,
		Dwarf_Half attr_val)
{
	int 		offset;
	Dwarf_Off	globaloffset;

	if (!Tflag) {
		printf ("%#x ", attr_val);
	}

	if (attr_val == DW_AT_sibling || attr_val == DW_AT_type) {
		get_refoffset(attr, &globaloffset);
		if (attr_val == DW_AT_sibling) {
			/*  no need to generate sibling attributes */
		} else {
			if (Tflag) {
				printf ("type:%lld ", (long long)globaloffset);
			} else {
				printf (
				  "DW_AT_type(%#x) ref_offset:%lld <%#llx> ",
					attr_val, (long long)globaloffset, 
                                        (unsigned long long)globaloffset);
			}
		}
	}
	if (attr_val == DW_AT_data_member_location) {
		offset = get_offset(dbg, attr, attr_val);
		printf ("member_offset:%d <%#x> ", offset, offset);
	}
	if (attr_val == DW_AT_location) {
		offset = get_offset(dbg, attr, attr_val);
		printf ("member_location:%d <%#x> ", offset, offset);
	}
	if (attr_val == DW_AT_upper_bound) {
		offset = get_upper_bound(attr);
		printf ("bound:%d <%#x> ", offset, offset);
	}
	return;
}

/*
 * returns all the identifying characteristics of a die
 *  "ref" is the offset of the die that this die points to, if any
 */
void
get_type_values(Dwarf_Debug dbg, Dwarf_Die die, char **namepp,
	unsigned long long int *hashp, int *nummembersp, int *sizep,
	Dwarf_Half *tagp, Dwarf_Off *ref, int *isptr)
{
	int		result, nummembers=1, pos, bound;
	unsigned long long int	hashdiesum, hashnamelen, hashnamesum=0,
			hashtot, hashfilenum=0;
	char		*die_name, *cp, *parent;
	Dwarf_Unsigned	bytesize;
	Dwarf_Die	this_die, sibling_die, type_die, child_die;
	Dwarf_Error	err;
	Dwarf_Half	dietag, type_tag;
	Dwarf_Attribute	attr;
	Dwarf_Off	type_offset, ptroffset=0; /* fill in with offset
					of pointer referenced, if any */
	Dwarf_Off	orig_offset;

	die_name = name_of_die(die);
	hashnamelen = (int)strlen(die_name);

	/* give the caller the name of the type */
	*namepp = die_name;  /* group leader's name is the type name */

	pos = 0;
	cp = die_name;
	while (*cp != '\0') {
		pos++;
		hashnamesum += (pos * (*cp));
		cp++;
	}
	parent = die_name;

	result = dwarf_bytesize(die, &bytesize, &err);
	if (result == DW_DLV_ERROR) {
		printf("get_type_values: problem reading byte size\n");
		exit(1);
	} else if (result == DW_DLV_NO_ENTRY) {
		bytesize = 0;
	}

	/* give the caller the byte size of the type */
	*sizep = bytesize;

	get_tag(die, &dietag);
	/* give the caller the tag of the type */
	*tagp = dietag;
	hashdiesum = dietag;
	if (dietag  == DW_TAG_array_type ) {
		/* unnamed array descriptors are common; add the bound */
		if (get_child(die, &child_die)) {
			bound = get_bound(child_die);
			if (bound) {
				if (debug)
				    printf ("adding bound %d to hashdiesum\n",
						bound);
				hashdiesum += bound;
			}
		}
	}

	/* make pointers, arrays, volatiles and constants unique to the
	   type they point to */
	if (dietag == DW_TAG_pointer_type  || dietag == DW_TAG_array_type ||
	    dietag == DW_TAG_volatile_type || dietag == DW_TAG_const_type) {
		if (get_attr(die, DW_AT_type, &attr)) {
			get_refoffset(attr, &ptroffset);
			hashnamesum += (ptroffset*
					(NUMCFILES-current_file_number));
			/* and still a collision may occur between files, so
			   put the file number in its own field for these
			   unnamed types */
			hashfilenum = current_file_number;
		}
	} else {
		hashfilenum = 0;
	}
	*isptr = 0;
	if (get_attr(die, DW_AT_type, &attr)) {
		/* this die references another type; check whether
		   this is a pointer (pointer or array) to that type */
		get_refoffset(attr, &type_offset);
		orig_offset = type_offset;
		/* the referenced die: */
		get_die(dbg, type_offset, &type_die);
		get_tag(type_die, &type_tag);
		if (type_tag == DW_TAG_pointer_type) {
			/* we are in some member that points to an
			   unnamed pointer; return the offset of the
			   die that it points to */
			if (!get_attr(type_die, DW_AT_type, &attr)) {
				/* having no type attribute is possible if
				   this is a void * */
				ptroffset = 0 ;
				*isptr = PTRLINK;
			} else {
				get_refoffset(attr, &type_offset);
				/* ptroffset will be passed to the caller */
				ptroffset = type_offset;
				/* pointers to pointers will be
				   handled the same way as other
				   types; only the final pointer to
				   the type will be returned in
				   *isptr as such */
				get_die(dbg, type_offset, &type_die);
				get_tag(type_die, &type_tag);
				/* do not return PTRLINK for pointer to
				   pointer or pointer to array */
				if (type_tag != DW_TAG_pointer_type &&
				    type_tag != DW_TAG_array_type) {
					/* setting this causes this pointer
					   to be handled specially by creating
					   the die at the end of the run;
					   see do_reference_translations */
					*isptr = PTRLINK;
				} else {
					ptroffset = orig_offset;
					goto countchildren;
				}
			}
		}
		if (type_tag == DW_TAG_array_type) {
			/* this thing points to an unnamed array link; return
			   the offset of the die the pointer points to */
			if (!get_attr(type_die, DW_AT_type, &attr)) {
				printf ("no type attribute on array; abort\n");
				exit(1);
			}
			get_refoffset(attr, &type_offset);
			/* ptroffset will be passed to the caller */
			ptroffset = type_offset;
			/* arrays of arrays will be
			   handled the same way as other
			   types; only the final array link to
			   the type will be returned in
			   *isptr as such */
			get_die(dbg, type_offset, &type_die);
			get_tag(type_die, &type_tag);
			/* do not return ARRAYLINK for array of
			   array or array of pointer */
			if (type_tag != DW_TAG_pointer_type &&
			    type_tag != DW_TAG_array_type) {
				/* setting this causes this array link
				   to be handled specially by creating
				   the die at the end of the run */
				*isptr = ARRAYLINK;
			}
		}
	}

countchildren:
	/* count the children: */
	if (get_child(die, &sibling_die)) {
		do {
			this_die = sibling_die;
			nummembers++;
			die_name = name_of_die(this_die);
			pos = 0;
			cp = die_name;
			while (*cp != '\0') {
				pos++;
				hashnamesum += (pos * (*cp));
				cp++;
			}
			hashnamelen += (int)strlen(die_name);
			get_tag(this_die, &dietag);
			hashdiesum += dietag;
		} while (get_sibling(dbg, this_die, &sibling_die));
	}
	/* 24 bits: 16M
	   18 bits: 256K
	   11 bits: 2048
	   namesum  200 names * 100 bytes each * 256 -> 5M	23 bits
	   namelen  200 names * 100 bytes each -> 20K  		15 bits
	   diesum   200 dies *  256(or much more) -> 50K 	16 bits
 	   filenum  just over 1000 for PP4			11 bits
	*/

	/* give the caller the hash total and number of members */
	/* give them    11      16   15    22    bits each */
	/* shifts:      53      37   22     0		   */
	/*            filenum  die namelen namesum */
	hashtot = hashfilenum << 53 | hashdiesum << 37 |
		  hashnamelen << 22 | hashnamesum;
#ifdef TEST
	/* this has no positive effect: */
	/* may want to hash on a non-ascending values: */
	/* give them    22      15   16    11    bits each */
	/* shifts:      42      27   11     0		   */
	/*           namesum namelen die filenum	   */
	hashtot = hashnamesum << 42 | hashnamelen << 27 |
		  hashdiesum  << 11 | hashfilenum;
	}
#endif

	*hashp = hashtot;
	*nummembersp = nummembers;
	/* give the pointer offset to the caller */
	*ref = ptroffset;
	return;
}

/*
 * return a requested attribute
 * return 0 if not present
 */
int
get_attr(Dwarf_Die die, Dwarf_Half attr_type, Dwarf_Attribute *attr)
{
	int		result;
	Dwarf_Error	err;
	Dwarf_Bool	tf;

	result = dwarf_hasattr(die, attr_type, &tf, &err);
	if (result == DW_DLV_ERROR) {
		printf ("get_attr: dwarf_hasattr failed\n");
		exit(1);
	}
	if (!tf) {
		return 0;
	}
	result = dwarf_attr(die, attr_type, attr, &err);
	if (result == DW_DLV_ERROR) {
		printf("get_attr: dwarf_attr error\n");
		exit(1);
	}
	if (result == DW_DLV_NO_ENTRY) {
		return 0;
	}
	return 1;
}

/*
 * return an offset from a type attribute (of known type)
 */
void
get_refoffset(Dwarf_Attribute attr, Dwarf_Off *offsetp)
{
	int		result;
	Dwarf_Error	err;

	result = dwarf_global_formref(attr, offsetp, &err);
	if (result != DW_DLV_OK) {
		printf("getref_offset: dwarf_global_fromref error\n");
		printf("  result: %d\n", result);
		exit(1);
	}
	return;
}

/*
 * get the value of attribute DW_AT_upper_bound
 * (upper_bound is the highest subscript of the array e.g. 3 for [4])
 */
int
get_upper_bound(Dwarf_Attribute attr)
{
	int		retval, result;
	Dwarf_Unsigned	bound;
	Dwarf_Error	error;

	result = dwarf_formudata(attr, &bound, &error);
	if (result == DW_DLV_ERROR) {
		printf("get_upper_bound: dwarf_formudata error\n");
		exit(1);
	}
	retval = bound;
	return retval;
}


/*
 * return the die, given an offset into a debug
 */
void
get_die(Dwarf_Debug dbg, Dwarf_Off offset, Dwarf_Die *diep)
{
	int		result;
	Dwarf_Error	err;

	result = dwarf_offdie(dbg, offset, diep, &err);
	if (result == DW_DLV_ERROR || result == DW_DLV_NO_ENTRY) {
		printf("get_die: dwarf_offdie error\n");
		exit(1);
	}
	return;
}

/*
 * get the first child of a die, return 1
 * if no children, return 0
 */
int
get_child(Dwarf_Die die, Dwarf_Die *child)
{
	int		result;
	Dwarf_Error	error;

	result = dwarf_child(die, child, &error);
	if (result == DW_DLV_ERROR) {
		printf("get_child: dwarf_child failed\n");
		exit(1);
	}
	if (result == DW_DLV_OK) {
		return 1;
	}
	return 0;
}

/*
 * get the first sibling of a die, return 1
 * if no sibling, return 0
 */
int
get_sibling(Dwarf_Debug dbg, Dwarf_Die die, Dwarf_Die *sibling)
{
	int		result;
	Dwarf_Error	err;

	result = dwarf_siblingof(dbg, die, sibling, &err);
	if (result == DW_DLV_ERROR) {
		//__asm__("int $3");
		printf ("get_sibling: dwarf_siblingof failed: %s\n",
				dwarf_errmsg(err));
		exit(1);
	}
	if (result == DW_DLV_OK) {
		return 1;
	}
	return 0;
}

/*
 * return the number of children of this die
 */
int
child_count(Dwarf_Debug dbg, Dwarf_Die die)
{
	int		nummembers=0;
	Dwarf_Die	this_die, child_die, sibling_die;
	Dwarf_Half	dietag;

	if (get_child(die, &child_die)) {
		this_die = child_die;
		nummembers++;
		while (get_sibling(dbg, this_die, &sibling_die)) {
			nummembers++;
			get_tag(sibling_die, &dietag);
			this_die = sibling_die;
		}
	}
	return nummembers;
}

/*
 * from lcrash
 */
int decode_unsigned_leb128 (unsigned char *data, int *soffset)
{
	Dwarf_Unsigned	result=0;
	unsigned char	byte;
	int		bytes_read=0, shift=0;

	do
	{
		byte = *data++;
		bytes_read++;
		/* For every byte read (from lower order),
		   get the lower 7 bits and multiply by 128^x
		   where x is 0,1,2...  */
		result |= (byte & 0x7f) << shift;
		shift += 7;
	} while (byte & 0x80); /* Loop till higher order byte */

	*soffset = result;
	return bytes_read;
}

/*
 * this is how lcrash gets an offset of out of DW_AT_frame_base
 */
int
get_offset(Dwarf_Debug dbg, Dwarf_Attribute attr, Dwarf_Half attr_val)
{
	int		offset, result;
	unsigned char	*start;
	Dwarf_Half	formtype;
	Dwarf_Unsigned	value;
	Dwarf_Error	error;
	Dwarf_Block 	*ptr_blk;

	result = dwarf_whatform(attr, &formtype, &error);
	if(result == DW_DLV_ERROR) {
		printf("get_offset: dwarf_whatform error\n");
		exit(1);
       	}
	if (formtype == DW_FORM_block1 || formtype == DW_FORM_block2 ||
	    formtype == DW_FORM_block4 || formtype == DW_FORM_block)   {
		result = dwarf_formblock(attr, &ptr_blk, &error);
		if(result == DW_DLV_ERROR) {
			printf(
		    	"get_offset: dwarf_formblock error\n");
			exit(1);
		}
       		start  = (unsigned char*) (ptr_blk->bl_data);
		/* point to the actual data by skipping the length */
		switch(formtype) {
			case DW_FORM_block1: { start +=1; break; }
			case DW_FORM_block2: { start +=2; break; }
			case DW_FORM_block4: { start +=4; break; }
			case DW_FORM_block :
	                  { start += decode_unsigned_leb128(start, &offset); }
		}
		/* Get the offset into offset */
		decode_unsigned_leb128(start, &offset);
		dwarf_dealloc(dbg, ptr_blk, DW_DLA_BLOCK);
		if (debug) {
			if (attr_val == DW_AT_data_member_location) {
			 printf ("DW_AT_data_member_location offset:%d <%#x>\n",
				offset, offset);
			}
		}
	} else {
		result = dwarf_formudata(attr, &value, &error);
		if (result == DW_DLV_ERROR) {
			printf ( "get_offset: dwarf_formudata error\n");
			exit(1);
		}
		offset = value;
	}

	return offset;
}

/*
 * Do the -c thing: concatenate the types in in_file_name and those
 * in cfilep[], producing out_file_name
 * The input file could have either one or multiple CU's
 */
void
concatenate (Elf *elf, int infd, int outfd)
{
	int		dres, filenumber, concat_fd;
	char		*filename;
	Dwarf_Debug	dbg;
	Dwarf_Error	error; /* a structure */
	Elf		*concat_elf;

	dres = dwarf_elf_init(elf, DW_DLC_READ, NULL, NULL, &dbg, &error);
	if (dres == DW_DLV_NO_ENTRY) {
		printf("No DWARF information present in %s\n", in_file_name);
		return;
	}
	if (dres != DW_DLV_OK) {
		printf ("dwarf_elf_init failed\n");
		exit(1);
	}

	dres = dwarf_producer_init(producer_flags, 
            producer_callback_c,
	    producer_errhandler, 
            producer_error, 
            producer_user_data,
            producer_isa,
            producer_version,
            0,
            &newdbg,
            &error);
	if (dres != DW_DLV_OK) {
		printf ("dwarf_producer_init failed\n");
		exit(1);
	}
	if (debug) {
		printf ("newdbg created at %p\n", newdbg);
	}

	common_base_type_die = (Dwarf_P_Die)0;
	/*
 	 * Iterate through dwarf of main input file and extract all type info.
 	 */
	if (pflag) {
		printf ("digesting the base types in %s\n",
			in_file_name);
	}
	if (debug) {
		printf ("walking the CUs tree of %s\n", in_file_name);
	}

	walk_cus(dbg, infd);
	if (debug) {
		printf ("total new dies created %d\n", total_newdies);
	}

	/* do not release this dbg,  we need the name for comparison
		during reference translations!
	dres = dwarf_finish(dbg, &error);	*/
	if (dres != DW_DLV_OK) {
		printf ("dwarf_finish failed\n");
		exit(1);
	}

	if (!sflag) {
		printf ("input file:\t\t\t\t%s\n", in_file_name);
		printf ("compilation units:\t\t%d\n", total_cus);
		printf ("structures captured:\t\t%d\n", total_structs);
	}

	for (filenumber=0; filenumber<num_cfiles; filenumber++) {
		current_file_number++; /* used to offset type references to
						make them unique by file */
		filename = cfilep[filenumber];
		if ((concat_fd = open(filename, O_RDONLY)) < 0) {
			printf ("cannot open %s\n", filename);
			perror("Open failed");
			exit(1);
		}
		concat_elf = open_as_elf(concat_fd, filename);
		needs_old=0;
		process_concat_file(concat_elf, filename, concat_fd, filenumber);
		if (needs_old == 0) {
			/* leave the files open because we need the dbg
			   in place for needarr_dbgp use; unless this file
			   had none of them */
			elf_end(concat_elf); close (concat_fd);
		}
	}

	if (pflag) {
		printf ("beginning the reference translations\n");
	}
	do_reference_translations();

	get_strings(infd, 1); /* get strings from input file and adjust */

	if (in_place) {
		/* the input file is also the output file */
		close (infd);
		if ((outfd = open(in_file_name,
					O_RDWR|O_CREAT|O_TRUNC, 0600)) < 0) {
			printf ("%s: overwrite of %s failed\n",
				program_name, in_file_name);
			exit(1);
		}
	}

	write_output_file(outfd);
	if (!sflag) {
		printf ("output file:\t\t\t\t%s\n",
			in_place ? in_file_name : out_file_name);
	}

	return;
}

/* get the string section from the input file,
 * and optionally adjust it with any strings we need to add;
 * sets global in_string_buffer (and other globals) and returns its address
 */
char *
get_strings(int infd, int adjust)
{
	int		i;
	SECTION_HEADER	*string_shp;

	string_shp = &base_shp[inelfhdr.e_shstrndx];
	string_idx     = string_shp->sh_name;
	in_string_size = string_shp->sh_size;
	if (adjust) {
		reldebug_strlen = (int)strlen(reldebug_string) + 1;
							/* include the \0 */
	} else {
		reldebug_strlen = 0;
	}
	in_string_buffer = (char *)malloc(in_string_size + reldebug_strlen);
	if (debug) {
		printf ("allocated %d bytes for string section\n",
			in_string_size);
	}
	if (tflag) {
		printf ("allocated %d bytes for string section\n",
			in_string_size + reldebug_strlen);
	}
	if ((i = lseek(infd, string_shp->sh_offset, 0)) < 0) {
		fprintf(stderr,"%s: cannot seek to %#lx\n",
			program_name, (unsigned long)string_shp->sh_offset);
		exit(1);
	}
	if (tflag) {
		printf ("seek to %#lx for string section okay\n",
			(unsigned long)string_shp->sh_offset);
		printf ("will read %d bytes of string section\n",
			in_string_size);
	}
	if ((i=read(infd,in_string_buffer,in_string_size)) != in_string_size) {
		printf ("cannot read string section; abort\n");
		exit(1);
	}
	out_string_size = in_string_size;

	/*   we may tack stuff onto it, make a bit bigger */
	if (adjust && no_input_rel_debug_info_section) {
		/* we didn't find a .rel.debug_info section, so describe
		   it in image of the strings section (tack it on) */
		strcpy(in_string_buffer+in_string_size, reldebug_string);
		reldebug_idx = in_string_size; /* position of this new string */
		out_string_size = in_string_size + reldebug_strlen;
	}
	return in_string_buffer;
}

/*
 * get the string section from some input file; nothing fancy
 */
char *
get_strings_basic(int fd, SECTION_HEADER *base, int index)
{
	int		i, size;
	char		*buffer;
	SECTION_HEADER	*string_shp;

	string_shp = &base[index];
	size = string_shp->sh_size;
	buffer = (char *)malloc(size);
	if ((i = lseek(fd, string_shp->sh_offset, 0)) < 0) {
		fprintf(stderr,"%s: cannot seek to %#lx\n",
			program_name, (unsigned long)string_shp->sh_offset);
		exit(1);
	}
	if ((i = read(fd,buffer,size)) != size) {
		printf ("cannot read string section; abort\n");
		exit(1);
	}
	return buffer;
}

/*
 * get the section header table
 */
int
get_debug_sectionhead(int fd, SECTION_HEADER **firstp,
		SECTION_HEADER **shpp, int off, int num, int size, int index)
{
	int		i, fnd;
	char		*strp;
	SECTION_HEADER	*shp, *first;

	first = (SECTION_HEADER *)malloc(num * size);
	if ((i = lseek(fd, off, 0)) < 0) {
		fprintf(stderr,"%s: cannot seek to %d\n", program_name, off);
		exit(1);
	}
	if ((i = read(fd, first, num*size)) != num*size) {
		printf ("cannot read section header table; abort\n");
		exit(1);
	}
	strp = get_strings_basic(fd, first, index);

	/* find the .debug_info section */
	fnd = 0;
	for (i=0, shp=first; i<num; i++, shp++) {
		if (!strcmp(strp+shp->sh_name, ".debug_info")) {
			fnd++;
			break;
		}
	}
	free (strp);
	if (fnd == 0) {
		return 1;
	}
	*shpp = shp;
	*firstp = first; /* leave it allocated, but tell the caller where */
	return 0;
}

/*
 * Determine the endian-ness of the file
 */
void
test_endianness(ELF_HEADER *elfp)
{
	switch (elfp->e_ident[EI_DATA]) {
	default: /* fall through */
	case ELFDATANONE: /* fall through */
	case ELFDATA2LSB:
		byte_get = byte_get_little_endian;
		byte_put = byte_put_little_endian;
		break;
	case ELFDATA2MSB:
		byte_get = byte_get_big_endian;
		byte_put = byte_put_big_endian;
		break;
	}
	return;
}

#ifdef OPAQUE_RELOCATION
/*
 * Do all the relocations in a file.  Strangely enough, even the
 * debug types have to be relocated in a relocatable file
 */
int
reloc_debug_info (unsigned char *start, unsigned char *end, int fd)
{
	int				j, offset_size;
	int				initial_length_size;
	int				string_index, num_sections;
	long				cu_offset;
	unsigned char			*section_begin, *hdrptr;
	DWARF2_Internal_CompUnit	compunit;
	ELF_HEADER			workelfhdr;
	SECTION_HEADER			*section, *first;

	read_elf_header (&workelfhdr, fd);
	string_index = workelfhdr.e_shstrndx;
	num_sections = workelfhdr.e_shnum;
	test_endianness(&workelfhdr); /* set up byte_get/byte_put */
	if (get_debug_sectionhead(fd, &first, &section, workelfhdr.e_shoff,
		num_sections, workelfhdr.e_shentsize, string_index)) {
		printf ("cannot find section header for debug info; abort\n");
		exit(1);
	} /* first is the base of this file's section header table */
	section_begin = start;

	/* relocate each CU separately (so as not to touch the CU header) */
	while (start < end) {
		hdrptr = start;
		compunit.cu_length = byte_get (hdrptr, 4);
		hdrptr += 4;

		if (compunit.cu_length == 0xffffffff) {
			compunit.cu_length = byte_get (hdrptr, 8);
			hdrptr += 8;
			offset_size = 8;
			initial_length_size = 12;
		} else {
			offset_size = 4;
			initial_length_size = 4;
		}
		compunit.cu_version = byte_get (hdrptr, 2);
		hdrptr += 2;
		cu_offset = start - section_begin;
		/* "start" becomes the pointer to the end of the CU */
		start += compunit.cu_length + initial_length_size;
		if (debug) {
			printf ("calling debug_apply_rela_addends\n");
		}
	  	j = debug_apply_rela_addends (fd, section, first, offset_size,
						section_begin, hdrptr, start,
						string_index, num_sections);
		if (!j) {
			printf ("debug_apply_rela_addends failed\n");
			exit(1);
		}
	}
	free (first);
	return 0;
}

/*
 * Apply addends of RELA relocations.
 */

int
debug_apply_rela_addends (int fd, SECTION_HEADER *section,
			  SECTION_HEADER *base, int reloc_size,
			  unsigned char *sec_data, unsigned char *start,
			  unsigned char *end, int string_index,
			  int num_sections)
{
	unsigned long		nrelas;
	unsigned char		*loc;
	char			*strp;
	SECTION_HEADER		*relsec;
	Elf_Internal_Rela	*rela, *rp;

	if (end - start < reloc_size) {
		return 1;
	}
	strp = get_strings_basic(fd, base, string_index);

	/* find the relocation segment */
	for (relsec = base; relsec < base + num_sections; ++relsec) {

		if (relsec->sh_type != SHT_RELA
			|| strcmp(strp+relsec->sh_name, ".rela.debug_info")
			|| relsec->sh_size == 0) {
			continue;
		}

		if (debug) {
			printf ("calling slurp_rela_relocs\n");
		}
		if (!slurp_rela_relocs(fd, relsec->sh_offset,
			relsec->sh_size, &rela, &nrelas)) {
			return 0;
		}

		for (rp = rela; rp < rela + nrelas; ++rp) {

			if (rp->r_offset >= (bfd_vma) (start - sec_data) &&
			    rp->r_offset <
				(bfd_vma) (end - sec_data) - reloc_size) {
				loc = sec_data + rp->r_offset;
			} else {
				continue;
			}
			byte_put (loc, rp->r_addend, reloc_size);
		}

		free (rela);
		free (strp);
		break;
	}
	return 1;
}
#endif /* OPAQUE_RELOCATION */

bfd_vma
byte_get_big_endian (unsigned char *field, int size)
{
	switch (size) {
	case 1:
		return *field;
	case 2:
		return ((unsigned int) (field[1])) | (((int) (field[0])) << 8);
	case 4:
		return ((unsigned long) (field[3]))
			| (((unsigned long) (field[2])) << 8)
			| (((unsigned long) (field[1])) << 16)
			| (((unsigned long) (field[0])) << 24);
#ifndef BFD64
	case 8:
	/* Although we are extracing data from an 8 byte wide field,
	   we are returning only 4 bytes of data.  */
		return ((unsigned long) (field[7]))
			| (((unsigned long) (field[6])) << 8)
			| (((unsigned long) (field[5])) << 16)
			| (((unsigned long) (field[4])) << 24);
#else
	case 8:
	case -8:
	/* This is a special case, generated by the BYTE_GET8 macro.
	   It means that we are loading an 8 byte value from a field
	   in an external structure into an 8 byte value in a field
	   in an internal structure.  */
		return ((bfd_vma) (field[7]))
			| (((bfd_vma) (field[6])) << 8)
			| (((bfd_vma) (field[5])) << 16)
			| (((bfd_vma) (field[4])) << 24)
			| (((bfd_vma) (field[3])) << 32)
			| (((bfd_vma) (field[2])) << 40)
			| (((bfd_vma) (field[1])) << 48)
			| (((bfd_vma) (field[0])) << 56);
#endif
	default:
		printf("Unhandled data length: %d\n", size);
		exit(1);
	}
}

void
byte_put_big_endian (unsigned char *field, bfd_vma value, int size)
{
	switch (size) {
	case 8:
		field[7] = value & 0xff;
		field[6] = (value >> 8) & 0xff;
		field[5] = (value >> 16) & 0xff;
		field[4] = (value >> 24) & 0xff;
		value >>= 16;
		value >>= 16;
		/* Fall through.  */
	case 4:
		field[3] = value & 0xff;
		field[2] = (value >> 8) & 0xff;
		value >>= 16;
		/* Fall through.  */
	case 2:
		field[1] = value & 0xff;
		value >>= 8;
		/* Fall through.  */
	case 1:
		field[0] = value & 0xff;
		break;
	default:
		printf("Unhandled data length: %d\n", size);
		exit (1);
	}
}

bfd_vma
byte_get_little_endian (unsigned char *field, int size)
{
	switch (size) {
	case 1:
		return *field;
	case 2:
		return  ((unsigned int) (field[0]))
			| (((unsigned int) (field[1])) << 8);
#ifndef BFD64
	case 8:
	/* We want to extract data from an 8 byte wide field and
	   place it into a 4 byte wide field.  Since this is a little
	   endian source we can just use the 4 byte extraction code.  */
	/* Fall through.  */
#endif
	case 4:
		return  ((unsigned long) (field[0]))
			| (((unsigned long) (field[1])) << 8)
			| (((unsigned long) (field[2])) << 16)
			| (((unsigned long) (field[3])) << 24);
#ifdef BFD64
	case 8:
	case -8:
	/* This is a special case, generated by the BYTE_GET8 macro.
	   It means that we are loading an 8 byte value from a field
	   in an external structure into an 8 byte value in a field
	   in an internal structure.  */
		return  ((bfd_vma) (field[0]))
			| (((bfd_vma) (field[1])) << 8)
			| (((bfd_vma) (field[2])) << 16)
			| (((bfd_vma) (field[3])) << 24)
			| (((bfd_vma) (field[4])) << 32)
			| (((bfd_vma) (field[5])) << 40)
			| (((bfd_vma) (field[6])) << 48)
			| (((bfd_vma) (field[7])) << 56);
#endif
	default:
		printf ("Unhandled data length: %d\n", size);
		exit (1);
	}
}

void
byte_put_little_endian (unsigned char *field, bfd_vma value, int size)
{
	switch (size) {
	case 8:
		field[7] = (((value >> 24) >> 24) >> 8) & 0xff;
		field[6] = ((value >> 24) >> 24) & 0xff;
		field[5] = ((value >> 24) >> 16) & 0xff;
		field[4] = ((value >> 24) >> 8) & 0xff;
		/* Fall through.  */
	case 4:
		field[3] = (value >> 24) & 0xff;
		field[2] = (value >> 16) & 0xff;
		/* Fall through.  */
	case 2:
		field[1] = (value >> 8) & 0xff;
		/* Fall through.  */
	case 1:
		field[0] = value & 0xff;
		break;

	default:
		printf ("Unhandled data length: %d\n", size);
		exit(1);
	}
}

#ifdef OPAQUE_RELOCATION
int
slurp_rela_relocs (int fd, unsigned long rel_offset, unsigned long rel_size,
		   Elf_Internal_Rela **relasp, unsigned long *nrelasp)
{
	Elf_Internal_Rela	*relas;
	unsigned long		nrelas;
	unsigned int		i;

#if SUPPORT32
  This part of relocation for 32-bit binaries is not tested;
   the slurp_rela_relocs function never gets called for .o's and .ko's
  if (is_32bit_elf) {
      Elf32_External_Rela *erelas;

      erelas = get_data (NULL, fd, rel_offset, rel_size,);
      if (!erelas)
      relas = malloc (nrelas * sizeof (Elf_Internal_Rela));

      if (relas == NULL)
	{
	  printf("out of memory parsing relocs");
	  return 0;
	}

      for (i = 0; i < nrelas; i++)
	{
	  relas[i].r_offset = BYTE_GET (erelas[i].r_offset);
	  relas[i].r_info   = BYTE_GET (erelas[i].r_info);
	  relas[i].r_addend = BYTE_GET (erelas[i].r_addend);
	}

      free (erelas);
    } else {
#endif
	Elf64_External_Rela *erelas;

	erelas = get_data(NULL, fd, rel_offset, rel_size);
	if (!erelas) {
		return 0;
	}

	nrelas = rel_size / sizeof (Elf64_External_Rela);

	relas = malloc (nrelas * sizeof (Elf_Internal_Rela));

	if (relas == NULL) {
		printf ("out of memory parsing relocs");
		return 0;
	}

	for (i = 0; i < nrelas; i++) {
		relas[i].r_offset = BYTE_GET8 (erelas[i].r_offset);
		relas[i].r_info   = BYTE_GET8 (erelas[i].r_info);
		relas[i].r_addend = BYTE_GET8 (erelas[i].r_addend);
	}

	free (erelas);
#if SUPPORT32
    }
#endif

	*relasp = relas;
	*nrelasp = nrelas;
	return 1;
}
#endif /* OPAQUE_RELOCATION */

void *
get_data (void *var, int fd, long offset, size_t size)
{
	void	*mvar;
	int 	i;

	if (size == 0) {
		return NULL;
	}

	if ((i = lseek(fd, offset, 0)) < 0) {
		printf ("cannot seek to %ld in fd %d\n", (long)offset, fd);
		exit(1);
	}

	mvar = var;
	if (mvar == NULL) {
		mvar = malloc (size);
		if (mvar == NULL) {
			printf("Out of memory allocating %#lx bytes\n", 
                          (unsigned long)size);
			return NULL;
		}
	}

	if ((i = read(fd, mvar, size)) != size) {
		printf("Unable to read in %#lx bytes\n", 
                          (unsigned long)size);
		if (mvar != var) {
			free (mvar);
		}
		return NULL;
	}
	return mvar;
}

/*
 * returns 1 if the file is an elf file marked relocatable
 * else returns 0
 */
int
file_is_relocatable(int fd)
{
	ELF_HEADER	workelfhdr;

	read_elf_header(&workelfhdr, fd);
	if (workelfhdr.e_type == ET_REL) {
		return 1;
	}
	return 0;
}

/*
 * return the caller's die global offset
 */
Dwarf_Off
current_offset(Dwarf_Die die)
{
	int		ores;
	Dwarf_Error	dwarf_err;
	Dwarf_Off	offset;

	ores = dwarf_dieoffset(die, &offset, &dwarf_err);
	if (ores != DW_DLV_OK) {
		printf ("current_offset: dwarf_dieoffset failed\n");
		exit(1);
	}
	return offset;
}

/*
 * sort the references list by offset - a shell sort
 */
void
sort_references()
{
	int		i, j, increment;
	int		array_size=num_refchecks;
	Dwarf_Off	temp, *numbers=ref_refp;
	Dwarf_P_Die	dhold, *dpointers=ref_diep;

	increment = 3;
	while (increment > 0) {
    		for (i=0; i < array_size; i++) {
			j = i;
			temp  = numbers  [i];
			dhold = dpointers[i];
			while ((j >= increment) && (numbers[j-increment] >
								     temp)) {
				numbers  [j] = numbers  [j - increment];
				dpointers[j] = dpointers[j - increment];
				j = j - increment;
			}
			numbers  [j] = temp;
			dpointers[j] = dhold;
		}
		if (increment/2 != 0) {
			increment = increment/2;
		} else if (increment == 1) {
      			increment = 0;
		} else {
			increment = 1;
		}
  	}
}

/*
 * lookup an alias by reference (offset) in the alias tree
 *  return aliasnode or 0 (for found or not)
 */
struct aliasnode *
lookup_alias_ref(Dwarf_Off offset)
{
	struct aliasnode	*workp, worktype;

	worktype.ref = offset;
	/* comparison is done in alias_compare */
	workp = (struct aliasnode *)avl_find(alias_tree, (void *)&worktype);
	if (!workp) {
		return 0;
	} else {
		return workp;
	}
}

/*
 * lookup an alias by reference (offset) in the sorted list
 *  return 1 or 0 (for found or not)
 * array index is passed back in *slot
 */
int
lookup_reference_ref(Dwarf_Off offset, int *slot1, int *slot2)
{
	int		segsize;
	Dwarf_Off	*base, *last, *mid;
	base = ref_refp;

	if (num_refchecks == 0) {
		return 0;
	}
	last = ref_refp + num_refchecks;
		/* last is one beyond the group we are considering */
	segsize = num_refchecks;
	while (1) {
		mid = base + segsize/2;
		if (offset < *mid) {
			last = mid;
			segsize = mid - base;
		} else if (offset > *mid) {
			base = mid+1;
			segsize = last - base;
		} else {
			/* back up to the first match in the list */
			while (mid >= ref_refp && offset == *mid) {
				mid--;
			}
			if (*mid != offset) {
				mid++;
			}
			*slot1 = mid - ref_refp;

			/* scan forward to the last match */
			last = mid;
			while (last < ref_refp+num_refchecks &&
							offset == *last) {
				last++;
			}
			*slot2 = (last-1) - ref_refp;

			return 1;
		}
		if (segsize == 0) {
			return 0;
		}
	}
}

/*
 * lookup a need_pointer by reference (offset) in the sorted list
 *  return 1 or 0 (for found or not)
 * array index is passed back in *slot
 */
int
lookup_needptr_ref(Dwarf_Off offset, int *slot1, int *slot2)
{
	int		segsize;
	Dwarf_Off	*base, *last, *mid;
	base = needptr_refp;

	if (num_needptrs == 0) {
		return 0;
	}
	last = needptr_refp + num_needptrs;
		/* last is one beyond the group we are considering */
	segsize = num_needptrs;
	while (1) {
		mid = base + segsize/2;
		if (offset < *mid) {
			last = mid;
			segsize = mid - base;
		} else if (offset > *mid) {
			base = mid+1;
			segsize = last - base;
		} else {
			/* back up to the first match in the list */
			while (mid >= needptr_refp && offset == *mid) {
				mid--;
			}
			if (*mid != offset) {
				mid++;
			}
			*slot1 = mid - needptr_refp;

			/* scan forward to the last match */
			last = mid;
			while (last < needptr_refp+num_needptrs &&
						offset == *last) {
				last++;
			}
			*slot2 = (last-1) - needptr_refp;

			return 1;
		}
		if (segsize == 0) {
			return 0;
		}
	}
}

/*
 * lookup a need-array by reference (offset) in the sorted list
 *  return 1 or 0 (for found or not)
 * array index is passed back in *slot
 */
int
lookup_needarr_ref(Dwarf_Off offset, int *slot1, int *slot2)
{
	int		segsize;
	Dwarf_Off	*base, *last, *mid;
	base = needarr_refp;

	if (num_needarrs == 0) {
		return 0;
	}
	last = needarr_refp + num_needarrs;
		/* last is one beyond the group we are considering */
	segsize = num_needarrs;
	while (1) {
		mid = base + segsize/2;
		if (offset < *mid) {
			last = mid;
			segsize = mid - base;
		} else if (offset > *mid) {
			base = mid+1;
			segsize = last - base;
		} else {
			/* back up to the first match in the list */
			while (mid >= needarr_refp && offset == *mid) {
				mid--;
			}
			if (*mid != offset) {
				mid++;
			}
			*slot1 = mid - needarr_refp;

			/* scan forward to the last match */
			last = mid;
			while (last < needarr_refp+num_needarrs &&
							offset == *last) {
				last++;
			}
			*slot2 = (last-1) - needarr_refp;

			return 1;
		}
		if (segsize == 0) {
			return 0;
		}
	}
}

/*
 * sort the need-pointer list - a shell sort
 * (don't need to sort the needptr_typep list; it's not filled in yet)
 */
void
sort_needptrs()
{
	int		i, j, increment;
	int		array_size=num_needptrs;
	Dwarf_Off	temp, *numbers=needptr_refp;
	Dwarf_P_Die	dhold, *dpointers=needptr_pdiep;

	increment = 3;
	while (increment > 0) {
    		for (i=0; i < array_size; i++) {
			j = i;
			temp  = numbers  [i];
			dhold = dpointers[i];
			while ((j >= increment) && (numbers[j-increment] >
								     temp)) {
				numbers  [j] = numbers  [j - increment];
				dpointers[j] = dpointers[j - increment];
				j = j - increment;
			}
			numbers  [j] = temp;
			dpointers[j] = dhold;
		}
		if (increment/2 != 0) {
			increment = increment/2;
		} else if (increment == 1) {
      			increment = 0;
		} else {
			increment = 1;
		}
  	}
}

/*
 * sort the need-array list - a  shell sort
 * (don't need to sort the needarr_typep list; it's not filled in yet)
 */
void
sort_needarrs()
{
	int		i, j, increment;
	int		array_size=num_needarrs;
	Dwarf_Off	temp, *numbers=needarr_refp;
	Dwarf_Off	otemp, *onumbers=needarr_origp;
	Dwarf_P_Die	dhold, *dpointers=needarr_pdiep;
	Dwarf_Debug 	ghold, *gpointers=needarr_dbgp;

	increment = 3;
	while (increment > 0) {
    		for (i=0; i < array_size; i++) {
			j = i;
			temp  = numbers  [i];
			otemp = onumbers [i];
			dhold = dpointers[i];
			ghold = gpointers[i];
			while ((j >= increment) && (numbers[j-increment] >
								     temp)) {
				numbers  [j] = numbers  [j - increment];
				onumbers [j] = onumbers [j - increment];
				dpointers[j] = dpointers[j - increment];
				gpointers[j] = gpointers[j - increment];
				j = j - increment;
			}
			numbers  [j] = temp;
			onumbers [j] = otemp;
			dpointers[j] = dhold;
			gpointers[j] = ghold;
		}
		if (increment/2 != 0) {
			increment = increment/2;
		} else if (increment == 1) {
      			increment = 0;
		} else {
			increment = 1;
		}
  	}
}

/*
 * make a list of all the names in the types tree (a hash total of the name)
 */
void
make_types_name_ref_hash()
{
	if ((tree_namehashp = (int *)malloc(total_types * sizeof(int))) ==
								NULL) {
		printf ("cannot allocate space for tree name hash\n");
		exit(1);
	}
	tree_nametypep = alloc_type_list(total_types);
	tree_refhashp  = alloc_offset_list(total_types);
	tree_reftypep  = alloc_type_list(total_types);
	walkTree_name_ref(tree_base);
	sort_treenames();
	sort_treerefs();

	return;
}

/*
 * walk the tree to find all the names and offsets for lookup-by-name
 * and lookup-by-offset
 */
void
walkTree_name_ref(struct avl_table *tree)
{
	int	pos, hashnamesum;
	char	*cp;
	struct typenode *tnode;

	num_namehash = 0;
	num_refhash = 0;
	avl_t_init(&my_trav, tree);
	tnode = (struct typenode *)avl_t_first(&my_trav, tree);
	while (tnode) {
		pos = 0;
		hashnamesum = 0;
		cp = tnode->namep;
		while (*cp != '\0') {
			pos++;
			hashnamesum += (pos * (*cp));
			cp++;
		}
		*(tree_namehashp + num_namehash) = hashnamesum;
		*(tree_nametypep + num_namehash) = tnode;
		num_namehash++;

		*(tree_refhashp  + num_refhash)  = tnode->offset;
		*(tree_reftypep  + num_refhash)  = tnode;
		num_refhash++;

		if (num_namehash > total_types) {
			printf ("walkTree_name_ref: num_namehash too big\n");
			exit(1);
		}
		tnode = (struct typenode *)avl_t_next(&my_trav);
	}
	return;
}

/*
 * Given an AVL binary search tree, print out
 * its data elements in increasing sorted order.
 */
void
printTree(struct avl_table *tree)
{
	struct typenode *node;

	avl_t_init(&my_trav, tree);
	node = (struct typenode *)avl_t_first(&my_trav, tree);
	while (node) {
		printf ("%p: (%d)(%lld)(%d)(%lld)(%s)",
			node,node->members,node->hash,node->size,
			node->tag,node->namep);
		printf ("ref:%lld\n", node->offset);
		node = (struct typenode *)avl_t_next(&my_trav);
	}
	return;
}

#ifdef OLD
/*
 * Given a binary search tree, print out
 * its data elements in increasing sorted order.
 */
void
printTree(struct typenode *node)
{
	char	c, *cp;

	if (node == NULL) {
		printf ("\n");
		return;
	}

	printTree(node->left);

	cp = node->namep;
	printf ("%#x: (%d)(%lld)(%d)(%lld)(%s)",
		node,node->members,node->hash,node->size,node->tag,node->namep);
	printf ("ref:%lld left:%#x right:%#x ",
		node->offset,node->left,node->right);
	printTree(node->right);
}
#endif


/*
 * sort the hash list of type names
 */
void
sort_treenames()
{
	int		i, j, increment;
	int		array_size=num_namehash;
	int		temp, *numbers=tree_namehashp;
	struct typenode *thold, **tpointers=tree_nametypep;

	increment = 3;
	while (increment > 0) {
    		for (i=0; i < array_size; i++) {
			j = i;
			temp  = numbers  [i];
			thold = tpointers[i];
			while ((j >= increment) && (numbers[j-increment] >
								     temp)) {
				numbers  [j] = numbers  [j - increment];
				tpointers[j] = tpointers[j - increment];
				j = j - increment;
			}
			numbers  [j] = temp;
			tpointers[j] = thold;
		}
		if (increment/2 != 0) {
			increment = increment/2;
		} else if (increment == 1) {
      			increment = 0;
		} else {
			increment = 1;
		}
  	}
}

/*
 * sort the hash list of type offsets
 */
void
sort_treerefs()
{
	int		i, j, increment;
	int		array_size=num_refhash;
	Dwarf_Off	temp, *numbers=tree_refhashp;
	struct typenode *thold, **tpointers=tree_reftypep;

	increment = 3;
	while (increment > 0) {
    		for (i=0; i < array_size; i++) {
			j = i;
			temp  = numbers  [i];
			thold = tpointers[i];
			while ((j >= increment) && (numbers[j-increment] >
								     temp)) {
				numbers  [j] = numbers  [j - increment];
				tpointers[j] = tpointers[j - increment];
				j = j - increment;
			}
			numbers  [j] = temp;
			tpointers[j] = thold;
		}
		if (increment/2 != 0) {
			increment = increment/2;
		} else if (increment == 1) {
      			increment = 0;
		} else {
			increment = 1;
		}
  	}
}

/*
 * lookup a name in the types tree, using the sorted hash list
 *  return 1 or 0 (for found or not)
 *  (if the name is ambiguous, return the non-declaration in preference
 *   to the declaration (prototype))
 *  pointer to tree node is passed back in typep
 */
int
lookup_type_name(char *namep, struct typenode **typep)
{
	int	 	segsize, pos, num_ndecl=0, num_decl=0;
	int		*base, *last, *mid, hashnamesum=0;
	char 		*cp;
	struct typenode *tp, *tp_ndecl=NULL, *tp_decl=NULL;

	if (num_namehash == 0) {
		return 0;
	}
	pos = 0;
	cp = namep;
	while (*cp != '\0') {
		pos++;
		hashnamesum += (pos * (*cp));
		cp++;
	}

	base = tree_namehashp;
	last = tree_namehashp + num_namehash;
		/* last is one beyond the group we are considering */
	segsize = num_namehash;
	while (1) {
		mid = base + segsize/2;
		if (hashnamesum < *mid) {
			last = mid;
			segsize = mid - base;
		} else if (hashnamesum > *mid) {
			base = mid+1;
			segsize = last - base;
		} else {
			/* match; back up to the first match in the list */
			while (mid >= tree_namehashp && hashnamesum == *mid) {
				mid--;
			}
			mid++;  /* to the first matching hash */
			while (hashnamesum == *mid) {
				tp = *(tree_nametypep + (mid-tree_namehashp));
				if (!strcmp(tp->namep, namep)) {
					if (tp->declaration == 0) {
						tp_ndecl = tp;
						num_ndecl++;
					} else if (num_decl == 0) {
						tp_decl = tp;
						num_decl++;
					}
				}
				mid++;
			}
			/* return a non-declaration type in preference */
			if (num_ndecl > 0) {
				*typep = tp_ndecl;
				if (num_ndecl > 1) {
					if (verbose) {
						printf(
						"Warning: ambiguous name %s\n",
						 	namep);
					}
				}
			} else {
				*typep = tp_decl;
			}
			return 1;
		}

		if (segsize == 0) {
			return 0;
		}
	}
}

/*
 * lookup an offset in the types tree, using the sorted hash list
 *  return 1 or 0 (for found or not)
 *  pointer to tree node is passed back in typep
 */
int
lookup_type_ref(Dwarf_Off offset, struct typenode **typep)
{
	int	 	segsize;
	Dwarf_Off	*base, *last, *mid;

	if (num_refhash == 0) {
		return 0;
	}
	base = tree_refhashp;
	last = tree_refhashp + num_refhash;
		/* last is one beyond the group we are considering */
	segsize = num_refhash;
	while (1) {
		mid = base + segsize/2;
		if (offset < *mid) {
			last = mid;
			segsize = mid - base;
		} else if (offset > *mid) {
			base = mid+1;
			segsize = last - base;
		} else {
			/* back up to the first match in the list */
			while (mid >= tree_refhashp && offset == *mid) {
				mid--;
			}
			if (*mid != offset) {
				mid++;
			}
			*typep = *(tree_reftypep + (mid - tree_refhashp));
			return 1;
		}
		if (segsize == 0) {
			return 0;
		}
	}
}


/* Creates and returns a new table
   with comparison function |compare| using parameter |param|
   and memory allocator |allocator|.
   Returns |NULL| if memory allocation failed. */
struct avl_table *
avl_create(avl_comparison_func *compare, void *param,
	 struct libavl_allocator *allocator)
{
	struct avl_table *tree;

	assert (compare != NULL);

	if (allocator == NULL)
	  allocator = &avl_allocator_default;

	tree = (struct avl_table *)allocator->libavl_malloc (allocator, sizeof *tree); /* MIKTEX */
	if (tree == NULL)
	  return NULL;

	tree->avl_root = NULL;
	tree->avl_compare = compare;
	tree->avl_param = param;
	tree->avl_alloc = allocator;
	tree->avl_count = 0;
	tree->avl_generation = 0;

	return tree;
}

/* Inserts |item| into |table|.
	 Returns |NULL| if |item| was successfully inserted
	 or if a memory allocation error occurred.
	 Otherwise, returns the duplicate item. */
void *
avl_insert (struct avl_table *table, void *item)
{
	void **p = avl_probe (table, item);
	return p == NULL || *p == item ? NULL : *p;
}

/* Inserts |item| into |tree| and returns a pointer to |item|'s address.
	 If a duplicate item is found in the tree,
	 returns a pointer to the duplicate without inserting |item|.
	 Returns |NULL| in case of memory allocation failure. */
void **
avl_probe (struct avl_table *tree, void *item)
{
	struct avl_node *y, *z;
		/* Top node to update balance factor, and parent. */
	struct avl_node *p, *q; /* Iterator, and parent. */
	struct avl_node *n;	/* Newly inserted node. */
	struct avl_node *w;	/* New root of rebalanced subtree. */
	int dir;		/* Direction to descend. */
	unsigned char da[AVL_MAX_HEIGHT]; /* Cached comparison results. */
	int k = 0;		/* Number of cached results. */

	assert (tree != NULL && item != NULL);
	z = (struct avl_node *) &tree->avl_root;
	y = tree->avl_root;
	dir = 0;
	for (q = z, p = y; p != NULL; q = p, p = p->avl_link[dir]) {
		int cmp = tree->avl_compare(item, p->avl_data, tree->avl_param);
		if (cmp == 0)
			return &p->avl_data;

		if (p->avl_balance != 0)
			z = q, y = p, k = 0;
		da[k++] = dir = cmp > 0;
	}

	n = q->avl_link[dir] = (struct avl_node *) /* MIKTEX */
		tree->avl_alloc->libavl_malloc (tree->avl_alloc, sizeof *n);
	if (n == NULL)
		return NULL;

	tree->avl_count++;
	n->avl_data = item;
	n->avl_link[0] = n->avl_link[1] = NULL;
	n->avl_balance = 0;
	if (y == NULL) {
		return &n->avl_data;
	}

	for (p = y, k = 0; p != n; p = p->avl_link[da[k]], k++) {
		if (da[k] == 0)
			p->avl_balance--;
		else
			p->avl_balance++;
	}

	if (y->avl_balance == -2) {
		struct avl_node *x = y->avl_link[0];
		if (x->avl_balance == -1) {
			w = x;
			y->avl_link[0] = x->avl_link[1];
			x->avl_link[1] = y;
			x->avl_balance = y->avl_balance = 0;
		} else {
			assert (x->avl_balance == +1);
			w = x->avl_link[1];
			x->avl_link[1] = w->avl_link[0];
			w->avl_link[0] = x;
			y->avl_link[0] = w->avl_link[1];
			w->avl_link[1] = y;
			if (w->avl_balance == -1)
				x->avl_balance = 0, y->avl_balance = +1;
			else if (w->avl_balance == 0)
				x->avl_balance = y->avl_balance = 0;
			else /* |w->avl_balance == +1| */
				x->avl_balance = -1, y->avl_balance = 0;
				w->avl_balance = 0;
		}
	} else if (y->avl_balance == +2) {
		struct avl_node *x = y->avl_link[1];
		if (x->avl_balance == +1) {
			w = x;
			y->avl_link[1] = x->avl_link[0];
			x->avl_link[0] = y;
			x->avl_balance = y->avl_balance = 0;
		} else {
			assert (x->avl_balance == -1);
			w = x->avl_link[0];
			x->avl_link[0] = w->avl_link[1];
			w->avl_link[1] = x;
			y->avl_link[1] = w->avl_link[0];
			w->avl_link[0] = y;
			if (w->avl_balance == +1)
				x->avl_balance = 0, y->avl_balance = -1;
			else if (w->avl_balance == 0)
				x->avl_balance = y->avl_balance = 0;
			else /* |w->avl_balance == -1| */
				x->avl_balance = +1, y->avl_balance = 0;
			w->avl_balance = 0;
		}
	} else {
		return &n->avl_data;
	}
	z->avl_link[y != z->avl_link[0]] = w;

	tree->avl_generation++;
	return &n->avl_data;
}

/* Allocates |size| bytes of space using |malloc()|.
	 Returns a null pointer if allocation fails. */
void *
avl_malloc (struct libavl_allocator *allocator, size_t size)
{
	assert (allocator != NULL && size > 0);
	return malloc (size);
}

/* Frees |block|. */
void
avl_free (struct libavl_allocator *allocator, void *block)
{
	assert (allocator != NULL && block != NULL);
	free (block);
}

/* Search |tree| for an item matching |item|, and return it if found.
         Otherwise return |NULL|. */
struct typenode *
avl_find (struct avl_table *tree, void *item)
{
	int	cmp;
	const struct avl_node *p;

	assert (tree != NULL && item != NULL);
	for (p = tree->avl_root; p != NULL; ) {
		cmp = tree->avl_compare (item, p->avl_data, tree->avl_param);
		if (cmp < 0) {
			p = p->avl_link[0];
		} else if (cmp > 0) {
			p = p->avl_link[1];
		} else { /* |cmp == 0| */
			return p->avl_data;
		}
	}
	return NULL;
}

/*
	 The return value is expected to be like that returned by strcmp:
	 negative if a < b,
	 zero if a = b,
	 positive if a > b.
	 param is an arbitrary value defined by the user when the AVL tree was
	 created.
*/
int
typenode_compare(const void *avl_a, const void *avl_b, void *avl_param)
{
	int 		i;
	struct typenode	*typea, *typeb;

	typea = (struct typenode *)avl_a;
	typeb = (struct typenode *)avl_b;

	/* compare nodes a and b */
 	/* determine less or greater for this multi-part key */
	/* hash, members, size, tag, name */
	if (typea->hash != typeb->hash) {
		if (typea->hash < typeb->hash) {
			return -1;
		}
		return 1;
	}
	if (typea->members != typeb->members) {
		if (typea->members < typeb->members) {
			return -1;
		}
		return 1;
	}
	if (typea->size != typeb->size) {
		if (typea->size < typeb->size) {
			return -1;
		}
		return 1;
	}
	if (typea->tag != typeb->tag) {
		if (typea->tag < typeb->tag) {
			return -1;
		}
		return 1;
	}
	i = strcmp(typea->namep,typeb->namep);
	if (i != 0) {
		if (i < 0) {
			return -1;
		}
		return 1;
	}
	return 0;
}

/*
   Compare two nodes in the alias_tree
*/
int
alias_compare(const void *avl_a, const void *avl_b, void *avl_param)
{
	struct aliasnode	*aliasa, *aliasb;

	aliasa = (struct aliasnode *)avl_a;
	aliasb = (struct aliasnode *)avl_b;

	/* compare nodes a and b */
	if (aliasa->ref < aliasb->ref) {
		return -1;
	} else if (aliasa->ref > aliasb->ref) {
		return 1;
	} else {
		return 0;
	}
}

/* Initializes |trav| for use with |tree|
	 and selects the null node. */
void
avl_t_init (struct avl_traverser *trav, struct avl_table *tree)
{
	trav->avl_table = tree;
	trav->avl_node = NULL;
	trav->avl_height = 0;
	trav->avl_generation = tree->avl_generation;
}

/* Initializes |trav| for |tree|
	 and selects and returns a pointer to its least-valued item.
	 Returns |NULL| if |tree| contains no nodes. */
void *
avl_t_first (struct avl_traverser *trav, struct avl_table *tree)
{
	struct avl_node *x;

	assert (tree != NULL && trav != NULL);

	trav->avl_table = tree;
	trav->avl_height = 0;
	trav->avl_generation = tree->avl_generation;

	x = tree->avl_root;
	if (x != NULL) {
		while (x->avl_link[0] != NULL) {
			assert (trav->avl_height < AVL_MAX_HEIGHT);
			trav->avl_stack[trav->avl_height++] = x;
			x = x->avl_link[0];
		}
	}
	trav->avl_node = x;

	return x != NULL ? x->avl_data : NULL;
}

/* Returns the next data item in inorder
	 within the tree being traversed with |trav|,
	 or if there are no more data items returns |NULL|. */
void *
avl_t_next (struct avl_traverser *trav)
{
	struct avl_node *x;

	assert (trav != NULL);

	if (trav->avl_generation != trav->avl_table->avl_generation)
		trav_refresh (trav);

	x = trav->avl_node;
	if (x == NULL) {
		return avl_t_first (trav, trav->avl_table);
	} else if (x->avl_link[1] != NULL) {
		assert (trav->avl_height < AVL_MAX_HEIGHT);
		trav->avl_stack[trav->avl_height++] = x;
		x = x->avl_link[1];

		while (x->avl_link[0] != NULL) {
			assert (trav->avl_height < AVL_MAX_HEIGHT);
			trav->avl_stack[trav->avl_height++] = x;
			x = x->avl_link[0];
		}
	} else {
			struct avl_node *y;

			do {
				if (trav->avl_height == 0) {
					trav->avl_node = NULL;
					return NULL;
				}

				y = x;
				x = trav->avl_stack[--trav->avl_height];
			} while (y == x->avl_link[1]);
	}
	trav->avl_node = x;

	return x->avl_data;
}

/* Refreshes the stack of parent pointers in |trav|
	 and updates its generation number. */
void
trav_refresh (struct avl_traverser *trav)
{
	avl_comparison_func	*cmp;
	assert (trav != NULL);

	trav->avl_generation = trav->avl_table->avl_generation;

	if (trav->avl_node != NULL) {
		cmp = trav->avl_table->avl_compare;
		void *param = trav->avl_table->avl_param;
		struct avl_node *node = trav->avl_node;
		struct avl_node *i;

		trav->avl_height = 0;
		for (i = trav->avl_table->avl_root; i != node; ) {
			assert (trav->avl_height < AVL_MAX_HEIGHT);
			assert (i != NULL);

			trav->avl_stack[trav->avl_height++] = i;
			i =
		 	 i->avl_link[cmp(node->avl_data,i->avl_data,param) > 0];
		}
	}
}
