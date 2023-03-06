/*******************************************************************
 *
* Copyright (c) 2007, 2014, Oracle and/or its affiliates. All rights reserved.
 *
 *      This material is the confidential property of Oracle Corporation
 *      or its licensors and may be used, reproduced, stored or transmitted
 *      only in accordance with a valid Oracle license or sublicense agreement.
 *
 *******************************************************************/

#ifndef lint
static const char Sccs_id[] = "@(#)%Portal Version: fm_rbs_utils_config.c :CUPmod7.3PatchInt:1:2007-Jan-10 20:07:40 %";
#endif

#include <stdio.h>	/* for FILE * in pcm.h */
#include "ops/act.h"
#include "pcm.h"
#include "cm_fm.h"
#include "str_adj_code_search.h"

#ifdef MSDOS
__declspec(dllexport) void * fm_rbs_utils_config();
#endif


/*******************************************************************
 *******************************************************************/

    /*
     * NOTE THAT THE DISPATCH ENTRIES ARE COMMENTED. WHEN YOU OVERRIDE
     * AN IMPLEMENTATION, UNCOMMENT THE LINE BELOW THAT MATCHES THE
     * OPCODE FOR WHICH YOU HAVE PROVIDED AN ALTERNATE IMPLEMENTATION.
     */

struct cm_fm_config fm_rbs_utils_config[] = {
	/* opcode as a u_int, function name (as a string) */
	{ RBS_OP_ADJCODE_SEARCH, "op_string_search" },
	{ RBS_OP_GLID_SEARCH, "op_rbs_utils_glid_search" },
	{ RBS_OP_GLID_DELETE, "op_rbs_utils_delete_glid" },
	{ 0,	(char *)0 }
};

#ifdef MSDOS
void *
fm_rbs_utils_config_func()
{
  return ((void *) (fm_rbs_utils_config));
}
#endif

