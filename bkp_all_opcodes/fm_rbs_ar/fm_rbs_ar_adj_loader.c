#ifndef lint
static const char Sccs_id[] = "@(#)%Portal Version: fm_rbs_ar_adj_loader.c:CUPmod7.3PatchInt:1:2006-Dec-01 16:31:18 %";
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
#include "pcm_ops.h"
#include "ops/ar.h"
#include "ops/bill.h"
#include "pin_decimal.h"
#include "rbs_ar_adjustment.h"

#define SUCCESS 0
#define FAILURE 1
#define READWRITE 1
#define LOCAL_TRANS_OPEN_SUCCESS 0

EXPORT_OP void
op_ar_adj_loader(
    cm_nap_connection_t *connp,
    int32           opcode,
    int32           flags,
    pin_flist_t     *i_flistp,
    pin_flist_t     **r_flistpp,
    pin_errbuf_t        *ebufp);

static void
fm_rbs_ar_adj_loader(
    pcm_context_t       *ctxp,
    pin_flist_t     *i_flistp,
    pin_flist_t     **r_flistpp,
    pin_errbuf_t        *ebufp);

static void
fm_get_item_details(
        pcm_context_t          *ctxp,
        pin_flist_t            *i_flistp,
        pin_flist_t            **r_flistp,
        pin_errbuf_t           *ebufp);

static void
fm_get_bill_details(
        pcm_context_t           *ctxp,
        pin_flist_t             *account_no,
        pin_flist_t             **rs_flistp,
        pin_errbuf_t            *ebufp);

static void
fm_check_reason_code(
        pcm_context_t          *ctxp,
        pin_flist_t            *i_flistp,
        pin_flist_t            **r_flistp,
        pin_errbuf_t           *ebufp);

static void
fm_get_account_details(
	pcm_context_t          *ctxp,
        pin_flist_t            *i_flistp,
        pin_flist_t            **r_flistp,
        pin_errbuf_t           *ebufp);

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

PIN_IMPORT void
fm_search_billinfo_details(
        pcm_context_t      *ctxp,
        pin_flist_t        *i_flistp,
        pin_flist_t        **r_flistp,
        pin_errbuf_t       *ebufp);
/*************************************
* Function Implementations
*************************************/
void
op_ar_adj_loader(
    	cm_nap_connection_t 	*connp,
    	int32           	opcode,
    	int32           	flags,
    	pin_flist_t     	*i_flistp,
    	pin_flist_t     	**r_flistpp,
    	pin_errbuf_t        	*ebufp)
{
    	pcm_context_t       	*ctxp = connp->dm_ctx;
    	poid_t          	*account_obj= NULL;
	poid_t			*a_pdp = NULL;
    	pin_flist_t     	*r_flistp = NULL;
    	int32                 	status = 1;
	int                     local = 1;
    	pin_decimal_t       	*amount = NULL;
    	pin_flist_t         	*s_oflistp = NULL;
    	pin_flist_t         	*result_flistp = NULL;

    	if (PIN_ERRBUF_IS_ERR(ebufp)) {
        	return;
    	}
    	PIN_ERRBUF_CLEAR(ebufp);

    	// Insanity Check
    	if (opcode != RBS_OP_ADJ_LOADER) {
        	pin_set_err(ebufp, PIN_ERRLOC_FM,
            	PIN_ERRCLASS_SYSTEM_DETERMINATE,
            	PIN_ERR_BAD_OPCODE, 0, 0, opcode);
            	PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
            	"op_ar_adj_loader error",
            	ebufp);
        	return;
    	}

	a_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp);
        local = fm_rbs_trans_open(ctxp, a_pdp, 3 , ebufp);
        if(PIN_ERRBUF_IS_ERR(ebufp))
        {
                PIN_ERRBUF_CLEAR(ebufp);
                result_flistp = PIN_FLIST_CREATE(ebufp);
                status = FAILURE;
                PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_STATUS, &status, ebufp);
                PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_ERROR_DESCR, "Error opening transaction", ebufp);
                PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, result_flistp, PIN_FLD_POID, ebufp);
                PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_ERROR_CODE,"53080" , ebufp);
                PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "ADJUSTMENT Fail output flist", result_flistp);
                *r_flistpp=PIN_FLIST_COPY(result_flistp, ebufp);
                PIN_FLIST_DESTROY_EX(&result_flistp, NULL);
                goto CLEANUP;
        }
        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Transaction Opened");

    	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "op_ar_adj_loader  Input Flist:", i_flistp);
    	fm_rbs_ar_adj_loader(ctxp, i_flistp, &r_flistp, ebufp);
    	if (PIN_ERRBUF_IS_ERR(ebufp))
    	{
    		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"op_rbs_ar_adj_loader error", ebufp);
        	PIN_ERRBUF_RESET(ebufp);
        	status = FAILURE;
        	*r_flistpp = PIN_FLIST_CREATE(ebufp);
        	account_obj = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 1, ebufp);
        	PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_POID, account_obj, ebufp);
        	PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_ERROR_CODE, "53094", ebufp);
        	PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_ERROR_DESCR,
                        "op_rbs_ar_adj_loader error", ebufp);
        	PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_STATUS, &status, ebufp);
		goto CLEANUP;
    	}
    	else
    	{
		status = SUCCESS;
        	*r_flistpp = PIN_FLIST_COPY(r_flistp, ebufp);
        	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,  "op_rbs_ar_adj_loader output flist", *r_flistpp);
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

