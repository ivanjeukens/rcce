/**CHeaderFile*****************************************************************

  FileName    [compInt.h]

  PackageName [comp]

  Synopsis    [This package defines several functions to manipulate the comp
  data structure.]

  Description [The comp data structure is used to describe an FPGA component,
  such as a clb, a routing switch etc. The component is described by a logic
  graph, a routing graph, and the io pins. It is possible to describe a
  component with both graph types, creating a mixed component. Additional
  information is also kept, such as a graphic view of the component and 
  timing data.]

  SeeAlso     [arch xv]

  Author      [Ivan Jeukens]

  Copyright   [1996-1997 Ivan Jeukens]

  Revision    [$Id: $]

******************************************************************************/

#ifndef _COMPINT
#define _COMPINT

#include "comp.h"
#include "util2.h"
#include "sis.h"

/*---------------------------------------------------------------------------*/
/* Constant declarations                                                     */
/*---------------------------------------------------------------------------*/

#define PIN_LENGHT 10
#define STEP 10

/* Code for identifying a specific action. Used by the compParser.c file. */
#define INSERT_ROUTING_PIN 100
#define INSERT_INPUT_LOGIC_PIN 101
#define INSERT_OUTPUT_LOGIC_PIN 102
#define INSERT_EAST 103
#define INSERT_WEST 104
#define INSERT_NORTH 105
#define INSERT_SOUTH 106
#define INSERT_UP 107
#define INSERT_DOWN 108

#define INSERT_EDGE 110
#define INSERT_MUX 111
#define INSERT_CLOCK_PIN 112
#define INSERT_LUT 113

#define INSERT_SHAREDI_PIN 117
#define INSERT_SHAREDO_PIN 118

#define INSERT_EXPR 120

/* Code for identifiying an error message. Used by the compParser.c file.*/
#define MISSING_PIN 200
#define MISSING_PIN_ADD_REDGE 201
#define WRONG_OUTPUT_ASSIGMENT 202
#define WRONG_INPUT_DECLARATION 203
#define NO_PIN 1
#define NO_LAYOUT 2
#define NO_STRUCTURE 3
#define NO_END 4
#define INVALID_PIN 5
#define INVALID_INPUT_LOGIC_PIN 6
#define INVALID_OUTPUT_LOGIC_PIN 7
#define INVALID_ROUTING_PIN 8
#define INVALID_LAYOUT 9
#define INVALID_EAST 10
#define INVALID_WEST 11
#define INVALID_NORTH 12
#define INVALID_SOUTH 13
#define INVALID_UP 14
#define INVALID_DOWN 15
#define INVALID_STRUCTURE 16
#define INVALID_EXPR 17
#define INVALID_ADD_EDGE 18
#define INVALID_MUX 19
#define INVALID_MEM 20
#define INVALID_LUT 21
#define PIN_NO_POS 22
#define WITH_POS 23
#define WRONG_MEM_COMPONENT 26
#define MULTIPLE_OUTPUT 27
#define INVALID_CLOCK_PIN 28
#define MULTIPLE_INPUT 29
#define INVALID_SHAREDI_PIN 30
#define INVALID_SHAREDO_PIN 31
#define MISSING_LAYOUT 32
#define WRONG_BUS_SIZE 33
#define AREA_INT 34
#define WRONG_PIN_DELAY 35
#define WRONG_PAREM_EXPR 36
#define WRONG_EXPR 37
#define NO_ERROR 0

/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/
typedef struct complnode complnode_t;
typedef struct compconstr compconstr_t;
typedef struct complpin complpin_t;
typedef struct compmemconstr compmemconstr_t;
typedef struct comprnode comprnode_t;
typedef struct comptoken comptoken_t;

/*---------------------------------------------------------------------------*/
/* Structure declarations                                                    */
/*---------------------------------------------------------------------------*/

/**Struct**********************************************************************

  Synopsis    [Represent a token.]

  Description [This structure describe a token.]

  SeeAlso     [comp.h]

******************************************************************************/
struct comptoken {
  char *text;
  int ln;
};

/**Struct**********************************************************************

  Synopsis    [Represent the constraints of a memory logic node pin.]

  Description [This structure describe the constraints of a memory logic 
  node pin.]

  SeeAlso     [comp.h]

******************************************************************************/
struct compmemconstr {
  char *clk_name;
  float setup;
  float hold;
};

/**Struct**********************************************************************

  Synopsis    [Represent a logic node pin.]

  Description [This structure describe a logic node pin.]

  SeeAlso     [comp.h]

******************************************************************************/
struct complpin {
  char *label;
  int type;
  compmemconstr_t *constr;
};

