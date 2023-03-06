/*******************************************************************
 *
 *      Copyright (c) 1999-2007 Oracle. All rights reserved.
 *
 *      This material is the confidential property of Oracle Corporation
 *      or its licensors and may be used, reproduced, stored or transmitted
 *      only in accordance with a valid Oracle license or sublicense agreement.
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

#define MAXFLDS 200
#define MAXFLDSIZE 50
FILE      *INPUT_FILE = NULL;
FILE	  *OUTPUT_FILE = NULL;
int       rec_count = 0;
int	  success_count = 0;
int 	  failed_count = 0;

void parse(
        char    *record,
        char    arr[][50],
        int     *fldcnt);

/************************************************************************
 * This function is trims the leading and trailing spaces
 ************************************************************************/
void trim(char *str);

/*******************************************************************
 * Configuration of application
 * Called prior MTA_CONFIG policy opcode
 *******************************************************************/
PIN_EXPORT void
pin_mta_config(
        pin_flist_t     *param_flistp,
        pin_flist_t     *app_flistp,
        pin_errbuf_t    *ebufp)
{
        int32           rec_id = 0;
        pin_cookie_t    cookie = 0;
        pin_cookie_t    prev_cookie = 0;
        pin_flist_t     *flistp = 0;
        char            *option = 0;
        int32           mta_flags = 0;
        void            *vp = 0;
        int32           i = 0;
        char            *value = NULL;
        char            *INPUT_FILENAME = NULL;
        char            str[1024];
        char            *output_file = NULL;
        char            fdate[80];
        char            output_fname[255]="";
	char            *input_file_atm = NULL;
	char            input_fname_atm[255] = "";
	char            *input_file_dragon = NULL;
        char            input_fname_dragon[255] = "";
	char            *input_file_pps = NULL;
        char            input_fname_pps[255] = "";
        char            buffer[1024];
        time_t          pvt = (time_t)0;
        int             err=0;


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
        //mta_flags |= MTA_FLAG_RETURN_WORKER_ERROR;
        cookie = NULL;
        while((flistp = PIN_FLIST_ELEM_GET_NEXT(param_flistp, PIN_FLD_PARAMS, &rec_id,
                                                                   1, &cookie, ebufp))!= NULL)
	{
                option = PIN_FLIST_FLD_GET (flistp, PIN_FLD_PARAM_NAME, 0, ebufp);
                value = PIN_FLIST_FLD_GET (flistp, PIN_FLD_PARAM_VALUE, 1, ebufp);
                /***************************************************
                 * Test options options.
                 ***************************************************/
                if(strcmp(option, "-i") == 0) 
		{
                        if (value == 0 || *value == 0) 
			{
                                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, "Missing input file name for -i option");

                                fprintf(stderr, "%s\n", "Missing input filename for -i option");

                                mta_flags = mta_flags | MTA_FLAG_USAGE_MSG;
                        } 
			else 
			{
                                INPUT_FILENAME = strdup(value);

                                snprintf(str, 1024, "Command line switch -i read. Input file set to '%s'.", INPUT_FILENAME);

                                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, str);
                		pin_conf("Pymt_rej_upload_input_file", "input_file_atm", PIN_FLDT_STR, &input_file_atm, &err);

                                pvt = pin_virtual_time((time_t *)NULL);
                                strftime(fdate, sizeof(fdate), "%Y%m%d", localtime(&pvt));
                                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, fdate);

                                sprintf(input_fname_atm, "%s", input_file_atm);
                                strcat(input_fname_atm, fdate);
                                strcat(input_fname_atm, ".csv");
				PIN_ERR_LOG_MSG(3, input_fname_atm);

				pin_conf("Pymt_rej_upload_input_file", "input_file_dragon", PIN_FLDT_STR, &input_file_dragon, &err);
				sprintf(input_fname_dragon, "%s", input_file_dragon);
                                strcat(input_fname_dragon, fdate);
                                strcat(input_fname_dragon, ".csv");
                                PIN_ERR_LOG_MSG(3, input_fname_dragon);

				pin_conf("Pymt_rej_upload_input_file", "input_file_pps", PIN_FLDT_STR, &input_file_pps, &err);
				sprintf(input_fname_pps, "%s", input_file_pps);
                                strcat(input_fname_pps, fdate);
                                strcat(input_fname_pps, ".csv");
                                PIN_ERR_LOG_MSG(3, input_fname_pps);

                        }

                } 
		else if ((strcmp(option, "-h") == 0) || (strcmp(option, "-help") == 0))
                {
                        mta_flags = mta_flags | MTA_FLAG_USAGE_MSG;
                        return;
                }
                else 
		{
                        mta_flags = mta_flags | MTA_FLAG_USAGE_MSG;

                        snprintf(str, 1024, "Unknown option '%s'", option);
                        PIN_ERR_LOG_MSG (PIN_ERR_LEVEL_ERROR, str);
                        fprintf(stderr, "%s\n", str);
                }
        }
	
	pin_conf("Pymt_rej_upload_output_file", "output_file", PIN_FLDT_STR,
                                        &output_file, &err);
        pvt = pin_virtual_time((time_t *)NULL);
        strftime(fdate, sizeof(fdate), "%Y-%m-%d.%H:%M", localtime(&pvt));
        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, fdate);

        sprintf(output_fname, "%s", output_file);
        strcat(output_fname, ".csv");

        OUTPUT_FILE = fopen(output_fname, "w");
        fprintf(OUTPUT_FILE, "payment Reject Upload starting time:%s\n", fdate);
        fflush(OUTPUT_FILE);	
	
	if (INPUT_FILENAME != NULL)
	{
	        if (input_fname_atm && (strcmp(INPUT_FILENAME, input_fname_atm) == 0))
		{
        	        INPUT_FILE = fopen(INPUT_FILENAME, "r+");
        	        if (INPUT_FILE == NULL)
        	        {
        	                sprintf(str, "Could not open input file '%s'", INPUT_FILENAME);
        	                fprintf(stderr, "%s", str);
        	                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, str);
        	                mta_flags = mta_flags | MTA_FLAG_USAGE_MSG;
        	        }
        	}
		else if (input_fname_dragon && (strcmp(INPUT_FILENAME, input_fname_dragon) == 0))
        	{
        	        INPUT_FILE = fopen(INPUT_FILENAME, "r+");
        	        if (INPUT_FILE == NULL)
        	        {
        	                sprintf(str, "Could not open input file '%s'", INPUT_FILENAME);
        	                fprintf(stderr, "%s", str);
        	                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, str);
        	                mta_flags = mta_flags | MTA_FLAG_USAGE_MSG;
        	        }
        	}
		else if (input_fname_pps && (strcmp(INPUT_FILENAME, input_fname_pps) == 0))
        	{
        	        INPUT_FILE = fopen(INPUT_FILENAME, "r+");
        	        if (INPUT_FILE == NULL)
        	        {
        	                sprintf(str, "Could not open input file '%s'", INPUT_FILENAME);
       	 	                fprintf(stderr, "%s", str);
                                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, str);
                        	mta_flags = mta_flags | MTA_FLAG_USAGE_MSG;
                	}
        	}
		else
        	{
        	        mta_flags = mta_flags | MTA_FLAG_USAGE_MSG;
			freopen(NULL, "w", OUTPUT_FILE);
			fprintf(OUTPUT_FILE, "Invalid File Format\n", fdate);
			fflush(OUTPUT_FILE);
	        }
	}
        PIN_FLIST_FLD_SET (app_flistp, PIN_FLD_FLAGS, &mta_flags, ebufp);

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
        char    	*prog)
{

        pin_errbuf_t    ebuf;
        pin_flist_t     *ext_flistp = NULL;
        char            usage_str[512] = {""};

        PIN_ERRBUF_CLEAR (&ebuf);

        sprintf(usage_str,"\nUsage: %s \n"
                           "\t-i <Input File>\n",
                           "\t-h or -help\n", prog);
        ext_flistp = pin_mta_global_flist_node_get_no_lock (PIN_FLD_EXTENDED_INFO, &ebuf);

        PIN_FLIST_FLD_SET (ext_flistp, PIN_FLD_DESCR, usage_str, &ebuf)

        if (PIN_ERR_IS_ERR(&ebuf)) {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                                                 "pin_mta_usage error", &ebuf);
                                                 }
        return;
}

