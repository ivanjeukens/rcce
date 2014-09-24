/**CFile***********************************************************************

  FileName    [placePre.c]

  PackageName [place]

  Synopsis    [The core of the algorithm.]

  Description []

  SeeAlso     []

  Author      []

  Copyright   [1996-1997 Ivan Jeukens]

******************************************************************************/

#include "placeInt.h"

/*
static char rcsid[] = \"$Id: $\";
USE(rcsid);
*/

/*---------------------------------------------------------------------------*/
/* Constant declarations                                                     */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Variable declarations                                                     */
/*---------------------------------------------------------------------------*/
st_table *blocks;

/**AutomaticStart*************************************************************/

/*---------------------------------------------------------------------------*/
/* Static function prototypes                                                */
/*---------------------------------------------------------------------------*/

static array_t * max_clique(graph_t *g, int *n);
static array_t * FindAdj(graph_t *g, array_t *ver);
static vertex_t * FindVertex(graph_t *g, netl_clb_t *clb);
static void PrintGraph(graph_t *g);

/**AutomaticEnd***************************************************************/


/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis           [Try to partition the clbs into cliques.]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
void
placeCluster(netl_clbl_t *clbl)
{
netl_clb_t *clb;
int i, j, k, m;
graph_t *g;
vertex_t *v, *v2;
netl_pin_t *pin;
netl_signet_t *sn;
array_t *clbs, *clique;
st_generator *gen;

  g = g_alloc();
  for(i = 0;i < array_n(clbl->clbs);i++) {
    clb = array_fetch(netl_clb_t *, clbl->clbs, i);
    v = g_add_vertex(g);
    v->user_data = (char *) clb;  
  }

  for(i = 0;i < array_n(clbl->clbs);i++) {
    clb = array_fetch(netl_clb_t *, clbl->clbs, i);
    v = FindVertex(g, clb);
    clbs = array_alloc(netl_clb_t *, 0);
    for(j = 0;j < array_n(clb->cb->out);j++) {
      pin = array_fetch(netl_pin_t *, clb->cb->out, j);   
      for(k = 0;k < array_n(pin->net->snet);k++) {
        sn = array_fetch(netl_signet_t *, pin->net->snet, k);
        if((sn->clb->cb->in != NIL(array_t)) && 
           (sn->clb->cb->out != NIL(array_t))) {
          v2 = FindVertex(g, sn->clb);
          if(v2 != v) {
            for(m = 0;m < array_n(clbs);m++) {
              if(array_fetch(netl_clb_t *, clbs, m) == sn->clb) {
                break;
              }
            }
            if(m == array_n(clbs)) {
              (void) g_add_edge(v, v2);
              (void) g_add_edge(v2, v);
              array_insert_last(netl_clb_t *, clbs, sn->clb);
            }
          }
        }
      }      
    }    
    array_free(clbs);
  }

  blocks = st_init_table(st_ptrcmp, st_ptrhash);
  i = array_n(clbl->clbs);
  while(i > 0) {
    clique = max_clique(g, &i);
    if(array_n(clique) >= 3) {
      for(j = 0;j < array_n(clique);j++) {
        clb = array_fetch(netl_clb_t *, clique, j);
        st_insert(blocks, (char *) clb, (char *) clique);
      }
    }
    else {
      array_free(clique);
    }
  }    
  g_free(g, NULL, NULL, NULL);
  if(st_count(blocks) == 0) {
    st_free_table(blocks);
    blocks = NIL(st_table);
  }
  else {
    printf("Cliques:\n");
    st_foreach_item(blocks, gen, &clb, &clique) {
      printf("CLB %s: ",clb->label);
      for(i = 0;i < array_n(clique);i++) {
        clb = array_fetch(netl_clb_t *, clique, i);
        printf("%s ", clb->label);
      }
      printf("\n");
    }
  }
}

/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis           [Heuristic algorithm that finds a maximal clique of a
  graph.]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static array_t *
