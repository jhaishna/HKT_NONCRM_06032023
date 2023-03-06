#ifndef lint
static const char Sccs_id[] = "@(#)%Portal Version: fm_rbs_cust_resume_service.c :BillingVelocityInt:3:2006-Sep-05 11:55:03 %";
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
#include "ops/bill.h"
#include "ops/cust.h"
#include "ops/subscription.h"
#include "custom_flds/custom_flds.h"
#include "custom_wrapper_ops.h"


/*************************************
*DEFINE FUNCTIONS
*************************************/

EXPORT_OP void
op_rbs_cust_resume_service(
    cm_nap_connection_t 	*connp,
    int32           		opcode,
    int32           		flags,
    pin_flist_t     		*i_flistp,
    pin_flist_t     		**r_flistpp,
    pin_errbuf_t    		*ebufp);

static void
fm_rbs_cust_resume_service(
        pcm_context_t           *ctxp,
        pin_flist_t             *i_flistp,
        pin_flist_t             **r_flistp,
        pin_errbuf_t            *ebufp);

/*****************************************
*FUNCTION IMPLIMENTATIONS
*****************************************/

 void
op_rbs_cust_resume_service(
    cm_nap_connection_t *connp,
    int32           opcode,
    int32           flags,
    pin_flist_t     *i_flistp,
    pin_flist_t     **r_flistpp,
    pin_errbuf_t    *ebufp)
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
        if (opcode != PCM_OP_RBS_CUST_RESUME_SERVICE ) {
                pin_set_err(ebufp, PIN_ERRLOC_FM,
                        PIN_ERRCLASS_SYSTEM_DETERMINATE,
                        PIN_ERR_BAD_OPCODE, 0, 0, opcode);
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                        "op_rbs_cust_resume_service",
                        ebufp);
                return;
        }

        /*******************************************************************
         * Debug: Input flist
        *******************************************************************/
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                "op_rbs_cust_resume_service input flist", i_flistp);

        /*******************************************************************
         * Call the default implementation
         *******************************************************************/

        fm_rbs_cust_resume_service(ctxp, i_flistp, &r_flistp, ebufp);

        /***********************************************************
         * Results.
         ***********************************************************/

	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"op_cust_purchase_service error", ebufp);
	}
	else
	{
		*r_flistpp = PIN_FLIST_COPY(r_flistp, ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "op_rbs_cust_resume_service output flist", *r_flistpp);
	}

	PIN_FLIST_DESTROY_EX(&r_flistp, NULL);
	return;
}

