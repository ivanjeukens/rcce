/**CFile***********************************************************************

  FileName    [rouMain.c]

  PackageName [rou]

  Synopsis    [The core of the routing package.]

  Description []

  SeeAlso     []

  Author      [Ivan Jeukens]

  Copyright   []

******************************************************************************/

#include "rouInt.h" 

/*
static char rcsid[] = \"$Id: $\";
USE(rcsid);
*/

/**AutomaticStart*************************************************************/

/*---------------------------------------------------------------------------*/
/* Static function prototypes                                                */
/*---------------------------------------------------------------------------*/

static int try_route(int chan_width, netl_clbl_t *clbl, array_t *netl, int nx, int ny, int io_rat);
static graph_t * build_rr_graph(netl_clbl_t *clbl, array_t *netl, int nx, int ny, int io_rat);
static void destroy_rr_graph(graph_t *rr_graph);
static void print_rr_graph(graph_t *rr_graph);

/**AutomaticEnd***************************************************************/

/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/
/**Function********************************************************************

  Synopsis           [Main routing routine.]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
void
rouMain(
  netl_clbl_t *clbl,
  array_t *netl,
  int nx,
  int ny,
  int io_rat,
  int vpr,
  int verify)
{
int current = 4, high = -1, low = -1, final = -1, success;
int prev_success, prev2_success;

  while((final == -1) && (current < MAX_WIDTH)) {
    success = try_route(current, clbl, netl, nx, ny, io_rat);

    if(success) {
      high = current;

      /* save_routing(best_routing); */

      if((high - low) <= 1) {
        final = high;
      } 
      if(low != -1) {
        current = (high+low)/2;
      } 
      else {
        current = high/2;
      } 
    } 
    else {
      low = current;
      if(high != -1) {
        if((high - low) <= 1) {
          final = high;
        }
        current = (high+low)/2;
      } 
      else {
        current = low*2;
      } 
    }
  }

  if(verify == 1) {
    prev_success = 1;
    prev2_success = 1;  
    current = final - 2;  
    while((prev2_success == 1) || (prev_success == 1)) {
      if(current < 1) break;
      
      success = try_route(current, clbl, netl, nx, nx, io_rat);
    
      if(success) {
        final = current;
        /*save_routing (best_routing);*/
      }
      prev2_success = prev_success;
      prev_success = success;
      current--;
    }
  }

}

/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis           [Try to route the circuit with a given channel width.]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static int
try_route(
  int chan_width,
  netl_clbl_t *clbl,
  array_t *netl,
  int nx,
  int ny,
  int io_rat)
{
graph_t *rr_graph;

  rr_graph = build_rr_graph(clbl, netl, nx, ny, io_rat);


  print_rr_graph(rr_graph);
  
  destroy_rr_graph(rr_graph);
}
  
/**Function********************************************************************

  Synopsis           []

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static graph_t *
build_rr_graph(
  netl_clbl_t *clbl,
  array_t *netl,
  int nx,
  int ny,
  int io_rat)
{
graph_t *ret;
vertex_t *v;
int i, j;
rr_node_t *rn;

  ret = g_alloc();

/*
  for(i = 0;i < array_n(clbl->clbs);i++) {
    clb = array_fetch(clbl->clbs);   
    v = g_add_vertex(ret);
    rn = (rr_node_t *) ALLOC(rr_node_t, 1);
    rn->prev_node = NIL(vertex_t);
    rn->cost = 0.;
    rn->path_cost = 0.;
    rn->target = 0;
    rn->xlow = i;
    rn->xhigh = i;
    rn->ylow = j;
    rn->yhigh = j;
    v->user_data = (char *) rn;

  }  
*/
    
  return ret;
}

/**Function********************************************************************

  Synopsis           []

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static void
destroy_rr_graph(graph_t *rr_graph)
{


}

/**Function********************************************************************

  Synopsis           []

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static void
print_rr_graph(graph_t *rr_graph)
{


}