/*******************************************************************
 * Application defined search criteria.
 * Called prior MTA_INIT_SEARCH policy opcode
 *******************************************************************/
PIN_EXPORT void
pin_mta_init_search(
        pin_flist_t     *app_flistp,
        pin_flist_t     **s_flistpp,
        pin_errbuf_t    *ebufp)
{

        int32           line_counter = 0;
        int32           fldcnt=0;
        int32           err=0;
	int32		exp_t = 0;
	int32		eff_t = 0;
        char    	buffer[1024];
        char          	arr[MAXFLDS][MAXFLDSIZE]={0x0};
        char            a_pdp_str[32];
        char            *output_file = NULL;
        char            fdate[80];
        char            output_fname[255]="";
        void            *vp = NULL;
        poid_t          *pdp_val = NULL;
        poid_t          *b_pdp = NULL;
	int64		db = -1;
        pin_flist_t     *result_flistp = NULL;
        pin_flist_t     *results_flistp = NULL;
        pin_flist_t     *actfind_flistp = NULL;
        pin_flist_t     *ractfind_flistp = NULL;
        time_t          pvt = (time_t)0;
        pcm_context_t   *ctxp = pin_mta_main_thread_pcm_context_get(ebufp);


        if (PIN_ERR_IS_ERR(ebufp)) {
                return;
        }
        PIN_ERRBUF_CLEAR (ebufp);

        results_flistp = PIN_FLIST_CREATE(ebufp);

        int              a_cntr = 0;
        *s_flistpp = 0;

        PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG, "pin_mta_init_search application info flist",
                                           app_flistp);				
	pdp_val = PIN_FLIST_FLD_GET(app_flistp, PIN_FLD_POID_VAL, 0, ebufp);
	db = PIN_POID_GET_DB(pdp_val);				   
										   
        while (fgets(buffer, sizeof buffer, INPUT_FILE) != NULL)
        {			
            	parse(buffer, arr, &fldcnt);
            	rec_count = rec_count+1;
		
		exp_t = (u_int64)(atof(arr[5]));
		eff_t = (u_int64)(atof(arr[6]));
        	PIN_FLIST_FLD_PUT(results_flistp, PIN_FLD_POID, (void *)PIN_POID_CREATE(db, "/account", -1,ebufp), ebufp);
        	result_flistp = PIN_FLIST_ELEM_ADD(results_flistp, PIN_FLD_RESULTS, a_cntr++, ebufp);
		PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_POID, pdp_val, ebufp);
        	PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_ACCOUNT_NO, arr[3], ebufp);
		PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_DEBIT_NUM, arr[4], ebufp);
		PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_EXPIRATION_T, &exp_t, ebufp);
		PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_EFFECTIVE_END_T, &eff_t, ebufp);
            	PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_AMOUNT, pbo_decimal_from_str(arr[7], ebufp), ebufp);
		PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_CHANNEL, arr[8], ebufp);
		trim(arr[9]);
		PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_REASON_CODE, arr[9], ebufp);
		PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_CHECK_SERIAL_NO, arr[2], ebufp);
		PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_STATUS_LABEL, arr[1], ebufp);
        }
        fclose(INPUT_FILE);
        if (PIN_ERR_IS_ERR(ebufp)) {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                                                 "pin_mta_init_search error", ebufp);

                PIN_FLIST_DESTROY_EX (&results_flistp, ebufp);
        }else{
				*s_flistpp = results_flistp;

                PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG, "pin_mta_init_search search flist",
                                                   results_flistp);
        }
        /*fprintf(OUTPUT_FILE, "%s\n", buffer);
        fflush(OUTPUT_FILE); */
        return;
}


