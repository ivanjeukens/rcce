/**CFile***********************************************************************

  FileName    [compUtil.c]

  PackageName [comp]

  Synopsis    [This file contain several procedures for handling the comp
  data structure.]

  Description [This file contain several procedures for handling the comp
  data structure.]

  SeeAlso     [comp.h]

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

static int comparePins(char *c1, char *c2);

/**AutomaticEnd***************************************************************/


/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis           [Release the memory hold by an array of compfigpin.]

  Description        []

  SideEffects        []
  
  SeeAlso	     []
  
******************************************************************************/
void
comp_FpinFree(array_t *pins)
{
int i;
comp_figpin_t *ptr;

  if(pins != NIL(array_t)) {
    for(i = 0;i < array_n(pins);i++) {
      ptr = array_fetch(comp_figpin_t *,pins,i);
      FREE(ptr);
    }
    array_free(pins);
  }
}

/**Function********************************************************************

  Synopsis           [Release the memory hold by a comp_data_t.]

  Description        []

  SideEffects        []
  
  SeeAlso	     []
  
******************************************************************************/
void
comp_Free(comp_data_t *c)
{
int i;
comp_pin_t *ptr;

  if(c != NIL(comp_data_t)) {
    if(c->label != NIL(char)) {
      FREE(c->label);
    }
    for(i = 0;i < array_n(c->pins);i++) {
      ptr = array_fetch(comp_pin_t *, c->pins, i);
      if(ptr->label != NIL(char)) {
        FREE(ptr->label);
      }
      FREE(ptr);               
    }
    array_free(c->pins);
    compRgraphFree(c->rg);
    compLgraphFree(c->lg);
    if(c->fig != NIL(comp_fig_t)) {
      comp_FpinFree(c->fig->pins);
      FREE(c->fig);
    }
    if(c->text != NIL(char)) {
      FREE(c->text);
    }
    if(c->delay != NIL(compconstr_t)) {
      FREE(c->delay);
    }
    FREE(c);
  }
}

/**Function********************************************************************

  Synopsis           [Release the memory hold by a component library.]

  Description        []

  SideEffects        []
  
  SeeAlso	     []
  
******************************************************************************/
void
comp_LibFree(lsList lib)
{
  lsDestroy(lib, comp_Free);
}

/**Function********************************************************************

  Synopsis           [Save the contents of a library of components.]

  Description        [Save the contents of a library of components. Currently,
  each component is saved by writing the text file describing it. This text
  is a correct formation of the component grammar, since it was successfully
  parsed.]

  SideEffects        []
  
  SeeAlso	     []
  
******************************************************************************/
void
comp_LibSave(
  lsList lib,
  char *filename)
{
FILE *fp;
comp_data_t *ptr;
lsGen gen;

  fp = fopen(filename,"w");
  if(fp != NIL(FILE)) {
    (void) fprintf(fp,"Component Library ");
    lsForeachItem(lib, gen, ptr) {
      (void) fprintf(fp,"%s ",ptr->label);
      (void) fprintf(fp,"%s",ptr->text);          
    }
    (void) fclose(fp);
  }
  else {
    (void) fprintf(siserr,"Could not open file %s for writing.\n", filename);
  }
}

/**Function********************************************************************

  Synopsis           [Find a component.]

  Description        []

  SideEffects        []
  
  SeeAlso	     []
  
******************************************************************************/
comp_data_t *
comp_Find(
  lsList lib,
  char *label)
{
comp_data_t *ptr;
lsGen gen;

  if(lib == NULL) {
    return NIL(comp_data_t);
  }  
  lsForeachItem(lib, gen, ptr) {
    if(strcmp(ptr->label, label) == 0) {
      return ptr;
    }
  }
  return NIL(comp_data_t);
}

