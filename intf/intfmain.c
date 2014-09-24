/**CFile***********************************************************************

  FileName    [intfMain.c]

  PackageName [intf]

  Synopsis    []

  Description []

  SeeAlso     []

  Author      []

  Copyright   []

******************************************************************************/

#include <xview/xview.h>
#include <xview/frame.h>
#include <xview/canvas.h>
#include <xview/xv_xrect.h>
#include <xview/cms.h>
#include <xview/panel.h>
#include <xview/openmenu.h>
#include <xview/scrollbar.h>
#include <stdio.h>
#include <ctype.h>
#include <fcntl.h>

#undef va_start

#include "intfInt.h"

/*
static char rcsid[] = \"$Id: $\";
USE(rcsid);
*/

/*---------------------------------------------------------------------------*/
/* Variable declarations                                                     */
/*---------------------------------------------------------------------------*/
Canvas main_canvas;
unsigned long *main_colors;

network_t *network;
arch_graph_t *main_arch = NIL(arch_graph_t);
array_t *llib = NIL(array_t);
netl_clbl_t *main_clbl = NIL(netl_clbl_t);

avl_tree *command_table;

FILE *sisout;
FILE *siserr;
FILE *sishist;

FILE *msgout;
FILE *msgerr;

Frame main_frame;
Panel lower_panel;
int virx = 0, viry = 0;
short draw = DISPLAY_NONE;
draw_t *drawing = NIL(draw_t);
short open_model = 1;

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

static int main_frame_done(Frame subframe);
static void KeyFree(char *key);
static void main_menu_proc(Menu menu, Menu_item menu_item);
static int callback_rblif(char *fname, int argc, char **argv);
static int callback_rpla(char *fname, int argc, char **argv);
static int callback_wblif(char *fname, int argc, char **argv);
static int callback_wpla(char *fname, int argc, char **argv);
static int callback_main_oclbl(char *fname, int argc, char **argv);
static int callback_main_wclbl(char *fname, int argc, char **argv);
static int callback_main_oarch(char *fname, int argc, char **argv);
static void display_menu_proc(Menu menu, Menu_item menu_item);
static void CommandValueFree(char *value);
static void main_draw_proc(Event *event, Notify_arg arg, Window window);
static int lower_panel_buttons(Panel_item item, Event *event);
static void main_repaint_proc(Canvas canvas, Xv_Window paint_window, Display *dpy, Window xwin, Xv_xrectlist *xrects);

/**AutomaticEnd***************************************************************/


/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis           [Start Function]

  Description        [optional]

  SideEffects        [required]

  SeeAlso            [optional]

