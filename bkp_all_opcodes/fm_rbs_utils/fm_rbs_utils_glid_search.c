#ifndef lint
static const char Sccs_id[] = "@(#)%Portal Version: fm_rbs_utils_glid_search.c:BillingVelocityInt:3:2006-Sep-05 21:55:03 %";
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
#include "custom_flds/custom_flds.h"
#include "str_adj_code_search.h"

#define FILE_LOGNAME "fm_rbs_utils_glid_search.c(1.5)"
/************
*DEFINE FUNCTIONS
*************/


EXPORT_OP void
op_rbs_utils_glid_search(
    cm_nap_connection_t *connp,
    int32               opcode,
    int32               flags,
    pin_flist_t         *i_flistp,
    pin_flist_t         **r_flistpp,
    pin_errbuf_t        *ebufp);

static void
fm_rbs_utils_glid_search(
        pcm_context_t           *ctxp,
        pin_flist_t             *i_flistp,
        pin_flist_t             **r_flistp,
        pin_errbuf_t            *ebufp);

static void
fm_rbs_glid_srch(
        pcm_context_t           *ctxp,
        pin_flist_t             *i_flistp,
        pin_flist_t             **r_flistp,
        pin_errbuf_t            *ebufp);

static void
fm_rbs_descr_srch(
        pcm_context_t           *ctxp,
        pin_flist_t             *i_flistp,
        pin_flist_t             **r_flistp,
        pin_errbuf_t            *ebufp);

static void
fm_rbs_gl_segment_srch(
        pcm_context_t           *ctxp,
        pin_flist_t             *i_flistp,
        pin_flist_t             **r_flistp,
        pin_errbuf_t            *ebufp);

static void
fm_rbs_history_srch(
        pcm_context_t           *ctxp,
        pin_flist_t             *i_flistp,
        pin_flist_t             **r_flistp,
        pin_errbuf_t            *ebufp);

static void
fm_rbs_prefix_srch(
        pcm_context_t           *ctxp,
        pin_flist_t             *rslt_flistp,
        pin_flist_t             **rs_flistp,
        pin_errbuf_t            *ebufp);

static void
fm_rbs_gl_seg_for_actions(
        pcm_context_t           *ctxp,
        pin_flist_t             *i_flistp,
        pin_flist_t             **r_flistp,
        pin_errbuf_t            *ebufp);

/***************
*FUNCTION IMPLIMENTATIONS
***************/
void
op_rbs_utils_glid_search(
    cm_nap_connection_t *connp,
    int32               opcode,
    int32               flags,
    pin_flist_t         *i_flistp,
    pin_flist_t         **r_flistpp,
    pin_errbuf_t        *ebufp)
   {
 pcm_context_t       *ctxp = connp->dm_ctx;

    pin_flist_t         *r_flistp= NULL;
    *r_flistpp = NULL;
    if (PIN_ERRBUF_IS_ERR(ebufp)) {
        return;
    }
    PIN_ERRBUF_CLEAR(ebufp);

 /***********************
 * Insanity Check
 * ***********************/
    if (opcode != RBS_OP_GLID_SEARCH) {
        pin_set_err(ebufp, PIN_ERRLOC_FM,
            PIN_ERRCLASS_SYSTEM_DETERMINATE,
            PIN_ERR_BAD_OPCODE, 0, 0, opcode);
        PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
            "op_rbs_utils_glid_search error",
            ebufp);
        return;
    }

 /***********************
 * Debug: Input flist
 ************************/
    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
     "op_rbs_utils_glid_search input", i_flistp);

 /**********************
 * Call the default implementation
 ************************/

    fm_rbs_utils_glid_search(ctxp, i_flistp, &r_flistp, ebufp);


/*********************
 * Results.
 ********************/

    if (PIN_ERRBUF_IS_ERR(ebufp))
    {
        PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "op_rbs_utils_glid_search error", ebufp);
    }
    else
    {
       *r_flistpp = PIN_FLIST_COPY(r_flistp, ebufp);
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "op_rbs_utils_glid_search: output flist::", *r_flistpp);
    }
    return;
}

