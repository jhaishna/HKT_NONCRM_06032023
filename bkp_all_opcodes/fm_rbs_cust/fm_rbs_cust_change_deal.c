#ifndef lint

static  char Sccs_Id[] = "@(#)%Portal Version: fm_rbs_cust_change_deal.c: BillingVelocityInt:4:2006-Sep-05 04:28:17 %";
#endif

#include <stdio.h>
#include <stdlib.h>
#include <pin_flds.h>
#include "pcm.h"
#include "ops/cust.h"
#include "ops/mrb.h"
#include "ops/rbs.h"
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
op_rbs_cust_change_deal(
        cm_nap_connection_t     *connp,
        int32                   opcode,
        int32                   flags,
        pin_flist_t             *i_flistp,
        pin_flist_t             **r_flistpp,
        pin_errbuf_t            *ebufp);


static void
fm_rbs_cust_change_deal (
         pcm_context_t           *ctxp,
         pin_flist_t             *i_flistp,
         pin_flist_t             **r_flistpp,
         pin_errbuf_t            *ebufp);


/*****************************************
*FUNCTION IMPLIMENTATIONS
*****************************************/
void
op_rbs_cust_change_deal (
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
        if (opcode != PCM_OP_RBS_CUST_CHANGE_DEAL) {
                pin_set_err(ebufp, PIN_ERRLOC_FM,
                        PIN_ERRCLASS_SYSTEM_DETERMINATE,
                        PIN_ERR_BAD_OPCODE, 0, 0, opcode);

                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                        "bad opcode in op_rbs_cust_change_deal", ebufp);
                return;
        }

        /***********************************************************
         * Debug: What did we get?
         ***********************************************************/
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                "op_rbs_cust_change_deal input flist", i_flistp);

	 /*******************************************************************
        * Call the default implementation
        *******************************************************************/
        fm_rbs_cust_change_deal(ctxp, i_flistp, &r_flistp, ebufp );

        /***********************************************************
        ** Results.
        ************************************************************/

        if (PIN_ERRBUF_IS_ERR(ebufp))
        {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"op_rbs_cust_change_deal error", ebufp);
                PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "op_rbs_cust_change_deal input flist", i_flistp);
                PIN_ERRBUF_RESET(ebufp);
                *r_flistpp = PIN_FLIST_COPY(i_flistp, ebufp);
        }
        else
        {
                *r_flistpp = PIN_FLIST_COPY(r_flistp, ebufp);
                PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,  "op_rbs_cust_change_deal output flist", *r_flistpp);
        }
        PIN_FLIST_DESTROY_EX(&r_flistp, NULL);
        return;
}

