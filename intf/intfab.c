/**CFile***********************************************************************

  FileName    [intfAb.c]

  PackageName [intf]

  Synopsis    [Routines for handling the design of an architecture block.]

  Description []

  SeeAlso     []

  Author      [Ivan Jeukens]

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
#include <fcntl.h>
#include <unistd.h>
#include <ctype.h>

#undef va_start

#include "intfInt.h"

/*
static char rcsid[] = \"$Id: $\";
USE(rcsid);
*/

/*---------------------------------------------------------------------------*/
/* Variable declarations                                                     */
/*---------------------------------------------------------------------------*/

Canvas ab_canvas;
Canvas comp_canvas;

short libshow = 1;
short ab_show = 1;
char addcshow = 1;
char show_display = 1;
char pdelay_show = 1;
short io = 0;
int liblistpos = 0;
int addclistpos = 0;

unsigned long *colors;
unsigned long *dc_colors;

int o_gx = -1;
int o_gy = -1;
int gx;
int gy;
int drag_x;
int drag_y;

arch_block_t *ab = NIL(arch_block_t);
lsList ab_lib;
comp_data_t *csel = NIL(comp_data_t);

short addwire = 0;
short seladdwire = 0;
short grid = 0;
short show_labels = 0;
int grid_step = 10;
short addarray = 0;
int nx,ny,gx,gy;
char *libname = NULL;
short edit_option = 0;
short drawbox = 0;
short zoomfact = 2;

static Xv_singlecolor cms_colors[] = {
  { 255, 255, 255 },
  { 255, 0, 0 },
  { 0, 255, 0 },
  { 0, 0, 255 },
  { 250, 130, 80 },
  { 30, 230, 250 },
  { 230, 30, 250 },
  {   0,  0,   0 },     
  { 205, 205, 205},
  { 215, 215, 215},
};


/*---------------------------------------------------------------------------*/
/* Macro declarations                                                        */
/*---------------------------------------------------------------------------*/


/**AutomaticStart*************************************************************/

/*---------------------------------------------------------------------------*/
/* Static function prototypes                                                */
/*---------------------------------------------------------------------------*/

static void BlockGen(Panel_item item, Event *event);
static void edit_menu_proc(Menu menu, Menu_item menu_item);
static void change_zoom(Menu menu, Menu_item menu_item);
static int library_frame_done(Frame subframe);
static int new_ab_button(Panel_item item, Event *event);
static int callback_lib(char *fname, int argc, char **argv);
static int callback_sab(char *fname, int argc, char **argv);
static int callback_lab(char *fname, int argc, char **argv);
static void ab_menu_proc(Menu menu, Menu_item menu_item);
static int prop_frame_done(Frame frame);
static int ok_prop_button(Panel_item item, Event *event);
static void show_grid(Panel_item item, int value, Event *event);
static void show_labels_proc(Panel_item item, int value, Event *event);
static int prop_button(Panel_item item, Event *event);
static void add_io(Panel_item item, int value, Event *event);
static int array_component_button(Panel_item item, Event *event);
static int select_component_button(Panel_item item, Event *event);
static int add_component_frame_done(Frame subframe);
static void add_component_list_proc(Panel_item item, char *string, caddr_t client_data, Panel_list_op op, Event *event);
static void ab_add_menu_proc(Menu menu, Menu_item menu_item);
static void in_grid();
static void ab_repaint_proc(Canvas canvas, Xv_Window paint_window, Display *dpy, Window xwin, Xv_xrectlist *xrects);
static void ab_redraw_proc(Canvas canvas, Xv_Window paint_window, Display *dpy, Window xwin, Xv_xrectlist *xrects);
static void AbDrawProc(Xv_Window window, Event *event);
static int ab_done(Frame frame);

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
intfAbResetGlobals()
{
  liblistpos = 0;
  libshow = 1;
  gx = -1;
  gy = -1;  
  arch_BlockFree(ab);
  ab = NIL(arch_block_t);
  if(libname != NIL(char)) {
   FREE(libname);
   libname = NIL(char);
  }
  if(csel != NIL(comp_data_t)) {
    comp_Free(csel);
  }
  csel = NIL(comp_data_t);
  addwire = 0;
  seladdwire = 0;
  grid = 0;
  grid_step = 10;
  addarray = 0;
  edit_option = 0;
  drawbox = 0;
  zoomfact = 2;  
  io = 0;
  show_display = 1;
  if(ab_lib != NULL) {
    comp_LibFree(ab_lib);
  }
  ab_lib = NULL;
  ab_show = 1;
}

