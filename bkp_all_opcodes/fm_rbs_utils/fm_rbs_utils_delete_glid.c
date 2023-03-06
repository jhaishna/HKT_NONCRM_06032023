#ifndef lint
static const char Sccs_id[] = "@(#)%Portal Version: fm_rbs_utils_delete_glid.c:CUPmod7.3PatchInt:1:2006-Dec-01 16:31:18 %";
#endif

#include <stdio.h>
#include <string.h>
#include <time.h>
#include "pcm.h"
#include "ops/act.h"
#include "ops/cust.h"
#include "ops/bill.h"
#include "ops/device.h"
#include "pin_bill.h"
#include "cm_fm.h"
#include "pin_errs.h"
#include "pinlog.h"
#include "fm_utils.h"
#include "pin_cust.h"
#include "pin_pymt.h"
#include "pin_inv.h"
#include "str_adj_code_search.h"

/*******************************************************************
 * Fuctions defined in this code
 *******************************************************************/

EXPORT_OP void
op_rbs_utils_delete_glid(
        cm_nap_connection_t             *connp,
        int32                           opcode,
        int32                           flags,
        pin_flist_t                     *in_flistp,
        pin_flist_t                     **ret_flistpp,
        pin_errbuf_t                    *ebufp);

static void
fm_rbs_utils_delete_glid(
        pcm_context_t                   *ctxp,
        pin_flist_t                     *in_flistp,
        int32                           *error_code,
        pin_flist_t                     **r_flistp,
        pin_errbuf_t                    *ebufp);

static void
get_config_object(
        pcm_context_t                   *ctxp,
        pin_flist_t                     *i_flistpp,
        poid_t                     	**gl_pdp,
        pin_errbuf_t                    *ebufp);


/*******************************************************************
 * Main routine for the RBS_OP_GLID_DELETE  command
 *******************************************************************/
void
op_rbs_utils_delete_glid(
        cm_nap_connection_t             *connp,
        int32                           opcode,
        int32                           flags,
        pin_flist_t                     *in_flistp,
        pin_flist_t                     **ret_flistpp,
        pin_errbuf_t                    *ebufp)
{
        pin_flist_t                     *r_flistp = NULL;
        pcm_context_t                   *ctxp = connp->dm_ctx;
        int32                           error_code = 0;


    /***********************************************************
     * Null out results until we have some.
     ***********************************************************/
    *ret_flistpp = NULL;
    PIN_ERRBUF_CLEAR(ebufp);

    /***********************************************************
     * Insanity check.
     ***********************************************************/
    if (opcode != RBS_OP_GLID_DELETE ) {
        pin_set_err(ebufp, PIN_ERRLOC_FM,
            PIN_ERRCLASS_SYSTEM_DETERMINATE,
                PIN_ERR_BAD_OPCODE, 0, 0, opcode);
        PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
            "bad opcode in op_rbs_utils_delete_glid", ebufp);
        return;
    }

    /***********************************************************
     * Log input flist
     ***********************************************************/
    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        "op_rbs_utils_delete_glid input flist", in_flistp);

    /***********************************************************
     * Call main function to do it
     ***********************************************************/
    fm_rbs_utils_delete_glid(ctxp, in_flistp, &error_code, &r_flistp, ebufp);

    /***********************************************************
     * Results.
     ***********************************************************/
    if (PIN_ERRBUF_IS_ERR(ebufp))
    {
        PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
            "op_rbs_utils_delete_glid error", ebufp);
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_ERROR,
                "op_rbs_utils_delete_glid: Failed Input Flist", in_flistp);
        PIN_ERRBUF_RESET(ebufp);
        PIN_FLIST_DESTROY_EX(&r_flistp, NULL);
    }
    else
    {
        *ret_flistpp = r_flistp;
        PIN_ERRBUF_RESET(ebufp);
    }

    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
         "op_rbs_utils_delete_glid return flist", *ret_flistpp);

    return;
}


