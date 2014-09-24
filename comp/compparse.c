/**CFile***********************************************************************

  FileName    [compParse.c]

  PackageName [comp]

  Synopsis    [Parse a component description]

  Description [Given a string describing a component, generate the respective
  comp_data structure. Additional, this files contains basic functions for
  breaking a text file into a set of tokens. This is also used by the
  architecture parser.]

  SeeAlso     [arch]

  Author      [Ivan]

  Copyright   [1996-1997 Ivan Jeukens]

******************************************************************************/

#include "compInt.h"

/*
static char rcsid[] = "$Id: $";
USE(rcsid);
*/


/*---------------------------------------------------------------------------*/
/* Variable declarations                                                     */
/*---------------------------------------------------------------------------*/

comp_data_t *component;

/**AutomaticStart*************************************************************/

/*---------------------------------------------------------------------------*/
/* Static function prototypes                                                */
/*---------------------------------------------------------------------------*/

static array_t * TokenIns(array_t *list, char *token, int linen);
static int Reserved(char *data);
static int ListScan(int ecode, int act, array_t *tokenlist, int *token_index, int *linen, array_t **siglist);
static int PinDecl(array_t *tokenlist, int *token_index, int *linen);
static int LayoutDecl(array_t *tokenlist, int *token_index, int *linen);
static int RedgeIns(char *tok1, char *tok2, array_t *tokenlist, int *token_index, int *linen);
static int LogicIns(char *data, int ecode, int act, array_t *tokenlist, int *token_index, int *linen);
static int MemIns(char *tok, char **clk, array_t *tokenlist, int *token_index, int *linen, array_t *siglist);
static int ConstrDecl(array_t *tokenlist, int *token_index, int *linen);
static int ParseExpr(char *output, array_t *tokenlist, int *token_index, int *linen);
static int StructDecl(array_t *tokenlist, int *token_index, int *linen);

/**AutomaticEnd***************************************************************/


/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis           [Release the memory associated with an array of tokens.]

  Description        []

  SideEffects        []
  
  SeeAlso	     []
  
******************************************************************************/
void
comp_TokenFree(array_t *tokens)
{
comptoken_t *ptr;
int i;

  if(tokens == NIL(array_t)) {
    return;
  }
  for(i = 0;i < array_n(tokens);i++) {
    ptr = array_fetch(comptoken_t *, tokens, i);
    FREE(ptr->text);
    FREE(ptr);
  }

  array_free(tokens);
}

/**Function********************************************************************

  Synopsis           [Given an string, extract the set of tokens determined
  by separators.]

  Description        []

  SideEffects        []
  
  SeeAlso	     [archParse.c]
  
******************************************************************************/
array_t *
comp_TokenExtr(char *data)
{
array_t *tokens;
int i = 0,a = 0;
char temp[80];
int linen = 1;

  if(data == NIL(char)) {
    return NIL(array_t);
  }  
  tokens = array_alloc(comptoken_t *,0);  

  while(data[i] != '\0') {
    switch(data[i]) {
      case '(' : { if(a > 0) {
                     temp[a] = '\0';
                     tokens = TokenIns(tokens, temp, linen);
      		     temp[0] = '\0';
      		     a = 0;
                   }
                   tokens = TokenIns(tokens,"(\0",linen);
                 } break;
      case ')' : { if(a > 0) {
                     temp[a] = '\0';
                     tokens = TokenIns(tokens,temp,linen);
      		     temp[0] = '\0';
      		     a = 0;
                   }
                   tokens = TokenIns(tokens,")\0",linen);
                 } break;            
      case '[' : { if(a > 0) {
                     temp[a] = '\0';
                     tokens = TokenIns(tokens,temp,linen);
      		     temp[0] = '\0';
      		     a = 0;
                   }
                   tokens = TokenIns(tokens,"[\0",linen);
                 } break;
      case ']' : { if(a > 0) {
                     temp[a] = '\0';
                     tokens = TokenIns(tokens,temp,linen);
      		     temp[0] = '\0';
      		     a = 0;
                   }
                   tokens = TokenIns(tokens,"]\0",linen);
                 } break;
      case '{' : { if(a > 0) {
                     temp[a] = '\0';
                     tokens = TokenIns(tokens,temp,linen);
      		     temp[0] = '\0';
      		     a = 0;
                   }
                   tokens = TokenIns(tokens,"{\0",linen);
                 } break;
      case '}' : { if(a > 0) {
                     temp[a] = '\0';
                     tokens = TokenIns(tokens,temp,linen);
      		     temp[0] = '\0';
      		     a = 0;
                   }
                   tokens = TokenIns(tokens,"}\0",linen);
                 } break;
      case '\t':
      case ' ' :
      case '\n': { if(a > 0) {
		     temp[a] = '\0';
		     tokens = TokenIns(tokens,temp,linen);
      		     temp[0] = '\0';
      		     a = 0;      		     
		   }
		   linen++;
		 } break;
      case ',' : { if(a > 0) {
		     temp[a] = '\0';
      		     tokens = TokenIns(tokens,temp,linen);
      		     temp[0] = '\0';
      		     a = 0;      		     
      		   }
      		   tokens = TokenIns(tokens,",\0",linen);
      		 } break;
      case ';' : { if(a > 0) {
      		     temp[a] = '\0';
      		     tokens = TokenIns(tokens,temp,linen);
      		     temp[0] = '\0';
      		     a = 0;      		     
      		   }
      		   tokens = TokenIns(tokens,";\0",linen);
      		 } break;
      case '+' : { if(a > 0) {
                     temp[a] = '\0';
                     tokens = TokenIns(tokens, temp, linen);
                     temp[0] = '\0';
                     a = 0;
                   }
                   tokens = TokenIns(tokens,"+\0",linen);
                 } break;
      case '-' : { if(a > 0) {
                     temp[a] = '\0';
                     tokens = TokenIns(tokens, temp, linen);
                     temp[0] = '\0';
                     a = 0;
                   }
                   tokens = TokenIns(tokens,"-\0",linen);
                 } break;
      case '/' : { if(a > 0) {
                     temp[a] = '\0';
                     tokens = TokenIns(tokens, temp, linen);
                     temp[0] = '\0';
                     a = 0;
                   }
                   tokens = TokenIns(tokens,"/\0",linen);
                 } break;                 
      case '*' : { if(a > 0) {
                     temp[a] = '\0';
                     tokens = TokenIns(tokens, temp, linen);
                     temp[0] = '\0';
                     a = 0;
                   }
                   tokens = TokenIns(tokens,"*\0",linen);
                 } break;
      case '>' : { if(a > 0) {
                     temp[a] = '\0';
                     tokens = TokenIns(tokens, temp, linen);
                     temp[0] = '\0';
                     a = 0;
                   }
                   tokens = TokenIns(tokens,">\0",linen);
                 } break;                 
      case '<' : { if(a > 0) {
                     temp[a] = '\0';
                     tokens = TokenIns(tokens, temp, linen);
                     temp[0] = '\0';
                     a = 0;
                   }
                   tokens = TokenIns(tokens,"<\0",linen);
                 } break;                 
      case '=' : { if(a > 0) {
      		     temp[a] = '\0';
      		     tokens = TokenIns(tokens, temp,linen);
      		     temp[0] = '\0';
      		     a = 0;      		     
      		   }
      		   tokens = TokenIns(tokens,"=\0",linen);
      		 } break;
      case '!' : { if(a > 0) {
      		     temp[a] = '\0';
      		     tokens = TokenIns(tokens, temp,linen);
      		     temp[0] = '\0';
      		     a = 0;      		     
      		   }
      		   tokens = TokenIns(tokens,"!\0",linen);
      		 } break;
      case ':' : { if(a > 0) {
      		     temp[a] = '\0';
      		     tokens = TokenIns(tokens, temp,linen);
      		     temp[0] = '\0';
      		     a = 0;      		     
      		   }
      		   tokens = TokenIns(tokens,":\0",linen);
      		 } break;      		 
      default : { temp[a] = tolower(data[i]); a++;} break;
    };
    i++;
  }
  if(a > 0) {
    temp[a] = '\0';
    tokens = TokenIns(tokens, temp, linen);
  }  

  return tokens;
}

