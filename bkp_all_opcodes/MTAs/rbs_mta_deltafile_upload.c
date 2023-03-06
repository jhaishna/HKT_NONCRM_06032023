/***************************************************************************

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
#include "rbs_delta_upload.h"
#define INR  356

/*******************************************************************
 * During real application development, there is no necessity to implement
 * empty MTA call back functions. Application will recognize implemented
 * functions automatically during run-time.
 *******************************************************************/
#define MAXFLDS 200
#define MAXFLDSIZE 300

FILE    *INPUT_FILE = NULL;
FILE    *OUTPUT_FILE = NULL;
int     rec_count = 0;
int 	current_date = 0;

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
        int32           days = 0;
        int32           *glid_upload_days = NULL;
        char            *value = NULL;
        char            *INPUT_FILENAME = NULL;
        char            str[1024];
        char            *output_file = NULL;
        char            fdate[80];
	char		filedate[7];
	char		extension[5];
        char            output_fname[255]="";
        char            buffer[1024];
        time_t          pvt = (time_t)0;
        int             err=0;
	char            *input_file = NULL;
	char		*input = NULL;
	char            infile[10];
	char            input_fname[255] = "";
	char		input_fformat[255] = "";
	char            yy[3];
	char		date[5];
	char		msg[100];


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
		pin_conf("rbs_deltafile_upload", "deltafile_upload_input_file", PIN_FLDT_STR, &input_file, &err);
		pvt = pin_virtual_time((time_t *)NULL);
                strftime(fdate, sizeof(fdate), "%d%m%y", localtime(&pvt));
                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, fdate);

                sprintf(input_fname, "%s", input_file);
                strcat(input_fname, fdate);
		strncpy(date, &fdate[4], 6);
		date[2] = '\0';
                strcat(input_fname, ".csv");	
		sprintf(input_fformat, "%s", input_fname);
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, input_fformat);
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
	input = (char *)INPUT_FILENAME;
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, input);
	strncpy(infile, &input[0], 9);
        infile[8] = '\0';
        PIN_ERR_LOG_MSG(3, infile);
	strncpy(filedate, &input[9],14);
	filedate[6] = '\0';
	PIN_ERR_LOG_MSG(3, filedate);
	strncpy(yy, &filedate[4], 6);
	yy[2] = '\0';
	PIN_ERR_LOG_MSG(3, yy);
	strncpy(extension, &input[15],18);
        extension[4] = '\0';
	PIN_ERR_LOG_MSG(3, extension);
	PIN_ERR_LOG_MSG(3, input_fname);
	if (input_fname && (strstr(input_fname, INPUT_FILENAME) || (strstr(input_fname, infile) && (strcmp(extension, ".csv") == 0))) && strcmp(yy, date) == 0)
	{
     		INPUT_FILE = fopen(input, "r+");
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
	pin_conf("rbs_deltafile_upload", "deltafile_upload_output_file", PIN_FLDT_STR, &output_file, &err);
	pvt = pin_virtual_time((time_t *)NULL);
	strftime(fdate, sizeof(fdate), "%Y%m%d", localtime(&pvt));

        sprintf(output_fname, "%s", output_file);
        strcat(output_fname, fdate);
        strcat(output_fname, ".csv");

	OUTPUT_FILE = fopen(output_fname, "w");
	fprintf(OUTPUT_FILE, "GLID,GL Description,Revenue Type,Segment,Attribute,Works Order (Debit),CCC (Debit),GL Account Code (Debit),Works Order (Credit),CCC (Credit),GL Account Code (Credit),description\n");
        fflush(OUTPUT_FILE);
	if ((strstr(input_fname, INPUT_FILENAME) || (strstr(input_fname, infile) && strcmp(extension, ".csv") == 0)) && strcmp(yy, date) == 0)
        {
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "File format validated");
	}
	else 
	{
		freopen(NULL, "w", OUTPUT_FILE);
		fprintf(OUTPUT_FILE, "File Format Mismatch", INPUT_FILENAME);
		fflush(OUTPUT_FILE);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "File Format Mismatch", ebufp);
        }

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
	int             year;
        int             month;
	int		month1 = 0;
        int             day;
	int32		records = 0;
        int32           upload_days = 0;
        int32           upload_t = 0;
        int32           days = 0;
        int32           *glid_upload_days = NULL;
        int32           line_counter = 0;
        int32           fldcnt=0;
        int32           err=0;
	int32		glid = 0;
	int32		glidlen = 0;	
	int64           db = -1;
        char   	 	buffer[1024];
        char          	arr[MAXFLDS][MAXFLDSIZE]={0x0};
        char            a_pdp_str[32];
        char            *input_file = NULL;
        char            *output_file = NULL;
        char            fdate[20];
	char            yyyy[5], mm[3], dd[3];
        char            validity[21];
        char            date[9];
        char            output_fname[255]="";
	char	 	msg[100];	
	char		*descr = NULL;
	char		*glidstr = NULL;
	char		*gl_segment = NULL;
	char		*type = NULL;
	char		*attribute = NULL;
	char            *Creditworksorder = NULL;
        char            *CreditCostCenterCode = NULL;
        char            *CreditGLaccountcode = NULL;
        char            *Debitworksorder = NULL;
        char            *DebitCostCenterCode = NULL;
        char            *DebitGLaccountc = NULL;	
        void            *vp = NULL;
        poid_t          *a_pdp = NULL;
        poid_t          *b_pdp = NULL;
        pin_flist_t     *result_flistp = NULL;
        pin_flist_t     *results_flistp = NULL;
	pin_flist_t	*rslt_flistp = NULL;
	int              a_cntr = 0;
	int32		inv_cnt = 0;
        time_t          pvt = (time_t)0;
	time_t          now_t = 0;
        struct tm       *timeeff;
	int32		errp = 0;

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
	PIN_ERR_LOG_POID(3, "Poid Val", vp);
	if(vp)
		db = PIN_POID_GET_DB ((poid_t*)vp);
		PIN_FLIST_FLD_PUT(results_flistp, PIN_FLD_POID, PIN_POID_CREATE(db, "/account", 1,ebufp), ebufp);
	        rslt_flistp = PIN_FLIST_ELEM_ADD(results_flistp, PIN_FLD_RESULTS, 0, ebufp);
		sprintf(date, "%d", current_date);
        	PIN_ERR_LOG_MSG(3, date);
        	pvt = pin_virtual_time((time_t *)NULL);
		while (fgets(buffer, sizeof buffer, INPUT_FILE) != NULL)
                {
                        parse(buffer, arr, &fldcnt);
                        rec_count = rec_count+1;
		
			gl_segment = (char *)(arr[1]);	
			glidstr = (char *)(arr[2]);
			glid = (int32)atoi(arr[2]);
			sprintf(msg, "glid: %d", glid);
			PIN_ERR_LOG_MSG(3, msg);
			descr = (char *)(arr[3]);
                        type = (char *)(arr[4]);
			attribute = (char *)(arr[5]);
			Creditworksorder = (char *)(arr[9]);
			CreditCostCenterCode = (char *)(arr[10]);
			CreditGLaccountcode = (char *)(arr[11]);
			Debitworksorder = (char *)(arr[6]);
			DebitCostCenterCode = (char *)(arr[7]);
			DebitGLaccountc = (char *)(arr[8]);
			if (strstr(gl_segment, "gl_segment"))
			{
				rec_count = rec_count-1;
				PIN_ERR_LOG_MSG(3, "heading");
			}
			else if (strstr(gl_segment, "H"))
			{
				rec_count = rec_count-1;
				PIN_ERR_LOG_MSG(3, "header");
				sprintf(fdate, "%s", arr[2]);
				PIN_ERR_LOG_MSG(3, fdate);
			}
			else if (strstr(gl_segment, "T"))
                        {
				rec_count = rec_count-1;
                                PIN_ERR_LOG_MSG(3, "Trailer");
                                records = (int32)(atoi(arr[2]));
                        }
			else
			{
				PIN_ERR_LOG_MSG(3, gl_segment);
                        	PIN_ERR_LOG_MSG(3, glidstr);
                        	PIN_ERR_LOG_MSG(3, Creditworksorder);
				PIN_ERR_LOG_MSG(3, CreditCostCenterCode);
				PIN_ERR_LOG_MSG(3, CreditGLaccountcode);
                	        PIN_ERR_LOG_MSG(3, Debitworksorder);
				PIN_ERR_LOG_MSG(3, DebitCostCenterCode);
				PIN_ERR_LOG_MSG(3, DebitGLaccountc);
				trim(CreditGLaccountcode);
        	                PIN_ERR_LOG_MSG(3, descr);
				sprintf(msg, "%s", glidstr);
				PIN_ERR_LOG_MSG(3, msg);
				if (strlen(msg) > 10)
				{
					fprintf(OUTPUT_FILE,"Invalid glid");
                		        fflush(OUTPUT_FILE);
		                        PIN_FLIST_DESTROY_EX (&results_flistp, ebufp);
					return;
				}
        	                sprintf(msg, "attr: %s, type: %s, glid: %d", attribute, type, glid);
	                        PIN_ERR_LOG_MSG(3, msg);
				
				PIN_FLIST_FLD_PUT(rslt_flistp, PIN_FLD_POID, PIN_POID_CREATE(db, "/config/glid", -1,ebufp), ebufp);
                        	result_flistp = PIN_FLIST_ELEM_ADD(rslt_flistp, PIN_FLD_RESULTS, a_cntr++, ebufp);
                        	PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_POID, PIN_POID_CREATE(db, "/search", -1,ebufp), ebufp);
				PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_GL_SEGMENT, gl_segment, ebufp);
				PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_GL_ID, &glid, ebufp);
				PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_DESCR, descr, ebufp);
				PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_STATUS_STR, type, ebufp);
				PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_ACTION, attribute, ebufp);
				PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_STRING, Debitworksorder, ebufp);
				PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_CODE_STR, DebitCostCenterCode, ebufp);
				PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_ACCOUNT_CODE, DebitGLaccountc, ebufp);	
				PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_ORDER_ID, Creditworksorder, ebufp);
                                PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_CODE, CreditCostCenterCode, ebufp);
                                PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_GLACCOUNT, CreditGLaccountcode, ebufp);
			}
		}
        	fclose(INPUT_FILE);
		pin_conf("deltafile_upload_days", "deltafile_upload_period", PIN_FLDT_INT, (caddr_t*)&glid_upload_days, &errp);
		if (errp != PIN_ERR_NONE)
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR,
				"pin_conf error!");
			return;
		}
		
		upload_days = (int32)*glid_upload_days;
		now_t = pin_virtual_time((time_t *)NULL);
		timeeff = localtime(&now_t);
		year = timeeff->tm_year + 1900;
		month = timeeff->tm_mon + 1;
		day = timeeff->tm_mday;
		memset(validity,'\0',21);
		sprintf(validity,"%04d%02d%02d",year,month,day);
		strncpy(dd, &fdate[6], 2);
		dd[2] = '\0'; 
		strncpy(mm, &fdate[4], 2);
		mm[2] = '\0';
		days = (int32)atoi(dd);
		month1 = (int32)atoi(mm);
		sprintf(msg, "month1 %d, month %d", month1, month);
		PIN_ERR_LOG_MSG(3, msg);
		if (month >= month1 && month1 <= 12)
		{
			month1 = (month - month1);
			month1 = (month1*30);
			days = (days - month1);
		}
		else 
		{
			freopen(NULL, "w", OUTPUT_FILE);
                        fprintf(OUTPUT_FILE,"Invalid date of the file\n");
                        fflush(OUTPUT_FILE);
                        PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Invalid date of the file", ebufp);
			PIN_FLIST_DESTROY_EX(&results_flistp, ebufp);
                }
		upload_t = (int32)(day - days);
		sprintf(msg, "upload_t %d, upload_days %d, day %d, days %d", upload_t, upload_days, day, days);
		PIN_ERR_LOG_MSG(3, msg);
        	if (PIN_ERR_IS_ERR(ebufp)) 
		{
                	PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "pin_mta_init_search error", ebufp);
                	PIN_FLIST_DESTROY_EX (&results_flistp, ebufp);
        	}
		else if (upload_t > upload_days)
		{
			freopen(NULL, "w", OUTPUT_FILE);
        	        fprintf(OUTPUT_FILE,"Invalid date of the file\n");
        	        fflush(OUTPUT_FILE);
	                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Invalid date of the file", ebufp);
		}
		else if (rec_count != records)
		{
			freopen(NULL, "w", OUTPUT_FILE);
			fprintf(OUTPUT_FILE, "No of Data Records not matching. Records in the file %d \nRecords count in the file %d", rec_count, records);
                        fflush(OUTPUT_FILE);
                        PIN_FLIST_DESTROY_EX (&results_flistp, ebufp);
		}
		else if (inv_cnt > 0)
		{
			fprintf(OUTPUT_FILE,"Invalid records encountered %d \nRecords not uploaded", inv_cnt);
			fflush(OUTPUT_FILE);
			PIN_FLIST_DESTROY_EX (&results_flistp, ebufp);
		}
		else
		{
            		*s_flistpp = results_flistp;
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
* Function executed after seach results were processed
* Called after MTA_JOB_DONE policy opcode
*******************************************************************/

PIN_EXPORT void
pin_mta_post_job_done(
        pin_flist_t             *app_flistp,
        pin_errbuf_t    	*ebufp)
{
        if (PIN_ERR_IS_ERR(ebufp)) 
	{
           return;
        }
        PIN_ERRBUF_CLEAR (ebufp);

        PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG, "pin_mta_post_job_done application info flist", app_flistp);

        if (PIN_ERR_IS_ERR(ebufp)) 
	{
            PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "pin_mta_post_job_done error", ebufp);
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
        char    		suc_count[255];
	char			fail_count[255];
        time_t          	pvt = (time_t)0;
        char    		fdate[80];
        if (PIN_ERR_IS_ERR(ebufp)) 
	{
           return;
        }
        PIN_ERRBUF_CLEAR(ebufp);

        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "pin_mta_exit application info flist", app_flistp);
        sprintf(record_count, "Total Records Processed: %d", rec_count);
        pvt = pin_virtual_time((time_t *)NULL);
        strftime(fdate, sizeof(fdate), "%d-%m-%Y.%H:%M", localtime(&pvt));

        fprintf(OUTPUT_FILE,"T,%d", rec_count);
        fflush(OUTPUT_FILE);
        fclose(OUTPUT_FILE);
	rec_count = 0;
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
	pin_flist_t		*in_flistp = NULL;
	pin_flist_t		*rslts = NULL;
	poid_t			*pdp = NULL;
	char			*acc_id_str = NULL;
	char            	buffer[1024];
	char			msg[100];
	char			*err_descr = NULL;
	char			*descr1 = NULL;
	char			*gl_segment1 = NULL; 
	char                    *type1 = NULL;	
	char                    *attr1 = NULL;
	char			*wod = NULL;
	char			*cccd = NULL;
	char			*glcd = NULL;
	char                    *woc = NULL;
        char                    *cccc = NULL;
        char                    *glcc = NULL;
	int32                   *glid1 = NULL;
	int32			*status = NULL;
	int32			fail_status = 0;
	int32			elem_id = 0;
	int64			db = -1;
	pin_cookie_t		cookie = NULL;
	void			*vp = NULL;

	if (PIN_ERR_IS_ERR(ebufp)) 
	{
            return;
        }
        PIN_ERRBUF_CLEAR (ebufp);

        PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG, "pin_mta_worker_opcode search results flist", srch_res_flistp);

        PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG, "pin_mta_worker_opcode prepared flist for main opcode", op_in_flistp);

        PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG, "pin_mta_worker_opcode thread info flist", ti_flistp);	
		
	pdp = PIN_FLIST_FLD_GET(srch_res_flistp, PIN_FLD_POID, 0, ebufp);
	db = PIN_POID_GET_DB(pdp);
	vp = PIN_POID_CREATE(db, "/account", 1, ebufp);
	in_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_SET(in_flistp, PIN_FLD_POID, vp, ebufp);
	PIN_FLIST_FLD_SET(in_flistp, PIN_FLD_PROGRAM_NAME, "MTA_DELTAFILE_UPLOAD", ebufp);
	while ((rslts = PIN_FLIST_ELEM_GET_NEXT(op_in_flistp, PIN_FLD_RESULTS, &elem_id, 1, &cookie, ebufp)) != NULL)
	{
		PIN_FLIST_ELEM_PUT(in_flistp, rslts, PIN_FLD_RESULTS, elem_id, ebufp);
	}
	PIN_ERR_LOG_FLIST(3, "pin_mta_worker_opcode input flist", in_flistp);

	PCM_OP(ctxp, RBS_OP_LOAD_PIN_GLID, 0, in_flistp, &op_out_flistp, ebufp);

        if (PIN_ERR_IS_ERR(ebufp)) 
	{

        /***************************************************
        * Log something and return nothing.
        ***************************************************/
        strcat(buffer, ",Error occured while calling deltafile upload");
        fprintf(OUTPUT_FILE, "%s\n", buffer);
        fflush(OUTPUT_FILE);
        PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in deltafile upload", ebufp);
        }
        else
        {
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "deltafile upload output", op_out_flistp);
		elem_id = 0;
		cookie = NULL;
		while((rslts = PIN_FLIST_ELEM_GET_NEXT(op_out_flistp, PIN_FLD_RESULTS, &elem_id, 1, &cookie, ebufp)) != NULL)
		{
			status = PIN_FLIST_FLD_GET(rslts, PIN_FLD_STATUS, 0, ebufp);
			if (status && *status == 0)
			{
				glid1 = PIN_FLIST_FLD_GET(rslts, PIN_FLD_GL_ID, 0, ebufp);
				type1 = PIN_FLIST_FLD_GET(rslts, PIN_FLD_STATUS_STR, 0, ebufp);
				attr1 = PIN_FLIST_FLD_GET(rslts, PIN_FLD_ACTION, 0, ebufp);
				descr1 = PIN_FLIST_FLD_GET(rslts, PIN_FLD_DESCR, 0, ebufp);
				gl_segment1 = PIN_FLIST_FLD_GET(rslts, PIN_FLD_GL_SEGMENT, 0, ebufp);
				woc = PIN_FLIST_FLD_GET(rslts, PIN_FLD_ORDER_ID, 0, ebufp);
				cccc = PIN_FLIST_FLD_GET(rslts, PIN_FLD_CODE, 0, ebufp);
				glcc = PIN_FLIST_FLD_GET(rslts, PIN_FLD_GLACCOUNT, 0, ebufp);
				wod = PIN_FLIST_FLD_GET(rslts, PIN_FLD_STRING, 0, ebufp);
				cccd = PIN_FLIST_FLD_GET(rslts, PIN_FLD_CODE_STR, 0, ebufp);
				glcd = PIN_FLIST_FLD_GET(rslts, PIN_FLD_ACCOUNT_CODE, 0, ebufp);
				err_descr = PIN_FLIST_FLD_GET(rslts, PIN_FLD_ERROR_DESCR, 0, ebufp);
				trim(cccc);
				trim(glcc);
        	    		fprintf(OUTPUT_FILE,"%d,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s\n", *glid1, descr1, type1, gl_segment1, attr1, wod, cccd, glcd, woc, cccc, glcc, err_descr);
        	    		fflush(OUTPUT_FILE);
			}
			else
			{
				fail_status = 1;
			}
		}
		if (fail_status == 1)
		{
			freopen(NULL, "w", OUTPUT_FILE);
			fprintf(OUTPUT_FILE, "GLID,GL Description,Revenue Type,Segment,Attribute,Works Order (Debit),CCC (Debit),GL Account Code (Debit),Works Order (Credit),CCC (Credit),GL Account Code (Credit),description\n");

			elem_id = 0;
                	cookie = NULL;
                	while((rslts = PIN_FLIST_ELEM_GET_NEXT(op_out_flistp, PIN_FLD_RESULTS, &elem_id, 1, &cookie, ebufp)) != NULL)
                	{
				status = PIN_FLIST_FLD_GET(rslts, PIN_FLD_STATUS, 0, ebufp);
	                        if (status && *status == 1)
				{
					glid1 = PIN_FLIST_FLD_GET(rslts, PIN_FLD_GL_ID, 0, ebufp);
                                	type1 = PIN_FLIST_FLD_GET(rslts, PIN_FLD_STATUS_STR, 0, ebufp);
                                	attr1 = PIN_FLIST_FLD_GET(rslts, PIN_FLD_ACTION, 0, ebufp);
                                	descr1 = PIN_FLIST_FLD_GET(rslts, PIN_FLD_DESCR, 0, ebufp);
                                	gl_segment1 = PIN_FLIST_FLD_GET(rslts, PIN_FLD_GL_SEGMENT, 0, ebufp);
                                	woc = PIN_FLIST_FLD_GET(rslts, PIN_FLD_ORDER_ID, 0, ebufp);
                                	cccc = PIN_FLIST_FLD_GET(rslts, PIN_FLD_CODE, 0, ebufp);
                                	glcc = PIN_FLIST_FLD_GET(rslts, PIN_FLD_GLACCOUNT, 0, ebufp);
                                	wod = PIN_FLIST_FLD_GET(rslts, PIN_FLD_STRING, 0, ebufp);
                                	cccd = PIN_FLIST_FLD_GET(rslts, PIN_FLD_CODE_STR, 0, ebufp);
                                	glcd = PIN_FLIST_FLD_GET(rslts, PIN_FLD_ACCOUNT_CODE, 0, ebufp);
                                	err_descr = PIN_FLIST_FLD_GET(rslts, PIN_FLD_ERROR_DESCR, 0, ebufp);
                                	trim(cccc);
                                	trim(glcc);
					fprintf(OUTPUT_FILE, "Record %d in the file failed due to %s\n", elem_id+1, err_descr);
        	                        fprintf(OUTPUT_FILE,"%d,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s\n", *glid1, descr1, type1, gl_segment1, attr1, wod, cccd, glcd, woc, cccc, glcc, err_descr);
	                                fflush(OUTPUT_FILE);
				}
			}
			fprintf(OUTPUT_FILE, "Invalid DATA Found. Records Not uploaded\n");
			fflush(OUTPUT_FILE);
		}
		else
		{
			fprintf(OUTPUT_FILE, "Total %d records successfully loaded\n", rec_count);
                        fflush(OUTPUT_FILE);
		}
        }
cleanup:
//        PIN_FLIST_DESTROY_EX(&in_flistp, NULL);
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

    /*
     * Trim leading white spaces
     */
    while(str[index] == ' ' || str[index] == '\t' || str[index] == '\n' || str[index] == '\r' )
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
        if(str[i] != ' ' && str[i] != '\t' && str[i] != '\n' && str[i] != '\r' )
        {
            index = i;
        }

        i++;
    }

    /* Mark the next character to last non white space character as NULL */
    str[index + 1] = '\0';
}

