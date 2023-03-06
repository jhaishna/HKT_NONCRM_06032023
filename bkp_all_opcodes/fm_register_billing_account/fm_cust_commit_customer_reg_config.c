#ifndef lint
static const char Sccs_id[] = "@(#)%Portal Version: fm_cust_commit_customer_reg_config.c:CUPmod7.3PatchInt:1:2007-Jan-10 20:07:40 %";
#endif

#include <stdio.h>      /* for FILE  in pcm.h */
#include "ops/act.h"
#include "pcm.h"
#include "cm_fm.h"
#include "custom_wrapper_ops.h"

#ifdef MSDOS
__declspec(dllexport) void * fm_cust_commit_customer_reg_config();
#endif


/*******************************************************************
 *******************************************************************/

    /*
     * NOTE THAT THE DISPATCH ENTRIES ARE COMMENTED. WHEN YOU OVERRIDE
     * AN IMPLEMENTATION, UNCOMMENT THE LINE BELOW THAT MATCHES THE
     * OPCODE FOR WHICH YOU HAVE PROVIDED AN ALTERNATE IMPLEMENTATION.
     */

struct cm_fm_config fm_cust_commit_customer_reg_config[] = {
        /* opcode as a u_int, function name (as a string) */
                { PCM_POL_CUST_COMMIT_CUSTOMER_REG, "op_cust_commit_customer_reg" },
		{ PCM_OP_SUB_ACCT_REG, "op_cust_sub_acct_reg" },
	
	        { 0,    (char *)0 }
};

#ifdef MSDOS
void *
fm_cust_commit_customer_reg_config()
{
  return ((void *) (fm_cust_commit_customer_reg_config));
}
#endif
