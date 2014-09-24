/**CHeaderFile*****************************************************************

  FileName    [netlInt.h]

  PackageName [netl]

  Synopsis    []

  Description []

  SeeAlso     []

  Author      []

  Copyright   []

  Revision    [$Id: $]

******************************************************************************/

#ifndef _NETLINT
#define _NETLINT

#include "sis.h"
#include "comp.h"
#include "arch.h"
#include "intf.h"
#include "netl.h"

/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/* Structure declarations                                                    */
/*---------------------------------------------------------------------------*/

/**Struct**********************************************************************

  Synopsis    [required]

  Description [optional]

  SeeAlso     [optional]

******************************************************************************/
struct netl_pin {
  char *label;
  netl_net_t *net;
  short skip;
};

/**Struct**********************************************************************

  Synopsis    [required]

  Description [optional]

  SeeAlso     [optional]

******************************************************************************/
struct netl_cb {
  array_t *mapin;		/* of char * */
  array_t *in;			/* of netl_pin_t * */
  array_t *mapout;		/* of char * */
  array_t *out;			/* of netl_pin_t * */
  char *comp;  
};

/**Struct**********************************************************************

  Synopsis    [required]

  Description [optional]

  SeeAlso     [optional]

******************************************************************************/
struct netl_clb {
  char *label;
  int x;
  int y;
  int z;
  int nio;
  netl_cb_t *cb;
  short noswap;
};

/**Struct**********************************************************************

  Synopsis    [required]

  Description [optional]

  SeeAlso     [optional]

******************************************************************************/
struct netl_clbl {
  char *name;
  array_t *inputs;
  array_t *outputs;
  array_t *clbs;
  array_t *globals;
};

/**Struct**********************************************************************

  Synopsis    [required]

  Description [optional]

  SeeAlso     [optional]

******************************************************************************/
struct netl_signet {
  int pin;
  short type;
  netl_clb_t *clb;
};

/**Struct**********************************************************************

  Synopsis    [required]

  Description [optional]

  SeeAlso     [optional]

******************************************************************************/
struct netl_net {
  char *name;
  float cost;
  float tempcost;
  int id;
  short global;
  array_t *snet;
};


/**AutomaticStart*************************************************************/

/*---------------------------------------------------------------------------*/
/* Function prototypes                                                       */
/*---------------------------------------------------------------------------*/

EXTERN void ClblPrint(netl_clbl_t *clbl);
EXTERN void NetlPrint(array_t *netl);

/**AutomaticEnd***************************************************************/

#endif /* _ */
