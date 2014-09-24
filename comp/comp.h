/**CHeaderFile*****************************************************************

  FileName    [comp.h]

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

#ifndef _COMP
#define _COMP

#include "ansi.h"
#include "array.h"
#include "list.h"

/*---------------------------------------------------------------------------*/
/* Constant declarations                                                     */
/*---------------------------------------------------------------------------*/

/* Possible component pin type */
#define COMP_ROUTING 1
#define COMP_LINPUT 2
#define COMP_LOUTPUT 3
#define COMP_CLK 4
#define COMP_SHAREDI 5
#define COMP_SHAREDO 6
/* Used for representing internal conectivity of the logic primitives */
#define COMP_ISIGNAL 7
#define COMP_OSIGNAL 8

/* Possible pin layout */
#define COMP_WEST 50
#define COMP_SOUTH 51
#define COMP_EAST 52
#define COMP_NORTH 53
#define COMP_UP 54
#define COMP_DOWN 55

/* Possible logic primitives */
#define COMP_LLOW 1
#define COMP_LHIGH 2
#define COMP_DFFF 3
#define COMP_DFFR 4
#define COMP_LASYN 5
#define COMP_MUX 6
#define COMP_LUT 7
#define COMP_EXPR 8

/* Possible types of a component.*/
#define COMP_RC 1
#define COMP_LC 2
#define COMP_RLC 3

/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/

typedef struct compdata comp_data_t;
typedef struct comppin comp_pin_t;
typedef struct comptoken comp_token_t;
typedef struct compfigpin comp_figpin_t;
typedef struct compfig comp_fig_t;

/*---------------------------------------------------------------------------*/
/* Structure declarations                                                    */
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/* Variable declarations                                                     */
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/* Macro declarations                                                        */
/*---------------------------------------------------------------------------*/


/**AutomaticStart*************************************************************/

/*---------------------------------------------------------------------------*/
/* Function prototypes                                                       */
/*---------------------------------------------------------------------------*/

EXTERN comp_data_t * comp_LprimIns(comp_data_t *c, int type, lsList list, char *expr);
EXTERN void comp_TokenFree(array_t *tokens);
EXTERN array_t * comp_TokenExtr(char *data);
EXTERN char * comp_NextToken(array_t *list, int *index, int *ln);
EXTERN void comp_TokenPrint(array_t *list);
EXTERN comp_data_t * comp_Parse(char *data, int *linen, int *er);
EXTERN char * comp_Error(int code);
EXTERN void comp_FpinFree(array_t *pins);
EXTERN void comp_Free(comp_data_t *c);
EXTERN void comp_LibFree(lsList lib);
EXTERN void comp_LibSave(lsList lib, char *filename);
EXTERN comp_data_t * comp_Find(lsList lib, char *label);
EXTERN lsList comp_Delete(lsList lib, char *label);
EXTERN array_t * comp_FpinIns(array_t *pins, int dx1, int dx2, int dy1, int dy2, int n);
EXTERN comp_data_t * comp_FigCopy(comp_data_t *c);
EXTERN short comp_Type(comp_data_t *c);
EXTERN lsList comp_LibLoad(char *filename);
EXTERN char * comp_Check(comp_data_t *c);
EXTERN char * comp_BlifExtr(comp_data_t *c);
EXTERN int comp_PinFind(array_t *pins, char *label);

/**AutomaticEnd***************************************************************/

#endif /* _ */
