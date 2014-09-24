/**CHeaderFile*****************************************************************

  FileName    [intf.h]

  PackageName [intf]

  Synopsis    []

  Description []

  SeeAlso     []

  Author      [Ivan Jeukens]

  Copyright   [1996-1997 Ivan Jeukens]

  Revision    [$Id: $]

******************************************************************************/

#ifndef _INTF
#define _INTF

#include "netl.h"
#include "comp.h"
#include "arch.h"

/*---------------------------------------------------------------------------*/
/* Constant declarations                                                     */
/*---------------------------------------------------------------------------*/

#define WHITE 0 + CMS_CONTROL_COLORS
#define RED 1 + CMS_CONTROL_COLORS
#define GREEN 2 + CMS_CONTROL_COLORS
#define BLUE 3 + CMS_CONTROL_COLORS
#define ORANGE 4 + CMS_CONTROL_COLORS
#define AQUA 5 + CMS_CONTROL_COLORS
#define PINK 6 + CMS_CONTROL_COLORS
#define BLACK 7 + CMS_CONTROL_COLORS
#define GRAY 8 + CMS_CONTROL_COLORS
#define GRAY2 9 + CMS_CONTROL_COLORS

#define INTF_FROM_SIS 1
#define INTF_FROM_MAP 2
#define INTF_FROM_PLAC 3
#define INTF_GENERIC 4
#define INTF_FROM_ARCH 5
#define INTF_FROM_NETL 6
#define INTF_FROM_ROUTING 8

/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/

typedef int (*PFI)();
typedef struct command command_t;

/*---------------------------------------------------------------------------*/
/* Structure declarations                                                    */
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/* Variable declarations                                                     */
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/* Macro declarations                                                        */
/*---------------------------------------------------------------------------*/

/**Macro***********************************************************************

  Synopsis     [Add a sis type command. Only use the network_t structure.]

  Description  [optional]

  SideEffects  [required]

  SeeAlso      [optional]

******************************************************************************/
#define COM_ADD_COMMAND_SIS(name, func, pkg) \
  (intf_ComAdd((name), (func), (pkg), INTF_FROM_SIS))

/**Macro***********************************************************************

  Synopsis     [Add a mapping based command. Use the network and logic library
  structures.]

  Description  [optional]

  SideEffects  [required]

  SeeAlso      [optional]

******************************************************************************/
#define COM_ADD_COMMAND_MAP(name, func) \
  (intf_ComAdd((name), (func), NIL(char), INTF_FROM_MAP))

/**Macro***********************************************************************

  Synopsis     [A placement type command.]

  Description  [optional]

  SideEffects  [required]

  SeeAlso      [optional]

******************************************************************************/
#define COM_ADD_COMMAND_PLAC(name, func) \
  (intf_ComAdd((name), (func), NIL(char), INTF_FROM_PLAC))

/**Macro***********************************************************************

  Synopsis     [A routing type command.]

  Description  [optional]

  SideEffects  [required]

  SeeAlso      [optional]

******************************************************************************/
#define COM_ADD_COMMAND_ROU(name, func) \
  (intf_ComAdd((name), (func), NIL(char), INTF_FROM_ROUTING))

/**Macro***********************************************************************

  Synopsis     [Add the commands from the arch package.]

  Description  [optional]

  SideEffects  [required]

  SeeAlso      [optional]

******************************************************************************/
#define COM_ADD_COMMAND_ARCH(name, func) \
  (intf_ComAdd((name), (func), NIL(char), INTF_FROM_ARCH))

/**Macro***********************************************************************

  Synopsis     [Add the commands from the netl package.]

  Description  [optional]

  SideEffects  [required]

  SeeAlso      [optional]

******************************************************************************/
#define COM_ADD_COMMAND_NETL(name, func) \
  (intf_ComAdd((name), (func), NIL(char), INTF_FROM_NETL))
/**AutomaticStart*************************************************************/

/*---------------------------------------------------------------------------*/
/* Function prototypes                                                       */
/*---------------------------------------------------------------------------*/

EXTERN void intf_ComAdd(char *name, PFI func, char *pkg, short owner);
EXTERN int intf_ComSis(network_t **net, char *cmd);
EXTERN int intf_Com(char *cmd, network_t **net, array_t **llib, netl_clbl_t **clbl, arch_graph_t **ag, short b);

/**AutomaticEnd***************************************************************/

#endif /* _ */