max_clique(
  graph_t *g,
  int *n)
{
array_t *ret, *ver;
vertex_t *v, *mv;
lsList vertices;
lsGen gen;
int maxdg = -1, i, j;
array_t *candidates;

  ret = array_alloc(netl_clb_t *, 0);
  ver = array_alloc(vertex_t *, 0);

  vertices = g_get_vertices(g);
  lsForeachItem(vertices, gen , v) {
    if(v->user_data != NIL(char)) {
      i = lsLength(g_get_out_edges(v));
      if(i > maxdg) {
        maxdg = i;
        mv = v;
      }
    }
  }
  (*n)--;
  array_insert_last(netl_clb_t *, ret, (netl_clb_t *) mv->user_data);
  array_insert_last(vertex_t *, ver, mv);
  mv->user_data = NIL(char);
 
  while(1) {
    while(1) {
      candidates = FindAdj(g, ver);
      if(candidates == NIL(array_t)) {
        return ret;
      }   
      else {
        maxdg = -1;
        for(i = 0;i < array_n(candidates);i++) {
          v = array_fetch(vertex_t *, candidates, i);        
          j = lsLength(g_get_out_edges(v));
          if(i > maxdg) {
            maxdg = i;
            mv = v;
          }
        }
        array_insert_last(netl_clb_t *, ret, (netl_clb_t *) mv->user_data);
        array_insert_last(vertex_t *, ver, mv);
        mv->user_data = NIL(char);
        (*n)--;
      }
      array_free(candidates);
    }
  }
  array_free(ver);

  return ret;
}
 
/**Function********************************************************************

  Synopsis           [Find all vertices that are connected to all vertices
  within the clique list.]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/ 
static array_t *
FindAdj(
  graph_t *g,
  array_t *ver)
{
array_t *candidates = NIL(array_t);
lsGen gen, gen2;
vertex_t *v, *vd, *vt;
lsList vertices;
lsList out;
edge_t *edge;
int i, connect;

  vertices = g_get_vertices(g);
  lsForeachItem(vertices, gen, v) {
    if(v->user_data != NIL(char)) {
      connect = 0;
      out = g_get_out_edges(v);
      for(i = 0;i < array_n(ver);i++) {
        vt = array_fetch(vertex_t *, ver, i);
        lsForeachItem(out, gen2, edge) {
          vd = g_e_dest(edge);
          if(vd == vt) {
            lsFinish(gen2);
            connect++;
            break;
          }
        }
      }
      if(connect == array_n(ver)) {
        if(candidates == NIL(array_t)) {
          candidates = array_alloc(vertex_t *, 0);
        }
        array_insert_last(vertex_t *, candidates, v);
      }
    }
  }

  return candidates;
}
  
/**Function********************************************************************

  Synopsis           [Find a vertex within a vertex list.]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static vertex_t *
FindVertex(
  graph_t *g,
  netl_clb_t *clb)
{
lsGen gen;
vertex_t *v;
netl_clb_t *c;
lsList vertices;

  vertices = g_get_vertices(g);
  lsForeachItem(vertices, gen, v) {
    c = (netl_clb_t *) v->user_data;
    if(c == clb) {
      lsFinish(gen);
      return v;
    }     
  }

  return NIL(vertex_t);
}

/**Function********************************************************************

  Synopsis           [Print the clbl graph.]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static void
PrintGraph(graph_t *g)
{
lsGen gen, gen2;
lsList vertices, edges;
vertex_t *v, *vd;
netl_clb_t *clb;
edge_t *edge;

  vertices = g_get_vertices(g);
  lsForeachItem(vertices, gen, v) {
    clb = (netl_clb_t *) v->user_data;
    printf("Node %s\n", clb->label);
    printf("Connected to: ");
    edges = g_get_out_edges(v);
    lsForeachItem(edges, gen2, edge) {
      vd = g_e_dest(edge);
      clb = (netl_clb_t *) vd->user_data;
      printf("%s ", clb->label);    
    }  
    printf("\n");
  }
}
