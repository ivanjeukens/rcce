/**CFile***********************************************************************

  FileName    [compLg.c]

  PackageName [comp]

  Synopsis    [This file contain several functions for manipulating the logic
  graph of a component.]

  Description []

  SeeAlso     [compUtil.c]

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

static void LpinFree(complpin_t *data);
static void LnodeFree(gGeneric data);
static complnode_t * LnodeAlloc(int type, lsList pinlst, char *expr);
static int LnodeCompare(complnode_t *node1, complnode_t *node2);
static complnode_t * LnodeFind(graph_t *graph, complnode_t *node);
static vertex_t * OsigFind(graph_t *lg, char *label);

/**AutomaticEnd***************************************************************/


/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis           [Release the memory hold by a logic graph.]

  Description        []

  SideEffects        []
  
  SeeAlso	     []
  
******************************************************************************/
void
compLgraphFree(graph_t *lg)
{
  g_free(lg, NULL, LnodeFree, NULL);
}

/**Function********************************************************************

  Synopsis           [Insert a new logic pin at the begining of the pin list.]

  Description        []

  SideEffects        []
  
  SeeAlso	     []
  
******************************************************************************/
lsList
compLpinIns(
  lsList lst,
  char *label,
  int type)
{
complpin_t *p;
int status;

  p = (complpin_t *) ALLOC(complpin_t, 1);
  p->label = util_strsav(label);
  p->type = type;
  p->constr = NIL(compmemconstr_t);
  if(lst == NULL) {
    lst = lsCreate();
  }
  status = lsNewBegin(lst, (lsGeneric) p, LS_NH);
  if(status != LS_OK) { 
    fprintf(siserr, "Error while trying to insert a new logic pin\n");    
  }
  return lst;  
}

/**Function********************************************************************

  Synopsis           [Set the memory constraints for a specified logic pin.]

  Description        []

  SideEffects        []
  
  SeeAlso	     []
  
******************************************************************************/
graph_t *
compMconstrIns(
  graph_t *lg,
  char *label,
  char *out,
  char *clk,
  float value1,
  float value2)
{
lsList vertices;
lsGen gen;
int status;
complnode_t *no;
complpin_t *p1,*p2;
vertex_t *v;

  if(lg == NIL(graph_t)) {
    return NIL(graph_t);
  }
  vertices = g_get_vertices(lg);
  gen = lsStart(vertices);  
  status = lsNext(gen, (lsGeneric *) &v, LS_NH);
  while(status != LS_NOMORE) {
    no = (complnode_t *) v->user_data;
    p1 = compLpinFind(no->pins, label);
    p2 = compLpinFind(no->pins, out);
    if((p1 != NIL(complpin_t)) && (p2 != NIL(complpin_t))) break;
    status = lsNext(gen, (lsGeneric *) &v, LS_NH);
  }
  if(status != LS_NOMORE) {
    p1->constr = (compmemconstr_t *) ALLOC(compmemconstr_t, 1);
    p1->constr->clk_name = util_strsav(clk);
    p1->constr->setup = value1;
    p1->constr->hold = value2;     
  }
  (void) lsFinish(gen);
  return lg;
}

/**Function********************************************************************

  Synopsis           [Search for a logic output pin within the logic graph.]

  Description        []

  SideEffects        []
  
  SeeAlso	     []
  
******************************************************************************/
vertex_t *
compLoutputFind(
  graph_t *lg,
  char *label)
{
lsGen gen;
lsList vertices;
complnode_t *no;
complpin_t *pin;
vertex_t *v;
int status;

  if(lg == NIL(graph_t)) {
    return NIL(vertex_t);
  }  
  vertices = g_get_vertices(lg);
  gen = lsStart(vertices);
  status = lsNext(gen, (lsGeneric *) &v, LS_NH);
  while(status != LS_NOMORE) {
    no = (complnode_t *) v->user_data;
    pin = compLpinFind(no->pins, label);
    if((pin != NIL(complpin_t)) && ((pin->type == COMP_OSIGNAL) || 
       (pin->type == COMP_LOUTPUT) || (pin->type == COMP_SHAREDO))) {
      (void) lsFinish(gen);
      return v;
    }
    status = lsNext(gen, (lsGeneric *) &v, LS_NH);  
  }
  (void) lsFinish(gen); 
  return NIL(vertex_t);
}