/**Function********************************************************************

  Synopsis           [Display the architecture block editor.]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
void
intfAbDisplay(
  Frame item,
  Event *event)
{
Cms cms;
Frame arch_builder_frame = xv_get(item, XV_KEY_DATA, ARCH_BUILDER_FRAME_KEY);
Frame ab_frame;
Panel ab_panel;
Menu_item save_ab_menu_item;
Menu_item new_ab_menu_item;
Menu_item open_ab_menu_item;
Menu_item open_lib_menu_item;
Menu_item close_lib_menu_item;
Menu_item quit_menu_item;
Menu ab_file_menu;
Menu ab_add_menu;
Menu ab_edit_menu;
Scrollbar v_ab_scrollbar;
Scrollbar h_ab_scrollbar;
Panel_item genblock_button;
Menu zoom;
Panel_item add_button;
Panel_item edit_button;
Panel_item zoom_button;

  if(ab_show == 0) return;
  ab_show = 0;

  ab_lib = lsCreate();

  cms = (Cms) xv_create((int)NULL, CMS,
    CMS_NAME, "ab__colormap",
    CMS_SIZE,  COLOR_SIZE + CMS_CONTROL_COLORS,
    CMS_CONTROL_CMS, TRUE,
    CMS_TYPE,  XV_STATIC_CMS,
    CMS_COLORS,  cms_colors,
    NULL);

  ab_frame = (Frame) xv_create(arch_builder_frame, FRAME,
    FRAME_LABEL, "Architecture Block Graphic Editor",
    XV_WIDTH, 700,
    XV_HEIGHT, 500,
    XV_SHOW, TRUE,
    FRAME_DONE_PROC, ab_done,
    FRAME_SHOW_FOOTER, TRUE,
    NULL);

  ab_panel = (Panel) xv_create(ab_frame, PANEL, 
    XV_HEIGHT, 30,
    NULL);

  save_ab_menu_item = (Menu_item) xv_create((int)NULL, MENUITEM,
    MENU_STRING, "Save ArchBlock",
    MENU_INACTIVE, TRUE,
    MENU_NOTIFY_PROC, ab_menu_proc,
    NULL);

  close_lib_menu_item = (Menu_item) xv_create((int)NULL, MENUITEM,
    MENU_STRING, "Close Lib",
    MENU_INACTIVE, TRUE,
    MENU_NOTIFY_PROC, ab_menu_proc,
    NULL);

  new_ab_menu_item = (Menu_item) xv_create((int)NULL, MENUITEM,
    MENU_STRING, "New ArchBlock",
    MENU_INACTIVE, FALSE,
    MENU_NOTIFY_PROC, ab_menu_proc,
    NULL);
    
  open_ab_menu_item = (Menu_item) xv_create((int)NULL, MENUITEM,
    MENU_STRING, "Load ArchBlock",
    MENU_INACTIVE, FALSE,
    MENU_NOTIFY_PROC, ab_menu_proc,
    NULL);
    
  open_lib_menu_item = (Menu_item) xv_create((int)NULL, MENUITEM,
    MENU_STRING, "Open Lib",
    MENU_INACTIVE, TRUE,
    MENU_NOTIFY_PROC, ab_menu_proc,
    NULL);

  quit_menu_item = (Menu_item) xv_create((int)NULL, MENUITEM,
    MENU_STRING, "Quit",
    MENU_NOTIFY_PROC, ab_menu_proc,
    NULL);
    
  ab_file_menu = (Menu) xv_create((int)NULL, MENU,
    MENU_APPEND_ITEM, new_ab_menu_item,
    MENU_APPEND_ITEM, open_ab_menu_item,
    MENU_APPEND_ITEM, save_ab_menu_item,
    MENU_APPEND_ITEM, open_lib_menu_item,
    MENU_APPEND_ITEM, close_lib_menu_item,
    MENU_APPEND_ITEM, quit_menu_item,
    MENU_NOTIFY_PROC, ab_menu_proc,
    XV_KEY_DATA, SAVE_AB_MENUITEM_KEY, save_ab_menu_item,
    XV_KEY_DATA, OPEN_LIB_MENUITEM_KEY, open_lib_menu_item,
    XV_KEY_DATA, CLOSE_LIB_MENUITEM_KEY, close_lib_menu_item,
    XV_KEY_DATA, AB_FRAME_KEY, ab_frame,
    NULL);

  (void) xv_create(ab_panel, PANEL_BUTTON,
    PANEL_LABEL_STRING,  "File",
    PANEL_ITEM_MENU,  ab_file_menu,
    NULL);  
    
  ab_add_menu = (Menu) xv_create((int)NULL, MENU,
    MENU_NOTIFY_PROC, ab_add_menu_proc,
    MENU_STRINGS, "Component", "Array", "Wire", NULL,
    XV_KEY_DATA, AB_FRAME_KEY, ab_frame,
    NULL);  

  add_button =  xv_create(ab_panel, PANEL_BUTTON,
    PANEL_LABEL_STRING, "Add",
    PANEL_ITEM_MENU, ab_add_menu,
    PANEL_INACTIVE, TRUE, 
    NULL);

  (void) xv_set(ab_file_menu, XV_KEY_DATA, AB_ADD_BUTTON_KEY, add_button, 
    NULL);
  
  ab_edit_menu = (Menu) xv_create((int)NULL, MENU,
    MENU_NOTIFY_PROC, edit_menu_proc,
    MENU_STRINGS,"Copy","Delete","Move","Rotate",NULL,
    NULL);
    
  edit_button = xv_create(ab_panel, PANEL_BUTTON,
    PANEL_LABEL_STRING, "Edit",
    PANEL_ITEM_MENU, ab_edit_menu,
    PANEL_INACTIVE, TRUE,     
    NULL);

  (void) xv_set(ab_file_menu, XV_KEY_DATA, EDIT_BUTTON_KEY, edit_button, NULL);

  zoom = (Menu) xv_create((int)NULL, MENU,
    MENU_NOTIFY_PROC, change_zoom,
    MENU_STRINGS, "In", "Out" ,NULL,
    NULL); 

  zoom_button = xv_create(ab_panel, PANEL_BUTTON,
    PANEL_LABEL_STRING, "Zoom",
    PANEL_ITEM_MENU, zoom,
    PANEL_INACTIVE, TRUE,
    NULL);

  (void) xv_set(ab_file_menu, XV_KEY_DATA, ZOOM_BUTTON_KEY, zoom_button, NULL);

  (void) xv_create(ab_panel, PANEL_BUTTON,
    PANEL_LABEL_STRING, "Proprieties",
    PANEL_NOTIFY_PROC, prop_button,
    XV_KEY_DATA, AB_FRAME_KEY, ab_frame,
    NULL);
       
  genblock_button = (Panel_item) xv_create(ab_panel, PANEL_BUTTON,
    PANEL_LABEL_STRING, "Gen ArchBlock",
    PANEL_NOTIFY_PROC, BlockGen,
    XV_KEY_DATA, AB_FRAME_KEY, ab_frame,
    PANEL_INACTIVE, TRUE,
    NULL);

  (void) xv_set(ab_file_menu, XV_KEY_DATA, GENBLOCK_BUTTON_KEY,
     genblock_button, NULL);
    
  ab_canvas = (Canvas) xv_create(ab_frame, CANVAS, 
    CANVAS_X_PAINT_WINDOW, TRUE,
    CANVAS_AUTO_SHRINK, FALSE,
    CANVAS_AUTO_EXPAND, FALSE,
    CANVAS_WIDTH, 2000,
    CANVAS_HEIGHT, 2000,
    CANVAS_RETAINED, FALSE,
    CANVAS_REPAINT_PROC, ab_repaint_proc,
    WIN_CMS, cms,
    NULL);  

  v_ab_scrollbar = xv_create(ab_canvas,SCROLLBAR,
    SCROLLBAR_DIRECTION, SCROLLBAR_VERTICAL,
    SCROLLBAR_PIXELS_PER_UNIT, 10,    
    NULL);

  h_ab_scrollbar = xv_create(ab_canvas,SCROLLBAR,
    SCROLLBAR_DIRECTION, SCROLLBAR_HORIZONTAL,
    SCROLLBAR_PIXELS_PER_UNIT, 10,    
    NULL);

  (void) xv_set(ab_canvas,
    WIN_BACKGROUND_COLOR, BLACK,
    NULL);
        
  (void) xv_set(canvas_paint_window(ab_canvas),
    WIN_CONSUME_EVENTS,
      LOC_DRAG, WIN_MOUSE_BUTTONS, LOC_MOVE,
      NULL,
    WIN_EVENT_PROC, AbDrawProc,
    NULL);

  colors = (unsigned long *) xv_get(ab_canvas, WIN_X_COLOR_INDICES);
}

/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis           [Translate the architecture block into an architecture
  graph.]

  Description        [optional]

  SideEffects        [required]

  SeeAlso            [optional]

******************************************************************************/
static void
BlockGen(
  Panel_item item,
  Event *event)
{
int rc;
char message[80];
Frame ab_frame = (Frame) xv_get(item, XV_KEY_DATA, AB_FRAME_KEY);

   rc = arch_BlockGen(ab);
   arch_GenError(rc, message);
   intfDisplayMsgbox(ab_frame, message);
}   

/**Function********************************************************************

  Synopsis           [Select the edit option.]

  Description        [optional]

  SideEffects        [required]

  SeeAlso            [optional]

******************************************************************************/
static void
edit_menu_proc(
  Menu menu,
  Menu_item menu_item)
{
  if(strcmp((char *) xv_get(menu_item,MENU_STRING),"Copy") == 0) {
    edit_option = COPY;
  }
  else
    if(strcmp((char *) xv_get(menu_item, MENU_STRING),"Delete") == 0) {
      edit_option = DELETE;
    }
    else
      if(strcmp((char *) xv_get(menu_item, MENU_STRING),"Move") == 0) {
        edit_option = MOVE;
      }
      else
        if(strcmp((char *) xv_get(menu_item, MENU_STRING),"Rotate") == 0) {
          edit_option = ROTATE;
        }
}

/**Function********************************************************************

  Synopsis           [Modify the zoom of the screen.]

  Description        [optional]

  SideEffects        [required]

  SeeAlso            [optional]

******************************************************************************/
static void
change_zoom(
  Menu menu,
  Menu_item menu_item)
{

  if(ab != NIL(arch_block_t)) {
    if((strcmp((char *) xv_get(menu_item,MENU_STRING),"In") == 0) && 
       (zoomfact > 1)) {
      ab = arch_BlockZoom(ab,0,0,0);
      zoomfact--;
      grid_step = grid_step/2;
    }
    else       
      if((strcmp((char *) xv_get(menu_item,MENU_STRING),"Out") == 0) && 
         (zoomfact < 5)) {
        ab = arch_BlockZoom(ab,1,0,0);     
        zoomfact++;
        grid_step = grid_step*2;
      }     
  }
  ab_repaint_proc(ab_canvas, canvas_paint_window(ab_canvas),
    (Display *) xv_get(ab_canvas ,XV_DISPLAY),
    xv_get(canvas_paint_window(ab_canvas), XV_XID), NULL);
}

/**Function********************************************************************

  Synopsis           [Release the library frame.]

  Description        [optional]

  SideEffects        [required]

  SeeAlso            [optional]

******************************************************************************/
static int
library_frame_done(Frame subframe)
{
  liblistpos = 0;
  libshow = 1;

  xv_destroy_safe(subframe);
  return XV_OK;
}

