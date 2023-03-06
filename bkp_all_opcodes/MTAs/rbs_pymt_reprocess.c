/*******************************************************************
 *
 *    Copyright (c) 1999-2007 Oracle. All rights reserved.
 *
 *    This material is the confidential property of Oracle Corporation
 *    or its licensors and may be used, reproduced, stored or transmitted
 *    only in accordance with a valid Oracle license or sublicense agreement.
 *    ====================================================================
 *    This application is created to do Automatic Payment Allocation for 
 *    Customer Accounts (Migrated / Created) having credit before billing
 *    and billing process not allocating the items.
 *
 *    Author: Vilva Sabarikanth
 *    Date: 14-May-2014
 *    Version: 1.0 
 *
 *******************************************************************/

#ifndef lint
static  char    Sccs_id[] = "@(#)%Portal Version: pin_mta_test.c:CUPmod7.3PatchInt:1:2007-Feb-07 06:51:33 %";
#endif

#include <stdio.h>

#include "pcm.h"
#include "pin_errs.h"
#include "pinlog.h"
#include "pin_mta.h"
#include "pin_flds.h"
#include "fm_rbs_pymt.h"

time_t get_unix_timestamp_from_str(const char *date_str);
FILE      *OUTPUT_FILE = NULL;
int       rec_count = 0;
int       success_count = 0;
int       failed_count = 0;
int	  records = 0;

/*******************************************************************
 * Configuration of application
 * Called prior MTA_CONFIG policy opcode
 *******************************************************************/
PIN_EXPORT void 
pin_mta_config(
	pin_flist_t		*param_flistp,
	pin_flist_t		*app_flistp,
	pin_errbuf_t	*ebufp)
{
	int32			rec_id = 0;
	pin_cookie_t	cookie = 0;
	pin_cookie_t	prev_cookie = 0;
	pin_flist_t		*flistp = 0;
	char			*option = 0;
	int32			mta_flags = 0;
	int32			err = 0;
	int32			i = 0;
	void			*vp = 0;
	int32			i_param_count = 0;
	char			*output_file = NULL;
	char			fdate[80];
	char			fmdate[80];
	char			output_fname[255]="";
	char			delimiter = '/';
	char			msg[20];
	char			tmp[256];
    	time_t          	end_t = 0;
	time_t			pvt = (time_t)0;

	if (PIN_ERR_IS_ERR(ebufp)) {
		return;
	}
	PIN_ERRBUF_CLEAR (ebufp);

	PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG, "pin_mta_config parameters flist", 
		param_flistp);
	PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG, "pin_mta_config application info flist", 
		app_flistp);
    
	vp = PIN_FLIST_FLD_GET (app_flistp, PIN_FLD_FLAGS, 0,ebufp);
	if(vp)
		mta_flags = *((int32*)vp);

	/***********************************************************
	 * The new flag MTA_FLAG_VERSION_NEW has been introduced to
	 * differentiate between new mta & old mta applications as
	 * only new applications have the ability to drop PIN_FLD_PARAMS
	 * for valid parameters . It is only for new apps that
	 * checking err_params_cnt is valid, otherwise for old applications
	 * this check without a distincion for new applications would hamper
	 * normal functioning.
	 ***********************************************************/
	mta_flags = mta_flags | MTA_FLAG_VERSION_NEW ;
	i_param_count = PIN_FLIST_ELEM_COUNT(param_flistp, PIN_FLD_PARAMS, ebufp);


	PIN_FLIST_FLD_SET(app_flistp, PIN_FLD_FLAGS, &mta_flags, ebufp);

	PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG, "pin_mta_config application info flist", 
		app_flistp);

	pin_conf("Pymt_reprocess_output_file", "output_file", PIN_FLDT_STR,
                                        &output_file, &err);
        pvt = pin_virtual_time((time_t *)NULL);
	strftime(fdate, sizeof(fdate), "%Y%m%d", localtime(&pvt));
        strftime(fmdate, sizeof(fmdate), "%Y%m%d.%H:%M", localtime(&pvt));
        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, fdate);

        sprintf(output_fname, "%s", output_file);
	strcat(output_fname, fdate);
        strcat(output_fname, ".csv");

        OUTPUT_FILE = fopen(output_fname, "w");
        fprintf(OUTPUT_FILE, "payment reprocess starting time:%s\n", fmdate);
	fprintf(OUTPUT_FILE, "Record Number,Account Number,Credit Card No,Trans Date,Amount,Channel,REASON,Try Count,Status,Success/Fail Description\n");
        fflush(OUTPUT_FILE);
	
	if (PIN_ERR_IS_ERR(ebufp)) {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                        "pin_mta_config error", ebufp);
        }
	return;
}

