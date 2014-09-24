/**CFile***********************************************************************

  FileName    [archParse.c]

  PackageName [arch]

  Synopsis    [Parse an architecture specification.]

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

/**AutomaticStart*************************************************************/

/*---------------------------------------------------------------------------*/
/* Static function prototypes                                                */
/*---------------------------------------------------------------------------*/

static short Reserved(char *data);
static int UseDecl(array_t *tokenlist, int *token_index, int *linen, array_t **liblist, array_t **ablist);
static int PortDecl(st_table *signals, array_t *tokenlist, int *token_index, int *linen);
static int LvarFind(array_t *vars, char *label);
static void LvarIns(array_t **var, char *label, int from);
static void LvarSetto(array_t *var, char *label, short type, int to);
static void LvarSetstep(array_t *var, char *label, short type, int step);
static int GetVars(array_t **var, array_t *tokenlist, int *token_index, int *linen);
static int Relation(int n, short op, int m);
static int Update(int n, int assig, int m);
static void CompInst(st_table *signals, st_table *components, array_t *tokenlist, int *token_index, int *linen, lsList *libs, array_t *liblist, char *cname, int *error, array_t *lv);
static void GetCpos(array_t *tokenlist, int *token_index, int *linen, int *error, int *cx1, int *cx2, int *cy1, int *cy2, int *cz1, int *cz2);
static void GetCposlv(char *tok, array_t *tokenlist, int *token_index, int *linen, int *error, int *coord, int *type, archloopvar_t **var, array_t *lv);
static avl_tree * SigIns(avl_tree *signals, char *bfrom, int sfrom, char *bto, int sto, int *coord, char **war);
static avl_tree * CoordsIns(avl_tree *signals, int *coord, int *type, archloopvar_t **var, int *from, int fsize, int *to, char *tok, char *tok2, array_t *lv, char **war);
static avl_tree * ConnectDecl(avl_tree *signals, avl_tree *bus, array_t *tokenlist, int *token_index, int *linen, int *error, char *tok, char **war, array_t *lv);
static arch_graph_t * MakeArch(arch_graph_t *g, st_table *signals, st_table *components, lsList *libs, int size, char **war);
static void Warnings(arch_graph_t *g, avl_tree *signals, avl_tree *bus, char **war);
static arch_graph_t * Body(arch_graph_t *g, st_table *signals, array_t *tokenlist, int *token_index, int *linen, array_t *liblist, array_t *ablist, int *error, char **war);

/**AutomaticEnd***************************************************************/


/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis           [Return the error string corresponding to the error code.]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
char *
arch_Ecode(int code)
{
char *message;

  message = (char *) ALLOC(char, 80);
  strcpy(message,"Parse error: ");
  switch(code) {
    case ARCHNO_TEXT: {
      strcat(message,"no text to be parsed");
    } break;
    case ARCHNO_USE: {
      strcat(message,"missing USE declaration");
    } break;
    case ARCHNO_PORT: {
      strcat(message,"missing PORT declaration");
    } break;
    case ARCHINVALID_USE: {
      strcat(message,"invalid USE declaration");
    } break;
    case ARCHINVALID_COMPONENT: {
      strcat(message,"invalid component instatiation");
    } break;
    case ARCHWRONG_PARAM_NUMBER: {
      strcat(message,"wrong number of parameters");
    } break;
    case ARCHCOORD_NOT_NUMBER: {
      strcat(message,"non numeric component coordinate");
    } break;
    case ARCHCOMP_SAME_POS: {
      strcat(message,"two components on the same position");
    } break;
    case ARCHWRONG_IO: {
      strcat(message,"non numeric io specification");
    } break;
    case ARCHMISPLACED_BUS: {
      strcat(message,"misplaced bus declaration");
    } break;
    case ARCHBAD_BUS: {
      strcat(message,"wrong bus declaration");
    } break;
    case ARCHWRONG_BUS_SIZE: {
      strcat(message,"non numeric bus size");
    } break;
    case ARCHNO_ARCHLABEL: {
      strcat(message,"missing architecture label");
    } break;
    case ARCHBUS_REDECLARED: {
      strcat(message,"bus redeclared");
    } break;
    case ARCHPARSE_ERROR: break;
    case ARCHWRONG_NUMERIC_PARAM: {
      strcat(message,"wrong numeric parameter");
    } break;
    case ARCHWRONG_BUS_PARAM: {
      strcat(message,"wrong bus parameter");
    } break;
    case ARCHNOT_BUS: {
      strcat(message, "parameter is not a bus");
    } break;
    case ARCHMISSING_EQU: {
      strcat(message, "missing '=' token");
    } break;
    case ARCHMISMATCH_BUS_WIDTH: {
      strcat(message,"mismatch between bus widths");
    } break;
    case ARCHMISSING_PV: {
      strcat(message, "missing ';' token");
    } break;
    case ARCHWRONG_EAST_SIZE: {
      strcat(message,"incompatible east bus size in component instatiation");
    } break;
    case ARCHWRONG_WEST_SIZE: {
      strcat(message,"incompatible west bus size in component instatiation");
    } break;
    case ARCHWRONG_NORTH_SIZE: {
      strcat(message,"incompatible north bus size in component instatiation");
    } break;
    case ARCHWRONG_SOUTH_SIZE: {
      strcat(message,"incompatible south bus size in component instatiation");
    } break;
    case ARCHWRONG_UP_SIZE: {
      strcat(message,"incompatible up bus size in component instatiation");
    } break;
    case ARCHWRONG_DOWN_SIZE: {
      strcat(message,"incompatible down bus size in component instatiation");
    } break;
    case ARCHWRONG_SIGNAL_SIZE: {
      strcat(message,"signal instance out of bus bounds");
    } break;
    case ARCHSAME_BUS_NAME: {
      strcat(message,"same bus in the connection declaration");
    } break;
    case ARCHMISSING_OPAR: {
      strcat(message,"missing '(' token");
    } break;
    case ARCHINVALID_VAR_ID: {
      strcat(message,"invalid variable identifier at the loop declaration");
    } break;
    case ARCHMISSING_VIR: {
      strcat(message,"missing ',' token");
    } break;
    case ARCHNOT_INTEGER: {
      strcat(message,"value is not an integer");
    } break;
    case ARCHINVALID_RELAT_OPER: {
      strcat(message, "invalid relational operator");
    } break;
    case ARCHINVALID_ASSIG_OPER: {
      strcat(message,"invalid assigment operator");
    } break;
    case ARCHMISSING_OCB: {
      strcat(message,"missing '{' token");
    } break;
    case ARCHMISSING_CCB: {
      strcat(message,"missing '}' token");
    } break;
    case ARCHINCOMPLETE_VAR: {
      strcat(message,"incomplete loop variable instatiation");
    } break;
    case ARCHMISSING_CPAR: {
      strcat(message,"missing ')' token");
    } break;
    case ARCHMISSING_BEGIN: {
      strcat(message,"missing 'begin' token");
    } break;
    case ARCHMISSING_COORD: {
      strcat(message,"missing component coordinates in connection declaration");
    } break;
    case ARCHRESERVED_PORT: {
      strcat(message, "Reserved name used in port declaration");
    } break;
    case ARCHINVALID_PORT: {
      strcat(message, "Invalid port declaration");
    } break;
    case ARCHEMPTY_PORT: {
      strcat(message, "Empty list in port declaration");
    } break;
    case ARCHINVALIDTYPE_PORT: {
      strcat(message, "Invalid type in port declaration");
    } break;
    case ARCHRESERVED_SIGNAL: {
      strcat(message, "Reserved keyword used in signal instantiation");
    } break;
  }
  return message;
}

