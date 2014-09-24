/**CFile***********************************************************************

  FileName    [archAbGen.c]

  PackageName [arch]

  Synopsis    [Given an architecture block, extract the corresponding 
  architecture graph.]

  Description []

  SeeAlso     []

  Author      [Ivan Jeukens]

  Copyright   [1996-1997 Ivan Jeukens]

******************************************************************************/

#include "archInt.h"
#include "compInt.h"

/*
static char rcsid[] = \"$Id: $\";
USE(rcsid);
*/

/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/
typedef struct abtype abtype_t;
typedef struct data_table data_table_t;
typedef struct pins pins_t;

/*---------------------------------------------------------------------------*/
/* Structure declarations                                                    */
/*---------------------------------------------------------------------------*/
struct abtype {
  short type;
  union ptr_t{
    arch_abseg_t *seg;
    arch_abcomp_t *comp;
  } ptr_t;
};

struct data_table {
  int x, y;
  array_t *dl;
  short deleted;
};

struct pins {
  char *lib;
  char *component;
  char *pin;
  int number;
  int pos;
  int type;
  short io;
  array_t *links;
};


/**AutomaticStart*************************************************************/

/*---------------------------------------------------------------------------*/
/* Static function prototypes                                                */
/*---------------------------------------------------------------------------*/

static void DataTableFree(array_t *table);
static void pinsFree(array_t *apins);
static int DataTableCmp(char *c1, char *c2);
static short NextInLine(int x, int y, int x1, int y1, int x2, int y2);
static int DataTableFind(array_t *table, int x, int y);
static int AbcompCmpY(lsGeneric o1, lsGeneric o2);
static int AbcompCmpX(lsGeneric o1, lsGeneric o2);
static arch_block_t * InstanceIns(arch_block_t *ab);
static arch_block_t * InstanceRm(arch_block_t *ab);
static array_t * PointsTableCreate(arch_block_t *ab);
static array_t * InterTableCreate(arch_block_t *ab, array_t *points);
static array_t * PinsCreate(array_t **aux, array_t *points);
static array_t * Connect(array_t *links, array_t *points, array_t *inter, arch_abseg_t *seg, int x, int y);
static array_t * LinksCreate(array_t *apins, array_t *points, array_t *inter, array_t *aux);
static arch_graph_t * Pin2Node(arch_graph_t *g, pins_t *apin);
static arch_graph_t * Pin2Edge(arch_graph_t *ag, pins_t *from, array_t *pins, array_t *links);
static void pinsPrint(array_t *pins);
static void DataTablePrint(array_t *data);

/**AutomaticEnd***************************************************************/


/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis           [Extract from an architecture block the corresponding
  architecture graph.]

  Description        []

  SideEffects        []
  
  SeeAlso	     []
  
