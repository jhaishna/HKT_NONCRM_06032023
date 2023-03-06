#ifndef lint
static  char Sccs_Id[] = "@(#)%Portal Version: fm_rbs_utils_common_functions.c:BillingVelocityInt:4:2006-Sep-05 04:28:17 %";
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


EXPORT_OP int32 
fm_rbs_trans_open(
	pcm_context_t	*ctxp,
	poid_t		*pdp,
	int32		flag,
	pin_errbuf_t	*ebufp);

EXPORT_OP void 
fm_rbs_trans_commit(
	pcm_context_t	*ctxp,
	poid_t		*pdp,
	pin_errbuf_t	*ebufp);

EXPORT_OP void 
fm_rbs_trans_abort(
	pcm_context_t	*ctxp,
	poid_t		*pdp,
	pin_errbuf_t	*ebufp);

void
fm_get_deal_from_name(
	pcm_context_t   *ctxp,
	pin_flist_t     *i_flistp,
	pin_flist_t     **r_flistp,
	pin_errbuf_t	*ebufp);

void
fm_search_billinfo_details(
	pcm_context_t      *ctxp,
	pin_flist_t        *i_flistp,
	pin_flist_t        **r_flistp,
	pin_errbuf_t       *ebufp);

void
fm_search_acct_details(
	pcm_context_t       *ctxp,
	pin_flist_t         *i_flistp,
	pin_flist_t         **r_flistp,
	pin_errbuf_t        *ebufp);

void
fm_get_service_info(
	pcm_context_t       *ctxp,
	pin_flist_t         *i_flistp,
	pin_flist_t         **r_flistp,
	pin_errbuf_t        *ebufp);

void
fm_get_purchased_product_info(
	pcm_context_t       *ctxp,
	pin_flist_t         *i_flistp,
	pin_flist_t         **r_flistp,
	pin_errbuf_t        *ebufp);
	
void 
fm_get_purchased_discount_info(
	pcm_context_t       *ctxp,
	pin_flist_t         *i_flistp,
	pin_flist_t         **r_flistp,
	pin_errbuf_t        *ebufp);

void
fm_rbs_get_payinfo(
	pcm_context_t       *ctxp,
	poid_t		    *acc_pdp,
	pin_flist_t         **r_flistp,
	pin_errbuf_t        *ebufp);


/**************************************************
* Function: 	fm_rbs_trans_open
* Decription:	For opening local transaction
* Input    
*          pdp  Poid to be locked
*          flag 0-READONLY
*               1-READWRITE
*               2-LOCK_OBJ
*
* Return   0   if transaction opened successfully
*          1   otherwise
***************************************************/

EXPORT_OP int32 
fm_rbs_trans_open(
	pcm_context_t	*ctxp,
	poid_t		*pdp,
	int32		flag,
	pin_errbuf_t	*ebufp)
{
	pin_flist_t	*t_flistp = NULL;
	pin_flist_t	*tr_flistp = NULL;
	pin_flist_t	*lock_flistp = NULL;
	pin_flist_t	*tar_flistp = NULL;

	int32		t_status = 0;

	t_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_SET(t_flistp, PIN_FLD_POID, (void *)pdp, ebufp);

	/* Open transaction */
	if (flag ==0 )
	{
		PCM_OP(ctxp, PCM_OP_TRANS_OPEN, PCM_TRANS_OPEN_READONLY, t_flistp, &tr_flistp, ebufp);
	}
	else if (flag ==1)
	{
		PCM_OP(ctxp, PCM_OP_TRANS_OPEN, PCM_TRANS_OPEN_READWRITE, t_flistp, &tr_flistp, ebufp);
	}
	else if (flag ==2)
	{
		PCM_OP(ctxp, PCM_OP_TRANS_OPEN, PCM_TRANS_OPEN_READWRITE, t_flistp, &tr_flistp, ebufp);
		
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Calling lock..");
	//	PCM_OPREF(ctxp, PCM_OP_LOCK_OBJ, PCM_TRANS_OPEN_LOCK_OBJ, t_flistp, &lock_flistp, ebufp);
	}
                else if (flag == 3)
        {

                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Invoked new flag inside utils..");
                PCM_OP(ctxp, PCM_OP_TRANS_OPEN,PCM_TRANS_OPEN_READWRITE | PCM_TRANS_OPEN_LOCK_DEFAULT, t_flistp, &tr_flistp, ebufp);
                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Calling new lock..");
        }
	else
	{
	PCM_OP(ctxp, PCM_OP_TRANS_OPEN, PCM_TRANS_OPEN_READWRITE, t_flistp, &tr_flistp, ebufp);
	}

	if(ebufp->pin_err == PIN_ERR_TRANS_ALREADY_OPEN)
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "fm_rbs_trans open:alrady open");
		PIN_ERRBUF_CLEAR(ebufp);
		t_status = 1;
	}
	else if(ebufp->pin_err != PIN_ERR_NONE)
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"fm_rbs_trans open:error", ebufp);
		t_status = 1;
	}
	else
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "fm_rbs_trans open:success");
	}

	/* cleanup */
	if(t_flistp)
		PIN_FLIST_DESTROY_EX(&t_flistp, NULL);
	if(tr_flistp)
		PIN_FLIST_DESTROY_EX(&tr_flistp, NULL);
	if(tar_flistp)
		PIN_FLIST_DESTROY_EX(&tar_flistp, NULL);
	if(lock_flistp)
		PIN_FLIST_DESTROY_EX(&lock_flistp, NULL);

	return t_status;
}