/*******************************************************************
 * Main application opcode is called here
 *******************************************************************/
PIN_EXPORT void
pin_mta_worker_opcode(
        pcm_context_t   *ctxp,
        pin_flist_t     *srch_res_flistp,
        pin_flist_t     *op_in_flistp,
        pin_flist_t     **op_out_flistpp,
        pin_flist_t     *ti_flistp,
        pin_errbuf_t    *ebufp)
{
        pin_flist_t     *tmp_flistp = NULL;
        pin_flist_t     *r_flistp = NULL;
	int32		currency = 0;
	int32		pay_type = 0;
        int32           opcode = 0;
	int32		status = 0;
        int32           mode = 0;
        int32           *rec_no = 0;
	int32           *succ_flag = NULL;
        void            *vp = 0;
	char    	buffer[1024];
	char            *acc_no = NULL;
	char		*err_descr = NULL;
	char		*records = NULL;
	char		*indicator = NULL;
	char		*cc_num = NULL;
	char		*channel = NULL;
	char		*reason_code = NULL;
	time_t		*exp_t = NULL;
	time_t		*eff_t = NULL;
	pin_decimal_t	*amount = NULL;
        pin_cookie_t    cookie = 0;
        int32           rec_id = 0;
	int32		command = 0;
        pin_flist_t     *app_flistp = NULL;
        pin_flist_t     *err_flistp = NULL;
	pin_flist_t	*in_flistp = NULL;
	pin_flist_t	*chrg_flistp = NULL;
	pin_flist_t     *update_serv_opflist = NULL;
	pin_flist_t	*update_serv_ipflist = NULL;
        pin_errbuf_t    ebuf1;

        if (PIN_ERRBUF_IS_ERR(ebufp)) {
                return;
        }
        PIN_ERRBUF_CLEAR (ebufp);
        PIN_ERRBUF_CLEAR (&ebuf1);

        PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG, "pin_mta_worker_opcode search results flist", srch_res_flistp);

        PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG, "pin_mta_worker_opcode prepared flist for main opcode", op_in_flistp);

        PIN_ERR_LOG_FLIST (3, "pin_mta_worker_opcode thread info flist", ti_flistp);
