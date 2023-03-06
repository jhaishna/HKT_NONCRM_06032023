/***************************************************************************
copyright 2012 Bell Aliant
*
* Created By: Sreekanth Yarraguntla - June 2012
*
* Description:
*
* USAGE: provisioning_service_reactivation [-i] <Input File> [-h or -help]
*
*****************************************************************************/
/*****************************************************************************
*******************************************************************************/

#ifndef lint
static  char    Sccs_id[] = "@(#)%Portal Version: pin_mta_test.c:CUPmod7.3PatchInt:1:2007-Feb-07 06:51:33 %";
#endif

#include <stdio.h>
#include "pcm.h"
#include "pin_errs.h"
#include "pinlog.h"
#include "pin_mta.h"
#include "pin_flds.h"
#include "rbs_ops_flds.h"
#include "rbs_ar_adjustment.h"

/*******************************************************************
 * During real application development, there is no necessity to implement
 * empty MTA call back functions. Application will recognize implemented
 * functions automatically during run-time.
 *******************************************************************/
#define MAXFLDS 200
#define MAXFLDSIZE 300
#define BUFFER_SIZE 1000

FILE    *INPUT_FILE = NULL;
FILE    *SUCC_OUTPUT_FILE = NULL;
FILE    *FAIL_OUTPUT_FILE = NULL;
FILE 	*BATCH_FILE = NULL;
int     rec_count = 0;
int     success_count = 0;
int	failed_count = 0;
char	h_batch_no[100];
char    *a_type = NULL;
char    *INPUT_FILENAME = NULL;