/**************************************************
* Function:     fm_rbs_trans_commit
* Decription:   For committing local transaction
***************************************************/

EXPORT_OP void 
fm_rbs_trans_commit(
	pcm_context_t	*ctxp,
	poid_t		*pdp,
	pin_errbuf_t	*ebufp)
{
	pin_flist_t	*t_flistp = NULL;
	pin_flist_t	*tr_flistp = NULL;

	t_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_SET(t_flistp, PIN_FLD_POID, (void *)pdp, ebufp);

	/* Commit transaction */
	PCM_OP(ctxp, PCM_OP_TRANS_COMMIT, 0, t_flistp, &tr_flistp, ebufp);

	if(PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"fm_rbs_trans commit error", ebufp);
	}
	else
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "fm_rbs_trans commit");
	}

	/* cleanup */
	if(t_flistp)
		PIN_FLIST_DESTROY_EX(&t_flistp, NULL);
	if(tr_flistp)
		PIN_FLIST_DESTROY_EX(&tr_flistp, NULL);

	return;
}

/**************************************************
* Function:     fm_rbs_trans_abort
* Decription:   For aborting local transaction
* Return   	0   if transaction opened successfully
*          	1   otherwise
***************************************************/
EXPORT_OP void 
fm_rbs_trans_abort(
	pcm_context_t	*ctxp,
	poid_t		*pdp,
	pin_errbuf_t	*ebufp)
{
	pin_flist_t	*t_flistp = NULL;
	pin_flist_t	*tr_flistp = NULL;


	if (PIN_ERRBUF_IS_ERR(ebufp) ) 
	{	
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"fm_rbs_trans abort error b4", ebufp);
		PIN_ERRBUF_CLEAR(ebufp);
	}

	PIN_ERR_LOG_POID(PIN_ERR_LEVEL_DEBUG, "POID to abort", pdp);
        t_flistp = PIN_FLIST_CREATE(ebufp);
        PIN_FLIST_FLD_SET(t_flistp, PIN_FLD_POID, (void *)pdp, ebufp);

	/* Open transaction */
	PCM_OP(ctxp, PCM_OP_TRANS_ABORT, 0, t_flistp, &tr_flistp, ebufp);

	if(ebufp->pin_err != PIN_ERR_NONE)
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"fm_rbs_trans abort error", ebufp);
	}
	else
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "fm_rbs_trans abort");
	}

	/* cleanup */
	if(t_flistp)
		PIN_FLIST_DESTROY_EX(&t_flistp, NULL);
	if(tr_flistp)
		PIN_FLIST_DESTROY_EX(&tr_flistp, NULL);

	return;
}


