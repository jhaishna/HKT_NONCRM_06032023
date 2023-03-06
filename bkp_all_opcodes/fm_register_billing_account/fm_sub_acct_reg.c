#ifndef lint
static  char Sccs_Id[] = "@(#)%Portal Version: fm_sub_acct_reg.c :BillingVelocityInt:4:2006-Sep-05 04:28:17 %";
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


#define PIN_FLD_REC_ID 0

/*******************************************************************
 * Routines contained herein.
 *******************************************************************/
EXPORT_OP void
op_cust_sub_acct_reg(
        cm_nap_connection_t     *connp,
        int32                   opcode,
        int32                   flags,
        pin_flist_t             *i_flistp,
        pin_flist_t             **r_flistpp,
        pin_errbuf_t            *ebufp);
		

static void
fm_cust_sub_acct_reg(
         pcm_context_t           *ctxp,
         pin_flist_t             *i_flistp,
         pin_flist_t             **r_flistpp,
         pin_errbuf_t            *ebufp);

void
mrb_get_parent_billinfo_details(
        pcm_context_t           *ctxp,
        poid_t                   *i_pdp,
        pin_flist_t             **r_flistp,
        pin_errbuf_t            *ebufp);

void
mrb_get_read_obj_details(
        pcm_context_t           *ctxp,
        poid_t                   *i_pdp,
        pin_flist_t             **r_flistp,
        pin_errbuf_t            *ebufp);

void
mrb_get_deal_details(
        pcm_context_t           *ctxp,
        char                    *namep,
        pin_flist_t             **r_flistp,
        pin_errbuf_t            *ebufp);

static void
mrb_create_service_details(
         pcm_context_t           *ctxp,
         pin_flist_t             *i_flistp,
	 pin_flist_t             *subord_flistp,
         pin_flist_t             *ret_flistp,
         pin_flist_t             **r_flistp,
         pin_errbuf_t            *ebufp);

void
mrb_get_account_details(
        pcm_context_t           *ctxp,
        pin_flist_t             *i_flistp,
        pin_flist_t             **r_flistp,
        pin_errbuf_t            *ebufp);
	
/**************************************************************************************************/

void
op_cust_sub_acct_reg(
        cm_nap_connection_t     *connp,
        int                     opcode,
        int                     flags,
        pin_flist_t             *i_flistp,
        pin_flist_t             **r_flistpp,
        pin_errbuf_t            *ebufp)
{
        pcm_context_t           *ctxp = connp->dm_ctx;
        pin_flist_t             *r_flistp = NULL;
        
	poid_t			*dummy_pdp = NULL;
	int32			flag = 0;
        int32			local_trans = 0;

        if (PIN_ERRBUF_IS_ERR(ebufp)) {
                return;
        }
        PIN_ERRBUF_CLEAR(ebufp);

        /***********************************************************
         * Insanity check.
         ***********************************************************/
        if (opcode != PCM_OP_SUB_ACCT_REG) {
                pin_set_err(ebufp, PIN_ERRLOC_FM,
                        PIN_ERRCLASS_SYSTEM_DETERMINATE,
                        PIN_ERR_BAD_OPCODE, 0, 0, opcode);
						
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                        "bad opcode in op_cust_sub_acct_reg", ebufp);
                return;
        }

        /***********************************************************
         * Debug: What did we get?
         ***********************************************************/
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                "op_cust_sub_acct_reg input flist", i_flistp);

	/*******************************************************************
         * Call the default implementation
         *******************************************************************/

	fm_cust_sub_acct_reg(ctxp, i_flistp, &r_flistp, ebufp );
		
	/***********************************************************
	** Results.
	************************************************************/

	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"op_cust_sub_acct_reg error", ebufp);
		*r_flistpp = NULL; 
	}
	else
	{
		 *r_flistpp = PIN_FLIST_COPY(r_flistp, ebufp);
		 PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,  "at end op_cust_sub_acct_reg", *r_flistpp);
	}
	PIN_FLIST_DESTROY_EX(&r_flistp, NULL);	
	return;	
}

