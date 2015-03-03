/*lint -e766*/ 
/*lint -e529*/ 
/*lint -e64*/ 
/*lint -e18*/ 

/*  A Bison parser, made from ..\atc.y
    by GNU Bison version 1.28  */

#define ATCBISON 1  /* Identify Bison output.  */

#define    AT_CMD_PREFIX    257
#define    AT_CMD_REPEAT    258
#define    AT_EXTENSION_CMD    259
#define    AT_BASIC_CMD    260
#define    AT_CMD_END    261
#define    AT_CMD_DIVIDE    262
#define    NUMBER    263
#define    HNUMBER    264
#define    STRING    265
#define    DIALSTRING    266
#define    SHARPSTRING    267
#define    DTMFSTRING    268


/*******************************************************************************
 ** File Name:      AT.y                                                       *
 ** Author:         Steven.Yao                                                 *
 ** DATE:           04/12/2002                                                 *
 ** Copyright:      2001 Spreatrum, Incoporated. All Rights Reserved.          *
 ** Description:    This file describe the grammar rules of AT commands. GNU   *
 **                 Bison reads this file to produce the AT commands parser we *
 **                 want.                                                      *
 *******************************************************************************
                                                                               
 *******************************************************************************
 **                        Edit History                                        *
 ** -------------------------------------------------------------------------- *
 ** DATE           NAME             DESCRIPTION                                *
 ** 04/12/2002     Steven.Yao       Create.                                    *
 ** 06/05/2002     Ivan.Yin         add the handle about "A/"                  *
 ******************************************************************************/
 
/*******************************************************************************
 **                        Common Header Files                                 *
 ******************************************************************************/
#include "sci_types.h"


/*******************************************************************************
 **                        ATC Header Files                                    *
 ******************************************************************************/
#include "atc.h"
#include "atc_common.h"
#include "atc_malloc.h"
#include "atc_yacc.h"

#define ATCINITDEPTH 50

extern uint8 * atctext;
extern int32   atcleng;
extern char * atc_c_buf_p;
int32 atclex(void);

void ATCY_ErrorHandle( ATC_STATUS err_code );

LOCAL int32 atc_convert_num_param(void); // Return numeric parameter value
LOCAL ATC_CUR_PARAM_STRING_T * atc_convert_str_param(void); 
LOCAL ATC_CUR_PARAM_STRING_T * atc_convert_dialstr_param(void); 

LOCAL ATC_STATUS ATCY_NewNumParam( // Return S_ATC_SUCCESS if success,
                                     // otherwise return error code.
        ATC_CUR_PARAM_INFO_LIST_T ** param_ptr_ptr // OUT argument, return a
                                                   // param info list struct.
        );

LOCAL ATC_STATUS ATCY_NewStrParam( // Return S_ATC_SUCCESS if success,
                                     // otherwise return error code.
        ATC_CUR_PARAM_INFO_LIST_T ** param_ptr_ptr  // OUT argument, return a
                                                    // param info list struct.
        );

LOCAL ATC_STATUS ATCY_NewDialStrParam( // Return S_ATC_SUCCESS if success,
                                         // otherwise return error code.
        ATC_CUR_PARAM_INFO_LIST_T ** param_ptr_ptr  // OUT argument, return a
                                                    // param info list struct.
        );

LOCAL ATC_STATUS ATCY_NewDefaultParam( // Return S_ATC_SUCCESS if success,
                                         // otherwise return error code.
        ATC_CUR_PARAM_INFO_LIST_T ** param_ptr_ptr  // OUT argument, return a
                                                    // param info list struct.
        );

LOCAL ATC_CUR_PARAM_INFO_LIST_T * ATCY_LinkTwoParams(// return the linked list
        ATC_CUR_PARAM_INFO_LIST_T * param_first_ptr,
        ATC_CUR_PARAM_INFO_LIST_T * param_last_ptr
        );

LOCAL ATC_STATUS ATCY_NewCommand( // Return S_ATC_SUCCESS if success,
                                    // otherwise return error code
        ATC_CUR_INFO_LIST_T ** info_ptr_ptr, // OUT argument
        uint    cmd_type,           // AT command type
        uint    cmd_index,          // AT command index
        ATC_CUR_PARAM_INFO_LIST_T * param_list_ptr // AT command param info
        );

LOCAL ATC_CUR_INFO_LIST_T * ATCY_LinkTwoCmdInfo(// return the linked list
        ATC_CUR_INFO_LIST_T * info_list_first_ptr,
        ATC_CUR_INFO_LIST_T * info_list_last_ptr
        );

LOCAL ATC_STATUS ATCY_ParseError(// @Steven
        ATC_CUR_INFO_LIST_T * info_ptr
        );
        
ATC_STATUS  parse_status = S_ATC_SUCCESS;



#define    ATCFINAL        38
#define    ATCFLAG        -32768
#define    ATCNTBASE    18

#define ATCTRANSLATE(x) ((unsigned)(x) <= 268 ? atctranslate[x] : 26)