/**Function********************************************************************

  Synopsis           [Parse an architecture specification.]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
arch_graph_t *
arch_Parse(
  char *data,
  int *linen,
  int *error,
  char **war)
{
array_t *tokenlist;
int token_index = 0;
char *tok;
arch_graph_t *ret;
array_t *liblist, *ablist;
st_table *signals;

  *war = NIL(char);  
  if(data == NIL(char)) {
    *error = ARCHNO_TEXT;
    return NIL(arch_graph_t);
  }
  tokenlist = comp_TokenExtr(data);
  tok = comp_NextToken(tokenlist, &token_index, linen);

  if(strcmp(tok,"architecture") != 0) {
    comp_TokenFree(tokenlist);
    *error = ARCHNO_USE;
    return NIL(arch_graph_t);
  }
  else {
    tok = comp_NextToken(tokenlist, &token_index, linen);
    if(Reserved(tok) == 1) {
      comp_TokenFree(tokenlist);
      *error = ARCHNO_ARCHLABEL;
      return NIL(arch_graph_t);
    }
    ret = archAlloc(tok);
    tok = comp_NextToken(tokenlist, &token_index, linen);
    if(strcmp(tok,"use") != 0) {
      arch_Free(ret);
      comp_TokenFree(tokenlist);
      *error = ARCHNO_USE;
      return NIL(arch_graph_t);
    }
    *error = UseDecl(tokenlist, &token_index, linen, &liblist, &ablist);
    if(*error != ARCHNO_ERROR) {
      arch_Free(ret);
      comp_TokenFree(tokenlist);      
      return NIL(arch_graph_t);
    }
    tok = comp_NextToken(tokenlist, &token_index, linen);
    if(strcmp(tok,"port") != 0) {
      arch_Free(ret);
      comp_TokenFree(tokenlist);
      *error = ARCHNO_PORT;
      return NIL(arch_graph_t);
    }
    signals = st_init_table(strcmp, st_strhash);
    *error = PortDecl(signals, tokenlist, &token_index, linen);
    if(*error != ARCHNO_ERROR) {
      arch_Free(ret);
      comp_TokenFree(tokenlist);
      return NIL(arch_graph_t);
    }
    tok = comp_NextToken(tokenlist, &token_index, linen);
    if(strcmp(tok,"begin") != 0) {
      arch_Free(ret);
      comp_TokenFree(tokenlist);
      *error = ARCHMISSING_BEGIN;
      return NIL(arch_graph_t);    
    }
    ret = Body(ret, signals, tokenlist, &token_index, linen, 
      liblist, ablist, error, war);
    if(*error != ARCHNO_ERROR) {
      arch_Free(ret);
      comp_TokenFree(tokenlist);
      return NIL(arch_graph_t);
    }
  }
  for(token_index = 0;token_index < array_n(liblist);token_index++) {
    tok = array_fetch(char *, liblist, token_index);
    FREE(tok);
  }
  array_free(liblist);
  for(token_index = 0;token_index < array_n(ablist);token_index++) {
    tok = array_fetch(char *, ablist, token_index);
    FREE(tok);
  }
  array_free(ablist);
          
  comp_TokenFree(tokenlist);
  *error = ARCHNO_ERROR;

  return ret;
}

/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis           [See if a given string is a keyword.]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static short
Reserved(char *data)
{
  if(!strcmp(data,"use") ||
     !strcmp(data,"begin") ||
     !strcmp(data,"end") ||
     !strcmp(data,"architecture") ||
     !strcmp(data,"for") ||
     !strcmp(data,"bus") )
    return 1;

  return 0; 
}

/**Function********************************************************************

  Synopsis           [Parse a use declaration.]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static int
UseDecl(
  array_t *tokenlist,
  int *token_index,
  int *linen,
  array_t **liblist,
  array_t **ablist)
{
char *tok,tmp[80],*pchar;

  *ablist = array_alloc(char *, 0);
  *liblist = array_alloc(char *,0);
  tok = comp_NextToken(tokenlist, token_index, linen);
  while(strcmp(tok,"end") != 0) {  
    strcpy(tmp, tok);
    pchar = strtok(tmp,".");
    if(pchar == NIL(char)) {
      return ARCHINVALID_USE;
    }
    pchar = strtok(NULL," ");
    if(pchar == NIL(char)) {
      return ARCHINVALID_USE;
    }
    if(strcmp(pchar,"clib") == 0)  {
      array_insert_last(char *, *liblist, util_strsav(tok));
    }
    else
      if(strcmp(pchar,"ab") == 0) {
        array_insert_last(char *, *ablist, util_strsav(tok));
      }
      else {
        return ARCHINVALID_USE;
      }
    tok = comp_NextToken(tokenlist, token_index, linen);
    if(strcmp(tok,"end") == 0) {
      break;
    }
    else 
      if((strcmp(tok,",") != 0) && (strcmp(tok,";") != 0)) {
        return ARCHINVALID_USE;
      }      
    tok = comp_NextToken(tokenlist, token_index, linen);    
  }
  return ARCHNO_ERROR;
}

/**Function********************************************************************

  Synopsis           [Parse the port declaration.]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static int
PortDecl(
  st_table *signals,
  array_t *tokenlist,
  int *token_index,
  int *linen)
{
archsignal_t *as;
array_t *names;
char *value, *tok;
int i;

  tok = comp_NextToken(tokenlist, token_index, linen);
  
  while(strcmp(tok,"end") != 0) {
    names = array_alloc(char *, 0);
    while(1) {
      if(!Reserved(tok)) {
        array_insert_last(char *, names, util_strsav(tok));
      }
      else {
        return ARCHRESERVED_PORT;
      }
      tok = comp_NextToken(tokenlist, token_index, linen);      
      if(strcmp(tok,":") == 0) {
        break;
      }
      else
        if(strcmp(tok,",") != 0) {
          return ARCHINVALID_PORT;
        }
      tok = comp_NextToken(tokenlist, token_index, linen);
    }    
    
    if(array_n(names) == 0) {
      return ARCHEMPTY_PORT;
    }
    else {
      tok = comp_NextToken(tokenlist, token_index, linen);
      if(strcmp(tok,"in") == 0) {
        for(i = 0;i < array_n(names);i++) {
          tok = array_fetch(char *, names, i);
          if(st_lookup(signals, tok, &value) == 0) {
            as = (archsignal_t *) ALLOC(archsignal_t, 1);
            as->comps = array_alloc(char *, 0);
            as->port = 1;
            st_insert(signals, util_strsav(tok), (char *) as);
          }
        }
      }
      else
        if(strcmp(tok,"out") == 0) {
          for(i = 0;i < array_n(names);i++) {
            tok = array_fetch(char *, names, i);
            if(st_lookup(signals, tok, &value) == 0) {
              as = (archsignal_t *) ALLOC(archsignal_t, 1);
              as->comps = array_alloc(char *, 0);
              as->port = 2;
              st_insert(signals, util_strsav(tok), (char *) as);
            }
          }        
        }
        else
          if(strcmp(tok,"inout") == 0) {
            for(i = 0;i < array_n(names);i++) {
              tok = array_fetch(char *, names, i);
              if(st_lookup(signals, tok, &value) == 0) {
                as = (archsignal_t *) ALLOC(archsignal_t, 1);
                as->port = 3;
                as->comps = array_alloc(char *, 0);
                st_insert(signals, util_strsav(tok), (char *) as);
              }
            }          
          }
          else {
            return ARCHINVALIDTYPE_PORT;
          }
          
      for(i = 0;i < array_n(names);i++) {
        tok = array_fetch(char *, names, i);
        FREE(tok);
      }
      array_free(names);
      tok = comp_NextToken(tokenlist, token_index, linen);
      if(strcmp(tok,";") != 0) {
        return ARCHINVALID_PORT;
      }
    }    
    tok = comp_NextToken(tokenlist, token_index, linen);    
  }

  return ARCHNO_ERROR;
}

/**Function********************************************************************

  Synopsis           [Find a loop variable.]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static int
LvarFind(
  array_t *vars,
  char *label)
{
unsigned int i;
archloopvar_t *lv;

  if(vars == NIL(array_t)) {
    return -1;
  }
  for(i = 0;i < array_n(vars);i++) {
    lv = array_fetch(archloopvar_t *,vars,i);
    if(strcmp(lv->label, label) == 0) 
      return i;  
  }
  return -1;
}

/**Function********************************************************************

  Synopsis           [Insert a loop variable.]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static void
LvarIns(
  array_t **var,
  char *label,
  int from)
{
archloopvar_t *lv;

  lv = (archloopvar_t *) ALLOC(archloopvar_t, 1);
  lv->label = util_strsav(label);
  lv->from = from;
  lv->relat = -1;
  lv->assig = -1;
  array_insert_last(archloopvar_t *, *var, lv);
}

/**Function********************************************************************

  Synopsis           [Set the to field of a loop variable]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static void
LvarSetto(
  array_t *var,
  char *label,
  short type,
  int to)
{
archloopvar_t *ptr;
unsigned int i;

  if(var == NIL(array_t)) {
    return;
  }
  
  for(i = 0;i < array_n(var);i++) {
    ptr = array_fetch(archloopvar_t *,var,i);
    if(strcmp(ptr->label,label) == 0) {
      ptr->relat = type;
      ptr->to = to;
      return;
    }
  }
}

/**Function********************************************************************

  Synopsis           [Set the step field of a loop variable]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static void
LvarSetstep(
  array_t *var,
  char *label,
  short type,
  int step)
{
archloopvar_t *ptr;
unsigned int i;

  if(var == NIL(array_t)) {
    return;
  }  
  for(i = 0;i < array_n(var);i++) {
    ptr = array_fetch(archloopvar_t *,var,i);
    if(strcmp(ptr->label,label) == 0) {
      ptr->assig = type;
      ptr->step = step;
      return;
    }
  }
}

/**Function********************************************************************

  Synopsis           [Parse a variable utilization. This occurs when using
  a for loop.]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static int
GetVars(
  array_t **var,
  array_t *tokenlist,
  int *token_index,
  int *linen)
{
char *tok, *tok2;
int number;
short type;
archloopvar_t *test;

  tok = comp_NextToken(tokenlist, token_index, linen);
  if((util2_ValidId(tok) == 0) || (Reserved(tok) == 1)) {
    return ARCHINVALID_VAR_ID;  
  }
  tok2 = comp_NextToken(tokenlist, token_index, linen);
  if(strcmp(tok2,"=") != 0) {
    return ARCHMISSING_EQU;  
  }
  tok2 = comp_NextToken(tokenlist, token_index, linen);
  for(number = 0;number < (int) strlen(tok2);number++) {
    if(isdigit(tok2[number]) == 0) {
      return ARCHNOT_INTEGER;
    }
  }  
  LvarIns(var, tok, atoi(tok2));
  tok = comp_NextToken(tokenlist, token_index,linen);
  while(strcmp(tok,";") != 0) {
    if(strcmp(tok,",") != 0) {
      return ARCHMISSING_VIR;
    }
    tok = comp_NextToken(tokenlist, token_index, linen);
    if((util2_ValidId(tok) == 0) || (Reserved(tok) == 1)) {
      return ARCHINVALID_VAR_ID;  
    }
    tok2 = comp_NextToken(tokenlist, token_index, linen);
    if(strcmp(tok2,"=") != 0) {
      return ARCHMISSING_EQU;  
    }
    tok2 = comp_NextToken(tokenlist, token_index, linen);
    for(number = 0;number < (int) strlen(tok2);number++) {
      if(isdigit(tok2[number]) == 0) {
        return ARCHNOT_INTEGER;
      }
    }  
    LvarIns(var, tok, atoi(tok2));
    tok = comp_NextToken(tokenlist, token_index, linen);
  }    
  
  tok = comp_NextToken(tokenlist, token_index, linen);
  if((util2_ValidId(tok) == 0) || (Reserved(tok) == 1)) {
    return ARCHINVALID_VAR_ID;
  }
  tok2 = comp_NextToken(tokenlist, token_index, linen);
  if(strcmp(tok2,">") == 0) { 
    tok2 = comp_NextToken(tokenlist, token_index, linen);
    if(strcmp(tok2,"=") == 0) {
      type = ARCHGE;
    }
    else {
      type = ARCHGT;   
      (*token_index)--;   
    }
  }
  else
    if(strcmp(tok2,"<") == 0) {
      tok2 = comp_NextToken(tokenlist, token_index, linen);
      if(strcmp(tok2,"=") == 0) {
        type = ARCHLE;
      }
      else {
        type = ARCHLT;
        (*token_index)--;
      }
    }
    else
      if(strcmp(tok2,"!") == 0) {
        tok2 = comp_NextToken(tokenlist, token_index, linen);            
        if(strcmp(tok2,"=") == 0) {
          type = ARCHDIF;
        }
        else {
          return ARCHINVALID_RELAT_OPER;
        }
      }
      else {
        return ARCHINVALID_RELAT_OPER;
      }
      
  tok2 = comp_NextToken(tokenlist, token_index, linen);
  for(number = 0;number < (int) strlen(tok2);number++) {
    if(isdigit(tok2[number]) == 0) {
      return ARCHNOT_INTEGER;
    }
  }
  LvarSetto(*var, tok, type, atoi(tok2));

  tok = comp_NextToken(tokenlist, token_index, linen);
  while(strcmp(tok,";") != 0) {
    if(strcmp(tok,",") != 0) {
      return ARCHMISSING_VIR;
    }
    tok = comp_NextToken(tokenlist, token_index, linen);
    if((util2_ValidId(tok) == 0) || (Reserved(tok) == 1)) {
      return ARCHINVALID_VAR_ID;
    }
    tok2 = comp_NextToken(tokenlist, token_index, linen);
    if(strcmp(tok2,">") == 0) { 
      tok2 = comp_NextToken(tokenlist, token_index, linen);
      if(strcmp(tok2,"=") == 0) {
        type = ARCHGE;
      }
      else {
        type = ARCHGT;
        (*token_index)--;
      }
    }
    else
      if(strcmp(tok2,"<") == 0) {
        tok2 = comp_NextToken(tokenlist, token_index, linen);
        if(strcmp(tok2,"=") == 0) {
          type = ARCHLE;
        }
        else {
          type = ARCHLT;
          (*token_index)--;
        }
      }
      else
        if(strcmp(tok2,"!") == 0) {
          tok2 = comp_NextToken(tokenlist, token_index, linen);            
          if(strcmp(tok2,"=") == 0) {
            type = ARCHDIF;
          }
          else {
            return ARCHINVALID_RELAT_OPER;
          }
        }
        else {
          return ARCHINVALID_RELAT_OPER;
        }                        
    tok2 = comp_NextToken(tokenlist, token_index, linen);
    for(number = 0;number < (int) strlen(tok2);number++) {
      if(isdigit(tok2[number]) == 0) {
        return ARCHNOT_INTEGER;
      }
    }
    LvarSetto(*var, tok, type, atoi(tok2));
    tok = comp_NextToken(tokenlist, token_index, linen);
  }
  
  tok = comp_NextToken(tokenlist, token_index, linen);
  if((util2_ValidId(tok) == 0) || (Reserved(tok) == 1)) {
    return ARCHINVALID_VAR_ID;
  }
  tok2 = comp_NextToken(tokenlist, token_index, linen);
  if(strcmp(tok2,"+") == 0) {
    type = ARCHADD;
  }
  else
    if(strcmp(tok2,"-") == 0) {
      type = ARCHSUB;
    }
    else
      if(strcmp(tok2,"*") == 0) {
        type = ARCHMULT;
      }
      else
        if(strcmp(tok2,"/") == 0) {
          type = ARCHDIV;
        }
        else {
          return ARCHINVALID_ASSIG_OPER;
        }
        
  tok2 = comp_NextToken(tokenlist, token_index, linen);
  for(number = 0;number < (int) strlen(tok2);number++) {
    if(isdigit(tok2[number]) == 0) {
      return ARCHNOT_INTEGER;
    }
  }
  
  LvarSetstep(*var, tok, type, atoi(tok2));
  tok = comp_NextToken(tokenlist, token_index, linen);  
  while(strcmp(tok,")") != 0) {
    if(strcmp(tok,",") != 0) {
      return ARCHMISSING_VIR;
    }
    tok = comp_NextToken(tokenlist, token_index, linen);
    if((util2_ValidId(tok) == 0) || (Reserved(tok) == 1)) {
      return ARCHINVALID_VAR_ID;
    }
    tok2 = comp_NextToken(tokenlist, token_index, linen);
    if(strcmp(tok2,"+") == 0) {
      type = ARCHADD;
    }
    else
      if(strcmp(tok2,"-") == 0) {
        type = ARCHSUB;
      }
      else
        if(strcmp(tok2,"*") == 0) {
          type = ARCHMULT;
        }
        else
          if(strcmp(tok2,"/") == 0) {
            type = ARCHDIV;
          }
          else {
            return ARCHINVALID_ASSIG_OPER;
          }
    tok2 = comp_NextToken(tokenlist, token_index, linen);
    for(number = 0;number < (int) strlen(tok2);number++) {
      if(isdigit(tok2[number]) == 0) {
        return ARCHNOT_INTEGER;
      }
    }
    LvarSetstep(*var, tok, type, atoi(tok2));
    tok = comp_NextToken(tokenlist, token_index, linen);
  }

  for(number = 0;number < array_n(*var);number++) {
    test = array_fetch(archloopvar_t *, *var, number);
    if((test->relat == -1) || (test->assig == -1)) {
      return ARCHINCOMPLETE_VAR;  
    }
  }
  
  return ARCHNO_ERROR;
}

/**Function********************************************************************

  Synopsis           [Perform a comparation. Used within the for declaration.]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static int
Relation(
  int n,
  short op,
  int m)
{
  switch(op) {
    case ARCHGT: return n > m; break;
    case ARCHLT: return n < m; break;
    case ARCHGE: return n >= m; break;
    case ARCHLE: return n <= m; break;
    case ARCHDIF: return n != m; break;
  }
  return 0;
}

/**Function********************************************************************

  Synopsis           [Update the loop variable, with respect to some basic
  operation.]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static int
Update(
  int n,
  int assig,
  int m)
{
  switch(assig) {
    case ARCHADD: return n + m; break;
    case ARCHSUB: return n - m; break;
    case ARCHMULT: return n * m; break;
    case ARCHDIV: return n / m; break;
  }
  return 0;
}

/**Function********************************************************************

  Synopsis           [This function handle a component instantiation, within
  or not a loop. The final result, if the declaration is consistent, is the
  insertion of the correspondent(s) node(s) in the architecture graph, plus
  the connected signals in the signal table.]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static void
CompInst(
  st_table *signals,
  st_table *components,
  array_t *tokenlist,
  int *token_index,
  int *linen,
  lsList *libs,
  array_t *liblist,
  char *cname,
  int *error,
  array_t *lv)
{
int i;
comp_data_t *c;
char *pchar, *value, key[100];
archsignal_t *as;
archcomp_t *ac;

int io,j, xv = -1, yv = -1, zv = -1;
int east, west, north, south, up, down;
int nparam, param;
char *tok;
archloopvar_t *var1, *var2, *var3;
comp_pin_t *cp;

  for(i = 0;i < array_n(liblist);i++) {
    c = comp_Find(libs[i], cname);
    if(c != NIL(comp_data_t)) break;
  }
  if(c == NIL(comp_data_t)) {
    *error = ARCHINVALID_COMPONENT;
  }
  pchar = array_fetch(char *, liblist, i);

  ac = (archcomp_t *) ALLOC(archcomp_t, 1);
  ac->libname = util_strsav(pchar);
  ac->logic = comp_Type(c);
  ac->io = 0;
  
  tok = comp_NextToken(tokenlist, token_index, linen);
  if(!util2_IsInt(tok)) {
    *error = ARCHCOORD_NOT_NUMBER;
    FREE(ac);
  }
  ac->x = atoi(tok);
  tok = comp_NextToken(tokenlist, token_index, linen);
  if(strcmp(tok,",") != 0) {
    *error = ARCHMISSING_VIR;
    FREE(ac);
  }
  tok = comp_NextToken(tokenlist, token_index, linen);
  if(!util2_IsInt(tok)) {
    *error = ARCHCOORD_NOT_NUMBER;
    FREE(ac);
  }
  ac->y = atoi(tok);
  tok = comp_NextToken(tokenlist, token_index, linen);
  if(strcmp(tok,",") != 0) {
    *error = ARCHMISSING_VIR;
    FREE(ac);
  }
  tok = comp_NextToken(tokenlist, token_index, linen);
  if(!util2_IsInt(tok)) {
    *error = ARCHCOORD_NOT_NUMBER;
    FREE(ac);
  }
  ac->z = atoi(tok);
  tok = comp_NextToken(tokenlist, token_index, linen);
  if(strcmp(tok,",") != 0) {
    *error = ARCHMISSING_VIR;
    FREE(ac);
  }
  ac->sigs = array_alloc(char *, 0);
  sprintf(key,"%s_%d_%d_%d", cname, ac->x, ac->y, ac->z);
  
  for(i = 0;i < array_n(c->pins);i++) {
    cp = array_fetch(comp_pin_t *, c->pins, i);
    tok = comp_NextToken(tokenlist, token_index, linen);
    if(Reserved(tok)) {
      *error = ARCHRESERVED_SIGNAL;
      for(i = 0;i < array_n(ac->sigs);i++) {
        tok = array_fetch(char *, ac->sigs, i);
        FREE(tok);
      }
      array_free(ac->sigs);
      FREE(ac);
    }
    else {
      array_insert_last(char *, ac->sigs, util_strsav(tok));
      if(st_lookup(signals, tok, &value) == 0) {
        as = (archsignal_t *) ALLOC(archsignal_t, 1);
        as->port = 0;
        as->comps = array_alloc(char *, 0);
        st_insert(signals, util_strsav(tok), (char *) as);
      }
      else {
        as = (archsignal_t *) value;
        if(as->port > 0) {
          ac->io = 1;
        }
      }
      array_insert_last(char *, as->comps, util_strsav(key));
    }

    tok = comp_NextToken(tokenlist, token_index, linen);
    if(i == array_n(c->pins) - 1) {
      if(strcmp(tok,")") != 0) {
        *error = ARCHMISSING_CPAR;
        for(i = 0;i < array_n(ac->sigs);i++) {
          tok = array_fetch(char *, ac->sigs, i);
          FREE(tok);
        }
        array_free(ac->sigs);
        FREE(ac);
      }
    }
    else {
      if(strcmp(tok,",") != 0) {
        *error = ARCHMISSING_VIR;
        for(i = 0;i < array_n(ac->sigs);i++) {
          tok = array_fetch(char *, ac->sigs, i);
          FREE(tok);
        }
        array_free(ac->sigs);
        FREE(ac);
      }
    }    
  }  
  if(st_insert(components, util_strsav(key), (char *) ac) == 1) {
    *error = ARCHCOMP_SAME_POS;
  }

/*
  nparam = 8;
  east = 0;
  north = 0;
  west = 0;
  up = 0;
  down = 0;
  south = 0;
  
  for(i = 0;i < array_n(c->pins);i++) {
    cp = array_fetch(comp_pin_t *, c->pins, i);
    switch(cp->pos) {
      case COMP_NORTH: north++; break;
      case COMP_SOUTH: south++; break;
      case COMP_EAST: east++; break;
      case COMP_WEST: west++; break;
      case COMP_UP: up++; break;
      case COMP_DOWN: down++; break;
    }
    if((cp->pos == COMP_UP) || (cp->pos == COMP_DOWN)) {
      nparam = 10;
    }
  }
  
  for(i = 0;i < nparam;i++) {
    tok = comp_NextToken(tokenlist, token_index, linen);
    if(i < 4) {
      if(lv == NIL(array_t)) {
        for(param = 0;param < (int) strlen(tok);param++)
          if(isdigit(tok[param]) == 0) {
            *error = ARCHWRONG_NUMERIC_PARAM;
            return g;
          }          
        switch(i) {
          case 0: x = atoi(tok); break;
          case 1: y = atoi(tok); break;
          case 2: z = atoi(tok); break;
          case 3: io = atoi(tok); break;
        }        
      }
      else {
        j = LvarFind(lv, tok);                   
        if(j == -1) {
          for(param = 0;param < (int) strlen(tok);param++)
            if(isdigit(tok[param]) == 0) {
              *error = ARCHWRONG_NUMERIC_PARAM;
              return g;
            }          
          switch(i) {
            case 0: x = atoi(tok); break;
            case 1: y = atoi(tok); break;
            case 2: z = atoi(tok); break;
            case 3: io = atoi(tok); break;
          }                
        }
        else {
          switch(i) {
            case 0: xv = j; break;
            case 1: yv = j; break;
            case 2: zv = j; break;
          }                                  
        }            
      }      
    }
    else {
      if(strcmp(tok, "0") != 0) {
        if(avl_lookup(bus, tok, (char **) &b) == 0) {
          *error = ARCHNOT_BUS;        
          return g;
        }
        switch(i - 4) {
          case 0: if(b->size != west) {
            *error = ARCHWRONG_WEST_SIZE;          
            return g;
          } break;
          case 1: if(b->size != south) {
            *error = ARCHWRONG_SOUTH_SIZE;
            return g;
          } break;
          case 2: if(b->size != east) {
            *error = ARCHWRONG_EAST_SIZE;
            return g;
          } break;
          case 3: if(b->size != north) {
            *error = ARCHWRONG_NORTH_SIZE;
            return g;
          } break;
          case 4: if(b->size != up) {
            *error = ARCHWRONG_UP_SIZE;
            return g;
          } break;
          case 5: if(b->size != down) {
            *error = ARCHWRONG_DOWN_SIZE;
            return g;
          } break;
        }
        
        if(lv == NIL(array_t)) {
          sprintf(key,"%d_%d_%d",x,y,z);
          bi = (archbinst_t *) ALLOC(archbinst_t,1);
          bi->comp_key = util_strsav(key);
          switch(i - 4) {
            case 0: bi->pos = COMP_WEST; break;
            case 1: bi->pos = COMP_SOUTH; break;
            case 2: bi->pos = COMP_EAST; break;
            case 3: bi->pos = COMP_NORTH; break;
            case 4: bi->pos = COMP_UP; break;
            case 5: bi->pos = COMP_DOWN; break;
          }
          bi->type = 0;
          array_insert_last(archbinst_t *, b->comps, bi);
        }
        else {
          if(xv != -1) {
            var1 = array_fetch(archloopvar_t *, lv, xv);
            x = var1->from;
          }
          if(yv != -1) {
            var1 = array_fetch(archloopvar_t *, lv, yv);
            y = var1->from;
          }
          if(zv != -1) {
            var1 = array_fetch(archloopvar_t *, lv, zv);
            z = var1->from;
          }
          sprintf(key,"%d_%d_%d",x,y,z);
          bi = (archbinst_t *) ALLOC(archbinst_t, 1);
          bi->comp_key = util_strsav(key);
          switch(i - 4) {
            case 0: bi->pos = COMP_WEST; break;
            case 1: bi->pos = COMP_SOUTH; break;
            case 2: bi->pos = COMP_EAST; break;
            case 3: bi->pos = COMP_NORTH; break;
            case 4: bi->pos = COMP_UP; break;
            case 5: bi->pos = COMP_DOWN; break;
          }
          bi->type = 1;
          array_insert_last(archbinst_t *, b->comps, bi);              
        }
      }
    }
    tok = comp_NextToken(tokenlist, token_index, linen);
    if(((i == nparam - 1) && (strcmp(tok,")") != 0)) ||
      ((i < nparam - 1) && (strcmp(tok,",") != 0)) ) {
      *error = ARCHPARSE_ERROR;
      return g;                
    }
  }
  param = comp_Type(c);  
  if(lv == NIL(array_t)) {
    g = archNodeIns(g, c->label, libname, x, y, z, (short) io, param);
  }
  else {
    if((xv != -1) && (yv != -1) && (zv != -1)) {
      var1 = array_fetch(archloopvar_t *, lv, xv);
      var2 = array_fetch(archloopvar_t *, lv, yv);
      var3 = array_fetch(archloopvar_t *, lv, zv);
      for(xv = var1->from; Relation(xv, var1->relat, var1->to);
        xv = Update(xv, var1->assig, var1->step)) {
        for(yv = var2->from; Relation(yv, var2->relat, var2->to);
          yv = Update(yv, var2->assig, var2->step)) {
          for(zv = var3->from; Relation(zv, var3->relat, var3->to);
            zv = Update(zv, var3->assig, var3->step)) {
            g = archNodeIns(g, c->label, libname, xv, yv, zv, (short) io,
              param);
          }
        }
      }    
    }
    else
      if((xv != -1) && (yv != -1) && (zv == -1)) {
        var1 = array_fetch(archloopvar_t *, lv, xv);
        var2 = array_fetch(archloopvar_t *, lv, yv);
        for(xv = var1->from; Relation(xv, var1->relat, var1->to);
          xv = Update(xv, var1->assig, var1->step)) {
          for(yv = var2->from; Relation(yv, var2->relat, var2->to);
            yv = Update(yv, var2->assig, var2->step)) {
            g = archNodeIns(g, c->label, libname, xv, yv, z, (short) io,
              param);
          }
        }         
      }
      else
        if((xv != -1) && (yv == -1) && (zv != -1)) {
          var1 = array_fetch(archloopvar_t *, lv, xv);
          var3 = array_fetch(archloopvar_t *, lv, zv);
          for(xv = var1->from; Relation(xv, var1->relat, var1->to);
            xv = Update(xv, var1->assig, var1->step)) {
            for(zv = var3->from; Relation(zv, var3->relat, var3->to);
              zv = Update(zv, var3->assig, var3->step)) {
              g = archNodeIns(g, c->label, libname, xv, y, zv, (short) io,
                param);
            }                       
          }          
        }
        else
          if((xv != -1) && (yv == -1) && (zv == -1)) {
            var1 = array_fetch(archloopvar_t *, lv, xv);          
            for(xv = var1->from; Relation(xv, var1->relat, var1->to);
              xv = Update(xv, var1->assig, var1->step)) {
              g = archNodeIns(g, c->label, libname, xv, y, z,(short) io,
                param);
            }
          }
          else
            if((xv == -1) && (yv != -1) && (zv != -1)) {
              var2 = array_fetch(archloopvar_t *, lv, yv);
              var3 = array_fetch(archloopvar_t *, lv, zv);
              for(yv = var2->from; Relation(yv, var2->relat, var2->to);
                yv = Update(yv, var2->assig, var2->step)) {
                for(zv = var3->from; Relation(zv, var3->relat, var3->to);
                  zv = Update(zv, var3->assig, var3->step)) {
                  g = archNodeIns(g, c->label, libname, x, yv, zv, (short) io,
                    param);
                }
              }            
            }
            else
              if((xv == -1) && (yv != -1) && (zv == -1)) {
                var2 = array_fetch(archloopvar_t *, lv, yv);
                for(yv = var2->from; Relation(yv, var2->relat, var2->to);
                  yv = Update(yv, var2->assig, var2->step)) {
                  g = archNodeIns(g, c->label, libname, x, yv, z,(short) io,
                    param);
                }              
              }
              else
                if((xv == -1) && (yv == -1) && (zv != -1)) {
                  var3 = array_fetch(archloopvar_t *, lv, zv);
                  for(zv = var3->from; Relation(zv, var3->relat, var3->to);
                    zv = Update(zv, var3->assig, var3->step)) {
                    g = archNodeIns(g, c->label, libname, x, y, zv,(short) io,
                       param);
                  }                              
                }
                else {
                  g = archNodeIns(g, c->label, libname, x, y, z, (short) io,
                    param);                  
                }

  }
*/
  
  *error = ARCHNO_ERROR;
}

