/**CFile***********************************************************************

  FileName    [dagmCom.c]

  PackageName [dagm]

  Synopsis    []

  Description []

  SeeAlso     []

  Author      [Ivan Jeukens]

  Copyright   [1996-1997 Ivan Jeukens]

******************************************************************************/

#include "dagmInt.h"
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

  Synopsis           [Dagmap command.]

  Description        []

  SideEffects        [required]

  SeeAlso            [optional]

  CommandName        [dagmap] 

  CommandSynopsis    [Execute the dagmap algorithm on a network composed
  of simple gates.]  

  CommandArguments   [-k lut_size -v -n]

  CommandDescription []

******************************************************************************/
int
dagm_Com(
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
      case 'k':{ 
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
  new_network = dagmRun(*network, k);

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

  Synopsis           [Initialize the dagm command.]

  Description        [optional]

  SideEffects        [required]

  SeeAlso            [optional]

******************************************************************************/
void
dagm_Init()
{
  COM_ADD_COMMAND_MAP("dagmap", dagm_Com);
}

/**Function********************************************************************

  Synopsis           [Dagmap cleanup procedure.]

  Description        [optional]

  SideEffects        [required]

  SeeAlso            [optional]

******************************************************************************/
void
dagm_End()
{


}

/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis           [Prints the dagmap command usage.]

  Description        [optional]

  SideEffects        [required]

  SeeAlso            [optional]

******************************************************************************/
static void
Usage()
{
  (void) fprintf(siserr,"usage: dagmap [-n -k lut]\n\n");
  (void) fprintf(siserr,"   -k lut \t Use K-Input Lookup Table\n");
  (void) fprintf(siserr,"   -n\t Write Netlist\n");
}

/**Function********************************************************************

  Synopsis           [Bind the network nodes to the corresponding k lut
  from the logic library.]

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
netl_pin_t *pin;
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
      new->in = array_alloc(netl_pin_t *, 0);
      new->mapout = array_alloc(char *, 0);
      new->out = array_alloc(netl_pin_t *, 0);
      new->comp = util_strsav(ll->label);
      m = 0;
      foreach_fanin(node, j, fan) {
        pin = (netl_pin_t *) ALLOC(netl_pin_t, 1);
        pin->label = util_strsav(node_name(fan));
        pin->net = NIL(netl_net_t);
        array_insert_last(netl_pin_t *, new->in, pin);

        pchar = array_fetch(char *, ll->in, m++);
        array_insert_last(char *, new->mapin, util_strsav(pchar));
      }    
      pin = (netl_pin_t *) ALLOC(netl_pin_t, 1);
      pin->label = util_strsav(node_name(node));
      pin->net = NIL(netl_net_t);        
      array_insert_last(netl_pin_t *, new->out, pin);
      
      pchar = array_fetch(char *, ll->out, 0);
      array_insert_last(char *, new->mapout, util_strsav(pchar));
      
      node->map = (char *) new;
    }
    else 
      if(node_type(node) == PRIMARY_INPUT) {
        new = (netl_cb_t *) ALLOC(netl_cb_t, 1);      
        new->comp = util_strsav(pad->label);
        new->mapout = array_alloc(char *, 0);
        new->out = array_alloc(netl_pin_t *, 0);

        pin = (netl_pin_t *) ALLOC(netl_pin_t, 1);
        pin->label = util_strsav(node_name(node));
        pin->net = NIL(netl_net_t);        
        array_insert_last(netl_pin_t *, new->out, pin);

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
          new->in = array_alloc(netl_pin_t *, 0);

          pin = (netl_pin_t *) ALLOC(netl_pin_t, 1);
          pin->label = util_strsav(node_name(node));
          pin->net = NIL(netl_net_t);                  
          array_insert_last(netl_pin_t *, new->in, pin);

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

  Synopsis           [Release the memory associated with a binded node.]

  Description        [optional]

  SideEffects        [required]

  SeeAlso            [optional]

******************************************************************************/
static void
BindFree(network_t *net)
{
netl_cb_t *new;
int i;
node_t *node;
lsGen gen;
netl_pin_t *pin;
char *pchar;

  foreach_node(net, gen, node) {
    if((node_type(node) == INTERNAL) && (node->map != 0)) {
      new = (netl_cb_t *) node->map;
      FREE(new->comp);
      if(new->in != NIL(array_t)) {
        for(i = 0;i < array_n(new->in);i++) {
          pin = array_fetch(netl_pin_t *, new->in, i);
          FREE(pin->label);
          FREE(pin);
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
          pin = array_fetch(netl_pin_t *, new->out, i);
          FREE(pin->label);
          FREE(pin);
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
    else
      if((node_type(node) == PRIMARY_OUTPUT) && (node->map != 0)) {
        new = (netl_cb_t *) node->map;
        FREE(new->comp);
        if(new->in != NIL(array_t)) {
          for(i = 0;i < array_n(new->in);i++) {
            pin = array_fetch(netl_pin_t *, new->in, i);
            FREE(pin->label);
            FREE(pin);
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
        FREE(new);
        node->map = 0;
      }
      else
        if((node_type(node) == PRIMARY_INPUT) && (node->map != 0)) {
          new = (netl_cb_t *) node->map;
          FREE(new->comp);
          if(new->out != NIL(array_t)) {
            for(i = 0;i < array_n(new->out);i++) {
              pin = array_fetch(netl_pin_t *, new->out, i);
              FREE(pin->label);
              FREE(pin);
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