/**Function********************************************************************

  Synopsis           [Delete a component from a library.]

  Description        []

  SideEffects        []
  
  SeeAlso	     []
  
******************************************************************************/
lsList
comp_Delete(
  lsList lib,
  char *label)
{
lsGen gen;
lsHandle handle;
comp_data_t *ptr;
int status;

  if(lib == NULL) {
    return NULL;
  }
  gen = lsStart(lib);
  status = lsNext(gen, (lsGeneric *) &ptr, &handle);
  while(status != LS_NOMORE) {
    if(strcmp(ptr->label, label) == 0) {
      lsRemoveItem(handle, (lsGeneric *) &ptr);
      comp_Free(ptr);
      lsFinish(gen);
      return lib;
    }
    status = lsNext(gen, (lsGeneric *) &ptr, &handle);  
  }
  lsFinish(gen);
  return lib;
}

/**Function********************************************************************

  Synopsis           [Add a pin to the component figure.]

  Description        []

  SideEffects        []
  
  SeeAlso	     []
  
******************************************************************************/
array_t *
comp_FpinIns(
  array_t *pins,
  int dx1,
  int dx2,
  int dy1,
  int dy2,
  int n)
{
comp_figpin_t *new;

  new = (comp_figpin_t *) ALLOC(comp_figpin_t,1);
  new->dx1 = dx1;
  new->dx2 = dx2;
  new->dy1 = dy1;
  new->dy2 = dy2;
  new->number = n;
  
  array_insert_last(comp_figpin_t *, pins, new);  
  return pins;
}

/**Function********************************************************************

  Synopsis           [Copy a component figure.]

  Description        []

  SideEffects        []
  
  SeeAlso	     []
  
******************************************************************************/
comp_data_t *
comp_FigCopy(comp_data_t *c)
{
comp_data_t *newcomp;
comp_figpin_t *ptr;
int i;

  if(c == NIL(comp_data_t)) {
    return NIL(comp_data_t);
  }
  newcomp = compAlloc(c->label);
  newcomp->fig->width = c->fig->width;
  newcomp->fig->height = c->fig->height;
  
  for(i = 0;i < array_n(c->fig->pins);i++) {
    ptr = array_fetch(comp_figpin_t *, c->fig->pins, i);
    newcomp->fig->pins = comp_FpinIns(newcomp->fig->pins,
      ptr->dx1, ptr->dy1, ptr->dx2, ptr->dy2, ptr->number);
  }
  
  return newcomp;
}

/**Function********************************************************************

  Synopsis           [Return the type of the component (1 == Routing,
  2 = Logic, 3 = Routing & Logic).]

  Description        []

  SideEffects        []
  
  SeeAlso	     []
  
******************************************************************************/
short
comp_Type(comp_data_t *c)
{
int i;
short rc = 0,lc = 0;
comp_pin_t *pin;

  for(i = 0;i < array_n(c->pins);i++) {
    pin = array_fetch(comp_pin_t *, c->pins, i);
    if(pin->type == COMP_ROUTING) 
      rc = 1;
    else
      if((pin->type == COMP_LINPUT) || (pin->type == COMP_LOUTPUT) || 
         (pin->type == COMP_CLK))
        lc = 1;
      else
        if((pin->type == COMP_SHAREDI) || (pin->type == COMP_SHAREDO)) {
          rc = 1;
          lc = 1;
        }
    if(lc && rc) break;        
  }
  if(rc & !lc) return COMP_RC;
  if(lc & !rc) return COMP_LC;
  return COMP_RLC;
}

