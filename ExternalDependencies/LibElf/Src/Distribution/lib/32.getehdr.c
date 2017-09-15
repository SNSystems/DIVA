/*
32.getehdr.c - implementation of the elf{32,64}_getehdr(3) functions.
Copyright (C) 1995 - 1998 Michael Riepe

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public
License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA
*/

#include <private.h>

#ifndef lint
static const char rcsid[] = "@(#) $Id: 32.getehdr.c,v 1.9 2008/05/23 08:15:34 michael Exp $";
#endif /* lint */

/* e_ident[EI_CLASS] */
static char *e_ident_class_literal[] = {
  /* 0 */ "ELFCLASSNONE",
  /* 1 */ "ELFCLASS32",
  /* 2 */ "ELFCLASS64",
};

/* e_ident[EI_DATA] */
static char *e_ident_data_literal[] = {
  /* 0 */ "ELFDATANONE",
  /* 1 */ "ELFDATA2LSB",
  /* 2 */ "ELFDATA2MSB",
};

/* e_ident[EI_VERSION], e_version */
static char *e_ident_version_literal[] = {
  /* 0 */ "EV_NONE",
  /* 1 */ "EV_CURRENT",
};

/* e_ident[EI_OSABI] */
static struct {
  char *short_name;
  char *long_name;
} e_ident_osabi_literal[] = {
  /*   0 */ "ELFOSABI_NONE",        "No extensions or unspecified",
                                    /* ELFOSABI_SYSV same as ELFOSABI_NONE */
  /*   1 */ "ELFOSABI_HPUX",        "Hewlett-Packard HP-UX",
  /*   2 */ "ELFOSABI_NETBSD",      "NetBSD",
  /*   3 */ "ELFOSABI_LINUX",       "Linux",
  /*   4 */ "?",                    "?",
  /*   5 */ "?",                    "?",
  /*   6 */ "ELFOSABI_SOLARIS",     "Sun Solaris",
  /*   7 */ "ELFOSABI_AIX",         "AIX",
  /*   8 */ "ELFOSABI_IRIX",        "IRIX",
  /*   9 */ "ELFOSABI_FREEBSD",     "FreeBSD",
  /*  10 */ "ELFOSABI_TRU64",       "Compaq TRU64 UNIX",
  /*  11 */ "ELFOSABI_MODESTO",     "Novell Modesto",
  /*  12 */ "ELFOSABI_OPENBSD",     "Open BSD",
  /*  13 */ "ELFOSABI_OPENVMS",     "Open VMS",
  /*  14 */ "ELFOSABI_NSK",         "Hewlett-Packard Non-Stop Kernel",
  /*  15 */ "ELFOSABI_AROS",        "Amiga Research OS",
  /* these are probably obsolete: */
  /*  97 */ "ELFOSABI_ARM",         "ARM",
  /* 255 */ "ELFOSABI_STANDALONE",  "standalone (embedded) application",
};

/* e_type */
static char *e_type_literal[] = {
  /* 0 */ "ET_NONE",
  /* 1 */ "ET_REL",
  /* 2 */ "ET_EXEC",
  /* 3 */ "ET_DYN",
  /* 4 */ "ET_CORE",
};

