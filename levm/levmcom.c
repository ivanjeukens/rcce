/**CFile***********************************************************************

  FileName    [levmCom.c]

  PackageName [levm]

  Synopsis    []

  Description []

  SeeAlso     []

  Author      []

  Copyright   []

******************************************************************************/

#include "levmInt.h"
#include "intf.h"

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

static void Usage();
static void Bind(network_t *net, array_t *llib, int k);
static void BindFree(network_t *net);

/**AutomaticEnd***************************************************************/


/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis           [Levelmap command.]

  Description        []

  SideEffects        []

  SeeAlso            [optional]

  CommandName        [optional] 	   

  CommandSynopsis    [optional]  

  CommandArguments   [optional]  

  CommandDescription [optional]  

******************************************************************************/
int
levm_Com(
  network_t **network,
  array_t *llib,
  int argc,
  char **argv)
{
int k = 4, c;
network_t *new_network;
short nl = 0, vpr = 0;
char *ret;
char name[200];
  
  util_getopt_reset();
  while ((c = util_getopt(argc, argv, "vnk:")) != EOF) {
    switch (c) {
      case 'k': {
        if((k = atoi(util_optarg)) < 2) {
          Usage();
          return 1;
        }
      } break;
      case 'n': {
        nl = 1;
      } break;     
      case 'v': {
        vpr = 1;
      } break; 
      default:
        Usage();
        return 1;
    }
  }
  if((llib == NIL(array_t)) && (nl == 1)) {
    fprintf(siserr,"No logic library loaded.\n");
    return 1;
  }
  new_network = levmRun(*network, k);

  if(new_network != NIL(network_t)) {
    network_free(*network); 
    *network = new_network;
  }

  if(nl == 1) {
    Bind(*network, llib, k);
    ret = netl_ClblCreate(*network);
    BindFree(*network);
    if(ret != NIL(char)) {
      fprintf(siserr,"%s\n",ret);
      FREE(ret);
    }
  }

  if(vpr == 1) {
    sprintf(name,"%s.net",network_name(*network));
    netl_VprNetWrite(*network, name ,k);  
  }

  return 0;
}

/**Function********************************************************************

  Synopsis           [Print the usage of the levelmap command.]

  Description        [optional]

  SideEffects        [required]

  SeeAlso            [optional]

******************************************************************************/
void
levm_Init()
{
  COM_ADD_COMMAND_MAP("levmap", levm_Com);
}

/**Function********************************************************************

  Synopsis           [Print the usage of the levelmap command.]

  Description        [optional]

  SideEffects        [required]

  SeeAlso            [optional]

******************************************************************************/
void
levm_End()
{

}

/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis           [Print the usage of the levelmap command.]

  Description        [optional]

  SideEffects        [required]

  SeeAlso            [optional]

******************************************************************************/
static void
Usage()
{
  (void) fprintf(siserr,"usage: levmap [-v -n -k lut]\n\n");
  (void) fprintf(siserr,"\t-k lut  Use K-Input Lookup Table\n");
  (void) fprintf(siserr,"\t-n      Write Clblist\n");
  (void) fprintf(siserr,"\t-v      Write Vpr netlist\n");
}