******************************************************************************/
void                                                            
main(
  int argc,
  char *argv[])
{
Cms cms;
Panel main_panel;
Menu file_menu;
Panel_item exec;
Menu display_menu;
Panel_item display_button;

  command_table = avl_init_table(strcmp);
  intfCommandsInit();
  
  msgout = fopen("/tmp/msgout","w");
  if(msgout == NIL(FILE)) {
    fprintf(stderr,"Coudn't create file in /tmp\n");
    exit(1);
  }
  sisout = msgout;
  msgerr = fopen("/tmp/msgerr","w");
  if(msgerr == NIL(FILE)) {
    fprintf(stderr,"Coudn't create file in /tmp\n");
    exit(1);
  }
  siserr = msgerr; 
  sishist = NIL(FILE);

  if((argc == 3) && (strcmp(argv[1],"-b") == 0)) {
    intfBatch(argv[2]);
  }
  
  (void) xv_init(XV_INIT_ARGC_PTR_ARGV, &argc, argv, NULL);

  cms = (Cms) xv_create(XV_NULL, CMS,
    CMS_NAME,        "main_colormap",
    CMS_SIZE,        COLOR_SIZE + CMS_CONTROL_COLORS,
    CMS_CONTROL_CMS, TRUE,
    CMS_TYPE,        XV_STATIC_CMS,
    CMS_COLORS,      cms_colors,    
    NULL);

  main_frame = (Frame) xv_create((int)NULL, FRAME,
    FRAME_LABEL,  "ICCE Floorplan",
    XV_WIDTH,  900,
    XV_HEIGHT, 600,
    FRAME_DONE_PROC, main_frame_done,
    NULL);

  main_panel = (Panel) xv_create(main_frame, PANEL, 
    XV_HEIGHT, 30,
    NULL);

  file_menu = (Menu) xv_create((int)NULL, MENU,
    MENU_NOTIFY_PROC,  main_menu_proc,
    MENU_STRINGS, "Read Blif", "Write Blif", "Read Pla",
      "Write Pla", "Read Clblist", "Write Clblist", "Read Arch", "Quit", 
      NULL,
    NULL);

  (void) xv_create(main_panel, PANEL_BUTTON,
    PANEL_LABEL_STRING, "File",
    PANEL_ITEM_MENU, file_menu,
    NULL); 
  
  exec = (Panel_item) xv_create(main_panel, PANEL_BUTTON,
    PANEL_LABEL_STRING, "Execute",
    PANEL_NOTIFY_PROC, intfExecProc,
    NULL);

  display_menu = (Menu) xv_create((int) NULL, MENU,
    MENU_STRINGS, "Network", "Clb List", NULL,
    MENU_NOTIFY_PROC, display_menu_proc,
    NULL);

  display_button = xv_create(main_panel, PANEL_BUTTON,
    PANEL_LABEL_STRING, "Display",
    PANEL_ITEM_MENU, display_menu,
    NULL);

  (void) xv_create(main_panel, PANEL_BUTTON,
    PANEL_LABEL_STRING,  "Arch Build",
    PANEL_NOTIFY_PROC,  intfDisplayArch,
    NULL);

  main_canvas = (Canvas) xv_create(main_frame, CANVAS, 
    CANVAS_REPAINT_PROC, main_repaint_proc,
    CANVAS_X_PAINT_WINDOW, TRUE,
    CANVAS_AUTO_SHRINK, TRUE,
    CANVAS_AUTO_EXPAND, TRUE,
    CANVAS_RETAINED, FALSE,
    WIN_CMS, cms,
    NULL);

  (void) xv_set(main_canvas,
    WIN_BACKGROUND_COLOR, GRAY2,
    WIN_FOREGROUND_COLOR, BLACK,
    NULL);
    
  (void) xv_set(canvas_paint_window(main_canvas),
    WIN_CONSUME_EVENTS,
      LOC_DRAG, WIN_MOUSE_BUTTONS, LOC_MOVE,
      NULL,
    WIN_EVENT_PROC, main_draw_proc,
    NULL);

  lower_panel = (Panel) xv_create(main_frame, PANEL,
    XV_Y, 570,
    XV_HEIGHT, 30,
    NULL);
    
  (void) xv_create(lower_panel, PANEL_BUTTON,
    PANEL_LABEL_STRING, "Dir",
    PANEL_NOTIFY_PROC, lower_panel_buttons,
    NULL);

  (void) xv_create(lower_panel, PANEL_BUTTON,
    PANEL_LABEL_STRING, "Esq",
    PANEL_NOTIFY_PROC, lower_panel_buttons,
    NULL);

  (void) xv_create(lower_panel, PANEL_BUTTON,
    PANEL_LABEL_STRING, "Up",
    PANEL_NOTIFY_PROC, lower_panel_buttons,
    NULL);

  (void) xv_create(lower_panel, PANEL_BUTTON,
    PANEL_LABEL_STRING, "Down",
    PANEL_NOTIFY_PROC, lower_panel_buttons,
    NULL);

  network = network_alloc();

  main_colors = (unsigned long *) xv_get(main_canvas, WIN_X_COLOR_INDICES);
  xv_main_loop(main_frame);
}

/**Function********************************************************************

  Synopsis           [Release the memory associated with global variables.]

  Description        [optional]

  SideEffects        [required]

  SeeAlso            [optional]

******************************************************************************/
void
intfMainResetGlobals(void)
{
  intfCompeResetGlobals();
  intfAbResetGlobals();
  intfArchResetGlobals();
  avl_free_table(command_table, KeyFree, CommandValueFree);
  network_free(network);
  if(main_arch != NIL(arch_graph_t)) {
    arch_Free(main_arch);
  }
  if(drawing != NIL(draw_t)) {
    draw_Free(drawing);
  }
  if(llib != NIL(array_t)) {
    llib_Free(llib);
  }
  if(main_clbl != NIL(netl_clbl_t)) {
    netl_ClblFree(main_clbl);
  }
  (void) fclose(msgout);
  (void) fclose(msgerr);
  system("rm /tmp/msgout /tmp/msgerr");

  intfCommandsEnd();
}

/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis           [Exit function.]

  Description        [optional]

  SideEffects        [required]

  SeeAlso            [optional]

******************************************************************************/
static int
main_frame_done(Frame subframe)
{

  intfCompeResetGlobals();
  intfAbResetGlobals();
  intfArchResetGlobals();
  intfMainResetGlobals();
  (void) xv_destroy_safe(subframe);

  return XV_OK;
}

/**Function********************************************************************

  Synopsis           [Release the memory associated with global variables.]

  Description        [optional]

  SideEffects        [required]

  SeeAlso            [optional]

******************************************************************************/
static void
KeyFree(char *key)
{
  FREE(key);
}

