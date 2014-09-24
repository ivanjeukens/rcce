/**CFile***********************************************************************

  FileName    [compRg.c]

  PackageName [comp]

  Synopsis    [Contain functions for manipulating the routing graph.]

  Description []

  SeeAlso     []

  Author      [Ivan Jeukens]

  Copyright   [1996-1997 Ivan Jeukens]

******************************************************************************/

#include "compInt.h"

/*
static char rcsid[] = "$Id: $";
USE(rcsid);
*/

/**AutomaticStart*************************************************************/

/*---------------------------------------------------------------------------*/
/* Static function prototypes                                                */
/*---------------------------------------------------------------------------*/

static void RnodeFree(gGeneric *data);
static comprnode_t * RnodeAlloc(char *label);
static vertex_t * RnodeFind(graph_t *rg, char *label);
static vertex_t * RnodeIns(graph_t *rg, char *label);

/**AutomaticEnd***************************************************************/


/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis           [Release de memory kept by a routing graph]

  Description        []

  SideEffects        []
  
  SeeAlso	     []
  
******************************************************************************/
void
compRgraphFree(graph_t *rg)
{
  g_free(rg, NULL, RnodeFree, NULL);
}

/**Function********************************************************************

  Synopsis           [Print a routing graph.]

  Description        []

  SideEffects        []
  
  SeeAlso	     []
  
******************************************************************************/
void
compRgraphPrint(graph_t *rg)
{
lsList vertices, edges;
lsGen gen, egen;
comprnode_t *data;
vertex_t *v;
edge_t *e;
int status;

  if(rg == NIL(graph_t)) {
    return;
  }
  vertices = g_get_vertices(rg);
  gen = lsStart(vertices);
  status = lsNext(gen, (lsGeneric *) &v, LS_NH);    
  while(status != LS_NOMORE) {
    data = (comprnode_t *) v->user_data;
    printf("No %s: ",data->label);
    edges = g_get_out_edges(v);
    egen = lsStart(edges);    
    status = lsNext(egen, (lsGeneric *) &e, LS_NH);
    while(status != LS_NOMORE) {
      v = g_e_dest(e);
      data = (comprnode_t *) v->user_data;
      printf("%s ", data->label);
      status = lsNext(egen, (lsGeneric *) &e, LS_NH);      
    }
    status = lsNext(gen, (lsGeneric *) &v, LS_NH);    
    printf("\n");
  }
  (void) lsFinish(gen);
}

/**Function********************************************************************

  Synopsis           [Print a routing graph.]

  Description        []

  SideEffects        []
  
  SeeAlso	     []
  
******************************************************************************/
comp_data_t *
compRnodeConnect(
  comp_data_t *c,
  char *label1,
  char *label2)
{
vertex_t *v1,*v2;
edge_t *edge;
   
  v1 = RnodeIns(c->rg, label1);
  v2 = RnodeIns(c->rg, label2);
  
  edge = g_add_edge(v1, v2);
  edge->user_data = (gGeneric) NULL;
  
  return c;
}

/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis           [Release de memory kept by a routing node.]

  Description        []

  SideEffects        []
  
  SeeAlso	     []
  
******************************************************************************/
static void		/* destroy_rgraph_data */
RnodeFree(gGeneric *data)
{
comprnode_t  *d;

  d = (comprnode_t *) data;
  FREE(d->label);
  FREE(d);
}

/**Function********************************************************************

  Synopsis           [Allocate memory for a routing node.]

  Description        []

  SideEffects        []
  
  SeeAlso	     []
  
******************************************************************************/
static comprnode_t *
RnodeAlloc(char *label)
{
comprnode_t *p;

  p = (comprnode_t *) ALLOC(comprnode_t, 1);
  p->label = util_strsav(label);

  return p;
}

/**Function********************************************************************

  Synopsis           [Find a routing node within the routing graph. If not 
  found, return NULL.]

  Description        []

  SideEffects        []
  
  SeeAlso	     []
  
******************************************************************************/
static vertex_t *
RnodeFind(
  graph_t *rg,
  char *label)
{
lsList vertices;
lsGen gen;
vertex_t *v;
int status;
comprnode_t *data;

  vertices = g_get_vertices(rg);
  gen = lsStart(vertices);
  status = lsNext(gen, (lsGeneric *) &v, LS_NH);    
  if(status == LS_NOMORE) {
    (void) lsFinish(gen);
    return NIL(vertex_t);
  }
  else
    data = (comprnode_t *) v->user_data;
  while(strcmp(data->label, label) != 0) {
    status = lsNext(gen, (lsGeneric *) &v, LS_NH);    
    if(status == LS_NOMORE) {
      (void) lsFinish(gen);
      return NIL(vertex_t);
    }
    else
      data = (comprnode_t *) v->user_data;
  }
  (void) lsFinish(gen);
  return v;
}

/**Function********************************************************************

  Synopsis           [Insert a routing node.]

  Description        []

  SideEffects        []
  
  SeeAlso	     []
  
******************************************************************************/
static vertex_t *
RnodeIns(
  graph_t *rg,
  char *label)
{
vertex_t *v;
comprnode_t *data;
 
  v = RnodeFind(rg, label);
  if(v == NIL(vertex_t)) {
    v = g_add_vertex(rg);
    data = RnodeAlloc(label);
    v->user_data = (gGeneric) data;
  }

  return v; 
}
