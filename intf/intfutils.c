/**CFile***********************************************************************

  FileName    [intfUtils.c]

  PackageName [intf]

  Synopsis    [Some utilities.]

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
#include <xview/textsw.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include <stdio.h>

#undef va_start

#include "intfInt.h"

/*
static char rcsid[] = \"$Id: $\";
USE(rcsid);
*/

/*---------------------------------------------------------------------------*/
/* Variable declarations                                                     */
/*---------------------------------------------------------------------------*/
short *flag;
int directories_listpos, files_listpos;
Textsw msgtextsw;

/**AutomaticStart*************************************************************/

/*---------------------------------------------------------------------------*/
/* Static function prototypes                                                */
/*---------------------------------------------------------------------------*/

static void MsgDone(Frame frame);
static short Match(char *data, char *filter);
static void getdir(char *curdir, char ***dirs, char ***files, char *filter);
static int open_files_frame_done(Frame frame);
static int open_files_button(Panel_item item, Event *event);
static void directories_proc(Panel_item item, char *string, caddr_t client_data, Panel_list_op op, Event *event);
static void files_proc(Panel_item item, char *string, caddr_t client_data, Panel_list_op op, Event *event);

/**AutomaticEnd***************************************************************/


/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis           [Release the msg frame.]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
void
intfDisplayMsgbox(
  Frame father_frame,
  char *msg)
{
Frame frame;
FILE *fp;

  frame = (Frame) xv_create(father_frame, FRAME,
    FRAME_LABEL, "Message",
    FRAME_DONE_PROC, MsgDone,
    XV_WIDTH, 480,
    XV_HEIGHT, 400,
    XV_SHOW, TRUE,
    XV_X, 100,
    XV_Y, 200,
    NULL);
      
  msgtextsw = (Textsw) xv_create(frame, TEXTSW,
    XV_HEIGHT, 350,
    NULL);

  if(strlen(msg) > MAXBUFLEN) {
    fp = fopen("/tmp/msgboxout", "w");
    fprintf(fp,"%s", msg);
    fclose(fp);
    (void) xv_set(msgtextsw, TEXTSW_FILE, "/tmp/msgboxout", NULL);
    system("rm /tmp/msgboxout");
  }
  else {
    (void) textsw_insert(msgtextsw, msg, (int) strlen(msg));
  }
         
  window_fit(frame);
}

