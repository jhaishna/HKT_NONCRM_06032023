#ifndef lint

static  char Sccs_Id[] = "@(#)%Portal Version: fm_rbs_cust_cancel_service.c: BillingVelocityInt:4:2006-Sep-05 04:28:17 %";
#endif

#include <stdio.h>
#include <stdlib.h>
#include <pin_flds.h>
#include "pcm.h"
#include "ops/cust.h"
#include "ops/mrb.h"
#include "cm_fm.h"
#include "pin_errs.h"
#include "pin_cust.h"
#include "pin_bill.h"
#include "pinlog.h"
#include "pin_currency.h"
#include "psiu_currency.h"
#include "custom_flds/custom_flds.h"
#include "custom_wrapper_ops.h"

/*******************************************************************
 * Routines contained herein.
 *******************************************************************/
EXPORT_OP void
op_rbs_cust_cancel_service(
        cm_nap_connection_t     *connp,
        int32                   opcode,
        int32                   flags,
        pin_flist_t             *i_flistp,
        pin_flist_t             **r_flistpp,
        pin_errbuf_t            *ebufp);
		

static void
fm_rbs_cust_cancel_service (
         pcm_context_t           *ctxp,
         pin_flist_t             *i_flistp,
         pin_flist_t             **r_flistpp,
         pin_errbuf_t            *ebufp);

/*****************************************
*FUNCTION IMPLIMENTATIONS
*****************************************/
void
op_rbs_cust_cancel_service (
        cm_nap_connection_t     *connp,
        int                     opcode,
        int                     flags,
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

        /***********************************************************
         * Insanity check.
         ***********************************************************/
        if (opcode != PCM_OP_RBS_CUST_CANCEL_SERVICE) {
                pin_set_err(ebufp, PIN_ERRLOC_FM,
                        PIN_ERRCLASS_SYSTEM_DETERMINATE,
                        PIN_ERR_BAD_OPCODE, 0, 0, opcode);
						
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                        "bad opcode in op_rbs_cust_cancel_service", ebufp);
                return;
        }

        /***********************************************************
         * Debug: What did we get?
         ***********************************************************/
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                "op_rbs_cust_cancel_service input flist", i_flistp);

	/*******************************************************************
        * Call the default implementation
        *******************************************************************/	 
	fm_rbs_cust_cancel_service(ctxp, i_flistp, &r_flistp, ebufp );	
	/***********************************************************
	** Results.
	************************************************************/

    	if (PIN_ERRBUF_IS_ERR(ebufp))
    	{
    		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"op_rbs_cust_cancel_service error", ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "op_rbs_cust_cancel_service input flist", i_flistp);
        	PIN_ERRBUF_RESET(ebufp);
        	*r_flistpp = PIN_FLIST_COPY(i_flistp, ebufp);
    	}
    	else
    	{
		*r_flistpp = PIN_FLIST_COPY(r_flistp, ebufp);
   		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,  "op_rbs_cust_cancel_service output flist", *r_flistpp);
    	}
	PIN_FLIST_DESTROY_EX(&r_flistp, NULL);	
	return;
}

static void
fm_rbs_cust_cancel_service (
        pcm_context_t   *ctxp,
        pin_flist_t     *i_flistp,
        pin_flist_t     **r_flistp,
	pin_errbuf_t    *ebufp)
{
	pin_flist_t     *in_flistp = NULL;
	pin_flist_t	*ret_flistp = NULL;
	pin_flist_t     *deal_get_flistp = NULL;
	pin_flist_t     *deal_set_flistp = NULL;
	pin_flist_t	*search_i_flistp = NULL;
	pin_flist_t	*search_o_flistp = NULL;
	pin_flist_t	*res_flistp = NULL;	
	char            *login_id = NULL;
	poid_t          *ser_pdp = NULL;
	poid_t          *deal_pdp = NULL;
	poid_t		*pdp = NULL;
	time_t		now_t = 0;
	int64 		db = -1;
	int32		elem_id = 0;
	pin_cookie_t	cookie = NULL;

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "start fm_rbs_cust_cancel_service input_flist", i_flistp);

	in_flistp = PIN_FLIST_CREATE(ebufp);

//	login_id = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_LOGIN, 0, ebufp);
	pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp);
	ser_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_SERVICE_OBJ, 0, ebufp); 

	PIN_FLIST_FLD_SET(in_flistp, PIN_FLD_LOGIN, login_id, ebufp);
	PIN_FLIST_FLD_SET(in_flistp, PIN_FLD_POID, pdp, ebufp);
	PIN_FLIST_FLD_SET(in_flistp, PIN_FLD_SERVICE_OBJ, ser_pdp, ebufp);

	now_t = pin_virtual_time((time_t *)NULL);
	PIN_FLIST_FLD_SET(in_flistp, PIN_FLD_START_T, &now_t, ebufp);
	PIN_FLIST_FLD_COPY(i_flistp,PIN_FLD_END_T, in_flistp, PIN_FLD_END_T, ebufp);
	PIN_FLIST_FLD_COPY(i_flistp,PIN_FLD_PROGRAM_NAME, in_flistp, PIN_FLD_PROGRAM_NAME, ebufp);
	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_DESCR, in_flistp, PIN_FLD_DESCR, ebufp);
	
	while ((deal_get_flistp = PIN_FLIST_ELEM_GET_NEXT(i_flistp, PIN_FLD_DEALS, &elem_id, 1, &cookie, ebufp)) != NULL)
	{
		deal_set_flistp = PIN_FLIST_SUBSTR_ADD(in_flistp, PIN_FLD_DEAL_INFO, ebufp);
		deal_pdp = PIN_FLIST_FLD_GET(deal_get_flistp, PIN_FLD_DEAL_OBJ, 0, ebufp);
		PIN_FLIST_FLD_SET(deal_set_flistp, PIN_FLD_DEAL_OBJ, deal_pdp, ebufp);
		PIN_FLIST_FLD_COPY(deal_get_flistp, PIN_FLD_PACKAGE_ID, deal_set_flistp, PIN_FLD_PACKAGE_ID, ebufp);

		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "end fm_rbs_cust_cancel_service input_flist", in_flistp);
		PCM_OP(ctxp, PCM_OP_MRB_POL_SUBSCRIPTION_CANCEL, 0, in_flistp, &ret_flistp, ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp))
        	{
        	        PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"fm_rbs_cust_cancel_service error", ebufp);
        	        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "error fm_mrb_pol_subscription_cancel flist", ret_flistp);
        	        PIN_ERRBUF_RESET(ebufp);
        		return;
        	}
	
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_rbs_cust_cancel_service return_flist", ret_flistp);
	}
	*r_flistp = PIN_FLIST_COPY(ret_flistp, ebufp);	
	return;
cleanup:
	*r_flistp = PIN_FLIST_COPY(i_flistp, ebufp);
	PIN_FLIST_DESTROY_EX(&in_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&ret_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&search_o_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&search_i_flistp, NULL);
	return;
}