/**Function********************************************************************

  Synopsis           [Given a list of tokens and an index, return the next
  token and modify the index. This function handle comments.]

  Description        []

  SideEffects        []
  
  SeeAlso	     [archParse.c]
  
******************************************************************************/
char *
comp_NextToken(
  array_t *list,
  int *index,
  int *ln)
{
comptoken_t *ptr, *ptr2;

  if(list == NIL(array_t)) {
    return "--EOF--";
  }
  if(*index == array_n(list)) {
    return "--EOF--";
  }
  ptr = array_fetch(comptoken_t *, list, *index);

  if(strcmp(ptr->text, "/") == 0) {
    (*index)++;
    if((*index) == array_n(list)) {
      return "--EOF--";
    }
    ptr2 = array_fetch(comptoken_t *, list, *index);
    while((strcmp(ptr->text,"/") == 0) && (strcmp(ptr2->text,"*") == 0)) {
      (*index)++;
      if((*index) == array_n(list)) {
        return "--EOF--";
      }
      ptr = array_fetch(comptoken_t *, list, *index);
      (*index)++;
      if((*index) == array_n(list)) {
        return "--EOF--";      
      }
      ptr2 = array_fetch(comptoken_t *, list, *index);
      while((strcmp(ptr->text,"*") != 0) || (strcmp(ptr2->text,"/") != 0)) {
        (*index)++;
        if((*index) == array_n(list)) {
          return "--EOF--";
        }
        ptr = array_fetch(comptoken_t *, list, *index);
        (*index)++;
        if((*index) == array_n(list)) {
          return "--EOF--";      
        }
        ptr2 = array_fetch(comptoken_t *, list, *index);
      }
      (*index)++;
      if((*index) == array_n(list)) {
        return "--EOF--";
      }
      ptr = array_fetch(comptoken_t *, list, *index);
      if(strcmp(ptr->text, "/") != 0) { 
        (*index) += 2; 
        break; 
      }
      (*index)++;
      if((*index) == array_n(list)) {
        return "--EOF--";      
      }
      ptr2 = array_fetch(comptoken_t *, list, *index);
    }
    (*index) -= 2;
  }
  (*index)++;
  *ln = ptr->ln;
  return ptr->text;
}

/**Function********************************************************************

  Synopsis           [Print a list of tokens]

  Description        []

  SideEffects        []
  
  SeeAlso	     []
  
******************************************************************************/
void
comp_TokenPrint(array_t *list)
{
comptoken_t *ptr;
int i;
 
   if(list == NIL(array_t)) {
     return;
   }
   for(i = 0;i < array_n(list);i++) {
     ptr = array_fetch(comptoken_t *,list,i);
     printf("%s %d\n",ptr->text, ptr->ln);
   }
}

/**Function********************************************************************

  Synopsis           [Parse a component]

  Description        []

  SideEffects        []
  
  SeeAlso	     []
  
******************************************************************************/
comp_data_t *
comp_Parse(
  char *data,
  int *linen,
  int *er)
{
array_t *tokenlist;
int token_index = 0, erro;
char *tok;

  component = NIL(comp_data_t);  
  tokenlist = comp_TokenExtr(data);
  tok = comp_NextToken(tokenlist, &token_index, linen);
  component = compAlloc(" ");

  if(strcmp(tok,"pin") != 0) {
   comp_TokenFree(tokenlist);
   comp_Free(component);
   *er = NO_PIN;   
   return NIL(comp_data_t);
  }
  else {
    erro = PinDecl(tokenlist, &token_index, linen);
    if(erro != NO_ERROR) {
      comp_TokenFree(tokenlist);
      comp_Free(component);
      *er = erro;
      return NIL(comp_data_t);
    }
    tok = comp_NextToken(tokenlist, &token_index, linen);
    if(strcmp(tok,"layout") != 0) {
      comp_TokenFree(tokenlist);
      comp_Free(component);
      *er = NO_LAYOUT;
      return NIL(comp_data_t);
    }
    else {
      erro = LayoutDecl(tokenlist, &token_index, linen);
      if(erro != NO_ERROR) {
        comp_TokenFree(tokenlist);
        comp_Free(component);
        *er = erro;
        return NIL(comp_data_t);
      }
      if(compPinNpos(component->pins)) {
        comp_TokenFree(tokenlist);
        comp_Free(component);
        *er = PIN_NO_POS;
        return NIL(comp_data_t);
      }          
      tok = comp_NextToken(tokenlist, &token_index, linen);
/*
      if(strcmp(tok, "constraints") == 0) {
        erro = ConstrDecl(tokenlist, &token_index, linen);
        if(erro != NO_ERROR) {
          comp_TokenFree(tokenlist);
          comp_Free(component);
          *er = erro;
          return NIL(comp_data_t);
        }
        tok = comp_NextToken(tokenlist, &token_index, linen);
      }
*/
      
      if(strcmp(tok,"structure") != 0) {
        comp_TokenFree(tokenlist);
        comp_Free(component);
        *er = NO_STRUCTURE;
        return NIL(comp_data_t);
      }
      else {
        erro = StructDecl(tokenlist, &token_index, linen);
        if(erro != NO_ERROR) {
          comp_TokenFree(tokenlist);
          comp_Free(component);
          *er = erro; 
          return NIL(comp_data_t);
	}
        comp_TokenFree(tokenlist);
        *er = NO_ERROR;
        component = compFigCreate(component);                                    
	return component;
      }        
    }
  }  
  return NIL(comp_data_t);
}

