#ifndef lint
static const char Sccs_id[] = "@(#)%Portal Version: fm_pymt_reprocess.c:BillingVelocityInt:3:2006-Sep-05 21:55:03 %";
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

#define FILE_LOGNAME "fm_pymt_reprocess.c(1.5)"
/************
*DEFINE FUNCTIONS
*************/
EXPORT_OP void
op_pymt_reprocess(
    cm_nap_connection_t *connp,
    int32               opcode,
    int32               flags,
    pin_flist_t         *i_flistp,
    pin_flist_t         **r_flistpp,
    pin_errbuf_t        *ebufp);

static void
fm_pymt_reprocess(
        pcm_context_t           *ctxp,
        pin_flist_t             *i_flistp,
        pin_flist_t             **r_flistp,
        pin_errbuf_t            *ebufp);

static void
fm_rbs_pymt_recycle(
        pcm_context_t           *ctxp,
        pin_flist_t             *i_flistp,
        pin_flist_t             **vali_oflistp,
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
***************************/
void
op_pymt_reprocess(
        cm_nap_connection_t 	*connp,
        int32               	opcode,
        int32               	flags,
        pin_flist_t         	*i_flistp,
        pin_flist_t         	**r_flistpp,
        pin_errbuf_t        	*ebufp)
{
        pcm_context_t       	*ctxp = connp->dm_ctx;
	poid_t                  *a_pdp = NULL;
        pin_flist_t             *r_flistp = NULL;
        int32                   status = 1;
        int                     local = 1;
	pin_flist_t             *result_flistp = NULL;	
        *r_flistpp = NULL;

        if (PIN_ERRBUF_IS_ERR(ebufp)) {
            return;
        }
        PIN_ERRBUF_CLEAR(ebufp);

        /***********************
        * Insanity Check
         **********************/
        if (opcode != RBS_PYMT_REPROCESS) {
                pin_set_err(ebufp, PIN_ERRLOC_FM, PIN_ERRCLASS_SYSTEM_DETERMINATE, PIN_ERR_BAD_OPCODE, 0, 0, opcode);
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "op_pymt_reprocess error", ebufp);
                return;
        }

        /***********************
        * Debug: Input flist
        ************************/
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "op_pymt_reprocess input", i_flistp);

	a_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_ACCOUNT_OBJ, 0, ebufp);
        local = fm_rbs_trans_open(ctxp, a_pdp, 3 , ebufp);
        if(PIN_ERRBUF_IS_ERR(ebufp))
        {
                PIN_ERRBUF_CLEAR(ebufp);
                result_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, result_flistp, PIN_FLD_POID, ebufp);
                status = FAILURE;
                PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_STATUS, &status, ebufp);
                PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_ERROR_DESCR, "Error in Opening Transaction", ebufp);
                PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_ERROR_CODE,"53001" , ebufp);
		
                PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "PAYMENT Fail output flist", result_flistp);
                *r_flistpp=PIN_FLIST_COPY(result_flistp, ebufp);
                PIN_FLIST_DESTROY_EX(&result_flistp, NULL);
                goto CLEANUP;
        }
        /**********************
        * Call the default implementation
        ************************/
        fm_pymt_reprocess(ctxp, i_flistp, &r_flistp, ebufp);

        /*********************
        * Results.
        ********************/
        if (PIN_ERRBUF_IS_ERR(ebufp))
        {
		status = FAILURE;
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "op_pymt_reprocess error", ebufp);
		goto CLEANUP;
        }
        else
        {
		status = SUCCESS;
                *r_flistpp = PIN_FLIST_COPY(r_flistp, ebufp);
                PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "op_pymt_reprocess: output flist::", *r_flistpp);
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

/***********************************
 *Function to implement
 *Payment reprocess
 ***********************************/