static void
fm_cust_sub_acct_reg(
	pcm_context_t           *ctxp,
	pin_flist_t             *i_flistp,
	pin_flist_t             **r_flistp,
	pin_errbuf_t            *ebufp)
{
	pin_flist_t             *in_flistp = NULL;
	pin_flist_t             *ret_flistp = NULL;
	pin_flist_t		*loc_flistp = NULL;
	pin_flist_t             *plan_pdp = NULL;
	pin_flist_t             *nameinfo_flistp = NULL;
	char                    *title = NULL;
	pin_flist_t             *acc_info_flistp = NULL;
	pin_flist_t             *bal_info_flistp = NULL;
	pin_flist_t             *bill_info_flistp = NULL;
	pin_flist_t		*grpinfo_flistp = NULL;
	pin_flist_t		*grp_info_flistp = NULL;
	char			*account_no = NULL;
	char			*name = NULL;	
	int64			db;

	poid_t			*parent_pdp = NULL;
	pin_flist_t		*r_oflistp = NULL;
	char 			account_no_str[BUFSIZ];
	poid_t			*poidp = NULL;
	char			*msd_idp = NULL;
	pin_cookie_t    	cookie = NULL;
	int32           	elemid = 0;
	pin_flist_t		*subord_flistp = NULL;
	pin_cookie_t            s_cookie = NULL;
        int32                   s_elemid = 0;
	int			subord = 10007;
	pin_flist_t		*bill_info1_flistp = NULL;
	pin_flist_t		*pbillinfo_flistp = NULL;
	pin_flist_t		*res_flistp = NULL;
	pin_flist_t		*bal_info2_flistp = NULL;
	pin_flist_t		*bal_info1_flistp = NULL;
	pin_flist_t		*out_flistp = NULL;
	pin_flist_t		*c_flistp = NULL;
	pin_flist_t		*concat_flistp = NULL;
	int32			flags = 0;

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "start fm_cust_commit_customer_reg_sub input_flist", i_flistp);


	/*grpinfo_flistp = PIN_FLIST_SUBSTR_GET(i_flistp, PIN_FLD_GROUP_INFO, 0, ebufp);
	if(grpinfo_flistp != NULL) {
		parent_pdp = PIN_FLIST_FLD_GET(grpinfo_flistp, PIN_FLD_PARENT, 1, ebufp);	
	}*/
	
	//mrb_get_read_obj_details(ctxp, parent_pdp, &r_oflistp, ebufp );
	mrb_get_account_details(ctxp, i_flistp, &r_oflistp, ebufp);
	if (PIN_ERR_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"after calling mrb_get_read_obj_details error", ebufp);
		goto CLEANUP;
	}
	if(r_oflistp == NULL) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                        "ERROR : Parent Account_no not found in Database", ebufp);
		pin_set_err(ebufp, PIN_ERRLOC_FM,
                        PIN_ERRCLASS_SYSTEM_DETERMINATE,
                        	PIN_ERR_NOT_FOUND, 0, 0, 0);	
		goto CLEANUP;
	}

	parent_pdp = PIN_FLIST_FLD_GET(r_oflistp, PIN_FLD_POID, 0, ebufp);
	mrb_get_parent_billinfo_details(ctxp, parent_pdp, &pbillinfo_flistp, ebufp);
	if (PIN_ERR_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"after calling mrb_get_parent_billinfo_details error", ebufp);
		goto CLEANUP;
	}
	
	while((subord_flistp = PIN_FLIST_ELEM_GET_NEXT(i_flistp, PIN_FLD_SUBORD_INFO, &s_elemid, 1, &s_cookie, ebufp)) != (pin_flist_t *)NULL)
	{
		in_flistp = PIN_FLIST_CREATE(ebufp);
		plan_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp);
		db = PIN_POID_GET_DB(plan_pdp);

		PIN_FLIST_FLD_SET(in_flistp, PIN_FLD_POID, plan_pdp, ebufp);
		PIN_FLIST_FLD_SET(in_flistp, PIN_FLD_FLAGS, &flags, ebufp);
		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_END_T, in_flistp, PIN_FLD_END_T, ebufp);

		loc_flistp = PIN_FLIST_ELEM_ADD(in_flistp, PIN_FLD_LOCALES, 1, ebufp);
		PIN_FLIST_FLD_SET(loc_flistp, PIN_FLD_LOCALE, "en_US", ebufp);
	
		cookie = NULL;
		while((nameinfo_flistp = PIN_FLIST_ELEM_GET_NEXT(r_oflistp, PIN_FLD_NAMEINFO, &elemid, 1, &cookie, ebufp)) != (pin_flist_t *)NULL)
		{
			PIN_FLIST_ELEM_COPY(r_oflistp, PIN_FLD_NAMEINFO, elemid, in_flistp, PIN_FLD_NAMEINFO, elemid, ebufp);
		}			
		acc_info_flistp = PIN_FLIST_ELEM_ADD(in_flistp, PIN_FLD_ACCTINFO, 0, ebufp);
		account_no = PIN_FLIST_FLD_GET(r_oflistp, PIN_FLD_ACCOUNT_NO, 0, ebufp);
		msd_idp = PIN_FLIST_FLD_GET(subord_flistp, PIN_FLD_MSID, 1, ebufp);	
		memset(account_no_str, '\0', sizeof(account_no_str));
		sprintf(account_no_str, "%s_M%s",  account_no, msd_idp);
		PIN_FLIST_FLD_SET(acc_info_flistp, PIN_FLD_ACCOUNT_NO, account_no_str, ebufp);
		poidp = PIN_POID_CREATE(db, "/account", -1, ebufp);
		PIN_FLIST_FLD_PUT(acc_info_flistp, PIN_FLD_POID, poidp, ebufp);
		PIN_FLIST_FLD_COPY(r_oflistp, PIN_FLD_GL_SEGMENT, acc_info_flistp, PIN_FLD_GL_SEGMENT, ebufp);
		PIN_FLIST_FLD_COPY(r_oflistp, PIN_FLD_ACTG_TYPE, acc_info_flistp, PIN_FLD_ACTG_TYPE, ebufp);
		PIN_FLIST_FLD_COPY(r_oflistp, PIN_FLD_CURRENCY, acc_info_flistp, PIN_FLD_CURRENCY, ebufp);	
		PIN_FLIST_FLD_COPY(r_oflistp, PIN_FLD_BUSINESS_TYPE, acc_info_flistp, PIN_FLD_BUSINESS_TYPE, ebufp);	
		bal_info_flistp = PIN_FLIST_ELEM_ADD(acc_info_flistp, PIN_FLD_BAL_INFO, 0, ebufp);

		res_flistp = PIN_FLIST_ELEM_GET(pbillinfo_flistp, PIN_FLD_RESULTS,
						PIN_ELEMID_ANY, 1, ebufp);
		if(res_flistp != NULL) {	
			bill_info_flistp = PIN_FLIST_ELEM_ADD(in_flistp, PIN_FLD_BILLINFO, 0, ebufp);
			poidp = PIN_POID_CREATE(db, "/billinfo", -1, ebufp);
			PIN_FLIST_FLD_PUT(bill_info_flistp, PIN_FLD_POID, poidp, ebufp);	
			PIN_FLIST_FLD_COPY(res_flistp, PIN_FLD_ACTG_CYCLE_DOM, bill_info_flistp, PIN_FLD_ACTG_FUTURE_DOM, ebufp);
			PIN_FLIST_FLD_COPY(res_flistp, PIN_FLD_BILLINFO_ID, bill_info_flistp, PIN_FLD_BILLINFO_ID, ebufp);
			PIN_FLIST_FLD_COPY(res_flistp, PIN_FLD_BILL_WHEN, bill_info_flistp, PIN_FLD_BILL_WHEN, ebufp);
			PIN_FLIST_FLD_SET(bill_info_flistp, PIN_FLD_PAY_TYPE, &subord, ebufp);
			PIN_FLIST_FLD_COPY(res_flistp, PIN_FLD_POID, bill_info_flistp, PIN_FLD_PARENT_BILLINFO_OBJ, ebufp);
			bal_info2_flistp = PIN_FLIST_ELEM_ADD(bill_info_flistp, PIN_FLD_BAL_INFO, 0, ebufp);
		}

		bal_info1_flistp = PIN_FLIST_ELEM_ADD(in_flistp, PIN_FLD_BAL_INFO, 0, ebufp);
		poidp = PIN_POID_CREATE(db, "/balance_group", -1, ebufp);
		PIN_FLIST_FLD_PUT(bal_info1_flistp, PIN_FLD_POID, poidp, ebufp);
		PIN_FLIST_FLD_SET(bal_info1_flistp, PIN_FLD_NAME, "bal_grp child", ebufp);
		bill_info1_flistp = PIN_FLIST_ELEM_ADD(bal_info1_flistp, PIN_FLD_BILLINFO, 0, ebufp);	

		//grpinfo_flistp = PIN_FLIST_SUBSTR_GET(i_flistp, PIN_FLD_GROUP_INFO, 0, ebufp);
		grp_info_flistp = PIN_FLIST_SUBSTR_ADD(in_flistp, PIN_FLD_GROUP_INFO, ebufp);
		PIN_FLIST_FLD_COPY(r_oflistp, PIN_FLD_POID, grp_info_flistp, PIN_FLD_PARENT, ebufp);

		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "end fm_cust_commit_customer_reg_sub input_flist", in_flistp);
	       
		PCM_OP(ctxp, PCM_OP_CUST_COMMIT_CUSTOMER, 0, in_flistp, &ret_flistp, ebufp);
		if(PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "fm_cust_commit_customer_reg_sub error after excecution", ebufp);
			goto CLEANUP;
		}
		PIN_ERR_LOG_FLIST(3, "Register billing sub account return flist: ", ret_flistp);

