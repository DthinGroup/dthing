/*******************************************************************************
 ** File Name:     atc_def.h                                                   *
 ** Author:        Steven.Yao                                                  *
 ** DATE:          04/11/2002                                                  *
 ** Copyright:     2001 Spreadtrum, Incoporated. All Rights Reserved.          *
 ** Description:   This file defines the AT commands, including basic commands *
 **                a extend commands.                                          *
 *******************************************************************************
                                                                               *
 *******************************************************************************
 **                         Edit History                                       *
 **----------------------------------------------------------------------------*
 ** DATE           NAME             DESCRIPTION                                *
 ** 04/11/2002     Steven.Yao       Create.                                    *
 ******************************************************************************/
#ifndef _ATC_DEF_H
#define _ATC_DEF_H

/* Define AT command index */

#define AT_CMD_AT       0
#define AT_CMD_SHARP_SHARP       1
#define AT_CMD_D       2
#define AT_CMD_Q       3
#define AT_CMD_V       4
#define AT_CMD_A       5
#define AT_CMD_H       6
#define AT_CMD_Z       7
#define AT_CMD_E       8
#define AT_CMD_and_F       9
#define AT_CMD_and_D      10
#define AT_CMD_and_C      11
#define AT_CMD_and_W      12
#define AT_CMD_W      13
#define AT_CMD_and_K      14
#define AT_CMD_S95      15
#define AT_CMD_THREE_PERCENT      16
#define AT_CMD_O      17
#define AT_CMD_CPAS      18
#define AT_CMD_CPIN      19
#define AT_CMD_CBC      20
#define AT_CMD_CLIP      21
#define AT_CMD_CLIR      22
#define AT_CMD_CCFC      23
#define AT_CMD_CCWA      24
#define AT_CMD_CREG      25
#define AT_CMD_CRC      26
#define AT_CMD_VTS      27
#define AT_CMD_CHLD      28
#define AT_CMD_CLCC      29
#define AT_CMD_COPS      30
#define AT_CMD_CSSN      31
#define AT_CMD_SIND      32
#define AT_CMD_SBCM      33
#define AT_CMD_CSQ      34
#define AT_CMD_CIMI      35
#define AT_CMD_CGMI      36
#define AT_CMD_CGMM      37
#define AT_CMD_CGSN      38
#define AT_CMD_CGMR      39
#define AT_CMD_CMEE      40
#define AT_CMD_CSCS      41
#define AT_CMD_SMUX      42
#define AT_CMD_CPOF      43
#define AT_CMD_S0      44
#define AT_CMD_S2      45
#define AT_CMD_S3      46
#define AT_CMD_S4      47
#define AT_CMD_S5      48
#define AT_CMD_S6      49
#define AT_CMD_S7      50
#define AT_CMD_S8      51
#define AT_CMD_S10      52
#define AT_CMD_I1      53
#define AT_CMD_I2      54
#define AT_CMD_I3      55
#define AT_CMD_I4      56
#define AT_CMD_I5      57
#define AT_CMD_I6      58
#define AT_CMD_I7      59
#define AT_CMD_CGDCONT      60
#define AT_CMD_CGPCO      61
#define AT_CMD_CGQREQ      62
#define AT_CMD_CGQMIN      63
#define AT_CMD_CGATT      64
#define AT_CMD_CGACT      65
#define AT_CMD_CGPADDR      66
#define AT_CMD_CGDATA      67
#define AT_CMD_CGAUTO      68
#define AT_CMD_CGANS      69
#define AT_CMD_CGCLASS      70
#define AT_CMD_CGEREP      71
#define AT_CMD_CGREG      72
#define AT_CMD_CGSMS      73
#define AT_CMD_CSMS      74
#define AT_CMD_CSDH      75
#define AT_CMD_CRES      76
#define AT_CMD_CPMS      77
#define AT_CMD_CMGF      78
#define AT_CMD_CMGR      79
#define AT_CMD_CMGL      80
#define AT_CMD_CMGD      81
#define AT_CMD_CMGS      82
#define AT_CMD_CMSS      83
#define AT_CMD_CSMP      84
#define AT_CMD_CMGC      85
#define AT_CMD_CMGW      86
#define AT_CMD_CSCA      87
#define AT_CMD_CSAS      88
#define AT_CMD_SMSC      89
#define AT_CMD_SUSS      90
#define AT_CMD_CSCB      91
#define AT_CMD_CNMI      92
#define AT_CMD_CPBR      93
#define AT_CMD_CPBF      94
#define AT_CMD_CPBW      95
#define AT_CMD_CPBP      96
#define AT_CMD_CPBN      97
#define AT_CMD_CNUM      98
#define AT_CMD_SSMP      99
#define AT_CMD_CSVM     100
#define AT_CMD_CPBS     101
#define AT_CMD_COLP     102
#define AT_CMD_COLR     103
#define AT_CMD_CPUC     104
#define AT_CMD_CUSD     105
#define AT_CMD_CAOC     106
#define AT_CMD_CACM     107
#define AT_CMD_STSF     108
#define AT_CMD_STIN     109
#define AT_CMD_STGI     110
#define AT_CMD_STGR     111
#define AT_CMD_CAMM     112
#define AT_CMD_CCLK     113
#define AT_CMD_XX     114
#define AT_CMD_VTD     115
#define AT_CMD_CCID     116
#define AT_CMD_VGR     117
#define AT_CMD_CRSL     118
#define AT_CMD_CMUT     119
#define AT_CMD_VGT     120
#define AT_CMD_SPEAKER     121
#define AT_CMD_SEQT     122
#define AT_CMD_SSEA     123
#define AT_CMD_SBAND     124
#define AT_CMD_SDTMF     125
#define AT_CMD_ECHO     126
#define AT_CMD_CLCK     127
#define AT_CMD_CPWD     128
#define AT_CMD_SSAM     129
#define AT_CMD_SSAP     130
#define AT_CMD_SADM     131
#define AT_CMD_SPCM     132
#define AT_CMD_SL1MON     133
#define AT_CMD_SADC     134
#define AT_CMD_SFUN     135
#define AT_CMD_CICB     136
#define AT_CMD_CFUN     137
#define AT_CMD_IPR     138
#define AT_CMD_CIND     139
#define AT_CMD_CCED     140
#define AT_CMD_STONE     141
#define AT_CMD_SAC     142
#define AT_CMD_ASSERT     143
#define AT_CMD_SSST     144
#define AT_CMD_SATT     145
#define AT_CMD_S32K     146
#define AT_CMD_CEER     147
#define AT_CMD_CR     148
#define AT_CMD_SAUTOATT     149
#define AT_CMD_SGPRSDATA     150
#define AT_CMD_SDCP     151
#define AT_CMD_SRWD     152
#define AT_CMD_SNVM     153
#define AT_CMD_SBRA     154
#define AT_CMD_SDMUT     155
#define AT_CMD_SLOG     156
#define AT_CMD_SUPS     157
#define AT_CMD_SSGF     158
#define AT_CMD_SSGS     159
#define AT_CMD_CPLS     160
#define AT_CMD_CPOL     161
#define AT_CMD_SPPSRATE     162
#define AT_CMD_xor_SYSINFO     163
#define AT_CMD_CBST     164
#define AT_CMD_CMOD     165
#define AT_CMD_DL     166
#define AT_CMD_CHUP     167
#define AT_CMD_CGDSCONT     168
#define AT_CMD_CGEQREQ     169
#define AT_CMD_CGEQMIN     170
#define AT_CMD_CGTFT     171
#define AT_CMD_SPCOMDEBUG     172
#define AT_CMD_CMUX     173
#define AT_CMD_xor_SPN     174
#define AT_CMD_CSTA     175
#define AT_CMD_xor_SCPBR     176
#define AT_CMD_xor_SCPBW     177
#define AT_CMD_ARMLOG     178
#define AT_CMD_UDISK     179
#define AT_CMD_CRSM     180
#define AT_CMD_CNMA     181
#define AT_CMD_STED     182
#define AT_CMD_CHUPVT     183
#define AT_CMD_TRACE     184
#define AT_CMD_OFF     185
#define AT_CMD_RESET     186
#define AT_CMD_CGEQNEG     187
#define AT_CMD_SPAUTO     188
#define AT_CMD_ECMMB     189
#define AT_CMD_ECMMBSTS     190
#define AT_CMD_ECMMBEER     191
#define AT_CMD_EUICC     192
#define AT_CMD_SPUSATCAPREQ     193
#define AT_CMD_ESQOPT     194
#define AT_CMD_CMMS     195
#define AT_CMD_ECPIN2     196
#define AT_CMD_ECIND     197
#define AT_CMD_EVTS     198
#define AT_CMD_CCWE     199
#define AT_CMD_SPUSATENVECMD     200
#define AT_CMD_SPUSATPROFILE     201
#define AT_CMD_SPUSATTERMINAL     202
#define AT_CMD_SPUSATCALLSETUP     203
#define AT_CMD_CLVL     204
#define AT_CMD_CMER     205
#define AT_CMD_STAR_USBCT     206
#define AT_CMD_star_PSNTRG     207
#define AT_CMD_GMM     208
#define AT_CMD_FCLASS     209
#define AT_CMD_GCI     210
#define AT_CMD_SHARP_CLS     211
#define AT_CMD_xor_DSCI     212
#define AT_CMD_PSRABR     213
#define AT_CMD_ERGA     214
#define AT_CMD_ERTCA     215
#define AT_CMD_M     216
#define AT_CMD_SPDUMMYKPD     217
#define AT_CMD_SPDUMMYKPDRSP     218
#define AT_CMD_SPCEER     219
#define AT_CMD_SPSMSFULL     220
#define AT_CMD_SPCHIPTYPE     221
#define AT_CMD_SPEQPARA     222
#define AT_CMD_SPREF     223
#define AT_CMD_NVOPERATE     224
#define AT_CMD_SCPFRE     225
#define AT_CMD_SPSTPARAM     226
#define AT_CMD_SPSTACTION     227
#define AT_CMD_SIPCONFIG     228
#define AT_CMD_SSYS     229
#define AT_CMD_xor_MBCELLID     230
#define AT_CMD_xor_MBAU     231
#define AT_CMD_SPHAR     232
#define AT_CMD_CTZR     233
#define AT_CMD_SCMUX     234
#define AT_CMD_SMMSWAP     235
#define AT_CMD_SPSETSAVESMS     236
#define AT_CMD_SGFD     237
#define AT_CMD_SGSIMG     238
#define AT_CMD_SGMR     239
#define AT_CMD_SDRMOD     240
#define AT_CMD_SFPL     241
#define AT_CMD_SEPL     242
#define AT_CMD_SPGSMFRQ     243
#define AT_CMD_SCDSLEEP     244
#define AT_CMD_SPENHA     245
#define AT_CMD_xor_CURC     246
#define AT_CMD_SCCBS     247
#define AT_CMD_SSRVT     248
#define AT_CMD_SSCR     249
#define AT_CMD_SLOOPTEST     250
#define AT_CMD_SUSB     251
#define AT_CMD_STPTEST     252
#define AT_CMD_SSWAPCOM     253
#define AT_CMD_X     254
#define AT_CMD_SASYNC     255
#define AT_CMD_SPTEST     256
#define AT_CMD_SPUPLMN     257
#define AT_CMD_xor_CARDMODE     258
#define AT_CMD_xor_SYSCONFIG     259
#define AT_CMD_SPDSP     260
#define AT_CMD_SPFRQ     261
#define AT_CMD_SPID     262
#define AT_CMD_SPDIAG     263
#define AT_CMD_SPBTTEST     264
#define AT_CMD_SADMDSP     265
#define AT_CMD_SPADLVVE     266
#define AT_CMD_SPAUDVOLTYPE     267
#define AT_CMD_SGETIQ     268
#define AT_CMD_NETAPN     269
#define AT_CMD_SOCKET     270
#define AT_CMD_IPOPEN     271
#define AT_CMD_IPSEND     272
#define AT_CMD_IPCLOSE     273
#define AT_CMD_IPDEACT     274
#define AT_CMD_SFSTEST     275
#define AT_CMD_AMS     276
#define AT_CMD_AMSOTA     277
#define AT_CMD_AMSINSTALL     278
#define AT_CMD_AMSDL     279
#define AT_CMD_AMSDELETE     280
#define AT_CMD_AMSDELETEALL     281
#define AT_CMD_AMSRUN     282
#define AT_CMD_AMSLIST     283
#define AT_CMD_AMSDESTROY     284
#define AT_CMD_AMSSTATUS     285
#define AT_CMD_AMSRESET     286
#define AT_CMD_AMSCFGINIT     287
#define AT_CMD_AMSCFGURL     288
#define AT_CMD_AMSCFGACCOUNT     289
#define AT_CMD_AMSCFGCANCEL     290
#define AT_CMD_AMSCFGCANCELALL     291

#endif /* End of #ifndef _ATC_DEF_H */