static void
fm_rbs_utils_glid_search(
        pcm_context_t           *ctxp,
        pin_flist_t             *i_flistp,
        pin_flist_t             **r_flistp,
        pin_errbuf_t            *ebufp)
{
        pin_flist_t             *ret_flistp = NULL;
        pin_flist_t             *ret_flistpp = NULL;
        pin_flist_t             *glid_accts = NULL;
        pin_flist_t             *reslts_flistp = NULL;
        pin_flist_t             *res_flistp = NULL;
        pin_flist_t             *return1_flistp = NULL;
        pin_flist_t             *ret_glid_flistp = NULL;
        pin_flist_t             *ret_glid_accts_flistp = NULL;
        pin_flist_t             *glid_flistp = NULL;
        pin_flist_t             *glid_reslts_flistp = NULL;
        pin_flist_t             *glaccts_flistp = NULL;
        pin_flist_t             *seg_iflistp = NULL;
        pin_flist_t             *seg_rflistp = NULL;
        pin_flist_t             *glaccts_reslts_flistp = NULL;
        char                    msg[100];
        char                    *descr = NULL;
        char                    *attribute = NULL;
        char                    *gl_offset_acct = NULL;
        char                    glid[100];
        char                    Debitworksorder[9];
        char                    DebitCostCenterCode[4];
        char                    DebitGLaccountc[7];
        char                    *gl_ar_acct = NULL;
        char                    Creditworksorder[9];
        char                    CreditCostCenterCode[4];
        char                    CreditGLaccountcode[7];
        char                    elem_glid[100];
        char                    gl_segment[100];
        char                    *gl_seg = NULL;
        char                    *bill_type = NULL;
        char                    *action_flags = NULL;
        char                    *segment = NULL;
        int32                   search_fail = 1;
        int64                   db = -1;
        int32                   *glid1 = NULL;
        int32                   *attr = NULL;
        int32                   *type = NULL;
        int32                   cnt = 0;
        int32                   cnt1 = 0;
        int32                   elem_id = 0;
        int32                   elem_id1 = 0;
        int32                   elem_id2 = 0;
        int32                   elem_cnt = 0;
        int32                   elem_cnt1 = 0;
        int32                   elem_cnt2 = 0;
        int32                   rec_id = 0;
        int32                   mode = 0;
	int32			rec_cnt = 0;
        pin_cookie_t            cookie4 = NULL;
        pin_cookie_t            cookie2 = NULL;
        pin_cookie_t            cookie = NULL;
        pin_cookie_t            cookie1 = NULL;
        poid_t                  *vp = NULL;
        poid_t                  *seg_pdp = NULL;

        if (PIN_ERRBUF_IS_ERR(ebufp))
                return;
        PIN_ERRBUF_CLEAR(ebufp);

        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Search GLID input flist:", i_flistp);

        action_flags = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_ACTION_MODE, 1, ebufp );
        if (!action_flags && action_flags == NULL)
        {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "PIN_ERR_MISSING_ARG Mandatory field is missing in input", ebufp);
                return;
        }
        vp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp );
        if (!vp && vp == NULL)
        {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "PIN_ERR_MISSING_ARG Mandatory field is missing in input", ebufp);
                return;
        }
        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "TEST001 --> BEFORE IF");
        /*******************************************************************
        * Mandatory fields validation
        *******************************************************************/
        if (action_flags && (*action_flags < 1 || *action_flags > 5))
        {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"Search: Invalid Flag", ebufp);
                PIN_ERRBUF_RESET(ebufp);
                *r_flistp = PIN_FLIST_CREATE(ebufp);
                PIN_FLIST_FLD_SET(*r_flistp, PIN_FLD_POID, vp, ebufp );
                PIN_FLIST_FLD_SET(*r_flistp, PIN_FLD_STATUS, &search_fail, ebufp);
                PIN_FLIST_FLD_SET(*r_flistp, PIN_FLD_ERROR_CODE, "51220", ebufp);
                PIN_FLIST_FLD_SET(*r_flistp, PIN_FLD_ERROR_DESCR, "Search: Invalid Flag", ebufp);
                return;
        }
        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "TEST001 --> After IF");

        mode = *action_flags;
        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "TEST001 --> After assign ");
        switch(mode)
        {
                case 1 :
                        fm_rbs_glid_srch(ctxp, i_flistp, &ret_flistp, ebufp);
                        break;
                case 2 :
                        fm_rbs_descr_srch(ctxp, i_flistp, &ret_flistp, ebufp);
                        break;
                case 3 :
                        fm_rbs_gl_segment_srch(ctxp, i_flistp, &ret_flistp, ebufp);
                        break;
                case 4 :
                        fm_rbs_history_srch(ctxp, i_flistp, &ret_flistp, ebufp);
                        break;
                case 5 :
                        fm_rbs_prefix_srch(ctxp, i_flistp, &ret_flistp, ebufp);
                        break;
                default :
                        break;
        }

        //Set error for return
        if (PIN_ERRBUF_IS_ERR(ebufp) || (!ret_flistp))
        {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"Error in calling rbs_op_search", ebufp);
                PIN_ERRBUF_RESET(ebufp);
                *r_flistp = PIN_FLIST_CREATE(ebufp);
                PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, *r_flistp, PIN_FLD_POID, ebufp);
                PIN_FLIST_FLD_SET(*r_flistp, PIN_FLD_STATUS, &search_fail, ebufp);
                PIN_FLIST_FLD_SET(*r_flistp, PIN_FLD_ERROR_CODE, "51221", ebufp);
                PIN_FLIST_FLD_SET(*r_flistp, PIN_FLD_ERROR_DESCR, "Error in calling pcm_op_search", ebufp);
                return;
        }
        if (PIN_FLIST_ELEM_COUNT(ret_flistp, PIN_FLD_RESULTS, ebufp) < 1)
        {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"Error in calling rbs_op_search", ebufp);
                PIN_ERRBUF_RESET(ebufp);
                *r_flistp = PIN_FLIST_COPY(ret_flistp, ebufp);
                return;
        }
        ret_flistpp = PIN_FLIST_CREATE(ebufp);
        PIN_FLIST_FLD_COPY(ret_flistp, PIN_FLD_POID, ret_flistpp, PIN_FLD_POID, ebufp);
        elem_id = 0;
        cookie = NULL;
        while((return1_flistp = PIN_FLIST_ELEM_GET_NEXT(ret_flistp, PIN_FLD_RESULTS, &elem_id, 1, &cookie, ebufp)) != (pin_flist_t *)NULL)
        {
                elem_id1 = 0;
                cookie1 = NULL;
                while((ret_glid_flistp = PIN_FLIST_ELEM_GET_NEXT(return1_flistp, PIN_FLD_GL_IDS, &elem_id1, 1, &cookie1, ebufp)) != (pin_flist_t *)NULL)
                {
                        elem_id2 = 0;
                        cookie2 = NULL;
                        while((ret_glid_accts_flistp = PIN_FLIST_ELEM_GET_NEXT(ret_glid_flistp, PIN_FLD_GL_ACCTS, &elem_id2, 1, &cookie2, ebufp)) != NULL)
                        {
                                reslts_flistp = PIN_FLIST_ELEM_ADD(ret_flistpp, PIN_FLD_RESULTS, elem_cnt2, ebufp);
                                seg_iflistp = PIN_FLIST_CREATE(ebufp);
                                if (action_flags && *action_flags != 4)
                                {
                                        PIN_FLIST_FLD_COPY(return1_flistp, PIN_FLD_POID, seg_iflistp, PIN_FLD_POID, ebufp);
                                        PIN_FLIST_FLD_COPY(return1_flistp, PIN_FLD_POID, reslts_flistp, PIN_FLD_POID, ebufp);
                                }
                                else
                                {
                                        PIN_FLIST_FLD_COPY(return1_flistp, PIN_FLD_AU_PARENT_OBJ, seg_iflistp, PIN_FLD_AU_PARENT_OBJ, ebufp);
                                        PIN_FLIST_FLD_COPY(return1_flistp, PIN_FLD_AU_PARENT_OBJ, reslts_flistp, PIN_FLD_POID, ebufp);
                                }
                                PIN_FLIST_FLD_SET(seg_iflistp, PIN_FLD_ACTION_MODE, action_flags, ebufp);
                                fm_rbs_gl_seg_for_actions(ctxp, seg_iflistp, &seg_rflistp, ebufp);
                                PIN_ERR_LOG_FLIST(3, "fm_rbs_gl_seg_for_actions output: ", seg_rflistp);
                                PIN_FLIST_FLD_COPY(seg_rflistp, PIN_FLD_SEGMENT_NAME, reslts_flistp, PIN_FLD_GL_SEGMENT, ebufp);

                                PIN_FLIST_FLD_COPY(return1_flistp, PIN_FLD_CREATED_T, reslts_flistp, PIN_FLD_CREATED_T, ebufp);
                                PIN_FLIST_FLD_COPY(return1_flistp, PIN_FLD_MOD_T, reslts_flistp, PIN_FLD_MOD_T, ebufp);
                                PIN_FLIST_FLD_SET(reslts_flistp, PIN_FLD_GL_ID, &elem_id1, ebufp);
                                PIN_FLIST_FLD_COPY(ret_glid_flistp, PIN_FLD_DESCR, reslts_flistp, PIN_FLD_DESCR, ebufp);
                                attr = PIN_FLIST_FLD_GET(ret_glid_accts_flistp, PIN_FLD_ATTRIBUTE, 0, ebufp);
                                gl_ar_acct = (char *)PIN_FLIST_FLD_GET(ret_glid_accts_flistp, PIN_FLD_GL_AR_ACCT, 0, ebufp);
                                gl_offset_acct =  (char *)PIN_FLIST_FLD_GET(ret_glid_accts_flistp, PIN_FLD_GL_OFFSET_ACCT, 0, ebufp);
                                type = PIN_FLIST_FLD_GET(ret_glid_accts_flistp, PIN_FLD_TYPE, 0, ebufp);

                                if (attr && *attr == 1)
                                {
                                        attribute = (char *) malloc(strlen("NET")+1);
                                        strcpy(attribute,"NET");
                                }
                                else if(attr && *attr == 2)
                                {
                                        attribute = (char *) malloc(strlen("DISC")+1);
                                        strcpy(attribute,"DISC");
                                }
                                else if(attr && *attr == 4)
                                {
                                        attribute = (char *) malloc(strlen("TAX")+1);
                                        strcpy(attribute,"TAX");
                                }
                                else
                                {
                                        attribute = (char *) malloc(strlen("GROSS")+1);
                                        strcpy(attribute,"GROSS");
                                }
                                PIN_FLIST_FLD_SET(reslts_flistp, PIN_FLD_ATTR_NAME, attribute, ebufp);

                                if (gl_ar_acct)
                                {
                                        sprintf(msg, "gl_ar_acct: %d", gl_ar_acct);
                                        PIN_ERR_LOG_MSG(3, msg);
                                        strncpy(Creditworksorder, &gl_ar_acct[0], 8);
                                        Creditworksorder[8] = '\0';
                                        PIN_ERR_LOG_MSG(3, Creditworksorder);
                                        PIN_FLIST_FLD_SET(reslts_flistp, PIN_FLD_ORDER_ID, &Creditworksorder, ebufp);

                                        strncpy(CreditCostCenterCode, &gl_ar_acct[8], 3);
                                        CreditCostCenterCode[3] = '\0';
                                        PIN_ERR_LOG_MSG(3, CreditCostCenterCode);
                                        PIN_FLIST_FLD_SET(reslts_flistp, PIN_FLD_CODE, &CreditCostCenterCode, ebufp);

                                        strncpy(CreditGLaccountcode, &gl_ar_acct[11], 7);
                                        CreditGLaccountcode[6] = '\0';
                                        PIN_ERR_LOG_MSG(3, CreditGLaccountcode);
                                        PIN_FLIST_FLD_SET(reslts_flistp, PIN_FLD_GLACCOUNT, &CreditGLaccountcode, ebufp);
                                }
                                if (gl_offset_acct)
                                {
                                        sprintf(msg, "gl_offset_acct: %d", gl_offset_acct);
                                        PIN_ERR_LOG_MSG(3, msg);

                                        strncpy(Debitworksorder, &gl_offset_acct[0], 8);
                                        Debitworksorder[8] = '\0';
                                        PIN_ERR_LOG_MSG(3, Debitworksorder);
                                        PIN_FLIST_FLD_SET(reslts_flistp, PIN_FLD_STRING, &Debitworksorder, ebufp);

                                        strncpy(DebitCostCenterCode, &gl_offset_acct[8], 3);
                                        DebitCostCenterCode[3] = '\0';
                                        PIN_ERR_LOG_MSG(3, DebitCostCenterCode);
                                        PIN_FLIST_FLD_SET(reslts_flistp, PIN_FLD_CODE_STR, &DebitCostCenterCode, ebufp);

                                        strncpy(DebitGLaccountc, &gl_offset_acct[11], 7);
                                        DebitGLaccountc[6] = '\0';
                                        PIN_ERR_LOG_MSG(3, DebitGLaccountc);
                                        PIN_FLIST_FLD_SET(reslts_flistp, PIN_FLD_ACCOUNT_CODE, &DebitGLaccountc, ebufp);
                                }

                                if (type && *type == 2)
                                {
                                        bill_type = (char *) malloc(strlen("BILLED")+1);
                                        strcpy(bill_type,"BILLED");
                                }
                                else if(type && *type == 1)
                                {
                                        bill_type = (char *) malloc(strlen("UNBILLED")+1);
                                        strcpy(bill_type,"UNBILLED");
                                }
                                else if(type && *type == 32)
                                {
                                        bill_type = (char *) malloc(strlen("BILLED_EARNED")+1);
                                        strcpy(bill_type,"BILLED_EARNED");
                                }
                                else if(type && *type == 16)
                                {
                                        bill_type = (char *) malloc(strlen("BILLED_UNEARNED")+1);
                                        strcpy(bill_type,"BILLED_UNEARNED");
                                }
                                else if(type && *type == 8)
                                {
                                        bill_type = (char *) malloc(strlen("UNBILLED_EARNED")+1);
                                        strcpy(bill_type,"UNBILLED_EARNED");
                                }
                                else if(type && *type == 4)
                                {
                                        bill_type = (char *) malloc(strlen("UNBILLED_UNEARNED")+1);
                                        strcpy(bill_type,"UNBILLED_UNEARNED");
                                }
                                else if(type && *type == 64)
                                {
                                        bill_type = (char *) malloc(strlen("PREV_BILLED_EARNED")+1);
                                        strcpy(bill_type,"PREV_BILLED_EARNED");
                                }
                                PIN_FLIST_FLD_SET(reslts_flistp, PIN_FLD_STATUS_STR, bill_type, ebufp);
                                elem_cnt2++;
				PIN_FLIST_DESTROY_EX(&seg_iflistp, NULL);
                        }
                }
        }
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_rbs_gl_search output flist", ret_flistpp);
	rec_cnt = PIN_FLIST_ELEM_COUNT(ret_flistpp, PIN_FLD_RESULTS, ebufp);
	if (rec_cnt > 39000)
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"ERROR! The output is over the system limit", ebufp);
                PIN_ERRBUF_RESET(ebufp);
                *r_flistp = PIN_FLIST_CREATE(ebufp);
                PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, *r_flistp, PIN_FLD_POID, ebufp);
                PIN_FLIST_FLD_SET(*r_flistp, PIN_FLD_STATUS, &search_fail, ebufp);
                PIN_FLIST_FLD_SET(*r_flistp, PIN_FLD_ERROR_CODE, "51222", ebufp);
                PIN_FLIST_FLD_SET(*r_flistp, PIN_FLD_ERROR_DESCR, "No. Of Records searched are over limit for display . Please use another search criteria",ebufp);
                return;
	}