/*******************************************************************
 * Usage information for the specific app.
 * Called prior MTA_USAGE policy opcode
 *******************************************************************/
PIN_EXPORT void
pin_mta_usage(
	char    *prog)
{

	pin_errbuf_t		ebuf;
	pin_flist_t			*ext_flistp = NULL;
	char				*usage_str = NULL;
	char				*format = "\nUsage:     %s [-verbose] \n\n";
            
	PIN_ERRBUF_CLEAR (&ebuf);
    
	usage_str = (char*)pin_malloc( strlen(format) + strlen(prog) + 1 );

	if (usage_str == NULL) {
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR,"No Memory error");
		return;
	}

	sprintf(usage_str, format ,prog);

	ext_flistp = pin_mta_global_flist_node_get_no_lock (PIN_FLD_EXTENDED_INFO, &ebuf);
	PIN_FLIST_FLD_SET(ext_flistp, PIN_FLD_DESCR, usage_str, &ebuf);

	if (PIN_ERR_IS_ERR(&ebuf)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"pin_mta_usage error", &ebuf);
	}
	if (usage_str) {
		pin_free(usage_str);
	}

	return;
}

/*******************************************************************
 * Usage information for the specific app.
 * Called after MTA_USAGE policy opcode
 * Information passed from customization layer
 * can be processed and displayed here
 *******************************************************************/
PIN_EXPORT void 
pin_mta_post_usage(
	pin_flist_t		*param_flistp,
	pin_flist_t		*app_flistp,
	pin_errbuf_t	*ebufp)
{
	pin_flist_t		*ext_flistp = NULL;
	void			*vp = 0;

	if (PIN_ERR_IS_ERR(ebufp)) {
		return;
	}
	PIN_ERRBUF_CLEAR (ebufp);

	PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG, "pin_mta_post_usage parameters flist", 
		param_flistp);
	PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG, "pin_mta_post_usage application info flist", 
		app_flistp);

	ext_flistp =  pin_mta_global_flist_node_get_no_lock (PIN_FLD_EXTENDED_INFO, ebufp);

	vp = PIN_FLIST_FLD_GET (ext_flistp, PIN_FLD_DESCR, 1, ebufp);

	if(vp) {
		printf("%s",(char*) vp);
	}

	if (PIN_ERR_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"pin_mta_post_usage error", ebufp);
	}
	return;
}

/*******************************************************************
 * Application defined search criteria.
 * Called prior MTA_INIT_SEARCH policy opcode
 *******************************************************************/
PIN_EXPORT void
pin_mta_init_search(
	pin_flist_t		*app_flistp,
	pin_flist_t		**s_flistpp,
	pin_errbuf_t	*ebufp)
{

	pin_flist_t		*s_flistp = NULL;
	pin_flist_t		*tmp_flistp = NULL;
    	char            	*template = "select X from /rbs_rejected_payment where F1 = V1 ";
	int32			s_flags = 256;
	poid_t			*s_pdp = NULL;
	poid_t			*pdp = NULL;
	int32           	i = 1;
    	int64           	id = -1;
    	int64           	db = 0;
	char			tmp[256];
    	time_t          	current_t = 0;
	void			*vp = NULL;


	if (PIN_ERR_IS_ERR(ebufp)) 
	{
		return;
	}
	PIN_ERRBUF_CLEAR (ebufp);
	*s_flistpp = 0;

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "pin_mta_init_search application info flist", app_flistp);
    
    s_flistp = PIN_FLIST_CREATE(ebufp);
    
    vp = PIN_FLIST_FLD_GET (app_flistp, PIN_FLD_POID_VAL, 0, ebufp);
    
    if(vp)
        db = PIN_POID_GET_DB ((poid_t*)vp);

    s_pdp = PIN_POID_CREATE(db, "/search", id, ebufp);
    PIN_FLIST_FLD_PUT(s_flistp, PIN_FLD_POID,
                      (void *)s_pdp, ebufp);

    PIN_FLIST_FLD_SET (s_flistp, PIN_FLD_FLAGS, &s_flags, ebufp);
    tmp_flistp = PIN_FLIST_ELEM_ADD (s_flistp, PIN_FLD_ARGS, 1, ebufp);
    PIN_FLIST_FLD_SET (tmp_flistp, PIN_FLD_STATUS_FLAGS, &i, ebufp);

    PIN_FLIST_FLD_SET (s_flistp, PIN_FLD_TEMPLATE, template, ebufp);
    tmp_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_RESULTS, 0, ebufp);

    /***********************************************************
     * Search results
     ***********************************************************/
    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "pin_mta_init_search search flist", s_flistp);
	
    if (PIN_ERR_IS_ERR(ebufp)) 
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "pin_mta_init_search error", ebufp);
		PIN_FLIST_DESTROY_EX (&s_flistp,0);
	}
	else
	{
		*s_flistpp = PIN_FLIST_COPY(s_flistp, ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "pin_mta_init_search search flist", s_flistp);
	}
	PIN_FLIST_DESTROY_EX(&s_flistp, NULL);
	return;
}


