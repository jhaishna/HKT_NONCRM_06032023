#ifndef lint
static const char Sccs_id[] = "@(#)%Portal Version: fm_pymt_rej_upload.c:BillingVelocityInt:3:2006-Sep-05 21:55:03 %";
#endif

#include <stdio.h>
#include <string.h>
#include <time.h>
#include "pcm.h"
#include "ops/cust.h"
#include "ops/act.h"
#include "ops/bill.h"
#include "ops/pymt.h"
#include "pin_bill.h"
#include "cm_fm.h"
#include "pin_flds.h"
#include "pin_errs.h"
#include "pinlog.h"
#include "fm_utils.h"
#include "pin_pymt.h"
#include "pin_flds.h"
#include "pin_currency.h"
#include "ops/bal.h"
#include "pin_cust.h"
#include "fm_rbs_pymt.h"

#define SUCCESS 0
#define FAILURE 1
#define READWRITE 1
#define LOCAL_TRANS_OPEN_SUCCESS 0
#define PAY_TYPE_CASH 10011
#define PAY_TYPE_CHECK 10012
#define STATUS_ACTIVE 10100
#define STATUS_INACTIVE 10102

#define FILE_LOGNAME "fm_pymt_rej_upload.c(1.5)"
/***********************
 *DEFINE FUNCTIONS
************************/


EXPORT_OP void
op_pymt_rej_upload(
    cm_nap_connection_t *connp,
    int32               opcode,
    int32               flags,
    pin_flist_t         *i_flistp,
    pin_flist_t         **r_flistpp,
    pin_errbuf_t        *ebufp);

static void
fm_pymt_rej_upload(
        pcm_context_t           *ctxp,
        pin_flist_t             *i_flistp,
        pin_flist_t             **r_flistp,
        pin_errbuf_t            *ebufp);

static void
fm_pymt_collect(
        pcm_context_t           *ctxp,
        pin_flist_t             *i_flistp,
        pin_flist_t             **pymt_oflistp,
        pin_errbuf_t            *ebufp);

static void
fm_get_billinfo_details(
        pcm_context_t           *ctxp,
        pin_flist_t             *i_flistp,
        pin_flist_t             **rs_flistp,
        pin_errbuf_t            *ebufp);

static void
fm_rbs_get_pymt_reason(
	pcm_context_t           *ctxp,
        char	            	*reason,
        pin_flist_t             **r_flistp,
        pin_errbuf_t            *ebufp);

PIN_IMPORT int32 fm_rbs_trans_open(
        pcm_context_t           *ctxp,
        poid_t                  *pdp,
        int32                   flag,
        pin_errbuf_t            *ebufp);

PIN_IMPORT void fm_rbs_trans_commit(
        pcm_context_t           *ctxp,
        poid_t                  *pdp,
        pin_errbuf_t            *ebufp);

PIN_IMPORT void fm_rbs_trans_abort(
        pcm_context_t           *ctxp,
        poid_t                  *pdp,
        pin_errbuf_t            *ebufp);

