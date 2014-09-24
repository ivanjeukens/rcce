/**CFile***********************************************************************

  FileName    [netlUtil.c]

  PackageName [netl]

  Synopsis    []

  Description []

  SeeAlso     []

  Author      []

  Copyright   [1996-1997 Ivan Jeukens]

******************************************************************************/

#include "netlInt.h"

/*
static char rcsid[] = \"$Id: $\";
USE(rcsid);
*/


/*---------------------------------------------------------------------------*/
/* Variable declarations                                                     */
/*---------------------------------------------------------------------------*/
extern FILE *msgerr;


/**AutomaticStart*************************************************************/

/*---------------------------------------------------------------------------*/
/* Static function prototypes                                                */
/*---------------------------------------------------------------------------*/

static void DuplicatePins(netl_clbl_t *clbl);
static int ClbCmpPos(char *o1, char *o2);
static int ClbCmpLabel(char *o1, char *o2);
static int NetlCmp(char *o1, char *o2);

/**AutomaticEnd***************************************************************/


/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis           [Release the memory associated with a clb_t list.]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
void
netl_ClblFree(netl_clbl_t *clbl)
{
int i, k;
netl_clb_t *c;
netl_cb_t *cb;
netl_pin_t *pin;
char *pchar;

  if(clbl == NIL(netl_clbl_t)) {
    return;
  }
  FREE(clbl->name);
  
  for(i = 0;i < array_n(clbl->inputs);i++) {
    c = array_fetch(netl_clb_t *, clbl->inputs, i);
    FREE(c->label);
    cb = c->cb;
    for(k = 0;k < array_n(cb->mapout);k++) {
      pchar = array_fetch(char *, cb->mapout, k);
      FREE(pchar);
    }
    array_free(cb->mapout);
    for(k = 0;k < array_n(cb->out);k++) {
      pin = array_fetch(netl_pin_t *, cb->out, k);
      FREE(pin->label);
      FREE(pin);
    }
    array_free(cb->out);
    
    array_free(cb->in);
    array_free(cb->mapin);
    
    FREE(cb->comp);
    FREE(cb);
    FREE(c);  
  }  
  array_free(clbl->inputs);
  for(i = 0;i < array_n(clbl->outputs);i++) {
    c = array_fetch(netl_clb_t *, clbl->outputs, i);
    FREE(c->label);
    cb = c->cb;
    for(k = 0;k < array_n(cb->mapin);k++) {
      pchar = array_fetch(char *, cb->mapin, k);
      FREE(pchar);
    }
    array_free(cb->mapin);
    for(k = 0;k < array_n(cb->in);k++) {
      pin = array_fetch(netl_pin_t *, cb->in, k);
      FREE(pin->label);
      FREE(pin);
    }
    array_free(cb->in);
    
    array_free(cb->out);
    array_free(cb->mapout);
    
    FREE(cb->comp);
    FREE(cb);
    FREE(c);  
  }
  array_free(clbl->outputs);  
  for(i = 0;i < array_n(clbl->clbs);i++) {
    c = array_fetch(netl_clb_t *, clbl->clbs, i);
    FREE(c->label);
    cb = c->cb;
    for(k = 0;k < array_n(cb->mapin);k++) {
      pchar = array_fetch(char *, cb->mapin, k);
      FREE(pchar);
    }
    array_free(cb->mapin);
    for(k = 0;k < array_n(cb->in);k++) {
      pin = array_fetch(netl_pin_t *, cb->in, k);
      FREE(pin);
    }
    array_free(cb->in);
    for(k = 0;k < array_n(cb->mapout);k++) {
      pchar = array_fetch(char *, cb->mapout, k);
      FREE(pchar);
    }
    array_free(cb->mapout);
    for(k = 0;k < array_n(cb->out);k++) {
      pin = array_fetch(netl_pin_t *, cb->out, k);
      FREE(pin->label);
      FREE(pin);
    }
    array_free(cb->out);
    FREE(cb->comp);
    FREE(cb);
    FREE(c);  
  }
  array_free(clbl->clbs);  
  for(i = 0;i < array_n(clbl->globals);i++) {
    pchar = array_fetch(char *, clbl->globals, i);
    FREE(pchar);  
  }
  array_free(clbl->globals);

  FREE(clbl);
}  