/**Function********************************************************************

  Synopsis           [Given an error code, return the corresponding error 
  string]

  Description        []

  SideEffects        []
  
  SeeAlso	     []
  
******************************************************************************/
char *
comp_Error(int code)
{
char *message;

  message = (char *) ALLOC(char, 80);
  strcpy(message,"Parse error: ");
  switch(code) {
    case NO_PIN: {
      strcat(message,"Pin declaration missing");
    } break;
    case NO_LAYOUT: {
      strcat(message,"Layout declaration missing");
    } break;
    case NO_STRUCTURE: {
      strcat(message,"Structure declaration missing");
    } break;
    case NO_END: {
      strcat(message,"END directive missing");
    } break;
    case INVALID_PIN: {
      strcat(message,"Invalid pin declaration");
    } break;
    case INVALID_INPUT_LOGIC_PIN: {
      strcat(message,"Invalid logic input pin declaration");
    } break;
    case INVALID_OUTPUT_LOGIC_PIN: {
      strcat(message,"Invalid logic output pin declaration");
    } break;
    case INVALID_ROUTING_PIN: {
      strcat(message,"Invalid routing pin declaration");
    } break;
    case INVALID_LAYOUT: {
      strcat(message,"Invalid layout declaration");
    } break;
    case INVALID_EAST: {
      strcat(message,"Invalid EAST declaration");
    } break;
    case INVALID_WEST: {
      strcat(message,"Invalid WEST declaration");
    } break;
    case INVALID_NORTH: {
      strcat(message,"Invalid NORTH declaration");
    } break;
    case INVALID_SOUTH: {
      strcat(message,"Invalid SOUTH declaration");
    } break;
    case INVALID_UP: {
      strcat(message,"Invalid UP declaration");
    } break;
    case INVALID_DOWN: {
      strcat(message,"Invalid DOWN declaration");
    } break;
    case INVALID_STRUCTURE: {
      strcat(message,"Invalid structure declaration");
    } break;
    case INVALID_ADD_EDGE: {
      strcat(message,"Invalid ADD_EDGE statement");
    } break;
    case INVALID_MUX: {
      strcat(message,"Invalid MUX statement");
    } break;
    case INVALID_MEM: {
      strcat(message,"Invalid MEM statement");
    } break;
    case INVALID_LUT: {
      strcat(message,"Invalid LUT statement");
    } break;
    case MISSING_PIN: {
      strcat(message,"Didn't find the specified pin");
    } break;
    case MISSING_PIN_ADD_REDGE: {
      strcat(message,"Inexistent pin in ADD_EDGE declaration");
    } break;
    case WRONG_OUTPUT_ASSIGMENT: {
      strcat(message,"Wrong output assigment");
    } break;
    case WRONG_INPUT_DECLARATION: {
      strcat(message,"Wrong input declaration");
    } break;
    case PIN_NO_POS: {
      strcat(message,"pins without declaration");
    } break;
    case WRONG_MEM_COMPONENT: {
      strcat(message,"Wrong memory component type");
    } break;
    case MULTIPLE_OUTPUT: {
      strcat(message,"Multiple outputs");
    } break;
    case INVALID_CLOCK_PIN: {
      strcat(message, "Invalid clock pin declaration");
    } break;
    case MULTIPLE_INPUT: {
      strcat(message,"Multiple inputs");
    } break;
    case INVALID_SHAREDI_PIN: {
      strcat(message,"Invalid shared input pin declaration");
    } break;
    case INVALID_SHAREDO_PIN: {
      strcat(message,"Invalid shared output pin declaration");
    } break;                              
    case MISSING_LAYOUT: {
      strcat(message,"Missing pins in the layout declaration"); 
    } break;   
    case AREA_INT: {
      strcat(message,"Area constraint must be an integer");
    } break; 
    case WRONG_PIN_DELAY: {
      strcat(message,"Delay constraint only valid for logic input pins");
    } break;
    case WRONG_PAREM_EXPR: {
      strcat(message,"Wrong use of parentheses in expression");
    } break;
    case WRONG_EXPR: {
      strcat(message,"Invalid expression");
    } break;
    case WITH_POS: {
      strcat(message,"Pin with more than one position");
    } break;
    case WRONG_BUS_SIZE: {
      strcat(message,"Wrong bus size");
    } break;
  }
  return message;
}

/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis           [Find a token.]

  Description        []

  SideEffects        []
  
  SeeAlso	     []
  
******************************************************************************/
static int
TokenFind(
  array_t *tokens,
  char *data)
{
comptoken_t *tok;
int i;

  if(tokens == NIL(array_t)) {
    return -1;
  }
  for(i = 0;i < array_n(tokens);i++) {
    tok = array_fetch(comptoken_t *, tokens, i);    
    if(strcmp(tok->text, data) == 0) {
      return i;
    }
  }
  
  return -1;
}

/**Function********************************************************************

  Synopsis           [Insert a token at the end of a token list.]

  Description        []

  SideEffects        []
  
  SeeAlso	     []
  
******************************************************************************/
static array_t *
TokenIns(
  array_t *list,
  char *token,
  int linen)
{
comptoken_t *new;

  new = (comptoken_t *) ALLOC(comptoken_t, 1);
  new->ln = linen;
  new->text = util_strsav(token);  
  array_insert_last(comptoken_t *, list, new);

  return list;
}

