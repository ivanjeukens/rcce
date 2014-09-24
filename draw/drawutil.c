/**CFile***********************************************************************

  FileName    [drawUtil.c]

  PackageName [draw]

  Synopsis    [Basic procedures for creating a drawing.]

  Description []

  SeeAlso     []

  Author      [Ivan Jeukens]

  Copyright   [1996-1997 Ivan Jeukens]

******************************************************************************/

#include <xview/frame.h>
#include <xview/xv_xrect.h>
#include <xview/cms.h>
#undef va_start
#include "drawInt.h"


/*
static char rcsid[] = \"$Id: $\";
USE(rcsid);
*/

/**AutomaticStart*************************************************************/

/*---------------------------------------------------------------------------*/
/* Static function prototypes                                                */
/*---------------------------------------------------------------------------*/

static int ObjFind(array_t *objs, int x, int y, short flag);

/**AutomaticEnd***************************************************************/


/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis           [Release the memory associated with a drawing.]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
void
draw_Free(draw_t *s)
{
drawobject_t *o;
int i;

  if(s == NIL(draw_t)) {
    return;
  }  
  for(i = 0;i < array_n(s->obj);i++) {
    o = array_fetch(drawobject_t *, s->obj, i);
    if(o->type == DRAWLINE) {
      FREE(o->obj.line);
    }
    else {
      FREE(o->obj.roc);  
    }
    FREE(o);
  }
  array_free(s->obj);  
  FREE(s);
}