void
fm_rbs_utils_delete_glid(
        pcm_context_t                   *ctxp,
        pin_flist_t                     *in_flistp,
        int32                           *error_code,
        pin_flist_t                     **out_flistp,
        pin_errbuf_t                    *ebufp)
{

        pin_flist_t                     *gl_acct_flistp = NULL;
        pin_flist_t                     *gl_acct_rflistp = NULL;
        pin_flist_t                     *gl_id_flistp = NULL;
        pin_flist_t                     *gl_oflistp = NULL;
        pin_flist_t                     *arr_flistp = NULL;
        pin_flist_t                     *rslt_flistp = NULL;
        pin_flist_t                     *rj_flistp = NULL;
        pin_flist_t                     *rj_oflistp = NULL;
	pin_flist_t                     *del_flistp = NULL;
        pin_flist_t                     *del_iflistp = NULL;
        pin_flist_t                     *del_oflistp = NULL;
        pin_flist_t                     *glid_flistp = NULL;
        pin_flist_t                     *write_iflistp = NULL;
        pin_flist_t                     *write_oflistp = NULL;
        pin_flist_t                     *rs_flistp = NULL;
        pin_flist_t                     *gl_accts_flistp = NULL;
        pin_cookie_t                    cookie = NULL;
        pin_cookie_t                    cookie1 = NULL;
        pin_cookie_t                    icookie = NULL;
        pin_cookie_t                    cookie2 = NULL;
        poid_t                          *gl_pdp = NULL;
        char                            *bill_type = NULL;
        char                            *Creditworksorder = NULL;
        char                            *CreditCostCenterCode = NULL;
        char                            *CreditGLaccountcode = NULL;
        char                            *Debitworksorder = NULL;
        char                            *DebitCostCenterCode = NULL;
        char                            *DebitGLaccountc = NULL;
        char                            *gl_ar_accts = NULL;
        char                            *gl_offset_acct = NULL;
        char                            *attr_name = NULL;
	char				ar_acct[100];
	char				*ar_acctp = NULL;
	char				*offset_acctp = NULL;
	char				offset_acct[100];
        char                            glid[100];
        char                            msg[100];
	int32				status = 1;
       // int32                           *in_glid = NULL;
	char				*in_glid = NULL;
	int32				search_fail = 1;
	int32				count = 0;
	int32				*suc_cnt = NULL;
        int32                           *attr = NULL;
        int32                           *type = NULL;
        int32                           gl_id = 0;
        int32                           elem_glid = 0;
        int32                           pre_glid = 0;
        int32                           rec_id = 0;
        int32                           elem_id2 = 0;
        int32                           elem_id = 0;
        int32                           elem_id1 = 0;

        if(PIN_ERRBUF_IS_ERR(ebufp))
	{
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                            "fm_rbs_utils_delete_glid: Error", ebufp);
                return;
        }
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_rbs_utils_delete_glid: Input flist", in_flistp);
	count = PIN_FLIST_ELEM_COUNT(in_flistp, PIN_FLD_RESULTS, ebufp);
	if (count < 1)
	{	
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "PIN_ERR_MISSING_ARG Mandatory field is missing in input flist", ebufp);
		return;
	}
	count = 0;
	del_oflistp = PIN_FLIST_CREATE(ebufp);
        PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_POID, del_oflistp, PIN_FLD_POID, ebufp);	

	rec_id = 0;
	icookie = NULL;
        while ((arr_flistp = PIN_FLIST_ELEM_GET_NEXT(in_flistp, PIN_FLD_RESULTS, &rec_id, 1, &icookie, ebufp)) != NULL)
        {
              //  in_glid = PIN_FLIST_FLD_GET(arr_flistp, PIN_FLD_GL_ID, 0, ebufp);
		in_glid = PIN_FLIST_FLD_GET(arr_flistp, PIN_FLD_GL_ACCT, 0, ebufp);
		sprintf(msg, "in_glid: %s", in_glid);
                PIN_ERR_LOG_MSG(3, msg);
		sprintf(msg, "Rec_id: %d", rec_id);
		PIN_ERR_LOG_MSG(3, msg);
		gl_pdp = NULL;
                //Call the function to get the config object
                get_config_object(ctxp, arr_flistp, &gl_pdp, ebufp);
                if(PIN_ERRBUF_IS_ERR(ebufp) || !gl_pdp)
                {
                        PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                                "fm_rbs_utils_delete_glid: Error", ebufp);
			rslt_flistp = PIN_FLIST_ELEM_ADD(del_oflistp, PIN_FLD_RESULTS, rec_id, ebufp);
    	                PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_POID, rslt_flistp, PIN_FLD_POID, ebufp);
    	                PIN_FLIST_FLD_SET(rslt_flistp, PIN_FLD_STATUS, &search_fail, ebufp);
   	                PIN_FLIST_FLD_SET(rslt_flistp, PIN_FLD_ERROR_CODE, "51221", ebufp);
   	                PIN_FLIST_FLD_SET(rslt_flistp, PIN_FLD_ERROR_DESCR, "Segment not found in Database", ebufp);
    	                PIN_ERR_LOG_FLIST(3, "Del Output flist:", del_oflistp);
                }
		PIN_ERR_LOG_FLIST(3, "fm_rbs_utils_delete_glid results: ", arr_flistp);	
		attr_name = PIN_FLIST_FLD_GET(arr_flistp, PIN_FLD_ACTION, 0, ebufp);
                Creditworksorder = (char *)PIN_FLIST_FLD_GET(arr_flistp, PIN_FLD_ORDER_ID, 0, ebufp);
                CreditCostCenterCode = (char *)PIN_FLIST_FLD_GET(arr_flistp, PIN_FLD_CODE, 0, ebufp);
                CreditGLaccountcode = (char *)PIN_FLIST_FLD_GET(arr_flistp, PIN_FLD_GLACCOUNT, 0, ebufp);
                Debitworksorder = (char *)PIN_FLIST_FLD_GET(arr_flistp, PIN_FLD_STRING, 0, ebufp);
                DebitCostCenterCode = (char *)PIN_FLIST_FLD_GET(arr_flistp, PIN_FLD_CODE_STR, 0, ebufp);
                DebitGLaccountc = (char *)PIN_FLIST_FLD_GET(arr_flistp, PIN_FLD_ACCOUNT_CODE, 0, ebufp);
                bill_type = PIN_FLIST_FLD_GET(arr_flistp, PIN_FLD_STATUS_STR, 0, ebufp);	
		sprintf(offset_acct, "%s%s%s", Debitworksorder, DebitCostCenterCode, DebitGLaccountc);
                PIN_ERR_LOG_MSG(3, offset_acct);
                offset_acctp = (char *)offset_acct;
                sprintf(ar_acct, "%s%s%s", Creditworksorder, CreditCostCenterCode, CreditGLaccountcode);
                PIN_ERR_LOG_MSG(3, ar_acct);
                ar_acctp = (char *)ar_acct;   
 
		if (gl_pdp)
		{
	        	rj_flistp = PIN_FLIST_CREATE(ebufp);
                	PIN_FLIST_FLD_SET(rj_flistp, PIN_FLD_POID, gl_pdp, ebufp);
                	PCM_OP(ctxp, PCM_OP_READ_OBJ, PCM_OPFLG_ORDER_BY_REC_ID, rj_flistp, &rj_oflistp, ebufp);
                	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Read object output flist: ", rj_oflistp);

			elem_id1 = 0;
			cookie1 = NULL;
                	while ((glid_flistp = PIN_FLIST_ELEM_GET_NEXT(rj_oflistp, PIN_FLD_GL_IDS, &elem_id1, 1, &cookie1, ebufp )) != NULL)
                	{
				sprintf(glid, "%d", (int32)elem_id1);
                        //	sprintf(msg, "%d", *in_glid);
                        	PIN_ERR_LOG_MSG(3, glid);
                        //	PIN_ERR_LOG_MSG(3, msg);
				PIN_ERR_LOG_MSG(3, in_glid);
                       // 	if (in_glid && (strcmp(glid, msg) == 0))
				if (in_glid && (strcmp(glid, in_glid) == 0))
                        	{
					PIN_ERR_LOG_MSG(3, "GLID matched");
					elem_id2 = 0;
					cookie2 = NULL;
                                	while ((gl_acct_flistp = PIN_FLIST_ELEM_GET_NEXT(glid_flistp, PIN_FLD_GL_ACCTS, &elem_id2, 1, &cookie2, ebufp )) != NULL)
                                	{
                                        	attr = PIN_FLIST_FLD_GET(gl_acct_flistp, PIN_FLD_ATTRIBUTE, 0, ebufp);
                                        	gl_ar_accts = PIN_FLIST_FLD_GET(gl_acct_flistp, PIN_FLD_GL_AR_ACCT, 0, ebufp);
                                        	gl_offset_acct = PIN_FLIST_FLD_GET(gl_acct_flistp, PIN_FLD_GL_OFFSET_ACCT, 0, ebufp);
						type = PIN_FLIST_FLD_GET(gl_acct_flistp, PIN_FLD_TYPE, 0, ebufp);
					
						sprintf(msg, "ar_acctp: %s, gl_ar_accts: %s, offset_acctp: %s, gl_offset_acct: %s", ar_acctp, gl_ar_accts, offset_acctp, gl_offset_acct);
						PIN_ERR_LOG_MSG(3, msg);	
                                        	if (gl_ar_accts && ar_acctp && gl_offset_acct && offset_acctp && (strcmp(ar_acctp, gl_ar_accts) == 0) && (strcmp(offset_acctp, gl_offset_acct) == 0))
                                        	{
							sprintf(msg, "attr: %d, attr_name: %s, bill_type: %s, type: %d", *attr, attr_name, bill_type, *type);
							PIN_ERR_LOG_MSG(3, msg);
                                        		if (attr && ((strcmp(attr_name, "NET") == 0) && *attr == 1) 
							|| ((strcmp(attr_name, "DISC") == 0) && *attr == 2) || ((strcmp(attr_name, "TAX") == 0) 
							&& *attr == 4) || ((strcmp(attr_name, "GROSS") == 0) && *attr == 8))
                                        		{	
                                       				sprintf(msg, "bill_type: %s, type: %d", bill_type, *type);
								PIN_ERR_LOG_MSG(3, msg); 
                                        			if (bill_type && type && (((strcmp(bill_type, "UNBILLED") == 0) && *type == 1) || 
								((strcmp(bill_type, "BILLED") == 0) && *type == 2) || 
								((strcmp(bill_type, "UNBILLED_UNEARNED") == 0) && *type == 4) || 
								((strcmp(bill_type, "UNBILLED_EARNED") == 0) && *type == 8) ||
                                                		((strcmp(bill_type, "BILLED_UNEARNED") == 0) && *type == 16) || 
								((strcmp(bill_type, "BILLED_EARNED") == 0) && *type == 32) || 
								((strcmp(bill_type, "PREV_BILLED_EARNED") == 0) && *type == 64)))
                                        			{
									PIN_ERR_LOG_MSG(3, "Delete result");
									del_iflistp = PIN_FLIST_CREATE(ebufp);
                	                        			PIN_FLIST_FLD_SET(del_iflistp, PIN_FLD_POID, gl_pdp, ebufp);
        	                                			gl_id_flistp = PIN_FLIST_ELEM_ADD(del_iflistp, PIN_FLD_GL_IDS, elem_id1, ebufp);
	                                        			gl_acct_rflistp = PIN_FLIST_ELEM_ADD(gl_id_flistp, PIN_FLD_GL_ACCTS, elem_id2, ebufp);
									PIN_FLIST_FLD_SET(gl_acct_rflistp,PIN_FLD_GL_AR_ACCT, ar_acctp, ebufp);
									PIN_FLIST_FLD_SET(gl_acct_rflistp, PIN_FLD_GL_OFFSET_ACCT, offset_acctp, ebufp);
									PIN_FLIST_FLD_SET(gl_acct_rflistp, PIN_FLD_ATTRIBUTE, attr, ebufp);
									PIN_FLIST_FLD_SET(gl_acct_rflistp, PIN_FLD_TYPE, type, ebufp);
									PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Del fields input flist: ", del_iflistp);
                                        				PCM_OP(ctxp, PCM_OP_DELETE_FLDS,  0, del_iflistp, &del_flistp, ebufp);
                                        				if(PIN_ERRBUF_IS_ERR(ebufp))
                                        				{
                                        			        	PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                                        			        	        "fm_rbs_utils_delete_glid: Error", ebufp);
                                        			        	goto cleanup;
                                        				}
                                        				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Del fields output flist: ", del_flistp);
									status = 0;
									count = count + 1;
									sprintf(msg, "count: %d", count);
									PIN_ERR_LOG_MSG(3, msg);
					                        	rslt_flistp = PIN_FLIST_ELEM_ADD(del_oflistp, PIN_FLD_RESULTS, rec_id, ebufp);
                					        	PIN_FLIST_FLD_COPY(del_flistp, PIN_FLD_POID, rslt_flistp, PIN_FLD_POID, ebufp);
                        						PIN_FLIST_FLD_SET(rslt_flistp, PIN_FLD_STATUS, &status, ebufp);
                        						PIN_FLIST_FLD_SET(rslt_flistp, PIN_FLD_DESCR, "Deleted Successfully", ebufp);
									PIN_ERR_LOG_FLIST(3, "Del Output flist:", del_oflistp);
								}	
							}
						}
                                	}
                        	}
                	}
		}
		if (gl_pdp && del_flistp == NULL)
                {
                        rslt_flistp = PIN_FLIST_ELEM_ADD(del_oflistp, PIN_FLD_RESULTS, rec_id, ebufp);
                        PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_POID, rslt_flistp, PIN_FLD_POID, ebufp);
                        PIN_FLIST_FLD_SET(rslt_flistp, PIN_FLD_STATUS, &search_fail, ebufp);
                        PIN_FLIST_FLD_SET(rslt_flistp, PIN_FLD_ERROR_CODE, "51220", ebufp);
                        PIN_FLIST_FLD_SET(rslt_flistp, PIN_FLD_ERROR_DESCR, "No matching record found in Database", ebufp);
			PIN_ERR_LOG_FLIST(3, "Del Output flist:", del_oflistp);
                }
		PIN_FLIST_DESTROY_EX(&del_flistp, NULL);
        }
	PIN_FLIST_FLD_SET(del_oflistp, PIN_FLD_COUNT, &count, ebufp);