/**Function********************************************************************

  Synopsis           [See if a string is a keyword.]

  Description        []

  SideEffects        []
  
  SeeAlso	     []
  
******************************************************************************/
static int
Reserved(char *data)
{
   if(!strcmp(data,"pin") ||
      !strcmp(data,"routing") ||
      !strcmp(data,"logic") ||
      !strcmp(data,"input") ||
      !strcmp(data,"output") ||
      !strcmp(data,"layout") ||
      !strcmp(data,"east") ||
      !strcmp(data,"west") ||
      !strcmp(data,"north") ||
      !strcmp(data,"south") ||
      !strcmp(data,"up") ||
      !strcmp(data,"down") ||
      !strcmp(data,"structure") ||
      !strcmp(data,"signal") ||
      !strcmp(data,"add_edge") ||
      !strcmp(data,"mux") ||
      !strcmp(data,"lut") ||
      !strcmp(data,"mem") ||
      !strcmp(data,"and") ||
      !strcmp(data,"or") ||
      !strcmp(data,"not") ||
      !strcmp(data,"clock") ||
      !strcmp(data,"shared") ||
      !strcmp(data,"end") )
     return 1;
   else
     return 0; 
}

/**Function********************************************************************

  Synopsis           [This function recursively scans a list of strings, 
   performing the specified action on each string. The strings are separated 
   by ',' and the list must end with an ';'. This function is used in
   parsing the pin declaration, layout declaration]

  Description        []

  SideEffects        [Possibly modify the component variable]
  
  SeeAlso	     []
  
******************************************************************************/
static int
ListScan(
  int ecode,
  int act,
  array_t *tokenlist,
  int *token_index,
  int *linen,
  array_t **siglist)
{
char *tok;
int erro = NO_ERROR, ins = 0;

  tok = comp_NextToken(tokenlist, token_index, linen);
  if(isalpha(tok[0]) && (Reserved(tok) == 0)) {
    switch(act) {
      case INSERT_ROUTING_PIN: {
        if(comp_PinFind(component->pins, tok) != -1) {
          return MULTIPLE_INPUT;
        }
        component->pins = compPinIns(component->pins, tok, COMP_ROUTING);
      } break;        

      case INSERT_INPUT_LOGIC_PIN: {
        if(comp_PinFind(component->pins, tok) != -1) {
          return MULTIPLE_INPUT;
        }
        component->pins = compPinIns(component->pins, tok, COMP_LINPUT);
      } break;

      case INSERT_OUTPUT_LOGIC_PIN: {
        if(comp_PinFind(component->pins, tok) != -1) {
          return MULTIPLE_INPUT;
        }
        component->pins = compPinIns(component->pins, tok, COMP_LOUTPUT);
      } break;
      
      case INSERT_CLOCK_PIN: {
        if(comp_PinFind(component->pins, tok) != -1) {
          return MULTIPLE_INPUT;
        }
        component->pins = compPinIns(component->pins, tok, COMP_CLK);
      } break;
      
      case INSERT_SHAREDI_PIN: {
        if(comp_PinFind(component->pins, tok) != -1) {
          return MULTIPLE_INPUT;
        }
        component->pins = compPinIns(component->pins, tok, COMP_SHAREDI);
      } break;
      
      case INSERT_SHAREDO_PIN: {
        if(comp_PinFind(component->pins, tok) != -1) {
          return MULTIPLE_INPUT;
        }
        component->pins = compPinIns(component->pins, tok, COMP_SHAREDO);
      } break;
              
      case INSERT_EAST: {
        erro = compPinSetpos(component->pins, tok, COMP_EAST);
      } break;

      case INSERT_WEST: {
        erro = compPinSetpos(component->pins, tok, COMP_WEST); 
      } break;
      
      case INSERT_NORTH: {
        erro = compPinSetpos(component->pins, tok, COMP_NORTH);
      } break;
      
      case INSERT_SOUTH: {
        erro = compPinSetpos(component->pins,tok, COMP_SOUTH); 
      } break;
      
      case INSERT_UP: {
        erro = compPinSetpos(component->pins, tok, COMP_UP); 
      } break;
        
      case INSERT_DOWN: {
        erro = compPinSetpos(component->pins, tok, COMP_DOWN);
      } break;
    }

    if(erro != NO_ERROR) {
      return erro;
    }

    tok = comp_NextToken(tokenlist, token_index, linen);

    ins = 1;
  }

  if(strcmp(tok,",") == 0) {
    switch(ecode) {
      case INVALID_ROUTING_PIN: {
        erro = ListScan(INVALID_ROUTING_PIN, INSERT_ROUTING_PIN, tokenlist,
          token_index, linen, siglist);
      } break;
      
      case INVALID_INPUT_LOGIC_PIN: {
        erro = ListScan(INVALID_INPUT_LOGIC_PIN, INSERT_INPUT_LOGIC_PIN, 
          tokenlist, token_index, linen, siglist); 
      } break;
      
      case INVALID_OUTPUT_LOGIC_PIN: {
        erro = ListScan(INVALID_OUTPUT_LOGIC_PIN, INSERT_OUTPUT_LOGIC_PIN, 
          tokenlist, token_index, linen, siglist);
      } break;
      
      case INVALID_CLOCK_PIN: {
        erro = ListScan(INVALID_CLOCK_PIN, INSERT_CLOCK_PIN, tokenlist, 
          token_index, linen, siglist);
      } break;
      
      case INVALID_SHAREDI_PIN: {
        erro = ListScan(INVALID_SHAREDI_PIN, INSERT_SHAREDI_PIN, tokenlist, 
          token_index, linen, siglist);
      } break;
          
      case INVALID_SHAREDO_PIN: {
        erro = ListScan(INVALID_SHAREDO_PIN, INSERT_SHAREDO_PIN, tokenlist, 
          token_index, linen, siglist);
      } break;
      
      case INVALID_EAST: {
        erro = ListScan(INVALID_EAST,INSERT_EAST, tokenlist, token_index, 
          linen, siglist);
      } break;
      
      case INVALID_WEST: {
        erro = ListScan(INVALID_WEST,INSERT_WEST, tokenlist, token_index, 
          linen, siglist);
      } break;
      
      case INVALID_NORTH: {
        erro = ListScan(INVALID_NORTH,INSERT_NORTH, tokenlist, token_index, 
          linen, siglist);
      } break;
      
      case INVALID_SOUTH: {
        erro = ListScan(INVALID_SOUTH,INSERT_SOUTH, tokenlist, token_index, 
          linen, siglist);
      } break;
      
      case INVALID_UP: {
        erro = ListScan(INVALID_UP,INSERT_UP, tokenlist, token_index, 
          linen, siglist); 
      } break;

      case INVALID_DOWN: {
        erro = ListScan(INVALID_DOWN,INSERT_DOWN, tokenlist, token_index, 
          linen, siglist);
      } break;
    }
    
    if(erro != NO_ERROR) {
      return erro;
    }
  }
  else {
    if(strcmp(tok,";") == 0) {
      if(ins == 0) {
        return ecode;
      }
      else {
        return 0;
      }
    }
    else {
      return ecode;
    }
  }
}

