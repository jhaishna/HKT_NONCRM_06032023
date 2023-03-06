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
#include "rbs_adjcode_upload.h"

/*******************************************************************
 * During real application development, there is no necessity to implement
 * empty MTA call back functions. Application will recognize implemented
 * functions automatically during run-time.
 *******************************************************************/
#define MAXFLDS 200
#define MAXFLDSIZE 500

FILE    *INPUT_FILE = NULL;
FILE    *SUCC_OUTPUT_FILE = NULL;
FILE    *FAIL_OUTPUT_FILE = NULL;
int     rec_count = 0;
int     success_count = 0;
int     failed_count = 0;

void parse(
        char    *record,
        char    arr[][500],
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
        char            *INPUT_FILENAME = NULL;
        char            str[1024];
        char            *output_file = NULL;
        char            fdate[80];
        char            fmdate[80];
        char            ftdate[80];
        char            succ_output_fname[255]="";
        char            fail_output_fname[255]="";
        char            buffer[1024];
        time_t          pvt = (time_t)0;
        int             err=0;
        char            *input_file = NULL;
        char            input_fname[255] = "";


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
        while((flistp = PIN_FLIST_ELEM_GET_NEXT(param_flistp, PIN_FLD_PARAMS, &rec_id, 1, &cookie, ebufp))!= NULL)
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

                		pin_conf("rbs_adjcode_upload", "adjcode_upload_input_file", PIN_FLDT_STR, &input_file, &err);
                		pvt = pin_virtual_time((time_t *)NULL);
                		strftime(fdate, sizeof(fdate), "%Y%m%d", localtime(&pvt));
                		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, fdate);
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
	while (i <= 8)
	{
		i = i + 1;
		sprintf(input_fname, "%s", input_file);
		strcat(input_fname, fdate);
		sprintf(num, "%d", i);
		strcat(input_fname, num);
		strcat(input_fname, ".csv");
		if (strcmp(INPUT_FILENAME, input_fname) == 0)
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, input_fname);
			break;
		}
	}
        PIN_ERR_LOG_MSG(3, input_fname);
        if (INPUT_FILENAME && input_fname && (strcmp(INPUT_FILENAME, input_fname) == 0))
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
        }

        PIN_FLIST_FLD_SET (app_flistp, PIN_FLD_FLAGS, &mta_flags, ebufp);
        pin_conf("rbs_adjcode_upload", "adjcode_upload_output_file", PIN_FLDT_STR, &output_file, &err);
        pvt = pin_virtual_time((time_t *)NULL);
        strftime(fdate, sizeof(fdate), "%Y%m%d", localtime(&pvt));
        strftime(fmdate, sizeof(fdate), "%Y-%m-%d-%H:%M", localtime(&pvt));
        strftime(ftdate, sizeof(fdate), "%d-%m-%y", localtime(&pvt));
        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, fdate);

        sprintf(succ_output_fname, "%s", output_file);
        strcat(succ_output_fname, fdate);
        strcat(succ_output_fname, ".csv.success");
        sprintf(fail_output_fname, "%s", output_file);
        strcat(fail_output_fname, fdate);
        strcat(fail_output_fname, ".csv.error");

        SUCC_OUTPUT_FILE = fopen(succ_output_fname, "w");
        FAIL_OUTPUT_FILE = fopen(fail_output_fname, "w");
        fprintf(SUCC_OUTPUT_FILE, "%s,%s,%s", succ_output_fname, fmdate, ftdate);
        fprintf(FAIL_OUTPUT_FILE, "%s,%s,%s", fail_output_fname, fmdate, ftdate);
	if (!strcmp(INPUT_FILENAME, input_fname) == 0)
	{
		freopen(NULL, "w", SUCC_OUTPUT_FILE);
		freopen(NULL, "w", FAIL_OUTPUT_FILE);
		fprintf(FAIL_OUTPUT_FILE, "Invalid File Format");
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
        char            *prog)
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
        int32           line_counter = 0;
        int32           fldcnt=0;
        int32           err=0;
        int32           glid = 0;
	int32		header = 0;
	int32		trailer = 0;
        int32           records = 0;
        int32           last_record = 0;
        int32           str_id = 0;
        int64           db = -1;
        char            buffer[1024];
        char            arr[MAXFLDS][MAXFLDSIZE]={0x0};
        char            a_pdp_str[32];
        char            *input_file = NULL;
        char            *output_file = NULL;
        char            fdate[80];
        char            output_fname[255]="";
        char            *end = NULL;
        char            *glidstr = NULL;
        char            msg[100];
        char            *action = NULL;
        char            *eng_descr = NULL;
        char            *ch_descr = NULL;
        void            *vp = NULL;
        poid_t          *a_pdp = NULL;
        poid_t          *b_pdp = NULL;
        pin_flist_t     *result_flistp = NULL;
        pin_flist_t     *results_flistp = NULL;
        pin_flist_t     *actfind_flistp = NULL;
        pin_flist_t     *ractfind_flistp = NULL;
        int              a_cntr = 0;
        time_t          pvt = (time_t)0;

        pcm_context_t   *ctxp = pin_mta_main_thread_pcm_context_get(ebufp);
        if (PIN_ERR_IS_ERR(ebufp))
        {
        	return;
        }
        PIN_ERRBUF_CLEAR (ebufp);

        *s_flistpp = 0;

        PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG, "pin_mta_init_search application info flist", app_flistp);
        vp = PIN_FLIST_FLD_GET (app_flistp, PIN_FLD_POID_VAL, 0, ebufp);

        if(vp)
	{
                db = PIN_POID_GET_DB ((poid_t*)vp);
		results_flistp = PIN_FLIST_CREATE(ebufp);
                while (fgets(buffer, sizeof buffer, INPUT_FILE) != NULL)
                {
                        parse(buffer, arr, &fldcnt);
                        rec_count = rec_count + 1;

                        end = arr[1];
                        trim(end);
                        if (rec_count == 1)
                        {
                                records = (int32)(atoi(arr[1]));
                                last_record = records + 2;
                        }
                        else if (end && (strcmp(end, "END") == 0))
                        {
                                sprintf(msg, "end: %s", end);
                                PIN_ERR_LOG_MSG(3, msg);
				trailer = trailer + 1;
                                //break;
                        }
                        else if (*end >= 'A' && *end <= 'Z')
                        {
                                action = arr[1];
                                str_id = (int32)(atoi(arr[2]));
                                eng_descr = arr[3];
                                ch_descr = arr[4];
                                glidstr = (arr[5]);
                                trim(glidstr);
                                glid = (int32)(atoi(glidstr));

                                sprintf(msg, "action: %s, str_id: %d, eng_descr: %s, ch_descr: %s, glid: %d", action, str_id, eng_descr, ch_descr, glid);
                                PIN_ERR_LOG_MSG(3, msg);

                                PIN_FLIST_FLD_PUT(results_flistp, PIN_FLD_POID, vp, ebufp);
                                result_flistp = PIN_FLIST_ELEM_ADD(results_flistp, PIN_FLD_RESULTS, a_cntr++, ebufp);
                                PIN_FLIST_FLD_PUT(result_flistp, PIN_FLD_POID, vp, ebufp);
                                PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_ACTION_TYPE, arr[1], ebufp);
                                PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_STRING_ID, &str_id, ebufp);
                                PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_STRING, arr[3], ebufp);
                                PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_REASON_CODE, arr[4], ebufp);
                                PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_GL_ID, &glid, ebufp);
                        }
			else if (!end || (strlen(end) < 1))
			{
				freopen(NULL, "w", SUCC_OUTPUT_FILE);
                                freopen(NULL, "w", FAIL_OUTPUT_FILE);
                                fprintf(FAIL_OUTPUT_FILE, "Record_type is missing");
                                fflush(FAIL_OUTPUT_FILE);
                                fflush(SUCC_OUTPUT_FILE);
                                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "pin_mta_init_search error", ebufp);
                                PIN_FLIST_DESTROY_EX (&result_flistp, NULL);
                                return; 
			}
			else if ((strcmp(end, "end") == 0) || (strcmp(end, "End") == 0))
        		{
		                freopen(NULL, "w", SUCC_OUTPUT_FILE);
                		freopen(NULL, "w", FAIL_OUTPUT_FILE);
		                fprintf(FAIL_OUTPUT_FILE, "Invalid Trailer. Case Sensitive");
                		fflush(FAIL_OUTPUT_FILE);
		                fflush(SUCC_OUTPUT_FILE);
		                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "pin_mta_init_search error", ebufp);
                		PIN_FLIST_DESTROY_EX (&result_flistp, NULL);
				return;
		        }
			else 
			{
				freopen(NULL, "w", SUCC_OUTPUT_FILE);
                                freopen(NULL, "w", FAIL_OUTPUT_FILE);
                                fprintf(FAIL_OUTPUT_FILE, "Multiple Headers not allowed");
                                fflush(FAIL_OUTPUT_FILE);
                                fflush(SUCC_OUTPUT_FILE);
                                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "pin_mta_init_search error", ebufp);
                                PIN_FLIST_DESTROY_EX (&result_flistp, NULL);
                                return;
			}
                }
	}         
       	rec_count = rec_count - 2;
        fclose(INPUT_FILE);
        if (PIN_ERR_IS_ERR(ebufp))
        {
       	 	PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "pin_mta_init_search error", ebufp);
                PIN_FLIST_DESTROY_EX (&results_flistp, NULL);
        }
	else if (trailer > 1)
        {
                freopen(NULL, "w", SUCC_OUTPUT_FILE);
                freopen(NULL, "w", FAIL_OUTPUT_FILE);
                fprintf(FAIL_OUTPUT_FILE, "Multiple Trailers not allowed");
                fflush(FAIL_OUTPUT_FILE);
                fflush(SUCC_OUTPUT_FILE);
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "pin_mta_init_search error Trailer", ebufp);
                PIN_FLIST_DESTROY_EX (&result_flistp, NULL);
        }
	else if ((rec_count != records) && (result_flistp != NULL))
	{
		freopen(NULL, "w", SUCC_OUTPUT_FILE);
                freopen(NULL, "w", FAIL_OUTPUT_FILE);
                fprintf(FAIL_OUTPUT_FILE, "Total number of records specify in header record does not match with actual number of Detail records");
                fflush(FAIL_OUTPUT_FILE);
                fflush(SUCC_OUTPUT_FILE);
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "pin_mta_init_search error", ebufp);
		PIN_FLIST_DESTROY_EX (&result_flistp, NULL);
	}
        else
        {
                *s_flistpp = PIN_FLIST_COPY(results_flistp, ebufp);
                PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG, "pin_mta_init_search search flist", results_flistp);
        }
        return;
}

