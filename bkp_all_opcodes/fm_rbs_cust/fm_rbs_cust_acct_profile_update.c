#ifndef lint
static const char Sccs_id[] = "@(#)%Portal Version: fm_rbs_cust_acct_profile_update.c :BillingVelocityInt:3:2006-Sep-05 21:55:03 %";
#endif

#include <stdio.h>
#include <string.h>
#include <time.h>
#include "pcm.h"
#include "ops/act.h"
#include "pin_bill.h"
#include "cm_fm.h"
#include "pin_errs.h"
#include "pinlog.h"
#include "fm_utils.h"
#include "ops/rbs.h"
#include "custom_flds/custom_flds.h"
#include "custom_wrapper_ops.h"


/*************************************
*DEFINE FUNCTIONS
*************************************/

EXPORT_OP void
op_rbs_cust_acct_profile_update(
    cm_nap_connection_t *connp,
    int32           opcode,
    int32           flags,
    pin_flist_t     *i_flistp,
    pin_flist_t     **r_flistpp,
    pin_errbuf_t    *ebufp);

static void
fm_rbs_cust_acct_profile_update(
        pcm_context_t           *ctxp,
        pin_flist_t             *i_flistp,
        pin_flist_t             **r_flistp,
        pin_errbuf_t            *ebufp);

