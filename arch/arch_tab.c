
/*  A Bison parser, made from arch.y with Bison version GNU Bison version 1.22
  */

#define YYBISON 1  /* Identify Bison output.  */

#define yyparse archparse
#define yylex archlex
#define yyerror archerror
#define yylval archlval
#define yychar archchar
#define yydebug archdebug
#define yynerrs archnerrs
#define	Ampersand	258
#define	Apostrophe	259
#define	LeftParen	260
#define	RightParen	261
#define	DoubleStar	262
#define	Star	263
#define	Plus	264
#define	Comma	265
#define	Minus	266
#define	VarAsgn	267
#define	Colon	268
#define	Semicolon	269
#define	_LESym	270
#define	Box	271
#define	_LTSym	272
#define	Arrow	273
#define	_EQSym	274
#define	_GESym	275
#define	_GTSym	276
#define	Bar	277
#define	_NESym	278
#define	Dot	279
#define	Slash	280
#define	Identifier	281
#define	DecimalInt	282
#define	DecimalReal	283
#define	AbstractLit	284
#define	BasedInt	285
#define	BasedReal	286
#define	CharacterLit	287
#define	StringLit	288
#define	BitStringLit	289
#define	ABS	290
#define	ACCESS	291
#define	AFTER	292
#define	ALIAS	293
#define	ALL	294
#define	_AND	295
#define	ARCHITECTURE	296
#define	ARRAY	297
#define	ASSERT	298
#define	ATTRIBUTE	299
#define	_BEGIN	300
#define	BIT	301
#define	BIT_VECTOR	302
#define	BLOCK	303
#define	BODY	304
#define	BUFFER	305
#define	BUS	306
#define	CASE	307
#define	COMPONENT	308
#define	CONFIGURATION	309
#define	CONSTANT	310
#define	CONVERT	311
#define	DISCONNECT	312
#define	DOWNTO	313
#define	ELSE	314
#define	ELSIF	315
#define	_END	316
#define	ENTITY	317
#define	ERROR	318
#define	_EXIT	319
#define	_FILE	320
#define	FOR	321
#define	FUNCTION	322
#define	GENERATE	323
#define	GENERIC	324
#define	GUARDED	325
#define	IF	326
#define	_INOUT	327
#define	_IN	328
#define	IS	329
#define	_LABEL	330
#define	LIBRARY	331
#define	_LINKAGE	332
#define	_LOOP	333
#define	MAP	334
#define	MOD	335
#define	MUX_BIT	336
#define	MUX_VECTOR	337
#define	_NAND	338
#define	NATURAL	339
#define	NATURAL_VECTOR	340
#define	NEW	341
#define	_NEXT	342
#define	_NOR	343
#define	_NOT	344
#define	_NULL	345
#define	OF	346
#define	ON	347
#define	OPEN	348
#define	_OR	349
#define	OTHERS	350
#define	_OUT	351
#define	PACKAGE	352
#define	PORT	353
#define	PROCEDURE	354
#define	PROCESS	355
#define	RANGE	356
#define	RECORD	357
#define	REG_BIT	358
#define	REG_VECTOR	359
#define	REGISTER	360
#define	REM	361
#define	REPORT	362
#define	RETURN	363
#define	SELECT	364
#define	SEVERITY	365
#define	SIGNAL	366
#define	_STABLE	367
#define	SUBTYPE	368
#define	THEN	369
#define	TO	370
#define	TRANSPORT	371
#define	_TYPE	372
#define	UNITS	373
#define	UNTIL	374
#define	USE	375
#define	VARIABLE	376
#define	WAIT	377
#define	WARNING	378
#define	WHEN	379
#define	WHILE	380
#define	WITH	381
#define	WOR_BIT	382
#define	WOR_VECTOR	383
#define	_XOR	384

#line 1 "arch.y"

#include <stdio.h>
#include "archInt.h"

#define MODE_IN 1
#define MODE_OUT 2
#define MODE_INOUT 3
#define MODE_LINKAGE 4

#define BIT_TYPE 1
#define WOR_BIT_TYPE 2
#define MUX_BIT_TYPE 3
#define BIT_VECTOR_TYPE 4
#define WOR_VECTOR_TYPE 5
#define	MUX_VECTOR_TYPE 6
#define NATURAL_TYPE 7
#define NATURAL_VECTOR_TYPE 8

extern arch_graph_t *main_arch;


#line 23 "arch.y"
typedef union
  {
  int         valu;
  char       *text;
  char        flag;
  char 	     *name;
  char       *expr;
/*
  mvl_name    name;
  mvl_expr    expr;
*/  
} YYSTYPE;

#ifndef YYLTYPE
typedef
  struct yyltype
    {
      int timestamp;
      int first_line;
      int first_column;
      int last_line;
      int last_column;
      char *text;
   }
  yyltype;

#define YYLTYPE yyltype
#endif

#include <stdio.h>

#ifndef __cplusplus
#ifndef __STDC__
#define const
#endif
#endif



#define	YYFINAL		208
#define	YYFLAG		-32768
#define	YYNTBASE	130

#define YYTRANSLATE(x) ((unsigned)(x) <= 384 ? yytranslate[x] : 204)

static const short yytranslate[] = {     0,
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
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     1,     2,     3,     4,     5,
     6,     7,     8,     9,    10,    11,    12,    13,    14,    15,
    16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
    26,    27,    28,    29,    30,    31,    32,    33,    34,    35,
    36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
    46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
    56,    57,    58,    59,    60,    61,    62,    63,    64,    65,
    66,    67,    68,    69,    70,    71,    72,    73,    74,    75,
    76,    77,    78,    79,    80,    81,    82,    83,    84,    85,
    86,    87,    88,    89,    90,    91,    92,    93,    94,    95,
    96,    97,    98,    99,   100,   101,   102,   103,   104,   105,
   106,   107,   108,   109,   110,   111,   112,   113,   114,   115,
   116,   117,   118,   119,   120,   121,   122,   123,   124,   125,
   126,   127,   128,   129
};