/**Function********************************************************************

  Synopsis           [Handle the new button.]

  Description        [optional]

  SideEffects        [required]

  SeeAlso            [optional]

******************************************************************************/
static int
new_ab_button(
  Panel_item item,
  Event *event)
{
Panel_item text = (Panel_item) xv_get(item, XV_KEY_DATA, LIB_TEXT_KEY);
Menu_item open_lib = (Menu_item) xv_get(item, XV_KEY_DATA,
  OPEN_LIB_MENUITEM_KEY);
Menu_item save_ab = (Menu_item) xv_get(item, XV_KEY_DATA,
  SAVE_AB_MENUITEM_KEY);  
char message[40];
Panel_item add_button = (Panel_item) xv_get(item, XV_KEY_DATA, 
  AB_ADD_BUTTON_KEY);
Panel_item zoom_button = (Panel_item) xv_get(item, XV_KEY_DATA, 
  ZOOM_BUTTON_KEY);
Panel_item genblock_button = (Panel_item) xv_get(item, XV_KEY_DATA,
  GENBLOCK_BUTTON_KEY);
Panel_item edit_button = (Panel_item) xv_get(item, XV_KEY_DATA,
  EDIT_BUTTON_KEY);
Frame father_frame = (Frame) xv_get(item, XV_KEY_DATA, AB_FILE_FRAME_KEY);

  if(strcmp((char *) xv_get(item, PANEL_LABEL_STRING), "Ok") == 0) {
    strcpy(message,(char *) xv_get(text,PANEL_VALUE));
    if(util2_ValidId(message) == 0) {
      intfDisplayMsgbox(father_frame, "Inadequate Architecture Block Name");
      return XV_OK;
    }      
    if(message != NIL(char)) {
      ab = arch_BlockAlloc(message);
      (void) xv_set(open_lib, MENU_INACTIVE, FALSE, NULL);
      (void) xv_set(add_button, PANEL_INACTIVE, FALSE, NULL);
      (void) xv_set(edit_button, PANEL_INACTIVE, FALSE, NULL);
      (void) xv_set(zoom_button, PANEL_INACTIVE, FALSE, NULL);
      (void) xv_set(genblock_button, PANEL_INACTIVE, FALSE, NULL);     
      (void) xv_set(save_ab, MENU_INACTIVE, FALSE, NULL);
    }
    xv_destroy_safe(father_frame);
    libshow = 1;
  }
  else
    if(strcmp((char *) xv_get(item,PANEL_LABEL_STRING),"Cancel") == 0) {
      xv_destroy_safe(father_frame);
      libshow = 1;
    }
  return XV_OK;
}

/**Function********************************************************************

  Synopsis           [Callback function for the open lib command.]

  Description        [optional]

  SideEffects        [required]

  SeeAlso            [optional]

******************************************************************************/
static int
callback_lib(
  char *fname,
  int argc,
  char **argv)
{
Frame ab_frame = (Frame) argv[0];
Menu_item open_lib = (Menu_item) argv[1];
Menu_item close_lib = (Menu_item) argv[2];
char message[248], temp[10];
int er, linen;

  if(fname != NIL(char)) {
    if(libname != NIL(char)) FREE(libname);
    libname = util_strsav(fname);
    if(ab_lib != NULL) {
      comp_LibFree(ab_lib);
    }
    ab_lib = comp_LibLoad(fname);
    if(ab_lib != NULL) {
     (void) xv_set(ab_frame, FRAME_RIGHT_FOOTER, fname, NULL);
     (void) xv_set(close_lib, MENU_INACTIVE, FALSE, NULL);
     (void) xv_set(open_lib, MENU_INACTIVE, TRUE, NULL);
    }
    else {
      strcpy(message, comp_Error(er));
      strcat(message," at line");
      sprintf(temp," %d",linen);
      strcat(message,temp);
      intfDisplayMsgbox(ab_frame, message);    
    }
  }
  
  FREE(argv);
  return 1;
}

/**Function********************************************************************

  Synopsis           [Save architecture block callback.]

  Description        [optional]

  SideEffects        [required]

  SeeAlso            [optional]

******************************************************************************/
static int
callback_sab(
  char *fname,
  int argc,
  char **argv)
{
  if(fname != NIL(char)) {
    switch(zoomfact) {
      case 1: {
        ab = arch_BlockZoom(ab,1,0,0);
      } break;
      case 2: break;
      case 3: {
        ab = arch_BlockZoom(ab,0,0,0);
      } break;
      case 4: {
        ab = arch_BlockZoom(ab,0,0,0);
        ab = arch_BlockZoom(ab,0,0,0);
      } break;
      default: break;
    }
    (void) arch_BlockSave(ab, fname);

    switch(zoomfact) {
      case 1: {
        ab = arch_BlockZoom(ab,0,0,0);
      } break;
      case 2: break;
      case 3: {
        ab = arch_BlockZoom(ab,1,0,0);
      } break;
      case 4: {
        ab = arch_BlockZoom(ab,1,0,0);
        ab = arch_BlockZoom(ab,1,0,0);
      }  break;
      default: break;
    }
  }  
  return 1;
}

/**Function********************************************************************

  Synopsis           [Load architecture block callback.]

  Description        [optional]

  SideEffects        [required]

  SeeAlso            [optional]

******************************************************************************/
static int
callback_lab(
  char *fname,
  int argc,
  char **argv)
{
Menu_item open_lib = (Menu_item) argv[0];
Panel_item add_button = (Panel_item) argv[1];
Panel_item edit_button = (Panel_item) argv[2];
Panel_item zoom_button = (Panel_item) argv[3];
Panel_item genblock_button = (Panel_item) argv[4];
Menu_item save_ab = (Menu_item) argv[5];

  if(fname != NIL(char)) {
    if(ab != NIL(arch_block_t)) {
      arch_BlockFree(ab);
    }
    ab = arch_BlockLoad(fname);
    if(ab != NIL(arch_block_t)) { 
      (void) xv_set(open_lib, MENU_INACTIVE, FALSE, NULL);
      (void) xv_set(add_button, PANEL_INACTIVE, FALSE, NULL);
      (void) xv_set(edit_button, PANEL_INACTIVE, FALSE, NULL);
      (void) xv_set(zoom_button, PANEL_INACTIVE, FALSE, NULL);
      (void) xv_set(genblock_button, PANEL_INACTIVE, FALSE, NULL);
      (void) xv_set(save_ab, MENU_INACTIVE, FALSE, NULL);    
    }
  }  
  FREE(argv);
  return 1;
}

/**Function********************************************************************

  Synopsis           [Architecture block menu proc.]

  Description        [optional]

  SideEffects        [required]

  SeeAlso            [optional]

******************************************************************************/
static void
ab_menu_proc(
  Menu menu,
  Menu_item menu_item)
{
Panel_item add_button = (Panel_item) xv_get(menu, XV_KEY_DATA,
  AB_ADD_BUTTON_KEY);
Panel_item zoom_button = (Panel_item) xv_get(menu, XV_KEY_DATA,
  ZOOM_BUTTON_KEY);
Panel_item edit_button = (Panel_item) xv_get(menu, XV_KEY_DATA,
  EDIT_BUTTON_KEY);
Panel_item genblock_button = (Panel_item) xv_get(menu, XV_KEY_DATA, 
  GENBLOCK_BUTTON_KEY);
Menu_item open_lib = (Menu_item) xv_get(menu, XV_KEY_DATA,
  OPEN_LIB_MENUITEM_KEY);
Menu_item close_lib = (Menu_item) xv_get(menu, XV_KEY_DATA,
  CLOSE_LIB_MENUITEM_KEY);
Menu_item save_ab = (Menu_item) xv_get(menu, XV_KEY_DATA, 
  SAVE_AB_MENUITEM_KEY);  
Frame ab_frame = (Frame) xv_get(menu, XV_KEY_DATA, AB_FRAME_KEY);
Frame frame;
Panel panel;
Panel_item text;
char **argv;
	
  if(strcmp((char *) xv_get(menu_item,MENU_STRING),"Open Lib") == 0) {
    if(libshow == 0) return;
    libshow = 0;
    argv = (char **) ALLOC(char *, 3);
    argv[0] = (char *) ab_frame;
    argv[1] = (char *) open_lib;
    argv[2] = (char *) close_lib;
    intfOpenFiles(ab_frame, "Open Component Library", "*.clib", &libshow,
      callback_lib, 3, argv);
  }  
  else
    if(strcmp((char *) xv_get(menu_item,MENU_STRING),"Close Lib") == 0) {
      if(ab_lib != NULL) comp_LibFree(ab_lib);
      ab_lib = lsCreate();
      FREE(libname);
      libname = NIL(char);
      libshow = 1;
      liblistpos = 0;      
      (void) xv_set(ab_frame, FRAME_RIGHT_FOOTER, "", NULL);
      (void) xv_set(open_lib, MENU_INACTIVE, FALSE, NULL);
      (void) xv_set(close_lib, MENU_INACTIVE, TRUE, NULL);
    }
    else
      if(strcmp((char *) xv_get(menu_item,MENU_STRING),"New ArchBlock") == 0) {
        if(libshow == 0) return;
        libshow = 0;
        frame = (Frame) xv_create(ab_frame, FRAME_CMD,
          FRAME_LABEL, "New ArchBlock",
          XV_WIDTH, 250,
          XV_HEIGHT, 80,
          XV_SHOW, TRUE,
          FRAME_SHOW_RESIZE_CORNER, FALSE,
          FRAME_DONE_PROC, library_frame_done,
          NULL);
   
        panel = (Panel) xv_get(frame, FRAME_CMD_PANEL);
  
        text = xv_create(panel, PANEL_TEXT,
          PANEL_LABEL_STRING, "Name: ",
          PANEL_VALUE_DISPLAY_LENGTH, 20,
          PANEL_LABEL_X, 10,
          PANEL_LABEL_Y, 20,
          NULL);              
          
        (void) xv_create(panel, PANEL_BUTTON,
          PANEL_LABEL_STRING, "Ok",
          XV_X, 70,
          XV_Y, 50,
          PANEL_NOTIFY_PROC, new_ab_button,
          XV_KEY_DATA, LIB_TEXT_KEY, text,
          XV_KEY_DATA, OPEN_LIB_MENUITEM_KEY, open_lib,
          XV_KEY_DATA, AB_ADD_BUTTON_KEY, add_button,
          XV_KEY_DATA, SAVE_AB_MENUITEM_KEY, save_ab,
          XV_KEY_DATA, EDIT_BUTTON_KEY, edit_button,
          XV_KEY_DATA, ZOOM_BUTTON_KEY, zoom_button,
          XV_KEY_DATA, GENBLOCK_BUTTON_KEY, genblock_button,
          XV_KEY_DATA, AB_FILE_FRAME_KEY, frame,
          NULL);
       
        (void) xv_create(panel, PANEL_BUTTON,
          PANEL_LABEL_STRING, "Cancel",
          XV_X, 140,
          XV_Y, 50,
          PANEL_NOTIFY_PROC, new_ab_button,
          XV_KEY_DATA, AB_FILE_FRAME_KEY, frame,          
          NULL);
      }
      else
        if(strcmp((char *) xv_get(menu_item,MENU_STRING),
          "Save ArchBlock") == 0) {
          if(libshow == 0) return;
          libshow = 0;
          intfOpenFiles(ab_frame, "Save Architecture Block", "*.ab", &libshow,
            callback_sab, 0, NIL(char*));
        }
        else
          if(strcmp((char *) xv_get(menu_item,MENU_STRING), 
            "Load ArchBlock") == 0) {
            if(libshow == 0) return;
            libshow = 0;
            argv = (char **) ALLOC(char *, 6);
            argv[0] = (char *) open_lib;
            argv[1] = (char *) add_button;
            argv[2] = (char *) edit_button;
            argv[3] = (char *) zoom_button;
            argv[4] = (char *) genblock_button;
            argv[5] = (char *) save_ab;                        
            intfOpenFiles(ab_frame, "Load Architecture Block", "*.ab", &libshow,
              callback_lab, 6, argv);
          }
          else
            if(strcmp((char *) xv_get(menu_item,MENU_STRING),"Quit") == 0) {
              intfAbResetGlobals();
              xv_destroy_safe(ab_frame);
            } 
}