/**Function********************************************************************

  Synopsis           [Parse the PIN section of the component specification.]

  Description        []

  SideEffects        []
  
  SeeAlso	     []
  
******************************************************************************/
static int
PinDecl(
  array_t *tokenlist,
  int *token_index,
  int *linen)
{
char *tok;
int count = 0,erro;

  tok = comp_NextToken(tokenlist, token_index, linen);
  while(strcmp(tok,"end") != 0) {
   if(strcmp(tok,"routing") == 0) {
     erro = ListScan(INVALID_ROUTING_PIN, INSERT_ROUTING_PIN, tokenlist,
       token_index, linen, NIL(array_t *));
     if(erro != NO_ERROR) {
       return erro;
     }
   }
   else 
     if(strcmp(tok,"logic") == 0) {
       tok = comp_NextToken(tokenlist, token_index, linen);
       if(strcmp(tok,"input") == 0) {
         erro = ListScan(INVALID_INPUT_LOGIC_PIN, INSERT_INPUT_LOGIC_PIN,
           tokenlist, token_index, linen, NIL(array_t *));
         if(erro != NO_ERROR) {
           return erro;
         }
       }
       else
         if(strcmp(tok,"output") == 0) {
           erro = ListScan(INVALID_OUTPUT_LOGIC_PIN, INSERT_OUTPUT_LOGIC_PIN,
             tokenlist, token_index, linen, NIL(array_t *));
           if(erro != NO_ERROR) {
             return erro;
           }
         }
         else {
           return INVALID_PIN;
         }
     }
     else 
       if(strcmp(tok,"clock") == 0) {
         erro = ListScan(INVALID_CLOCK_PIN, INSERT_CLOCK_PIN, tokenlist,
           token_index, linen, NIL(array_t *));
         if(erro != NO_ERROR) {
           return erro;
         }
       }
       else
        if(strcmp(tok,"shared") == 0) {
          tok = comp_NextToken(tokenlist, token_index, linen);
          if(strcmp(tok,"input") == 0) {
            erro = ListScan(INVALID_SHAREDI_PIN, INSERT_SHAREDI_PIN,
              tokenlist, token_index, linen, NIL(array_t *));
            if(erro != NO_ERROR) {
              return erro;
            }
          }
          else
           if(strcmp(tok,"output") == 0) {
             erro = ListScan(INVALID_SHAREDO_PIN, INSERT_SHAREDO_PIN,
               tokenlist, token_index, linen, NIL(array_t *));
             if(erro != NO_ERROR) {
               return erro;
             }
           }
           else {
             return INVALID_PIN;             
           }
        }
        else
          return INVALID_PIN;	

     tok = comp_NextToken(tokenlist, token_index, linen); 
     count++; 
  }
  
  if(count == 0) {
    return NO_PIN;
  }

  return NO_ERROR;
}

/**Function********************************************************************

  Synopsis           [Parse the LAYOUT section of the component specification.]

  Description        []

  SideEffects        []
  
  SeeAlso	     []
  
******************************************************************************/
static int
LayoutDecl(
  array_t *tokenlist,
  int *token_index,
  int *linen)
{
char *tok;
int erro,count = 0;

  tok = comp_NextToken(tokenlist, token_index, linen);
  while(strcmp(tok,"end") != 0) {
    if(strcmp(tok,"east") == 0) {
      erro = ListScan(INVALID_EAST,INSERT_EAST, tokenlist, token_index, 
        linen, NIL(array_t *));
      if(erro != NO_ERROR) {
        return erro;
      }
    }
    else
      if(strcmp(tok,"west") == 0) {
       erro = ListScan(INVALID_WEST,INSERT_WEST,tokenlist, token_index, 
         linen, NIL(array_t *));
       if(erro != NO_ERROR) {
         return erro;
       }
      }
      else
        if(strcmp(tok,"north") == 0) {
          erro = ListScan(INVALID_NORTH,INSERT_NORTH, tokenlist, token_index, 
            linen, NIL(array_t *));
          if(erro != NO_ERROR) {
            return erro;
          }
        }
        else
          if(strcmp(tok,"south") == 0) {
            erro = ListScan(INVALID_SOUTH,INSERT_SOUTH, tokenlist, 
              token_index, linen, NIL(array_t *));
            if(erro != NO_ERROR) {
              return erro;
            }
          }
	  else
	    if(strcmp(tok,"up") == 0) {
	      erro = ListScan(INVALID_UP,INSERT_UP, tokenlist, token_index, 
	      linen, NIL(array_t *));
	      if(erro != NO_ERROR) {
	        return erro;
	      }
	    }
	    else
	      if(strcmp(tok,"down") == 0) {
	        erro = ListScan(INVALID_DOWN, INSERT_DOWN, tokenlist, 
	          token_index, linen, NIL(array_t *));
	        if(erro != NO_ERROR) {
	          return erro;
	        }
	      }
	      else {
	        return INVALID_LAYOUT;
	      }

    tok = comp_NextToken(tokenlist, token_index, linen);
    count++;  
  }

  if(count == 0) {
    return INVALID_LAYOUT;  
  }

  return NO_ERROR;
}

