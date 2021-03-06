// Generated by astgen // -*- mode: C++; c-file-style: "cc-mode" -*-
    enum en {
	atActive,
	atAdd,
	atAddD,
	atAlways,
	atAlwaysPost,
	atAlwaysPublic,
	atAnd,
	atArg,
	atArraySel,
	atAssign,
	atAssignAlias,
	atAssignDly,
	atAssignPost,
	atAssignPre,
	atAssignVarScope,
	atAssignW,
	atAttrOf,
	atBasicDType,
	atBegin,
	atBind,
	atBitsToRealD,
	atBreak,
	atBufIf1,
	atCCall,
	atCCast,
	atCFile,
	atCFunc,
	atCLog2,
	atCMath,
	atCReset,
	atCReturn,
	atCStmt,
	atCase,
	atCaseItem,
	atCast,
	atCastParse,
	atCastSize,
	atCeilD,
	atCell,
	atCellArrayRef,
	atCellInline,
	atCellRef,
	atChangeDet,
	atChangeXor,
	atClocking,
	atComment,
	atConcat,
	atConcatN,
	atCond,
	atCondBound,
	atConst,
	atConstDType,
	atContinue,
	atCountOnes,
	atCoverDecl,
	atCoverInc,
	atCoverToggle,
	atCvtPackString,
	atDefImplicitDType,
	atDefParam,
	atDisable,
	atDisplay,
	atDiv,
	atDivD,
	atDivS,
	atDot,
	atDpiExport,
	atEnumDType,
	atEnumItem,
	atEnumItemRef,
	atEq,
	atEqCase,
	atEqD,
	atEqN,
	atEqWild,
	atExpD,
	atExtend,
	atExtendS,
	atFClose,
	atFEof,
	atFFlush,
	atFGetC,
	atFGetS,
	atFOpen,
	atFScanF,
	atFinal,
	atFinish,
	atFloorD,
	atForeach,
	atFunc,
	atFuncRef,
	atGatePin,
	atGenCase,
	atGenFor,
	atGenIf,
	atGenerate,
	atGt,
	atGtD,
	atGtN,
	atGtS,
	atGte,
	atGteD,
	atGteN,
	atGteS,
	atIToRD,
	atIf,
	atIface,
	atIfaceRefDType,
	atImplicit,
	atInitArray,
	atInitial,
	atInside,
	atInsideRange,
	atIsUnknown,
	atJumpGo,
	atJumpLabel,
	atLenN,
	atLog10D,
	atLogAnd,
	atLogD,
	atLogIf,
	atLogIff,
	atLogNot,
	atLogOr,
	atLt,
	atLtD,
	atLtN,
	atLtS,
	atLte,
	atLteD,
	atLteN,
	atLteS,
	atMemberDType,
	atMemberSel,
	atMethodSel,
	atModDiv,
	atModDivS,
	atModport,
	atModportFTaskRef,
	atModportVarRef,
	atModule,
	atMul,
	atMulD,
	atMulS,
	atNegate,
	atNegateD,
	atNeq,
	atNeqCase,
	atNeqD,
	atNeqN,
	atNeqWild,
	atNetlist,
	atNot,
	atNotFoundModule,
	atOneHot,
	atOneHot0,
	atOr,
	atPackArrayDType,
	atPackage,
	atPackageExport,
	atPackageExportStarStar,
	atPackageImport,
	atPackageRef,
	atParamTypeDType,
	atParseRef,
	atParseTypeDType,
	atPatMember,
	atPattern,
	atPin,
	atPort,
	atPow,
	atPowD,
	atPowSS,
	atPowSU,
	atPowUS,
	atPragma,
	atPrimitive,
	atPslClocked,
	atPslCover,
	atPull,
	atRToIRoundS,
	atRToIS,
	atRand,
	atRange,
	atReadMem,
	atRealToBits,
	atRedAnd,
	atRedOr,
	atRedXnor,
	atRedXor,
	atRefDType,
	atRepeat,
	atReplicate,
	atReplicateN,
	atReturn,
	atSFormat,
	atSFormatF,
	atSScanF,
	atScCtor,
	atScDtor,
	atScHdr,
	atScImp,
	atScImpHdr,
	atScInt,
	atScope,
	atScopeName,
	atSel,
	atSelBit,
	atSelExtract,
	atSelMinus,
	atSelPlus,
	atSenGate,
	atSenItem,
	atSenTree,
	atShiftL,
	atShiftR,
	atShiftRS,
	atSigned,
	atSliceSel,
	atSqrtD,
	atStop,
	atStreamL,
	atStreamR,
	atStructDType,
	atSub,
	atSubD,
	atSysIgnore,
	atSystemF,
	atSystemT,
	atTask,
	atTaskRef,
	atTestPlusArgs,
	atText,
	atTime,
	atTimeD,
	atTopScope,
	atTraceDecl,
	atTraceInc,
	atTypeTable,
	atTypedef,
	atTypedefFwd,
	atUCFunc,
	atUCStmt,
	atUdpTable,
	atUdpTableLine,
	atUnionDType,
	atUnlinkedRef,
	atUnpackArrayDType,
	atUnsigned,
	atUnsizedArrayDType,
	atUnsizedRange,
	atUntilStable,
	atVAssert,
	atValuePlusArgs,
	atVar,
	atVarRef,
	atVarScope,
	atVarXRef,
	atWhile,
	atWordSel,
	atXnor,
	atXor,
	_ENUM_END
    };
    const char* ascii() const {
	const char* const names[] = {
	"ACTIVE",
	"ADD",
	"ADDD",
	"ALWAYS",
	"ALWAYSPOST",
	"ALWAYSPUBLIC",
	"AND",
	"ARG",
	"ARRAYSEL",
	"ASSIGN",
	"ASSIGNALIAS",
	"ASSIGNDLY",
	"ASSIGNPOST",
	"ASSIGNPRE",
	"ASSIGNVARSCOPE",
	"ASSIGNW",
	"ATTROF",
	"BASICDTYPE",
	"BEGIN",
	"BIND",
	"BITSTOREALD",
	"BREAK",
	"BUFIF1",
	"CCALL",
	"CCAST",
	"CFILE",
	"CFUNC",
	"CLOG2",
	"CMATH",
	"CRESET",
	"CRETURN",
	"CSTMT",
	"CASE",
	"CASEITEM",
	"CAST",
	"CASTPARSE",
	"CASTSIZE",
	"CEILD",
	"CELL",
	"CELLARRAYREF",
	"CELLINLINE",
	"CELLREF",
	"CHANGEDET",
	"CHANGEXOR",
	"CLOCKING",
	"COMMENT",
	"CONCAT",
	"CONCATN",
	"COND",
	"CONDBOUND",
	"CONST",
	"CONSTDTYPE",
	"CONTINUE",
	"COUNTONES",
	"COVERDECL",
	"COVERINC",
	"COVERTOGGLE",
	"CVTPACKSTRING",
	"DEFIMPLICITDTYPE",
	"DEFPARAM",
	"DISABLE",
	"DISPLAY",
	"DIV",
	"DIVD",
	"DIVS",
	"DOT",
	"DPIEXPORT",
	"ENUMDTYPE",
	"ENUMITEM",
	"ENUMITEMREF",
	"EQ",
	"EQCASE",
	"EQD",
	"EQN",
	"EQWILD",
	"EXPD",
	"EXTEND",
	"EXTENDS",
	"FCLOSE",
	"FEOF",
	"FFLUSH",
	"FGETC",
	"FGETS",
	"FOPEN",
	"FSCANF",
	"FINAL",
	"FINISH",
	"FLOORD",
	"FOREACH",
	"FUNC",
	"FUNCREF",
	"GATEPIN",
	"GENCASE",
	"GENFOR",
	"GENIF",
	"GENERATE",
	"GT",
	"GTD",
	"GTN",
	"GTS",
	"GTE",
	"GTED",
	"GTEN",
	"GTES",
	"ITORD",
	"IF",
	"IFACE",
	"IFACEREFDTYPE",
	"IMPLICIT",
	"INITARRAY",
	"INITIAL",
	"INSIDE",
	"INSIDERANGE",
	"ISUNKNOWN",
	"JUMPGO",
	"JUMPLABEL",
	"LENN",
	"LOG10D",
	"LOGAND",
	"LOGD",
	"LOGIF",
	"LOGIFF",
	"LOGNOT",
	"LOGOR",
	"LT",
	"LTD",
	"LTN",
	"LTS",
	"LTE",
	"LTED",
	"LTEN",
	"LTES",
	"MEMBERDTYPE",
	"MEMBERSEL",
	"METHODSEL",
	"MODDIV",
	"MODDIVS",
	"MODPORT",
	"MODPORTFTASKREF",
	"MODPORTVARREF",
	"MODULE",
	"MUL",
	"MULD",
	"MULS",
	"NEGATE",
	"NEGATED",
	"NEQ",
	"NEQCASE",
	"NEQD",
	"NEQN",
	"NEQWILD",
	"NETLIST",
	"NOT",
	"NOTFOUNDMODULE",
	"ONEHOT",
	"ONEHOT0",
	"OR",
	"PACKARRAYDTYPE",
	"PACKAGE",
	"PACKAGEEXPORT",
	"PACKAGEEXPORTSTARSTAR",
	"PACKAGEIMPORT",
	"PACKAGEREF",
	"PARAMTYPEDTYPE",
	"PARSEREF",
	"PARSETYPEDTYPE",
	"PATMEMBER",
	"PATTERN",
	"PIN",
	"PORT",
	"POW",
	"POWD",
	"POWSS",
	"POWSU",
	"POWUS",
	"PRAGMA",
	"PRIMITIVE",
	"PSLCLOCKED",
	"PSLCOVER",
	"PULL",
	"RTOIROUNDS",
	"RTOIS",
	"RAND",
	"RANGE",
	"READMEM",
	"REALTOBITS",
	"REDAND",
	"REDOR",
	"REDXNOR",
	"REDXOR",
	"REFDTYPE",
	"REPEAT",
	"REPLICATE",
	"REPLICATEN",
	"RETURN",
	"SFORMAT",
	"SFORMATF",
	"SSCANF",
	"SCCTOR",
	"SCDTOR",
	"SCHDR",
	"SCIMP",
	"SCIMPHDR",
	"SCINT",
	"SCOPE",
	"SCOPENAME",
	"SEL",
	"SELBIT",
	"SELEXTRACT",
	"SELMINUS",
	"SELPLUS",
	"SENGATE",
	"SENITEM",
	"SENTREE",
	"SHIFTL",
	"SHIFTR",
	"SHIFTRS",
	"SIGNED",
	"SLICESEL",
	"SQRTD",
	"STOP",
	"STREAML",
	"STREAMR",
	"STRUCTDTYPE",
	"SUB",
	"SUBD",
	"SYSIGNORE",
	"SYSTEMF",
	"SYSTEMT",
	"TASK",
	"TASKREF",
	"TESTPLUSARGS",
	"TEXT",
	"TIME",
	"TIMED",
	"TOPSCOPE",
	"TRACEDECL",
	"TRACEINC",
	"TYPETABLE",
	"TYPEDEF",
	"TYPEDEFFWD",
	"UCFUNC",
	"UCSTMT",
	"UDPTABLE",
	"UDPTABLELINE",
	"UNIONDTYPE",
	"UNLINKEDREF",
	"UNPACKARRAYDTYPE",
	"UNSIGNED",
	"UNSIZEDARRAYDTYPE",
	"UNSIZEDRANGE",
	"UNTILSTABLE",
	"VASSERT",
	"VALUEPLUSARGS",
	"VAR",
	"VARREF",
	"VARSCOPE",
	"VARXREF",
	"WHILE",
	"WORDSEL",
	"XNOR",
	"XOR",
	"_ENUM_END"
	};
	return names[m_e];
    };
