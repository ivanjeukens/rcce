/**CFile***********************************************************************

  FileName    [levmInt.c]

  PackageName [levm]

  Synopsis    []

  Description []

  SeeAlso     []

  Author      []

  Copyright   []

*******************************************************************************/

#include "levmInt.h"

/*
static char rcsid[] = \"$Id: $\";
USE(rcsid);
*/

/*---------------------------------------------------------------------------*/
/* Constant declarations                                                     */
/*---------------------------------------------------------------------------*/
#define FF 2

/*---------------------------------------------------------------------------*/
/* Variable declarations                                                     */
/*---------------------------------------------------------------------------*/
extern FILE *siserr;


/**AutomaticStart*************************************************************/

/*---------------------------------------------------------------------------*/
/* Static function prototypes                                                */
/*---------------------------------------------------------------------------*/

static int priority(char *c1, char *c2);
static node_t * merge_lut(node_t *n, st_table *Z);

/**AutomaticEnd***************************************************************/


/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis           [Core of the level map algorithm]

  Description        [optional]

  SideEffects        [required]

  SeeAlso            [optional]

******************************************************************************/
network_t *
levmRun(
  network_t *network,
  int k)
{
network_t *new_network;
array_t *aux, *children, *luts, *names;
int i, dep, j, z;
node_t *n, *fanin, *new, *n2;
char *value;
st_table *Z, *D, *processed, *fanin_names, *fanout_names;
char name[100];
lsGen gen, gen2;
st_generator *stgen;
char *key;

  aux = network_dfs(network);

  Z = st_init_table(st_ptrcmp, st_ptrhash);
  D = st_init_table(st_ptrcmp, st_ptrhash);

  luts = array_alloc(node_t *, 0);
  
  for(i = 0;i < array_n(aux);i++) {  
    n = array_fetch(node_t *, aux, i);
    if(node_type(n) == PRIMARY_INPUT) {
      st_insert(Z, (char *) n, (char *) 1);
      st_insert(D, (char *) n, (char *) 1);
    }
    else 
      if(node_type(n) == INTERNAL) {
        dep = 0;
        children = array_alloc(node_t *, 0);
        foreach_fanin(n, j, fanin) {
          st_lookup(Z, (char *) fanin, &value);
          dep += (int) value;
          fanin->map = value;
          if(node_type(fanin) != PRIMARY_INPUT) {
            array_insert_last(node_t *, children, fanin);
          }
        }        
        if(dep > k) {
          array_sort(children, priority);
          j = 0;
          z = 0;
          do {
            fanin = array_fetch(node_t *, children, j);
            fanin->map = 0;
            st_delete(Z, (char **) &fanin, &value);            
            st_insert(Z, (char *) fanin, (char *) 1);
            st_lookup(D, (char *) fanin, &value);
            array_insert_last(node_t *, luts, fanin);
            dep -= ((int) value - 1);
            j++;
          } while(dep > k);
        }
        st_insert(Z, (char *) n, (char *) dep);
        st_insert(D, (char *) n, (char *) dep);
        array_free(children);  
      }  
  }
  array_free(aux);
  st_free_table(D);

  foreach_primary_output(network, gen, n) {
    foreach_fanin(n, i, fanin) {
      if(node_type(fanin) != PRIMARY_INPUT) {
        for(j = 0;j < array_n(luts);j++) {
          n2 = array_fetch(node_t *, luts, j);
          if(strcmp(n2->name, fanin->name) == 0) break;
        }
        if(j == array_n(luts)) {
          array_insert_last(node_t *, luts, fanin);
        }
      }
    }  
  }      
  new_network = network_alloc();
  strcpy(name, network_name(network));
  strcat(name, "_mapped");
  network_set_name(new_network, name);

  processed = st_init_table(st_ptrcmp, st_ptrhash);
  fanin_names = st_init_table(st_ptrcmp, st_ptrhash);
  fanout_names = st_init_table(strcmp, st_strhash);
  
  for(dep = 0;dep < array_n(luts);dep++) {
    n = array_fetch(node_t *, luts, dep);    
    if(st_lookup(processed, (char *) n, &value) == 0) {
      new = merge_lut(n, Z);

      foreach_fanout(n, gen, fanin) {
        if(node_type(fanin) == PRIMARY_OUTPUT) {
          st_insert(fanout_names, util_strsav(node_long_name(new)),
            util_strsav(node_long_name(fanin)) );
          break; 
        }
      }            
      names = array_alloc(char *, 0);
      foreach_fanin(new, i, fanin) {
        array_insert_last(char *, names, util_strsav(node_long_name(fanin)));
      }  
      new->nin = 0;
      FREE(new->fanin);
      new->fanin = NIL(node_t *);
        
      value = (char *) names;
      st_insert(fanin_names, (char *) new, value);
      value = (char *) 1;
      st_insert(processed, (char *) n, value);
      network_add_node(new_network, new);
    }
  }
  array_free(luts);
  st_free_table(Z);

  aux = array_alloc(node_t *, 0);
  foreach_primary_input(network, gen, n) {
    new = node_dup(n);    
    network_add_primary_input(new_network, new);
    foreach_fanout(n, gen2, fanin) {
      if(node_type(fanin) == PRIMARY_OUTPUT) {
        n2 = node_alloc();
        n2->type = PRIMARY_OUTPUT;
        n2->nin = 1;
        n2->fanin = (node_t **) ALLOC(node_t *, 1);
        n2->fanin[0] = new;
        n2->name = util_strsav(fanin->name);
        array_insert_last(node_t *, aux, n2);
      }
    }
  }
  foreach_node(new_network, gen, n) {
    if(node_type(n) != PRIMARY_INPUT) {    
      if(st_lookup(fanin_names, (char *) n, &value) == 0) {
        fprintf(siserr,"Undefined new node!\n");
        network_free(new_network);
        return NIL(network_t);
      }
      names = (array_t *) value;
      for(i = 0;i < array_n(names);i++) {
        value = array_fetch(char *, names, i);
        fanin = network_find_node(new_network, value);
        if(fanin == NIL(node_t)) fprintf(stdout,"Fanin NULL!!!\n");
        n->nin++;
        n->fanin = (node_t **) REALLOC(node_t *, n->fanin, n->nin);
        n->fanin[n->nin - 1] = fanin;
      }
      fanin_add_fanout(n);
      
      if(st_lookup(fanout_names, node_long_name(n), &value) == 1) {
        new = node_alloc();
        new->type = PRIMARY_OUTPUT;
        new->nin = 1;
        new->fanin = (node_t **) ALLOC(node_t *, 1);
        new->fanin[0] = n;
        new->name = util_strsav(value);
        array_insert_last(node_t *, aux, new);
      }
    }
  }  
  for(i = 0;i < array_n(aux);i++) {
    new = array_fetch(node_t *, aux, i);
    network_add_node(new_network, new);
  }
  array_free(aux);
 
  st_free_table(processed);

  st_foreach_item(fanout_names, stgen, &key, &value) {
    FREE(key);
    FREE(value);
  }
  st_free_table(fanout_names);

  st_foreach_item(fanin_names, stgen, &key, &value) {
    names = (array_t *) value;
    for(i = 0;i < array_n(names);i++) {
      key = array_fetch(char *, names, i);
      FREE(key);
    }
    array_free(names);
  }
  st_free_table(fanin_names);  

  network_sweep(new_network);

  return new_network;
}