/*
        app_flistp = pin_mta_global_flist_node_get_with_lock (PIN_FLD_APPLICATION_INFO, ebufp);

        vp = PIN_FLIST_FLD_GET (app_flistp, PIN_FLD_MODE, 1,ebufp);
        if(vp){
                mode = *((int32*)vp);
        }

        pin_mta_global_flist_node_release (PIN_FLD_APPLICATION_INFO, ebufp);
*/
	indicator = PIN_FLIST_FLD_GET(op_in_flistp, PIN_FLD_STATUS_LABEL, 0, ebufp);
	records = PIN_FLIST_FLD_GET(op_in_flistp, PIN_FLD_CHECK_SERIAL_NO, 0, ebufp);
	acc_no = PIN_FLIST_FLD_GET(op_in_flistp, PIN_FLD_ACCOUNT_NO, 0, ebufp);
	cc_num = PIN_FLIST_FLD_GET(op_in_flistp, PIN_FLD_DEBIT_NUM, 0, ebufp);
	exp_t = PIN_FLIST_FLD_GET(op_in_flistp, PIN_FLD_EXPIRATION_T, 0, ebufp);
	eff_t = PIN_FLIST_FLD_GET(op_in_flistp, PIN_FLD_EFFECTIVE_END_T, 0, ebufp);
	amount = PIN_FLIST_FLD_GET(op_in_flistp, PIN_FLD_AMOUNT, 0, ebufp);
	channel = PIN_FLIST_FLD_GET(op_in_flistp, PIN_FLD_CHANNEL, 0, ebufp);
	reason_code = PIN_FLIST_FLD_GET(op_in_flistp, PIN_FLD_REASON_CODE, 0, ebufp);
	
	/*************************************************************************
	* Create input flist for opcode calling 
	**************************************************************************/
	in_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_COPY(op_in_flistp, PIN_FLD_POID, in_flistp, PIN_FLD_POID, ebufp);	
	PIN_FLIST_FLD_SET(in_flistp, PIN_FLD_PROGRAM_NAME, "PAYMENT COLLECT", ebufp);
	PIN_FLIST_FLD_SET(in_flistp, PIN_FLD_DESCR, "payment collect", ebufp);
	PIN_FLIST_FLD_SET(in_flistp, PIN_FLD_CHANNEL, channel, ebufp);
	PIN_FLIST_FLD_SET(in_flistp, PIN_FLD_REASON_CODE, reason_code, ebufp);
	chrg_flistp = PIN_FLIST_ELEM_ADD(in_flistp, PIN_FLD_CHARGES, 0, ebufp);
	status = 15;
	PIN_FLIST_FLD_SET(chrg_flistp, PIN_FLD_STATUS, &status, ebufp);
	pay_type = 10011;
	PIN_FLIST_FLD_SET(chrg_flistp, PIN_FLD_PAY_TYPE, &pay_type, ebufp);
	PIN_FLIST_FLD_COPY(op_in_flistp, PIN_FLD_ACCOUNT_NO, chrg_flistp, PIN_FLD_ACCOUNT_NO, ebufp);
	PIN_FLIST_FLD_SET(chrg_flistp, PIN_FLD_COMMAND, &command, ebufp);
	currency = 344;
	PIN_FLIST_FLD_SET(chrg_flistp, PIN_FLD_CURRENCY, &currency, ebufp);
	PIN_FLIST_FLD_SET(chrg_flistp, PIN_FLD_AMOUNT, amount, ebufp);
	PIN_ERR_LOG_FLIST(3, "payment rej_upload input", in_flistp);
	opcode = RBS_PYMT_REJ_UPLOAD;
       	PCM_OP(ctxp, opcode, 0, in_flistp, op_out_flistpp, ebufp);
        if (PIN_ERR_IS_ERR(ebufp))
	{
                /***************************************************
                * Log something and return nothing.
                ***************************************************/
                strcat(buffer, ",Error occured while calling rej_upload payment");
		fprintf(OUTPUT_FILE, "%s,%s,%s,%s,%ld,%ld,%s,%s,%s,%s\n", indicator, records, acc_no, cc_num, *exp_t, *eff_t, pbo_decimal_to_str(amount, ebufp), channel, reason_code, buffer);
                fflush(OUTPUT_FILE);
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                "Error in rej_upload payment", ebufp);
        }
        else
        {
                PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "payment rej_upload output", *op_out_flistpp);
                //succ_flag = 0;
                succ_flag = PIN_FLIST_FLD_GET(*op_out_flistpp, PIN_FLD_STATUS_FLAGS, 1, ebufp);
                if(*succ_flag == 0)
                {
                        success_count = success_count + 1;
			fprintf(OUTPUT_FILE, "%s,%s,%s,%s,%ld,%ld,%s,%s,%s,SUCCESS\n", indicator, records, acc_no, cc_num, *exp_t, *eff_t, pbo_decimal_to_str(amount, ebufp), channel, reason_code);

                        fflush(OUTPUT_FILE);
                }
                else
                {
			err_descr = PIN_FLIST_FLD_GET(*op_out_flistpp, PIN_FLD_ERROR_DESCR, 0, ebufp);
			failed_count = failed_count + 1;
                        PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                                "payment collect error", ebufp);
			fprintf(OUTPUT_FILE, "%s,%s,%s,%s,%ld,%ld,%s,%s,%s,%s\n", indicator, records, acc_no, cc_num, *exp_t, *eff_t, pbo_decimal_to_str(amount, ebufp), channel, reason_code, err_descr);
                        fflush(OUTPUT_FILE);
                }
        }

        PIN_FLIST_DESTROY_EX(&update_serv_ipflist, NULL);
        return;
}

