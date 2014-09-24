/**CFile***********************************************************************

  FileName    [intfComp.c]

  PackageName [intf]

  Synopsis    [Interface routines for the component editor.]

  Description []

  SeeAlso     []

  Author      [Ivan Jeukens]

  Copyright   [1996-1997 Ivan Jeukens]

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
char compshow = 1;
short fileshow = 1;
int listpos = 0;
lsList comp_lib;

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

unsigned long *vc_colors;

/**AutomaticStart*************************************************************/

/*---------------------------------------------------------------------------*/
/* Static function prototypes                                                */
/*---------------------------------------------------------------------------*/

static int comp_edit_frame_done(Frame subframe);
static void list_comp_edit(Panel_item item, char *string, caddr_t client_data, Panel_list_op op, Event *event);
static void viewcomp_repaint_proc(Canvas canvas, Xv_Window paint_window, Display *dpy, Window xwin, Xv_xrectlist *xrects);
static int button_view_comp(Panel_item item, Event *event);
static int button_comp_edit(Panel_item item, Event *event);
static int callback_olib(char *fname, int argc, char **argv);
static int callback_slib(char *fname, int argc, char **argv);
static void menu_comp_edit(Menu menu, Menu_item menu_item);

/**AutomaticEnd***************************************************************/


/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis           [Free and reset global variables.]

  Description        []

  SideEffects        [Might release the library global variable.]

  SeeAlso            []

******************************************************************************/
void
intfCompeResetGlobals()
{
  fileshow = 1;
  compshow = 1;
  listpos = 0;
  if(comp_lib != NULL) comp_LibFree(comp_lib);
  comp_lib = NULL;  
}