CLEANUP:
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_rbs_glid_search output flist", ret_flistpp);
        *r_flistp = PIN_FLIST_COPY(ret_flistpp, ebufp);
        PIN_FLIST_DESTROY_EX(&ret_flistp, NULL);
        return;
}

void
fm_rbs_glid_srch(
        pcm_context_t           *ctxp,
        pin_flist_t             *i_flistp,
        pin_flist_t             **r_flistp,
        pin_errbuf_t            *ebufp)
{
        pin_flist_t     *in_flistp = NULL;
        pin_flist_t     *ret_flistp = NULL;
        pin_flist_t     *args_flistp = NULL;
        pin_flist_t     *glid_flistp = NULL;
        pin_flist_t     *rslt_flistp = NULL;
        pin_flist_t     *glid_accts_flistp = NULL;
        pin_rec_id_t    gl_id = 0;
        char            *templatep = "select '.1010_csc' GL_SEGMENT from /config where F1 = V1";
        char            glid[100];
        char            msg[100];
	char		glid_str[100];
        int32           search_fail = 1;
//        int32           *glid1 = NULL;
	char		*glid1 = NULL;
        int32           flags = 256;
	int32		count = 0;
        int64           db = -1;
        poid_t          *vp = NULL;
	int32		elem_id = 0;
	int32           elem_id1 = 0;
	pin_cookie_t	cookie = NULL;
	pin_cookie_t    cookie1 = NULL;

        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_rbs_glid_srch input flist:", i_flistp);
        vp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0 ,ebufp);
//        glid1 = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_GL_ID, 0, ebufp);
	glid1 = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_GL_ACCT, 0, ebufp);
	PIN_ERR_LOG_MSG(3, glid1);
        if (!glid1)
        {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "PIN_ERR_MISSING_ARG Mandatory field is missing in input", ebufp);
                PIN_ERRBUF_RESET(ebufp);
                *r_flistp = PIN_FLIST_CREATE(ebufp);
                PIN_FLIST_FLD_SET(*r_flistp, PIN_FLD_POID, vp, ebufp );
                PIN_FLIST_FLD_SET(*r_flistp, PIN_FLD_STATUS, &search_fail, ebufp);
                PIN_FLIST_FLD_SET(*r_flistp, PIN_FLD_ERROR_CODE, "51220", ebufp);
                PIN_FLIST_FLD_SET(*r_flistp, PIN_FLD_ERROR_DESCR, "Search: GLID Missing Error", ebufp);
                return;
        }
        if (glid1 != NULL)
        {
		gl_id = (int32)atoi(glid1);
                sprintf(msg, "gl_id: %d", gl_id);
                PIN_ERR_LOG_MSG(3, msg);
		
            //gl_id = *(pin_rec_id_t *)gl_id;
            //sprintf(msg, "glid: %d", gl_id);
            //PIN_ERR_LOG_MSG(3, msg);
        }
        in_flistp = PIN_FLIST_CREATE(ebufp);
        db = PIN_POID_GET_DB(vp);
        PIN_FLIST_FLD_SET(in_flistp, PIN_FLD_POID, (poid_t *)vp, ebufp);
        PIN_FLIST_FLD_SET(in_flistp, PIN_FLD_FLAGS, &flags, ebufp);
        PIN_FLIST_FLD_SET(in_flistp, PIN_FLD_TEMPLATE, templatep, ebufp);

        args_flistp = PIN_FLIST_ELEM_ADD(in_flistp, PIN_FLD_ARGS, 1, ebufp);
        PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_POID, PIN_POID_CREATE(db, "/config/glid", -1, ebufp), ebufp);

        args_flistp = PIN_FLIST_ELEM_ADD(in_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, ebufp);
        PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_GL_SEGMENT, NULL, ebufp);
        PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_CREATED_T, NULL, ebufp);
        PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_MOD_T, NULL, ebufp);
        glid_flistp = PIN_FLIST_ELEM_ADD(args_flistp, PIN_FLD_GL_IDS, gl_id, ebufp);
        PIN_FLIST_FLD_SET(glid_flistp, PIN_FLD_DESCR, NULL, ebufp);
        glid_accts_flistp = PIN_FLIST_ELEM_ADD(glid_flistp, PIN_FLD_GL_ACCTS, PIN_ELEMID_ANY, ebufp);

        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"fm_rbs_glid_srch: search input flist", in_flistp);
        PCM_OP(ctxp, PCM_OP_SEARCH, PCM_OPFLG_ORDER_BY_REC_ID, in_flistp, &ret_flistp, ebufp);
        if (PIN_ERRBUF_IS_ERR(ebufp))
        {
            PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "ERROR! GL ID Not Found", ebufp);
            PIN_ERRBUF_RESET(ebufp);
            PIN_FLIST_DESTROY_EX(&ret_flistp, NULL);
            goto CLEANUP;
        }

        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"fm_rbs_glid_srch: search output flist", ret_flistp);
	*r_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, *r_flistp, PIN_FLD_POID, ebufp);
	elem_id = 0;
	cookie = NULL;
        while((rslt_flistp = PIN_FLIST_ELEM_TAKE_NEXT(ret_flistp, PIN_FLD_RESULTS, &elem_id, 1, &cookie, ebufp)) != (pin_flist_t *)NULL)
	{
		count = PIN_FLIST_ELEM_COUNT(rslt_flistp, PIN_FLD_GL_IDS, ebufp);
		if (count > 0)
		{
			PIN_FLIST_ELEM_PUT(*r_flistp, rslt_flistp, PIN_FLD_RESULTS, elem_id, ebufp);
			PIN_ERR_LOG_FLIST(3, "rslt flist", rslt_flistp);
			elem_id1 = 0;
			cookie1 = NULL;
			while((glid_flistp = PIN_FLIST_ELEM_GET_NEXT(rslt_flistp, PIN_FLD_GL_IDS, &elem_id1, 1, &cookie1, ebufp)) != (pin_flist_t *)NULL)
			{
				PIN_ERR_LOG_MSG(3, "glid array");
				sprintf(glid_str, "%d", elem_id1);       
				PIN_ERR_LOG_MSG(3, glid_str);
			}
		}	
	}
	PIN_ERR_LOG_MSG(3, glid1);
	PIN_ERR_LOG_MSG(3, glid_str);
        if ((PIN_FLIST_ELEM_COUNT(*r_flistp, PIN_FLD_RESULTS, ebufp) < 1) || (strcmp(glid1, glid_str) != 0))
        {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"ERROR! GL ID Not Found", ebufp);
                PIN_ERRBUF_RESET(ebufp);
                *r_flistp = PIN_FLIST_CREATE(ebufp);
                PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, *r_flistp, PIN_FLD_POID, ebufp);
                PIN_FLIST_FLD_SET(*r_flistp, PIN_FLD_STATUS, &search_fail, ebufp);
                PIN_FLIST_FLD_SET(*r_flistp, PIN_FLD_ERROR_CODE, "51221", ebufp);
                PIN_FLIST_FLD_SET(*r_flistp, PIN_FLD_ERROR_DESCR, "ERROR! Given GL ID Not Found", ebufp);
                return;
        }

CLEANUP:
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_rbs_glid_srch output flist", *r_flistp);
        PIN_FLIST_DESTROY_EX(&ret_flistp, NULL);
        return;
}