cleanup:
        *out_flistp = PIN_FLIST_COPY(del_oflistp, ebufp);
	PIN_ERR_LOG_FLIST(3, "glid delete outputflist", *out_flistp);
        PIN_FLIST_DESTROY_EX(&del_iflistp, NULL);
	PIN_FLIST_DESTROY_EX(&del_oflistp, NULL);
        return;
}

void
get_config_object(
        pcm_context_t                   *ctxp,
        pin_flist_t                     *i_flistpp,
        poid_t                          **gl_pdp,
        pin_errbuf_t                    *ebufp)
{
        pin_flist_t                     *s_iflistp = NULL;
        pin_flist_t                     *s_oflistp = NULL;
        pin_flist_t                     *args_flistp = NULL;
        pin_flist_t                     *glid_flistp = NULL;
        pin_flist_t                     *glid_accts_flistp = NULL;
        pin_flist_t                     *in_flistp = NULL;
        poid_t                          *config_pdp = NULL;
        poid_t                          *pdp = NULL;
        char                            *templ = "select X from /config/gl_segment where F1 = V1 ";
        char                            *gl_seg = NULL;
        char                            *valp = NULL;
        int64                           db = -1;
        int32                           flag = 256;
        int32                           count = 0;

        if(PIN_ERRBUF_IS_ERR(ebufp))
        {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                        "get_config_object: Error", ebufp);
                return;
        }
        gl_seg = PIN_FLIST_FLD_GET(i_flistpp, PIN_FLD_GL_SEGMENT, 0, ebufp);
        pdp = PIN_FLIST_FLD_GET(i_flistpp, PIN_FLD_POID, 0, ebufp);
        if(PIN_ERRBUF_IS_ERR(ebufp))
        {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                        "get_config_object: Error", ebufp);
                return;
        }
        s_iflistp = PIN_FLIST_CREATE(ebufp);
        db = PIN_POID_GET_DB(pdp);
        PIN_FLIST_FLD_PUT(s_iflistp, PIN_FLD_POID, (void *)PIN_POID_CREATE(db, "/search", -1, ebufp), ebufp);
        PIN_FLIST_FLD_SET(s_iflistp, PIN_FLD_FLAGS, (void *)&flag, ebufp);
        PIN_FLIST_FLD_SET(s_iflistp, PIN_FLD_TEMPLATE, (void *)templ, ebufp);
        //Add argument
        args_flistp = PIN_FLIST_ELEM_ADD(s_iflistp, PIN_FLD_ARGS, 1, ebufp);
	glid_flistp = PIN_FLIST_ELEM_ADD(args_flistp, PIN_FLD_GL_SEGMENTS, PIN_ELEMID_ANY, ebufp);
        PIN_FLIST_FLD_SET(glid_flistp, PIN_FLD_SEGMENT_NAME, gl_seg, ebufp);
        //Add Results
        args_flistp = PIN_FLIST_ELEM_ADD(s_iflistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, ebufp);
        PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_POID, NULL, ebufp);
        PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_CREATED_T, NULL, ebufp);
        PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_MOD_T, NULL, ebufp);
        PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_VALUE, NULL, ebufp);
        glid_flistp = PIN_FLIST_ELEM_ADD(args_flistp, PIN_FLD_GL_SEGMENTS, 0, ebufp);
        PIN_FLIST_FLD_SET(glid_flistp, PIN_FLD_SEGMENT_NAME, NULL, ebufp);
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "get_config_object: Search Input flist", s_iflistp);
        //Call the PCM_OP_SEARCH
        PCM_OP(ctxp, PCM_OP_SEARCH, 0, s_iflistp, &s_oflistp, ebufp);
        if(PIN_ERRBUF_IS_ERR(ebufp))
        {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "get_config_object: Error", ebufp);
                goto cleanup;
        }
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "get_config_object: Search Output flist", s_oflistp);
        count = PIN_FLIST_ELEM_COUNT(s_oflistp, PIN_FLD_RESULTS, ebufp);
        args_flistp = PIN_FLIST_ELEM_GET(s_oflistp, PIN_FLD_RESULTS, 0, 1, ebufp);
        if (args_flistp)
        {
                valp = PIN_FLIST_FLD_GET(args_flistp, PIN_FLD_VALUE, 0, ebufp);
                *gl_pdp = PIN_POID_FROM_STR(valp, NULL, ebufp);
        }
        if(count == 0){
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                        "get_config_object: Error BAD VALUE", ebufp);
                goto cleanup;
        }
cleanup:
        PIN_FLIST_DESTROY_EX(&s_iflistp, NULL);
        PIN_FLIST_DESTROY_EX(&s_oflistp, NULL);
        return;
}

