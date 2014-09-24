/**CHeaderFile*****************************************************************

  FileName    [drawInt.h]

  PackageName [draw]

  Synopsis    [This file contains the drawing structures.]

  Description []

  SeeAlso     []

  Author      [Ivan Jeukens]

  Copyright   [1996-1997 Ivan Jeukens]

  Revision    [$Id: $]

******************************************************************************/

#ifndef _DRAWINT
#define _DRAWINT

#include "ansi.h"
#include "array.h"
#include "util.h"
#include "list.h"
#include "graph.h"
#include "comp.h"
#include "arch.h"
#include "draw.h"

/*---------------------------------------------------------------------------*/
/* Constant declarations                                                     */
/*---------------------------------------------------------------------------*/

#define DRAWLINE 1
#define DRAWRECT 2
#define DRAWCIRC 3

#define DRAWLOWER_FLAG 1
#define DRAWUPPER_FLAG 2

#define DRAWTOP 1
#define DRAWBOTTON 2
#define DRAWLEFT 3
#define DRAWRIGHT 4
#define DRAWCENTER 5

/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/

typedef struct obj_line drawobj_line_t;
typedef struct obj_roc drawobj_roc_t;
typedef struct object drawobject_t;

/*---------------------------------------------------------------------------*/
/* Structure declarations                                                    */
/*---------------------------------------------------------------------------*/

struct obj_line {
  int x2;
  int y2;
};

struct obj_roc {
  short filled;  
  int height;
  int width;
};

struct object {
  short type;
  int color;
  int x1;
  int y1;
  int style;
  int cap_style;
  int join_style;
  int line_width;
  union {
    drawobj_line_t *line;
    drawobj_roc_t *roc;
  } obj;  
};

struct draw {
  int x;
  int y;
  int width;
  int height;
  int backcolor;
  array_t *obj;
};

/*---------------------------------------------------------------------------*/
/* Macro declarations                                                        */
/*---------------------------------------------------------------------------*/

/**Macro***********************************************************************

  Synopsis     [Sorts an array of objects.]

  Description  []

  SideEffects  []

  SeeAlso      []

******************************************************************************/
#define DRAWOBJ_SORT(obj) ( array_sort((obj), drawObjCmp) )

/**AutomaticStart*************************************************************/

/*---------------------------------------------------------------------------*/
/* Function prototypes                                                       */
/*---------------------------------------------------------------------------*/

EXTERN draw_t * drawRocIns(draw_t *s, int x1, int y1, short type, int style, int cap_style, int join_style, int color, int line_width, short filled, int height, int width);
EXTERN draw_t * drawLineIns(draw_t *s, int x1, int y1, int x2, int y2, int color, int width, int style, int cap_style, int join_style);
EXTERN int drawObjCmp(char *o1, char *o2);
EXTERN draw_t * drawAlloc(int x, int y, int w, int h, int backcolor);
EXTERN void print_draw(draw_t *s);

/**AutomaticEnd***************************************************************/

#endif /* _ */