/**Function********************************************************************

  Synopsis           [Display on a canvas, the set of objects within it's
  drawing area.]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
void
draw_Update(
  Canvas canvas,
  draw_t *s)
{
Xv_Window paint_window;
Display *dpy;
Window xwin;
Xv_xrectlist *xrects;
GC gc;
int o1,o2;
drawobject_t *obj;
int i;
unsigned long *colors;

  if(s == NIL(draw_t)) {
    return;
  }
  
  paint_window = canvas_paint_window(canvas);
  dpy = (Display *) xv_get(canvas ,XV_DISPLAY);
  xwin = (Window) xv_get(paint_window, XV_XID);
  xrects = NULL;
  
  gc = DefaultGC(dpy, DefaultScreen(dpy));
  XClearWindow(dpy, xwin);
  colors = (unsigned long *) xv_get(canvas, WIN_X_COLOR_INDICES);
  
  (void) xv_set(canvas, WIN_BACKGROUND_COLOR, s->backcolor, NULL);
 
  o1 = ObjFind(s->obj, s->x, s->y, DRAWLOWER_FLAG);
  o2 = ObjFind(s->obj, s->x + s->width, s->y + s->height, DRAWUPPER_FLAG);
  if((o1 == -1) || (o2 == -1)) {
    return;
  }
  
  for(i = o1;i < o2;i++) {
    obj = array_fetch(drawobject_t *, s->obj, i);
    XSetForeground(dpy, gc, colors[obj->color]);
    XSetLineAttributes(dpy, gc, obj->line_width, obj->style, obj->cap_style,
      obj->join_style);
    switch(obj->type) {
      case DRAWLINE: {
        XDrawLine(dpy, xwin, gc, obj->x1 - s->x, obj->y1 - s->y,
          obj->obj.line->x2 - s->x, obj->obj.line->y2 - s->y);      
      } break;
        
      case DRAWCIRC: {
        if(obj->obj.roc->filled == 1) {
          XFillArc(dpy, xwin, gc, obj->x1 - s->x, obj->y1 - s->y,
            obj->obj.roc->width, obj->obj.roc->height, 0, 360*64);        
        }
        else {
          XDrawArc(dpy, xwin, gc, obj->x1 - s->x, obj->y1 - s->y,
            obj->obj.roc->width, obj->obj.roc->height, 0, 360*64);
        }            
      } break;
      
      case DRAWRECT: {
        if(obj->obj.roc->filled == 1) {
          XFillRectangle(dpy, xwin, gc, obj->x1 - s->x, obj->y1 - s->y,
            obj->obj.roc->width, obj->obj.roc->height);
        }
        else {
          XDrawRectangle(dpy, xwin, gc, obj->x1 - s->x, obj->y1 - s->y,
            obj->obj.roc->width, obj->obj.roc->height);
        }
      } break;
    }         
  }  
}

/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis           [Insert an roc object.]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
draw_t *
drawRocIns(
  draw_t *s,
  int x1,
  int y1,
  short type,
  int style,
  int cap_style,
  int join_style,
  int color,
  int line_width,
  short filled,
  int height,
  int width)
{
drawobject_t *n;

  n = (drawobject_t *) ALLOC(drawobject_t, 1);
  n->type = type;
  n->x1 = x1;
  n->y1 = y1;
  n->line_width = line_width;
  n->color = color;
  n->style = style;
  n->cap_style = cap_style;
  n->join_style = join_style;
  
  n->obj.roc = (drawobj_roc_t *) ALLOC(drawobj_roc_t, 1);
  n->obj.roc->filled = filled;
  n->obj.roc->height = height;
  n->obj.roc->width = width;

  array_insert_last(drawobject_t *, s->obj, n);
  
  return s;
}


/**Function********************************************************************

  Synopsis           [Insert an line object.]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
draw_t *
drawLineIns(
  draw_t *s,
  int x1,
  int y1,
  int x2,
  int y2,
  int color,
  int width,
  int style,
  int cap_style,
  int join_style)
{
drawobject_t *n;

  n = (drawobject_t *) ALLOC(drawobject_t, 1);
  n->type = DRAWLINE;
  n->x1 = x1;
  n->y1 = y1;
  n->color = color;
  n->style = style;
  n->cap_style = cap_style;
  n->join_style = join_style;
  n->line_width = width;

  n->obj.line = (drawobj_line_t *) ALLOC(drawobj_line_t, 1);
  n->obj.line->x2 = x2;
  n->obj.line->y2 = y2;
    
  array_insert_last(drawobject_t *, s->obj, n);
  
  return s;
}

/**Function********************************************************************

  Synopsis           [Compare two objects]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
int
drawObjCmp(
  char *o1,
  char *o2)
{
drawobject_t **obj1, **obj2;

  obj1 = (drawobject_t **) o1;
  obj2 = (drawobject_t **) o2;

  if((*obj1)->x1 < (*obj2)->x1) {
    return -1;
  }
  else
    if((*obj1)->x1 > (*obj2)->x1) {
      return 1;
    }
    else 
      if((*obj1)->y1 < (*obj2)->y1) {
        return -1;
      }
      else
       if((*obj1)->y1 > (*obj2)->y1) {
         return 1;
       }         
  return 0;
}

/**Function********************************************************************

  Synopsis           [Allocate a new drawing.]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
draw_t *
drawAlloc(
  int x,
  int y,
  int w,
  int h,
  int backcolor)
{
draw_t *s;

  s = (draw_t *) ALLOC(draw_t, 1);
  s->x = x;
  s->y = y;
  s->width = w;
  s->height = h;
  s->backcolor = backcolor;
  s->obj = array_alloc(drawobject_t *, 0);
  
  return s;
}

/**Function********************************************************************

  Synopsis           [Print a drawing.]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
void
print_draw(draw_t *s)
{
int i;
drawobject_t *o;

  if(s == NIL(draw_t)) {
    return;
  }
  printf("X = %d  Y = %d  W = %d  H = %d\n",s->x, s->y, s->width, s->height);
  printf("Objects:\n");
  for(i = 0;i < array_n(s->obj);i++) {
    o = array_fetch(drawobject_t *, s->obj, i);
    printf("X1 = %d Y1 = %d\n",o->x1, o->y1);
  }
}


/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis           [Find an object.]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static int
ObjFind(
  array_t *objs,
  int x,
  int y,
  short flag)
{
int low, high, i;
drawobject_t *o;

  if(objs == NIL(array_t)) {
    return -1;
  }

  low = 0;
  high = array_n(objs) - 1;
  i = (high + low)/2;
  
  while(low <= high) {
    o = array_fetch(drawobject_t *, objs, i);
    if((o->x1 == x) && (o->y1 == y)) {
      return i;
    }
    else 
      if(o->x1 < x) {
        high = i  - 1;
      }
      else
        if(o->x1 > x) {
          low = i + 1;
        }
        else
          if(o->y1 < y) {
            high = i - 1;
          }
          else {
            low = i + 1;
          }
    i = (high + low)/2;
  }
   
  if(flag == DRAWLOWER_FLAG) {
    o = array_fetch(drawobject_t *, objs, i);
    while(((o->x1 > x) || (o->x1 == x && o->y1 > y)) && (i > 0)) {
      i--;
      o = array_fetch(drawobject_t *, objs, i);
    }
    if((i == 0) && ((o->x1 > x) || (o->x1 == x && o->y1 > y))) return i;    
    return i+1;
  }
  
  if(flag == DRAWUPPER_FLAG) {
    o = array_fetch(drawobject_t *, objs, i);
    while(((o->x1 < x) || (o->x1 == x && o->y1 < y))) {
      i++;
      if(i == array_n(objs)) break;
      o = array_fetch(drawobject_t *, objs, i);
    }            
    return i;
  }
  
  return -1;
}