******************************************************************************/
int
arch_BlockGen(arch_block_t *ab)
{
array_t *points, *inter, *aux, *apins;
pins_t *ap;
int i;
arch_graph_t *g;

  if(ab == NIL(arch_block_t)) {
    return ARCH_NO_ARCH_BLOCK;  
  }

  ab = InstanceRm(ab);
  ab = InstanceIns(ab);

  points = PointsTableCreate(ab);
  inter = InterTableCreate(ab, points);
  apins = PinsCreate(&aux, points);
  apins = LinksCreate(apins, points, inter, aux);

  g = archAlloc(ab->label);
  
  for(i = 0;i < array_n(apins);i++) {
    ap = array_fetch(pins_t *, apins, i);
    g = Pin2Node(g, ap);
    g = Pin2Edge(g, ap, apins, ap->links);
  }
   archPrint(g);
/*    
    strcpy(stmp,"");
    for(i = 0;i < gsize;i++) {
      links = apins[i].links;
      while(links != NULL) {
        links2 = apins[links->index].links;
        while(links2 != NULL) {
          if(links2->index == i) break;
          links2 = links2->next;
        }
        if(links2 == NULL) {          
          apins[links->index].links = ins_link(apins[links->index].links,
            i, links->pdelay);
        }
        links = links->next;
      }

      if(strcmp(stmp,apins[i].lib)) {
        if(lib != NULL) destroy_lib(lib);
        lib = load_library(apins[i].lib, &er, &ln);
        if(lib == NULL) return NO_LIB;        
        strcpy(stmp, apins[i].lib);
      }
      strcpy(label,apins[i].component);
      pchar = strtok(label,"_");
      lib2 = find_comp(lib, pchar);
      if(lib2->component->rg != NULL) {
        ppin = lib2->component->pins;
        while(ppin != NULL) {
          if(!strcmp(ppin->label, apins[i].pin)) {
            if((ppin->type == ROUTING) || (ppin->type == SHAREDI) ||
               (ppin->type == SHAREDO)) {
              rg = lib2->component->rg;
              while(rg != NULL) {
                if(!strcmp(rg->node->label, ppin->label)) {
                  rlist = rg->list;
                  while(rlist != NULL) {
                    j = 0;
                    while(j < gsize) {
                      if(!strcmp(apins[j].pin, rlist->adj->node->label) &&
                         !strcmp(apins[j].component, apins[i].component)) 
                        break;
                      j++;
                    }
                  
                    if(j < gsize) 
                      apins[i].links = ins_link(apins[i].links, j, 0);                                    

                    rlist = rlist->next;
                  }
                  break;
                }
                rg = rg->next;
              }             
            }
            break;
          }
          ppin = ppin->next;
        }
      } 
    }          
         
    strcpy(stmp, a->label);
    strcat(stmp, ".layout");    
    j = save_arch_pins(stmp, apins, gsize);
    if(j != NO_ERROR) return j;
            
    free(aux);
    pinsFree(apins);
    free(points);
    free(inter);
    l = l->next;    
  }  
  
  j = save_draw(a);
  if(j != 0) return FERROR;   
*/

  arch_Free(g);
  
  ab = InstanceRm(ab); 
  DataTableFree(points);
  DataTableFree(inter);
  DataTableFree(aux);
  pinsFree(apins);
  
  return ARCH_NO_ERROR;
}

/**Function********************************************************************

  Synopsis           [Given an error code, return the corresponding error 
  string.]

  Description        []

  SideEffects        []
  
  SeeAlso	     []
  
******************************************************************************/
void 
arch_GenError(
  int error, 
  char *message)
{
  switch(error) {
    case ARCH_NO_ERROR: {
      strcpy(message,"Architecture block successfuly generated");
    } break;
    case ARCH_NO_ARCH_BLOCK: {
      strcpy(message,"No architecture block to be generated");
    } break;
    case ARCH_FERROR: {
      strcpy(message,"File error");
    } break;
    case ARCH_NO_LABEL: {
      strcpy(message,"Missing component label");
    } break;
    case ARCH_NO_LIB: {
      strcpy(message,"Library file not found");
    } break;
    default: strcpy(message,""); break;
  }
}


/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis           [Release the memory associated with an data_table 
  array.]

  Description        []

  SideEffects        []
  
  SeeAlso	     []
  
******************************************************************************/
static void	/* destroy_data_table */
DataTableFree(array_t *table)
{
int i,j;
data_table_t *dt;
abtype_t *at;

  if(table == NIL(array_t)) {
    return;
  }
  
  for(i = 0;i < array_n(table);i++) {
    dt = array_fetch(data_table_t *, table, i);
    if(dt->dl != NIL(array_t)) {
      for(j = 0;j < array_n(dt->dl);j++) {
        at = array_fetch(abtype_t *, dt->dl, j);
        FREE(at);
      }      
      array_free(dt->dl);
    }
    FREE(dt);      
  }
  array_free(table);
}

/**Function********************************************************************

  Synopsis           [Release the memory associated with a pins structure.]

  Description        []

  SideEffects        []
  
  SeeAlso	     []
  
******************************************************************************/
static void		/* destroy_arch_pins */
pinsFree(array_t *apins)
{
pins_t *ap;
int i, k, *n;

  if(apins == NIL(array_t)) {
    return;
  }  
  for(i = 0;i < array_n(apins);i++) {
    ap = array_fetch(pins_t *, apins, i);
    FREE(ap->lib);
    FREE(ap->component);
    FREE(ap->pin);
    if(ap->links != NIL(array_t)) {
      for(k = 0;k < array_n(ap->links);k++) {
        n = array_fetch(int *, ap->links, k);
        FREE(n);        
      }
      array_free(ap->links);  
    }
    FREE(ap);
  }
  array_free(apins);
}