/******************************************************
fm_get_deal_from_name function to get deal info
******************************************************/
void
fm_get_deal_from_name(
        pcm_context_t   *ctxp,
        pin_flist_t     *i_flistp,
        pin_flist_t     **r_flistp,
        pin_errbuf_t    *ebufp)
{
        pin_flist_t     *input_flistp = NULL;
        pin_flist_t     *return_flistp = NULL;
        pin_flist_t     *search_iflistp  = NULL;
        pin_flist_t     *args_flistp = NULL;
        pin_flist_t     *result_flistp = NULL;
	pin_flist_t	*deal_flistp = NULL;
	pin_flist_t	*deal_infop = NULL;
        char            *name = NULL;
	char		*templatep = "select X from /deal where F1 = V1";
	char		msg[100];
	int32      	flags = 256;
	int32		cnt = 1;
	int32		elem_id = 0;
	int64		db = -1;
	poid_t		*pdp = NULL;
	poid_t		*search_pdp = NULL;
	pin_cookie_t	cookie = NULL;	

	PIN_ERRBUF_CLEAR(ebufp);
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_get_deal_from_name input_flist: ", i_flistp);
		
	pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp);
	db = PIN_POID_GET_DB(pdp);
	search_pdp = (poid_t *)PIN_POID_CREATE(db, "/search", -1, ebufp);

	search_iflistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_SET(search_iflistp, PIN_FLD_POID, search_pdp, ebufp);
	PIN_FLIST_FLD_SET(search_iflistp, PIN_FLD_FLAGS, &flags, ebufp);
        PIN_FLIST_FLD_SET(search_iflistp, PIN_FLD_TEMPLATE, templatep, ebufp);

	deal_infop = PIN_FLIST_ELEM_ADD(search_iflistp, PIN_FLD_ARGS, 1, ebufp);
	if (PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_NAME, 1, ebufp))
	{
		name = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_NAME, 0, ebufp);
		PIN_FLIST_FLD_SET(deal_infop, PIN_FLD_NAME, name, ebufp);
	}
	else
	{
		PIN_FLIST_FLD_SET(deal_infop, PIN_FLD_POID, pdp, ebufp);
	}
	result_flistp = PIN_FLIST_ELEM_ADD(search_iflistp, PIN_FLD_RESULTS, 0, ebufp);
		
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_search_input_flist: ", search_iflistp);
	PCM_OP(ctxp, PCM_OP_SEARCH, 1, search_iflistp, &return_flistp, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp))
        {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "PCM_OP_SEARCH error: ", ebufp);
                return;
        }
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_search_return_flist: ", return_flistp);
		 
	*r_flistp = PIN_FLIST_COPY(return_flistp,ebufp);
	return;
}

/**********************************
fm_search_billinfo_details function 
to get billinfo from account object
**********************************/ 
void		
fm_search_billinfo_details(
	pcm_context_t      *ctxp,
	pin_flist_t        *i_flistp,
	pin_flist_t        **r_flistp,
	pin_errbuf_t       *ebufp)
{
	pin_flist_t    	*input_flistp = NULL;
	pin_flist_t    	*return_flistp = NULL;
	pin_flist_t	*search_iflistp  = NULL;
	pin_flist_t	*args_flistp = NULL;
	pin_flist_t	*result_flistp = NULL;
	poid_t		*acct_obj = NULL;
	poid_t		*pdp = NULL;
	poid_t		*search_pdp = NULL;
	int32          	flags = 256;
	int64		db = -1;
	char		*templatep = "select X from /billinfo where F1 = V1";
	
	if (PIN_ERRBUF_IS_ERR(ebufp))
	        return;
	
        PIN_ERRBUF_CLEAR(ebufp);
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_search_billinfo_details input_flist: ", i_flistp);
		
	pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp);
	db = PIN_POID_GET_DB(pdp);
	search_pdp = (poid_t *)PIN_POID_CREATE(db, "/search", -1, ebufp);

	search_iflistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_SET(search_iflistp, PIN_FLD_POID, search_pdp, ebufp);
	PIN_FLIST_FLD_SET(search_iflistp, PIN_FLD_FLAGS, &flags, ebufp);
        PIN_FLIST_FLD_SET(search_iflistp, PIN_FLD_TEMPLATE, templatep, ebufp);
		
	args_flistp = PIN_FLIST_ELEM_ADD(search_iflistp, PIN_FLD_ARGS, 1, ebufp);
	if (PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_ACCOUNT_OBJ, 1, ebufp ))
	{
		acct_obj= PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_ACCOUNT_OBJ, 0, ebufp );
        	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_ACCOUNT_OBJ, acct_obj, ebufp);
	}
	if (PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_AR_BILLINFO_OBJ, 1, ebufp ))
        {
                acct_obj= PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_AR_BILLINFO_OBJ, 0, ebufp );
                PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_AR_BILLINFO_OBJ, acct_obj, ebufp);
        }
	
	result_flistp = PIN_FLIST_ELEM_ADD(search_iflistp, PIN_FLD_RESULTS, 0, ebufp);
	
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_search_input_flist: ", search_iflistp);
	PCM_OP(ctxp, PCM_OP_SEARCH, 1, search_iflistp, &return_flistp, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp))
        {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "PCM_OP_SEARCH error: ", ebufp);
                return;
        }
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_search_return_flist: ", return_flistp);
		 
	*r_flistp = PIN_FLIST_COPY(return_flistp,ebufp);
	return;
}
	