void
fm_rbs_descr_srch(
        pcm_context_t           *ctxp,
        pin_flist_t             *i_flistp,
        pin_flist_t             **r_flistp,
        pin_errbuf_t            *ebufp)
{
        pin_flist_t             *in_flistp = NULL;
        pin_flist_t             *ret_flistp = NULL;
        pin_flist_t             *arg_flistp = NULL;
        pin_flist_t             *arg1_flistp = NULL;
        pin_flist_t             *arg2_flistp = NULL;
        pin_flist_t             *glid_flistp = NULL;
        pin_flist_t             *gl_ids_flistp = NULL;
        pin_flist_t             *gl_ids_rflistp = NULL;
        pin_flist_t             *gl_accts_flistp = NULL;
        pin_flist_t             *glid_accts_rflistp = NULL;
        pin_flist_t             *reslt_flistp = NULL;
        pin_flist_t             *res_flistp = NULL;
        pin_flist_t             *rslt_flistp = NULL;
        pin_flist_t             *result_flistp = NULL;
        poid_t                  *pdp = NULL;
        poid_t                  *glid_pdp = NULL;
        int32                   search_fail = 1;
        int32                   *attr = NULL;
        int32                   *type = NULL;
        int32                   elem_cnt = 0;
        int32                   cnt = 0;
        int32                   cnt1 = 0;
        int32                   elem_id = 0;
        int32                   elem_id1 = 0;
        int32                   elem_id2 = 0;
        int32                   flags = 256;
        int32                   *action_flags = NULL;
        int64                   db = -1;
        char                    *gl_ar_acct = NULL;
        char                    *gl_offset_acct = NULL;
        char                    *i_descr = NULL;
        char                    *r_descr = NULL;
        char                    *template = "select * from /config/glid where F1 = V1";
        char                    elem_glid[100];
        char                    msg[100];
        pin_cookie_t            cookie = NULL;
        pin_cookie_t            cookie1 = NULL;
        pin_cookie_t            cookie2 = NULL;

        if (PIN_ERRBUF_IS_ERR(ebufp))
        return;
        PIN_ERRBUF_CLEAR(ebufp);

        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "start fm_rbs_descr_srch input_flist", i_flistp);

        pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp);
        db = PIN_POID_GET_DB(pdp);

        glid_pdp = PIN_POID_CREATE(db, "/config/glid", -1, ebufp);

        in_flistp = PIN_FLIST_CREATE(ebufp);
        PIN_FLIST_FLD_SET(in_flistp, PIN_FLD_POID, pdp, ebufp);
        PIN_FLIST_FLD_SET(in_flistp, PIN_FLD_FLAGS, &flags, ebufp);
        PIN_FLIST_FLD_SET(in_flistp, PIN_FLD_TEMPLATE, template, ebufp);

        arg1_flistp = PIN_FLIST_ELEM_ADD(in_flistp, PIN_FLD_ARGS, 1, ebufp);
        PIN_FLIST_FLD_SET(arg1_flistp, PIN_FLD_POID, glid_pdp, ebufp);

        arg2_flistp = PIN_FLIST_ELEM_ADD(in_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, ebufp);
        PIN_FLIST_FLD_SET(arg2_flistp, PIN_FLD_GL_SEGMENT, NULL, ebufp);
        PIN_FLIST_FLD_SET(arg2_flistp, PIN_FLD_CREATED_T, NULL, ebufp);
        PIN_FLIST_FLD_SET(arg2_flistp, PIN_FLD_MOD_T, NULL, ebufp);

        gl_ids_flistp = PIN_FLIST_ELEM_ADD(arg2_flistp, PIN_FLD_GL_IDS, PIN_ELEMID_ANY, ebufp);
        PIN_FLIST_FLD_SET(gl_ids_flistp, PIN_FLD_DESCR, NULL, ebufp);
        gl_accts_flistp = PIN_FLIST_ELEM_ADD(gl_ids_flistp, PIN_FLD_GL_ACCTS, PIN_ELEMID_ANY, ebufp);

        PIN_ERR_LOG_FLIST(3, "end fm_rbs_descr_srch input_flist", in_flistp);

        PCM_OP(ctxp, PCM_OP_SEARCH, PCM_OPFLG_ORDER_BY_REC_ID, in_flistp, &res_flistp, ebufp);

        PIN_ERR_LOG_FLIST(3, "SEARCH output flist", res_flistp);

        ret_flistp = PIN_FLIST_CREATE(ebufp);
        PIN_FLIST_FLD_COPY(res_flistp, PIN_FLD_POID, ret_flistp, PIN_FLD_POID, ebufp);
        i_descr = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_DESCR, 0, ebufp);
        if (!i_descr || strcmp(i_descr, "") == 0)
        {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "PIN_ERR_MISSING_ARG Mandatory field is missing in input", ebufp);
                PIN_ERRBUF_RESET(ebufp);
                *r_flistp = PIN_FLIST_CREATE(ebufp);
                PIN_FLIST_FLD_SET(*r_flistp, PIN_FLD_POID, pdp, ebufp );
                PIN_FLIST_FLD_SET(*r_flistp, PIN_FLD_STATUS, &search_fail, ebufp);
                PIN_FLIST_FLD_SET(*r_flistp, PIN_FLD_ERROR_CODE, "51220", ebufp);
                PIN_FLIST_FLD_SET(*r_flistp, PIN_FLD_ERROR_DESCR, "Search: Description Missing Error", ebufp);
                return;
        }

        elem_id = 0;
        cookie = NULL;
        while ((rslt_flistp = PIN_FLIST_ELEM_TAKE_NEXT(res_flistp, PIN_FLD_RESULTS, &elem_id, 1, &cookie, ebufp)) != NULL)
        {
                sprintf(msg, "count: %d", cnt);
                PIN_ERR_LOG_MSG(3, msg);
                reslt_flistp = PIN_FLIST_ELEM_ADD(ret_flistp, PIN_FLD_RESULTS, elem_cnt, ebufp);
                PIN_FLIST_FLD_COPY(rslt_flistp, PIN_FLD_POID, reslt_flistp, PIN_FLD_POID, ebufp);
                PIN_FLIST_FLD_COPY(rslt_flistp, PIN_FLD_CREATED_T, reslt_flistp, PIN_FLD_CREATED_T, ebufp);
                PIN_FLIST_FLD_COPY(rslt_flistp, PIN_FLD_MOD_T, reslt_flistp, PIN_FLD_MOD_T, ebufp);
                PIN_ERR_LOG_FLIST(3, "results getting flist:", rslt_flistp);

                elem_id1 = 0;
                cookie1 = NULL;
                while ((gl_ids_rflistp = PIN_FLIST_ELEM_GET_NEXT(rslt_flistp, PIN_FLD_GL_IDS, &elem_id1, 1, &cookie1, ebufp)) != NULL)
                {
                        r_descr = PIN_FLIST_FLD_GET(gl_ids_rflistp, PIN_FLD_DESCR, 0, ebufp);
                        if (i_descr && r_descr && strstr(r_descr, i_descr))
                        {
                                sprintf(msg, "i_descr: %s, r_descr: %s", i_descr, r_descr);
                                PIN_ERR_LOG_MSG(3, msg);
                                PIN_FLIST_ELEM_PUT(reslt_flistp, gl_ids_rflistp, PIN_FLD_GL_IDS, elem_id1, ebufp);
                        }
                        cnt++;
                }
                elem_cnt++;
        }
        if (PIN_FLIST_ELEM_COUNT(reslt_flistp, PIN_FLD_GL_IDS, ebufp) < 1)
        {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"ERROR! GL Description Not Found", ebufp);
                PIN_ERRBUF_RESET(ebufp);
                *r_flistp = PIN_FLIST_CREATE(ebufp);
                PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, *r_flistp, PIN_FLD_POID, ebufp);
                PIN_FLIST_FLD_SET(*r_flistp, PIN_FLD_STATUS, &search_fail, ebufp);
                PIN_FLIST_FLD_SET(*r_flistp, PIN_FLD_ERROR_CODE, "51221", ebufp);
                PIN_FLIST_FLD_SET(*r_flistp, PIN_FLD_ERROR_DESCR, "ERROR! GL Description Not Found", ebufp);
                return;
        }

        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"fm_rbs_descr_srch: search output flist", ret_flistp);

CLEANUP:
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_rbs_descr_srch output flist", ret_flistp);
        *r_flistp = PIN_FLIST_COPY(ret_flistp, ebufp);
        PIN_FLIST_DESTROY_EX(&ret_flistp, NULL);
        return;
}