/**Function********************************************************************

  Synopsis           [Parse the specification of the components position, when
  in a connection declaration. This version is for declarations outside a for
  loop.]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static void
GetCpos(
  array_t *tokenlist,
  int *token_index,
  int *linen,
  int *error,
  int *cx1,
  int *cx2,
  int *cy1,
  int *cy2,
  int *cz1,
  int *cz2)
{
char *tok3;
int i, j;

  tok3 = comp_NextToken(tokenlist, token_index, linen);
  if(strcmp(tok3,"(") == 0) {
    for(i = 0;i < 2;i++) {
      tok3 = comp_NextToken(tokenlist, token_index, linen);
      for(j = 0;j < (int) strlen(tok3);j++) {
        if(isdigit(tok3[j]) == 0) {
          *error = ARCHWRONG_NUMERIC_PARAM;
          return;
        }
      }
      if(i == 0) {
        *cx1 = atoi(tok3);
      }
      else {
        *cx2 = atoi(tok3);
      }        
      tok3 = comp_NextToken(tokenlist, token_index, linen);
      if(strcmp(tok3,",") != 0) {
        *error = ARCHMISSING_VIR;
        return;
      }
      tok3 = comp_NextToken(tokenlist, token_index, linen);
      for(j = 0;j < (int) strlen(tok3);j++) {
        if(isdigit(tok3[j]) == 0) {
          *error = ARCHWRONG_NUMERIC_PARAM;
          return;
        }
      }
      if(!i) {
        *cy1 = atoi(tok3);
      }
      else {
        *cy2 = atoi(tok3);
      }
      tok3 = comp_NextToken(tokenlist, token_index, linen);
      if(strcmp(tok3,",") != 0) {
        *error = ARCHMISSING_VIR;
        return;
      }
      tok3 = comp_NextToken(tokenlist, token_index, linen);
      for(j = 0;j < (int) strlen(tok3);j++) {
        if(isdigit(tok3[j]) == 0) {
          *error = ARCHWRONG_NUMERIC_PARAM;
          return;
        }
      }
      if(i == 0){
        *cz1 = atoi(tok3);
        tok3 = comp_NextToken(tokenlist, token_index, linen);
        if(strcmp(tok3,";") != 0) {
          *error = ARCHMISSING_PV;
          return;
        }
      }
      else {
        *cz2 = atoi(tok3);
        tok3 = comp_NextToken(tokenlist, token_index, linen);
        if(strcmp(tok3,")") != 0) {
          *error = ARCHMISSING_CPAR;
          return;
        }
      }
    }
  }  
  else {
    (*token_index)--;      
  }
  *error = ARCHNO_ERROR;
}

/**Function********************************************************************

  Synopsis           [Parse the specification of the components position, when
  in a connection declaration. This version is for declarations inside a for
  loop.]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static void
GetCposlv(
  char *tok,
  array_t *tokenlist,
  int *token_index,
  int *linen,
  int *error,
  int *coord,
  int *type,
  archloopvar_t **var,
  array_t *lv)
{
int i;
char *tok3;

  *coord = LvarFind(lv, tok);
  if(*coord == -1){
    *var = (archloopvar_t *) ALLOC(archloopvar_t, 1);
    (*var)->from = atoi(tok);
    (*var)->label = NIL(char);
    (*var)->to = atoi(tok);
    (*var)->relat = ARCHLE;
    (*var)->step = 1;
    (*var)->assig = ARCHADD;    
    *type = ARCHADD;
    *coord = 0;
  }
  else {
    *var = array_fetch(archloopvar_t *, lv, *coord);
    tok3 = comp_NextToken(tokenlist, token_index, linen);
    if(strcmp(tok3,",") != 0) {
      if(strcmp(tok3,"+") == 0) {
        *type = ARCHADD;
      }
      else
        if(strcmp(tok3,"-") == 0) {
          *type = ARCHSUB;
        }
        else
          if(strcmp(tok3,"*") == 0) {
            *type = ARCHMULT;
          }
          else
            if(strcmp(tok3,"/") == 0) {
              *type = ARCHDIV;
            }
            else {
              *error = ARCHINVALID_ASSIG_OPER;
              return;
            }            
      tok3 = comp_NextToken(tokenlist, token_index, linen);         
      for(i = 0;i < (int) strlen(tok3);i++) {
        if(isdigit(tok3[i]) == 0) {
          *error = ARCHWRONG_NUMERIC_PARAM;
           return;
        }
      }
      *coord = atoi(tok3);                
    }
    else {
      (*token_index)--;
      *coord = 0;
      *type = ARCHADD;
    }
  }
  *error = ARCHNO_ERROR;
}

/**Function********************************************************************

  Synopsis           [Given the buses and signals, add the corresponding 
  connections to the intermediary signals structure.]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static avl_tree *
SigIns(
  avl_tree *signals,
  char *bfrom,
  int sfrom,
  char *bto,
  int sto,
  int *coord,
  char **war)
{
int i;
char key[100], key2[100], *tok3;
array_t *sigs;
short noins;

  sprintf(key,"%s_%d_%d_%d_%d", bfrom, sfrom,coord[0],coord[2],coord[4]);
  i = avl_lookup(signals, key, (char **) &sigs);
  if(i == 0) {
    sigs = array_alloc(char *, 0);
    sprintf(key2,"%s_%d_%d_%d_%d", bto, sto, coord[1], coord[3], coord[5]);
    array_insert_last(char *, sigs, util_strsav(key2));
    avl_insert(signals, util_strsav(key), (char *) sigs);
  }
  else {
    noins = 0;
    sprintf(key2,"%s_%d_%d_%d_%d", bto, sto, coord[1], coord[3], coord[5]);
    for(i = 0;(i < array_n(sigs)) && (noins == 0);i++) {
      tok3 = array_fetch(char *,sigs,i);
      if(strcmp(tok3,key2) == 0) {
        sprintf(key2,"Warning: connection declaration repeated - %s(%d) - %s(%d)\n",
          bfrom, sfrom, bto, sto);
        if(*war == NIL(char)) {
          *war = (char *) ALLOC(char *,strlen(key2)+1);
          strcpy(*war,key2);
        }
        else {
          *war = (char *) REALLOC(char *, *war, strlen(*war) + strlen(key2) + 1);
           strcat(*war,key2);
        }          
        noins = 1;
      }                 
    }    
    if(noins == 0)
      array_insert_last(char *, sigs, util_strsav(key2)); 
  }    

  return signals;
}

/**Function********************************************************************

  Synopsis           [Handle the just parsed connection declaration. Look
  for the loop case also.]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static avl_tree *
CoordsIns(
  avl_tree *signals,
  int *coord,
  int *type,
  archloopvar_t **var,
  int *from,
  int fsize,
  int *to,
  char *tok,
  char *tok2,
  array_t *lv,
  char **war)
{
int i, param, noins, c[6], newcoord[6];
char key[100], key2[100];
char *tok3;
array_t *sigs;

  if((coord[0] == -1) && (coord[2] == -1) && (coord[4] == -1)) {
    for(i = 0;i < fsize;i++) {
      sprintf(key,"%s_%d",tok,from[i]);
      param = avl_lookup(signals, key, (char **) &sigs);
      if(param == 0) {
        sigs = array_alloc(char *, 0);
        sprintf(key2,"%s_%d", tok2,to[i]);
        array_insert_last(char *, sigs, util_strsav(key2));
        avl_insert(signals, util_strsav(key), (char *) sigs);
      }
      else {
        noins = 0;
        sprintf(key2,"%s_%d",tok2, to[i]);
        for(i = 0;(i < array_n(sigs)) && !noins;i++) {
          tok3 = array_fetch(char *,sigs,i);
          if(strcmp(tok3,key2) == 0) {
            sprintf(key2,"Warning: connection declaration repeated - %s(%d) - %s(%d)\n",
              tok,from[i],tok2,to[i]);
            if(*war == NIL(char)) {
              *war = (char *) ALLOC(char *,strlen(key2)+1);
              strcpy(*war,key2);
            }
            else {
              *war = (char *) REALLOC(char *, *war,
                strlen(*war) + strlen(key2) + 1);
              strcat(*war,key2);
            }          
            noins = 1;
          }                 
        }
        if(noins == 0)
          array_insert_last(char *, sigs, util_strsav(key2)); 
      }  
    }
  }
  else {
    if((type[0] == -1) && (type[2] == -1) && (type[4] == -1)) {
      for(i = 0;i < fsize;i++) {
        signals = SigIns(signals, tok, from[i], tok2, to[i], coord, war);
      }
    }
    else {
      for(i = 0;i < fsize;i++) {   
        c[0] = var[0]->from;
        c[1] = var[1]->from;
        newcoord[0] = Update(c[0], type[0], coord[0]);
        newcoord[1] = Update(c[1], type[1], coord[1]);
        while(Relation(newcoord[0], var[0]->relat, var[0]->to) &&
              Relation(newcoord[1], var[1]->relat, var[1]->to)) {                    
          c[2] = var[2]->from;
          c[3] = var[3]->from;
          newcoord[2] = Update(c[2], type[2], coord[2]);
          newcoord[3] = Update(c[3], type[3], coord[3]);
          while(Relation(newcoord[2], var[2]->relat, var[2]->to) &&
                Relation(newcoord[3], var[3]->relat, var[3]->to)) {       
            c[4] = var[4]->from;
            c[5] = var[5]->from;
            newcoord[4] = Update(c[4], type[4], coord[4]);
            newcoord[5] = Update(c[5], type[5], coord[5]);
            while(Relation(newcoord[4], var[4]->relat, var[4]->to) &&
                  Relation(newcoord[5], var[5]->relat, var[5]->to)) {

              signals = SigIns(signals, tok, from[i], tok2, to[i],
                newcoord, war);

              c[4] = Update(c[4], var[4]->assig, var[4]->step);
              c[5] = Update(c[5], var[5]->assig, var[5]->step);
              newcoord[4] = Update(c[4], type[4], coord[4]);
              newcoord[5] = Update(c[5], type[5], coord[5]);          
            }                                      
            c[2] = Update(c[2], var[2]->assig, var[2]->step);
            c[3] = Update(c[3], var[3]->assig, var[3]->step);
            newcoord[2] = Update(c[2], type[2], coord[2]);
            newcoord[3] = Update(c[3], type[3], coord[3]);          
          }              
          c[0] = Update(c[0], var[0]->assig, var[0]->step);
          c[1] = Update(c[1], var[1]->assig, var[1]->step);
          newcoord[0] = Update(c[0], type[0], coord[0]);
          newcoord[1] = Update(c[1], type[1], coord[1]);          
        }
      }
    }
  }
  
  return signals;
}

/**Function********************************************************************

  Synopsis           [Parse a connection declaration.]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static avl_tree *
ConnectDecl(
  avl_tree *signals,
  avl_tree *bus,
  array_t *tokenlist,
  int *token_index,
  int *linen,
  int *error,
  char *tok,
  char **war,
  array_t *lv)
{
char *tok2, *tok3;
int *from, *to, fsize, tsize, i;
int coord[6];
int type[6];
archloopvar_t *var[6];

  for(i = 0;i < 6;i++) {
    var[i] = NIL(archloopvar_t);
    type[i] = -1;
    coord[i] = -1;
  }
  GetSigs(&from, &fsize, &to, &tsize, tok, &tok2, bus, tokenlist, 
    token_index, linen, error);    
  if(*error != ARCHNO_ERROR) {
    return signals;
  }  
  if(lv == NIL(array_t)) {
    GetCpos(tokenlist, token_index, linen, error, &coord[0], &coord[1], 
      &coord[2], &coord[3], &coord[4], &coord[5]);
    if(*error != ARCHNO_ERROR) {
      return signals;
    }
  }
  else {
    tok3 = comp_NextToken(tokenlist, token_index, linen);
    if(strcmp(tok3,"(") != 0) {
      *error = ARCHMISSING_COORD;
      return signals;
    }
    for(i = 0;i < 2;i++) {
      tok3 = comp_NextToken(tokenlist, token_index, linen);
      if(i == 0) {
        GetCposlv(tok3, tokenlist, token_index, linen, error, &coord[0], 
          &type[0], &var[0], lv);
      }
      else {
        GetCposlv(tok3, tokenlist, token_index, linen, error, &coord[1], 
          &type[1], &var[1], lv);
      }
      if(*error != ARCHNO_ERROR) {
        return signals;
      }

      tok3 = comp_NextToken(tokenlist, token_index, linen);
      if(strcmp(tok3,",") != 0) {
        *error = ARCHMISSING_VIR;
        return signals;
      }
      tok3 = comp_NextToken(tokenlist, token_index, linen);
      if(i == 0) {
        GetCposlv(tok3, tokenlist, token_index, linen, error, &coord[2], 
          &type[2], &var[2], lv);
       }
      else {
        GetCposlv(tok3, tokenlist, token_index, linen, error, &coord[3],
          &type[3], &var[3], lv);
      }          
      if(*error != ARCHNO_ERROR) {
        return signals;
      }
      tok3 = comp_NextToken(tokenlist, token_index, linen);
      if(strcmp(tok3,",") != 0) {
        *error = ARCHMISSING_VIR;
        return signals;
      }
      tok3 = comp_NextToken(tokenlist, token_index, linen);
      if(i == 0) {
        GetCposlv(tok3, tokenlist, token_index, linen, error, &coord[4],
          &type[4], &var[4], lv);
        if(*error != ARCHNO_ERROR) {
          return signals;
        }
        tok3 = comp_NextToken(tokenlist, token_index, linen);
        if(strcmp(tok3,";") != 0) {
          *error = ARCHMISSING_PV;
          return signals;
        }        
      }      
      else {
        GetCposlv(tok3, tokenlist, token_index, linen, error, &coord[5],
          &type[5], &var[5], lv);
        if(*error != ARCHNO_ERROR) {
          return signals;
        }
        tok3 = comp_NextToken(tokenlist, token_index, linen);
        if(strcmp(tok3,")") != 0) {
          *error = ARCHMISSING_CPAR;
          return signals;
        }
      }
    }  
  }
    
  signals = CoordsIns(signals, coord, type, var, from, fsize, to, tok,
    tok2, lv, war);
  
  FREE(from);
  FREE(to);

  *error = ARCHNO_ERROR;  
  return signals;  
}

/**Function********************************************************************

  Synopsis           [Given the intermediary signal structure, build the
  architecture graph.]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static arch_graph_t *
MakeArch(
  arch_graph_t *g,
  st_table *signals,
  st_table *components,
  lsList *libs,
  int size,
  char **war)
{
char *key, *value, *cname, *sig, *tmp;
archsignal_t *as;
archcomp_t *ac, *ac2;
st_generator *stgen;
comp_data_t *c, *c2;
comp_pin_t *cp, *cp2;
int i, j, k, ret;
vertex_t *v1, *v2;
arch_node_t *n1, *n2;

  st_foreach_item(components, stgen, &key, &value) {
    ac = (archcomp_t *) value;
    tmp = util_strsav(key);
    cname = strtok(tmp, "_");
    g = archNodeIns(g, cname, ac->libname, ac->x, ac->y, ac->z, ac->io,
      ac->logic);
    FREE(tmp);
  }

  st_foreach_item(components, stgen, &key, &value) {
    ac = (archcomp_t *) value;
    v1 = arch_NodeFind(g, ac->x, ac->y, ac->z);    
    if(v1 == NIL(vertex_t)) {
      printf("Nao achei o c1 no grafo\n");
    }
    n1 = (arch_node_t *) v1->user_data;
    tmp = util_strsav(key);
    cname = strtok(tmp, "_");
    c = NIL(comp_data_t);
    for(i = 0;i < size;i++) {    
      c = comp_Find(libs[i], cname);
      if(c != NIL(comp_data_t)) break;
    }
    if(c == NIL(comp_data_t)) {
      printf("Nao achou o componente\n");
    }
    cname = util_strsav(key);
    FREE(tmp); 
    for(i = 0;i < array_n(ac->sigs);i++) {
      sig = array_fetch(char *, ac->sigs, i);
      cp = array_fetch(comp_pin_t *, c->pins, i);
      ret = st_lookup(signals, sig, &value);
      if(ret == 0) {
        printf("Nao achou o signal %s.\n", sig);
      }
      as = (archsignal_t *) value;
      for(j = 0;j < array_n(as->comps);j++) {
        key = array_fetch(char *, as->comps, j);
        if(strcmp(key, cname) == 0) continue;
        ret = st_lookup(components, key, &value);
        if(ret == 0) {
          printf("Nao achou o componente pela tabela de sinais.\n");
        }
        ac2 = (archcomp_t *) value;
        v2 = arch_NodeFind(g, ac2->x, ac2->y, ac2->z);
        if(v2 == NIL(vertex_t)) {
          printf("Nao achei o c2 no grafo\n");
        }
        n2 = (arch_node_t *) v2->user_data;
        c2 = NIL(comp_data_t);
        tmp = util_strsav(key);
        key = strtok(tmp,"_");
        for(k = 0;k < size;k++) {
          c2 = comp_Find(libs[k], key);
          if(c2 != NIL(comp_data_t)) break;
        }
        if(c2 == NIL(comp_data_t)) {
          printf("O componente c2 nao existe.\n");
        }
        FREE(tmp);
        for(k = 0;k < array_n(ac2->sigs);k++) {
          key = array_fetch(char *, ac2->sigs, k);
          if(strcmp(key, sig) == 0) break;
        }  
        if(k == array_n(ac2->sigs)) {
          printf("Nao achou o signal na lista do c2");
        }
        cp2 = array_fetch(comp_pin_t *, c2->pins, k);
        
        if(((cp->type == COMP_LINPUT) && (cp2->type == COMP_LOUTPUT)) ||
           ((cp->type == COMP_LINPUT) && (cp2->type == COMP_LINPUT)) ||
           ((cp->type == COMP_LOUTPUT) && (cp2->type == COMP_LOUTPUT)) ||
           ((cp->type == COMP_ROUTING) && (cp2->type == COMP_LINPUT)) ||
           ((cp->type == COMP_ROUTING) && (cp2->type == COMP_LOUTPUT))) {
          continue;   
        }         
        archEdgeIns(g, n1, n2, i, cp->pos, cp->type, k, cp2->pos,
          cp2->type);
      }
    }  
    FREE(cname);    
  }    

  return g;
}

/**Function********************************************************************

  Synopsis           [Given some intermediary structures, and the architecture
  generate some warnings.]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static void
Warnings(
  arch_graph_t *g,
  avl_tree *signals,
  avl_tree *bus,
  char **war)
{
/*
avl_generator *gen;
char *tok,*tok2;
archbus_t *b;
char tmp[200];
array_t *ub;
int i;

  ub = array_alloc(char *,0);
  avl_foreach_item(bus, gen, AVL_FORWARD, &tok, &tok2) {  
    b = (archbus_t *) tok2;    
    if(array_n(b->comps) == 0) {
      array_insert_last(char *, ub, util_strsav(tok));
      if((*war) == NIL(char)) {
        sprintf(tmp,"Warning: unused bus %s\n",tok);
        (*war) = (char *) ALLOC(char *,strlen(tmp)+1);
        strcpy((*war),tmp);              
      }
      else {
        sprintf(tmp,"Warning: unused bus %s\n",tok);
        (*war) = (char *) REALLOC(char *, (*war), strlen((*war)) + 
          strlen(tmp) + 1);
        strcat((*war),tmp);
      }
    }  
  }  
  
  if(array_n(ub) > 0) {
    avl_foreach_item(signals, gen, AVL_FORWARD, &tok, &tok2) {
      strcpy(tmp,tok);
      tok2 = strtok(tmp,"_");
      for(i = 0;i < array_n(ub);i++) {
        tok = array_fetch(char *,ub,i);
        if(strcmp(tok,tok2) == 0) {
          sprintf(tmp,"Warning: unused bus %s in a connection declaration\n",
            tok);
          (*war) = (char *) REALLOC(char *, (*war), strlen((*war)) + 
            strlen(tmp) + 1);
          strcat((*war),tmp);         
          strcpy(tok," ");          
          break;
        }
      }  
    }
  }
  
  for(i = 0;i < array_n(ub);i++) {
    tok = array_fetch(char *,ub,i);
    FREE(tok);
  }
  array_free(ub);
*/  
}