/**Function********************************************************************

  Synopsis           [The main menu procedure.]

  Description        [optional]

  SideEffects        [required]

  SeeAlso            [optional]

******************************************************************************/
static void
main_menu_proc(
  Menu menu,
  Menu_item menu_item)
{

  if(strcmp((char *) xv_get(menu_item,MENU_STRING),"Read Blif") == 0) {
    if(open_model == 0) return;
    open_model = 0;
    intfOpenFiles(main_frame, "Read Blif", "*.blif", &open_model,
      callback_rblif, 0, NIL(char*));
  }
  else  
    if(strcmp((char *) xv_get(menu_item,MENU_STRING),"Read Pla") == 0) {
      if(open_model == 0) return;
      open_model = 0;
      intfOpenFiles(main_frame, "Read Pla", "*.pla", &open_model,
        callback_rpla, 0, NIL(char*));
    }
    else
      if(strcmp((char *) xv_get(menu_item,MENU_STRING),"Quit") == 0) {
        intfCompeResetGlobals();
        intfAbResetGlobals();
        intfArchResetGlobals();
        intfMainResetGlobals();
       (void) xv_destroy_safe(main_frame);
      } 
      else
        if(strcmp((char *) xv_get(menu_item, MENU_STRING),"Write Blif") == 0) {
          if(open_model == 0) return;
          open_model = 0;
          intfOpenFiles(main_frame, "Write Blif", "*.blif", &open_model,
            callback_wblif, 0, NIL(char *));
        }
        else
          if(strcmp((char *) xv_get(menu_item, MENU_STRING),
            "Write Pla") == 0) {
            if(open_model == 0) return;
            open_model = 0;
            intfOpenFiles(main_frame, "Write Pla", "*.pla", &open_model,
              callback_wpla, 0, NIL(char *));
          }
          else
            if(strcmp((char *) xv_get(menu_item, MENU_STRING), 
              "Read Arch") == 0) {
              if(open_model == 0) return;
              open_model = 0;
              intfOpenFiles(main_frame, "Read Architecture", "*.arch", 
                &open_model, callback_main_oarch, 0, NIL(char *));
            }
            else
              if(strcmp((char *) xv_get(menu_item, MENU_STRING),
                "Read Clblist") == 0) {
                if(open_model == 0) return;
                open_model = 0;
                intfOpenFiles(main_frame, "Read Clblist", "*.mapped",
                  &open_model, callback_main_oclbl, 0, NIL(char *));
              }
              else
                if(strcmp((char *) xv_get(menu_item, MENU_STRING),
                  "Write Clblist") == 0) {
                  if(open_model == 0) return;
                  open_model = 0;
                  intfOpenFiles(main_frame, "Write Clblist", "*.mapped", 
                    &open_model, callback_main_wclbl, 0, NIL(char *));
                }

}  

/**Function********************************************************************

  Synopsis           [The callback function for the read blif command.]

  Description        [optional]

  SideEffects        [required]

  SeeAlso            [optional]

******************************************************************************/
static int
callback_rblif(
  char *fname,
  int argc,
  char **argv)
{
char *cmd;

  cmd = (char *) ALLOC(char, (int) strlen(fname) + 11);
  strcpy(cmd,"read_blif ");
  strcat(cmd, fname);  

  intf_ComSis(&network, cmd);

  FREE(cmd);
  return 1;
}

/**Function********************************************************************

  Synopsis           [The callback function for the read pla command.]

  Description        [optional]

  SideEffects        [required]

  SeeAlso            [optional]

******************************************************************************/
static int
callback_rpla(
  char *fname,
  int argc,
  char **argv)
{
char *cmd;

  cmd = (char *) ALLOC(char, (int) strlen(fname) + 10);
  strcpy(cmd,"read_pla ");
  strcat(cmd, fname);  
  intf_ComSis(&network, cmd);
  FREE(cmd);
  
  return 1;
}

/**Function********************************************************************

  Synopsis           [The callback function for the write blif command.]

  Description        [optional]

  SideEffects        [required]

  SeeAlso            [optional]

******************************************************************************/
static int
callback_wblif(
  char *fname,
  int argc,
  char **argv)
{
char *cmd;

  cmd = (char *) ALLOC(char, strlen(fname) + 12);
  strcpy(cmd,"write_blif ");
  strcat(cmd, fname);  
  intf_ComSis(&network, cmd);
  FREE(cmd);
  
  return 1;
}