/*******************************************************************
 * Main application opcode is called here
 * Here our main logic will be written to update the services
 *******************************************************************/
PIN_EXPORT void
pin_mta_worker_opcode(
        pcm_context_t           *ctxp,
        pin_flist_t             *srch_res_flistp,
        pin_flist_t             *op_in_flistp,
        pin_flist_t             **op_out_flistpp,
        pin_flist_t             *ti_flistp,
        pin_errbuf_t            *ebufp)
{
        pin_flist_t             *rslts = NULL;
        pin_flist_t             *adj_iflistp = NULL;
	pin_flist_t		*in_flistp = NULL;
        char                    *acc_id_str = NULL;
        char                    buffer[1024];
        char                    msg[100];
        char                    *act_type = NULL;
        char                    *str = NULL;
        char                    *ch_str = NULL;
        char                    *err_desc = NULL;
        char                    *eng_descr = NULL;
        char                    *chn_descr = NULL;
        char                   	*string_id = NULL;
        char                    *action = NULL;
	int32			*status = NULL;
        int32                  	*glid = NULL;
        int32                   *gl_id = NULL;
        int32                   *str_id = NULL;

        if (PIN_ERR_IS_ERR(ebufp))
        {
            return;
        }
        PIN_ERRBUF_CLEAR (ebufp);

        PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG, "pin_mta_worker_opcode search results flist", srch_res_flistp);

        PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG, "pin_mta_worker_opcode prepared flist for main opcode", op_in_flistp);

        PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG, "pin_mta_worker_opcode thread info flist", ti_flistp);

        act_type = PIN_FLIST_FLD_GET(op_in_flistp, PIN_FLD_ACTION_TYPE, 0, ebufp);
        str_id = PIN_FLIST_FLD_GET(op_in_flistp, PIN_FLD_STRING_ID,0, ebufp);
        str = PIN_FLIST_FLD_GET(op_in_flistp, PIN_FLD_STRING, 0, ebufp);
        ch_str = PIN_FLIST_FLD_GET(op_in_flistp, PIN_FLD_REASON_CODE, 0, ebufp);
        gl_id = PIN_FLIST_FLD_GET(op_in_flistp, PIN_FLD_GL_ID, 0, ebufp);
	
	in_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_COPY(op_in_flistp, PIN_FLD_POID, in_flistp, PIN_FLD_POID, ebufp);
	PIN_FLIST_FLD_SET(in_flistp, PIN_FLD_ACTION_TYPE, act_type, ebufp);
	PIN_FLIST_FLD_SET(in_flistp, PIN_FLD_STRING_ID, str_id, ebufp);
	PIN_FLIST_FLD_SET(in_flistp, PIN_FLD_STRING, str, ebufp);
	PIN_FLIST_FLD_SET(in_flistp, PIN_FLD_REASON_CODE, ch_str, ebufp);
	PIN_FLIST_FLD_SET(in_flistp, PIN_FLD_GL_ID, gl_id, ebufp);

	PIN_ERR_LOG_FLIST(3, "RBS_OP_ADJCODE_UPLOAD input flist:", in_flistp);
        PCM_OP(ctxp, RBS_OP_ADJCODE_UPLOAD, 0, in_flistp, op_out_flistpp, ebufp);
        if (PIN_ERR_IS_ERR(ebufp))
        {

        /***************************************************
        * Log something and return nothing.
        ***************************************************/
        strcat(buffer, ",Error occured while calling adjcode upload");
        fprintf(FAIL_OUTPUT_FILE, "%s\n", buffer);
        fflush(FAIL_OUTPUT_FILE);
        PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in adjcode upload", ebufp);
        }
        else
        {
                PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "adjcode upload output", *op_out_flistpp);
		status = PIN_FLIST_FLD_GET(*op_out_flistpp, PIN_FLD_STATUS, 0, ebufp);
                if (status && *status == 1)
                {
                        PIN_ERR_LOG_MSG(3, "Failed status");
                        err_desc = PIN_FLIST_FLD_GET(*op_out_flistpp, PIN_FLD_ERROR_DESCR, 0, ebufp);

                        fprintf(FAIL_OUTPUT_FILE,"\n%s,%d,%s,%s,%d,%s", act_type, *str_id, str, ch_str, *gl_id, err_desc);
                        fflush(FAIL_OUTPUT_FILE);
                }
                else
                {
                        PIN_ERR_LOG_MSG(3, "Success status");
                        success_count = success_count + 1;
                        fprintf(SUCC_OUTPUT_FILE,"\n%s,%d,%s,%s,%d,Success", act_type, *str_id, str, ch_str, *gl_id);
                        fflush(SUCC_OUTPUT_FILE);
                }
        }
	PIN_FLIST_DESTROY_EX(&in_flistp, NULL);
	PIN_FLIST_DESTROY_EX(op_out_flistpp, NULL);
        return;
}