/**Function********************************************************************

  Synopsis           [Display the component editor.]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
void
intfCompeDisplay(
  Frame item,
  Event *event)
{
Menu file_menu;
Frame comp_edit_frame;
Panel comp_edit_panel;
Textsw comp_text;
Panel_item comp_list;
Panel_item comp_label;
Frame arch_builder_frame = (Frame) xv_get(item, XV_KEY_DATA, ARCH_BUILDER_FRAME_KEY);

  comp_lib = lsCreate();
  if(compshow == 0) return;
  compshow = 0;
    
  comp_edit_frame = (Frame) xv_create(arch_builder_frame, FRAME,
    FRAME_LABEL, "Component Editor",
    XV_WIDTH, 485,
    XV_HEIGHT, 600,
    XV_SHOW, TRUE,
    FRAME_DONE_PROC, comp_edit_frame_done,
    NULL);
    
  comp_edit_panel = (Panel) xv_create(comp_edit_frame, PANEL,
    XV_WIDTH, 485,
    XV_HEIGHT, 600,
    XV_SHOW, TRUE,
    NULL);    
  
  file_menu = (Menu) xv_create((int) NULL, MENU,
    MENU_NOTIFY_PROC,  menu_comp_edit,
    MENU_STRINGS, "Open Lib", "Save Lib", "Quit", NULL,
    XV_KEY_DATA, COMP_EDIT_FRAME_KEY, comp_edit_frame,
    NULL);
    
  (void) xv_create(comp_edit_panel, PANEL_BUTTON,
    PANEL_LABEL_STRING, "File",
    PANEL_NOTIFY_PROC, button_comp_edit,
    PANEL_ITEM_MENU, file_menu,
    NULL);

  comp_text = (Textsw) xv_create(comp_edit_frame, TEXTSW,
     XV_X, 10,
     XV_Y, 190,
     WIN_ROWS, 28,
     WIN_COLUMNS, 62,
     NULL); 

  (void) xv_set(file_menu, XV_KEY_DATA, COMP_TEXT_KEY, comp_text, NULL);    
  
  comp_label = xv_create(comp_edit_panel, PANEL_TEXT,
    PANEL_LABEL_STRING, "Label:",
    PANEL_VALUE_DISPLAY_LENGTH, 40,
    PANEL_LABEL_X, 10,
    PANEL_LABEL_Y, 160,
    NULL);
    
  comp_list = (Panel_item) xv_create(comp_edit_panel, PANEL_LIST,
    PANEL_LIST_DISPLAY_ROWS, 5,
    XV_X, 10,
    XV_Y, 35,
    PANEL_LIST_ROW_HEIGHT, 20,
    PANEL_LIST_WIDTH, 450,
    PANEL_NOTIFY_PROC, list_comp_edit,
    XV_KEY_DATA, COMP_TEXT_KEY, comp_text,
    NULL);   

  (void) xv_set(file_menu, XV_KEY_DATA, COMP_LIST_KEY, comp_list, NULL);
  
  (void) xv_create(comp_edit_panel, PANEL_BUTTON,
    PANEL_LABEL_STRING, "Insert",
    PANEL_NOTIFY_PROC, button_comp_edit,
    XV_X, 120,
    XV_Y, 575,
    XV_KEY_DATA, COMP_TEXT_KEY, comp_text,
    XV_KEY_DATA, COMP_EDIT_FRAME_KEY, comp_edit_frame,
    XV_KEY_DATA, COMP_LABEL_KEY, comp_label,
    XV_KEY_DATA, COMP_LIST_KEY, comp_list,
    NULL);
    
  (void) xv_create(comp_edit_panel, PANEL_BUTTON,
    PANEL_LABEL_STRING, "Delete",
    PANEL_NOTIFY_PROC, button_comp_edit,
    XV_KEY_DATA, COMP_LIST_KEY, comp_list,    
    XV_X, 200,
    XV_Y, 575,
    NULL);
    
  (void) xv_create(comp_edit_panel, PANEL_BUTTON,
    PANEL_LABEL_STRING, "View",
    PANEL_NOTIFY_PROC, button_comp_edit,
    XV_KEY_DATA, COMP_EDIT_FRAME_KEY, comp_edit_frame,
    XV_KEY_DATA, COMP_LIST_KEY, comp_list,    
    XV_X, 290,
    XV_Y, 575,
    NULL);   
}

/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis           [Release the component editor frame. Exit function.]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static int
comp_edit_frame_done(Frame subframe)
{
  intfCompeResetGlobals();
  xv_destroy_safe(subframe);
  return XV_OK;
}

/**Function********************************************************************

  Synopsis           [Procedure that handle when a component is selected.]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static void
list_comp_edit(
  Panel_item item,
  char *string,
  caddr_t client_data,
  Panel_list_op op,
  Event *event)
{
Textsw comp_text = (Textsw) xv_get(item, XV_KEY_DATA, COMP_TEXT_KEY);
char *tmp,*tmp2;
comp_data_t *c;

  if(op) {
    tmp2 = util_strsav(string);
    tmp = strtok(tmp2," ");
    tmp = strtok(NULL," ");
    tmp = strtok(NULL," ");
    c = comp_Find(comp_lib, tmp);
    if(c != NIL(comp_data_t)) {
      textsw_reset(comp_text,0,0);
      (void) textsw_insert(comp_text,c->text,(int) strlen(c->text));
    }
    FREE(tmp2);
  }
}

/**Function********************************************************************

  Synopsis           [Handle the reapaint of the view component canvas.]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static void
viewcomp_repaint_proc(
  Canvas canvas,
  Xv_Window paint_window,
  Display *dpy,
  Window xwin,
  Xv_xrectlist *xrects)
{
GC gc = DefaultGC(dpy, DefaultScreen(dpy));
comp_data_t *c = (comp_data_t *) xv_get(canvas, XV_KEY_DATA, VIEW_COMP_KEY);
comp_figpin_t *fptr;
comp_pin_t *pptr;
char number[5],message[40];
int i, maxy = -1,pn = 0;
int *xp,*yp;
char **names;
lsList nodes;
lsGen gen, gen2;
vertex_t *rv,*rv2;
edge_t *re;
/*
rgraph_data *rptr;
*/
  if(c == NIL(comp_data_t)) return;
  
  XSetLineAttributes(dpy, gc, 2, LineSolid, CapButt, JoinMiter);
  XSetForeground(dpy, gc, vc_colors[RED]);
  XDrawRectangle(dpy, xwin, gc, 30, 30, c->fig->width, c->fig->height);
  XSetForeground(dpy, gc, vc_colors[AQUA]);

  for(i = 0;i < array_n(c->fig->pins);i++) {
    fptr = array_fetch(comp_figpin_t *,c->fig->pins,i);
    XDrawLine(dpy, xwin, gc, 30 + fptr->dx1, 30 + fptr->dy1, 30 + fptr->dx2,
      30 + fptr->dy2);
    sprintf(number,"%d",fptr->number);
    if(fptr->dx1 > fptr->dx2) 
      XDrawString(dpy, xwin, gc, 30 + fptr->dx2 - 10, 30 + fptr->dy1, 
        number, strlen(number));
    else
      if(fptr->dx1 < fptr->dx2) 
        XDrawString(dpy, xwin, gc, 30 + fptr->dx1 + 10, 30 + fptr->dy1, 
          number, strlen(number));
      else
        if(fptr->dy1 > fptr->dy2) 
          XDrawString(dpy, xwin, gc, 30 + fptr->dx1 - 5,30 + fptr->dy2 - 1, 
            number, strlen(number));        
        else {
          XDrawString(dpy, xwin, gc, 30 + fptr->dx1 - 5,30 + fptr->dy1 + 20, 
            number, strlen(number));        
          if((30 + fptr->dy1 + 20) > maxy) maxy = 30 + fptr->dy1 + 20;
         }
    pn++;                                  
  }
  for(i = 0;i < array_n(c->pins);i++) {
    pptr = array_fetch(comp_pin_t *, c->pins, i);
    sprintf(message,"%d. %s ",i+1,pptr->label);
    switch(pptr->type) {
      case COMP_ROUTING: strcat(message,"ROUTING"); break;
      case COMP_LINPUT: strcat(message,"LOGIC INPUT"); break;
      case COMP_LOUTPUT: strcat(message,"LOGIC OUTPUT"); break;
      case COMP_SHAREDI: strcat(message,"SHARED INPUT"); break;
      case COMP_SHAREDO: strcat(message,"SHARED OUTPUT"); break;
      case COMP_CLK: strcat(message,"CLOCK"); break;
      default: break;
    }
    XDrawString(dpy, xwin, gc, 10, maxy + 10 + 15*i, message, strlen(message));
  }  

  i = comp_Type(c);
  if((i == 3) || (i == 2)) {
  
  
  }
  else
    if((i == 3) || (i == 1)) {
      XDrawString(dpy, xwin, gc, 190, 30, "Routing Graph",13);    
      xp = (int *) ALLOC(int, pn);
      yp = (int *) ALLOC(int, pn);
      names = (char **) ALLOC(char *,pn);
      for(i = 0;i < array_n(c->fig->pins);i++) {
        fptr = array_fetch(comp_figpin_t *,c->fig->pins,i);
        XDrawLine(dpy, xwin, gc, 200 + fptr->dx1, 80 + fptr->dy1, 
         200 + fptr->dx2, 80 + fptr->dy2);
        xp[fptr->number - 1] = fptr->dx1 + 200;
        yp[fptr->number - 1] = 80 + fptr->dy1;
      }   
      for(i = 0;i < array_n(c->pins);i++) {
        pptr = array_fetch(comp_pin_t *,c->pins,i);
        names[i] = util_strsav(pptr->label);
      }
      nodes = g_get_vertices(c->rg);
/*
      gen = lsStart(nodes);
      lsForeachItem(nodes, gen, rv) {
        rptr = (rgraph_data *) rv->user_data;
        for(i = 0;i < pn;i++)
          if(!strcmp(names[i], rptr->label)) break;
        foreach_out_edge(rv, gen2, re) {
          rv2 = g_e_dest(re);
          rptr = (rgraph_data *) rv2->user_data;
          for(maxy = 0;maxy < pn;maxy++)
            if(!strcmp(names[maxy], rptr->label)) break;
          if(maxy < pn)
            XDrawLine(dpy, xwin, gc, xp[i], yp[i], xp[maxy], yp[maxy]);                      
        }    
      }    
*/
      FREE(xp);
      FREE(yp);
      for(i = 0;i < pn;i++) {
        FREE(names[i]);
      }
      FREE(names);
    }   
}


