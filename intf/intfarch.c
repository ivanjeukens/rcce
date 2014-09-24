/**CFile***********************************************************************

  FileName    [intfArch.c]

  PackageName [intf]

  Synopsis    [Interface for creating an architecture.]

  Description []

  SeeAlso     []

  Author      []

  Copyright   []

******************************************************************************/
#include <xview/xview.h>
#include <xview/frame.h>
#include <xview/canvas.h>
#include <xview/xv_xrect.h>
#include <xview/panel.h>
#include <xview/openmenu.h>
#include <xview/scrollbar.h>
#include <xview/cms.h>
#include <xview/textsw.h>

#undef va_start

#include "intfInt.h"

/*
static char rcsid[] = \"$Id: $\";
USE(rcsid);
*/

/*---------------------------------------------------------------------------*/
/* Variable declarations                                                     */
/*---------------------------------------------------------------------------*/
extern Frame main_frame;

short buildshow = 1;
short abfshow = 1;
int abflistpos = 0;
lsList arch_builder_lib;
int arch_builder_listpos;
draw_t *arch_builder_screen = NIL(draw_t);

unsigned long *disp_colors;

extern arch_graph_t *main_arch;

/*---------------------------------------------------------------------------*/
/* Macro declarations                                                        */
/*---------------------------------------------------------------------------*/


/**AutomaticStart*************************************************************/

/*---------------------------------------------------------------------------*/
/* Static function prototypes                                                */
/*---------------------------------------------------------------------------*/

static int compile_arch(Panel_item item, Event *event);
static int callback_sarch(char *fname, int argc, char **argv);
static int callback_oarch(char *fname, int argc, char **argv);
static int callback_olib(char *fname, int argc, char **argv);
static void arch_builder_menu(Menu menu, Menu_item menu_item);
static int arch_builder_frame_done(Frame subframe);
static void archbuilder_list_proc(Panel_item item, char *string, caddr_t client_data, Panel_list_op op, Event *event);

/**AutomaticEnd***************************************************************/


/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis           [Free and reset global variables.]

  Description        [optional]

  SideEffects        [required]

  SeeAlso            [optional]

******************************************************************************/
void
intfArchResetGlobals()
{
  intfAbResetGlobals();
  intfCompeResetGlobals();
  abfshow = 1;
  abflistpos = 0;
  buildshow = 1;
  if(arch_builder_lib != NULL) {
    comp_LibFree(arch_builder_lib);
  }
  if(arch_builder_screen != NIL(draw_t)) {
    draw_Free(arch_builder_screen);
  }
  arch_builder_lib = NULL;  
}