/* e_machine */
static struct {
  char *short_name;
  char *long_name;
} e_machine_literal[] = {
  /*   0 */ "EM_NONE",        "No machine",
  /*   1 */ "EM_M32",         "AT&T WE 32100",
  /*   2 */ "EM_SPARC",       "SPARC",
  /*   3 */ "EM_386",         "Intel 80386",
  /*   4 */ "EM_68K",         "Motorola 68000",
  /*   5 */ "EM_88K",         "Motorola 88000",
  /*   6 */ "EM_486",         "Intel i486",
  /*   7 */ "EM_860",         "Intel 80860",
  /*   8 */ "EM_MIPS",        "MIPS I Architecture",
  /*   9 */ "EM_S370",        "IBM System/370 Processor",
  /*  10 */ "EM_MIPS_RS3_LE", "MIPS RS3000 Little-endian",
  /*  11 */ "EM_SPARC64",     "SPARC 64-bit",
  /*  12 */ "?",              "?",
  /*  13 */ "?",              "?",
  /*  14 */ "?",              "?",
  /*  15 */ "EM_PARISC",      "Hewlett-Packard PA-RISC",
  /*  16 */ "?",              "?",
  /*  17 */ "EM_VPP500",      "Fujitsu VPP500",
  /*  18 */ "EM_SPARC32PLUS", "Enhanced instruction set SPARC",
  /*  19 */ "EM_960",         "Intel 80960",
  /*  20 */ "EM_PPC",         "PowerPC",
  /*  21 */ "EM_PPC64",       "64-bit PowerPC",
  /*  22 */ "EM_S390",        "IBM System/390 Processor",
  /*  23 */ "?",              "?",
  /*  24 */ "?",              "?",
  /*  25 */ "?",              "?",
  /*  26 */ "?",              "?",
  /*  27 */ "?",              "?",
  /*  28 */ "?",              "?",
  /*  29 */ "?",              "?",
  /*  30 */ "?",              "?",
  /*  31 */ "?",              "?",
  /*  32 */ "?",              "?",
  /*  33 */ "?",              "?",
  /*  34 */ "?",              "?",
  /*  35 */ "?",              "?",
  /*  36 */ "EM_V800",        "NEC V800",
  /*  37 */ "EM_FR20",        "Fujitsu FR20",
  /*  38 */ "EM_RH32",        "TRW RH-32",
  /*  39 */ "EM_RCE",         "Motorola RCE",
  /*  40 */ "EM_ARM",         "Advanced RISC Machines ARM",
  /*  41 */ "EM_ALPHA",       "Digital Alpha",
  /*  42 */ "EM_SH",          "Hitachi SH",
  /*  43 */ "EM_SPARCV9",     "SPARC Version 9",
  /*  44 */ "EM_TRICORE",     "Siemens TriCore embedded processor",
  /*  45 */ "EM_ARC",         "Argonaut RISC Core, Argonaut Technologies Inc.",
  /*  46 */ "EM_H8_300",      "Hitachi H8/300",
  /*  47 */ "EM_H8_300H",     "Hitachi H8/300H",
  /*  48 */ "EM_H8S",         "Hitachi H8S",
  /*  49 */ "EM_H8_500",      "Hitachi H8/500",
  /*  50 */ "EM_IA_64",       "Intel IA-64 processor architecture",
  /*  51 */ "EM_MIPS_X",      "Stanford MIPS-X",
  /*  52 */ "EM_COLDFIRE",    "Motorola ColdFire",
  /*  53 */ "EM_68HC12",      "Motorola M68HC12",
  /*  54 */ "EM_MMA",         "Fujitsu MMA Multimedia Accelerator",
  /*  55 */ "EM_PCP",         "Siemens PCP",
  /*  56 */ "EM_NCPU",        "Sony nCPU embedded RISC processor",
  /*  57 */ "EM_NDR1",        "Denso NDR1 microprocessor",
  /*  58 */ "EM_STARCORE",    "Motorola Star*Core processor",
  /*  59 */ "EM_ME16",        "Toyota ME16 processor",
  /*  60 */ "EM_ST100",       "STMicroelectronics ST100 processor",
  /*  61 */ "EM_TINYJ",       "Advanced Logic Corp. TinyJ embedded processor family",
  /*  62 */ "EM_X86_64",      "AMD x86-64 architecture", /* Same as EM_AMD64 */
  /*  63 */ "EM_PDSP",        "Sony DSP Processor",
  /*  64 */ "?",              "?",
  /*  65 */ "?",              "?",
  /*  66 */ "EM_FX66",        "Siemens FX66 microcontroller",
  /*  67 */ "EM_ST9PLUS",     "STMicroelectronics ST9+ 8/16 bit microcontroller",
  /*  68 */ "EM_ST7",         "STMicroelectronics ST7 8-bit microcontroller",
  /*  69 */ "EM_68HC16",      "Motorola MC68HC16 Microcontroller",
  /*  70 */ "EM_68HC11",      "Motorola MC68HC11 Microcontroller",
  /*  71 */ "EM_68HC08",      "Motorola MC68HC08 Microcontroller",
  /*  72 */ "EM_68HC05",      "Motorola MC68HC05 Microcontroller",
  /*  73 */ "EM_SVX",         "Silicon Graphics SVx",
  /*  74 */ "EM_ST19",        "STMicroelectronics ST19 8-bit microcontroller",
  /*  75 */ "EM_VAX",         "Digital VAX",
  /*  76 */ "EM_CRIS",        "Axis Communications 32-bit embedded processor",
  /*  77 */ "EM_JAVELIN",     "Infineon Technologies 32-bit embedded processor",
  /*  78 */ "EM_FIREPATH",    "Element 14 64-bit DSP Processor",
  /*  79 */ "EM_ZSP",         "LSI Logic 16-bit DSP Processor",
  /*  80 */ "EM_MMIX",        "Donald Knuth's educational 64-bit processor",
  /*  81 */ "EM_HUANY",       "Harvard University machine-independent object files",
  /*  82 */ "EM_PRISM",       "SiTera Prism",
  /*  83 */ "EM_AVR",         "Atmel AVR 8-bit microcontroller",
  /*  84 */ "EM_FR30",        "Fujitsu FR30",
  /*  85 */ "EM_D10V",        "Mitsubishi D10V",
  /*  86 */ "EM_D30V",        "Mitsubishi D30V",
  /*  87 */ "EM_V850",        "NEC v850",
  /*  88 */ "EM_M32R",        "Mitsubishi M32R",
  /*  89 */ "EM_MN10300",     "Matsushita MN10300",
  /*  90 */ "EM_MN10200",     "Matsushita MN10200",
  /*  91 */ "EM_PJ",          "picoJava",
  /*  92 */ "EM_OPENRISC",    "OpenRISC 32-bit embedded processor",
  /*  93 */ "EM_ARC_A5",      "ARC Cores Tangent-A5",
  /*  94 */ "EM_XTENSA",      "Tensilica Xtensa Architecture",
  /*  95 */ "EM_VIDEOCORE",   "Alphamosaic VideoCore processor",
  /*  96 */ "EM_TMM_GPP",     "Thompson Multimedia General Purpose Processor",
  /*  97 */ "EM_NS32K",       "National Semiconductor 32000 series",
  /*  98 */ "EM_TPC",         "Tenor Network TPC processor",
  /*  99 */ "EM_SNP1K",       "Trebia SNP 1000 processor",
  /* 100 */ "EM_ST200",       "STMicroelectronics (www.st.com) ST200 microcontroller",
  /* 101 */ "EM_IP2K",        "Ubicom IP2xxx microcontroller family",
  /* 102 */ "EM_MAX",         "MAX Processor",
  /* 103 */ "EM_CR",          "National Semiconductor CompactRISC microprocessor",
  /* 104 */ "EM_F2MC16",      "Fujitsu F2MC16",
  /* 105 */ "EM_MSP430",      "Texas Instruments embedded microcontroller msp430",
  /* 106 */ "EM_BLACKFIN",    "Analog Devices Blackfin (DSP) processor",
  /* 107 */ "EM_SE_C33",      "S1C33 Family of Seiko Epson processors",
  /* 108 */ "EM_SEP",         "Sharp embedded microprocessor",
  /* 109 */ "EM_ARCA",        "Arca RISC Microprocessor",
  /* 110 */ "EM_UNICORE",     "Microprocessor series from PKU-Unity Ltd. and MPRC of Peking University",
};