static void
fm_rbs_ar_adj_loader(
        pcm_context_t   *ctxp,
        pin_flist_t     *i_flistp,
        pin_flist_t     **r_flistpp,
        pin_errbuf_t    *ebufp)
{
	pin_flist_t     *d_iflistp =NULL;
	pin_flist_t     *r_flistp = NULL;
        pin_flist_t     *s_oflistp = NULL;
        pin_flist_t     *nameinfo_flistp = NULL;
        pin_flist_t     *phones_flistp = NULL;
        pin_flist_t     *res_flistp = NULL;
	pin_flist_t     *srch_iflistp = NULL;
        pin_flist_t     *srch_oflistp = NULL;
        pin_flist_t     *rs_flistp = NULL;
        pin_flist_t     *sched_flistp = NULL;
        pin_flist_t     *sched_oflistp = NULL;
	pin_flist_t	*ad_res_flistp = NULL;
	pin_flist_t	*rd_iflistp = NULL;
	pin_flist_t	*rd_oflistp = NULL;
	pin_flist_t	*event_flistp = NULL;
	pin_flist_t	*total_flistp = NULL;
	pin_flist_t	*misc_flistp = NULL;
	pin_flist_t	*ser_iflistp = NULL;
	pin_flist_t	*ser_oflistp = NULL;
	pin_flist_t	*ser_res_flistp = NULL;
	poid_t		*c_acc_obj = NULL;
	poid_t		*ser_pdp = NULL;
	poid_t		*ses_obj = NULL;
	poid_t		*billinfo_pdp = NULL;
	poid_t		*ad_evt_pdp = NULL;
        poid_t          *bal_grp_pdp = NULL;
        poid_t          *acc_obj = NULL;
	poid_t          *pdp = NULL;
	char		*prog_name = NULL;
	char            *type_str = NULL;
        char            *mode_str = NULL;
        char            *acc_no = NULL;
	char		*login = NULL;
        char            *r_mob_no = NULL;
	char		*reason_code = NULL;
        char            *o_descr = NULL;
        char            *mobile_no = NULL;
	char            *descr = NULL;
        char            *action_name = NULL;
        char            msg[100];
        char            str_var[100];
	char            *char_buf = NULL;
	char		*f_eff_date = NULL;
	char            yyyy[5], mm[3], dd[3];
        int32           *flag = NULL;
        int32           *reason_id = NULL;
	int32		var = 0;
        int32           status = 0;
	int32		*r_status = NULL;
	int32		*currency = NULL;
	int32		phn_count = 0;
	int32		elem_id = 0;
	int32		elem_id1 = 0;
	int32		elem_id2 = 0;
	int32		res_cnt = 0;
	int32		cnt = 0;
	int32		*s_cnt = NULL;
	int32		len = 0;
	int		opcode = 0;
        int64           db = -1;
	int             *date = NULL;
        int32           datep = 0;
        int32           header = 0;
        int32           trailer = 0;
        int32           monthp = 0;
        int32           yearp = 0;
        int64           current_date = 0;
        int             year;
        int             month;
        int             day;
	time_t		now_t = 0;
	time_t		post_t = 0;
	time_t		pvt = 0;
	pin_decimal_t   *amount = NULL;
	pin_decimal_t	*amtp = pbo_decimal_from_str("-1.0", ebufp);
	pin_cookie_t	cookie = NULL;
	pin_cookie_t	cookie1 = NULL;
	pin_cookie_t	cookie2 = NULL;
	pin_buf_t	*pin_bufp = NULL;
	struct tm	tmp;

        if (PIN_ERRBUF_IS_ERR(ebufp))
                return;
        PIN_ERRBUF_CLEAR(ebufp);

        PIN_ERR_LOG_FLIST(3, "start fm_rbs_ar_adj_loader input flist: ", i_flistp);

        pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp);
	db = PIN_POID_GET_DB(pdp);
	prog_name = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_PROGRAM_NAME, 1, ebufp);
	if (!prog_name)
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"Mandatory Field missing - Program_name error", ebufp);
		PIN_ERRBUF_RESET(ebufp);
		status = 1;
		r_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_POID, pdp, ebufp);
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &status, ebufp);
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "53099", ebufp);
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, "Mandatory Field missing - Program_name error", ebufp);
		goto CLEANUP;
	}
	if (PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_ACTION, 1, ebufp))
        {
                mode_str = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_ACTION, 0, ebufp);
                if (mode_str && strlen(mode_str) > 1)
                {
                        PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"Invalid Indicator Length", ebufp);
                        PIN_ERRBUF_RESET(ebufp);
                        status = 1;
                        r_flistp = PIN_FLIST_CREATE(ebufp);
                        PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_POID, pdp, ebufp);
                        PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &status, ebufp);
                        PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "53097", ebufp);
                        PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, "Invalid Indicator Length", ebufp);
                        goto CLEANUP;
                }
        }
	acc_no = PIN_FLIST_FLD_GET (i_flistp, PIN_FLD_ACCOUNT_NO, 1, ebufp);
	PIN_ERR_LOG_MSG(3, acc_no);
	if (!acc_no || strcmp(acc_no, "") == 0)
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"fm_rbs_ar_adj_loader mandatory field missing", ebufp);
		PIN_ERRBUF_RESET(ebufp);
		status = 1;
               	r_flistp = PIN_FLIST_CREATE(ebufp);
       	        PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_POID, pdp, ebufp);
                PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &status, ebufp);
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "53079", ebufp);
               	PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, "Account_no is missing", ebufp);
		goto CLEANUP;
	}
	if (acc_no && strlen(acc_no) > 14)
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Invalid Account_no Length", ebufp);
                PIN_ERRBUF_RESET(ebufp);
                status = 1;
                r_flistp = PIN_FLIST_CREATE(ebufp);
                PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_POID, pdp, ebufp);
                PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &status, ebufp);
                PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "53091", ebufp);
                PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, "Invalid Account_no Length", ebufp);
                goto CLEANUP;
	}
	mobile_no = (char *)PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_PHONE, 1, ebufp);	