/**Function********************************************************************

  Synopsis           [Release the proprieties frame.]

  Description        [optional]

  SideEffects        [required]

  SeeAlso            [optional]

******************************************************************************/
static int
prop_frame_done(Frame frame)
{
  xv_destroy_safe(frame);
  return XV_OK;
}

/**Function********************************************************************

  Synopsis           [Handle the proprieties ok button.]

  Description        [optional]

  SideEffects        [required]

  SeeAlso            [optional]

******************************************************************************/
static int
ok_prop_button(
  Panel_item item,
  Event *event)
{
Panel_item gridf = (Panel_item) xv_get(item, XV_KEY_DATA, GRIDF_TEXT_KEY);
Panel_item layer = (Panel_item) xv_get(item, XV_KEY_DATA, LAYER_TEXT_KEY);
Frame father_frame = (Frame) xv_get(item,XV_KEY_DATA, AB_PROPRIETIES_FRAME_KEY);
int tmp;

  tmp = atoi((char *) xv_get(gridf, PANEL_VALUE));
  if(tmp > 0) {
    grid_step = tmp;
  }      
  tmp = atoi((char *) xv_get(layer, PANEL_VALUE));
  if(tmp >= 0) {
/*
    switch(zoomfact) {
      case 1: subarchitecture = arch_BlockZoom(subarchitecture,1,0,0); break;
      case 2: break;
      case 3: subarchitecture = arch_BlockZoom(subarchitecture,0,0,0); break;
      case 4:  {
                subarchitecture = arch_BlockZoom(subarchitecture,0,0,0);
                subarchitecture = arch_BlockZoom(subarchitecture,0,0,0);
               }
               break;
      default: break;
    }
    zoomfact = 2;
    grid_step = 10;
*/
  }

  xv_destroy_safe(father_frame);       

  ab_repaint_proc(ab_canvas, canvas_paint_window(ab_canvas),
    (Display *) xv_get(ab_canvas ,XV_DISPLAY), 
    xv_get(canvas_paint_window(ab_canvas), XV_XID), NULL);

  return XV_OK;
}

/**Function********************************************************************

  Synopsis           [Display a grid of specified spacing.]

  Description        [optional]

  SideEffects        [required]

  SeeAlso            [optional]

******************************************************************************/
static void
show_grid(
  Panel_item item,
  int value,
  Event *event)
{
GC gc;
Rect *rec;
int i,j;

  if(value == 2) {
    if(grid == 1) {
      if(ab == NIL(arch_block_t)) {
        XClearWindow((Display *) xv_get(ab_canvas ,XV_DISPLAY),
          xv_get(canvas_paint_window(ab_canvas), XV_XID));
      }
      else {
        gc = DefaultGC((Display *) xv_get(ab_canvas ,XV_DISPLAY),
          DefaultScreen((Display *) xv_get(ab_canvas ,XV_DISPLAY)));        
        XSetForeground((Display *) xv_get(ab_canvas ,XV_DISPLAY),
          gc,colors[BLACK]);      
        rec = (Rect *) xv_get(ab_canvas, CANVAS_VIEWABLE_RECT,
          canvas_paint_window(ab_canvas),NULL);
        for(i = rec->r_left;i < rec->r_left + rec->r_width;i += grid_step) {
          for(j = rec->r_top;j < rec->r_height + rec->r_top; j += grid_step) {
             XDrawPoint((Display *) xv_get(ab_canvas ,XV_DISPLAY), 
              xv_get(canvas_paint_window(ab_canvas), XV_XID), gc, i, j);
          }
        }
      }
    }
    grid = 0;
  }
  else
    if(value == 3) {
     grid = 1;   
   }
}

/**Function********************************************************************

  Synopsis           [Show the components labels.]

  Description        [optional]

  SideEffects        [required]

  SeeAlso            [optional]

******************************************************************************/
static void
show_labels_proc(
  Panel_item item,
  int value,
  Event *event)
{
  if(value == 2) {
   show_labels = 0;
 }
  else
    if(value == 3) {
     show_labels = 1;
    }
}

/**Function********************************************************************

  Synopsis           [Display the proprieties window.]

  Description        [optional]

  SideEffects        [required]

  SeeAlso            [optional]

******************************************************************************/
static int
prop_button(
  Panel_item item,
  Event *event)
{
Frame ab_frame = (Frame) xv_get(item, XV_KEY_DATA, AB_FRAME_KEY);
Frame frame;
Panel panel;
Panel_item layer;
Panel_item gridf;

  frame = (Frame) xv_create(ab_frame, FRAME_CMD,
    FRAME_LABEL, "Proprieties",
    XV_WIDTH, 250,
    XV_HEIGHT, 140,
    XV_SHOW, TRUE,
    FRAME_SHOW_RESIZE_CORNER, FALSE,
    FRAME_DONE_PROC, prop_frame_done,
    NULL);
   
  panel = (Panel) xv_get(frame, FRAME_CMD_PANEL);
  
  layer = xv_create(panel, PANEL_TEXT,
    PANEL_LABEL_STRING, "Layer: ",
    PANEL_VALUE_DISPLAY_LENGTH, 5,
    PANEL_LABEL_X, 20,
    PANEL_LABEL_Y, 10,
    NULL);              

  gridf = xv_create(panel, PANEL_TEXT,
    PANEL_LABEL_STRING, "Grid Factor: ",
    PANEL_VALUE_DISPLAY_LENGTH, 5,
    PANEL_LABEL_X, 20,
    PANEL_LABEL_Y, 40,
    NULL);

  (void) xv_create(panel, PANEL_CHECK_BOX,
    PANEL_LAYOUT, PANEL_HORIZONTAL,
    PANEL_LABEL_STRING, "Grid",
    PANEL_LABEL_X, 20,
    PANEL_LABEL_Y, 65,
    PANEL_CHOICE_STRINGS, "", NULL,
    PANEL_NOTIFY_PROC, show_grid,
    PANEL_VALUE, grid ? 3 : 2,
    NULL);
             
  (void) xv_create(panel, PANEL_CHECK_BOX,
    PANEL_LAYOUT, PANEL_HORIZONTAL,
    PANEL_LABEL_STRING, "Labels",
    PANEL_LABEL_X, 20,
    PANEL_LABEL_Y, 95,
    PANEL_NOTIFY_PROC, show_labels_proc,
    PANEL_CHOICE_STRINGS, "", NULL,
    PANEL_VALUE, show_labels ? 3 : 2,
    NULL);

  (void) xv_create(panel, PANEL_BUTTON,
    PANEL_LABEL_STRING, "Ok",
    XV_X, 115,
    XV_Y, 110,
    PANEL_NOTIFY_PROC, ok_prop_button,
    XV_KEY_DATA, LAYER_TEXT_KEY, layer,
    XV_KEY_DATA, GRIDF_TEXT_KEY, gridf,
    XV_KEY_DATA, AB_PROPRIETIES_FRAME_KEY, frame,
    NULL);
    
  return XV_OK;
}