void
fm_rbs_gl_segment_srch(
        pcm_context_t           *ctxp,
        pin_flist_t             *i_flistp,
        pin_flist_t             **r_flistp,
        pin_errbuf_t            *ebufp)
{
        pin_flist_t             *in_flistp = NULL;
        pin_flist_t             *args_flistp = NULL;
        pin_flist_t             *args1_flistp = NULL;
        pin_flist_t             *args2_flistp = NULL;
        pin_flist_t             *args3_flistp = NULL;
        pin_flist_t             *gl_seg_iflistp = NULL;
        pin_flist_t             *so_flistp = NULL;
        pin_flist_t             *robj_iflistp = NULL;
        pin_flist_t             *robj_rflistp = NULL;
        pin_flist_t             *res_flistp = NULL;
        pin_flist_t             *rslt_flistp = NULL;
        pin_flist_t             *ret_flistp = NULL;
        pin_flist_t             *glid_flistp = NULL;
        pin_flist_t             *glid_accts_flistp = NULL;
        pin_flist_t             *rs_get_flistp = NULL;
        poid_t                  *vp = NULL;
        int32                   search_fail = 1;
        int32                   flags = 256;
        int64                   db = -1;
        int32                   *attr = NULL;
        int32                   *type = NULL;
	int32			seg_len = 0;
        int32                   elem_cnt = 0;
        int32                   cnt = 0;
        int32                   elem_id = 0;
        int32                   elem_id1 = 0;
        int32                   elem_id2 = 0;
	int32			elem_id3 = 0;
        pin_cookie_t            cookie = NULL;
        pin_cookie_t            cookie1 = NULL;
        pin_cookie_t            cookie2 = NULL;
	pin_cookie_t		cookie3 = NULL;
        char                    msg[100];
        char                    *descr = NULL;
        char                    *gl_seg = NULL;
	char			gl_seg1[100];
        char                    gl_segment[100];
        char                    *gl_ar_acct = NULL;
        char                    *gl_offset_acct = NULL;
        char                    *gl_name = NULL;
        char                    *glid_pdp1 = NULL;
        char                    *templatep = "select x from /config/gl_segment where F1 like V1";
        char                    *templatep1 = "select x from /config/glid where F1 = V1";

        if (PIN_ERRBUF_IS_ERR(ebufp))
        return;
        PIN_ERRBUF_CLEAR(ebufp);

        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "start fm_rbs_gl_segment_srch input flist:", i_flistp);
        vp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0 ,ebufp);
        gl_seg =  PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_GL_SEGMENT, 1, ebufp);
	if (gl_seg)
		seg_len = strlen(gl_seg);
	
        if (!gl_seg || strcmp(gl_seg, "") == 0)
        {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "PIN_ERR_MISSING_ARG Mandatory field is missing in input", ebufp);
                PIN_ERRBUF_RESET(ebufp);
                *r_flistp = PIN_FLIST_CREATE(ebufp);
                PIN_FLIST_FLD_SET(*r_flistp, PIN_FLD_POID, vp, ebufp );
                PIN_FLIST_FLD_SET(*r_flistp, PIN_FLD_STATUS, &search_fail, ebufp);
                PIN_FLIST_FLD_SET(*r_flistp, PIN_FLD_ERROR_CODE, "51220", ebufp);
                PIN_FLIST_FLD_SET(*r_flistp, PIN_FLD_ERROR_DESCR, "Search: GL Segment Missing Error", ebufp);
                return;
        }
	else if(seg_len > 1 && strstr(gl_seg, "."))
	{
		gl_seg = (char *)malloc(100*sizeof(char));
		gl_seg =  PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_GL_SEGMENT, 0, ebufp);
		scanf("%s", gl_seg);
		gl_seg = &gl_seg[1];
		PIN_ERR_LOG_MSG(3, gl_seg);
        	strcpy(gl_seg1, "%");
	        strcat(gl_seg1, gl_seg);
		PIN_ERR_LOG_MSG(3, gl_seg1);
	}
	else
	{
		PIN_ERR_LOG_MSG(3, gl_seg);
		strcpy(gl_seg1, "%");
		strcat(gl_seg1, gl_seg);
	}
	PIN_ERR_LOG_MSG(3, gl_seg1);

        in_flistp = PIN_FLIST_CREATE(ebufp);
        db = PIN_POID_GET_DB(vp);
        PIN_FLIST_FLD_SET(in_flistp, PIN_FLD_POID, (poid_t *)vp, ebufp);
        PIN_FLIST_FLD_SET(in_flistp, PIN_FLD_FLAGS, &flags, ebufp);
        PIN_FLIST_FLD_SET(in_flistp, PIN_FLD_TEMPLATE, templatep, ebufp);

        args_flistp = PIN_FLIST_ELEM_ADD(in_flistp, PIN_FLD_ARGS, 1, ebufp);
        gl_seg_iflistp = PIN_FLIST_ELEM_ADD(args_flistp, PIN_FLD_GL_SEGMENTS, 0, ebufp);
        PIN_FLIST_FLD_SET(gl_seg_iflistp, PIN_FLD_SEGMENT_NAME, &gl_seg1, ebufp);

        args1_flistp = PIN_FLIST_ELEM_ADD(in_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, ebufp);
        PIN_FLIST_FLD_SET(args1_flistp, PIN_FLD_NAME, NULL, ebufp);
        PIN_FLIST_FLD_SET(args1_flistp, PIN_FLD_VALUE, NULL, ebufp);

        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"fm_glid_search: search input flist", in_flistp);
        PCM_OP(ctxp, PCM_OP_SEARCH, PCM_OPFLG_ORDER_BY_REC_ID, in_flistp, &so_flistp, ebufp);
        if (PIN_ERRBUF_IS_ERR(ebufp))
        {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "ERROR! GL_SEGMENT Not Found", ebufp);
                PIN_ERRBUF_RESET(ebufp);
                goto CLEANUP;
        }
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"fm_gl_segment_search: search output flist ", so_flistp);

        if (PIN_FLIST_ELEM_COUNT(so_flistp, PIN_FLD_RESULTS, ebufp) < 1)
        {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"ERROR! GL Segment Not Found", ebufp);
                PIN_ERRBUF_RESET(ebufp);
                *r_flistp = PIN_FLIST_CREATE(ebufp);
                PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, *r_flistp, PIN_FLD_POID, ebufp);
                PIN_FLIST_FLD_SET(*r_flistp, PIN_FLD_STATUS, &search_fail, ebufp);
                PIN_FLIST_FLD_SET(*r_flistp, PIN_FLD_ERROR_CODE, "51221", ebufp);
                PIN_FLIST_FLD_SET(*r_flistp, PIN_FLD_ERROR_DESCR, "ERROR! GL Segment Not Found", ebufp);
                return;
        }

	ret_flistp = PIN_FLIST_CREATE(ebufp);

	elem_id3 = 0;
	cookie3 = NULL;
        while ((rs_get_flistp = PIN_FLIST_ELEM_GET_NEXT(so_flistp, PIN_FLD_RESULTS, &elem_id3, 1, &cookie3, ebufp)) != NULL)
	{
        	gl_name = PIN_FLIST_FLD_GET(rs_get_flistp, PIN_FLD_NAME, 0, ebufp);
        	glid_pdp1 = PIN_FLIST_FLD_GET(rs_get_flistp, PIN_FLD_VALUE, 0, ebufp);
	
        	PIN_ERR_LOG_MSG(3, glid_pdp1);
        	poid_t* glid_pdp = PIN_POID_FROM_STR(glid_pdp1, NULL, ebufp);
        	PIN_ERR_LOG_POID(3, "value1 poid: ", glid_pdp);

        	robj_iflistp = PIN_FLIST_CREATE(ebufp);
        	PIN_FLIST_FLD_SET(robj_iflistp, PIN_FLD_POID, (poid_t *)vp, ebufp);
        	PIN_FLIST_FLD_SET(robj_iflistp, PIN_FLD_FLAGS, &flags, ebufp);
        	PIN_FLIST_FLD_SET(robj_iflistp, PIN_FLD_TEMPLATE, templatep1, ebufp);

        	args2_flistp = PIN_FLIST_ELEM_ADD(robj_iflistp, PIN_FLD_ARGS, 1, ebufp);
		PIN_FLIST_FLD_SET(args2_flistp, PIN_FLD_POID, glid_pdp, ebufp);

		args3_flistp = PIN_FLIST_ELEM_ADD(robj_iflistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, ebufp);
		PIN_FLIST_FLD_SET(args3_flistp, PIN_FLD_GL_SEGMENT, NULL, ebufp);
		PIN_FLIST_FLD_SET(args3_flistp, PIN_FLD_CREATED_T, NULL, ebufp);
		PIN_FLIST_FLD_SET(args3_flistp, PIN_FLD_MOD_T, NULL, ebufp);

		glid_flistp = PIN_FLIST_ELEM_ADD(args3_flistp, PIN_FLD_GL_IDS, PIN_ELEMID_ANY, ebufp);
		PIN_FLIST_FLD_SET(glid_flistp, PIN_FLD_DESCR, NULL, ebufp);
		glid_accts_flistp = PIN_FLIST_ELEM_ADD(glid_flistp, PIN_FLD_GL_ACCTS, PIN_ELEMID_ANY, ebufp);

		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"fm_glid_search: search input flist", robj_iflistp);
		PCM_OP(ctxp, PCM_OP_SEARCH, PCM_OPFLG_ORDER_BY_REC_ID, robj_iflistp, &robj_rflistp, ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "ERROR! GL_SEGMENT Not Found", ebufp);
			PIN_ERRBUF_RESET(ebufp);
			PIN_FLIST_DESTROY_EX(&robj_rflistp, NULL);
			goto CLEANUP;
		}
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"fm_gl_segment_search: search output_flist robj_rflistp: ", robj_rflistp);

	//	ret_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_COPY(robj_rflistp, PIN_FLD_POID, ret_flistp, PIN_FLD_POID, ebufp);

		elem_id = 0;
		cookie = NULL;
		while ((res_flistp = PIN_FLIST_ELEM_GET_NEXT(robj_rflistp, PIN_FLD_RESULTS, &elem_id, 1, &cookie, ebufp)) != NULL)
		{
			sprintf(msg, "gl_name: %s, gl_seg: %s", gl_name, gl_seg);
			PIN_ERR_LOG_MSG(3, msg);

			if (gl_name && gl_seg && strstr(gl_name, gl_seg) )
			{
				PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_GL_SEGMENT, gl_name, ebufp);
			}
			else
			{
				sprintf(gl_segment, "%s", ".1010_csc");
				PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_GL_SEGMENT, &gl_segment, ebufp);
			}
			PIN_FLIST_ELEM_PUT(ret_flistp, res_flistp, PIN_FLD_RESULTS, cnt, ebufp);
			cnt++;
		}
		PIN_ERR_LOG_FLIST(3, "ret_flistp: ", ret_flistp);
	}
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"fm_rbs_gl_segment_srch: search output flist", ret_flistp);