/*
	if (prog_name && !strstr(prog_name, "Delete") && (!mobile_no || strcmp(mobile_no, "") == 0))
        {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"fm_rbs_ar_adj_loader mandatory field missing", ebufp);
                PIN_ERRBUF_RESET(ebufp);
                status = 1;
                r_flistp = PIN_FLIST_CREATE(ebufp);
                PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_POID, pdp, ebufp);
                PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &status, ebufp);
                PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "53079", ebufp);
                PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, "Mobile_no is missing", ebufp);
                goto CLEANUP;
        }
*/
	if (mobile_no && strlen(mobile_no) > 12)
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"Invalid Mobile_no Length", ebufp);
                PIN_ERRBUF_RESET(ebufp);
                status = 1;
                r_flistp = PIN_FLIST_CREATE(ebufp);
                PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_POID, pdp, ebufp);
                PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &status, ebufp);
                PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "53092", ebufp);
                PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, "Invalid Mobile_no Length", ebufp);
                goto CLEANUP;
	}
	reason_id = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_REASON_ID, 1, ebufp);
	if (!reason_id || *reason_id == 0)
        {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"fm_rbs_ar_adj_loader mandatory field missing", ebufp);
                PIN_ERRBUF_RESET(ebufp);
                status = 1;
                r_flistp = PIN_FLIST_CREATE(ebufp);
                PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_POID, pdp, ebufp);
                PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &status, ebufp);
                PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "53081", ebufp);
                PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, "Adjustment code is missing", ebufp);
             	goto CLEANUP;
	}
	sprintf(str_var, "%d", *reason_id);
	PIN_ERR_LOG_MSG(3, str_var);
	if (reason_id && strlen(str_var) > 9)
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"Adjustment code is out of range", ebufp);
                PIN_ERRBUF_RESET(ebufp);
                status = 1;
                r_flistp = PIN_FLIST_CREATE(ebufp);
                PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_POID, pdp, ebufp);
                PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &status, ebufp);
                PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "53093", ebufp);
                PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, "Adjustment code is out of range", ebufp);
                goto CLEANUP;
	}
	type_str = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_TYPE_STR, 0, ebufp);
	if (!type_str|| strcmp(type_str, "") == 0)
	{
		PIN_ERR_LOG_MSG(3, type_str);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"fm_rbs_ar_adj_loader mandatory field missing", ebufp);
                PIN_ERRBUF_RESET(ebufp);
                status = 1;
                r_flistp = PIN_FLIST_CREATE(ebufp);
                PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_POID, pdp, ebufp);
                PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &status, ebufp);
                PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "53081", ebufp);
                PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, "Credit/Debit Adjustment is missing", ebufp);
                goto CLEANUP;
	}
	if (type_str && strlen(type_str) > 1)
	{
		PIN_ERR_LOG_MSG(3, type_str);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"Invalid Credit/Debit Adjustment Length", ebufp);
                PIN_ERRBUF_RESET(ebufp);
                status = 1;
                r_flistp = PIN_FLIST_CREATE(ebufp);
                PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_POID, pdp, ebufp);
                PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &status, ebufp);
                PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "53094", ebufp);
                PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, "Invalid adjustment type- must be C or D", ebufp);
                goto CLEANUP;
	} 
	else if (type_str && ((strcmp(type_str, "C") != 0) && (strcmp(type_str, "D") != 0)))
	{
		PIN_ERR_LOG_MSG(3, type_str);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"Invalid Credit/Debit Adjustment Type", ebufp);
                PIN_ERRBUF_RESET(ebufp);
                status = 1;
                r_flistp = PIN_FLIST_CREATE(ebufp);
                PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_POID, pdp, ebufp);
                PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &status, ebufp);
                PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "53094", ebufp);
                PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, "Invalid adjustment type - must be C or D", ebufp);
                goto CLEANUP;
	}
	if (PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_REASON_CODE, 1, ebufp))
        {
                reason_code = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_REASON_CODE, 0, ebufp);
		if (strlen(reason_code) > 60)
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"Invalid Adjustment Reason Length", ebufp);
                	PIN_ERRBUF_RESET(ebufp);
                	status = 1;
                	r_flistp = PIN_FLIST_CREATE(ebufp);
                	PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_POID, pdp, ebufp);
                	PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &status, ebufp);
                	PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "53098", ebufp);
                	PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, "Invalid Adjustment Reason Length", ebufp);
                	goto CLEANUP;
		}
        }

	currency = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_CURRENCY, 0, ebufp);	
	if (!currency || *currency == 0)
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"fm_rbs_ar_adj_loader mandatory field missing", ebufp);
                PIN_ERRBUF_RESET(ebufp);
                status = 1;
                r_flistp = PIN_FLIST_CREATE(ebufp);
                PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_POID, pdp, ebufp);
                PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &status, ebufp);
                PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "53081", ebufp);
                PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, "Currency is missing", ebufp);
                goto CLEANUP;
	}
        sprintf(str_var, "%d", *currency);
        PIN_ERR_LOG_MSG(3, str_var);
	if (currency && strlen(str_var) > 3)
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"Invalid currency - must be 344", ebufp);
                PIN_ERRBUF_RESET(ebufp);
                status = 1;
                r_flistp = PIN_FLIST_CREATE(ebufp);
                PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_POID, pdp, ebufp);
                PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &status, ebufp);
                PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "53095", ebufp);
                PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, "Invalid currency - must be 344", ebufp);
                goto CLEANUP;
	}
	if (PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_DESCR, 1, ebufp))
        {
                descr = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_DESCR, 0, ebufp);
		if (descr && strlen(descr) > 255)
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"Invalid Remarks Length", ebufp);
                	PIN_ERRBUF_RESET(ebufp);
                	status = 1;
                	r_flistp = PIN_FLIST_CREATE(ebufp);
                	PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_POID, pdp, ebufp);
                	PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &status, ebufp);
                	PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "53096", ebufp);
                	PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, "Invalid Remarks Length", ebufp);
                	goto CLEANUP;
		}
        }
	now_t = pin_virtual_time((time_t *)NULL);
        now_t = fm_utils_time_round_to_midnight(now_t);
	f_eff_date = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_START_CREATION_DATE, 1, ebufp);
	if (f_eff_date && strlen(f_eff_date) > 8)
        {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"Invalid Future Effective Date Length", ebufp);
                PIN_ERRBUF_RESET(ebufp);
                status = 1;
                r_flistp = PIN_FLIST_CREATE(ebufp);
                PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_POID, pdp, ebufp);
                PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &status, ebufp);
                PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "53098", ebufp);
                PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, "Invalid Future Effective Date Length", ebufp);
                goto CLEANUP;
        }
        else
        {
                if ((!f_eff_date || (strcmp(f_eff_date, "") == 0)) && (prog_name && (strstr(prog_name, "Delete"))))
                {
                        PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"fm_rbs_ar_adj_loader mandatory field missing", ebufp);
                        PIN_ERRBUF_RESET(ebufp);
                        status = 1;
                        r_flistp = PIN_FLIST_CREATE(ebufp);
                        PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_POID, pdp, ebufp);
                        PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &status, ebufp);
                        PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "53083", ebufp);
                        PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, "Adjustment Date is missing", ebufp);
                        goto CLEANUP;
                }
                else if ((!f_eff_date || (strcmp(f_eff_date, "") == 0)) && (prog_name && (strstr(prog_name, "Adjustment"))))
                {
                        post_t = (time_t)now_t;
			sprintf(msg, "post_t: %d, now_t: %d", post_t, now_t);
		        PIN_ERR_LOG_MSG(3, msg);
                }
		else
		{
			memset(yyyy, '\0', 5*sizeof(char));
			memset(mm, '\0', 3*sizeof(char));
			memset(dd, '\0', 3*sizeof(char));
			sscanf(f_eff_date, "%4c%2c%2c", yyyy, mm, dd);
	
			datep = (int32 )atoi(dd);
			monthp = (int32 )atoi(mm);
			yearp = (int32 )atoi(yyyy);
			sprintf(msg, "year: %d, month: %d, date: %d", yearp, monthp, datep);
        		PIN_ERR_LOG_MSG(3, msg);

			if ( (monthp < 1 || monthp > 12) || (datep < 1 || datep > 31))
        		{
                		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Invalid Adjustment Date Format", ebufp);
                		PIN_ERRBUF_RESET(ebufp);
                		status = 1;
                		r_flistp = PIN_FLIST_CREATE(ebufp);
                		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_POID, pdp, ebufp);
                		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &status, ebufp);
                		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "53078", ebufp);
                		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, "Invalid Adjustment Date Format", ebufp);
                		goto CLEANUP;
        		}
			
			tmp.tm_year = yearp - 1900;	// Year - 1900
			tmp.tm_mon = monthp - 1;	// Month, where 0 = jan
			tmp.tm_mday = datep;		// Day of the month
			tmp.tm_hour = 00;
			tmp.tm_min = 00;
			tmp.tm_sec = 00;
			tmp.tm_isdst = -1;		// Is DST on? 1 = yes, 0 = no, -1 = unknown
			post_t = mktime(&tmp);
		}
	}
        sprintf(msg, "post_t: %d, now_t: %d", post_t, now_t);
        PIN_ERR_LOG_MSG(3, msg);
