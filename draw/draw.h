/**CHeaderFile*****************************************************************

  FileName    [draw.h]

  PackageName [draw]

  Synopsis    [This package is used to create a drawing, of any particular
  structure, such as a network of a BDD. The drawing can the be displayed
  and manipulated on a canvas. This package also enable the creation of
  animations, better ilustrating the behavior of an algorithm.]

  Description []

  SeeAlso     []

  Author      [Ivan Jeukens]

  Copyright   [1996-1997 Ivan Jeukens]

  Revision    [$Id: $]

******************************************************************************/

#ifndef _DRAW
#define _DRAW

#include <xview/canvas.h>
#undef va_start
#include "compInt.h"
#include "netl.h"

/*---------------------------------------------------------------------------*/
/* Constant declarations                                                     */
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/

typedef struct draw draw_t;

/*---------------------------------------------------------------------------*/
/* Structure declarations                                                    */
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/* Variable declarations                                                     */
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/* Macro declarations                                                        */
/*---------------------------------------------------------------------------*/

/**Macro***********************************************************************

  Synopsis     [Change the X Y coordinates of the screen.]

  Description  []

  SideEffects  []

  SeeAlso      []

******************************************************************************/
#define SCREEN_XY(s, X, Y) ( (s)->x = X  (s)->y = Y )

/**Macro***********************************************************************

  Synopsis     [Change the Widht an Height of the screen.]

  Description  []

  SideEffects  []

  SeeAlso      []

******************************************************************************/
#define SCREEN_WH(s, w, h) ((s)->w = (w) (s)->h = (h))

/**AutomaticStart*************************************************************/

/*---------------------------------------------------------------------------*/
/* Function prototypes                                                       */
/*---------------------------------------------------------------------------*/

EXTERN draw_t * draw_Clbl(netl_clbl_t *clbl, int w, int h);
EXTERN draw_t * draw_Comp(comp_data_t *c, int w, int h);
EXTERN draw_t * draw_Net(network_t *network, int w, int h);
EXTERN void draw_Free(draw_t *s);
EXTERN void draw_Update(Canvas canvas, draw_t *s);

/**AutomaticEnd***************************************************************/

#endif /* _ */