/***********************************
fm_search_acct_details function to 
get account info from account number
***********************************/
void	
fm_search_acct_details(
	pcm_context_t   *ctxp,
	pin_flist_t     *i_flistp,
	pin_flist_t     **r_flistp,
	pin_errbuf_t    *ebufp)
{
	pin_flist_t     *input_flistp = NULL;
	pin_flist_t     *return_flistp = NULL;
	pin_flist_t	*search_iflistp  = NULL;
	pin_flist_t	*args_flistp = NULL;
	pin_flist_t	*result_flistp = NULL;
	char		*acct_no = NULL;
	poid_t		*search_pdp = NULL;
	poid_t		*pdp = NULL;
	int32           flags = 256;
	int64		db = -1;
	char		*templatep = "select X from /account where F1 = V1";
		
	if (PIN_ERRBUF_IS_ERR(ebufp))
		return;
	
        PIN_ERRBUF_CLEAR(ebufp);
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_search_acct_details input_flist: ", i_flistp);
		
	pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp);
	db = PIN_POID_GET_DB(pdp);
	search_pdp = (poid_t *)PIN_POID_CREATE(db, "/search", -1, ebufp);

	search_iflistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_SET(search_iflistp, PIN_FLD_POID, search_pdp, ebufp);
	PIN_FLIST_FLD_SET(search_iflistp, PIN_FLD_FLAGS, &flags, ebufp);
        PIN_FLIST_FLD_SET(search_iflistp, PIN_FLD_TEMPLATE, templatep, ebufp);
		
	acct_no= PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_ACCOUNT_NO, 0, ebufp );
	args_flistp = PIN_FLIST_ELEM_ADD(search_iflistp, PIN_FLD_ARGS, 1, ebufp);
        PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_ACCOUNT_NO, acct_no, ebufp);
		
	result_flistp = PIN_FLIST_ELEM_ADD(search_iflistp, PIN_FLD_RESULTS, 0, ebufp);
		
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_search_input_flist: ", search_iflistp);
	PCM_OP(ctxp, PCM_OP_SEARCH, 1, search_iflistp, &return_flistp, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp))
        {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "PCM_OP_SEARCH error: ", ebufp);
                return;
        }
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_search_return_flist: ", return_flistp);
		 
	*r_flistp = PIN_FLIST_COPY(return_flistp,ebufp);
	return;
}

/******************************************
fm_get_service_info function to
get service info from login and poid_type
*******************************************/

void
fm_get_service_info(
	pcm_context_t	*ctxp,
	pin_flist_t	*i_flistp,
	pin_flist_t	**r_flistp,
	pin_errbuf_t	*ebufp)
{
	pin_flist_t	*input_flistp = NULL;
	pin_flist_t	*return_flistp = NULL;
	pin_flist_t	*search_iflistp  = NULL;
	pin_flist_t	*args_flistp = NULL;
	pin_flist_t	*args1_flistp = NULL;
	pin_flist_t	*result_flistp = NULL;
	poid_t		*search_pdp = NULL;
	poid_t		*acc_pdp = NULL;
	poid_t		*pdp = NULL;
	poid_t		*ser_pd = NULL;
	int32           flags = 256;
	int64		db = -1;
	char		*login = NULL;
	char		*poid_type = NULL;
	char		*template = "select X from /service where F1 = V1 and F2 = V2";
	
			
	if (PIN_ERRBUF_IS_ERR(ebufp))
        return;
		
        PIN_ERRBUF_CLEAR(ebufp);
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_get_service_info input_flist", i_flistp);
		
	pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp);
	db = PIN_POID_GET_DB(pdp);
	search_pdp = (poid_t *)PIN_POID_CREATE(db, "/search", -1, ebufp);

	search_iflistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_SET(search_iflistp, PIN_FLD_POID, search_pdp, ebufp);
	PIN_FLIST_FLD_SET(search_iflistp, PIN_FLD_FLAGS, &flags, ebufp);
        PIN_FLIST_FLD_SET(search_iflistp, PIN_FLD_TEMPLATE, template, ebufp);
		
	args_flistp = PIN_FLIST_ELEM_ADD(search_iflistp, PIN_FLD_ARGS, 1, ebufp);
		if (PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_ACCOUNT_OBJ, 1, ebufp) && PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_ACCOUNT_OBJ, 1, ebufp) != NULL)
	//	if (acc_pdp)
		{
			acc_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_ACCOUNT_OBJ, 0, ebufp);
			PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_ACCOUNT_OBJ, acc_pdp, ebufp);
		}
		else 
		{
			PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_LOGIN, args_flistp, PIN_FLD_LOGIN, ebufp);
		}
	args1_flistp = PIN_FLIST_ELEM_ADD(search_iflistp, PIN_FLD_ARGS, 2, ebufp);
	PIN_FLIST_FLD_SET(args1_flistp, PIN_FLD_POID, pdp, ebufp);
		
	result_flistp = PIN_FLIST_ELEM_ADD(search_iflistp, PIN_FLD_RESULTS, 0, ebufp);
		
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_get_service_info input_flist1", search_iflistp);
		
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, search_iflistp, &return_flistp, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp))
        {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "PCM_OP_SEARCH error: ", ebufp);
                return;
        }	
	
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_get_service_info return_flist1", return_flistp);
		 
	*r_flistp = PIN_FLIST_COPY(return_flistp,ebufp);
	return;
}

