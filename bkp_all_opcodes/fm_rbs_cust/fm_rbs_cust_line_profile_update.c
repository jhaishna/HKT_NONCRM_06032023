#ifndef lint
static const char Sccs_id[] = "@(#)%Portal Version: fm_rbs_cust_line_profile_update.c :BillingVelocityInt:3:2006-Sep-05 2:55:03 %";
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
#include "custom_flds/custom_flds.h"
#include "custom_wrapper_ops.h"

/*************************************
*DEFINE FUNCTIONS
*************************************/

EXPORT_OP void
op_rbs_cust_line_profile_update(
    cm_nap_connection_t *connp,
    int32           opcode,
    int32           flags,
    pin_flist_t     *i_flistp,
    pin_flist_t     **r_flistpp,
    pin_errbuf_t    *ebufp);

static void
fm_rbs_cust_line_profile_update(
        pcm_context_t           *ctxp,
        pin_flist_t             *i_flistp,
        pin_flist_t             **r_flistp,
        pin_errbuf_t            *ebufp);


/*****************************************
*FUNCTION IMPLIMENTATIONS
*****************************************/
void
op_rbs_cust_line_profile_update(
        cm_nap_connection_t     *connp,
        int32                   opcode,
        int32                   flags,
        pin_flist_t             *i_flistp,
        pin_flist_t             **r_flistpp,
        pin_errbuf_t            *ebufp)
{
        pcm_context_t           *ctxp = connp->dm_ctx;
        pin_flist_t             *r_flistp = NULL;

        *r_flistpp              = NULL;

        pin_flist_t             *rp_flistp = NULL;
        pin_flist_t             *rs_flistp = NULL;

        if (PIN_ERRBUF_IS_ERR(ebufp)) {
                return;
        }
        PIN_ERRBUF_CLEAR(ebufp);

        /*******************************************************************
         * Insanity Check
         *******************************************************************/
        if (opcode != PCM_OP_RBS_CUST_LINE_PROFILE_UPDATE) {
                pin_set_err(ebufp, PIN_ERRLOC_FM,
                        PIN_ERRCLASS_SYSTEM_DETERMINATE,
                        PIN_ERR_BAD_OPCODE, 0, 0, opcode);
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                        "op_rbs_cust_line_profile_update",
                        ebufp);
                return;
        }

        /*******************************************************************
         * Debug: Input flist
        *******************************************************************/
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                "calling op_rbs_cust_line_profile_update input flist", i_flistp);

        /*******************************************************************
         * Call the default implementation
         *******************************************************************/

                fm_rbs_cust_line_profile_update(ctxp, i_flistp, &r_flistp, ebufp);

                PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "after calling the fm function return flist", r_flistp);



        /***********************************************************
         * Results.
         ***********************************************************/

        if (PIN_ERRBUF_IS_ERR(ebufp))
        {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"op_rbs_cust_line_profile_update error", ebufp);
                PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                        "error op_rbs_cust_line_profile_update input flist", i_flistp);
                PIN_ERRBUF_RESET(ebufp);
                *r_flistpp = PIN_FLIST_COPY(i_flistp, ebufp);
        }
        else
        {
                *r_flistpp = PIN_FLIST_COPY(r_flistp, ebufp);
                PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "output op_rbs_cust_line_profile_update output flist", *r_flistpp);
        }

        PIN_FLIST_DESTROY_EX(&r_flistp, NULL);
        return;
}