/**Function********************************************************************

  Synopsis           [The callback function for the write pla command.]

  Description        [optional]

  SideEffects        [required]

  SeeAlso            [optional]

******************************************************************************/
static int
callback_wpla(
  char *fname,
  int argc,
  char **argv)
{
char *cmd;

  cmd = (char *) ALLOC(char, (int) strlen(fname) + 11);
  strcpy(cmd,"write_pla ");
  strcat(cmd, fname);  
  intf_ComSis(&network, cmd);
  FREE(cmd);
  
  return 1;
}

/**Function********************************************************************

  Synopsis           [The callback function for the open clblist command.]

  Description        [optional]

  SideEffects        [required]

  SeeAlso            [optional]

******************************************************************************/
static int
callback_main_oclbl(
  char *fname,
  int argc,
  char **argv)
{
  if(main_clbl != NIL(netl_clbl_t)) {
    netl_ClblFree(main_clbl);
  }
  netl_ClblLoad(&main_clbl, fname);
  
  return 1;
}

/**Function********************************************************************

  Synopsis           [The callback function for the write clblist command.]

  Description        [optional]

  SideEffects        [required]

  SeeAlso            [optional]

******************************************************************************/
static int
callback_main_wclbl(
  char *fname,
  int argc,
  char **argv)
{
  netl_ClblSave(main_clbl, fname);  
  return 1;
}

/**Function********************************************************************

  Synopsis           [The callback function for the read arch command.]

  Description        []

  SideEffects        [required]

  SeeAlso            [optional]

******************************************************************************/
static int
callback_main_oarch(
  char *fname,
  int argc,
  char **argv)
{
char buf[1024];
char *pchar = NIL(char);

  if(fname != NIL(char)) {
    if(main_arch != NIL(arch_graph_t)) {
      arch_Free(main_arch);
    }
    main_arch = arch_Read(fname, &llib);
  }
  buf[0] = '\0';  
  (void) fclose(msgerr);
  msgerr = fopen("/tmp/msgerr","r");
  while(!feof(msgerr)) {
    fgets(buf, 1024, msgerr);
    if(pchar == NIL(char)) {
      pchar = (char *) ALLOC(char, strlen(buf) + 1);
      strcpy(pchar, buf);
    }
    else {
      pchar = (char *) REALLOC(char, pchar, strlen(pchar) + strlen(buf) + 1);
      strcat(pchar, buf);
    }
    buf[0] = '\0';
  }
  (void) fclose(msgerr);
  msgerr = fopen("/tmp/msgerr","w");  
  if((pchar != NIL(char)) && (strcmp(pchar,"") != 0)) {
    intfDisplayMsgbox(main_frame, pchar);
  }  
  if(pchar != NIL(char)) FREE(pchar);
  
     
  return 1;      
}

/**Function********************************************************************

  Synopsis           [Select a structure two display, and display it.]

  Description        [optional]

  SideEffects        [required]

  SeeAlso            [optional]

******************************************************************************/
static void
display_menu_proc(
  Menu menu,
  Menu_item menu_item)
{

  if(!strcmp((char *) xv_get(menu_item, MENU_STRING),"Network")) {
    if(draw == DISPLAY_NETWORK) {
      draw = DISPLAY_NONE;
      if(drawing != NIL(draw_t)) {
        draw_Free(drawing);
      }
      drawing = NIL(draw_t);
      XClearWindow((Display *) xv_get(main_canvas, XV_DISPLAY),
        (Window) xv_get(canvas_paint_window(main_canvas),XV_XID));
    }
    else {      
      if(drawing != NIL(draw_t)) {
        draw_Free(drawing);
      }
      drawing = draw_Net(network,
       (int) xv_get(canvas_paint_window(main_canvas), XV_WIDTH),
       (int) xv_get(canvas_paint_window(main_canvas), XV_HEIGHT));  
      if(drawing != NIL(draw_t)) {
        drawing->x = virx;
        drawing->y = viry;
        draw_Update(main_canvas, drawing);
        draw = DISPLAY_NETWORK;        
      }
    }
  }
  else
    if(strcmp((char *) xv_get(menu_item, MENU_STRING), "Clb List") == 0) {
      if(drawing != NIL(draw_t)) {
        draw_Free(drawing);      
      }    
      drawing = draw_Clbl(main_clbl,
       (int) xv_get(canvas_paint_window(main_canvas), XV_WIDTH),
       (int) xv_get(canvas_paint_window(main_canvas), XV_HEIGHT));  
      if(drawing != NIL(draw_t)) {
        drawing->x = virx;
        drawing->y = viry;
        draw_Update(main_canvas, drawing);
        draw = DISPLAY_CLBL;
      }      
    }

/*
void main_repaint_proc();

  if(!strcmp((char *) xv_get(menu_item, MENU_STRING),"Network") && 
     (net != NULL)) { 
    destroy_net_draw(nd);
    nd = NULL;
    nd = create_network_draw(nd, net);
    display_opt = DISPLAY_NETWORK;
  
    main_repaint_proc(main_canvas, canvas_paint_window(main_canvas),
      xv_get(main_canvas ,XV_DISPLAY), 
      xv_get(canvas_paint_window(main_canvas), XV_XID), NULL); 
  }
  else
    if(!strcmp((char *) xv_get(menu_item, MENU_STRING),"Nodes") &&
       (net != NULL)) {
      
    }
    else
      if(!strcmp((char *) xv_get(menu_item, MENU_STRING),"Statistics") &&
          (net != NULL)) {
        display_opt = DISPLAY_STATISTICS;
  
        main_repaint_proc(main_canvas, canvas_paint_window(main_canvas),
          xv_get(main_canvas ,XV_DISPLAY), 
          xv_get(canvas_paint_window(main_canvas), XV_XID), NULL);         
      }
*/
}