static const char atctranslate[] = {     0,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,    17,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
    16,     2,    15,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     1,     3,     4,     5,     6,
     7,     8,     9,    10,    11,    12,    13,    14
};





static const short atcr1[] = {     0,
    18,    18,    19,    19,    19,    19,    19,    19,    19,    20,
    20,    21,    21,    21,    21,    21,    22,    22,    22,    22,
    22,    23,    23,    23,    23,    24,    24,    25,    25,    25
};

static const short atcr2[] = {     0,
     3,     2,     3,     3,     3,     2,     1,     1,     1,     2,
     2,     1,     2,     2,     3,     3,     1,     1,     1,     1,
     1,     1,     2,     3,     3,     1,     3,     1,     1,     0
};

static const short atcdefact[] = {     0,
     0,    22,    12,     2,     0,     7,     8,     9,    23,    30,
    17,    18,    19,    20,    21,    14,     0,    13,     1,     0,
    10,     6,    11,    28,    29,    24,    25,    26,    15,    16,
     3,     4,     5,    30,    27,     0,     0,     0
};

static const short atcdefgoto[] = {    36,
     5,     6,    21,    18,     8,    27,    28
};

static const short atcpact[] = {    10,
    -3,    14,    -4,-32768,    26,    30,    11,-32768,-32768,    17,
-32768,-32768,-32768,-32768,-32768,-32768,     9,-32768,-32768,    30,
-32768,-32768,-32768,-32768,-32768,-32768,     8,-32768,-32768,-32768,
    11,    11,-32768,     5,-32768,    27,    37,-32768
};

static const short atcpgoto[] = {-32768,
-32768,    18,    -1,    22,    -5,-32768,     6
};


#define    ATCLAST        40


static const short atctable[] = {     7,
    22,     2,     3,     4,    11,    23,    12,    13,    14,    15,
    16,    17,     1,    24,    33,    25,     3,    11,    32,    12,
    13,    14,    15,    29,    34,    24,    37,    25,     9,    10,
    23,    26,    19,    20,     2,     3,    38,    31,    30,    35
};

static const short atccheck[] = {     1,
     6,     5,     6,     7,     9,     7,    11,    12,    13,    14,
    15,    16,     3,     9,    20,    11,     6,     9,    20,    11,
    12,    13,    14,    15,    17,     9,     0,    11,    15,    16,
    32,    15,     7,     8,     5,     6,     0,    20,    17,    34
};

#define ATCEMPTY        -2
#define ATCEOF        0
#ifndef ATCPURE
#define ATCLEX        atclex()
#endif


/* If nonreentrant, generate the variables here */

#ifndef ATCPURE

int    atcchar;            /*  the lookahead symbol        */
ATCSTYPE    atclval;            /*  the semantic value of the        */
                /*  lookahead symbol            */


#endif  /* not ATCPURE */


/*  ATCINITDEPTH indicates the initial size of the parser's stacks    */

#ifndef    ATCINITDEPTH
#define ATCINITDEPTH 200
#endif