/**Function********************************************************************

  Synopsis           [Load a library of components.]

  Description        []

  SideEffects        []
  
  SeeAlso	     []
  
******************************************************************************/
lsList
comp_LibLoad(char *filename)
{
lsList lib = lsCreate();
comp_data_t *tmp = NIL(comp_data_t);
char tok1[80],tok2[80],tok3[80],tok4[80],tok5[80];
FILE *fp;
int size = 0,end = 0, i, error, linen;
char *data = NIL(char),*pchar, *war;

  fp = fopen(filename,"r");
  if(fp != NIL(FILE)) {
    fscanf(fp,"%s %s",tok1,tok2);   
    if((strcmp(tok1,"Component") == 0) &&
       (strcmp(tok2,"Library") == 0)) {
      fscanf(fp,"%s",tok2);
      while(!feof(fp)) {
        fscanf(fp,"%s",tok1);
        strcpy(tok3,tok1);        
        strcat(tok1," ");
        for(i = 0;i < (int) strlen(tok1);i++) 
          tok1[i] = tolower(tok1[i]);      
        while((end < 3) && !feof(fp)) {
          if(strcmp(tok1,"end ") == 0) { 
           end++;
           strcat(tok1,"\n");
           strcat(tok3,"\n");
          }
          if((tok1[strlen(tok1) - 2] == ';') ||
             (strcmp(tok1,"pin ") == 0) ||
             (strcmp(tok1,"layout ") == 0) ||
             (strcmp(tok1,"structure ") == 0)) {
            strcat(tok1,"\n");
            strcat(tok3,"\n");
          }
          data = (char *) REALLOC(char, data, size + strlen(tok3) + 1);
          if(size == 0)
            strcpy(data,tok3);
          else
            strcat(data,tok3);              
          size += strlen(data);
          fscanf(fp,"%s",tok1);
          strcpy(tok3,tok1);
          strcat(tok1," ");
          for(i = 0;i < (int) strlen(tok1);i++) 
            tok1[i] = tolower(tok1[i]);      
          if(strcmp(tok1,"pin ") && strcmp(tok1,"layout ") &&
             strcmp(tok1,"structure ") && strcmp(tok1,"end ")) {
            strcpy(tok4,tok3);
            strcpy(tok3," ");
            strcat(tok3,tok4);
          }                 
        }
        tmp = comp_Parse(data, &linen, &error);
        if(error != NO_ERROR) {
          FREE(data);
          comp_LibFree(lib);          
          (void) fclose(fp);
          data = comp_Error(error);
          fprintf(siserr,"Parse error: %s at line %d\n",data,linen);
          FREE(data);
          return NULL;
        }
        FREE(tmp->label);
        strcpy(tok5, tok2);
        pchar = strtok(tok5," ");        
        tmp->label = util_strsav(pchar);
        FREE(tmp->text);
        tmp->text = util_strsav(data);
/*   
        war = comp_Check(tmp);
        if(war != NIL(char)) {
          fprintf(siserr,"Warnings:\n%s",war);
          FREE(war);  
        }             
*/        
        comp_Ins(lib, tmp);            
        FREE(data);      
        data = NIL(char);
        tmp = NIL(comp_data_t);
        end = 0;
        size = 0;
        strcpy(tok2,tok1);
      }
    }        
  }
  else {
    fprintf(siserr,"Could not open file %s for reading.\n",filename);
    return NULL;
  }  

  (void) fclose(fp);
  return lib;
}