/**Function********************************************************************

  Synopsis           [Compare two instances of a data_table structure.]

  Description        []

  SideEffects        []
  
  SeeAlso	     []
  
******************************************************************************/
static int
DataTableCmp(
  char *c1,
  char *c2)
{
data_table_t **t1;
data_table_t **t2;

  t1 = (data_table_t **) c1;
  t2 = (data_table_t **) c2;
  
  if(((*t1)->deleted == 0) && ((*t2)->deleted == 1)) return -1;
    
  if(((*t1)->deleted == 1) && ((*t2)->deleted == 0)) return 1;
    
  if((*t1)->x < (*t2)->x) return -1;
   
  if((*t1)->x > (*t2)->x) return 1;
    
  if((*t1)->x == (*t2)->x) {
    if((*t1)->y < (*t2)->y) return -1;
      
    if((*t1)->y > (*t2)->y) return 1;
      
    if((*t1)->y == (*t2)->y) return 0;
  }
  return 0;
}

/**Function********************************************************************

  Synopsis           []

  Description        []

  SideEffects        []
  
  SeeAlso	     []
  
******************************************************************************/
static short
NextInLine(
  int x,
  int y,
  int x1,
  int y1,
  int x2,
  int y2)
{
int t;

  if(((x2 > x1) && (y1 < y2)) || ((x2 > x1) && (y2 > y1))) {
    t = x1;
    x1 = x2;
    x2 = t;
    t = y1;
    y1 = y2;
    y2 = t;
  }
    
  if((x1 < x2) && (y1 > y2)) {
    if( (x > x1) && (x < x2) && (y > y2) && (y < y1) && ((x - x1) != 0) &&
      ((x2 - x) != 0) && 
      ( ( (float) ((y1 - y)/(x - x1)) <= (float) ((y - y2)/(x2 - x)) + 0.5 ) ||
        ( (float) ((y1 - y)/(x - x1)) >= (float) ((y - y2)/(x2 - x)) - 0.5 ) ))
      return 1;
  }
  else
    if(y1 == y2) {
      if( (((x > x1) && (x < x2)) || ((x > x2) && (x < x1))) && (y == y1) )
       return 1;
    }
    else
    if((x1 > x2) && (y1 > y2)) {
      if( (x > x2) && (y > y2) && (x < x1) && (y < y1) && ((x - x2) != 0) &&
          ((x1 - x) != 0) &&
          ( ( (float) ((y - y2)/(x - x2)) <= (float) ((y1 - y)/(x1 - x)) + 5 ) ||
            ( (float) ((y - y2)/(x - x2)) >= (float) ((y1 - y)/(x1 - x)) - 5 ) ))
        return 1;    
    }
    if(x1 == x2) {
      if( (((y > y1) && (y < y2)) || ((y > y2) && (y < y1))) && (x == x1) )
        return 1;
    }

  return 0;
}

/**Function********************************************************************

  Synopsis           [Find a data_table instance within an array.]

  Description        []

  SideEffects        []
  
  SeeAlso	     []
  
******************************************************************************/
static int 
DataTableFind(
  array_t *table,
  int x,
  int y)
{
int low, high, i;
data_table_t *dt;

  low = 0;
  high = array_n(table) - 1;
  i = (high + low)/2;
  
  while(low <= high) {
    dt = array_fetch(data_table_t *, table, i);    
    if((dt->x == x) && (dt->y == y)) 
      return i;
    else
      if(x > dt->x) 
        low = i + 1;
      else
        if(x < dt->x)
          high = i - 1;
        else
          if(x == dt->x) {
            if(y > dt->y)
              low = i + 1;
            else
              high = i - 1;           
          }
    i = (low + high)/2;
  }     
  return -1;
}


/**Function********************************************************************

  Synopsis           [Compare two instances of the ab_comp structure, based
  on their Y coordinate.]

  Description        []

  SideEffects        []
  
  SeeAlso	     []
  
******************************************************************************/
static int
AbcompCmpY(
  lsGeneric o1,
  lsGeneric o2)
{
arch_abcomp_t *aux1, *aux2;

  aux1 = (arch_abcomp_t *) o1;
  aux2 = (arch_abcomp_t *) o2;

  if(aux1->y > aux2->y) {
    return 1;
  }
  else 
    if(aux1->y < aux2->y) {
      return -1; 
    }
    else {
      if(aux1->x > aux2->x) {
        return 1;
      }
      else 
        if(aux1->x < aux1->x) {
          return -1;        
        }
    }
  return 0;
}