/*******************************************************************
 * Search results are modified to consider extra outouts into worker opcode
 * Called after MTA_TUNE policy opcode
 *******************************************************************/
PIN_EXPORT void 
pin_mta_post_tune(
	pin_flist_t		*app_flistp,
	pin_flist_t		*srch_op_in_flistp,
	pin_errbuf_t	*ebufp)
{
	pin_flist_t		*rest_flistp=NULL;
	pin_flist_t		*multirest_flistp=NULL;
	pin_cookie_t	cookie = NULL;
	int				elem_id = 0;
	if (PIN_ERR_IS_ERR(ebufp)) {
		return;
	}
	PIN_ERRBUF_CLEAR (ebufp);

	PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG, "pin_mta_post_tune application info flist", 
		app_flistp);
	PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG, "pin_mta_post_tune search results flist", 
		srch_op_in_flistp);
	multirest_flistp = PIN_FLIST_ELEM_GET(srch_op_in_flistp,PIN_FLD_MULTI_RESULTS,PIN_ELEMID_ANY,1,ebufp);
	while((rest_flistp = PIN_FLIST_ELEM_GET_NEXT(multirest_flistp, PIN_FLD_RESULTS,
		&elem_id, 1, &cookie, ebufp)) != (pin_flist_t *)NULL)
	{
		PIN_FLIST_ELEM_COPY(multirest_flistp, PIN_FLD_EXTRA_RESULTS, elem_id + 1, rest_flistp, PIN_FLD_EXTRA_RESULTS, 0, ebufp);
		PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG, "pin_mta_post_tune search results modify flist", rest_flistp);
		//records = (int)elem_id;
	}
	if (PIN_ERR_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			 "pin_mta_post_tune error", ebufp);
	}
	PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG, "pin_mta_post_tune search results output flist", srch_op_in_flistp);

	return;
}

/*******************************************************************
 * Main application opcode is called here
 *******************************************************************/