/*****************************************
*FUNCTION IMPLIMENTATIONS
*****************************************/
void
op_rbs_cust_acct_profile_update(
 cm_nap_connection_t     *connp,
        int32                   opcode,
        int32                   flags,
        pin_flist_t             *i_flistp,
        pin_flist_t             **r_flistpp,
        pin_errbuf_t            *ebufp)
{
        pcm_context_t           *ctxp = connp->dm_ctx;
        pin_flist_t             *r_flistp = NULL;
        *r_flistpp              = NULL;

        if (PIN_ERRBUF_IS_ERR(ebufp)) {
                return;
        }
        PIN_ERRBUF_CLEAR(ebufp);

        /*******************************************************************
         * Insanity Check
         *******************************************************************/
        if (opcode != PCM_OP_RBS_CUST_ACCT_PROFILE_UPDATE ) {
                pin_set_err(ebufp, PIN_ERRLOC_FM,
                        PIN_ERRCLASS_SYSTEM_DETERMINATE,
                        PIN_ERR_BAD_OPCODE, 0, 0, opcode);
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                        "op_rbs_cust_acct_profile_update",
                        ebufp);
                return;
        }

        /*******************************************************************
         * Debug: Input flist
        *******************************************************************/
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                "op_rbs_cust_acct_profile_update input flist", i_flistp);

        /*******************************************************************
         * Call the default implementation
         *******************************************************************/

        fm_rbs_cust_acct_profile_update(ctxp, i_flistp, &r_flistp, ebufp);

        /***********************************************************
         * Results.
         ***********************************************************/

        if (PIN_ERRBUF_IS_ERR(ebufp))
        {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"op_rbs_cust_acct_profile_update error", ebufp);
                PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                        "op_rbs_cust_acct_profile_update input flist", i_flistp);
                PIN_ERRBUF_RESET(ebufp);
                *r_flistpp = PIN_FLIST_COPY(i_flistp, ebufp);
        }
        else
        {
                *r_flistpp = PIN_FLIST_COPY(r_flistp, ebufp);
                PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "op_rbs_cust_acct_profile_update output flist", *r_flistpp);
        }

        PIN_FLIST_DESTROY_EX(&r_flistp, NULL);
        return;
}
void
fm_rbs_cust_acct_profile_update(
        pcm_context_t           *ctxp,
        pin_flist_t             *i_flistp,
        pin_flist_t             **r_flistp,
        pin_errbuf_t            *ebufp)
{
        pin_flist_t              *in_flistp = NULL;
        pin_flist_t              *ret_flistp = NULL;
	pin_flist_t              *inher_info_get_flistp = NULL;
        pin_flist_t              *inher_info_set_flistp = NULL;
	pin_flist_t              *acc_get_flistp = NULL;
	pin_flist_t              *acc_set_flistp = NULL;
	poid_t                   *prof_acc_pd = NULL;

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "start fm_rbs_cust_acct_profile_update input_flist", i_flistp);
	in_flistp = PIN_FLIST_CREATE(ebufp);

	prof_acc_pd = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp);
	PIN_FLIST_FLD_SET(in_flistp, PIN_FLD_POID, prof_acc_pd, ebufp);

	inher_info_get_flistp = PIN_FLIST_SUBSTR_GET(i_flistp, PIN_FLD_INHERITED_INFO, 0, ebufp);
	acc_get_flistp = PIN_FLIST_SUBSTR_GET(inher_info_get_flistp, PIN_FLD_ACCOUNT, 0, ebufp);

	inher_info_set_flistp = PIN_FLIST_SUBSTR_ADD(in_flistp, PIN_FLD_INHERITED_INFO, ebufp);
	acc_set_flistp = PIN_FLIST_SUBSTR_ADD(inher_info_set_flistp, PIN_FLD_ACCOUNT, ebufp);
	
	if (PIN_FLIST_FLD_GET(acc_get_flistp, MRB_FLD_CUSTOMER_SEGMENT, 1, ebufp) && PIN_FLIST_FLD_GET(acc_get_flistp, MRB_FLD_CUSTOMER_SEGMENT, 1, ebufp) != NULL)
	{
		PIN_FLIST_FLD_COPY(acc_get_flistp, MRB_FLD_CUSTOMER_SEGMENT, acc_set_flistp, MRB_FLD_CUSTOMER_SEGMENT, ebufp);
	}
	if (PIN_FLIST_FLD_GET(acc_get_flistp, MRB_FLD_PAY_METHOD_KEY, 1, ebufp) && PIN_FLIST_FLD_GET(acc_get_flistp, MRB_FLD_PAY_METHOD_KEY, 1, ebufp) != NULL)
        {
		PIN_FLIST_FLD_COPY(acc_get_flistp, MRB_FLD_PAY_METHOD_KEY, acc_set_flistp, MRB_FLD_PAY_METHOD_KEY, ebufp);
	}
	if (PIN_FLIST_FLD_GET(acc_get_flistp, MRB_FLD_CC_REJ_CNT, 1, ebufp) && PIN_FLIST_FLD_GET(acc_get_flistp, MRB_FLD_CC_REJ_CNT, 1, ebufp) != NULL)
        {
		PIN_FLIST_FLD_COPY(acc_get_flistp, MRB_FLD_CC_REJ_CNT, acc_set_flistp, MRB_FLD_CC_REJ_CNT, ebufp);
	}
	if (PIN_FLIST_FLD_GET(acc_get_flistp, MRB_FLD_DD_REJ_CNT, 1, ebufp) && PIN_FLIST_FLD_GET(acc_get_flistp, MRB_FLD_DD_REJ_CNT, 1, ebufp) != NULL)
        {
		PIN_FLIST_FLD_COPY(acc_get_flistp, MRB_FLD_DD_REJ_CNT, acc_set_flistp, MRB_FLD_DD_REJ_CNT, ebufp);
	}
	if (PIN_FLIST_FLD_GET(acc_get_flistp, MRB_FLD_ID_TYPE, 1, ebufp) && PIN_FLIST_FLD_GET(acc_get_flistp, MRB_FLD_ID_TYPE, 1, ebufp) != NULL)
        {
		PIN_FLIST_FLD_COPY(acc_get_flistp, MRB_FLD_ID_TYPE, acc_set_flistp, MRB_FLD_ID_TYPE, ebufp);
	}
	if (PIN_FLIST_FLD_GET(acc_get_flistp, MRB_FLD_SUBSCRIBER_SEGMENT, 1, ebufp) && PIN_FLIST_FLD_GET(acc_get_flistp, MRB_FLD_SUBSCRIBER_SEGMENT, 1, ebufp) != NULL)
        {
		PIN_FLIST_FLD_COPY(acc_get_flistp, MRB_FLD_SUBSCRIBER_SEGMENT, acc_set_flistp, MRB_FLD_SUBSCRIBER_SEGMENT, ebufp);
	}
	
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "end fm_rbs_cust_acct_profile_update input_flist", in_flistp);
	PCM_OP(ctxp, PCM_OP_CUST_MODIFY_PROFILE, 0, in_flistp, &ret_flistp, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
	        PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "fm_rbs_cust_acct_profile_update error after excecution", ebufp);
	        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_rbs_cust_acct_profile_update input flist", in_flistp);
	        PIN_ERRBUF_RESET(ebufp);
		goto cleanup;
	}

	*r_flistp = PIN_FLIST_COPY(ret_flistp, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_rbs_cust_acct_profile_update return_flist", *r_flistp);
	return;

cleanup:
	*r_flistp = PIN_FLIST_COPY(i_flistp, ebufp);
	PIN_FLIST_DESTROY_EX(&ret_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&in_flistp, NULL);
	return;
}
          	
