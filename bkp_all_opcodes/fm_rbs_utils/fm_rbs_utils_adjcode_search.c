#ifndef lint
static const char Sccs_id[] = "@(#)%Portal Version: fm_string_search.c:BillingVelocityInt:3:2006-Sep-05 21:55:03 %";
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

#define FILE_LOGNAME "fm_string_search.c(1.5)"
/************
*DEFINE FUNCTIONS
*************/


EXPORT_OP void
op_string_search(
    cm_nap_connection_t *connp,
    int32               opcode,
    int32               flags,
    pin_flist_t         *i_flistp,
    pin_flist_t         **r_flistpp,
    pin_errbuf_t        *ebufp);

static void
fm_string_search(
        pcm_context_t           *ctxp,
        pin_flist_t             *i_flistp,
        pin_flist_t             **r_flistpp,
        pin_errbuf_t            *ebufp);

static void
fm_glid_string_search(
        pcm_context_t           *ctxp,
        pin_flist_t             *i_flistp,
        pin_flist_t             **r_flistpp,
        pin_errbuf_t            *ebufp);

/***************
*FUNCTION IMPLIMENTATIONS
***************/
void
op_string_search(
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
    if (opcode != RBS_OP_ADJCODE_SEARCH) {
        pin_set_err(ebufp, PIN_ERRLOC_FM,
            PIN_ERRCLASS_SYSTEM_DETERMINATE,
            PIN_ERR_BAD_OPCODE, 0, 0, opcode);
        PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
            "op_string_search error",
            ebufp);
        return;
    }

 /***********************
 * Debug: Input flist
 ************************/
    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
     "op_string_search input", i_flistp);

 /**********************
 * Call the default implementation
 ************************/

    fm_string_search(ctxp,i_flistp,&r_flistp,ebufp);


/*********************
 * Results.
 ********************/

    if (PIN_ERRBUF_IS_ERR(ebufp))
    {
        PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "op_string_search error", ebufp);
    }
    else
    {
       *r_flistpp =PIN_FLIST_COPY(r_flistp, ebufp);
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "op_string_search: output flist::", *r_flistpp);
    }
    return;
}