PIN_EXPORT void
pin_mta_worker_opcode(
	pcm_context_t		*ctxp,
	pin_flist_t			*srch_op_in_flistp,
	pin_flist_t			*op_in_flistp,
	pin_flist_t			**op_out_flistpp,
	pin_flist_t			*ti_flistp,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t			*in_flistp=NULL;
	pin_flist_t			*wf_iflistp=NULL;
	pin_flist_t			*wf_oflistp=NULL;
	time_t				*created_t = NULL;
	time_t				*current_t = NULL;
	time_t				susp_t = 0;
	time_t				now_t = pin_virtual_time(NULL);
	struct tm			*timeeff;
	char				tmp[256];
	char				msg[200];
	char				*acc_no = NULL;
	char				*err_descr = NULL;
	int32				*status = NULL;
	int32				*reprocess_days = NULL;
	int32				rep_days = 0;
	int32				susp_days = 0;
	int32               		status_done = 1;
	int32				out_status = 0;
	int32				errp = 0;
	char				buffer[1024];
	char				*acc_id_str = NULL;
	char				*credit_num = NULL;
	char				*channel = NULL;
	time_t				*eff_t = NULL;
	time_t				*exp_t = NULL;
	pin_decimal_t			*amount = pbo_decimal_from_str("0.0", ebufp);
	int32				*reason = NULL;
	int32				*rej_cnt = NULL;


	if (PIN_ERR_IS_ERR(ebufp)) 
	{
		return;
	}

	PIN_ERRBUF_CLEAR (ebufp);

	PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG, "pin_mta_worker_opcode search flist", srch_op_in_flistp);
	PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG, "pin_mta_worker_opcode op_in_flistp flist", op_in_flistp);  
	
	created_t = PIN_FLIST_FLD_GET(op_in_flistp, PIN_FLD_CREATED_T, 0, ebufp);
    	pin_conf("pymt_susp_reprocess_days", "pymt_susp_reprocess_period", PIN_FLDT_INT, (caddr_t*)&reprocess_days, &errp);
	if (errp != PIN_ERR_NONE)
        {
                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR,
                        "pin_conf error!");
                return;
        } 
    	timeeff = localtime(created_t);
    	current_t = (time_t *)&now_t;
    	susp_t = (time_t)(*current_t - *created_t);
    	susp_days = (int32)susp_t / 86400;

	rep_days = (int32 )*reprocess_days;
	sprintf(msg, "susp_days: %d, reprocess_days: %d, rep_days: %d", susp_days, *reprocess_days, rep_days);
	PIN_ERR_LOG_MSG(3, msg);

    	if (susp_days <= rep_days)
	{
		in_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_COPY(op_in_flistp, PIN_FLD_POID, in_flistp, PIN_FLD_POID, ebufp);
		acc_no = PIN_FLIST_FLD_GET(op_in_flistp, PIN_FLD_ACCOUNT_NO, 1, ebufp);
		PIN_FLIST_FLD_SET(in_flistp, PIN_FLD_ACCOUNT_NO, acc_no, ebufp);
		eff_t = PIN_FLIST_FLD_GET(op_in_flistp, PIN_FLD_EFFECTIVE_END_T, 0, ebufp);
		PIN_FLIST_FLD_SET(in_flistp, PIN_FLD_EFFECTIVE_END_T, eff_t, ebufp);
		amount = PIN_FLIST_FLD_GET(op_in_flistp, PIN_FLD_AMOUNT, 0, ebufp);
		PIN_FLIST_FLD_SET(in_flistp, PIN_FLD_AMOUNT, amount, ebufp);
		channel = PIN_FLIST_FLD_GET(op_in_flistp, PIN_FLD_CHANNEL, 0, ebufp);
		PIN_FLIST_FLD_SET(in_flistp, PIN_FLD_CHANNEL, channel, ebufp);
		reason = PIN_FLIST_FLD_GET(op_in_flistp, PIN_FLD_REASON, 0, ebufp);
		PIN_FLIST_FLD_SET(in_flistp, PIN_FLD_REASON, reason, ebufp);
		PIN_FLIST_FLD_COPY(op_in_flistp, PIN_FLD_EVENT_OBJ, in_flistp, PIN_FLD_EVENT_OBJ, ebufp);
		PIN_FLIST_FLD_COPY(op_in_flistp, PIN_FLD_ITEM_OBJ, in_flistp, PIN_FLD_ITEM_OBJ, ebufp);
		credit_num = PIN_FLIST_FLD_GET(op_in_flistp, PIN_FLD_DEBIT_NUM, 0, ebufp);
		PIN_FLIST_FLD_SET(in_flistp, PIN_FLD_DEBIT_NUM, credit_num, ebufp);
		status = PIN_FLIST_FLD_GET(op_in_flistp, PIN_FLD_STATUS_FLAGS, 0, ebufp);
		PIN_FLIST_FLD_SET(in_flistp, PIN_FLD_STATUS_FLAGS, status, ebufp);
		rej_cnt = PIN_FLIST_FLD_GET(op_in_flistp, PIN_FLD_ACCOUNT_CNT, 0, ebufp);
		PIN_FLIST_FLD_SET(in_flistp, PIN_FLD_ACCOUNT_CNT, rej_cnt, ebufp);
		PIN_FLIST_FLD_COPY(op_in_flistp, PIN_FLD_ACCOUNT_OBJ, in_flistp, PIN_FLD_ACCOUNT_OBJ, ebufp);
		rec_count = rec_count + 1;
		/********************************************
		*      prepare flist for opcode call in
		*           pin_mta_worker_opcode
		*********************************************/
		PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG, "pin_mta_worker_job prepared flist for main opcode after adding allocation flag", in_flistp);
		PCM_OP(ctxp, RBS_PYMT_REPROCESS, 0, in_flistp, op_out_flistpp, ebufp);
		records = records + 1;
		out_status = *(int32 *)PIN_FLIST_FLD_GET(*op_out_flistpp, PIN_FLD_STATUS, 0, ebufp);
		sprintf(msg, "%d,%s,%s,%ld,%s,%s,%d,%d,%d", records, acc_no, credit_num, *eff_t, pbo_decimal_to_str(amount, ebufp), channel, *reason, *rej_cnt, out_status);
		PIN_ERR_LOG_MSG(3, msg);		

		if (PIN_ERR_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "pin_mta_worker_opcode error", ebufp);
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_ERROR, "pin_mta_worker_job RBS_PYMT_REPROCESS failed", in_flistp);
			strcat(buffer, "Error occured while calling reprocess payment");
			fprintf(OUTPUT_FILE, "%s,%s\n", msg, buffer);
                	fflush(OUTPUT_FILE);
			
			PIN_ERR_LOG_MSG(3, "Payment Failure");
                        failed_count = failed_count + 1;
		}
		else
		{
			PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG, "pin_mta_worker_opcode output flist", *op_out_flistpp);
			status = PIN_FLIST_FLD_GET(*op_out_flistpp, PIN_FLD_STATUS, 0, ebufp);
			if (status && *status == 1)
			{
				PIN_ERR_LOG_MSG(3, "Failed");
				err_descr = PIN_FLIST_FLD_GET(*op_out_flistpp, PIN_FLD_ERROR_DESCR, 0, ebufp);
                        	fprintf(OUTPUT_FILE, "%s,FAIL - %s\n", msg, err_descr);
                        	fflush(OUTPUT_FILE);
				failed_count = failed_count + 1;
			}
			else
			{
				PIN_ERR_LOG_MSG(3, "Success");
				success_count = success_count + 1;
				fprintf(OUTPUT_FILE,"%s,SUCCESS\n", msg);
                       		fflush(OUTPUT_FILE);
			}
		}
	}
	else
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "pin_mta_worker_opcode error", ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_ERROR, "pin_mta_worker_job older data failed", in_flistp);
		PIN_ERRBUF_RESET(ebufp);
		status_done = 2;
	}
	