ATC_STATUS atcparse(uint8 * atcbuf, ATC_CUR_INFO_LIST_T ** info_ptr_ptr)
{
  register int atcstate;
  register int atcn;
  register short *atcssp;
  register ATCSTYPE *atcvsp;
  int atcchar1 = 0;        /*  lookahead token as an internal (translated) token number */

  short    atcssa[ATCINITDEPTH];    /*  the state stack            */
  ATCSTYPE atcvsa[ATCINITDEPTH];    /*  the semantic value stack        */

  short *atcss = atcssa;        /*  refer to the stacks thru separate pointers */
  ATCSTYPE *atcvs = atcvsa;    /*  to allow atcoverflow to reallocate them elsewhere */


  int atcstacksize = ATCINITDEPTH;
  int atcfree_stacks = 0;


  ATCSTYPE atcval;        /*  the variable used to return        */
                /*  semantic values from the action    */
                /*  routines                */

  int atclen;
    atc_c_buf_p = atcbuf;


  atcstate = 0;
  atcchar = ATCEMPTY;        /* Cause a token to be read.  */

  /* Initialize stack pointers.
     Waste one element of value and location stack
     so that they stay on the same level as the state stack.
     The wasted elements are never initialized.  */

  atcssp = atcss - 1;
  atcvsp = atcvs;

/* Push a new state, which is found in  atcstate  .  */
/* In all cases, when you get here, the value and location stacks
   have just been pushed. so pushing a state here evens the stacks.  */
atcnewstate:

  *++atcssp = atcstate;

  goto atcbackup;
 atcbackup:

/* Do appropriate processing given the current state.  */
/* Read a lookahead token if we need one and don't already have one.  */
/* atcresume: */

  /* First try to decide what to do without reference to lookahead token.  */

  atcn = atcpact[atcstate];
  if (atcn == ATCFLAG)
    goto atcdefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* atcchar is either ATCEMPTY or ATCEOF
     or a valid token in external form.  */

  if (atcchar == ATCEMPTY)
    {
      atcchar = ATCLEX;
    }

  /* Convert token to internal form (in atcchar1) for indexing tables with */

  if (atcchar <= 0)        /* This means end of input. */
    {
      atcchar1 = 0;
      atcchar = ATCEOF;        /* Don't call ATCLEX any more */

    }
  else
    {
      atcchar1 = ATCTRANSLATE(atcchar);

    }

  atcn += atcchar1;
  if (atcn < 0 || atcn > ATCLAST || atccheck[atcn] != atcchar1)
    goto atcdefault;

  atcn = atctable[atcn];

  /* atcn is what to do for this token type in this state.
     Negative => reduce, -atcn is rule number.
     Positive => shift, atcn is new state.
       New state is final state => don't bother to shift,
       just return success.
     0, or most negative number => error.  */

  if (atcn < 0)
    {
      if (atcn == ATCFLAG)
    goto atcerrlab;
      atcn = -atcn;
      goto atcreduce;
    }
  else if (atcn == 0)
    goto atcerrlab;

  if (atcn == ATCFINAL)
    return 0;

  /* Shift the lookahead token.  */


  /* Discard the token being shifted unless it is eof.  */
  if (atcchar != ATCEOF)
    atcchar = ATCEMPTY;

  *++atcvsp = atclval;

  /* count tokens shifted since error; after three, turn off error status.  */

  atcstate = atcn;
  goto atcnewstate;

/* Do the default action for the current state.  */
atcdefault:

  atcn = atcdefact[atcstate];
  if (atcn == 0)
    goto atcerrlab;

/* Do a reduction.  atcn is the number of a rule to reduce with.  */
atcreduce:
  atclen = atcr2[atcn];
  if (atclen > 0)
    atcval = atcvsp[1-atclen]; /* implement default value of the action */



  switch (atcn) {

case 1:
{
                            *info_ptr_ptr = atcvsp[-1].atc_cur_info_ptr;
                            return parse_status;
                        ;
    break;}
case 2:
{
                            parse_status = ATCY_NewCommand( 
                                                info_ptr_ptr,
                                                ATC_CMD_TYPE_EXECUTE | ATC_CMD_TYPE_BASIC,
                                                0,
                                                NULL);
                            if ( S_ATC_SUCCESS != parse_status )
                            {
                                ATCY_ErrorHandle( parse_status );
                                return parse_status;
                            }
                            
                            return parse_status;       
                        ;
    break;}
case 3:
{
                            atcval.atc_cur_info_ptr = ATCY_LinkTwoCmdInfo( atcvsp[-2].atc_cur_info_ptr, atcvsp[0].atc_cur_info_ptr);
                            
                            *info_ptr_ptr = atcval.atc_cur_info_ptr; // @Steven
                        ;
    break;}
case 4:
{
                            atcval.atc_cur_info_ptr = ATCY_LinkTwoCmdInfo( atcvsp[-2].atc_cur_info_ptr, atcvsp[0].atc_cur_info_ptr);
                            
                            *info_ptr_ptr = atcval.atc_cur_info_ptr; // @Steven
                        ;
    break;}
case 5:
{
                            atcval.atc_cur_info_ptr = ATCY_LinkTwoCmdInfo( atcvsp[-2].atc_cur_info_ptr, atcvsp[0].atc_cur_info_ptr);
                            
                            *info_ptr_ptr = atcval.atc_cur_info_ptr; // @Steven
                        ;
    break;}
case 6:
{
                            atcval.atc_cur_info_ptr = ATCY_LinkTwoCmdInfo( atcvsp[-1].atc_cur_info_ptr, atcvsp[0].atc_cur_info_ptr);
                            
                            *info_ptr_ptr = atcval.atc_cur_info_ptr; // @Steven
                        ;
    break;}
case 7:
{
                            atcval.atc_cur_info_ptr = atcvsp[0].atc_cur_info_ptr;
                            
                            *info_ptr_ptr = atcval.atc_cur_info_ptr; // @Steven
                        ;
    break;}
case 8:
{
                            atcval.atc_cur_info_ptr = atcvsp[0].atc_cur_info_ptr;
                            
                            *info_ptr_ptr = atcval.atc_cur_info_ptr; // @Steven
                        ;
    break;}
case 9:
{
                            atcval.atc_cur_info_ptr = atcvsp[0].atc_cur_info_ptr;
                            
                            *info_ptr_ptr = atcval.atc_cur_info_ptr; // @Steven
                        ;
    break;}
case 10:
{
                            atcval.atc_cur_info_ptr = ATCY_LinkTwoCmdInfo( atcvsp[-1].atc_cur_info_ptr, atcvsp[0].atc_cur_info_ptr);
                            
                            *info_ptr_ptr = atcval.atc_cur_info_ptr; // @Steven
                        ;
    break;}
case 11:
{
                            atcval.atc_cur_info_ptr = ATCY_LinkTwoCmdInfo( atcvsp[-1].atc_cur_info_ptr, atcvsp[0].atc_cur_info_ptr);
                            
                            *info_ptr_ptr = atcval.atc_cur_info_ptr; // @Steven
                        ;
    break;}
case 12:
{
                            parse_status = ATCY_NewCommand( 
                                                &atcval.atc_cur_info_ptr,
                                                ATC_CMD_TYPE_EXECUTE | ATC_CMD_TYPE_BASIC,
                                                atcvsp[0].cmd_index,
                                                NULL);
                            if ( S_ATC_SUCCESS != parse_status )
                            {
                                ATCY_ErrorHandle( parse_status );
                                return parse_status;
                            }       
                        ;
    break;}
case 13:
{
                            parse_status = ATCY_NewCommand( 
                                                &atcval.atc_cur_info_ptr,
                                                ATC_CMD_TYPE_EXECUTE | ATC_CMD_TYPE_BASIC,
                                                atcvsp[-1].cmd_index,
                                                atcvsp[0].cur_param_ptr);
                            if ( S_ATC_SUCCESS != parse_status )
                            {
                                ATCY_ErrorHandle( parse_status );
                                return parse_status;
                            }       
                        ;
    break;}
case 14:
{
                            parse_status = ATCY_NewCommand( 
                                                &atcval.atc_cur_info_ptr,
                                                ATC_CMD_TYPE_READ | ATC_CMD_TYPE_BASIC,
                                                atcvsp[-1].cmd_index,
                                                NULL);
                            if ( S_ATC_SUCCESS != parse_status )
                            {
                                ATCY_ErrorHandle( parse_status );
                                return parse_status;
                            }       
                        ;
    break;}
case 15:
{
                            parse_status = ATCY_NewCommand( 
                                                &atcval.atc_cur_info_ptr,
                                                ATC_CMD_TYPE_TEST | ATC_CMD_TYPE_BASIC,
                                                atcvsp[-2].cmd_index,
                                                NULL);
                            if ( S_ATC_SUCCESS != parse_status )
                            {
                                ATCY_ErrorHandle( parse_status );
                                return parse_status;
                            }       
                        ;
    break;}
case 16:
{
                            parse_status = ATCY_NewCommand( 
                                                &atcval.atc_cur_info_ptr,
                                                ATC_CMD_TYPE_SET | ATC_CMD_TYPE_BASIC,
                                                atcvsp[-2].cmd_index,
                                                atcvsp[0].cur_param_ptr);
                            if ( S_ATC_SUCCESS != parse_status )
                            {
                                ATCY_ErrorHandle( parse_status );
                                return parse_status;
                            }       
                        ;
    break;}
case 17:
{
                            parse_status = ATCY_NewNumParam(&atcval.cur_param_ptr);
                            if ( S_ATC_SUCCESS != parse_status )
                            {
                                ATCY_ErrorHandle( parse_status );
                                return parse_status;
                            }
                        ;
    break;}
case 18:
{
                            parse_status = ATCY_NewStrParam(&atcval.cur_param_ptr);
                            if ( S_ATC_SUCCESS != parse_status )
                            {
                                ATCY_ErrorHandle( parse_status );
                                return parse_status;
                            }
                        ;
    break;}
case 19:
{
                            parse_status = ATCY_NewDialStrParam(&atcval.cur_param_ptr);
                            if ( S_ATC_SUCCESS != parse_status )
                            {
                                ATCY_ErrorHandle( parse_status );
                                return parse_status;
                            }
                        ;
    break;}
case 20:
{
                            parse_status = ATCY_NewDialStrParam(&atcval.cur_param_ptr);
                            if ( S_ATC_SUCCESS != parse_status )
                            {
                                ATCY_ErrorHandle( parse_status );
                                return parse_status;
                            }
                        ;
    break;}
case 21:
{
                            parse_status = ATCY_NewDialStrParam(&atcval.cur_param_ptr);
                            if ( S_ATC_SUCCESS != parse_status )
                            {
                                ATCY_ErrorHandle( parse_status );
                                return parse_status;
                            }
                        ;
    break;}
case 22:
{
                            parse_status = ATCY_NewCommand( 
                                                &atcval.atc_cur_info_ptr,
                                                ATC_CMD_TYPE_EXECUTE | ATC_CMD_TYPE_EXTEND,
                                                atcvsp[0].cmd_index,
                                                NULL);
                            if ( S_ATC_SUCCESS != parse_status )
                            {
                                ATCY_ErrorHandle( parse_status );
                                return parse_status;
                            }       
                        ;
    break;}
case 23:
{
                            parse_status = ATCY_NewCommand( 
                                                &atcval.atc_cur_info_ptr,
                                                ATC_CMD_TYPE_READ | ATC_CMD_TYPE_EXTEND,
                                                atcvsp[-1].cmd_index,
                                                NULL);
                            if ( S_ATC_SUCCESS != parse_status )
                            {
                                ATCY_ErrorHandle( parse_status );
                                return parse_status;
                            }       
                        ;
    break;}
case 24:
{
                            parse_status = ATCY_NewCommand( 
                                                &atcval.atc_cur_info_ptr,
                                                ATC_CMD_TYPE_TEST | ATC_CMD_TYPE_EXTEND,
                                                atcvsp[-2].cmd_index,
                                                NULL);
                            if ( S_ATC_SUCCESS != parse_status )
                            {
                                ATCY_ErrorHandle( parse_status );
                                return parse_status;
                            }       
                        ;
    break;}
case 25:
{
                            parse_status = ATCY_NewCommand( 
                                                &atcval.atc_cur_info_ptr,
                                                ATC_CMD_TYPE_SET | ATC_CMD_TYPE_EXTEND,
                                                atcvsp[-2].cmd_index,
                                                atcvsp[0].cur_param_ptr);
                            if ( S_ATC_SUCCESS != parse_status )
                            {
                                ATCY_ErrorHandle( parse_status );
                                return parse_status;
                            }       
                        ;
    break;}
case 26:
{
                            atcval.cur_param_ptr = atcvsp[0].cur_param_ptr;
                        ;
    break;}
case 27:
{
                            atcval.cur_param_ptr = ATCY_LinkTwoParams( atcvsp[-2].cur_param_ptr, atcvsp[0].cur_param_ptr );
                        ;
    break;}
case 28:
{
                            parse_status = ATCY_NewNumParam(&atcval.cur_param_ptr);
                            if ( S_ATC_SUCCESS != parse_status )
                            {
                                ATCY_ErrorHandle( parse_status );
                                return parse_status;
                            }
                        ;
    break;}
case 29:
{
                            parse_status = ATCY_NewStrParam(&atcval.cur_param_ptr);
                            if ( S_ATC_SUCCESS != parse_status )
                            {
                                ATCY_ErrorHandle( parse_status );
                                return parse_status;
                            }
                        ;
    break;}
case 30:
{
                            parse_status = ATCY_NewDefaultParam(&atcval.cur_param_ptr);
                            if ( S_ATC_SUCCESS != parse_status )
                            {
                                ATCY_ErrorHandle( parse_status );
                                return parse_status;
                            }
                        ;
    break;}
}
   /* the action file gets copied in in place of this dollarsign */


  atcvsp -= atclen;
  atcssp -= atclen;


  *++atcvsp = atcval;


  /* Now "shift" the result of the reduction.
     Determine what state that goes to,
     based on the state we popped back to
     and the rule number reduced by.  */

  atcn = atcr1[atcn];

  atcstate = atcpgoto[atcn - ATCNTBASE] + *atcssp;
  if (atcstate >= 0 && atcstate <= ATCLAST && atccheck[atcstate] == *atcssp)
    atcstate = atctable[atcstate];
  else
    atcstate = atcdefgoto[atcn - ATCNTBASE];

  goto atcnewstate;

atcerrlab:   /* here on detecting error */

    return ATCY_ParseError(*info_ptr_ptr);
}