/*******************************************************
fm_get_purchased_details function to
get service info from login and poid_type
*******************************************************/
void
fm_get_purchased_product_info(
        pcm_context_t   *ctxp,
        pin_flist_t     *i_flistp,
        pin_flist_t     **r_flistp,
        pin_errbuf_t    *ebufp)
{
	pin_flist_t     *input_flistp = NULL;
	pin_flist_t     *return_flistp = NULL;
	pin_flist_t	*search_iflistp  = NULL;
	pin_flist_t	*args_flistp = NULL;
	pin_flist_t	*args1_flistp = NULL;
	pin_flist_t	*args2_flistp = NULL;
	pin_flist_t	*args3_flistp = NULL;
	pin_flist_t	*args4_flistp = NULL;
	pin_flist_t	*args5_flistp = NULL;
	pin_flist_t	*args6_flistp = NULL;
	pin_flist_t	*result_flistp = NULL;
	int32          	act_status = 1;	
	int32          	inact_status = 2;
	int64		db = -1;
	poid_t		*pdp = NULL;
	poid_t		*search_pdp = NULL;
	int32           flags = 256;
	char		*templatep = "select x from /purchased_product 1, /product 2, /service 3 where 1.F1 = 2.F2 and 3.F3 = 1.F4 and (1.F5 = 1 OR 1.F6 = 2) and 1.F7 = V7";
	char		*type = NULL;	
		
	if(PIN_ERRBUF_IS_ERR(ebufp))
        return;
		
        PIN_ERRBUF_CLEAR(ebufp);
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_get_purchased_product_info_input_flist", i_flistp);
		
	pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp);
	type = (char *)PIN_POID_GET_TYPE(pdp);
	db = PIN_POID_GET_DB(pdp);
	search_pdp = (poid_t *)PIN_POID_CREATE(db, "/search", -1, ebufp);

	search_iflistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_SET(search_iflistp, PIN_FLD_POID, search_pdp, ebufp);
	PIN_FLIST_FLD_SET(search_iflistp, PIN_FLD_FLAGS, &flags, ebufp);
	PIN_FLIST_FLD_SET(search_iflistp, PIN_FLD_TEMPLATE, templatep, ebufp);
	
	args_flistp = PIN_FLIST_ELEM_ADD(search_iflistp, PIN_FLD_ARGS, 1, ebufp);
	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_PRODUCT_OBJ, NULL, ebufp);

	args1_flistp = PIN_FLIST_ELEM_ADD(search_iflistp, PIN_FLD_ARGS, 2, ebufp);
        PIN_FLIST_FLD_SET(args1_flistp, PIN_FLD_POID, NULL, ebufp);
		
	args2_flistp = PIN_FLIST_ELEM_ADD(search_iflistp, PIN_FLD_ARGS, 3, ebufp);
        PIN_FLIST_FLD_SET(args2_flistp, PIN_FLD_POID, NULL, ebufp);
		
	args3_flistp = PIN_FLIST_ELEM_ADD(search_iflistp, PIN_FLD_ARGS, 4, ebufp);
        PIN_FLIST_FLD_SET(args3_flistp, PIN_FLD_SERVICE_OBJ, NULL, ebufp);
		
	args4_flistp = PIN_FLIST_ELEM_ADD(search_iflistp, PIN_FLD_ARGS, 5, ebufp);
	PIN_FLIST_FLD_SET(args4_flistp, PIN_FLD_STATUS, &act_status, ebufp);
		
	args5_flistp = PIN_FLIST_ELEM_ADD(search_iflistp, PIN_FLD_ARGS, 6, ebufp);
	PIN_FLIST_FLD_SET(args5_flistp, PIN_FLD_STATUS, &inact_status, ebufp);
		
	args6_flistp = PIN_FLIST_ELEM_ADD(search_iflistp, PIN_FLD_ARGS, 7, ebufp);
	if (type && strstr(type, "/purchased"))
	{
        	PIN_FLIST_FLD_SET(args6_flistp, PIN_FLD_POID, pdp, ebufp);
	}
	else if(type && strstr(type, "/account"))
	{
		PIN_FLIST_FLD_SET(args6_flistp, PIN_FLD_ACCOUNT_OBJ, pdp, ebufp);
	}
	result_flistp = PIN_FLIST_ELEM_ADD(search_iflistp, PIN_FLD_RESULTS, 0, ebufp);
		
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_get_purchased_product_info input flist: ", search_iflistp);
	
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, search_iflistp, &return_flistp, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp))
        {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "PCM_OP_SEARCH error: ", ebufp);
                return;
        }	
	
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_get_purchased_product_info output flist: ", return_flistp);
		 
	*r_flistp = PIN_FLIST_COPY(return_flistp,ebufp);
	return;
	
}