/***********mrb_create_service_details***********/
		mrb_create_service_details(ctxp, i_flistp, subord_flistp, ret_flistp, &out_flistp, ebufp);
		if(PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error : after calling mrb_create_service_details", ebufp);
			goto CLEANUP;
		}
                if(concat_flistp == NULL) {
                        concat_flistp = PIN_FLIST_CREATE(ebufp);
                        PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, concat_flistp, PIN_FLD_POID, ebufp);

                }
                c_flistp = PIN_FLIST_ELEM_ADD(concat_flistp, PIN_FLD_SUBORD_INFO, s_elemid, ebufp);
                PIN_FLIST_CONCAT(c_flistp, out_flistp, ebufp);

		PIN_FLIST_DESTROY_EX(&in_flistp, NULL);
		PIN_FLIST_DESTROY_EX(&ret_flistp, NULL);	
		PIN_FLIST_DESTROY_EX(&out_flistp, NULL);
	}
	*r_flistp = PIN_FLIST_COPY(concat_flistp, ebufp);

	CLEANUP:
	PIN_FLIST_DESTROY_EX(&pbillinfo_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&r_oflistp, NULL);
	PIN_FLIST_DESTROY_EX(&in_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&out_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&ret_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&concat_flistp, NULL);
	
	return;	
		
}