void
fm_rbs_cust_line_profile_update(
        pcm_context_t           *ctxp,
        pin_flist_t             *i_flistp,
        pin_flist_t             **r_flistp,
        pin_errbuf_t            *ebufp)
{

	pin_flist_t              *in_flistp = NULL;
        pin_flist_t              *ret_prof_flistp = NULL;
        pin_flist_t              *inhe_flistp = NULL;
	pin_flist_t              *inhe_set_flistp = NULL;
	pin_flist_t              *inhe1_flistp = NULL;
        pin_flist_t              *attr_flistp = NULL;
	pin_flist_t              *attr_set_flistp = NULL;
        pin_flist_t              *ret_serv_flistp = NULL;
        pin_flist_t              *ser_ip_flistp = NULL;
	pin_flist_t              *ser_ip_set_flistp = NULL;
	pin_flist_t     	*ret_flistp = NULL;		

	int32			s_succ = 0;
	int32                   s_fail = 0;

	poid_t                   *prof_pd = NULL;
        poid_t                   *r_prof_pd = NULL;
	poid_t                   *serv_pd = NULL;
		
	char   *v_last_name = NULL;
        char   *v_staff_ccc = NULL;
        char   *v_first_name = NULL;
        char   *v_company = NULL;
        char   *v_salutation = NULL;
        char   *v_id_type = NULL;
        char   *v_l_gprs_opt_out = NULL;
        char   *v_r_gprs_opt_out = NULL;
        char   *v_e_gprs_opt_out = NULL;
        char   *v_idd_roam_opt_in = NULL;
        int    *v_idd_roam_threshold = NULL;
        char   *v_JOC_CITY = NULL;
        char   *v_opt_out_pccw = NULL;
        char   *v_opt_out_sms = NULL;
        char   *v_opt_out_email = NULL;
        char   *v_opt_out_dm = NULL;

        char     *v_EPP_EID = NULL;
        char     *v_EPP_ROLE = NULL;
        char     *v_REAL_MRT_IND = NULL;
        char     *v_invoice_data = NULL;
        char     *v_LAST_ROLE = NULL;
        char     *v_status_msg = NULL;
        
	poid_t   *v_MUP_TYPE = NULL;
        poid_t   *v_EPP_TYPE = NULL;

	int32	start_t = 0;
	int32	*end_t = 0;

  PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "start fm_rbs_cust_line_profile_update input_flist", i_flistp);

	in_flistp = PIN_FLIST_CREATE(ebufp);

	prof_pd = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp);
	serv_pd = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_SERVICE_OBJ, 0, ebufp);
	start_t = pin_virtual_time((time_t *)NULL);
	end_t = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_END_T, 0, ebufp);
	
	inhe_flistp = PIN_FLIST_SUBSTR_GET(i_flistp, PIN_FLD_INHERITED_INFO, 0, ebufp);
	attr_flistp = PIN_FLIST_SUBSTR_GET(inhe_flistp, PIN_FLD_ATTRIBUTE_INFO, 0, ebufp);


	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "debug");