/*******************************************************************
 * Function executed after seach results were processed
 * Called prior MTA_JOB_DONE policy opcode
 *******************************************************************/
PIN_EXPORT void
pin_mta_job_done(
       pin_flist_t             *app_flistp,
       pin_errbuf_t            *ebufp)
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
        pin_errbuf_t            *ebufp)
{
        char                    record_count[255];
        char                    suc_count[255];
        char                    fail_count[255];
        time_t                  pvt = (time_t)0;
        char                    fdate[80];

        if (PIN_ERR_IS_ERR(ebufp))
        {
        	return;
        }
        PIN_ERRBUF_CLEAR (ebufp);

        PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG, "pin_mta_exit application info flist", app_flistp);
        failed_count = (int)(rec_count - success_count);
        fprintf(FAIL_OUTPUT_FILE,"\n%d", failed_count);
        fflush(FAIL_OUTPUT_FILE);
        fclose(FAIL_OUTPUT_FILE);
        fprintf(SUCC_OUTPUT_FILE,"\n%d", success_count);
        fflush(SUCC_OUTPUT_FILE);
        fclose(SUCC_OUTPUT_FILE);
        rec_count = 0;
        success_count = 0;
        if (PIN_ERR_IS_ERR(ebufp))
        {
        	PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "pin_mta_exit error", ebufp);
        }
        return;
}

