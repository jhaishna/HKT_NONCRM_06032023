#ifndef lint
static const char Sccs_id[] = "@(#)%Portal Version: fm_rbs_gl_delta_upload.c:BillingVelocityInt:3:2006-Sep-05 21:55:03 %";
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
#include "rbs_delta_upload.h"

#define SUCCESS 0
#define FAILURE 1
#define READWRITE 1
#define LOCAL_TRANS_OPEN_SUCCESS 0
/************
*DEFINE FUNCTIONS
*************/
EXPORT_OP void
op_rbs_gl_delta_upload(
    cm_nap_connection_t *connp,
    int32               opcode,
    int32               flags,
    pin_flist_t         *i_flistp,
    pin_flist_t         **r_flistpp,
    pin_errbuf_t        *ebufp);

void
fm_rbs_gl_delta_validation(
        pcm_context_t           *ctxp,
        pin_flist_t             *i_flistp,
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

/***************
*FUNCTION IMPLIMENTATIONS
***************/
void
op_rbs_gl_delta_upload(
        cm_nap_connection_t *connp,
        int32               opcode,
        int32               flags,
        pin_flist_t         *i_flistp,
        pin_flist_t         **r_flistpp,
        pin_errbuf_t        *ebufp)
{
	pcm_context_t           *ctxp = connp->dm_ctx;
        poid_t                  *a_pdp = NULL;
        pin_flist_t             *r_flistp = NULL;
	pin_flist_t		*rslt_flistp = NULL;
	pin_flist_t		*rest_flistp = NULL;
        pin_flist_t             *result_flistp = NULL;
	char			*descr = NULL;
	char			msg[100];
	char			*program_name = NULL;
	int32			*ret_status = NULL;
        int32                   status = 2;
	int32			elem_id = 0;
        int                     local = 1;
	pin_cookie_t		cookie = NULL;
        *r_flistpp = NULL;

        if (PIN_ERRBUF_IS_ERR(ebufp)) {
                return;
        }
        PIN_ERRBUF_CLEAR(ebufp);

        /***********************
        * Insanity Check
        **********************/
    if (opcode != RBS_OP_LOAD_GLID) {
        pin_set_err(ebufp, PIN_ERRLOC_FM,
            PIN_ERRCLASS_SYSTEM_DETERMINATE,
            PIN_ERR_BAD_OPCODE, 0, 0, opcode);
        PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
            "op_rbs_gl_delta_upload error",
            ebufp);
        return;
    }

	/***********************
        * Debug: Input flist
        ************************/
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                "op_rbs_gl_delta_upload input", i_flistp);
        a_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp);
	if (PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_PROGRAM_NAME, 1, ebufp) != NULL)
	{
		program_name = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_PROGRAM_NAME, 0, ebufp);
	}
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
                PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "DELTAFILE_UPLOAD Fail output flist", result_flistp);
                *r_flistpp=PIN_FLIST_COPY(result_flistp, ebufp);
                PIN_FLIST_DESTROY_EX(&result_flistp, NULL);
                goto CLEANUP;
        }
        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Transaction Opened");

	if (program_name && strcmp(program_name, "MTA_DELTAFILE_UPLOAD") == 0)
	{
		r_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_POID, a_pdp, ebufp);
		while ((rest_flistp = PIN_FLIST_ELEM_GET_NEXT(i_flistp, PIN_FLD_RESULTS, &elem_id, 1, &cookie, ebufp)) != NULL)
		{
        		/**********************
        		* Call the default implementation
        		************************/
    			fm_rbs_gl_delta_validation(ctxp, rest_flistp, &rslt_flistp, ebufp);
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "DELTAFILE_UPLOAD output flist", rslt_flistp);
			ret_status = PIN_FLIST_FLD_GET(rslt_flistp, PIN_FLD_STATUS, 0, ebufp);
			PIN_FLIST_ELEM_PUT(r_flistp, rslt_flistp, PIN_FLD_RESULTS, elem_id, ebufp);
			sprintf(msg, "%d", *ret_status);
			PIN_ERR_LOG_MSG(3, msg);
			if (PIN_ERRBUF_IS_ERR(ebufp) || (ret_status && *ret_status == 1))
        		{
				status = FAILURE;
                		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "op_rbs_gl_delta_upload error", ebufp);
        		        PIN_ERRBUF_CLEAR(ebufp);
        		}
		}
		if (status == 1)
		{
			goto CLEANUP;
		}
		else
		{
			status = SUCCESS;
		}
	}
	else
	{
		/**********************
		* Call the default implementation
		************************/
		fm_rbs_gl_delta_validation(ctxp, i_flistp, &r_flistp, ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "DELTAFILE_UPLOAD output flist", r_flistp);
		ret_status = PIN_FLIST_FLD_GET(r_flistp, PIN_FLD_STATUS, 0, ebufp);
		sprintf(msg, "%d", *ret_status);
		PIN_ERR_LOG_MSG(3, msg);
		if (PIN_ERRBUF_IS_ERR(ebufp) || (ret_status && *ret_status == 1))
        	{
        	        status = FAILURE;
        	        PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "op_rbs_gl_delta_upload error", ebufp);
        	        PIN_ERRBUF_CLEAR(ebufp);
        	        goto CLEANUP;
        	}
	}
	/*********************
         * Results.
         ********************/
	if ((ret_status && (*ret_status == 0)) || status == 0)
        {
                status = SUCCESS;
                PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "op_rbs_gl_delta_upload: output flist::", r_flistp);
                goto CLEANUP;
        }