/**Function********************************************************************

  Synopsis           [Compare two instances of the ab_comp structure, based
  on their X coordinate.]

  Description        []

  SideEffects        []
  
  SeeAlso	     []
  
******************************************************************************/
static int
AbcompCmpX(
  lsGeneric o1,
  lsGeneric o2)
{
arch_abcomp_t *aux1, *aux2;

  aux1 = (arch_abcomp_t *) o1;
  aux2 = (arch_abcomp_t *) o2;

  if(aux1->x > aux2->x) {
    return 1;
  }
  else 
    if(aux1->x < aux2->x) {
      return -1;
    }
    else {
      if(aux1->y > aux2->y) {
        return 1;
      }
      else 
        if(aux1->y < aux1->y) {
          return -1;        
        }
    }

  return 0;
}

/**Function********************************************************************

  Synopsis           [Insert a position for each component within an 
  architecture block.]

  Description        []

  SideEffects        []
  
  SeeAlso	     []
  
******************************************************************************/
static arch_block_t *
InstanceIns(arch_block_t *ab)
{
char label[30];
lsGen gen;
arch_abcomp_t *cptr;
int count, lastp;

  if(ab == NIL(arch_block_t)) {
    return ab;
  }
  
  (void) lsSort(ab->comp, AbcompCmpX);
  
  count = -1;
  lastp = -1;
  lsForeachItem(ab->comp, gen, cptr) {
    if(cptr->x > (lastp + 10)) {
      count++;
      lastp = cptr->x;
    }
    sprintf(label,"_%d",count);
    cptr->label = (char *) REALLOC(char, cptr->label, strlen(cptr->label) +
      strlen(label) + 1);
    strcat(cptr->label, label);  
  }

  (void) lsSort(ab->comp, AbcompCmpY);

  count = -1;
  lastp = -1;
  lsForeachItem(ab->comp, gen, cptr) {
    if(cptr->y > (lastp + 10)) {
      count++;
      lastp = cptr->y;
    }
    sprintf(label,"_%d",count);
    cptr->label = (char *) REALLOC(char, cptr->label, strlen(cptr->label) +
      strlen(label) + 1);
    strcat(cptr->label, label);  
  }    

  return ab;
}

/**Function********************************************************************

  Synopsis           [Remove the position of each component within an 
  architecture block.]

  Description        []

  SideEffects        []
  
  SeeAlso	     []
  
******************************************************************************/
static arch_block_t *
InstanceRm(arch_block_t *ab)
{
char *pchar;
char label[30];
lsGen gen;
arch_abcomp_t *cptr;

  if(ab == NIL(arch_block_t)) return ab;
  
  lsForeachItem(ab->comp, gen, cptr) {
    pchar = strtok(cptr->label,"_");    
    strcpy(label, pchar);
    FREE(cptr->label);
    cptr->label = util_strsav(label);
  }

  return ab;
}