/**Function********************************************************************

  Synopsis           [Add a logic node to the graph.]

  Description        []

  SideEffects        []
  
  SeeAlso	     []
  
******************************************************************************/
graph_t *
compLnodeIns(
  graph_t *graph,
  complnode_t *node)
{
vertex_t *v;

  if(graph == NIL(graph_t)) {
    return NIL(graph_t);
  }
  if(LnodeFind(graph, node) == NIL(complnode_t)) {  
    v = g_add_vertex(graph);
    v->user_data = (gGeneric) node;
  }
  return graph;  
}

/**Function********************************************************************

  Synopsis           [Print a logic graph.]

  Description        []

  SideEffects        []
  
  SeeAlso	     []
  
******************************************************************************/
void
compLgraphPrint(graph_t *lg)
{
lsList vertices, edges;
lsGen gen, genpin;
complnode_t *no;
vertex_t *v;
int status;
complpin_t *pin;
edge_t *e;

  if(lg == NIL(graph_t)) {
    return;
  }
  vertices = g_get_vertices(lg);
  gen = lsStart(vertices);
  status = lsNext(gen, (lsGeneric *) &v, LS_NH);
  while(status != LS_NOMORE) {
    no = (complnode_t *) v->user_data;
    switch(no->type) {
      case COMP_LLOW: printf("Active low latch\n"); break;
      case COMP_LHIGH: printf("Active high latch\n"); break;
      case COMP_DFFF: printf("Falling edge flip flop\n"); break;
      case COMP_DFFR: printf("Rising edge flip flop\n"); break;
      case COMP_LASYN: printf("Asynchronous latch\n"); break;
      case COMP_MUX: printf("Multiplexer\n"); break;
      case COMP_LUT: printf("Look-up table\n"); break;
      case COMP_EXPR: printf("Expression\n%s\n", no->expr); break;
      default: printf("Unkown function!\n"); break;
    }     
    printf("Node pins\n");
    genpin = lsStart(no->pins);
    status = lsNext(genpin,(lsGeneric *) &pin, LS_NH);
    while(status != LS_NOMORE) {
      printf("Label = %s\n",pin->label);    
      switch(pin->type) {
        case COMP_LINPUT: printf("Pin type = logic input\n"); break;
        case COMP_LOUTPUT: printf("Pin type = logic output\n"); break;
        case COMP_ISIGNAL: printf("Pin type = input signal\n"); break;
        case COMP_OSIGNAL: printf("Pin type = output signal\n"); break;
        case COMP_SHAREDI: printf("Pin type = shared input\n"); break;
        case COMP_SHAREDO: printf("Pin type = shared output\n"); break;
        case COMP_CLK: printf("Pin type = clk\n"); break;
        default: printf("Unkown pin type!\n"); break;
      }  
      if(pin->constr) {
        printf("Memory constraint present\n");
        printf("Clock name = %s\n",pin->constr->clk_name);
        printf("Setup = %f\n",pin->constr->setup);
        printf("Hold = %f\n",pin->constr->hold);
      }
      else
        printf("No memory constraint\n");
      status = lsNext(genpin,(lsGeneric *) &pin, LS_NH);      
    }
    lsFinish(genpin);
    printf("Node connected to\n");
    edges = g_get_out_edges(v);
    genpin = lsStart(edges);
    status = lsNext(genpin, (lsGeneric *) &e, LS_NH);
    while(status != LS_NOMORE) {
      v = g_e_dest(e);
      no = (complnode_t *) v->user_data;      
      switch(no->type) {
        case COMP_LLOW: printf("Active low latch\n"); break;
        case COMP_LHIGH: printf("Active high latch\n"); break;
        case COMP_DFFF: printf("Falling edge flip flop\n"); break;
        case COMP_DFFR: printf("Rising edge flip flop\n"); break;
        case COMP_LASYN: printf("Asynchronous latch\n"); break;
        case COMP_MUX: printf("Multiplexer\n"); break;
        case COMP_LUT: printf("Look-up table\n"); break;
        case COMP_EXPR: printf("Expression\n"); break;
        default: printf("Unkown function\n"); break;
      }        
      status = lsNext(genpin, (lsGeneric *) &e, LS_NH);    
    }        
    printf("---------------------------\n\n");
    status = lsNext(gen, (lsGeneric *) &v, LS_NH);
  }
  (void) lsFinish(gen);
}

