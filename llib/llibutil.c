/**CFile***********************************************************************

  FileName    [llibUtil.c]

  PackageName [llib]

  Synopsis    [Functions for manipulating the llib_t structure.]

  Description []

  SeeAlso     []

  Author      [Ivan Jeukens]

  Copyright   [1996-1997 Ivan Jeukens]

******************************************************************************/

#include "llibInt.h"
#include "compInt.h"
#include "archInt.h"
#include "arch.h"
#include "intf.h"

/*
static char rcsid[] = \"$Id: $\";
USE(rcsid);
*/

/**AutomaticStart*************************************************************/

/*---------------------------------------------------------------------------*/
/* Static function prototypes                                                */
/*---------------------------------------------------------------------------*/

static array_t * CompIns(array_t *llib, comp_data_t *c, char *lib);

/**AutomaticEnd***************************************************************/


/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis           [Release the memory associated with an array of lib_t.]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
void
llib_Free(array_t *l)
{
llib_t *c;
int i, j;
char *pchar;

  if(l == NIL(array_t)) {
    return;
  }
  for(i = 0;i < array_n(l);i++) {
    c = array_fetch(llib_t *, l, i);
    FREE(c->label);
    FREE(c->lib);
    for(j = 0;j < array_n(c->in);j++) {
      pchar = array_fetch(char *, c->in, j);
      FREE(pchar);
    }
    array_free(c->in);
    for(j = 0;j < array_n(c->out);j++) {
      pchar = array_fetch(char *, c->out, j);
      FREE(pchar);
    }
    array_free(c->out);
    network_free(c->net);
    FREE(c);      
  }
  array_free(l);
}

/**Function********************************************************************

  Synopsis           [Find a K-Lut in a given library.]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
int
llib_KlutFind(
  array_t *llib,
  int k)
{
llib_t *ll;
int i, j;
lsGen gen;
node_t *node, *fanin;

  for(i = 0;i < array_n(llib);i++) {
    ll = array_fetch(llib_t *, llib, i);
    foreach_node(ll->net, gen, node) {
      foreach_fanin(node, j, fanin) {
       if((strcmp(node_name(fanin),"lut") == 0) && 
          ((node_num_fanin(node) - 1) == k)) {
          (void) lsFinish(gen);
          return i;
        }
      }
    }  
  }
  return -1;
}

/**Function********************************************************************

  Synopsis           [Create a logic library from an architecture graph.]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
array_t *
llib_Create(arch_graph_t *ag) 
{
int i;
arch_clabel_t *cl;
char *buf;
lsList l = NULL;
comp_data_t *c;
array_t *ret;

  if(ag == NIL(arch_graph_t)) {
    return NIL(array_t);
  }
  ret = array_alloc(llib_t *, 0);  
  for(i = 0;i < array_n(ag->labels);i++) {
    cl = array_fetch(arch_clabel_t *, ag->labels, i);
    buf = array_fetch(char *, ag->libs, cl->lib);
    if(l != NULL) {
      comp_LibFree(l);
    }
    l = comp_LibLoad(buf);
    c = comp_Find(l, cl->label);
    if((comp_Type(c) == COMP_LC) || (comp_Type(c) == COMP_RLC)) {
      ret = CompIns(ret, c, buf);
    }
  }
  return ret;
}

/**Function********************************************************************

  Synopsis           [Find a pad in a logic library.]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
int
llib_PadFind(array_t *llib)
{
int i;
llib_t *ll;
lsGen gen;
node_t *node;

  for(i = 0;i < array_n(llib);i++) {
    ll = array_fetch(llib_t *, llib, i);
    foreach_node(ll->net, gen, node) {
      if((node_num_fanin(node) == 1) && (node_type(node) == PRIMARY_OUTPUT)
          && (node_type(node_get_fanin(node,0)) == PRIMARY_INPUT)) {
        (void) lsFinish(gen);
        return i;
      }
    }
  }
  return -1;
}

/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis           [Insert a new component into the library.]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static array_t *
CompIns(
  array_t *llib,
  comp_data_t *c,
  char *lib) 
{
llib_t *new;
char *blif;
FILE *fp;
char cmd[22];
int i;
comp_pin_t *cptr;

  new = (llib_t *) ALLOC(llib_t, 1);
  new->label = util_strsav(c->label);
  new->lib = util_strsav(lib);
  new->net = NIL(network_t);
  new->in = array_alloc(char *, 0);
  new->out = array_alloc(char *, 0);
  for(i = 0;i < array_n(c->pins);i++) {
    cptr = array_fetch(comp_pin_t *, c->pins,i);
    if(((cptr->type == COMP_LINPUT) || (cptr->type == COMP_SHAREDI)) &&
        (strcmp(cptr->label, "lut") != 0)) {
      array_insert_last(char *, new->in, util_strsav(cptr->label));
    }
    else
      if((cptr->type == COMP_LOUTPUT) || (cptr->type == COMP_SHAREDO)) {
        array_insert_last(char *, new->out, util_strsav(cptr->label));      
      }  
  }
  blif = comp_BlifExtr(c);  
  fp = fopen("/tmp/llibtmp","w");
  fprintf(fp,"%s",blif);
  (void) fclose(fp);
  
  strcpy(cmd,"read_blif /tmp/llibtmp");
  intf_ComSis(&new->net, cmd);

  system("rm /tmp/llibtmp");
  FREE(blif);
  
  if(llib == NIL(array_t)) {
    llib = array_alloc(llib_t *, 0);
  }
  array_insert_last(llib_t *, llib, new);

  return llib;
}

/**Function********************************************************************

  Synopsis           [Print a logic library.]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static void
Print(array_t *llib)
{
int i;
llib_t *ll;

  if(llib == NIL(array_t)) {
    return;
  }
  for(i = 0;i < array_n(llib);i++) {
    ll = array_fetch(llib_t *, llib, i);
    printf("%s %s\n",ll->label, ll->lib);
    
  }
}