/**Function********************************************************************

  Synopsis           [Add an edge between to pins (routing nodes).]

  Description        []

  SideEffects        []
  
  SeeAlso	     []
  
******************************************************************************/
static int
RedgeIns(
  char *tok1,
  char *tok2,
  array_t *tokenlist,
  int *token_index,
  int *linen)
{
char *tok3;
int t1,t2;
comp_pin_t *ptr1,*ptr2;

  if(isalpha(tok1[0]) && (Reserved(tok1) == 0)) {
    if(strcmp(tok2,",") == 0) {
      tok2 = comp_NextToken(tokenlist, token_index, linen);
      if(isalpha(tok2[0]) && (Reserved(tok2) == 0)) {
        tok3 = comp_NextToken(tokenlist, token_index, linen);
        if(strcmp(tok3,";") == 0) {
          t1 = comp_PinFind(component->pins,tok1); 
          t2 = comp_PinFind(component->pins,tok2);
          ptr1 = array_fetch(comp_pin_t *, component->pins, t1);
          ptr2 = array_fetch(comp_pin_t *, component->pins, t2);          
          if(((ptr1->type == COMP_ROUTING) || (ptr1->type == COMP_SHAREDO) || 
              (ptr1->type == COMP_SHAREDI)) && 
             ((ptr2->type == COMP_ROUTING) || (ptr2->type == COMP_SHAREDO) || 
              (ptr2->type == COMP_SHAREDI))) {
	    component = compRnodeConnect(component, tok1, tok2);
            return NO_ERROR;
          }          
          else {
            return MISSING_PIN_ADD_REDGE;        
          }
        }
        else
          if(strcmp(tok3,",") == 0) {
            t1 = comp_PinFind(component->pins,tok1); 
            t2 = comp_PinFind(component->pins,tok2);
            ptr1 = array_fetch(comp_pin_t *, component->pins, t1);
            ptr2 = array_fetch(comp_pin_t *, component->pins, t2);          
 	    if(((ptr1->type == COMP_ROUTING) || (ptr1->type == COMP_SHAREDO) || 
 	        (ptr1->type == COMP_SHAREDI)) && 
 	       ((ptr2->type == COMP_ROUTING) || (ptr2->type == COMP_SHAREDO) || 
 	        (ptr2->type == COMP_SHAREDI))) {
	      component = compRnodeConnect(component,tok1,tok2);
	    }
            else {
              return MISSING_PIN_ADD_REDGE;
            }              
            return RedgeIns(tok2,tok3,tokenlist, token_index, linen);
          }
      }
    }
  }
  else
    if(strcmp(tok1,";") == 0) {
      return NO_ERROR;
    }

  return INVALID_ADD_EDGE;
}

/**Function********************************************************************

  Synopsis           [Add a logic primitive.]

  Description        []

  SideEffects        []
  
  SeeAlso	     []
  
******************************************************************************/
static int
LogicIns(
  char *data,
  int ecode,
  int act,
  array_t *tokenlist,
  int *token_index,
  int *linen)
{
char *tok;
comptoken_t *ptr;
array_t *sig = array_alloc(comptoken_t *,0);
int ins = 0,i,j;
lsList lpins = NULL;
comp_pin_t *cptr;

  tok = comp_NextToken(tokenlist, token_index, linen);
  while((strcmp(tok,";") != 0) && (strcmp(tok,"--EOF--") != 0)) {
    if(isalpha(tok[0]) && (Reserved(tok) == 0)) {
      ins++;
      sig = TokenIns(sig, tok, *linen);
    }
    else {
      comp_TokenFree(sig);
      return ecode;
    }
    tok = comp_NextToken(tokenlist, token_index, linen);
    if((strcmp(tok,",") != 0) && (strcmp(tok,";") != 0)) {
      comp_TokenFree(sig);
      return ecode;
    }   
    if(strcmp(tok,",") == 0) {
      tok = comp_NextToken(tokenlist, token_index, linen);  
    }
  }

  if((ins < 2) || ((act == INSERT_MUX) && (ins != 3))) {
    comp_TokenFree(sig);     
    return ecode;
  }
  else {
    i = comp_PinFind(component->pins, data);
    if(i != -1) {
      cptr = array_fetch(comp_pin_t *,component->pins, i);
      if(cptr->type == COMP_LOUTPUT) {
        lpins = compLpinIns(lpins,data,COMP_LOUTPUT);
      }
      else {
        if(cptr->type == COMP_SHAREDO) {
          lpins = compLpinIns(lpins,data, COMP_SHAREDO); 
        }
        else {
          return WRONG_OUTPUT_ASSIGMENT;
        }
      }    
    }
    else {
      lpins = compLpinIns(lpins, data, COMP_OSIGNAL);
    }
        
    for(i = 0;i < array_n(sig);i++) {
      ptr = array_fetch(comptoken_t *, sig, i);
      j = comp_PinFind(component->pins, ptr->text);
      if(j != -1) {
        cptr = array_fetch(comp_pin_t *,component->pins, j);   
        if(cptr->type == COMP_LINPUT) {
          lpins = compLpinIns(lpins, ptr->text, COMP_LINPUT);        
        }
        else {
          if(cptr->type == COMP_SHAREDI) {
            lpins = compLpinIns(lpins,ptr->text, COMP_SHAREDI);
          }
          else {
            return WRONG_INPUT_DECLARATION;
          }
        }      
      }
      else {
        lpins = compLpinIns(lpins, ptr->text, COMP_ISIGNAL);      
      }
    } 

    comp_TokenFree(sig);
    switch(act) {
      case INSERT_MUX : act = COMP_MUX; break;
      case INSERT_LUT : act = COMP_LUT; break;
    }
    if(compLoutputFind(component->lg, data) != NIL(vertex_t)) {
      return MULTIPLE_OUTPUT;
    }      
    component = comp_LprimIns(component, act, lpins, NIL(char));
    return 0;
  }
}