/*
	if (post_t < now_t)
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"fm_rbs_ar_adj_loader mandatory field missing", ebufp);
                PIN_ERRBUF_RESET(ebufp);
                status = 1;
                r_flistp = PIN_FLIST_CREATE(ebufp);
                PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_POID, pdp, ebufp);
                PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &status, ebufp);
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "53083", ebufp);
                PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, "The Effective date should not be less than the current date", ebufp);
                goto CLEANUP;
        }
*/
	srch_iflistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_SET(srch_iflistp, PIN_FLD_POID, PIN_POID_CREATE(db, "/search", -1, ebufp), ebufp);
	PIN_FLIST_FLD_SET(srch_iflistp, PIN_FLD_ACCOUNT_NO, acc_no, ebufp);

	PIN_ERR_LOG_FLIST(3, "fm_get_account_details input flist:", srch_iflistp);
	fm_get_account_details(ctxp, srch_iflistp, &srch_oflistp, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp))
       	{
               	PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "fm_get_account_details call search error", ebufp);
               	return;
       	}
	PIN_ERR_LOG_FLIST(3, "fm_get_account_details output flist:", srch_oflistp);
	res_cnt = (int32)PIN_FLIST_ELEM_COUNT(srch_oflistp, PIN_FLD_RESULTS, ebufp);
	if (res_cnt > 0)
	{
		rs_flistp = PIN_FLIST_ELEM_GET(srch_oflistp, PIN_FLD_RESULTS, 0, 1, ebufp);
		acc_obj = PIN_FLIST_FLD_GET(rs_flistp, PIN_FLD_POID, 0, ebufp);
		bal_grp_pdp = PIN_FLIST_FLD_GET(rs_flistp, PIN_FLD_BAL_GRP_OBJ, 0, ebufp);
		elem_id = 0;
		cookie = NULL;
		while ((nameinfo_flistp = PIN_FLIST_ELEM_GET_NEXT(rs_flistp, PIN_FLD_NAMEINFO, &elem_id, 1, &cookie, ebufp)) != (pin_flist_t *)NULL)
		{
			phn_count = PIN_FLIST_ELEM_COUNT(nameinfo_flistp, PIN_FLD_PHONES, ebufp);
			if (mobile_no && (phn_count > 0))
                       	{
				elem_id1 = 0;
				cookie1 = NULL;
				while ((phones_flistp = PIN_FLIST_ELEM_GET_NEXT(nameinfo_flistp, PIN_FLD_PHONES, &elem_id1, 1, &cookie1, ebufp)) != (pin_flist_t *)NULL)
				{
					r_mob_no = PIN_FLIST_FLD_GET(phones_flistp, PIN_FLD_PHONE, 0, ebufp);
					sprintf(msg, "mobile_no: %s, r_mob_no: %s", mobile_no, r_mob_no);
              	        		PIN_ERR_LOG_MSG(3, msg);
					if (mobile_no && r_mob_no && strcmp(mobile_no, r_mob_no) == 0)
					{
						PIN_ERR_LOG_MSG(3, "Given Mobile_No matched with the Account_No");
						break;
					}
				}
			}
		}
		PIN_ERR_LOG_MSG(3, "Mobile number validation");
		if (!r_mob_no && mobile_no && (strcmp(mobile_no, "") != 0) && ((strcmp(r_mob_no, "") == 0) || (!strstr(mobile_no, r_mob_no))))
		{
			//Call fm_search_billinfo_details with acc_obj
			PIN_ERR_LOG_MSG(3, "Billing details");
			rd_iflistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_SET(rd_iflistp, PIN_FLD_POID, pdp, ebufp);
			PIN_FLIST_FLD_SET(rd_iflistp, PIN_FLD_ACCOUNT_OBJ, acc_obj, ebufp);
			PIN_ERR_LOG_FLIST(3, "call fm_search_billinfo_details input flist:", rd_iflistp);
			fm_search_billinfo_details(ctxp, rd_iflistp, &rd_oflistp, ebufp);
			if (PIN_ERRBUF_IS_ERR(ebufp))
			{
				PIN_ERR_LOG_EBUF(1, "Calling fm_search_billinfo_details error", ebufp);
				return;
			}
			PIN_ERR_LOG_FLIST(3, "call fm_search_billinfo_details output flist:", rd_oflistp);
			ad_res_flistp = PIN_FLIST_ELEM_GET(rd_oflistp, PIN_FLD_RESULTS, 0, 1, ebufp);
			billinfo_pdp = PIN_FLIST_FLD_GET(ad_res_flistp, PIN_FLD_POID, 0, ebufp);
		
			rd_iflistp = PIN_FLIST_CREATE(ebufp);
                	PIN_FLIST_FLD_SET(rd_iflistp, PIN_FLD_POID, pdp, ebufp);
                	PIN_FLIST_FLD_SET(rd_iflistp, PIN_FLD_AR_BILLINFO_OBJ, billinfo_pdp, ebufp);
                	PIN_ERR_LOG_FLIST(3, "call fm_search_billinfo_details input flist:", rd_iflistp);
                	fm_search_billinfo_details(ctxp, rd_iflistp, &rd_oflistp, ebufp);
                	if (PIN_ERRBUF_IS_ERR(ebufp))
                	{
                        	PIN_ERR_LOG_EBUF(1, "Calling fm_search_billinfo_details error", ebufp);
                        	return;
                	}
                	PIN_ERR_LOG_FLIST(3, "call fm_search_billinfo_details output flist:", rd_oflistp);
			elem_id1 = 0;
			cookie1 =  NULL;
			while ((ad_res_flistp = PIN_FLIST_ELEM_GET_NEXT(rd_oflistp, PIN_FLD_RESULTS,  &elem_id1, 1, &cookie1, ebufp)) != NULL)
			{
				c_acc_obj = PIN_FLIST_FLD_GET(ad_res_flistp, PIN_FLD_ACCOUNT_OBJ, 0, ebufp);
				PIN_ERR_LOG_POID(3, "child acc_obj: ", c_acc_obj);
				ser_pdp = PIN_POID_CREATE(db, "/service/subscription", -1, ebufp);

				ser_iflistp = PIN_FLIST_CREATE(ebufp);
				PIN_FLIST_FLD_SET(ser_iflistp, PIN_FLD_POID, ser_pdp, ebufp);
				PIN_FLIST_FLD_SET(ser_iflistp, PIN_FLD_ACCOUNT_OBJ, c_acc_obj, ebufp);

				//Call fm_get_service_info with acc_obj and subscription poid
				PIN_ERR_LOG_FLIST(3, "call fm_get_service_info input flist:", ser_iflistp);
				fm_get_service_info(ctxp, ser_iflistp, &ser_oflistp, ebufp);
				if (PIN_ERRBUF_IS_ERR(ebufp))
               			{
                       			PIN_ERR_LOG_EBUF(1, "Calling fm_get_service_info error", ebufp);
                       			return;
               			}
				PIN_ERR_LOG_FLIST(3, "call fm_get_service_info output flist:", ser_oflistp);
				cnt = (int32)PIN_FLIST_ELEM_COUNT(ser_oflistp, PIN_FLD_RESULTS, ebufp);
				if (cnt < 1)
					PIN_ERR_LOG_MSG(3, "Child Accont Not Activated/don't have subscription");

				elem_id2 = 0;
               			cookie2 =  NULL;
               			while ((ser_res_flistp = PIN_FLIST_ELEM_GET_NEXT(ser_oflistp, PIN_FLD_RESULTS, &elem_id2, 1, &cookie2, ebufp)) != NULL)
				{
					login = PIN_FLIST_FLD_GET(ser_res_flistp, PIN_FLD_LOGIN, 0, ebufp);	
					if (mobile_no && login && strstr(mobile_no, login) )
					{
						PIN_ERR_LOG_MSG(3, "Given Mobile_No matched with the Account_No");
						// strcpy(r_mob_no, login);
						break;
					}
				}
			}
		}
		if (mobile_no && (!strcmp(mobile_no, "") == 0) && (!r_mob_no || (strcmp(mobile_no, r_mob_no) != 0)) && (!login || !strstr(mobile_no, login)))
                {
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Mobile number does not exist in BRM", ebufp);
			r_flistp = PIN_FLIST_CREATE(ebufp);
                        status = 1;
                        PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, r_flistp, PIN_FLD_POID, ebufp);
                        PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ACCOUNT_NO, acc_no, ebufp);
                        PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &status, ebufp);
                        PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, "Mobile number does not exist in BRM", ebufp);
                        PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "53084", ebufp);
                        goto CLEANUP;
                }
		r_status = PIN_FLIST_FLD_GET(rs_flistp, PIN_FLD_STATUS, 0, ebufp);
		sprintf(msg, "status: %d", *r_status);
		PIN_ERR_LOG_MSG(3, msg);
		if (r_status && *r_status == 10103)
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error! Given Account is Closed/Deactive", ebufp);
                        r_flistp = PIN_FLIST_CREATE(ebufp);
                        status = 1;
                        PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, r_flistp, PIN_FLD_POID, ebufp);
                        PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ACCOUNT_NO, acc_no, ebufp);
                        PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &status, ebufp);
			PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, "Account is Closed/Deactive", ebufp);
                        PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "53085", ebufp);
                        goto CLEANUP;
		}
	}
	else
	{
		PIN_ERR_LOG_MSG(3, "test3");
		status = 1;
		r_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, r_flistp, PIN_FLD_POID, ebufp);
               	PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ACCOUNT_NO, acc_no, ebufp);
              	PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "53086", ebufp);
               	PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, "Account_no not found", ebufp);
               	PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &status, ebufp);
		goto CLEANUP;
	}
        srch_iflistp = PIN_FLIST_CREATE(ebufp);
        PIN_FLIST_FLD_SET(srch_iflistp, PIN_FLD_POID, PIN_POID_CREATE(db, "/search", -1, ebufp), ebufp);
	PIN_FLIST_FLD_SET(srch_iflistp, PIN_FLD_STRING_ID, reason_id, ebufp);

	PIN_ERR_LOG_FLIST(3, "fm_check_reason_code input flist", srch_iflistp);
	fm_check_reason_code(ctxp, srch_iflistp, &srch_oflistp, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "fm_check_reason_code call search error", ebufp);
		return;
	}
	PIN_ERR_LOG_FLIST(3, "fm_check_reason_code output flist:", srch_oflistp);
	s_cnt = PIN_FLIST_FLD_GET(srch_oflistp, PIN_FLD_COUNT, 0, ebufp);
	cnt = *(int32 *)s_cnt;
	sprintf(msg, "s_cnt: %d, cnt: %d", *s_cnt, cnt);
	PIN_ERR_LOG_MSG(3, msg);
	if (cnt == 0)
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"fm_rbs_ar_adj_loader mandatory field missing", ebufp);
		status = 1;
		r_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_POID, pdp, ebufp);
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_REASON_ID, reason_id, ebufp);
                PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &status, ebufp);
                PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "53087", ebufp);
                PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, "Adjustment_Code or Reason_id does not exist in BRM", ebufp);
                goto CLEANUP;
	}

	d_iflistp = PIN_FLIST_CREATE(ebufp);
	amount = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_AMOUNT, 0, ebufp);
	PIN_ERR_LOG_MSG(3, type_str);
	if (type_str && strcmp(type_str, "C") == 0)
	{
		amount = pbo_decimal_multiply(amount, amtp, ebufp);
	}
	sprintf(msg, "amount: %s", pbo_decimal_to_str(amount, ebufp));
        PIN_ERR_LOG_MSG(3, msg);
        if ((pbo_decimal_is_null(amount, ebufp)) || (pbo_decimal_is_zero(amount, ebufp)))
        {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"fm_rbs_ar_adj_loader mandatory field missing", ebufp);
                PIN_ERRBUF_RESET(ebufp);
                status = 1;
                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, "The adjustment amount can't be zero");
                r_flistp = PIN_FLIST_CREATE(ebufp);
                PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_POID, pdp, ebufp);
                PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "53088", ebufp);
                PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR,
                                                "The adjustment amount can't be zero", ebufp);
                PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &status, ebufp);
                goto CLEANUP;
        }
	if (amount && strlen(pbo_decimal_to_str(amount, ebufp)) > 12)
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"Invalid adjustment amount length", ebufp);
                PIN_ERRBUF_RESET(ebufp);
                status = 1;
                r_flistp = PIN_FLIST_CREATE(ebufp);
                PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_POID, pdp, ebufp);
                PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &status, ebufp);
                PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "53098", ebufp);
                PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, "Invalid adjustment amount length", ebufp);
                goto CLEANUP;
	}
	PIN_FLIST_FLD_SET(d_iflistp, PIN_FLD_AMOUNT, amount, ebufp);
	
	if (PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_REASON_DOMAIN_ID, 1, ebufp) != NULL)
        {
                PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_REASON_DOMAIN_ID, d_iflistp, PIN_FLD_REASON_DOMAIN_ID, ebufp);
        }
	PIN_FLIST_FLD_SET(d_iflistp, PIN_FLD_DESCR, descr, ebufp);
	PIN_FLIST_FLD_SET(d_iflistp, PIN_FLD_REASON_ID, reason_id, ebufp);
	PIN_FLIST_FLD_SET(d_iflistp, PIN_FLD_EFFECTIVE_T, &post_t, ebufp);
	PIN_FLIST_FLD_SET(d_iflistp, PIN_FLD_AMOUNT, amount, ebufp);
	if (type_str && strcmp(type_str, "C") == 0){
		PIN_FLIST_FLD_SET(d_iflistp, PIN_FLD_PROGRAM_NAME, "CREDIT ADJUSTMENT", ebufp);
		PIN_ERR_LOG_MSG(3, "CREDIT ADJUSTMENT");
	}
	else {
		PIN_FLIST_FLD_SET(d_iflistp, PIN_FLD_PROGRAM_NAME, "DEDIT ADJUSTMENT", ebufp);
		PIN_ERR_LOG_MSG(3, "DEDIT ADJUSTMENT");
	}

	if (prog_name && strstr(prog_name, "Adjustment"))
	{
		PIN_ERR_LOG_MSG(3, prog_name);
        	if ((mode_str && strcmp(mode_str, "A") == 0) || ((!mode_str || (strcmp(mode_str, "") == 0)) && type_str && strcmp(type_str, "C") == 0))
        	{
        	        PIN_FLIST_FLD_SET(d_iflistp, PIN_FLD_POID, acc_obj, ebufp);
			PIN_FLIST_FLD_SET(d_iflistp, PIN_FLD_BAL_GRP_OBJ, bal_grp_pdp, ebufp);
			if (post_t && (post_t <= now_t))
			{
				PIN_ERR_LOG_FLIST(3, "RBS_OP_AR_ACC_ADJUSTMENT input flist", d_iflistp );
              			PCM_OP(ctxp, RBS_OP_AR_ACC_ADJUSTMENT, 0, d_iflistp, &s_oflistp, ebufp);
				PIN_ERR_LOG_FLIST(3, "RBS_OP_AR_ACC_ADJUSTMENT output flist", s_oflistp);
				cnt = PIN_FLIST_ELEM_COUNT(s_oflistp, PIN_FLD_RESULTS, ebufp);
                		if (PIN_ERRBUF_IS_ERR(ebufp) || cnt < 1)
                		{
                		        status = 1;
	                        	r_flistp = PIN_FLIST_CREATE(ebufp);
	                        	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, r_flistp, PIN_FLD_POID, ebufp);
	                        	PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ACCOUNT_NO, acc_no, ebufp);
	                        	PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &status, ebufp);
	                        	PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "53091", ebufp);
	                        	PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, "Error! Unable to process Account Adjustment", ebufp);
	                        	goto CLEANUP;
        	        	}
	                	o_descr = "ACCOUNT ADJUSTMENT SUCCESSFUL";
			}
        	}
		else if ((mode_str && strcmp(mode_str, "B") == 0) || ((!mode_str || (strcmp(mode_str, "") == 0))  && type_str && strcmp(type_str, "D") == 0))
        	{
			srch_iflistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, srch_iflistp, PIN_FLD_POID, ebufp);
			PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_ACCOUNT_NO, srch_iflistp, PIN_FLD_ACCOUNT_NO, ebufp);
			fm_get_bill_details(ctxp, srch_iflistp, &srch_oflistp, ebufp);
			cnt = PIN_FLIST_ELEM_COUNT(srch_oflistp, PIN_FLD_RESULTS, ebufp);
			if (PIN_ERRBUF_IS_ERR(ebufp))
			{
				PIN_ERR_LOG_EBUF(3, "Unable to process Bill Adjustment", ebufp);
				status = 1;
                        	r_flistp = PIN_FLIST_CREATE(ebufp);
                        	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, r_flistp, PIN_FLD_POID, ebufp);
                        	PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ACCOUNT_NO, acc_no, ebufp);
                        	PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &status, ebufp);
                        	PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "53091", ebufp);
                        	PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, "Unable to process Bill Adjustment", ebufp);
                        	goto CLEANUP;
			}
			else if (cnt < 1)
			{
				opcode = PCM_OP_ACT_USAGE;
				PIN_ERR_LOG_MSG(3, "No Bill due Amount for this account - continue with accuont Adjustment");
				PIN_FLIST_FLD_SET(d_iflistp, PIN_FLD_POID, acc_obj, ebufp);
				PIN_FLIST_FLD_SET(d_iflistp, PIN_FLD_BAL_GRP_OBJ, bal_grp_pdp, ebufp);
				
				//Call fm_search_billinfo_details with acc_obj
				rd_iflistp = PIN_FLIST_CREATE(ebufp);
				PIN_FLIST_FLD_SET(rd_iflistp, PIN_FLD_POID, pdp, ebufp);
				PIN_FLIST_FLD_SET(rd_iflistp, PIN_FLD_ACCOUNT_OBJ, acc_obj, ebufp);
				PIN_ERR_LOG_FLIST(3, "call fm_search_billinfo_details input flist:", rd_iflistp);
				fm_search_billinfo_details(ctxp, rd_iflistp, &rd_oflistp, ebufp);
				if (PIN_ERRBUF_IS_ERR(ebufp))
                                {
                                        PIN_ERR_LOG_EBUF(1, "Calling fm_search_billinfo_details error", ebufp);
                                        return;
                                }
				PIN_ERR_LOG_FLIST(3, "call fm_search_billinfo_details output flist:", rd_oflistp);
				ad_res_flistp = PIN_FLIST_ELEM_GET(rd_oflistp, PIN_FLD_RESULTS, 0, 1, ebufp);
				billinfo_pdp = PIN_FLIST_FLD_GET(ad_res_flistp, PIN_FLD_POID, 0, ebufp);
				pvt = pin_virtual_time((time_t *)NULL);
				
				ad_evt_pdp = PIN_POID_CREATE(db, "/event/billing/adjustment/bill", -1, ebufp);
				event_flistp = PIN_FLIST_SUBSTR_ADD(d_iflistp, PIN_FLD_EVENT, ebufp);
        			PIN_FLIST_FLD_PUT(event_flistp, PIN_FLD_POID, ad_evt_pdp, ebufp);
        			PIN_FLIST_FLD_SET(event_flistp, PIN_FLD_ACCOUNT_OBJ, acc_obj, ebufp);
			//	PIN_FLIST_FLD_SET(event_flistp, PIN_FLD_SESSION_OBJ, ses_obj, ebufp);
				PIN_FLIST_FLD_SET(event_flistp, PIN_FLD_BILLINFO_OBJ, billinfo_pdp, ebufp);
				PIN_FLIST_FLD_SET(event_flistp, PIN_FLD_NAME, "BILL ADJUSTMENT", ebufp);
				PIN_FLIST_FLD_COPY(d_iflistp, PIN_FLD_PROGRAM_NAME, event_flistp, PIN_FLD_PROGRAM_NAME, ebufp);
				PIN_FLIST_FLD_SET(event_flistp, PIN_FLD_SYS_DESCR, "Event for Bill Adjustment", ebufp);
				PIN_FLIST_FLD_SET(event_flistp, PIN_FLD_DESCR, descr, ebufp);
				PIN_FLIST_FLD_SET(event_flistp, PIN_FLD_START_T, &pvt, ebufp);
				PIN_FLIST_FLD_SET(event_flistp, PIN_FLD_END_T, &pvt, ebufp);
				PIN_FLIST_FLD_SET(event_flistp, PIN_FLD_EFFECTIVE_T, &pvt, ebufp);
				total_flistp = PIN_FLIST_ELEM_ADD(event_flistp, PIN_FLD_TOTAL, *(int32 *)currency, ebufp);
				PIN_FLIST_FLD_SET(total_flistp, PIN_FLD_AMOUNT, amount, ebufp);
				misc_flistp = PIN_FLIST_ELEM_ADD(event_flistp, PIN_FLD_EVENT_MISC_DETAILS, 0, ebufp);
				PIN_FLIST_FLD_SET(misc_flistp, PIN_FLD_REASON_ID, reason_id, ebufp);
				var = 100;
				PIN_FLIST_FLD_SET(misc_flistp, PIN_FLD_REASON_DOMAIN_ID, &var, ebufp);
				
				
				if (post_t && (post_t <= now_t))
				{
					PIN_ERR_LOG_FLIST(3, "call PCM_OP_ACT_USAGE input flist:", d_iflistp);
					PCM_OP(ctxp, PCM_OP_ACT_USAGE, 0, d_iflistp, &s_oflistp, ebufp);
					if (PIN_ERRBUF_IS_ERR(ebufp))
	                                {
                        	                PIN_ERR_LOG_EBUF(1, "Calling PCM_OP_ACT_USAGE error", ebufp);
                	                        return;
        	                        }
					PIN_ERR_LOG_FLIST(3, "call PCM_OP_ACT_USAGE output flist:", s_oflistp);
					o_descr = "BILL ADJUSTMENT SUCCESSFUL";
				}
			}
			else
			{
				opcode = RBS_OP_AR_BILL_ADJUSTMENT;
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_get_bill_details output flist:", srch_oflistp );
				rs_flistp = PIN_FLIST_ELEM_GET(srch_oflistp, PIN_FLD_RESULTS, 0, 1, ebufp);
				PIN_FLIST_FLD_COPY(rs_flistp, PIN_FLD_POID, d_iflistp, PIN_FLD_POID, ebufp);
				PIN_FLIST_FLD_COPY(rs_flistp, PIN_FLD_BILLINFO_OBJ, d_iflistp, PIN_FLD_BILLINFO_OBJ, ebufp);
				if (post_t && (post_t <= now_t))
                        	{
                			PIN_ERR_LOG_FLIST(3, "RBS_OP_AR_BILL_ADJUSTMENT input flist", d_iflistp );
               				PCM_OP(ctxp, RBS_OP_AR_BILL_ADJUSTMENT, 0, d_iflistp, &s_oflistp, ebufp);
					cnt = PIN_FLIST_ELEM_COUNT(s_oflistp, PIN_FLD_RESULTS, ebufp);
       					if (PIN_ERRBUF_IS_ERR(ebufp) || cnt < 1)
       					{
               					PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "ADJUSTMENT opcode calling error", ebufp);
               					status = 1;
               					r_flistp = PIN_FLIST_CREATE(ebufp);
               					PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, r_flistp, PIN_FLD_POID, ebufp);
               					PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ACCOUNT_NO, acc_no, ebufp);
               					PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &status, ebufp);
               					PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "53093", ebufp);
               					PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, "Error! Unable to process Bill Adjustment", ebufp);
               					goto CLEANUP;
       					}
					PIN_ERR_LOG_FLIST(3, "RBS_OP_AR_BILL_ADJUSTMENT output flist", s_oflistp);
					if (PIN_FLIST_FLD_GET(s_oflistp, PIN_FLD_DESCR, 1, ebufp))
					{
						o_descr = PIN_FLIST_FLD_GET(s_oflistp, PIN_FLD_DESCR, 0, ebufp);
					}
                			o_descr = "BILL ADJUSTMENT SUCCESSFUL";
				}
        		}
		}
	}
        else if (prog_name && (strstr(prog_name, "Delete")))
        {
		PIN_ERR_LOG_MSG(3, prog_name);
                srch_iflistp = PIN_FLIST_CREATE(ebufp);
                PIN_FLIST_FLD_SET(srch_iflistp, PIN_FLD_POID, acc_obj, ebufp);
		PIN_FLIST_FLD_SET(srch_iflistp, PIN_FLD_AMOUNT, amount, ebufp);

                PIN_ERR_LOG_FLIST(3, "fm_get_item_details input flist", srch_iflistp);
                fm_get_item_details(ctxp, srch_iflistp, &srch_oflistp, ebufp);
                if (PIN_ERRBUF_IS_ERR(ebufp))
                {
                        PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "fm_get_item_details calling error", ebufp);
                        return;
                }
                PIN_ERR_LOG_FLIST(3, "fm_get_item_details output flist", srch_oflistp);
		cnt = PIN_FLIST_ELEM_COUNT(srch_oflistp, PIN_FLD_RESULTS, ebufp);
		if (cnt < 1)
                {
                        PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "ADJUSTMENT Amount not found error", ebufp);
                        status = 1;
                        r_flistp = PIN_FLIST_CREATE(ebufp);
                        PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, r_flistp, PIN_FLD_POID, ebufp);
                        PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ACCOUNT_NO, acc_no, ebufp);
                        PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &status, ebufp);
                        PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "53099", ebufp);
                        PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, "Adjusment Transaction Not Found", ebufp);
                        goto CLEANUP;
                }
                res_flistp = PIN_FLIST_ELEM_GET(srch_oflistp, PIN_FLD_RESULTS, 0, 1, ebufp);
                PIN_FLIST_FLD_COPY(res_flistp, PIN_FLD_POID, d_iflistp, PIN_FLD_POID, ebufp);
                PIN_FLIST_FLD_SET(d_iflistp, PIN_FLD_STRING_ID, reason_id, ebufp);
		pbo_decimal_multiply_assign(amtp, amount, ebufp);
                PIN_FLIST_FLD_SET(d_iflistp, PIN_FLD_AMOUNT, amtp, ebufp);
		if (post_t && (post_t <= now_t))
		{
               		PIN_ERR_LOG_FLIST(3, "RBS_OP_AR_DELETE_ADJUSTMENT input flist", d_iflistp);
               		PCM_OP(ctxp, RBS_OP_AR_DELETE_ADJUSTMENT, 0, d_iflistp, &s_oflistp, ebufp);
			cnt = PIN_FLIST_ELEM_COUNT(s_oflistp, PIN_FLD_RESULTS, ebufp);
                	if (PIN_ERRBUF_IS_ERR(ebufp) || cnt < 1)
                	{
                        	PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "ADJUSTMENT opcode calling error", ebufp);
                        	status = 1;
                        	r_flistp = PIN_FLIST_CREATE(ebufp);
                        	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, r_flistp, PIN_FLD_POID, ebufp);
                        	PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ACCOUNT_NO, acc_no, ebufp);
                        	PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &status, ebufp);
                        	PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "53093", ebufp);
                        	PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, "Error! Unable to process Delete Adjustment", ebufp);
                        	goto CLEANUP;
                	}
			o_descr = "ADJUSTMENT DELETED SUCCESSFULLY";
		}
	}
	if (post_t && now_t && (post_t > now_t))
	{
		sched_flistp = PIN_FLIST_CREATE(ebufp);
                PIN_FLIST_FLD_SET(sched_flistp, PIN_FLD_POID, acc_obj, ebufp);
                PIN_FLIST_FLD_SET(sched_flistp, PIN_FLD_WHEN_T, &post_t, ebufp);
                PIN_FLIST_FLD_SET(sched_flistp, PIN_FLD_FLAGS, (void *)NULL, ebufp);
                PIN_FLIST_FLD_COPY(d_iflistp, PIN_FLD_PROGRAM_NAME, sched_flistp, PIN_FLD_PROGRAM_NAME, ebufp);
		if (prog_name && (strstr(prog_name, "Adjustment")))
		{
			if ((mode_str && strstr (mode_str, "B")) || ((!mode_str || (strcmp(mode_str, "") == 0))  && type_str && strcmp(type_str, "D") == 0))
			{
				PIN_FLIST_FLD_SET(sched_flistp, PIN_FLD_SYS_DESCR, "BILL ADJUSTMENT from schedule", ebufp);
				PIN_FLIST_FLD_SET(sched_flistp, PIN_FLD_ACTION_NAME, "BILL_ADJUSTMENT", ebufp);
			//	opcode = RBS_OP_AR_BILL_ADJUSTMENT;
			}
			else if ((mode_str && strstr (mode_str, "A")) || ((!mode_str || (strcmp(mode_str, "") == 0))  && type_str && strcmp(type_str, "C") == 0))
			{
                		PIN_FLIST_FLD_SET(sched_flistp, PIN_FLD_SYS_DESCR, "ACCOUNT ADJUSTMENT from schedule", ebufp);
                		PIN_FLIST_FLD_SET(sched_flistp, PIN_FLD_ACTION_NAME, "ACCOUNT_ADJUSTMENT", ebufp);
				opcode = RBS_OP_AR_ACC_ADJUSTMENT;
			}
		}
		else if (prog_name && (strstr(prog_name, "Delete")))
		{
			PIN_FLIST_FLD_SET(sched_flistp, PIN_FLD_SYS_DESCR, "ITEM ADJUSTMENT from schedule", ebufp);
                        PIN_FLIST_FLD_SET(sched_flistp, PIN_FLD_ACTION_NAME, "DELETE_ADJUSTMENT", ebufp);
                        opcode = RBS_OP_AR_DELETE_ADJUSTMENT;
		}
		PIN_FLIST_FLD_SET(sched_flistp, PIN_FLD_OPCODE, (void *)&opcode, ebufp);
                PIN_FLIST_TO_STR(d_iflistp, &char_buf, &len, ebufp);
                pin_bufp = (pin_buf_t *)calloc(1, sizeof(pin_buf_t));
                pin_bufp->data = (caddr_t)char_buf;
                pin_bufp->size = len + 1;
                pin_bufp->flag = 0;
                pin_bufp->offset = 0;
                pin_bufp->xbuf_file = NULL;
                PIN_FLIST_FLD_SET(sched_flistp, PIN_FLD_INPUT_FLIST, (void *)pin_bufp, ebufp);

                PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Schedule create input flist", sched_flistp);
                PCM_OP(ctxp, PCM_OP_ACT_SCHEDULE_CREATE, 0, sched_flistp, &sched_oflistp, ebufp);
		cnt = PIN_FLIST_ELEM_COUNT(sched_oflistp, PIN_FLD_RESULTS, ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp) || cnt < 1)
                {
                        PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Schedule opcode call failed.", ebufp);
                        PIN_ERRBUF_RESET(ebufp);
                        status = 1;
                        r_flistp = PIN_FLIST_CREATE(ebufp);
                        PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_POID, acc_obj, ebufp);
                        PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ACCOUNT_NO, acc_no, ebufp);
                        PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ACTION_NAME, action_name, ebufp);
                        PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &status, ebufp);
                        PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "53094", ebufp);
                        PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, "Unable to process Scheduled Adjustment", ebufp);
                        goto CLEANUP;
                }
                PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Schedule create return flist", sched_oflistp);
		o_descr = "Adjustment Schedule Successful";
	}
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "ADJUSTMENT opcode output flist", s_oflistp);
	status = 0;
        r_flistp = PIN_FLIST_CREATE(ebufp);
        PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, r_flistp, PIN_FLD_POID, ebufp);
        acc_no = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_ACCOUNT_NO, 1, ebufp);
	PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ACCOUNT_NO, acc_no, ebufp);
        PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &status, ebufp);
        PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_DESCR, o_descr, ebufp);

	*r_flistpp = PIN_FLIST_COPY(r_flistp, ebufp);
        PIN_ERR_LOG_FLIST(3, "adj output flist", *r_flistpp);	