#if YYDEBUG != 0
static const short yyprhs[] = {     0,
     0,     3,     4,     5,    16,    19,    20,    22,    28,    32,
    35,    36,    40,    48,    50,    51,    53,    59,    63,    66,
    67,    71,    72,    73,    82,    84,    87,    89,    91,    96,
    97,   101,   103,   104,   106,   110,   114,   116,   118,   130,
   133,   135,   136,   139,   141,   143,   146,   154,   161,   162,
   168,   171,   172,   176,   184,   186,   188,   189,   192,   194,
   197,   198,   204,   205,   207,   213,   216,   217,   221,   225,
   227,   229,   231,   233,   235,   237,   239,   244,   247,   249,
   251,   253,   255,   259,   261,   263,   265,   267,   269,   273,
   278,   279,   281,   282,   284,   286,   288,   290,   291,   293,
   295,   297,   299,   301,   303,   305,   307,   309,   311,   313,
   315,   316,   318,   319,   323,   324,   328,   331,   332,   334,
   336,   338,   340,   342
};

static const short yyrhs[] = {   131,
   155,     0,     0,     0,    62,   198,   132,    74,   139,   134,
   203,   198,   133,   200,     0,    62,     1,     0,     0,   135,
     0,    98,     5,   136,   202,   200,     0,    98,     1,   200,
     0,   138,   137,     0,     0,   137,   200,   138,     0,   189,
   194,    13,   191,   192,   151,   193,     0,     1,     0,     0,
   140,     0,    69,     5,   141,   202,   200,     0,    69,     1,
   200,     0,   143,   142,     0,     0,   142,   200,   143,     0,
     0,     0,    55,   144,   194,   145,    13,   192,   151,   146,
     0,     1,     0,    12,   147,     0,   201,     0,   148,     0,
     5,   150,   149,   202,     0,     0,   149,    10,   150,     0,
   201,     0,     0,   152,     0,     5,   153,   202,     0,   201,
   154,   201,     0,   115,     0,    58,     0,    41,   199,    91,
   199,    74,   156,    45,   165,   203,   198,   200,     0,    41,
     1,     0,   157,     0,     0,   157,   158,     0,   159,     0,
   160,     0,     1,   200,     0,   111,   194,    13,   192,   151,
   193,   200,     0,    53,    26,   161,   203,    53,   200,     0,
     0,    98,     5,   162,   202,   200,     0,   164,   163,     0,
     0,   163,   200,   164,     0,   189,   194,    13,   190,   192,
   151,   193,     0,     1,     0,   166,     0,     0,   166,   167,
     0,   168,     0,     1,   200,     0,     0,   197,   199,   169,
   170,   200,     0,     0,   171,     0,    98,    79,     5,   172,
   202,     0,   174,   173,     0,     0,   173,    10,   174,     0,
   175,    18,   176,     0,   176,     0,     1,     0,   177,     0,
   180,     0,   199,     0,   178,     0,   179,     0,   199,     5,
   201,   202,     0,   199,   152,     0,   181,     0,   182,     0,
   183,     0,   184,     0,   183,     3,   184,     0,   185,     0,
   186,     0,   187,     0,   188,     0,   177,     0,     5,   180,
   202,     0,    56,     5,   180,   202,     0,     0,   111,     0,
     0,    73,     0,    96,     0,    72,     0,    77,     0,     0,
    73,     0,    96,     0,    72,     0,    77,     0,    46,     0,
   127,     0,    81,     0,    47,     0,   128,     0,    82,     0,
    84,     0,    85,     0,     0,    51,     0,     0,    26,   195,
   196,     0,     0,   196,    10,    26,     0,    26,    13,     0,
     0,   199,     0,    26,     0,    14,     0,    29,     0,     6,
     0,    61,     0
};

#endif

#if YYDEBUG != 0
static const short yyrline[] = { 0,
   197,   202,   205,   210,   210,   215,   216,   220,   225,   231,
   236,   237,   243,   253,   260,   261,   265,   270,   276,   281,
   282,   288,   293,   297,   300,   304,   309,   313,   320,   327,
   328,   334,   339,   343,   350,   359,   368,   372,   379,   390,
   395,   399,   400,   405,   406,   407,   412,   422,   431,   432,
   440,   445,   446,   452,   459,   463,   467,   468,   473,   474,
   482,   488,   495,   496,   500,   508,   513,   514,   520,   526,
   530,   537,   544,   551,   555,   559,   566,   576,   585,   592,
   599,   606,   610,   620,   628,   635,   639,   643,   650,   659,
   670,   671,   675,   679,   683,   687,   691,   698,   702,   706,
   710,   714,   721,   725,   729,   733,   737,   741,   745,   749,
   756,   758,   763,   768,   771,   772,   781,   787,   788,   792,
   796,   800,   807,   811
};