if (prof_pd)
{ 
	PIN_ERR_LOG_FLIST(3, "inside if in_flistp", in_flistp);	
	PIN_FLIST_FLD_SET(in_flistp, PIN_FLD_POID, prof_pd, ebufp);
	inhe_set_flistp = PIN_FLIST_SUBSTR_ADD(in_flistp, PIN_FLD_INHERITED_INFO, ebufp);
        attr_set_flistp = PIN_FLIST_SUBSTR_ADD(inhe_set_flistp, PIN_FLD_ATTRIBUTE_INFO, ebufp);
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Before Setting the values to the flist", attr_flistp);

	if (PIN_FLIST_FLD_GET(attr_flistp, PIN_FLD_LAST_NAME, 1, ebufp) && PIN_FLIST_FLD_GET(attr_flistp, PIN_FLD_LAST_NAME, 1, ebufp) != NULL)
	{
        	v_last_name = PIN_FLIST_FLD_GET(attr_flistp, PIN_FLD_LAST_NAME, 0, ebufp);
		PIN_ERR_LOG_MSG(3, v_last_name);
		PIN_FLIST_FLD_SET(attr_set_flistp, PIN_FLD_LAST_NAME, v_last_name, ebufp);
	}
	if (PIN_FLIST_FLD_GET(attr_flistp, PIN_FLD_FIRST_NAME, 1, ebufp) && PIN_FLIST_FLD_GET(attr_flistp, PIN_FLD_FIRST_NAME, 1, ebufp) != NULL)
	{
        	v_first_name = PIN_FLIST_FLD_GET(attr_flistp, PIN_FLD_FIRST_NAME, 0, ebufp);
		PIN_FLIST_FLD_SET(attr_set_flistp, PIN_FLD_FIRST_NAME, v_first_name, ebufp);
	}
	if (PIN_FLIST_FLD_GET(attr_flistp, MRB_FLD_STAFF_CCC, 1, ebufp) && PIN_FLIST_FLD_GET(attr_flistp, MRB_FLD_STAFF_CCC, 1, ebufp) != NULL)
	{
		v_staff_ccc = PIN_FLIST_FLD_GET(attr_flistp, MRB_FLD_STAFF_CCC, 0, ebufp);
		PIN_FLIST_FLD_SET(attr_set_flistp, MRB_FLD_STAFF_CCC, v_staff_ccc, ebufp);
	}
	if (PIN_FLIST_FLD_GET(attr_flistp, PIN_FLD_COMPANY, 1, ebufp) && PIN_FLIST_FLD_GET(attr_flistp, PIN_FLD_COMPANY, 1, ebufp) != NULL)
	{
        	v_company = PIN_FLIST_FLD_GET(attr_flistp, PIN_FLD_COMPANY, 0, ebufp);
		PIN_FLIST_FLD_SET(attr_set_flistp, PIN_FLD_COMPANY, v_company, ebufp);
	}
	if (PIN_FLIST_FLD_GET(attr_flistp, PIN_FLD_TITLE, 1, ebufp) && PIN_FLIST_FLD_GET(attr_flistp, PIN_FLD_TITLE, 1, ebufp) != NULL)
	{
        	v_salutation = PIN_FLIST_FLD_GET(attr_flistp, PIN_FLD_TITLE, 0, ebufp);
		PIN_FLIST_FLD_SET(attr_set_flistp, PIN_FLD_TITLE, v_salutation, ebufp);
	}
	if (PIN_FLIST_FLD_GET(attr_flistp, MRB_FLD_ID_TYPE, 1, ebufp) && PIN_FLIST_FLD_GET(attr_flistp, MRB_FLD_ID_TYPE, 1, ebufp) != NULL)
	{
        	v_id_type = PIN_FLIST_FLD_GET(attr_flistp, MRB_FLD_ID_TYPE, 0, ebufp);
		PIN_FLIST_FLD_SET(attr_set_flistp, MRB_FLD_ID_TYPE, v_id_type, ebufp);
	}
	if (PIN_FLIST_FLD_GET(attr_flistp, PIN_FLD_OPERATOR_STR, 1, ebufp) && PIN_FLIST_FLD_GET(attr_flistp, PIN_FLD_OPERATOR_STR, 1, ebufp) != NULL)
	{
        	v_l_gprs_opt_out = PIN_FLIST_FLD_GET(attr_flistp, PIN_FLD_OPERATOR_STR, 0, ebufp);
		PIN_FLIST_FLD_SET(attr_set_flistp, PIN_FLD_OPERATOR_STR, v_l_gprs_opt_out, ebufp);
	}
	if (PIN_FLIST_FLD_GET(attr_flistp, PIN_FLD_STATUS_STR, 1, ebufp) && PIN_FLIST_FLD_GET(attr_flistp, PIN_FLD_STATUS_STR, 1, ebufp) != NULL)
	{
	        v_r_gprs_opt_out = PIN_FLIST_FLD_GET(attr_flistp, PIN_FLD_STATUS_STR, 0, ebufp);
		PIN_FLIST_FLD_SET(attr_set_flistp, PIN_FLD_STATUS_STR, v_r_gprs_opt_out, ebufp);
	}
	if (PIN_FLIST_FLD_GET(attr_flistp, PIN_FLD_STRING, 1, ebufp) && PIN_FLIST_FLD_GET(attr_flistp, PIN_FLD_STRING, 1, ebufp) != NULL)
	{
        	v_e_gprs_opt_out = PIN_FLIST_FLD_GET(attr_flistp, PIN_FLD_STRING, 0, ebufp);
		PIN_FLIST_FLD_SET(attr_set_flistp, PIN_FLD_STRING, v_e_gprs_opt_out, ebufp);
	}
	if (PIN_FLIST_FLD_GET(attr_flistp, PIN_FLD_STATUS_MSG, 1, ebufp) && PIN_FLIST_FLD_GET(attr_flistp, PIN_FLD_STATUS_MSG, 1, ebufp) != NULL)
	{
		v_idd_roam_opt_in = PIN_FLIST_FLD_GET(attr_flistp, PIN_FLD_STATUS_MSG, 0, ebufp);
		PIN_FLIST_FLD_SET(attr_set_flistp, PIN_FLD_STATUS_MSG, v_idd_roam_opt_in, ebufp);
	}
	if (PIN_FLIST_FLD_GET(attr_flistp, PIN_FLD_STATUS_FLAGS, 1, ebufp) && PIN_FLIST_FLD_GET(attr_flistp, PIN_FLD_STATUS_FLAGS, 1, ebufp) != NULL)
	{
        	v_idd_roam_threshold = PIN_FLIST_FLD_GET(attr_flistp, PIN_FLD_STATUS_FLAGS, 0, ebufp);
		PIN_FLIST_FLD_SET(attr_set_flistp, PIN_FLD_STATUS_FLAGS, v_idd_roam_threshold, ebufp);
	}
	if (PIN_FLIST_FLD_GET(attr_flistp, PIN_FLD_DESTINATION_NETWORK, 1, ebufp) && PIN_FLIST_FLD_GET(attr_flistp, PIN_FLD_DESTINATION_NETWORK, 1, ebufp) != NULL)
	{
        	v_JOC_CITY = PIN_FLIST_FLD_GET(attr_flistp, PIN_FLD_DESTINATION_NETWORK, 0, ebufp);
		PIN_FLIST_FLD_SET(attr_set_flistp, PIN_FLD_DESTINATION_NETWORK, v_JOC_CITY, ebufp);
	}
	if (PIN_FLIST_FLD_GET(attr_flistp, MRB_FLD_DELIVERY_DESCR4, 1, ebufp) && PIN_FLIST_FLD_GET(attr_flistp, MRB_FLD_DELIVERY_DESCR4, 1, ebufp) != NULL)
	{
        	v_opt_out_pccw = PIN_FLIST_FLD_GET(attr_flistp, MRB_FLD_DELIVERY_DESCR4, 0, ebufp);
		PIN_FLIST_FLD_SET(attr_set_flistp, MRB_FLD_DELIVERY_DESCR4, v_opt_out_pccw, ebufp);
	}
	if (PIN_FLIST_FLD_GET(attr_flistp, MRB_FLD_DELIVERY_DESCR3, 1, ebufp) && PIN_FLIST_FLD_GET(attr_flistp, MRB_FLD_DELIVERY_DESCR3, 1, ebufp) != NULL)
	{
        	v_opt_out_sms = PIN_FLIST_FLD_GET(attr_flistp, MRB_FLD_DELIVERY_DESCR3, 0, ebufp);
		PIN_FLIST_FLD_SET(attr_set_flistp, MRB_FLD_DELIVERY_DESCR3, v_opt_out_sms, ebufp);
	}
	if (PIN_FLIST_FLD_GET(attr_flistp, MRB_FLD_DELIVERY_DESCR2, 1, ebufp) && PIN_FLIST_FLD_GET(attr_flistp, MRB_FLD_DELIVERY_DESCR2, 1, ebufp) != NULL)
	{
        	v_opt_out_email = PIN_FLIST_FLD_GET(attr_flistp, MRB_FLD_DELIVERY_DESCR2, 0, ebufp);
		PIN_FLIST_FLD_SET(attr_set_flistp, MRB_FLD_DELIVERY_DESCR2, v_opt_out_email, ebufp);
	}
	if (PIN_FLIST_FLD_GET(attr_flistp, MRB_FLD_DELIVERY_DESCR1, 1, ebufp) && PIN_FLIST_FLD_GET(attr_flistp, MRB_FLD_DELIVERY_DESCR1, 1, ebufp) != NULL)
	{
        	v_opt_out_dm = PIN_FLIST_FLD_GET(attr_flistp, MRB_FLD_DELIVERY_DESCR1, 0, ebufp);
		PIN_FLIST_FLD_SET(attr_set_flistp, MRB_FLD_DELIVERY_DESCR1, v_opt_out_dm, ebufp);
	}

PIN_ERR_LOG_FLIST(3, "end fm_rbs_cust_line_profile_update input_flist", in_flistp);

	PCM_OP(ctxp, PCM_OP_CUST_MODIFY_PROFILE, 0, in_flistp, &ret_prof_flistp, ebufp);
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_rbs_cust_line_profile_update return_flist", ret_prof_flistp);

        if (PIN_ERRBUF_IS_ERR(ebufp))
        {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"fm_rbs_cust_line_profile_update error", ebufp);
                PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "error fm_rbs_cust_line_profile_update input flist", in_flistp);
                PIN_ERRBUF_RESET(ebufp);
                goto cleanup;
        }
        else
        {
                *r_flistp = PIN_FLIST_COPY(ret_prof_flistp, ebufp);
                PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_rbs_cust_line_profile_update output flist", *r_flistp);
        }
}
PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "debug3");	

        r_prof_pd = PIN_FLIST_FLD_GET(*r_flistp, PIN_FLD_POID, 0, ebufp);
        in_flistp = PIN_FLIST_CREATE(ebufp);

        PIN_ERR_LOG_POID(PIN_ERR_LEVEL_DEBUG, "profile poid from i_flistp", prof_pd);
        PIN_ERR_LOG_POID(PIN_ERR_LEVEL_DEBUG, "profile poid from r_flistp", r_prof_pd);
  
	if (PIN_POID_COMPARE(prof_pd, r_prof_pd, 0, ebufp) == 0)
        {

            PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "start fm_cust_line_modify_service input_flist", i_flistp);

            PIN_FLIST_FLD_SET(in_flistp, PIN_FLD_POID, serv_pd, ebufp);
            inhe1_flistp = PIN_FLIST_SUBSTR_ADD(in_flistp, PIN_FLD_INHERITED_INFO, ebufp);
            ser_ip_set_flistp = PIN_FLIST_SUBSTR_ADD(inhe1_flistp, PIN_FLD_SERVICE_IP, ebufp);
            PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "start fm_cust_line_modify_service: ", in_flistp);
			
		    ser_ip_flistp = PIN_FLIST_SUBSTR_GET(inhe_flistp, PIN_FLD_SERVICE_IP, 0, ebufp);

		if (PIN_FLIST_FLD_GET(ser_ip_flistp, PIN_FLD_EDR_FIELD_NAME, 1, ebufp) && PIN_FLIST_FLD_GET(ser_ip_flistp, PIN_FLD_EDR_FIELD_NAME, 1, ebufp) != NULL)
                {
			v_EPP_EID = PIN_FLIST_FLD_GET(ser_ip_flistp, PIN_FLD_EDR_FIELD_NAME, 0, ebufp);
                        PIN_FLIST_FLD_SET(ser_ip_set_flistp, PIN_FLD_EDR_FIELD_NAME, v_EPP_EID, ebufp);
                }

                if (PIN_FLIST_FLD_GET(ser_ip_flistp, PIN_FLD_EDR_STATUS, 1, ebufp) && PIN_FLIST_FLD_GET(ser_ip_flistp, PIN_FLD_EDR_STATUS, 1, ebufp) != NULL)
                {
			v_EPP_ROLE = PIN_FLIST_FLD_GET(ser_ip_flistp, PIN_FLD_EDR_STATUS, 0, ebufp);
                        PIN_FLIST_FLD_SET(ser_ip_set_flistp, PIN_FLD_EDR_STATUS, v_EPP_ROLE, ebufp);
                }

                if (PIN_FLIST_FLD_GET(ser_ip_flistp, PIN_FLD_INFO, 1, ebufp) && PIN_FLIST_FLD_GET(ser_ip_flistp, PIN_FLD_INFO, 1, ebufp) != NULL)
                {
			v_REAL_MRT_IND = PIN_FLIST_FLD_GET(ser_ip_flistp, PIN_FLD_INFO, 0, ebufp);
                        PIN_FLIST_FLD_SET(ser_ip_set_flistp, PIN_FLD_INFO, v_REAL_MRT_IND, ebufp);
                }

                if (PIN_FLIST_FLD_GET(ser_ip_flistp, PIN_FLD_INVOICE_DATA, 1, ebufp) && PIN_FLIST_FLD_GET(ser_ip_flistp, PIN_FLD_INVOICE_DATA, 1, ebufp) != NULL)
                {
			v_invoice_data = PIN_FLIST_FLD_GET(ser_ip_flistp, PIN_FLD_INVOICE_DATA, 0, ebufp);
                        PIN_FLIST_FLD_SET(ser_ip_set_flistp, PIN_FLD_INVOICE_DATA, v_invoice_data, ebufp);
                }

                if (PIN_FLIST_FLD_GET(ser_ip_flistp, PIN_FLD_LABEL, 1, ebufp) && PIN_FLIST_FLD_GET(ser_ip_flistp, PIN_FLD_LABEL, 1, ebufp) != NULL)
                {
			v_LAST_ROLE = PIN_FLIST_FLD_GET(ser_ip_flistp, PIN_FLD_LABEL, 0, ebufp);
                        PIN_FLIST_FLD_SET(ser_ip_set_flistp, PIN_FLD_LABEL, v_LAST_ROLE, ebufp);
                }

                if (PIN_FLIST_FLD_GET(ser_ip_flistp, PIN_FLD_STATUS_MSG, 1, ebufp) && PIN_FLIST_FLD_GET(ser_ip_flistp, PIN_FLD_STATUS_MSG, 1, ebufp) != NULL)
                {
			v_status_msg = PIN_FLIST_FLD_GET(ser_ip_flistp, PIN_FLD_STATUS_MSG, 0, ebufp);
                        PIN_FLIST_FLD_SET(ser_ip_set_flistp, PIN_FLD_STATUS_MSG, v_status_msg, ebufp);
                }

                if (PIN_FLIST_FLD_GET(ser_ip_flistp, PIN_FLD_GROUP_OBJ, 1, ebufp) && PIN_FLIST_FLD_GET(ser_ip_flistp, PIN_FLD_GROUP_OBJ, 1, ebufp) != NULL)
                {
			v_MUP_TYPE = PIN_FLIST_FLD_GET(ser_ip_flistp, PIN_FLD_GROUP_OBJ, 0, ebufp);
                        PIN_FLIST_FLD_SET(ser_ip_set_flistp, PIN_FLD_GROUP_OBJ, v_MUP_TYPE, ebufp);
                }

                if (PIN_FLIST_FLD_GET(ser_ip_flistp, PIN_FLD_SCOPE_OBJ, 1, ebufp) && PIN_FLIST_FLD_GET(ser_ip_flistp, PIN_FLD_SCOPE_OBJ, 1, ebufp) != NULL)
                {
			v_EPP_TYPE = PIN_FLIST_FLD_GET(ser_ip_flistp, PIN_FLD_SCOPE_OBJ, 0, ebufp);
                        PIN_FLIST_FLD_SET(ser_ip_set_flistp, PIN_FLD_SCOPE_OBJ, v_EPP_TYPE, ebufp);
                }

                PIN_FLIST_FLD_SET(in_flistp, PIN_FLD_START_T, &start_t, ebufp);
                PIN_FLIST_FLD_SET(in_flistp, PIN_FLD_END_T, end_t, ebufp);

                PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "end fm_cust_line_modify_service input_flist", in_flistp);

                PCM_OP(ctxp, PCM_OP_CUST_MODIFY_SERVICE, 0, in_flistp, &ret_serv_flistp, ebufp);

                PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_cust_line_modify_service return_flist", ret_serv_flistp);


                if (PIN_ERRBUF_IS_ERR(ebufp))
                  {
                         ret_flistp = PIN_FLIST_CREATE(ebufp);
                         s_fail = 1;
                         PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"fm_cust_line_modify_service error", ebufp);
                         PIN_FLIST_FLD_COPY(ret_serv_flistp, PIN_FLD_POID, ret_flistp, PIN_FLD_POID, ebufp);
                         PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &s_fail, ebufp);
                         PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "error fm_cust_line_modify_service input flist", ret_flistp);
                         PIN_ERRBUF_RESET(ebufp);
                         return;
                  }
                  else
                  {
                         s_succ = 0;
                         PIN_FLIST_FLD_COPY(ret_serv_flistp, PIN_FLD_POID, *r_flistp, PIN_FLD_SERVICE_OBJ, ebufp);
                         PIN_FLIST_FLD_SET(*r_flistp, PIN_FLD_STATUS, &s_succ, ebufp);
                         PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_cust_line_modify_service output flist", *r_flistp);
                  }

          }
        return;
cleanup:
        *r_flistp = PIN_FLIST_COPY(i_flistp, ebufp);
        PIN_FLIST_DESTROY_EX(&ret_flistp, NULL);
        PIN_FLIST_DESTROY_EX(&ret_prof_flistp, NULL);
        PIN_FLIST_DESTROY_EX(&ret_serv_flistp, NULL);
        PIN_FLIST_DESTROY_EX(&in_flistp, NULL);
}