/**Function********************************************************************

  Synopsis           [Check if a component is well constructed.]

  Description        []

  SideEffects        []
  
  SeeAlso	     []
  
******************************************************************************/
char *
comp_Check(comp_data_t *c)
{
char *ret = NIL(char),*key, *value;
comp_pin_t *cptr;
int i;
short in = 0, out = 0;
lsList ver;
lsGen gen, genpin;
complnode_t *ln;
complpin_t *pin;
vertex_t *vertex;
st_table *ins, *itmps, *otmps;
st_generator *stgen;

  ret = (char *) ALLOC(char, 1);
  ret[0] = '\0';
  
  if(array_n(c->pins) == 0) {
    ret = (char *) REALLOC(char, ret, strlen(ret) + 18);
    strcat(ret, "No pins declared.\n");  
  }
  ins = st_init_table(strcmp, st_strhash);
  itmps = st_init_table(strcmp, st_strhash);
  otmps = st_init_table(strcmp, st_strhash);
   
  for(i = 0;i < array_n(c->pins);i++) {
    cptr = array_fetch(comp_pin_t *, c->pins, i);
    switch(cptr->type) {
      case COMP_LOUTPUT:
      case COMP_SHAREDO: {
        out = 1; 
      } break;
      case COMP_LINPUT: 
      case COMP_SHAREDI: {
        (void) st_insert(ins, util_strsav(cptr->label), (char *) 0);
        in = 1; 
      } break;
    }  
  }
  
  if((in == 0) && ((comp_Type(c) == COMP_LC) || (comp_Type(c) == COMP_RLC))) {
    ret = (char *) REALLOC(char, ret, strlen(ret) + 16);
    strcat(ret, "No input pins.\n");  
  }
  if((out == 0) && ((comp_Type(c) == COMP_LC) || 
     (comp_Type(c) == COMP_RLC))) {  
    ret = (char *) REALLOC(char, ret, strlen(ret) + 17);
    strcat(ret, "No output pins.\n");  
  }

  if((comp_Type(c) == COMP_LC) || (comp_Type(c) == COMP_RLC)) {
    ver = g_get_vertices(c->lg);
    lsForeachItem(ver, gen, vertex) {
      ln = (complnode_t *) vertex->user_data;
      if(lsLength(g_get_out_edges(vertex)) == 0) {
        out = 0;
        lsForeachItem(ln->pins, genpin, key) {
          pin = (complpin_t *) key;
          if((pin->type == COMP_LOUTPUT) || (pin->type == COMP_SHAREDO)) {
            out = 1;
          }
        }  
        if(out == 0) {
          ret = (char *) REALLOC(char, ret, strlen(ret) + 33);
          strcat(ret, "Output node without output pin.\n");          
        }    
      }
      lsForeachItem(ln->pins, genpin, key) {
        pin = (complpin_t *) key;
        if((pin->type == COMP_LINPUT) || (pin->type == COMP_SHAREDI)) {
          (void) st_insert(ins, util_strsav(pin->label), (char *) 1);
        }
        else
          if((pin->type == COMP_ISIGNAL)) {
            (void) st_insert(itmps, util_strsav(pin->label), (char *) 0);
          }
          else
            if((pin->type == COMP_OSIGNAL)) {
              (void) st_insert(otmps, util_strsav(pin->label), (char *) 0);
            }
      }      
    }
  }

  in = 0;
  st_foreach_item(ins, stgen, &key, &value) {
    FREE(key);
    if(value == 0) in = 1;   
  }
  st_free_table(ins);
 
  if(in == 1) {
    ret = (char *) REALLOC(char, ret, strlen(ret) + 18);
    strcat(ret, "Unused input pin.\n");
  }

  in = 0;
  st_foreach_item(itmps, stgen, &key, &value) {
    if(st_lookup(otmps, key, &value) == 0) in = 1;
    FREE(key);
  }
  st_free_table(itmps);
  
  st_foreach_item(otmps, stgen, &key, &value) {
    FREE(key);
  }
  st_free_table(otmps);
 
  if(in == 1) {
    ret = (char *) REALLOC(char, ret, strlen(ret) + 33);
    strcat(ret, "No logic node driving a signal.\n");
  }
  
  if(ret[0] == '\0') {
    FREE(ret);
    return NIL(char);
  }
  
  return ret;
}

/**Function********************************************************************

  Synopsis           [Extract a blif text from a component, based on it's 
  logic graph.]

  Description        []

  SideEffects        []
  
  SeeAlso	     []
  
******************************************************************************/
char *
comp_BlifExtr(comp_data_t *c)
{
char *ret = NIL(char), *pchar;
char buf[200];
int i;
comp_pin_t *cptr;
short lut = 0;
lsList vertices;
vertex_t *v;
lsGen gen;
complnode_t *ln;

  if(c == NIL(comp_data_t) || c->lg == NIL(graph_t)) {
    return NIL(char);
  }
  sprintf(buf,".model %s\n",c->label);
  ret = util_strsav(buf);

  vertices = g_get_vertices(c->lg);
  lsForeachItem(vertices, gen, v) {
    ln = (complnode_t *) v->user_data;
    if(ln->type == COMP_LUT) {
      lut = 1;
      (void) lsFinish(gen);
      break;
    }
  }
  
  if(lut == 1) {
    ret = (char *) REALLOC(char, ret, strlen(ret) + 13);
    strcat(ret, ".inputs lut\n");
  }

  for(i = 0;i < array_n(c->pins);i++) {
    cptr = array_fetch(comp_pin_t *, c->pins, i);
    switch(cptr->type) {
      case COMP_SHAREDI:
      case COMP_LINPUT: {
        sprintf(buf,".inputs %s\n", cptr->label);
        ret = (char *) REALLOC(char, ret, strlen(ret)+strlen(buf)+1);
        strcat(ret, buf);
      } break;
      case COMP_LOUTPUT:
      case COMP_SHAREDO: {
        sprintf(buf,".outputs %s\n", cptr->label);
        ret = (char *) REALLOC(char, ret, strlen(ret)+strlen(buf)+1);
        strcat(ret, buf);
      } break;
      case COMP_CLK: {
        sprintf(buf,".clock %s\n", cptr->label);
        ret = (char *) REALLOC(char, ret, strlen(ret)+strlen(buf)+1);
        strcat(ret, buf);
      } break;            
    }    
  } 
    
  if(c->delay != NIL(compconstr_t)) {
    if(c->delay->area != -1) {
      sprintf(buf,".area %d\n", c->delay->area);
      ret = (char *) REALLOC(char, ret, strlen(ret)+strlen(buf)+1);
      strcat(ret, buf);      
    }  
  }
  
  pchar = compLgraph2Blif(c);
  if(pchar != NIL(char)) {
    ret = (char *) REALLOC(char, ret, strlen(ret) + strlen(pchar) + 1);  
    strcat(ret, pchar);
    FREE(pchar);    
  }     
  ret = (char *) REALLOC(char, ret, strlen(ret) + 6);
  strcat(ret,".end\n");
  

  return ret;
}