static const char * const yytname[] = {   "$","error","$illegal.","Ampersand",
"Apostrophe","LeftParen","RightParen","DoubleStar","Star","Plus","Comma","Minus",
"VarAsgn","Colon","Semicolon","_LESym","Box","_LTSym","Arrow","_EQSym","_GESym",
"_GTSym","Bar","_NESym","Dot","Slash","Identifier","DecimalInt","DecimalReal",
"AbstractLit","BasedInt","BasedReal","CharacterLit","StringLit","BitStringLit",
"ABS","ACCESS","AFTER","ALIAS","ALL","_AND","ARCHITECTURE","ARRAY","ASSERT",
"ATTRIBUTE","_BEGIN","BIT","BIT_VECTOR","BLOCK","BODY","BUFFER","BUS","CASE",
"COMPONENT","CONFIGURATION","CONSTANT","CONVERT","DISCONNECT","DOWNTO","ELSE",
"ELSIF","_END","ENTITY","ERROR","_EXIT","_FILE","FOR","FUNCTION","GENERATE",
"GENERIC","GUARDED","IF","_INOUT","_IN","IS","_LABEL","LIBRARY","_LINKAGE","_LOOP",
"MAP","MOD","MUX_BIT","MUX_VECTOR","_NAND","NATURAL","NATURAL_VECTOR","NEW",
"_NEXT","_NOR","_NOT","_NULL","OF","ON","OPEN","_OR","OTHERS","_OUT","PACKAGE",
"PORT","PROCEDURE","PROCESS","RANGE","RECORD","REG_BIT","REG_VECTOR","REGISTER",
"REM","REPORT","RETURN","SELECT","SEVERITY","SIGNAL","_STABLE","SUBTYPE","THEN",
"TO","TRANSPORT","_TYPE","UNITS","UNTIL","USE","VARIABLE","WAIT","WARNING","WHEN",
"WHILE","WITH","WOR_BIT","WOR_VECTOR","_XOR","design_file","entity_declaration",
"@1","@2",".port_clause.","port_clause","formal_port_list","...formal_port_element..",
"formal_port_element",".generic_clause.","generic_clause","formal_generic_list",
"...formal_generic_element..","formal_generic_element","@3","@4","generic_VarAsgn__expression",
"generic_expression","generic_aggregate","...generic_element_association..",
"generic_element_association",".constraint.","constraint","range","direction",
"architecture_body","architecture_declarative_part","..block_declaration_item..",
"block_declaration_item","signal_declaration","component_declaration",".PORT__local_port_list.",
"local_port_list","...local_port_element..","local_port_element","architecture_statement_part",
"..concurrent_statement..","concurrent_statement","component_instantiation_statement",
"@5",".port_map_aspect.","port_map_aspect","association_list","...association_element..",
"association_element","formal_port_name","actual_port_name","name","indexed_name",
"slice_name","expression","relation","simple_expression",".sign.term..add_op__term..",
"term","factor","primary","aggregate","type_convertion",".SIGNAL.",".local_port_mode.",
".mode.","type_mark",".BUS.","identifier_list","@6","...identifier..","a_label",
".simple_name.","simple_name","Semicolon_ERR","abstractlit","RightParen_ERR",
"END_ERR",""
};
#endif

static const short yyr1[] = {     0,
   130,   132,   133,   131,   131,   134,   134,   135,   135,   136,
   137,   137,   138,   138,   139,   139,   140,   140,   141,   142,
   142,   144,   145,   143,   143,   146,   147,   147,   148,   149,
   149,   150,   151,   151,   152,   153,   154,   154,   155,   155,
   156,   157,   157,   158,   158,   158,   159,   160,   161,   161,
   162,   163,   163,   164,   164,   165,   166,   166,   167,   167,
   169,   168,   170,   170,   171,   172,   173,   173,   174,   174,
   174,   175,   176,   177,   177,   177,   178,   179,   180,   181,
   182,   183,   183,   184,   185,   186,   186,   186,   187,   188,
   189,   189,   190,   190,   190,   190,   190,   191,   191,   191,
   191,   191,   192,   192,   192,   192,   192,   192,   192,   192,
   193,   193,   195,   194,   196,   196,   197,   198,   198,   199,
   200,   201,   202,   203
};

static const short yyr2[] = {     0,
     2,     0,     0,    10,     2,     0,     1,     5,     3,     2,
     0,     3,     7,     1,     0,     1,     5,     3,     2,     0,
     3,     0,     0,     8,     1,     2,     1,     1,     4,     0,
     3,     1,     0,     1,     3,     3,     1,     1,    11,     2,
     1,     0,     2,     1,     1,     2,     7,     6,     0,     5,
     2,     0,     3,     7,     1,     1,     0,     2,     1,     2,
     0,     5,     0,     1,     5,     2,     0,     3,     3,     1,
     1,     1,     1,     1,     1,     1,     4,     2,     1,     1,
     1,     1,     3,     1,     1,     1,     1,     1,     3,     4,
     0,     1,     0,     1,     1,     1,     1,     0,     1,     1,
     1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
     0,     1,     0,     3,     0,     3,     2,     0,     1,     1,
     1,     1,     1,     1
};

static const short yydefact[] = {     0,
     0,     0,     5,   120,     2,   119,     0,     1,     0,    40,
     0,    15,     0,     0,     6,    16,     0,     0,     0,     0,
     0,     7,    42,   121,    18,    25,    22,     0,    20,     0,
     0,   124,   118,     0,     0,     0,   123,     0,    19,     9,
    14,    92,     0,    11,     0,     3,    57,     0,     0,     0,
    43,    44,    45,   113,    23,    17,     0,     0,    10,     0,
     0,     0,     0,    46,    49,     0,   115,     0,    21,     8,
     0,    98,     4,   118,     0,     0,    58,    59,     0,     0,
     0,     0,   114,     0,    12,   101,    99,   102,   100,     0,
     0,    60,   117,    61,     0,     0,   103,   106,   105,   108,
   109,   110,   104,   107,    33,     0,    33,    33,    39,    63,
    55,     0,    52,     0,     0,     0,   111,    34,   116,     0,
   111,     0,     0,    64,     0,    51,     0,    48,   122,     0,
     0,   112,     0,     0,    24,    13,     0,    62,    50,     0,
    93,    35,    38,    37,     0,    47,     0,    26,    28,    27,
     0,    53,    96,    94,    97,    95,     0,    36,    30,    32,
    71,     0,     0,     0,    67,     0,    70,    88,    75,    76,
    73,    79,    80,    81,    82,    84,    85,    86,    87,    74,
    33,     0,    88,     0,     0,    65,    66,     0,     0,     0,
    78,   111,     0,    29,    89,     0,     0,    69,    83,     0,
    54,    31,    90,    68,    77,     0,     0,     0
};

static const short yydefgoto[] = {   206,
     2,     9,    61,    21,    22,    43,    59,    44,    15,    16,
    28,    39,    29,    36,    68,   135,   148,   149,   182,   159,
   117,   118,   130,   145,     8,    34,    35,    51,    52,    53,
    81,   112,   126,   113,    62,    63,    77,    78,   110,   123,
   124,   164,   187,   165,   166,   167,   183,   169,   170,   171,
   172,   173,   174,   175,   176,   177,   178,   179,    45,   157,
    90,   105,   133,    55,    67,    83,    79,     5,   180,    25,
   160,    38,    33
};

