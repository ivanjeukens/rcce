/**CFile***********************************************************************

  FileName    [dagmInt.c]

  PackageName [dagm]

  Synopsis    [Core of the dagmap algorithm.]

  Description []

  SeeAlso     []

  Author      [1996-1997 Ivan Jeukens]

  Copyright   []

******************************************************************************/

#include "dagmInt.h"

/*
static char rcsid[] = \"$Id: $\";
USE(rcsid);
*/

/*---------------------------------------------------------------------------*/
/* Variable declarations                                                     */
/*---------------------------------------------------------------------------*/
extern FILE *siserr;

/**AutomaticStart*************************************************************/

/*---------------------------------------------------------------------------*/
/* Static function prototypes                                                */
/*---------------------------------------------------------------------------*/

static int GetPredNumber(node_t *n, int level, st_table *visited);
static node_t * MergeLevel(node_t *n);
static node_t * nonpi(array_t *L, int *curpos);

/**AutomaticEnd***************************************************************/


/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis           [required]

  Description        [optional]

  SideEffects        [required]

  SeeAlso            [optional]

******************************************************************************/
network_t *
dagmRun(
  network_t *network,
  int k)
{
array_t *aux, *names;
int i, level, j, number, curpos = 0;
node_t *n, *fanin, *new, *n2;
lsGen gen, gen2;
network_t *new_network;
char name[248], *value, *key;
st_table *processed, *fanin_names, *fanout_names;
st_generator *stgen;

  network_level(network);
  aux = network_dfs(network);
    
  for(i = 0;i < array_n(aux);i++) {  
    n = array_fetch(node_t *, aux, i);
    if(node_num_fanin(n) > 2) {
      fprintf(siserr,"Network must be 2-bounded!\n");
      return NIL(network_t);
    }
    level = 0;
    foreach_fanin(n, j, fanin) {
      if(fanin->level > level)
        level = fanin->level;
    }
    processed = st_init_table(strcmp, st_strhash);    
    number = GetPredNumber(n, level, processed);
    st_foreach_item(processed, stgen, &key, &value) {
      FREE(key);
    }    
    st_free_table(processed);    
    if(number > k) {
      n->level = level + 1;
    }
    else {
      n->level = level;    
    }
  }  
  array_free(aux);
   
  aux = array_alloc(node_t *, 0);
  foreach_primary_output(network, gen, n) {
    array_insert_last(node_t *, aux, n);
  }
 
  new_network = network_alloc();
  strcpy(name, network_name(network));
  strcat(name, "_mapped");
  network_set_name(new_network, name);
  processed = st_init_table(strcmp, st_strhash);
  fanin_names = st_init_table(strcmp, st_strhash);
  fanout_names = st_init_table(strcmp, st_strhash);

  n = nonpi(aux, &curpos);
  while(n != NIL(node_t)) {
    if(st_lookup(processed, node_long_name(n), &value) == 0) {
      new = MergeLevel(n);

      foreach_fanout(n, gen, fanin) {
        if(node_type(fanin) == PRIMARY_OUTPUT) {
          st_insert(fanout_names, util_strsav(node_long_name(new)),
            util_strsav(node_long_name(fanin)) );
          break;
        }
      }
            
      if(node_type(n) == PRIMARY_OUTPUT) {
        array_insert_last(node_t *, aux, n->fanin[0]);        
      }
      else {
        names = array_alloc(char *, 0);
        foreach_fanin(new, i, fanin) {
          array_insert_last(node_t *, aux, fanin);
          array_insert_last(char *, names, util_strsav(node_long_name(fanin)));
        }  
        new->nin = 0;
        FREE(new->fanin);
        new->fanin = NIL(node_t *);
        
        value = (char *) names;
        st_insert(fanin_names, util_strsav(node_long_name(new)), value);
        value = (char *) 1;
        st_insert(processed, util_strsav(node_long_name(n)), value);
        network_add_node(new_network, new);
      }
    }
    n = nonpi(aux, &curpos);
  }  
  array_free(aux);

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
      if(st_lookup(fanin_names, node_long_name(n), &value) == 0) {
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
 
  st_foreach_item(processed, stgen, &key, &value) {
    FREE(key);
  }
  st_free_table(processed);

  st_foreach_item(fanout_names, stgen, &key, &value) {
    FREE(key);
    FREE(value);
  }
  st_free_table(fanout_names);

  st_foreach_item(fanin_names, stgen, &key, &value) {
    FREE(key);
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

  Synopsis           [required]

  Description        [optional]

  SideEffects        [required]

  SeeAlso            [optional]

******************************************************************************/
static int
GetPredNumber(
  node_t *n,
  int level,
  st_table *visited)
{
int ret = 0, i;
node_t *fanin;
char *value;

  if(node_num_fanin(n) == 0) {
    if(st_lookup(visited, node_long_name(n), &value) == 0) {
      st_insert(visited, util_strsav(node_long_name(n)), value);
      return 1;
    }
    else {
      return 0;    
    }
  }
  foreach_fanin(n, i, fanin) {
    if((fanin->level != level) && (st_lookup(visited, node_long_name(fanin), 
      &value) == 0)) {
      ret++;
      st_insert(visited, util_strsav(node_long_name(n)), value);      
    }
    else {
      ret += GetPredNumber(fanin, level, visited);
    }
  }    
  return ret;
}

/**Function********************************************************************

  Synopsis           [required]

  Description        [optional]

  SideEffects        [required]

  SeeAlso            [optional]

******************************************************************************/
static node_t *
MergeLevel(node_t *n)
{
node_t *fanin, *ret, *n2;
int i, j;
array_t *fanins;

  if(node_type(n) == PRIMARY_OUTPUT) {
    return NIL(node_t);  
  }
  ret = node_dup(n);

  fanins = array_alloc(node_t *, 0);
  foreach_fanin(ret, i, fanin) {
    if((node_type(fanin) != PRIMARY_INPUT) && (ret->level == fanin->level)) {
      array_insert_last(node_t *, fanins, fanin);
    }
  }  
  i = 0;
  while(i < array_n(fanins)) {
    fanin = array_fetch(node_t *, fanins, i);
    (void) node_collapse(ret, fanin);
    foreach_fanin(fanin, j, n2) {
      if((node_type(n2) != PRIMARY_INPUT) && (ret->level == n2->level)) {
        array_insert_last(node_t *, fanins, n2);
      }      
    }
    i++;  
  }
  array_free(fanins);
    
  return ret;
}

/**Function********************************************************************

  Synopsis           [required]

  Description        [optional]

  SideEffects        [required]

  SeeAlso            [optional]

******************************************************************************/
static node_t *
nonpi(
  array_t *L,
  int *curpos)
{
int i;
node_t *n;

  for(i = *curpos;i < array_n(L);i++) {
    n = array_fetch(node_t *, L, i);
    if((n != NIL(node_t)) && (node_type(n) != PRIMARY_INPUT)) {
      array_insert(node_t *, L, i, NIL(node_t));
      return n; 
    }
  }
  *curpos = i;
  return NIL(node_t);
}