/**Function********************************************************************

  Synopsis           [Set the io flag.]

  Description        [optional]

  SideEffects        [required]

  SeeAlso            [optional]

******************************************************************************/
static void
add_io(
  Panel_item item,
  int value,
  Event *event)
{
  if(value == 2) {
    io = 0;
  }
  else
    if(value == 3) {
      io = 1;
    }
}

/**Function********************************************************************

  Synopsis           [Insert an array of the same selected component.]

  Description        [optional]

  SideEffects        [required]

  SeeAlso            [optional]

******************************************************************************/
static int
array_component_button(
  Panel_item item,
  Event *event)
{
Frame father_frame = (Frame) xv_get(item, XV_KEY_DATA, AB_ADD_COMP_FRAME_KEY);
Panel_item list = (Panel_item) xv_get(item, XV_KEY_DATA, AB_ADDC_LIST_KEY);
Panel_item dx = (Panel_item) xv_get(item, XV_KEY_DATA, NX_TEXT_KEY);
Panel_item dy = (Panel_item) xv_get(item, XV_KEY_DATA, NY_TEXT_KEY);
Panel_item gapx = (Panel_item) xv_get(item, XV_KEY_DATA, GAPX_TEXT_KEY);
Panel_item gapy = (Panel_item) xv_get(item, XV_KEY_DATA, GAPY_TEXT_KEY);
int sel = 0,i;
char message[40],*text_buffer;
comp_data_t *tmp;
comp_figpin_t *fptr;

  if(strcmp((char *) xv_get(item,PANEL_LABEL_STRING),"Ok") == 0) {
    while(xv_get(list, PANEL_LIST_SELECTED, sel) == 0) sel++;
    strcpy(message,(char *) xv_get(list, PANEL_LIST_STRING, sel));
    text_buffer = strtok(message," ");
    text_buffer = strtok(NULL," ");
    text_buffer = strtok(NULL," ");    
    tmp = comp_Find(ab_lib, text_buffer);        
    if(tmp != NIL(comp_data_t)) {
      nx = atoi((char *) xv_get(dx, PANEL_VALUE, NULL));
      ny = atoi((char *) xv_get(dy, PANEL_VALUE, NULL));
      gx = atoi((char *) xv_get(gapx, PANEL_VALUE, NULL));
      gy = atoi((char *) xv_get(gapy, PANEL_VALUE, NULL));
      comp_Free(csel);
      csel = comp_FigCopy(tmp);
      switch(zoomfact) {
        case 1: {
          csel->fig->width = csel->fig->width/2;
          csel->fig->height = csel->fig->height/2;
          for(i = 0;i < array_n(csel->fig->pins);i++) {
            fptr = array_fetch(comp_figpin_t *,csel->fig->pins,i);
            if(fptr->dx1 > fptr->dx2) {
              fptr->dy1 = fptr->dy1/2;
              fptr->dy2 = fptr->dy2/2;
              fptr->dx2 = fptr->dx2/2;
            }
            else {
              fptr->dy1 = fptr->dy1/2;
              fptr->dy2 = fptr->dy2/2;
              fptr->dx1 = fptr->dx1/2;
              fptr->dx2 = fptr->dx2/2;
            }
          }
        }  break;
        case 2: break;
        case 3: {
          csel->fig->width = csel->fig->width*2;
          csel->fig->height = csel->fig->height*2;
          for(i = 0;i < array_n(csel->fig->pins);i++) {
            fptr = array_fetch(comp_figpin_t *,csel->fig->pins, i);
            if(fptr->dx1 > fptr->dx2) {
              fptr->dy1 = fptr->dy1*2;
              fptr->dy2 = fptr->dy2*2;
              fptr->dx2 = fptr->dx2*2;
            }
            else {
              fptr->dy1 = fptr->dy1*2;
              fptr->dy2 = fptr->dy2*2;
              fptr->dx1 = fptr->dx1*2;
              fptr->dx2 = fptr->dx2*2;
            }
          }
        }  break;
        case 4: {
          csel->fig->width = csel->fig->width*4;
          csel->fig->height = csel->fig->height*4;
          for(i = 0;i < array_n(csel->fig->pins);i++) {
            fptr = array_fetch(comp_figpin_t *,csel->fig->pins, i);
            if(fptr->dx1 > fptr->dx2) {
              fptr->dy1 = fptr->dy1*4;
              fptr->dy2 = fptr->dy2*4;
              fptr->dx2 = fptr->dx2*4;
            }
            else {
              fptr->dy1 = fptr->dy1*4;
              fptr->dy2 = fptr->dy2*4;
              fptr->dx1 = fptr->dx1*4;
              fptr->dx2 = fptr->dx2*4;
            }
          }
        } break;
      }
      addarray = 1;
      addclistpos = 0;
      addcshow = 1;
      seladdwire = 0;
    }
  }
  else
    if(!strcmp((char *) xv_get(item,PANEL_LABEL_STRING),"Cancel")) {
      addclistpos = 0;
      addcshow = 1;
    }
    
  xv_destroy_safe(father_frame);
  return XV_OK;
}

/**Function********************************************************************

  Synopsis           [Insert a selected component.]

  Description        [optional]

  SideEffects        [required]

  SeeAlso            [optional]

******************************************************************************/
static int
select_component_button(
  Panel_item item,
  Event *event)
{
Frame father_frame = (Frame) xv_get(item, XV_KEY_DATA, AB_ADD_COMP_FRAME_KEY);
Panel_item list = (Panel_item) xv_get(item, XV_KEY_DATA, AB_ADDC_LIST_KEY);
int sel = 0, i;
char message[40],*text_buffer;
comp_data_t *tmp;
comp_figpin_t *fptr;

  if(strcmp((char *) xv_get(item,PANEL_LABEL_STRING),"Ok") == 0) {
    while(xv_get(list, PANEL_LIST_SELECTED, sel) == 0) sel++;
    strcpy(message,(char *) xv_get(list, PANEL_LIST_STRING, sel));
    text_buffer = strtok(message," ");
    text_buffer = strtok(NULL," ");
    text_buffer = strtok(NULL," ");
    tmp = comp_Find(ab_lib, text_buffer);
    if(tmp != NIL(comp_data_t)) {
      comp_Free(csel);      
      csel = comp_FigCopy(tmp);
      switch(zoomfact) {
        case 1: {
          csel->fig->width = csel->fig->width/2;
          csel->fig->height = csel->fig->height/2;
          for(i = 0;i < array_n(csel->fig->pins);i++) {
            fptr = array_fetch(comp_figpin_t *,csel->fig->pins,i);
            if(fptr->dx1 > fptr->dx2) {
              fptr->dy1 = fptr->dy1/2;
              fptr->dy2 = fptr->dy2/2;
              fptr->dx2 = fptr->dx2/2;
            }
            else {
              fptr->dy1 = fptr->dy1/2;
              fptr->dy2 = fptr->dy2/2;
              fptr->dx1 = fptr->dx1/2;
              fptr->dx2 = fptr->dx2/2;
            }
          }
        } break;
        case 2: break;
        case 3: {
          csel->fig->width = csel->fig->width*2;
          csel->fig->height = csel->fig->height*2;
          for(i = 0;i < array_n(csel->fig->pins);i++) {
            fptr = array_fetch(comp_figpin_t *,csel->fig->pins, i);
            if(fptr->dx1 > fptr->dx2) {
              fptr->dy1 = fptr->dy1*2;
              fptr->dy2 = fptr->dy2*2;
              fptr->dx2 = fptr->dx2*2;
            }
            else {
              fptr->dy1 = fptr->dy1*2;
              fptr->dy2 = fptr->dy2*2;
              fptr->dx1 = fptr->dx1*2;
              fptr->dx2 = fptr->dx2*2;
            }
          }
        }  break;
        case 4: {
          csel->fig->width = csel->fig->width*4;
          csel->fig->height = csel->fig->height*4;
          for(i = 0;i < array_n(csel->fig->pins);i++) {
            fptr = array_fetch(comp_figpin_t *,csel->fig->pins, i);
            if(fptr->dx1 > fptr->dx2) {
              fptr->dy1 = fptr->dy1*4;
              fptr->dy2 = fptr->dy2*4;
              fptr->dx2 = fptr->dx2*4;
            }
            else {
              fptr->dy1 = fptr->dy1*4;
              fptr->dy2 = fptr->dy2*4;
              fptr->dx1 = fptr->dx1*4;
              fptr->dx2 = fptr->dx2*4;
            }
          }
        }  break;
      }
      addclistpos = 0;
      addcshow = 1;
      seladdwire = 0;
    }
  }  
  else
    if(strcmp((char *) xv_get(item,PANEL_LABEL_STRING),"Cancel") == 0) {
      addclistpos = 0;
      addcshow = 1;
    }    
  xv_destroy_safe(father_frame);
  return XV_OK;
}

