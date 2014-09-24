/**CFile***********************************************************************

  FileName    [intfCommand.c]

  PackageName [intf]

  Synopsis    []

  Description []

  SeeAlso     []

  Author      []

  Copyright   []

******************************************************************************/
#include <xview/xview.h>
#include <xview/frame.h>
#include <xview/panel.h>
#include <xview/cms.h>
#include <xview/canvas.h>
#include <xview/xv_xrect.h>
#include <xview/tty.h>
#include <xview/termsw.h>
#include <stdio.h>
#include <ctype.h>
#include <math.h>

#undef va_start

#include "intfInt.h"

/*
static char rcsid[] = \"$Id: $\";
USE(rcsid);
*/

/*---------------------------------------------------------------------------*/
/* Variable declarations                                                     */
/*---------------------------------------------------------------------------*/

short exec_show = 1;
extern avl_tree *command_table;
extern FILE *msgout;
extern FILE *msgerr;
extern network_t *network;
extern array_t *llib;
extern Frame main_frame;
extern netl_clbl_t *main_clbl;
extern arch_graph_t *main_arch;


/**AutomaticStart*************************************************************/

/*---------------------------------------------------------------------------*/
/* Static function prototypes                                                */
/*---------------------------------------------------------------------------*/

static int exec_frame_done(Frame frame);
static void exec_menu_proc(Menu menu, Menu_item menu_item);
static int callback_sscript(char *fname, int argc, char **argv);
static int callback_oscript(char *fname, int argc, char **argv);
static void options_menu_proc(Menu menu, Menu_item menu_item);
static int close_exec(Panel_item item, Event *event);
static int add_exec(Panel_item item, Event *event);
static int del_exec(Panel_item item, Event *event);
static int run_exec(Panel_item item, Event *event);
static int help_proc(Panel_item item, Event *event);

/**AutomaticEnd***************************************************************/


/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis           [Insert a command.]

  Description        [optional]

  SideEffects        [required]

  SeeAlso            [optional]

******************************************************************************/
void
intf_ComAdd(
  char *name,
  PFI func,
  char *pkg,
  short owner)
{
command_t *new;

  new = (command_t *) ALLOC(command_t, 1);
  if(pkg == NIL(char)) {
    new->pkg = NIL(char);
  }
  else {
    new->pkg = util_strsav(pkg);
  }
  new->owner = owner;
  new->func = func;
  (void) avl_insert(command_table, util_strsav(name), (char *) new);
}