/**Function********************************************************************

  Synopsis           [Insert a logic primitive = logic graph node + pins + 
  type]

  Description        []

  SideEffects        []
  
  SeeAlso	     [compParse.c]
  
******************************************************************************/
comp_data_t *
comp_LprimIns(
  comp_data_t *c,
  int type,
  lsList list,
  char *expr)
{
complnode_t *node;

  node = LnodeAlloc(type, list, expr);
  c->lg = compLnodeIns(c->lg, node);

  return c;
}

/**Function********************************************************************

  Synopsis           [Connect a logic graph.]

  Description        []

  SideEffects        []
  
  SeeAlso	     [compParse.c]
  
******************************************************************************/
comp_data_t *
compLgraphConnect(comp_data_t *c)
{
lsList vertices;
lsGen gen, genpin;
int status;
vertex_t *v,*v2;
complnode_t *no;
complpin_t *pin;

  if(c->lg == NIL(graph_t)) {
    return c;
  }
  vertices = g_get_vertices(c->lg);
  gen = lsStart(vertices);
  status = lsNext(gen, (lsGeneric *) &v, LS_NH);
  while(status != LS_NOMORE) {
    no = (complnode_t *) v->user_data;
    genpin = lsStart(no->pins);
    status = lsNext(genpin, (lsGeneric *) &pin, LS_NH);
    while(status != LS_NOMORE) {
      if(pin->type == COMP_ISIGNAL) {
        v2 = OsigFind(c->lg, pin->label);
        if(v2 != NIL(vertex_t)) {
          (void) g_add_edge(v2, v);
        }
      }
      status = lsNext(genpin, (lsGeneric *) &pin, LS_NH);    
    }  
    status = lsNext(gen, (lsGeneric *) &v, LS_NH);  
  }
    
  return c;
}

/**Function********************************************************************

  Synopsis           [Find a logic pin within the logic node pin list.]

  Description        []

  SideEffects        []
  
  SeeAlso	     []
  
******************************************************************************/
complpin_t *
compLpinFind(
  lsList lst,
  char *label)
{
lsGen gen;
complpin_t *pin = NIL(complpin_t);
int status;

  if(lst == NULL) {
    return NIL(complpin_t);
  }
  gen = lsStart(lst);
  status = lsNext(gen, (lsGeneric *) &pin, LS_NH);
  while(status != LS_NOMORE) {
    if(strcmp(pin->label,label) == 0) {
      (void) lsFinish(gen);
      return pin;
    }
    status = lsNext(gen, (lsGeneric *) &pin, LS_NH);
  }
  (void) lsFinish(gen);
  return NIL(complpin_t);
}