/**Struct**********************************************************************

  Synopsis    [Represent a logic primitive node.]

  Description [This structure describe a logic primitive node.]

  SeeAlso     [comp.h]

******************************************************************************/
struct complnode {
  int type;
  lsList pins;
  char *expr;
};


/**Struct**********************************************************************

  Synopsis    [Represent the data associated with a routing node.]

  Description [This structure describe the data associated with a routing node.]

  SeeAlso     []

******************************************************************************/
struct comprnode {
  char *label;
} comprnode;


/**Struct**********************************************************************

  Synopsis    [Represent a component pin.]

  Description [This structure describe a component pin.]

  SeeAlso     []

******************************************************************************/
struct comppin {
  char *label;
  int type;
  int pos;
  short prog;
};


/**Struct**********************************************************************

  Synopsis    [Represent a graphic description of a component pin.]

  Description [This structure stores the endpoints of a segment, with respect
  to an upper left origin, that describe a component pin. The numbering goes
  anti-clockwise, starting from the left(west) side. The first pin number 
  is 1.]

  SeeAlso     []

******************************************************************************/
struct compfigpin {
  int dx1;
  int dy1;
  int dx2;
  int dy2;
  int number;
};

/**Struct**********************************************************************

  Synopsis    [Represent a graphic description of a component.]

  Description [This structure contains the width and height of the rectangle
  representing a component, plus the graphic infomation about the component's
  pins.]

  SeeAlso     []

******************************************************************************/
struct compfig {
  int width;
  int height;
  array_t *pins;  
};


/**Struct**********************************************************************

  Synopsis    [Contain information regarding timing.]

  Description [Contain information about timing constraints, such as area,
  rise/fall times, etc.]

  SeeAlso     []

******************************************************************************/
struct compconstr {
  int area;
    
};


/**Struct**********************************************************************

  Synopsis    [Represent a generic FPGA component.]

  Description [Represent a generic FPGA component by it's pins, logic graph 
  and routing graph.]

  SeeAlso     []

******************************************************************************/
struct compdata {
  char *label;
  array_t *pins;
  graph_t *rg;
  graph_t *lg;
  char *text;
  comp_fig_t *fig;  
  compconstr_t *delay;
};


/*---------------------------------------------------------------------------*/
/* Variable declarations                                                     */
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/* Macro declarations                                                        */
/*---------------------------------------------------------------------------*/

/**Macro***********************************************************************

  Synopsis     [Add a component to a library]

  Description  [Insert the component into the beggining of a lsList structure
  used as a component library. Discard the returned list handle.]

  SideEffects  []

  SeeAlso      [list]

******************************************************************************/
#define comp_Ins(lib,c) (void) lsNewBegin(lib, (lsGeneric) c, LS_NH);

/**AutomaticStart*************************************************************/

/*---------------------------------------------------------------------------*/
/* Function prototypes                                                       */
/*---------------------------------------------------------------------------*/

EXTERN void compLgraphFree(graph_t *lg);
EXTERN lsList compLpinIns(lsList lst, char *label, int type);
EXTERN graph_t * compMconstrIns(graph_t *lg, char *label, char *out, char *clk, float value1, float value2);
EXTERN vertex_t * compLoutputFind(graph_t *lg, char *label);
EXTERN graph_t * compLnodeIns(graph_t *graph, complnode_t *node);
EXTERN void compLgraphPrint(graph_t *lg);
EXTERN comp_data_t * compLgraphConnect(comp_data_t *c);
EXTERN complpin_t * compLpinFind(lsList lst, char *label);
EXTERN char * compLgraph2Blif(comp_data_t *c);
EXTERN void compRgraphFree(graph_t *rg);
EXTERN void compRgraphPrint(graph_t *rg);
EXTERN comp_data_t * compRnodeConnect(comp_data_t *c, char *label1, char *label2);
EXTERN void compSigLFree(array_t *list);
EXTERN array_t * compSigIns(array_t *list, char *label);
EXTERN int compSigFind(array_t *list, char *label);
EXTERN comp_data_t * compAlloc(char *label);
EXTERN array_t * compPinIns(array_t *pins, char *label, int type);
EXTERN int compPinNpos(array_t *pins);
EXTERN int compPinSetpos(array_t *pins, char *label, int pos);
EXTERN int compPinSetprog(array_t *pins, char *label);
EXTERN comp_data_t * compFigCreate(comp_data_t *c);
EXTERN void compPrint(comp_data_t *c);

/**AutomaticEnd***************************************************************/

#endif /* _ */
