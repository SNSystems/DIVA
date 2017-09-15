/* fprintf() */
#include <stdio.h>

/* open() */
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

/* elf_*() */
#include <libelf.h>

/* dwarf_*() */
#include <libdwarf.h>

int main( int argc, char ** argv ) {
	int status = 0;

	int fid = -1;
	Elf * elfp = NULL;
	
	Dwarf_Error error = (Dwarf_Error) NULL;
	Dwarf_Debug dbg;
	
	Dwarf_Cie * cie_data = NULL;
	Dwarf_Fde * fde_data = NULL;
	Dwarf_Signed cie_count, fde_count;

	if( argc != 2 ) {
		fprintf( stderr, "usage: %s <binary>\n", argv[0] );
		return 1;
		}
		
	fid = open( argv[1], O_RDONLY );
	if( fid == -1 ) {
		fprintf( stderr, "failed to open( %s ).\n", argv[1] );
		return 4;
		}
		
	if( elf_version( EV_CURRENT ) == EV_NONE ) {
		fprintf( stderr, "elf version check failed.\n" );
		return 6;
		}

#if defined( USE_ELF_POINTER )			
	elfp = elf_begin( fid, ELF_C_READ, (Elf *)NULL );
	if( elfp == NULL ) {
		fprintf( stderr, "failed to elf_begin().\n" );	
		return 5;
		}
		
	status = dwarf_elf_init( elfp, DW_DLC_READ, NULL, NULL, & dbg, & error );
	if( status != DW_DLV_OK ) {
		fprintf( stderr, "failed to dwarf_elf_init().\n" );
		return 2;
		}
#else
	status = dwarf_init( fid, DW_DLC_READ, NULL, NULL, & dbg, NULL );
	if( status != DW_DLV_OK ) {
		fprintf( stderr, "failed to dwarf_init().\n" );
		return 2;
		}
#endif
	
	status = dwarf_get_fde_list_eh( dbg, & cie_data, & cie_count, & fde_data, & fde_count, & error );
	if( status != DW_DLV_OK ) {
		fprintf( stderr, "failed to dwarf_get_fde_list_eh().\n" );
		return 3;
		}
	
	return 0;
	} /* end main() */
