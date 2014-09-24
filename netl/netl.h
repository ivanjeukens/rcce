/**CHeaderFile*****************************************************************

  FileName    [netl.h]

  PackageName [netl]

  Synopsis    [A netlist package.]

  Description []

  SeeAlso     []

  Author      []

  Copyright   []

  Revision    [$Id: $]

******************************************************************************/

#ifndef _NETL
#define _NETL

/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/

typedef struct netl_pin netl_pin_t;
typedef struct netl_cb netl_cb_t;
typedef struct netl_clb netl_clb_t;
typedef struct netl_clbl netl_clbl_t;
typedef struct netl_signet netl_signet_t;
typedef struct netl_net netl_net_t;

/*---------------------------------------------------------------------------*/
/* Structure declarations                                                    */
/*---------------------------------------------------------------------------*/


/**AutomaticStart*************************************************************/

/*---------------------------------------------------------------------------*/
/* Function prototypes                                                       */
/*---------------------------------------------------------------------------*/

EXTERN void netl_ClblFree(netl_clbl_t *clbl);
EXTERN void netl_Free(array_t *netl);
EXTERN int netl_ClblFindLabel(array_t *clbs, char *label);
EXTERN netl_clbl_t * netl_ClblCopy(netl_clbl_t *clbl);
EXTERN char * netl_ClblCreate(network_t *net);
EXTERN void netl_Init();
EXTERN void netl_End();
EXTERN int netl_ClblLoadCmd(netl_clbl_t **clbl, int argc, char **argv);
EXTERN int netl_VprNetLoadCmd(netl_clbl_t **clbl, int argc, char **argv);
EXTERN int netl_VprNetWriteCmd(network_t **network, int argc, char **argv);
EXTERN int netl_ClblLoad(netl_clbl_t **clbl, char *fname);
EXTERN void netl_ClblSave(netl_clbl_t *clbl, char *fname);
EXTERN void netl_VprNetWrite(network_t *net, char *fname, int kl);
EXTERN array_t * netl_Extr(netl_clbl_t *clbl);
EXTERN void netl_UpdateId(array_t *netl);
EXTERN int netl_Find(array_t *netl, char *label);
EXTERN void netl_VprPlaceWrite(netl_clbl_t *clbl, int nx, int ny, char *fname);
EXTERN int netl_VprNetLoad(netl_clbl_t **clbl, char *fname);

/**AutomaticEnd***************************************************************/

#endif /* _ */