CLEANUP:
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_rbs_gl_segment_srch output flist", ret_flistp);
        *r_flistp = PIN_FLIST_COPY(ret_flistp, ebufp);
        PIN_FLIST_DESTROY_EX(&ret_flistp, NULL);
        return;
}

void
fm_rbs_history_srch(
        pcm_context_t           *ctxp,
        pin_flist_t             *i_flistp,
        pin_flist_t             **r_flistp,
        pin_errbuf_t            *ebufp)
{
        pin_flist_t             *in_flistp = NULL;
        pin_flist_t             *ret_flistp = NULL;
        pin_flist_t             *args_flistp = NULL;
        pin_flist_t             *args1_flistp = NULL;
        pin_flist_t             *args2_flistp = NULL;
        pin_flist_t             *args3_flistp = NULL;
        pin_flist_t             *glid_flistp = NULL;
        pin_flist_t             *glid_accts = NULL;
        pin_flist_t             *reslts_flistp = NULL;
        pin_flist_t             *res_flistp = NULL;
        pin_flist_t             *rslt_flistp = NULL;
        pin_flist_t             *return1_flistp = NULL;
        pin_flist_t             *ret_glid_flistp = NULL;
        pin_flist_t             *ret_glid_accts_flistp = NULL;
        poid_t                  *pdp = NULL;
        poid_t                  *glid_pdp = NULL;
        int32                   *attr = NULL;
        int32                   *type = NULL;
        int32                   gl_id = 0;
        int32                   cnt = 0;
        int32                   cnt1 = 0;
        int32                   elem_id = 0;
        int32                   elem_id1 = 0;
        int32                   elem_id2 = 0;
        int32                   elem_cnt = 0;
        int32                   elem_cnt1 = 0;
        int32                   flags = 256;
        int32                   *action_flags = NULL;
        int32                   search_fail = 1;
	int			date = 0;
	int			month = 0;
	int			year = 0;
        int64                   db = -1;
	char			c_date[20];
	char			ce_date[20];
        char                    *glid = NULL;
        char                    *gl_ar_acct = NULL;
        char                    *gl_offset_acct = NULL;
        char                    *i_descr = NULL;
        char                    *r_descr = NULL;
        char                    *template = "select X from /au_config  where F1 > V1 and F2 < V2 and F3 = V3 order by created_t asc";
        char                    elem_glid[100];
        char                    glid1[100];
        char                    msg[100];
	char			*i_start_t = NULL;
	char			*i_end_t = NULL;
	char			yyyy[5], mm[3], dd[3], t_str[20];    
	char			*yr;
	char			*mn = NULL;
	char			*dy = NULL;
        time_t                  *start_t = NULL;
        time_t                  *end_t = NULL;
	int32			var = 0;
	struct tm		tmp;
	struct tm		e_tmp;
        pin_rec_id_t            gl_id1 = 0;
        pin_cookie_t            cookie = NULL;
        pin_cookie_t            cookie1 = NULL;
        pin_cookie_t            cookie2 = NULL;

        if (PIN_ERRBUF_IS_ERR(ebufp))
        return;
        PIN_ERRBUF_CLEAR(ebufp);

        PIN_ERR_LOG_FLIST(3, "start fm_rbs_history_srch input_flist", i_flistp);

        pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp);
        db = PIN_POID_GET_DB(pdp);

        i_start_t = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_TIME_FROM_STR, 0, ebufp);
        if (!i_start_t || *i_start_t == 0)
        {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "PIN_ERR_MISSING_ARG Mandatory field is missing in input", ebufp);
                PIN_ERRBUF_RESET(ebufp);
                *r_flistp = PIN_FLIST_CREATE(ebufp);
                PIN_FLIST_FLD_SET(*r_flistp, PIN_FLD_POID, pdp, ebufp );
                PIN_FLIST_FLD_SET(*r_flistp, PIN_FLD_STATUS, &search_fail, ebufp);
                PIN_FLIST_FLD_SET(*r_flistp, PIN_FLD_ERROR_CODE, "51220", ebufp);
                PIN_FLIST_FLD_SET(*r_flistp, PIN_FLD_ERROR_DESCR, "Search: START_T Missing Error", ebufp);
                return;
        }
	else
	{
		PIN_ERR_LOG_MSG(3, i_start_t);
		
		strcat(c_date, i_start_t); 

		strncpy(yyyy, &c_date[0], 4);
		yyyy[4] = '\0';
                yr = (char *)yyyy;
                year = (int)atoi(yr);
                sprintf(msg, "yr: %s, year: %d", yr, year);
		
		strncpy(mm, &c_date[5], 7);
		mm[2] = '\0';
                mn = (char *)mm;
                month = (int)atoi(mn);
                sprintf(msg, "mn: %s, month: %d", mn, month);
                PIN_ERR_LOG_MSG(3, msg);

		dy = NULL;
                date = 0;
		strncpy(dd, &c_date[8], 10);
		dd[2] = '\0';
		dy = (char *)dd;
		date = (int)atoi(dy);
		sprintf(msg, "dy: %s, date: %d", dy, date);
		PIN_ERR_LOG_MSG(3, msg);

		sprintf(msg, "year: %d, month: %d, date: %d", year, month, date);
		PIN_ERR_LOG_MSG(3, msg);

    		tmp.tm_year = year - 1900;  // Year - 1900
    		tmp.tm_mon = month - 1;           // Month, where 0 = jan
    		tmp.tm_mday = date;          // Day of the month
    		tmp.tm_hour = 00;
    		tmp.tm_min = 00;
    		tmp.tm_sec = 00;
    		tmp.tm_isdst = -1;        // Is DST on? 1 = yes, 0 = no, -1 = unknown
    		start_t = (time_t *)mktime(&tmp);
		
		sprintf(msg, "start_t: %d", start_t);
		PIN_ERR_LOG_MSG(3, msg);
		

	}
        i_end_t = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_TIME_TO_STR, 0, ebufp);
        if (!i_end_t || *i_end_t == 0)
        {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "PIN_ERR_MISSING_ARG Mandatory field is missing in input", ebufp);
                PIN_ERRBUF_RESET(ebufp);
                *r_flistp = PIN_FLIST_CREATE(ebufp);
                PIN_FLIST_FLD_SET(*r_flistp, PIN_FLD_POID, pdp, ebufp );
                PIN_FLIST_FLD_SET(*r_flistp, PIN_FLD_STATUS, &search_fail, ebufp);
                PIN_FLIST_FLD_SET(*r_flistp, PIN_FLD_ERROR_CODE, "51220", ebufp);
                PIN_FLIST_FLD_SET(*r_flistp, PIN_FLD_ERROR_DESCR, "Search: END_T Missing Error", ebufp);
                return;
        }
	else
	{
		PIN_ERR_LOG_MSG(3, i_end_t);

                strcat(ce_date, i_end_t);
		
		strncpy(yyyy, &ce_date[0], 4);
                yyyy[4] = '\0';
                yr = (char *)yyyy;
                year = (int)atoi(yr);
                sprintf(msg, "yr: %s, year: %d", yr, year);

                strncpy(mm, &ce_date[5], 7);
                mm[2] = '\0';
                mn = (char *)mm;
                month = (int)atoi(mn);
                sprintf(msg, "mn: %s, month: %d", mn, month);
                PIN_ERR_LOG_MSG(3, msg);

                dy = NULL;
                date = 0;
                strncpy(dd, &ce_date[8], 10);
                dd[2] = '\0';
                dy = (char *)dd;
                date = (int)atoi(dy);
                sprintf(msg, "dy: %s, date: %d", dy, date);
                PIN_ERR_LOG_MSG(3, msg);
		
                sprintf(msg, "year: %d, month: %d, date: %d", year, month, date);
                PIN_ERR_LOG_MSG(3, msg);

                e_tmp.tm_year = year - 1900;  // Year - 1900
                e_tmp.tm_mon = month - 1;           // Month, where 0 = jan
                e_tmp.tm_mday = date;          // Day of the month
                e_tmp.tm_hour = 23;
                e_tmp.tm_min = 59;
                e_tmp.tm_sec = 59;
                e_tmp.tm_isdst = -1;        // Is DST on? 1 = yes, 0 = no, -1 = unknown
                end_t = (time_t *)mktime(&e_tmp);

                sprintf(msg, "end_t: %d", end_t);
		PIN_ERR_LOG_MSG(3, msg);
	}

        glid_pdp = PIN_POID_CREATE(db, "/au_config/glid", -1, ebufp);
        in_flistp = PIN_FLIST_CREATE(ebufp);
        PIN_FLIST_FLD_SET(in_flistp, PIN_FLD_POID, pdp, ebufp);
        PIN_FLIST_FLD_SET(in_flistp, PIN_FLD_FLAGS, &flags, ebufp);
        PIN_FLIST_FLD_SET(in_flistp, PIN_FLD_TEMPLATE, template, ebufp);

	var = (int32 )start_t;
        args_flistp = PIN_FLIST_ELEM_ADD(in_flistp, PIN_FLD_ARGS, 1, ebufp);
        PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_CREATED_T, &var, ebufp);

	var = (int32 )end_t;
        args1_flistp = PIN_FLIST_ELEM_ADD(in_flistp, PIN_FLD_ARGS, 2, ebufp);
        PIN_FLIST_FLD_SET(args1_flistp, PIN_FLD_CREATED_T, &var, ebufp);

        args2_flistp = PIN_FLIST_ELEM_ADD(in_flistp, PIN_FLD_ARGS, 3, ebufp);
        PIN_FLIST_FLD_SET(args2_flistp, PIN_FLD_POID, glid_pdp, ebufp);

        args3_flistp = PIN_FLIST_ELEM_ADD(in_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, ebufp);
        PIN_FLIST_FLD_SET(args3_flistp, PIN_FLD_GL_SEGMENT, NULL, ebufp);
        PIN_FLIST_FLD_SET(args3_flistp, PIN_FLD_CREATED_T, NULL, ebufp);
        PIN_FLIST_FLD_SET(args3_flistp, PIN_FLD_MOD_T, NULL, ebufp);
        PIN_FLIST_FLD_SET(args3_flistp, PIN_FLD_AU_PARENT_OBJ, NULL, ebufp);

        glid = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_GL_ACCT, 0, ebufp);
        if (!glid)
        {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "PIN_ERR_MISSING_ARG Mandatory field is missing in input", ebufp);
                PIN_ERRBUF_RESET(ebufp);
                *r_flistp = PIN_FLIST_CREATE(ebufp);
                PIN_FLIST_FLD_SET(*r_flistp, PIN_FLD_POID, pdp, ebufp );
                PIN_FLIST_FLD_SET(*r_flistp, PIN_FLD_STATUS, &search_fail, ebufp);
                PIN_FLIST_FLD_SET(*r_flistp, PIN_FLD_ERROR_CODE, "51220", ebufp);
                PIN_FLIST_FLD_SET(*r_flistp, PIN_FLD_ERROR_DESCR, "Search: GLID Missing Error", ebufp);
                return;
        }
        if (glid && !strcmp(glid, "%") == 0)
        {
                gl_id = (int32)atoi(glid);
                sprintf(msg, "gl_id: %d", gl_id);
                PIN_ERR_LOG_MSG(3, msg);
                gl_id1 = (pin_rec_id_t)gl_id;
                glid_flistp = PIN_FLIST_ELEM_ADD(args3_flistp, PIN_FLD_GL_IDS, gl_id1, ebufp);
        }
        else
        {
                glid_flistp = PIN_FLIST_ELEM_ADD(args3_flistp, PIN_FLD_GL_IDS, PIN_ELEMID_ANY, ebufp);
        }
        PIN_FLIST_FLD_SET(glid_flistp, PIN_FLD_DESCR, NULL, ebufp);

        glid_accts = PIN_FLIST_ELEM_ADD(glid_flistp, PIN_FLD_GL_ACCTS, PIN_ELEMID_ANY, ebufp);

        PIN_ERR_LOG_FLIST(3, "end fm_rbs_history_srch input_flist", in_flistp);

        PCM_OP(ctxp, PCM_OP_SEARCH, 0, in_flistp, &ret_flistp, ebufp);

        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"fm_rbs_history_srch: search output flist", ret_flistp);
        rslt_flistp = PIN_FLIST_ELEM_GET(ret_flistp, PIN_FLD_RESULTS, 0, 1, ebufp);
        if(PIN_FLIST_ELEM_COUNT(rslt_flistp, PIN_FLD_GL_IDS, ebufp) < 1)
        {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"ERROR! GL History Not Found", ebufp);
                PIN_ERRBUF_RESET(ebufp);
                *r_flistp = PIN_FLIST_CREATE(ebufp);
                PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, *r_flistp, PIN_FLD_POID, ebufp);
                PIN_FLIST_FLD_SET(*r_flistp, PIN_FLD_STATUS, &search_fail, ebufp);
                PIN_FLIST_FLD_SET(*r_flistp, PIN_FLD_ERROR_CODE, "51221", ebufp);
                PIN_FLIST_FLD_SET(*r_flistp, PIN_FLD_ERROR_DESCR, "ERROR! GL History Not Found", ebufp);
                return;
        }

