#ifndef lint
static const char Sccs_id[] = "@(#)%Portal Version: fm_rbs_cust_swap_handset_rebate_purchase.c :BillingVelocityInt:3:2006-Sep-05 21:55:03 %";
#endif

#include <stdio.h>
#include <stdlib.h>

#include "pcm.h"
#include "ops/cust.h"
#include "cm_fm.h"
#include "pin_errs.h"
#include "pin_cust.h"
#include "pin_bill.h"
#include "pinlog.h"
#include "pin_currency.h"
#include "psiu_currency.h"
#include "pin_os_string.h"
#include "pin_type.h"
#include "custom_flds/custom_flds.h"
#include "custom_wrapper_ops.h"

/*******************************************************************
 * Routines contained herein.
 *******************************************************************/
EXPORT_OP void
op_rbs_cust_swap_handset_rebate_purchase(
        cm_nap_connection_t     *connp,
        int32                   opcode,
        int32                   flags,
        pin_flist_t             *i_flistp,
        pin_flist_t             **r_flistpp,
        pin_errbuf_t            *ebufp);

static void
fm_rbs_cust_swap_handset_rebate_purchase(
         pcm_context_t           *ctxp,
         pin_flist_t             *i_flistp,
         pin_flist_t             **r_flistpp,
         pin_errbuf_t            *ebufp);

static void
fm_get_swap_rebate_info(
         pcm_context_t           *ctxp,
         pin_flist_t             *i_flistp,
         pin_flist_t             **r_flistpp,
         pin_errbuf_t            *ebufp);

/*********************************************************************
* Function Implementations
*********************************************************************/

void
op_rbs_cust_swap_handset_rebate_purchase(
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
        if (opcode != PCM_OP_RBS_CUST_SWAP_HANDSET_REBATE_PURCHASE) 
	{
                pin_set_err(ebufp, PIN_ERRLOC_FM,
                        PIN_ERRCLASS_SYSTEM_DETERMINATE,
                        PIN_ERR_BAD_OPCODE, 0, 0, opcode);

                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                        "bad opcode in op_rbs_cust_swap_handset_rebate_purchase", ebufp);
                return;
        }

        /***********************************************************
         * Debug: What did we get?
         ***********************************************************/
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                "op_cust_purchase_handset_rebate input flist", i_flistp);

        /*******************************************************************
         * Call the default implementation
         *******************************************************************/

                fm_rbs_cust_swap_handset_rebate_purchase(ctxp ,i_flistp ,&r_flistp ,ebufp );