/**************************
*FUNCTION IMPLIMENTATIONS
**************************/
void
op_pymt_rej_upload(
        cm_nap_connection_t *connp,
        int32               opcode,
        int32               flags,
        pin_flist_t         *i_flistp,
        pin_flist_t         **r_flistpp,
        pin_errbuf_t        *ebufp)
{
        pcm_context_t       	*ctxp = connp->dm_ctx;
        pin_flist_t         	*r_flistp= NULL;
	pin_flist_t		*charges_flistp = NULL;
	pin_flist_t		*err_flistp = NULL;
	pin_flist_t             *result_flistp = NULL;
	pin_decimal_t		*zero_p = pbo_decimal_from_str("0.0", ebufp);
	pin_decimal_t		*pymt_amt = pbo_decimal_from_str("0.0", ebufp);
	int32			status = 1;
	int                     local = 1;
	int64			db = -1;
	poid_t                  *a_pdp = NULL;
	poid_t 			*d_pdp = NULL;

        *r_flistpp = NULL;
        if (PIN_ERRBUF_IS_ERR(ebufp)) 
	{
            return;
        }
        PIN_ERRBUF_CLEAR(ebufp);

	/***********************
	* Insanity Check
	* ***********************/
    	if (opcode != RBS_PYMT_REJ_UPLOAD) 
	{
        	pin_set_err(ebufp, PIN_ERRLOC_FM, PIN_ERRCLASS_SYSTEM_DETERMINATE,
        	    	PIN_ERR_BAD_OPCODE, 0, 0, opcode);
        	PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "op_pymt_rej_upload error", ebufp);
        	return;
    	}

	/*************************
	 * Debug: Input flist
	**************************/
    	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "op_pymt_rej_upload input", i_flistp);

        a_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp);
	db = PIN_POID_GET_DB(a_pdp);
        local = fm_rbs_trans_open(ctxp, a_pdp, 3 , ebufp);
        if(PIN_ERRBUF_IS_ERR(ebufp))
        {
                PIN_ERRBUF_CLEAR(ebufp);
                result_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, result_flistp, PIN_FLD_POID, ebufp);
                status = FAILURE;
                PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_STATUS_FLAGS, &status, ebufp);
		PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_ERROR_CODE,"53080" , ebufp);
                PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_ERROR_DESCR, "Error in Opening Transaction", ebufp);
		
                PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "PAYMENT Fail output flist", result_flistp);
                *r_flistpp = PIN_FLIST_COPY(result_flistp, ebufp);
                PIN_FLIST_DESTROY_EX(&result_flistp, NULL);
                goto CLEANUP;
        }
        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Transaction Opened");

	charges_flistp = PIN_FLIST_ELEM_GET(i_flistp, PIN_FLD_CHARGES, PIN_ELEMID_ANY, 0, ebufp);
	if (charges_flistp && charges_flistp != NULL )
        {
                pymt_amt = (pin_decimal_t *) PIN_FLIST_FLD_GET(charges_flistp, PIN_FLD_AMOUNT, 0, ebufp);
                zero_p = pbo_decimal_from_str("0.0", ebufp);
		if(pymt_amt && pymt_amt != NULL && zero_p && (pbo_decimal_compare(pymt_amt, zero_p, ebufp) <= 0))
                {
                	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, "Validation :  Payment less than or equal to zero not allowed");
                   	PIN_ERRBUF_RESET(ebufp);
			err_flistp = PIN_FLIST_CREATE(ebufp);
                   	status = FAILURE;
                   	PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_POID, PIN_POID_CREATE(db, "/account", 1, ebufp), ebufp);
			PIN_ERR_LOG_FLIST(3, "test1 err_flistp:", err_flistp);
                   	PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_CODE, "53005", ebufp);
                   	PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_DESCR, "Payment less than or equal to zero is Not Allowed", ebufp);
                  	PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_STATUS_FLAGS, &status, ebufp);
                   	if (zero_p && !pbo_decimal_is_null(zero_p, ebufp))
                       		pbo_decimal_destroy(&zero_p);
			*r_flistpp = PIN_FLIST_COPY(err_flistp, ebufp);
			PIN_ERR_LOG_FLIST(3, "error flistp:", *r_flistpp);
                  	goto CLEANUP;
                }
	}
	/**********************************
	 * Call the default implementation
	***********************************/

    	fm_pymt_rej_upload(ctxp, i_flistp, &r_flistp, ebufp);

	/*********************
	 * Results.
	**********************/

	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		status = FAILURE;
        	PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "op_pymt_rej_upload error", ebufp);
		goto CLEANUP;
    	}
    	else
    	{
		status = SUCCESS;
       		*r_flistpp = PIN_FLIST_COPY(r_flistp, ebufp);
        	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "op_pymt_rej_upload: output flist::", *r_flistpp);
		goto CLEANUP;
    	}