/**Function********************************************************************

  Synopsis           [Parse the body of the architecture specification.]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static arch_graph_t *
Body(
  arch_graph_t *g,
  st_table *signals,  
  array_t *tokenlist,
  int *token_index,
  int *linen,
  array_t *liblist,
  array_t *ablist,
  int *error,
  char **war)
{
st_table *components;
lsList *libs;
char *tok, *tok2, *key, *value;
st_generator *gen;
archsignal_t *as;
archcomp_t *ac;
int i;

  components = st_init_table(strcmp, st_strhash);
  libs = (lsList *) ALLOC(lsList, array_n(liblist));
  for(i = 0;i < array_n(liblist);i++) {
    tok = array_fetch(char *, liblist, i);
    libs[i] = comp_LibLoad(tok);
  }
  
  tok = comp_NextToken(tokenlist, token_index, linen);
  while(strcmp(tok,"end") != 0) {
    if(Reserved(tok) == 0) {
      tok2 = comp_NextToken(tokenlist, token_index, linen);
      if(strcmp(tok2, "(") == 0) {                 
        CompInst(signals, components, tokenlist, token_index,
          linen, libs, liblist, tok, error, NIL(array_t));          
        if(*error != ARCHNO_ERROR) {
          return g;
        }
      }
      else {
        *error = ARCHPARSE_ERROR;
         return g;
      }
    }
    else {
      *error = ARCHPARSE_ERROR;
      return g;                        
    }

/*
    if(strcmp(tok, "bus") == 0) {
      bus = BusDecl(bus, g, tokenlist, token_index, linen, error);
      if(*error != ARCHNO_ERROR) {
        return g;
      }
    }    
    else 
      if(strcmp(tok, "for") == 0) {
        array_t *lv;
        
        lv = array_alloc(archloopvar_t *, 0);              
        tok = comp_NextToken(tokenlist,token_index, linen);
        if(strcmp(tok,"(") != 0) {
          *error = ARCHMISSING_OPAR;
          return g;          
        }
        *error = GetVars(&lv, tokenlist, token_index, linen);  
        if(*error != ARCHNO_ERROR) {
          return g;
        }        
        tok = comp_NextToken(tokenlist,token_index, linen);
        if(strcmp(tok,"begin") != 0) {
          *error = ARCHMISSING_BEGIN;
          return g;          
        }               
        tok = comp_NextToken(tokenlist, token_index, linen);
        while(strcmp(tok,"end") != 0) {
          if(Reserved(tok) == 0) {
            tok2 = comp_NextToken(tokenlist, token_index, linen);
            if(strcmp(tok2,"(") == 0) {
              g = CompInst(g, tokenlist, token_index, linen, libs, liblist,
                array_n(liblist), bus, tok, error, lv);
              if(*error != ARCHNO_ERROR) {
                return g;
              }                        
            }
            else
              if(strcmp(tok2,"[") == 0) {
                signals = ConnectDecl(signals, bus, tokenlist, token_index,
                  linen, error, tok, war, lv);
                if(*error != ARCHNO_ERROR) {
                  return g;
                } 
              }
              else {
                *error = ARCHPARSE_ERROR;
                return g;              
              }
          }
          tok = comp_NextToken(tokenlist, token_index, linen);        
        }
      }
      else
*/        
    tok = comp_NextToken(tokenlist, token_index, linen);    
  }  

  g = MakeArch(g, signals,components, libs, array_n(liblist), war); 

  archPrint(g);

  st_foreach_item(signals, gen, &key, &value) {
    FREE(key);
    as = (archsignal_t *) value;
    for(i = 0;i < array_n(as->comps);i++) {
      value = array_fetch(char *, as->comps, i);
      FREE(value);
    }
    array_free(as->comps);
    FREE(as);
  }
  st_free_table(signals);

  st_foreach_item(components, gen, &key, &value) {
    FREE(key);
    ac = (archcomp_t *) value;
    for(i = 0;i < array_n(ac->sigs);i++) {
      value = array_fetch(char *, ac->sigs, i);
      FREE(value);
    }
    array_free(ac->sigs);
    FREE(ac->libname);
    FREE(ac);
  }
  st_free_table(components);
  
/*
  Warnings(g, signals, bus, war); 
*/

  *error = ARCHNO_ERROR;
  
  return g;  
}