/**Function********************************************************************

  Synopsis           [Release the component frame.]

  Description        [optional]

  SideEffects        [required]

  SeeAlso            [optional]

******************************************************************************/
static int
add_component_frame_done(Frame subframe)
{
  addclistpos = 0;
  addcshow = 1;
  xv_destroy_safe(subframe);
  return XV_OK;
}

/**Function********************************************************************

  Synopsis           [Should handle when a component is selected.]

  Description        [optional]

  SideEffects        [required]

  SeeAlso            [optional]

******************************************************************************/
static void
add_component_list_proc(
  Panel_item item,
  char *string,
  caddr_t client_data,
  Panel_list_op op,
  Event *event)
{


}

/**Function********************************************************************

  Synopsis           [Add menu procedure.]

  Description        [optional]

  SideEffects        [required]

  SeeAlso            [optional]

******************************************************************************/
static void
ab_add_menu_proc(
  Menu menu,
  Menu_item menu_item)
{
Frame ab_frame = (Frame) xv_get(menu, XV_KEY_DATA, AB_FRAME_KEY);
Frame add_component_frame;
Panel add_component_panel;
Panel_item add_component_list;
Panel_item dx,dy,gapx,gapy;
Panel_item check;
char message[80];
lsGen gen;
comp_data_t *c;

  if(strcmp((char *) xv_get(menu_item,MENU_STRING),"Component") == 0) {
    if(addcshow == 0) return;
    addcshow = 0;
    add_component_frame = (Frame) xv_create(ab_frame, FRAME_CMD,
      FRAME_LABEL, "Select Component",
      XV_WIDTH, 500,
      XV_HEIGHT, 290,
      XV_SHOW, TRUE,
      FRAME_SHOW_RESIZE_CORNER, FALSE,
      FRAME_DONE_PROC, add_component_frame_done,
      NULL);
   
    add_component_panel = (Panel) xv_get(add_component_frame, FRAME_CMD_PANEL);
    
    add_component_list = (Panel_item) xv_create(add_component_panel, PANEL_LIST,
      PANEL_LIST_DISPLAY_ROWS, 10,
      XV_X, 10,
      XV_Y, 10,
      PANEL_LIST_ROW_HEIGHT, 20,
      PANEL_LIST_WIDTH, 465,   
      PANEL_NOTIFY_PROC, add_component_list_proc,
      NULL);     
      
   (void) xv_create( add_component_panel, PANEL_BUTTON,
     PANEL_LABEL_STRING, "Ok",
     XV_X, 185,
     XV_Y, 260,
     PANEL_NOTIFY_PROC, select_component_button,
     XV_KEY_DATA, AB_ADDC_LIST_KEY, add_component_list,
     XV_KEY_DATA, AB_ADD_COMP_FRAME_KEY, add_component_frame,
     NULL);
     
    (void) xv_create( add_component_panel, PANEL_BUTTON,
      PANEL_LABEL_STRING, "Cancel",
      XV_X, 285,
      XV_Y, 260,
      PANEL_NOTIFY_PROC, select_component_button,
      XV_KEY_DATA, AB_ADD_COMP_FRAME_KEY, add_component_frame,
      NULL);

    lsForeachItem(ab_lib, gen, c) {
      strcpy(message,"Component -> ");
      strcat(message,c->label);
        if((c->rg != NULL) && (c->lg != NULL)) 
          strcat(message,"    --    routing/logic");
        else
          if((c->rg != NULL) && (c->lg == NULL))
            strcat(message,"    --    routing");
          else
            if((c->rg == NULL) && (c->lg != NULL))
              strcat(message,"    --    logic");     
      (void) xv_set(add_component_list, PANEL_LIST_INSERT, addclistpos, 
        PANEL_LIST_STRING, addclistpos, message, NULL);       
      addclistpos++;
    }
       
    check =  xv_create(add_component_panel, PANEL_CHECK_BOX,
      PANEL_LAYOUT, PANEL_HORIZONTAL,
      PANEL_LABEL_STRING, "IO",
      PANEL_LABEL_X, 20,
      PANEL_LABEL_Y, 240,
      PANEL_CHOICE_STRINGS, "", NULL,
      PANEL_NOTIFY_PROC, add_io,
      PANEL_VALUE, 2,
      NULL);
  }
  else
    if(strcmp((char *) xv_get(menu_item,MENU_STRING),"Array") == 0) {
      if(addcshow == 0) return;
      addcshow = 0;
      add_component_frame = (Frame) xv_create(ab_frame, FRAME_CMD,
        FRAME_LABEL, "Select Component",
        XV_WIDTH, 500,
        XV_HEIGHT, 320,
        XV_SHOW, TRUE,
        FRAME_SHOW_RESIZE_CORNER, FALSE,
        FRAME_DONE_PROC, add_component_frame_done,
        NULL);
   
      add_component_panel = (Panel) xv_get(add_component_frame, 
        FRAME_CMD_PANEL);
    
      add_component_list = (Panel_item) xv_create(add_component_panel, 
        PANEL_LIST,
        PANEL_LIST_DISPLAY_ROWS, 10,
        XV_X, 10,
        XV_Y, 10,
        PANEL_LIST_ROW_HEIGHT, 20,
        PANEL_LIST_WIDTH, 465,   
        PANEL_NOTIFY_PROC, add_component_list_proc,
        NULL);     

     dx = xv_create(add_component_panel, PANEL_TEXT,
      PANEL_LABEL_STRING, "NX: ",
      PANEL_VALUE_DISPLAY_LENGTH, 5,
      PANEL_LABEL_X, 30,
      PANEL_LABEL_Y, 250,
      NULL);              
      
     dy = xv_create(add_component_panel, PANEL_TEXT,
      PANEL_LABEL_STRING, "NY: ",
      PANEL_VALUE_DISPLAY_LENGTH, 5,
      PANEL_LABEL_X, 130,
      PANEL_LABEL_Y, 250,
      NULL);            
      
     gapx = xv_create(add_component_panel, PANEL_TEXT,
      PANEL_LABEL_STRING, "Gap X: ",
      PANEL_VALUE_DISPLAY_LENGTH, 5,
      PANEL_LABEL_X, 230,
      PANEL_LABEL_Y, 250,
      NULL);            
      
     gapy =  xv_create(add_component_panel, PANEL_TEXT,
      PANEL_LABEL_STRING, "Gap Y: ",
      PANEL_VALUE_DISPLAY_LENGTH, 5,
      PANEL_LABEL_X, 350,
      PANEL_LABEL_Y, 250,
      NULL);                  
      
     (void) xv_create( add_component_panel, PANEL_BUTTON,
       PANEL_LABEL_STRING, "Ok",
       XV_X, 185,
       XV_Y, 290,
       PANEL_NOTIFY_PROC, array_component_button,
       XV_KEY_DATA, AB_ADDC_LIST_KEY, add_component_list,
       XV_KEY_DATA, GAPY_TEXT_KEY, gapy,
       XV_KEY_DATA, GAPX_TEXT_KEY, gapx,
       XV_KEY_DATA, NX_TEXT_KEY, dx,
       XV_KEY_DATA, NY_TEXT_KEY, dy,
       XV_KEY_DATA, AB_ADD_COMP_FRAME_KEY, add_component_frame,            
       NULL);
     
     (void) xv_create( add_component_panel, PANEL_BUTTON,
       PANEL_LABEL_STRING, "Cancel",
       XV_X, 285,
       XV_Y, 290,
       PANEL_NOTIFY_PROC, array_component_button,
       XV_KEY_DATA, AB_ADD_COMP_FRAME_KEY, add_component_frame, 
       NULL);
       
     lsForeachItem(ab_lib, gen, c) {
       strcpy(message,"Component -> ");
       strcat(message,c->label);
         if((c->rg != NULL) && (c->lg != NULL)) 
           strcat(message,"    --    routing/logic");
         else
           if((c->rg != NULL) && (c->lg == NULL))
             strcat(message,"    --    routing");
           else
             if((c->rg == NULL) && (c->lg != NULL))
               strcat(message,"    --    logic");     
         (void) xv_set(add_component_list, PANEL_LIST_INSERT, addclistpos, 
            PANEL_LIST_STRING, addclistpos, message, NULL);       
          addclistpos++;
      }       
    }
    else
      if(strcmp((char *) xv_get(menu_item,MENU_STRING),"Wire") == 0) {
        seladdwire = 1;      
      }
}