void parse(
        char    *record,
        char    arr[][300],
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
	char		num[5];
        char            *value = NULL;
        char            str[1024];
        char            *output_file = NULL;
	char		*batch_file = NULL;
        char            fdate[80];
	char		ftdate[80];
	char		batch_name[255]="";
	char            succ_output_fname[255]="";
        char            fail_output_fname[255]="";
        char            buffer[1024];
        time_t          pvt = (time_t)0;
        int             err=0;
	char		*input_file_dept = NULL;
	char		input_fname_dept[255] = "";
	char		*input_file_del_adj = NULL;
	char            input_fname_del[255] = "";
	char		msg[200];

        if (PIN_ERR_IS_ERR(ebufp)) 
	{
            return;
        }
        PIN_ERRBUF_CLEAR (ebufp);
	PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG, "pin_mta_config parameters flist", param_flistp);
        PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG, "pin_mta_config application info flist", app_flistp);
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
        while ((flistp = PIN_FLIST_ELEM_GET_NEXT(param_flistp, PIN_FLD_PARAMS, &rec_id, 1, &cookie, ebufp))!= NULL)
	{
            	option = PIN_FLIST_FLD_GET (flistp, PIN_FLD_PARAM_NAME, 0, ebufp);
	    	value = PIN_FLIST_FLD_GET (flistp, PIN_FLD_PARAM_VALUE, 1, ebufp);
        	/***************************************************
         	* Test options options.
         	***************************************************/
        	if (strcmp(option, "-i") == 0) 
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
		
				pvt = pin_virtual_time((time_t *)NULL);
                		strftime(fdate, sizeof(fdate), "_%Y%m%d", localtime(&pvt));
                		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, fdate);
			
				if (strstr(INPUT_FILENAME, "dept"))
				{
					pin_conf("rbs_adjbatch_upload", "adjbatch_upload_file_dept", PIN_FLDT_STR, &input_file_dept, &err);
				}
				else
				{
					pin_conf("rbs_adjbatch_upload", "adjbatch_upload_file_del_adj", PIN_FLDT_STR, &input_file_del_adj, &err);
					sprintf(input_fname_del, "%s", input_file_del_adj);
                			strcat(input_fname_del, fdate);
                			strcat(input_fname_del, ".csv");
				}
        	    	}
		} 
		else if (strcmp(option, "-t") == 0)
		{
			if (value == 0 || *value == 0)
                        {
                                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, "Missing type of adjustment for -t option");

                                fprintf(stderr, "%s\n", "Missing type of adjustment for -t option");

                                mta_flags = mta_flags | MTA_FLAG_USAGE_MSG;
                        }
			else
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, value);
				PIN_FLIST_FLD_COPY(flistp, PIN_FLD_PARAM_VALUE, app_flistp, PIN_FLD_TYPE_STR, ebufp);
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
	if (input_file_dept)
	{
		while (i <= 4)
		{
			i = i + 1;
			sprintf(num, "%d", i);
			PIN_ERR_LOG_MSG(3, num);
			sprintf(input_fname_dept, "%s", input_file_dept);
			strcat(input_fname_dept, num);
			strcat(input_fname_dept, fdate);
			strcat(input_fname_dept, ".csv");
			PIN_ERR_LOG_MSG(3, input_fname_dept);
			if (INPUT_FILENAME && (input_fname_dept && (strcmp(INPUT_FILENAME, input_fname_dept) == 0)))
			{
				PIN_ERR_LOG_MSG(3, input_fname_dept);
				break;
			}		
		}
		sprintf(succ_output_fname, "%s", input_fname_dept);
                strcat(succ_output_fname, ".success");
                sprintf(fail_output_fname, "%s", input_fname_dept);
                strcat(fail_output_fname, ".error");
		PIN_ERR_LOG_MSG(3, fail_output_fname);
                PIN_ERR_LOG_MSG(3, succ_output_fname);
        }
	else if (input_file_del_adj)
	{
		while (i <= 9)
                {
                        i = i + 1;
                        sprintf(num, "%d", i);
                        PIN_ERR_LOG_MSG(3, num);
                        sprintf(input_fname_del, "%s", input_file_del_adj);
                        strcat(input_fname_del, fdate);
                        strcat(input_fname_del, num);
                        strcat(input_fname_del, ".csv");
                        PIN_ERR_LOG_MSG(3, input_fname_del);
                        if (INPUT_FILENAME && (input_fname_dept && (strcmp(INPUT_FILENAME, input_fname_del) == 0)))
                        {
                                PIN_ERR_LOG_MSG(3, input_fname_del);
                                break;
                        }
                }
		sprintf(succ_output_fname, "%s", input_fname_del);
                strcat(succ_output_fname, ".success");
                sprintf(fail_output_fname, "%s", input_fname_del);
                strcat(fail_output_fname, ".error");
		PIN_ERR_LOG_MSG(3, fail_output_fname);
		PIN_ERR_LOG_MSG(3, succ_output_fname);
	}
	pin_conf("rbs_adjbatch_upload", "adjbatch_file", PIN_FLDT_STR, &batch_file, &err);
        sprintf(batch_name, "%s", batch_file);
	PIN_ERR_LOG_MSG(3, batch_name);
        BATCH_FILE = fopen(batch_name, "r+");
	SUCC_OUTPUT_FILE = fopen(succ_output_fname, "w");
        FAIL_OUTPUT_FILE = fopen(fail_output_fname, "w");

        sprintf(msg, "input_fname_del: %s, input_fname_dept: %s",input_fname_del, input_fname_dept);
        PIN_ERR_LOG_MSG(3, msg);
	if (INPUT_FILENAME && (input_fname_dept && (strcmp(INPUT_FILENAME, input_fname_dept) == 0)))
        {
                 INPUT_FILE = fopen(INPUT_FILENAME, "r+");
                 if (INPUT_FILE == NULL)
                 {
                        sprintf(str, "Could not open input file '%s'", INPUT_FILENAME);
                        fprintf(stderr, "%s", str);
                        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, str);
                        mta_flags = mta_flags | MTA_FLAG_USAGE_MSG;
                 }
		fprintf(SUCC_OUTPUT_FILE, "Record Type,Account Number,Mobile Number,DR/CR,Currency,Amount,Adjustment Code,Adjustment Reason,Remarks,To AR / To BILL,Future Effective Date\n");
                fprintf(FAIL_OUTPUT_FILE, "Record Type,Account Number,Mobile Number,DR/CR,Currency,Amount,Adjustment Code,Adjustment Reason,Remarks,To AR / To BILL,Future Effective Date\n");
        }
	else if (INPUT_FILENAME && input_fname_del && (strcmp(INPUT_FILENAME, input_fname_del) == 0))
	{
		INPUT_FILE = fopen(INPUT_FILENAME, "r+");
            	if (INPUT_FILE == NULL)
            	{
                	sprintf(str, "Could not open input file '%s'", INPUT_FILENAME);
                	fprintf(stderr, "%s", str);
                	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, str);
                	mta_flags = mta_flags | MTA_FLAG_USAGE_MSG;
            	}
		fprintf(SUCC_OUTPUT_FILE, "Record Type,Account Number,Mobile Number,DR/CR,Currency,Amount,Adjustment Code,Adjustment Reason,Remarks,To AR / To BILL,Future Effective Date\n");
                fprintf(FAIL_OUTPUT_FILE, "Record Type,Account Number,Mobile Number,DR/CR,Currency,Amount,Adjustment Code,Adjustment Reason,Remarks,To AR / To BILL,Future Effective Date\n");
	}
	else
	{
		freopen(NULL, "w", SUCC_OUTPUT_FILE);
                freopen(NULL, "w", FAIL_OUTPUT_FILE);
                fprintf(FAIL_OUTPUT_FILE, "Invalid File Format\n");
		mta_flags = mta_flags | MTA_FLAG_USAGE_MSG;
	}
	fflush(SUCC_OUTPUT_FILE);
        fflush(FAIL_OUTPUT_FILE);	
        if (PIN_ERR_IS_ERR(ebufp)) 
	{
            PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "pin_mta_config error", ebufp);
        }
        return;
}
/*******************************************************************
 * Post configuration of application
 * Called after MTA_CONFIG policy opcode
 *******************************************************************/