CLEANUP:
	*r_flistpp =PIN_FLIST_COPY(r_flistp, ebufp);
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
void
fm_rbs_gl_delta_validation(
        pcm_context_t                           *ctxp,
        pin_flist_t                             *i_flistp,
        pin_flist_t                             **r_flistp,
        pin_errbuf_t                            *ebufp)
{
        pin_flist_t                             *in_flistp = NULL;
        pin_flist_t                             *args_flistp = NULL;
        pin_flist_t                             *args1_flistp = NULL;
        pin_flist_t                             *gl_seg_iflistp = NULL;
        pin_flist_t                             *rs_get_flistp = NULL;
        pin_flist_t                             *write_flistp = NULL;
        pin_flist_t                             *so_flistp = NULL;
        pin_flist_t                             *write_oflistp = NULL;
        pin_flist_t                             *return_flistp = NULL;
        pin_flist_t                             *result_array = NULL;
        pin_flist_t                             *gl_ids = NULL;
        pin_flist_t                             *gl_accts = NULL;
        pin_flist_t                             *rj_flistp = NULL;
        pin_flist_t                             *rj_oflistp = NULL;
        pin_flist_t                             *glid_flistp = NULL;
        pin_flist_t                             *writein_flistp = NULL;
        pin_flist_t                             *gl_acct_flistp = NULL;
        pin_flist_t                             *r_flistpp = NULL;
        pin_flist_t                             *ret_flistp = NULL;
        poid_t                                  *vp = NULL;
        poid_t                                  *glid_pdp1 = NULL;
	poid_t					*srch_poid = NULL;
        poid_t                                  *ret_poid = NULL;
        char                                    *gl_segment = NULL;
        char                                    *descr = NULL;
        char                                    *descr1 = NULL;
        char                                    gl_offset_accts[100];
        char                                    *gl_offset_acct1 = NULL;
        char                                    gl_ar_accts[100];
        char                                    *gl_ar_accts1 = NULL;
        char                                    *templatep = "select x from /config/gl_segment where F1 = V1";
        char                                    *templatep1 = "select x from /config/glid where F1 = V1";
        char                                    *gl_name = NULL;
        char                                    *bill_type = NULL;
        char                                    gl_id_seg[100];
        char                                    msg[100];
        char                                    msg1[100];
        char                                    glid1[100];
        char                                    glidstr[50];
        char                                    *Creditworksorder = NULL;
        char                                    *CreditCostCenterCode = NULL;
        char                                    *CreditGLaccountcode = NULL;
        char                                    *Debitworksorder = NULL;
        char                                    *DebitCostCenterCode = NULL;
        char                                    *DebitGLaccountcode = NULL;
        char                                    *attribute1 = NULL;
	int64					db = -1;
	int32					status = 0;
        char                                    *attribute = NULL;
        int32                                   *glid = NULL;
        char                                    *type = NULL;
        int32                                   *type1 = NULL;
        int32                                   *attr = NULL;
        int32                                   flags = 256;
        int32                                   status_failure = 1;
        int32                                   elem_id1 = 0;
        int32                                   elem_id2 = 0;
	int32					type2 = 0;
	char					*attribute2 = NULL;
	int32					type3 = 0;
	int32					attribute3 = 0;
        pin_cookie_t                            cookie1 = NULL;
        pin_cookie_t                            cookie2 = NULL;


        if (PIN_ERRBUF_IS_ERR(ebufp))
                return;
        PIN_ERRBUF_CLEAR(ebufp);

        PIN_ERR_LOG_FLIST(3, "start fm_gl_delta_validation input flist: ", i_flistp);
	srch_poid = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp);
	db = PIN_POID_GET_DB(srch_poid);
	vp = PIN_POID_CREATE(db, "/search", -1, ebufp);
        gl_segment = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_GL_SEGMENT, 0, ebufp);
	if (gl_segment && strlen(gl_segment) > 20)
        {
                PIN_ERRBUF_RESET(ebufp);
                return_flistp = PIN_FLIST_COPY(i_flistp, ebufp);
                PIN_FLIST_FLD_SET(return_flistp, PIN_FLD_STATUS, &status_failure, ebufp);
                PIN_FLIST_FLD_SET(return_flistp, PIN_FLD_ERROR_CODE, "51231", ebufp);
                PIN_FLIST_FLD_SET(return_flistp, PIN_FLD_ERROR_DESCR, "Invalid gl_segment", ebufp);
                goto CLEANUP;
        }
        glid = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_GL_ID, 0, ebufp);
	sprintf(glidstr, "%d", *glid);
        PIN_ERR_LOG_MSG(3, glidstr);
        if ((strlen(glidstr) < 6 ) || (strlen(glidstr) > 10))
        {
                PIN_ERRBUF_RESET(ebufp);
                return_flistp = PIN_FLIST_COPY(i_flistp, ebufp);
                PIN_FLIST_FLD_SET(return_flistp, PIN_FLD_STATUS, &status_failure, ebufp);
                PIN_FLIST_FLD_SET(return_flistp, PIN_FLD_ERROR_CODE, "51230", ebufp);
                PIN_FLIST_FLD_SET(return_flistp, PIN_FLD_ERROR_DESCR, "Invalid glid", ebufp);
                goto CLEANUP;
        }
	descr = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_DESCR, 0, ebufp);
	if (!descr ||  (strlen(descr) > 245))
	{
        
         	PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "pin_field_search error", ebufp);
         	PIN_ERRBUF_RESET(ebufp);
		return_flistp = PIN_FLIST_COPY(i_flistp,ebufp);
         	PIN_FLIST_FLD_SET(return_flistp, PIN_FLD_STATUS, &status_failure, ebufp);
         	PIN_FLIST_FLD_SET(return_flistp, PIN_FLD_ERROR_CODE, "51221", ebufp);
         	PIN_FLIST_FLD_SET(return_flistp, PIN_FLD_ERROR_DESCR, "Invalid description", ebufp);
         	goto CLEANUP;
         }
        type = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_STATUS_STR, 0, ebufp);
	PIN_ERR_LOG_MSG(3, type);
	if (strcmp(type, "unbilled") == 0)
	{
		type3 = 1;
	}
	else if (strcmp(type, "billed") == 0)
	{
		type3 = 2;
	}
	else if (strcmp(type, "billed_earned") == 0)
	{
		type3 = 32;
	}
	else if (strcmp(type, "billed_unearned") == 0)
	{
		type3 = 16;
	}
	else if (strcmp(type, "unbilled_earned") == 0)
	{
		type3 = 8;
	}
	else if (strcmp(type, "unbilled_unearned") == 0)
	{
		type3 = 4;
	}
	else if (strcmp(type, "prev_billed_earned") == 0)
	{
		 type3 = 64;
	}
	else
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "pin_field_search error", ebufp);
                PIN_ERRBUF_RESET(ebufp);
                return_flistp = PIN_FLIST_COPY(i_flistp,ebufp);
                PIN_FLIST_FLD_SET(return_flistp, PIN_FLD_STATUS, &status_failure, ebufp);
                PIN_FLIST_FLD_SET(return_flistp, PIN_FLD_ERROR_CODE, "51222", ebufp);
                PIN_FLIST_FLD_SET(return_flistp, PIN_FLD_ERROR_DESCR, "Invalid type", ebufp);
                goto CLEANUP;
	}
	sprintf(msg, "type3: %d", type3);
	PIN_ERR_LOG_MSG(3, msg);

        attribute = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_ACTION, 0, ebufp);
	PIN_ERR_LOG_MSG(3, attribute);
	if(!strstr(attribute, "net") || (!strstr(attribute, "disc")) || (!strstr(attribute, "tax")) || (!strstr(attribute, "gross")))
	{
	     PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "pin_field_search error", ebufp);
             PIN_ERRBUF_RESET(ebufp);
             return_flistp = PIN_FLIST_COPY(i_flistp,ebufp);
             PIN_FLIST_FLD_SET(return_flistp, PIN_FLD_STATUS, &status_failure, ebufp);
             PIN_FLIST_FLD_SET(return_flistp, PIN_FLD_ERROR_CODE, "51223", ebufp);
             PIN_FLIST_FLD_SET(return_flistp, PIN_FLD_ERROR_DESCR, "Invalid attribute", ebufp);
             goto CLEANUP;
	}

	Creditworksorder = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_ORDER_ID, 0, ebufp);
	PIN_ERR_LOG_MSG(3, Creditworksorder);
	if (Creditworksorder && (strlen(Creditworksorder) > 8))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "pin_field_search error", ebufp);
         	PIN_ERRBUF_RESET(ebufp);
		return_flistp = PIN_FLIST_COPY(i_flistp,ebufp);
         	PIN_FLIST_FLD_SET(return_flistp, PIN_FLD_STATUS, &status_failure, ebufp);
         	PIN_FLIST_FLD_SET(return_flistp, PIN_FLD_ERROR_CODE, "51224", ebufp);
         	PIN_FLIST_FLD_SET(return_flistp, PIN_FLD_ERROR_DESCR, "Invalid Work Order (Credit)", ebufp);
         	goto CLEANUP;
        }
	CreditCostCenterCode = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_CODE, 0, ebufp);
	PIN_ERR_LOG_MSG(3, CreditCostCenterCode);
	if (CreditCostCenterCode && (strlen(CreditCostCenterCode) > 3))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "pin_field_search error", ebufp);
         	PIN_ERRBUF_RESET(ebufp);
		return_flistp = PIN_FLIST_COPY(i_flistp,ebufp);
         	PIN_FLIST_FLD_SET(return_flistp, PIN_FLD_STATUS, &status_failure, ebufp);
         	PIN_FLIST_FLD_SET(return_flistp, PIN_FLD_ERROR_CODE, "51225", ebufp);
         	PIN_FLIST_FLD_SET(return_flistp, PIN_FLD_ERROR_DESCR, "Invalid Cost center code (Credit)", ebufp);
         	goto CLEANUP;
         }
	CreditGLaccountcode  = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_GLACCOUNT, 0, ebufp);
	PIN_ERR_LOG_MSG(3, CreditGLaccountcode);
	if (CreditGLaccountcode && (strlen(CreditGLaccountcode) > 6))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "pin_field_search error", ebufp);
         	PIN_ERRBUF_RESET(ebufp);
		return_flistp = PIN_FLIST_COPY(i_flistp,ebufp);
         	PIN_FLIST_FLD_SET(return_flistp, PIN_FLD_STATUS, &status_failure, ebufp);
         	PIN_FLIST_FLD_SET(return_flistp, PIN_FLD_ERROR_CODE, "51226", ebufp);
         	PIN_FLIST_FLD_SET(return_flistp, PIN_FLD_ERROR_DESCR, "Invalid GL Account code (Credit)", ebufp);
         	goto CLEANUP;
        }
	sprintf(gl_ar_accts, "%s%s%s", Creditworksorder, CreditCostCenterCode, CreditGLaccountcode);
        PIN_ERR_LOG_MSG(3, gl_ar_accts);

	Debitworksorder = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_STRING, 0, ebufp);
	PIN_ERR_LOG_MSG(3, Debitworksorder);
	if (Debitworksorder && (strlen(Debitworksorder) > 8))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "pin_field_search error", ebufp);
         	PIN_ERRBUF_RESET(ebufp);
		return_flistp = PIN_FLIST_COPY(i_flistp,ebufp);
         	PIN_FLIST_FLD_SET(return_flistp, PIN_FLD_STATUS, &status_failure, ebufp);
         	PIN_FLIST_FLD_SET(return_flistp, PIN_FLD_ERROR_CODE, "51227", ebufp);
         	PIN_FLIST_FLD_SET(return_flistp, PIN_FLD_ERROR_DESCR, "Invalid Work Order (Debit)", ebufp);
         	goto CLEANUP;
         }
	DebitCostCenterCode = PIN_FLIST_FLD_GET (i_flistp, PIN_FLD_CODE_STR, 0, ebufp);
	PIN_ERR_LOG_MSG(3, DebitCostCenterCode);
	if (DebitCostCenterCode && (strlen(DebitCostCenterCode) > 3))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "pin_field_search error", ebufp);
         	PIN_ERRBUF_RESET(ebufp);
		return_flistp = PIN_FLIST_COPY(i_flistp,ebufp);
         	PIN_FLIST_FLD_SET(return_flistp, PIN_FLD_STATUS, &status_failure, ebufp);
         	PIN_FLIST_FLD_SET(return_flistp, PIN_FLD_ERROR_CODE, "51228", ebufp);
         	PIN_FLIST_FLD_SET(return_flistp, PIN_FLD_ERROR_DESCR, "Invalid Cost center code (Debit)", ebufp);
         	goto CLEANUP;
         }
	DebitGLaccountcode = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_ACCOUNT_CODE, 0, ebufp);
	PIN_ERR_LOG_MSG(3, DebitGLaccountcode);
	if (DebitGLaccountcode && (strlen(DebitGLaccountcode) > 6))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "pin_field_search error", ebufp);
         	PIN_ERRBUF_RESET(ebufp);
		return_flistp = PIN_FLIST_COPY(i_flistp,ebufp);
         	PIN_FLIST_FLD_SET(return_flistp, PIN_FLD_STATUS, &status_failure, ebufp);
         	PIN_FLIST_FLD_SET(return_flistp, PIN_FLD_ERROR_CODE, "51229", ebufp);
         	PIN_FLIST_FLD_SET(return_flistp, PIN_FLD_ERROR_DESCR, "Invalid GL Account code (Debit)", ebufp);
         	goto CLEANUP;
        }
        sprintf(gl_offset_accts, "%s%s%s", Debitworksorder, DebitCostCenterCode, DebitGLaccountcode);
	PIN_ERR_LOG_MSG(3, gl_offset_accts);
	
        in_flistp = PIN_FLIST_CREATE(ebufp);
        PIN_FLIST_FLD_SET(in_flistp, PIN_FLD_POID, (poid_t *)vp, ebufp);
        PIN_FLIST_FLD_SET(in_flistp, PIN_FLD_FLAGS, &flags, ebufp);
        PIN_FLIST_FLD_SET(in_flistp, PIN_FLD_TEMPLATE, templatep, ebufp);

        args_flistp = PIN_FLIST_ELEM_ADD(in_flistp, PIN_FLD_ARGS, 1, ebufp);
        gl_seg_iflistp = PIN_FLIST_ELEM_ADD(args_flistp, PIN_FLD_GL_SEGMENTS, 0, ebufp);
        PIN_FLIST_FLD_SET(gl_seg_iflistp, PIN_FLD_SEGMENT_NAME, gl_segment, ebufp);

        args1_flistp = PIN_FLIST_ELEM_ADD(in_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, ebufp);
        PIN_FLIST_FLD_SET(args1_flistp, PIN_FLD_NAME, NULL, ebufp);
        PIN_FLIST_FLD_SET(args1_flistp, PIN_FLD_VALUE, NULL, ebufp);

        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"fm_gl_segment_search: search input flist", in_flistp);
        PCM_OP(ctxp, PCM_OP_SEARCH, PCM_OPFLG_ORDER_BY_REC_ID, in_flistp, &so_flistp, ebufp);
        if (PIN_ERRBUF_IS_ERR(ebufp) || (PIN_FLIST_ELEM_COUNT(so_flistp, PIN_FLD_RESULTS, ebufp) == 0))
        {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "ERROR! GL_SEGMENT Not Found", ebufp);
                return_flistp = PIN_FLIST_COPY(i_flistp, ebufp);
                PIN_FLIST_FLD_SET(return_flistp, PIN_FLD_STATUS, &status_failure, ebufp);
                PIN_FLIST_FLD_SET(return_flistp, PIN_FLD_ERROR_CODE, "51232", ebufp);
                PIN_FLIST_FLD_SET(return_flistp, PIN_FLD_ERROR_DESCR, "ERROR! GL_SEGMENT Not Found", ebufp);
                PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"fm_gl_segment_search error", return_flistp);
                PIN_ERRBUF_RESET(ebufp);
		goto CLEANUP;
        }
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"fm_gl_segment_search: search output flist", so_flistp);

        rs_get_flistp = PIN_FLIST_ELEM_GET(so_flistp, PIN_FLD_RESULTS, 0, 1, ebufp);
        gl_name = PIN_FLIST_FLD_GET(rs_get_flistp, PIN_FLD_NAME, 0, ebufp);
        glid_pdp1 = PIN_FLIST_FLD_GET(rs_get_flistp, PIN_FLD_VALUE, 0, ebufp);

        PIN_ERR_LOG_MSG(3, glid_pdp1);
        poid_t* glid_pdp = PIN_POID_FROM_STR(glid_pdp1, NULL, ebufp);
        PIN_ERR_LOG_POID(3, "value1 poid: ", glid_pdp);

        rj_flistp = PIN_FLIST_CREATE(ebufp);
        PIN_FLIST_FLD_SET(rj_flistp, PIN_FLD_POID, glid_pdp, ebufp);
        PCM_OP(ctxp, PCM_OP_READ_OBJ, PCM_OPFLG_ORDER_BY_REC_ID, rj_flistp, &rj_oflistp, ebufp);
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Read object output flist: ", rj_oflistp);

        elem_id1 = 0;
        cookie1 = NULL;
        while ((glid_flistp = PIN_FLIST_ELEM_GET_NEXT(rj_oflistp, PIN_FLD_GL_IDS, &elem_id1, 1, &cookie1, ebufp )) != NULL)
        {
                sprintf(glid1, "%d", (int32)elem_id1);
                sprintf(msg, "%d", *glid);
                sprintf(msg1, "glid1: %s, msg: %s", glid1, msg);
                PIN_ERR_LOG_MSG(3, msg1);
                if (glid && (strcmp(glid1, msg) == 0))
                {
                        PIN_ERR_LOG_MSG(3, "inside glids");
                        descr1 = PIN_FLIST_FLD_GET(glid_flistp, PIN_FLD_DESCR, 0, ebufp);
                        elem_id2 = 0;
                        cookie2 = NULL;
                        while ((gl_acct_flistp = PIN_FLIST_ELEM_GET_NEXT(glid_flistp, PIN_FLD_GL_ACCTS, &elem_id2, 1, &cookie2, ebufp )) != NULL)
                        {
                                PIN_ERR_LOG_FLIST(3, "inside gl_accts_flist: ", gl_acct_flistp);
                                attr = PIN_FLIST_FLD_GET(gl_acct_flistp, PIN_FLD_ATTRIBUTE, 0, ebufp);
				if ( attr && *attr != 0)
    				{
					sprintf(msg, "attr: %d", *attr);
					PIN_ERR_LOG_MSG(3, msg);
					if (attr && *attr == 1)
                                	{
                                        	attribute2 = (char *) malloc(strlen("net")+1);
                                        	strcpy(attribute2,"net");
                                	}
                                	else if(attr && *attr == 2)
                                	{
                                        	attribute2 = (char *) malloc(strlen("disc")+1);
                                        	strcpy(attribute2,"disc");
                                	}
                                	else if(attr && *attr == 4)
                                	{
                                	        attribute2 = (char *) malloc(strlen("tax")+1);
                                        	strcpy(attribute2,"tax");
                                	}
                                	else
                                	{
                                        	attribute2 = (char *) malloc(strlen("gross")+1);
                                        	strcpy(attribute2,"gross");
                                	}
					PIN_ERR_LOG_MSG(3, attribute2);
					PIN_ERR_LOG_MSG(3, attribute);
    				}
                                gl_ar_accts1 = PIN_FLIST_FLD_GET(gl_acct_flistp, PIN_FLD_GL_AR_ACCT, 0, ebufp);
                                gl_offset_acct1 = PIN_FLIST_FLD_GET(gl_acct_flistp, PIN_FLD_GL_OFFSET_ACCT, 0, ebufp);
                                type1 = PIN_FLIST_FLD_GET(gl_acct_flistp, PIN_FLD_TYPE, 0, ebufp);
				if (type1 && *type1 != 0)
				{
					sprintf(msg, "type1: %d", *type1);
					PIN_ERR_LOG_MSG(3, msg);
					type2 = *type1;
					sprintf(msg, "type2: %d", type2);
					PIN_ERR_LOG_MSG(3, msg);
				}
                                PIN_ERR_LOG_MSG(3, "inside gl_accts fields");

                                sprintf(msg1, "gl_ar_accts1: %s, gl_offset_acct1: %s, type2: %d, descr1: %s", gl_ar_accts1, gl_offset_acct1, type2, descr1);
                                PIN_ERR_LOG_MSG(3, msg1);
                                sprintf(msg1, "gl_ar_accts : %s, gl_offset_accts : %s, type3 : %d, descr: %s", gl_ar_accts, gl_offset_accts, type3, descr);
                                PIN_ERR_LOG_MSG(3, msg1);

                                if (strstr(attribute, attribute2) && type2 == type3 && gl_ar_accts1 && gl_ar_accts
                                                && (strcmp(gl_ar_accts1, gl_ar_accts) == 0) && gl_offset_acct1 && gl_offset_accts
                                                && (strcmp(gl_offset_acct1, gl_offset_accts) == 0)
                                                        &&(strcmp(descr,descr1) == 0))
                                {
					return_flistp = PIN_FLIST_COPY(i_flistp, ebufp);
					PIN_FLIST_FLD_SET(return_flistp, PIN_FLD_STATUS, &status, ebufp);
                                        PIN_FLIST_FLD_SET(return_flistp, PIN_FLD_ERROR_DESCR, "existing record", ebufp);
                                        PIN_ERR_LOG_FLIST(3, "existing record return flist: ", return_flistp);
					goto CLEANUP;
                                }
                        }
                }
       }
       if (return_flistp == NULL)
       {
        	write_flistp = PIN_FLIST_CREATE(ebufp);
        	PIN_FLIST_FLD_SET(write_flistp, PIN_FLD_POID, glid_pdp, ebufp);
       		PIN_FLIST_FLD_SET(write_flistp, PIN_FLD_NAME, "new glid object", ebufp);
        	PIN_FLIST_FLD_SET(write_flistp, PIN_FLD_PROGRAM_NAME, "load_pin_glid", ebufp);
        	gl_ids = PIN_FLIST_ELEM_ADD(write_flistp, PIN_FLD_GL_IDS, *(int32 *)glid, ebufp);
                PIN_FLIST_FLD_SET(gl_ids, PIN_FLD_DESCR, descr, ebufp);
        	PIN_FLIST_FLD_SET(gl_ids, PIN_FLD_TYPE, &type3, ebufp);

		if (strstr(attribute, "net"))
  		{
          		attribute3 = 1;
			gl_accts = PIN_FLIST_ELEM_ADD(gl_ids, PIN_FLD_GL_ACCTS, PCM_RECID_ASSIGN, ebufp);
			PIN_FLIST_FLD_SET(gl_accts, PIN_FLD_ATTRIBUTE, &attribute3, ebufp);
			PIN_FLIST_FLD_SET(gl_accts, PIN_FLD_TYPE, &type3, ebufp);
            		PIN_FLIST_FLD_SET(gl_accts, PIN_FLD_GL_AR_ACCT, gl_ar_accts, ebufp);
              		PIN_FLIST_FLD_SET(gl_accts, PIN_FLD_GL_OFFSET_ACCT, gl_offset_accts, ebufp);
  		}
  		if (strstr(attribute, "disc"))
  		{
          		attribute3 = 2;
			gl_accts = PIN_FLIST_ELEM_ADD(gl_ids, PIN_FLD_GL_ACCTS, PCM_RECID_ASSIGN, ebufp);
                        PIN_FLIST_FLD_SET(gl_accts, PIN_FLD_ATTRIBUTE, &attribute3, ebufp);
			PIN_FLIST_FLD_SET(gl_accts, PIN_FLD_TYPE, &type3, ebufp);
                  	PIN_FLIST_FLD_SET(gl_accts, PIN_FLD_GL_AR_ACCT, gl_ar_accts, ebufp);
                 	PIN_FLIST_FLD_SET(gl_accts, PIN_FLD_GL_OFFSET_ACCT, gl_offset_accts, ebufp);
  		}
  		if (strstr(attribute, "tax"))
  		{
         		attribute3 = 4;
			gl_accts = PIN_FLIST_ELEM_ADD(gl_ids, PIN_FLD_GL_ACCTS, PCM_RECID_ASSIGN, ebufp);
              		PIN_FLIST_FLD_SET(gl_accts, PIN_FLD_ATTRIBUTE, &attribute3, ebufp);
			PIN_FLIST_FLD_SET(gl_accts, PIN_FLD_TYPE, &type3, ebufp);
                	PIN_FLIST_FLD_SET(gl_accts, PIN_FLD_GL_AR_ACCT, gl_ar_accts, ebufp);
                  	PIN_FLIST_FLD_SET(gl_accts, PIN_FLD_GL_OFFSET_ACCT, gl_offset_accts, ebufp);
  		}
 		if (strstr(attribute, "gross"))
 		{
         		attribute3 = 8;
			gl_accts = PIN_FLIST_ELEM_ADD(gl_ids, PIN_FLD_GL_ACCTS, PCM_RECID_ASSIGN, ebufp);  
	                PIN_FLIST_FLD_SET(gl_accts, PIN_FLD_ATTRIBUTE, &attribute3, ebufp);
			PIN_FLIST_FLD_SET(gl_accts, PIN_FLD_TYPE, &type3, ebufp);
               		PIN_FLIST_FLD_SET(gl_accts, PIN_FLD_GL_AR_ACCT, gl_ar_accts, ebufp);
                  	PIN_FLIST_FLD_SET(gl_accts, PIN_FLD_GL_OFFSET_ACCT, gl_offset_accts, ebufp);
 		}
        	PCM_OP(ctxp, PCM_OP_WRITE_FLDS, PCM_OPFLG_ADD_ENTRY, write_flistp, &write_oflistp, ebufp);
        	if (PIN_ERRBUF_IS_ERR(ebufp))
        	{
                	PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "ERROR! write fields", ebufp);
                	PIN_ERRBUF_RESET(ebufp);
			return_flistp = PIN_FLIST_COPY(i_flistp, ebufp);
			PIN_FLIST_FLD_SET(return_flistp, PIN_FLD_STATUS, &status_failure, ebufp);
                        PIN_FLIST_FLD_SET(return_flistp, PIN_FLD_ERROR_CODE, "51226", ebufp);
                        PIN_FLIST_FLD_SET(return_flistp, PIN_FLD_ERROR_DESCR, "ERROR! write fields", ebufp);
                	PIN_FLIST_DESTROY_EX(&write_oflistp, NULL);
			goto CLEANUP;
        	}
        	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"write fields output flist", write_oflistp);
		return_flistp = PIN_FLIST_COPY(i_flistp,ebufp);
		PIN_FLIST_FLD_COPY(write_oflistp, PIN_FLD_POID, return_flistp, PIN_FLD_POID, ebufp);
		PIN_FLIST_FLD_SET(return_flistp, PIN_FLD_STATUS, &status, ebufp);
		PIN_FLIST_FLD_SET(return_flistp, PIN_FLD_ERROR_DESCR, "Inserted/Updated Successfully", ebufp);
        }
CLEANUP:
	
	*r_flistp = PIN_FLIST_COPY(return_flistp, ebufp);
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"output flist", *r_flistp);
        PIN_FLIST_DESTROY_EX(&so_flistp, ebufp);
        PIN_FLIST_DESTROY_EX(&write_oflistp, NULL);
        PIN_FLIST_DESTROY_EX(&return_flistp, NULL);
        return;

}
