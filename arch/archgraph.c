/**CFile***********************************************************************

  FileName    [archGraph.c]

  PackageName [arch]

  Synopsis    [This file contains several functions for creating and 
  manipulating the FPGA architecture.]

  Description [optional]

  SeeAlso     [comp]

  Author      [Ivan Jeukens]

  Copyright   [1996-1997 Ivan Jeukens]

******************************************************************************/

#include "archInt.h"
#include "fcntl.h"

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

static void EdgeFree(gGeneric data);
static void NodeFree(gGeneric data);

/**AutomaticEnd***************************************************************/


/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis           [Release the memory kept by an architecture.]

  Description        []

  SideEffects        []
  
  SeeAlso	     []
  
******************************************************************************/
void
arch_Free(arch_graph_t *ag)
{
int i;
arch_clabel_t *l;
char *pchar, *key;
st_generator *stgen;

  if(ag == NIL(arch_graph_t)) {
    return;
  }
  for(i = 0;i < array_n(ag->labels);i++) {
    l = array_fetch(arch_clabel_t *, ag->labels, i);
    FREE(l->label);
    FREE(l);    
  }
  array_free(ag->labels);
  for(i = 0;i < array_n(ag->libs);i++) {
    pchar = array_fetch(char *, ag->libs, i);
    FREE(pchar);
  }
  array_free(ag->libs);
  st_foreach_item(ag->nodes, stgen, &key, &pchar) {
    FREE(key);
  }
  st_free_table(ag->nodes);
  FREE(ag->label);
/*  g_free(ag->ag, NULL, NodeFree, EdgeFree); */
}

/**Function********************************************************************

  Synopsis           [Initializations.]

  Description        []

  SideEffects        []
  
  SeeAlso	     []
  
******************************************************************************/
void
arch_Init()
{
  COM_ADD_COMMAND_ARCH("read_arch", arch_ReadCmd);
}

/**Function********************************************************************

  Synopsis           [End command function.]

  Description        []

  SideEffects        []
  
  SeeAlso	     []
  
******************************************************************************/
void
arch_End()
{


}

/**Function********************************************************************

  Synopsis           [Command interface for the arch_read.]

  Description        []

  SideEffects        []
  
  SeeAlso	     []
  
******************************************************************************/
int
arch_ReadCmd(
  array_t **llib,
  arch_graph_t **ag,
  int argc,
  char **argv)
{
  *ag = arch_Read(argv[1], llib);
  
  return 1;
}

/**Function********************************************************************

  Synopsis           [Parse an architecture from a file.]

  Description        []

  SideEffects        []
  
  SeeAlso	     []
  
******************************************************************************/
arch_graph_t *
arch_Read(
  char *fname,
  array_t **llib)
{
FILE *fp;
long size;
char *buf, *war, es[512];
int fd, er, linen;
arch_graph_t *ret;

  if(fname == NIL(char)) return NIL(arch_graph_t);
  fp = fopen(fname,"r");
  if(fp == NIL(FILE)) return NIL(arch_graph_t);
  fseek(fp, 0, SEEK_END);
  size = ftell(fp);
  (void) fclose(fp);
  buf = (char *) ALLOC(char, size);
  fd = open(fname, O_RDONLY);
  read(fd, buf, (size_t) size);
  (void) close(fd);

/*   
  ret = arch_Parse(buf, &linen, &er, &war);
  FREE(buf);

  if(er != ARCH_NO_ERROR) {
    buf = arch_Ecode(er);
    sprintf(es,"%s at line %d.", buf, linen);
    fprintf(msgerr,"%s\n",es);
    FREE(buf);
    arch_Free(ret);
    return NIL(arch_graph_t);
  }
  else {
    buf = arch_Check(ret);
    if(buf != NIL(char)) {
      if(war != NIL(char)) {
        buf = (char *) REALLOC(char, buf, strlen(buf) + strlen(war) + 1);
        strcat(buf, war);
        FREE(war);
      }
      fprintf(msgerr,"%s\n",buf);      
      FREE(buf);
      arch_Free(ret);
      return NIL(arch_graph_t);
    }
    else
      if(war != NIL(char)) {
        fprintf(msgerr,"%s\n", war);
        FREE(war);        
      }
  }
*/

  if(*llib != NIL(array_t)) {
    llib_Free(*llib);  
  }
  *llib = llib_Create(ret);
   
  return ret;
}