void ATCY_ErrorHandle( ATC_STATUS err_code )
{
    ATCS_TRACE_STATUS( err_code);

    ATCM_FreeAll();

    return;
}

/******************************************************************************/
// Description : This function converts the numeric parameter string to an 
//               int32 value.
// Global resource dependence : atctext
//                              atcleng
// Author : Steven.Yao 
// Note : The numeric parameter string is contain in atctext.
/******************************************************************************/
LOCAL int32 atc_convert_num_param(void) // Return numeric parameter value
{
    int32       num_val;
    uint8       hold_char;

    SCI_ASSERT( NULL != atctext );/*assert verified: check null pointer*/
    SCI_ASSERT( 0 < atcleng );/*assert verified: check valid value*/

    hold_char = atctext[atcleng];
    atctext[atcleng] = '\0';

    num_val = atoi( (const char *)atctext );

    atctext[atcleng] = hold_char;

    return num_val;
}

/******************************************************************************/
// Description : This function stores a string parameter in an
//               ATC_CUR_PARAM_STRING_T struct type.
// Global resource dependence : atctext
//                              atcleng
// Author : Steven.Yao 
// Note : None
/******************************************************************************/
LOCAL ATC_CUR_PARAM_STRING_T * atc_convert_str_param(void) 
{
    ATC_CUR_PARAM_STRING_T  * str_ptr = NULL;

    ATC_STATUS  status = S_ATC_SUCCESS;

    uint8 hold_char;

    SCI_ASSERT( NULL != atctext );/*assert verified: check null pointer*/
    SCI_ASSERT( 1 < atcleng );/*assert verified: check valid value*/

    hold_char = atctext[atcleng - 1];
    atctext[atcleng - 1] = '\0';

    status = ATCM_Alloc( (void **)&str_ptr, sizeof(ATC_CUR_PARAM_STRING_T) );
    if ( S_ATC_SUCCESS != status )
    {
        return NULL;
    }

    str_ptr->str_len = atcleng - 2;
    status = ATCM_Alloc( (void **)&str_ptr->str_ptr, (atcleng - 1) );
    if ( S_ATC_SUCCESS != status )
    {
        return NULL;
    }

    memcpy( str_ptr->str_ptr, (atctext + 1), (atcleng - 1) );

    atctext[atcleng - 1] = hold_char;

    return str_ptr;
}