/**Function********************************************************************

  Synopsis           [Extract a data_table of all points (segments end points
  and components pins endpoints) of an architecture block.]

  Description        []

  SideEffects        []
  
  SeeAlso	     []
  
******************************************************************************/
static array_t *
PointsTableCreate(arch_block_t *ab)
{
array_t *ret;
lsGen gen;
arch_abcomp_t *cptr;
arch_abseg_t *sptr;
data_table_t *dt, *dt2;
array_t *pins;
comp_figpin_t *fptr;
abtype_t *at, *at2;
int i, j, k;

  ret = array_alloc(data_table_t *, 0);
  lsForeachItem(ab->comp, gen, cptr) {
    pins = cptr->fig->pins;
    for(i = 0;i < array_n(pins);i++) {
      fptr = array_fetch(comp_figpin_t *, pins, i);
      dt = (data_table_t *) ALLOC(data_table_t, 1);
      dt->x = fptr->dx2 + cptr->x;
      dt->y = fptr->dy2 + cptr->y;
      dt->deleted = 0;
      dt->dl = array_alloc(abtype_t *,0);
      at = (abtype_t *) ALLOC(abtype_t ,1);
      at->type = ARCHCOMP;
      at->ptr_t.comp = cptr;
      array_insert_last(abtype_t *, dt->dl, at);      
      array_insert_last(data_table_t *, ret, dt);
    }          
  }

  lsForeachItem(ab->seg, gen, sptr) {
    dt = (data_table_t *) ALLOC(data_table_t, 1);
    dt->x = sptr->x;
    dt->y = sptr->y;
    dt->deleted = 0;
    at = (abtype_t *) ALLOC(abtype_t, 1);
    at->type = ARCHSEG;
    at->ptr_t.seg = sptr;
    dt->dl = array_alloc(abtype_t *, 0);
    array_insert_last(abtype_t *, dt->dl, at);
    array_insert_last(data_table_t *, ret, dt);

    dt = (data_table_t *) ALLOC(data_table_t, 1);
    dt->x = sptr->x2;
    dt->y = sptr->y2;
    dt->deleted = 0;
    at = (abtype_t *) ALLOC(abtype_t, 1);
    at->type = ARCHSEG;
    at->ptr_t.seg = sptr;
    dt->dl = array_alloc(abtype_t *, 0);
    array_insert_last(abtype_t *, dt->dl, at);
    array_insert_last(data_table_t *, ret, dt);
  }       
  (void) array_sort(ret, DataTableCmp); 
  
  if(array_n(ret) < 1) return ret;
  
  i = 0;
  j = 1;
  dt = array_fetch(data_table_t *, ret, 0);
  dt2 = array_fetch(data_table_t *, ret, 1);
  while(1) {
    while((dt->x >= dt2->x - 2) &&
          (dt->x <= dt2->x + 2)) {
      if((dt2->deleted != 1) && (dt->y >= dt2->y - 2) &&
         (dt->y <= dt2->y + 2)) {
        for(k = 0;k < array_n(dt2->dl);k++) {
          at = array_fetch(abtype_t *, dt2->dl, k);
          at2 = (abtype_t *) ALLOC(abtype_t, 1);
          at2->type = at->type;
          if(at->type == ARCHCOMP) 
            at2->ptr_t.comp = at->ptr_t.comp;
          else
            at2->ptr_t.seg = at->ptr_t.seg;
          array_insert_last(abtype_t *, dt->dl, at2);
        }
        dt2->deleted = 1;      
      }
      j++;           
      if(j == array_n(ret)) break;         
      dt2 = array_fetch(data_table_t *, ret, j);      
    }      
    i++;
    if(i == array_n(ret)) break;
    dt = array_fetch(data_table_t *, ret, i);
    while(dt->deleted != 0) {
      i++;
      if(i == array_n(ret)) break;      
      dt = array_fetch(data_table_t *, ret, i);
    }
    
    j = i+1;
    if(j == array_n(ret)) break;   
    dt2 = array_fetch(data_table_t *, ret, j);
    while(dt2->deleted != 0) {
      j++;
      if(j == array_n(ret)) break;         
      dt2 = array_fetch(data_table_t *, ret, j);      
    }
  }

  (void) array_sort(ret, DataTableCmp);   
  for(i = 0;i < array_n(ret);i++) {
    dt = array_fetch(data_table_t *, ret, i);
    if(dt->deleted == 1) break;
  }
  
  for(j = i;j < array_n(ret);j++) {
    dt = array_fetch(data_table_t *, ret, j);
    for(k = 0;k < array_n(dt->dl);k++) {
      at = array_fetch(abtype_t *, dt->dl, k);
      FREE(at);
    }
    array_free(dt->dl);
    FREE(dt);
  }  
  ret->num = i;
  ret->space = (char *) REALLOC(char, ret->space, ret->obj_size * ret->num);

  return ret;
}

/**Function********************************************************************

  Synopsis           [Extract a data_table of all points (segments end points
  and components pins endpoints) of an architecture block.]

  Description        []

  SideEffects        []
  
  SeeAlso	     []
  
******************************************************************************/
static array_t *
InterTableCreate(
  arch_block_t *ab,
  array_t *points)
{
array_t *ret;
lsGen gen;
arch_abseg_t *sptr;
data_table_t *dt, *dt2;
abtype_t *at, *at2;
int i,j;

  ret = array_alloc(data_table_t *, 0);

  lsForeachItem(ab->seg, gen, sptr) {
    for(i = 0;i < array_n(points);i++) {
      dt = array_fetch(data_table_t *, points, i);
      if(NextInLine(dt->x, dt->y, sptr->x, sptr->y, sptr->x2, sptr->y2)) {
        dt2 = (data_table_t *) ALLOC(data_table_t, 1);
        dt2->x = dt->x;
        dt2->y = dt->y;
        dt2->deleted = 0;
        dt2->dl = array_alloc(abtype_t *, 0);
        for(j = 0;j < array_n(dt->dl);j++) {
          at = array_fetch(abtype_t *, dt->dl, j);
          at2 = (abtype_t *) ALLOC(abtype_t, 1);
          at2->type = at->type;
          if(at->type == ARCHCOMP) {
            at2->ptr_t.comp = at->ptr_t.comp;
          }
          else {
            at2->ptr_t.seg = at->ptr_t.seg;
          }
          array_insert_last(abtype_t *, dt2->dl, at2);          
        }       
        at2 = (abtype_t *) ALLOC(abtype_t, 1);
        at2->type = ARCHSEG;
        at2->ptr_t.seg = sptr;
        array_insert_last(abtype_t *, dt2->dl, at2);
        array_insert_last(data_table_t *, ret, dt2);              
      }    
    }  
  }
  (void) array_sort(ret, DataTableCmp);

  return ret;
}

