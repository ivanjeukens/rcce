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


extern YYSTYPE archlval;