/**Function********************************************************************

  Synopsis           [Extract a blif text from the logic graph.]

  Description        []

  SideEffects        []
  
  SeeAlso	     []
  
******************************************************************************/
char *
compLgraph2Blif(comp_data_t *c)
{
char *ret = NIL(char), *pchar, *out, *clk = NIL(char);
char buf[200];
lsList vertices;
lsGen gen, genpin;
complnode_t *no;
vertex_t *v;
complpin_t *pin;
array_t *ins;
int i, j;

  ret = (char *) ALLOC(char, 1);
  ret[0] = '\0';
    
  vertices = g_get_vertices(c->lg);
  lsForeachItem(vertices, gen, v) {
    ins = array_alloc(char *, 0);
    no = (complnode_t *) v->user_data;
    lsForeachItem(no->pins, genpin, pin) {
      if((pin->type == COMP_LINPUT) || (pin->type == COMP_SHAREDI) || 
         (pin->type == COMP_ISIGNAL)) {
        array_insert_last(char *, ins, pin->label);
      }      
      else
        if((pin->type == COMP_LOUTPUT) || (pin->type == COMP_SHAREDO) ||
           (pin->type == COMP_OSIGNAL)) {
          out = util_strsav(pin->label);
        }
        else
          if(pin->type == COMP_CLK) {
            clk = util_strsav(pin->label);
          }          
    }    
    switch(no->type) {
      case COMP_LLOW: {
        strcpy(buf,".latch ");
        pchar = array_fetch(char *, ins, 0);
        strcat(buf, pchar);
        strcat(buf, " ");
        strcat(buf, out);
        strcat(buf, " al ");
        strcat(buf, clk);
        strcat(buf, "\n");
        ret = (char *) REALLOC(char , ret, strlen(ret) + strlen(buf) + 1);
        strcat(ret, buf);
      } break;
      case COMP_LHIGH: {
        strcpy(buf,".latch ");
        pchar = array_fetch(char *, ins, 0);
        strcat(buf, pchar);
        strcat(buf, " ");
        strcat(buf, out);
        strcat(buf, " ah ");
        strcat(buf, clk);
        strcat(buf, "\n");
        ret = (char *) REALLOC(char , ret, strlen(ret) + strlen(buf) + 1);
        strcat(ret, buf);
      } break;
      case COMP_DFFF: {
        strcpy(buf,".latch ");
        pchar = array_fetch(char *, ins, 0);
        strcat(buf, pchar);
        strcat(buf, " ");
        strcat(buf, out);
        strcat(buf, " fe ");
        strcat(buf, clk);
        strcat(buf, "\n");
        ret = (char *) REALLOC(char , ret, strlen(ret) + strlen(buf) + 1);
        strcat(ret, buf);
      } break;
      case COMP_DFFR: {
        strcpy(buf,".latch ");
        pchar = array_fetch(char *, ins, 0);
        strcat(buf, pchar);
        strcat(buf, " ");
        strcat(buf, out);
        strcat(buf, " re ");
        strcat(buf, clk);
        strcat(buf, "\n");
        ret = (char *) REALLOC(char , ret, strlen(ret) + strlen(buf) + 1);
        strcat(ret, buf);
      } break;
      case COMP_LASYN: {
        strcpy(buf,".latch ");
        pchar = array_fetch(char *, ins, 0);
        strcat(buf, pchar);
        strcat(buf, " ");
        strcat(buf, out);
        strcat(buf, " as ");
        strcat(buf, clk);
        strcat(buf, "\n");
        ret = (char *) REALLOC(char , ret, strlen(ret) + strlen(buf) + 1);
        strcat(ret, buf);      
      } break;
      case COMP_MUX: {
        strcpy(buf,".names ");
        for(i = 0;i < array_n(ins);i++) {
          pchar = array_fetch(char *, ins, i);
          strcat(buf, pchar);
          strcat(buf," ");
        }
        strcat(buf, out);        
        strcat(buf,"\n0-1 1\n11- 1\n");
        ret = (char *) REALLOC(char , ret, strlen(ret) + strlen(buf) + 1);
        strcat(ret, buf);                
      } break;
      case COMP_LUT: {
        strcpy(buf,".names ");
        for(i = 0;i < array_n(ins);i++) {
          pchar = array_fetch(char *, ins, i);
          strcat(buf, pchar);
          strcat(buf," ");
        }
        strcat(buf,"lut ");
        strcat(buf, out);        
        strcat(buf,"\n");
        ret = (char *) REALLOC(char , ret, strlen(ret) + strlen(buf) + 1);
        strcat(ret, buf);

        strcpy(buf,"1");
        for(i = 1;i <= array_n(ins);i++) strcat(buf,"1");
        strcat(buf," 1\n");
        ret = (char *) REALLOC(char , ret, strlen(ret) + strlen(buf) + 1);
        strcat(ret, buf);        
      } break;
/*
      case COMP_OR: {
        strcpy(buf,".names ");
        for(i = 0;i < array_n(ins);i++) {
          pchar = array_fetch(char *, ins, i);
          strcat(buf, pchar);
          strcat(buf," ");
        }
        strcat(buf, out);
        strcat(buf,"\n");
        ret = (char *) REALLOC(char , ret, strlen(ret) + strlen(buf) + 1);
        strcat(ret, buf);
        for(i = 0;i < array_n(ins);i++) {          
          buf[0] = '\0';
          for(j = 0;j < array_n(ins);j++) {
            if(i == j)
              strcat(buf, "1");
            else
              strcat(buf, "-");          
          }
          strcat(buf," 1\n");
          ret = (char *) REALLOC(char , ret, strlen(ret) + strlen(buf) + 1);
          strcat(ret, buf);          
        }
      } break;
      case COMP_AND: {
        strcpy(buf,".names ");
        for(i = 0;i < array_n(ins);i++) {
          pchar = array_fetch(char *, ins, i);
          strcat(buf, pchar);
          strcat(buf," ");
        }
        strcat(buf, out);        
        strcat(buf,"\n");
        ret = (char *) REALLOC(char , ret, strlen(ret) + strlen(buf) + 1);
        strcat(ret, buf);

        strcpy(buf,"1");
        for(i = 1;i < array_n(ins);i++) strcat(buf,"1");
        strcat(buf," 1\n");
        ret = (char *) REALLOC(char , ret, strlen(ret) + strlen(buf) + 1);
        strcat(ret, buf);        
      } break;
      case COMP_NOT: { 
        strcpy(buf,".names ");
        for(i = 0;i < array_n(ins);i++) {
          pchar = array_fetch(char *, ins, i);
          strcat(buf, pchar);
          strcat(buf," ");
        }
        strcat(buf, out);
        strcat(buf,"\n0 1\n");
        ret = (char *) REALLOC(char , ret, strlen(ret) + strlen(buf) + 1);              
        strcat(ret, buf);
      } break;
      case COMP_BUF: {
        strcpy(buf,".names ");
        for(i = 0;i < array_n(ins);i++) {
          pchar = array_fetch(char *, ins, i);
          strcat(buf, pchar);
          strcat(buf," ");
        }
        strcat(buf, out);
        strcat(buf,"\n1 1\n");
        ret = (char *) REALLOC(char , ret, strlen(ret) + strlen(buf) + 1);      
        strcat(ret, buf);
      } break;
*/
    }     
    array_free(ins);
    FREE(out);
    if(clk != NIL(char)) {
      FREE(clk);
    }
    clk = NIL(char);
  }

  return ret;
}