/**Function********************************************************************

  Synopsis           [Create a table of all component pins.]

  Description        []

  SideEffects        []
  
  SeeAlso	     []
  
******************************************************************************/
static array_t *
PinsCreate(
  array_t **aux,
  array_t *points)
{
array_t *ret, *pins;
char curlib[40];
int i,j, k;
data_table_t *dt, *dt2;
abtype_t *at;
pins_t *ap;
lsList lib = NULL;
char label[30];
comp_data_t *c;
comp_figpin_t *fptr;
comp_pin_t *cp;
char *pchar;

  ret = array_alloc(pins_t *, 0);
  *aux = array_alloc(data_table_t *, 0);
  
  strcpy(curlib,"");  
  for(i = 0;i < array_n(points);i++) {
    dt = array_fetch(data_table_t *, points, i);
    for(j = 0;j < array_n(dt->dl);j++) {
      at = array_fetch(abtype_t *, dt->dl, j);      
      if(at->type == ARCHCOMP) {
        dt2 = (data_table_t *) ALLOC(data_table_t, 1);
        dt2->x = dt->x;
        dt2->y = dt->y;
        dt2->deleted = 0;
        dt2->dl = NIL(array_t);

        ap = (pins_t *) ALLOC(pins_t, 1);
        ap->lib = util_strsav(at->ptr_t.comp->library);
        ap->component = util_strsav(at->ptr_t.comp->label);        
        
        if(strcmp(curlib, ap->lib) != 0) {
          if(lib != NULL) comp_LibFree(lib);
          lib = comp_LibLoad(ap->lib);
          strcpy(curlib, ap->lib);
        }
        strcpy(label, ap->component);
        pchar = strtok(label,"_");
        c = comp_Find(lib, label);
        pins = at->ptr_t.comp->fig->pins;
        for(k = 0;k < array_n(pins);k++) {
          fptr = array_fetch(comp_figpin_t *, pins, k);
          if((dt->x == (fptr->dx2 + at->ptr_t.comp->x)) &&
             (dt->y == (fptr->dy2 + at->ptr_t.comp->y)))
            break;
        }       
        cp = array_fetch(comp_pin_t *, c->pins, k);                         
        ap->pin = util_strsav(cp->label);
        ap->number = k;
        ap->type = cp->type;
        ap->pos = cp->pos;
        ap->links = array_alloc(int *, 0);
        ap->io = at->ptr_t.comp->io; 
        array_insert_last(pins_t *, ret, ap);
        array_insert_last(data_table_t *, (*aux), dt2);
      }   
    }
  }
  
  (void) array_sort((*aux), DataTableCmp);

  return ret;
}

