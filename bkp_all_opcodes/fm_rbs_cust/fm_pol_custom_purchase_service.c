#ifndef lint
static const char Sccs_id[] = "@(#)%Portal Version: fm_pol_custom_purchase_service.c :BillingVelocityInt:3:2006-Sep-05 21:55:03 %";
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
#include "ops/rbs.h"
#include "ops/bill.h"
#include "custom_wrapper_ops.h"

#define PIN_FLD_CRM_KEY 17001

/*************************************
*DEFINE FUNCTIONS
*************************************/

EXPORT_OP void
op_cust_purchase_service(
    cm_nap_connection_t *connp,
    int32           opcode,
    int32           flags,
    pin_flist_t     *i_flistp,
    pin_flist_t     **r_flistpp,
    pin_errbuf_t    *ebufp);

static void
fm_cust_purchase_service(
        pcm_context_t           *ctxp,
        pin_flist_t             *i_flistp,
        pin_flist_t             **r_flistp,
        pin_errbuf_t            *ebufp);

void
mrb_get_service_details(
        pcm_context_t           *ctxp,
        pin_flist_t             *i_flistp,
        char                    *permitted,
        pin_flist_t             **r_flistp,
        pin_errbuf_t            *ebufp);

/*****************************************
*FUNCTION IMPLIMENTATIONS
*****************************************/
void
op_cust_purchase_service(
cm_nap_connection_t     	*connp,
        int32                   opcode,
        int32                   flags,
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

        /*******************************************************************
         * Insanity Check
         *******************************************************************/
        if (opcode != PCM_CUSTOM_PURCHASE_SERVICE ) {
                pin_set_err(ebufp, PIN_ERRLOC_FM,
                        PIN_ERRCLASS_SYSTEM_DETERMINATE,
                        PIN_ERR_BAD_OPCODE, 0, 0, opcode);
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                        "op_cust_purchase_service",
                        ebufp);
                return;
        }

        /*******************************************************************
         * Debug: Input flist
        *******************************************************************/
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                "op_cust_purchase_service input flist", i_flistp);

        /*******************************************************************
         * Call the default implementation
         *******************************************************************/

        fm_cust_purchase_service(ctxp, i_flistp, &r_flistp, ebufp);

        /***********************************************************
         * Results.
         ***********************************************************/

        if (PIN_ERRBUF_IS_ERR(ebufp))
        {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"op_cust_purchase_service error", ebufp);
        }
        else
        {
                *r_flistpp = PIN_FLIST_COPY(r_flistp, ebufp);
                PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "op_cust_purchase_service output flist", *r_flistpp);
        }

        PIN_FLIST_DESTROY_EX(&r_flistp, NULL);
        return;
}
void
fm_cust_purchase_service(
        pcm_context_t           *ctxp,
        pin_flist_t             *i_flistp,
        pin_flist_t             **r_flistp,
        pin_errbuf_t            *ebufp)
{
        poid_t                  *acc_pd = NULL;
        pin_flist_t             *deal_flistp = NULL;
        pin_flist_t             *tmp_flistp = NULL;
        char                    *v_permitted = NULL;
        char                    *namep = NULL;
        char                    *package_idp = NULL;
        char                    deal_name[BUFSIZ];
        pin_flist_t             *serv_flistp = NULL;
        pin_flist_t             *purchase_iflistp = NULL;
        pin_flist_t             *purchase_oflistp = NULL;
        pin_flist_t             *deal_infop = NULL;
	pin_flist_t		*arg_flistp = NULL;
	int32           	elem_id = 0;
	pin_cookie_t   		cookie = NULL;
	pin_flist_t		*c_flistp = NULL;
	pin_flist_t		*concat_flistp = NULL;

        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_cust_purchase_service input_flist", i_flistp);
        acc_pd = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp);

	while ((arg_flistp = PIN_FLIST_ELEM_GET_NEXT(i_flistp, PIN_FLD_DEALS, &elem_id, 1, &cookie, ebufp)) != (pin_flist_t *)NULL)
	{
		namep = PIN_FLIST_FLD_GET(arg_flistp, PIN_FLD_DEAL_NAME, 0, ebufp);
		package_idp = PIN_FLIST_FLD_GET(arg_flistp, PIN_FLD_DEAL_CODE, 0, ebufp);
		memset(deal_name, '\0', sizeof(deal_name));
		sprintf(deal_name, "%s|%s", namep, package_idp);
		mrb_get_deal_details(ctxp, &deal_name, &deal_flistp, ebufp);
		if (PIN_ERR_IS_ERR(ebufp)) {
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
				"after calling mrb_get_deal_details error", ebufp);
			goto CLEANUP;
		}
		if(deal_flistp) {
			v_permitted = PIN_FLIST_FLD_GET(deal_flistp, PIN_FLD_PERMITTED, 1, ebufp);
		}
		else {
			pin_set_err(ebufp, PIN_ERRLOC_FM,
				PIN_ERRCLASS_SYSTEM_DETERMINATE,
					PIN_ERR_NOT_FOUND, 0, 0, 0);
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"Error : Deal not found in the Database ", ebufp);
			goto CLEANUP;
		}
		mrb_get_service_details(ctxp, i_flistp, v_permitted, &serv_flistp, ebufp);
		if (PIN_ERR_IS_ERR(ebufp)) {
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
				"after calling mrb_get_service_details error", ebufp);
			goto CLEANUP;
		}
		if(serv_flistp == NULL) {
			pin_set_err(ebufp, PIN_ERRLOC_FM,
				PIN_ERRCLASS_SYSTEM_DETERMINATE,
					PIN_ERR_NOT_FOUND, 0, 0, 0);
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"Error : Service for corresponding login not found ", ebufp);
			goto CLEANUP;
		}
		purchase_iflistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_COPY(serv_flistp, PIN_FLD_ACCOUNT_OBJ, purchase_iflistp, PIN_FLD_POID, ebufp);
		PIN_FLIST_FLD_COPY(serv_flistp, PIN_FLD_POID, purchase_iflistp, PIN_FLD_SERVICE_OBJ, ebufp);
		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_END_T, purchase_iflistp, PIN_FLD_END_T, ebufp);
		PIN_FLIST_FLD_SET(purchase_iflistp, PIN_FLD_PROGRAM_NAME, "ADD OFFER", ebufp);
		deal_infop = PIN_FLIST_SUBSTR_ADD(purchase_iflistp, PIN_FLD_DEAL_INFO, ebufp);
		PIN_FLIST_FLD_COPY(deal_flistp, PIN_FLD_POID, deal_infop, PIN_FLD_DEAL_OBJ, ebufp);

		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "SUBSSCRIPTION PURCHASE DEAL INPUT FLIST ", purchase_iflistp);
		PCM_OP(ctxp, PCM_OP_SUBSCRIPTION_PURCHASE_DEAL, 0, purchase_iflistp, &purchase_oflistp, ebufp);
		if (PIN_ERR_IS_ERR(ebufp)) {
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
				"after calling purchase_deal  error", ebufp);
			goto CLEANUP;
		}
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "SUBSSCRIPTION PURCHASE DEAL OUTPUT FLIST ", purchase_oflistp);
		if(concat_flistp == NULL) {
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_WARNING, "check_1");
			concat_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, concat_flistp, PIN_FLD_POID, ebufp);
			
		}
		c_flistp = PIN_FLIST_ELEM_ADD(concat_flistp, PIN_FLD_ARGS, elem_id, ebufp);
		PIN_FLIST_CONCAT(c_flistp, purchase_oflistp, ebufp);

	        PIN_FLIST_DESTROY_EX(&purchase_iflistp, NULL);
        	PIN_FLIST_DESTROY_EX(&purchase_oflistp, NULL);
	        PIN_FLIST_DESTROY_EX(&serv_flistp, NULL);
        	PIN_FLIST_DESTROY_EX(&deal_flistp, NULL);

	}
        *r_flistp = PIN_FLIST_COPY(concat_flistp, ebufp);

        CLEANUP:
        PIN_FLIST_DESTROY_EX(&purchase_iflistp, NULL);
        PIN_FLIST_DESTROY_EX(&purchase_oflistp, NULL);
        PIN_FLIST_DESTROY_EX(&serv_flistp, NULL);
        PIN_FLIST_DESTROY_EX(&deal_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&concat_flistp, NULL);
        return;
}