CLEANUP:
	if (status == FAILURE)
        {
                if(local == LOCAL_TRANS_OPEN_SUCCESS )
                {
                        fm_rbs_trans_abort(ctxp, a_pdp, ebufp);
                        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Transaction Aborted");
                }
        }
        else
        {
                if(local == LOCAL_TRANS_OPEN_SUCCESS )
                {
                        fm_rbs_trans_commit(ctxp, a_pdp, ebufp);
                        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Transaction Committed");
                }
        }
	return;
}

/******************************************
 *Function implementation for 
 *Payment reject upload
 ******************************************/
void
fm_pymt_rej_upload(
        pcm_context_t           *ctxp,
        pin_flist_t             *i_flistp,
        pin_flist_t             **r_flistp,
        pin_errbuf_t            *ebufp)
{
        pin_flist_t             *pymt_iflistp = NULL;
        pin_flist_t             *evt_oflistp = NULL;
        pin_flist_t             *charges_flistp = NULL;
        pin_flist_t             *cr_flistp = NULL;
        pin_flist_t             *cr_oflistp = NULL;
        pin_flist_t             *bal_flistp = NULL;
        pin_flist_t             *pymt_flistp = NULL;
        pin_flist_t             *pymts_flistp = NULL;
        pin_flist_t             *payment_flistp = NULL;
        pin_flist_t             *chrg_flistp = NULL;
	pin_flist_t		*reason_flistp = NULL;
	pin_flist_t		*acc_flistp = NULL;
        pin_flist_t             *res_flistp = NULL;
        poid_t                  *pdp = NULL;
        poid_t                  *pymt_rej_pdp = NULL;
	poid_t			*acc_obj = NULL;
        poid_t                  *item_obj = NULL;
        poid_t                  *event_obj = NULL;
        int32                   var = 0;
	int32			status = 0;
	int32			*reason_id = NULL;
	int32			*channel_id = NULL;
        int32                   *res_val = NULL;
        int64                   db = -1;
	int32			reason = 0;
	char			*descr = NULL;
	char			*acc_no = NULL;
	char			*channel = NULL;
        char                    msg[100];
        pin_decimal_t           *amount = NULL;
        time_t                  eff_t = 0;

        if (PIN_ERRBUF_IS_ERR(ebufp))
                return;
        PIN_ERRBUF_CLEAR(ebufp);

        PIN_ERR_LOG_FLIST(3, "start fm_pymt_rej_upload input flist: ", i_flistp);
        chrg_flistp = PIN_FLIST_ELEM_GET(i_flistp, PIN_FLD_CHARGES, 0, 1, ebufp);
        amount = PIN_FLIST_FLD_GET(chrg_flistp, PIN_FLD_AMOUNT, 0, ebufp);
	channel = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_CHANNEL, 0, ebufp);
	if (PIN_FLIST_FLD_GET(chrg_flistp, PIN_FLD_ACCOUNT_NO, 1, ebufp))
	{
		acc_no = PIN_FLIST_FLD_GET(chrg_flistp, PIN_FLD_ACCOUNT_NO, 0, ebufp);
	}
	else
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error! Account_no Missing", ebufp);
                *r_flistp = PIN_FLIST_CREATE(ebufp);
                PIN_FLIST_FLD_SET(*r_flistp, PIN_FLD_POID, pdp, ebufp);
                status = 1;
                PIN_FLIST_FLD_SET(*r_flistp, PIN_FLD_STATUS_FLAGS, &status, ebufp);
                PIN_FLIST_FLD_SET(*r_flistp, PIN_FLD_ERROR_CODE, "53002", ebufp);
                PIN_FLIST_FLD_COPY(charges_flistp, PIN_FLD_DESCR, *r_flistp, PIN_FLD_ERROR_DESCR, ebufp);
                goto CLEANUP;
	}
	
	//Calling function to get the payment channel id from DB
        fm_rbs_get_pymt_reason(ctxp, channel, &reason_flistp, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp))
        {
               PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "fm_rbs_get_pymt_reason calling error: ", ebufp);
               goto CLEANUP;
        }
        PIN_ERR_LOG_FLIST(3, "fm_rbs_get_pymt_reason output flist: ", reason_flistp);
	if (PIN_FLIST_FLD_GET(reason_flistp, PIN_FLD_STATUS_FLAGS, 1, ebufp))
	{
		*r_flistp = PIN_FLIST_COPY(reason_flistp, ebufp);
                PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_rbs_get_pymt_reason error flist::", *r_flistp);
                goto CLEANUP;
	}
        channel_id = PIN_FLIST_FLD_GET(reason_flistp, PIN_FLD_STRING_ID, 0, ebufp);
        PIN_FLIST_FLD_SET(chrg_flistp, PIN_FLD_CHANNEL_ID, channel_id, ebufp);

        //call fm_pymt_collect function to pass payment for suspense account 
        fm_pymt_collect(ctxp, i_flistp, &pymt_flistp, ebufp);
        if (PIN_ERRBUF_IS_ERR(ebufp))
        {
               PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "payment_rejected fm_pymt_collect error: ", ebufp);
               goto CLEANUP;
        }
        PIN_ERR_LOG_FLIST(3, "fm_pymt_collect output flist: ", pymt_flistp);

        eff_t = (time_t)pin_virtual_time((time_t *)NULL);
        pdp = PIN_FLIST_FLD_GET(pymt_flistp, PIN_FLD_POID, 0, ebufp);
        db = PIN_POID_GET_DB(pdp);

        charges_flistp = PIN_FLIST_ELEM_GET(pymt_flistp, PIN_FLD_RESULTS, 0, 1, ebufp);
        if (PIN_FLIST_ELEM_COUNT(charges_flistp, PIN_FLD_BAL_IMPACTS, ebufp) > 0)
        {
                bal_flistp = PIN_FLIST_ELEM_GET(charges_flistp, PIN_FLD_BAL_IMPACTS, 0, 1, ebufp);
        }
        res_val = PIN_FLIST_FLD_GET(charges_flistp, PIN_FLD_RESULT, 0, ebufp);
        sprintf(msg, "res_val: %d", *res_val);
        PIN_ERR_LOG_MSG(3, msg);

	//If payment posting failed for suspense account
        if (*res_val == 0)
        {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error! in payment collect", ebufp);
		PIN_ERRBUF_RESET(ebufp);
		*r_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(*r_flistp, PIN_FLD_POID, pdp, ebufp);
                status = 1;
                PIN_FLIST_FLD_SET(*r_flistp, PIN_FLD_STATUS_FLAGS, &status, ebufp);
		PIN_FLIST_FLD_SET(*r_flistp, PIN_FLD_ERROR_CODE, "53002", ebufp);
                PIN_FLIST_FLD_COPY(charges_flistp, PIN_FLD_DESCR, *r_flistp, PIN_FLD_ERROR_DESCR, ebufp);
                goto CLEANUP;
        }
        else
        {
                PIN_ERR_LOG_FLIST(3, "PYMT_COLLECT: output flist:", pymt_flistp);

                res_flistp = PIN_FLIST_ELEM_GET(pymt_flistp, PIN_FLD_RESULTS, 0, 1, ebufp);
                event_obj = PIN_FLIST_FLD_GET(res_flistp, PIN_FLD_POID, 0, ebufp);
                bal_flistp = PIN_FLIST_ELEM_GET(res_flistp, PIN_FLD_BAL_IMPACTS, 0, 1, ebufp);
		acc_obj = PIN_FLIST_FLD_GET(bal_flistp, PIN_FLD_ACCOUNT_OBJ, 0, ebufp);
                item_obj = PIN_FLIST_FLD_GET(bal_flistp, PIN_FLD_ITEM_OBJ, 0, ebufp);

                cr_flistp = PIN_FLIST_CREATE(ebufp);
		pymt_rej_pdp = PIN_POID_CREATE(db, "/rbs_rejected_payment", -1, ebufp);
                PIN_FLIST_FLD_SET(cr_flistp, PIN_FLD_POID, pymt_rej_pdp, ebufp);
                PIN_FLIST_FLD_SET(cr_flistp, PIN_FLD_ACCOUNT_OBJ, acc_obj, ebufp);
                PIN_FLIST_FLD_SET(cr_flistp, PIN_FLD_EFFECTIVE_END_T, &eff_t, ebufp);
                PIN_FLIST_FLD_SET(cr_flistp, PIN_FLD_AMOUNT, amount, ebufp);
                PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_CHANNEL, cr_flistp, PIN_FLD_CHANNEL, ebufp);
		if (PIN_FLIST_FLD_GET(chrg_flistp, PIN_FLD_ACCOUNT_NO, 1, ebufp))
		{
                	PIN_FLIST_FLD_COPY(chrg_flistp, PIN_FLD_ACCOUNT_NO, cr_flistp, PIN_FLD_ACCOUNT_NO, ebufp);
		}
                var = 1;
                PIN_FLIST_FLD_SET(cr_flistp, PIN_FLD_ACCOUNT_CNT, &var, ebufp);
		status = 1;
                PIN_FLIST_FLD_SET(cr_flistp, PIN_FLD_STATUS_FLAGS, &status, ebufp);
                PIN_FLIST_FLD_SET(cr_flistp, PIN_FLD_ITEM_OBJ, item_obj, ebufp);
                PIN_FLIST_FLD_SET(cr_flistp, PIN_FLD_EVENT_OBJ, event_obj, ebufp);

                if (PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_DEBIT_NUM, 1, ebufp) != NULL)
                {
                        PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_DEBIT_NUM, cr_flistp, PIN_FLD_DEBIT_NUM, ebufp);
                }
                if (PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_REASON_CODE, 1, ebufp) != NULL)
                {
                        descr = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_REASON_CODE, 0, ebufp);

			//Calling function to get suspense reason id
                        fm_rbs_get_pymt_reason(ctxp, descr, &reason_flistp, ebufp);
                        PIN_ERR_LOG_FLIST(3, "fm_rbs_get_pymt_reason output flist: ", reason_flistp);
			if (PIN_FLIST_FLD_GET(reason_flistp, PIN_FLD_STRING_ID, 1, ebufp) != NULL)
			{
				reason_id = PIN_FLIST_FLD_GET(reason_flistp, PIN_FLD_STRING_ID, 0, ebufp);
				reason = *reason_id;
			}
			else
			{
				reason = 2999;
			}
                        PIN_FLIST_FLD_SET(cr_flistp, PIN_FLD_REASON, &reason, ebufp);
                }

		//Create object in /rbs_rejected_payment
                PIN_ERR_LOG_FLIST(3, "payment_rejected create_obj input flist: ", cr_flistp);
                PCM_OP(ctxp, PCM_OP_CREATE_OBJ, 0, cr_flistp, &cr_oflistp, ebufp);
                if (PIN_ERRBUF_IS_ERR(ebufp))
                {
                        PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "payment_rejected create_obj opcode error: ", ebufp);
                        goto CLEANUP;
                }
		PIN_FLIST_FLD_SET(cr_oflistp, PIN_FLD_ACCOUNT_OBJ, acc_obj, ebufp);
                status = 0;
                PIN_FLIST_FLD_SET(cr_oflistp, PIN_FLD_STATUS_FLAGS, &status, ebufp);
                PIN_FLIST_FLD_SET(cr_oflistp, PIN_FLD_DESCR, "PYMT_COLLECT success", ebufp);
        }
	*r_flistp = PIN_FLIST_COPY(cr_oflistp, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_pymt_rej_upload output flist::", *r_flistp);
CLEANUP:
	PIN_FLIST_DESTROY_EX(&cr_oflistp, NULL);	
	PIN_FLIST_DESTROY_EX(&reason_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&pymt_flistp, NULL);
return;
}