/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis           [Priority function used when ordering the children of
  the current inspected node.]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static int
priority(
  char *c1,
  char *c2)
{
node_t **n1, **n2;
float p1, p2;

  n1 = (node_t **) c1;
  n2 = (node_t **) c2;

  p1 = (int) (*n1)->map + FF*node_num_fanout((*n1));
  p2 = (int) (*n2)->map + FF*node_num_fanout((*n2));
  
  return ((int) p2 - p1);
}

/**Function********************************************************************

  Synopsis           [Collapse all nodes into a lut.]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static node_t *
merge_lut(
  node_t *n,
  st_table *Z)
{
node_t *fanin, *ret, *n2;
int i, j;
array_t *fanins;
char *value;

  if(node_type(n) == PRIMARY_OUTPUT) return NIL(node_t);  
  ret = node_dup(n);

  fanins = array_alloc(node_t *, 0);
  foreach_fanin(ret, i, fanin) {
    st_lookup(Z, (char *) fanin, &value);
    if((node_type(fanin) != PRIMARY_INPUT) && ((int) value != 1)) {
      array_insert_last(node_t *, fanins, fanin);
    }
  }  
  i = 0;
  while(i < array_n(fanins)) {
    fanin = array_fetch(node_t *, fanins, i);
    (void) node_collapse(ret, fanin);
    foreach_fanin(fanin, j, n2) {
      st_lookup(Z, (char *) n2, &value);
      if((node_type(n2) != PRIMARY_INPUT) && ((int) value != 1)) {
        array_insert_last(node_t *, fanins, n2);
      }      
    }
    i++;  
  }
  array_free(fanins);
    
  return ret;
}
