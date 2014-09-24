/**CFile***********************************************************************

  FileName    [drawClbl.c]

  PackageName [draw]

  Synopsis    [Create a drawing of a placed clb list.]

  Description []

  SeeAlso     []

  Author      [Ivan Jeukens]

  Copyright   [1996-1997 Ivan Jeukens]

******************************************************************************/
#include <xview/cms.h>
#include <xview/xv_xrect.h>
#undef va_start
#include "drawInt.h"
#include "netlInt.h"
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

  Synopsis           [Main procedure.]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
draw_t *
draw_Clbl(
  netl_clbl_t *clbl,
  int w,
  int h)
{
netl_clb_t *clb;
draw_t *new;
int i;
  
  if(clbl == NIL(netl_clbl_t)) return NIL(draw_t);
  
  new = drawAlloc(0, 0, w, h, GRAY2);

  for(i = 0;i < array_n(clbl->clbs);i++) {
    clb = array_fetch(netl_clb_t *, clbl->clbs, i);
    if(clb->x != -1) {
      new = drawRocIns(new, 20 + 30*clb->x, 20 + 30*clb->y, DRAWRECT, 
        LineSolid, CapButt, JoinMiter, BLUE, 2, 0, 20, 20);        
    }
  } 
  for(i = 0;i < array_n(clbl->inputs);i++) {
    clb = array_fetch(netl_clb_t *, clbl->inputs, i);
    if(clb->x != -1) {
      new = drawRocIns(new, 20 + 30*clb->x, 20 + 30*clb->y, DRAWRECT, 
        LineSolid, CapButt, JoinMiter, RED, 2, 0, 20, 20);        
    }
  } 
  for(i = 0;i < array_n(clbl->outputs);i++) {
    clb = array_fetch(netl_clb_t *, clbl->outputs, i);
    if(clb->x != -1) {
      new = drawRocIns(new, 20 + 30*clb->x, 20 + 30*clb->y, DRAWRECT, 
        LineSolid, CapButt, JoinMiter, RED, 2, 0, 20, 20);        
    }
  } 

  return new;
}

/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/