/**Function********************************************************************

  Synopsis           [Handle the ok button from the view component.]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static int
button_view_comp(
  Panel_item item,
  Event *event)
{
Frame father_frame = (Frame) xv_get(item, XV_KEY_DATA, VIEW_COMP_FRAME_KEY);

  xv_destroy_safe(father_frame);
  return XV_OK;
}


/**Function********************************************************************

  Synopsis           [Component editor buttons.]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static int
button_comp_edit(
  Panel_item item,
  Event *event)
{
int erro,len,linen;
char *text_buffer;
char message[80],temp[10];
comp_data_t *c = NIL(comp_data_t);
char tmp[40];
Frame comp_edit_frame = (Frame) xv_get(item, XV_KEY_DATA, COMP_EDIT_FRAME_KEY);
Frame parser_frame;
Textsw comp_text = (Textsw) xv_get(item, XV_KEY_DATA, COMP_TEXT_KEY);
Panel parser_panel;
Panel_item comp_label = (Panel_item) xv_get(item, XV_KEY_DATA, COMP_LABEL_KEY);
Panel_item comp_list = (Panel_item) xv_get(item, XV_KEY_DATA, COMP_LIST_KEY);
Cms cms;
short type;
char *war;

  if(strcmp((char *) xv_get(item, PANEL_LABEL_STRING), "Insert") == 0) {
    strcpy(tmp, (char *) xv_get(comp_label, PANEL_VALUE, NULL));   
    for(erro = 0;erro < (int) strlen(tmp);erro++) {
      if(isalnum(tmp[erro]) == 0) {
        intfDisplayMsgbox(comp_edit_frame, "Inadequate component label");        
        return XV_OK;
      }
    }
    len = (int) xv_get(comp_text,TEXTSW_LENGTH, NULL);
    text_buffer = (char *) ALLOC(char,len+1);
    (void) xv_get(comp_text, TEXTSW_CONTENTS, 0, text_buffer, len); 
    text_buffer[len] = '\0';
    c = comp_Parse(text_buffer, &linen, &erro);
    if((erro != 0) || (comp_PinFind(c->pins, tmp) != -1)) {
      if(erro != 0) {
        strcpy(message, comp_Error(erro));
        strcat(message," at line");
        sprintf(temp," %d",linen);
        strcat(message,temp);
      }
      else {
        sprintf(message,"Error: Component label match a pin label");
      }
      intfDisplayMsgbox(comp_edit_frame, message);
    } 
    else {
/*
      war = comp_Check(c);
      if(war != NIL(char)) {
        intfDisplayMsgbox(comp_edit_frame, war);
        FREE(war);
        return XV_OK;
      }
*/      
      FREE(c->label);
      c->label = util_strsav(tmp);      
      if((comp_Find(comp_lib, c->label) == NIL(comp_data_t)) && 
          isalpha(c->label[0])) {
        (void) xv_set(comp_label, PANEL_VALUE,"",NULL);
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
        (void) xv_set(comp_list, PANEL_LIST_INSERT, listpos, PANEL_LIST_STRING, 
          listpos, message, NULL);       
        listpos++;
        c->text = util_strsav(text_buffer);
        FREE(text_buffer);
        textsw_reset(comp_text,0,0);
        comp_Ins(comp_lib, c);
        compPrint(c);
      }
    }
  }
  else
    if((strcmp((char *) xv_get(item, PANEL_LABEL_STRING), "Delete") == 0) &&
      (listpos > 0)) {
      len = 0;
      while(xv_get(comp_list, PANEL_LIST_SELECTED, len) == 0) len++;
      strcpy(message,(char *) xv_get(comp_list, PANEL_LIST_STRING, len));
      text_buffer = strtok(message," ");
      text_buffer = strtok(NULL," ");
      text_buffer = strtok(NULL," ");
      comp_lib = comp_Delete(comp_lib, text_buffer);
      (void) xv_set(comp_list, PANEL_LIST_DELETE, len, NULL);
      listpos--;
    }
    else
      if((strcmp((char *) xv_get(item, PANEL_LABEL_STRING),"View") == 0) &&
         (listpos > 0)) {
        Canvas view_canvas;
        len = 0;
        
        while(xv_get(comp_list, PANEL_LIST_SELECTED, len) == 0) len++;
        strcpy(message,(char *) xv_get(comp_list, PANEL_LIST_STRING, len));
        text_buffer = strtok(message," ");
        text_buffer = strtok(NULL," ");
        text_buffer = strtok(NULL," ");
        c = comp_Find(comp_lib, text_buffer);
        if(c != NIL(comp_data_t)) {
          cms = (Cms) xv_create((int)NULL, CMS,
            CMS_NAME, "view_component_colormap",
            CMS_SIZE, COLOR_SIZE + CMS_CONTROL_COLORS,
            CMS_CONTROL_CMS, TRUE,
            CMS_TYPE, XV_STATIC_CMS,
            CMS_COLORS, cms_colors,
            NULL);
            
          parser_frame = (Frame) xv_create(comp_edit_frame, FRAME,
            FRAME_LABEL, "View",
            XV_WIDTH, 400,
            XV_HEIGHT, 500,
            XV_SHOW, TRUE,            
            FRAME_SHOW_RESIZE_CORNER, FALSE,
            NULL);

          view_canvas = (Canvas) xv_create(parser_frame, CANVAS, 
            CANVAS_X_PAINT_WINDOW, TRUE,
            CANVAS_AUTO_SHRINK, FALSE,
            CANVAS_AUTO_EXPAND, FALSE,
            CANVAS_WIDTH, 400,
            CANVAS_HEIGHT, 470,
            CANVAS_RETAINED, FALSE,
            CANVAS_REPAINT_PROC, viewcomp_repaint_proc,
            WIN_CMS, cms,
            WIN_BACKGROUND_COLOR, BLACK,
            XV_KEY_DATA, VIEW_COMP_KEY, c,
            NULL);  

          parser_panel = (Panel) xv_create(parser_frame, PANEL,
            XV_Y, 470,
            NULL);                      
            
          (void) xv_create(parser_panel, PANEL_BUTTON,
            PANEL_LABEL_STRING, "Ok",
            PANEL_NOTIFY_PROC, button_view_comp,
            XV_KEY_DATA, VIEW_COMP_FRAME_KEY, parser_frame,
            XV_X, 180,
            NULL);
         
          vc_colors = (unsigned long *) xv_get(view_canvas, 
            WIN_X_COLOR_INDICES);
        }
      }
  return XV_OK;  
}