/**Function********************************************************************

  Synopsis           [Add a memory primitive.]

  Description        []

  SideEffects        []
  
  SeeAlso	     []
  
******************************************************************************/
static int
MemIns(
  char *tok,
  char **clk,
  array_t *tokenlist,
  int *token_index,
  int *linen,
  array_t *siglist)
{
char *tok1,*tok2,*tok3,*tok4;
lsList lpins = NULL;
int type, i;
float value1,value2;
comp_pin_t *ptr;

  tok1 = comp_NextToken(tokenlist, token_index, linen);
  if(isalpha(tok1[0]) && (Reserved(tok1) == 0)) {
    tok2 = comp_NextToken(tokenlist, token_index, linen);
    if(strcmp(tok2,",") == 0) {
      tok2 = comp_NextToken(tokenlist, token_index, linen);
      if(isalpha(tok2[0]) && (Reserved(tok2) == 0)) {
        tok3 = comp_NextToken(tokenlist, token_index, linen);
        if(strcmp(tok3,",") == 0) {
          tok3 = comp_NextToken(tokenlist, token_index, linen);
          if(isdigit(tok3[0]) && (Reserved(tok3) == 0)) {
            tok4 = comp_NextToken(tokenlist, token_index, linen);
            if(strcmp(tok4,";") == 0) {
              i = comp_PinFind(component->pins, tok);
              if(i != -1) {
                ptr = array_fetch(comp_pin_t *,component->pins, i);
                if(ptr->type == COMP_LOUTPUT) {
                  lpins = compLpinIns(lpins, tok, COMP_LOUTPUT);
                }
                else
                 if(ptr->type == COMP_SHAREDO) {
                   lpins = compLpinIns(lpins, tok, COMP_SHAREDO);                   
                 }
                 else {
                   return WRONG_OUTPUT_ASSIGMENT;
                 }              
              }
              else {
                lpins = compLpinIns(lpins, tok, COMP_OSIGNAL);              
              }

              i = comp_PinFind(component->pins, tok1);
              if(i != -1) {
                ptr = array_fetch(comp_pin_t *,component->pins,i);
                if(ptr->type == COMP_LINPUT) {
                  lpins = compLpinIns(lpins,tok1,COMP_LINPUT);
                }
                else
                  if(ptr->type == COMP_SHAREDI) {
                    lpins = compLpinIns(lpins, tok1, COMP_SHAREDI);
                  }
                  else {
                    return WRONG_INPUT_DECLARATION;
                  }              
              }
              else {
                lpins = compLpinIns(lpins, tok1, COMP_ISIGNAL);
              }              
              type = atoi(tok3);
              if((type > 5) || (type < 1)) {
                return WRONG_MEM_COMPONENT;
              }
              i = comp_PinFind(component->pins, tok2);
              ptr = array_fetch(comp_pin_t *,component->pins,i);              
              if(ptr->type == COMP_CLK) {
                lpins = compLpinIns(lpins,tok2, COMP_CLK);
              }
              else {
                return WRONG_INPUT_DECLARATION;
              }           
              (*clk) = util_strsav(tok2);                                         
              component = comp_LprimIns(component,type,lpins,NIL(char));
              return NO_ERROR;
            }              
          }
        }
      }
    }
    else
      if(strcmp(tok2,"(") == 0) {
        tok2 = comp_NextToken(tokenlist, token_index, linen);
        if(isdigit(tok2[0])) {
          tok3 = comp_NextToken(tokenlist, token_index, linen);
          if(strcmp(tok3,",") == 0) {
            tok3 = comp_NextToken(tokenlist, token_index, linen);
            if(isdigit(tok3[0])) {
              tok4 = comp_NextToken(tokenlist, token_index, linen);
	      if(strcmp(tok4,")") == 0) {
                value1 = atof(tok2);
                value2 = atof(tok3);
                tok2 = comp_NextToken(tokenlist, token_index, linen);
                if(strcmp(tok2,",") == 0) {
                  tok2 = comp_NextToken(tokenlist, token_index, linen);
                  if(isalpha(tok2[0]) && (Reserved(tok2) == 0)) {
                    tok3 = comp_NextToken(tokenlist, token_index, linen);
                    if(strcmp(tok3,",") == 0) {
                      tok3 = comp_NextToken(tokenlist, token_index, linen);
                      if(isdigit(tok3[0]) && (Reserved(tok3) == 0)) {
                        tok4 = comp_NextToken(tokenlist, token_index, linen);
                        if(strcmp(tok4,";") == 0) {
                          i = comp_PinFind(component->pins, tok);
                          if(i != -1) {
                            ptr = array_fetch(comp_pin_t *,
                              component->pins,i);
                            if(ptr->type == COMP_LOUTPUT) {
                              lpins = compLpinIns(lpins,tok, COMP_LOUTPUT);
                            }
                            else
                              if(ptr->type == COMP_SHAREDO) {
                                lpins = compLpinIns(lpins, tok, COMP_SHAREDO);
                              }
                              else {
                                return WRONG_OUTPUT_ASSIGMENT;
                              }                          
                          }
                          else {
                            lpins = compLpinIns(lpins,tok,COMP_OSIGNAL);
                          }

                          i = comp_PinFind(component->pins, tok1);
                          if(i != -1) {
                            ptr = array_fetch(comp_pin_t *,
                              component->pins, i);
                            if(ptr->type == COMP_LINPUT) {
                              lpins = compLpinIns(lpins,tok1, COMP_LINPUT);
                            }
                            else
                              if(ptr->type == COMP_SHAREDI) {
                                lpins = compLpinIns(lpins,tok1, COMP_SHAREDI);
                              }
                              else {
                                return WRONG_INPUT_DECLARATION;
                              }                          
                          }
                          else {
                            lpins = compLpinIns(lpins,tok1,COMP_ISIGNAL);
                          }

                          type = atoi(tok3);
                          if((type > 5) || (type < 1)) {
                            return WRONG_MEM_COMPONENT;
                          }
                          i = comp_PinFind(component->pins, tok2);
                          ptr = array_fetch(comp_pin_t *,
                            component->pins,i);   
                          if(ptr->type == COMP_CLK) {
                            lpins = compLpinIns(lpins,tok2,COMP_CLK);
                          }
                          else {
                            return WRONG_INPUT_DECLARATION;
                          }           
                          (*clk) = util_strsav(tok2);
                          component =comp_LprimIns(component, type, 
                           lpins, NIL(char));
                          component->lg = compMconstrIns(component->lg,
                            tok1, tok, tok2, value1, value2);                                     
                          return NO_ERROR;
                        }              
                      }
                    }
                  }
                }
              }
            }
          }
        }
      }      
  }  
  return INVALID_MEM;     
}

/**Function********************************************************************

  Synopsis           [Parse the constraint declaration.]

  Description        []

  SideEffects        []
  
  SeeAlso	     []
  
******************************************************************************/
static int
ConstrDecl(
  array_t *tokenlist,
  int *token_index,
  int *linen)
{
char *tok;
compconstr_t *new;
int i;
comp_pin_t *cptr;

  new = (compconstr_t *) ALLOC(compconstr_t, 1);
  new->area = -1;
 
  tok = comp_NextToken(tokenlist, token_index, linen);
  while(strcmp(tok,"end") != 0) {
    if(strcmp(tok, "area") == 0) {
      tok = comp_NextToken(tokenlist, token_index, linen);
      if(util2_IsInt(tok) == 0) {
        FREE(new);
        return AREA_INT;
      }
      new->area = atoi(tok);
    }
    else
      if(strcmp(tok, "delay") == 0) {
        tok = comp_NextToken(tokenlist, token_index, linen);
        i = comp_PinFind(component->pins, tok);
        if(i == -1) {
          FREE(new);
          return MISSING_PIN;
        }
        cptr = array_fetch(comp_pin_t *, component->pins, i);
        if(cptr->type != COMP_LINPUT) {
          FREE(new);
          return WRONG_PIN_DELAY;
        }        
      } 
    tok = comp_NextToken(tokenlist, token_index, linen);
  }

  component->delay = new;

  return NO_ERROR;
}