/**Function********************************************************************

  Synopsis           [Search for a specified node.]

  Description        []

  SideEffects        []
  
  SeeAlso	     []
  
******************************************************************************/
vertex_t *
arch_NodeFind(
  arch_graph_t *g,
  int x,
  int y,
  int z)
{
char key[100];
vertex_t *v = NIL(vertex_t);
int ret;

  if(g == NIL(arch_graph_t)) {
    return NIL(vertex_t);
  }
  sprintf(key,"%d_%d_%d",x,y,z);
  ret = st_lookup(g->nodes, key, (char **) &v);
    
  if(ret == 0) {
    return NIL(vertex_t);
  }
    
  return v; 
}

/**Function********************************************************************

  Synopsis           [Dump the contents of an architecture.]

  Description        []

  SideEffects        []
  
  SeeAlso	     []
  
******************************************************************************/
void
arch_Dump(arch_graph_t *ag)
{
FILE *fp;
char *name;
int i;
arch_clabel_t *cl;
lsGen gen;
vertex_t *v;
edge_t *e;
arch_edge_t *edge;
arch_node_t *node, *node2;
lsList nl;

  name = (char *) ALLOC(char, (int) strlen(ag->label) + 3);
  strcpy(name, ag->label);
  strcat(name,".a");
  fp = fopen(name, "w");
  if(fp == NIL(FILE)) {
    return;
  }
  FREE(name);

  fprintf(fp,"Architecture File %s\n",ag->label);
  fprintf(fp,"%d ",array_n(ag->labels));
  for(i = 0;i < array_n(ag->labels);i++) {
    cl = array_fetch(arch_clabel_t *, ag->labels, i);
    fprintf(fp,"%s %d ",cl->label, cl->lib);
  }
  fprintf(fp,"\n");
  
  fprintf(fp,"%d ",array_n(ag->libs));
  for(i = 0;i < array_n(ag->libs);i++) {
    name = array_fetch(char *, ag->libs, i);
    fprintf(fp,"%s ",name);
  }
  fprintf(fp,"\n");
  
  nl = g_get_vertices(ag->ag);
  fprintf(fp,"%d ",lsLength(nl));
  lsForeachItem(nl, gen, name) {
    v = (vertex_t *) name;
    node = (arch_node_t *) v->user_data;
    fprintf(fp,"%d %d %d %d %d %d ",node->label, node->x, node->y, node->z,
      node->io, node->logic);    
  }
  lsDestroy(nl, NULL);
  fprintf(fp,"\n");
  
  nl = g_get_edges(ag->ag);
  fprintf(fp,"%d ",lsLength(nl));
  lsForeachItem(nl, gen, name) {  
    e = (edge_t *) name;
    edge = (arch_edge_t *) e->user_data;
    v = g_e_source(e);
    node = (arch_node_t *) v->user_data;
    v = g_e_dest(e);
    node2 = (arch_node_t *) v->user_data;
    fprintf(fp,"%d %d %d %d %d %d ",node->x, node->y, node->z, node2->x,
      node2->y, node2->z);
    fprintf(fp,"%d %d %d %d %d %d %d",edge->pin, edge->pinpos, edge->type,
      edge->adjpin, edge->adjpinpos, edge->adjtype, edge->connect);         
  }
  lsDestroy(nl, NULL);
  
  (void) fclose(fp);
}