/*******************************************************************
 * Function called when new job is avaialble to worker
 * Called prior MTA_WORKER_JOB policy opcode
 *******************************************************************/
PIN_EXPORT void
pin_mta_worker_job(
	pcm_context_t   	*ctxp,
	pin_flist_t		*srch_res_flistp,
	pin_flist_t             **op_in_flistpp,
	pin_flist_t             *ti_flistp,
	pin_errbuf_t    	*ebufp)
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

PIN_EXPORT void
pin_mta_worker_job_done(
        pcm_context_t   *ctxp,
        pin_flist_t     *srch_res_flistp,
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
                                   srch_res_flistp);
	
    	PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG, "pin_mta_worker_job_done prepared flist for main opcode",
                                   op_in_flistp);

    	PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG, "pin_mta_worker_job_done output flist from main opcode",
                                   op_out_flistp);

    	PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG, "pin_mta_worker_job_done thread info flist",
                                   ti_flistp);

    	if (PIN_ERR_IS_ERR(ebufp)) 
	{
        	PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                          "pin_mta_worker_job_done error", ebufp);
    	}
    	return;
}

void parse(char *record, char arr[][500], int *fldcnt)
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
                if (*stptr != '\0' && *stptr == ','){
                        stptr++;
		}
               // strcpy(line2,stptr);
                //stptr = line2;

                strcpy(arr[lcount],line3);
        }
        *fldcnt = lcount + 1;
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
        	if (str[i] != ' ' && str[i] != '\t' && str[i] != '\n' && str[i] != '\r')
        	{
            		index = i;
        	}
        	i++;
    	}

    	/* Mark the next character to last non white space character as NULL */
    	str[index + 1] = '\0';
}