/******************************************************************************/
// Description : This function stores a dial string parameter in an 
//               ATC_CUR_PARAM_STRING_T struct type.
// Global resource dependence : atctext
//                              atcleng
// Author : Steven.Yao 
// Note : None
/******************************************************************************/
LOCAL ATC_CUR_PARAM_STRING_T * atc_convert_dialstr_param(void) 
{
    ATC_CUR_PARAM_STRING_T  * str_ptr = NULL;

    ATC_STATUS  status = S_ATC_SUCCESS;

    uint8 hold_char;

    SCI_ASSERT( NULL != atctext );/*assert verified: check null pointer*/
    SCI_ASSERT( 0 < atcleng );/*assert verified: check valid value*/

    hold_char = atctext[atcleng];
    atctext[atcleng] = '\0';

    status = ATCM_Alloc( (void **)&str_ptr, sizeof(ATC_CUR_PARAM_STRING_T) );
    if ( S_ATC_SUCCESS != status )
    {
        return NULL;
    }

    str_ptr->str_len = atcleng;
    status = ATCM_Alloc( (void **)&str_ptr->str_ptr, (atcleng + 1) );
    if ( S_ATC_SUCCESS != status )
    {
        return NULL;
    }

    memcpy( str_ptr->str_ptr, atctext, (atcleng + 1) );

    atctext[atcleng] = hold_char;

    return str_ptr;
}

