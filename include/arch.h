/**CHeaderFile*****************************************************************

  FileName    [arch.h]

  PackageName [arch]

  Synopsis    [Used in describing the architecture of an FPGA device.]

  Description []

  SeeAlso     [comp]

  Author      [Ivan Jeukens]

  Copyright   []

  Revision    [$Id: $]

******************************************************************************/

#ifndef _ARCH
#define _ARCH

/*---------------------------------------------------------------------------*/
/* Constant declarations                                                     */
/*---------------------------------------------------------------------------*/

#define ARCH_NO_ERROR 0
#define ARCH_NO_FILE 1
#define ARCH_NO_ARCH_BLOCK 2
#define ARCH_FERROR 3
#define ARCH_NO_LABEL 4
#define ARCH_NO_LIB 5

/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/

typedef struct arch_graph arch_graph_t;
typedef struct arch_node arch_node_t;
typedef struct arch_edge arch_edge_t;
typedef struct block arch_block_t;
typedef struct abcomp arch_abcomp_t;
typedef struct abseg arch_abseg_t;
typedef struct archclabel arch_clabel_t;

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

EXTERN int arch_BlockGen(arch_block_t *ab);
EXTERN void arch_GenError(int error, char *message);
EXTERN void arch_BlockFree(arch_block_t *a);
EXTERN arch_block_t * arch_BlockAlloc(char *label);
EXTERN arch_block_t * arch_AbcompIns(arch_block_t *ab, comp_fig_t *cfig, char *label, char *lib, int x, int y, short io, short pos);
EXTERN arch_block_t * arch_AbsegIns(arch_block_t *ab, int x, int y, int x2, int y2);
EXTERN arch_block_t * arch_BlockZoom(arch_block_t *ab, short f, int px, int py);
EXTERN arch_block_t * arch_BlockRm(arch_block_t *ab, int x1, int y1, int x2, int y2);
EXTERN arch_block_t * arch_BlockRotate(arch_block_t *ab, int x1, int y1, int x2, int y2);
EXTERN arch_block_t * arch_BlockCopy(arch_block_t *ab, int x1, int y1, int x2, int y2, int xd, int yd, int zf);
EXTERN arch_block_t * arch_BlockMove(arch_block_t *ab, int x1, int y1, int x2, int y2, int xd, int yd, int zf);
EXTERN short arch_BlockSave(arch_block_t *ab, char *filename);
EXTERN arch_block_t * arch_BlockLoad(char *filename);
EXTERN void arch_Free(arch_graph_t *ag);
EXTERN void arch_Init();
EXTERN void arch_End();
EXTERN int arch_ReadCmd(array_t **llib, arch_graph_t **ag, int argc, char **argv);
EXTERN arch_graph_t * arch_Read(char *fname, array_t **llib);
EXTERN vertex_t * arch_NodeFind(arch_graph_t *g, int x, int y, int z);
EXTERN void arch_Dump(arch_graph_t *ag);
EXTERN char * arch_Check(arch_graph_t *ag);
EXTERN char * arch_Ecode(int code);
EXTERN arch_graph_t * arch_Parse(char *data, int *linen, int *error, char **war);

/**AutomaticEnd***************************************************************/

#endif /* _ */