/*******************************************************************
 * Function called when new job is avaialble to worker
 * Called prior MTA_WORKER_JOB policy opcode
 *******************************************************************/
PIN_EXPORT void
pin_mta_worker_job(
        pcm_context_t   *ctxp,
        pin_flist_t     *srch_res_flistp,
        pin_flist_t     **op_in_flistpp,
        pin_flist_t     *ti_flistp,
        pin_errbuf_t    *ebufp)
{

        if (PIN_ERR_IS_ERR(ebufp)) {
                return;
        }
        PIN_ERRBUF_CLEAR (ebufp);

        PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG, "pin_mta_worker_job search results flist",
                                           srch_res_flistp);

        PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG, "pin_mta_worker_job thread info flist",
                                           ti_flistp);


        *op_in_flistpp = PIN_FLIST_COPY (srch_res_flistp, ebufp);

        PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG, "pin_mta_worker_job prepared flist for main opcode",
                                           *op_in_flistpp);

        if (PIN_ERR_IS_ERR(ebufp)) {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                                                 "pin_mta_worker_job error", ebufp);
        }
        return;
}


/*******************************************************************
 * Function called when new job is avaialble to worker
 * Called after MTA_WORKER_JOB policy opcode
 *******************************************************************/
PIN_EXPORT void
pin_mta_post_worker_job(
        pcm_context_t   *ctxp,
        pin_flist_t     *srch_res_flistp,
        pin_flist_t     *op_in_flistp,
        pin_flist_t     *ti_flistp,
        pin_errbuf_t    *ebufp)
{
        if (PIN_ERR_IS_ERR(ebufp)) {
                return;
        }
        PIN_ERRBUF_CLEAR (ebufp);

        PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG, "pin_mta_post_worker_job search results flist",
                                           srch_res_flistp);

        PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG, "pin_mta_post_worker_job thread info flist",
                                           ti_flistp);

        PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG, "pin_mta_post_worker_job prepared flist for main opcode",
                                           op_in_flistp);

        if (PIN_ERR_IS_ERR(ebufp)) {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                                                 "pin_mta_post_worker_job error", ebufp);
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
	failed_count = (int32)(rec_count - success_count);

        fprintf(OUTPUT_FILE, "\n\n=========================================================\n");
        fprintf(OUTPUT_FILE, "\nDetailed Report is given below\n\n");
        fprintf(OUTPUT_FILE,"%s\n", record_count);
        sprintf(suc_count, "Successful Records Count: %d", success_count);
        fprintf(OUTPUT_FILE,"%s\n", suc_count);
	sprintf(fail_cnt, "Failed Records Count: %d", failed_count);
        fprintf(OUTPUT_FILE,"%s\n", fail_cnt);
        fprintf(OUTPUT_FILE, "\n=========================================================\n");
        fprintf(OUTPUT_FILE, "Payment Reject Upload end time: %s",fdate);
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