CLEANUP:
	*r_flistpp = PIN_FLIST_COPY(r_flistp, ebufp);
        PIN_ERR_LOG_FLIST(3, "adj output flist", *r_flistpp);
	PIN_FLIST_DESTROY_EX(&srch_iflistp, NULL);
	PIN_FLIST_DESTROY_EX(&srch_oflistp, NULL);
	PIN_FLIST_DESTROY_EX(&sched_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&d_iflistp, NULL);
	PIN_FLIST_DESTROY_EX(&r_flistp, NULL);
	return;
}

void
fm_get_bill_details(
        pcm_context_t           *ctxp,
        pin_flist_t             *i_flistp,
        pin_flist_t             **rs_flistp,
        pin_errbuf_t            *ebufp)
{
        pin_flist_t             *s_iflistp = NULL;
        pin_flist_t             *s_oflistp = NULL;
        pin_flist_t             *args_flistp = NULL;
        poid_t                  *pdp = NULL;
        poid_t                  *s_pdp = NULL;
        int32                   flags = 256;
	pin_decimal_t		*due = 0;
        int64                   db = -1;
        char                    *templatep = "select x from /bill 1,/account 2 where 1.F1 = 2.F2 and 2.F3 = V3 and bill_t.due != 0 order by bill_t.created_t desc";



        PIN_ERR_LOG_FLIST(3, "fm_get_bill_details input flist: ", i_flistp);

        s_iflistp = PIN_FLIST_CREATE(ebufp);
        pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp);
        db = PIN_POID_GET_DB(pdp);
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

        PIN_ERR_LOG_FLIST(3, "SEARCH input flist: ", s_iflistp);
        PCM_OP(ctxp, PCM_OP_SEARCH, 0, s_iflistp, &s_oflistp, ebufp);
        if (PIN_ERRBUF_IS_ERR(ebufp))
        {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "fm_get_bill_details search error", ebufp);
                return;
        }
        *rs_flistp = PIN_FLIST_COPY(s_oflistp, ebufp);
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_get_bill_details: output flist: ", *rs_flistp);
        return;
}