CLEANUP:
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_rbs_history_srch output flist", ret_flistp);
        *r_flistp = PIN_FLIST_COPY(ret_flistp, ebufp);
        PIN_FLIST_DESTROY_EX(&ret_flistp, NULL);
        return;
}

void
fm_rbs_prefix_srch(
        pcm_context_t           *ctxp,
        pin_flist_t             *i_flistp,
        pin_flist_t             **r_flistp,
        pin_errbuf_t            *ebufp)
{
        pin_flist_t             *in_flistp = NULL;
        pin_flist_t             *ret_flistp = NULL;
        pin_flist_t             *arg_flistp = NULL;
        pin_flist_t             *arg1_flistp = NULL;
        pin_flist_t             *arg2_flistp = NULL;
        pin_flist_t             *glid_flistp = NULL;
        pin_flist_t             *gl_ids_flistp = NULL;
        pin_flist_t             *gl_ids_rflistp = NULL;
        pin_flist_t             *gl_accts_flistp = NULL;
        pin_flist_t             *glid_accts_rflistp = NULL;
        pin_flist_t             *res_flistp = NULL;
        pin_flist_t             *rslt_flistp = NULL;
        pin_flist_t             *rslt1_flistp = NULL;
        pin_flist_t             *result_flistp = NULL;
        poid_t                  *pdp = NULL;
        poid_t                  *glid_pdp = NULL;
        int32                   search_fail = 1;
        int32                   *attr = NULL;
        int32                   *type = NULL;
        int32                   cnt = 0;
        int32                   cnt1 = 0;
        int32                   elem_id = 0;
        int32                   elem_id1 = 0;
        int32                   elem_id2 = 0;
        int32                   elem_cnt = 0;
        int32                   flags = 256;
        int32                   *action_flags = NULL;
        int64                   db = -1;
        char                    *gl_ar_acct = NULL;
        char                    *gl_offset_acct = NULL;
        char                    *i_descr = NULL;
        char                    *r_descr = NULL;
        char                    *template = "select * from /config/glid where F1 = V1";
        char                    elem_glid[100];
        char                    msg[100];
        char                    pr_glid[100];
        char                    *i_glid = NULL;
        char                    *glid1 = NULL;
        pin_cookie_t            cookie = NULL;
        pin_cookie_t            cookie1 = NULL;
        pin_cookie_t            cookie2 = NULL;


        if (PIN_ERRBUF_IS_ERR(ebufp))
        return;
        PIN_ERRBUF_CLEAR(ebufp);

        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "start fm_prefix_glid_search input_flist", i_flistp);

        pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp);
        db = PIN_POID_GET_DB(pdp);

        glid_pdp = PIN_POID_CREATE(db, "/config/glid", -1, ebufp);

        in_flistp = PIN_FLIST_CREATE(ebufp);
        PIN_FLIST_FLD_SET(in_flistp, PIN_FLD_POID, pdp, ebufp);
        PIN_FLIST_FLD_SET(in_flistp, PIN_FLD_FLAGS, &flags, ebufp);
        PIN_FLIST_FLD_SET(in_flistp, PIN_FLD_TEMPLATE, template, ebufp);

        arg1_flistp = PIN_FLIST_ELEM_ADD(in_flistp, PIN_FLD_ARGS, 1, ebufp);
        PIN_FLIST_FLD_SET(arg1_flistp, PIN_FLD_POID, glid_pdp, ebufp);

        arg2_flistp = PIN_FLIST_ELEM_ADD(in_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, ebufp);
        PIN_FLIST_FLD_SET(arg2_flistp, PIN_FLD_POID, NULL, ebufp);
        PIN_FLIST_FLD_SET(arg2_flistp, PIN_FLD_CREATED_T, NULL, ebufp);
        PIN_FLIST_FLD_SET(arg2_flistp, PIN_FLD_MOD_T, NULL, ebufp);

        gl_ids_flistp = PIN_FLIST_ELEM_ADD(arg2_flistp, PIN_FLD_GL_IDS, PIN_ELEMID_ANY, ebufp);
        PIN_FLIST_FLD_SET(gl_ids_flistp, PIN_FLD_DESCR, NULL, ebufp);
        gl_accts_flistp = PIN_FLIST_ELEM_ADD(gl_ids_flistp, PIN_FLD_GL_ACCTS, PIN_ELEMID_ANY, ebufp);

        PIN_ERR_LOG_FLIST(3, "end fm_prefix_glid_search input_flist", in_flistp);

        PCM_OP(ctxp, PCM_OP_SEARCH, PCM_OPFLG_ORDER_BY_REC_ID, in_flistp, &res_flistp, ebufp);

        PIN_ERR_LOG_FLIST(3, "SEARCH output flist", res_flistp);

        ret_flistp = PIN_FLIST_CREATE(ebufp);
        PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, ret_flistp, PIN_FLD_POID, ebufp);
        i_glid = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_GL_ACCT, 0, ebufp);
        if (!i_glid)
        {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "PIN_ERR_MISSING_ARG Mandatory field is missing in input", ebufp);
                PIN_ERRBUF_RESET(ebufp);
                *r_flistp = PIN_FLIST_CREATE(ebufp);
                PIN_FLIST_FLD_SET(*r_flistp, PIN_FLD_POID, pdp, ebufp );
                PIN_FLIST_FLD_SET(*r_flistp, PIN_FLD_STATUS, &search_fail, ebufp);
                PIN_FLIST_FLD_SET(*r_flistp, PIN_FLD_ERROR_CODE, "51220", ebufp);
                PIN_FLIST_FLD_SET(*r_flistp, PIN_FLD_ERROR_DESCR, "Search: GLID Missing Error", ebufp);
                return;
        }
        if (i_glid &&!strcmp(i_glid, "%") == 0)
        {
                sprintf(pr_glid, "%s", i_glid);
                PIN_ERR_LOG_MSG(3, pr_glid);
        }

        if (i_glid && strcmp(i_glid, "%") == 0)
        {
                *r_flistp = PIN_FLIST_CREATE(ebufp);
                *r_flistp = PIN_FLIST_COPY(res_flistp, ebufp);
                return;
        }

        elem_id = 0;
        cookie = NULL;
        while ((rslt_flistp = PIN_FLIST_ELEM_TAKE_NEXT(res_flistp, PIN_FLD_RESULTS, &elem_id, 1, &cookie, ebufp)) != NULL)
        {
                PIN_ERR_LOG_FLIST(3, "results getting flist:", rslt_flistp);

                elem_id1 = 0;
                cookie1 = NULL;
                while ((gl_ids_rflistp = PIN_FLIST_ELEM_GET_NEXT(rslt_flistp, PIN_FLD_GL_IDS, &elem_id1, 1, &cookie1, ebufp)) != NULL)
                {
                        cnt = *(int32 *)&elem_id1;
                        sprintf(elem_glid, "%d", elem_id1);
                        PIN_ERR_LOG_MSG(3, elem_glid);
                        if (elem_glid && pr_glid && strncmp(pr_glid, elem_glid, strlen(pr_glid)) == 0)
                        {
                                rslt1_flistp = PIN_FLIST_ELEM_ADD(ret_flistp, PIN_FLD_RESULTS, elem_cnt, ebufp);
                                PIN_FLIST_ELEM_PUT(rslt1_flistp, gl_ids_rflistp, PIN_FLD_GL_IDS, cnt, ebufp);
                                PIN_FLIST_FLD_COPY(rslt_flistp, PIN_FLD_POID, rslt1_flistp, PIN_FLD_POID, ebufp);
                                PIN_FLIST_FLD_COPY(rslt_flistp, PIN_FLD_CREATED_T, rslt1_flistp, PIN_FLD_CREATED_T, ebufp);
                                PIN_FLIST_FLD_COPY(rslt_flistp, PIN_FLD_MOD_T, rslt1_flistp, PIN_FLD_MOD_T, ebufp);
                                PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"fm_glid_search ret_flistp", ret_flistp);
                        }
                        elem_cnt++;
                }
        }
        if (PIN_FLIST_ELEM_COUNT(ret_flistp, PIN_FLD_RESULTS, ebufp) < 1)
        {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"ERROR! GL Prefix Not Found", ebufp);
                PIN_ERRBUF_RESET(ebufp);
                *r_flistp = PIN_FLIST_CREATE(ebufp);
                PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, *r_flistp, PIN_FLD_POID, ebufp);
                PIN_FLIST_FLD_SET(*r_flistp, PIN_FLD_STATUS, &search_fail, ebufp);
                PIN_FLIST_FLD_SET(*r_flistp, PIN_FLD_ERROR_CODE, "51221", ebufp);
                PIN_FLIST_FLD_SET(*r_flistp, PIN_FLD_ERROR_DESCR, "ERROR! GL Prefix Not Found", ebufp);
                return;
        }
