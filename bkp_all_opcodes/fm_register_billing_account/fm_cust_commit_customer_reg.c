#ifndef lint
static  char Sccs_Id[] = "@(#)%Portal Version: fm_cust_commit_customer_reg.c:BillingVelocityInt:4:2006-Sep-05 04:28:17 %";
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
#include "ops/rbs.h"

#include "custom_flds/custom_flds.h"
#include "custom_wrapper_ops.h"


/*******************************************************************
 * Routines contained herein.
 *******************************************************************/
EXPORT_OP void
op_cust_commit_customer_reg(
        cm_nap_connection_t     *connp,
        int32                   opcode,
        int32                   flags,
        pin_flist_t             *i_flistp,
        pin_flist_t             **r_flistpp,
        pin_errbuf_t            *ebufp);
		

static void
fm_cust_commit_customer_reg(
         pcm_context_t           *ctxp,
         pin_flist_t             *i_flistp,
         pin_flist_t             **r_flistpp,
         pin_errbuf_t            *ebufp);
		
/**************************************************************************************************/

void
op_cust_commit_customer_reg(
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
        if (opcode != PCM_POL_CUST_COMMIT_CUSTOMER_REG) {
                pin_set_err(ebufp, PIN_ERRLOC_FM,
                        PIN_ERRCLASS_SYSTEM_DETERMINATE,
                        PIN_ERR_BAD_OPCODE, 0, 0, opcode);
						
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                        "bad opcode in op_cust_commit_customer_reg", ebufp);
                return;
        }

        /***********************************************************
         * Debug: What did we get?
         ***********************************************************/
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                "op_cust_commit_customer_reg input flist", i_flistp);

	/*******************************************************************
         * Call the default implementation
         *******************************************************************/
		 
		fm_cust_commit_customer_reg( ctxp ,i_flistp ,&r_flistp ,ebufp );
		
		
/***********************************************************
	** Results.
 ************************************************************/

    if (PIN_ERRBUF_IS_ERR(ebufp))
    {
        PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"op_mso_cust_get_accinfo error", ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "op_cust_commit_customer_reg input flist", i_flistp);
                PIN_ERRBUF_RESET(ebufp);
        *r_flistpp = PIN_FLIST_COPY(i_flistp, ebufp);
    }
    else
    {
   	 *r_flistpp = PIN_FLIST_COPY(r_flistp, ebufp);
         PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,  "op_cust_commit_customer_reg", *r_flistpp);
 
    }
	
	PIN_FLIST_DESTROY_EX(&r_flistp, NULL);	
	return;	
}