void
mrb_get_read_obj_details(
	pcm_context_t           *ctxp,
	poid_t		         *i_pdp,
	pin_flist_t             **r_flistp,
	pin_errbuf_t            *ebufp)
{

	pin_flist_t		*out_flistp = NULL;
	pin_flist_t              *in_flistp = NULL;

	if (PIN_ERRBUF_IS_ERR(ebufp)) {
        	return;
        }
        PIN_ERRBUF_CLEAR(ebufp);

	in_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_SET(in_flistp, PIN_FLD_POID, i_pdp, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "mrb_get_read_obj_details : Input flist", in_flistp);	
	PCM_OP(ctxp, PCM_OP_READ_OBJ, 0, in_flistp, &out_flistp, ebufp);
	PIN_FLIST_DESTROY_EX(&in_flistp, NULL);

	if (PIN_ERR_IS_ERR(ebufp)) {
		PIN_FLIST_DESTROY_EX(&out_flistp, NULL);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"mrb_get_read_obj_details error", ebufp);
		return;
        }
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "mrb_get_read_obj_details : Output flist", out_flistp);
	*r_flistp = PIN_FLIST_COPY(out_flistp, ebufp);
	PIN_FLIST_DESTROY_EX(&out_flistp, NULL);
	return;
}

void
mrb_get_parent_billinfo_details(
        pcm_context_t           *ctxp,
        poid_t                   *i_pdp,
        pin_flist_t             **r_flistp,
        pin_errbuf_t            *ebufp)
{

        pin_flist_t              *out_flistp = NULL;
	pin_flist_t		*search_flistp = NULL;
	pin_flist_t		*flistp = NULL;
	int			flag = 0;
	int32			database = 0;
	poid_t			*search_pdp = NULL;
	char			*search_str = "select X from /billinfo where F1 = V1 ";

        if (PIN_ERRBUF_IS_ERR(ebufp)) {
                return;
        }
        PIN_ERRBUF_CLEAR(ebufp);

	search_flistp = PIN_FLIST_CREATE(ebufp);
	database = PIN_POID_GET_DB(i_pdp);
	search_pdp = PIN_POID_CREATE(database, "/search", -1, ebufp);
	PIN_FLIST_FLD_PUT(search_flistp, PIN_FLD_POID, (void *)search_pdp, ebufp);

	PIN_FLIST_FLD_SET(search_flistp, PIN_FLD_TEMPLATE, (void *)search_str, ebufp);

	flag = SRCH_EXACT;
	PIN_FLIST_FLD_SET(search_flistp, PIN_FLD_FLAGS, &flag, ebufp);

	flistp = PIN_FLIST_ELEM_ADD(search_flistp, PIN_FLD_ARGS, 1, ebufp);
	PIN_FLIST_FLD_SET(flistp, PIN_FLD_ACCOUNT_OBJ, i_pdp, ebufp);

	PIN_FLIST_ELEM_SET(search_flistp, NULL, PIN_FLD_RESULTS, 0, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "mrb_get_parent_billinfo_details : input flist", search_flistp);

	PCM_OP(ctxp, PCM_OP_SEARCH, PCM_OPFLG_CACHEABLE, search_flistp, &out_flistp, ebufp);

        PIN_FLIST_DESTROY_EX(&search_flistp, NULL);

        if (PIN_ERR_IS_ERR(ebufp)) {
                PIN_FLIST_DESTROY_EX(&out_flistp, NULL);
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                        "mrb_get_parent_billinfo_details error", ebufp);
                return;
        }
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "mrb_get_parent_billinfo_details : Output flist", out_flistp);
        *r_flistp = PIN_FLIST_COPY(out_flistp, ebufp);
        PIN_FLIST_DESTROY_EX(&out_flistp, NULL);
        return;
}