CLEANUP:
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_rbs_prefix_srch output flist", ret_flistp);
        *r_flistp = PIN_FLIST_COPY(ret_flistp, ebufp);
        //PIN_FLIST_DESTROY_EX(&ret_flistp, NULL);
        return;
}

void
fm_rbs_gl_seg_for_actions(
        pcm_context_t           *ctxp,
        pin_flist_t             *i_flistp,
        pin_flist_t             **r_flistp,
        pin_errbuf_t            *ebufp)
{
        pin_flist_t             *in_flistp = NULL;
        pin_flist_t             *ret_flistp = NULL;
        pin_flist_t             *so_flistp = NULL;
        pin_flist_t             *rslt_flistp = NULL;
        pin_flist_t             *args_flistp = NULL;
        pin_flist_t             *gl_seg_iflistp = NULL;
        poid_t                  *vp = NULL;
        poid_t                  *pppdp = NULL;
        pin_cookie_t            cookie = NULL;
        pin_cookie_t            cookie1 = NULL;
        int32                   elem_id = 0;
        int32                   elem_id1 = 0;
        int64                   db = -1;
        int32                   *action = NULL;
        int32                   flags = 256;
        char                    *gl_segment = NULL;
        char                    msg[100];
        char                    *valp = NULL;
        char                    *templatep = "select X from /config where F1 = V1";
        char                    *template = "select X from /au_config where F1 = V1 order by created_t desc";

        if (PIN_ERRBUF_IS_ERR(ebufp))
                return;
        PIN_ERRBUF_CLEAR(ebufp);
	PIN_ERR_LOG_FLIST(3, "fm_glid_search main input", i_flistp);
        if (PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_AU_PARENT_OBJ, 1, ebufp) != NULL)
        {
                vp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_AU_PARENT_OBJ, 0, ebufp);
        }
        else
        {
                vp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp);
        }
        action = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_ACTION_MODE, 0, ebufp);
        db = PIN_POID_GET_DB(vp);
        in_flistp = PIN_FLIST_CREATE(ebufp);
        PIN_FLIST_FLD_SET(in_flistp, PIN_FLD_POID, PIN_POID_CREATE(db, "/search", -1, ebufp), ebufp);
        PIN_FLIST_FLD_SET(in_flistp, PIN_FLD_FLAGS, &flags, ebufp);

        if (action && *action != 4)
        {
                PIN_FLIST_FLD_SET(in_flistp, PIN_FLD_TEMPLATE, templatep, ebufp);
                args_flistp = PIN_FLIST_ELEM_ADD(in_flistp, PIN_FLD_ARGS, 1, ebufp);
                PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_POID, PIN_POID_CREATE(db, "/config/gl_segment", -1, ebufp), ebufp);
        }
        else
        {
                PIN_FLIST_FLD_SET(in_flistp, PIN_FLD_TEMPLATE, template, ebufp);
                args_flistp = PIN_FLIST_ELEM_ADD(in_flistp, PIN_FLD_ARGS, 1, ebufp);
                PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_POID, PIN_POID_CREATE(db, "/au_config/gl_segment", -1, ebufp), ebufp);
        }

        rslt_flistp = PIN_FLIST_ELEM_ADD(in_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, ebufp);

        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"fm_glid_search: search input flist", in_flistp);
        PCM_OP(ctxp, PCM_OP_SEARCH, 0, in_flistp, &so_flistp, ebufp);
        if (PIN_ERRBUF_IS_ERR(ebufp))
        {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "ERROR! GL_SEGMENT Not Found", ebufp);
                PIN_ERRBUF_RESET(ebufp);
                goto CLEANUP;
        }
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"fm_gl_segment_search: search output flist ", so_flistp);
        *r_flistp = PIN_FLIST_CREATE(ebufp);
        while ((rslt_flistp = PIN_FLIST_ELEM_GET_NEXT(so_flistp, PIN_FLD_RESULTS, &elem_id, 1, &cookie, ebufp)) != NULL)
        {
                PIN_ERR_LOG_MSG(3, "Inside loop");
                valp = PIN_FLIST_FLD_GET(rslt_flistp, PIN_FLD_VALUE, 0, ebufp);
                poid_t* pdp = PIN_POID_FROM_STR(valp, NULL, ebufp);
                PIN_ERR_LOG_POID(3, "glid poid: ", pdp);
		PIN_ERR_LOG_POID(3, "au parent object: ", vp);
                if (PIN_POID_COMPARE(pdp, vp, 0, ebufp) == 0)
                {
                        gl_segment = PIN_FLIST_FLD_GET(rslt_flistp, PIN_FLD_NAME, 0, ebufp);
                        sprintf(msg, "gl_seg: %s", gl_segment);
                        PIN_ERR_LOG_MSG(3, msg);
                        PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, *r_flistp, PIN_FLD_POID, ebufp);
                        PIN_FLIST_FLD_SET(*r_flistp, PIN_FLD_SEGMENT_NAME, gl_segment, ebufp);
                        break;
                }
        }
        PIN_ERR_LOG_FLIST(3, "fm_rbs_gl_seg_for_actions return flist: ", *r_flistp);
        return;
        /*if (!gl_segment)
        {
                pin_set_err(ebufp, PIN_ERRLOC_FM, PIN_ERRCLASS_SYSTEM_DETERMINATE, PIN_ERR_NO_MATCH, PIN_FLD_CODE, 0, 0);
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "fm_rbs_gl_seg_for_actions error", ebufp);
                goto CLEANUP;
        }*/
CLEANUP:
        *r_flistp = PIN_FLIST_COPY(i_flistp, ebufp);
        PIN_FLIST_DESTROY_EX(&so_flistp, NULL);
        return;
}