/******************************************************************************/
// Description : This function takes an numeric value and creates a new 
//               ATC_CUR_PARAM_INFO_LIST_T struct to include the value.
// Global resource dependence : None
// Author : Steven.Yao
// Note : This function is called when grammar parser do the following induce:
//        NUMBER ------> ExtParam 
//        NUMBER ------> Param
/******************************************************************************/
LOCAL ATC_STATUS ATCY_NewNumParam( // Return S_ATC_SUCCESS if success,
                                     // otherwise return error code.
        ATC_CUR_PARAM_INFO_LIST_T ** param_ptr_ptr // OUT argument, return a
                                                   // param info list struct.
        )
{
    ATC_STATUS  status = S_ATC_SUCCESS;

    ATC_CUR_PARAM_INFO_LIST_T * tmp_param_ptr = NULL;

    SCI_ASSERT( NULL != param_ptr_ptr );/*assert verified: check null pointer*/

    status = ATCM_Alloc( (void **)&tmp_param_ptr, sizeof( ATC_CUR_PARAM_INFO_LIST_T ) );
    if ( S_ATC_SUCCESS != status )
    {
        return status;
    }

    SCI_ASSERT( NULL != tmp_param_ptr );/*assert verified: check null pointer*/
    tmp_param_ptr->param_type = ATC_CMD_PARAM_TYPE_NUMERIC;
    tmp_param_ptr->param_info.num = atc_convert_num_param();
    tmp_param_ptr->next = NULL;

    *param_ptr_ptr = tmp_param_ptr;

    return status;
}

/******************************************************************************/
// Description : This function takes an string value and creates a new 
//               ATC_CUR_PARAM_INFO_LIST_T struct to include the value.
// Global resource dependence : None
// Author : Steven.Yao
// Note : This function is called when grammar parser does the following induce:
//        STRING ------> ExtParam 
//        STRING ------> Param
//        DIALSTRING --> Param
/******************************************************************************/
LOCAL ATC_STATUS ATCY_NewStrParam( // Return S_ATC_SUCCESS if success,
                                     // otherwise return error code.
        ATC_CUR_PARAM_INFO_LIST_T ** param_ptr_ptr // OUT argument, return a
                                                   // param info list struct.
        )
{
    ATC_STATUS  status = S_ATC_SUCCESS;

    ATC_CUR_PARAM_INFO_LIST_T * tmp_param_ptr = NULL;

    SCI_ASSERT( NULL != param_ptr_ptr );/*assert verified: check null pointer*/

    status = ATCM_Alloc( (void **)&tmp_param_ptr, sizeof( ATC_CUR_PARAM_INFO_LIST_T ) );
    if ( S_ATC_SUCCESS != status )
    {
        *param_ptr_ptr = NULL;

        return status;
    }

    SCI_ASSERT( NULL != tmp_param_ptr );/*assert verified: check null pointer*/
    tmp_param_ptr->param_type = ATC_CMD_PARAM_TYPE_STRING;
    tmp_param_ptr->param_info.str_ptr = atc_convert_str_param();
    tmp_param_ptr->next = NULL;

    if ( NULL == tmp_param_ptr->param_info.str_ptr )
    {
        *param_ptr_ptr = NULL;

        return ERR_MEMORY_FULL;
    }

    *param_ptr_ptr = tmp_param_ptr;

    return status;
}