void
mrb_get_deal_details(
        pcm_context_t           *ctxp,
        char                    *namep,
        pin_flist_t             **r_flistp,
        pin_errbuf_t            *ebufp)
{

        pin_flist_t              *out_flistp = NULL;
        pin_flist_t             *search_flistp = NULL;
        pin_flist_t             *flistp = NULL;
        int                     flag = 0;
        int32                   database = 1;
        poid_t                  *search_pdp = NULL;
        char                    *search_str = "select X from /deal where F1 = V1 ";

        if (PIN_ERRBUF_IS_ERR(ebufp)) {
                return;
        }
        PIN_ERRBUF_CLEAR(ebufp);

        search_flistp = PIN_FLIST_CREATE(ebufp);
        search_pdp = PIN_POID_CREATE(database, "/search", -1, ebufp);
        PIN_FLIST_FLD_PUT(search_flistp, PIN_FLD_POID, (void *)search_pdp, ebufp);

        PIN_FLIST_FLD_SET(search_flistp, PIN_FLD_TEMPLATE, (void *)search_str, ebufp);

        flag = SRCH_EXACT;
        PIN_FLIST_FLD_SET(search_flistp, PIN_FLD_FLAGS, &flag, ebufp);

        flistp = PIN_FLIST_ELEM_ADD(search_flistp, PIN_FLD_ARGS, 1, ebufp);
        PIN_FLIST_FLD_SET(flistp, PIN_FLD_NAME, namep, ebufp);

        PIN_FLIST_ELEM_SET(search_flistp, NULL, PIN_FLD_RESULTS, 0, ebufp);

        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "mrb_get_deal_details : input flist", search_flistp);

        PCM_OP(ctxp, PCM_OP_SEARCH, PCM_OPFLG_CACHEABLE, search_flistp, &out_flistp, ebufp);

        PIN_FLIST_DESTROY_EX(&search_flistp, NULL);


        if (PIN_ERR_IS_ERR(ebufp)) {
                PIN_FLIST_DESTROY_EX(&out_flistp, NULL);
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                        "mrb_get_deal_details error", ebufp);
                return;
        }
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "mrb_get_deal_details : Output flist", out_flistp);

	flistp = PIN_FLIST_ELEM_GET(out_flistp, PIN_FLD_RESULTS, 0, 1, ebufp);		
	if(flistp) {
        	*r_flistp = PIN_FLIST_COPY(flistp, ebufp);
	}
        PIN_FLIST_DESTROY_EX(&out_flistp, NULL);
        return;
}

