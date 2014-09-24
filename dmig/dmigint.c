/**CFile***********************************************************************

  FileName    [dmigInt.c]

  PackageName [dmig]

  Synopsis    [The core of the dmig algorithm.]

  Description []

  SeeAlso     []

  Author      [1996-1997 Ivan Jeukens]

  Copyright   []

******************************************************************************/

#include "dmigInt.h"

/*
static char rcsid[] = \"$Id: $\";
USE(rcsid);
*/

/**AutomaticStart*************************************************************/

/*---------------------------------------------------------------------------*/
/* Static function prototypes                                                */
/*---------------------------------------------------------------------------*/

static void NodeDm(network_t *network, node_t *node, short nfunc, short flag);
static void GetSmallestLevel(node_t *node, node_t **n1, node_t **n2);

/**AutomaticEnd***************************************************************/


/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis           [Main dmig procedure.]

  Description        [optional]

  SideEffects        [required]

  SeeAlso            [optional]

******************************************************************************/
void
dmigRun(network_t *network)
{
array_t *aux;
int i;
node_t *node;

  network_level(network); 
  aux = network_dfs(network);  

  for(i = 0;i < array_n(aux);i++) {
    node = array_fetch(node_t *, aux, i);    
    if((node_function(node) == NODE_COMPLEX) ||
       (node_function(node) == NODE_UNDEFINED)) {
      fprintf(siserr,"Network must be and-or only!\n");
      return;
    }
    if(node->type == INTERNAL) {
      switch(node_function(node)) {
        case NODE_AND: {
          if(node_num_fanin(node) > 2) {
            NodeDm(network, node, 1, 0);
          }
        } break;
        case NODE_OR: {
          if(node_num_fanin(node) > 2) {
            NodeDm(network, node, 0, 1);
          }
        } break;
        default: ;
      }    
    }  
  }
  array_free(aux);

  (void) network_sweep(network);
}

/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis           [Decompose the given node.]

  Description        [optional]

  SideEffects        [required]

  SeeAlso            [optional]

******************************************************************************/
static void
NodeDm(
  network_t *network,
  node_t *node,
  short nfunc,  
  short flag)
{
node_t *n1, *n2, *new, *new2, **fanin, *t1, *t2;
int ph1,ph2, i, a;
pset ps, last, p2;
pset_family newfunc;
  
  while(node_num_fanin(node) > 2) { 
    GetSmallestLevel(node, &n1, &n2);
    if(node_input_phase(node, n1) == POS_UNATE) {
      ph1 = 1;
    }
    else {
      ph1 = 0;
    }
      
    if(node_input_phase(node, n2) == POS_UNATE) {
      ph2 = 1;
    }
    else {
      ph2 = 0;    
    }
    t1 = node_literal(n1, ph1);
    t2 = node_literal(n2, ph2);
    if(nfunc == 0) {
      new = node_or(t1, t2);
    }
    else {
      new = node_and(t1, t2);
    }
    node_free(t1);
    node_free(t2);    
    new->level = MAX(n1->level, n2->level) + 1;

    ph1 = node_get_fanin_index(node, n1);
    ph2 = node_get_fanin_index(node, n2);    

    if(flag == 0) {
      newfunc = sf_new(1, (node->nin - 2)*2);
      foreach_set(node->F, last, ps) {
        p2 = GETSET(newfunc, newfunc->count++);
        (void) set_clear(p2, (node->nin - 2)*2);
        a = 0;
        for(i = 0;i < node->nin;i++) {
          if((i != ph1) && (i != ph2)) {
            switch(GETINPUT(ps, i)) {
              case ONE: set_insert(p2, 2*a + 1); break;
              case ZERO: set_insert(p2, 2*a); break;
              case TWO: { set_insert(p2, 2*a + 1); set_insert(p2, 2*a); } 
                        break;
              default: {
 	        fprintf(siserr,"dm_node: improper set encountered\n");
                exit(1);
              }
            }        
            a++;
          }
        }        
      }
    }
    else {
      newfunc = sf_new(node->F->count - 2, (node->nin - 2)*2);
      foreach_set(node->F, last, ps) {
        if((GETINPUT(ps, ph1) == TWO) && (GETINPUT(ps, ph2) == TWO)) {
          p2 = GETSET(newfunc, newfunc->count++);
          (void) set_clear(p2, (node->nin - 2)*2);
          a = 0;
          for(i = 0;i < node->nin;i++) {
            if((i != ph1) && (i != ph2)) {
              switch(GETINPUT(ps, i)) {
                case ONE: set_insert(p2, 2*a + 1); break;
                case ZERO: set_insert(p2, 2*a); break;
                case TWO: { set_insert(p2, 2*a + 1); set_insert(p2, 2*a); }
                          break;
 	        default: {
 	          fprintf(siserr,"dm_node: improper set encountered\n");
                  exit(1);
                }
              }        
              a++;
            }
          }          
        }
      }
    }        
    fanin = (node_t **) ALLOC(node_t *, node->nin-2);
    a = 0;
    for(i = 0;i < node->nin;i++) {
      if((i != ph1) && (i != ph2)) {
        fanin[a] = node->fanin[i]; 
        a++;
      }   
    }
    new2 = node_create(newfunc, fanin, node->nin - 2);
    new2->is_dup_free = node->is_dup_free;
    node_minimum_base(new2);
    node_replace(node, new2);
        
    t1 = node_literal(new, 1);
    if(nfunc == 0) {
      new2 = node_or(node, t1);
    }
    else {
      new2 = node_and(node, t1);
    }
    node_replace(node, new2);    
    node_free(t1); 
        
    network_add_node(network, new);
  }
}

/**Function********************************************************************

  Synopsis           [Given a node, search its fanin list for the two nodes
  with least level.]

  Description        [optional]

  SideEffects        [required]

  SeeAlso            [optional]

******************************************************************************/
static void
GetSmallestLevel(
  node_t *node,
  node_t **n1,
  node_t **n2)
{
int min1 = INFINITY, min2 = INFINITY, i;
node_t *n;

  for(i = 0;i < node_num_fanin(node);i++) {
    n = node_get_fanin(node, i);
    if(n->level < min1) {
      min2 = min1;
      *n2 = *n1;
      min1 = n->level;
      *n1 = n;
    }
    else 
      if(n->level < min2) {
        min2 = n->level;
        *n2 = n; 
      }
  }
}