/**Function********************************************************************

  Synopsis           [Release the memory associated with a command_t structure.]

  Description        [optional]

  SideEffects        [required]

  SeeAlso            [optional]

******************************************************************************/
static void
CommandValueFree(char *value)
{
/* command_t *c;

  c = (command_t *) value;
  if(c->pkg != NIL(char)) FREE(c->pkg);
  FREE(c);*/
}

/**Function********************************************************************

  Synopsis           [Upon an event on the main_canvas, run this procedure.]

  Description        [optional]

  SideEffects        [required]

  SeeAlso            [optional]

******************************************************************************/
static void
main_draw_proc(
  Event *event,
  Notify_arg arg,
  Window window)
{
int i;

  switch(event_action(event)) {
    case WIN_RESIZE: {
      i = (int) xv_get(main_frame, XV_HEIGHT, NULL);
      (void) xv_set(lower_panel, XV_Y, i - 30, NULL);
    } break;
    default:  
  }
}

/**Function********************************************************************

  Synopsis           [Handle some buttons.]

  Description        [optional]

  SideEffects        [required]

  SeeAlso            [optional]

******************************************************************************/
static int
lower_panel_buttons(
  Panel_item item,
  Event *event)
{
char *label;

  if((draw != DISPLAY_NONE) && (drawing != NIL(draw_t))) {
    label = (char *) xv_get(item, PANEL_LABEL_STRING, NULL);
    if(strcmp(label,"Dir") == 0) {
      virx += 10;   
    }
    else
      if(strcmp(label,"Esq") == 0) {
        virx -= 10; 
      }
      else
        if(strcmp(label,"Up") == 0) {
          viry -= 10; 
        }
        else 
          if(strcmp(label,"Down") == 0) {
            viry += 10; 
          }

    drawing->x = virx;
    drawing->y = viry;
    draw_Update(main_canvas, drawing);
  }
      
  return XV_OK;
}