/**Function********************************************************************

  Synopsis           [Find a component pin within it's pin list. Return the
  position of the pin if found, otherwise -1.]

  Description        []

  SideEffects        []
  
  SeeAlso	     []
  
******************************************************************************/
int
comp_PinFind(
  array_t *pins,
  char *label)
{
comp_pin_t *ptr;
int i;

  if(pins == NIL(array_t)) {
    return -1;
  }
  for(i = 0;i < array_n(pins);i++) {
    ptr = array_fetch(comp_pin_t *,pins,i);
    if(strcmp(ptr->label, label) == 0) return i;
  }
  return -1;
}  

/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis           [Release the memory associated with a signal list.]

  Description        []

  SideEffects        []
  
  SeeAlso	     []
  
******************************************************************************/
void
compSigLFree(array_t *list)
{
int i;
char *ptr;

  if(list == NIL(array_t)) return;
  for(i = 0;i < array_n(list);i++) {
    ptr = array_fetch(char *, list, i);
    FREE(ptr);
  }
  array_free(list);
}

/**Function********************************************************************

  Synopsis           [Add a signal to the end of a signal list.]

  Description        []

  SideEffects        []
  
  SeeAlso	     []
  
******************************************************************************/
array_t *
compSigIns(
  array_t *list,
  char *label)
{
  if(list == NIL(array_t)) {
    list = array_alloc(char *,0);
  }
  array_insert_last(char *, list, util_strsav(label));
  return list;
}

/**Function********************************************************************

  Synopsis           [Search for a specified signal within a signal list.]

  Description        [Return the position of the signal within the array,
  otherwise return -1.]

  SideEffects        []
  
  SeeAlso	     []
  
******************************************************************************/
int
compSigFind(
  array_t *list,
  char *label)
{
int i;
char *ptr;

  if(list == NIL(array_t)) {
    return -1;
  }  
  for(i = 0;i < array_n(list);i++) {
    ptr = array_fetch(char *, list, i);
    if(strcmp(ptr, label) == 0) return i;
  }
  return -1;
}  

/**Function********************************************************************

  Synopsis           [Allocate a new component.]

  Description        []

  SideEffects        []
  
  SeeAlso	     []
  
******************************************************************************/
comp_data_t *
compAlloc(char *label)
{
comp_data_t *tmp;

  tmp = (comp_data_t *) ALLOC(comp_data_t,1);
  tmp->label = util_strsav(label);
  tmp->pins = array_alloc(comp_pin_t *, 0);
  tmp->lg = g_alloc();
  tmp->rg = g_alloc();
  tmp->text = NIL(char);
  tmp->delay = NIL(compconstr_t);
  tmp->fig = (comp_fig_t *) ALLOC(comp_fig_t, 1);
  tmp->fig->pins = array_alloc(comp_figpin_t *, 0);

  return tmp;
}