void
mrb_get_service_details(
        pcm_context_t           *ctxp,
        pin_flist_t             *i_flistp,
        char                    *permitted,
        pin_flist_t             **r_flistp,
        pin_errbuf_t            *ebufp)
{

        pin_flist_t              *out_flistp = NULL;
        pin_flist_t             *search_flistp = NULL;
        pin_flist_t             *flistp = NULL;
        int                     flag = 0;
        int32                   database = 0;
        poid_t                  *search_pdp = NULL;
        char                    *search_str = "select X from /service where F1 = V1 and F2.type = V2 ";
        poid_t                  *poidp = NULL;
        poid_t                  *serv_pdp = NULL;

        if (PIN_ERRBUF_IS_ERR(ebufp)) {
                return;
        }
        PIN_ERRBUF_CLEAR(ebufp);
        poidp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp);
        if(poidp) {
                database = PIN_POID_GET_DB(poidp);
        }

        search_flistp = PIN_FLIST_CREATE(ebufp);
        search_pdp = PIN_POID_CREATE(database, "/search", -1, ebufp);
        PIN_FLIST_FLD_PUT(search_flistp, PIN_FLD_POID, (void *)search_pdp, ebufp);

        PIN_FLIST_FLD_SET(search_flistp, PIN_FLD_TEMPLATE, (void *)search_str, ebufp);

        flag = SRCH_EXACT;
        PIN_FLIST_FLD_SET(search_flistp, PIN_FLD_FLAGS, &flag, ebufp);

        flistp = PIN_FLIST_ELEM_ADD(search_flistp, PIN_FLD_ARGS, 1, ebufp);
        PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, flistp, PIN_FLD_ACCOUNT_OBJ, ebufp);

        flistp = PIN_FLIST_ELEM_ADD(search_flistp, PIN_FLD_ARGS, 2, ebufp);
        serv_pdp = PIN_POID_CREATE(database, permitted, -1, ebufp);
        PIN_FLIST_FLD_PUT(flistp, PIN_FLD_POID, (void *)serv_pdp, ebufp);

        PIN_FLIST_ELEM_SET(search_flistp, NULL, PIN_FLD_RESULTS, 0, ebufp);

        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "mrb_get_service_details : input flist", search_flistp);

        PCM_OP(ctxp, PCM_OP_SEARCH, PCM_OPFLG_CACHEABLE, search_flistp, &out_flistp, ebufp);

        PIN_FLIST_DESTROY_EX(&search_flistp, NULL);

        if (PIN_ERR_IS_ERR(ebufp)) {
                PIN_FLIST_DESTROY_EX(&out_flistp, NULL);
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                        "mrb_get_deal_details error", ebufp);
                return;
        }
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "mrb_get_service_details : Output flist", out_flistp);

        flistp = PIN_FLIST_ELEM_GET(out_flistp, PIN_FLD_RESULTS, 0, 1, ebufp);
        if(flistp) {
                *r_flistp = PIN_FLIST_COPY(flistp, ebufp);
        }
        PIN_FLIST_DESTROY_EX(&out_flistp, NULL);
        return;
}