static const short yypact[] = {   -21,
     8,    44,-32768,-32768,-32768,-32768,    38,-32768,    -4,-32768,
     3,    22,    69,    47,     4,-32768,    24,    82,    29,    66,
    40,-32768,-32768,-32768,-32768,-32768,-32768,    97,-32768,    82,
     1,-32768,    69,    61,     0,    83,-32768,    82,    82,-32768,
-32768,-32768,    97,-32768,    83,-32768,-32768,    82,    84,    83,
-32768,-32768,-32768,-32768,-32768,-32768,    29,    82,    82,    95,
    82,    40,    18,-32768,    17,   103,-32768,   104,-32768,-32768,
     1,   -22,-32768,    69,    82,   107,-32768,-32768,    69,   117,
    40,   -23,   114,   -23,-32768,-32768,-32768,-32768,-32768,   -23,
    82,-32768,-32768,-32768,     2,    72,-32768,-32768,-32768,-32768,
-32768,-32768,-32768,-32768,   121,   101,   121,   121,-32768,    30,
-32768,    97,-32768,    83,    82,   100,    79,-32768,-32768,   119,
    79,    53,    82,-32768,    82,    82,   120,-32768,-32768,    97,
   -47,-32768,    82,    49,-32768,-32768,   129,-32768,-32768,     2,
    46,-32768,-32768,-32768,   100,-32768,   100,-32768,-32768,-32768,
    37,-32768,-32768,-32768,-32768,-32768,   -23,-32768,-32768,-32768,
-32768,    20,   130,    97,-32768,   118,-32768,   122,-32768,-32768,
-32768,-32768,-32768,   134,-32768,-32768,-32768,-32768,-32768,   133,
   121,    67,-32768,    97,    20,-32768,   135,    20,    20,   100,
-32768,    79,   100,-32768,-32768,    97,    37,-32768,-32768,    -1,
-32768,-32768,-32768,-32768,-32768,   143,   144,-32768
};

static const short yypgoto[] = {-32768,
-32768,-32768,-32768,-32768,-32768,-32768,-32768,    75,-32768,-32768,
-32768,-32768,    90,-32768,-32768,-32768,-32768,-32768,-32768,   -45,
  -101,   -31,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,
-32768,-32768,-32768,    10,-32768,-32768,-32768,-32768,-32768,-32768,
-32768,-32768,-32768,   -46,-32768,   -36,  -122,-32768,-32768,  -119,
-32768,-32768,-32768,   -35,-32768,-32768,-32768,-32768,   -80,-32768,
-32768,   -74,  -104,   -24,-32768,-32768,-32768,    -2,     7,   -26,
   -98,   -43,   -25
};


#define	YYLAST		157


static const short yytable[] = {    58,
    48,    41,   111,    40,    37,   120,   121,     6,     3,   107,
   143,    56,    57,    11,   114,   108,   136,   131,    75,    17,
    60,    64,    97,    98,   162,    66,   -91,   -91,   168,    26,
    46,    70,    71,     4,    73,   150,    74,   161,    10,     6,
     1,   162,   184,    76,   -41,     4,   158,    18,    92,    86,
    87,    19,    49,   147,    88,    96,   143,    99,   100,   114,
   101,   102,     4,     4,   109,   196,    30,   144,   125,    12,
    31,    91,    37,    89,   168,   163,   193,   129,   -56,   192,
     6,  -118,   181,    27,     7,    94,   142,   201,   128,   127,
    14,   200,   163,    13,     4,    24,   138,    23,   139,   140,
    32,    20,    37,   103,   104,    47,   146,    72,    54,    65,
    50,    42,    42,   144,    80,    82,    84,   153,   154,    93,
   186,    95,   155,   106,   115,   116,   119,   122,   129,   132,
   134,   137,   141,   151,   185,   188,   189,   190,   194,   -72,
   195,   156,   207,   208,   197,    85,    69,   202,   191,   152,
   204,   198,   203,   199,     0,     0,   205
};

static const short yycheck[] = {    43,
     1,     1,     1,    30,     6,   107,   108,     1,     1,    84,
    58,    38,    39,     7,    95,    90,   121,   116,     1,    13,
    45,    48,    46,    47,     5,    50,    26,    26,   151,     1,
    33,    58,    59,    26,    61,   134,    62,     1,     1,    33,
    62,     5,   162,    26,    45,    26,   145,     1,    75,    72,
    73,     5,    53,     5,    77,    81,    58,    81,    82,   140,
    84,    85,    26,    26,    91,   185,     1,   115,   112,    74,
     5,    74,     6,    96,   197,    56,    10,    29,    61,   181,
    74,    74,   157,    55,    41,    79,   130,   192,   115,   114,
    69,   190,    56,    91,    26,    14,   123,    74,   125,   126,
    61,    98,     6,   127,   128,    45,   133,    13,    26,    26,
   111,   111,   111,   115,    98,    13,    13,    72,    73,    13,
   164,     5,    77,    10,    53,     5,    26,    98,    29,    51,
    12,    79,    13,     5,     5,    18,     3,     5,   182,    18,
   184,    96,     0,     0,    10,    71,    57,   193,   180,   140,
   197,   188,   196,   189,    -1,    -1,   200
};
/* -*-C-*-  Note some compilers choke on comments on `#line' lines.  */
#line 3 "/usr/lib/bison.simple"

/* Skeleton output parser for bison,
   Copyright (C) 1984, 1989, 1990 Bob Corbett and Richard Stallman

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 1, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.  */


#ifndef alloca
#ifdef __GNUC__
#define alloca __builtin_alloca
#else /* not GNU C.  */
#if (!defined (__STDC__) && defined (sparc)) || defined (__sparc__) || defined (__sparc) || defined (__sgi)
#include <alloca.h>
#else /* not sparc */
#if defined (MSDOS) && !defined (__TURBOC__)
#include <malloc.h>
#else /* not MSDOS, or __TURBOC__ */
#if defined(_AIX)
#include <malloc.h>
 #pragma alloca