/**Function********************************************************************

  Synopsis           [A recursive procedure for finding the endpoint of a
  pin connection.]

  Description        []

  SideEffects        []
  
  SeeAlso	     []
  
******************************************************************************/
static array_t *
Connect(
  array_t *links,
  array_t *points,
  array_t *inter,
  arch_abseg_t *seg,
  int x,
  int y)
{
int x1,y1,i,*n,j;
data_table_t *dt, *dt2;
abtype_t *at;
  
  if((seg->x == x) && (seg->y == y)) {
    x1 = seg->x2;
    y1 = seg->y2;
  }
  else {
    x1 = seg->x;
    y1 = seg->y;
  }
   
  i = DataTableFind(points, x1, y1);
  dt = array_fetch(data_table_t *, points, i);

  if(dt->deleted == 1) {
    return links;
  }
  else {
    dt->deleted = 1;
  }

  for(j = 0;j < array_n(dt->dl);j++) {
    at = array_fetch(abtype_t *, dt->dl, j);
    if(at->type == ARCHCOMP) break;
  }

  if(j < array_n(dt->dl)) {
    n = (int *) ALLOC(int, 1);
    *n = i;
    array_insert_last(int *, links, n);
  }
  else {
    for(i = 0;i < array_n(dt->dl);i++) {
      at = array_fetch(abtype_t *, dt->dl, i);
      links = Connect(links, points, inter, at->ptr_t.seg, 
        dt->x, dt->y);
    }
    
    i = DataTableFind(inter, dt->x, dt->y);
    if(i != -1) {
      dt2 = array_fetch(data_table_t *, inter, i);
      for(i = 0;i < array_n(dt2->dl);i++) {
        at = array_fetch(abtype_t *, dt2->dl, i);
        links = Connect(links, points, inter, at->ptr_t.seg,
          at->ptr_t.seg->x, at->ptr_t.seg->y);
        links = Connect(links, points, inter, at->ptr_t.seg,
          at->ptr_t.seg->x2, at->ptr_t.seg->y2);        
      }    
    }      
  }
     
  return links;
}

/**Function********************************************************************

  Synopsis           [Based on the points and intersection table, create
  the required links between components pins.]

  Description        []

  SideEffects        []
  
  SeeAlso	     []
  
******************************************************************************/
static array_t *
LinksCreate(
  array_t *apins,
  array_t *points,
  array_t *inter,
  array_t *aux)
{
int i, j, k, m, *index;
data_table_t *dta, *dt2;
abtype_t *at;
pins_t *ap;
  
  for(i = 0;i < array_n(apins);i++) {
    ap = array_fetch(pins_t *, apins, i);
    dta = array_fetch(data_table_t *, aux, i);
    j = DataTableFind(points, dta->x, dta->y);
    dta = array_fetch(data_table_t *, points, j);
    for(j = 0;j < array_n(dta->dl);j++) {
      at = array_fetch(abtype_t *, dta->dl, j);
      if(at->type == ARCHSEG) {
        dta->deleted = 1;
        ap->links = Connect(ap->links, points, inter, at->ptr_t.seg,
          dta->x, dta->y);
        for(k = 0;k < array_n(points);k++) {
          dt2 = array_fetch(data_table_t *, points, k);
          dt2->deleted = 0;
        }
      }    
      
      for(k = 0;k < array_n(ap->links);k++) {
        index = array_fetch(int *, ap->links, k);
        dt2 = array_fetch(data_table_t *, points, *index);        
        m = DataTableFind(aux, dt2->x, dt2->y);
        *index = m;
      }
    }    
  }

  return apins;
}

/**Function********************************************************************

  Synopsis           [Create an architecture graph node based on a architecture
  block pin.]

  Description        []

  SideEffects        []
  
  SeeAlso	     []
  
******************************************************************************/
static arch_graph_t *
Pin2Node(
  arch_graph_t *g,
  pins_t *apin)
{
int x, y;
vertex_t *v;
char label[100], *pchar, name[100];
comp_data_t *c;
lsList lib;
  
  if(apin == NIL(pins_t)) {
    return g;
  }  
  lib = comp_LibLoad(apin->lib);
  
  if(apin->type == COMP_ROUTING) {
    strcpy(label, apin->component);
    pchar = strtok(label, "_");
    strcpy(name, pchar);
    c = comp_Find(lib, name);  
    pchar = strtok(NULL,"_");
    x = atoi(pchar);
    pchar = strtok(NULL,"_");
    y = atoi(pchar);
    v = arch_NodeFind(g, x, y, 0);
    if(v == NIL(vertex_t)) {
      g = archNodeIns(g, name, apin->lib, x, y, 0, apin->io, comp_Type(c));
    }
  }
  else
    if((apin->type == COMP_LINPUT) || (apin->type == COMP_LOUTPUT) || 
       (apin->type == COMP_CLK)) {
      strcpy(label, apin->component);
      pchar = strtok(label, "_");
      strcpy(name, pchar);
      c = comp_Find(lib, name);  
      pchar = strtok(NULL,"_");
      x = atoi(pchar);
      pchar = strtok(NULL,"_");
      y = atoi(pchar);
      v = arch_NodeFind(g, x, y, 0);
      if(v == NIL(vertex_t)) {
        g = archNodeIns(g, name, apin->lib, x, y, 0, apin->io, comp_Type(c));
      }
    }    

  comp_LibFree(lib);
  return g;
}