void
fm_pymt_reprocess(
        pcm_context_t           *ctxp,
        pin_flist_t             *i_flistp,
        pin_flist_t             **r_flistp,
        pin_errbuf_t            *ebufp)
{
        pin_flist_t             *vali_oflistp = NULL;
	pin_flist_t		*srch_out_flistp = NULL;
        pin_flist_t             *charges_flistp = NULL;
        pin_flist_t             *payment_flistp = NULL;
	pin_flist_t		*err_flistp = NULL;
	pin_flist_t		*result = NULL;
        pin_flist_t             *evt_oflistp = NULL;
        pin_flist_t             *cr_flistp = NULL;
        pin_flist_t             *cr_oflistp = NULL;
        pin_flist_t             *rec_oflistp = NULL;
        pin_flist_t             *cr_iflistp = NULL;
	pin_flist_t		*act_res_flistp = NULL;
        poid_t                  *pdp = NULL;
        poid_t                  *pymt_rej_pdp = NULL;
        int32                   *res_val = NULL;
	int32			res = 2;
        int32                   *succ_flagp = NULL;
        int32                   *rej_cnt = NULL;
	int32			count = 0;
        int32                   var =0;
	int32			status = 0;
	int32			*act_status = NULL;
        int64                   db = -1;
	char			*account_no = NULL;
        char                    msg[100];
        pin_decimal_t           *amount = NULL;

        if (PIN_ERRBUF_IS_ERR(ebufp))
                return;
        PIN_ERRBUF_CLEAR(ebufp);

        PIN_ERR_LOG_FLIST(3, "start fm_pymt_reprocess input flist: ", i_flistp);

	//Function to get account details
	fm_search_acct_details(ctxp, i_flistp, &srch_out_flistp, ebufp);
	PIN_ERR_LOG_FLIST(3, "fm_search_acct_details output flist:", srch_out_flistp);
	count = PIN_FLIST_ELEM_COUNT(srch_out_flistp, PIN_FLD_RESULTS, ebufp);
	//If account number not exist in DB
	if ((PIN_ERRBUF_IS_ERR(ebufp)) || (count < 1))
	{
		err_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_ACCOUNT_OBJ, err_flistp, PIN_FLD_POID, ebufp);
		status = 1;
		PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_STATUS, &status, ebufp);
		PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_CODE, "53003", ebufp);
		PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_DESCR, "Account_No Not Found", ebufp);
		PIN_ERR_LOG_FLIST(3, "error flist1: ", err_flistp);
		// PIN_ERR_LOG_EBUF(1, "Invalid Account_No", ebufp);
	}

       	amount = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_AMOUNT, 0, ebufp);
        pymt_rej_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp);
        rej_cnt = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_ACCOUNT_CNT, 1, ebufp);
        succ_flagp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_STATUS_FLAGS, 1, ebufp);

	//If account number exists in DB, then reverse the payment for suspense account
	if (count > 0)
	{
		result = PIN_FLIST_ELEM_GET(srch_out_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, 1, ebufp);
	        PIN_ERR_LOG_FLIST(3, "fm_search_acct_details results:", result);
	        PIN_FLIST_FLD_COPY(result, PIN_FLD_POID, i_flistp, PIN_FLD_TO_OBJ, ebufp);

	        //call fm_rbs_pymt_recycle function to validate
	        fm_rbs_pymt_recycle(ctxp, i_flistp, &vali_oflistp, ebufp);
	        PIN_ERR_LOG_FLIST(3, "fm_rbs_pymt_recycle output flist: ", vali_oflistp);
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			res = 0;
		}
	        pdp = PIN_FLIST_FLD_GET(vali_oflistp, PIN_FLD_POID, 0, ebufp);
	        charges_flistp = PIN_FLIST_ELEM_GET(vali_oflistp, PIN_FLD_RESULTS, 0, 1, ebufp);
	        res_val = PIN_FLIST_FLD_GET(charges_flistp, PIN_FLD_RESULT, 1, ebufp);
		if (res_val)
		{
			res = *res_val;
		}
	}
	//If payment reversal fail
        if (res == 0)
        {
		err_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, err_flistp, PIN_FLD_POID, ebufp);
                PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_ACCOUNT_OBJ, err_flistp, PIN_FLD_ACCOUNT_OBJ, ebufp);
		*rej_cnt = *rej_cnt + 1;
		sprintf(msg, "rej_cnt: %d", rej_cnt);
		PIN_ERR_LOG_MSG(3, msg);
		PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ACCOUNT_CNT, rej_cnt, ebufp);
                status = 1;
                PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_STATUS, &status, ebufp);
                PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_CODE, "53002", ebufp);
                PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_DESCR, "Invalid Data", ebufp);
		PIN_ERR_LOG_FLIST(3, "error flist2: ", err_flistp);
		PIN_ERR_LOG_EBUF(1, "REPROCESS Error", ebufp);

	}
	if (err_flistp)	
	{
		//Increment the rejected count in /rbs_rejected_payment
		PIN_ERR_LOG_MSG(3, "Unable to reprocess");
                cr_flistp = PIN_FLIST_CREATE(ebufp);
                PIN_FLIST_FLD_SET(cr_flistp, PIN_FLD_POID, pymt_rej_pdp, ebufp);
                *rej_cnt = *rej_cnt + 1;
                PIN_FLIST_FLD_SET(cr_flistp, PIN_FLD_ACCOUNT_CNT, rej_cnt, ebufp);
                if (!succ_flagp && *succ_flagp != 1)
                {
                        *succ_flagp = 1;
                        PIN_FLIST_FLD_SET(cr_flistp, PIN_FLD_STATUS_FLAGS, succ_flagp, ebufp);
                }

                PIN_ERR_LOG_FLIST(3, "Write_flds input flist: ", cr_flistp);
                PCM_OP(ctxp, PCM_OP_WRITE_FLDS, 0, cr_flistp, &cr_oflistp, ebufp);
                if (PIN_ERRBUF_IS_ERR(ebufp))
                {
                        PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                               "fm_pymt_reprocess: WRITE_FLDS Error", ebufp);
                        goto CLEANUP;
                }
                PIN_ERR_LOG_FLIST(3, "Write_flds output flist: ", cr_oflistp);
		*r_flistp = PIN_FLIST_COPY(err_flistp, ebufp);
		goto CLEANUP;
        }
        else
        {
		//Payment reversal success, update status as '0' in /rbs_rejected_payment
                cr_flistp = PIN_FLIST_CREATE(ebufp);
                PIN_FLIST_FLD_SET(cr_flistp, PIN_FLD_POID, pymt_rej_pdp, ebufp);
                status = 0;
                PIN_FLIST_FLD_SET(cr_flistp, PIN_FLD_STATUS_FLAGS, &status, ebufp);

                PIN_ERR_LOG_FLIST(3, "Write_flds input flist: ", cr_flistp);
                PCM_OP(ctxp, PCM_OP_WRITE_FLDS, 0, cr_flistp, &cr_oflistp, ebufp);
                if (PIN_ERRBUF_IS_ERR(ebufp))
                {
                        PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                                "fm_pymt_reprocess: WRITE_FLDS Error", ebufp);
                        goto CLEANUP;
                }
                PIN_ERR_LOG_FLIST(3, "Write_flds output flist: ", cr_oflistp);
                PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_ACCOUNT_OBJ, cr_oflistp, PIN_FLD_ACCOUNT_OBJ, ebufp);
                PIN_FLIST_FLD_SET(cr_oflistp, PIN_FLD_STATUS, &status, ebufp);
                PIN_FLIST_FLD_SET(cr_oflistp, PIN_FLD_DESCR, "PAYMENT REPROCESS success", ebufp);
        }
	*r_flistp = PIN_FLIST_COPY(cr_oflistp, ebufp);
	PIN_ERR_LOG_FLIST(3, "RECYCLE_PAYMENT output flist: ", *r_flistp);