PIN_EXPORT void
pin_mta_post_config(
        pin_flist_t     *param_flistp,
        pin_flist_t     *app_flistp,
        pin_errbuf_t    *ebufp)
{
        if (PIN_ERR_IS_ERR(ebufp)) 
	{
           return;
        }
        PIN_ERRBUF_CLEAR (ebufp);

        PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG, "pin_mta_post_config parameters flist", param_flistp);
        PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG, "pin_mta_post_config application info flist", app_flistp);

        if (PIN_ERR_IS_ERR(ebufp)) 
	{
            PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "pin_mta_post_config error", ebufp);
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

        if (PIN_ERR_IS_ERR(&ebuf)) 
	{
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "pin_mta_usage error", &ebuf);
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
	int32		records = 0;
        int32           fldcnt=0;
	int32		colcnt = 0;
        int32           err=0;
        int32           last_record = 0;
        int64           db = -1;
        char            buffer[1024];
	char		buf[1024];
        char            arr[MAXFLDS][MAXFLDSIZE]={0x0};
	char		*record = NULL;
        char            msg[100];
        void            *vp = NULL;
        pin_flist_t     *result_flistp = NULL;
        pin_flist_t     *results_flistp = NULL;
        int             a_cntr = 0;
	int32		currency = 0;
	char		t_batch_no[50];
	char		*bbatch_no = NULL;
	int32		t_batch_no1 = 0;
	int32		h_batch_no1 = 0;
	int64		adj_code = 0;
	char		*f_eff_date = NULL;
	char		*rec_type = NULL;
	char		*batch_name = NULL;
	char		*curr_time = NULL;
	char		*time = NULL;
	char		*c_amt = NULL;
	char		*d_amt = NULL;
	char		*str_amt = NULL;
	char		*reason_code = NULL;
	char		*type_str = NULL;
	pin_decimal_t	*amount = pbo_decimal_from_str("0.0", ebufp);
	pin_decimal_t	*zerop = pbo_decimal_from_str("0.0", ebufp);
	pin_decimal_t	*tc_amt = pbo_decimal_from_str("0.0", ebufp);
	pin_decimal_t	*td_amt = pbo_decimal_from_str("0.0", ebufp);
	pin_decimal_t	*neg_amtp = pbo_decimal_from_str("-1.0", ebufp);
	pin_decimal_t	*cr_amtp = pbo_decimal_from_str("0.0", ebufp);
	pin_decimal_t	*dr_amtp = pbo_decimal_from_str("0.0", ebufp);
	struct          tm *current_time;
	time_t		now_t = 0;
	int32		header = 0;
	int32		trailer = 0;
	int64		current_date = 0;
	int             year;
    	int             month;
    	int             day;
	char            validity[21];

	pcm_context_t   *ctxp = pin_mta_main_thread_pcm_context_get(ebufp);
        if (PIN_ERR_IS_ERR(ebufp))
        {
            return;
        }
        PIN_ERRBUF_CLEAR (ebufp);

        results_flistp = PIN_FLIST_CREATE(ebufp);
        *s_flistpp = 0;

        PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG, "pin_mta_init_search application info flist", app_flistp);
        vp = PIN_FLIST_FLD_GET (app_flistp, PIN_FLD_POID_VAL, 0, ebufp);
	a_type = PIN_FLIST_FLD_GET (app_flistp, PIN_FLD_TYPE_STR, 0, ebufp);
        if(vp)
                db = PIN_POID_GET_DB ((poid_t*)vp);
		now_t = pin_virtual_time((time_t *)NULL);
                current_time = localtime(&now_t);
                year = current_time->tm_year + 1900;
                month = current_time->tm_mon +1;
                day = current_time->tm_mday;

                memset(validity,'\0',21);
                sprintf(validity,"%04d%02d%02d",year,month,day);		
		PIN_ERR_LOG_MSG(3, validity);
		current_date = (u_int64)atoi(validity);
		curr_time = (char *)validity;
		PIN_FLIST_FLD_SET(results_flistp, PIN_FLD_POID,	(void *)PIN_POID_CREATE(db, "/account", 1, ebufp), ebufp);
                while (fgets(buffer, sizeof buffer, INPUT_FILE) != NULL)
                {
                        parse(buffer, arr, &fldcnt);
                        rec_count = rec_count + 1;
                        rec_type = arr[1];
                        if (rec_type && strcmp(rec_type, "H") == 0)
                        {
				PIN_ERR_LOG_MSG(3, "HEADER");
				header = header + 1;
				rec_count = rec_count - 1;
				h_batch_no1 = (int32 )atoi(arr[2]);
				sprintf(h_batch_no, "%d", h_batch_no1);
				PIN_ERR_LOG_MSG(3, h_batch_no);
                        }
                        else if (rec_type && (strcmp(rec_type, "T") == 0))
                        {
				PIN_ERR_LOG_MSG(3, "TRAILER");
				trailer = trailer + 1;
				rec_count = rec_count - 1;
				t_batch_no1 = (int32 )atoi(arr[2]);
				trim(t_batch_no);
				record = (char *)arr[3];
				records = (int32)atoi(record);
				if (!record || strlen(record) > 6)
                                {
                                        PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Invalid Record", ebufp);
                                        freopen(NULL, "w", SUCC_OUTPUT_FILE);
                                        freopen(NULL, "w", FAIL_OUTPUT_FILE);
                                        fprintf(FAIL_OUTPUT_FILE, "Invalid Record Length\n");
                                        fflush(FAIL_OUTPUT_FILE);
                                        PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "pin_mta_init_search error, Invalid Record", ebufp);
                                        PIN_FLIST_DESTROY_EX (&results_flistp, ebufp);
                                        return;
                                }
				PIN_ERR_LOG_MSG(3, record);
				if (record && (*record >= 'A' && *record <= 'Z') || (*record >= 'a' && *record <= 'z'))
				{
					PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Total number of records specify in trailer record must be numeric", ebufp);
                                        freopen(NULL, "w", SUCC_OUTPUT_FILE);
                                        freopen(NULL, "w", FAIL_OUTPUT_FILE);
                                        fprintf(FAIL_OUTPUT_FILE, "Total number of records specify in trailer record must be numeric\n");
                                        fflush(FAIL_OUTPUT_FILE);
                                        PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "pin_mta_init_search error, Invalid Record", ebufp);
                                        PIN_FLIST_DESTROY_EX (&results_flistp, ebufp);
                                        return;
				}
				sprintf(msg, "%s, %d, %d", t_batch_no, records, trailer);
                                PIN_ERR_LOG_MSG(3, msg);
				c_amt = (char *)arr[4];
				cr_amtp = pbo_decimal_from_str(arr[4], ebufp);
				d_amt = (char *)arr[5];
				dr_amtp = pbo_decimal_from_str(arr[5], ebufp);
				if (!c_amt || strlen(c_amt) > 13)
				{
					PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Invalid Total_Credit_Adjustment_Amount", ebufp);
                                        freopen(NULL, "w", SUCC_OUTPUT_FILE);
                                        freopen(NULL, "w", FAIL_OUTPUT_FILE);
                                        fprintf(FAIL_OUTPUT_FILE, "Invalid Total_Credit_Adjustment_Amount Length\n");
                                        fflush(FAIL_OUTPUT_FILE);
                                        PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "pin_mta_init_search error, Invalid Total_Credit_Adjustment_Amount", ebufp);
                                        PIN_FLIST_DESTROY_EX (&results_flistp, ebufp);
                                        return;
				}
				else if (c_amt && (*c_amt >= 'A' && *c_amt <= 'Z') || (*c_amt >= 'a' && *c_amt <= 'z'))
                                {
                                        PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Invalid Total_Dedit_Adjustment_Amount", ebufp);
                                        freopen(NULL, "w", SUCC_OUTPUT_FILE);
                                        freopen(NULL, "w", FAIL_OUTPUT_FILE);
                                        fprintf(FAIL_OUTPUT_FILE, "Total credit adjustment amounts specify in trailer record must be numeric\n");
                                        fflush(FAIL_OUTPUT_FILE);
                                        PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "pin_mta_init_search error, Invalid Total_Dedit_Adjustment_Amount", ebufp);
                                        PIN_FLIST_DESTROY_EX (&results_flistp, ebufp);
                                        return;
                                }
				if (!d_amt || strlen(d_amt) > 13)
                                {
                                        PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Invalid Total_Dedit_Adjustment_Amount Length", ebufp);
                                        freopen(NULL, "w", SUCC_OUTPUT_FILE);
                                        freopen(NULL, "w", FAIL_OUTPUT_FILE);
                                        fprintf(FAIL_OUTPUT_FILE, "Invalid Total_Dedit_Adjustment_Amount Length\n");
                                        fflush(FAIL_OUTPUT_FILE);
                                        PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "pin_mta_init_search error, Invalid Total_Dedit_Adjustment_Amount", ebufp);
                                        PIN_FLIST_DESTROY_EX (&results_flistp, ebufp);
                                        return;
                                }
				else if (d_amt && (*d_amt >= 'A' && *d_amt <= 'Z') || (*d_amt >= 'a' && *d_amt <= 'z'))
				{
					PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Invalid Total_Dedit_Adjustment_Amount Length", ebufp);
                                        freopen(NULL, "w", SUCC_OUTPUT_FILE);
                                        freopen(NULL, "w", FAIL_OUTPUT_FILE);
                                        fprintf(FAIL_OUTPUT_FILE, "Total debit adjustment amounts specify in trailer record must be numeric\n");
                                        fflush(FAIL_OUTPUT_FILE);
                                        PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "pin_mta_init_search error, Invalid Total_Dedit_Adjustment_Amount", ebufp);
                                        PIN_FLIST_DESTROY_EX (&results_flistp, ebufp);
                                        return;
				}
				time = arr[6];
				trim(time);
				if (!time || strlen(time) > 8)
                                {
                                        PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Invalid Date Length", ebufp);
                                        freopen(NULL, "w", SUCC_OUTPUT_FILE);
                                        freopen(NULL, "w", FAIL_OUTPUT_FILE);
                                        fprintf(FAIL_OUTPUT_FILE, "Invalid Date Length\n");
                                        fflush(FAIL_OUTPUT_FILE);
                                        PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "pin_mta_init_search error, Invalid Date", ebufp);
                                        PIN_FLIST_DESTROY_EX (&results_flistp, ebufp);
                                        return;
                                }
				if (curr_time && time && !strcmp(curr_time, time) == 0)
				{
					PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Invalid date format in trailer record", ebufp);
                                        freopen(NULL, "w", SUCC_OUTPUT_FILE);
                                        freopen(NULL, "w", FAIL_OUTPUT_FILE);
                                        fprintf(FAIL_OUTPUT_FILE, "Invalid date format in trailer record\n");
                                        fflush(FAIL_OUTPUT_FILE);
                                        PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "pin_mta_init_search error, Invalid Date", ebufp);
                                        PIN_FLIST_DESTROY_EX (&results_flistp, ebufp);
                                        return;
				}
                        }
                        else if (rec_type && (strcmp(rec_type, "D") == 0))
                        {
				result_flistp = PIN_FLIST_ELEM_ADD(results_flistp, PIN_FLD_RESULTS, a_cntr++, ebufp);
                                PIN_FLIST_FLD_PUT(result_flistp, PIN_FLD_POID, PIN_POID_CREATE(db, "/account", 1, ebufp), ebufp);
                                PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_RECORD_TYPE, rec_type, ebufp);
                                PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_ACCOUNT_NO, arr[2], ebufp);

				if (strstr(a_type, "adjustment"))
				{
					adj_code = (int32)(atoi(arr[7]));
					reason_code = (char *)arr[7];
					if (reason_code && (*reason_code >= 'A' && *reason_code <= 'Z') || (*reason_code >= 'a' && *reason_code <= 'z'))
                                        {
                                                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Invalid adjustment code - must be numeric", ebufp);
                                                freopen(NULL, "w", SUCC_OUTPUT_FILE);
                                                freopen(NULL, "w", FAIL_OUTPUT_FILE);
                                                fprintf(FAIL_OUTPUT_FILE, "Invalid adjustment code - must be numeric\n");
                                                fflush(FAIL_OUTPUT_FILE);
                                                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "pin_mta_init_search error, Invalid Adjustment code", ebufp);
                                                PIN_FLIST_DESTROY_EX (&results_flistp, ebufp);
                                                return;
                                        }
                                	currency = (int32)(atoi(arr[5]));
					type_str = (char *)arr[4];
					amount = pbo_decimal_from_str(arr[6], ebufp);
					str_amt = (char *)arr[6];
					if (str_amt && (*str_amt >= 'A' && *str_amt <= 'Z') || (*str_amt >= 'a' && *str_amt <= 'z'))
					{
						PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Invalid adjustment amount - must be numeric", ebufp);
                                        	freopen(NULL, "w", SUCC_OUTPUT_FILE);
                                        	freopen(NULL, "w", FAIL_OUTPUT_FILE);
                                        	fprintf(FAIL_OUTPUT_FILE, "Invalid adjustment amount - must be numeric\n");
                                        	fflush(FAIL_OUTPUT_FILE);
                                        	PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "pin_mta_init_search error, Invalid Amount", ebufp);
                                        	PIN_FLIST_DESTROY_EX (&results_flistp, ebufp);
                                        	return;
					}
					f_eff_date = arr[11];
					trim(f_eff_date);
					PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_PROGRAM_NAME, "Adjustment", ebufp);
					PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_PHONE, arr[3], ebufp);
					PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_CURRENCY, &currency, ebufp);
					PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_AMOUNT, amount, ebufp);
                                	PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_REASON_ID, &adj_code, ebufp);
                                	PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_TYPE_STR, arr[4], ebufp);
                         	        PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_REASON_CODE, arr[8], ebufp);
					PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_DESCR, arr[9], ebufp);
					PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_ACTION, arr[10], ebufp);
					PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_START_CREATION_DATE, f_eff_date, ebufp);
                        	}
				else
				{
					type_str = (char *)arr[5];
					adj_code = (int32 )atoi(arr[4]);
					currency = (int32 )atoi(arr[7]);
					amount = pbo_decimal_from_str(arr[8], ebufp);
					PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_PROGRAM_NAME, "Delete", ebufp);		
					PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_START_CREATION_DATE, arr[3], ebufp);
					PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_REASON_ID, &adj_code, ebufp);
					PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_TYPE_STR, arr[5], ebufp);
					PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_ACTION, arr[6], ebufp);
					PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_CURRENCY, &currency, ebufp);
					PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_AMOUNT, amount, ebufp);
				}
				if (amount && type_str && strcmp(type_str, "D") == 0)
                                {
                                        pbo_decimal_add_assign(td_amt, amount, ebufp);
					PIN_ERR_LOG_MSG(3, pbo_decimal_to_str(td_amt, ebufp));
                                }
                                else if (amount && type_str && strcmp(type_str, "C") == 0)
                                {
                                        pbo_decimal_add_assign(tc_amt, amount, ebufp);
					PIN_ERR_LOG_MSG(3, pbo_decimal_to_str(tc_amt, ebufp));
                                }
				pbo_decimal_multiply(amount, zerop, ebufp);
			}
			else
			{
				PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Invalid Record_Type Length", ebufp);
                		freopen(NULL, "w", SUCC_OUTPUT_FILE);
		                freopen(NULL, "w", FAIL_OUTPUT_FILE);
                		fprintf(FAIL_OUTPUT_FILE, "Invalid Record_Type Length\n");
		                fflush(FAIL_OUTPUT_FILE);
                		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "pin_mta_init_search error, Invalid Record_Type", ebufp);
		                PIN_FLIST_DESTROY_EX (&results_flistp, ebufp);			
				return;
			}
                }
       	fclose(INPUT_FILE);
	sprintf(h_batch_no, "%d", h_batch_no1);
	sprintf(t_batch_no, "%d", t_batch_no1);
	sprintf(msg, "h_batch_no1: %d, t_batch_no1: %d, h_batch_no: %s, t_batch_no: %s", h_batch_no1, t_batch_no1, h_batch_no, t_batch_no);
        PIN_ERR_LOG_MSG(3, msg);
	while (fgets(buf, sizeof buf, BATCH_FILE) != NULL)
        {
                parse(buf, arr, &colcnt);
                bbatch_no = arr[1];
                batch_name = (char *)arr[2];
		trim(h_batch_no);
		trim(bbatch_no);
		trim(batch_name);
		trim(a_type);
                sprintf(msg, "%s, %s, %s, %s", h_batch_no, bbatch_no, a_type, batch_name);
                PIN_ERR_LOG_MSG(3, msg);
	        if (bbatch_no && h_batch_no && a_type && batch_name && ((strcmp(bbatch_no, h_batch_no) == 0) && (strcmp(a_type, batch_name) == 0)))
        	{
			PIN_ERR_LOG_MSG(3, "Batch_no Matching");
                	PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "The file had uploaded, can't process the file again.", ebufp);
                	freopen(NULL, "w", SUCC_OUTPUT_FILE);
                	freopen(NULL, "w", FAIL_OUTPUT_FILE);
                	fprintf(FAIL_OUTPUT_FILE, "The file had uploaded, can't process the file again.\n");
                	fflush(FAIL_OUTPUT_FILE);
                	PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "pin_mta_init_search error, Invalid Batch_no", ebufp);
                	PIN_FLIST_DESTROY_EX (&results_flistp, ebufp);
			return;
		}
	}
       	if (PIN_ERR_IS_ERR(ebufp))
       	{
	        PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "pin_mta_init_search error", ebufp);
                PIN_FLIST_DESTROY_EX (&results_flistp, ebufp);
        }
	sprintf(msg, "header: %d, trailer: %d", header, trailer);
	PIN_ERR_LOG_MSG(3, msg);
	PIN_ERR_LOG_MSG(3, pbo_decimal_to_str(cr_amtp, ebufp));
	PIN_ERR_LOG_MSG(3, pbo_decimal_to_str(tc_amt, ebufp));
	if (header == 0 && trailer == 0)
        {
        	PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Header and Trailer is missing", ebufp);
                freopen(NULL, "w", SUCC_OUTPUT_FILE);
                freopen(NULL, "w", FAIL_OUTPUT_FILE);
                fprintf(FAIL_OUTPUT_FILE, "Header and Trailer is missing\n");
                fflush(FAIL_OUTPUT_FILE);
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "pin_mta_init_search error, Invalid Record_Type", ebufp);
                PIN_FLIST_DESTROY_EX (&results_flistp, ebufp);
        }
        else if (header > 1)
        {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Multiple headers Not allowed", ebufp);
                freopen(NULL, "w", SUCC_OUTPUT_FILE);
                freopen(NULL, "w", FAIL_OUTPUT_FILE);
                fprintf(FAIL_OUTPUT_FILE, "Multiple headers Not allowed\n");
                fflush(FAIL_OUTPUT_FILE);
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "pin_mta_init_search error, Invalid data", ebufp);
                PIN_FLIST_DESTROY_EX (&results_flistp, ebufp);
        }
	else if (trailer > 1)
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Multiple trailers Not allowed", ebufp);
                freopen(NULL, "w", SUCC_OUTPUT_FILE);
                freopen(NULL, "w", FAIL_OUTPUT_FILE);
                fprintf(FAIL_OUTPUT_FILE, "Multiple trailers Not allowed\n");
                fflush(FAIL_OUTPUT_FILE);
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "pin_mta_init_search error, Invalid data", ebufp);
                PIN_FLIST_DESTROY_EX (&results_flistp, ebufp);
	}
        else if ((strlen(t_batch_no) > 8) || (strlen(h_batch_no) > 8))
        {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Invalid Batch_No Length", ebufp);
                freopen(NULL, "w", SUCC_OUTPUT_FILE);
                freopen(NULL, "w", FAIL_OUTPUT_FILE);
                fprintf(FAIL_OUTPUT_FILE, "Invalid Batch_No Length\n");
                fflush(FAIL_OUTPUT_FILE);
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "pin_mta_init_search error, Invalid Batch_No", ebufp);
                PIN_FLIST_DESTROY_EX (&results_flistp, ebufp);
        }
	else if (header != 0 && h_batch_no1 != current_date)
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Mismatch time", ebufp);
                freopen(NULL, "w", SUCC_OUTPUT_FILE);
                freopen(NULL, "w", FAIL_OUTPUT_FILE);
                fprintf(FAIL_OUTPUT_FILE, "Batch number specify in header record does not match with the date in input filename\n");
                fflush(FAIL_OUTPUT_FILE);
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "pin_mta_init_search error, Invalid data", ebufp);
                PIN_FLIST_DESTROY_EX (&results_flistp, ebufp);
	}
	else if (header != 0 && trailer != 0 && h_batch_no1 != t_batch_no1)
        {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Mismatch time", ebufp);
                freopen(NULL, "w", SUCC_OUTPUT_FILE);
                freopen(NULL, "w", FAIL_OUTPUT_FILE);
                fprintf(FAIL_OUTPUT_FILE, "Batch number in header record does not match with batch number in trailer record\n");
                fflush(FAIL_OUTPUT_FILE);
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "pin_mta_init_search error, Invalid data", ebufp);
                PIN_FLIST_DESTROY_EX (&results_flistp, ebufp);
        }
	else if (trailer != 0 && rec_count != records)
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Mismatch Records", ebufp);
                freopen(NULL, "w", SUCC_OUTPUT_FILE);
		freopen(NULL, "w", FAIL_OUTPUT_FILE);
                fprintf(FAIL_OUTPUT_FILE, "Total number of records specify in trailer record does not match with actual number of Detail records\n");
                fflush(FAIL_OUTPUT_FILE);
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "pin_mta_init_search error, Invalid data", ebufp);
                PIN_FLIST_DESTROY_EX (&results_flistp, ebufp);
	}
	else if ((pbo_decimal_compare(cr_amtp, tc_amt, ebufp)!= 0))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Total amount mismatch in trailer", ebufp);
                freopen(NULL, "w", SUCC_OUTPUT_FILE);
		freopen(NULL, "w", FAIL_OUTPUT_FILE);
                fprintf(FAIL_OUTPUT_FILE, "Total credit adjustment amounts specify in trailer record does not match with actual total credit adjustment amounts\n");
                fflush(FAIL_OUTPUT_FILE);
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "pin_mta_init_search error, Invalid data", ebufp);
                PIN_FLIST_DESTROY_EX (&results_flistp, ebufp);
	}
	else if ((pbo_decimal_compare(dr_amtp, td_amt, ebufp) != 0))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Total amount mismatch in trailer", ebufp);
                freopen(NULL, "w", SUCC_OUTPUT_FILE);
                freopen(NULL, "w", FAIL_OUTPUT_FILE);
                fprintf(FAIL_OUTPUT_FILE, "Total debit adjustment amounts specify in trailer record does not match with actual total debit adjustment amounts\n");
                fflush(FAIL_OUTPUT_FILE);
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "pin_mta_init_search error, Invalid data", ebufp);
                PIN_FLIST_DESTROY_EX (&results_flistp, ebufp);
	}
	else
	{
		*s_flistpp = PIN_FLIST_COPY(results_flistp, ebufp);
                PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG, "pin_mta_init_search search flist", results_flistp);
	}
	return;
}