/**Function********************************************************************

  Synopsis           [Responsible for repainting the main canvas.]

  Description        [optional]

  SideEffects        [required]

  SeeAlso            [optional]

******************************************************************************/
static void
main_repaint_proc(
  Canvas canvas,
  Xv_Window paint_window,
  Display *dpy,
  Window xwin,
  Xv_xrectlist *xrects)
{
/*
pinpos *pins;
layer *tmp;
element *data;  
Rect *rec;
net_draw *aux;
GC gc;
int i,x1,y1,x2,y2;
connect *links;
pTH xt,yt;
netlist *netaux;
signet *snet;

  gc = DefaultGC(dpy, DefaultScreen(dpy));
  XClearWindow(dpy,xwin);
  main_colors = (unsigned long *) xv_get(main_canvas, WIN_X_COLOR_INDICES);

  if((display_opt == DISPLAY_NETWORK) && (nd != NULL)) {
    (void) xv_set(main_canvas, WIN_BACKGROUND_COLOR, GRAY2, 0); 
    aux = nd;
    while(aux != NULL) {
      if(aux->link != NULL) {
        XPoint verts[3];
        double angle;
        int dx, dy, iangle;

        XSetLineAttributes(dpy, gc, 1, LineSolid, CapButt, JoinMiter);
        XSetForeground(dpy, gc, main_colors[BLACK]);
        XDrawLine(dpy, xwin, gc, aux->x, aux->y, aux->link->x2, aux->link->y2);

        dy = aux->link->y2 - aux->y;
        dx = aux->link->x2 - aux->x;
        if (ABS(dx) < ARROW_LENGTH && ABS(dy) < ARROW_LENGTH) return;
        angle = (dx == 0 && dy == 0) ? 0.0 : atan2((double)-dy,(double)dx);

        if (aux->x == aux->link->x2) {
          iangle = (int) ceil(angle * 180. * 64. / 3.1415926);
          XFillArc(dpy,xwin,gc,
		      aux->x - ARROW_LENGTH, aux->y - ARROW_LENGTH,
		      (unsigned int) 2 * ARROW_LENGTH,
		      (unsigned int) 2* ARROW_LENGTH,
		      iangle-18*64,(18+18)*64);
        } 
        else {
  	  verts[0].x = aux->x + ceil(ARROW_LENGTH * cos(angle - ARROW_ANGLE));
	  verts[0].y = aux->y - ceil(ARROW_LENGTH * sin(angle - ARROW_ANGLE));
  	  verts[1].x = aux->x;
  	  verts[1].y = aux->y;
	  verts[2].x = aux->x + ceil(ARROW_LENGTH * cos(angle + ARROW_ANGLE));
	  verts[2].y = aux->y - ceil(ARROW_LENGTH * sin(angle + ARROW_ANGLE));
	  XDrawLines(dpy,xwin,gc,verts,3,CoordModeOrigin);
        }        
      }
      else
        if((aux->nnode->link->fanin == NULL) || 
           (aux->nnode->link->fanout == NULL)) {
          XSetLineAttributes(dpy, gc, 2, LineSolid, CapButt, JoinMiter);
          XSetForeground(dpy, gc, main_colors[RED]);
          XDrawRectangle(dpy, xwin, gc, aux->x, aux->y, 
           (unsigned int) aux->nnode->width,(unsigned int) aux->nnode->height);
        }
        else
          if(aux->nnode->link->clk != NULL) {
            XSetLineAttributes(dpy, gc, 1, LineSolid, CapButt, JoinMiter);
            XSetForeground(dpy, gc, main_colors[BLUE]);
            XFillRectangle(dpy, xwin, gc, aux->x, aux->y, 
            (unsigned int) aux->nnode->width,(unsigned int) aux->nnode->height);               
          }       
          else {
            XSetLineAttributes(dpy, gc, 1, LineSolid, CapButt, JoinMiter);
            XSetForeground(dpy, gc, main_colors[BLUE]);
            XFillArc(dpy, xwin, gc, aux->x, aux->y,
            (unsigned int) aux->nnode->width,(unsigned int) aux->nnode->height,
             0, 360*64);        
         }
      aux = aux->next;
    }
  }   
  
  if((display_opt == DISPLAY_TOPOLOGY) && (main_arch != NULL)) {

    rec = (Rect *) xv_get(main_canvas, CANVAS_VIEWABLE_RECT, 
      paint_window, NULL);       
    XSetLineAttributes(dpy, gc, 2, LineSolid, CapButt, JoinMiter);      
    (void) xv_set(main_canvas, WIN_BACKGROUND_COLOR, BLACK, 0); 
    tmp = main_arch->l;
    while((tmp != NULL) && (tmp->n != 0)) tmp = tmp->next;
    if(tmp != NULL) {
      data = tmp->data;
      while(data != NULL) {
        if(data->comp && (in_rect(data->x + data->comp->fig->width,
          data->y + data->comp->fig->height,rec->r_left, 
          rec->r_top, rec->r_width,rec->r_height) ||
          in_rect(data->x, data->y, rec->r_left, rec->r_top, rec->r_width,
          rec->r_height) )) {
          XSetForeground(dpy, gc, main_colors[RED]);
          XDrawRectangle(dpy, xwin, gc, data->x, data->y, 
            data->comp->fig->width, data->comp->fig->height);
          pins = data->comp->fig->pins;
          XSetForeground(dpy,gc,main_colors[AQUA]);
          while(pins != NULL) {          
            XDrawLine(dpy, xwin, gc, data->x + pins->dx1,
              data->y + pins->dy1, data->x + pins->dx2, 
              data->y + pins->dy2);
            pins = pins->next;          
          }
        }
        else
          if(data->seg && (in_rect(data->x,data->y,rec->r_left, rec->r_top,
            rec->r_width,rec->r_height) || in_rect(data->seg->x, data->seg->y,
            rec->r_left, rec->r_top, rec->r_width, rec->r_height))) {
              XSetForeground(dpy, gc, main_colors[AQUA]);
              XDrawLine(dpy, xwin, gc, data->x, data->y, data->seg->x,
                data->seg->y);
          }
        data = data->next;      
      }      
    } 
    for(i = 0;i < npins;i++) {
      pinxy(apins[i].component, apins[i].pin, main_arch, 
        &x1, &y1);
      links = apins[i].links;
      while(links != NULL) {
        pinxy(apins[links->index].component, 
        apins[links->index].pin, main_arch, &x2, &y2);
        if((apins[i].type != ROUTING) ||
           (apins[links->index].type != ROUTING)) {
          XDrawLine(dpy, xwin, gc, x1, y1, x2, y2);
        }           
        links = links->next;
      }      
    }
  }
  
  if((display_opt == DISPLAY_NETLIST) && (main_arch != NULL) && (netl != NULL)) {
    rec = (Rect *) xv_get(main_canvas, CANVAS_VIEWABLE_RECT, 
      paint_window, NULL);       
    XSetLineAttributes(dpy, gc, 2, LineSolid, CapButt, JoinMiter);      
    (void) xv_set(main_canvas, WIN_BACKGROUND_COLOR, BLACK, 0); 

    tmp = main_arch->l;
    while((tmp != NULL) && (tmp->n != 0)) tmp = tmp->next;    

    xt = createTH(tmp->ncomp);
    yt = createTH(tmp->ncomp);
    
    if(tmp != NULL) {
      data = tmp->data;
      while(data != NULL) {
        if(data->comp && (in_rect(data->x + data->comp->fig->width,
          data->y + data->comp->fig->height,rec->r_left, 
          rec->r_top, rec->r_width,rec->r_height) ||
          in_rect(data->x, data->y, rec->r_left, rec->r_top, rec->r_width,
          rec->r_height) )) {
          XSetForeground(dpy, gc, main_colors[RED]);
          XDrawRectangle(dpy, xwin, gc, data->x, data->y, 
            data->comp->fig->width, data->comp->fig->height);
          pins = data->comp->fig->pins;
          XSetForeground(dpy,gc,main_colors[AQUA]);
          while(pins != NULL) {          
            XDrawLine(dpy, xwin, gc, data->x + pins->dx1,
              data->y + pins->dy1, data->x + pins->dx2, 
              data->y + pins->dy2);
            pins = pins->next;          
          }       
          addTH(xt, namealloc(data->comp->label), 
            (int) data->x + data->comp->fig->width/2);
          addTH(yt, namealloc(data->comp->label), 
            (int) data->y + data->comp->fig->height/2);          
        }
        data = data->next;      
      }      
    } 
    netaux = netl;
    while(netaux != NULL) {
      snet = netaux->snet;
      do {
        x1 = searchTH(xt, namealloc(snet->clb->name));
        y1 = searchTH(yt, namealloc(snet->clb->name));
        snet = snet->next;
      } while( ((x1 == -1) || (y1 == -1)) && (snet != NULL));      
      while(snet != NULL) {
        do {    
          if(snet == NULL) break;              
          x2 = searchTH(xt, namealloc(snet->clb->name));
          y2 = searchTH(yt, namealloc(snet->clb->name));
          snet = snet->next;
        } while((x2 == -1) || (y2 == -1));
        if((x1 != -1) && (y1 != -1) && (x2 != -1) && (y2 != -1)) {
          XDrawLine(dpy, xwin, gc, x1, y1, x2, y2);
          x1 = x2;
          y1 = y2;                                    
        }
      }      
      netaux = netaux->next;        
    }        
    destroyTH(xt);
    destroyTH(yt);
  }  
  
  if((display_opt == DISPLAY_STATISTICS) && (net != NULL)) {
    char message[256];
    int inputs[8];
    node_list *aux;
    
    sprintf(message,"Network %s",net->name);
    XDrawString(dpy, xwin, gc, 20, 20, message, strlen(message));
    sprintf(message,"Primary Inputs = %d",net->npi);
    XDrawString(dpy, xwin, gc,  20, 35, message, strlen(message));
    sprintf(message,"Primary Outputs = %d",net->npo);
    XDrawString(dpy, xwin, gc,  20, 50, message, strlen(message));
    sprintf(message,"Network Nodes = %d",net->nnodes);
    XDrawString(dpy, xwin, gc,  20, 65, message, strlen(message));
    inputs[0] = 0;
    inputs[1] = 0;
    inputs[2] = 0;
    inputs[3] = 0;
    inputs[4] = 0;                
    inputs[5] = 0;
    inputs[6] = 0;                
    inputs[7] = 0;                    
    aux = net->nlist;
    while(aux != NULL) {
      switch(aux->pnode->F->csize/2) {
        case 1 : inputs[0]++; break;
        case 2 : inputs[1]++; break;
        case 3 : inputs[2]++; break;
        case 4 : inputs[3]++; break;                      
        case 5 : inputs[4]++; break;
        case 6 : inputs[5]++; break;
        case 7 : inputs[6]++; break;
        default: inputs[7]++; break;
      }    
      aux = aux->next;
    }
    sprintf(message,"1 Input = %d",inputs[0]);
    XDrawString(dpy, xwin, gc, 20, 80, message, strlen(message));    
    sprintf(message,"2 Input = %d",inputs[1]);
    XDrawString(dpy, xwin, gc, 20, 95, message, strlen(message));    
    sprintf(message,"3 Input = %d",inputs[2]);
    XDrawString(dpy, xwin, gc, 20, 110, message, strlen(message));    
    sprintf(message,"4 Input = %d",inputs[3]);
    XDrawString(dpy, xwin, gc, 20, 125, message, strlen(message));    
    sprintf(message,"5 Input = %d",inputs[4]);
    XDrawString(dpy, xwin, gc, 20, 140, message, strlen(message));    
    sprintf(message,"6 Input = %d",inputs[5]);
    XDrawString(dpy, xwin, gc, 20, 155, message, strlen(message));    
    sprintf(message,"7 Input = %d",inputs[6]);
    XDrawString(dpy, xwin, gc, 20, 170, message, strlen(message));     
    sprintf(message,">7 Input = %d",inputs[7]);
    XDrawString(dpy, xwin, gc, 20, 185, message, strlen(message));     
  }
*/
}