/**Function********************************************************************

  Synopsis           []

  Description        [optional]

  SideEffects        [required]

  SeeAlso            [optional]

******************************************************************************/
void
intfDisplayArch(
  Frame item,
  Event *event)
{
Menu file_menu;
Frame arch_builder_frame;
Panel arch_builder_panel;
Panel_item arch_builder_list;
Textsw arch_builder_text;

  if(buildshow == 0) return;
  buildshow = 0;

  arch_builder_lib = NULL;
  arch_builder_listpos = 0;  

  arch_builder_frame = (Frame) xv_create(main_frame, FRAME,
    FRAME_LABEL, "Architecture Builder",
    XV_WIDTH, 550,
    XV_HEIGHT, 650,
    XV_SHOW, TRUE,
    FRAME_DONE_PROC, arch_builder_frame_done,
    NULL);    
   
  arch_builder_panel = (Panel) xv_create(arch_builder_frame, PANEL,
    XV_HEIGHT, 120,
    XV_WIDTH, 550,
    NULL);    
    
  arch_builder_text = (Textsw) xv_create(arch_builder_frame, TEXTSW,
     XV_X, 2,
     XV_Y, 120,
     NULL);

  file_menu = (Menu) xv_create((int) NULL, MENU,
    MENU_NOTIFY_PROC, arch_builder_menu,
    MENU_STRINGS, "Open Arch", "Save Arch", "Open Lib", "Quit", NULL,
    XV_KEY_DATA, ARCH_BUILDER_FRAME_KEY, arch_builder_frame,
    XV_KEY_DATA, ARCH_BUILDER_TEXT_KEY, arch_builder_text,
    NULL);

  (void) xv_create(arch_builder_panel, PANEL_BUTTON,
    PANEL_LABEL_STRING, "File",
    PANEL_ITEM_MENU, file_menu,
    NULL);
    
  (void) xv_create(arch_builder_panel, PANEL_BUTTON,
    PANEL_LABEL_STRING, "Compile",
    PANEL_NOTIFY_PROC, compile_arch,
    XV_KEY_DATA, ARCH_BUILDER_FRAME_KEY, arch_builder_frame,
    XV_KEY_DATA, ARCH_BUILDER_TEXT_KEY, arch_builder_text,
    NULL);

  (void) xv_create(arch_builder_panel, PANEL_BUTTON,
    PANEL_LABEL_STRING, "Arch Edt",
    PANEL_NOTIFY_PROC, intfAbDisplay,
    XV_KEY_DATA, ARCH_BUILDER_FRAME_KEY, arch_builder_frame,
    NULL);
    
  (void) xv_create(arch_builder_panel, PANEL_BUTTON,
    PANEL_LABEL_STRING, "Comp Edit",
    PANEL_NOTIFY_PROC, intfCompeDisplay,
    XV_KEY_DATA, ARCH_BUILDER_FRAME_KEY, arch_builder_frame,
    NULL);
    
  arch_builder_list = (Panel_item) xv_create(arch_builder_panel, PANEL_LIST,
    XV_Y, 30,
    XV_X, 5,
    PANEL_LIST_WIDTH, 520,
    PANEL_LIST_DISPLAY_ROWS, 3,
    PANEL_LIST_ROW_HEIGHT, 20,
    PANEL_NOTIFY_PROC, archbuilder_list_proc,
    XV_KEY_DATA, ARCH_BUILDER_TEXT_KEY, arch_builder_text,
    NULL);

  (void) xv_set(file_menu, XV_KEY_DATA, ARCH_BUILDER_LIST_KEY,
    arch_builder_list,  NULL);         
}

/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis           [Handle the compile button.]

  Description        [optional]

  SideEffects        [required]

  SeeAlso            [optional]

******************************************************************************/
static int
compile_arch(
  Panel_item item,
  Event *event)
{
Frame frame = (Frame) xv_get(item, XV_KEY_DATA, ARCH_BUILDER_FRAME_KEY);
Textsw arch_builder_text = (Textsw) xv_get(item, XV_KEY_DATA, 
  ARCH_BUILDER_TEXT_KEY);
char *text_buffer, *er, *msg, *war = NIL(char);
int len,linen;

  len = (int) xv_get(arch_builder_text,TEXTSW_LENGTH, NULL);
  text_buffer = (char *) ALLOC(char, len + 1);
  (void) xv_get(arch_builder_text, TEXTSW_CONTENTS, 0, text_buffer, len);   
  text_buffer[len] = '\0';

  if(main_arch != NIL(arch_graph_t)) {
    arch_Free(main_arch);
  }
  
  main_arch = arch_Parse(text_buffer, &linen, &len, &war);
  
  if(len != NO_ERROR) {
    msg = arch_Ecode(len);
    er = (char *) ALLOC(char, strlen(msg) + 10);
    sprintf(er,"%s at line %d.",msg,linen);
    intfDisplayMsgbox(frame, er);
    FREE(er);
    FREE(msg);
  }
  else {
    msg = arch_Check(main_arch);
    if(msg != NIL(char)) {
      if(war != NIL(char)) {
        msg = (char *) REALLOC(char, msg, strlen(msg) + strlen(war) + 1);
        strcat(msg, war);
        FREE(war);

      }
      intfDisplayMsgbox(frame, msg);
    }
    else
      if(war != NIL(char)) {
        intfDisplayMsgbox(frame, war);
        FREE(war);    
      }
      else {
        msg = util_strsav("No errors found.\n");
        intfDisplayMsgbox(frame, msg);
      }
    FREE(msg);    
  }

  if(text_buffer != NULL) {
    FREE(text_buffer);
  }

  return XV_OK;
}