/**Function********************************************************************

  Synopsis           [Run the command using only a network.]

  Description        [optional]

  SideEffects        [required]

  SeeAlso            [optional]

******************************************************************************/
int
intf_ComSis(
  network_t **net,
  char *cmd)
{
char **argv = NULL, *pchar = NIL(char), *name;
int argc = 0, ret;
command_t *c;
char buf[1024];

  if(cmd[0] == '\0') return 1;
  
  pchar = strtok(cmd," ");
  name = util_strsav(pchar);
  argc++;
  argv = (char **) REALLOC(char *, argv, argc);
  argv[argc - 1] = util_strsav(name);  
  pchar = strtok(NULL," ");  
  while(pchar != NULL) {
    argc++;
    argv = (char **) REALLOC(char *, argv, argc);
    argv[argc - 1] = util_strsav(pchar);
    pchar = strtok(NULL," ");      
  }    
  ret = avl_lookup(command_table, name, &pchar);
  if(ret != 0) {
    c = (command_t *) pchar;  
    ret = (*c->func)(net, argc, argv);
  }           
  buf[0] = '\0';
  pchar = NULL;
  (void) fclose(msgout);
  msgout = fopen("/tmp/msgout","r");
  while(!feof(msgout)) {
    fgets(buf, 1024, msgout);
    if(pchar == NULL) {
      pchar = (char *) ALLOC(char, strlen(buf) + 1);
      strcpy(pchar, buf);
    }
    else {
      pchar = (char *) REALLOC(char, pchar, strlen(pchar) + strlen(buf) + 1);
      strcat(pchar, buf);
    }
    buf[0] = '\0';
  }
  (void) fclose(msgout);
  msgout = fopen("/tmp/msgout","w");
  buf[0] = '\0';  

  (void) fclose(msgerr);
  msgerr = fopen("/tmp/msgerr","r");
  while(!feof(msgerr)) {
    fgets(buf, 1024, msgerr);
    if(pchar == NULL) {
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
  
  FREE(pchar);
  FREE(name);
  return ret;
}

/**Function********************************************************************

  Synopsis           [Run a generic command.]

  Description        [optional]

  SideEffects        [required]

  SeeAlso            [optional]

******************************************************************************/
int
intf_Com(
  char *cmd,
  network_t **net,
  array_t **llib,
  netl_clbl_t **clbl,
  arch_graph_t **ag,
  short b)
{
char **argv = NIL(char *), *pchar, *name;
int argc = 0, ret;
command_t *c;
char buf[1024];
FILE *fp;

  if(cmd[0] == '\0') return 1;
  
  pchar = strtok(cmd," ");
  name = util_strsav(pchar);
  argc++;
  argv = (char **) REALLOC(char *, argv, argc);
  argv[argc - 1] = util_strsav(name);  
  pchar = strtok(NULL," ");  
  while(pchar != NULL) {
    argc++;
    argv = (char **) REALLOC(char *, argv, argc);
    argv[argc - 1] = util_strsav(pchar);
    pchar = strtok(NULL," ");      
  }    
  ret = avl_lookup(command_table, name, &pchar);
  if(ret != 0) {
    c = (command_t *) pchar;
    switch(c->owner) {
      case INTF_FROM_SIS: {
        ret = (*c->func)(net, argc, argv);
      } break;
      case INTF_FROM_MAP: {
        ret = (*c->func)(net, *llib, argc, argv);
      } break;
      case INTF_FROM_PLAC: {
        ret = (*c->func)(clbl, *ag, argc, argv);
      } break;        
      case INTF_FROM_ROUTING: {
        ret = (*c->func)(clbl, argc, argv);
      } break;        
      case INTF_GENERIC: {
        ret = (*c->func)(net, *llib, clbl, *ag, argc, argv);
      } break;
      case INTF_FROM_ARCH: {
        ret = (*c->func)(llib, ag, argc, argv);
      } break;
      case INTF_FROM_NETL: {
        ret = (*c->func)(clbl, argc, argv);
      } break;
      default:
    }
  }           
  buf[0] = '\0';
  pchar = NIL(char);
  (void) fclose(msgout);
  msgout = fopen("/tmp/msgout","r");
  while(!feof(msgout)) {
    fgets(buf, 1024, msgout);
    if(buf[0] != '\0') {
      if(pchar == NIL(char)) {
        pchar = (char *) ALLOC(char, strlen(buf) + 1);
        strcpy(pchar, buf);
      }  
      else {
        pchar = (char *) REALLOC(char, pchar, strlen(pchar) + strlen(buf) + 1);
        strcat(pchar, buf);
      }
    }
    buf[0] = '\0';
  }
  (void) fclose(msgout);
  msgout = fopen("/tmp/msgout","w");
  sisout = msgout;
  misout = sisout;

  buf[0] = '\0';  

  (void) fclose(msgerr);
  msgerr = fopen("/tmp/msgerr","r");
  while(!feof(msgerr)) {
    fgets(buf, 1024, msgerr);
    if(buf[0] != '\0') {
      if(pchar == NIL(char)) {
        pchar = (char *) ALLOC(char, strlen(buf) + 1);
        strcpy(pchar, buf);
      }
      else {
        pchar = (char *) REALLOC(char, pchar, strlen(pchar) + strlen(buf) + 1);
        strcat(pchar, buf);
      }
    }
    buf[0] = '\0';
  }
  (void) fclose(msgerr);
  msgerr = fopen("/tmp/msgerr","w");
  siserr = msgerr;
  miserr = siserr;
  
  if((pchar != NIL(char)) && (strcmp(pchar,"") != 0)) {
    if(b == 0) {
      intfDisplayMsgbox(main_frame, pchar);
    }
    else {
      fp = fopen("data.out","a");
      fprintf(fp,"%s",pchar);
      (void) fclose(fp);
    }  
  }

  if(pchar != NIL(char)) FREE(pchar);
  FREE(name);
  return ret;
}

/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis           [Initialize all the commands.]

  Description        [optional]

  SideEffects        [required]

  SeeAlso            [optional]

******************************************************************************/
void
intfCommandsInit()
{
  init_io();
  init_network();
  init_decomp();
  init_extract();
  init_factor();
  init_latch();
  init_simplify();
  init_resub();
  init_node();
  init_ntbdd();
  init_pld();

  dmig_Init();
  dagm_Init();
  levm_Init();
  place_Init();
  rou_Init();
  
  netl_Init();
  arch_Init(); 
}

/**Function********************************************************************

  Synopsis           [Terminate all commands.]

  Description        [optional]

  SideEffects        [required]

  SeeAlso            [optional]

******************************************************************************/
void
intfCommandsEnd()
{
  end_io();
  end_network();
  end_decomp();
  end_extract();
  end_factor();
  end_latch();
  end_simplify();
  end_resub();
  end_node();
  end_ntbdd();  
  end_pld();

  dmig_End();
  dagm_End();
  levm_End();  
  place_End();
  rou_End();
  
  arch_End();
  netl_End();
}

/**Function********************************************************************

  Synopsis           [Terminate all commands.]

  Description        [optional]

  SideEffects        [required]

  SeeAlso            [optional]

******************************************************************************/
void
intfExecProc(
  Panel_item item,
  Event *event)
{
Frame frame;
Panel panel;
Menu sis_menu, mapping_menu, placement_menu, routing_menu, options_menu;
Menu hercules_menu;
Panel_item text, list, add, del, run;
Menu_item mi;
static int exec_listpos;
avl_generator *gen;
char *value, *key, **pkg = NULL;
command_t *com;
int i, pkgsize = 0;
Menu *pkgm = NULL;

  if(exec_show == 0) return;
  exec_show = 0;
  exec_listpos = 0;
  
  frame = (Frame) xv_create(main_frame, FRAME,
    FRAME_LABEL, "Execute",
    XV_WIDTH, 400,
    XV_HEIGHT, 370,
    XV_SHOW, TRUE,
    FRAME_SHOW_RESIZE_CORNER, FALSE,
    FRAME_DONE_PROC, exec_frame_done,
    NULL);
   
  panel = (Panel) xv_create(frame, PANEL, NULL);

  sis_menu = (Menu) xv_create((int)NULL, MENU,
    NULL);

  mapping_menu = (Menu) xv_create((int)NULL, MENU,
    NULL);
    
  placement_menu = (Menu) xv_create((int)NULL, MENU,
    NULL);
    
  routing_menu = (Menu) xv_create((int)NULL, MENU,
    NULL);
    
  hercules_menu = (Menu) xv_create((int)NULL, MENU,
    NULL);

  avl_foreach_item(command_table, gen, AVL_FORWARD, &key, &value) {
    com = (command_t *) value;

    if((strcmp(key,"read_blif") == 0) ||
       (strcmp(key,"read_pla") == 0) ||
       (strcmp(key,"write_blif") == 0) ||
       (strcmp(key,"write_pla") == 0) ||
       (strcmp(key,"clbl_load") == 0)) continue;
    
    if(com->pkg != NIL(char)) {
      for(i = 0;i < pkgsize;i++)
        if(strcmp(pkg[i], com->pkg) == 0) break;
    }
  
    switch(com->owner) {
      case INTF_FROM_SIS: {
        if(i < pkgsize) {
          mi = (Menu_item) xv_create((int) NULL, MENUITEM,
            MENU_STRING, key,
            MENU_NOTIFY_PROC, exec_menu_proc,
            NULL);        
          (void) xv_set(pkgm[i], MENU_APPEND_ITEM, mi, NULL);
        }
        else {        
          pkgsize++;
          pkg = (char **) REALLOC(char *, pkg, pkgsize);
          pkg[pkgsize - 1] = util_strsav(com->pkg);
          pkgm = (Menu *) REALLOC(Menu, pkgm, pkgsize);

          pkgm[pkgsize - 1]= (Menu) xv_create((int)NULL, MENU,
            NULL);
        
          mi = (Menu_item) xv_create((int) NULL, MENUITEM,
            MENU_STRING, key,
            MENU_NOTIFY_PROC, exec_menu_proc,        
            NULL);        
          (void) xv_set(pkgm[pkgsize - 1], MENU_APPEND_ITEM, mi, NULL);
                    
          mi = (Menu_item) xv_create((int)NULL, MENUITEM,
            MENU_STRING, com->pkg,
            MENU_PULLRIGHT, pkgm[pkgsize - 1], NULL,            
            NULL);              
          (void) xv_set(sis_menu, MENU_APPEND_ITEM, mi, NULL);      
        }        
      } break;
      
      case INTF_FROM_MAP: {
        mi = (Menu_item) xv_create((int)NULL, MENUITEM,
          MENU_STRING, key,
          MENU_NOTIFY_PROC, exec_menu_proc,
          NULL);
        (void) xv_set(mapping_menu, MENU_APPEND_ITEM, mi, NULL);
      } break;

      case INTF_FROM_PLAC: {
        mi = (Menu_item) xv_create((int)NULL, MENUITEM,
          MENU_STRING, key,
          MENU_NOTIFY_PROC, exec_menu_proc,
          NULL);
        (void) xv_set(placement_menu, MENU_APPEND_ITEM, mi, NULL);
      } break;

      case INTF_FROM_ROUTING: {
        mi = (Menu_item) xv_create((int)NULL, MENUITEM,
          MENU_STRING, key,
          MENU_NOTIFY_PROC, exec_menu_proc,
          NULL);
        (void) xv_set(routing_menu, MENU_APPEND_ITEM, mi, NULL);
      } break;
      default: break;
    }
  }
  for(i = 0;i < pkgsize;i++) FREE(pkg[i]);
  FREE(pkg);

  (void) xv_create(panel, PANEL_BUTTON,
    PANEL_LABEL_STRING,  "Sis",
    PANEL_ITEM_MENU, sis_menu,
    XV_Y, 10,
    XV_X, 10,
    NULL); 
    
  (void) xv_create(panel, PANEL_BUTTON,
    PANEL_LABEL_STRING, "Mapping",
    PANEL_ITEM_MENU, mapping_menu,
    XV_Y, 10,
    XV_X, 115,
    NULL);

  (void) xv_create(panel, PANEL_BUTTON,
    PANEL_LABEL_STRING, "Placement",
    PANEL_ITEM_MENU, placement_menu,
    XV_X, 10,
    XV_Y, 40,
    NULL);

  (void) xv_create(panel, PANEL_BUTTON,
    PANEL_LABEL_STRING, "Routing",
    PANEL_ITEM_MENU, routing_menu,
    XV_Y, 40,
    NULL);
       
  options_menu = (Menu) xv_create((int) NULL, MENU,
    MENU_NOTIFY_PROC, options_menu_proc,
    MENU_STRINGS, "Open Script", "Save Script", NULL,
    XV_KEY_DATA, EXEC_FRAME_KEY, frame,
    NULL);       
    
  (void) xv_create(panel, PANEL_BUTTON,
    PANEL_LABEL_STRING, "Options",
    PANEL_ITEM_MENU, options_menu,
    XV_Y, 10, 
    XV_X, 210,
    NULL);
        
  (void) xv_create(panel, PANEL_BUTTON,
    PANEL_LABEL_STRING, "Close",
    PANEL_NOTIFY_PROC, close_exec,
    XV_Y, 10,
    XV_X, 300,
    XV_KEY_DATA, EXEC_FRAME_KEY, frame,
    NULL);

  text = xv_create(panel, PANEL_TEXT,
    PANEL_LABEL_STRING, "Procedure: ",
    PANEL_VALUE_DISPLAY_LENGTH, 35,
    XV_Y, 70,
    XV_X, 15,
    NULL);

  (void) xv_create(panel, PANEL_BUTTON,
    PANEL_LABEL_STRING, "Help",
    XV_X, 300,
    XV_Y, 40,
    PANEL_NOTIFY_PROC, help_proc,
    XV_KEY_DATA, EXEC_TEXT_KEY, text,
    XV_KEY_DATA, EXEC_FRAME_KEY, frame,    
    NULL);
  
  for(i = 0;i < pkgsize;i++) 
    (void) xv_set(pkgm[i], XV_KEY_DATA, EXEC_TEXT_KEY, text, NULL);

  (void) xv_set(mapping_menu, XV_KEY_DATA, EXEC_TEXT_KEY, text, NULL);
  (void) xv_set(placement_menu, XV_KEY_DATA, EXEC_TEXT_KEY, text, NULL);  
  (void) xv_set(routing_menu, XV_KEY_DATA, EXEC_TEXT_KEY, text, NULL);    
  (void) xv_set(hercules_menu, XV_KEY_DATA, EXEC_TEXT_KEY, text, NULL);

  list = (Panel_item) xv_create(panel, PANEL_LIST,
    PANEL_LIST_DISPLAY_ROWS, 10,
    XV_X, 10,
    XV_Y, 100,
    PANEL_LIST_ROW_HEIGHT, 20,
    PANEL_LIST_WIDTH, 360,   
    XV_KEY_DATA, EXEC_TEXT_KEY, text,
    NULL);

  (void) xv_set(options_menu, XV_KEY_DATA, EXEC_LIST_KEY, list, NULL);
  (void) xv_set(options_menu, XV_KEY_DATA, EXEC_LISTPOS_KEY, &exec_listpos,
    NULL);
    
  add = xv_create(panel, PANEL_BUTTON,
    PANEL_LABEL_STRING, "Add",
    PANEL_NOTIFY_PROC, add_exec,
    XV_KEY_DATA, EXEC_TEXT_KEY, text,
    XV_KEY_DATA, EXEC_LIST_KEY, list,
    XV_KEY_DATA, EXEC_LISTPOS_KEY, &exec_listpos,
    XV_X, 120,
    XV_Y, 330,
    NULL);

  del = xv_create(panel, PANEL_BUTTON,
    PANEL_LABEL_STRING, "Del",
    PANEL_NOTIFY_PROC, del_exec,
    XV_KEY_DATA, EXEC_LIST_KEY, list,
    XV_KEY_DATA, EXEC_LISTPOS_KEY, &exec_listpos,
    XV_X, 220,
    XV_Y, 330,
    NULL);

  run = xv_create(panel, PANEL_BUTTON,
    PANEL_LABEL_STRING, "Run",
    PANEL_NOTIFY_PROC, run_exec,
    XV_KEY_DATA, EXEC_LIST_KEY, list,
    XV_KEY_DATA, EXEC_LISTPOS_KEY, &exec_listpos,
    XV_KEY_DATA, EXEC_TEXT_KEY, text,
    XV_X, 170,
    XV_Y, 345,
    NULL);
}

/**Function********************************************************************

  Synopsis           [Remove unespected characters from a command to be run.]

  Description        [optional]

  SideEffects        [required]

  SeeAlso            [optional]

******************************************************************************/
char *
intfClearCmd(char *cmd)
{
int i = 0, a = 0;
char *tmp, *ret;

  if(cmd == NIL(char)) {
    return NIL(char);
  }  
  tmp = (char *) ALLOC(char, strlen(cmd) + 1);
  while((cmd[i] != '\0') && (cmd[i] != '\n')) {
    if(((cmd[i] == ' ') || (cmd[i] == '\t')) && (a == 0)) {
      i++;
    }
    else {    
      if(cmd[i] == '\t') {
        tmp[a] = ' ';
      }
      else {
        tmp[a] = cmd[i];
      }
      a++;
      i++;
    }
  }
  tmp[a] = '\0';

  ret = util_strsav(tmp);
  FREE(tmp);

  return ret;
}

/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis           [Release the exec frame.]

  Description        [optional]

  SideEffects        [required]

  SeeAlso            [optional]

******************************************************************************/
static int
exec_frame_done(Frame frame)
{
  exec_show = 1;

  (void) xv_destroy_safe(frame);
  return XV_OK;  
}

/**Function********************************************************************

  Synopsis           [Exec menu procedure.]

  Description        [optional]

  SideEffects        [required]

  SeeAlso            [optional]

******************************************************************************/
static void
exec_menu_proc(
  Menu menu,
  Menu_item menu_item)
{
Panel_item text = (Panel_item) xv_get(menu, XV_KEY_DATA, EXEC_TEXT_KEY);
char *func = NIL(char);

  func = (char *) xv_get(menu_item, MENU_STRING);
  (void) xv_set(text, PANEL_VALUE, func, NULL);
}

/**Function********************************************************************

  Synopsis           [Callback function from the save script menu option.]

  Description        [optional]

  SideEffects        [required]

  SeeAlso            [optional]

******************************************************************************/
static int
callback_sscript(
  char *fname,
  int argc,
  char **argv)
{
Panel_item list;
int i, *exec_listpos;
char *pchar;
FILE *fp;

  list = (Panel_item) argv[0];
  exec_listpos = (int *) argv[1];
  if((*exec_listpos) < 1) {
    return 1;
  }  
  fp = fopen(fname, "w");      
  for(i = 0;i < (*exec_listpos);i++) {
    pchar = (char *) xv_get(list, PANEL_LIST_STRING, i);
    if(pchar != NIL(char)) {
      fprintf(fp,"%s\n", pchar);
    }
  }  
  (void) fclose(fp);

  return 1;
}

/**Function********************************************************************

  Synopsis           [Callback procedure from the open script menu option.]

  Description        [optional]

  SideEffects        [required]

  SeeAlso            [optional]

******************************************************************************/
static int
callback_oscript(
  char *fname,
  int argc,
  char **argv)
{
Panel_item list;
int *exec_listpos;
char buf[80];
FILE *fp;

  list = (Panel_item) argv[0];
  exec_listpos = (int *) argv[1];

  (*exec_listpos)--;
  while((*exec_listpos) >= 0) {
    (void) xv_set(list, PANEL_LIST_DELETE, (*exec_listpos), NULL);  
    (*exec_listpos)--;
  }  
  (*exec_listpos)++;
  
  fp = fopen(fname, "r");
  while(feof(fp) == 0) {
    buf[0] = '\0';
    fgets(buf, 80, fp);
    if((buf != NIL(char)) && (buf[0] != '\0')) {
      buf[strlen(buf) - 1] = '\0';
      (void ) xv_set(list, PANEL_LIST_INSERT, (*exec_listpos), 
        PANEL_LIST_STRING, (*exec_listpos), buf, NULL);       
      (*exec_listpos)++; 
    }
  }
  (void) fclose(fp);

  return 1;
}

/**Function********************************************************************

  Synopsis           [Options menu procedure.]

  Description        [optional]

  SideEffects        [required]

  SeeAlso            [optional]

******************************************************************************/
static void
options_menu_proc(
  Menu menu,
  Menu_item menu_item)
{
Panel_item list = (Panel_item) xv_get(menu, XV_KEY_DATA, EXEC_LIST_KEY);
int *exec_listpos = (int *) xv_get(menu, XV_KEY_DATA,EXEC_LISTPOS_KEY);
Frame father_frame = (Frame) xv_get(menu, XV_KEY_DATA, EXEC_FRAME_KEY);
static short open_script = 1;
char **argv;

  if(strcmp((char *) xv_get(menu_item, MENU_STRING), "Open Script") == 0) {
    if(open_script == 0) return;
    open_script = 0;
    argv = (char **) ALLOC(char *, 2);
    argv[0] = (char *) list;
    argv[1] = (char *) exec_listpos;
    intfOpenFiles(father_frame, "Open Script", "*", &open_script,
      callback_oscript, 2, argv);
  }
  else 
    if(strcmp((char *) xv_get(menu_item, MENU_STRING), "Save Script") == 0) {
      if(open_script == 0) return;
      open_script = 0;
      argv = (char **) ALLOC(char *, 2);
      argv[0] = (char *) list;
      argv[1] = (char *) exec_listpos;
      intfOpenFiles(father_frame, "Save Script", "*", &open_script,
        callback_sscript, 2, argv);
    }
}

/**Function********************************************************************

  Synopsis           [Close button procedure.]

  Description        [optional]

  SideEffects        [required]

  SeeAlso            [optional]

******************************************************************************/
static int
close_exec(
  Panel_item item,
  Event *event)
{
Frame frame = (Frame) xv_get(item, XV_KEY_DATA, EXEC_FRAME_KEY);

  exec_show = 1;
  (void) xv_destroy_safe(frame);
  return XV_OK;  
}

/**Function********************************************************************

  Synopsis           [Add button procedure]

  Description        [optional]

  SideEffects        [required]

  SeeAlso            [optional]

******************************************************************************/
static int
add_exec(
  Panel_item item,
  Event *event)
{
Panel_item text = (Panel_item) xv_get(item, XV_KEY_DATA, EXEC_TEXT_KEY);
Panel_item list = (Panel_item) xv_get(item, XV_KEY_DATA, EXEC_LIST_KEY);
int *exec_listpos = (int *) xv_get(item, XV_KEY_DATA,EXEC_LISTPOS_KEY);
char *pchar = NIL(char);

  pchar = (char *) xv_get(text, PANEL_VALUE);
  if(strcmp(pchar,"") != 0) {
    (void ) xv_set(list, PANEL_LIST_INSERT, (*exec_listpos), PANEL_LIST_STRING,
        (*exec_listpos), pchar, NULL);       
    (*exec_listpos)++; 
  }

  return XV_OK;
}

/**Function********************************************************************

  Synopsis           [Del button procedure]

  Description        [optional]

  SideEffects        [required]

  SeeAlso            [optional]

******************************************************************************/
static int
del_exec(
  Panel_item item,
  Event *event)
{
int *exec_listpos = (int *) xv_get(item, XV_KEY_DATA, EXEC_LISTPOS_KEY);
Panel_item list = (Panel_item) xv_get(item, XV_KEY_DATA, EXEC_LIST_KEY);
int i = 0;

  if((*exec_listpos) > 0) {
    while(xv_get(list, PANEL_LIST_SELECTED, i) == 0) i++;
    (void) xv_set(list, PANEL_LIST_DELETE, i, NULL);
    (*exec_listpos)--;
  }
  return XV_OK;
}

/**Function********************************************************************

  Synopsis           [Run button procedure]

  Description        [optional]

  SideEffects        [required]

  SeeAlso            [optional]

******************************************************************************/
static int
run_exec(
  Panel_item item,
  Event *event)
{
int *exec_listpos = (int *) xv_get(item, XV_KEY_DATA, EXEC_LISTPOS_KEY);
Panel_item list = (Panel_item) xv_get(item, XV_KEY_DATA, EXEC_LIST_KEY);
Panel_item text = (Panel_item) xv_get(item, XV_KEY_DATA, EXEC_TEXT_KEY);
char *pchar = NIL(char), tmp[100], **cmds = NIL(char *);
int i, cmdsize = 0, a;

  if((*exec_listpos) > 0) {
    for(i = 0;i < (*exec_listpos);i++) {
      pchar = (char *) xv_get(list, PANEL_LIST_STRING, i);
      strcpy(tmp, pchar);      
      pchar = strtok(tmp,";");
      while(pchar != NIL(char)) {
        cmdsize++;
        cmds = (char **) REALLOC(char *, cmds, cmdsize);
        cmds[cmdsize - 1] = intfClearCmd(pchar);
        pchar = strtok(NULL,";");
      }
      for(a = 0;a < cmdsize;a++) {
        intf_Com(cmds[a], &network, &llib, &main_clbl, &main_arch, 0);
        FREE(cmds[a]);
      }
      if(cmds != NIL(char *)) FREE(cmds);
      cmdsize = 0;
      cmds = NIL(char *);
    }
  }
  else 
    if((*exec_listpos) == 0) {
      pchar = (char *) xv_get(text, PANEL_VALUE);
      strcpy(tmp, pchar);
      pchar = strtok(tmp,";");
      while(pchar != NIL(char)) {
        cmdsize++;
        cmds = (char **) REALLOC(char *, cmds, cmdsize);
        cmds[cmdsize - 1] = intfClearCmd(pchar);
        pchar = strtok(NULL,";");
      }
      for(i = 0;i < cmdsize;i++) {
        intf_Com(cmds[i], &network, &llib, &main_clbl, &main_arch, 0);
        FREE(cmds[i]);
      }
      if(cmds != NIL(char *)) FREE(cmds);
      (void) xv_set(text, PANEL_VALUE, "", NULL);
    } 

  return XV_OK;
}

/**Function********************************************************************

  Synopsis           [Help button procedure]

  Description        [optional]

  SideEffects        [required]

  SeeAlso            [optional]

******************************************************************************/
static int
help_proc(
  Panel_item item,
  Event *event)
{
Frame frame = (Frame) xv_get(item, XV_KEY_DATA, EXEC_FRAME_KEY);
Panel_item text = (Panel_item) xv_get(item, XV_KEY_DATA, EXEC_TEXT_KEY);
char *pchar, *p, buf[1024], *data = NIL(char);
int ret;
command_t *c;
FILE *fp;

  pchar = (char *) xv_get(text, PANEL_VALUE);
  if((pchar != NIL(char)) && (strcmp(pchar,"") != 0)) {
    p = strtok(pchar," ");
    ret = avl_lookup(command_table, p, &pchar);
    if(ret != 0) {
      c = (command_t *) pchar;
      switch(c->owner) {
        case INTF_FROM_SIS: {
          sprintf(buf,"/home/ivan/RCCE/help/sis/%s.fmt", p);
          fp = fopen(buf, "r");
          if(fp == NIL(FILE)) break;
          while(feof(fp) == 0) {
            fgets(buf, 1024, fp);
            if(buf[0] != '\0') {
              if(data == NIL(char)) {
                data = (char *) ALLOC(char, strlen(buf) + 1);
                strcpy(data, buf);
              }
              else {
                data = (char *) REALLOC(char, data, strlen(buf) +
                  strlen(data) + 1);
                strcat(data, buf);
              }                           
            }
          }
          intfDisplayMsgbox(frame, data);
          FREE(data);                    
          (void) fclose(fp);                    
        } break;
      }      
    }
  }  
  
  return XV_OK;
}