/*
void
main_redraw_proc(canvas, paint_window, dpy, xwin, xrects)
Canvas canvas;
Xv_Window paint_window;
Display *dpy;
Window xwin;
Xv_xrectlist *xrects;
{
pinpos *pins;
layer *tmp;
element *data;  
Rect *rec;
net_draw *aux;
GC gc;
int i,x1,y1,x2,y2;
connect *links;
pTH xt,yt;
netlist *netaux;
signet *snet;

  gc = DefaultGC(dpy, DefaultScreen(dpy));
  (void) xv_set(main_canvas, WIN_BACKGROUND_COLOR, GRAY, 0); 

  if((display_opt == 3) && (main_arch != NULL) && (netl != NULL)) {        
    rec = (Rect *) xv_get(main_canvas, CANVAS_VIEWABLE_RECT, 
      paint_window, NULL);       
    XSetLineAttributes(dpy, gc, 2, LineSolid, CapButt, JoinMiter);      
    (void) xv_set(main_canvas, WIN_BACKGROUND_COLOR, BLACK, 0); 

    tmp = main_arch->l;
    while((tmp != NULL) && (tmp->n != 0)) tmp = tmp->next;    

    xt = createTH(tmp->ncomp);
    yt = createTH(tmp->ncomp);
    
    if(tmp != NULL) {
      data = tmp->data;
      while(data != NULL) {
        if(data->comp && (in_rect(data->x + data->comp->fig->width,
          data->y + data->comp->fig->height,rec->r_left, 
          rec->r_top, rec->r_width,rec->r_height) ||
          in_rect(data->x, data->y, rec->r_left, rec->r_top, rec->r_width,
          rec->r_height) )) {
          XSetForeground(dpy, gc, main_colors[RED]);
          XDrawRectangle(dpy, xwin, gc, data->x, data->y, 
            data->comp->fig->width, data->comp->fig->height);
          pins = data->comp->fig->pins;
          XSetForeground(dpy,gc,main_colors[AQUA]);
          while(pins != NULL) {          
            XDrawLine(dpy, xwin, gc, data->x + pins->dx1,
              data->y + pins->dy1, data->x + pins->dx2, 
              data->y + pins->dy2);
            pins = pins->next;          
          }       
          addTH(xt, namealloc(data->comp->label), 
            (int) data->x + data->comp->fig->width/2);
          addTH(yt, namealloc(data->comp->label), 
            (int) data->y + data->comp->fig->height/2);          
        }
        data = data->next;      
      }      
    } 
    netaux = netl;
    while(netaux != NULL) {
      snet = netaux->snet;
      do {
        x1 = searchTH(xt, namealloc(snet->clb->name));
        y1 = searchTH(yt, namealloc(snet->clb->name));
        snet = snet->next;
      } while( ((x1 == -1) || (y1 == -1)) && (snet != NULL));      
      while(snet != NULL) {
        do {    
          if(snet == NULL) break;              
          x2 = searchTH(xt, namealloc(snet->clb->name));
          y2 = searchTH(yt, namealloc(snet->clb->name));
          snet = snet->next;
        } while((x2 == -1) || (y2 == -1));
        if((x1 != -1) && (y1 != -1) && (x2 != -1) && (y2 != -1)) {
          XDrawLine(dpy, xwin, gc, x1, y1, x2, y2);
          x1 = x2;
          y1 = y2;                                    
        }
      }
      netaux = netaux->next;        
    }        
    destroyTH(xt);
    destroyTH(yt);
  }    
}  
*/
