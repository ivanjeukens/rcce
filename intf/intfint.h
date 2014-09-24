/**CHeaderFile*****************************************************************

  FileName    [intfInt.h]

  PackageName [intf]

  Synopsis    []

  Description []

  SeeAlso     []

  Author      [Ivan Jeukens]

  Copyright   [1996-1997 Ivan Jeukens]

  Revision    [$Id: $]

******************************************************************************/

#ifndef _INTFINT
#define _INTFINT

#include "sis.h"
#include "comp.h"
#include "archInt.h"
#include "llib.h"
#include "netl.h"
#include "drawInt.h"
#include "dmig.h"
#include "dagm.h"
#include "levm.h"
#include "vprsa.h"
#include "intf.h"
#include "rou.h"

/*---------------------------------------------------------------------------*/
/* Constant declarations                                                     */
/*---------------------------------------------------------------------------*/

#define COMP_TEXT_KEY 1
#define COMP_EDIT_FRAME_KEY 2
#define PARSER_FRAME_KEY 3
#define FILENAME_TEXT_KEY 4
#define COMP_LIST_KEY 5
#define COMP_LABEL_KEY 6
#define AB_FRAME_KEY 7
#define AB_CANVAS_KEY 8
#define ARCH_BUILDER_LIST_KEY 9
#define AB_SHOW_KEY 10
#define COMP_EDIT_SHOW_KEY 11
#define COMP_EDIT_LISTPOS_KEY 12
#define LIB_FILENAME_KEY 13
#define LIB_LIST_KEY 14
#define LIB_TEXT_KEY 15
#define OPEN_LIB_MENUITEM_KEY 16
#define AB_ADDC_LIST_KEY 17
#define AB_ADD_BUTTON_KEY 18
#define CLOSE_LIB_MENUITEM_KEY 20
#define LAYER_TEXT_KEY 21
#define GRIDF_TEXT_KEY 22
#define NX_TEXT_KEY 23
#define NY_TEXT_KEY 24
#define GAPX_TEXT_KEY 25
#define GAPY_TEXT_KEY 26
#define EDIT_BUTTON_KEY 27
#define ZOOM_BUTTON_KEY 28
#define COMP_EDIT_FILE_FRAME_KEY 29
#define AB_FILE_FRAME_KEY 30
#define AB_PROPRIETIES_FRAME_KEY 31
#define AB_ADD_COMP_FRAME_KEY 32
#define SAVE_AB_MENUITEM_KEY 33
#define DISPLAY_COMP_INFO_FRAME_KEY 34
#define PDELAY_TEXT_KEY 35
#define PDELAY_FRAME_KEY 36
#define PDELAY_NL_KEY 37
#define PDELAY_X_KEY 38
#define PDELAY_Y_KEY 39
#define PDELAY_X1_KEY 40
#define PDELAY_Y1_KEY 41
#define GENBLOCK_BUTTON_KEY 42
#define GENBLOCK_FRAME_KEY 43
#define FILES_LIST_KEY 44
#define OPEN_FILES_TEXT_KEY 45
#define FILTER_TEXT_KEY 46
#define VIEW_COMP_FRAME_KEY 47
#define VIEW_COMP_KEY 48
#define EXEC_FRAME_KEY 49
#define DECOMPILE_BUTTON_KEY 50
#define BEFIG_LIST_KEY 51
#define NETWORK_KEY 52
#define EXEC_BUTTON_KEY 53
#define EXEC_TEXT_KEY 54
#define EXEC_LIST_KEY 55
#define EXEC_LISTPOS_KEY 56
#define DISPLAY_BUTTON_KEY 57
#define ARCH_BUILDER_FRAME_KEY 58
#define ARCH_BUILDER_FILE_FRAME_KEY 59
#define ABLIB_LIST_KEY 60
#define ABLIB_TEXT_KEY 61
#define ARCH_BUILDER_TEXT_KEY 62

#define SYNTHESIS_FUNC 100
#define MAPPING_FUNC 101
#define PLACEMENT_FUNC 102
#define ROUTING_FUNC 103

#define DISPLAY_NONE 0
#define DISPLAY_NETWORK 1
#define DISPLAY_CLBL 2

#define MAX_CANVAS_HEIGHT 2000
#define MAX_CANVAS_WIDTH 2000

#define COPY 1
#define MOVE 2
#define DELETE 3
#define ROTATE 4

#define COLOR_SIZE 10

#define MSG_FRAME_KEY 0
#define MSG_TEXT_KEY 1
#define ARGC_OPEN_FILES_KEY 2
#define ARGV_OPEN_FILES_KEY 3
#define CALLBACK_FUNCTION_KEY 4
#define DIRECTORIES_LIST_KEY 5
#define OPEN_FILES_FRAME_KEY 6

#define MAXBUFLEN 1024

#define ARROW_LENGTH 12
#define ARROW_ANGLE  (3.1415926 / 10)

/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/* Structure declarations                                                    */
/*---------------------------------------------------------------------------*/

/**Struct**********************************************************************

  Synopsis    [A command structure. Indicate what structures the command use.]

  Description [optional]

  SeeAlso     [optional]

******************************************************************************/
struct command {
  PFI func;
  char *pkg;
  short owner;
};

/*---------------------------------------------------------------------------*/
/* Variable declarations                                                     */
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/* Macro declarations                                                        */
/*---------------------------------------------------------------------------*/

/**Macro***********************************************************************

  Synopsis     [Absolute value of a number.]

  Description  [optional]

  SideEffects  [required]

  SeeAlso      [optional]

******************************************************************************/
#define ABS(a) ((a) < 0 ? -(a) : (a)) 

/**AutomaticStart*************************************************************/

/*---------------------------------------------------------------------------*/
/* Function prototypes                                                       */
/*---------------------------------------------------------------------------*/

EXTERN void intfAbResetGlobals();
EXTERN void intfAbDisplay(Frame item, Event *event);
EXTERN void intfArchResetGlobals();
EXTERN void intfDisplayArch(Frame item, Event *event);
EXTERN int show_window_frame_done(Frame subframe);
EXTERN void intfBatch(char *fname);
EXTERN void intfCommandsInit();
EXTERN void intfCommandsEnd();
EXTERN void intfExecProc(Panel_item item, Event *event);
EXTERN char * intfClearCmd(char *cmd);
EXTERN void intfCompeResetGlobals();
EXTERN void intfCompeDisplay(Frame item, Event *event);
EXTERN void main(int argc, char *argv[]);
EXTERN void intfMainResetGlobals(void);
EXTERN void intfDisplayMsgbox(Frame father_frame, char *msg);
EXTERN void intfOpenFiles(Frame father_frame, char *label, char *filter, short *show_flag, PFI callback, int argc, char **argv);

/**AutomaticEnd***************************************************************/

#endif /* _ */