void
fm_rbs_cust_resume_service(
        pcm_context_t           *ctxp,
        pin_flist_t             *i_flistp,
        pin_flist_t             **r_flistp,
        pin_errbuf_t            *ebufp)
{

        pin_flist_t             *in_flistp = NULL;
        pin_flist_t             *inp_flistp = NULL;
        pin_flist_t             *ret_flistp = NULL;
	pin_flist_t		*search_iflistp = NULL;
	pin_flist_t             *search_oflistp = NULL;
	pin_flist_t		*ret_prod_flistp = NULL;
	pin_flist_t             *ret_disc_flistp = NULL;
	pin_flist_t		*status_flistp = NULL;
	pin_flist_t		*prod_flistp = NULL;
	pin_flist_t		*disc_flistp = NULL;
	pin_flist_t 		*result_flistp  = NULL;
	pin_flist_t		*read_flistp = NULL;
	pin_flist_t		*read_oflistp = NULL;
	pin_flist_t             *ar_in_flistp = NULL;
	pin_flist_t             *ar_o_flistp = NULL;
	pin_flist_t             *fr_o_flistp = NULL;
	poid_t			*prod_pdp = NULL;
	poid_t			*disc_pdp = NULL;	
        poid_t                  *acc_pd = NULL;
	poid_t			*pur_pdct = NULL;
	poid_t                  *pur_dsct = NULL;
	poid_t                  *ser_obj = NULL;
	poid_t			*pdp = NULL;
	time_t                  start_t = 0;
        time_t                  *end_t = NULL;
	int32			*s_flags = 0;
	int32                   *flags = 0;
	int32                   *status = 0;
	int32			*p_status = 0;
	char			*type_str = NULL;
	int32			s_fail = 0;
	int32			s_succ = 0;
	char			msg[200];
	char			*pd_type = NULL;
	char                    *p_descr = NULL;


        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "start fm_rbs_cust_resume_service input_flist", i_flistp);

        in_flistp = PIN_FLIST_CREATE(ebufp);
        acc_pd = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_ACCOUNT_OBJ, 0, ebufp);

	start_t = pin_virtual_time((time_t *)NULL);
        end_t  = (time_t *)PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_END_T, 0, ebufp);
        status = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_STATUS, 0, ebufp);
	pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp);
		pd_type = (char *)PIN_POID_GET_TYPE(pdp);
        PIN_ERR_LOG_MSG(3, pd_type);

        if (pd_type && strcmp(pd_type, "/purchased_product") == 0)
        {
                pur_pdct = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp);
                PIN_ERR_LOG_POID(3, "product_obj:", pur_pdct);

                if (PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_OFFERING_OBJ, 1, ebufp) != NULL)
                {
                        pur_dsct = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_OFFERING_OBJ, 0, ebufp);
                        PIN_ERR_LOG_POID(3, "offering obj:", pur_dsct);
                }
        }
        else if(pd_type && strcmp(pd_type, "/purchased_discount") == 0)
        {
                pur_dsct = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp);
                PIN_ERR_LOG_POID(3, "discount_obj:", pur_dsct);
        }

        if (PIN_ERRBUF_IS_ERR(ebufp))
        {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                    "fm_rbs_cust_suspense_service error: required field missing in input flist", ebufp);
                return;
        }
	type_str = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_TYPE_STR, 0, ebufp);
	
	PIN_FLIST_FLD_SET(in_flistp, PIN_FLD_POID, acc_pd, ebufp);
	PIN_FLIST_FLD_SET(in_flistp, PIN_FLD_START_T, &start_t, ebufp);
	PIN_FLIST_FLD_SET(in_flistp, PIN_FLD_END_T, end_t, ebufp);
	PIN_FLIST_FLD_SET(in_flistp, PIN_FLD_PROGRAM_NAME, "Mobile Billing Integration", ebufp);
	PIN_ERR_LOG_FLIST(3, "product status input", in_flistp);
	status_flistp = PIN_FLIST_ELEM_ADD(in_flistp, PIN_FLD_STATUSES, 0, ebufp);
	PIN_FLIST_FLD_SET(status_flistp, PIN_FLD_STATUS_FLAGS, s_flags, ebufp);
	PIN_FLIST_FLD_SET(status_flistp, PIN_FLD_STATUS, status, ebufp);
	PIN_ERR_LOG_FLIST(3, "product status input", in_flistp);
	if (pur_pdct)
        {
                search_iflistp = PIN_FLIST_CREATE(ebufp);
                PIN_FLIST_FLD_SET(search_iflistp, PIN_FLD_POID, pur_pdct, ebufp);
                PIN_ERR_LOG_FLIST(3, "fm_get_purchased_product_info input: ", search_iflistp);
                fm_get_purchased_product_info(ctxp, search_iflistp, &search_oflistp, ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp))
                {
                        PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling SEARCH", ebufp);
                        goto CLEANUP;
                }
                PIN_ERR_LOG_FLIST(3, "fm_get_purchased_product_info output: ", search_oflistp);
                result_flistp = PIN_FLIST_ELEM_GET(search_oflistp, PIN_FLD_RESULTS, 0, 1, ebufp);
                p_status = (int32 *)PIN_FLIST_FLD_GET(result_flistp, PIN_FLD_STATUS, 0, ebufp);
                prod_pdp = PIN_FLIST_FLD_GET(result_flistp, PIN_FLD_PRODUCT_OBJ, 0, ebufp);
                ser_obj = PIN_FLIST_FLD_GET(result_flistp, PIN_FLD_SERVICE_OBJ, 0, ebufp);
                PIN_ERR_LOG_POID(3,"Service_obj: ", ser_obj);
                read_flistp = PIN_FLIST_CREATE(ebufp);
                PIN_FLIST_FLD_SET(read_flistp, PIN_FLD_POID, prod_pdp, ebufp);
		PIN_ERR_LOG_FLIST(3, "products input: ", read_flistp);
                PCM_OP(ctxp, PCM_OP_READ_OBJ, 0, read_flistp, &read_oflistp, ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp))
                {
                        PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling READ_OBJ", ebufp);
                        goto CLEANUP;
                }
                PIN_ERR_LOG_FLIST(3, "products output: ", read_oflistp);
                p_descr = PIN_FLIST_FLD_GET(read_oflistp, PIN_FLD_DESCR, 0, ebufp);
	}
        if (pur_pdct && p_status && p_descr && *p_status == 2 && strstr(p_descr, "SUS:I;"))
        {
        	PIN_FLIST_FLD_SET(status_flistp, PIN_FLD_OFFERING_OBJ, pur_pdct, ebufp);
                PIN_ERR_LOG_FLIST(3, "product status input", in_flistp);
                PCM_OP(ctxp, PCM_OP_SUBSCRIPTION_SET_PRODUCT_STATUS, 0, in_flistp, &ret_prod_flistp, ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp))
                {
                        PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling PCM_OP_SUBSCRIPTION_SET_PRODUCT_STATUS", ebufp);
                        goto CLEANUP;
                }
                PIN_ERR_LOG_FLIST(3, "product status output", ret_prod_flistp);
        }
        if (pur_dsct)
        {
                search_iflistp = PIN_FLIST_CREATE(ebufp);
                PIN_FLIST_FLD_SET(search_iflistp, PIN_FLD_POID, pur_dsct, ebufp);
                PIN_ERR_LOG_FLIST(3, "fm_get_purchased_discount_info input: ", search_iflistp);
                fm_get_purchased_discount_info(ctxp, search_iflistp, &search_oflistp, ebufp);
                if (PIN_ERRBUF_IS_ERR(ebufp))
                {
                        PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling SEARCH", ebufp);
                        goto CLEANUP;
                }
		PIN_ERR_LOG_FLIST(3, "fm_get_purchased_discount_info output: ", search_oflistp);
                result_flistp = PIN_FLIST_ELEM_GET(search_oflistp, PIN_FLD_RESULTS, 0, 1, ebufp);
                p_status = (int32 *)PIN_FLIST_FLD_GET(result_flistp, PIN_FLD_STATUS, 0, ebufp);
                disc_pdp = PIN_FLIST_FLD_GET(result_flistp, PIN_FLD_DISCOUNT_OBJ, 0, ebufp);
                ser_obj = PIN_FLIST_FLD_GET(result_flistp, PIN_FLD_SERVICE_OBJ, 0, ebufp);
                PIN_ERR_LOG_POID(3,"Service_obj: ", ser_obj);
                read_flistp = PIN_FLIST_CREATE(ebufp);
                PIN_FLIST_FLD_SET(read_flistp, PIN_FLD_POID, disc_pdp, ebufp);
                PCM_OP(ctxp, PCM_OP_READ_OBJ, 0, read_flistp, &read_oflistp, ebufp);
                if (PIN_ERRBUF_IS_ERR(ebufp))
                {
                        PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling READ_OBJ", ebufp);
                        goto CLEANUP;
                }
		PIN_ERR_LOG_FLIST(3, "discounts output: ", read_oflistp);
                p_descr = PIN_FLIST_FLD_GET(read_oflistp, PIN_FLD_DESCR, 0, ebufp);
	}        
        if (pur_dsct && p_status && p_descr && *p_status == 2 && strstr(p_descr, "SUS:I;"))
        {
        	PIN_FLIST_FLD_SET(status_flistp, PIN_FLD_OFFERING_OBJ, pur_dsct, ebufp);
                PIN_ERR_LOG_FLIST(3, "discount status input", in_flistp);
                PCM_OP(ctxp, PCM_OP_SUBSCRIPTION_SET_DISCOUNT_STATUS, 0, in_flistp, &ret_disc_flistp, ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp))
                {
                        PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling PCM_OP_SUBSCRIPTION_SET_DISCOUNT_STATUS", ebufp);
                        goto CLEANUP;
                }
                PIN_ERR_LOG_FLIST(3, "discount status output", ret_disc_flistp);
        }

	if (acc_pd && ser_obj)
        {
                ar_in_flistp = PIN_FLIST_CREATE(ebufp);
                PIN_FLIST_FLD_SET(ar_in_flistp, PIN_FLD_POID, acc_pd, ebufp);
                PIN_FLIST_FLD_SET(ar_in_flistp, PIN_FLD_FLAGS, &flags, ebufp);
                PIN_FLIST_FLD_SET(ar_in_flistp, PIN_FLD_SERVICE_OBJ, ser_obj, ebufp);
                PIN_FLIST_FLD_SET(ar_in_flistp, PIN_FLD_PROGRAM_NAME, "Cycle Arrear Action", ebufp);
                PIN_FLIST_FLD_SET(ar_in_flistp, PIN_FLD_START_T, &start_t, ebufp);
                PIN_FLIST_FLD_SET(ar_in_flistp, PIN_FLD_END_T, end_t, ebufp);
                PIN_FLIST_ELEM_COPY(result_flistp, PIN_FLD_RESULTS, 0, ar_in_flistp, PIN_FLD_PRODUCTS, 0, ebufp);


                if (p_status && p_descr && *p_status == 2 && strstr(p_descr, "CYC:CA"))
                {
                        PIN_ERR_LOG_FLIST(3, "PCM_OP_SUBSCRIPTION_CYCLE_ARREARS input: ", ar_in_flistp);
                        PCM_OP(ctxp, PCM_OP_SUBSCRIPTION_CYCLE_ARREARS, 0, ar_in_flistp, &ar_o_flistp, ebufp);
                        if (PIN_ERR_IS_ERR(ebufp))
                        {
                                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "PCM_OP_SUBSCRIPTION_CYCLE_ARREARS error!", ebufp);
                                goto CLEANUP;
                        }

                        PIN_ERR_LOG_FLIST(3, "PCM_OP_SUBSCRIPTION_CYCLE_ARREARS output: ", ar_o_flistp);
                }
		if (p_status && p_descr && *p_status == 2 && strstr(p_descr, "CYC:CA"))
                {
                        PIN_ERR_LOG_FLIST(3, "PCM_OP_SUBSCRIPTION_CYCLE_ARREARS input: ", ar_in_flistp);
                        PCM_OP(ctxp, PCM_OP_SUBSCRIPTION_CYCLE_FORWARD, 0, ar_in_flistp, &fr_o_flistp, ebufp);
                        if (PIN_ERR_IS_ERR(ebufp))
                        {
                                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "PCM_OP_SUBSCRIPTION_CYCLE_FORWARD error!", ebufp);
                                goto CLEANUP;
                        }

                        PIN_ERR_LOG_FLIST(3, "PCM_OP_SUBSCRIPTION_CYCLE_FORWARD output: ", fr_o_flistp);
                }
        }
	
	if (type_str && strcmp(type_str, "RT") == 0)
	{
		inp_flistp = PIN_FLIST_CREATE(ebufp);
 		PIN_FLIST_FLD_SET(inp_flistp, PIN_FLD_POID, acc_pd, ebufp);
        	PIN_FLIST_FLD_SET(inp_flistp, PIN_FLD_START_T, &start_t, ebufp);	
		PIN_FLIST_FLD_SET(inp_flistp, PIN_FLD_PROGRAM_NAME, "Mobile Billing Integration", ebufp);
		PIN_FLIST_FLD_SET(inp_flistp, PIN_FLD_TYPE_STR, type_str, ebufp);
		PIN_FLIST_FLD_SET(inp_flistp, PIN_FLD_SERVICE_OBJ, ser_obj, ebufp);
		PIN_FLIST_FLD_SET(inp_flistp, PIN_FLD_OFFERING_OBJ, pur_pdct, ebufp);
		sprintf(msg, "p_descr: %s", p_descr);
		PIN_ERR_LOG_MSG(3, msg);	
	        if (p_status && p_descr && *p_status == 2 && strstr(p_descr, "SUS:R;"))
		{
			PIN_ERR_LOG_FLIST(3, "PCM_OP_MRB_POL_REBATE_RESUME input: ", inp_flistp);
			PCM_OP(ctxp, PCM_OP_MRB_POL_REBATE_RESUME, 0, inp_flistp, &ret_flistp, ebufp);
			if (PIN_ERR_IS_ERR(ebufp))
        		{
                        	PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "PCM_OP_MRB_POL_REBATE_RESUME error!", ebufp);
                        	goto CLEANUP;
                	}
	
			PIN_ERR_LOG_FLIST(3, "PCM_OP_MRB_POL_REBATE_RESUME output: ", ret_flistp);	
		}
                if (p_status && p_descr && *p_status == 2 && strstr(p_descr, "SUS:O;"))
		{
			PIN_ERR_LOG_FLIST(3, "PCM_OP_MRB_POL_OINCENTIVE_RESUME input", inp_flistp);
			PCM_OP(ctxp, PCM_OP_MRB_POL_OINCENTIVE_RESUME, 0, inp_flistp, &ret_flistp, ebufp);
			if (PIN_ERR_IS_ERR(ebufp))
			{
                		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "PCM_OP_MRB_POL_OINCENTIVE_RESUME error!", ebufp);
                	        goto CLEANUP;
                	}
			PIN_ERR_LOG_FLIST(3, "PCM_OP_MRB_POL_OINCENTIVE_RESUME output", ret_flistp);
		}
		if (p_status && p_descr && *p_status == 2 && strstr(p_descr, "SUS:PTC;"))
                {
        		PIN_ERR_LOG_FLIST(3, "PCM_OP_MRB_POL_RESUME_CONTRACT input", inp_flistp);
        		PCM_OP(ctxp, PCM_OP_MRB_POL_RESUME_CONTRACT, 0, inp_flistp, &ret_flistp, ebufp);
			if (PIN_ERR_IS_ERR(ebufp))
                	{
                	        PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "PCM_OP_MRB_POL_RESUME_CONTRACT error!", ebufp);
                	        goto CLEANUP;
                	}
                	PIN_ERR_LOG_FLIST(3, "PCM_OP_MRB_POL_RESUME_CONTRACT output", ret_flistp);
       		}
                if (p_status && p_descr && *p_status == 2 && strstr(p_descr, "SUS:ITC;"))
		{ 
        		PIN_ERR_LOG_FLIST(3, "PCM_OP_MRB_POL_RESUME_INSTALLMENT input", inp_flistp);
                	PCM_OP(ctxp, PCM_OP_MRB_POL_RESUME_INSTALLMENT, 0, inp_flistp, &ret_flistp, ebufp);
               		if (PIN_ERR_IS_ERR(ebufp))
                	{
                	        PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "PCM_OP_MRB_POL_RESUME_INSTALLMENT error!", ebufp);
                	        goto CLEANUP;
                	}
                	PIN_ERR_LOG_FLIST(3, "PCM_OP_MRB_POL_RESUME_INSTALLMENT output", ret_flistp);
        	}	
	}
	if (PIN_ERRBUF_IS_ERR(ebufp))
       	{
       		ret_flistp = PIN_FLIST_CREATE(ebufp);
                s_fail = 1;
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"fm_rbs_cust_resume_service error", ebufp);
                PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, ret_flistp, PIN_FLD_POID, ebufp);
                PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &s_fail, ebufp);
                PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "error fm_rbs_cust_resume_service flist", ret_flistp);
                PIN_ERRBUF_RESET(ebufp);
		return;
        }
	else
	{	 
		ret_flistp = PIN_FLIST_CREATE(ebufp);
                s_succ = 0;
                PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_ACCOUNT_OBJ, ret_flistp, PIN_FLD_POID, ebufp);
                PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &s_succ, ebufp);
                PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_rbs_cust_resume_service output flist", ret_flistp);
		*r_flistp = PIN_FLIST_COPY(ret_flistp, ebufp);
		return;
	}
CLEANUP:
	*r_flistp = PIN_FLIST_COPY(ret_flistp, ebufp);
	PIN_FLIST_DESTROY_EX(&inp_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&ar_in_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&in_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&ret_flistp, NULL);
}