/**Function********************************************************************

  Synopsis           [Parse an expression.]

  Description        []

  SideEffects        []
  
  SeeAlso	     []
  
******************************************************************************/
static int
ParseExpr(
  char *output,
  array_t *tokenlist,
  int *token_index,
  int *linen)
{
char *tok, *expr;
int nparem = 0, var = 0;
comp_pin_t *cptr;
int i;
lsList lpins = NULL;

  expr = util_strsav("");

  tok = comp_NextToken(tokenlist, token_index, linen);
  while(strcmp(tok,";") != 0) {
    if(strcmp(tok,"(") == 0) {
      nparem++;
      var = 2;
      if((var == 1) || (var == 3)) {
        FREE(expr);
        return WRONG_EXPR;      
      }
    }
    else
      if(strcmp(tok,")") == 0) {
        nparem--;
        var = 3;
        if(nparem < 0) {
          FREE(expr);
          return WRONG_PAREM_EXPR;
        }
        if((var == 2) || (var == 0)) {
          FREE(expr);
          return WRONG_EXPR;
        }
      }
      else
        if(isalpha(tok[0]) && (Reserved(tok) == 0)) {
          if((var == 1) || (var == 3)) {
            FREE(expr);
            return WRONG_EXPR;
          }
          var = 1;          
          if(compLpinFind(lpins, tok) != NIL(complpin_t)) {
            i = comp_PinFind(component->pins, tok);
            if(i != -1) {
              cptr = array_fetch(comp_pin_t *, component->pins, i);
              if(cptr->type == COMP_LINPUT) {
                lpins = compLpinIns(lpins, tok, COMP_LINPUT);        
              }
              else {
                if(cptr->type == COMP_SHAREDI) {
                  lpins = compLpinIns(lpins, tok, COMP_SHAREDI);
                }
                else {
                  return WRONG_INPUT_DECLARATION;
                }
              }
            }              
          }
          else {
            lpins = compLpinIns(lpins, tok, COMP_ISIGNAL);      
          }
        }
        else
          if((strcmp(tok,"*") == 0) || (strcmp(tok,"!") == 0) ||
             (strcmp(tok,"+") == 0)) {
            if(((var == 0) && (strcmp(tok,"!") != 0)) || 
               ((var == 2) && (strcmp(tok,"!") != 0)) ||
               ((var == 3) && (strcmp(tok,"!") == 0))) {
              FREE(expr);
              return WRONG_EXPR;            
            }
            var = 0;
          }
          else {
            FREE(expr);
            return WRONG_EXPR;
          }

    expr = (char *) REALLOC(char, expr, strlen(expr) + strlen(tok) + 1);
    strcat(expr, tok);
    tok = comp_NextToken(tokenlist, token_index, linen);    
  }

  i = comp_PinFind(component->pins, output);
  if(i != -1) {
    cptr = array_fetch(comp_pin_t *,component->pins, i);
    if(cptr->type == COMP_LOUTPUT) {
      lpins = compLpinIns(lpins, output, COMP_LOUTPUT);
    }
    else {
      if(cptr->type == COMP_SHAREDO) {
        lpins = compLpinIns(lpins, output, COMP_SHAREDO); 
      }
      else {
        FREE(expr);
        return WRONG_OUTPUT_ASSIGMENT;
      }
    }    
  }
  else {
    lpins = compLpinIns(lpins, output, COMP_OSIGNAL);
  }
  
  if(compLoutputFind(component->lg, output) != NIL(vertex_t)) {
    FREE(expr);
    return MULTIPLE_OUTPUT;
  }      

  component = comp_LprimIns(component, COMP_EXPR, lpins, expr);
  FREE(expr);

  return NO_ERROR;
}

/**Function********************************************************************

  Synopsis           [Parse the structure declaration.]

  Description        []

  SideEffects        []
  
  SeeAlso	     []
  
******************************************************************************/
static int
StructDecl(
  array_t *tokenlist,
  int *token_index,
  int *linen)
{
char *tok,*tok2,*tok3;
int erro,count = 0;
char *clk;

  tok = comp_NextToken(tokenlist, token_index, linen);
  while(strcmp(tok,"end") != 0) {
    if(strcmp(tok,"add_edge") == 0) {
      tok2 = comp_NextToken(tokenlist, token_index, linen);
      tok3 = comp_NextToken(tokenlist, token_index, linen); 
      erro = RedgeIns(tok2, tok3, tokenlist, token_index, linen);
      if(erro != NO_ERROR) { 
        return erro;
      }
    }
      else
        if(isalpha(tok[0]) && (Reserved(tok) == 0)) {
          tok2 = comp_NextToken(tokenlist, token_index, linen);
          if(strcmp(tok2,"=") == 0) {
            tok2 = comp_NextToken(tokenlist, token_index, linen);
            if(strcmp(tok2,"mux") == 0) {
              erro = LogicIns(tok, INVALID_MUX, INSERT_MUX, tokenlist,
                token_index, linen);
              if(erro != NO_ERROR) {
                return erro;
              }
            }
            else 
              if(strcmp(tok2,"lut") == 0) {
                erro = LogicIns(tok,INVALID_LUT,INSERT_LUT, tokenlist,
                  token_index, linen);
                if(erro != NO_ERROR) { 
                  return erro;
                }
              }
              else
                if(strcmp(tok2,"mem") == 0) {
                  erro = MemIns(tok, &clk, tokenlist, token_index, linen, 
                    NIL(array_t));
                  if(erro != NO_ERROR) {
                    return erro;
                  }
                }
                else
                  if(strcmp(tok2,"expr") == 0) {
                    erro = ParseExpr(tok, tokenlist, token_index, linen);
                    if(erro != NO_ERROR) {
                      return erro;
                    }                  
                  }
                  else {
                    return INVALID_STRUCTURE;
                  }
          }
        }
        else {
          return INVALID_STRUCTURE;
        }

    tok = comp_NextToken(tokenlist, token_index, linen);
    count++;
  } 

  if(count == 0)  {
    return INVALID_STRUCTURE;
  }
  
  component = compLgraphConnect(component);

  return NO_ERROR;
}