/**Function********************************************************************

  Synopsis           [Check the consistency of an architecture.]

  Description        []

  SideEffects        []
  
  SeeAlso	     []
  
******************************************************************************/
char *
arch_Check(arch_graph_t *ag)
{
st_generator *stgen;
char *tok,*tok2;
char tmp[200];
char *ret = NIL(char);
vertex_t *v;
lsList in, out;
lsGen gen;
edge_t *e;
arch_edge_t *edge;
arch_node_t *n1,*n2;

  if(ag == NIL(arch_graph_t)) {
    return util_strsav("No architecture!\n");
  }
  
  st_foreach_item(ag->nodes, stgen, &tok, &tok2) {
    v = (vertex_t *) tok2;
    in = g_get_in_edges(v);
    out = g_get_out_edges(v);
    if((lsLength(in) + lsLength(out)) == 0) {
      sprintf(tmp,"Warning: component %s without connections\n",tok);
      if(ret == NIL(char)) {
        ret = (char *) ALLOC(char *,strlen(tmp)+1);
        strcpy(ret, tmp);              
      }
      else {
        ret = (char *) REALLOC(char *, ret, strlen(ret) + strlen(tmp) + 1);
        strcat(ret, tmp);
      }    
    }
  }
  
  in = g_get_edges(ag->ag);
  lsForeachItem(in, gen, tok) {
    e = (edge_t *) tok;
    edge = (arch_edge_t *) e->user_data;
    v = g_e_source(e);
    n1 = (arch_node_t *) v->user_data;
    v = g_e_dest(e);
    n2 = (arch_node_t *) v->user_data;
    switch(edge->type) {
      case COMP_LINPUT: {
        if(edge->adjtype == COMP_LINPUT) {
          sprintf(tmp,"Invalid conection: logic input(%d %d %d) with logic input(%d %d %d)\n",
            n1->x, n1->y, n1->z, n2->x, n2->y, n2->z);
          if(ret == NIL(char)) {
            ret = (char *) ALLOC(char *,strlen(tmp)+1);
            strcpy(ret, tmp);              
          }
          else {
            ret = (char *) REALLOC(char *, ret, strlen(ret) + strlen(tmp) + 1);
            strcat(ret, tmp);
          }                                 
        }
        else 
          if(edge->adjtype == COMP_SHAREDI) {
            sprintf(tmp,"Invalid conection: logic input(%d %d %d) with shared input(%d %d %d)\n",
              n1->x, n1->y, n1->z, n2->x, n2->y, n2->z);
            if(ret == NIL(char)) {
              ret = (char *) ALLOC(char *,strlen(tmp)+1);
              strcpy(ret, tmp);              
            }
            else {
              ret = (char *) REALLOC(char *, ret, strlen(ret) + strlen(tmp) + 1);
              strcat(ret, tmp);
            }                                           
          }
          else 
            if(edge->adjtype == COMP_CLK) {
              sprintf(tmp,"Invalid conection: logic input(%d %d %d) with clock input(%d %d %d)\n",
                n1->x, n1->y, n1->z, n2->x, n2->y, n2->z);
              if(ret == NIL(char)) {
                ret = (char *) ALLOC(char *,strlen(tmp)+1);
                strcpy(ret, tmp);              
              }
              else {
                ret = (char *) REALLOC(char *, ret, strlen(ret) + strlen(tmp) + 1);
                strcat(ret, tmp);
              }
            }      
      } break;
      case COMP_LOUTPUT: {
        if(edge->adjtype == COMP_LOUTPUT) {
          sprintf(tmp,"Invalid conection: logic output(%d %d %d) with logic output(%d %d %d)\n",
            n1->x, n1->y, n1->z, n2->x, n2->y, n2->z);
          if(ret == NIL(char)) {
            ret = (char *) ALLOC(char *,strlen(tmp)+1);
            strcpy(ret, tmp);              
          }
          else {
            ret = (char *) REALLOC(char *, ret, strlen(ret) + strlen(tmp) + 1);
            strcat(ret, tmp);
          }                                         
        }
        else
          if(edge->adjtype == COMP_SHAREDO) {
            sprintf(tmp,"Invalid conection: logic output(%d %d %d) with shared output(%d %d %d)\n",
              n1->x, n1->y, n1->z, n2->x, n2->y, n2->z);
            if(ret == NIL(char)) {
              ret = (char *) ALLOC(char *,strlen(tmp)+1);
              strcpy(ret, tmp);
            }
            else {
              ret = (char *) REALLOC(char *, ret, strlen(ret) + strlen(tmp) + 1);
              strcat(ret, tmp);
            }          
          }      
      } break;    
    }
  }
  lsDestroy(in, NULL); 
  
  return ret;
}