/******************************************
 *Function to collect the payments
 ******************************************/
void
fm_pymt_collect(
        pcm_context_t           *ctxp,
        pin_flist_t             *i_flistp,
        pin_flist_t             **pymt_oflistp,
        pin_errbuf_t            *ebufp)
{

        pin_flist_t             *ch_flistp = NULL;
        pin_flist_t             *pymt_flistp = NULL;
        pin_flist_t             *err_flistp = NULL;
        pin_flist_t             *inh_flistp = NULL;
        pin_flist_t             *p_flistp = NULL;
        pin_flist_t             *tmp_flistp = NULL;
        pin_flist_t             *pymt_iflistp = NULL;
        poid_t                  *pdp = NULL;
        poid_t                  *bal_grp_obj = NULL;
        poid_t                  *billinfo_pdp = NULL;
        char                    *acc_num = NULL;
        char                    *descr = NULL;
        char                    *channel = NULL;
        char                    *ref_no = NULL;
        int32                   var = 0;
        int32                   status = 0;
        int32                   rec_id = 0;
        int32                   *pay_type = NULL;
        time_t                  *vp = NULL;
        time_t                  *eff_t = NULL;
        pin_decimal_t           *amount = 0;
        pin_cookie_t            cookie = NULL;


        if (PIN_ERRBUF_IS_ERR(ebufp))
                return;
        PIN_ERRBUF_CLEAR(ebufp);

        PIN_ERR_LOG_FLIST(3, "start fm_pymt_collect input flist: ", i_flistp);
        pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp);

        pymt_iflistp = PIN_FLIST_CREATE(ebufp);
        err_flistp = PIN_FLIST_CREATE(ebufp);
        PIN_FLIST_FLD_SET(pymt_iflistp, PIN_FLD_POID, pdp, ebufp);
        PIN_FLIST_FLD_SET(pymt_iflistp, PIN_FLD_PROGRAM_NAME, "PAYMENT COLLECT", ebufp);
        PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_DESCR, pymt_iflistp, PIN_FLD_DESCR, ebufp);
        PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_CHANNEL, pymt_iflistp, PIN_FLD_CHANNEL, ebufp);

        while((ch_flistp = PIN_FLIST_ELEM_GET_NEXT(i_flistp, PIN_FLD_CHARGES, &rec_id, 1, &cookie, ebufp))!=(pin_flist_t*)NULL)
        {
                PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"ch_flistp start", ch_flistp);
                pay_type = (int32 *)PIN_FLIST_FLD_GET(ch_flistp, PIN_FLD_PAY_TYPE, 0, ebufp);

                PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"ch_flistp end", ch_flistp);
                PIN_FLIST_ELEM_SET(pymt_iflistp, (pin_flist_t *)ch_flistp, PIN_FLD_CHARGES, rec_id, ebufp);
        }

        PIN_ERR_LOG_FLIST(3, "PCM_OP_PYMT_COLLECT input flist: ", pymt_iflistp);
        PCM_OP(ctxp, PCM_OP_PYMT_COLLECT, 0, pymt_iflistp, pymt_oflistp, ebufp);
        if (PIN_ERRBUF_IS_ERR(ebufp))
        {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Pymt collect opcode error: ", ebufp);
                return;
        }
        PIN_ERR_LOG_FLIST(3, "PCM_OP_PYMT_COLLECT output flist: ", *pymt_oflistp);
        return;

}