/**Function********************************************************************

  Synopsis           [Callback procedure for the save arch command.]

  Description        [optional]

  SideEffects        [required]

  SeeAlso            [optional]

******************************************************************************/
static int
callback_sarch(
  char *fname,
  int argc,
  char **argv)
{
Textsw arch_builder_text = (Textsw) argv[0];
unsigned int ret;
char msg[80];

  if((fname != NIL(char)) && (strcmp(fname,"") != 0)) {
    ret = textsw_store_file(arch_builder_text, fname, 0, 0);
    if(ret != 0) {
      sprintf(msg, "Error while saving file %s.", fname);
      intfDisplayMsgbox(main_frame, msg);
    }
  }
  FREE(argv);
  return 1;
}

/**Function********************************************************************

  Synopsis           [Callback procedure for the open arch command.]

  Description        [optional]

  SideEffects        [required]

  SeeAlso            [optional]

******************************************************************************/
static int
callback_oarch(
  char *fname,
  int argc,
  char **argv)
{
Textsw arch_builder_text = (Textsw) argv[0];

  if((fname != NIL(char)) && (strcmp(fname,"") != 0)) {
    (void) xv_set(arch_builder_text, TEXTSW_FILE, fname, TEXTSW_FIRST, 0,
      NULL);
  }
  FREE(argv);  
  return 1;      
}

/**Function********************************************************************

  Synopsis           [Callback procedure for the open lib command.]

  Description        [optional]

  SideEffects        [required]

  SeeAlso            [optional]

******************************************************************************/
static int
callback_olib(
  char *fname,
  int argc,
  char **argv)
{
Panel_item list = (Panel_item) argv[0];
Frame frame = (Frame) argv[1];
char message[248], temp[10];
int type, error = NO_ERROR, linen = 0;
lsGen gen;
comp_data_t *c;

  if(fname != NIL(char)) {
    if(arch_builder_lib != NULL) {
      comp_LibFree(arch_builder_lib);
    }
    arch_builder_lib = comp_LibLoad(fname);
    if(arch_builder_lib != NULL) {
      arch_builder_listpos--;
      while(arch_builder_listpos >= 0) {
        (void) xv_set(list, PANEL_LIST_DELETE, arch_builder_listpos, NULL);
        arch_builder_listpos--;
      }
      arch_builder_listpos++; 
      lsForeachItem(arch_builder_lib, gen, c) {
        strcpy(message,"Component -> ");
        strcat(message,c->label);
        type = comp_Type(c);
        if(type == 3) {
          strcat(message,"    --    routing/logic");
        }
        else
          if(type == 1) {
            strcat(message,"    --    routing");
          }
          else
            if(type == 2) {
              strcat(message,"    --    logic");     
            }
        (void) xv_set(list, PANEL_LIST_INSERT, arch_builder_listpos, 
           PANEL_LIST_STRING, arch_builder_listpos, message, NULL);       
        arch_builder_listpos++;
      }    
    }
    else {
      strcpy(message, comp_Error(error));
      strcat(message," at line");
      sprintf(temp," %d",linen);
      strcat(message,temp);
      intfDisplayMsgbox(frame, message);
    }
  }
  FREE(argv);

  return 1;
}

/**Function********************************************************************

  Synopsis           [Arch menu function.]

  Description        [optional]

  SideEffects        [required]

  SeeAlso            [optional]

******************************************************************************/
static void
arch_builder_menu(
  Menu menu,
  Menu_item menu_item)
{
Frame arch_builder_frame = (Frame) xv_get(menu, XV_KEY_DATA, ARCH_BUILDER_FRAME_KEY);
Textsw arch_builder_text = (Textsw) xv_get(menu, XV_KEY_DATA,
  ARCH_BUILDER_TEXT_KEY);
Panel_item list = (Panel_item) xv_get(menu, XV_KEY_DATA, ARCH_BUILDER_LIST_KEY);
char **argv;
  
  if(strcmp((char *) xv_get(menu_item,MENU_STRING),"Quit") == 0) {
/*    compe_reset_globals();
    ab_reset_globals();*/
    intfArchResetGlobals();
    xv_destroy_safe(arch_builder_frame);
  }
  else
    if(strcmp((char *) xv_get(menu_item,MENU_STRING),"Open Arch") == 0) {
      if(abfshow == 0) return;
      abfshow = 0;
      argv = (char **) ALLOC(char *, 1);
      argv[0] = (char *) arch_builder_text;
      intfOpenFiles(arch_builder_frame, "Open Architecture", "*.arch",
        &abfshow, callback_oarch, 1, argv);
    }
    else
      if(strcmp((char *) xv_get(menu_item,MENU_STRING),"Save Arch") == 0) {
        if(abfshow == 0) return;
        abfshow = 0;
        argv = (char **) ALLOC(char *, 1);
        argv[0] = (char *) arch_builder_text;
        intfOpenFiles(arch_builder_frame, "Save Architecture", "*.arch",
          &abfshow, callback_sarch, 1, argv);
      }
      else
        if(strcmp((char *) xv_get(menu_item, MENU_STRING), "Open Lib") == 0) {
          if(abfshow == 0) return;
          abfshow = 0;
          argv = (char **) ALLOC(char *, 2);
          argv[0] = (char *) list;
          argv[1] = (char *) arch_builder_frame;
          intfOpenFiles(arch_builder_frame, "Open Component Library", "*.clib", 
            &abfshow, callback_olib, 1, argv);       
        
        }
}