static void
fm_string_search(
        pcm_context_t           *ctxp,
        pin_flist_t             *i_flistp,
        pin_flist_t             **r_flistpp,
        pin_errbuf_t            *ebufp)
{
        pin_flist_t            *srch_flistp = NULL;
        pin_flist_t            *arg_flist = NULL;
        pin_flist_t            *arg_flist1 = NULL;
        pin_flist_t            *result_flist = NULL;
        pin_flist_t            *srch_out_flistp = NULL;
        pin_flist_t            *srch_out_flistp1 = NULL;
        pin_flist_t            *results_flistp = NULL;
        pin_flist_t            *results_flist1 = NULL;
        pin_flist_t            *results_flistp2= NULL;
        pin_flist_t            *srch_out_flistp2 = NULL;
        pin_flist_t            *results_flistp3 = NULL;
        pin_flist_t            *results_flist2 = NULL;
        pin_cookie_t           cookie = NULL;
        pin_cookie_t           icookie = NULL;
      /*  char                   *template= "select x from /strings 1 where (1.F2=V2 or 1.F3=V3) and 1.F3 = V3 and domain in ('Reason Codes-Debit Reasons', 'Reason Codes-Credit Reasons')";*/

	char			*template= "select X from /strings 1, /config/map_glid 2 where (1.F1=V1 or 1.F2=V2) and 1.F3 = 2.F4 and 1.F5 = 2.F6 and strings_t.domain in ('Reason Codes-Debit Reasons', 'Reason Codes-Credit Reasons') and 1.F7 = V7";

        char                   msg[100];
        char                   *template1 = "select x from /config/map_glid 1, /strings 2 where (1.F1=V1 or 1.F2=V2) and 1.F3 = V3 and 1.F4 = 2.F5 and 1.F6 = 2.F7 and strings_t.domain in ('Reason Codes-Debit Reasons', 'Reason Codes-Credit Reasons')";
	char		       template2[256];
	char		       r_code[128] = "";
	char		       *reason_codep = NULL;
	char		       *streng_descr = NULL;
	char		       *string = NULL;
	int32		       glid_dummy = 0;
	int32		       ret_status = 0;
        int32                  flag = 256;
        int32                  var1 = 1;
        int32                  var2 = 101;
        int32                  *engstr_idp = NULL;
        int32                  *chnstr_idp = NULL;
	int32		       *streng_verp = NULL;
        int32                  rec_id = 0;
	int32		       *action_flags = NULL;
        int                    elem_id = 0;
        int                    ielem_id = 0;

        if (PIN_ERRBUF_IS_ERR(ebufp))
	{
                sprintf(msg,"%s: fm_string_search::ERROR");
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, msg, ebufp);
              goto CLEANUP;
        }

	action_flags = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_ACTION_MODE, 1, ebufp);
	if (action_flags && *action_flags != 4)
	{
		srch_flistp = PIN_FLIST_CREATE(ebufp);
        	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, srch_flistp, PIN_FLD_POID, ebufp);
       	 	PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_FLAGS, &flag, ebufp);
		action_flags = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_ACTION_MODE, 1, ebufp);
		if (action_flags && *action_flags == 1)
		{
			PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_TEMPLATE, template, ebufp);
                        arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 7, ebufp );
                        PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_STRING_ID, arg_flist, PIN_FLD_STRING_ID, ebufp);
		}
		else
		{
			if (PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_REASON_CODE, 1, ebufp))
                        {
                                reason_codep = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_REASON_CODE, 0, ebufp);
                                strcat(r_code, reason_codep);
                        }
                        strcat(r_code, "%");
                        sprintf(template2, "select X from /strings 1, /config/map_glid 2 where (1.F1=V1 or 1.F2=V2) and 1.F3 = 2.F4 and 1.F5 = 2.F6 and strings_t.domain in ('Reason Codes-Debit Reasons', 'Reason Codes-Credit Reasons') and strings_t.string_id like '%s'", r_code);
                        PIN_ERR_LOG_MSG(3, template2);
                        PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_TEMPLATE, &template2, ebufp);
		}
        	arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 1, ebufp );
        	PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_STR_VERSION, &var1, ebufp);
		arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 2, ebufp );
		PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_STR_VERSION, &var2 , ebufp);

		arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 3, ebufp );
                PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_STRING_ID, NULL, ebufp);
		arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 4, ebufp );
                arg_flist1 = PIN_FLIST_ELEM_ADD(arg_flist, PIN_FLD_CHOICES, PIN_ELEMID_ANY, ebufp );
                PIN_FLIST_FLD_SET(arg_flist1, PIN_FLD_STRING_ID, NULL, ebufp);

		arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 5, ebufp );
                PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_STR_VERSION, NULL, ebufp);
		arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 6, ebufp );
                arg_flist1 = PIN_FLIST_ELEM_ADD(arg_flist, PIN_FLD_CHOICES, PIN_ELEMID_ANY, ebufp );
                PIN_FLIST_FLD_SET(arg_flist1, PIN_FLD_STR_VERSION, NULL, ebufp);


       		result_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, ebufp );
        	PIN_FLIST_FLD_SET(result_flist, PIN_FLD_CREATED_T, NULL , ebufp);
        	PIN_FLIST_FLD_SET(result_flist, PIN_FLD_STRING_ID, NULL , ebufp);
        	PIN_FLIST_FLD_SET(result_flist, PIN_FLD_STRING, NULL , ebufp);
       	 	PIN_FLIST_FLD_SET(result_flist, PIN_FLD_STR_VERSION, NULL , ebufp);
        	PIN_FLIST_FLD_SET(result_flist, PIN_FLD_DOMAIN, NULL , ebufp);
        	PIN_FLIST_FLD_SET(result_flist, PIN_FLD_LOCALE, NULL , ebufp);
        	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Search strings input flist", srch_flistp);
        	PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_flistp, &srch_out_flistp, ebufp);
		PIN_FLIST_DESTROY_EX(&srch_flistp, NULL);
        	if (PIN_ERRBUF_IS_ERR(ebufp))
        	{
        	        PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling SEARCH", ebufp);
       		        goto CLEANUP;
        	}
        	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Search strings output flist", srch_out_flistp);
		results_flistp = PIN_FLIST_ELEM_GET (srch_out_flistp, PIN_FLD_RESULTS, 0, 1, ebufp);
		if (results_flistp == NULL)
		{
			ret_status = 1;
			*r_flistpp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_ERROR_CODE, "100010", ebufp);
			PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_ERROR_DESCR, "Adjustment code not found !", ebufp);
			PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_STATUS, &ret_status, ebufp);
			goto CLEANUP;
		}
		srch_out_flistp1 = PIN_FLIST_COPY(srch_out_flistp, ebufp);
		*r_flistpp = PIN_FLIST_CREATE(ebufp);
		elem_id = 0;
		cookie = NULL;
		rec_id = 0;
		while ((results_flist1 = PIN_FLIST_ELEM_TAKE_NEXT(srch_out_flistp, PIN_FLD_RESULTS, &elem_id, 1, &cookie, ebufp)) != NULL)
		{
			streng_verp = (int32 *) PIN_FLIST_FLD_GET(results_flist1, PIN_FLD_STR_VERSION, 0, ebufp);
			streng_descr = PIN_FLIST_FLD_GET(results_flist1, PIN_FLD_STRING, 0, ebufp);
			if (action_flags && *action_flags == 3)
			{
				if (strlen(PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_STRING, 1, ebufp)) > 1) 
					string = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_STRING, 0, ebufp);
				else
					string = NULL;
				
				if (string && streng_descr && *streng_verp == 1 && strstr(streng_descr, string))
				{
					PIN_FLIST_ELEM_PUT(*r_flistpp, results_flist1, PIN_FLD_RESULTS, rec_id, ebufp);
                                	rec_id++;
				}
				else
				{
					PIN_FLIST_DESTROY_EX(&results_flist1, NULL);
				}
			}
			else 
			{
				if (*streng_verp == 1)
				{
					PIN_FLIST_ELEM_PUT(*r_flistpp, results_flist1, PIN_FLD_RESULTS, rec_id, ebufp);
					rec_id++;
				}
				else
				{
					 PIN_FLIST_DESTROY_EX(&results_flist1, NULL);
				}
			}
		}
		if (PIN_FLIST_ELEM_COUNT(*r_flistpp, PIN_FLD_RESULTS, ebufp) == 0)
		{
			ret_status = 1;
                        *r_flistpp = PIN_FLIST_CREATE(ebufp);
                        PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_ERROR_CODE, "100010", ebufp);
                        PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_ERROR_DESCR, "Adjustment code not found !", ebufp);
                        PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_STATUS, &ret_status, ebufp);
                        goto CLEANUP;
		}
		elem_id = 0;
		cookie = NULL;
		rec_id = 0;
		while ((results_flist1 = PIN_FLIST_ELEM_GET_NEXT(*r_flistpp, PIN_FLD_RESULTS, &elem_id, 1, &cookie, ebufp)) != NULL)
		{
			engstr_idp = (int32 *) PIN_FLIST_FLD_GET(results_flist1, PIN_FLD_STRING_ID, 0, ebufp);
			icookie = NULL;
			ielem_id = 0;
			while ((results_flist2 = PIN_FLIST_ELEM_GET_NEXT(srch_out_flistp1, PIN_FLD_RESULTS, &ielem_id, 1, 
				&icookie, ebufp)) != NULL)
			{
				chnstr_idp = (int32 *) PIN_FLIST_FLD_GET(results_flist2, PIN_FLD_STRING_ID, 0, ebufp);
				streng_verp = (int32 *) PIN_FLIST_FLD_GET(results_flist2, PIN_FLD_STR_VERSION, 0, ebufp);
				if (*engstr_idp == *chnstr_idp && *streng_verp == 101)
				{
					PIN_FLIST_FLD_COPY(results_flist2, PIN_FLD_STRING, results_flist1, PIN_FLD_REASON_CODE, ebufp);
				}
			}
		}

		PIN_FLIST_DESTROY_EX(&srch_out_flistp, NULL);
		PIN_FLIST_DESTROY_EX(&srch_out_flistp1, NULL);
	  	
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "String search final oflist", *r_flistpp);
	        srch_flistp = PIN_FLIST_CREATE(ebufp);
	        PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, srch_flistp, PIN_FLD_POID, ebufp);
	        PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_FLAGS, &flag, ebufp);
	        PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_TEMPLATE, template1 , ebufp);

		 arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 4, ebufp );
                arg_flist1 = PIN_FLIST_ELEM_ADD(arg_flist, PIN_FLD_CHOICES, PIN_ELEMID_ANY, ebufp );
                PIN_FLIST_FLD_SET(arg_flist1, PIN_FLD_STRING_ID, NULL, ebufp);

		arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 5, ebufp );
		PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_STRING_ID, NULL, ebufp);

		arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 6, ebufp );
                arg_flist1 = PIN_FLIST_ELEM_ADD(arg_flist, PIN_FLD_CHOICES, PIN_ELEMID_ANY, ebufp );
                PIN_FLIST_FLD_SET(arg_flist1, PIN_FLD_STR_VERSION, NULL, ebufp);

		arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 7, ebufp );
                PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_STR_VERSION, NULL, ebufp);

		elem_id = 0;
		cookie = NULL;
		while ((results_flist1 = PIN_FLIST_ELEM_GET_NEXT(*r_flistpp, PIN_FLD_RESULTS, &elem_id, 1, &cookie, ebufp)) != NULL)
		{
	                PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_string_search glid enrichment iflist: ", results_flist1);
        	        engstr_idp = PIN_FLIST_FLD_GET(results_flist1, PIN_FLD_STRING_ID, 0, ebufp);
        		arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 1, ebufp );
        		arg_flist1 = PIN_FLIST_ELEM_ADD(arg_flist, PIN_FLD_CHOICES, PIN_ELEMID_ANY, ebufp );
        		//PIN_FLIST_FLD_COPY(results_flist1, PIN_FLD_STR_VERSION, arg_flist1, PIN_FLD_STR_VERSION, ebufp);
			PIN_FLIST_FLD_SET(arg_flist1, PIN_FLD_STR_VERSION, &var1, ebufp);

			arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 2, ebufp );
			arg_flist1 = PIN_FLIST_ELEM_ADD(arg_flist, PIN_FLD_CHOICES, PIN_ELEMID_ANY, ebufp );
			PIN_FLIST_FLD_SET(arg_flist1, PIN_FLD_STR_VERSION, &var2, ebufp);

	        	arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 3, ebufp );
	        	arg_flist1 = PIN_FLIST_ELEM_ADD(arg_flist, PIN_FLD_CHOICES, PIN_ELEMID_ANY, ebufp );
	        	PIN_FLIST_FLD_COPY(results_flist1, PIN_FLD_STRING_ID, arg_flist1, PIN_FLD_STRING_ID, ebufp);
	        	result_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, ebufp );
        		//arg_flist1 = PIN_FLIST_ELEM_ADD(result_flist, PIN_FLD_CHOICES, PIN_ELEMID_ANY, ebufp );
        		//PIN_FLIST_FLD_SET(arg_flist1, PIN_FLD_GL_ID, NULL , ebufp);
        		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "GLID search input list", srch_flistp);
       		 	PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_flistp, &srch_out_flistp1, ebufp);
        		if (PIN_ERRBUF_IS_ERR(ebufp))
        		{
        	        	PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling SEARCH", ebufp);
        	        	goto CLEANUP;
        		}
        		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "GL ID Search output flist", srch_out_flistp1);
			results_flistp = PIN_FLIST_ELEM_GET(srch_out_flistp1, PIN_FLD_RESULTS, 0, 1, ebufp);
			if (results_flistp == NULL)
			{
        	               PIN_FLIST_FLD_SET(results_flist1, PIN_FLD_GL_ID, &glid_dummy, ebufp);
				continue;
				/*ret_status = 1;
				*r_flistpp = PIN_FLIST_CREATE(ebufp);
				PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_ERROR_CODE, "100011", ebufp);
				PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_ERROR_DESCR, "GL ID not found for given Adjustment code", ebufp);
				PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_STATUS, &ret_status, ebufp);
				goto CLEANUP;
				*/
			}
	
        	        results_flistp2 = PIN_FLIST_ELEM_GET(srch_out_flistp1, PIN_FLD_RESULTS, 0, 1, ebufp );
        	        ielem_id = 0;
        	        icookie = NULL;
        	        while((results_flistp3 = PIN_FLIST_ELEM_GET_NEXT(results_flistp2, PIN_FLD_CHOICES, &ielem_id, 1, &icookie, ebufp )) != NULL)
        	        {
        	                chnstr_idp = PIN_FLIST_FLD_GET(results_flistp3, PIN_FLD_STRING_ID, 0, ebufp);
				streng_verp = (int32 *) PIN_FLIST_FLD_GET(results_flist1, PIN_FLD_STR_VERSION, 0, ebufp);
        	                if(*engstr_idp == *chnstr_idp && *streng_verp == 1 )
        	                {
        	                        PIN_FLIST_FLD_COPY(results_flistp3, PIN_FLD_GL_ID, results_flist1, PIN_FLD_GL_ID, ebufp);
					break;
        	                }
	       	         }
                	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_string_search glid enrichment oflist: ", results_flist1);
			PIN_FLIST_DESTROY_EX(&srch_out_flistp1, NULL);
		}
	}
	else
 	{
		fm_glid_string_search(ctxp, i_flistp, r_flistpp, ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling fm_glid_string_search", ebufp);
                        goto CLEANUP;			
		}
	}
	PIN_ERR_LOG_FLIST(3, "Return Flist: ", *r_flistpp);