/***********************************************************
        ** Results.
 ************************************************************/

    if (PIN_ERRBUF_IS_ERR(ebufp))
    {
        PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"op_mso_cust_get_accinfo error", ebufp);
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "op_rbs_cust_swap_handset_rebate_purchase_sub input flist", i_flistp);
                PIN_ERRBUF_RESET(ebufp);
        *r_flistpp = PIN_FLIST_COPY(i_flistp, ebufp);
    }
    else
    {
         *r_flistpp = PIN_FLIST_COPY(r_flistp, ebufp);
         PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,  "op_rbs_cust_swap_handset_rebate_purchase_sub", *r_flistpp);

    }

        PIN_FLIST_DESTROY_EX(&r_flistp, NULL);
        return;
}
void
fm_rbs_cust_swap_handset_rebate_purchase(
        pcm_context_t           *ctxp,
        pin_flist_t             *i_flistp,
        pin_flist_t             **r_flistp,
        pin_errbuf_t            *ebufp)
{
	pin_flist_t             *in_flistp = NULL;
        pin_flist_t             *ret_flistp = NULL;
        pin_flist_t             *srvc_inflistp = NULL;
        pin_flist_t             *srvc_outflistp = NULL;
        pin_flist_t             *inp_flistp = NULL;
        pin_flist_t             *out_flistp = NULL;
        pin_flist_t             *res_flistp = NULL;
        pin_flist_t             *result_flistp = NULL;
        pin_flist_t             *prod_flistp = NULL;
        pin_flist_t             *prod_set_flistp = NULL;
        pin_flist_t             *deal_infop  = NULL;
	pin_flist_t             *search_flistp = NULL;
	pin_flist_t             *search_oflistp = NULL;
	pin_flist_t		*rlt_flistp = NULL;
        poid_t                  *acc_pd = NULL;
        poid_t                  *deal_pd = NULL;
        poid_t                  *pdp = NULL;
        char                    *deal_name = NULL;
        char                    *permitted = NULL;
	char			*line_id = NULL;
        int64                   deal_id = NULL;
        int64                   db = -1;
        int32                   cnt = 0;
        int32                   elem_id = 0;
	int32			s_status = 0;
        pin_cookie_t            cookie = NULL;
        time_t                  now_t = 0;
        char                    msg[100];

	PIN_ERR_LOG_FLIST(3, "start fm_rbs_cust_swap_handset_rebate_purchase input flist", i_flistp);
        pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp);
        deal_name = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_DEAL_NAME, 0, ebufp);
        in_flistp = PIN_FLIST_CREATE(ebufp);
        
	PIN_FLIST_FLD_SET(in_flistp, PIN_FLD_POID, pdp, ebufp);
	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_PROGRAM_NAME, in_flistp, PIN_FLD_PROGRAM_NAME, ebufp);
	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_AMOUNT, in_flistp, PIN_FLD_AMOUNT, ebufp);
        now_t = pin_virtual_time((time_t *)NULL);
        PIN_FLIST_FLD_SET(in_flistp, PIN_FLD_START_T, &now_t, ebufp);
        PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_END_T, in_flistp, PIN_FLD_END_T, ebufp);
	line_id = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_LOGIN, 0, ebufp);
        if(deal_name)
        {
                inp_flistp = PIN_FLIST_CREATE(ebufp);
                PIN_FLIST_FLD_SET(inp_flistp, PIN_FLD_POID, pdp, ebufp);
                PIN_FLIST_FLD_SET(inp_flistp, PIN_FLD_NAME, deal_name, ebufp);
                PIN_ERR_LOG_FLIST(3, "fm_get_deal_from_name flistp debug", inp_flistp);
                fm_get_deal_from_name(ctxp, inp_flistp, &out_flistp, ebufp);
                if (PIN_ERRBUF_IS_ERR(ebufp))
                {
                        PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "fm_rbs_cust_swap_handset_rebate_purchase error after excecution", ebufp);
                        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_rbs_cust_swap_handset_rebate_purchase input flist", ret_flistp);
                        PIN_ERRBUF_RESET(ebufp);
                        goto CLEANUP;
                }
                PIN_ERR_LOG_FLIST(3, "fm_get_deal_from_name output flist: ", out_flistp);
                result_flistp = PIN_FLIST_ELEM_GET(out_flistp, PIN_FLD_RESULTS, 0, 1, ebufp);
                deal_pd = PIN_FLIST_FLD_GET(result_flistp, PIN_FLD_POID, 0, ebufp);
                permitted = PIN_FLIST_FLD_GET(result_flistp, PIN_FLD_PERMITTED, 0, ebufp);
        }
        if (permitted && pdp)
        {
                srvc_inflistp = PIN_FLIST_CREATE(ebufp);
                db = PIN_POID_GET_DB(pdp);
                PIN_FLIST_FLD_SET(srvc_inflistp, PIN_FLD_POID, PIN_POID_CREATE(db, permitted, -1, ebufp), ebufp);
                PIN_FLIST_FLD_SET(srvc_inflistp, PIN_FLD_LOGIN, line_id, ebufp);
		PIN_FLIST_FLD_SET(srvc_inflistp, PIN_FLD_ACCOUNT_OBJ, pdp, ebufp);

                PIN_ERR_LOG_FLIST(3, "fm_get_service_info input: ", srvc_inflistp);
                fm_get_service_info(ctxp, srvc_inflistp, &srvc_outflistp, ebufp);
                PIN_ERR_LOG_FLIST(3, "fm_get_service_info output: ", srvc_outflistp);

                res_flistp = PIN_FLIST_ELEM_GET(srvc_outflistp, PIN_FLD_RESULTS, 0, 1, ebufp);
                PIN_FLIST_FLD_COPY(res_flistp, PIN_FLD_POID, in_flistp, PIN_FLD_SERVICE_OBJ, ebufp);
                PIN_FLIST_FLD_COPY(res_flistp, PIN_FLD_ACCOUNT_OBJ, in_flistp, PIN_FLD_POID, ebufp);
		// line_id = PIN_FLIST_FLD_GET(res_flistp, PIN_FLD_LOGIN, 0, ebufp);
        }

        deal_id = PIN_POID_GET_ID(deal_pd);
        PIN_ERR_LOG_POID(3, "deal poid deal_pd: ", deal_pd);
        sprintf(msg, "deal_id %d", deal_id);
        PIN_ERR_LOG_MSG(3, msg);

        if (deal_id != -1 || deal_pd != NULL)
        {
                deal_infop = PIN_FLIST_SUBSTR_ADD(in_flistp, PIN_FLD_DEAL_INFO, ebufp);
                PIN_FLIST_FLD_COPY(result_flistp, PIN_FLD_POID, deal_infop, PIN_FLD_POID, ebufp);
                PIN_FLIST_FLD_COPY(result_flistp, PIN_FLD_NAME, deal_infop, PIN_FLD_NAME, ebufp);
                PIN_FLIST_FLD_COPY(result_flistp, PIN_FLD_DESCR, deal_infop, PIN_FLD_DESCR, ebufp);
                PIN_FLIST_FLD_COPY(result_flistp, PIN_FLD_START_T, deal_infop, PIN_FLD_START_T, ebufp);
                PIN_FLIST_FLD_COPY(result_flistp, PIN_FLD_END_T, deal_infop, PIN_FLD_END_T, ebufp);
                while ((prod_flistp = PIN_FLIST_ELEM_GET_NEXT(result_flistp, PIN_FLD_PRODUCTS, &elem_id, 1, &cookie, ebufp)) != NULL)
                {
                        prod_set_flistp = PIN_FLIST_ELEM_ADD(deal_infop, PIN_FLD_PRODUCTS, cnt, ebufp);
                        PIN_FLIST_FLD_COPY(prod_flistp, PIN_FLD_CYCLE_DISCOUNT, prod_set_flistp, PIN_FLD_CYCLE_DISCOUNT, ebufp);
                        PIN_FLIST_FLD_COPY(prod_flistp, PIN_FLD_CYCLE_END_DETAILS, prod_set_flistp, PIN_FLD_CYCLE_END_DETAILS, ebufp);
                        PIN_FLIST_FLD_COPY(prod_flistp, PIN_FLD_CYCLE_END_T, prod_set_flistp, PIN_FLD_CYCLE_END_T, ebufp);
                        PIN_FLIST_FLD_COPY(prod_flistp, PIN_FLD_CYCLE_START_DETAILS, prod_set_flistp, PIN_FLD_CYCLE_START_DETAILS, ebufp);
                        PIN_FLIST_FLD_COPY(prod_flistp, PIN_FLD_CYCLE_START_T, prod_set_flistp, PIN_FLD_CYCLE_START_T, ebufp);
                        PIN_FLIST_FLD_COPY(prod_flistp, PIN_FLD_PRODUCT_OBJ, prod_set_flistp,PIN_FLD_PRODUCT_OBJ, ebufp);
                        PIN_FLIST_FLD_COPY(prod_flistp, PIN_FLD_PURCHASE_DISCOUNT, prod_set_flistp, PIN_FLD_PURCHASE_DISCOUNT, ebufp);
                        PIN_FLIST_FLD_COPY(prod_flistp, PIN_FLD_PURCHASE_END_DETAILS, prod_set_flistp, PIN_FLD_PURCHASE_END_DETAILS, ebufp);
                        PIN_FLIST_FLD_COPY(prod_flistp, PIN_FLD_PURCHASE_END_T, prod_set_flistp, PIN_FLD_PURCHASE_END_T, ebufp);
                        PIN_FLIST_FLD_COPY(prod_flistp, PIN_FLD_PURCHASE_START_DETAILS, prod_set_flistp, PIN_FLD_PURCHASE_START_DETAILS, ebufp);
                        PIN_FLIST_FLD_COPY(prod_flistp, PIN_FLD_PURCHASE_START_T, prod_set_flistp, PIN_FLD_PURCHASE_START_T, ebufp);
                        PIN_FLIST_FLD_COPY(prod_flistp, PIN_FLD_QUANTITY, prod_set_flistp, PIN_FLD_QUANTITY, ebufp);
                        PIN_FLIST_FLD_COPY(prod_flistp, PIN_FLD_STATUS, prod_set_flistp, PIN_FLD_STATUS, ebufp);
                        PIN_FLIST_FLD_COPY(prod_flistp, PIN_FLD_STATUS_FLAGS, prod_set_flistp, PIN_FLD_STATUS_FLAGS, ebufp);
                        PIN_FLIST_FLD_COPY(prod_flistp, PIN_FLD_USAGE_DISCOUNT, prod_set_flistp, PIN_FLD_USAGE_DISCOUNT, ebufp);
                        PIN_FLIST_FLD_COPY(prod_flistp, PIN_FLD_USAGE_END_DETAILS, prod_set_flistp, PIN_FLD_USAGE_END_DETAILS, ebufp);
                        PIN_FLIST_FLD_COPY(prod_flistp, PIN_FLD_USAGE_END_T, prod_set_flistp, PIN_FLD_USAGE_END_T , ebufp);
                        PIN_FLIST_FLD_COPY(prod_flistp, PIN_FLD_USAGE_START_DETAILS, prod_set_flistp, PIN_FLD_USAGE_START_DETAILS , ebufp);
                        PIN_FLIST_FLD_COPY(prod_flistp, PIN_FLD_USAGE_START_T, prod_set_flistp, PIN_FLD_USAGE_START_T, ebufp);
			
			if(line_id)
			{	
				sprintf(msg, "L_%s", line_id);
				PIN_FLIST_FLD_SET(prod_set_flistp, PIN_FLD_DESCR, msg, ebufp);
				PIN_ERR_LOG_MSG(3, msg);
			}
                        cnt++;
                }
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "end fm_rbs_cust_swap_handset_rebate_purchase input_flist", in_flistp);

	PCM_OP(ctxp, PCM_OP_MRB_POL_REBATE_PURCHASE, 0, in_flistp, &ret_flistp, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_rbs_cust_swap_handset_rebate_purchase return_flist", ret_flistp);

	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
	        PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"fm_rbs_cust_swap_handset_rebate_purchase error after excecution", ebufp);
	        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_rbs_cust_swap_handset_rebate_purchase input flist", in_flistp);
	        PIN_ERRBUF_RESET(ebufp);
		goto CLEANUP;
	}
	search_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_COPY(ret_flistp, PIN_FLD_POID, search_flistp, PIN_FLD_POID, ebufp);
	PIN_FLIST_FLD_COPY(ret_flistp, PIN_FLD_START_T, search_flistp, PIN_FLD_MOD_T, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_get_swap_rebate_info input_flist", search_flistp);
        fm_get_swap_rebate_info(ctxp, search_flistp, &search_oflistp, ebufp);
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_get_swap_rebate_info return_flist", search_oflistp);
	rlt_flistp = PIN_FLIST_ELEM_GET(search_oflistp, PIN_FLD_RESULTS, 0, 1, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp))
        {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"fm_rbs_cust_swap_handset_rebate_purchase error after excecution", ebufp);
                PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_rbs_cust_swap_handset_rebate_purchase input flist", in_flistp);
                PIN_ERRBUF_RESET(ebufp);
		goto CLEANUP;
        }
	PIN_FLIST_DESTROY_EX(&ret_flistp, ebufp);
	ret_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_COPY(rlt_flistp, PIN_FLD_POID, ret_flistp, PIN_FLD_POID, ebufp);
	PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &s_status, ebufp);
	*r_flistp = PIN_FLIST_COPY(ret_flistp, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,  "fm_rbs_cust_swap_handset_rebate_purchase output flist", *r_flistp);
	return;