/*******************************************************************
 * Function executed after seach results were processed
 * Called prior MTA_JOB_DONE policy opcode
 *******************************************************************/
PIN_EXPORT void
pin_mta_job_done(
       pin_flist_t             *app_flistp,
       pin_errbuf_t    	       *ebufp)
{
        if (PIN_ERR_IS_ERR(ebufp)) 
	{
            return;
        }
        PIN_ERRBUF_CLEAR (ebufp);

        PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG, "pin_mta_job_done application info flist", app_flistp);

        if (PIN_ERR_IS_ERR(ebufp)) 
	{
            PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "pin_mta_job_done error", ebufp);
        }
        return;
}

/*******************************************************************
 * Function executed at application exit
 * Called prior MTA_EXIT policy opcode
 *******************************************************************/
PIN_EXPORT void
pin_mta_exit(
        pin_flist_t             *app_flistp,
        pin_errbuf_t    	*ebufp)
{
        char    		record_count[255];
        time_t          	pvt = (time_t)0;
        char    		fdate[80];
        if (PIN_ERR_IS_ERR(ebufp)) 
	{
           return;
        }
        PIN_ERRBUF_CLEAR (ebufp);

	PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG, "pin_mta_exit application info flist", app_flistp);
        sprintf(record_count, "Total Records Processed: %d", rec_count);
         pvt = pin_virtual_time((time_t *)NULL);
        strftime(fdate, sizeof(fdate), "%d-%m-%Y.%H:%M", localtime(&pvt));

	fprintf(FAIL_OUTPUT_FILE,"Total no.of records Processed : %d", failed_count);
        fflush(FAIL_OUTPUT_FILE);
        fclose(FAIL_OUTPUT_FILE);
        fprintf(SUCC_OUTPUT_FILE,"Total no.of records Processed: %d", success_count);
        fflush(SUCC_OUTPUT_FILE);
        fclose(SUCC_OUTPUT_FILE);
	fprintf(BATCH_FILE, "%s,%s\n", h_batch_no, a_type);
        fflush(BATCH_FILE);
	fclose(BATCH_FILE);
	rec_count = 0;
        success_count = 0;
        if (PIN_ERR_IS_ERR(ebufp)) 
	{
            PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "pin_mta_exit error", ebufp);
        }
        return;
}	

