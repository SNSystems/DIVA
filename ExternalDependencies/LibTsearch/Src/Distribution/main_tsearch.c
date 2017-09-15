/* Copyright (c) 2013-2014, David Anderson
All rights reserved.

Redistribution and use in source and binary forms, with
or without modification, are permitted provided that the
following conditions are met:

    Redistributions of source code must retain the above
    copyright notice, this list of conditions and the following
    disclaimer.

    Redistributions in binary form must reproduce the above
    copyright notice, this list of conditions and the following
    disclaimer in the documentation and/or other materials
    provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND
CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/


/*  The interfaces follow tsearch (See the Single
    Unix Specification) but the implementation is
    written without reference to the source of any
    version of tsearch.

    See http://www.prevanders.net/tsearch.html
    for information and an example of use.
*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#if defined(TSEARCH_USE_BAL)
#include "dwarf_tsearchbal.c"
#elif defined(TSEARCH_USE_BIN)
#include "dwarf_tsearchbin.c"
#elif defined(TSEARCH_USE_EPP)
#include "dwarf_tsearchepp.c"
#elif defined(TSEARCH_USE_HASH)
#include "dwarf_tsearchhash.c"
#elif defined(TSEARCH_USE_RED)
#include "dwarf_tsearchred.c"
#elif defined(TSEARCH_USE_GNU)
#include "gnu_tsearch.c"
#else
#error Missing tsearch algorithm
#endif

#if defined(TSEARCH_USE_GNU)

static void
dumptree_inner(const struct hs_base *h,
    char *(* keyprint)(const void *),
    const char *descr, int printdetails)
{
}

/*  Dumping the tree.  */
void
dwarf_tdump(const void*headp_in,
    char *(* keyprint)(const void *),
    const char *msg)
{
    const struct hs_base *head = (const struct hs_base *)headp_in;
    if(!head) {
        printf("dumptree null tree ptr : %s\n",msg);
        return;
    }
    dumptree_inner(head,keyprint,msg,1);
}

void *
dwarf_initialize_search_hash( void **treeptr,
    DW_TSHASHTYPE(*hashfunc)(const void *key),
    unsigned long size_estimate)
{
  return NULL;
}

#endif /* TSEARCH_USE_GNU */