static void
fm_cust_commit_customer_reg(
        pcm_context_t           *ctxp,
        pin_flist_t             *i_flistp,
        pin_flist_t             **r_flistp,
        pin_errbuf_t            *ebufp)
{
	pin_flist_t		*in_flistp = NULL;
	pin_flist_t		*ret_flistp = NULL;
	pin_flist_t             *loc_flistp = NULL;
	pin_flist_t             *payinfo_flistp = NULL;
	pin_flist_t             *pay_info_flistp = NULL;
	pin_flist_t             *inherit_flistp = NULL;
	pin_flist_t             *inhrt_flistp = NULL;
	pin_flist_t             *acc_info = NULL;
	pin_flist_t             *acc_infop = NULL;
	pin_flist_t             *billinfo = NULL;
	pin_flist_t             *billinfop = NULL;
	pin_flist_t		*inv_flistp = NULL;
	pin_flist_t		*cc_infop = NULL;
	pin_flist_t		*dd_infop = NULL;
	pin_flist_t		*cc_info = NULL;
	pin_flist_t		*dd_info = NULL;
	pin_flist_t		*invc_flistp = NULL;
	pin_flist_t		*nameinfo_flistp = NULL;
	pin_flist_t		*nameinfop = NULL;
	pin_flist_t		*prof_arr = NULL;
	pin_flist_t		*prof_arrp = NULL;
	pin_flist_t		*inheritinfo_flistp = NULL;
	pin_flist_t		*inhrtinfo_flistp = NULL;
	pin_flist_t		*custcare_info_flistp = NULL;
	pin_flist_t		*custcare_flistp = NULL;
	pin_flist_t		*prof_arr1 = NULL;
	pin_flist_t		*prof_arrp1 = NULL;
	pin_flist_t             *prof_arr2 = NULL;
        pin_flist_t             *prof_arrp2 = NULL;
	pin_flist_t		*inheritinfo_flistp1 = NULL;
	pin_flist_t		*inhrtinfo_flistp1 = NULL;
	pin_flist_t		*billinfo_flistp = NULL;
	pin_flist_t		*attr_flistp = NULL;
	pin_flist_t		*attr_flistp1 = NULL;
	pin_flist_t		*acc_flistp = NULL;
	pin_flist_t		*acct_flistp = NULL;
	pin_flist_t		*bal_info = NULL;
	pin_flist_t		*bal_infop = NULL;
	int32			pay_type = NULL;
	int64			db = -1;
	poid_t			*plan_pdp = NULL;
	poid_t			*deal_pdp = NULL;
	poid_t			*billinfo_pdp = NULL;
	poid_t			*pay_pdp = NULL;
	char			*pay_pd_typ = NULL;
	char			*namep = NULL;
	int32			rec_id = 0;
	int32			elem_id = 0;
	pin_rec_id_t		cnt = 0;
        pin_cookie_t		cookie = NULL;
	char			msg[100];
	
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "start fm_cust_commit_customer_reg input_flist", i_flistp);
	
	in_flistp = PIN_FLIST_CREATE(ebufp);	
	
	plan_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp);
	db = PIN_POID_GET_DB(plan_pdp);
	deal_pdp = PIN_POID_CREATE(db, "/deal", -1, ebufp);
	billinfo_pdp = PIN_POID_CREATE(db, "/billinfo", -1, ebufp);
		
	PIN_FLIST_FLD_SET(in_flistp, PIN_FLD_POID, plan_pdp, ebufp);
	PIN_FLIST_FLD_SET(in_flistp, PIN_FLD_DEAL_OBJ, deal_pdp, ebufp);
	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_NAME, in_flistp, PIN_FLD_NAME, ebufp);
	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_FLAGS, in_flistp, PIN_FLD_FLAGS, ebufp);
	loc_flistp = PIN_FLIST_ELEM_ADD(in_flistp, PIN_FLD_LOCALES, 1, ebufp);
	PIN_FLIST_FLD_SET(loc_flistp, PIN_FLD_LOCALE, "en_US", ebufp);
 	
	payinfo_flistp = PIN_FLIST_ELEM_GET(i_flistp, PIN_FLD_PAYINFO, 0, 1, ebufp);	
	pay_info_flistp = PIN_FLIST_ELEM_ADD(in_flistp, PIN_FLD_PAYINFO, 0, ebufp);
	pay_pdp = PIN_FLIST_FLD_GET(payinfo_flistp, PIN_FLD_POID, 0, ebufp);
	PIN_FLIST_FLD_SET(pay_info_flistp, PIN_FLD_POID, pay_pdp, ebufp);
	PIN_FLIST_FLD_COPY(payinfo_flistp, PIN_FLD_FLAGS, pay_info_flistp, PIN_FLD_FLAGS, ebufp); 	
	pay_pd_typ = (char *)PIN_POID_GET_TYPE(pay_pdp); 
	if(pay_pd_typ && strcmp(pay_pd_typ, "/payinfo/invoice") == 0)
	{
		pay_type = 10001;
		namep = "Cash Invoice";
		PIN_FLIST_FLD_SET(pay_info_flistp, PIN_FLD_PAY_TYPE, &pay_type, ebufp); 
		PIN_FLIST_FLD_SET(pay_info_flistp, PIN_FLD_NAME, namep, ebufp);
	}
	else if(pay_pd_typ && strcmp(pay_pd_typ, "/payinfo/cc") == 0)
	{
		pay_type = 10003;
                namep = "CC Invoice";
                PIN_FLIST_FLD_SET(pay_info_flistp, PIN_FLD_PAY_TYPE, &pay_type, ebufp);
                PIN_FLIST_FLD_SET(pay_info_flistp, PIN_FLD_NAME, namep, ebufp);
	}
	else
	{
		pay_type = 10005;
                namep = "DD Invoice";
                PIN_FLIST_FLD_SET(pay_info_flistp, PIN_FLD_PAY_TYPE, &pay_type, ebufp);
                PIN_FLIST_FLD_SET(pay_info_flistp, PIN_FLD_NAME, namep, ebufp);
	}
	inherit_flistp = PIN_FLIST_SUBSTR_ADD(pay_info_flistp, PIN_FLD_INHERITED_INFO, ebufp);
	inhrt_flistp = PIN_FLIST_SUBSTR_GET(payinfo_flistp, PIN_FLD_INHERITED_INFO, 0, ebufp);
	inv_flistp = PIN_FLIST_ELEM_GET(inhrt_flistp, PIN_FLD_INV_INFO, 0, 1, ebufp);
	invc_flistp = PIN_FLIST_ELEM_ADD(inherit_flistp, PIN_FLD_INV_INFO, 0, ebufp);
	if(pay_type && pay_type == 10001)
	{
		PIN_FLIST_FLD_COPY(inv_flistp, PIN_FLD_ADDRESS, invc_flistp, PIN_FLD_ADDRESS, ebufp);
	}
	else if(pay_type && pay_type == 10003)
	{
		cc_infop = PIN_FLIST_ELEM_GET(inv_flistp, PIN_FLD_CC_INFO, 0, 1, ebufp);
		cc_info = PIN_FLIST_ELEM_ADD(invc_flistp, PIN_FLD_CC_INFO, 0, ebufp);
		PIN_FLIST_FLD_COPY(cc_infop, PIN_FLD_ADDRESS, cc_info, PIN_FLD_ADDRESS, ebufp);
		PIN_FLIST_FLD_COPY(cc_infop, PIN_FLD_CITY, cc_info, PIN_FLD_CITY, ebufp);
		PIN_FLIST_FLD_COPY(cc_infop, PIN_FLD_COUNTRY, cc_info, PIN_FLD_COUNTRY, ebufp);
		PIN_FLIST_FLD_COPY(cc_infop, PIN_FLD_DEBIT_EXP, cc_info, PIN_FLD_DEBIT_EXP, ebufp);
		PIN_FLIST_FLD_COPY(cc_infop, PIN_FLD_DEBIT_NUM, cc_info, PIN_FLD_DEBIT_NUM, ebufp);
		PIN_FLIST_FLD_COPY(cc_infop, PIN_FLD_NAME, cc_info, PIN_FLD_NAME, ebufp);
        }
	else 
	{
		dd_infop = PIN_FLIST_ELEM_GET(inv_flistp, PIN_FLD_DD_INFO, 0, 1, ebufp);
		dd_info = PIN_FLIST_ELEM_ADD(invc_flistp, PIN_FLD_DD_INFO, 0, ebufp);
		PIN_FLIST_FLD_COPY(dd_infop, PIN_FLD_ADDRESS, dd_info, PIN_FLD_ADDRESS, ebufp);
                PIN_FLIST_FLD_COPY(dd_infop, PIN_FLD_CITY, dd_info, PIN_FLD_CITY, ebufp);
                PIN_FLIST_FLD_COPY(dd_infop, PIN_FLD_COUNTRY, dd_info, PIN_FLD_COUNTRY, ebufp);
                PIN_FLIST_FLD_COPY(dd_infop, PIN_FLD_DEBIT_NUM, dd_info, PIN_FLD_DEBIT_NUM, ebufp);
                PIN_FLIST_FLD_COPY(dd_infop, PIN_FLD_NAME, dd_info, PIN_FLD_NAME, ebufp);
		PIN_FLIST_FLD_COPY(dd_infop, PIN_FLD_BANK_NO, dd_info, PIN_FLD_BANK_NO, ebufp);
		PIN_FLIST_FLD_COPY(dd_infop, PIN_FLD_TYPE, dd_info, PIN_FLD_TYPE, ebufp); 
	}

	PIN_ERR_LOG_FLIST(3, "input flist: ", in_flistp);
	nameinfo_flistp = PIN_FLIST_ELEM_GET(i_flistp, PIN_FLD_NAMEINFO, 1, 1, ebufp);
	nameinfop = PIN_FLIST_ELEM_ADD(in_flistp, PIN_FLD_NAMEINFO, 1, ebufp);
	PIN_FLIST_FLD_COPY(nameinfo_flistp, PIN_FLD_TITLE, nameinfop, PIN_FLD_TITLE, ebufp);
	PIN_FLIST_FLD_COPY(nameinfo_flistp, PIN_FLD_SALUTATION, nameinfop, PIN_FLD_SALUTATION, ebufp);	
        PIN_FLIST_FLD_COPY(nameinfo_flistp, PIN_FLD_FIRST_NAME, nameinfop, PIN_FLD_FIRST_NAME, ebufp);
	PIN_FLIST_FLD_COPY(nameinfo_flistp, PIN_FLD_LAST_NAME, nameinfop, PIN_FLD_LAST_NAME, ebufp);
	PIN_FLIST_FLD_COPY(nameinfo_flistp, PIN_FLD_MIDDLE_NAME, nameinfop, PIN_FLD_MIDDLE_NAME, ebufp);        
	PIN_FLIST_FLD_COPY(nameinfo_flistp, PIN_FLD_CONTACT_TYPE, nameinfop, PIN_FLD_CONTACT_TYPE, ebufp);
	PIN_FLIST_FLD_COPY(nameinfo_flistp, PIN_FLD_EMAIL_ADDR, nameinfop, PIN_FLD_EMAIL_ADDR, ebufp);
	PIN_FLIST_FLD_COPY(nameinfo_flistp, PIN_FLD_COUNTRY, nameinfop, PIN_FLD_COUNTRY, ebufp);
	PIN_FLIST_FLD_COPY(nameinfo_flistp, PIN_FLD_CITY, nameinfop, PIN_FLD_CITY, ebufp);
	PIN_FLIST_FLD_COPY(nameinfo_flistp, PIN_FLD_ADDRESS, nameinfop, PIN_FLD_ADDRESS, ebufp);
 	PIN_FLIST_FLD_COPY(nameinfo_flistp, PIN_FLD_COMPANY, nameinfop, PIN_FLD_COMPANY, ebufp);
	
	elem_id = 0;
	cookie = NULL;
	cnt = 0;
	while ((prof_arrp = PIN_FLIST_ELEM_GET_NEXT(i_flistp, PIN_FLD_PROFILES, &elem_id, 1, &cookie, ebufp)) != (pin_flist_t *)NULL)
        {
		PIN_FLIST_ELEM_COPY(i_flistp, PIN_FLD_PROFILES, cnt, in_flistp, PIN_FLD_PROFILES, cnt, ebufp);
		sprintf(msg, "count: %d", cnt); 
		PIN_ERR_LOG_MSG(3, msg);
		cnt++;
	}