/**Function********************************************************************

  Synopsis           [Move the global position to the nearest valid grid
  position.]

  Description        [optional]

  SideEffects        [Change the gx, gy variables.]

  SeeAlso            [optional]

******************************************************************************/
static void
in_grid()
{
int dx,dy;
int left, top;

  dx = gx;
  while((dx%grid_step) != 0) {
    dx--;  
  }
  left = dx;
  
  dy = gy;
  while((dy%grid_step) != 0) { 
    dy--;
  }
  top = dy;

  if(util2_InRect(gx, gy, left, top, grid_step, grid_step)) {
    gx = left;
    gy = top;
  }
  else
    if(util2_InRect(gx, gy, left + grid_step/2, top, grid_step, grid_step)) {
      gx = left + grid_step;
      gy = top;
    }
    else
      if(util2_InRect(gx, gy, left, top + grid_step/2, grid_step, grid_step)) {
        gx = left;
        gy = top + grid_step;
      }
      else
        if(util2_InRect(gx, gy, left + grid_step/2, top + grid_step/2,
          grid_step, grid_step)) {
          gx = left + grid_step;
          gy = top + grid_step;
        }
}

/**Function********************************************************************

  Synopsis           [Handle the repaint of the canvas.]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static void
ab_repaint_proc(
  Canvas canvas,
  Xv_Window paint_window,
  Display *dpy,
  Window xwin,
  Xv_xrectlist *xrects)
{
GC gc;
Rect *rec;
int i,j;
arch_abcomp_t *cptr;
arch_abseg_t *sptr;
comp_figpin_t *fptr;
lsGen gen;

  gc = DefaultGC(dpy, DefaultScreen(dpy));
  (void) xv_set(ab_canvas, WIN_BACKGROUND_COLOR, BLACK, 0);
  XSetLineAttributes(dpy, gc, 2, LineSolid, CapButt, JoinMiter);

  rec = (Rect *) xv_get(ab_canvas, CANVAS_VIEWABLE_RECT, paint_window,NULL);   
  
  if(ab != NIL(arch_block_t)) {
    XClearWindow(dpy, xwin);
    lsForeachItem(ab->comp, gen, cptr) {
      if(util2_InRect(cptr->x + cptr->fig->width, cptr->y + cptr->fig->height,
           rec->r_left, rec->r_top, rec->r_width, rec->r_height) ||
         util2_InRect(cptr->x, cptr->y, rec->r_left, rec->r_top, rec->r_width,
           rec->r_height) ) {
        XSetForeground(dpy, gc, colors[RED]);
        if(cptr->fig->width < 0) {
          XDrawRectangle(dpy, xwin, gc, cptr->x + cptr->fig->width, 
            cptr->y, abs(cptr->fig->width), cptr->fig->height);
        }
        else
          if(cptr->fig->height < 0) {
            XDrawRectangle(dpy, xwin, gc, cptr->x, cptr->y + cptr->fig->height,
              cptr->fig->width, abs(cptr->fig->height));   
          }         
          else {
            XDrawRectangle(dpy, xwin, gc, cptr->x, cptr->y, 
              cptr->fig->width, cptr->fig->height);
          }
                
        XSetForeground(dpy,gc,colors[ORANGE]);
        for(i = 0;i < array_n(cptr->fig->pins);i++) {
          fptr = array_fetch(comp_figpin_t *, cptr->fig->pins, i);
          XDrawLine(dpy, xwin, gc, cptr->x + fptr->dx1, cptr->y + fptr->dy1,
            cptr->x + fptr->dx2, cptr->y + fptr->dy2);
        }          
        if(show_labels == 1) {
          XDrawString(dpy, xwin, gc, cptr->x + 2, 
            cptr->y + cptr->fig->height/2, cptr->label, strlen(cptr->label));
        }
      }
    }
    lsForeachItem(ab->seg, gen, sptr) {
      if(util2_InRect(sptr->x, sptr->y, rec->r_left, rec->r_top,
           rec->r_width,rec->r_height) || 
         util2_InRect(sptr->x2, sptr->y2, rec->r_left, rec->r_top, rec->r_width, 
           rec->r_height) ) {
        XSetForeground(dpy, gc, colors[AQUA]);
        XDrawLine(dpy, xwin, gc, sptr->x, sptr->y, sptr->x2, sptr->y2);           
      }
    }
  }
  
  if(grid) {
    XSetForeground(dpy, gc, colors[WHITE]);  
    if(ab == NIL(arch_block_t)) XClearWindow(dpy, xwin);
    rec = (Rect *) xv_get(ab_canvas, CANVAS_VIEWABLE_RECT, paint_window,NULL);
    for(i = rec->r_left;i < rec->r_left + rec->r_width;i += grid_step)
      for(j = rec->r_top;j < rec->r_height + rec->r_top; j += grid_step)
         XDrawPoint(dpy, xwin, gc, i, j);
  }
}

/**Function********************************************************************

  Synopsis           [Handle the redraw of the canvas.]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static void
ab_redraw_proc(
  Canvas canvas,
  Xv_Window paint_window,
  Display *dpy,
  Window xwin,
  Xv_xrectlist *xrects)
{
GC gc;
Rect *rec;
int i,j;
comp_figpin_t *fptr;

  gc = DefaultGC(dpy, DefaultScreen(dpy));
  (void) xv_set(ab_canvas, WIN_BACKGROUND_COLOR, BLACK, 0);
  XSetLineAttributes(dpy, gc, 2, LineSolid, CapButt, JoinMiter);

  rec = (Rect *) xv_get(ab_canvas, CANVAS_VIEWABLE_RECT, paint_window,NULL);   

  if(csel != NIL(comp_data_t)) {
    if(o_gx != -1) {
      XSetForeground(dpy, gc, colors[BLACK]);
      XDrawRectangle(dpy, xwin, gc, o_gx, 
        o_gy, csel->fig->width, csel->fig->height);
      for(i = 0;i < array_n(csel->fig->pins);i++) {
        fptr = array_fetch(comp_figpin_t *, csel->fig->pins, i);
        XDrawLine(dpy, xwin, gc, o_gx + fptr->dx1, o_gy + fptr->dy1, 
          o_gx + fptr->dx2, o_gy + fptr->dy2);
      }     
    }
    XSetForeground(dpy, gc, colors[WHITE]);
    XDrawRectangle(dpy, xwin, gc, gx, gy, csel->fig->width, csel->fig->height);
    for(i = 0;i < array_n(csel->fig->pins);i++) {
      fptr = array_fetch(comp_figpin_t *,csel->fig->pins,i);
      XDrawLine(dpy, xwin, gc, gx + fptr->dx1, gy + fptr->dy1, gx + fptr->dx2,
        gy + fptr->dy2);
    }
  }
  else  
  if(addwire == 1) {
    XSetForeground(dpy, gc, colors[BLACK]);
    XDrawLine(dpy, xwin, gc, drag_x, drag_y, o_gx, o_gy);
    XSetForeground(dpy, gc, colors[WHITE]);
    XDrawLine(dpy, xwin, gc, drag_x, drag_y, gx, gy);    
  }

  if(grid == 1) {   
    XSetForeground(dpy, gc, colors[WHITE]);  
    if(ab == NIL(arch_block_t))
      XClearWindow(dpy, xwin);
      
    rec = (Rect *) xv_get(ab_canvas, CANVAS_VIEWABLE_RECT, paint_window,NULL);
    for(i = rec->r_left;i < rec->r_left + rec->r_width;i += grid_step)
      for(j = rec->r_top;j < rec->r_height + rec->r_top; j += grid_step)
         XDrawPoint(dpy, xwin, gc, i, j);
  }
  
  if(drawbox == 1) {
    if(o_gx != -1) {
      XSetForeground(dpy,gc,colors[BLACK]);
      i = o_gx - drag_x;
      j = o_gy - drag_y;
      if((i > 0) && (j > 0)) 
        XDrawRectangle(dpy, xwin, gc, drag_x, drag_y, (unsigned int) i,
          (unsigned int) j);
      else
        if((i > 0) && (j < 0)) 
          XDrawRectangle(dpy, xwin, gc, drag_x, o_gy, (unsigned int) i,
           (unsigned int) drag_y - o_gy);
        else
          if((i < 0) && (j > 0))
            XDrawRectangle(dpy, xwin, gc, o_gx, drag_y, 
              (unsigned int) drag_x - o_gx, (unsigned int) j);
          else
            if((i < 0) && (j < 0))
              XDrawRectangle(dpy, xwin, gc, o_gx, o_gy, 
                (unsigned int) drag_x - o_gx,(unsigned int) drag_y - o_gy);
    }
    XSetForeground(dpy, gc, colors[WHITE]);
    i = gx - drag_x;
    j = gy - drag_y;
    if((i > 0) && (j > 0)) 
      XDrawRectangle(dpy, xwin, gc, drag_x, drag_y,(unsigned int) i,
        (unsigned int) j);
    else
      if((i > 0) && (j < 0)) 
        XDrawRectangle(dpy, xwin, gc, drag_x, gy,(unsigned int) i,
         (unsigned int) drag_y - gy);
      else
        if((i < 0) && (j > 0))
          XDrawRectangle(dpy, xwin, gc, gx, drag_y, (unsigned int) drag_x - gx,
            (unsigned int) j);
        else
          if((i < 0) && (j < 0))
            XDrawRectangle(dpy, xwin, gc, gx, gy, (unsigned int) drag_x - gx,
              (unsigned int) drag_y - gy);
  }
}

/**Function********************************************************************

  Synopsis           [Handle the various canvas events.]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static void
AbDrawProc(
  Xv_Window window,
  Event *event)
{
GC gc;
static short final_point = 0; 
int i,j;
static int xf, yf; 

  o_gx = gx;
  o_gy = gy;
  gx = event_x(event);
  gy = event_y(event);
  
  if(grid) in_grid();
    
  switch(event_action(event)) {
    case LOC_DRAG: {
      if(event_left_is_down(event) && seladdwire) {
        ab_redraw_proc(ab_canvas, window, 
          (Display *) xv_get(ab_canvas, XV_DISPLAY), 
          xv_get(window, XV_XID), NULL);
        addwire = 1;
      } 
      else
        if(event_right_is_down(event) && drawbox) {  
          ab_redraw_proc(ab_canvas, window,
            (Display *) xv_get(ab_canvas ,XV_DISPLAY),
            xv_get(window, XV_XID), NULL);        
        }
    } break;

    case LOC_MOVE: {
      if(csel) {
        ab_redraw_proc(ab_canvas, window,
          (Display *) xv_get(ab_canvas ,XV_DISPLAY), 
          xv_get(window, XV_XID), NULL);
      }
      else
        if(final_point) {
          gc = DefaultGC(xv_get(ab_canvas,XV_DISPLAY),
            DefaultScreen(xv_get(ab_canvas ,XV_DISPLAY)));

          XSetForeground((Display *) xv_get(ab_canvas,XV_DISPLAY), gc, 
            colors[BLACK]);
          XDrawLine((Display *) xv_get(ab_canvas ,XV_DISPLAY),
           xv_get(canvas_paint_window(ab_canvas), XV_XID), 
           gc, drag_x, drag_y, o_gx, o_gy);

          XSetForeground((Display *) xv_get(ab_canvas,XV_DISPLAY), gc,
            colors[WHITE]);
          XDrawLine((Display *) xv_get(ab_canvas ,XV_DISPLAY),
           xv_get(canvas_paint_window(ab_canvas), XV_XID), 
           gc, drag_x, drag_y, gx, gy);
        }
    } break;

    case ACTION_SELECT: {
      if(event_is_down(event)) {
        if(final_point) {
          if(edit_option == COPY) {
            ab = arch_BlockCopy(ab, drag_x, drag_y, xf, yf, gx, gy, zoomfact);
          }
          else
            if(edit_option == MOVE) {
              ab = arch_BlockMove(ab, drag_x, drag_y, xf, yf, gx, gy, zoomfact);                
            }
          ab_repaint_proc(ab_canvas, window,
            (Display *) xv_get(ab_canvas ,XV_DISPLAY), xv_get(window, XV_XID),            
            NULL);                                         
        }
        final_point = 0;
        if(csel && !addarray) {
          ab = arch_AbcompIns(ab, csel->fig, csel->label, libname, gx, gy,
            io, 0);
          io = 0;
          comp_Free(csel);
          csel = NIL(comp_data_t);
          ab_repaint_proc(ab_canvas, window,
            (Display *) xv_get(ab_canvas ,XV_DISPLAY), xv_get(window, XV_XID),
             NULL);
        }
        else
          if(seladdwire) {
            drag_x = gx;
            drag_y = gy;
          }
         else
           if(csel && addarray) {
             if(!nx && ny) {
               for(j = 0;j < ny;j++)
                 ab = arch_AbcompIns(ab, csel->fig, csel->label, libname, gx, 
                   gy + j*(csel->fig->height + gy), io, 0);               
             }
             else
               if(!ny && nx) {
                 for(i = 0;i < nx;i++)
                   ab = arch_AbcompIns(ab, csel->fig, csel->label, libname, 
                     gx + i*(gx + csel->fig->width), gy, io, 0);                                  
               }
               else
                 if(nx && ny) {
                   for(i = 0;i < nx;i++)
                     for(j = 0;j < ny;j++)
                       ab = arch_AbcompIns(ab, csel->fig, csel->label,
                         libname, gx + i*(gx + csel->fig->width), 
                         gy + j*(csel->fig->height + gy), io,0);               
                 }
             comp_Free(csel);
             csel = NIL(comp_data_t);
             addarray = 0;
             ab_repaint_proc(ab_canvas, window,
               (Display *) xv_get(ab_canvas ,XV_DISPLAY), 
                xv_get(window, XV_XID), NULL);                        
           }       
      }
      else
        if(seladdwire) {
          ab = arch_AbsegIns(ab, drag_x, drag_y, gx, gy);
          addwire = 0;
          ab_repaint_proc(ab_canvas, window,
            (Display *) xv_get(ab_canvas ,XV_DISPLAY), xv_get(window, XV_XID),
             NULL);
        }
    } break;

    case ACTION_MENU: {
      if(event_is_down(event)) {
        drag_x = gx;
        drag_y = gy;
        drawbox = 1;      
      }
      else {
        drawbox = 0;
        if(edit_option == DELETE) {
          ab = arch_BlockRm(ab, drag_x, drag_y, gx, gy);
        }
        else
          if(edit_option == ROTATE) {
            ab = arch_BlockRotate(ab, drag_x, drag_y, gx, gy);
          }
          else
            if((edit_option == COPY) || (edit_option == MOVE)) {
              xf = gx;
              yf = gy;
              final_point = 1;
            }            
        ab_repaint_proc(ab_canvas, window,
         (Display *) xv_get(ab_canvas ,XV_DISPLAY), xv_get(window, XV_XID),
          NULL);
      }
    } break;
    
    case ACTION_ADJUST: {
      if(event_is_down(event) && (ab != NIL(arch_block_t))) {
/*
        while(data != NULL) {
            if(data->comp && (in_rect(subarchgen_gx, subarchgen_gy,
                 data->x, data->y, data->comp->fig->width,
                 data->comp->fig->height))) {
               sprintf(message,"Component %s from library %s",data->comp->label,
                 data->comp->library);
               (void) xv_set(subarchgen_frame, FRAME_LEFT_FOOTER, message, NULL);                
              break;
            }
            else
              if(data->seg && in_line(subarchgen_gx,subarchgen_gy, data->x, data->y,
                data->seg->x, data->seg->y)) {
                get_pdelay(data->seg->pdelay,tmp->n,data->x,data->y,
                  data->seg->x, data->seg->y);
                break;
              }
              else
                data = data->next;
          }
*/
      }        
    } break;

    default: break;
  }
}

/**Function********************************************************************

  Synopsis           [Exit function.]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static int
ab_done(Frame frame)
{
  intfAbResetGlobals();
  xv_destroy_safe(frame);
  return XV_OK;
}
