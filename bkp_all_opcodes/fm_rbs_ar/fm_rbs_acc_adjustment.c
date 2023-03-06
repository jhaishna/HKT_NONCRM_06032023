#ifndef lint
static const char Sccs_id[] = "@(#)%Portal Version: fm_rbs_acc_adjustment.c:CUPmod7.3PatchInt:1:2006-Dec-01 16:31:18 %";
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
#include "pcm_ops.h"
#include "ops/ar.h"
#include "ops/bill.h"
#include "pin_decimal.h"
#include "rbs_ar_adjustment.h"

EXPORT_OP void
op_ar_acc_adjustment(
    cm_nap_connection_t *connp,
    int32           opcode,
    int32           flags,
    pin_flist_t     *i_flistp,
    pin_flist_t     **r_flistpp,
    pin_errbuf_t        *ebufp);

static void
fm_rbs_ar_adjustment_details(
    pcm_context_t       *ctxp,
    int32           flags,
    pin_flist_t     *i_flistp,
    pin_flist_t     **r_flistpp,
    pin_errbuf_t        *ebufp);



void
op_ar_acc_adjustment(
    cm_nap_connection_t *connp,
    int32           opcode,
    int32           flags,
    pin_flist_t     *i_flistp,
    pin_flist_t     **r_flistpp,
    pin_errbuf_t        *ebufp)
{
    pcm_context_t       *ctxp = connp->dm_ctx;

    *r_flistpp      = NULL;

    poid_t          *account_obj= NULL;


    int32               *flag_adj_type = NULL;
    int32               *tax_adj_type = NULL;
    int32               *flag = NULL;
    int                 status = 0;
    pin_decimal_t       *amount = NULL;
    pin_decimal_t       *abs_amount = NULL;
    pin_flist_t         *s_oflistp = NULL;
    pin_flist_t         *result_flistp = NULL;
    pin_decimal_t       *valid_amount = pbo_decimal_from_str("12340 = $123.40", ebufp);

    if (PIN_ERRBUF_IS_ERR(ebufp)) {
        return;
    }

    PIN_ERRBUF_CLEAR(ebufp);

    // Insanity Check

    if (opcode != RBS_OP_AR_ACC_ADJUSTMENT) {
        pin_set_err(ebufp, PIN_ERRLOC_FM,
            PIN_ERRCLASS_SYSTEM_DETERMINATE,
            PIN_ERR_BAD_OPCODE, 0, 0, opcode);
        PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
            "op_ar_acc_adjustment error",
            ebufp);
        return;
    }

    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "op_ar_acc_adjustment Input Flist:", i_flistp);

    //Check if adjustment amount is greater than 12340 = $123.40
    account_obj = PIN_FLIST_FLD_GET(i_flistp,PIN_FLD_POID,0,ebufp);
        if ( PIN_POID_IS_NULL(account_obj))
    {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"fm_rbs_ar_adjustment_details Validation : Adjustment more than 12340 = $123.40 not allowed", ebufp);
                PIN_ERRBUF_RESET(ebufp);
                status = 1;
                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, "fm_rbs_ar_adjustment_details error");
                *r_flistpp = PIN_FLIST_CREATE(ebufp);
                PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_POID,
                PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 1, ebufp ), ebufp);
                PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_ERROR_CODE, "53086", ebufp);
                PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_ERROR_DESCR,
                                                "Account poid should not null", ebufp);
                PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_STATUS, &status, ebufp);
        }
    amount = PIN_FLIST_FLD_GET(i_flistp,PIN_FLD_AMOUNT,0,ebufp);
    abs_amount  = pbo_decimal_abs(amount,ebufp);
    PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, "DEBUG 1");
    if(abs_amount && valid_amount &&  pbo_decimal_compare(abs_amount,valid_amount,ebufp) > 0)
    {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"fm_rbs_ar_adjustment_details Validation : Adjustment more than 12340 = $123.40 not allowed", ebufp);
                PIN_ERRBUF_RESET(ebufp);
                status = 1;
                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, "fm_rbs_ar_adjustment_details error");
                *r_flistpp = PIN_FLIST_CREATE(ebufp);
                PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_POID,
                PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 1, ebufp ), ebufp);
                PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_ERROR_CODE, "53085", ebufp);
                PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_ERROR_DESCR,
                                                "Validation : Adjustment more than 12340 = $123.40 not allowed", ebufp);
                PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_STATUS, &status, ebufp);
         if (abs_amount && !pbo_decimal_is_null(abs_amount, ebufp))
                       pbo_decimal_destroy(&abs_amount);

             if (valid_amount && !pbo_decimal_is_null(valid_amount, ebufp))
                       pbo_decimal_destroy(&valid_amount);


        return;
        }
        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, "DEBUG 3");
    // Call the main function to process the adjustments
    fm_rbs_ar_adjustment_details(ctxp, flags, i_flistp, r_flistpp, ebufp);

    if (PIN_ERRBUF_IS_ERR(ebufp))
    {
        PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"fm_rbs_ar_adjustment_details error", ebufp);
        PIN_ERRBUF_RESET(ebufp);
        /*status = 1;
        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, "fm_rbs_ar_adjustment_details error");
        *r_flistpp = PIN_FLIST_CREATE(ebufp);
        PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_POID,
        PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 1, ebufp ), ebufp);
        PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_ERROR_CODE, "53080", ebufp);
        PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_ERROR_DESCR,
                        "fm_rbs_ar_adjustment_details error", ebufp);
        PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_STATUS, &status, ebufp);*/
    }
    else
    {
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,  "op_ar_acc_adjustment output flist", *r_flistpp);
    }

    return;
}