/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis           [Release the memory hold by a logic_pin structure.]

  Description        []

  SideEffects        []
  
  SeeAlso	     []
  
******************************************************************************/
static void
LpinFree(complpin_t *data)
{
  FREE(data->label);
  if(data->constr != NIL(compmemconstr_t)) {
    FREE(data->constr->clk_name);
    FREE(data->constr);
  }
  FREE(data);
}

/**Function********************************************************************

  Synopsis           [Release the memory hold by a complnode structure.]

  Description        []

  SideEffects        []
  
  SeeAlso	     []
  
******************************************************************************/
static void
LnodeFree(gGeneric data)
{
complnode_t *no;

  no = (complnode_t *) data;
  (void) lsDestroy(no->pins, LpinFree);
  if(no->expr != NIL(char)) {
    FREE(no->expr);
  }
  FREE(no);  
}

/**Function********************************************************************

  Synopsis           [Allocate memory for a logic graph node. The pin list, as
  the type of the done must be available.]

  Description        []

  SideEffects        []
  
  SeeAlso	     []
  
******************************************************************************/
static complnode_t *
LnodeAlloc(
  int type,
  lsList pinlst,
  char *expr)
{
complnode_t *node;

  node = (complnode_t *) ALLOC(complnode_t, 1);
  node->type = type;
  node->pins = pinlst;
  if(expr != NIL(char)) {
    node->expr = util_strsav(expr);
  }
  else {
    node->expr = NIL(char);
  }

  return node;
}