/****************************************
 *Function to get billinfo details
 ****************************************/
void
fm_get_billinfo_details(
        pcm_context_t           *ctxp,
        pin_flist_t             *i_flistp,
        pin_flist_t             **rs_flistp,
        pin_errbuf_t            *ebufp)
{
        pin_flist_t             *s_iflistp = NULL;
        pin_flist_t             *s_oflistp = NULL;
        pin_flist_t             *args_flistp = NULL;
	pin_flist_t		*rslt_flistp = NULL;
        poid_t                  *pdp = NULL;
        poid_t                  *s_pdp = NULL;
	poid_t			*bal_grp_obj = NULL;
        poid_t                  *item_obj = NULL;
        int32                   flags = 256;
        int64                   db = -1;
        char                    *templatep = "select x from /billinfo 1,/account 2 where 1.F1 = 2.F2 and 2.F3 = V3";

	if (PIN_ERRBUF_IS_ERR(ebufp))
                return;
        PIN_ERRBUF_CLEAR(ebufp);

        PIN_ERR_LOG_FLIST(3, "fm_get_billinfo_details input flist: ", i_flistp);

        s_iflistp = PIN_FLIST_CREATE(ebufp);

        pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp);
        db = PIN_POID_GET_DB(pdp);

	rslt_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_SET(rslt_flistp, PIN_FLD_POID, pdp, ebufp);
	PIN_ERR_LOG_FLIST(3, "robj input flist: ", rslt_flistp);
        PCM_OP(ctxp, PCM_OP_READ_OBJ, 0, rslt_flistp, &s_oflistp, ebufp);
        if (PIN_ERRBUF_IS_ERR(ebufp))
        {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "fm_get_billinfo_details search error", ebufp);
                return;
        }
	PIN_ERR_LOG_FLIST(3, "robj input flist: ", s_oflistp);
	bal_grp_obj = PIN_FLIST_FLD_GET(s_oflistp, PIN_FLD_BAL_GRP_OBJ, 0, ebufp);
	
        s_pdp = PIN_POID_CREATE(db, "/search", -1, ebufp);
        PIN_FLIST_FLD_SET(s_iflistp, PIN_FLD_POID, s_pdp, ebufp);
        PIN_FLIST_FLD_SET(s_iflistp, PIN_FLD_FLAGS, &flags, ebufp);
        PIN_FLIST_FLD_SET(s_iflistp, PIN_FLD_TEMPLATE, templatep, ebufp);

        args_flistp = PIN_FLIST_ELEM_ADD(s_iflistp, PIN_FLD_ARGS, 1, ebufp);
        PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_ACCOUNT_OBJ, NULL, ebufp);

        args_flistp=PIN_FLIST_ELEM_ADD(s_iflistp, PIN_FLD_ARGS, 2, ebufp);
        PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_POID, NULL, ebufp);

        args_flistp = PIN_FLIST_ELEM_ADD(s_iflistp, PIN_FLD_ARGS, 3, ebufp);
        if (PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_ACCOUNT_NO, 1, ebufp) != NULL)
        {
                PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_ACCOUNT_NO, args_flistp, PIN_FLD_ACCOUNT_NO, ebufp);
        }
        else if (PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_ACCOUNT_OBJ, 1, ebufp) != NULL)
        {
                PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_ACCOUNT_OBJ, args_flistp, PIN_FLD_POID, ebufp);
        }

        args_flistp = PIN_FLIST_ELEM_ADD(s_iflistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, ebufp);
        /*
        PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_ACCOUNT_OBJ, NULL, ebufp);
        PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_BAL_GRP_OBJ, NULL, ebufp);
        PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_POID, NULL, ebufp);
        */

        PIN_ERR_LOG_FLIST(3, "SEARCH input flist: ", s_iflistp);
        PCM_OP(ctxp, PCM_OP_SEARCH, 0, s_iflistp, &s_oflistp, ebufp);

        if (PIN_ERRBUF_IS_ERR(ebufp))
        {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "fm_get_billinfo_details search error", ebufp);
                return;
        }
        else
        {
		rslt_flistp = PIN_FLIST_ELEM_GET(s_oflistp, PIN_FLD_RESULTS, 0, 1, ebufp);
                *rs_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_COPY(rslt_flistp, PIN_FLD_POID, *rs_flistp, PIN_FLD_POID, ebufp);
		PIN_FLIST_FLD_SET(*rs_flistp, PIN_FLD_BAL_GRP_OBJ, bal_grp_obj, ebufp);
                PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_get_billinfo_details: output flist: ", *rs_flistp);
                return;
        }

}

