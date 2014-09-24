/**CFile***********************************************************************

  FileName    [drawComp.c]

  PackageName [draw]

  Synopsis    [Create a drawing of a comp_data_t structure.]

  Description []

  SeeAlso     []

  Author      [Ivan Jeukens]

  Copyright   [1996-1997 Ivan Jeukens]

******************************************************************************/
#include <xview/cms.h>
#include <xview/xv_xrect.h>
#undef va_start
#include "drawInt.h"
#include "comp.h"
#include "compInt.h"
#include "intf.h"

/*
static char rcsid[] = \"$Id: $\";
USE(rcsid);
*/

/**AutomaticStart*************************************************************/

/*---------------------------------------------------------------------------*/
/* Static function prototypes                                                */
/*---------------------------------------------------------------------------*/

/**AutomaticEnd***************************************************************/


/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis           [Create a draw_t structure representing a comp_data_t.]

  Description        [optional]

  SideEffects        [required]

  SeeAlso            [optional]

******************************************************************************/
draw_t *
draw_Comp(
  comp_data_t *c,
  int w,
  int h)
{
draw_t *new;
int i;
comp_figpin_t *fptr;
char number[10];
comp_pin_t *cptr;

  new = drawAlloc(0, 0, w, h, BLACK);
 
  new = drawRocIns(new, 30, 30, DRAWRECT, LineSolid, CapButt, JoinMiter, RED,
    2, 0, c->fig->width, c->fig->height);
    
  for(i = 0;i < array_n(c->fig->pins);i++) {
    fptr = array_fetch(comp_figpin_t *,c->fig->pins,i);
    sprintf(number,"%d",fptr->number);
    new = drawLineIns(new, 30 + fptr->dx1, 30 + fptr->dy1, 30 + fptr->dx2,
      30 + fptr->dy2, AQUA, 2, LineSolid, CapButt, JoinMiter);
  }

  for(i = 0;i < array_n(c->pins);i++) {
    cptr = array_fetch(comp_pin_t *, c->pins, i);
  }  

  return new;
}

/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/