static char *unknown = "?";
char*
_elf_getehdr(Elf *elf, unsigned cls) {
  if (!elf) {
  	return NULL;
  }

  elf_assert(elf->e_magic == ELF_MAGIC);
  if (elf->e_kind != ELF_K_ELF) {
    seterr(ERROR_NOTELF);
  }
  else if (elf->e_class != cls) {
    seterr(ERROR_CLASSMISMATCH);
  }
  else if (elf->e_ehdr || _elf_cook(elf)) {
    return elf->e_ehdr;
  }

  return NULL;
}

Elf32_Ehdr*
elf32_getehdr(Elf *elf) {
  return (Elf32_Ehdr*)_elf_getehdr(elf, ELFCLASS32);
}

#ifdef _WIN32
void
elf32_gethdr_literals(Elf32_Ehdr *elf_ehdr,Elf_Ehdr_Literal *eh_literals) {
  unsigned char uc;
  static char file_id[12];
  sprintf(file_id,"%02x %02x %02x %02x",
    elf_ehdr->e_ident[EI_MAG0],elf_ehdr->e_ident[EI_MAG1],
    elf_ehdr->e_ident[EI_MAG2],elf_ehdr->e_ident[EI_MAG3]);
  eh_literals->e_ident_file_id = file_id;

  uc = elf_ehdr->e_ident[EI_CLASS];
  eh_literals->e_ident_file_class = unknown;
  if (uc >= 0 && uc < ELFCLASSNUM) {
    eh_literals->e_ident_file_class = e_ident_class_literal[uc];
  }

  uc = elf_ehdr->e_ident[EI_DATA];
  eh_literals->e_ident_data_encoding = unknown;
  if (uc >= 0 && uc < ELFDATANUM) {
    eh_literals->e_ident_data_encoding = e_ident_data_literal[uc];
  }

  uc = elf_ehdr->e_ident[EI_VERSION];
  eh_literals->e_ident_file_version = unknown;
  if (uc >= 0 && uc < EV_NUM) {
    eh_literals->e_ident_file_version = e_ident_version_literal[uc];
  }

  uc = elf_ehdr->e_ident[EI_OSABI];
  eh_literals->e_ident_os_abi_s = unknown;
  eh_literals->e_ident_os_abi_l = unknown;
  if (uc >= 0 && uc < ELFOSABI_NUM) {
  	eh_literals->e_ident_os_abi_s = e_ident_osabi_literal[uc].short_name;
    eh_literals->e_ident_os_abi_l = e_ident_osabi_literal[uc].long_name;
  }

  eh_literals->e_ident_abi_version = unknown;

  eh_literals->e_type = "?";
  if (elf_ehdr->e_type >= 0 && elf_ehdr->e_type < ET_NUM) {
  	eh_literals->e_type = e_type_literal[elf_ehdr->e_type];
  }

  eh_literals->e_machine_s = "?";
  eh_literals->e_machine_l = "?";
  if (elf_ehdr->e_machine >= 0 && elf_ehdr->e_machine < EM_NUM) {
  	eh_literals->e_machine_s = e_machine_literal[elf_ehdr->e_machine].short_name;
    eh_literals->e_machine_l = e_machine_literal[elf_ehdr->e_machine].long_name;
  }
}
#endif /* _WIN32 */

