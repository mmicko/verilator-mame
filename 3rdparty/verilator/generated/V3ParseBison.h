/* A Bison parser, made by GNU Bison 3.0.4.  */

/* Bison interface for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015 Free Software Foundation, Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

#ifndef YY_YY_V3PARSEBISON_PRETMP_H_INCLUDED
# define YY_YY_V3PARSEBISON_PRETMP_H_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int yydebug;
#endif

/* Token type.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    yaFLOATNUM = 258,
    yaID__ETC = 259,
    yaID__LEX = 260,
    yaID__aPACKAGE = 261,
    yaID__aTYPE = 262,
    yaINTNUM = 263,
    yaTIMENUM = 264,
    yaSTRING = 265,
    yaSTRING__IGNORE = 266,
    yaTIMINGSPEC = 267,
    yaTABLELINE = 268,
    yaSCHDR = 269,
    yaSCINT = 270,
    yaSCIMP = 271,
    yaSCIMPH = 272,
    yaSCCTOR = 273,
    yaSCDTOR = 274,
    yVLT_COVERAGE_OFF = 275,
    yVLT_COVERAGE_ON = 276,
    yVLT_LINT_OFF = 277,
    yVLT_LINT_ON = 278,
    yVLT_TRACING_OFF = 279,
    yVLT_TRACING_ON = 280,
    yVLT_D_FILE = 281,
    yVLT_D_LINES = 282,
    yVLT_D_MSG = 283,
    yaD_IGNORE = 284,
    yaD_DPI = 285,
    yALWAYS = 286,
    yALWAYS_FF = 287,
    yALWAYS_COMB = 288,
    yALWAYS_LATCH = 289,
    yAND = 290,
    yASSERT = 291,
    yASSIGN = 292,
    yAUTOMATIC = 293,
    yBEGIN = 294,
    yBIND = 295,
    yBIT = 296,
    yBREAK = 297,
    yBUF = 298,
    yBUFIF0 = 299,
    yBUFIF1 = 300,
    yBYTE = 301,
    yCASE = 302,
    yCASEX = 303,
    yCASEZ = 304,
    yCHANDLE = 305,
    yCLOCKING = 306,
    yCONST__ETC = 307,
    yCONST__LEX = 308,
    yCMOS = 309,
    yCONTEXT = 310,
    yCONTINUE = 311,
    yCOVER = 312,
    yDEASSIGN = 313,
    yDEFAULT = 314,
    yDEFPARAM = 315,
    yDISABLE = 316,
    yDO = 317,
    yEDGE = 318,
    yELSE = 319,
    yEND = 320,
    yENDCASE = 321,
    yENDCLOCKING = 322,
    yENDFUNCTION = 323,
    yENDGENERATE = 324,
    yENDINTERFACE = 325,
    yENDMODULE = 326,
    yENDPACKAGE = 327,
    yENDPRIMITIVE = 328,
    yENDPROGRAM = 329,
    yENDPROPERTY = 330,
    yENDSPECIFY = 331,
    yENDTABLE = 332,
    yENDTASK = 333,
    yENUM = 334,
    yEXPORT = 335,
    yFINAL = 336,
    yFOR = 337,
    yFOREACH = 338,
    yFOREVER = 339,
    yFUNCTION = 340,
    yGENERATE = 341,
    yGENVAR = 342,
    yGLOBAL__CLOCKING = 343,
    yGLOBAL__LEX = 344,
    yIF = 345,
    yIFF = 346,
    yIMPORT = 347,
    yINITIAL = 348,
    yINOUT = 349,
    yINPUT = 350,
    yINSIDE = 351,
    yINT = 352,
    yINTEGER = 353,
    yINTERFACE = 354,
    yLOCALPARAM = 355,
    yLOGIC = 356,
    yLONGINT = 357,
    yMODPORT = 358,
    yMODULE = 359,
    yNAND = 360,
    yNEGEDGE = 361,
    yNMOS = 362,
    yNOR = 363,
    yNOT = 364,
    yNOTIF0 = 365,
    yNOTIF1 = 366,
    yOR = 367,
    yOUTPUT = 368,
    yPACKAGE = 369,
    yPACKED = 370,
    yPARAMETER = 371,
    yPMOS = 372,
    yPOSEDGE = 373,
    yPRIMITIVE = 374,
    yPRIORITY = 375,
    yPROGRAM = 376,
    yPROPERTY = 377,
    yPULLDOWN = 378,
    yPULLUP = 379,
    yPURE = 380,
    yRAND = 381,
    yRANDC = 382,
    yRCMOS = 383,
    yREAL = 384,
    yREALTIME = 385,
    yREG = 386,
    yREPEAT = 387,
    yRETURN = 388,
    yRNMOS = 389,
    yRPMOS = 390,
    yRTRAN = 391,
    yRTRANIF0 = 392,
    yRTRANIF1 = 393,
    ySCALARED = 394,
    ySHORTINT = 395,
    ySIGNED = 396,
    ySPECIFY = 397,
    ySPECPARAM = 398,
    ySTATIC = 399,
    ySTRING = 400,
    ySTRUCT = 401,
    ySUPPLY0 = 402,
    ySUPPLY1 = 403,
    yTABLE = 404,
    yTASK = 405,
    yTIME = 406,
    yTIMEPRECISION = 407,
    yTIMEUNIT = 408,
    yTRAN = 409,
    yTRANIF0 = 410,
    yTRANIF1 = 411,
    yTRI = 412,
    yTRI0 = 413,
    yTRI1 = 414,
    yTRUE = 415,
    yTYPE = 416,
    yTYPEDEF = 417,
    yUNION = 418,
    yUNIQUE = 419,
    yUNIQUE0 = 420,
    yUNSIGNED = 421,
    yVAR = 422,
    yVECTORED = 423,
    yVOID = 424,
    yWHILE = 425,
    yWIRE = 426,
    yWREAL = 427,
    yXNOR = 428,
    yXOR = 429,
    yD_BITS = 430,
    yD_BITSTOREAL = 431,
    yD_C = 432,
    yD_CEIL = 433,
    yD_CLOG2 = 434,
    yD_COUNTONES = 435,
    yD_DIMENSIONS = 436,
    yD_DISPLAY = 437,
    yD_ERROR = 438,
    yD_EXP = 439,
    yD_FATAL = 440,
    yD_FCLOSE = 441,
    yD_FDISPLAY = 442,
    yD_FEOF = 443,
    yD_FFLUSH = 444,
    yD_FGETC = 445,
    yD_FGETS = 446,
    yD_FINISH = 447,
    yD_FLOOR = 448,
    yD_FOPEN = 449,
    yD_FSCANF = 450,
    yD_FWRITE = 451,
    yD_HIGH = 452,
    yD_INCREMENT = 453,
    yD_INFO = 454,
    yD_ISUNKNOWN = 455,
    yD_ITOR = 456,
    yD_LEFT = 457,
    yD_LN = 458,
    yD_LOG10 = 459,
    yD_LOW = 460,
    yD_ONEHOT = 461,
    yD_ONEHOT0 = 462,
    yD_POW = 463,
    yD_RANDOM = 464,
    yD_READMEMB = 465,
    yD_READMEMH = 466,
    yD_REALTIME = 467,
    yD_REALTOBITS = 468,
    yD_RIGHT = 469,
    yD_RTOI = 470,
    yD_SFORMAT = 471,
    yD_SFORMATF = 472,
    yD_SIGNED = 473,
    yD_SIZE = 474,
    yD_SQRT = 475,
    yD_SSCANF = 476,
    yD_STIME = 477,
    yD_STOP = 478,
    yD_SWRITE = 479,
    yD_SYSTEM = 480,
    yD_TESTPLUSARGS = 481,
    yD_TIME = 482,
    yD_UNIT = 483,
    yD_UNPACKED_DIMENSIONS = 484,
    yD_UNSIGNED = 485,
    yD_VALUEPLUSARGS = 486,
    yD_WARNING = 487,
    yD_WRITE = 488,
    yVL_CLOCK = 489,
    yVL_CLOCKER = 490,
    yVL_NO_CLOCKER = 491,
    yVL_CLOCK_ENABLE = 492,
    yVL_COVERAGE_BLOCK_OFF = 493,
    yVL_FULL_CASE = 494,
    yVL_INLINE_MODULE = 495,
    yVL_ISOLATE_ASSIGNMENTS = 496,
    yVL_NO_INLINE_MODULE = 497,
    yVL_NO_INLINE_TASK = 498,
    yVL_SC_BV = 499,
    yVL_SFORMAT = 500,
    yVL_PARALLEL_CASE = 501,
    yVL_PUBLIC = 502,
    yVL_PUBLIC_FLAT = 503,
    yVL_PUBLIC_FLAT_RD = 504,
    yVL_PUBLIC_FLAT_RW = 505,
    yVL_PUBLIC_MODULE = 506,
    yP_TICK = 507,
    yP_TICKBRA = 508,
    yP_OROR = 509,
    yP_ANDAND = 510,
    yP_NOR = 511,
    yP_XNOR = 512,
    yP_NAND = 513,
    yP_EQUAL = 514,
    yP_NOTEQUAL = 515,
    yP_CASEEQUAL = 516,
    yP_CASENOTEQUAL = 517,
    yP_WILDEQUAL = 518,
    yP_WILDNOTEQUAL = 519,
    yP_GTE = 520,
    yP_LTE = 521,
    yP_LTE__IGNORE = 522,
    yP_SLEFT = 523,
    yP_SRIGHT = 524,
    yP_SSRIGHT = 525,
    yP_POW = 526,
    yP_PLUSCOLON = 527,
    yP_MINUSCOLON = 528,
    yP_MINUSGT = 529,
    yP_MINUSGTGT = 530,
    yP_EQGT = 531,
    yP_ASTGT = 532,
    yP_ANDANDAND = 533,
    yP_POUNDPOUND = 534,
    yP_DOTSTAR = 535,
    yP_ATAT = 536,
    yP_COLONCOLON = 537,
    yP_COLONEQ = 538,
    yP_COLONDIV = 539,
    yP_ORMINUSGT = 540,
    yP_OREQGT = 541,
    yP_BRASTAR = 542,
    yP_BRAEQ = 543,
    yP_BRAMINUSGT = 544,
    yP_PLUSPLUS = 545,
    yP_MINUSMINUS = 546,
    yP_PLUSEQ = 547,
    yP_MINUSEQ = 548,
    yP_TIMESEQ = 549,
    yP_DIVEQ = 550,
    yP_MODEQ = 551,
    yP_ANDEQ = 552,
    yP_OREQ = 553,
    yP_XOREQ = 554,
    yP_SLEFTEQ = 555,
    yP_SRIGHTEQ = 556,
    yP_SSRIGHTEQ = 557,
    yP_LOGIFF = 558,
    prUNARYARITH = 559,
    prREDUCTION = 560,
    prNEGATION = 561,
    prLOWER_THAN_ELSE = 562
  };
#endif

/* Value type.  */


extern YYSTYPE yylval;

int yyparse (void);

#endif /* !YY_YY_V3PARSEBISON_PRETMP_H_INCLUDED  */