#ifndef lint
static  char Sccs_Id[] = "@(#)%Portal Version: fm_rbs_cust_acct_update.c :BillingVelocityInt:4:2006-Sep-05 04:28:17 %";
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
#include "custom_flds/custom_flds.h"
#include "custom_wrapper_ops.h"

/*******************************************************************
 * Routines contained herein.
 *******************************************************************/
EXPORT_OP void
op_rbs_cust_acct_update(
        cm_nap_connection_t     *connp,
        int32                   opcode,
        int32                   flags,
        pin_flist_t             *i_flistp,
        pin_flist_t             **r_flistpp,
        pin_errbuf_t            *ebufp);
		

static void
fm_rbs_cust_acct_update(
         pcm_context_t           *ctxp,
         pin_flist_t             *i_flistp,
         pin_flist_t             **r_flistpp,
         pin_errbuf_t            *ebufp);

void
op_rbs_cust_acct_update(
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
        if (opcode != PCM_OP_RBS_CUST_UPDATE_CUSTOMER) {
                pin_set_err(ebufp, PIN_ERRLOC_FM,
                        PIN_ERRCLASS_SYSTEM_DETERMINATE,
                        PIN_ERR_BAD_OPCODE, 0, 0, opcode);
						
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                        "bad opcode in op_rbs_cust_acct_update", ebufp);
                return;
        }

        /***********************************************************
         * Debug: What did we get?
         ***********************************************************/
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                "op_rbs_cust_acct_update input flist", i_flistp);

	/*******************************************************************
         * Call the default implementation
         *******************************************************************/
		 
		fm_rbs_cust_acct_update(ctxp ,i_flistp ,&r_flistp ,ebufp);
		
		
/***********************************************************
	** Results.
 ************************************************************/

    if (PIN_ERRBUF_IS_ERR(ebufp))
    {
        PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"op_mso_cust_get_accinfo error", ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "op_rbs_cust_acct_update input flist", i_flistp);
                PIN_ERRBUF_RESET(ebufp);
        *r_flistpp = PIN_FLIST_COPY(i_flistp, ebufp);
    }
    else
    {
   	 *r_flistpp = PIN_FLIST_COPY(r_flistp, ebufp);
         PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,  "op_rbs_cust_acct_update", *r_flistpp);
 
    }
	
	PIN_FLIST_DESTROY_EX(&r_flistp, NULL);	
	return;
}

