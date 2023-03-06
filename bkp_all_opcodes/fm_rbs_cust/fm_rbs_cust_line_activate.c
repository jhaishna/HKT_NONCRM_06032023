#ifndef lint
static const char Sccs_id[] = "@(#)%Portal Version: fm_rbs_cust_line_activate.c:BillingVelocityInt:3:2006-Sep-05 21:55:03 %";
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
#include "custom_flds/custom_flds.h"
#include "custom_wrapper_ops.h"

/*************************************
*DEFINE FUNCTIONS
*************************************/

EXPORT_OP void
op_rbs_cust_line_activate(
    cm_nap_connection_t *connp,
    int32           opcode,
    int32           flags,
    pin_flist_t     *i_flistp,
    pin_flist_t     **r_flistpp,
    pin_errbuf_t    *ebufp);

static void
fm_rbs_cust_line_activate(
        pcm_context_t           *ctxp,
        pin_flist_t             *i_flistp,
        pin_flist_t             **r_flistp,
        pin_errbuf_t            *ebufp);
		
/*****************************************
*FUNCTION IMPLIMENTATIONS
*****************************************/
void
op_rbs_cust_line_activate(
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

        if (PIN_ERRBUF_IS_ERR(ebufp)) {
                return;
        }
        PIN_ERRBUF_CLEAR(ebufp);

        /*******************************************************************
         * Insanity Check
         *******************************************************************/
        if (opcode != PCM_OP_RBS_CUST_LINE_ACTIVATE ) {
                pin_set_err(ebufp, PIN_ERRLOC_FM,
                        PIN_ERRCLASS_SYSTEM_DETERMINATE,
                        PIN_ERR_BAD_OPCODE, 0, 0, opcode);
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                        "op_rbs_cust_line_activate",
                        ebufp);
                return;
        }

	/*******************************************************************
         * Debug: Input flist
        *******************************************************************/
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                "op_rbs_cust_line_activate input flist", i_flistp);

        /*******************************************************************
         * Call the default implementation
         *******************************************************************/

                fm_rbs_cust_line_activate(ctxp, i_flistp, &r_flistp, ebufp);

	/***********************************************************
         * Results.
         ***********************************************************/

        if (PIN_ERRBUF_IS_ERR(ebufp))
        {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"op_rbs_cust_line_activate error", ebufp);
                PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                        "op_rbs_cust_line_activate input flist", i_flistp);
                PIN_ERRBUF_RESET(ebufp);
                *r_flistpp = PIN_FLIST_COPY(i_flistp, ebufp);
        }
        else
        {
                *r_flistpp = PIN_FLIST_COPY(r_flistp, ebufp);
                PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,  "op_rbs_cust_line_activate output flist", *r_flistpp);
        }

        PIN_FLIST_DESTROY_EX(&r_flistp, NULL);
        return;
}
void
fm_rbs_cust_line_activate(
        pcm_context_t   *ctxp,
        pin_flist_t     *i_flistp,
        pin_flist_t     **r_flistp,
        pin_errbuf_t    *ebufp)
{
        pin_flist_t     *in_flistp = NULL;
	pin_flist_t     *ret_flistp = NULL;
	pin_flist_t     *bal_info_flistp = NULL;
        pin_flist_t     *limit_flistp = NULL;
        pin_flist_t     *ser_flistp = NULL;
        pin_flist_t     *ser_code_flistp = NULL;
        pin_flist_t     *inher_info_flistp = NULL;
        pin_flist_t     *alias_list_flistp = NULL;
        pin_flist_t     *prof_flistp = NULL;
        pin_flist_t     *inher_info1_flistp = NULL;
        pin_flist_t     *attr1_info_flistp = NULL;
        pin_flist_t     *ser1_flistp = NULL;
        pin_flist_t     *alias_listp = NULL;
        pin_flist_t     *bal_info1_flistp = NULL;
        pin_flist_t     *prod_flistp = NULL;
        pin_flist_t     *ser_ip_flistp = NULL;
        pin_flist_t     *deals_flistp = NULL;
	pin_flist_t     *bal_info_flistp1 = NULL;
        pin_flist_t     *bal_info_flistp2 = NULL;
        pin_flist_t     *limit_flistp1 = NULL;
        pin_flist_t     *tmp_ser_flistp = NULL;
        pin_flist_t     *ser1_code_flistp = NULL;
        pin_flist_t     *inher1_info_flistp = NULL;
        pin_flist_t     *ser_ip1_flistp = NULL;
        pin_flist_t     *tmp_prof_flistp = NULL;
        pin_flist_t     *inher2_info_flistp = NULL;
        pin_flist_t     *inher3_info_flistp = NULL;
        pin_flist_t     *attr_info_flistp = NULL;
        pin_flist_t     *prof1_flistp = NULL;
        pin_flist_t     *tmp_deal_flistp = NULL;
        pin_flist_t     *tmp_disc_flistp = NULL;
        pin_flist_t     *disc_flistp = NULL;
        pin_flist_t     *tmp_prod_flistp = NULL;
        pin_flist_t     *deals1_flistp = NULL;
	poid_t          *plan_pd = NULL;
        poid_t          *bal_grp_pd = NULL;
        poid_t          *acc_obj = NULL;
        poid_t          *deal_obj = NULL;
        poid_t          *deal1_obj = NULL;
	poid_t  	*v_sub_tel = NULL;
        poid_t  	*v_sub_sms = NULL;
        poid_t  	*v_sub_mms = NULL;
        poid_t  	*v_sub_data = NULL;
        poid_t  	*v_sub_bb = NULL;
        poid_t  	*v_sub_data_mc = NULL;
	poid_t          *billinfo_pd = NULL;
        poid_t          *ser_sub_obj = NULL;
        time_t          *v_end_t = NULL;
        pin_rec_id_t    v_rec_id = 0;
        pin_cookie_t    cookie = NULL;
        pin_cookie_t    cookie1 = NULL;
        pin_cookie_t    cookie2 = NULL;
        pin_cookie_t    cookie3 = NULL;
	pin_cookie_t    cookie_l = NULL;
        char    	*v_last_name = NULL;
        char    	*v_staff_ccc = NULL;
        char    	*v_first_name = NULL;
        char    	*v_company = NULL;
        char    	*v_salutation = NULL;
        char    	*v_id_type = NULL;
        char    	*v_l_gprs_opt_out = NULL;
        char    	*v_r_gprs_opt_out = NULL;
        char    	*v_e_gprs_opt_out = NULL;
        char    	*v_idd_roam_opt_in = NULL;
        char    	*v_idd_roam_threshold = NULL;
        char    	*v_JOC_CITY = NULL;
        char    	*v_opt_out_pccw = NULL;
        char    	*v_opt_out_sms = NULL;
        char    	*v_opt_out_email = NULL;
        char    	*v_opt_out_dm = NULL;
	char    	msg[100];
	int64   	deal_id = NULL;
	int64   	db = 0;
        int32   	elem_cnt = 1;
        int32   	cnt = 0;
        int32   	elem_id = 0;
        int32   	elem_id1 = 0;
	int32           *v_line_id = 0;
        int32           elem_id_l = 0;
        int32           elem_p_id = 0;
        int32           deal_elem_cnt = 0;
        int32           deal_elem_id = 0;
	
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "start fm_rbs_cust_line_activate input_flist", i_flistp);

	plan_pd = (poid_t *)PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 1, ebufp);
	v_end_t = (time_t *)PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_END_T, 1, ebufp);
	db = PIN_POID_GET_DB(plan_pd);

	in_flistp = PIN_FLIST_CREATE(ebufp);

	PIN_FLIST_FLD_SET(in_flistp, PIN_FLD_POID, plan_pd, ebufp);
	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_ACCOUNT_OBJ, in_flistp, PIN_FLD_ACCOUNT_OBJ, ebufp);
	PIN_FLIST_FLD_SET(in_flistp, PIN_FLD_END_T, v_end_t, ebufp);

	bal_info_flistp = PIN_FLIST_ELEM_GET(i_flistp, PIN_FLD_BAL_INFO, PIN_ELEMID_ANY, 1, ebufp);
	if(bal_info_flistp)
	{
		PIN_ERR_LOG_FLIST(3, "bal_info_flistp debug1", bal_info_flistp);
	        bal_grp_pd = PIN_FLIST_FLD_GET(bal_info_flistp, PIN_FLD_POID, 0, ebufp);
	        billinfo_pd = PIN_FLIST_FLD_GET(bal_info_flistp, PIN_FLD_BILLINFO_OBJ, 0, ebufp);
	        PIN_ERR_LOG_MSG(3, msg);
	}

	bal_info_flistp1 = PIN_FLIST_ELEM_ADD(in_flistp, PIN_FLD_BAL_INFO, 1, ebufp);
	PIN_FLIST_FLD_SET(bal_info_flistp1, PIN_FLD_POID, bal_grp_pd, ebufp);
	PIN_FLIST_FLD_SET(bal_info_flistp1, PIN_FLD_BILLINFO_OBJ, billinfo_pd ,ebufp);
	PIN_ERR_LOG_FLIST(3, "after bal_info_flistp debug1", in_flistp); 
	elem_id_l = 0;
	cookie_l = NULL;
	while ((limit_flistp = PIN_FLIST_ELEM_GET_NEXT(bal_info_flistp, PIN_FLD_LIMIT, &elem_id_l, 1, &cookie_l, ebufp)) != (pin_flist_t *)NULL)
	{
		limit_flistp1 = PIN_FLIST_ELEM_ADD(bal_info_flistp1, PIN_FLD_LIMIT, 2005006, ebufp);
		PIN_FLIST_FLD_COPY(limit_flistp, PIN_FLD_CREDIT_LIMIT, limit_flistp1, PIN_FLD_CREDIT_LIMIT, ebufp);
		PIN_FLIST_FLD_COPY(limit_flistp, PIN_FLD_CREDIT_THRESHOLDS, limit_flistp1, PIN_FLD_CREDIT_THRESHOLDS, ebufp);
		PIN_FLIST_FLD_COPY(limit_flistp, PIN_FLD_CREDIT_FLOOR, limit_flistp1, PIN_FLD_CREDIT_FLOOR, ebufp);
		PIN_ERR_LOG_FLIST(3, "after limit_flistp debug1", limit_flistp);
	}
	PIN_ERR_LOG_FLIST(3, "after adding limit_flistp debug1", in_flistp);
	PIN_ERR_LOG_FLIST(3, "before services i_flistp debug", i_flistp);
	elem_id = 0;
	cookie = NULL;
	while ((tmp_ser_flistp = PIN_FLIST_ELEM_GET_NEXT(i_flistp, PIN_FLD_SERVICES, &elem_id, 1, &cookie, ebufp)) != (pin_flist_t *)NULL)
	{
		PIN_ERR_LOG_FLIST(3, "inside while loop tmp_ser_flistp", tmp_ser_flistp);
		sprintf(msg, "count: %d", elem_cnt);
		PIN_ERR_LOG_MSG(3, msg);
		ser_flistp = PIN_FLIST_CREATE(ebufp);
		
		ser_flistp = PIN_FLIST_ELEM_ADD(in_flistp, PIN_FLD_SERVICES, elem_cnt, ebufp);
		PIN_FLIST_FLD_COPY(tmp_ser_flistp, PIN_FLD_PASSWD_CLEAR, ser_flistp, PIN_FLD_PASSWD_CLEAR, ebufp);
		PIN_FLIST_FLD_COPY(tmp_ser_flistp, PIN_FLD_LOGIN, ser_flistp, PIN_FLD_LOGIN, ebufp);
		PIN_FLIST_FLD_COPY(tmp_ser_flistp, PIN_FLD_SUBSCRIPTION_OBJ, ser_flistp, PIN_FLD_SUBSCRIPTION_OBJ, ebufp);
		PIN_FLIST_FLD_COPY(tmp_ser_flistp, PIN_FLD_SERVICE_ID, ser_flistp, PIN_FLD_SERVICE_ID, ebufp);
		PIN_FLIST_FLD_COPY(tmp_ser_flistp, PIN_FLD_SUBSCRIPTION_INDEX, ser_flistp, PIN_FLD_SUBSCRIPTION_INDEX, ebufp);
		PIN_FLIST_FLD_COPY(tmp_ser_flistp, PIN_FLD_BAL_INFO_INDEX, ser_flistp, PIN_FLD_BAL_INFO_INDEX, ebufp);
		
		ser_code_flistp = PIN_FLIST_ELEM_GET(tmp_ser_flistp, PIN_FLD_SERVICE_CODES, PIN_ELEMID_ANY, 1, ebufp);
		ser1_code_flistp = PIN_FLIST_ELEM_ADD(ser_flistp, PIN_FLD_SERVICE_CODES, 1, ebufp);
		PIN_FLIST_FLD_COPY(ser_code_flistp, PIN_FLD_STATE_ID, ser1_code_flistp, PIN_FLD_STATE_ID, ebufp);
	
		v_line_id = PIN_FLIST_FLD_GET(ser_flistp, PIN_FLD_LOGIN, 0, ebufp);
	        if (v_line_id)
	        {
	                elem_id1 = 0;
	                cookie1 = NULL;
	                while ((alias_listp = PIN_FLIST_ELEM_GET_NEXT(tmp_ser_flistp, PIN_FLD_ALIAS_LIST, &elem_id1, 1, &cookie1, ebufp)) != (pin_flist_t *)NULL)
	                {
	                        alias_list_flistp = PIN_FLIST_ELEM_ADD(ser_flistp, PIN_FLD_ALIAS_LIST, cnt, ebufp);
	                        PIN_FLIST_FLD_COPY(alias_listp, PIN_FLD_NAME, alias_list_flistp, PIN_FLD_NAME, ebufp);
	                        cnt++;
	                }
	        }
		//if (PIN_FLIST_SUBSTR_GET(tmp_ser_flistp, PIN_FLD_INHERITED_INFO, 0, ebufp) != NULL)
		if (elem_cnt ==1)
	        {
			inher_info_flistp = PIN_FLIST_SUBSTR_GET(tmp_ser_flistp, PIN_FLD_INHERITED_INFO, 0, ebufp);
			ser_ip_flistp = PIN_FLIST_SUBSTR_GET(inher_info_flistp, PIN_FLD_SERVICE_IP, 0, ebufp);
	
			inher1_info_flistp = PIN_FLIST_SUBSTR_ADD(ser_flistp, PIN_FLD_INHERITED_INFO, ebufp);
		  	ser_ip1_flistp = PIN_FLIST_SUBSTR_ADD(inher1_info_flistp, PIN_FLD_SERVICE_IP, ebufp);
       			PIN_FLIST_FLD_COPY(ser_ip_flistp, PIN_FLD_STATUS_MSG, ser_ip1_flistp, PIN_FLD_STATUS_MSG, ebufp);
		}
		elem_p_id = 0;
		cookie2 = NULL;
		while ((tmp_prof_flistp = PIN_FLIST_ELEM_GET_NEXT(tmp_ser_flistp, PIN_FLD_PROFILES,  &elem_p_id, 1, &cookie2, ebufp)) != (pin_flist_t *)NULL)
		{
		PIN_ERR_LOG_FLIST(3, "Profiles: tmp_prof_flistp1", tmp_prof_flistp);
		if (elem_cnt == 1)
		{
			inher2_info_flistp = PIN_FLIST_SUBSTR_GET(tmp_prof_flistp, PIN_FLD_INHERITED_INFO, 0, ebufp);
			attr_info_flistp = PIN_FLIST_SUBSTR_GET(inher2_info_flistp, PIN_FLD_ATTRIBUTE_INFO, 0, ebufp);
			v_first_name = PIN_FLIST_FLD_GET(attr_info_flistp, PIN_FLD_FIRST_NAME, 0, ebufp);
			v_last_name = PIN_FLIST_FLD_GET(attr_info_flistp, PIN_FLD_LAST_NAME, 0, ebufp);
	
	/*
	v_salutation = PIN_FLIST_FLD_GET(attr_info_flistp, PIN_FLD_TITLE, 0, ebufp);
	v_staff_ccc = PIN_FLIST_FLD_GET(attr_info_flistp, MRB_FLD_STAFF_CCC, 0, ebufp);
	v_company = PIN_FLIST_FLD_GET(attr_info_flistp, PIN_FLD_COMPANY, 0, ebufp);
	v_id_type = PIN_FLIST_FLD_GET(attr_info_flistp, MRB_FLD_ID_TYPE, 0, ebufp);
	v_l_gprs_opt_out = PIN_FLIST_FLD_GET(attr_info_flistp, PIN_FLD_OPERATOR_STR, 0, ebufp);
	v_r_gprs_opt_out = PIN_FLIST_FLD_GET(attr_info_flistp, PIN_FLD_STATUS_STR, 0, ebufp);
	v_e_gprs_opt_out = PIN_FLIST_FLD_GET(attr_info_flistp, PIN_FLD_STRING, 0, ebufp);
	v_idd_roam_opt_in = PIN_FLIST_FLD_GET(attr_info_flistp, PIN_FLD_STATUS_MSG, 0, ebufp);
	v_idd_roam_threshold = PIN_FLIST_FLD_GET(attr_info_flistp, PIN_FLD_STATUS_FLAGS, 0, ebufp);
	v_JOC_CITY = PIN_FLIST_FLD_GET(attr_info_flistp, PIN_FLD_DESTINATION_NETWORK, 0, ebufp);
	v_opt_out_pccw = PIN_FLIST_FLD_GET(attr_info_flistp, MRB_FLD_DELIVERY_DESCR4, 0, ebufp);
	v_opt_out_sms = PIN_FLIST_FLD_GET(attr_info_flistp, MRB_FLD_DELIVERY_DESCR3, 0, ebufp);
	v_opt_out_email = PIN_FLIST_FLD_GET(attr_info_flistp, MRB_FLD_DELIVERY_DESCR2, 0, ebufp);
	v_opt_out_dm = PIN_FLIST_FLD_GET(attr_info_flistp, MRB_FLD_DELIVERY_DESCR1, 0, ebufp);
	*/
	prof1_flistp = PIN_FLIST_ELEM_ADD(ser_flistp, PIN_FLD_PROFILES, 0, ebufp);
	inher3_info_flistp = PIN_FLIST_SUBSTR_ADD(prof1_flistp, PIN_FLD_INHERITED_INFO, ebufp);
	attr1_info_flistp = PIN_FLIST_SUBSTR_ADD(inher3_info_flistp, PIN_FLD_ATTRIBUTE_INFO, ebufp);
	PIN_ERR_LOG_FLIST(3, "inherited_info flist: ",ser_flistp);
	if (v_last_name)
	{
	    PIN_FLIST_FLD_SET(attr1_info_flistp, PIN_FLD_LAST_NAME, v_last_name, ebufp);
	}
		
	if (v_staff_ccc)
	{
	    PIN_FLIST_FLD_SET(attr1_info_flistp, MRB_FLD_STAFF_CCC, v_staff_ccc, ebufp);
	}
		
	if (v_first_name)
	{
	     PIN_FLIST_FLD_SET(attr1_info_flistp, PIN_FLD_FIRST_NAME, v_first_name, ebufp)
	}
		 
	if (v_company)
	{
	     PIN_FLIST_FLD_SET(attr1_info_flistp, PIN_FLD_COMPANY, v_company, ebufp);
	}
	
	if (v_salutation)
	{
	      PIN_FLIST_FLD_SET(attr1_info_flistp, PIN_FLD_TITLE, v_salutation, ebufp);
	}	

    	if (v_id_type)
	{
	    PIN_FLIST_FLD_SET(attr1_info_flistp, MRB_FLD_ID_TYPE, v_id_type, ebufp);
	}

    	if (v_l_gprs_opt_out)
	{
	    PIN_FLIST_FLD_SET(attr1_info_flistp, PIN_FLD_OPERATOR_STR, v_l_gprs_opt_out, ebufp);
	}
    	
	if (v_r_gprs_opt_out)
	{
        	PIN_FLIST_FLD_SET(attr1_info_flistp, PIN_FLD_STATUS_STR, v_r_gprs_opt_out, ebufp);
	}
   		 
    	if (v_e_gprs_opt_out)
	{
	      PIN_FLIST_FLD_SET(attr1_info_flistp, PIN_FLD_STRING, v_e_gprs_opt_out, ebufp);
	}
		  
	if (v_idd_roam_opt_in)
	{
	       PIN_FLIST_FLD_SET(attr1_info_flistp, PIN_FLD_STATUS_MSG, v_idd_roam_opt_in, ebufp);
	}
		   
	if (v_idd_roam_threshold)
	{
	       PIN_FLIST_FLD_SET(attr1_info_flistp, PIN_FLD_STATUS_FLAGS, v_idd_roam_threshold, ebufp);
	}
		   
    	if (v_JOC_CITY)
	{
	     PIN_FLIST_FLD_SET(attr1_info_flistp, PIN_FLD_DESTINATION_NETWORK, v_JOC_CITY, ebufp);
	}
		 
	if (v_opt_out_pccw)
	{
	      PIN_FLIST_FLD_SET(attr1_info_flistp, MRB_FLD_DELIVERY_DESCR4, v_opt_out_pccw, ebufp);
	}
		  
	if (v_opt_out_sms)
	{
		PIN_FLIST_FLD_SET(attr1_info_flistp, MRB_FLD_DELIVERY_DESCR3, v_opt_out_sms, ebufp);
	}
		
    	if (v_opt_out_email)
	{
		PIN_FLIST_FLD_SET(attr1_info_flistp, MRB_FLD_DELIVERY_DESCR2, v_opt_out_email, ebufp);
	}
		
	if (v_opt_out_dm)
	{
		PIN_FLIST_FLD_SET(attr1_info_flistp, MRB_FLD_DELIVERY_DESCR1, v_opt_out_dm, ebufp);
	}
	if(PIN_FLIST_FLD_GET(tmp_prof_flistp, PIN_FLD_PROFILE_OBJ, 0, ebufp) != NULL)
	{	
		PIN_FLIST_FLD_COPY(tmp_prof_flistp, PIN_FLD_PROFILE_OBJ, prof1_flistp, PIN_FLD_PROFILE_OBJ, ebufp);
	}	
	}
	PIN_ERR_LOG_FLIST(3, "profile flist", prof1_flistp);
	}

	PIN_ERR_LOG_FLIST(3, "after profile flist", in_flistp);

	ser_sub_obj = PIN_POID_CREATE(db, "/service/subscription", -1, ebufp);
	//deal_obj = PIN_POID_CREATE(db, "/deal", -1, ebufp);

	bal_info_flistp2 = PIN_FLIST_ELEM_ADD(ser_flistp, PIN_FLD_BAL_INFO, 1, ebufp);
	PIN_FLIST_FLD_SET(ser_flistp, PIN_FLD_DEAL_OBJ, NULL, ebufp);
        PIN_FLIST_FLD_PUT(ser_flistp, PIN_FLD_SERVICE_OBJ, ser_sub_obj, ebufp);
	
	sprintf(msg, "service count: %d, DB: %d", elem_cnt, db);
	PIN_ERR_LOG_MSG(3, msg);
	PIN_ERR_LOG_FLIST(3, "near deals tmp_ser_flistp", tmp_ser_flistp);	

        v_sub_tel = PIN_POID_CREATE(db, "/service/telco/gsm/telephony", -1, ebufp);
        v_sub_sms = PIN_POID_CREATE(db, "/service/telco/gsm/sms", -1, ebufp);
        v_sub_mms = PIN_POID_CREATE(db, "/service/telco/gsm/mms", -1, ebufp);
        v_sub_data = PIN_POID_CREATE(db, "/service/telco/gsm/data", -1, ebufp);
        v_sub_bb = PIN_POID_CREATE(db, "/service/broadband", -1, ebufp);
        v_sub_data_mc = PIN_POID_CREATE(db, "/service/telco/gsm/data/mc", -1, ebufp);
	if (elem_cnt == 2) {
		PIN_FLIST_FLD_SET(ser_flistp, PIN_FLD_SERVICE_OBJ, v_sub_tel, ebufp); 
	}
        if (elem_cnt == 3 && v_sub_sms) {
		PIN_FLIST_FLD_SET(ser_flistp, PIN_FLD_SERVICE_OBJ, v_sub_sms, ebufp);
          }
        if (elem_cnt == 4 && v_sub_mms) {
                PIN_FLIST_FLD_SET(ser_flistp, PIN_FLD_SERVICE_OBJ, v_sub_mms, ebufp);
          }
        if (elem_cnt == 5 && v_sub_data) {
                PIN_FLIST_FLD_SET(ser_flistp, PIN_FLD_SERVICE_OBJ, v_sub_data, ebufp);
          }
        if (elem_cnt == 6 && v_sub_bb) {
		PIN_FLIST_FLD_SET(ser_flistp, PIN_FLD_SERVICE_OBJ, v_sub_bb, ebufp);
	  }
	if (elem_cnt == 7 && v_sub_data_mc) {
		PIN_FLIST_FLD_SET(ser_flistp, PIN_FLD_SERVICE_OBJ, v_sub_data_mc, ebufp);
	  }	 

	PIN_ERR_LOG_FLIST(3, "before deals ser_flist", ser_flistp);	
	sprintf(msg, "d_count: %d", elem_cnt);
	PIN_ERR_LOG_MSG(3, msg);
		
	deal_obj = PIN_FLIST_FLD_GET(tmp_ser_flistp, PIN_FLD_DEAL_OBJ, 0, ebufp);
	
        deal_id = PIN_POID_GET_ID(deal_obj);
        PIN_ERR_LOG_POID(3,"deal1_obj", deal_obj);
	sprintf(msg, "deal_id %d", deal_id);
        PIN_ERR_LOG_MSG(3, msg);

 	if (deal_id > 0 || deal_obj != NULL) 
	{
		PIN_ERR_LOG_POID(3,"deal1_obj", deal_obj);
		PIN_FLIST_FLD_SET(ser_flistp, PIN_FLD_DEAL_OBJ, deal_obj, ebufp);
		deals1_flistp = PIN_FLIST_ELEM_GET(tmp_ser_flistp, PIN_FLD_DEALS, 0, 1, ebufp);
		PIN_ERR_LOG_FLIST(3, "get deals1_flist", deals1_flistp);
		
		deals_flistp = PIN_FLIST_ELEM_ADD(ser_flistp, PIN_FLD_DEALS, 1, ebufp);
    		PIN_FLIST_FLD_SET(deals_flistp, PIN_FLD_DEAL_OBJ, deal_obj, ebufp);
		PIN_FLIST_FLD_COPY(deals1_flistp, PIN_FLD_NAME, deals_flistp, PIN_FLD_NAME, ebufp);
	deal_elem_id = 0;
	cookie3 = NULL;
	while ((tmp_prod_flistp = PIN_FLIST_ELEM_GET_NEXT(deals1_flistp, PIN_FLD_PRODUCTS,
                           &deal_elem_id, 1, &cookie3, ebufp)) != (pin_flist_t *)NULL)
        {
		prod_flistp = PIN_FLIST_ELEM_ADD(deals_flistp, PIN_FLD_PRODUCTS, deal_elem_cnt, ebufp);
   		PIN_FLIST_FLD_COPY(tmp_prod_flistp, PIN_FLD_PRODUCT_OBJ, prod_flistp, PIN_FLD_PRODUCT_OBJ, ebufp);
   		PIN_FLIST_FLD_COPY(tmp_prod_flistp, PIN_FLD_CYCLE_DISCOUNT, prod_flistp, PIN_FLD_CYCLE_DISCOUNT, ebufp);
   		PIN_FLIST_FLD_COPY(tmp_prod_flistp, PIN_FLD_CYCLE_END_DETAILS, prod_flistp, PIN_FLD_CYCLE_END_DETAILS, ebufp);
   		PIN_FLIST_FLD_COPY(tmp_prod_flistp, PIN_FLD_CYCLE_END_T, prod_flistp, PIN_FLD_CYCLE_END_T, ebufp);
  		PIN_FLIST_FLD_COPY(tmp_prod_flistp, PIN_FLD_CYCLE_START_DETAILS, prod_flistp, PIN_FLD_CYCLE_START_DETAILS, ebufp);
   		PIN_FLIST_FLD_COPY(tmp_prod_flistp, PIN_FLD_CYCLE_START_T, prod_flistp, PIN_FLD_CYCLE_START_T, ebufp);
   		PIN_FLIST_FLD_COPY(tmp_prod_flistp, PIN_FLD_PURCHASE_DISCOUNT, prod_flistp, PIN_FLD_PURCHASE_DISCOUNT, ebufp);
   		PIN_FLIST_FLD_COPY(tmp_prod_flistp, PIN_FLD_PURCHASE_END_DETAILS, prod_flistp, PIN_FLD_PURCHASE_END_DETAILS, ebufp);
   		PIN_FLIST_FLD_COPY(tmp_prod_flistp, PIN_FLD_PURCHASE_END_T, prod_flistp, PIN_FLD_PURCHASE_END_T, ebufp);
   		PIN_FLIST_FLD_COPY(tmp_prod_flistp, PIN_FLD_PURCHASE_START_DETAILS, prod_flistp, PIN_FLD_PURCHASE_START_DETAILS, ebufp);
   		PIN_FLIST_FLD_COPY(tmp_prod_flistp, PIN_FLD_PURCHASE_START_T, prod_flistp, PIN_FLD_PURCHASE_START_T, ebufp);
   		PIN_FLIST_FLD_COPY(tmp_prod_flistp, PIN_FLD_USAGE_DISCOUNT, prod_flistp, PIN_FLD_USAGE_DISCOUNT, ebufp);
   		PIN_FLIST_FLD_COPY(tmp_prod_flistp, PIN_FLD_USAGE_END_DETAILS, prod_flistp, PIN_FLD_USAGE_END_DETAILS, ebufp);
   		PIN_FLIST_FLD_COPY(tmp_prod_flistp, PIN_FLD_USAGE_END_T, prod_flistp, PIN_FLD_USAGE_END_T, ebufp);
   		PIN_FLIST_FLD_COPY(tmp_prod_flistp, PIN_FLD_USAGE_START_DETAILS, prod_flistp, PIN_FLD_USAGE_START_DETAILS, ebufp);
   		PIN_FLIST_FLD_COPY(tmp_prod_flistp, PIN_FLD_USAGE_START_T, prod_flistp, PIN_FLD_USAGE_START_T, ebufp);
   		PIN_FLIST_FLD_COPY(tmp_prod_flistp, PIN_FLD_QUANTITY, prod_flistp, PIN_FLD_QUANTITY, ebufp);
   		PIN_FLIST_FLD_COPY(tmp_prod_flistp, PIN_FLD_STATUS, prod_flistp, PIN_FLD_STATUS, ebufp);
   		PIN_FLIST_FLD_COPY(tmp_prod_flistp, PIN_FLD_STATUS_FLAGS, prod_flistp, PIN_FLD_STATUS_FLAGS, ebufp);
   		PIN_FLIST_FLD_COPY(tmp_prod_flistp, PIN_FLD_DESCR, prod_flistp, PIN_FLD_DESCR, ebufp);
	deal_elem_cnt++;
	PIN_ERR_LOG_FLIST(3, "Products flist prod_flistp ", prod_flistp);
	}
	deal_elem_cnt = 0;
	deal_elem_id = 0;
	cookie3 = NULL;
	while ((tmp_disc_flistp = PIN_FLIST_ELEM_GET_NEXT(deals1_flistp, PIN_FLD_DISCOUNTS,
                           &deal_elem_id, 1, &cookie3, ebufp)) != (pin_flist_t *)NULL)
        {
                disc_flistp = PIN_FLIST_ELEM_ADD(deals_flistp, PIN_FLD_DISCOUNTS, deal_elem_cnt, ebufp);
                deal_elem_cnt =  deal_elem_cnt + 1;
                PIN_FLIST_FLD_COPY(tmp_disc_flistp, PIN_FLD_CYCLE_END_DETAILS, disc_flistp, PIN_FLD_CYCLE_END_DETAILS, ebufp);
                PIN_FLIST_FLD_COPY(tmp_disc_flistp, PIN_FLD_CYCLE_END_T, disc_flistp, PIN_FLD_CYCLE_END_T, ebufp);
                PIN_FLIST_FLD_COPY(tmp_disc_flistp, PIN_FLD_CYCLE_START_DETAILS, disc_flistp, PIN_FLD_CYCLE_START_DETAILS, ebufp);
                PIN_FLIST_FLD_COPY(tmp_disc_flistp, PIN_FLD_CYCLE_START_T, disc_flistp, PIN_FLD_CYCLE_START_T, ebufp);
                PIN_FLIST_FLD_COPY(tmp_disc_flistp, PIN_FLD_DISCOUNT_OBJ, disc_flistp, PIN_FLD_DISCOUNT_OBJ, ebufp);
                PIN_FLIST_FLD_COPY(tmp_disc_flistp, PIN_FLD_PURCHASE_END_DETAILS, disc_flistp, PIN_FLD_PURCHASE_END_DETAILS, ebufp);
                PIN_FLIST_FLD_COPY(tmp_disc_flistp, PIN_FLD_PURCHASE_END_T, disc_flistp, PIN_FLD_PURCHASE_END_T, ebufp);
                PIN_FLIST_FLD_COPY(tmp_disc_flistp, PIN_FLD_START_DETAILS, disc_flistp, PIN_FLD_START_DETAILS, ebufp);
                PIN_FLIST_FLD_COPY(tmp_disc_flistp, PIN_FLD_START_T, disc_flistp, PIN_FLD_START_T, ebufp);
                PIN_FLIST_FLD_COPY(tmp_disc_flistp, PIN_FLD_QUANTITY, disc_flistp, PIN_FLD_QUANTITY, ebufp);
                PIN_FLIST_FLD_COPY(tmp_disc_flistp, PIN_FLD_STATUS, disc_flistp, PIN_FLD_STATUS, ebufp);
                PIN_FLIST_FLD_COPY(tmp_disc_flistp, PIN_FLD_STATUS_FLAGS, disc_flistp, PIN_FLD_STATUS_FLAGS, ebufp);
                PIN_FLIST_FLD_COPY(tmp_disc_flistp, PIN_FLD_USAGE_START_DETAILS, disc_flistp, PIN_FLD_USAGE_START_DETAILS, ebufp);
                PIN_FLIST_FLD_COPY(tmp_disc_flistp, PIN_FLD_USAGE_START_T, disc_flistp, PIN_FLD_USAGE_START_T, ebufp);
                PIN_FLIST_FLD_COPY(tmp_disc_flistp, PIN_FLD_USAGE_END_DETAILS, disc_flistp, PIN_FLD_USAGE_END_DETAILS, ebufp);
                PIN_FLIST_FLD_COPY(tmp_disc_flistp, PIN_FLD_USAGE_END_T, disc_flistp, PIN_FLD_USAGE_END_T, ebufp);

                PIN_ERR_LOG_FLIST(3, "Discounts flist disc_flistp ", disc_flistp);
        }
	}
elem_cnt++;
PIN_ERR_LOG_FLIST(3, "end of loop ser_flistp", ser_flistp);
}   

PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "end fm_rbs_cust_line_activate input_flist", in_flistp);
PCM_OP(ctxp, PCM_OP_CUST_MODIFY_CUSTOMER, 0, in_flistp, &ret_flistp, ebufp);
PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_rbs_cust_line_activate return_flist", ret_flistp);

if (PIN_ERRBUF_IS_ERR(ebufp))
{
 	PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"fm_rbs_cust_line_activate error after excecution", ebufp);
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_rbs_cust_line_activate input flist", in_flistp);
        PIN_ERRBUF_RESET(ebufp);
	return;
}
*r_flistp = PIN_FLIST_COPY(ret_flistp, ebufp);
PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_rbs_cust_line_activate output flist", *r_flistp);
return;

}