/**Function********************************************************************

  Synopsis           [Open file utility.]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
void
intfOpenFiles(
  Frame father_frame,
  char *label,
  char *filter,
  short *show_flag,
  PFI callback,
  int argc,
  char **argv)
{
Frame frame;
Panel panel;
Panel_item text;
Panel_item directories;
Panel_item files;
Panel_item filter_text;
char **d, **f;
int i;

  flag = show_flag;
  files_listpos = 0;
  directories_listpos = 0;

  frame = (Frame) xv_create(father_frame, FRAME,
    FRAME_LABEL, label,
    XV_WIDTH, 400,
    XV_HEIGHT, 390,
    XV_SHOW, TRUE,
    FRAME_SHOW_RESIZE_CORNER, FALSE,
    FRAME_DONE_PROC, open_files_frame_done,
    NULL);
   
  panel = (Panel) xv_create(frame, PANEL, NULL);

  filter_text = xv_create(panel, PANEL_TEXT,
    PANEL_LABEL_STRING, "Filter: ",
    PANEL_VALUE_DISPLAY_LENGTH, 30,
    PANEL_VALUE, filter,
    PANEL_LABEL_X, 10,
    PANEL_LABEL_Y, 10,
    NULL);
        
  text = xv_create(panel, PANEL_TEXT,
    PANEL_LABEL_STRING, "Filename: ",
    PANEL_VALUE_DISPLAY_LENGTH, 30,
    PANEL_LABEL_X, 10,
    PANEL_LABEL_Y, 335,
    NULL);

  directories = (Panel_item) xv_create(panel, PANEL_LIST,
    XV_X, 10,
    XV_Y, 55,
    PANEL_LIST_DISPLAY_ROWS, 12,
    PANEL_LIST_ROW_HEIGHT, 20,
    PANEL_LIST_WIDTH, 180,
    PANEL_NOTIFY_PROC, directories_proc,
    XV_KEY_DATA, FILTER_TEXT_KEY, filter_text, 
    NULL);   

  (void) xv_create(panel, PANEL_MESSAGE,
    PANEL_LABEL_STRING, "Directories",
    PANEL_LABEL_X, 10,
    PANEL_LABEL_Y, 40,
    NULL);
    
  files = (Panel_item) xv_create(panel, PANEL_LIST,
    PANEL_LIST_DISPLAY_ROWS, 12,
    XV_X, 220,
    XV_Y, 55,
    PANEL_LIST_ROW_HEIGHT, 20,
    PANEL_LIST_WIDTH, 160,
    XV_KEY_DATA, OPEN_FILES_TEXT_KEY, text, 
    PANEL_NOTIFY_PROC, files_proc,
    NULL);

  (void) xv_create(panel, PANEL_MESSAGE,
    PANEL_LABEL_STRING, "Files",
    PANEL_LABEL_X, 220,
    PANEL_LABEL_Y, 40,
    NULL);
    
  (void) xv_set(directories, XV_KEY_DATA, DIRECTORIES_LIST_KEY, directories,
    NULL);
  (void) xv_set(directories, XV_KEY_DATA, FILES_LIST_KEY, files, NULL);
  (void) xv_set(files, XV_KEY_DATA, FILES_LIST_KEY, files, NULL);

  getdir(".", &d, &f, filter);
  i = 0;
  while(d[i] != NULL) {
    (void) xv_set(directories, PANEL_LIST_INSERT, directories_listpos,
      PANEL_LIST_STRING, directories_listpos, d[i], NULL);
    directories_listpos++;
    FREE(d[i]);
    i++;
  }    
  FREE(d);
  i = 0;
  while(f[i] != NULL) {
    (void) xv_set(files, PANEL_LIST_INSERT, files_listpos,
      PANEL_LIST_STRING, files_listpos, f[i], NULL);
    files_listpos++;
    FREE(f[i]);
    i++;
  }    
  FREE(f);
   
  (void) xv_create(panel, PANEL_BUTTON,
    PANEL_LABEL_STRING, "Ok",
    XV_X, 110,
    XV_Y, 365,
    PANEL_NOTIFY_PROC, open_files_button,
    XV_KEY_DATA, OPEN_FILES_TEXT_KEY, text,
    XV_KEY_DATA, ARGC_OPEN_FILES_KEY, argc,
    XV_KEY_DATA, ARGV_OPEN_FILES_KEY, argv,
    XV_KEY_DATA, OPEN_FILES_FRAME_KEY, frame,
    XV_KEY_DATA, CALLBACK_FUNCTION_KEY, callback,
    XV_KEY_DATA, FILES_LIST_KEY, files,
    NULL);
       
  (void) xv_create(panel, PANEL_BUTTON,
    PANEL_LABEL_STRING, "Cancel",
    XV_X, 210,
    XV_Y, 365,
    PANEL_NOTIFY_PROC, open_files_button,
    XV_KEY_DATA, OPEN_FILES_FRAME_KEY, frame,      
    NULL);    
}

/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis           [Release the msg frame.]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static void
MsgDone(Frame frame)
{
  textsw_reset(msgtextsw, 0, 0);
  xv_destroy_safe(frame);
}

/**Function********************************************************************

  Synopsis           [Match a string with a filter, possibly using wildcards.]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static short
Match(
  char *data,
  char *filter)
{
int i = 0,j = 0;

  while((data[i] != '\0') && (filter[j] != '\0')) {
    if(filter[j] == '*') {
      j++;
      if(filter[j] == '\0') return 1;
      while((data[i] != '\0') && (filter[j] != data[i])) i++;
      if(data[i] == '\0') return 0;
      i++;
      j++;
    }
    if(data[i] != filter[j]) return 0;
    i++;
    j++;  
  }  
  if((data[i] == '\0') && (filter[j] == '\0')) return 1;
  
  return 0;  
}

/**Function********************************************************************

  Synopsis           [Read a directory and return the files, after filtered,
  and the directories.]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static void
getdir(
  char *curdir,
  char ***dirs,
  char ***files,
  char *filter)
{
struct dirent *d;
DIR *dir,*test;
int dsize = 0, fsize = 0;

  (*dirs) = NULL;
  (*files) = NULL;
  
  dir = opendir(curdir);
  d = readdir(dir);
  
  while(d != NIL(struct dirent)) {
    if(strcmp(d->d_name,".") != 0) {
      if(strcmp(d->d_name,"..") != 0) {
        test = opendir(d->d_name);
        if(test != NULL) {
          closedir(test);
          dsize++;
          (*dirs) = (char **) REALLOC(char *, (*dirs), dsize);
          (*dirs)[dsize - 1] = util_strsav(d->d_name);
        }
        else
          if(Match(d->d_name, filter) == 1) {          
            fsize++;
            (*files) = (char **) REALLOC(char *, (*files), fsize);
            (*files)[fsize - 1] = util_strsav(d->d_name);
          }
      }
      else {
        dsize++;
        (*dirs) = (char **) REALLOC(char *, (*dirs), dsize);
        (*dirs)[dsize - 1] = util_strsav("..");      
      }
    }
    d = readdir(dir);
  }  
  dsize++;
  (*dirs) = (char **) REALLOC(char *, (*dirs), dsize);
  (*dirs)[dsize - 1] = NULL;
  fsize++;
  (*files) = (char **) REALLOC(char *, (*files), fsize);
  (*files)[fsize - 1] = NULL;    
  
  closedir(dir);
}

/**Function********************************************************************

  Synopsis           [Release the open file frame.]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static int
open_files_frame_done(Frame frame)
{
  *flag = 1;
  (void) xv_destroy_safe(frame);
  return XV_OK;
}

/**Function********************************************************************

  Synopsis           [Handle the open file button.]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static int
open_files_button(
  Panel_item item,
  Event *event)
{
char filename[40];
Panel_item filename_text = (Panel_item) xv_get(item, XV_KEY_DATA, 
  OPEN_FILES_TEXT_KEY);
Frame father_frame = (Frame) xv_get(item, XV_KEY_DATA, OPEN_FILES_FRAME_KEY);
PFI callback = (PFI) xv_get(item, XV_KEY_DATA, CALLBACK_FUNCTION_KEY);
int argc = (int) xv_get(item, XV_KEY_DATA, ARGC_OPEN_FILES_KEY);
char **argv = (char **) xv_get(item, XV_KEY_DATA, ARGV_OPEN_FILES_KEY);
Panel_item files_list = (Panel_item) xv_get(item, XV_KEY_DATA, FILES_LIST_KEY);
int i = 0;

  if(strcmp((char *) xv_get(item,PANEL_LABEL_STRING),"Ok") == 0) {
    strcpy(filename,(char *) xv_get(filename_text,PANEL_VALUE));
    if((filename != NULL) && (strcmp(filename,"") != 0)) {
      xv_destroy_safe(father_frame);
      *flag = 1;
      callback(filename, argc, argv);
    }
    else {
      while(xv_get(files_list, PANEL_LIST_SELECTED, i) == 0) i++;
      strcpy(filename, (char *) xv_get(files_list, PANEL_LIST_STRING, i));      
      xv_destroy_safe(father_frame);
      *flag = 1;
      callback(filename, argc, argv);      
    }
  }
  else
    if(!strcmp((char *) xv_get(item,PANEL_LABEL_STRING),"Cancel")) {
      *flag = 1;
      xv_destroy_safe(father_frame);
    }

  return XV_OK;
}

/**Function********************************************************************

  Synopsis           [Procedure that handle when a directory is selected.]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static void
directories_proc(
  Panel_item item,
  char *string,
  caddr_t client_data,
  Panel_list_op op,
  Event *event)
{
char text[80];
Panel_item directories_list = (Panel_item) xv_get(item, XV_KEY_DATA, 
  DIRECTORIES_LIST_KEY);
Panel_item files_list = (Panel_item) xv_get(item, XV_KEY_DATA, FILES_LIST_KEY);
Panel_item filter_text = (Panel_item) xv_get(item, XV_KEY_DATA,
  FILTER_TEXT_KEY);
int i = 0;
char **d, **f;

  if(op) {
    while(xv_get(directories_list, PANEL_LIST_SELECTED, i) == 0) i++;
    strcpy(text, (char *) xv_get(directories_list, PANEL_LIST_STRING, i));
    chdir(text);
    strcpy(text, (char *) xv_get(filter_text, PANEL_VALUE, NULL));
    getdir(".", &d, &f, text);    
    while(directories_listpos >= 0) {
      (void) xv_set(directories_list, PANEL_LIST_DELETE, 
        directories_listpos, NULL);
      directories_listpos--;
    }
    while(files_listpos >= 0) {
      (void) xv_set(files_list, PANEL_LIST_DELETE, files_listpos, NULL);
      files_listpos--;
    }
    files_listpos = 0;
    directories_listpos = 0;
    i = 0;
    while(d[i] != NULL) {
      (void) xv_set(directories_list, PANEL_LIST_INSERT, directories_listpos,
        PANEL_LIST_STRING, directories_listpos, d[i], NULL);
      directories_listpos++;
      FREE(d[i]);
      i++;
    }    
    FREE(d);
    i = 0;
    while(f[i] != NULL) {
      (void) xv_set(files_list, PANEL_LIST_INSERT, files_listpos,
        PANEL_LIST_STRING, files_listpos, f[i], NULL);
      files_listpos++;
      FREE(f[i]);
      i++;
    }    
    FREE(f);
  }
}

/**Function********************************************************************

  Synopsis           [Procedure that handle when a file is selected.]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static void
files_proc(
  Panel_item item,
  char *string,
  caddr_t client_data,
  Panel_list_op op,
  Event *event)
{
Panel_item file_list = (Panel_item) xv_get(item, XV_KEY_DATA, FILES_LIST_KEY);
Panel_item filename_text = (Panel_item) xv_get(item, XV_KEY_DATA, 
  OPEN_FILES_TEXT_KEY);
int pos = 0;
char text[80];

  if(op) {
    while(xv_get(file_list, PANEL_LIST_SELECTED, pos) == 0) pos++;
    strcpy(text, (char *) xv_get(file_list, PANEL_LIST_STRING, pos));        
    (void) xv_set(filename_text, PANEL_VALUE, text, NULL);
  }
}