/**Function********************************************************************

  Synopsis           [Bind the mapped network into a library component.]

  Description        [optional]

  SideEffects        [required]

  SeeAlso            [optional]

******************************************************************************/
static void
Bind(
  network_t *net,
  array_t *llib,
  int k)
{
int i, j, m;
llib_t *ll, *pad;
lsGen gen;
node_t *node, *fan;
netl_cb_t *new;
char *pchar;

  i = llib_KlutFind(llib, k);
  if(i == -1) {
    (void) fprintf(siserr,"Library does not contain a %d lookup table.\n",k);
    return;
  }
  ll = array_fetch(llib_t *, llib, i);
  i = llib_PadFind(llib);
  if(i == -1) {
    (void) fprintf(siserr,"Library does not contain a pad.\n");
    return;
  }
  pad = array_fetch(llib_t *, llib, i);
  
  foreach_node(net, gen, node) {
    if(node_type(node) == INTERNAL) {
      new = (netl_cb_t *) ALLOC(netl_cb_t, 1);
      new->mapin = array_alloc(char *, 0);
      new->in = array_alloc(char *, 0);
      new->mapout = array_alloc(char *, 0);
      new->out = array_alloc(char *, 0);
      new->comp = util_strsav(ll->label);
      m = 0;
      foreach_fanin(node, j, fan) {
        array_insert_last(char *, new->in, util_strsav(node_name(fan)));
        pchar = array_fetch(char *, ll->in, m++);
        array_insert_last(char *, new->mapin, util_strsav(pchar));
      }    
      array_insert_last(char *, new->out, util_strsav(node_name(node)));
      pchar = array_fetch(char *, ll->out, 0);
      array_insert_last(char *, new->mapout, util_strsav(pchar));      

      node->map = (char *) new;
    }
    else 
      if(node_type(node) == PRIMARY_INPUT) {
        new = (netl_cb_t *) ALLOC(netl_cb_t, 1);      
        new->comp = util_strsav(pad->label);
        new->mapout = array_alloc(char *, 0);
        new->out = array_alloc(char *, 0);
        array_insert_last(char *, new->out, util_strsav(node_name(node)));
        pchar = array_fetch(char *, pad->out, 0);
        array_insert_last(char *, new->mapout, util_strsav(pchar));      
        new->in = NIL(array_t);
        new->mapin = NIL(array_t);

        node->map = (char *) new;
      }
      else
        if(node_type(node) == PRIMARY_OUTPUT) {
          new = (netl_cb_t *) ALLOC(netl_cb_t, 1);
          new->comp = util_strsav(pad->label);
          new->mapin = array_alloc(char *, 0);
          new->in = array_alloc(char *, 0);
          array_insert_last(char *, new->in, util_strsav(node_name(node)));
          pchar = array_fetch(char *, pad->in, 0);
          array_insert_last(char *, new->mapin, util_strsav(pchar));      
          new->out = NIL(array_t);
          new->mapout = NIL(array_t);

          node->map = (char *) new;
        }
        else {
          fprintf(siserr,"Undefined node type.\n");
          return;
        }
  }
}

/**Function********************************************************************

  Synopsis           [Release the memory associated with a binding of a 
  component]

  Description        [optional]

  SideEffects        [required]

  SeeAlso            [optional]

******************************************************************************/
static void
BindFree(network_t *net)
{
netl_cb_t *new;
char *pchar;
int i;
node_t *node;
lsGen gen;

  foreach_node(net, gen, node) {
    if((node_type(node) == INTERNAL) && (node->map != 0)) {
      new = (netl_cb_t *) node->map;
      FREE(new->comp);
      if(new->in != NIL(array_t)) {
        for(i = 0;i < array_n(new->in);i++) {
          pchar = array_fetch(char *, new->in, i);
          FREE(pchar);
        }      
        array_free(new->in);
      }
      if(new->mapin != NIL(array_t)) {
        for(i = 0;i < array_n(new->mapin);i++) {
          pchar = array_fetch(char *, new->mapin, i);
          FREE(pchar);
        }      
        array_free(new->mapin);
      }
      if(new->out != NIL(array_t)) {
        for(i = 0;i < array_n(new->out);i++) {
          pchar = array_fetch(char *, new->out, i);
          FREE(pchar);
        }      
        array_free(new->out);
      }
      if(new->mapout != NIL(array_t)) {
        for(i = 0;i < array_n(new->mapout);i++) {
          pchar = array_fetch(char *, new->mapout, i);
          FREE(pchar);
        }      
        array_free(new->mapout);
      }
      FREE(new);
      node->map = 0;
    }  
  }
}