#else /* not MSDOS, __TURBOC__, or _AIX */
#ifdef __hpux
#ifdef __cplusplus
extern "C" {
void *alloca (unsigned int);
};
#else /* not __cplusplus */
void *alloca ();
#endif /* not __cplusplus */
#endif /* __hpux */
#endif /* not _AIX */
#endif /* not MSDOS, or __TURBOC__ */
#endif /* not sparc.  */
#endif /* not GNU C.  */
#endif /* alloca not defined.  */

/* This is the parser code that is written into each bison parser
  when the %semantic_parser declaration is not specified in the grammar.
  It was written by Richard Stallman by simplifying the hairy parser
  used when %semantic_parser is specified.  */

/* Note: there must be only one dollar sign in this file.
   It is replaced by the list of actions, each action
   as one case of the switch.  */

#define yyerrok		(yyerrstatus = 0)
#define yyclearin	(yychar = YYEMPTY)
#define YYEMPTY		-2
#define YYEOF		0
#define YYACCEPT	return(0)
#define YYABORT 	return(1)
#define YYERROR		goto yyerrlab1
/* Like YYERROR except do call yyerror.
   This remains here temporarily to ease the
   transition to the new meaning of YYERROR, for GCC.
   Once GCC version 2 has supplanted version 1, this can go.  */
#define YYFAIL		goto yyerrlab
#define YYRECOVERING()  (!!yyerrstatus)
#define YYBACKUP(token, value) \
do								\
  if (yychar == YYEMPTY && yylen == 1)				\
    { yychar = (token), yylval = (value);			\
      yychar1 = YYTRANSLATE (yychar);				\
      YYPOPSTACK;						\
      goto yybackup;						\
    }								\
  else								\
    { yyerror ("syntax error: cannot back up"); YYERROR; }	\
while (0)

#define YYTERROR	1
#define YYERRCODE	256

#ifndef YYPURE
#define YYLEX		yylex()
#endif

#ifdef YYPURE
#ifdef YYLSP_NEEDED
#define YYLEX		yylex(&yylval, &yylloc)
#else
#define YYLEX		yylex(&yylval)
#endif
#endif

/* If nonreentrant, generate the variables here */

#ifndef YYPURE

int	yychar;			/*  the lookahead symbol		*/
YYSTYPE	yylval;			/*  the semantic value of the		*/
				/*  lookahead symbol			*/

#ifdef YYLSP_NEEDED
YYLTYPE yylloc;			/*  location data for the lookahead	*/
				/*  symbol				*/
#endif

int yynerrs;			/*  number of parse errors so far       */
#endif  /* not YYPURE */

#if YYDEBUG != 0
int yydebug;			/*  nonzero means print parse trace	*/
/* Since this is uninitialized, it does not stop multiple parsers
   from coexisting.  */
#endif

/*  YYINITDEPTH indicates the initial size of the parser's stacks	*/

#ifndef	YYINITDEPTH
#define YYINITDEPTH 200
#endif

/*  YYMAXDEPTH is the maximum size the stacks can grow to
    (effective only if the built-in stack extension method is used).  */

#if YYMAXDEPTH == 0
#undef YYMAXDEPTH
#endif

#ifndef YYMAXDEPTH
#define YYMAXDEPTH 10000
#endif

/* Prevent warning if -Wstrict-prototypes.  */
#ifdef __GNUC__
int yyparse (void);
#endif

#if __GNUC__ > 1		/* GNU C and GNU C++ define this.  */
#define __yy_bcopy(FROM,TO,COUNT)	__builtin_memcpy(TO,FROM,COUNT)
#else				/* not GNU C or C++ */
#ifndef __cplusplus

/* This is the most reliable way to avoid incompatibilities
   in available built-in functions on various systems.  */
static void
__yy_bcopy (from, to, count)
     char *from;
     char *to;
     int count;
{
  register char *f = from;
  register char *t = to;
  register int i = count;

  while (i-- > 0)
    *t++ = *f++;
}

#else /* __cplusplus */

/* This is the most reliable way to avoid incompatibilities
   in available built-in functions on various systems.  */
static void
__yy_bcopy (char *from, char *to, int count)
{
  register char *f = from;
  register char *t = to;
  register int i = count;

  while (i-- > 0)
    *t++ = *f++;
}

#endif
#endif