/*******************************************************************
 * Main application opcode is called here
 * Here our main logic will be written to update the services
 *******************************************************************/
PIN_EXPORT void
pin_mta_worker_opcode(
        pcm_context_t   	*ctxp,
        pin_flist_t     	*srch_res_flistp,
        pin_flist_t     	*op_in_flistp,
        pin_flist_t     	**op_out_flistpp,
        pin_flist_t     	*ti_flistp,
        pin_errbuf_t    	*ebufp)
{
	pin_flist_t		*op_out_flistp = NULL;
	pin_flist_t		*rslts = NULL;
	pin_decimal_t           *amount = pbo_decimal_from_str("0.0", ebufp);
	int32                   *currency = NULL;
	int32                   *adj_code = NULL;
	char			*prog_name = NULL;
	char			*acc_id_str = NULL;
	char            	buffer[1024];
	char			msg[256];
	char			*rec_type = NULL;
	char			*acc_no = NULL;
	char			*mob_no = NULL;
	char			*adj_type = NULL;
	char			*adj_reason = NULL;
	char			*remarks = NULL;
	char			*action = NULL;
	char			*descr = NULL;
	char			*posted_t = NULL;	
	int32			*status =  NULL;

	if (PIN_ERR_IS_ERR(ebufp)) 
	{
            return;
        }
        PIN_ERRBUF_CLEAR (ebufp);

        PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG, "pin_mta_worker_opcode search results flist", srch_res_flistp);

        PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG, "pin_mta_worker_opcode prepared flist for main opcode", op_in_flistp);

        PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG, "pin_mta_worker_opcode thread info flist", ti_flistp);	

	prog_name = PIN_FLIST_FLD_GET(op_in_flistp, PIN_FLD_PROGRAM_NAME, 0, ebufp);
	rec_type = PIN_FLIST_FLD_GET(op_in_flistp, PIN_FLD_RECORD_TYPE, 0, ebufp);
	acc_no = PIN_FLIST_FLD_GET(op_in_flistp, PIN_FLD_ACCOUNT_NO, 0, ebufp);
	if (PIN_FLIST_FLD_GET(op_in_flistp, PIN_FLD_PHONE, 1, ebufp))
		mob_no = PIN_FLIST_FLD_GET(op_in_flistp, PIN_FLD_PHONE, 0, ebufp);
	currency = PIN_FLIST_FLD_GET(op_in_flistp, PIN_FLD_CURRENCY, 0, ebufp);
	amount = PIN_FLIST_FLD_GET(op_in_flistp, PIN_FLD_AMOUNT, 0, ebufp);
	adj_code = PIN_FLIST_FLD_GET(op_in_flistp, PIN_FLD_REASON_ID, 0, ebufp);
	adj_type = PIN_FLIST_FLD_GET(op_in_flistp, PIN_FLD_TYPE_STR, 0, ebufp);
	action = PIN_FLIST_FLD_GET(op_in_flistp, PIN_FLD_ACTION, 0, ebufp);
	if (PIN_FLIST_FLD_GET(op_in_flistp, PIN_FLD_REASON_CODE, 1, ebufp) != NULL)
		adj_reason = PIN_FLIST_FLD_GET(op_in_flistp, PIN_FLD_REASON_CODE, 1, ebufp);
	if (PIN_FLIST_FLD_GET(op_in_flistp, PIN_FLD_DESCR, 1, ebufp) != NULL)
		remarks = PIN_FLIST_FLD_GET(op_in_flistp, PIN_FLD_DESCR, 1, ebufp);
	if (PIN_FLIST_FLD_GET(op_in_flistp, PIN_FLD_START_CREATION_DATE, 1, ebufp) != NULL)
		posted_t = PIN_FLIST_FLD_GET(op_in_flistp, PIN_FLD_START_CREATION_DATE, 0, ebufp);
	if (prog_name && strstr(prog_name, "Adjustment"))
	{
		sprintf(msg, "%s,%s,%s,%d,%s,%d,%s,%s,%s,%s,%s", rec_type, acc_no, mob_no, *currency, pbo_decimal_to_str(amount, ebufp), *adj_code, adj_type, adj_reason, remarks, action, posted_t);
	}
	else
	{
		sprintf(msg, "%s,%s,%s,%d,%s,%s,%d,%s", rec_type, acc_no, posted_t, *adj_code, adj_type, action, *currency, pbo_decimal_to_str(amount, ebufp));	
	}
	PIN_ERR_LOG_MSG(3, msg);
	PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG, "pin_mta_worker_opcode prepared flist for main opcode", op_in_flistp);
	PCM_OP(ctxp, RBS_OP_ADJ_LOADER, 0, op_in_flistp, &op_out_flistp, ebufp);
	
	PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG, "pin_mta_worker_opcode prepared flist for main opcode output", op_out_flistp);
	
       	if (PIN_ERR_IS_ERR(ebufp)) 
	{

       		/***************************************************
       		* Log something and return nothing.
       		***************************************************/
       		strcat(buffer, ",Error occured while calling adj_batch upload");
		fprintf(FAIL_OUTPUT_FILE, "%s\n", buffer);
		fprintf(FAIL_OUTPUT_FILE, "%s\n", msg);
       		fflush(FAIL_OUTPUT_FILE);
       		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in adj_batch upload", ebufp);
		failed_count = failed_count + 1;
       	}
	else	
	{
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "adj_batch upload output", op_out_flistp);	
		status = PIN_FLIST_FLD_GET(op_out_flistp, PIN_FLD_STATUS, 0, ebufp);
		if (*status == 0)
		{
                        descr = PIN_FLIST_FLD_GET(op_out_flistp, PIN_FLD_DESCR, 0, ebufp);
                        success_count = success_count + 1;
                        fprintf(SUCC_OUTPUT_FILE, "%s,%s\n", msg, descr);
                        fflush(SUCC_OUTPUT_FILE);
                }
		else
		{
			descr = PIN_FLIST_FLD_GET(op_out_flistp, PIN_FLD_ERROR_DESCR, 0, ebufp);
			failed_count = failed_count + 1;
       	                fprintf(FAIL_OUTPUT_FILE, "%s,%s\n", msg, descr);
       	                fflush(FAIL_OUTPUT_FILE);
		}
	}
        return;
}
/*******************************************************************
 * Function called when worker completed assigned job
 * Called prior MTA_WORKER_JOB_DONE policy opcode
 *******************************************************************/
PIN_EXPORT void
pin_mta_worker_job_done(
        pcm_context_t   	*ctxp,
        pin_flist_t             *srch_res_flistp,
        pin_flist_t             *op_in_flistp,
        pin_flist_t             *op_out_flistp,
        pin_flist_t             *ti_flistp,
        pin_errbuf_t    	*ebufp)
{
        if (PIN_ERR_IS_ERR(ebufp)) 
	{
            return;
        }
        PIN_ERRBUF_CLEAR (ebufp);

        PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG, "pin_mta_worker_job_done search results flist", srch_res_flistp);

        PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG, "pin_mta_worker_job_done prepared flist for main opcode", op_in_flistp);

        PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG, "pin_mta_worker_job_done output flist from main opcode", op_out_flistp);

        PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG, "pin_mta_worker_job_done thread info flist", ti_flistp);

        if (PIN_ERR_IS_ERR(ebufp)) 
	{
            PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "pin_mta_worker_job_done error", ebufp);
        }
        return;
}	
void parse(char *record, char arr[][300], int *fldcnt)
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
//                strcpy(line2,stptr);
//                stptr = line2;
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

    /* Trim leading white spaces */
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

    /* Trim trailing white spaces */
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