static void
fm_rbs_cust_acct_update(
         pcm_context_t           *ctxp,
         pin_flist_t             *i_flistp,
         pin_flist_t             **r_flistp,
         pin_errbuf_t            *ebufp)
{ 
        pin_flist_t             *ret_flistp = NULL;
	pin_flist_t		*in_flistp = NULL;
	pin_flist_t		*acctinfo_get_flistp = NULL;
	pin_flist_t             *acctinfo_set_flistp = NULL;
	pin_flist_t		*nameinfo_get_flistp = NULL;
	pin_flist_t             *nameinfo_set_flistp = NULL;
	pin_flist_t		*phones_get_flistp = NULL;
	pin_flist_t		*phones_set_flistp = NULL;
	pin_flist_t		*billinfo_set_flistp = NULL;
	pin_flist_t		*billinfo_get_flistp = NULL;
	pin_flist_t		*payinfo_get_flistp = NULL;
	pin_flist_t		*payinfo_set_flistp = NULL;
	pin_flist_t		*inv_get_info = NULL;
	pin_flist_t		*inv_set_info = NULL;
	pin_flist_t		*cc_get_info = NULL;
	pin_flist_t		*cc_set_info = NULL ;
	pin_flist_t		*dd_get_info = NULL;
	pin_flist_t		*dd_set_info = NULL;
	pin_flist_t		*prof_get_flistp = NULL;
	pin_flist_t		*prof_set_flistp = NULL;
	pin_flist_t		*prof_inher_get_flistp = NULL;
	pin_flist_t		*prof_inher_set_flistp = NULL;
	pin_flist_t		*cust_care_get_flistp = NULL;
	pin_flist_t		*cust_care_set_flistp = NULL;
	pin_flist_t             *prof_billing_get_flistp = NULL;
        pin_flist_t             *prof_billing_set_flistp = NULL;
	pin_flist_t             *inhrt_info_get_flistp = NULL;
	pin_flist_t             *inhrt_info_set_flistp = NULL;
        poid_t                  *acc_pd = NULL;
	poid_t			*prof_obj = NULL;
        poid_t                  *v_payinfo_pd = NULL;
	char			*v_payinfo_type = NULL;
        char                    type[128];
	char			*acc_no = NULL;
	char			*inv_typ = NULL;
	char			msg[100];
	char			*prof_type = NULL;
	int32			flags1 = 1;
	int32			pay_type = 0;
	int32			elem_id = 0;
	pin_cookie_t		cookie = NULL;
	pin_rec_id_t		cnt = 0;
	int32			*v_zip = 0;
	

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "start fm_rbs_cust_acct_update input_flist", i_flistp);

	in_flistp = PIN_FLIST_CREATE(ebufp);
	acc_pd = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp);

	PIN_FLIST_FLD_SET(in_flistp, PIN_FLD_POID, acc_pd, ebufp);
	PIN_FLIST_FLD_SET(in_flistp, PIN_FLD_ACCOUNT_OBJ, acc_pd, ebufp);
	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_PROGRAM_NAME, in_flistp, PIN_FLD_PROGRAM_NAME, ebufp);

	if (PIN_FLIST_ELEM_GET(i_flistp, PIN_FLD_ACCTINFO, 0, 1, ebufp))
	{
		acctinfo_get_flistp = PIN_FLIST_ELEM_GET(i_flistp, PIN_FLD_ACCTINFO, 0, 1, ebufp);
		acc_no = (char *)PIN_FLIST_FLD_GET(acctinfo_get_flistp, PIN_FLD_ACCOUNT_NO, 0, ebufp);
		PIN_FLIST_ELEM_COPY(i_flistp, PIN_FLD_ACCTINFO, 0, in_flistp, PIN_FLD_ACCTINFO, 0, ebufp);
		PIN_ERR_LOG_FLIST(3, "after accinfo in_flistp", in_flistp);
	}

	if (PIN_FLIST_ELEM_GET(i_flistp, PIN_FLD_NAMEINFO, 1, 1, ebufp))
	{
		nameinfo_get_flistp = PIN_FLIST_ELEM_GET(i_flistp, PIN_FLD_NAMEINFO, 1, 1, ebufp);
		PIN_ERR_LOG_FLIST(3, "nameinfo_get_flistp", nameinfo_get_flistp);
		PIN_FLIST_ELEM_COPY(i_flistp, PIN_FLD_NAMEINFO, 1, in_flistp, PIN_FLD_NAMEINFO, 1, ebufp);

		PIN_ERR_LOG_FLIST(3, "after copying nameinfo flistp in_flistp: ", in_flistp);
	}

	if (PIN_FLIST_ELEM_GET(i_flistp, PIN_FLD_BILLINFO, 0, 1, ebufp))
	{
		billinfo_get_flistp = PIN_FLIST_ELEM_GET(i_flistp, PIN_FLD_BILLINFO, 0, 1, ebufp);
		PIN_ERR_LOG_FLIST(3, "billinfo_get_flistp", billinfo_get_flistp)
		PIN_FLIST_ELEM_COPY(i_flistp, PIN_FLD_BILLINFO, 0, in_flistp, PIN_FLD_BILLINFO, 0, ebufp);
	
		PIN_ERR_LOG_FLIST(3, "after copying billinfo flist in_flistp: ", in_flistp);
	}

	if (PIN_FLIST_ELEM_GET(i_flistp, PIN_FLD_PAYINFO, 0, 1, ebufp))
	{
		payinfo_get_flistp = PIN_FLIST_ELEM_GET(i_flistp, PIN_FLD_PAYINFO, 0, 1, ebufp);
		v_payinfo_pd = PIN_FLIST_FLD_GET(payinfo_get_flistp, PIN_FLD_POID, 0, ebufp);
		v_payinfo_type = (char *)PIN_POID_GET_TYPE(v_payinfo_pd);
	
		PIN_ERR_LOG_FLIST(3, "payinfo_get_flistp", payinfo_get_flistp);
		PIN_ERR_LOG_POID(3, "payinfo poid v_payinfo_pd", v_payinfo_pd);
		sprintf(msg, "v_payinfo_type %s", v_payinfo_type);
		PIN_ERR_LOG_MSG(3, msg);
	
		payinfo_set_flistp = PIN_FLIST_ELEM_ADD(in_flistp, PIN_FLD_PAYINFO, 1, ebufp);
		PIN_FLIST_FLD_SET(payinfo_set_flistp, PIN_FLD_POID, v_payinfo_pd, ebufp);
		PIN_FLIST_FLD_SET(payinfo_set_flistp, PIN_FLD_FLAGS, &flags1, ebufp);
 		PIN_ERR_LOG_FLIST(3, "payinfo_set_flistp", payinfo_set_flistp);	
        	if ( v_payinfo_type && strcmp(v_payinfo_type, "/payinfo/invoice") == 0 )
        	{
			PIN_ERR_LOG_MSG(3, "inv type");
			pay_type = 10001;
			PIN_FLIST_FLD_SET(payinfo_set_flistp, PIN_FLD_PAY_TYPE, &pay_type, ebufp);
			inv_typ = "Cash_Invoice-";
			memset(type,'\0',sizeof(type));
			strcpy(type, inv_typ);
			strcat(type, acc_no);
			PIN_FLIST_FLD_SET(payinfo_set_flistp, PIN_FLD_NAME, type, ebufp);
		}
        	else if ( v_payinfo_type && strcmp(v_payinfo_type, "/payinfo/cc") == 0 )
        	{
			pay_type = 10003;
			PIN_FLIST_FLD_SET(payinfo_set_flistp, PIN_FLD_PAY_TYPE, &pay_type, ebufp);
			inv_typ = "CC_Invoice-";
			memset(type,'\0',sizeof(type));
			strcpy(type, inv_typ);
                	strcat(type, acc_no);
			PIN_FLIST_FLD_SET(payinfo_set_flistp, PIN_FLD_NAME, &type, ebufp);
		}
		else
		{ 
			pay_type = 10005;
			PIN_FLIST_FLD_SET(payinfo_set_flistp, PIN_FLD_PAY_TYPE, &pay_type, ebufp);
			inv_typ = "DD_Invoice-";
			memset(type,'\0',sizeof(type));
			strcpy(type, inv_typ);
                	strcat(type, acc_no);
			PIN_FLIST_FLD_SET(payinfo_set_flistp, PIN_FLD_NAME, type, ebufp);
		}
	
		inhrt_info_get_flistp = PIN_FLIST_SUBSTR_GET(payinfo_get_flistp, PIN_FLD_INHERITED_INFO, 1, ebufp);
		inhrt_info_set_flistp = PIN_FLIST_SUBSTR_ADD(payinfo_set_flistp, PIN_FLD_INHERITED_INFO, ebufp);
	
 		if ( v_payinfo_type && strcmp(v_payinfo_type, "/payinfo/invoice") == 0 )
 		{
			inv_get_info = PIN_FLIST_ELEM_GET(inhrt_info_get_flistp, PIN_FLD_INV_INFO, 0, 1, ebufp);
			PIN_ERR_LOG_FLIST(3, "inv_get_info flist: ", inv_get_info);
			PIN_FLIST_ELEM_COPY(inhrt_info_get_flistp, PIN_FLD_INV_INFO, 0, inhrt_info_set_flistp, PIN_FLD_INV_INFO, 0, ebufp);
		
			PIN_ERR_LOG_FLIST(3, "after copying inv_get_info flist inhrt_info_set_flistp: ", inhrt_info_set_flistp);
		}
  		else if ( v_payinfo_type && strcmp(v_payinfo_type, "/payinfo/cc") == 0 )
        	{	
			cc_get_info = PIN_FLIST_ELEM_GET(inhrt_info_get_flistp, PIN_FLD_CC_INFO, 0, 1, ebufp);	
			PIN_ERR_LOG_FLIST(3, "cc_get_info flist: ", cc_get_info);
                	PIN_FLIST_ELEM_COPY(inhrt_info_get_flistp, PIN_FLD_CC_INFO, 0, inhrt_info_set_flistp, PIN_FLD_CC_INFO, 0, ebufp);
		
			PIN_ERR_LOG_FLIST(3, "after copying cc_get_info flist inhrt_info_set_flistp: ", inhrt_info_set_flistp);
		}
		else 
		{
			dd_get_info = PIN_FLIST_ELEM_GET(inhrt_info_get_flistp, PIN_FLD_DD_INFO, 0, 1, ebufp);
			PIN_ERR_LOG_FLIST(3, "dd_get_info flist: ", dd_get_info);
                	PIN_FLIST_ELEM_COPY(inhrt_info_get_flistp, PIN_FLD_DD_INFO, 0, inhrt_info_set_flistp, PIN_FLD_DD_INFO, 0, ebufp);
		
			PIN_ERR_LOG_FLIST(3, "after copying dd_get_info flist inhrt_info_set_flistp: ", inhrt_info_set_flistp);
		}
		PIN_ERR_LOG_FLIST(3, "payinfo_set_flistp", payinfo_set_flistp);
	}
	PIN_ERR_LOG_FLIST(3, "after payinfo in_flistp", in_flistp);

	if (PIN_FLIST_ELEM_COUNT(i_flistp, PIN_FLD_PROFILES, ebufp) > 0 )
	{
		cnt = 0;	
		while ( (prof_get_flistp = PIN_FLIST_ELEM_GET_NEXT(i_flistp, PIN_FLD_PROFILES, &elem_id, 1, &cookie, ebufp)) != (pin_flist_t *)NULL )
		{
			PIN_ERR_LOG_FLIST(3, "prof_get_flistp: ", prof_get_flistp);
			PIN_FLIST_ELEM_COPY(i_flistp, PIN_FLD_PROFILES, cnt, in_flistp, PIN_FLD_PROFILES, cnt, ebufp);
			sprintf(msg, "count: %d", cnt);
			PIN_ERR_LOG_MSG(3, msg);
			cnt++;
		}
	}

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "end fm_rbs_cust_acct_update input_flist", in_flistp);
	PCM_OP(ctxp, PCM_OP_CUST_UPDATE_CUSTOMER, 0, in_flistp, &ret_flistp, ebufp);

	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"fm_rbs_cust_acct_update input  error", ebufp);
        	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_rbs_cust_acct_update input flist", in_flistp);
        	PIN_ERRBUF_RESET(ebufp);
		return;
	}
	else
	{
		*r_flistp = PIN_FLIST_COPY(ret_flistp, ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,  "fm_rbs_cust_acct_update return flist", *r_flistp);
	}
	return;
	PIN_FLIST_DESTROY_EX(&ret_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&in_flistp, NULL);
}