static void
fm_rbs_cust_change_deal (
        pcm_context_t   *ctxp,
        pin_flist_t     *i_flistp,
        pin_flist_t     **r_flistp,
        pin_errbuf_t    *ebufp)
{

        pin_flist_t     *ret_flistp = NULL;
	pin_flist_t     *args_flistp = NULL;
        pin_flist_t     *deal_info = NULL;
	pin_flist_t     *deal_info_flistp = NULL;
	pin_flist_t     *p_deals = NULL;
        pin_flist_t     *res_flistp = NULL;
	pin_flist_t     *res_get_flistp = NULL;
	pin_flist_t     *rs_flistp = NULL;
	pin_flist_t     *canc_flistp = NULL;
	pin_flist_t     *s_flistp = NULL;
	pin_flist_t     *search_iflistp = NULL;
	pin_flist_t     *seach_oflistp = NULL;
	pin_flist_t     *ser_iflistp = NULL;
	pin_flist_t     *ser_oflistp = NULL;
	pin_flist_t     *ser_rflistp = NULL;
	pin_flist_t     *so_flistp = NULL;
	pin_flist_t     *po_flistp = NULL;
	pin_flist_t     *canc_iflistp = NULL;
	pin_flist_t     *canc_oflistp = NULL;
	pin_flist_t     *canc_deal_get_flist = NULL;
	pin_flist_t     *c_res_flistp = NULL;
	pin_flist_t     *pur_iflistp = NULL;
	pin_flist_t     *pur_deal_info = NULL;
	pin_flist_t     *prod_flistp = NULL;
	pin_flist_t     *prod_flistp1 = NULL;
	pin_flist_t     *disc_flistp = NULL;
	pin_flist_t     *disc1_flistp = NULL;
	pin_flist_t     *sr_flistp = NULL;
	poid_t		*search_pdp = NULL;
        poid_t          *ser_obj = NULL;
        poid_t          *deal_pdp = NULL;
	poid_t          *d_pdp = NULL;
	poid_t          *deal_obj = NULL;
	poid_t          *acc_obj = NULL;
	poid_t          *acc_robj =NULL;
        poid_t          *pdp = NULL;
        time_t          now_t = 0;
	time_t		*end_t = NULL;
        int64           db = -1;
	int32           *pkg_id = NULL;
        int32           elem_id = 0;
	int32           elem_id1 = 0;
	int32           elem_id2 = 0;
	int32		cnt = 0;
	int32           cnt1 = 0;
	int32		flags = 256;
	char		*ser_type = NULL;
	char		*descr = NULL;
	char            *d_descr = NULL;
	char		*deal_name = NULL;
	char            *d_name = NULL;
	char            *permitted = NULL;
	char		*templatep = "select * from /purchased_product 1, /deal 2, /account 3 where 1.F1 = 2.F2 and 1.F3 = 3.F4 and 3.F5 = V5 and 2.F6 = V6";
        pin_cookie_t    cookie = NULL;
	pin_cookie_t    cookie1 = NULL;
	pin_cookie_t    cookie2 = NULL;


	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "start fm_rbs_cust_change_deal input_flist: ", i_flistp);

	pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp);
	descr = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_DESCR, 0, ebufp);
	db = PIN_POID_GET_DB(pdp);
	search_pdp = PIN_POID_CREATE(db, "/search", -1, ebufp);

	now_t = pin_virtual_time((time_t *)NULL);
	end_t = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_END_T, 0, ebufp);

	ret_flistp = PIN_FLIST_CREATE(ebufp);

	canc_deal_get_flist = PIN_FLIST_SUBSTR_GET(i_flistp, PIN_FLD_CANCEL_DEAL_INFO, 0, ebufp);
	elem_id = 0;
	cookie = 0;
	while ((deal_info = PIN_FLIST_ELEM_GET_NEXT(canc_deal_get_flist, PIN_FLD_DEALS, &elem_id, 1, &cookie, ebufp)) != NULL)
	{
		search_iflistp = PIN_FLIST_CREATE(ebufp);
        	PIN_FLIST_FLD_SET(search_iflistp, PIN_FLD_POID, search_pdp, ebufp);
		PIN_FLIST_FLD_SET(search_iflistp, PIN_FLD_FLAGS, &flags, ebufp);	
		PIN_FLIST_FLD_SET(search_iflistp, PIN_FLD_TEMPLATE, templatep, ebufp);
	
		args_flistp = PIN_FLIST_ELEM_ADD(search_iflistp, PIN_FLD_ARGS, 1, ebufp);
		PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_DEAL_OBJ, NULL, ebufp);

		args_flistp = PIN_FLIST_ELEM_ADD(search_iflistp, PIN_FLD_ARGS, 2, ebufp);
        	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_POID, NULL, ebufp);

		args_flistp = PIN_FLIST_ELEM_ADD(search_iflistp, PIN_FLD_ARGS, 3, ebufp);
        	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_ACCOUNT_OBJ, NULL, ebufp);

		args_flistp = PIN_FLIST_ELEM_ADD(search_iflistp, PIN_FLD_ARGS, 4, ebufp);
        	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_POID, NULL, ebufp);

		args_flistp = PIN_FLIST_ELEM_ADD(search_iflistp, PIN_FLD_ARGS, 5, ebufp);
                PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_POID, pdp, ebufp);

		args_flistp = PIN_FLIST_ELEM_ADD(search_iflistp, PIN_FLD_ARGS, 6, ebufp);
		if (PIN_FLIST_FLD_GET(deal_info, PIN_FLD_DEAL_NAME, 1, ebufp) != NULL)
		{
			d_name = PIN_FLIST_FLD_GET(deal_info, PIN_FLD_DEAL_NAME, 0, ebufp);
			PIN_ERR_LOG_MSG(3, d_name);
			PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_NAME, d_name, ebufp);
		}
		else
		{
			d_pdp = PIN_FLIST_FLD_GET(deal_info, PIN_FLD_POID, 0, ebufp);
			PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_POID, d_pdp, ebufp);
		}

		args_flistp = PIN_FLIST_ELEM_ADD(search_iflistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, ebufp);
		PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_ACCOUNT_OBJ, NULL, ebufp);
                PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_SERVICE_OBJ, NULL, ebufp);
                PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_DEAL_OBJ, NULL, ebufp);
                PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_PACKAGE_ID, NULL, ebufp);
                PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_STATUS, NULL, ebufp);

		PIN_ERR_LOG_FLIST(3, "end search input flist: ", search_iflistp);
		PCM_OP(ctxp, PCM_OP_SEARCH, 0, search_iflistp, &seach_oflistp, ebufp);

		PIN_ERR_LOG_FLIST(3, "search output flist: ", seach_oflistp);
		
		res_get_flistp = PIN_FLIST_ELEM_GET(seach_oflistp, PIN_FLD_RESULTS, 0, 1, ebufp);
				
		acc_obj = PIN_FLIST_FLD_GET(res_get_flistp, PIN_FLD_ACCOUNT_OBJ, 0, ebufp);
		ser_obj = PIN_FLIST_FLD_GET(res_get_flistp, PIN_FLD_SERVICE_OBJ, 0, ebufp);
		deal_obj = PIN_FLIST_FLD_GET(res_get_flistp, PIN_FLD_DEAL_OBJ, 0, ebufp);
		pkg_id = PIN_FLIST_FLD_GET(res_get_flistp, PIN_FLD_PACKAGE_ID, 0, ebufp);

		canc_iflistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(canc_iflistp, PIN_FLD_POID, acc_obj, ebufp);
		PIN_FLIST_FLD_SET(canc_iflistp, PIN_FLD_SERVICE_OBJ, ser_obj, ebufp);
		PIN_FLIST_FLD_SET(canc_iflistp, PIN_FLD_START_T, &now_t, ebufp);
		PIN_FLIST_FLD_SET(canc_iflistp, PIN_FLD_END_T, end_t, ebufp);
        	PIN_FLIST_FLD_SET(canc_iflistp, PIN_FLD_PROGRAM_NAME, "CANCEL DEAL", ebufp);
        	PIN_FLIST_FLD_SET(canc_iflistp, PIN_FLD_DESCR, descr, ebufp);

		deal_info = PIN_FLIST_SUBSTR_ADD(canc_iflistp, PIN_FLD_DEAL_INFO, ebufp);
		PIN_FLIST_FLD_SET(deal_info, PIN_FLD_DEAL_OBJ, deal_obj, ebufp);
		PIN_FLIST_FLD_SET(deal_info, PIN_FLD_PACKAGE_ID, pkg_id, ebufp);

		PIN_ERR_LOG_FLIST(3, "cancel deal input flist: ", canc_iflistp);
		PCM_OP(ctxp, PCM_OP_RBS_CRM_SUBSCRIPTION_CANCEL, 0, canc_iflistp, &canc_oflistp, ebufp);
		PIN_ERR_LOG_FLIST(3, "cancel deal output flist: ", canc_oflistp);
	}
	if (!canc_oflistp)
	{
		acc_robj = PIN_FLIST_FLD_GET(canc_oflistp, PIN_FLD_POID, 0, ebufp);
	}
	
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"purchase_service error ", ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, acc_obj, ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, ser_obj, ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_DESCR, "cancel deal error", ebufp);

		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "cancel_service error flist", ret_flistp);
		PIN_ERRBUF_RESET(ebufp);
		goto CLEANUP;
	}
	else
	{
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "op_cust_rbs_cust_purchase_service_item output flist", po_flistp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, acc_obj, ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_SERVICE_OBJ, ser_obj, ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_DEAL_OBJ, deal_obj, ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_DESCR, "cancel deal success proceeding to purchase new deal", ebufp);
	}

	if (PIN_POID_COMPARE(pdp, acc_obj, 0, ebufp) == 0)
	{

		PIN_ERR_LOG_MSG(3, "Purchase start:");

		pur_deal_info =  PIN_FLIST_SUBSTR_GET(i_flistp, PIN_FLD_PURCHASE_DEAL_INFO, 0, ebufp);
		elem_id2 = 0;
		cookie2 = NULL;
		while ((p_deals = PIN_FLIST_ELEM_GET_NEXT(pur_deal_info, PIN_FLD_DEALS, &elem_id2, 1, &cookie2, ebufp)) != NULL)
		{
	
			d_name = PIN_FLIST_FLD_GET(p_deals, PIN_FLD_DEAL_NAME, 0, ebufp);

			s_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_POID, search_pdp, ebufp);			

			if (PIN_FLIST_FLD_GET(p_deals, PIN_FLD_DEAL_NAME, 1, ebufp) != NULL)
			{
				d_name = PIN_FLIST_FLD_GET(p_deals, PIN_FLD_DEAL_NAME, 0, ebufp);
				PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_NAME, d_name, ebufp);
			}
			else
			{
				d_pdp = PIN_FLIST_FLD_GET(p_deals, PIN_FLD_POID, 0, ebufp);
                                PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_POID, d_pdp, ebufp);
			}
			
			PIN_ERR_LOG_FLIST(3, "deal_info search input flist: ", s_flistp);
			fm_get_deal_from_name(ctxp, s_flistp, &so_flistp, ebufp);
			PIN_ERR_LOG_FLIST(3, "deal_info search output flist: ", so_flistp);
			
			sr_flistp = PIN_FLIST_ELEM_GET(so_flistp, PIN_FLD_RESULTS, 0, 1, ebufp);	
			deal_obj = PIN_FLIST_FLD_GET(sr_flistp, PIN_FLD_POID, 0, ebufp);
			permitted = PIN_FLIST_FLD_GET(sr_flistp, PIN_FLD_PERMITTED, 0, ebufp);
			deal_name = PIN_FLIST_FLD_GET(sr_flistp, PIN_FLD_NAME, 0, ebufp);
			
			ser_iflistp = PIN_FLIST_CREATE(ebufp);
                        PIN_FLIST_FLD_SET(ser_iflistp, PIN_FLD_POID, PIN_POID_CREATE(db, permitted, -1, ebufp), ebufp);
			PIN_FLIST_FLD_SET(ser_iflistp, PIN_FLD_ACCOUNT_OBJ, acc_obj, ebufp);

			PIN_ERR_LOG_FLIST(3, "service_info search input flist: ", ser_iflistp);
                        fm_get_service_info(ctxp, ser_iflistp, &ser_oflistp, ebufp);
                        PIN_ERR_LOG_FLIST(3, "service_info search output flist: ", ser_oflistp);

			ser_rflistp = PIN_FLIST_ELEM_GET(ser_oflistp, PIN_FLD_RESULTS, 0, 1, ebufp);
                        ser_obj = PIN_FLIST_FLD_GET(ser_rflistp, PIN_FLD_POID, 0, ebufp);

			pur_iflistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_SET(pur_iflistp, PIN_FLD_POID, acc_obj, ebufp);
                        PIN_FLIST_FLD_SET(pur_iflistp, PIN_FLD_SERVICE_OBJ, ser_obj, ebufp);
                        PIN_FLIST_FLD_SET(pur_iflistp, PIN_FLD_START_T, &now_t, ebufp);
                        PIN_FLIST_FLD_SET(pur_iflistp, PIN_FLD_END_T, end_t, ebufp);
                        PIN_FLIST_FLD_SET(pur_iflistp, PIN_FLD_PROGRAM_NAME, "CANCEL DEAL", ebufp);

			deal_info_flistp = PIN_FLIST_SUBSTR_ADD(pur_iflistp, PIN_FLD_DEAL_INFO, ebufp);
                	PIN_FLIST_FLD_SET(deal_info_flistp, PIN_FLD_POID, deal_obj, ebufp);
			PIN_FLIST_FLD_SET(deal_info_flistp, PIN_FLD_NAME, deal_name, ebufp);
			PIN_FLIST_FLD_SET(deal_info_flistp, PIN_FLD_DESCR, descr, ebufp);
			PIN_FLIST_FLD_SET(deal_info_flistp, PIN_FLD_START_T, NULL, ebufp);
			PIN_FLIST_FLD_SET(deal_info_flistp, PIN_FLD_END_T, NULL, ebufp);

			PIN_ERR_LOG_FLIST(3, "check1:", pur_iflistp);

			cnt = 0;
			elem_id = 0;
			cookie = NULL;
			while ((prod_flistp = PIN_FLIST_ELEM_GET_NEXT(sr_flistp, PIN_FLD_PRODUCTS, &elem_id, 1, &cookie, ebufp)) != NULL)
			{
				prod_flistp1 = PIN_FLIST_ELEM_ADD(deal_info_flistp, PIN_FLD_PRODUCTS, cnt, ebufp);
                        	PIN_FLIST_FLD_COPY(prod_flistp, PIN_FLD_PRODUCT_OBJ, prod_flistp1, PIN_FLD_PRODUCT_OBJ, ebufp);
                        	PIN_FLIST_FLD_COPY(prod_flistp, PIN_FLD_CYCLE_DISCOUNT, prod_flistp1, PIN_FLD_CYCLE_DISCOUNT, ebufp);
                        	PIN_FLIST_FLD_COPY(prod_flistp, PIN_FLD_CYCLE_END_DETAILS, prod_flistp1, PIN_FLD_CYCLE_END_DETAILS, ebufp);
                        	PIN_FLIST_FLD_COPY(prod_flistp, PIN_FLD_CYCLE_END_T, prod_flistp1, PIN_FLD_CYCLE_END_T, ebufp);
                        	PIN_FLIST_FLD_COPY(prod_flistp, PIN_FLD_CYCLE_START_DETAILS, prod_flistp1, PIN_FLD_CYCLE_START_DETAILS, ebufp);
                        	PIN_FLIST_FLD_COPY(prod_flistp, PIN_FLD_CYCLE_START_T, prod_flistp1, PIN_FLD_CYCLE_START_T, ebufp);
                        	PIN_FLIST_FLD_COPY(prod_flistp, PIN_FLD_PURCHASE_DISCOUNT, prod_flistp1, PIN_FLD_PURCHASE_DISCOUNT, ebufp);
                        	PIN_FLIST_FLD_COPY(prod_flistp, PIN_FLD_PURCHASE_END_DETAILS, prod_flistp1, PIN_FLD_PURCHASE_END_DETAILS, ebufp);
                        	PIN_FLIST_FLD_COPY(prod_flistp, PIN_FLD_PURCHASE_END_T, prod_flistp1, PIN_FLD_PURCHASE_END_T, ebufp);
                        	PIN_FLIST_FLD_COPY(prod_flistp, PIN_FLD_PURCHASE_START_DETAILS, prod_flistp1, PIN_FLD_PURCHASE_START_DETAILS, ebufp);
                        	PIN_FLIST_FLD_COPY(prod_flistp, PIN_FLD_PURCHASE_START_T, prod_flistp1, PIN_FLD_PURCHASE_START_T, ebufp);
                        	PIN_FLIST_FLD_COPY(prod_flistp, PIN_FLD_USAGE_DISCOUNT, prod_flistp1, PIN_FLD_USAGE_DISCOUNT, ebufp);
                        	PIN_FLIST_FLD_COPY(prod_flistp, PIN_FLD_USAGE_END_DETAILS, prod_flistp1, PIN_FLD_USAGE_END_DETAILS, ebufp);
                        	PIN_FLIST_FLD_COPY(prod_flistp, PIN_FLD_USAGE_END_T, prod_flistp1, PIN_FLD_USAGE_END_T, ebufp);
                        	PIN_FLIST_FLD_COPY(prod_flistp, PIN_FLD_USAGE_START_T, prod_flistp1, PIN_FLD_USAGE_START_T, ebufp);
                        	PIN_FLIST_FLD_COPY(prod_flistp, PIN_FLD_STATUS,prod_flistp1, PIN_FLD_STATUS, ebufp);
                        	PIN_FLIST_FLD_COPY(prod_flistp, PIN_FLD_STATUS_FLAGS, prod_flistp1, PIN_FLD_STATUS_FLAGS, ebufp);
                        	PIN_FLIST_FLD_COPY(prod_flistp, PIN_FLD_QUANTITY, prod_flistp1, PIN_FLD_QUANTITY, ebufp);
                        	PIN_FLIST_FLD_COPY(prod_flistp, PIN_FLD_USAGE_START_DETAILS, prod_flistp1, PIN_FLD_USAGE_START_DETAILS, ebufp);
                        	PIN_FLIST_FLD_SET(prod_flistp1, PIN_FLD_DESCR, descr, ebufp);
                        	cnt++;
                	}
			PIN_ERR_LOG_FLIST(3, "purchase input after products", pur_iflistp);

			cnt1 = 0;
			elem_id1 = 0;
                	cookie1 = NULL;
			while ((disc_flistp = PIN_FLIST_ELEM_GET_NEXT(sr_flistp, PIN_FLD_DISCOUNTS, &elem_id, 1, &cookie, ebufp)) != NULL)
                	{
                	        disc1_flistp = PIN_FLIST_ELEM_ADD(deal_info_flistp, PIN_FLD_DISCOUNTS, cnt1, ebufp);
                	        PIN_FLIST_FLD_COPY(disc_flistp, PIN_FLD_CYCLE_END_DETAILS, disc1_flistp, PIN_FLD_CYCLE_END_DETAILS, ebufp);
                        	PIN_FLIST_FLD_COPY(disc_flistp, PIN_FLD_CYCLE_END_T, disc1_flistp, PIN_FLD_CYCLE_END_T, ebufp);
                        	PIN_FLIST_FLD_COPY(disc_flistp, PIN_FLD_CYCLE_START_DETAILS, disc1_flistp, PIN_FLD_CYCLE_START_DETAILS, ebufp);
                        	PIN_FLIST_FLD_COPY(disc_flistp, PIN_FLD_CYCLE_START_T, disc1_flistp, PIN_FLD_CYCLE_START_T, ebufp);
                        	PIN_FLIST_FLD_COPY(disc_flistp, PIN_FLD_DISCOUNT_OBJ, disc1_flistp, PIN_FLD_DISCOUNT_OBJ, ebufp);
                        	PIN_FLIST_FLD_COPY(disc_flistp, PIN_FLD_PURCHASE_END_DETAILS, disc1_flistp, PIN_FLD_PURCHASE_END_DETAILS, ebufp);
                        	PIN_FLIST_FLD_COPY(disc_flistp, PIN_FLD_PURCHASE_END_T, disc1_flistp, PIN_FLD_PURCHASE_END_T, ebufp);
                        	PIN_FLIST_FLD_COPY(disc_flistp, PIN_FLD_START_DETAILS, disc1_flistp, PIN_FLD_START_DETAILS, ebufp);
                        	PIN_FLIST_FLD_COPY(disc_flistp, PIN_FLD_START_T, disc1_flistp, PIN_FLD_START_T, ebufp);
                        	PIN_FLIST_FLD_COPY(disc_flistp, PIN_FLD_QUANTITY, disc1_flistp, PIN_FLD_QUANTITY, ebufp);
                        	PIN_FLIST_FLD_COPY(disc_flistp, PIN_FLD_STATUS, disc1_flistp, PIN_FLD_STATUS, ebufp);
                        	PIN_FLIST_FLD_COPY(disc_flistp, PIN_FLD_STATUS_FLAGS, disc1_flistp, PIN_FLD_STATUS_FLAGS, ebufp);
                        	PIN_FLIST_FLD_COPY(disc_flistp, PIN_FLD_USAGE_START_DETAILS, disc1_flistp, PIN_FLD_USAGE_START_DETAILS, ebufp);
                        	PIN_FLIST_FLD_COPY(disc_flistp, PIN_FLD_USAGE_START_T, disc1_flistp, PIN_FLD_USAGE_START_T, ebufp);
                        	PIN_FLIST_FLD_COPY(disc_flistp, PIN_FLD_USAGE_END_DETAILS, disc1_flistp, PIN_FLD_USAGE_END_DETAILS, ebufp);
                        	PIN_FLIST_FLD_COPY(disc_flistp, PIN_FLD_USAGE_END_T, disc1_flistp, PIN_FLD_USAGE_END_T, ebufp);
                        	cnt1++;
                	}

			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "end fm_rbs_cust_purchase_service input_flist", pur_iflistp);
			PCM_OP(ctxp, PCM_OP_RBS_CRM_SUBSCRIPTION_PURCHASE, 1, pur_iflistp, &po_flistp, ebufp);
		}
		PIN_ERR_LOG_FLIST(3, "purchase output flist", po_flistp);
        	if (PIN_ERRBUF_IS_ERR(ebufp))
        	{
                	PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"purchase_service error ", ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, acc_obj, ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_DESCR, "error when purchasing the deal", ebufp);

                	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "purchase_service error flist", ret_flistp);
                	PIN_ERRBUF_RESET(ebufp);
                	goto CLEANUP;
        	}
        	else
        	{
                	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "op_cust_rbs_cust_purchase_service_item output", po_flistp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, acc_obj, ebufp);
        		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_SERVICE_OBJ, ser_obj, ebufp);
        		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_DEAL_OBJ, deal_obj, ebufp);
        		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_DESCR, "DEAL change success", ebufp);
        	}
	}
	*r_flistp = PIN_FLIST_COPY(ret_flistp, ebufp);
        PIN_ERR_LOG_FLIST(3, "fm_rbs_cust_change_deal output_flist:",  *r_flistp);
        return;
	
	CLEANUP:
		*r_flistp = PIN_FLIST_COPY(ret_flistp, ebufp);
		PIN_FLIST_DESTROY_EX(&pur_iflistp, NULL);
                PIN_FLIST_DESTROY_EX(&ret_flistp, NULL);
		PIN_FLIST_DESTROY_EX(&res_flistp, NULL);
		PIN_FLIST_DESTROY_EX(&po_flistp, NULL);
		PIN_FLIST_DESTROY_EX(&s_flistp, NULL);
		PIN_FLIST_DESTROY_EX(&so_flistp, NULL);
		PIN_FLIST_DESTROY_EX(&prod_flistp, NULL);
		PIN_FLIST_DESTROY_EX(&prod_flistp1, NULL);
		PIN_FLIST_DESTROY_EX(&disc_flistp, NULL);
		PIN_FLIST_DESTROY_EX(&disc1_flistp, NULL);

	return;
}