/**Function********************************************************************

  Synopsis           [Add a new pin to the end of the component pin list.]

  Description        []

  SideEffects        []
  
  SeeAlso	     []
  
******************************************************************************/
array_t *
compPinIns(
  array_t *pins,
  char *label,
  int type)
{
comp_pin_t *new;

  new = (comp_pin_t *) ALLOC(comp_pin_t, 1);
  new->label = util_strsav(label);
  new->type = type;
  new->pos = -1;	      /* Must be -1. Check compPinNpos below. */
  new->prog = 0;

  array_insert_last(comp_pin_t *, pins, new);

  return pins;
}

/**Function********************************************************************

  Synopsis           [Find out if there is some pin without a position 
  specified.]

  Description        []

  SideEffects        []
  
  SeeAlso	     [compParse.c]
  
******************************************************************************/
int
compPinNpos(array_t *pins)
{
int i;
comp_pin_t *ptr;

  if(pins == NIL(array_t)) {
    return 1;
  }
  for(i = 0;i < array_n(pins);i++) {
    ptr = array_fetch(comp_pin_t *, pins, i);
    if(ptr->pos == -1) return 1;
  }
  return 0;
}

/**Function********************************************************************

  Synopsis           [Set the position for a component pin.]

  Description        [Given the label of the pin, set the given position. If
  the pin is not found, return a MISSING_PIN code. This is used by the parser.]

  SideEffects        []
  
  SeeAlso	     [compParse.c]
  
******************************************************************************/
int
compPinSetpos(
  array_t *pins,
  char *label,
  int pos)
{
comp_pin_t *ptr;
int i;

  i = comp_PinFind(pins, label);
  if(i == -1) {
    return MISSING_PIN;
  }    
  ptr = array_fetch(comp_pin_t *,pins,i);
  if(ptr->pos != -1) {
    return WITH_POS;
  }
  ptr->pos = pos;

  return NO_ERROR;  
}

/**Function********************************************************************

  Synopsis           [Set a component pin as programmable.]

  Description        [Given the label of the pin, set it as programmable. If
  the pin is not found, return a MISSING_PIN code. This is used by the parser.]

  SideEffects        []
  
  SeeAlso	     [compParse.c]
  
******************************************************************************/
int
compPinSetprog(
  array_t *pins,
  char *label)
{
comp_pin_t *ptr;
int i;

  i = comp_PinFind(pins, label);
  if(i == -1) {
    return MISSING_PIN;
  }  
  ptr = array_fetch(comp_pin_t *, pins, i);
  ptr->prog = 1;
  
  return NO_ERROR;
}