CLEANUP:
	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, *r_flistpp, PIN_FLD_POID, ebufp);
	PIN_FLIST_DESTROY_EX(&srch_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&srch_out_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&srch_out_flistp1, NULL);
	return;
}

void
fm_glid_string_search(
        pcm_context_t           *ctxp,
        pin_flist_t             *i_flistp,
        pin_flist_t             **r_flistpp,
        pin_errbuf_t            *ebufp)
{
        pin_flist_t            *srch_flistp = NULL;
        pin_flist_t            *arg_flist = NULL;
        pin_flist_t            *arg_flist1 = NULL;
        pin_flist_t            *result_flist = NULL;
        pin_flist_t            *srch_out_flistp = NULL;
        pin_flist_t            *srch_out_flistp1 = NULL;
	pin_flist_t	       *srch_out_flistp3 = NULL;
        pin_flist_t            *results_flistp = NULL;
        pin_flist_t            *results_flist1 = NULL;
        pin_flist_t            *results_flistp2= NULL;
        pin_flist_t            *srch_out_flistp2 = NULL;
        pin_flist_t            *results_flistp3 = NULL;
        pin_flist_t            *results_flist2 = NULL;
        pin_cookie_t           cookie = NULL;
        pin_cookie_t           icookie = NULL;
        char                   *template= "select x from /strings 1 where 1.F1=V1 and 1.F2=V2 and domain in ('Reason Codes-Debit Reasons', 'Reason Codes-Credit Reasons')";
        char                   *string_id = NULL;
        char                   *template1 = "select x from /config/map_glid 1 where 1.F1=V1 and (1.F2=V2 or 1.F3=V3)";
        char                   msg[100];
        int32                  ret_status = 0;
	int32		       *glid = NULL;
	int32		       *glid1 = NULL;
        int32                  flag = 256;
        int32                  ver1 = 1;
        int32                  ver2 = 101;
        int32                  *version = NULL;
        int32                  *engstr_idp = NULL;
        int32                  *chnstr_idp = NULL;
        int32                  *streng_verp = NULL;
        int32                  rec_id = 0;
	int32		       rec_id1 = 0;
        int32                  *action_flags = NULL;
        int                    elem_id = 0;
        int                    ielem_id = 0;

        if (PIN_ERRBUF_IS_ERR(ebufp)){
              sprintf(msg,"%s: fm_string_search::ERROR");
              PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, msg, ebufp);
              goto CLEANUP;
        }

	glid = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_GL_ID, 0, ebufp);
        srch_flistp = PIN_FLIST_CREATE(ebufp);
        PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, srch_flistp, PIN_FLD_POID, ebufp);
        PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_FLAGS, &flag, ebufp);
        PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_TEMPLATE, template1 , ebufp);

        arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 1, ebufp);
        arg_flist1 = PIN_FLIST_ELEM_ADD(arg_flist, PIN_FLD_CHOICES, PIN_ELEMID_ANY, ebufp);
        PIN_FLIST_FLD_SET(arg_flist1, PIN_FLD_GL_ID, glid, ebufp);

        arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 2, ebufp);
        arg_flist1 = PIN_FLIST_ELEM_ADD(arg_flist, PIN_FLD_CHOICES, PIN_ELEMID_ANY, ebufp);
        PIN_FLIST_FLD_SET(arg_flist1, PIN_FLD_STR_VERSION, &ver1, ebufp);

        arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 3, ebufp);
        arg_flist1 = PIN_FLIST_ELEM_ADD(arg_flist, PIN_FLD_CHOICES, PIN_ELEMID_ANY, ebufp);
        PIN_FLIST_FLD_SET(arg_flist1, PIN_FLD_STR_VERSION, &ver2, ebufp);

        result_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, ebufp);
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "GLID search input list", srch_flistp);
        PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_flistp, &srch_out_flistp1, ebufp);
        if (PIN_ERRBUF_IS_ERR(ebufp))
        {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling SEARCH", ebufp);
                goto CLEANUP;
        }
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "GL ID Search output flist", srch_out_flistp1);
        results_flistp = PIN_FLIST_ELEM_GET(srch_out_flistp1, PIN_FLD_RESULTS, 0, 1, ebufp);
        if (results_flistp == NULL)
        {
                ret_status = 1;
                *r_flistpp = PIN_FLIST_CREATE(ebufp);
                PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_ERROR_CODE, "100011", ebufp);
                PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_ERROR_DESCR, "GL ID not found for given Adjustment code", ebufp);
                PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_STATUS, &ret_status, ebufp);
                goto CLEANUP;
        }

        results_flistp2 = PIN_FLIST_ELEM_GET(srch_out_flistp1, PIN_FLD_RESULTS, 0, 1, ebufp );
	srch_flistp = PIN_FLIST_CREATE(ebufp);
	srch_out_flistp1 = PIN_FLIST_CREATE(ebufp);
	srch_out_flistp3 = PIN_FLIST_CREATE(ebufp);
	*r_flistpp = PIN_FLIST_CREATE(ebufp);
        PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, srch_flistp, PIN_FLD_POID, ebufp);
        PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_FLAGS, &flag, ebufp);
        PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_TEMPLATE, template, ebufp);
        ielem_id = 0;
        icookie = NULL;
        while ((results_flistp3 = PIN_FLIST_ELEM_GET_NEXT(results_flistp2, PIN_FLD_CHOICES, &ielem_id, 1, &icookie, ebufp )) != NULL)
        {
                string_id = PIN_FLIST_FLD_GET(results_flistp3, PIN_FLD_STRING_ID, 0, ebufp);
                version = PIN_FLIST_FLD_GET(results_flistp3, PIN_FLD_STR_VERSION, 0, ebufp);
		glid1 = PIN_FLIST_FLD_GET(results_flistp3, PIN_FLD_GL_ID, 0, ebufp);
                 
		arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 1, ebufp);
                PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_STRING_ID, string_id, ebufp);
		arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 2, ebufp);
                PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_STR_VERSION, version, ebufp);
                
		result_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, ebufp );
                PIN_FLIST_FLD_SET(result_flist, PIN_FLD_CREATED_T, NULL , ebufp);
                PIN_FLIST_FLD_SET(result_flist, PIN_FLD_STRING_ID, NULL , ebufp);
                PIN_FLIST_FLD_SET(result_flist, PIN_FLD_STRING, NULL , ebufp);
                PIN_FLIST_FLD_SET(result_flist, PIN_FLD_STR_VERSION, NULL , ebufp);
                PIN_FLIST_FLD_SET(result_flist, PIN_FLD_DOMAIN, NULL , ebufp);
                PIN_FLIST_FLD_SET(result_flist, PIN_FLD_LOCALE, NULL , ebufp);
                PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_glid_string_search input list", srch_flistp);
                PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_flistp, &srch_out_flistp, ebufp);
                if (PIN_ERRBUF_IS_ERR(ebufp))
                {
                        PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling SEARCH", ebufp);
                        goto CLEANUP;
                }
                PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_glid_string_search output flist", srch_out_flistp);
		srch_out_flistp3 = PIN_FLIST_COPY(srch_out_flistp, ebufp);
                results_flistp = PIN_FLIST_ELEM_GET(srch_out_flistp, PIN_FLD_RESULTS, 0, 1, ebufp);
		results_flist2 = PIN_FLIST_ELEM_TAKE(srch_out_flistp3, PIN_FLD_RESULTS, 0, 1, ebufp);
                if (results_flistp != NULL)
		{
			PIN_FLIST_ELEM_PUT(srch_out_flistp1, results_flist2, PIN_FLD_RESULTS, rec_id1, ebufp);
			rec_id1++;	
                	results_flist1 = PIN_FLIST_ELEM_TAKE(srch_out_flistp, PIN_FLD_RESULTS, 0, 1, ebufp);
                	streng_verp = (int32 *) PIN_FLIST_FLD_GET(results_flist1, PIN_FLD_STR_VERSION, 0, ebufp);
                	if (glid && glid1 && *glid == *glid1 && *streng_verp == 1 )
                	{
				PIN_FLIST_ELEM_PUT(*r_flistpp, results_flist1, PIN_FLD_RESULTS, rec_id, ebufp);
				PIN_FLIST_FLD_SET(results_flist1, PIN_FLD_GL_ID, glid1, ebufp);
			        rec_id++;	
                	}
                	else
                	{
                	        PIN_FLIST_DESTROY_EX(&results_flist1, NULL);
                	}
		}
	}
	PIN_ERR_LOG_FLIST(3, "String search ret flist:", *r_flistpp);
        elem_id = 0;
        cookie = NULL;
        rec_id = 0;
        while ((results_flist1 = PIN_FLIST_ELEM_GET_NEXT(*r_flistpp, PIN_FLD_RESULTS, &elem_id, 1, &cookie, ebufp)) != NULL)
        {
       		engstr_idp = (int32 *) PIN_FLIST_FLD_GET(results_flist1, PIN_FLD_STRING_ID, 0, ebufp);
                icookie = NULL;
               	ielem_id = 0;
               	while ((results_flist2 = PIN_FLIST_ELEM_GET_NEXT(srch_out_flistp1, PIN_FLD_RESULTS, &ielem_id, 1, &icookie, ebufp)) != NULL)
               	{
               	        chnstr_idp = (int32 *) PIN_FLIST_FLD_GET(results_flist2, PIN_FLD_STRING_ID, 0, ebufp);
               	        streng_verp = (int32 *) PIN_FLIST_FLD_GET(results_flist2, PIN_FLD_STR_VERSION, 0, ebufp);
               	        if (*engstr_idp == *chnstr_idp && *streng_verp == 101)
               	        {
        	        	PIN_FLIST_FLD_COPY(results_flist2, PIN_FLD_STRING, results_flist1, PIN_FLD_REASON_CODE, ebufp);
			}
        	}
	}
        PIN_FLIST_DESTROY_EX(&srch_out_flistp, NULL);
        PIN_FLIST_DESTROY_EX(&srch_out_flistp1, NULL);
	PIN_FLIST_DESTROY_EX(&srch_out_flistp3, NULL);
       	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "String search final oflist", *r_flistpp);
CLEANUP:
        PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, *r_flistpp, PIN_FLD_POID, ebufp);
        PIN_FLIST_DESTROY_EX(&srch_flistp, NULL);
        PIN_FLIST_DESTROY_EX(&srch_out_flistp, NULL);
        PIN_FLIST_DESTROY_EX(&srch_out_flistp1, NULL);
        return;
}