/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis           [Insert an architecture edge between two nodes.]

  Description        []

  SideEffects        []
  
  SeeAlso	     []
  
******************************************************************************/
void
archEdgeIns(
  arch_graph_t *g,
  arch_node_t *n1,
  arch_node_t *n2,
  int pin,
  int pinpos,
  int pintype,
  int adjpin,
  int adjpinpos,
  int adjtype)
{
edge_t *e;
arch_edge_t *new;
vertex_t *v1,*v2;

  if((n1 == NIL(arch_node_t)) || (n2 == NIL(arch_node_t))) {
    return;
  }
  v1 = arch_NodeFind(g, n1->x, n1->y, n1->z);
  v2 = arch_NodeFind(g, n2->x, n2->y, n2->z);
  if((v1 == NIL(vertex_t)) || (v2 == NIL(vertex_t))) {
    return;
  }
  
  e = g_add_edge(v1, v2);
  new = (arch_edge_t *) ALLOC(arch_edge_t, 1);
  new->pinpos = pinpos;
  new->pin = pin;
  new->type = pintype;
  new->adjpinpos = adjpinpos;
  new->adjpin = adjpin;
  new->adjtype = adjtype;
  new->connect = 0;
  e->user_data = (gGeneric) new;
}

/**Function********************************************************************

  Synopsis           [Insert an architecture node.]

  Description        []

  SideEffects        []
  
  SeeAlso	     []
  
******************************************************************************/
arch_graph_t *
archNodeIns(
  arch_graph_t *g,
  char *label,
  char *lib,
  int x, 
  int y,
  int z,
  short io,
  short logic)
{
arch_node_t *new;
vertex_t *v;
char key[100];
int i;
arch_clabel_t *l;
char *pchar;

  new = (arch_node_t *) ALLOC(arch_node_t, 1);

  for(i = 0;i < array_n(g->labels); i++) {
    l = array_fetch(arch_clabel_t *, g->labels, i);
    if(strcmp(label, l->label) == 0) break;  
  }
  
  if(i < array_n(g->labels)) {
    new->label = i;
  }
  else {
    l = (arch_clabel_t *) ALLOC(arch_clabel_t, 1);
    l->label = util_strsav(label);
    for(i = 0;i < array_n(g->libs);i++) {
      pchar = array_fetch(char *, g->libs, i);
      if(strcmp(pchar, lib) == 0) break;      
    }    
    if(i < array_n(g->libs)) {
      l->lib = i;
    }
    else {
      array_insert_last(char *, g->libs, util_strsav(lib));
      l->lib = array_n(g->libs) - 1;
    }    
    array_insert_last(arch_clabel_t *,g->labels, l);   
    new->label = array_n(g->labels) - 1;     
  }
  new->x = x;
  new->y = y;
  new->z = z;
  new->io = io;
  new->logic = logic;

  v = g_add_vertex(g->ag);  
  v->user_data = (gGeneric) new; 

  sprintf(key,"%d_%d_%d",x,y,z);
  st_insert(g->nodes, util_strsav(key), (char *) v);

  return g;  
}

/**Function********************************************************************

  Synopsis           [Allocate memory for an architecture.]

  Description        []

  SideEffects        []
  
  SeeAlso	     []
  
******************************************************************************/
arch_graph_t *
archAlloc(char *label)
{
arch_graph_t *ret;

  ret = (arch_graph_t *) ALLOC(arch_graph_t, 1);
  ret->label = util_strsav(label);
  ret->ag = g_alloc();
  ret->nodes = st_init_table(strcmp, st_strhash);
  ret->labels = array_alloc(arch_clabel_t *, 0);
  ret->libs = array_alloc(char *, 0);

  return ret;
}