static void
fm_rbs_ar_adjustment_details(
    pcm_context_t       *ctxp,
    int32           flags,
    pin_flist_t     *i_flistp,
    pin_flist_t     **r_flistp,
    pin_errbuf_t        *ebufp)
{

        poid_t          *bal_grp_pdp = NULL;
        pin_flist_t     *d_iflistp = NULL;
        pin_flist_t     *s_oflistp = NULL;
        pin_flist_t     *result_flistp = NULL;
        int32           status= 0;

        PIN_ERR_LOG_FLIST(3, "start fm_ar_acc_adjustment_details input flist: ", i_flistp);

        d_iflistp = PIN_FLIST_CREATE(ebufp);
        PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, d_iflistp, PIN_FLD_POID, ebufp);
        PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_AMOUNT, d_iflistp, PIN_FLD_AMOUNT, ebufp); //amount entered must be <= bill due amount
        PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_PROGRAM_NAME, d_iflistp, PIN_FLD_PROGRAM_NAME, ebufp);
        PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_DESCR, d_iflistp, PIN_FLD_DESCR, ebufp);
        PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_REASON_DOMAIN_ID, d_iflistp, PIN_FLD_STR_VERSION, ebufp);
        PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_REASON_ID, d_iflistp, PIN_FLD_STRING_ID, ebufp);
        PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_BAL_GRP_OBJ, d_iflistp, PIN_FLD_BAL_GRP_OBJ, ebufp);

       // PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_USERID, d_iflistp, PIN_FLD_USERID, ebufp); //
        if(bal_grp_pdp)
                {
            PIN_FLIST_FLD_SET(d_iflistp, PIN_FLD_BAL_GRP_OBJ, bal_grp_pdp, ebufp);
        }
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "PCM_OP_AR_ACCOUNT_ADJUSTMENT input flist", d_iflistp );
        PCM_OP(ctxp, PCM_OP_AR_ACCOUNT_ADJUSTMENT, 0, d_iflistp, &s_oflistp, ebufp);

        if (PIN_ERRBUF_IS_ERR(ebufp))
         {
            PIN_ERRBUF_CLEAR(ebufp);
            result_flistp = PIN_FLIST_CREATE(ebufp);
            status = 1;
            PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_STATUS, &status, ebufp);
            PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_ERROR_DESCR, "Account Adjustment Failed", ebufp);
            PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, result_flistp, PIN_FLD_POID, ebufp);
            PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_ERROR_CODE,"53084" , ebufp);

            PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "ACCOUNT_ADJUSTMENT Fail outflist flist", result_flistp);
            *r_flistp=PIN_FLIST_COPY(result_flistp, ebufp);
            PIN_FLIST_DESTROY_EX(&result_flistp, NULL);
                        return;
         }
	 PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "PCM_OP_AR_ACCOUNT_ADJUSTMENT output flist", s_oflistp );
         *r_flistp = PIN_FLIST_COPY(s_oflistp, ebufp);
	 PIN_FLIST_DESTROY_EX(&d_iflistp, NULL);
         return;

        }