/**Function********************************************************************

  Synopsis           [Exit function.]

  Description        [optional]

  SideEffects        [required]

  SeeAlso            [optional]

******************************************************************************/
static int
arch_builder_frame_done(Frame subframe)
{
/*  compe_reset_globals();
  ab_reset_globals();*/
  intfArchResetGlobals();
  xv_destroy_safe(subframe);
  return XV_OK;
}

/**Function********************************************************************

  Synopsis           [Handle a double click on the show list.]

  Description        [optional]

  SideEffects        [required]

  SeeAlso            [optional]

******************************************************************************/
static void
archbuilder_list_proc(
  Panel_item item,
  char *string,
  caddr_t client_data,
  Panel_list_op op,
  Event *event)
{
Textsw atext = (Textsw) xv_get(item, XV_KEY_DATA, ARCH_BUILDER_TEXT_KEY);
static int i = 0, lastj = -1;
double t;
int j = 0;
char *pchar, tmp[200];
comp_data_t *c;
comp_pin_t *cp;

  if(event_is_down(event) == 0) return;
  
  if(op == PANEL_LIST_OP_DESELECT) {
    i = 0;
    return;
  }  
  while(xv_get(item, PANEL_LIST_SELECTED, j) == 0) j++;  
  
  if(i == 0) {
    util2_StartTimer();
    lastj = j;
    i = 1;
  }
  else {
    util2_StopTimer();
    i = 0;
    t = util2_GetRtime();
    if((t < 0.40) && (lastj == j)) {
      strcpy(tmp, string);
      pchar = strtok(tmp, " ");
      pchar = strtok(NULL," ");
      pchar = strtok(NULL," ");
      c = comp_Find(arch_builder_lib, pchar);
      if(c != NIL(comp_data_t)) {
        (void) textsw_insert(atext, c->label, strlen(c->label));
        (void) textsw_insert(atext, "(x,y,z",6);
        for(i = 0;i < array_n(c->pins) - 1;i++) {
          cp = array_fetch(comp_pin_t *, c->pins, i);
          switch(cp->type) {
            case COMP_LINPUT: {
             (void) textsw_insert(atext, ",li",3);
            } break;
            case COMP_LOUTPUT: {
             (void) textsw_insert(atext, ",lo",3);
            } break;
            case COMP_SHAREDI: {
             (void) textsw_insert(atext, ",si",3);
            } break;
            case COMP_SHAREDO: {
             (void) textsw_insert(atext, ",so",3);
            } break;
            case COMP_ROUTING: {
             (void) textsw_insert(atext, ",r",2);
            } break;            
            case COMP_CLK: {
             (void) textsw_insert(atext, ",clk",4);
            } break;            
          }
        }
        cp = array_fetch(comp_pin_t *, c->pins, i);              
        switch(cp->type) {
          case COMP_LINPUT: {
           (void) textsw_insert(atext, ",li)",4);
          } break;
          case COMP_LOUTPUT: {
           (void) textsw_insert(atext, ",lo)",4);
          } break;
          case COMP_SHAREDI: {
           (void) textsw_insert(atext, ",si)",4);
          } break;
          case COMP_SHAREDO: {
           (void) textsw_insert(atext, ",so)",4);
          } break;
          case COMP_ROUTING: {
           (void) textsw_insert(atext, ",r)",3);
          } break;            
          case COMP_CLK: {
           (void) textsw_insert(atext, ",clk)",5);
          } break;            
        }
      }
    }
  }
}
