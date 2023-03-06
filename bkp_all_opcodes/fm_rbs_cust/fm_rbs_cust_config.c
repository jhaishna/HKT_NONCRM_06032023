#ifndef lint
static const char Sccs_id[] = "@(#)%Portal Version: fm_rbs_cust_config.c :CUPmod7.3PatchInt:1:2007-Jan-10 20:07:40 %";
#endif

#include <stdio.h>      /* for FILE  in pcm.h */
#include "ops/act.h"
#include "pcm.h"
#include "cm_fm.h"
#include "pin_errs.h"
#include "pin_cust.h"
#include "pin_bill.h"
#include "pinlog.h"
#include "pin_currency.h"
#include "custom_wrapper_ops.h"

#ifdef MSDOS
__declspec(dllexport) void * fm_rbs_cust_config();
#endif


/*******************************************************************
 *******************************************************************/

    /*
     * NOTE THAT THE DISPATCH ENTRIES ARE COMMENTED. WHEN YOU OVERRIDE
     * AN IMPLEMENTATION, UNCOMMENT THE LINE BELOW THAT MATCHES THE
     * OPCODE FOR WHICH YOU HAVE PROVIDED AN ALTERNATE IMPLEMENTATION.
     */

struct cm_fm_config fm_rbs_cust_config[] = {
        /* opcode as a u_int, function name (as a string) */
		{ PCM_OP_RBS_CUST_SUSPEND_LINE, "op_rbs_cust_line_suspend" },
		{ PCM_OP_RBS_CUST_RESUME_LINE, "op_rbs_cust_line_resume" },
		{ PCM_OP_RBS_CUST_UPDATE_CUSTOMER, "op_rbs_cust_acct_update" },		
        	{ PCM_OP_RBS_CUST_CANCEL_SERVICE, "op_rbs_cust_cancel_service"},
		{ PCM_OP_RBS_CUST_LINE_PROFILE_UPDATE, "op_rbs_cust_line_profile_update" },
		{ PCM_OP_RBS_CUST_ACCT_PROFILE_UPDATE, "op_rbs_cust_acct_profile_update" },
		{ PCM_OP_RBS_CUST_SWAP_HANDSET_REBATE_PURCHASE, "op_rbs_cust_swap_handset_rebate_purchase" },
		{ PCM_OP_RBS_CUST_SWAP_HANDSET_REBATE_PURCHASE_CANCEL, "op_rbs_cust_swap_handset_rebate_purchase_cancel" },
		{ PCM_OP_RBS_CUST_PURCHASE_SERVICE_ITEM, "op_rbs_cust_purchase_service_item"},
		{ PCM_CUSTOM_PURCHASE_SERVICE, "op_cust_purchase_service" },
		{ PCM_OP_RBS_CUST_SUSPENSE_SERVICE, "op_rbs_cust_suspense_service" },
                { PCM_OP_RBS_CUST_RESUME_SERVICE, "op_rbs_cust_resume_service"},
		{ PCM_OP_RBS_CUST_LINE_ACTIVATE, "op_rbs_cust_line_activate"},
		
		{ PCM_OP_RBS_CUST_CHANGE_DEAL, "op_rbs_cust_change_deal" },
	{ 0,    (char *)0 }
};

#ifdef MSDOS
void *
fm_rbs_cust_config()
{
  return ((void *) (fm_rbs_cust_config));
}
#endif