/**Function********************************************************************

  Synopsis           [Callback routine for the open lib commmand.]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static int
callback_olib(
  char *fname,
  int argc,
  char **argv)
{
Panel_item comp_list = (Panel_item) argv[0];
char message[80];
int type;
lsGen gen;
comp_data_t *c;

  if(fname != NIL(char)) {
    comp_LibFree(comp_lib);
    comp_lib = comp_LibLoad(fname);
    if(comp_lib != NULL) {
      listpos--;
      while(listpos >= 0) {
        (void) xv_set(comp_list, PANEL_LIST_DELETE, listpos, NULL);
        listpos--;
      }
      listpos++; 
      lsForeachItem(comp_lib, gen, c) {
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
        (void) xv_set(comp_list, PANEL_LIST_INSERT, listpos, 
           PANEL_LIST_STRING, listpos, message, NULL);       
        listpos++;
      }    
    }
  }

  FREE(argv);
  return 1;
}

/**Function********************************************************************

  Synopsis           [Callback function for the save lib command.]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static int
callback_slib(
  char *fname,
  int argc,
  char **argv)
{
  if((comp_lib != NULL) && (fname != NIL(char))) {
    comp_LibSave(comp_lib, fname);
  }
  return 1;
}

/**Function********************************************************************

  Synopsis           [Component editor menu.]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static void
menu_comp_edit(
  Menu menu,
  Menu_item menu_item)
{
Frame comp_edit_frame = (Frame) xv_get(menu, XV_KEY_DATA, COMP_EDIT_FRAME_KEY);
Textsw textsw = (Textsw) xv_get(menu, XV_KEY_DATA, COMP_TEXT_KEY);
Panel_item comp_list = (Panel_item) xv_get(menu, XV_KEY_DATA, COMP_LIST_KEY);
char **argv;

  if(strcmp((char *) xv_get(menu_item,MENU_STRING),"Quit") == 0) {
    intfCompeResetGlobals();
    xv_destroy_safe(comp_edit_frame);
    textsw_reset(textsw,0,0);
  }
  else
    if(strcmp((char *) xv_get(menu_item,MENU_STRING),"Open Lib") == 0) {
      if(fileshow == 0) return;
      fileshow = 0;
      argv = (char **) ALLOC(char *, 2);
      argv[0] = (char *) comp_list;
      intfOpenFiles(comp_edit_frame, "Open Component Library", "*.clib", 
        &fileshow, callback_olib, 1, argv);
    }
    else
      if(strcmp((char *) xv_get(menu_item,MENU_STRING),"Save Lib") == 0) {
        if(fileshow == 0) return;
        fileshow = 0;
        intfOpenFiles(comp_edit_frame, "Save Component Library", "*.clib",
          &fileshow, callback_slib, 0, NIL(char *));
      }
}