/*
	prof_arr = PIN_FLIST_ELEM_GET(i_flistp, PIN_FLD_PROFILES, 0, 1, ebufp);
	prof_arrp = PIN_FLIST_ELEM_ADD(in_flistp, PIN_FLD_PROFILES, 0, ebufp);
	PIN_FLIST_FLD_COPY(prof_arr, PIN_FLD_PROFILE_OBJ, prof_arrp, PIN_FLD_PROFILE_OBJ, ebufp);
	inheritinfo_flistp = PIN_FLIST_SUBSTR_ADD(prof_arrp, PIN_FLD_INHERITED_INFO, ebufp);
        inhrtinfo_flistp = PIN_FLIST_SUBSTR_GET(prof_arr, PIN_FLD_INHERITED_INFO, 0, ebufp);
	custcare_info_flistp = PIN_FLIST_SUBSTR_ADD(inheritinfo_flistp, PIN_FLD_CUSTOMER_CARE_INFO, ebufp);
        custcare_flistp = PIN_FLIST_SUBSTR_GET(inhrtinfo_flistp, PIN_FLD_CUSTOMER_CARE_INFO, 0, ebufp);
	PIN_FLIST_FLD_COPY(custcare_flistp, PIN_FLD_CUSTOMER_TYPE, custcare_info_flistp, PIN_FLD_CUSTOMER_TYPE, ebufp);
	
	PIN_ERR_LOG_FLIST(3, "input flist1: ", in_flistp);
	PIN_FLIST_ELEM_COPY(i_flistp, PIN_FLD_PROFILES, 1, in_flistp, PIN_FLD_PROFILES, 1, ebufp);	
	prof_arr1 = PIN_FLIST_ELEM_GET(i_flistp, PIN_FLD_PROFILES, 1, 1, ebufp);
        prof_arrp1 = PIN_FLIST_ELEM_ADD(in_flistp, PIN_FLD_PROFILES, 1, ebufp);	
        PIN_FLIST_FLD_COPY(prof_arr1, PIN_FLD_PROFILE_OBJ, prof_arrp1, PIN_FLD_PROFILE_OBJ, ebufp);
        inheritinfo_flistp1 = PIN_FLIST_SUBSTR_ADD(prof_arrp1, PIN_FLD_INHERITED_INFO, ebufp);
        inhrtinfo_flistp1 = PIN_FLIST_SUBSTR_GET(prof_arr1, PIN_FLD_INHERITED_INFO, 0, ebufp);
	billinfo_flistp = PIN_FLIST_ELEM_GET(inhrtinfo_flistp1, PIN_FLD_BILLING_INFO, 0, 1, ebufp);
	billinfo = PIN_FLIST_ELEM_ADD(inheritinfo_flistp1, PIN_FLD_BILLING_INFO, 0, ebufp);
	PIN_FLIST_FLD_COPY(billinfo_flistp, MRB_FLD_INTERCOM_CCC, billinfo, MRB_FLD_INTERCOM_CCC, ebufp);	
	PIN_FLIST_FLD_COPY(billinfo_flistp, MRB_FLD_RECEIPT_ACCOUNT_NO, billinfo, MRB_FLD_RECEIPT_ACCOUNT_NO, ebufp);
	PIN_FLIST_FLD_COPY(billinfo_flistp, MRB_FLD_PAYMENT_NO, billinfo, MRB_FLD_PAYMENT_NO, ebufp);
	PIN_FLIST_FLD_COPY(billinfo_flistp, MRB_FLD_CCC, billinfo, MRB_FLD_CCC, ebufp);
	PIN_FLIST_FLD_COPY(billinfo_flistp, MRB_FLD_BILL_DIVERT_LOCATION, billinfo, MRB_FLD_BILL_DIVERT_LOCATION, ebufp);
	PIN_FLIST_FLD_COPY(billinfo_flistp, MRB_FLD_BILL_DIVERT_RMN_CYC, billinfo, MRB_FLD_BILL_DIVERT_RMN_CYC, ebufp);
	PIN_FLIST_FLD_COPY(billinfo_flistp, MRB_FLD_BILL_WHOLD_RMN_CYC, billinfo, MRB_FLD_BILL_WHOLD_RMN_CYC, ebufp);
	PIN_FLIST_FLD_COPY(billinfo_flistp, MRB_FLD_ITEMIZED_BILL_RMN_CYC, billinfo, MRB_FLD_ITEMIZED_BILL_RMN_CYC, ebufp);
	
	attr_flistp = PIN_FLIST_ELEM_GET(inhrtinfo_flistp1, PIN_FLD_ATTRIBUTES, 0, 1, ebufp);
	PIN_FLIST_ELEM_COPY(inhrtinfo_flistp1, PIN_FLD_ATTRIBUTES, 0, inheritinfo_flistp1, PIN_FLD_ATTRIBUTES, 0, ebufp);

        attr_flistp1 = PIN_FLIST_ELEM_ADD(inheritinfo_flistp1, PIN_FLD_ATTRIBUTES, rec_id, ebufp);
        PIN_FLIST_FLD_COPY(attr_flistp, PIN_FLD_STATUS, attr_flistp1, PIN_FLD_STATUS, ebufp);
	PIN_FLIST_FLD_COPY(attr_flistp, PIN_FLD_LOCALE, attr_flistp1, PIN_FLD_LOCALE, ebufp);
	
	if (PIN_FLIST_FLD_GET(attr_flistp, MRB_FLD_DELIVERY_DESCR5, 1, ebufp) != NULL)
	{
		PIN_FLIST_FLD_COPY(attr_flistp, MRB_FLD_DELIVERY_DESCR5, attr_flistp1, MRB_FLD_DELIVERY_DESCR5, ebufp);
	}
	if (PIN_FLIST_FLD_GET(attr_flistp, MRB_FLD_DELIVERY_DESCR4, 1, ebufp) != NULL)
        {	
		PIN_FLIST_FLD_COPY(attr_flistp, MRB_FLD_DELIVERY_DESCR4, attr_flistp1, MRB_FLD_DELIVERY_DESCR4, ebufp);
	}
	if (PIN_FLIST_FLD_GET(attr_flistp, MRB_FLD_DELIVERY_DESCR3, 1, ebufp) != NULL)
        {
		PIN_FLIST_FLD_COPY(attr_flistp, MRB_FLD_DELIVERY_DESCR3, attr_flistp1, MRB_FLD_DELIVERY_DESCR3, ebufp);
	}
        if (PIN_FLIST_FLD_GET(attr_flistp, MRB_FLD_DELIVERY_DESCR2, 1, ebufp) != NULL)
        {
		PIN_FLIST_FLD_COPY(attr_flistp, MRB_FLD_DELIVERY_DESCR2, attr_flistp1, MRB_FLD_DELIVERY_DESCR2, ebufp);
	}
        if (PIN_FLIST_FLD_GET(attr_flistp, MRB_FLD_DELIVERY_DESCR1, 1, ebufp) != NULL)
        {
		PIN_FLIST_FLD_COPY(attr_flistp, MRB_FLD_DELIVERY_DESCR1, attr_flistp1, MRB_FLD_DELIVERY_DESCR1, ebufp);
	}	

	PIN_ERR_LOG_FLIST(3, "input flist2: ", in_flistp);
	prof_arr2 = PIN_FLIST_ELEM_GET(i_flistp, PIN_FLD_PROFILES, 2, 1, ebufp);
        prof_arrp2 = PIN_FLIST_ELEM_ADD(in_flistp, PIN_FLD_PROFILES, 2, ebufp);
        PIN_FLIST_FLD_COPY(prof_arr2, PIN_FLD_PROFILE_OBJ, prof_arrp2, PIN_FLD_PROFILE_OBJ, ebufp);
	inheritinfo_flistp1 = PIN_FLIST_SUBSTR_ADD(prof_arrp2, PIN_FLD_INHERITED_INFO, ebufp);
        inhrtinfo_flistp1 = PIN_FLIST_SUBSTR_GET(prof_arr2, PIN_FLD_INHERITED_INFO, 0, ebufp);
	acc_flistp = PIN_FLIST_SUBSTR_GET(inhrtinfo_flistp1, PIN_FLD_ACCOUNT, 0, ebufp);	
	acct_flistp = PIN_FLIST_SUBSTR_ADD(inheritinfo_flistp1, PIN_FLD_ACCOUNT, ebufp);
        PIN_FLIST_FLD_COPY(acc_flistp, MRB_FLD_CUSTOMER_SEGMENT, acct_flistp, MRB_FLD_CUSTOMER_SEGMENT, ebufp);        
	PIN_FLIST_FLD_COPY(acc_flistp, MRB_FLD_PAY_METHOD_KEY, acct_flistp, MRB_FLD_PAY_METHOD_KEY, ebufp);
	PIN_FLIST_FLD_COPY(acc_flistp, MRB_FLD_CREDIT_STATUS, acct_flistp, MRB_FLD_CREDIT_STATUS, ebufp);
*/	
	bal_info = PIN_FLIST_ELEM_GET(i_flistp, PIN_FLD_BAL_INFO, 0, 1, ebufp);
	bal_infop = PIN_FLIST_ELEM_ADD(in_flistp, PIN_FLD_BAL_INFO, 0, ebufp);
	PIN_FLIST_FLD_COPY(bal_info, PIN_FLD_NAME, bal_infop, PIN_FLD_NAME, ebufp);
	PIN_FLIST_FLD_COPY(bal_info, PIN_FLD_POID, bal_infop, PIN_FLD_POID, ebufp);
	billinfo = PIN_FLIST_ELEM_GET(bal_info, PIN_FLD_BILLINFO, 0, 1, ebufp);
	billinfop = PIN_FLIST_ELEM_ADD(bal_infop, PIN_FLD_BILLINFO, 0, ebufp);
	
	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_DESCR, in_flistp, PIN_FLD_DESCR, ebufp);
	acc_info = PIN_FLIST_ELEM_GET(i_flistp, PIN_FLD_ACCTINFO, 0, 1, ebufp);
	acc_infop = PIN_FLIST_ELEM_ADD(in_flistp, PIN_FLD_ACCTINFO, 0, ebufp);
	PIN_FLIST_FLD_COPY(acc_info, PIN_FLD_ACCOUNT_NO, acc_infop, PIN_FLD_ACCOUNT_NO, ebufp);	
	PIN_FLIST_FLD_COPY(acc_info, PIN_FLD_POID, acc_infop, PIN_FLD_POID, ebufp);
	PIN_FLIST_FLD_COPY(acc_info, PIN_FLD_GL_SEGMENT, acc_infop, PIN_FLD_GL_SEGMENT, ebufp);
	PIN_FLIST_FLD_COPY(acc_info, PIN_FLD_ACTG_TYPE, acc_infop, PIN_FLD_ACTG_TYPE, ebufp);
	bal_info = PIN_FLIST_ELEM_GET(acc_info, PIN_FLD_BAL_INFO, 0, 1, ebufp);
	bal_infop = PIN_FLIST_ELEM_ADD(acc_infop, PIN_FLD_BAL_INFO, 0, ebufp);
	PIN_FLIST_FLD_COPY(acc_info, PIN_FLD_CURRENCY, acc_infop, PIN_FLD_CURRENCY, ebufp);
	PIN_FLIST_FLD_COPY(acc_info, PIN_FLD_BUSINESS_TYPE, acc_infop, PIN_FLD_BUSINESS_TYPE, ebufp);
	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_END_T, in_flistp, PIN_FLD_END_T, ebufp);
	billinfo = PIN_FLIST_ELEM_GET(i_flistp, PIN_FLD_BILLINFO, 0, 1, ebufp);
	billinfop = PIN_FLIST_ELEM_ADD(in_flistp, PIN_FLD_BILLINFO, 0, ebufp);
	PIN_FLIST_FLD_COPY(billinfo, PIN_FLD_POID, billinfop, PIN_FLD_POID, ebufp);
	PIN_FLIST_FLD_COPY(billinfo, PIN_FLD_ACTG_FUTURE_DOM, billinfop, PIN_FLD_ACTG_FUTURE_DOM, ebufp);
	PIN_FLIST_FLD_COPY(billinfo, PIN_FLD_BILLINFO_ID, billinfop, PIN_FLD_BILLINFO_ID, ebufp);
	PIN_FLIST_FLD_COPY(billinfo, PIN_FLD_BILL_WHEN, billinfop, PIN_FLD_BILL_WHEN, ebufp);
	PIN_ERR_LOG_FLIST(3, "input flist3: ", in_flistp);
	
	payinfo_flistp = PIN_FLIST_ELEM_GET(billinfo, PIN_FLD_PAYINFO, 0, 1, ebufp);
        pay_info_flistp = PIN_FLIST_ELEM_ADD(billinfop, PIN_FLD_PAYINFO, 0, ebufp);
        if(pay_pd_typ && strcmp(pay_pd_typ, "/payinfo/invoice") == 0)
        {
                pay_type = 10001;
                PIN_FLIST_FLD_SET(billinfop, PIN_FLD_PAY_TYPE, &pay_type, ebufp);
        }
        else if(pay_pd_typ && strcmp(pay_pd_typ, "/payinfo/cc") == 0)
        {
                pay_type = 10003;
                PIN_FLIST_FLD_SET(billinfop, PIN_FLD_PAY_TYPE, &pay_type, ebufp);
        }
        else
        {
                pay_type = 10005;
                PIN_FLIST_FLD_SET(billinfop, PIN_FLD_PAY_TYPE, &pay_type, ebufp);
        } 	
	bal_info = PIN_FLIST_ELEM_ADD(billinfop, PIN_FLD_BAL_INFO, 0, ebufp); 
	PIN_ERR_LOG_FLIST(3, "Register billing account input flist: ", in_flistp);             
	PCM_OP(ctxp, PCM_OP_CUST_COMMIT_CUSTOMER, 0, in_flistp, &ret_flistp, ebufp);
	if(PIN_ERRBUF_IS_ERR(ebufp))
	{
        	PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "fm_cust_commit_customer_reg error after excecution", ebufp);
       	 	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_cust_commit_customer_reg input flist", ret_flistp);
        	PIN_ERRBUF_RESET(ebufp);
		return;
}

	PIN_ERR_LOG_FLIST(3, "Register billing account input flist: ", ret_flistp);
	*r_flistp = PIN_FLIST_COPY(ret_flistp, ebufp);
return;
}