void
fm_get_item_details(
        pcm_context_t          *ctxp,
        pin_flist_t            *i_flistp,
        pin_flist_t            **r_flistp,
        pin_errbuf_t           *ebufp)
{

        pin_flist_t             *in_flistp = NULL;
        pin_flist_t             *search_oflistp = NULL;
        pin_flist_t             *args1 = NULL;
        pin_flist_t             *results = NULL;
        poid_t                  *acc_pdp = NULL;
        int64                   db = -1;
        char                    *template2 = "select x from /item where F1 = V1 and F2 = V2 and F3 = V3 and F4 = V4 order by created_t desc";
        int32                   flags = 256;


        PIN_ERR_LOG_FLIST(3, "start fm_get_item_details input flist", i_flistp);

        acc_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp);
        db = PIN_POID_GET_DB(acc_pdp);

        in_flistp = PIN_FLIST_CREATE(ebufp);
        PIN_FLIST_FLD_SET(in_flistp, PIN_FLD_POID, PIN_POID_CREATE(db, "/search", -1, ebufp), ebufp);
        PIN_FLIST_FLD_SET(in_flistp, PIN_FLD_FLAGS, &flags, ebufp);
        PIN_FLIST_FLD_SET(in_flistp, PIN_FLD_TEMPLATE, template2, ebufp);

        args1 = PIN_FLIST_ELEM_ADD(in_flistp, PIN_FLD_ARGS, 1, ebufp);
        PIN_FLIST_FLD_SET(args1, PIN_FLD_POID, PIN_POID_CREATE(db, "/item/adjustment", -1, ebufp), ebufp);

        args1 = PIN_FLIST_ELEM_ADD(in_flistp, PIN_FLD_ARGS, 2, ebufp);
        PIN_FLIST_FLD_SET(args1, PIN_FLD_ACCOUNT_OBJ, acc_pdp, ebufp);

	args1 = PIN_FLIST_ELEM_ADD(in_flistp, PIN_FLD_ARGS, 3, ebufp);
	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_AMOUNT, args1, PIN_FLD_ITEM_TOTAL, ebufp);

	args1 = PIN_FLIST_ELEM_ADD(in_flistp, PIN_FLD_ARGS, 4, ebufp);
        PIN_FLIST_FLD_SET(args1, PIN_FLD_CLOSED_T, NULL, ebufp);

        results = PIN_FLIST_ELEM_ADD(in_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, ebufp);

        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "item search input flist:", in_flistp);
        PCM_OP(ctxp, PCM_OP_SEARCH, 0, in_flistp, &search_oflistp, ebufp);
        if (PIN_ERRBUF_IS_ERR(ebufp))
        {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "ERROR IN CALLING item PCM_OP_SEARCH", ebufp);
                PIN_ERRBUF_RESET(ebufp);
                return;
        }
        PIN_ERR_LOG_FLIST(3, "item search output flist:", search_oflistp);
        *r_flistp = PIN_FLIST_COPY(search_oflistp, ebufp);
        return;
}