#if __LIBELF64

Elf64_Ehdr*
elf64_getehdr(Elf *elf) {
  return (Elf64_Ehdr*)_elf_getehdr(elf, ELFCLASS64);
}

/* SN-Carlos: Get the ELF header as literal strings */
#ifdef _WIN32
void
elf64_gethdr_literals(Elf64_Ehdr *elf_ehdr,Elf_Ehdr_Literal *eh_literals) {
  unsigned char uc;
  static char file_id[12];
  sprintf(file_id,"%02x %02x %02x %02x",
    elf_ehdr->e_ident[EI_MAG0],elf_ehdr->e_ident[EI_MAG1],
    elf_ehdr->e_ident[EI_MAG2],elf_ehdr->e_ident[EI_MAG3]);
  eh_literals->e_ident_file_id = file_id;

  uc = elf_ehdr->e_ident[EI_CLASS];
  eh_literals->e_ident_file_class = unknown;
  if (uc >= 0 && uc < ELFCLASSNUM) {
    eh_literals->e_ident_file_class = e_ident_class_literal[uc];
  }

  uc = elf_ehdr->e_ident[EI_DATA];
  eh_literals->e_ident_data_encoding = unknown;
  if (uc >= 0 && uc < ELFDATANUM) {
    eh_literals->e_ident_data_encoding = e_ident_data_literal[uc];
  }

  uc = elf_ehdr->e_ident[EI_VERSION];
  eh_literals->e_ident_file_version = unknown;
  if (uc >= 0 && uc < EV_NUM) {
    eh_literals->e_ident_file_version = e_ident_version_literal[uc];
  }

  uc = elf_ehdr->e_ident[EI_OSABI];
  eh_literals->e_ident_os_abi_s = unknown;
  eh_literals->e_ident_os_abi_l = unknown;
  if (uc >= 0 && uc < ELFOSABI_NUM) {
  	eh_literals->e_ident_os_abi_s = e_ident_osabi_literal[uc].short_name;
    eh_literals->e_ident_os_abi_l = e_ident_osabi_literal[uc].long_name;
  }

  eh_literals->e_ident_abi_version = unknown;

  eh_literals->e_type = "?";
  if (elf_ehdr->e_type >= 0 && elf_ehdr->e_type < ET_NUM) {
  	eh_literals->e_type = e_type_literal[elf_ehdr->e_type];
  }

  eh_literals->e_machine_s = "?";
  eh_literals->e_machine_l = "?";
  if (elf_ehdr->e_machine >= 0 && elf_ehdr->e_machine < EM_NUM) {
  	eh_literals->e_machine_s = e_machine_literal[elf_ehdr->e_machine].short_name;
    eh_literals->e_machine_l = e_machine_literal[elf_ehdr->e_machine].long_name;
  }
}
#endif /* _WIN32 */

#endif /* __LIBELF64 */
