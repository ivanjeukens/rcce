/**CFile***********************************************************************

  FileName    [drawNet.c]

  PackageName [draw]

  Synopsis    [Create a drawing of a network_t structure.]

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
#include "st.h"
#include "node.h"
#include "network.h"

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

  Synopsis           [Create a draw_t structure representing a network.]

  Description        [optional]

  SideEffects        [required]

  SeeAlso            [optional]

******************************************************************************/
draw_t *
draw_Net(
  network_t *network,
  int w,
  int h)
{
draw_t *new;
lsGen gen;
int x = 10, y = 20, nl, level, i;
st_table *xt, *yt;
char *value, *key;
array_t *nodes;
node_t *n, *fanin;
int *lasty, lsize = 0, x1, y1, x2, y2;
st_generator *stgen;

  if((network == NIL(network_t)) || (network_num_pi(network) == 0) ||
     (network_num_po(network) == 0)) {
    return NIL(draw_t);
  }

  xt = st_init_table(strcmp, st_strhash);
  yt = st_init_table(strcmp, st_strhash);
      
  new = drawAlloc(0, 0, w, h, GRAY2);
  
  foreach_primary_input(network, gen, n) {
    new = drawRocIns(new, x, y, DRAWRECT, LineSolid, CapButt, JoinMiter, RED, 
      2, 0, 20, 20);
    value = (char *) x;
    (void) st_insert(xt, util_strsav(node_name(n)), value);
    value = (char *) y + 10;
    (void) st_insert(yt, util_strsav(node_name(n)), value);
    y += 40;
  }
  
  (void) network_level(network);
  foreach_primary_output(network, gen, n) {
    if(node_level(n) > lsize) lsize = node_level(n);
  }  
  lasty = (int *) ALLOC(int, ++lsize);
  for(i = 0;i < lsize;i++) {
    lasty[i] = -1;
  }
    
  nodes = network_dfs(network);
  y = -20;
  level = 1;
  for(i = 0;i < array_n(nodes);i++) {
    n = array_fetch(node_t *, nodes, i);
    nl = node_level(n);
    if(nl == 0) continue;

    if(level == nl) {
      y += 40;
      lasty[nl] = y;
    }
    else {
      if(lasty[nl] == -1) {
        y = 20;
        lasty[nl] = 10;
      }
      else {
        y = lasty[nl] + 40;
        lasty[nl] += 40;
      }
      level = nl;      
    }
    if(node_type(n) == PRIMARY_OUTPUT) {
      new = drawRocIns(new, x + 80*nl, y, DRAWRECT, LineSolid, CapButt, 
        JoinMiter, RED, 2, 0, 20, 20); 
    }
    else {
      new = drawRocIns(new, x + 80*nl, y, DRAWCIRC, LineSolid, CapButt, 
        JoinMiter, BLUE, 1, 1, 20, 20);
    }
    value = (char *) x + 80*nl;
    (void) st_insert(xt, util_strsav(node_name(n)), value);
    value = (char *) y + 10;
    (void) st_insert(yt, util_strsav(node_name(n)), value);
  }
  FREE(lasty);
  array_free(nodes);  

  foreach_node(network, gen, n) {
    if(node_type(n) == PRIMARY_INPUT) continue;
    (void) st_lookup(xt, node_name(n), &value);
    x1 = (int) value;
    (void) st_lookup(yt, node_name(n), &value);
    y1 = (int) value;
    foreach_fanin(n, i, fanin) {
      (void) st_lookup(xt, node_name(fanin), &value);
      x2 = (int) value;
      (void) st_lookup(yt, node_name(fanin), &value);
      y2 = (int) value;
      new = drawLineIns(new, x1, y1, x2 + 20, y2, BLACK, 1, LineSolid, CapButt,
        JoinMiter);
    } 
  }

  st_foreach_item(xt, stgen, &key, &value) {
    FREE(key);
  }
  st_free_table(xt);
  st_foreach_item(yt, stgen, &key, &value) {
    FREE(key);
  }
  st_free_table(yt);
  return new;
}
