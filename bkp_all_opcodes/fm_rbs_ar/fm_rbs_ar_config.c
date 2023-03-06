
#ifndef lint
static const char Sccs_id[] = "@(#)%Portal Version: fm_rbs_ar_config.c :CUPmod7.3PatchInt:1:2007-Jan-10 20:07:40 %";
#endif

#include <stdio.h>      /* for FILE * in pcm.h */
#include "ops/act.h"
#include "pcm.h"
#include "cm_fm.h"
#include "pin_cust.h"
#include "rbs_ar_adjustment.h"
#ifdef MSDOS
__declspec(dllexport) void * fm_rbs_ar_config();
#endif

/*******************************************************************
MSO_OP_GLID_CONFIG
 *******************************************************************/

    /*
     * NOTE THAT THE DISPATCH ENTRIES ARE COMMENTED. WHEN YOU OVERRIDE
     * AN IMPLEMENTATION, UNCOMMENT THE LINE BELOW THAT MATCHES THE
     * OPCODE FOR WHICH YOU HAVE PROVIDED AN ALTERNATE IMPLEMENTATION.
     */

struct cm_fm_config fm_rbs_ar_config[] = {
        /* opcode as a u_int, function name (as a string) */
        { RBS_OP_AR_ACC_ADJUSTMENT, "op_ar_acc_adjustment" },
	{ RBS_OP_AR_BILL_ADJUSTMENT, "op_ar_bill_adjustment" },
	{ RBS_OP_AR_DELETE_ADJUSTMENT, "op_ar_item_adjustment" },
	{ RBS_OP_ADJ_LOADER, "op_ar_adj_loader"},
        { 0,    (char *)0 }
};

#ifdef MSDOS
void *
fm_rbs_ar_config()
{
  return ((void *) (fm_rbs_ar_config));
}
#endif