void
fm_check_reason_code(
	pcm_context_t          *ctxp,
        pin_flist_t            *i_flistp,
        pin_flist_t            **r_flistp,
        pin_errbuf_t           *ebufp)
{
        pin_flist_t             *in_flistp = NULL;
        pin_flist_t             *search_oflistp = NULL;
        pin_flist_t             *args1 = NULL;
        pin_flist_t             *results = NULL;
	pin_flist_t		*ret_flistp = NULL;
        poid_t                  *pdp = NULL;
	int32			var = 0;
        int64                   db = -1;
        char                    *template3 = "select x from /strings where F1 = V1";
	char			msg[100];
        int32                   flags = 256;
	int32			*str_id = NULL;
	int32			res_cnt = 0;


        PIN_ERR_LOG_FLIST(3, "start fm_check_reason_code input flist", i_flistp);

        pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp);
	str_id = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_STRING_ID, 0, ebufp);
        db = PIN_POID_GET_DB(pdp);

        in_flistp = PIN_FLIST_CREATE(ebufp);
        PIN_FLIST_FLD_SET(in_flistp, PIN_FLD_POID, PIN_POID_CREATE(db, "/search", -1, ebufp), ebufp);
        PIN_FLIST_FLD_SET(in_flistp, PIN_FLD_FLAGS, &flags, ebufp);
        PIN_FLIST_FLD_SET(in_flistp, PIN_FLD_TEMPLATE, template3, ebufp);

        args1 = PIN_FLIST_ELEM_ADD(in_flistp, PIN_FLD_ARGS, 1, ebufp);
        PIN_FLIST_FLD_SET(args1, PIN_FLD_STRING_ID, str_id, ebufp);

        results = PIN_FLIST_ELEM_ADD(in_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, ebufp);

        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "strings reason_code search input flist:", in_flistp);
        PCM_OP(ctxp, PCM_OP_SEARCH, 0, in_flistp, &search_oflistp, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp))
        {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "ERROR IN CALLING strings PCM_OP_SEARCH", ebufp);
                PIN_ERRBUF_RESET(ebufp);
                return;
        }
	PIN_ERR_LOG_FLIST(3, "strings reason_code search output flist:", search_oflistp);
	
	res_cnt = (int32)PIN_FLIST_ELEM_COUNT(search_oflistp, PIN_FLD_RESULTS, ebufp);
	ret_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, pdp, ebufp);
	if (res_cnt > 0)
	{
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_COUNT, &res_cnt, ebufp);
		var = 0;
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &var, ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_REASON_ID, str_id, ebufp);
	}
	else
	{
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_COUNT, &res_cnt, ebufp);
                var = 1;
                PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &var, ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_REASON_ID, str_id, ebufp);
	}
	*r_flistp = PIN_FLIST_COPY(ret_flistp, ebufp);
	PIN_ERR_LOG_FLIST(3, "fm_check_reason_code output flist: ", *r_flistp);
        return;
}