/**Function********************************************************************

  Synopsis           [Create a graphic view of the component, based on the 
  number and position of it's pins.]

  Description        []

  SideEffects        []
  
  SeeAlso	     []
  
******************************************************************************/
comp_data_t *
compFigCreate(comp_data_t *c)
{
int emax = 0, nmax = 0, smax = 0, wmax = 0,count = 1;
int dx1 = 0,dy1 = 0,dx2 = 0,dy2 = 0, mwestep,mnsstep;
int i;
comp_pin_t *ptr;

  for(i = 0;i < array_n(c->pins);i++) {
    ptr = array_fetch(comp_pin_t *, c->pins, i);
    switch(ptr->pos) {
      case COMP_EAST: emax++; break;
      case COMP_WEST: wmax++; break;
      case COMP_NORTH: nmax++; break;
      case COMP_SOUTH: smax++; break;
    }
  } 
  if(emax >= wmax) {
    c->fig->height = (unsigned) emax*STEP + STEP;
    mwestep = c->fig->height/(wmax + 1);
  } 
  else {
    c->fig->height = (unsigned) wmax*STEP + STEP;
    mwestep = c->fig->height/(emax + 1);
  }
      
  if(nmax >= smax) {
    c->fig->width = (unsigned) nmax*STEP + STEP;
    mnsstep = c->fig->width/(smax + 1);
  }
  else {
    c->fig->width = (unsigned) smax*STEP + STEP;
    mnsstep = c->fig->width/(nmax + 1);
  }
  array_sort(c->pins, comparePins);
    
  i = 0;
  ptr = array_fetch(comp_pin_t *, c->pins, 0);
  while(ptr->pos == COMP_WEST) {
    dy1 = dy1 + STEP;     
    dy2 = dy1;
    dx1 = 0;
    dx2 = -PIN_LENGHT;    
    c->fig->pins = comp_FpinIns(c->fig->pins, dx1, dy1, dx2, dy2, count);
    count++;
    i++;
    if(i == array_n(c->pins)) return c;
    ptr = array_fetch(comp_pin_t *,c->pins,i);
  }
  
  dy1 = 0;
  dy2 = 0;
  dx1 = 0;
  dx2 = 0;

  while(ptr->pos == COMP_SOUTH) {
    dx1 = dx1 + STEP;
    dx2 = dx1;
    dy1 = (int) c->fig->height;
    dy2 = dy1 + PIN_LENGHT;
    c->fig->pins = comp_FpinIns(c->fig->pins,dx1, dy1, dx2, dy2, count);
    count++;
    i++;
    if(i == array_n(c->pins)) return c;
    ptr = array_fetch(comp_pin_t *, c->pins, i);
  }
  dx1 = 0;
  dx2 = 0;
  dy1 = (int) c->fig->height;
  dy2 = 0;

  while(ptr->pos == COMP_EAST) {
    dy1 = dy1 - STEP;
    dy2 = dy1;
    dx1 = (int) c->fig->width;
    dx2 = dx1 + PIN_LENGHT;               
    c->fig->pins = comp_FpinIns(c->fig->pins,dx1, dy1, dx2, dy2, count);
    count++;
    i++;
    if(i == array_n(c->pins)) return c;
    ptr = array_fetch(comp_pin_t *, c->pins, i);
  }

  dx1 = (int) c->fig->width;
  dx2 = 0;
  dy1 = 0;
  dy2 = 0;
  
  while(ptr->pos == COMP_NORTH) {
    dx1 = dx1 - STEP;
    dx2 = dx1;
    dy1 = 0;
    dy2 = -PIN_LENGHT;
    c->fig->pins = comp_FpinIns(c->fig->pins,dx1, dy1, dx2, dy2, count);
    count++;
    i++;
    if(i == array_n(c->pins)) return c;
    ptr = array_fetch(comp_pin_t *, c->pins, i);
  }
  
  return c;
}

/**Function********************************************************************

  Synopsis           [Print a component.]

  Description        []

  SideEffects        []
  
  SeeAlso	     []
  
******************************************************************************/
void
compPrint(comp_data_t *c)
{
int i;
comp_pin_t *pptr;
comp_figpin_t *fptr;

  printf("\n ----- Component %s -----\n",c->label);
  printf("PINS:\n");
  for(i = 0;i < array_n(c->pins);i++) {
    pptr = array_fetch(comp_pin_t *, c->pins, i);
    printf("%s %d %d %d\n",pptr->label, pptr->type,
      pptr->pos, pptr->prog);
  }
  printf("\nLOGIC GRAPH\n");
  compLgraphPrint(c->lg);
  printf("\nROUTING GRAPH\n");
  compRgraphPrint(c->rg);
  printf("\nFIGURE\n");
  printf("w = %d  h = %d\n",c->fig->width,c->fig->height);
  for(i = 0;i < array_n(c->fig->pins);i++) {
    fptr = array_fetch(comp_figpin_t *, c->fig->pins, i);
    printf("n = %d %d %d %d %d\n",fptr->number,fptr->dx1,fptr->dy1,
      fptr->dx2, fptr->dy2);  
  }
}

/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis           [Compare two pins. The result is based on their position.]

  Description        []

  SideEffects        []
  
  SeeAlso	     []
  
******************************************************************************/
static int
comparePins(
  char *c1,
  char *c2)
{
comp_pin_t **p1,**p2;

  p1 = (comp_pin_t **) c1;
  p2 = (comp_pin_t **) c2;

  return ((*p1)->pos - (*p2)->pos);
}