/**Function********************************************************************

  Synopsis           [Check if the two given logic graph nodes are compatible.]

  Description        []

  SideEffects        []
  
  SeeAlso	     []
  
******************************************************************************/
static int
LnodeCompare(
  complnode_t *node1,
  complnode_t *node2)
{
lsGen gen;
int status;
complpin_t *p1;

  if(node1->type == node2->type) {
    gen = lsStart(node1->pins);    
    status = lsNext(gen, (lsGeneric *) &p1, LS_NH);
    while(status != LS_NOMORE) {
      if(compLpinFind(node2->pins, p1->label) == NIL(complpin_t)) {
        (void) lsFinish(gen);
        return 0;
      }
      status = lsNext(gen, (lsGeneric *) &p1, LS_NH);
    }
    (void) lsFinish(gen);
    return 1;
  } 
  return 0;
}

/**Function********************************************************************

  Synopsis           [Search for the given node within the logic graph.]

  Description        []

  SideEffects        []
  
  SeeAlso	     []
  
******************************************************************************/
static complnode_t *
LnodeFind(
  graph_t *graph,
  complnode_t *node)
{
lsList vertices;
lsGen gen;
int status;
complnode_t *no;
vertex_t *v;

  if(graph == NIL(graph_t)) {
    return NIL(complnode_t);
  }
  vertices = g_get_vertices(graph);
  gen = lsStart(vertices);
  status = lsNext(gen, (lsGeneric *) &v, LS_NH);
  while(status != LS_NOMORE) {
    no = (complnode_t *) v->user_data;
    if(LnodeCompare(no, node) == 1) {
      (void) lsFinish(gen);
      return no;
    }
    status = lsNext(gen, (lsGeneric *) &v, LS_NH);
  }
  (void) lsFinish(gen);
  return NIL(complnode_t); 
}

/**Function********************************************************************

  Synopsis           [Search for an output signal.]

  Description        []

  SideEffects        []
  
  SeeAlso	     []
  
******************************************************************************/
static vertex_t *
OsigFind(
  graph_t *lg,
  char *label)
{
lsGen gen;
lsList vertices;
complnode_t *no;
complpin_t *pin;
vertex_t *v;
int status;

  if(lg == NIL(graph_t)) {
    return NIL(vertex_t);
  }
  vertices = g_get_vertices(lg);
  gen = lsStart(vertices);
  status = lsNext(gen, (lsGeneric *) &v, LS_NH);
  while(status != LS_NOMORE) {
    no = (complnode_t *) v->user_data;
    pin = compLpinFind(no->pins, label);
    if((pin != NIL(complpin_t)) && (pin->type == COMP_OSIGNAL)) {
      (void) lsFinish(gen);
      return v;
    }
    status = lsNext(gen, (lsGeneric *) &v, LS_NH);  
  }
  (void) lsFinish(gen);
  return NIL(vertex_t);
}