static void
mrb_create_service_details(
         pcm_context_t           *ctxp,
         pin_flist_t             *i_flistp,
	 pin_flist_t             *subord_flistp,
	 pin_flist_t		 *ret_flistp,
         pin_flist_t             **r_flistp,
         pin_errbuf_t            *ebufp)
    {
	int64                   db = 0;
	poid_t                  *poidp = NULL;
	pin_cookie_t            cookie = NULL;
	int32                   elemid = 0;
	pin_flist_t             *bill_info1_flistp = NULL;
	pin_flist_t             *pbillinfo_flistp = NULL;
	pin_flist_t             *res_flistp = NULL;
	pin_flist_t             *bal_info2_flistp = NULL;
	pin_flist_t             *arg1_flistp = NULL;
	pin_flist_t             *arg2_flistp = NULL;
	int                     subscr_index = 1;
	int                     state_id = 1;
	int                     bal_index = 1;
	pin_flist_t             *serv_flistp = NULL;
	pin_flist_t             *temp_flistp = NULL;
	pin_flist_t             *arg_flistp = NULL;
	static char             *deal_name = NULL;
	pin_flist_t             *deal_flistp = NULL;
	int32                   err = 0;
	pin_flist_t             *serv_iflistp = NULL;
	pin_flist_t		*bal_info_flistp = NULL;
	pin_flist_t		*serv_oflistp = NULL;
	pin_flist_t		*inh_info_flistp = NULL;
	pin_flist_t		*serv_ip_flistp = NULL;

        if (PIN_ERRBUF_IS_ERR(ebufp)) {
                return;
        }
        PIN_ERRBUF_CLEAR(ebufp);

	poidp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp);
        db = PIN_POID_GET_DB(poidp);
	poidp = NULL;

	serv_iflistp = PIN_FLIST_CREATE(ebufp);
	serv_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, serv_iflistp, PIN_FLD_POID, ebufp);
	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_END_T, serv_iflistp, PIN_FLD_END_T, ebufp);
	temp_flistp = PIN_FLIST_ELEM_GET(ret_flistp, PIN_FLD_BAL_INFO, PIN_ELEMID_ANY, 1, ebufp);
	if(temp_flistp) {
		PIN_FLIST_FLD_COPY(temp_flistp, PIN_FLD_ACCOUNT_OBJ, serv_iflistp, PIN_FLD_ACCOUNT_OBJ, ebufp);
		bal_info_flistp = PIN_FLIST_ELEM_ADD(serv_iflistp, PIN_FLD_BAL_INFO, 0, ebufp);
		//PIN_FLIST_FLD_COPY(temp_flistp, PIN_FLD_POID, bal_info_flistp, PIN_FLD_POID, ebufp);
		poidp = PIN_POID_CREATE(db, "/balance_group", -1, ebufp);
		PIN_FLIST_FLD_PUT(bal_info_flistp, PIN_FLD_POID, poidp, ebufp);
		PIN_FLIST_FLD_COPY(temp_flistp, PIN_FLD_BILLINFO_OBJ, bal_info_flistp, PIN_FLD_BILLINFO_OBJ, ebufp);
		PIN_FLIST_FLD_SET(bal_info_flistp, PIN_FLD_NAME, "bal_grp child at service level", ebufp);

	}
	arg_flistp = PIN_FLIST_ELEM_ADD(serv_flistp, PIN_FLD_SERVICES, 0, ebufp);
	PIN_FLIST_FLD_COPY(subord_flistp, PIN_FLD_MSID, arg_flistp, PIN_FLD_LOGIN, ebufp);
	poidp = PIN_POID_CREATE(db, "/service/subscription", -1, ebufp);
	PIN_FLIST_FLD_PUT(arg_flistp, PIN_FLD_SUBSCRIPTION_OBJ, poidp, ebufp);
	PIN_FLIST_FLD_SET(arg_flistp, PIN_FLD_SUBSCRIPTION_INDEX, &subscr_index, ebufp);
	PIN_FLIST_FLD_SET(arg_flistp, PIN_FLD_SERVICE_ID, "", ebufp);
	arg1_flistp = PIN_FLIST_ELEM_ADD(arg_flistp, PIN_FLD_SERVICE_CODES, 0, ebufp);
	PIN_FLIST_FLD_SET(arg1_flistp, PIN_FLD_STATE_ID, &state_id, ebufp);
	PIN_FLIST_FLD_SET(arg_flistp, PIN_FLD_BAL_INFO_INDEX, &bal_index, ebufp);
	bal_info2_flistp = PIN_FLIST_ELEM_ADD(arg_flistp, PIN_FLD_BAL_INFO, 0, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "serv flist", subord_flistp);	
	temp_flistp = PIN_FLIST_ELEM_GET(subord_flistp, PIN_FLD_SERVICES, PIN_ELEMID_ANY, 1, ebufp);
	cookie = NULL;
	while((arg1_flistp = PIN_FLIST_ELEM_GET_NEXT(temp_flistp, PIN_FLD_ALIAS_LIST, &elemid, 1, &cookie, ebufp)) != (pin_flist_t *)NULL)
	{
		arg2_flistp = PIN_FLIST_ELEM_GET(serv_flistp, PIN_FLD_SERVICES, PIN_ELEMID_ANY, 1, ebufp);
		PIN_FLIST_ELEM_COPY(temp_flistp, PIN_FLD_ALIAS_LIST, elemid, arg2_flistp, PIN_FLD_ALIAS_LIST, elemid, ebufp);
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "serv_flistp details", serv_flistp);

	//Adding service/subscription
	elemid=1;
	PIN_FLIST_ELEM_COPY(serv_flistp, PIN_FLD_SERVICES, 0, serv_iflistp, PIN_FLD_SERVICES, elemid, ebufp);
	poidp = PIN_POID_CREATE(db, "/service/subscription", -1, ebufp);
	arg2_flistp = PIN_FLIST_ELEM_GET(serv_iflistp, PIN_FLD_SERVICES, elemid, 1, ebufp);
	PIN_FLIST_FLD_PUT(arg2_flistp, PIN_FLD_SERVICE_OBJ, poidp, ebufp);
	PIN_FLIST_FLD_SET(arg2_flistp, PIN_FLD_SUBSCRIPTION_OBJ, NULL, ebufp);
	inh_info_flistp = PIN_FLIST_SUBSTR_ADD(arg2_flistp, PIN_FLD_INHERITED_INFO, ebufp);
	serv_ip_flistp = PIN_FLIST_SUBSTR_ADD(inh_info_flistp, PIN_FLD_SERVICE_IP, ebufp);
	PIN_FLIST_FLD_SET(serv_ip_flistp, PIN_FLD_STATUS_MSG, "ACT", ebufp);
	
	/*temp_flistp = PIN_FLIST_ELEM_GET(subord_flistp, PIN_FLD_SERVICES, PIN_ELEMID_ANY, 1, ebufp);
	if(temp_flistp) {
		arg_flistp = PIN_FLIST_SUBSTR_GET(temp_flistp, PIN_FLD_INHERITED_INFO, 0, ebufp);
		if(arg_flistp) {
			PIN_FLIST_SUBSTR_SET(arg2_flistp, arg_flistp, PIN_FLD_INHERITED_INFO, ebufp);
		}
	}*/
	PIN_FLIST_ELEM_COPY(subord_flistp, PIN_FLD_PROFILES, 0, arg2_flistp, PIN_FLD_PROFILES, 0, ebufp);

	//Adding service/telco/gsm/telephony
	elemid++;
	PIN_FLIST_ELEM_COPY(serv_flistp, PIN_FLD_SERVICES, 0, serv_iflistp, PIN_FLD_SERVICES, elemid, ebufp);
	poidp = PIN_POID_CREATE(db, "/service/telco/gsm/telephony", -1, ebufp);
	arg2_flistp = PIN_FLIST_ELEM_GET(serv_iflistp, PIN_FLD_SERVICES, elemid, 1, ebufp);
	PIN_FLIST_FLD_PUT(arg2_flistp, PIN_FLD_SERVICE_OBJ, poidp, ebufp);
	pin_conf("mrb_global_deal", "/service/telco/gsm/telephony", PIN_FLDT_STR, (caddr_t *)&deal_name, &err);
	mrb_get_deal_details(ctxp, deal_name, &deal_flistp, ebufp);
	if (PIN_ERR_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"after calling mrb_get_deal_details error", ebufp);
		return;
	}
	if(deal_flistp) {
		PIN_FLIST_FLD_COPY(deal_flistp, PIN_FLD_POID, arg2_flistp, PIN_FLD_DEAL_OBJ, ebufp);
	}

	//Adding /service/telco/gsm/sms
	elemid++;
	PIN_FLIST_ELEM_COPY(serv_flistp, PIN_FLD_SERVICES, 0, serv_iflistp, PIN_FLD_SERVICES, elemid, ebufp);
	poidp = PIN_POID_CREATE(db, "/service/telco/gsm/sms", -1, ebufp);
	arg2_flistp = PIN_FLIST_ELEM_GET(serv_iflistp, PIN_FLD_SERVICES, elemid, 1, ebufp);
	PIN_FLIST_FLD_PUT(arg2_flistp, PIN_FLD_SERVICE_OBJ, poidp, ebufp);
	pin_conf("mrb_global_deal", "/service/telco/gsm/sms", PIN_FLDT_STR, (caddr_t *)&deal_name, &err);
	mrb_get_deal_details(ctxp, deal_name, &deal_flistp, ebufp);
	if (PIN_ERR_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"after calling mrb_get_deal_details error", ebufp);
		return;
	}

	if(deal_flistp) {
		PIN_FLIST_FLD_COPY(deal_flistp, PIN_FLD_POID, arg2_flistp, PIN_FLD_DEAL_OBJ, ebufp);
	}

	//Adding /service/telco/gsm/mms
	elemid++;
	PIN_FLIST_ELEM_COPY(serv_flistp, PIN_FLD_SERVICES, 0, serv_iflistp, PIN_FLD_SERVICES, elemid, ebufp);
	poidp = PIN_POID_CREATE(db, "/service/telco/gsm/mms", -1, ebufp);
	arg2_flistp = PIN_FLIST_ELEM_GET(serv_iflistp, PIN_FLD_SERVICES, elemid, 1, ebufp);
	PIN_FLIST_FLD_PUT(arg2_flistp, PIN_FLD_SERVICE_OBJ, poidp, ebufp);
	pin_conf("mrb_global_deal", "/service/telco/gsm/mms", PIN_FLDT_STR, (caddr_t *)&deal_name, &err);
	mrb_get_deal_details(ctxp, deal_name, &deal_flistp, ebufp);
	if (PIN_ERR_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"after calling mrb_get_deal_details error", ebufp);
		return;
	}
	if(deal_flistp) {
		PIN_FLIST_FLD_COPY(deal_flistp, PIN_FLD_POID, arg2_flistp, PIN_FLD_DEAL_OBJ, ebufp);
	}

	//Adding /service/telco/gsm/data
	elemid++;
	PIN_FLIST_ELEM_COPY(serv_flistp, PIN_FLD_SERVICES, 0, serv_iflistp, PIN_FLD_SERVICES, elemid, ebufp);
	poidp = PIN_POID_CREATE(db, "/service/telco/gsm/data", -1, ebufp);
	arg2_flistp = PIN_FLIST_ELEM_GET(serv_iflistp, PIN_FLD_SERVICES, elemid, 1, ebufp);
	PIN_FLIST_FLD_PUT(arg2_flistp, PIN_FLD_SERVICE_OBJ, poidp, ebufp);
	pin_conf("mrb_global_deal", "/service/telco/gsm/data", PIN_FLDT_STR, (caddr_t *)&deal_name, &err);
	mrb_get_deal_details(ctxp, deal_name, &deal_flistp, ebufp);
	if (PIN_ERR_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"after calling mrb_get_deal_details error", ebufp);
		return;
	}
	if(deal_flistp) {
		PIN_FLIST_FLD_COPY(deal_flistp, PIN_FLD_POID, arg2_flistp, PIN_FLD_DEAL_OBJ, ebufp);
	}

	//Adding /service/broadband
	elemid++;
	PIN_FLIST_ELEM_COPY(serv_flistp, PIN_FLD_SERVICES, 0, serv_iflistp, PIN_FLD_SERVICES, elemid, ebufp);
	poidp = PIN_POID_CREATE(db, "/service/broadband", -1, ebufp);
	arg2_flistp = PIN_FLIST_ELEM_GET(serv_iflistp, PIN_FLD_SERVICES, elemid, 1, ebufp);
	PIN_FLIST_FLD_PUT(arg2_flistp, PIN_FLD_SERVICE_OBJ, poidp, ebufp);
	/*pin_conf("mrb_global_deal", "/service/broadband", PIN_FLDT_STR, (caddr_t *)&deal_name, &err);
	mrb_get_deal_details(ctxp, deal_name, &deal_flistp, ebufp);
	if (PIN_ERR_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"after calling mrb_get_deal_details error", ebufp);
		return;
	}
	if(deal_flistp) {
		PIN_FLIST_FLD_COPY(deal_flistp, PIN_FLD_POID, arg2_flistp, PIN_FLD_DEAL_OBJ, ebufp);
	} */

	//Adding /service/telco/gsm/data/mc
	elemid++;
	PIN_FLIST_ELEM_COPY(serv_flistp, PIN_FLD_SERVICES, 0, serv_iflistp, PIN_FLD_SERVICES, elemid, ebufp);
	poidp = PIN_POID_CREATE(db, "/service/telco/gsm/data/mc", -1, ebufp);
	arg2_flistp = PIN_FLIST_ELEM_GET(serv_iflistp, PIN_FLD_SERVICES, elemid, 1, ebufp);
	PIN_FLIST_FLD_PUT(arg2_flistp, PIN_FLD_SERVICE_OBJ, poidp, ebufp);
	/*pin_conf("mrb_global_deal", "/service/telco/gsm/data/mc", PIN_FLDT_STR, (caddr_t *)&deal_name, &err);
	mrb_get_deal_details(ctxp, deal_name, &deal_flistp, ebufp);
	if (PIN_ERR_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"after calling mrb_get_deal_details error", ebufp);
		return;
	}
	if(deal_flistp) {
		PIN_FLIST_FLD_COPY(deal_flistp, PIN_FLD_POID, arg2_flistp, PIN_FLD_DEAL_OBJ, ebufp);
	}*/

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_cust_commit_customer_reg_sub Service creation input_flist", serv_iflistp);

	PCM_OP(ctxp, PCM_OP_CUST_MODIFY_CUSTOMER, 0, serv_iflistp, &serv_oflistp, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_cust_commit_customer_reg_sub Service creation out_flist", serv_oflistp);
	if(PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "fm_cust_commit_customer_reg_sub error after excecution", ebufp);
		goto CLEANUP;
	}


	*r_flistp = PIN_FLIST_COPY(serv_oflistp, ebufp);
	CLEANUP:
	PIN_FLIST_DESTROY_EX(&serv_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&serv_iflistp, NULL);
	PIN_FLIST_DESTROY_EX(&serv_oflistp, NULL);
	return;

}