/******************************************************************************/
// Description : This function takes an dial string value and creates a new 
//               ATC_CUR_PARAM_INFO_LIST_T struct to include the value.
// Global resource dependence : None
// Author : Steven.Yao
// Note : This function is called when grammar parser does the following induce:
//        DIALSTRING --> Param
/******************************************************************************/
LOCAL ATC_STATUS ATCY_NewDialStrParam( // Return S_ATC_SUCCESS if success,
                                     // otherwise return error code.
        ATC_CUR_PARAM_INFO_LIST_T ** param_ptr_ptr // OUT argument, return a
                                                   // param info list struct.
        )
{
    ATC_STATUS  status = S_ATC_SUCCESS;

    ATC_CUR_PARAM_INFO_LIST_T * tmp_param_ptr = NULL;

    SCI_ASSERT( NULL != param_ptr_ptr );/*assert verified: check null pointer*/

    status = ATCM_Alloc( (void **)&tmp_param_ptr, sizeof( ATC_CUR_PARAM_INFO_LIST_T ) );
    if ( S_ATC_SUCCESS != status )
    {
        *param_ptr_ptr = NULL;

        return status;
    }

    SCI_ASSERT( NULL != tmp_param_ptr );/*assert verified: check null pointer*/
    tmp_param_ptr->param_type = ATC_CMD_PARAM_TYPE_STRING;
    tmp_param_ptr->param_info.str_ptr = atc_convert_dialstr_param();
    tmp_param_ptr->next = NULL;

    if ( NULL == tmp_param_ptr->param_info.str_ptr )
    {
        *param_ptr_ptr = NULL;

        return ERR_MEMORY_FULL;
    }

    *param_ptr_ptr = tmp_param_ptr;

    return status;
}

/******************************************************************************/
// Description : This function creates a new ATC_CUR_PARAM_INFO_LIST_T struct 
//               whose parameter type is default.
// Global resource dependence : None
// Author : Steven.Yao
// Note : This function is called when grammar parser does the following induce:
//               ------> ExtParam 
/******************************************************************************/
LOCAL ATC_STATUS ATCY_NewDefaultParam( // Return S_ATC_SUCCESS if success,
                                         // otherwise return error code.
        ATC_CUR_PARAM_INFO_LIST_T ** param_ptr_ptr  // OUT argument, return a
                                                    // param info list struct.
        )
{
    ATC_STATUS  status = S_ATC_SUCCESS;

    ATC_CUR_PARAM_INFO_LIST_T * tmp_param_ptr = NULL;

    SCI_ASSERT( NULL != param_ptr_ptr );/*assert verified: check null pointer*/

    status = ATCM_Alloc( (void **)&tmp_param_ptr, sizeof( ATC_CUR_PARAM_INFO_LIST_T ) );
    if ( S_ATC_SUCCESS != status )
    {
        return status;
    }

    SCI_ASSERT( NULL != tmp_param_ptr );/*assert verified: check null pointer*/
    tmp_param_ptr->param_type = ATC_CMD_PARAM_TYPE_DEFAULT;
    tmp_param_ptr->param_info.str_ptr = NULL;
    tmp_param_ptr->next = NULL;

    *param_ptr_ptr = tmp_param_ptr;

    return status;
}

/******************************************************************************/
// Description : This function links two parameter info list into one.
// Global resource dependence : None
// Author : Steven.Yao
// Note : This function is called when grammar parser does the following induce:
//        ExtParams ',' ExtParam ------> ExtParams 
/******************************************************************************/
LOCAL ATC_CUR_PARAM_INFO_LIST_T * ATCY_LinkTwoParams(// return the linked list
        ATC_CUR_PARAM_INFO_LIST_T * param_first_ptr,
        ATC_CUR_PARAM_INFO_LIST_T * param_last_ptr
        )
{
    ATC_CUR_PARAM_INFO_LIST_T * param_ptr = NULL;

    SCI_ASSERT( NULL != param_first_ptr );/*assert verified: check null pointer*/
    SCI_ASSERT( NULL != param_last_ptr );/*assert verified: check null pointer*/

    param_ptr = param_first_ptr;
    
    /* Find the last param of param_ptr */
    while ( NULL != param_ptr->next )
    {
        param_ptr = param_ptr->next;
    }

    param_ptr->next = param_last_ptr;

    return param_first_ptr;
}