CLEANUP:
	PIN_FLIST_DESTROY_EX(&cr_oflistp, NULL);
	PIN_FLIST_DESTROY_EX(&srch_out_flistp, NULL);
return;
}

/****************************************
 *Funtion for payment reversal
 ****************************************/
void
fm_rbs_pymt_recycle(
        pcm_context_t           *ctxp,
        pin_flist_t             *i_flistp,
        pin_flist_t             **r_flistp,
        pin_errbuf_t            *ebufp)
{

        pin_flist_t             *vali_iflistp = NULL;
        pin_flist_t             *vali_oflistp = NULL;
        pin_flist_t             *charges_flistp = NULL;
        pin_flist_t             *pay_flistp = NULL;
        pin_flist_t             *inh_info_flistp = NULL;
        pin_flist_t             *cash_info_flistp = NULL;
        pin_flist_t             *check_info_flistp = NULL;
        pin_flist_t             *cc_info_flistp = NULL;
        pin_flist_t             *dd_info_flistp = NULL;
        pin_flist_t             *rd_flistp = NULL;
        pin_flist_t             *rd_oflistp = NULL;
        pin_flist_t             *er_flistp = NULL;
        pin_flist_t             *er_res_flistp = NULL;
        poid_t                  *pdp = NULL;
        poid_t                  *bal_grp_obj = NULL;
        poid_t                  *billinfo_pdp = NULL;
        poid_t                  *event_pdp = NULL;
        poid_t                  *item_obj = NULL;
        char                    *acc_num = NULL;
        char                    *evnt_type = NULL;
        char                    *descr = NULL;
        char                    *channel = NULL;
        int32                   pay_type = 0;
        int32                   var = 0;
	int32   		currency = 344;
	int32  			command = 0;
	int32			status = 0;
	int32			susp_status = 0;
        pin_decimal_t           *amount = 0;

        if (PIN_ERRBUF_IS_ERR(ebufp))
                return;
        PIN_ERRBUF_CLEAR(ebufp);

        PIN_ERR_LOG_FLIST(3, "start fm_rbs_pymt_recycle input flist: ", i_flistp);

        acc_num = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_ACCOUNT_NO, 0, ebufp);
        descr = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_DESCR, 1, ebufp);
        channel = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_CHANNEL, 0, ebufp);
        pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_ACCOUNT_OBJ, 0, ebufp);
        event_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_EVENT_OBJ, 0, ebufp);
        amount = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_AMOUNT, 0, ebufp);

        vali_iflistp = PIN_FLIST_CREATE(ebufp);
        PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_TO_OBJ, vali_iflistp, PIN_FLD_POID, ebufp);
        PIN_FLIST_FLD_SET(vali_iflistp, PIN_FLD_PROGRAM_NAME, "PAYMENT RECYCLE", ebufp);
	if (descr)
	{
        	PIN_FLIST_FLD_SET(vali_iflistp, PIN_FLD_DESCR, descr, ebufp);
        }
	PIN_FLIST_FLD_SET(vali_iflistp, PIN_FLD_CHANNEL, channel, ebufp);
	
        charges_flistp = PIN_FLIST_ELEM_ADD(vali_iflistp, PIN_FLD_CHARGES, 0, ebufp);
	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_TO_OBJ, charges_flistp, PIN_FLD_ACCOUNT_OBJ, ebufp);
        PIN_FLIST_FLD_SET(charges_flistp, PIN_FLD_EVENT_OBJ, event_pdp, ebufp);

        PIN_ERR_LOG_POID(3, "event obj: ", event_pdp);
        rd_flistp = PIN_FLIST_CREATE(ebufp);
        PIN_FLIST_FLD_SET(rd_flistp, PIN_FLD_POID, event_pdp, ebufp);

        PIN_ERR_LOG_FLIST(3, "event read_obj input flist: ", rd_flistp);
        PCM_OP(ctxp, PCM_OP_READ_OBJ, 0, rd_flistp, &rd_oflistp, ebufp);
        if (PIN_ERRBUF_IS_ERR(ebufp))
        {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                               "fm_pymt_reprocess: event READ_OBJ Error", ebufp);
                goto CLEANUP;
        }

        PIN_ERR_LOG_FLIST(3, "event read_obj output flist: ", rd_oflistp);
        event_pdp = PIN_FLIST_FLD_GET(rd_oflistp, PIN_FLD_POID, 0, ebufp);
        evnt_type = (char *)PIN_POID_GET_TYPE(event_pdp);

        susp_status = 0;
        PIN_FLIST_FLD_SET(charges_flistp, PIN_FLD_STATUS, &susp_status, ebufp);
        PIN_FLIST_FLD_SET(charges_flistp, PIN_FLD_AMOUNT, amount, ebufp);
        PIN_FLIST_FLD_SET(charges_flistp, PIN_FLD_ACCOUNT_NO, acc_num, ebufp);
        pay_flistp = PIN_FLIST_SUBSTR_ADD(charges_flistp, PIN_FLD_PAYMENT, ebufp);
        inh_info_flistp = PIN_FLIST_SUBSTR_ADD(pay_flistp, PIN_FLD_INHERITED_INFO,  ebufp);

        if (evnt_type && strcmp(evnt_type, "/event/billing/payment/cash") == 0)
        {
                cash_info_flistp = PIN_FLIST_ELEM_GET(rd_oflistp, PIN_FLD_CASH_INFO, 0, 1, ebufp);
                PIN_FLIST_ELEM_PUT(inh_info_flistp, cash_info_flistp, PIN_FLD_CASH_INFO, 0, ebufp);
                PIN_ERR_LOG_FLIST(3, "pay_flistp after cash_info: ", pay_flistp);
                pay_type = 10011;
                PIN_FLIST_FLD_SET(charges_flistp, PIN_FLD_PAY_TYPE, &pay_type, ebufp);
        }
        else if (evnt_type && strcmp(evnt_type, "/event/billing/payment/check") == 0)
        {
                check_info_flistp = PIN_FLIST_ELEM_GET(rd_oflistp, PIN_FLD_CHECK_INFO, 0, 1, ebufp);
                PIN_FLIST_ELEM_PUT(inh_info_flistp, check_info_flistp, PIN_FLD_CHECK_INFO, 0, ebufp);
                PIN_ERR_LOG_FLIST(3, "pay_flistp after check_info: ", pay_flistp);
                pay_type = 10012;
                PIN_FLIST_FLD_SET(charges_flistp, PIN_FLD_PAY_TYPE, &pay_type, ebufp);
        }
        currency = 344;
        PIN_FLIST_FLD_SET(charges_flistp, PIN_FLD_CURRENCY, &currency, ebufp);
        command = 0;
        PIN_FLIST_FLD_SET(charges_flistp, PIN_FLD_COMMAND, &command, ebufp);
        PIN_FLIST_FLD_SET(i_flistp, PIN_FLD_PAY_TYPE, &pay_type, ebufp);
        PIN_FLIST_SUBSTR_SET(i_flistp, pay_flistp, PIN_FLD_PAYMENT, ebufp);
        PIN_ERR_LOG_FLIST(3, "RECYCLE_PAYMENT opcode input flist: ", vali_iflistp);
        PCM_OP(ctxp, PCM_OP_PYMT_RECYCLE_PAYMENT, 0, vali_iflistp, &vali_oflistp, ebufp);
        if (PIN_ERRBUF_IS_ERR(ebufp))
        {
                PIN_ERRBUF_RESET(ebufp);
                er_flistp = PIN_FLIST_CREATE(ebufp);
                PIN_FLIST_FLD_SET(er_flistp, PIN_FLD_POID, pdp, ebufp);
                er_res_flistp = PIN_FLIST_ELEM_ADD(er_flistp, PIN_FLD_RESULTS, 0, ebufp);
		var = 0;
		PIN_FLIST_FLD_SET(er_res_flistp, PIN_FLD_RESULT, &var, ebufp);
                status = 0;
                PIN_FLIST_FLD_SET(er_res_flistp, PIN_FLD_STATUS, &status, ebufp);
		PIN_FLIST_FLD_SET(er_res_flistp, PIN_FLD_ERROR_CODE, "53005", ebufp);
                PIN_FLIST_FLD_SET(er_res_flistp, PIN_FLD_ERROR_DESCR , "Invalid Data", ebufp);
                *r_flistp = PIN_FLIST_COPY(er_flistp, ebufp);
                PIN_ERR_LOG_FLIST(3, "fm_rbs_pymt_recycle error flist: ", *r_flistp);
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "fm_pymt_reprocess: RECYCLE_PAYMENT Error", ebufp);
		
		*r_flistp = PIN_FLIST_COPY(er_flistp, ebufp);
                goto CLEANUP;
        }
        PIN_ERR_LOG_FLIST(3, "fm_rbs_pymt_recycle function output flist: ", vali_oflistp);
        *r_flistp = PIN_FLIST_COPY(vali_oflistp, ebufp);
CLEANUP:
	PIN_FLIST_DESTROY_EX(&vali_oflistp, NULL);
	PIN_FLIST_DESTROY_EX(&rd_oflistp, NULL);
return;
}

