/**CHeaderFile*****************************************************************

  FileName    [archInt.h]

  PackageName [arch]

  Synopsis    [Internal structures of the FPGA.]

  Description []

  SeeAlso     [comp]

  Author      [Ivan Jeukens]

  Copyright   []

  Revision    [$Id: $]

******************************************************************************/

#ifndef _ARCHINT
#define _ARCHINT

#include <stdlib.h>
#include "sis.h"

#include "util2.h"
#include "comp.h"
#include "arch.h"
#include "intf.h"
#include "llib.h"

/*---------------------------------------------------------------------------*/
/* Constant declarations                                                     */
/*---------------------------------------------------------------------------*/

#define ARCHNO_ERROR 0

#define ARCHCOMP 0
#define ARCHSEG 1

#define ARCHNO_TEXT 1
#define ARCHNO_USE 2
#define ARCHINVALID_USE 3
#define ARCHINVALID_COMPONENT 4
#define ARCHWRONG_PARAM_NUMBER 5
#define ARCHCOORD_NOT_NUMBER 6
#define ARCHCOMP_SAME_POS 7
#define ARCHWRONG_IO 8
#define ARCHMISPLACED_BUS 9
#define ARCHBAD_BUS 10
#define ARCHWRONG_BUS_SIZE 11
#define ARCHNO_ARCHLABEL 12
#define ARCHBUS_REDECLARED 13
#define ARCHPARSE_ERROR 14
#define ARCHWRONG_NUMERIC_PARAM 15
#define ARCHWRONG_BUS_PARAM 16
#define ARCHNOT_BUS 17
#define ARCHMISSING_EQU 18
#define ARCHMISMATCH_BUS_WIDTH 19
#define ARCHMISSING_PV 20
#define ARCHWRONG_EAST_SIZE 21
#define ARCHWRONG_WEST_SIZE 22
#define ARCHWRONG_NORTH_SIZE 23
#define ARCHWRONG_SOUTH_SIZE 24
#define ARCHWRONG_UP_SIZE 25
#define ARCHWRONG_DOWN_SIZE 26
#define ARCHWRONG_SIGNAL_SIZE 27
#define ARCHSAME_BUS_NAME 28
#define ARCHMISSING_OPAR 29
#define ARCHINVALID_VAR_ID 30
#define ARCHMISSING_VIR 31
#define ARCHNOT_INTEGER 32
#define ARCHINVALID_RELAT_OPER 33
#define ARCHINVALID_ASSIG_OPER 34
#define ARCHMISSING_CCB 35
#define ARCHMISSING_OCB 36
#define ARCHINCOMPLETE_VAR 37
#define ARCHMISSING_BEGIN 38
#define ARCHMISSING_CPAR 39
#define ARCHMISSING_COORD 40
#define ARCHNO_PORT 41
#define ARCHRESERVED_PORT 42
#define ARCHINVALID_PORT 43
#define ARCHEMPTY_PORT 44
#define ARCHINVALIDTYPE_PORT 45
#define ARCHRESERVED_SIGNAL 46

#define ARCHGT 1
#define ARCHLT 2
#define ARCHGE 3
#define ARCHLE 4
#define ARCHDIF 5

#define ARCHADD 1
#define ARCHSUB 2
#define ARCHMULT 3
#define ARCHDIV 4


/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/

typedef struct archloopvar archloopvar_t;
typedef struct archsignal archsignal_t;
typedef struct archcomp archcomp_t;

/*---------------------------------------------------------------------------*/
/* Structure declarations                                                    */
/*---------------------------------------------------------------------------*/

/**Struct**********************************************************************

  Synopsis    [An architecture block segment.]

  Description []

  SeeAlso     []

******************************************************************************/
struct abseg {
  int x;
  int y;
  int x2;
  int y2;  
};

/**Struct**********************************************************************

  Synopsis    [An architecture block component. The comp_fig_t is copyed 
  exactly from the comp_data_t structure. The io field indicate whether the
  component is an io pad or not, an the pos field is used when rotating
  the component.]

  Description []

  SeeAlso     []

******************************************************************************/
struct abcomp {
  int x;
  int y;
  comp_fig_t *fig;
  char *label;
  char *library;
  short io;
  short pos;  
};

/**Struct**********************************************************************

  Synopsis    [The architecture block.]

  Description []

  SeeAlso     []

******************************************************************************/
struct block {
  char *label;
  int ncomp;
  lsList comp;
  int nseg;
  lsList seg;  
};


/**Struct**********************************************************************

  Synopsis    []

  Description []

  SeeAlso     []

******************************************************************************/
struct archsignal {
  short port;
  array_t *comps;
};

/**Struct**********************************************************************

  Synopsis    []

  Description []

  SeeAlso     []

******************************************************************************/
struct archcomp {
  short io;
  short logic;
  char *libname;
  int x;
  int y;
  int z;
  array_t *sigs;
};  

/**Struct**********************************************************************

  Synopsis    []

  Description []

  SeeAlso     []

******************************************************************************/
struct archloopvar {
  char *label;
  int from;
  short relat;
  int to;
  short assig;
  int step;
};

/**Struct**********************************************************************

  Synopsis    []

  Description []

  SeeAlso     []

******************************************************************************/
struct archclabel {
  char *label;
  int lib;
};

/**Struct**********************************************************************

  Synopsis    []

  Description []

  SeeAlso     []

******************************************************************************/
struct arch_edge {
  int pin;
  int pinpos;
  int type;  
  int adjpin;
  int adjpinpos;
  int adjtype;
  short connect;
};

/**Struct**********************************************************************

  Synopsis    []

  Description []

  SeeAlso     []

******************************************************************************/
struct arch_node {
  int label;
  int x;
  int y;
  int z;
  short io;  
  short logic;
};

/**Struct**********************************************************************

  Synopsis    []

  Description []

  SeeAlso     []

******************************************************************************/
struct arch_graph {
  char *label;
  graph_t *ag;
  array_t *labels;
  array_t *libs;
  st_table *nodes;
};

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

EXTERN void archBlockPrint(arch_block_t *ab);
EXTERN void archEdgeIns(arch_graph_t *g, arch_node_t *n1, arch_node_t *n2, int pin, int pinpos, int pintype, int adjpin, int adjpinpos, int adjtype);
EXTERN arch_graph_t * archNodeIns(arch_graph_t *g, char *label, char *lib, int x, int y, int z, short io, short logic);
EXTERN arch_graph_t * archAlloc(char *label);
EXTERN void archPrint(arch_graph_t *g);

/**AutomaticEnd***************************************************************/

#endif /* _ */