void
mrb_get_account_details(
        pcm_context_t           *ctxp,
        pin_flist_t             *i_flistp,
        pin_flist_t             **r_flistp,
        pin_errbuf_t            *ebufp)
{

        pin_flist_t              *out_flistp = NULL;
        pin_flist_t             *search_flistp = NULL;
        pin_flist_t             *flistp = NULL;
        int                     flag = 0;
        int32                   database = 1;
        poid_t                  *search_pdp = NULL;
        char                    *search_str = "select X from /account where F1 = V1 ";

        if (PIN_ERRBUF_IS_ERR(ebufp)) {
                return;
        }
        PIN_ERRBUF_CLEAR(ebufp);

        search_flistp = PIN_FLIST_CREATE(ebufp);
        search_pdp = PIN_POID_CREATE(database, "/search", -1, ebufp);
        PIN_FLIST_FLD_PUT(search_flistp, PIN_FLD_POID, (void *)search_pdp, ebufp);

        PIN_FLIST_FLD_SET(search_flistp, PIN_FLD_TEMPLATE, (void *)search_str, ebufp);

        flag = SRCH_EXACT;
        PIN_FLIST_FLD_SET(search_flistp, PIN_FLD_FLAGS, &flag, ebufp);

        flistp = PIN_FLIST_ELEM_ADD(search_flistp, PIN_FLD_ARGS, 1, ebufp);
	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_ACCOUNT_NO, flistp, PIN_FLD_ACCOUNT_NO, ebufp);

        PIN_FLIST_ELEM_SET(search_flistp, NULL, PIN_FLD_RESULTS, 0, ebufp);

        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "mrb_get_account_details : input flist", search_flistp);

        PCM_OP(ctxp, PCM_OP_SEARCH, PCM_OPFLG_CACHEABLE, search_flistp, &out_flistp, ebufp);

        PIN_FLIST_DESTROY_EX(&search_flistp, NULL);


        if (PIN_ERR_IS_ERR(ebufp)) {
                PIN_FLIST_DESTROY_EX(&out_flistp, NULL);
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                        "mrb_get_deal_details error", ebufp);
                return;
        }
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "mrb_get_account_details : Output flist", out_flistp);

        flistp = PIN_FLIST_ELEM_GET(out_flistp, PIN_FLD_RESULTS, 0, 1, ebufp);
        if(flistp) {
                *r_flistp = PIN_FLIST_COPY(flistp, ebufp);
        }
        PIN_FLIST_DESTROY_EX(&out_flistp, NULL);
        return;
}