/****************************************
 *Function to get Reason ID's for
 *Payment suspense or Payment channel
 ****************************************/
void
fm_rbs_get_pymt_reason(
        pcm_context_t           *ctxp,
        char			*reason,
        pin_flist_t             **r_flistp,
        pin_errbuf_t            *ebufp)
{
	pin_flist_t		*in_flistp = NULL;
	pin_flist_t		*ret_flistp = NULL;
	pin_flist_t		*args_flistp = NULL;
	pin_flist_t		*rslt_flistp = NULL;
	pin_flist_t		*err_flistp = NULL;
	poid_t			*pdp = NULL;
	char			*templatep = "select X from /strings where F1 like V1 and (F2 = V2 or F3 = V3) ";
	char			string[255];
	int32			flags = 256;
	int32			reason_id = 0;
	int32			status = 0;
	int64			db = 1;
	
	if (PIN_ERRBUF_IS_ERR(ebufp))
		return;
	PIN_ERRBUF_CLEAR(ebufp);

	strcpy(string, reason);
	strcat(string, "%");
	PIN_ERR_LOG_MSG(3, string);

	in_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_SET(in_flistp, PIN_FLD_POID, PIN_POID_CREATE(db, "/search", -1, ebufp), ebufp);
	PIN_FLIST_FLD_SET(in_flistp, PIN_FLD_FLAGS, &flags, ebufp);
	PIN_FLIST_FLD_SET(in_flistp, PIN_FLD_TEMPLATE, templatep, ebufp);
	
	args_flistp = PIN_FLIST_ELEM_ADD(in_flistp, PIN_FLD_ARGS, 1, ebufp);
	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_STRING, &string, ebufp);

	args_flistp = PIN_FLIST_ELEM_ADD(in_flistp, PIN_FLD_ARGS, 2, ebufp);
        PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_DOMAIN, "Payment Channel", ebufp);

	args_flistp = PIN_FLIST_ELEM_ADD(in_flistp, PIN_FLD_ARGS, 3, ebufp);
        PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_DOMAIN, "Reason codes-Payment Suspense Management", ebufp);	

	args_flistp = PIN_FLIST_ELEM_ADD(in_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, ebufp);
	
	PIN_ERR_LOG_FLIST(3, "fm_rbs_get_pymt_reason search input flist: ", in_flistp);
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, in_flistp, &ret_flistp, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp))
        {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "fm_rbs_get_pymt_reason search error", ebufp);
                return;
        }
	PIN_ERR_LOG_FLIST(3, "fm_rbs_get_pymt_reason search output flist: ", ret_flistp);

	rslt_flistp = PIN_FLIST_ELEM_GET(ret_flistp, PIN_FLD_RESULTS, 0, 1, ebufp);
	if (!rslt_flistp)
	{
		PIN_ERRBUF_RESET(ebufp);
		err_flistp = PIN_FLIST_CREATE(ebufp);
		status = FAILURE;
		PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_POID, PIN_POID_CREATE(db, "/account", 1, ebufp), ebufp);
		PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_CODE, "53002", ebufp);
		PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_DESCR, "Given Payment Channel or Adjustment Reason Not Found", ebufp);
		PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_STATUS_FLAGS, &status, ebufp);

		*r_flistp = PIN_FLIST_COPY(err_flistp, ebufp);
		return;
	}
	*r_flistp = PIN_FLIST_CREATE(ebufp);	
	PIN_FLIST_FLD_COPY(ret_flistp, PIN_FLD_POID, *r_flistp, PIN_FLD_POID, ebufp);
	if (PIN_FLIST_FLD_GET(rslt_flistp, PIN_FLD_STRING_ID, 1, ebufp) != NULL)
	{
		PIN_FLIST_FLD_COPY(rslt_flistp, PIN_FLD_STRING_ID, *r_flistp, PIN_FLD_STRING_ID, ebufp);
	}
	return;
}