/*******************************************************
fm_get_purchased_details function to
get service info from login and poid_type
*******************************************************/
void
fm_get_purchased_discount_info(
pcm_context_t       *ctxp,
pin_flist_t         *i_flistp,
pin_flist_t         **r_flistp,
pin_errbuf_t        *ebufp)
{
        pin_flist_t     *input_flistp = NULL;
        pin_flist_t     *return_flistp = NULL;
        pin_flist_t     *search_iflistp  = NULL;
        pin_flist_t     *args_flistp = NULL;
        pin_flist_t     *args1_flistp = NULL;
        pin_flist_t     *args2_flistp = NULL;
        pin_flist_t     *args3_flistp = NULL;
        pin_flist_t     *args4_flistp = NULL;
        pin_flist_t     *args5_flistp = NULL;
        pin_flist_t     *args6_flistp = NULL;
        pin_flist_t     *result_flistp = NULL;
        int32           act_status = 1;
        int32           inact_status = 2;
        int64           db = -1;
        poid_t          *pdp = NULL;
        poid_t          *search_pdp = NULL;
        int32           flags = 256;
        char            *templatep = "select x from /purchased_discount 1, /discount 2, /service 3 where 1.F1 = 2.F2 and 3.F3 = 1.F4 and (1.F5 = 1 OR 1.F6 = 2) and 1.F7 = V7";
        char            *type = NULL;

        if(PIN_ERRBUF_IS_ERR(ebufp))
        return;

        PIN_ERRBUF_CLEAR(ebufp);
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_get_purchased_discount_info_input_flist", i_flistp);

        pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp);
        type = (char *)PIN_POID_GET_TYPE(pdp);
        db = PIN_POID_GET_DB(pdp);
        search_pdp = (poid_t *)PIN_POID_CREATE(db, "/search", -1, ebufp);

        search_iflistp = PIN_FLIST_CREATE(ebufp);
        PIN_FLIST_FLD_SET(search_iflistp, PIN_FLD_POID, search_pdp, ebufp);
        PIN_FLIST_FLD_SET(search_iflistp, PIN_FLD_FLAGS, &flags, ebufp);
        PIN_FLIST_FLD_SET(search_iflistp, PIN_FLD_TEMPLATE, templatep, ebufp);

        args_flistp = PIN_FLIST_ELEM_ADD(search_iflistp, PIN_FLD_ARGS, 1, ebufp);
        PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_DISCOUNT_OBJ, NULL, ebufp);

        args1_flistp = PIN_FLIST_ELEM_ADD(search_iflistp, PIN_FLD_ARGS, 2, ebufp);
        PIN_FLIST_FLD_SET(args1_flistp, PIN_FLD_POID, NULL, ebufp);

        args2_flistp = PIN_FLIST_ELEM_ADD(search_iflistp, PIN_FLD_ARGS, 3, ebufp);
        PIN_FLIST_FLD_SET(args2_flistp, PIN_FLD_POID, NULL, ebufp);

        args3_flistp = PIN_FLIST_ELEM_ADD(search_iflistp, PIN_FLD_ARGS, 4, ebufp);
        PIN_FLIST_FLD_SET(args3_flistp, PIN_FLD_SERVICE_OBJ, NULL, ebufp);

        args4_flistp = PIN_FLIST_ELEM_ADD(search_iflistp, PIN_FLD_ARGS, 5, ebufp);
        PIN_FLIST_FLD_SET(args4_flistp, PIN_FLD_STATUS, &act_status, ebufp);

        args5_flistp = PIN_FLIST_ELEM_ADD(search_iflistp, PIN_FLD_ARGS, 6, ebufp);
        PIN_FLIST_FLD_SET(args5_flistp, PIN_FLD_STATUS, &inact_status, ebufp);

        args6_flistp = PIN_FLIST_ELEM_ADD(search_iflistp, PIN_FLD_ARGS, 7, ebufp);
        if (type && strstr(type, "/purchased"))
        {
                PIN_FLIST_FLD_SET(args6_flistp, PIN_FLD_POID, pdp, ebufp);
        }
        else if(type && strstr(type, "/account"))
        {
                PIN_FLIST_FLD_SET(args6_flistp, PIN_FLD_ACCOUNT_OBJ, pdp, ebufp);
        }

        result_flistp = PIN_FLIST_ELEM_ADD(search_iflistp, PIN_FLD_RESULTS, 0, ebufp);

        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_get_purchased_discount_info input flist: ", search_iflistp);

        PCM_OP(ctxp, PCM_OP_SEARCH, 0, search_iflistp, &return_flistp, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp))
        {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "PCM_OP_SEARCH error: ", ebufp);
                return;
        }

        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_get_purchased_discount_info output flist: ", return_flistp);

        *r_flistp = PIN_FLIST_COPY(return_flistp,ebufp);
        return;

}