void
fm_get_account_details(
        pcm_context_t          *ctxp,
        pin_flist_t            *i_flistp,
        pin_flist_t            **r_flistp,
        pin_errbuf_t           *ebufp)
{
	pin_flist_t             *in_flistp = NULL;
        pin_flist_t             *args1_flistp = NULL;
	pin_flist_t             *args2_flistp = NULL;
	pin_flist_t             *args3_flistp = NULL;
	pin_flist_t             *results_flistp = NULL;
	pin_flist_t		*phones_flistp = NULL;
	pin_flist_t		*search_oflistp = NULL;
	poid_t                  *vp = NULL;
	char                    *templatep = "select x from /account where F1 = V1";
	char			*account_no = NULL;
	char			status = 0;
	char			*mobile_no = NULL;
	int32			flags = 256;
	int64                   db = -1;
		

	if (PIN_ERRBUF_IS_ERR(ebufp))
                return;
        PIN_ERRBUF_CLEAR(ebufp);
	
	PIN_ERR_LOG_FLIST(3, "start fm_get_account_details input flist:", i_flistp);
	
	vp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0 ,ebufp);
        db = PIN_POID_GET_DB(vp);
		
	in_flistp = PIN_FLIST_CREATE(ebufp);
        PIN_FLIST_FLD_SET(in_flistp, PIN_FLD_POID, (poid_t *)vp, ebufp);
        PIN_FLIST_FLD_SET(in_flistp, PIN_FLD_FLAGS, &flags, ebufp);
        PIN_FLIST_FLD_SET(in_flistp, PIN_FLD_TEMPLATE, templatep, ebufp);
	
	args1_flistp = PIN_FLIST_ELEM_ADD(in_flistp, PIN_FLD_ARGS, 1, ebufp);
	if (PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_ACCOUNT_NO, 1, ebufp))
	{
		account_no = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_ACCOUNT_NO, 0, ebufp);
		PIN_FLIST_FLD_SET(args1_flistp, PIN_FLD_ACCOUNT_NO, account_no, ebufp);
	}
	
	results_flistp = PIN_FLIST_ELEM_ADD(in_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, ebufp);
	
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "search input flist:", in_flistp);
        PCM_OP(ctxp, PCM_OP_SEARCH, 0, in_flistp, &search_oflistp, ebufp);
        if (PIN_ERRBUF_IS_ERR(ebufp))
        {
        	PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "ERROR IN CALLING PCM_OP_SEARCH", ebufp);
        	PIN_ERRBUF_RESET(ebufp);
        	return;
        }
        PIN_ERR_LOG_FLIST(3, "fm_get_account_details search output flist:", search_oflistp);
        *r_flistp = PIN_FLIST_COPY(search_oflistp, ebufp);
        return;
}