#line 184 "/usr/lib/bison.simple"
int
yyparse()
{
  register int yystate;
  register int yyn;
  register short *yyssp;
  register YYSTYPE *yyvsp;
  int yyerrstatus;	/*  number of tokens to shift before error messages enabled */
  int yychar1 = 0;		/*  lookahead token as an internal (translated) token number */

  short	yyssa[YYINITDEPTH];	/*  the state stack			*/
  YYSTYPE yyvsa[YYINITDEPTH];	/*  the semantic value stack		*/

  short *yyss = yyssa;		/*  refer to the stacks thru separate pointers */
  YYSTYPE *yyvs = yyvsa;	/*  to allow yyoverflow to reallocate them elsewhere */

#ifdef YYLSP_NEEDED
  YYLTYPE yylsa[YYINITDEPTH];	/*  the location stack			*/
  YYLTYPE *yyls = yylsa;
  YYLTYPE *yylsp;

#define YYPOPSTACK   (yyvsp--, yyssp--, yylsp--)
#else
#define YYPOPSTACK   (yyvsp--, yyssp--)
#endif

  int yystacksize = YYINITDEPTH;

#ifdef YYPURE
  int yychar;
  YYSTYPE yylval;
  int yynerrs;
#ifdef YYLSP_NEEDED
  YYLTYPE yylloc;
#endif
#endif

  YYSTYPE yyval;		/*  the variable used to return		*/
				/*  semantic values from the action	*/
				/*  routines				*/

  int yylen;

#if YYDEBUG != 0
  if (yydebug)
    fprintf(stderr, "Starting parse\n");
#endif

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY;		/* Cause a token to be read.  */

  /* Initialize stack pointers.
     Waste one element of value and location stack
     so that they stay on the same level as the state stack.
     The wasted elements are never initialized.  */

  yyssp = yyss - 1;
  yyvsp = yyvs;
#ifdef YYLSP_NEEDED
  yylsp = yyls;
#endif

/* Push a new state, which is found in  yystate  .  */
/* In all cases, when you get here, the value and location stacks
   have just been pushed. so pushing a state here evens the stacks.  */
yynewstate:

  *++yyssp = yystate;

  if (yyssp >= yyss + yystacksize - 1)
    {
      /* Give user a chance to reallocate the stack */
      /* Use copies of these so that the &'s don't force the real ones into memory. */
      YYSTYPE *yyvs1 = yyvs;
      short *yyss1 = yyss;
#ifdef YYLSP_NEEDED
      YYLTYPE *yyls1 = yyls;
#endif

      /* Get the current used size of the three stacks, in elements.  */
      int size = yyssp - yyss + 1;

#ifdef yyoverflow
      /* Each stack pointer address is followed by the size of
	 the data in use in that stack, in bytes.  */
#ifdef YYLSP_NEEDED
      /* This used to be a conditional around just the two extra args,
	 but that might be undefined if yyoverflow is a macro.  */
      yyoverflow("parser stack overflow",
		 &yyss1, size * sizeof (*yyssp),
		 &yyvs1, size * sizeof (*yyvsp),
		 &yyls1, size * sizeof (*yylsp),
		 &yystacksize);
#else
      yyoverflow("parser stack overflow",
		 &yyss1, size * sizeof (*yyssp),
		 &yyvs1, size * sizeof (*yyvsp),
		 &yystacksize);
#endif

      yyss = yyss1; yyvs = yyvs1;
#ifdef YYLSP_NEEDED
      yyls = yyls1;
#endif
#else /* no yyoverflow */
      /* Extend the stack our own way.  */
      if (yystacksize >= YYMAXDEPTH)
	{
	  yyerror("parser stack overflow");
	  return 2;
	}
      yystacksize *= 2;
      if (yystacksize > YYMAXDEPTH)
	yystacksize = YYMAXDEPTH;
      yyss = (short *) alloca (yystacksize * sizeof (*yyssp));
      __yy_bcopy ((char *)yyss1, (char *)yyss, size * sizeof (*yyssp));
      yyvs = (YYSTYPE *) alloca (yystacksize * sizeof (*yyvsp));
      __yy_bcopy ((char *)yyvs1, (char *)yyvs, size * sizeof (*yyvsp));
#ifdef YYLSP_NEEDED
      yyls = (YYLTYPE *) alloca (yystacksize * sizeof (*yylsp));
      __yy_bcopy ((char *)yyls1, (char *)yyls, size * sizeof (*yylsp));
#endif
#endif /* no yyoverflow */

      yyssp = yyss + size - 1;
      yyvsp = yyvs + size - 1;
#ifdef YYLSP_NEEDED
      yylsp = yyls + size - 1;
#endif

#if YYDEBUG != 0
      if (yydebug)
	fprintf(stderr, "Stack size increased to %d\n", yystacksize);
#endif

      if (yyssp >= yyss + yystacksize - 1)
	YYABORT;
    }

#if YYDEBUG != 0
  if (yydebug)
    fprintf(stderr, "Entering state %d\n", yystate);
#endif

  goto yybackup;
 yybackup:

/* Do appropriate processing given the current state.  */
/* Read a lookahead token if we need one and don't already have one.  */
/* yyresume: */

  /* First try to decide what to do without reference to lookahead token.  */

  yyn = yypact[yystate];
  if (yyn == YYFLAG)
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* yychar is either YYEMPTY or YYEOF
     or a valid token in external form.  */

  if (yychar == YYEMPTY)
    {
#if YYDEBUG != 0
      if (yydebug)
	fprintf(stderr, "Reading a token: ");
#endif
      yychar = YYLEX;
    }

  /* Convert token to internal form (in yychar1) for indexing tables with */

  if (yychar <= 0)		/* This means end of input. */
    {
      yychar1 = 0;
      yychar = YYEOF;		/* Don't call YYLEX any more */

#if YYDEBUG != 0
      if (yydebug)
	fprintf(stderr, "Now at end of input.\n");
#endif
    }
  else
    {
      yychar1 = YYTRANSLATE(yychar);

#if YYDEBUG != 0
      if (yydebug)
	{
	  fprintf (stderr, "Next token is %d (%s", yychar, yytname[yychar1]);
	  /* Give the individual parser a way to print the precise meaning
	     of a token, for further debugging info.  */
#ifdef YYPRINT
	  YYPRINT (stderr, yychar, yylval);
#endif
	  fprintf (stderr, ")\n");
	}
#endif
    }

  yyn += yychar1;
  if (yyn < 0 || yyn > YYLAST || yycheck[yyn] != yychar1)
    goto yydefault;

  yyn = yytable[yyn];

  /* yyn is what to do for this token type in this state.
     Negative => reduce, -yyn is rule number.
     Positive => shift, yyn is new state.
       New state is final state => don't bother to shift,
       just return success.
     0, or most negative number => error.  */

  if (yyn < 0)
    {
      if (yyn == YYFLAG)
	goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }
  else if (yyn == 0)
    goto yyerrlab;

  if (yyn == YYFINAL)
    YYACCEPT;

  /* Shift the lookahead token.  */

#if YYDEBUG != 0
  if (yydebug)
    fprintf(stderr, "Shifting token %d (%s), ", yychar, yytname[yychar1]);
#endif

  /* Discard the token being shifted unless it is eof.  */
  if (yychar != YYEOF)
    yychar = YYEMPTY;

  *++yyvsp = yylval;
#ifdef YYLSP_NEEDED
  *++yylsp = yylloc;
#endif

  /* count tokens shifted since error; after three, turn off error status.  */
  if (yyerrstatus) yyerrstatus--;

  yystate = yyn;
  goto yynewstate;

/* Do the default action for the current state.  */
yydefault:

  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;

/* Do a reduction.  yyn is the number of a rule to reduce with.  */
yyreduce:
  yylen = yyr2[yyn];
  if (yylen > 0)
    yyval = yyvsp[1-yylen]; /* implement default value of the action */

#if YYDEBUG != 0
  if (yydebug)
    {
      int i;

      fprintf (stderr, "Reducing via rule %d (line %d), ",
	       yyn, yyrline[yyn]);

      /* Print the symbols being reduced, and their result.  */
      for (i = yyprhs[yyn]; yyrhs[i] > 0; i++)
	fprintf (stderr, "%s ", yytname[yyrhs[i]]);
      fprintf (stderr, " -> %s\n", yytname[yyr1[yyn]]);
    }
#endif


  switch (yyn) {

case 2:
#line 203 "arch.y"
{ main_arch = archAlloc(yyvsp[0].text); ;
    break;}
case 3:
#line 208 "arch.y"
{ printf("%s\n",yyvsp[-3]); ;
    break;}
case 13:
#line 250 "arch.y"
{

		;
    break;}
case 14:
#line 254 "arch.y"
{

		;
    break;}
case 22:
#line 289 "arch.y"
{

            ;
    break;}
case 23:
#line 293 "arch.y"
{

            ;
    break;}
case 27:
#line 310 "arch.y"
{ 
		
		;
    break;}
case 28:
#line 314 "arch.y"
{ 
		
		;
    break;}
case 32:
#line 335 "arch.y"
{ yyval.valu = 0; ;
    break;}
case 33:
#line 340 "arch.y"
{

                ;
    break;}
case 34:
#line 344 "arch.y"
{ 
                
                ;
    break;}
case 35:
#line 353 "arch.y"
{
                
                ;
    break;}
case 36:
#line 362 "arch.y"
{

                ;
    break;}
case 37:
#line 369 "arch.y"
{ 
                
                ;
    break;}
case 38:
#line 373 "arch.y"
{
                
                ;
    break;}
case 60:
#line 476 "arch.y"
{

		;
    break;}
case 61:
#line 484 "arch.y"
{

		;
    break;}
case 62:
#line 489 "arch.y"
{

		;
    break;}
case 69:
#line 523 "arch.y"
{

		;
    break;}
case 70:
#line 527 "arch.y"
{

		;
    break;}
case 71:
#line 531 "arch.y"
{

		;
    break;}
case 72:
#line 538 "arch.y"
{

		;
    break;}
case 73:
#line 545 "arch.y"
{
                
                ;
    break;}
case 74:
#line 552 "arch.y"
{

                ;
    break;}
case 75:
#line 556 "arch.y"
{
                
                ;
    break;}
case 76:
#line 560 "arch.y"
{ 
                
                ;
    break;}
case 77:
#line 570 "arch.y"
{

                ;
    break;}
case 78:
#line 578 "arch.y"
{

                ;
    break;}
case 79:
#line 586 "arch.y"
{
                
                ;
    break;}
case 80:
#line 593 "arch.y"
{
                
                ;
    break;}
case 81:
#line 600 "arch.y"
{
                
                ;
    break;}
case 82:
#line 607 "arch.y"
{
                
                ;
    break;}
case 83:
#line 613 "arch.y"
{

                ;
    break;}
case 84:
#line 621 "arch.y"
{ 
                
                ;
    break;}
case 85:
#line 629 "arch.y"
{
                
                ;
    break;}
case 86:
#line 636 "arch.y"
{ 
                
                ;
    break;}
case 87:
#line 640 "arch.y"
{ 
                
                ;
    break;}
case 88:
#line 644 "arch.y"
{

		;
    break;}
case 89:
#line 653 "arch.y"
{
                
                ;
    break;}
case 90:
#line 663 "arch.y"
{
                
                ;
    break;}
case 93:
#line 676 "arch.y"
{
		
		;
    break;}
case 94:
#line 680 "arch.y"
{ 
		
		;
    break;}
case 95:
#line 684 "arch.y"
{
		
		;
    break;}
case 96:
#line 688 "arch.y"
{ 
		
		;
    break;}
case 97:
#line 692 "arch.y"
{

		;
    break;}
case 98:
#line 699 "arch.y"
{ 
		
		;
    break;}
case 99:
#line 703 "arch.y"
{
		  yyval.valu = MODE_IN;
		;
    break;}
case 100:
#line 707 "arch.y"
{ 
	          yyval.valu = MODE_OUT;	
		;
    break;}
case 101:
#line 711 "arch.y"
{
		  yyval.valu = MODE_INOUT;
		;
    break;}
case 102:
#line 715 "arch.y"
{ 
		  yyval.valu = MODE_LINKAGE;
		;
    break;}
case 103:
#line 722 "arch.y"
{
			yyval.valu = BIT_TYPE;
		;
    break;}
case 104:
#line 726 "arch.y"
{	 
			yyval.valu = WOR_BIT_TYPE;
		;
    break;}
case 105:
#line 730 "arch.y"
{ 
		 	yyval.valu = MUX_BIT_TYPE;
		;
    break;}
case 106:
#line 734 "arch.y"
{
			yyval.valu = BIT_VECTOR_TYPE;
		;
    break;}
case 107:
#line 738 "arch.y"
{ 
			yyval.valu = WOR_VECTOR_TYPE;
		;
    break;}
case 108:
#line 742 "arch.y"
{ 
			yyval.valu = MUX_VECTOR_TYPE;
		;
    break;}
case 109:
#line 746 "arch.y"
{
			yyval.valu = NATURAL_TYPE;
		;
    break;}
case 110:
#line 750 "arch.y"
{ 
			yyval.valu = NATURAL_VECTOR_TYPE;
		;
    break;}
case 111:
#line 757 "arch.y"
{ ;
    break;}
case 112:
#line 759 "arch.y"
{ ;
    break;}
case 113:
#line 764 "arch.y"
{ 

                ;
    break;}
case 116:
#line 775 "arch.y"
{ 

                ;
    break;}
case 117:
#line 783 "arch.y"
{               ;
    break;}
case 118:
#line 787 "arch.y"
{ ;
    break;}
case 119:
#line 788 "arch.y"
{ yyval.text = yyvsp[0].text ;
    break;}
case 120:
#line 792 "arch.y"
{ yyval.text = yyvsp[0].text ;
    break;}
case 121:
#line 796 "arch.y"
{ yyerrok; ;
    break;}
case 122:
#line 801 "arch.y"
{ 
                
                ;
    break;}
case 123:
#line 807 "arch.y"
{ yyerrok; ;
    break;}
case 124:
#line 811 "arch.y"
{ yyerrok; ;
    break;}
}
   /* the action file gets copied in in place of this dollarsign */
#line 465 "/usr/lib/bison.simple"

  yyvsp -= yylen;
  yyssp -= yylen;
#ifdef YYLSP_NEEDED
  yylsp -= yylen;
#endif

#if YYDEBUG != 0
  if (yydebug)
    {
      short *ssp1 = yyss - 1;
      fprintf (stderr, "state stack now");
      while (ssp1 != yyssp)
	fprintf (stderr, " %d", *++ssp1);
      fprintf (stderr, "\n");
    }
#endif

  *++yyvsp = yyval;

#ifdef YYLSP_NEEDED
  yylsp++;
  if (yylen == 0)
    {
      yylsp->first_line = yylloc.first_line;
      yylsp->first_column = yylloc.first_column;
      yylsp->last_line = (yylsp-1)->last_line;
      yylsp->last_column = (yylsp-1)->last_column;
      yylsp->text = 0;
    }
  else
    {
      yylsp->last_line = (yylsp+yylen-1)->last_line;
      yylsp->last_column = (yylsp+yylen-1)->last_column;
    }
#endif

  /* Now "shift" the result of the reduction.
     Determine what state that goes to,
     based on the state we popped back to
     and the rule number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTBASE] + *yyssp;
  if (yystate >= 0 && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTBASE];

  goto yynewstate;

yyerrlab:   /* here on detecting error */

  if (! yyerrstatus)
    /* If not already recovering from an error, report this error.  */
    {
      ++yynerrs;

#ifdef YYERROR_VERBOSE
      yyn = yypact[yystate];

      if (yyn > YYFLAG && yyn < YYLAST)
	{
	  int size = 0;
	  char *msg;
	  int x, count;

	  count = 0;
	  /* Start X at -yyn if nec to avoid negative indexes in yycheck.  */
	  for (x = (yyn < 0 ? -yyn : 0);
	       x < (sizeof(yytname) / sizeof(char *)); x++)
	    if (yycheck[x + yyn] == x)
	      size += strlen(yytname[x]) + 15, count++;
	  msg = (char *) malloc(size + 15);
	  if (msg != 0)
	    {
	      strcpy(msg, "parse error");

	      if (count < 5)
		{
		  count = 0;
		  for (x = (yyn < 0 ? -yyn : 0);
		       x < (sizeof(yytname) / sizeof(char *)); x++)
		    if (yycheck[x + yyn] == x)
		      {
			strcat(msg, count == 0 ? ", expecting `" : " or `");
			strcat(msg, yytname[x]);
			strcat(msg, "'");
			count++;
		      }
		}
	      yyerror(msg);
	      free(msg);
	    }
	  else
	    yyerror ("parse error; also virtual memory exceeded");
	}
      else
#endif /* YYERROR_VERBOSE */
	yyerror("parse error");
    }

  goto yyerrlab1;
yyerrlab1:   /* here on error raised explicitly by an action */

  if (yyerrstatus == 3)
    {
      /* if just tried and failed to reuse lookahead token after an error, discard it.  */

      /* return failure if at end of input */
      if (yychar == YYEOF)
	YYABORT;

#if YYDEBUG != 0
      if (yydebug)
	fprintf(stderr, "Discarding token %d (%s).\n", yychar, yytname[yychar1]);
#endif

      yychar = YYEMPTY;
    }

  /* Else will try to reuse lookahead token
     after shifting the error token.  */

  yyerrstatus = 3;		/* Each real token shifted decrements this */

  goto yyerrhandle;

yyerrdefault:  /* current state does not do anything special for the error token. */

#if 0
  /* This is wrong; only states that explicitly want error tokens
     should shift them.  */
  yyn = yydefact[yystate];  /* If its default is to accept any token, ok.  Otherwise pop it.*/
  if (yyn) goto yydefault;
#endif

yyerrpop:   /* pop the current state because it cannot handle the error token */

  if (yyssp == yyss) YYABORT;
  yyvsp--;
  yystate = *--yyssp;
#ifdef YYLSP_NEEDED
  yylsp--;
#endif

#if YYDEBUG != 0
  if (yydebug)
    {
      short *ssp1 = yyss - 1;
      fprintf (stderr, "Error: state stack now");
      while (ssp1 != yyssp)
	fprintf (stderr, " %d", *++ssp1);
      fprintf (stderr, "\n");
    }
#endif

yyerrhandle:

  yyn = yypact[yystate];
  if (yyn == YYFLAG)
    goto yyerrdefault;

  yyn += YYTERROR;
  if (yyn < 0 || yyn > YYLAST || yycheck[yyn] != YYTERROR)
    goto yyerrdefault;

  yyn = yytable[yyn];
  if (yyn < 0)
    {
      if (yyn == YYFLAG)
	goto yyerrpop;
      yyn = -yyn;
      goto yyreduce;
    }
  else if (yyn == 0)
    goto yyerrpop;

  if (yyn == YYFINAL)
    YYACCEPT;

#if YYDEBUG != 0
  if (yydebug)
    fprintf(stderr, "Shifting error token, ");
#endif

  *++yyvsp = yylval;
#ifdef YYLSP_NEEDED
  *++yylsp = yylloc;
#endif

  yystate = yyn;
  goto yynewstate;
}
#line 814 "arch.y"


archwrap()
{

}

archerror()
{
  fprintf(stderr,"Se fudeu.\n");
}