/******************************************************************************/
// Description : This function creates a new AT cmd info list struct according
//               to the input info.
// Global resource dependence : 
// Author : Steven.Yao
// Note : This function is called when grammar parser does the following induce:
//        AT_BASIC_CMD              ------> AT_basic_cmd 
//        AT_BASIC_CMD Param        ------> AT_basic_cmd 
//        AT_BASIC_CMD ?            ------> AT_basic_cmd 
//        AT_BASIC_CMD = ?          ------> AT_basic_cmd 
//        AT_BASIC_CMD = Param      ------> AT_basic_cmd 
//        AT_EXTEND_CMD             ------> AT_ext_cmd
//        AT_EXTEND_CMD ?           ------> AT_ext_cmd
//        AT_EXTEND_CMD = ?         ------> AT_ext_cmd
//        AT_EXTEND_CMD = ExtParams ------> AT_ext_cmd
/******************************************************************************/
LOCAL ATC_STATUS ATCY_NewCommand( // Return S_ATC_SUCCESS if success,
                                    // otherwise return error code
        ATC_CUR_INFO_LIST_T ** info_ptr_ptr, // OUT argument
        uint    cmd_type,           // AT command type
        uint    cmd_index,          // AT command index
        ATC_CUR_PARAM_INFO_LIST_T * param_list_ptr // AT command param info
        )
{
    ATC_STATUS  status = S_ATC_SUCCESS;
    
    uint    i;

    ATC_CUR_INFO_LIST_T * tmp_info_ptr = NULL;

    SCI_ASSERT( NULL != info_ptr_ptr );/*assert verified: check null pointer*/
    
    status = ATCM_Alloc( (void **)&tmp_info_ptr, sizeof(ATC_CUR_INFO_LIST_T) );
    if ( S_ATC_SUCCESS != status )
    {
        return status;
    }

    SCI_ASSERT( NULL != tmp_info_ptr );/*assert verified: check null pointer*/
    tmp_info_ptr->cmd_type = cmd_type;
    tmp_info_ptr->cmd_index = cmd_index;
    tmp_info_ptr->cur_param_list_ptr = param_list_ptr;
    tmp_info_ptr->next = NULL;
    tmp_info_ptr->cur_param_num = 0;
    
    for( i = 0; i < ATC_MAX_PARAM_NUM; i++)
    {
        tmp_info_ptr->param_used[i] = FALSE;
    }
    
    while ( NULL != param_list_ptr )
    {
        if ( ATC_CMD_PARAM_TYPE_DEFAULT != param_list_ptr->param_type )
        {
            tmp_info_ptr->param_used[tmp_info_ptr->cur_param_num] = TRUE;
        }
        
        tmp_info_ptr->cur_param_num++;

        param_list_ptr = param_list_ptr->next;
    }

    *info_ptr_ptr = tmp_info_ptr;

    return status;
}

/******************************************************************************/
// Description : This function links two cmd info list into one.
// Global resource dependence : 
// Author : Steven.Yao
// Note : This function is called when grammar parser does the following induce:
//        AT_basic_cmd AT_ext_cmd               ------> AT_compound_cmd
//        AT_cmds AT_CMD_DIVIDE AT_compound_cmd ------> AT_cmds
//        AT_cmds AT_CMD_DIVIDE AT_basic_cmd    ------> AT_cmds
//        AT_cmds AT_CMD_DIVIDE AT_ext_cmd      ------> AT_cmds
/******************************************************************************/
LOCAL ATC_CUR_INFO_LIST_T * ATCY_LinkTwoCmdInfo(// return the linked list
        ATC_CUR_INFO_LIST_T * info_list_first_ptr,
        ATC_CUR_INFO_LIST_T * info_list_last_ptr
        )
{
    ATC_CUR_INFO_LIST_T *   info_list_ptr = NULL;

    SCI_ASSERT( NULL != info_list_first_ptr );/*assert verified: check null pointer*/
    SCI_ASSERT( NULL != info_list_last_ptr );/*assert verified: check null pointer*/

    info_list_ptr = info_list_first_ptr;

    /* Find the last cmd info in info_list_ptr */
    while ( NULL != info_list_ptr->next )
    {
        info_list_ptr = info_list_ptr->next;
    }

    info_list_ptr->next = info_list_last_ptr;

    return info_list_first_ptr;
}

LOCAL ATC_STATUS ATCY_ParseError(// @Steven
        ATC_CUR_INFO_LIST_T * info_ptr
        )
{
    //@Shijun.cui 2005-04-05 do not do anything here CR17325
    /*ATC_CUR_INFO_LIST_T *   error_info_ptr= NULL;
    
    if ( NULL == info_ptr )
    {
        return ERR_OPERATION_NOT_SUPPORTED;
    }
    
    parse_status = ATCY_NewCommand(
                        &error_info_ptr,
                        ATC_CMD_TYPE_ERROR,
                        0,
                        NULL);
                        
    if ( S_ATC_SUCCESS != parse_status )
    {
        ATCY_ErrorHandle( parse_status );
        return parse_status;
    }
    
    ATCY_LinkTwoCmdInfo( info_ptr, error_info_ptr);
    
    return S_ATC_SUCCESS;*/
    return ERR_OPERATION_NOT_SUPPORTED;
}
/*lint +e18*/ 
/*lint +e64*/ 
/*lint +e529*/ 
/*lint +e766*/ 