void
fm_rbs_get_payinfo(
	pcm_context_t   *ctxp,
	poid_t          *acc_pdp,
	pin_flist_t     **r_flistp,
	pin_errbuf_t    *ebufp)
{
	pin_flist_t	*payinfo_iflistp = NULL;
	pin_flist_t     *payinfo_oflistp = NULL;	
	pin_flist_t	*args_flistp = NULL;
	pin_flist_t	*result_flistp = NULL;
	poid_t		*search_pdp = NULL;
	char		*template = " select X from /payinfo 1 where F1 = V1 ";
	int32		flags = 256;
	int64		db = -1;
	
	if(PIN_ERRBUF_IS_ERR(ebufp))
        return;
        PIN_ERRBUF_CLEAR(ebufp);

        db = PIN_POID_GET_DB(acc_pdp);
        search_pdp = (poid_t *)PIN_POID_CREATE(db, "/search", -1, ebufp);

        payinfo_iflistp = PIN_FLIST_CREATE(ebufp);
        PIN_FLIST_FLD_SET(payinfo_iflistp, PIN_FLD_POID, search_pdp, ebufp);
        PIN_FLIST_FLD_SET(payinfo_iflistp, PIN_FLD_FLAGS, &flags, ebufp);
        PIN_FLIST_FLD_SET(payinfo_iflistp, PIN_FLD_TEMPLATE, template, ebufp);

        args_flistp = PIN_FLIST_ELEM_ADD(payinfo_iflistp, PIN_FLD_ARGS, 1, ebufp);
        PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_ACCOUNT_OBJ, acc_pdp, ebufp);

	result_flistp = PIN_FLIST_ELEM_ADD(payinfo_iflistp, PIN_FLD_RESULTS, 0, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_rbs_get_payinfo input flist: ", payinfo_iflistp);
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, payinfo_iflistp, &payinfo_oflistp, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp))
        {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "PCM_OP_SEARCH error: ", ebufp);
        	return;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_rbs_get_payinfo output flist: ", payinfo_oflistp);
	
	*r_flistp = PIN_FLIST_COPY(payinfo_oflistp, ebufp);
	return;
}