cleanup:
	return;
}


PIN_EXPORT void
pin_mta_worker_job_done(
        pcm_context_t   *ctxp,
        pin_flist_t     *srch_op_in_flistp,
        pin_flist_t     *op_in_flistp,
        pin_flist_t     *op_out_flistp,
        pin_flist_t     *ti_flistp,
        pin_errbuf_t    *ebufp)

{
    if (PIN_ERR_IS_ERR(ebufp)) {
        return;

    }

    PIN_ERRBUF_CLEAR (ebufp);

    PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG, "pin_mta_worker_job_done search results flist",
                                   srch_op_in_flistp);

    PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG, "pin_mta_worker_job_done prepared flist for main opcode",
                                   op_in_flistp);

    PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG, "pin_mta_worker_job_done output flist from main opcode",
                                   op_out_flistp);

    PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG, "pin_mta_worker_job_done thread info flist",
                                   ti_flistp);

    if (PIN_ERR_IS_ERR(ebufp)) {
        PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                          "pin_mta_worker_job_done error", ebufp);
    }
    return;

}

/*******************************************************************
 * Function executed at application exit
 * Called prior MTA_EXIT policy opcode
 *******************************************************************/
PIN_EXPORT void
pin_mta_exit(
        pin_flist_t     *app_flistp,
        pin_errbuf_t    *ebufp)
{
        char    record_count[255];
        char    suc_count[255];
        char    fail_cnt[255];
        time_t          pvt = (time_t)0;
        char    fdate[80];
        if (PIN_ERR_IS_ERR(ebufp)) {
                return;
        }
        PIN_ERRBUF_CLEAR (ebufp);

        PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG, "pin_mta_exit application info flist",
                                           app_flistp);
        sprintf(record_count, "Total Records Processed: %d", rec_count);
         pvt = pin_virtual_time((time_t *)NULL);
        //strftime(fdate, sizeof(fdate), "-%Y%m%d", localtime(&pvt));
        strftime(fdate, sizeof(fdate), "%d-%m-%Y.%H:%M", localtime(&pvt));

        fprintf(OUTPUT_FILE, "\n\n=========================================================\n");
        fprintf(OUTPUT_FILE, "\nDetailed Report is given below\n\n");
        fprintf(OUTPUT_FILE,"%s\n", record_count);
        sprintf(suc_count, "Successful Records Count: %d", success_count);
        fprintf(OUTPUT_FILE,"%s\n", suc_count);
        sprintf(fail_cnt, "Failed Records Count: %d", failed_count);
        fprintf(OUTPUT_FILE,"%s\n", fail_cnt);
        fprintf(OUTPUT_FILE, "\n=========================================================\n");
        fprintf(OUTPUT_FILE, "Payment Reprocess end time: %s",fdate);
        fflush(OUTPUT_FILE);
        fclose(OUTPUT_FILE);
        rec_count = 0;
        success_count = 0;
        if (PIN_ERR_IS_ERR(ebufp)) {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                                                 "pin_mta_exit error", ebufp);
        }
        return;
}