/**Function********************************************************************

  Synopsis           [Print an architecture]

  Description        []

  SideEffects        []
  
  SeeAlso	     []
  
******************************************************************************/
void
archPrint(arch_graph_t *g)
{
lsList vertices, out;
lsGen gen, gen2;
arch_node_t *node,*n2;
arch_edge_t *ae;
vertex_t *v, *v2;
edge_t *e;
arch_clabel_t *l;
int i;
char *pchar;

  if(g == NIL(arch_graph_t)) {
    printf("No architecture.\n");
    return;
  }
  printf("%s\n",g->label);
  vertices = g_get_vertices(g->ag);
  lsForeachItem(vertices, gen, v) {
    node = (arch_node_t *) v->user_data;
    l = array_fetch(arch_clabel_t *, g->labels, node->label);
    printf("Node %s (%d,%d,%d) io = %d logic = %d\n",l->label, node->x,
      node->y, node->z, node->io, node->logic);
    printf("Ligado a:\n");
    out = g_get_out_edges(v);
    lsForeachItem(out, gen2, e) {
      ae = (arch_edge_t *) e->user_data;
      v2 = g_e_dest(e);
      n2 = (arch_node_t *) v2->user_data;
      l = array_fetch(arch_clabel_t *, g->labels, n2->label);
      printf("%s (%d,%d,%d): ",l->label,n2->x,n2->y,n2->z);
      printf("do pino %d(",ae->pin);
      switch(ae->pinpos) {
        case COMP_EAST: printf("EAST) "); break;
        case COMP_WEST: printf("WEST) "); break;
        case COMP_NORTH: printf("NORTH) "); break;
        case COMP_SOUTH: printf("SOUTH) "); break;
        case COMP_UP: printf("UP) "); break;
        case COMP_DOWN: printf("DOWN) "); break;                                        
      }
      printf("tipo ");
      switch(ae->type) {
        case COMP_LINPUT: printf("LOGIC INPUT "); break;
        case COMP_LOUTPUT: printf("LOGIC OUTPUT "); break;
        case COMP_CLK: printf("CLOCK "); break;
        case COMP_ROUTING: printf("ROUTING "); break;
        case COMP_SHAREDI: printf("SHARED INPUT "); break;
        case COMP_SHAREDO: printf("SHARED OUTPUT "); break;
      }
      printf("ao pino %d(",ae->adjpin);
      switch(ae->adjpinpos) {
        case COMP_EAST: printf("EAST) "); break;
        case COMP_WEST: printf("WEST) "); break;
        case COMP_NORTH: printf("NORTH) "); break;
        case COMP_SOUTH: printf("SOUTH) "); break;
        case COMP_UP: printf("UP) "); break;
        case COMP_DOWN: printf("DOWN) "); break;                                        
      }
      printf("tipo ");
      switch(ae->adjtype) {
        case COMP_LINPUT: printf("LOGIC INPUT "); break;
        case COMP_LOUTPUT: printf("LOGIC OUTPUT "); break;
        case COMP_CLK: printf("CLOCK "); break;
        case COMP_ROUTING: printf("ROUTING "); break;
        case COMP_SHAREDI: printf("SHARED INPUT "); break;
        case COMP_SHAREDO: printf("SHARED OUTPUT "); break;
      }
      if(ae->connect) 
        printf(" CONNECTED\n");
      else
        printf(" NOT CONNECTED\n");
    }   
  }
  printf("Used components\n");
  for(i = 0;i < array_n(g->labels);i++) {
    l = array_fetch(arch_clabel_t *, g->labels, i);
    pchar = array_fetch(char *, g->libs, l->lib);
    printf("%s from library %s\n", l->label, pchar);
  }
}

/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis           [Release the memory kept by an architecture edge.]

  Description        []

  SideEffects        []
  
  SeeAlso	     []
  
******************************************************************************/
static void
EdgeFree(gGeneric data)
{
arch_edge_t *e;

  e = (arch_edge_t *) data;
  if(e != NIL(arch_edge_t)) {
    FREE(e);
  }
}

/**Function********************************************************************

  Synopsis           [Release the memory kept by an architecture node.]

  Description        []

  SideEffects        []
  
  SeeAlso	     []
  
******************************************************************************/
static void
NodeFree(gGeneric data)
{
arch_node_t *node;

  node = (arch_node_t *) data;
  if(node != NIL(arch_node_t)) {
    FREE(node);
  }
}
