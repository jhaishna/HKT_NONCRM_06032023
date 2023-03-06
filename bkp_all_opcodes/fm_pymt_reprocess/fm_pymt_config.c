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
static const char Sccs_id[] = "@(#)%Portal Version: fm_rbs_pymt_config.c :CUPmod7.3PatchInt:1:2007-Jan-10 20:07:40 %";
#endif

#include <stdio.h>      /* for FILE  in pcm.h */
#include "ops/act.h"
#include "ops/pymt.h"
#include "pcm.h"
#include "cm_fm.h"
#include "pinlog.h"
#include "fm_utils.h"
#include "pin_pymt.h"
#include "pin_flds.h"
#include "fm_rbs_pymt.h"

#ifdef MSDOS
__declspec(dllexport) void * fm_rbs_pymt_config();
#endif


/*******************************************************************
 *******************************************************************/

    /*
     * NOTE THAT THE DISPATCH ENTRIES ARE COMMENTED. WHEN YOU OVERRIDE
     * AN IMPLEMENTATION, UNCOMMENT THE LINE BELOW THAT MATCHES THE
     * OPCODE FOR WHICH YOU HAVE PROVIDED AN ALTERNATE IMPLEMENTATION.
     */

struct cm_fm_config fm_rbs_pymt_config[] = {
        /* opcode as a u_int, function name (as a string) */
        { RBS_PYMT_REJ_UPLOAD, "op_pymt_rej_upload" },
        { RBS_PYMT_REPROCESS, "op_pymt_reprocess" },
        { 0,    (char *)0 }
};

#ifdef MSDOS
void *
fm_rbs_pymt_config()
{
  return ((void *) (fm_rbs_pymt_config));
}
#endif