/**Function********************************************************************

  Synopsis           [Insert the various graph edges based on the architecture
  block pin.]

  Description        []

  SideEffects        []
  
  SeeAlso	     []
  
******************************************************************************/
static arch_graph_t *
Pin2Edge(
  arch_graph_t *ag,
  pins_t *from,
  array_t *pins,
  array_t *links)
{
int i, *index, x, y;
pins_t *p;
vertex_t *v, *v2;
char label[100], *pchar, name[100];
lsList lib;
comp_data_t *c;

  strcpy(label, from->component);
  pchar = strtok(label,"_");
  pchar = strtok(NULL,"_");
  x = atoi(pchar);
  pchar = strtok(NULL,"_");
  y = atoi(pchar);      
  v2 = arch_NodeFind(ag, x, y, 0);
  
  for(i = 0;i < array_n(links);i++) {
    index = array_fetch(int *, links, i);
    p = array_fetch(pins_t *, pins, *index);
    strcpy(label, p->component);
    pchar = strtok(label,"_");
    strcpy(name, pchar);
    pchar = strtok(NULL,"_");
    x = atoi(pchar);
    pchar = strtok(NULL,"_");
    y = atoi(pchar);      
    v = arch_NodeFind(ag, x, y, 0);
    if(v == NIL(vertex_t)) {
      lib = comp_LibLoad(p->lib);
      c = comp_Find(lib, name);
      ag = archNodeIns(ag, name, p->lib, x, y, 0, p->io, comp_Type(c));
      comp_LibFree(lib);
      v = arch_NodeFind(ag, x, y, 0);
    }        
    archEdgeIns(ag, (arch_node_t *) v2->user_data, 
      (arch_node_t *) v->user_data, from->number, from->pos, from->type,
      p->number, p->pos, p->type);
    archEdgeIns(ag, (arch_node_t *) v->user_data, 
      (arch_node_t *) v2->user_data, p->number, p->pos, p->type,
      from->number, from->pos, from->type);
  }  
    
  return ag;
}

#ifdef DEBUG
/**Function********************************************************************

  Synopsis           [Print a pins_t structure.]

  Description        []

  SideEffects        []
  
  SeeAlso	     []
  
******************************************************************************/
static void
pinsPrint(array_t *pins)
{
int i,j, *index;
pins_t *ap, *ap2;

  for(i = 0;i < array_n(pins);i++) {
    ap = array_fetch(pins_t *, pins, i);
    printf("%s pins %s ligado a:\n",ap->component, ap->pin);
    if(ap->links != NIL(array_t)) {
      for(j = 0;j < array_n(ap->links);j++) {
        index = array_fetch(int *, ap->links, j);
        ap2 = array_fetch(pins_t *, pins, *index);
        printf("%s pino %s\n",ap2->component, ap2->pin);
      }
    }
    printf("\n");
  }
}

/**Function********************************************************************

  Synopsis           [Print data table structure.]

  Description        []

  SideEffects        []
  
  SeeAlso	     []
  
******************************************************************************/
static void
DataTablePrint(array_t *data)
{
data_table_t *dt;
int i, j;
abtype_t *at;

  if(data == NIL(array_t)) return;

  for(i = 0;i < array_n(data);i++) {
    dt = array_fetch(data_table_t *, data, i);    
    printf("X = %d  Y = %d  Deleted = %d\n",dt->x, dt->y, dt->deleted);
    if(dt->dl != NIL(array_t)) {
      for(j = 0;j < array_n(dt->dl);j++) {
        at = array_fetch(abtype_t *, dt->dl, j);
        if(at->type == ARCHCOMP)
          printf("Component %s at x = %d y = %d\n",at->ptr_t.comp->label,
            at->ptr_t.comp->x, at->ptr_t.comp->y);      
        else 
          printf("Segment from (%d,%d) to (%d,%d)\n",at->ptr_t.seg->x,
            at->ptr_t.seg->y, at->ptr_t.seg->x2, at->ptr_t.seg->y2);
      }
    }
  }
  printf("\n%d points\n",array_n(data));
}
#endif