CLEANUP:
	*r_flistp = PIN_FLIST_COPY(ret_flistp, ebufp);	
	PIN_FLIST_DESTROY_EX(&i_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&search_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&in_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&res_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&inp_flistp, NULL);
return;
}

static void
fm_get_swap_rebate_info(
	pcm_context_t           *ctxp,
	pin_flist_t             *i_flistp,
	pin_flist_t             **r_flistp,
	pin_errbuf_t            *ebufp)
{
	pin_flist_t		*input_flistp = NULL;
	pin_flist_t		*return_flistp = NULL;
	pin_flist_t		*search_iflistp  = NULL;
	pin_flist_t		*args_flistp = NULL;
	pin_flist_t		*args1_flistp = NULL;
	pin_flist_t		*result_flistp = NULL;
	char			*acct_obj = NULL;
	char			*mod_t = NULL;
	int64			db = -1;
	poid_t			*search_pdp = NULL;
	poid_t			*pdp = NULL;
	int32			flags = 256;
	char			*template = "select x from /swap_rebate where F1 = V1 and F2 = V2";
	
	if (PIN_ERRBUF_IS_ERR(ebufp))
    	return;
	
	PIN_ERRBUF_CLEAR(ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_search_input_flist", i_flistp);
	
	pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp);
	db = PIN_POID_GET_DB(pdp);
	search_pdp = (poid_t *)PIN_POID_CREATE(db, "/search", -1, ebufp);

	search_iflistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_SET(search_iflistp, PIN_FLD_POID, search_pdp, ebufp);
	PIN_FLIST_FLD_SET(search_iflistp, PIN_FLD_FLAGS, &flags, ebufp);
    	PIN_FLIST_FLD_SET(search_iflistp, PIN_FLD_TEMPLATE, template, ebufp);

	args_flistp = PIN_FLIST_ELEM_ADD(search_iflistp, PIN_FLD_ARGS, 1, ebufp);
   	 PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_ACCOUNT_OBJ, pdp, ebufp);
	
	mod_t = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_MOD_T, 0, ebufp);
	args1_flistp = PIN_FLIST_ELEM_ADD(search_iflistp, PIN_FLD_ARGS, 2, ebufp);
	PIN_FLIST_FLD_SET(args1_flistp, PIN_FLD_MOD_T, mod_t, ebufp);
	
	result_flistp = PIN_FLIST_ELEM_ADD(search_iflistp, PIN_FLD_RESULTS, 0, ebufp);
	PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_POID, NULL, ebufp);
	
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_search_input_flist1", search_iflistp);
	
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, search_iflistp, &return_flistp, ebufp);
	
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_search_return_flist1", return_flistp);
	 
	*r_flistp = PIN_FLIST_COPY(return_flistp,ebufp);
	PIN_FLIST_DESTROY_EX(&i_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&search_iflistp, NULL);
	PIN_FLIST_DESTROY_EX(&return_flistp, NULL);
	return;
}