/**Function********************************************************************

  Synopsis           [Release the memory associated with a netlist.]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
void
netl_Free(array_t *netl)
{
int i, j;
netl_net_t *nl;
netl_signet_t *sn;

  for(i = 0;i < array_n(netl);i++) {
    nl = array_fetch(netl_net_t *, netl, i);
    FREE(nl->name);
    for(j = 0;j < array_n(nl->snet);j++) {
      sn = array_fetch(netl_signet_t *, nl->snet, j);
      FREE(sn);
    }
    array_free(nl->snet);
    FREE(nl);
  }
  array_free(netl);
}

/**Function********************************************************************

  Synopsis           [Find a clb based on it the label]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
int
netl_ClblFindLabel(
  array_t *clbs,
  char *label)
{
int low, high, i, res;
netl_clb_t *c;

  low = 0;
  high = array_n(clbs) - 1;
  i = (high + low)/2;
  
  while(low <= high) {
    c = array_fetch(netl_clb_t *, clbs, i);
    res = strcmp(label, c->label);
    if(res == 0) {
      return i;
    }
    else
      if(res > 0) {
        low = i + 1;
      }
      else {
        high = i - 1;
      }
    i = (low + high)/2;
  }

  return -1;
}

/**Function********************************************************************

  Synopsis           [Copy a clb list.]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
netl_clbl_t *
netl_ClblCopy(netl_clbl_t *clbl)
{
netl_clbl_t *new;
register int i, j;
netl_clb_t *clb, *cnew;
char *pchar;
netl_pin_t *pin, *npin;

  if(clbl == NIL(netl_clbl_t)) {
    return NIL(netl_clbl_t);
  }  
  new = (netl_clbl_t *) ALLOC(netl_clbl_t, 1);
  new->name = util_strsav(clbl->name);
  new->inputs = array_alloc(netl_clb_t *, 0);
  new->outputs = array_alloc(netl_clb_t *, 0);
  new->clbs = array_alloc(netl_clb_t *, 0);    
  new->globals = array_alloc(char *, 0);
  for(i = 0;i < array_n(clbl->inputs);i++) {
    clb = array_fetch(netl_clb_t *, clbl->inputs, i);
    cnew = (netl_clb_t *) ALLOC(netl_clb_t, 1);
    cnew->label = util_strsav(clb->label);
    cnew->x = clb->x;
    cnew->y = clb->y;
    cnew->z = clb->z;
    cnew->nio = clb->nio;
    cnew->noswap = clb->noswap;
    cnew->cb = (netl_cb_t *) ALLOC(netl_cb_t, 1);

    cnew->cb->in = array_alloc(netl_pin_t *, 0);
    cnew->cb->mapin = array_alloc(char *, 0);
    
    cnew->cb->mapout = array_alloc(char *, 0);
    for(j = 0;j < array_n(clb->cb->mapout);j++) {
      pchar = array_fetch(char *, clb->cb->mapout, j);
      array_insert_last(char *, cnew->cb->mapout, util_strsav(pchar));
    }    

    cnew->cb->out = array_alloc(netl_pin_t *, 0);
    for(j = 0;j < array_n(clb->cb->out);j++) {
      pin = array_fetch(netl_pin_t *, clb->cb->out, j);
      npin = (netl_pin_t *) ALLOC(netl_pin_t, 1);
      npin->label = util_strsav(pin->label);
      npin->net = NIL(netl_net_t);
      npin->skip = 0;
      array_insert_last(netl_pin_t *, cnew->cb->out, npin);
    }     
    cnew->cb->comp = util_strsav(clb->cb->comp);      
    array_insert_last(netl_clb_t *, new->inputs, cnew);
  }
  
  for(i = 0;i < array_n(clbl->outputs);i++) {
    clb = array_fetch(netl_clb_t *, clbl->outputs, i);
    cnew = (netl_clb_t *) ALLOC(netl_clb_t, 1);
    cnew->label = util_strsav(clb->label);
    cnew->x = clb->x;
    cnew->y = clb->y;
    cnew->z = clb->z;
    cnew->nio = clb->nio;
    cnew->noswap = clb->noswap;
    cnew->cb = (netl_cb_t *) ALLOC(netl_cb_t, 1);
    cnew->cb->mapout = array_alloc(char *, 0);
    cnew->cb->out = array_alloc(netl_pin_t *, 0);

    cnew->cb->mapin = array_alloc(char *, 0);
    for(j = 0;j < array_n(clb->cb->mapin);j++) {
      pchar = array_fetch(char *, clb->cb->mapin, j);
      array_insert_last(char *, cnew->cb->mapin, util_strsav(pchar));
    }

    cnew->cb->in = array_alloc(netl_pin_t *, 0);
    for(j = 0;j < array_n(clb->cb->in);j++) {
      pin = array_fetch(netl_pin_t *, clb->cb->in, j);
      npin = (netl_pin_t *) ALLOC(netl_pin_t, 1);
      npin->label = util_strsav(pin->label);
      npin->net = NIL(netl_net_t);
      npin->skip = 0;
      array_insert_last(netl_pin_t *, cnew->cb->in, npin);
    }    
    cnew->cb->comp = util_strsav(clb->cb->comp);      
    array_insert_last(netl_clb_t *, new->outputs, cnew);
  }
    
  for(i = 0;i < array_n(clbl->clbs); i++) {
    clb = array_fetch(netl_clb_t *, clbl->clbs, i);
    cnew = (netl_clb_t *) ALLOC(netl_clb_t, 1);
    cnew->label = util_strsav(clb->label);
    cnew->x = clb->x;
    cnew->y = clb->y;
    cnew->z = clb->z;
    cnew->nio = clb->nio;
    cnew->noswap = clb->noswap;
    cnew->cb = (netl_cb_t *) ALLOC(netl_cb_t, 1);

    cnew->cb->mapin = array_alloc(char *, 0);
    for(j = 0;j < array_n(clb->cb->mapin);j++) {
      pchar = array_fetch(char *, clb->cb->mapin, j);
      array_insert_last(char *, cnew->cb->mapin, util_strsav(pchar));
    }
    cnew->cb->in = array_alloc(netl_pin_t *, 0);
    for(j = 0;j < array_n(clb->cb->in);j++) {
      pin = array_fetch(netl_pin_t *, clb->cb->in, j);
      npin = (netl_pin_t *) ALLOC(netl_pin_t, 1);
      npin->label = util_strsav(pin->label);
      npin->net = NIL(netl_net_t);
      npin->skip = 0;
      array_insert_last(netl_pin_t *, cnew->cb->in, npin);
    }    
    cnew->cb->mapout = array_alloc(char *, 0);
    for(j = 0;j < array_n(clb->cb->mapout);j++) {
      pchar = array_fetch(char *, clb->cb->mapout, j);
      array_insert_last(char *, cnew->cb->mapout, util_strsav(pchar));
    }    
    cnew->cb->out = array_alloc(netl_pin_t *, 0);
    for(j = 0;j < array_n(clb->cb->out);j++) {
      pin = array_fetch(netl_pin_t *, clb->cb->out, j);
      npin = (netl_pin_t *) ALLOC(netl_pin_t, 1);
      npin->label = util_strsav(pin->label);
      npin->net = NIL(netl_net_t);
      array_insert_last(netl_pin_t *, cnew->cb->out, npin);
    }    
    cnew->cb->comp = util_strsav(clb->cb->comp);      
    array_insert_last(netl_clb_t *, new->clbs, cnew);  
  }

  for(i = 0;i < array_n(clbl->globals);i++) {
    pchar = array_fetch(char *, clbl->globals, i);
    array_insert_last(char *, new->globals, util_strsav(pchar));  
  }
  return new;
}

/**Function********************************************************************

  Synopsis           [Create a clb list based on the given mapped network.]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
char *
netl_ClblCreate(network_t *net)
{
FILE *fp;
char pchar[80], *ret;
lsGen gen, genout;
node_t *node, *fanout;
int i, bs = 0;
netl_cb_t *cb;
netl_pin_t *pin, *pin2;
char *pc, **bufin = NIL(char *), **bufout = NIL(char *);
  
  strcpy(pchar, network_name(net));
  strcat(pchar,".mapped");
  fp = fopen(pchar, "w");
  
  if(fp == NIL(FILE)) {
    ret = (char *) ALLOC(char, strlen(pchar) + 34);
    sprintf(ret,"Couldn't open file %s for writing.\n", pchar);
    return ret;
  }    
  fprintf(fp,"Clblist %s\n", network_name(net));
  
  foreach_primary_input(net, gen, node) {
    if(node->map == 0) {
      ret = (char *) ALLOC(char, 21);
      sprintf(ret,"Found unmapped node!\n");
      return ret;    
    }
    cb = (netl_cb_t *) node->map;
    fprintf(fp,"input %s %s -1 -1 -1 -1\n", cb->comp, node_name(node));
    fprintf(fp,"fanin open open\n");
    pc = array_fetch(char *, cb->mapout, 0);
    pin2 = array_fetch(netl_pin_t *, cb->out, 0);
    fprintf(fp,"fanout %s %s\n\n", pc, pin2->label);
    foreach_fanout(node, genout, fanout) {
      if(node_type(fanout) == PRIMARY_OUTPUT) {
        bs++;
        bufin = (char **) REALLOC(char *, bufin, bs);
        bufout = (char **) REALLOC(char *, bufout, bs);
        bufin[bs - 1] = util_strsav(node_name(node));
        bufout[bs - 1] = util_strsav(node_name(fanout));      
      }
    }
  }
  
  fprintf(fp,"\n");  
  foreach_primary_output(net, gen, node) {
    if(node->map == 0) {
      ret = (char *) ALLOC(char, 21);
      sprintf(ret,"Found unmapped node!\n");
      return ret;    
    }
    cb = (netl_cb_t *) node->map;
    fprintf(fp,"output %s %s_out -1 -1 -1 -1\n", cb->comp, node_name(node));
    pc = array_fetch(char *, cb->mapin, 0);
    pin2 = array_fetch(netl_pin_t *, cb->in, 0);
    fprintf(fp,"fanin %s %s\n", pc, pin2->label);
    fprintf(fp,"fanout open open\n\n");
  }
  fprintf(fp,"\n");
  
  while(bs > 0) {
    fprintf(fp,"clb BUF %s -1 -1 -1\n", bufout[bs - 1]);
    fprintf(fp,"fanin INPUT %s\n", bufin[bs - 1]);
    fprintf(fp,"fanout OUTPUT %s\n\n", bufout[bs - 1]);    
    FREE(bufout[bs - 1]);
    FREE(bufin[bs - 1]);
    bs--;
  }
  if(bufout != NIL(char *)) {
    FREE(bufout);
    FREE(bufin);
  }
    
  foreach_node(net, gen, node) {
    if(node_type(node) == INTERNAL) {
      if(node->map == 0) {
        ret = (char *) ALLOC(char, 21);
        sprintf(ret,"Found unmapped node!\n");
        return ret;
      }
      cb = (netl_cb_t *) node->map;
      fprintf(fp,"clb %s %s -1 -1 -1\n", cb->comp, node_name(node));
      for(i = 0;i < array_n(cb->in);i++) {
        pin = array_fetch(netl_pin_t *, cb->in, i);
        pc = array_fetch(char *, cb->mapin, i);
        fprintf(fp,"fanin %s %s\n", pc, pin->label);
      }      
      for(i = 0;i < array_n(cb->out);i++) {
        pin = array_fetch(netl_pin_t *, cb->out, i);
        pc = array_fetch(char *, cb->mapout, i);
        fprintf(fp,"fanout %s %s\n", pc, pin->label);
      } 
      fprintf(fp,"\n");
    }
  }  
  fprintf(fp,"end\n");
    
  (void) fclose(fp);
  return NIL(char);
}

/**Function********************************************************************

  Synopsis           [Initialize the netl package.]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
void
netl_Init()
{
  COM_ADD_COMMAND_NETL("clbl_load", netl_ClblLoadCmd);
  COM_ADD_COMMAND_NETL("vprnet_load", netl_VprNetLoadCmd);  
  COM_ADD_COMMAND_SIS("vprnet_save", netl_VprNetWriteCmd, "misc");
}

/**Function********************************************************************

  Synopsis           [Exit functions.]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
void
netl_End()
{


}

/**Function********************************************************************

  Synopsis           [The command interface for the clbl load procedure]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
int
netl_ClblLoadCmd(
  netl_clbl_t **clbl,
  int argc,
  char **argv)
{
int ret;

  ret = netl_ClblLoad(clbl, argv[1]);

  return ret;
}

/**Function********************************************************************

  Synopsis           [The command interface for the vpr netlist load procedure]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
int
netl_VprNetLoadCmd(
  netl_clbl_t **clbl,
  int argc,
  char **argv)
{
int ret;

  ret = netl_VprNetLoad(clbl, argv[1]);

  return ret;
}

/**Function********************************************************************

  Synopsis           [The command interface for the vpr netlist load procedure]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
int
netl_VprNetWriteCmd(
  network_t **network,
  int argc,
  char **argv)
{

  if(argc < 3) return 0;
  
  netl_VprNetWrite(*network, argv[1], atoi(argv[2]));

  return 1;
}


/**Function********************************************************************

  Synopsis           [Load a clb list from a file.]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
int
netl_ClblLoad(
  netl_clbl_t **clbl,
  char *fname)
{
FILE *fp;
char buf[100];
netl_clb_t *c;
netl_cb_t *cb;
netl_pin_t *pin;

  fp = fopen(fname,"r");
  if(fp == NIL(FILE)) {
    return 1;
  }
  (void) fscanf(fp,"%s",buf);
  if(strcmp(buf,"Clblist") == 0) {
    (*clbl) = (netl_clbl_t *) ALLOC(netl_clbl_t, 1);    
    (void) fscanf(fp,"%s",buf);
    (*clbl)->name = util_strsav(buf);
    (*clbl)->inputs = array_alloc(netl_clb_t *, 0);
    (*clbl)->outputs = array_alloc(netl_clb_t *, 0);
    (*clbl)->clbs = array_alloc(netl_clb_t *, 0);
    (*clbl)->globals = array_alloc(char *, 0);
    (void) fscanf(fp,"%s",buf);
    while(strcmp(buf, "input") == 0) {
      c = (netl_clb_t *) ALLOC(netl_clb_t, 1);
      cb = (netl_cb_t *) ALLOC(netl_cb_t, 1);
      cb->mapin = array_alloc(char *, 0);
      cb->in = array_alloc(netl_pin_t *, 0);
      cb->out = array_alloc(netl_pin_t *, 0);
      cb->mapout = array_alloc(char *, 0);
      (void) fscanf(fp,"%s",buf);
      cb->comp = util_strsav(buf);
      (void) fscanf(fp,"%s",buf);
      c->label = util_strsav(buf);
      (void) fscanf(fp,"%s",buf);
      c->x = atoi(buf);
      (void) fscanf(fp,"%s",buf);
      c->y = atoi(buf);
      (void) fscanf(fp,"%s",buf);
      c->z = atoi(buf);
      (void) fscanf(fp,"%s",buf);
      c->nio = atoi(buf);
      c->noswap = 0;
      (void) fscanf(fp,"%s",buf);
      (void) fscanf(fp,"%s",buf);
      (void) fscanf(fp,"%s",buf);
      (void) fscanf(fp,"%s",buf);
      (void) fscanf(fp,"%s",buf);
      array_insert_last(char *, cb->mapout, util_strsav(buf));
      (void) fscanf(fp,"%s",buf);                              
      pin = (netl_pin_t *) ALLOC(netl_pin_t, 1);
      pin->net = NIL(netl_net_t);
      pin->label = util_strsav(buf);
      pin->skip = 0;
      array_insert_last(netl_pin_t *, cb->out, pin);
      c->cb = cb;
      array_insert_last(netl_clb_t *, (*clbl)->inputs, c);
      (void) fscanf(fp,"%s",buf);      
    }
    while(strcmp(buf,"output") == 0) {
      c = (netl_clb_t *) ALLOC(netl_clb_t, 1);
      cb = (netl_cb_t *) ALLOC(netl_cb_t, 1);
      cb->mapout = array_alloc(char *, 0);
      cb->out = array_alloc(netl_pin_t *, 0);
      cb->in = array_alloc(netl_pin_t *, 0);
      cb->mapin = array_alloc(char *, 0);
      (void) fscanf(fp,"%s",buf);   
      cb->comp = util_strsav(buf);
      (void) fscanf(fp,"%s",buf);
      c->label = util_strsav(buf);
      (void) fscanf(fp,"%s",buf);
      c->x = atoi(buf);
      (void) fscanf(fp,"%s",buf);
      c->y = atoi(buf);
      (void) fscanf(fp,"%s",buf);
      c->z = atoi(buf);
      (void) fscanf(fp,"%s",buf);
      c->nio = atoi(buf);
      c->noswap = 0;      
      (void) fscanf(fp,"%s",buf);
      (void) fscanf(fp,"%s",buf);
      array_insert_last(char *, cb->mapin, util_strsav(buf));
      (void) fscanf(fp,"%s",buf);
      pin = (netl_pin_t *) ALLOC(netl_pin_t, 1);
      pin->label = util_strsav(buf);
      pin->net = NIL(netl_net_t);
      pin->skip = 0;
      array_insert_last(netl_pin_t *, cb->in, pin);
      (void) fscanf(fp,"%s",buf);
      (void) fscanf(fp,"%s",buf);
      (void) fscanf(fp,"%s",buf);
      c->cb = cb;
      array_insert_last(netl_clb_t *, (*clbl)->outputs, c);
      (void) fscanf(fp,"%s",buf);      
    }
    while(strcmp(buf,"end") != 0) {
      c = (netl_clb_t *) ALLOC(netl_clb_t, 1);
      cb = (netl_cb_t *) ALLOC(netl_cb_t, 1); 
      cb->mapin = array_alloc(char *, 0);
      cb->in = array_alloc(netl_pin_t *, 0);
      cb->out = array_alloc(char *, 0);
      cb->mapout = array_alloc(netl_pin_t *, 0);     
      (void) fscanf(fp,"%s",buf);
      cb->comp = util_strsav(buf);
      (void) fscanf(fp,"%s",buf);      
      c->label = util_strsav(buf);
      (void) fscanf(fp,"%s",buf);
      c->x = atoi(buf);
      (void) fscanf(fp,"%s",buf);
      c->y = atoi(buf);
      (void) fscanf(fp,"%s",buf);
      c->z = atoi(buf);
      c->nio = -1;
      c->noswap = 0;      
      (void) fscanf(fp,"%s",buf);
      while(strcmp(buf,"fanin") == 0) {
        (void) fscanf(fp,"%s",buf);
        array_insert_last(char *, cb->mapin, util_strsav(buf));
        (void) fscanf(fp,"%s",buf);
        pin = (netl_pin_t *) ALLOC(netl_pin_t, 1);
        pin->net = NIL(netl_net_t);
        pin->label = util_strsav(buf);
        pin->skip = 0;
        array_insert_last(netl_pin_t *, cb->in, pin);
        (void) fscanf(fp,"%s",buf);
      }
      while(strcmp(buf,"fanout") == 0) {        
        (void) fscanf(fp,"%s",buf);
        array_insert_last(char *, cb->mapout, util_strsav(buf));
        (void) fscanf(fp,"%s",buf);
        pin = (netl_pin_t *) ALLOC(netl_pin_t, 1);
        pin->net = NIL(netl_net_t);
        pin->label = util_strsav(buf);
        pin->skip = 0;
        array_insert_last(netl_pin_t *, cb->out, pin);
        (void) fscanf(fp,"%s",buf);      
      }
      c->cb = cb;
      array_insert_last(netl_clb_t *, (*clbl)->clbs, c);
    }
  }  
  (void) fclose(fp);
  array_sort((*clbl)->inputs, ClbCmpLabel);
  array_sort((*clbl)->clbs, ClbCmpLabel);
  array_sort((*clbl)->outputs, ClbCmpLabel);
  
  return 0;
}

/**Function********************************************************************

  Synopsis           [Write the given clb list into a file.]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
void
netl_ClblSave(
  netl_clbl_t *clbl,
  char *fname)
{
FILE *fp;
int i, j;
netl_clb_t *c;
netl_pin_t *pin2;
char *pchar;

  if(clbl == NIL(netl_clbl_t)) return;
  
  fp = fopen(fname,"w");
  if(fp == NIL(FILE)) {
    return;
  }
  fprintf(fp,"Clblist %s\n", clbl->name);  
  for(i = 0;i < array_n(clbl->inputs);i++) {
    c = array_fetch(netl_clb_t *, clbl->inputs, i);
    fprintf(fp,"input %s %s %d %d %d %d\n",c->cb->comp, c->label, c->x,
      c->y, c->z, c->nio);
    fprintf(fp,"fanin open open\n");
    pchar = array_fetch(char *, c->cb->mapout, 0);
    pin2 = array_fetch(netl_pin_t *, c->cb->out, 0);
    fprintf(fp,"fanout %s %s\n\n", pchar, pin2->label);
  }
  for(i = 0;i < array_n(clbl->outputs);i++) {
    c = array_fetch(netl_clb_t *, clbl->outputs, i);
    fprintf(fp,"output %s %s %d %d %d %d\n",c->cb->comp, c->label, c->x,
      c->y, c->z, c->nio);
    pchar = array_fetch(char *, c->cb->mapin, 0);
    pin2 = array_fetch(netl_pin_t *, c->cb->in, 0);
    fprintf(fp,"fanin %s %s\n", pchar, pin2->label);
    fprintf(fp,"fanout open open\n\n");
  }
  for(i = 0;i < array_n(clbl->clbs);i++) {
    c = array_fetch(netl_clb_t *, clbl->clbs, i);
    fprintf(fp,"clb %s %s %d %d %d\n",c->cb->comp, c->label, c->x, c->y,
      c->z);
    for(j = 0;j < array_n(c->cb->mapin);j++) {
      pchar = array_fetch(char *, c->cb->mapin, j);
      pin2 = array_fetch(netl_pin_t *, c->cb->in, j);
      fprintf(fp,"fanin %s %s\n", pchar, pin2->label);
    }  
    for(j = 0;j < array_n(c->cb->mapout);j++) {
      pchar = array_fetch(char *, c->cb->mapout, j);
      pin2 = array_fetch(netl_pin_t *, c->cb->out, j);
      fprintf(fp,"fanout %s %s\n", pchar, pin2->label);
    }  
    fprintf(fp,"\n");     
  }    
  (void) fclose(fp);
}

/**Function********************************************************************

  Synopsis           [Write the given network in the vpr netlist format.]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
void
netl_VprNetWrite(
  network_t *net,
  char *fname,
  int kl)
{
FILE *fp;
int j,k, bs = 0;
node_t *node, *fanin, *fanout;
lsGen gen, genout;
char **bufin = NIL(char *), **bufout = NIL(char *);

  fp = fopen(fname,"w");
  if(fp == NIL(FILE)) {
    fprintf(msgerr, "Could not open file %s for writing.", fname);
    return;
  }
  foreach_primary_input(net, gen, node) {
    fprintf(fp,".input %s\npinlist: %s\n\n", node_name(node), node_name(node));
    foreach_fanout(node, genout, fanout) {
      if(node_type(fanout) == PRIMARY_OUTPUT) {
        bs++;
        bufin = (char **) REALLOC(char *, bufin, bs);
        bufout = (char **) REALLOC(char *, bufout, bs);
        bufin[bs - 1] = util_strsav(node_name(node));
        bufout[bs - 1] = util_strsav(node_name(fanout));      
      }    
    }
  }
  foreach_primary_output(net, gen, node) {
    fprintf(fp,".output %s_out\npinlist: %s\n\n", node_name(node), 
      node_name(node));
  }

  while(bs > 0) {
    fprintf(fp,".clb %s\n", bufout[bs - 1]);
    fprintf(fp,"pinlist: %s open open open %s open\n", bufin[bs - 1],
      bufout[bs - 1]);
    fprintf(fp,"subblock: %s 0 open open open 4 open\n\n", bufout[bs - 1]);    
    FREE(bufout[bs - 1]);
    FREE(bufin[bs - 1]);
    bs--;
  }
  if(bufout != NIL(char *)) {
    FREE(bufout);
    FREE(bufin);
  }  
  
  foreach_node(net, gen, node) {
    if(node_type(node) == INTERNAL) {
      fprintf(fp,".clb %s\npinlist: ",node_name(node));
      foreach_fanin(node, k, fanin) {
        fprintf(fp,"%s ",node_name(fanin));
      }
      for(j = k;j < kl;j++) fprintf(fp,"open ");
      fprintf(fp,"%s open\n", node_name(node));
      fprintf(fp,"subblock: %s ", node_name(node));
      for(j = 0;j < k;j++) fprintf(fp,"%d ",j);
      for(;j < kl;j++) fprintf(fp,"open ");
      fprintf(fp,"%d open\n\n",kl);
    }  
  }
  (void) fclose(fp);
}

/**Function********************************************************************

  Synopsis           [Extract a netlist from a clblist.]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
array_t *
netl_Extr(netl_clbl_t *clbl)
{
array_t *ret = NIL(array_t);
int i, j, k;
netl_clb_t *clb;
netl_net_t *nl;
netl_signet_t *sn;
netl_pin_t *pin;
char *pchar;

  if(clbl == NIL(netl_clbl_t)) {
    return ret;
  }  
  ret = array_alloc(netl_net_t *, 0);
  
  for(i = 0;i < array_n(clbl->clbs);i++) {
    clb = array_fetch(netl_clb_t *, clbl->clbs, i);  
    for(j = 0;j < array_n(clb->cb->in);j++) {
      pin = array_fetch(netl_pin_t *, clb->cb->in, j);
      for(k = 0;k < array_n(ret);k++) {
        nl = array_fetch(netl_net_t *, ret, k);
        if(strcmp(nl->name, pin->label) == 0) break;
      }

      if(k < array_n(ret)) {
        for(k = 0;k < array_n(nl->snet);k++) {
          sn = array_fetch(netl_signet_t *, nl->snet, k);
          if((strcmp(sn->clb->label, clb->label) == 0) &&
             (sn->pin == j)) break;
        }
        if(k == array_n(nl->snet)) {
          sn = (netl_signet_t *) ALLOC(netl_signet_t, 1);
          sn->pin = j;
          sn->clb = clb;
          sn->type = 0;
          array_insert_last(netl_signet_t *, nl->snet, sn);
        }
      }
      else {
        nl = (netl_net_t *) ALLOC(netl_net_t, 1);
        nl->name = util_strsav(pin->label);
        nl->global = 0;
        for(k = 0;k < array_n(clbl->globals);k++) {
          pchar = array_fetch(char *, clbl->globals, k);
          if(strcmp(pchar, pin->label) == 0) {
            nl->global = 1;
            break;
          }
        }
        nl->cost = 0.;
        nl->tempcost = 0.;
        nl->snet = array_alloc(netl_signet_t *, 0);
        sn = (netl_signet_t *) ALLOC(netl_signet_t, 1);
        sn->pin = j;
        sn->clb = clb;
        sn->type = 0;
        array_insert_last(netl_signet_t *, nl->snet, sn);
        array_insert_last(netl_net_t *, ret, nl);
      }
      pin->net = nl;      
    }
    
    for(j = 0;j < array_n(clb->cb->out);j++) {
      pin = array_fetch(netl_pin_t *, clb->cb->out, j);
      for(k = 0;k < array_n(ret);k++) {
        nl = array_fetch(netl_net_t *, ret, k);
        if(strcmp(nl->name, pin->label) == 0) break;
      }
      if(k < array_n(ret)) {
        for(k = 0;k < array_n(nl->snet);k++) {
          sn = array_fetch(netl_signet_t *, nl->snet, k);
          if((strcmp(sn->clb->label, clb->label) == 0) &&
             (sn->pin == j)) break;
        }
        if(k == array_n(nl->snet)) {
          sn = (netl_signet_t *) ALLOC(netl_signet_t, 1);
          sn->pin = j;
          sn->type = 1;
          sn->clb = clb;
          array_insert_last(netl_signet_t *, nl->snet, sn);
        }
      }
      else {
        nl = (netl_net_t *) ALLOC(netl_net_t, 1);
        nl->name = util_strsav(pin->label);
        nl->global = 0;
        for(k = 0;k < array_n(clbl->globals);k++) {
          pchar = array_fetch(char *, clbl->globals, k);
          if(strcmp(pchar, pin->label) == 0) {
            nl->global = 1;
            break;
          }
        }
        nl->cost = 0.;
        nl->tempcost = 0.;
        nl->snet = array_alloc(netl_signet_t *, 0);
        sn = (netl_signet_t *) ALLOC(netl_signet_t, 1);
        sn->pin = j;
        sn->clb = clb;
        sn->type = 1;
        array_insert_last(netl_signet_t *, nl->snet, sn);
        array_insert_last(netl_net_t *, ret, nl);
      }
      pin->net = nl;
    }           
  }

  for(i = 0;i < array_n(clbl->inputs);i++) {
    clb = array_fetch(netl_clb_t *, clbl->inputs, i);
    for(j = 0;j < array_n(clb->cb->out);j++) {
      pin = array_fetch(netl_pin_t *, clb->cb->out, j);
      for(k = 0;k < array_n(ret);k++) {
        nl = array_fetch(netl_net_t *, ret, k);
        if(strcmp(nl->name, pin->label) == 0) break;
      }
      if(k < array_n(ret)) {
        for(k = 0;k < array_n(nl->snet);k++) {
          sn = array_fetch(netl_signet_t *, nl->snet, k);
          if((strcmp(sn->clb->label, clb->label) == 0) &&
             (sn->pin == j)) break;
        }
        if(k == array_n(nl->snet)) {
          sn = (netl_signet_t *) ALLOC(netl_signet_t, 1);
          sn->pin = j;
          sn->type = 1;
          sn->clb = clb;
          array_insert_last(netl_signet_t *, nl->snet, sn);
        }
      }
      else {
        nl = (netl_net_t *) ALLOC(netl_net_t, 1);
        nl->name = util_strsav(pin->label);
        nl->global = 0;
        for(k = 0;k < array_n(clbl->globals);k++) {
          pchar = array_fetch(char *, clbl->globals, k);
          if(strcmp(pchar, pin->label) == 0) {
            nl->global = 1;
            break;
          }
        }
        nl->cost = 0.;
        nl->tempcost = 0.;
        nl->snet = array_alloc(netl_signet_t *, 0);
        sn = (netl_signet_t *) ALLOC(netl_signet_t, 1);
        sn->pin = j;
        sn->type = 1;
        sn->clb = clb;
        array_insert_last(netl_signet_t *, nl->snet, sn);
        array_insert_last(netl_net_t *, ret, nl);
      }
      pin->net = nl;
    }     
  }

  for(i = 0;i < array_n(clbl->outputs);i++) {
    clb = array_fetch(netl_clb_t *, clbl->outputs, i);
    for(j = 0;j < array_n(clb->cb->in);j++) {
      pin = array_fetch(netl_pin_t *, clb->cb->in, j);
      for(k = 0;k < array_n(ret);k++) {
        nl = array_fetch(netl_net_t *, ret, k);
        if(strcmp(nl->name, pin->label) == 0) break;
      }
      if(k < array_n(ret)) {
        for(k = 0;k < array_n(nl->snet);k++) {
          sn = array_fetch(netl_signet_t *, nl->snet, k);
          if((strcmp(sn->clb->label, clb->label) == 0) &&
             (sn->pin == j)) break;
        }
        if(k == array_n(nl->snet)) {
          sn = (netl_signet_t *) ALLOC(netl_signet_t, 1);
          sn->pin = j;
          sn->type = 0;
          sn->clb = clb;
          array_insert_last(netl_signet_t *, nl->snet, sn);
        }
      }
      else {
        nl = (netl_net_t *) ALLOC(netl_net_t, 1);
        nl->name = util_strsav(pin->label);
        nl->global = 0;
        for(k = 0;k < array_n(clbl->globals);k++) {
          pchar = array_fetch(char *, clbl->globals, k);
          if(strcmp(pchar, pin->label) == 0) {
            nl->global = 1;
            break;
          }
        }
        nl->cost = 0.;
        nl->tempcost = 0.;
        nl->snet = array_alloc(netl_signet_t *, 0);
        sn = (netl_signet_t *) ALLOC(netl_signet_t, 1);
        sn->pin = j;
        sn->type = 0;
        sn->clb = clb;
        array_insert_last(netl_signet_t *, nl->snet, sn);
        array_insert_last(netl_net_t *, ret, nl);
      }
      pin->net = nl;
    }
  }
  array_sort(ret, NetlCmp);
  netl_UpdateId(ret);
  
  return ret;
}
/**Function********************************************************************

  Synopsis           [Update the id field of a net in order to match the
  index of the net in the netlist array.]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
void
netl_UpdateId(array_t *netl)
{
int i;
netl_net_t *n;

  for(i = 0;i < array_n(netl);i++) {
    n = array_fetch(netl_net_t *, netl, i);
    n->id = i;
  }
}

/**Function********************************************************************

  Synopsis           [Find a netlist.]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
int
netl_Find(
  array_t *netl,
  char *label)
{
int low, high, i, res;
netl_net_t *nl;

  low = 0;
  high = array_n(netl) - 1;
  i = (high + low)/2;
  
  while(low <= high) {
    nl = array_fetch(netl_net_t *, netl, i);
    res = strcmp(label, nl->name);
    if(res == 0) {
      return i;
    }
    else
      if(res > 0) {
        low = i + 1;
      }
      else
        if(res < 0) {
          high = i - 1;
        }

    i = (low + high)/2;
  }     
  return -1;
}

/**Function********************************************************************

  Synopsis           [Write a given placed clb list in the vpr place format.]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
void
netl_VprPlaceWrite(
  netl_clbl_t *clbl,
  int nx,
  int ny,
  char *fname)
{
FILE *fp;
int i = 0;
netl_clb_t *clb;

  fp = fopen(fname,"w");
  if(fp == NIL(FILE)) {
    fprintf(msgerr, "VPR place writer: cannot create file %s\n", fname);
    return;
  }
  fprintf(fp,"Netlist file: %s.net  Architecture file: sample.arch\n",
    clbl->name);
  fprintf(fp,"Array size: %d x %d logic blocks\n", nx, ny); 

  for(i = 0;i < array_n(clbl->inputs);i++) {
    clb = array_fetch(netl_clb_t *, clbl->inputs, i);
    fprintf(fp,"%s\t%d\t%d\t%d\n", clb->label, clb->x, clb->y, clb->nio);
  }
  for(i = 0;i < array_n(clbl->outputs);i++) {
    clb = array_fetch(netl_clb_t *, clbl->outputs, i);
    fprintf(fp,"%s\t%d\t%d\t%d\n", clb->label, clb->x, clb->y, clb->nio);
  }  
  for(i = 0;i < array_n(clbl->clbs);i++) {
    clb = array_fetch(netl_clb_t *, clbl->clbs, i);
    fprintf(fp,"%s\t%d\t%d\t0\n", clb->label, clb->x, clb->y);
  }    
  (void) fclose(fp);
}

/**Function********************************************************************

  Synopsis           [Read a vpr netlist file.]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
int
netl_VprNetLoad(
  netl_clbl_t **clbl,
  char *fname)
{
char buf[100], *name, **pinlist = NIL(char *), **plnum = NIL(char *), buf2[100];
int i, plcount = 0, pncount = 0;
netl_clb_t *clb;
netl_pin_t *pin;
FILE *fp;

  fp = fopen(fname,"r");
  if(fp == NIL(FILE)) {
    *clbl = NIL(netl_clbl_t);
    return 1;
  }

  if(*clbl != NIL(netl_clbl_t)) {
    netl_ClblFree(*clbl);
  }
  *clbl = (netl_clbl_t *) ALLOC(netl_clbl_t, 1);

  strcpy(buf, fname);
  name = strtok(buf, "/");
  while(name != NIL(char)) {
    strcpy(buf2, name);
    name = strtok(NULL,"/");  
  }
  name = strtok(buf2,".");
  (*clbl)->name = util_strsav(name);
  (*clbl)->inputs = array_alloc(netl_clb_t *, 0);
  (*clbl)->outputs = array_alloc(netl_clb_t *, 0);
  (*clbl)->clbs = array_alloc(netl_clb_t *, 0);
  (*clbl)->globals = array_alloc(char *, 0);

  (void) fscanf(fp,"%s", buf);
  if((strcmp(buf,"#") == 0) || (buf[0] == '#')) {
    while(getc(fp) != '\n');
    (void) fscanf(fp,"%s", buf);    
  }
  while(!feof(fp)) {
    if(strcmp(buf,".input") == 0) {
      (void) fscanf(fp,"%s", buf);
      if((strcmp(buf,"#") == 0) || (buf[0] == '#')) {
        while(getc(fp) != '\n');
        (void) fscanf(fp,"%s", buf);    
      }
      name = util_strsav(buf);
      (void) fscanf(fp,"%s", buf);
      if((strcmp(buf,"#") == 0) || (buf[0] == '#')) {
        while(getc(fp) != '\n');
        (void) fscanf(fp,"%s", buf);    
      }
      if(strcmp(buf,"pinlist:") != 0) {
        printf("Error: missing pinlist at %s input!\n", name);
        FREE(name);
        netl_ClblFree(*clbl);
        return 1;
      }
      (void) fscanf(fp,"%s",buf);
      if((strcmp(buf,"#") == 0) || (buf[0] == '#')) {
        while(getc(fp) != '\n');
        (void) fscanf(fp,"%s", buf);    
      }
      clb = (netl_clb_t *) ALLOC(netl_clb_t, 1);
      clb->label = util_strsav(name);
      FREE(name);
      clb->x = -1;
      clb->y = -1;
      clb->z = -1;
      clb->nio = -1;
      clb->noswap = 0;
      clb->cb = (netl_cb_t *) ALLOC(netl_cb_t, 1);
      clb->cb->comp = util_strsav("INPUT");
      clb->cb->mapin = array_alloc(char *, 0);
      clb->cb->in = array_alloc(netl_pin_t *, 0);

      name = util_strsav("i");
      clb->cb->mapout = array_alloc(char *, 0);
      array_insert_last(char *, clb->cb->mapout, name);

      clb->cb->out = array_alloc(netl_pin_t *, 0);
      pin = (netl_pin_t *) ALLOC(netl_pin_t, 1);
      pin->label = util_strsav(buf);
      pin->net = NIL(netl_net_t);
      pin->skip = 0;
      array_insert_last(netl_pin_t *, clb->cb->out, pin);
      
      array_insert_last(netl_clb_t *, (*clbl)->inputs, clb);
    }
    else
      if(strcmp(buf,".output") == 0) {
        (void) fscanf(fp,"%s",buf);
        if((strcmp(buf,"#") == 0) || (buf[0] == '#')) {
          while(getc(fp) != '\n');
          (void) fscanf(fp,"%s", buf);    
        }
        name = util_strsav(buf);
        (void) fscanf(fp,"%s", buf);
        if((strcmp(buf,"#") == 0) || (buf[0] == '#')) {
          while(getc(fp) != '\n');
          (void) fscanf(fp,"%s", buf);    
        }
        if(strcmp(buf,"pinlist:") != 0) {
          printf("Error: missing pinlist at %s output!\n", name);
          FREE(name);
          netl_ClblFree(*clbl);
          return 1;
        }
        (void) fscanf(fp,"%s",buf);
        if((strcmp(buf,"#") == 0) || (buf[0] == '#')) {
          while(getc(fp) != '\n');
          (void) fscanf(fp,"%s", buf);    
        }
        clb = (netl_clb_t *) ALLOC(netl_clb_t, 1);
        clb->label = util_strsav(name);
        FREE(name);
        clb->x = -1;
        clb->y = -1;
        clb->z = -1;
        clb->nio = -1;
        clb->noswap = 0;        
        clb->cb = (netl_cb_t *) ALLOC(netl_cb_t, 1);
        clb->cb->comp = util_strsav("OUTPUT");
        clb->cb->mapout = array_alloc(char *, 0);
        clb->cb->out = array_alloc(netl_pin_t *, 0);

        name = util_strsav("o");
        clb->cb->mapin = array_alloc(char *, 0);
        array_insert_last(char *, clb->cb->mapin, name);

        clb->cb->in = array_alloc(netl_pin_t *, 0);
        pin = (netl_pin_t *) ALLOC(netl_pin_t, 1);
        pin->label = util_strsav(buf);
        pin->net = NIL(netl_net_t);
        pin->skip = 0;
        array_insert_last(netl_pin_t *, clb->cb->in, pin);
      
        array_insert_last(netl_clb_t *, (*clbl)->outputs, clb);
      }
      else
        if(strcmp(buf,".clb") == 0) {
          (void) fscanf(fp,"%s",buf);
          if((strcmp(buf,"#") == 0) || (buf[0] == '#')) {
            while(getc(fp) != '\n');
            (void) fscanf(fp,"%s", buf);    
          }
          name = util_strsav(buf);
          (void) fscanf(fp,"%s", buf);
          if((strcmp(buf,"#") == 0) || (buf[0] == '#')) {
            while(getc(fp) != '\n');
            (void) fscanf(fp,"%s", buf);    
          }
          if(strcmp(buf,"pinlist:") != 0) {
            printf("Error: missing pinlist at %s output!\n", name);
            FREE(name);
            netl_ClblFree(*clbl);
            return 1;
          }
          (void) fscanf(fp,"%s",buf);
          if((strcmp(buf,"#") == 0) || (buf[0] == '#')) {
            while(getc(fp) != '\n');
            (void) fscanf(fp,"%s", buf);    
          }
          while(strcmp(buf,"subblock:") != 0) {
            plcount++;
            pinlist = (char **) REALLOC(char *, pinlist, plcount);
            pinlist[plcount - 1] = util_strsav(buf);
            (void) fscanf(fp,"%s",buf);
            if((strcmp(buf,"#") == 0) || (buf[0] == '#')) {
              while(getc(fp) != '\n');
              (void) fscanf(fp,"%s", buf);    
            }
          }
          (void) fscanf(fp,"%s",buf);
          if((strcmp(buf,"#") == 0) || (buf[0] == '#')) {
            while(getc(fp) != '\n');
            (void) fscanf(fp,"%s", buf);    
          }
          for(i = 0;i < plcount;i++) {
            (void) fscanf(fp,"%s",buf);
            if((strcmp(buf,"#") == 0) || (buf[0] == '#')) {
              while(getc(fp) != '\n');
              (void) fscanf(fp,"%s", buf);    
            }
            pncount++;
            plnum = (char **) REALLOC(char *, plnum, pncount);
            plnum[pncount - 1] = util_strsav(buf);          
          }
          pncount = 0;
          for(i = 0;i < plcount;i++) {
            if(strcmp(pinlist[i],"open") != 0) pncount++;
          }

          clb = (netl_clb_t *) ALLOC(netl_clb_t, 1);
          clb->label = util_strsav(name);
          FREE(name);
          clb->x = -1;
          clb->y = -1;
          clb->z = -1;
          clb->nio = -1;
          clb->noswap = 0;          
          clb->cb = (netl_cb_t *) ALLOC(netl_cb_t, 1);
          clb->cb->comp = util_strsav("COMP");
          clb->cb->mapout = array_alloc(char *, 1);
          clb->cb->out = array_alloc(netl_pin_t *, 1);
          clb->cb->mapin = array_alloc(char *, pncount - 1);
          clb->cb->in = array_alloc(netl_pin_t *, pncount -1);
          
          pncount = 0;
          for(i = 0;i < plcount;i++) {
            if(strcmp(pinlist[i],"open") != 0) {
              name = util_strsav(plnum[i]);
              if(strcmp(name, "4") == 0) {
                array_insert(char *, clb->cb->mapout, 0, name);
                pin = (netl_pin_t *) ALLOC(netl_pin_t, 1);
                pin->label = util_strsav(pinlist[i]);
                pin->net = NIL(netl_net_t);
                pin->skip = 0;
                array_insert(netl_pin_t *, clb->cb->out, 0, pin);
              }
              else {
                array_insert(char *, clb->cb->mapin, pncount, name);
                pin = (netl_pin_t *) ALLOC(netl_pin_t, 1);
                pin->label = util_strsav(pinlist[i]);
                pin->net = NIL(netl_net_t);
                array_insert(netl_pin_t *, clb->cb->in, pncount, pin);
                pncount++;
              }
            }
          }          
          array_insert_last(netl_clb_t *, (*clbl)->clbs, clb);

          for(i = 0;i < plcount;i++) {
            FREE(pinlist[i]);
            FREE(plnum[i]);
          }
          FREE(pinlist);
          FREE(plnum);
          plnum = NIL(char *);
          pinlist = NIL(char *);
          plcount = 0;
          pncount = 0;
        }
        else
          if(strcmp(buf,".global") == 0) {
            (void) fscanf(fp,"%s",buf);
            if((strcmp(buf,"#") == 0) || (buf[0] == '#')) {
              while(getc(fp) != '\n');
              (void) fscanf(fp,"%s", buf);    
            }
            name = util_strsav(buf);
            array_insert_last(char *, (*clbl)->globals, name);            
          }
          else {
            printf("Netlist file corrupted!\n");
            return 1;
          }

    (void) fscanf(fp,"%s",buf);
    if((strcmp(buf,"#") == 0) || (buf[0] == '#')) {
      while(getc(fp) != '\n');
      (void) fscanf(fp,"%s", buf);    
    }
  }
  
  DuplicatePins(*clbl);
   
  (void) fclose(fp);
  return 0;
}

/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis           [Compare two clbs based on their position.]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static void
DuplicatePins(netl_clbl_t *clbl)
{
int i, j, k, ns;
netl_clb_t *clb;
char **names;
netl_pin_t *pin;

  if(clbl == NIL(netl_clbl_t)) return;
  
  for(i = 0;i < array_n(clbl->clbs);i++) {
    clb = array_fetch(netl_clb_t *, clbl->clbs, i);
    ns = 0;
    names = NIL(char *);
    for(j = 0;j < array_n(clb->cb->out);j++) {
      pin = array_fetch(netl_pin_t *, clb->cb->out, j);
      for(k = 0;k < ns;k++) {
        if(strcmp(names[k], pin->label) == 0) break;
      }
      ns++;
      names = (char **) REALLOC(char *, names, ns);
      names[ns - 1] = util_strsav(pin->label);
    }      
    for(j = 0;j < array_n(clb->cb->in);j++) {
      pin = array_fetch(netl_pin_t *, clb->cb->in, j);
      for(k = 0;k < ns;k++) {
        if(strcmp(names[k], pin->label) == 0) break;
      }
      if(k == ns) {
        ns++;
        names = (char **) REALLOC(char *, names, ns);
        names[ns - 1] = util_strsav(pin->label);
      }
      else {
        if(k != 0) {
          printf("Warning: there are two input pins with the same signal.\n");
        }
        else {
          pin->skip = 1;
        }
      }    
    }
    for(j = 0;j < ns;j++) {
      FREE(names[j]);
    }  
    FREE(names);
  }
}

/**Function********************************************************************

  Synopsis           [Compare two clbs based on their position.]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static int
ClbCmpPos(
  char *o1,
  char *o2)
{
netl_clb_t **c1, **c2;

  c1 = (netl_clb_t **) o1;
  c2 = (netl_clb_t **) o2;
  
  if((*c1)->x < (*c2)->x) {
   return -1;
 }
  else
    if((*c1)->x > (*c2)->x) {
      return 1;
    }
    else {
      if((*c1)->y < (*c2)->y) {
        return -1;
      }
      else
        if((*c1)->y > (*c2)->y) {
          return 1;
        }
        else {
          if((*c1)->z < (*c2)->z) {          
            return -1;
          }
          else
            if((*c1)->z > (*c2)->z) {
              return 1;
            }
        }    
    }          
  return 0;
}

/**Function********************************************************************

  Synopsis           [Compare two clbs base on their labels.]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static int
ClbCmpLabel(
  char *o1,
  char *o2)
{
netl_clb_t **c1, **c2;

  c1 = (netl_clb_t **) o1;
  c2 = (netl_clb_t **) o2;

  return strcmp((*c1)->label, (*c2)->label);
}

/**Function********************************************************************

  Synopsis           [Compare two netl_net_t.]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static int
NetlCmp(
  char *o1,
  char *o2)
{
netl_net_t **n1, **n2;

  n1 = (netl_net_t **) o1;
  n2 = (netl_net_t **) o2;
  
  return strcmp((*n1)->name, (*n2)->name);
}

/**Function********************************************************************

  Synopsis           [Print a clblist.]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
void
ClblPrint(netl_clbl_t *clbl)
{
int i;
netl_clb_t *c;
netl_pin_t *pin;
char *pchar;

  if(clbl == NIL(netl_clbl_t)) {
    return;
  }
  printf("Clblist %s\n", clbl->name);
  printf("Inputs:\n");
  for(i = 0;i < array_n(clbl->inputs);i++) {
    c = array_fetch(netl_clb_t *, clbl->inputs, i);
    printf("Label = %s  X = %d  Y = %d  Z= %d  NIO = %d\n", c->label, c->x, 
      c->y, c->z, c->nio);
    pin = array_fetch(netl_pin_t *, c->cb->out, 0);
    printf("Connected to %s\n", pin->label);    
  }
  printf("Outputs:\n");
  for(i = 0;i < array_n(clbl->outputs);i++) {
    c = array_fetch(netl_clb_t *, clbl->outputs, i);
    printf("Label = %s  X = %d  Y = %d  Z= %d  NIO = %d\n", c->label, c->x, 
      c->y, c->z, c->nio);
    pin = array_fetch(netl_pin_t *, c->cb->in, 0);
    printf("Connected to %s\n", pin->label);
  }
  printf("Clbs:\n");
  for(i = 0;i < array_n(clbl->clbs);i++) {
    c = array_fetch(netl_clb_t *, clbl->clbs, i);
    printf("Label = %s  X = %d  Y = %d  Z= %d  NIO = %d\n", c->label, c->x, 
      c->y, c->z, c->nio);
  }
  for(i = 0;i < array_n(clbl->globals);i++) {
    pchar = array_fetch(char *, clbl->globals, i);
    printf("Global Net: %s\n", pchar);
  }
}

/**Function********************************************************************

  Synopsis           [Print a netlist.]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
void
NetlPrint(array_t *netl)
{
int i,j;
netl_net_t *nl;
netl_signet_t *sn;

  if(netl == NIL(array_t)) {
    return;
  }
  for(i = 0;i < array_n(netl);i++) {
    nl = array_fetch(netl_net_t *, netl, i);
    printf("Netlist %s  Cost = %f  Global = %d\n", nl->name, nl->cost,
      nl->global);
    for(j = 0;j < array_n(nl->snet);j++) {
      sn = array_fetch(netl_signet_t *, nl->snet, j);
      printf("%s %d %d\n", sn->clb->label, sn->pin, sn->type);
    }
    printf("\n\n");
  }
}