void parse(char *record, char arr[][50], int *fldcnt)
{
        char line1[1024];
        char line2[1024];
        char line3[1024];
        char *stptr;
        int flag = 0;
        int idx = 0;
        int lcount = 0;
        int count =0;

        strcpy(line2,record);
        stptr = line2;

        while (*stptr != '\0')
        {
                lcount++;
                idx = 0;
                while (*stptr != '\0' && *stptr != ',')
                {
                        line3[idx] = *stptr;
                        idx++;
                        stptr++;
                }
                line3[idx] = '\0';
                if (*stptr != '\0' && *stptr == ',')
                        stptr++;
                //strcpy(line2,stptr);
                //stptr = line2;

                strcpy(arr[lcount],line3);
        }
        *fldcnt=lcount+1;
}
/*******************************************************************************
 * trim()
 *
 * Inputs: String
 *
 * Output: void; ebuf set if errors encountered
 *
 * Description:
 * This function is used to trim leading and trailing spaces
 ******************************************************************************/
void trim(char *str)
{
    int index = 0;
        int i = 0;

    /*
     * Trim leading white spaces
     */
    while(str[index] == ' ' || str[index] == '\t' || str[index] == '\n' || str[index] == '\r')
    {
        index++;
    }

    /* Shift all trailing characters to its left */
    while(str[i + index] != '\0')
    {
        str[i] = str[i + index];
        i++;
    }
    str[i] = '\0'; // Terminate string with NULL


    /*
     * Trim trailing white spaces
     */
    i = 0;
    index = -1;
    while(str[i] != '\0')
    {
        if(str[i] != ' ' && str[i] != '\t' && str[i] != '\n' && str[i] != '\r')
        {
            index = i;
        }

        i++;
    }

    /* Mark the next character to last non white space character as NULL */
    str[index + 1] = '\0';
}

