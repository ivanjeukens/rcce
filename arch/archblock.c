/**CFile***********************************************************************

  FileName    [archBlock.c]

  PackageName [arch]

  Synopsis    [Functions for manipulating the archblock structure]

  Description []

  SeeAlso     []

  Author      [Ivan Jeukens]

  Copyright   [1996-1997 Ivan Jeukens]

******************************************************************************/

#include "archInt.h"
#include "compInt.h"

/*
static char rcsid[] = \"$Id: $\";
USE(rcsid);
*/

/*---------------------------------------------------------------------------*/
/* Variable declarations                                                     */
/*---------------------------------------------------------------------------*/
EXTERN FILE *msgerr;

/**AutomaticStart*************************************************************/

/*---------------------------------------------------------------------------*/
/* Static function prototypes                                                */
/*---------------------------------------------------------------------------*/

static void AbcompFree(arch_abcomp_t *c);
static void AbsegFree(arch_abseg_t *seg);

/**AutomaticEnd***************************************************************/


/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/
/**Function********************************************************************

  Synopsis           [Release the memory associated with an block structure.]

  Description        []

  SideEffects        []
  
  SeeAlso	     []
  
******************************************************************************/
void
arch_BlockFree(arch_block_t *a)
{
  if(a != NIL(arch_block_t)) {
    FREE(a->label);
    lsDestroy(a->comp, AbcompFree);
    lsDestroy(a->seg, AbsegFree);
    FREE(a);
  }
}

/**Function********************************************************************

  Synopsis           [Allocate a new block structure.]

  Description        []

  SideEffects        []
  
  SeeAlso	     []
  
******************************************************************************/
arch_block_t *
arch_BlockAlloc(char *label)
{
arch_block_t *new;

  new = (arch_block_t *) ALLOC(arch_block_t, 1);
  new->label = util_strsav(label);
  new->ncomp = 0;
  new->comp = lsCreate();
  new->nseg = 0;
  new->seg = lsCreate();

  return new;
}

/**Function********************************************************************

  Synopsis           [Insert a component into the block structure.]

  Description        []

  SideEffects        []
  
  SeeAlso	     []
  
******************************************************************************/
arch_block_t *		/* add_ab_comp */
arch_AbcompIns(
  arch_block_t *ab,
  comp_fig_t *cfig,
  char *label,
  char *lib,
  int x,
  int y,
  short io,
  short pos)
{
arch_abcomp_t *new;
int i;
comp_figpin_t *ptr;
lsHandle handle;

  new = (arch_abcomp_t *) ALLOC(arch_abcomp_t, 1);
  new->x = x;
  new->y = y;

  new->fig = (comp_fig_t *) ALLOC(comp_fig_t, 1);
  new->fig->width = cfig->width;
  new->fig->height = cfig->height;

  new->fig->pins = array_alloc(comp_figpin_t *,0);
  for(i = 0;i < array_n(cfig->pins);i++) {
    ptr = array_fetch(comp_figpin_t *, cfig->pins, i);
    new->fig->pins = comp_FpinIns(new->fig->pins, ptr->dx1, ptr->dy1, 
      ptr->dx2, ptr->dy2, ptr->number);
  }
  new->library = util_strsav(lib);
  new->label = util_strsav(label);
  new->io = io;
  new->pos = pos;

  ab->ncomp++;
  (void) lsNewBegin(ab->comp, (lsGeneric) new, &handle);
    
  return ab;
}

/**Function********************************************************************

  Synopsis           [Insert a segment into the block structure.]

  Description        []

  SideEffects        []
  
  SeeAlso	     []
  
******************************************************************************/
arch_block_t *
arch_AbsegIns(
  arch_block_t *ab,
  int x,
  int y,
  int x2,
  int y2)
{
arch_abseg_t *new;
lsHandle handle;

  if((abs(x - x2) < 5) && (abs(y - y2) < 5)) return ab;

  new = (arch_abseg_t *) ALLOC(arch_abseg_t, 1);
  new->x = x;
  new->y = y;
  new->x2 = x2;
  new->y2 = y2;
  
  ab->nseg++;
  (void) lsNewBegin(ab->seg, (lsGeneric) new, &handle);

  return ab;
}

/**Function********************************************************************

  Synopsis           [Given an architecture block, zoom in or out.]

  Description        []

  SideEffects        []
  
  SeeAlso	     []
  
******************************************************************************/
arch_block_t *		/* zoom_fig */
arch_BlockZoom(
  arch_block_t *ab,
  short f,
  int px,
  int py)
{
lsGen gen;
arch_abcomp_t *cptr;
arch_abseg_t *sptr;
comp_figpin_t *fptr;
int i;

  if(f == 0) {
    if(ab->ncomp > 0) {
      lsForeachItem(ab->comp, gen, cptr) {
        cptr->fig->width = cptr->fig->width/2;
        cptr->fig->height = cptr->fig->height/2;
        cptr->x = cptr->x + (px - cptr->x)/2;
        cptr->y = cptr->y + (py - cptr->y)/2;        
        for(i = 0;i < array_n(cptr->fig->pins);i++) {
          fptr = array_fetch(comp_figpin_t *, cptr->fig->pins, i);
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
      }
      if(ab->nseg > 0) {
        lsForeachItem(ab->seg, gen, sptr) {
          sptr->x = sptr->x + (px - sptr->x)/2;
          sptr->y = sptr->y + (py - sptr->y)/2;
          sptr->x2 = sptr->x2 + (px - sptr->x2)/2;
          sptr->y2 = sptr->y2 + (py - sptr->y2)/2;      
        }
      }
    }
  }
  else {
    if(ab->ncomp > 0) {
      lsForeachItem(ab->comp, gen, cptr) {
        cptr->fig->width = cptr->fig->width*2;
        cptr->fig->height = cptr->fig->height*2;
        cptr->x = cptr->x - (px - cptr->x);
        cptr->y = cptr->y - (py - cptr->y);
        for(i = 0;i < array_n(cptr->fig->pins);i++) {
          fptr = array_fetch(comp_figpin_t *, cptr->fig->pins, i);
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
      }
    }
    if(ab->nseg > 0) {
      lsForeachItem(ab->seg, gen, sptr) {
        sptr->x = sptr->x - (px - sptr->x);
        sptr->y = sptr->y - (py - sptr->y);
        sptr->x2 = sptr->x2 - (px - sptr->x2);
        sptr->y2 = sptr->y2 - (py - sptr->y2);
      }
    }
  }

  return ab;
}      

/**Function********************************************************************

  Synopsis           [Delete items from the architecture block, specified by
  the endpoints of a selection square.]

  Description        []

  SideEffects        []
  
  SeeAlso	     []
  
******************************************************************************/
arch_block_t *		/* deleteb */
arch_BlockRm(
  arch_block_t *ab,
  int x1,
  int y1,
  int x2,
  int y2)
{
int t, status;
lsGen gen;
arch_abcomp_t *cptr;
arch_abseg_t *sptr;
lsHandle handle;

   if(x1 > x2) {
     t = x1;
     x1 = x2;
     x2 = t;
   }
   
   if(y1 > y2) {
     t = y1;
     y1 = y2;
     y2 = t;
   }
 
   if((x1 != x2) || (y1 != y2)) {
     if(ab->ncomp > 0) {
       gen = lsStart(ab->comp);
       status = lsNext(gen, (lsGeneric *) &cptr, &handle);
       while(status != LS_NOMORE) {
         if(util2_InRect(cptr->x, cptr->y, x1, y1, x2 - x1, y2 - y1) ||
            util2_InRect(cptr->fig->width + cptr->x, 
              cptr->fig->height + cptr->y, x1, y1, x2 - x1, y2 - x1)) {   
           (void) lsRemoveItem(handle, (lsGeneric *) &cptr);
           AbcompFree(cptr);
           ab->ncomp--;
         }
         status = lsNext(gen, (lsGeneric *) &cptr, &handle);
       }       
       (void) lsFinish(gen);                  
     }
     if(ab->nseg > 0) {
       gen = lsStart(ab->seg);
       status = lsNext(gen, (lsGeneric *) &sptr, &handle);
       while(status != LS_NOMORE) {
         if(util2_InRect(sptr->x, sptr->y, x1, y1, x2 - x1, y2 - y1) ||
            util2_InRect(sptr->x2, sptr->y2, x1, y1, x2 - x1, y2 - y1)) {             
           (void) lsRemoveItem(handle, (lsGeneric *) &sptr);
           FREE(sptr);
           ab->nseg--;
         }
         status = lsNext(gen, (lsGeneric *) &sptr, &handle);
       }
       (void) lsFinish(gen);
     }
   }
   else
     if((x1 == x2) && (y1 == y2)) {
       if(ab->ncomp > 0) {
         gen = lsStart(ab->comp);
         status = lsNext(gen, (lsGeneric *) &cptr, &handle);
         while(status != LS_NOMORE) {
           if(util2_InRect(x1, y1, cptr->x, cptr->y, cptr->fig->width,
                cptr->fig->height)) {
             (void) lsRemoveItem(handle, (lsGeneric *) &cptr);
             AbcompFree(cptr);
             ab->ncomp--;
             return ab;
           }
           status = lsNext(gen, (lsGeneric *) &cptr, &handle);
         }
         (void) lsFinish(gen);
       }
       if(ab->nseg > 0) {
         gen = lsStart(ab->seg);
         status = lsNext(gen, (lsGeneric *) &sptr, &handle);
         while(status != LS_NOMORE) {
           if(util2_InLine(x1, y1, sptr->x, sptr->y, sptr->x2, sptr->y2)) {
             (void) lsRemoveItem(handle, (lsGeneric *) &sptr);
             FREE(sptr);
             (void) lsFinish(gen);
             ab->nseg--;
             return ab;
           }
          status = lsNext(gen, (lsGeneric *) &sptr, &handle);
         }
         (void) lsFinish(gen);
       }
     }
     
  return ab;
}

/**Function********************************************************************

  Synopsis           [Rotate items from the architecture block, specified by
  the endpoints of a selection square.]

  Description        []

  SideEffects        []
  
  SeeAlso	     []
  
******************************************************************************/
arch_block_t *
arch_BlockRotate(
  arch_block_t *ab,
  int x1,
  int y1,
  int x2,
  int y2)
{
int t,i;
lsGen gen;
arch_abcomp_t *cptr;
comp_figpin_t *fptr;

  if((x1 == x2) && (y1 == y2)) {
     if(ab->ncomp > 0) {
       lsForeachItem(ab->comp, gen, cptr) {
         if(util2_InRect(x1, y1, cptr->x, cptr->y, cptr->fig->width, 
              cptr->fig->height)) {
           switch(cptr->pos) {
             case 0: {
               t = cptr->fig->width;
               cptr->fig->width = cptr->fig->height;
               cptr->fig->height = t;
               for(i = 0; i < array_n(cptr->fig->pins);i++) {
                 fptr = array_fetch(comp_figpin_t *,cptr->fig->pins,i);
                 t = fptr->dx1;
                 fptr->dx1 = fptr->dy1;
                 fptr->dy1 = t;
                 t = fptr->dx2;
                 fptr->dx2 = fptr->dy2;
                 fptr->dy2 = t;             
               }
               cptr->pos++;
             } break;
             case 1: {
               t = cptr->fig->height;
               cptr->fig->height = cptr->fig->width;
               cptr->fig->width = -t;
               for(i = 0; i < array_n(cptr->fig->pins);i++) {
                 fptr = array_fetch(comp_figpin_t *,cptr->fig->pins,i);               
                 t = fptr->dy1;
                 fptr->dy1 = fptr->dx1;
                 fptr->dx1 = -t;
                 t = fptr->dy2;
                 fptr->dy2 = fptr->dx2;
                 fptr->dx2 = -t;
               }           
               cptr->pos++;           
             } break;
             case 2: {
               t = cptr->fig->width;
               cptr->fig->width = cptr->fig->height;
               cptr->fig->height = t;
               for(i = 0; i < array_n(cptr->fig->pins);i++) {
                 fptr = array_fetch(comp_figpin_t *,cptr->fig->pins,i);               
                 t = fptr->dx1;
                 fptr->dx1 = fptr->dy1;
                 fptr->dy1 = t;
                 t = fptr->dx2;
                 fptr->dx2 = fptr->dy2;
                 fptr->dy2 = t;
               }           
               cptr->pos++;           
             } break;
             case 3: {
               t = cptr->fig->width;
               cptr->fig->width = -cptr->fig->height;
               cptr->fig->height = t;
               for(i = 0; i < array_n(cptr->fig->pins);i++) {
                 fptr = array_fetch(comp_figpin_t *,cptr->fig->pins,i);
                 t = fptr->dx1;
                 fptr->dx1 = -fptr->dy1;
                 fptr->dy1 = t;
                 t = fptr->dx2;
                 fptr->dx2 = -fptr->dy2;
                 fptr->dy2 = t;
               }
               cptr->pos = 0;           
             } break;
           }
           return ab;
         }
       }
     }
  }
  return ab;
}

/**Function********************************************************************

  Synopsis           [Copy items from the architecture block, specified by
  the endpoints of a selection square.]

  Description        []

  SideEffects        []
  
  SeeAlso	     []
  
******************************************************************************/
arch_block_t *
arch_BlockCopy(
  arch_block_t *ab,
  int x1,
  int y1,
  int x2,
  int y2,
  int xd,
  int yd,
  int zf)
{
arch_abcomp_t *cptr;
arch_abseg_t *sptr;
lsGen gen;
char curlib[100] = "\0";
lsList lib;
comp_data_t *c,*cfig;
int i;
comp_figpin_t *fptr;
int t;

  if(x1 > x2) {
    t = x1;
    x1 = x2;
    x2 = t;
  }   
  if(y1 > y2) {
    t = y1;
    y1 = y2;
    y2 = t;
  }  
  if((x1 != x2) || (y1 != y2)) {
    if(ab->ncomp > 0) {
      lib = lsCreate();     
      lsForeachItem(ab->comp, gen, cptr) {
        if(util2_InRect(cptr->x, cptr->y, x1, y1, x2 - x1, y2 - y1) ||
           util2_InRect(cptr->fig->width + cptr->x,
             cptr->fig->height + cptr->y, x1, y1, x2 - x1, y2 - y1)) {
          if(strcmp(curlib, cptr->library)) {
            comp_LibFree(lib);
            lib = lsCreate();
            lib = comp_LibLoad(cptr->library);
            strcpy(curlib, cptr->library);
          }
          c = comp_Find(lib, cptr->label);
          if(c == NIL(comp_data_t)) return ab;
          cfig = comp_FigCopy(c);
          switch(zf) {
            case 1: {
              cfig->fig->width = cfig->fig->width/2;
              cfig->fig->height = cfig->fig->height/2;
              for(i = 0;i < array_n(cfig->fig->pins);i++) {
                fptr = array_fetch(comp_figpin_t *,cfig->fig->pins,i++);
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
            }
            break;
            case 2: break;
            case 3: {
                cfig->fig->width = cfig->fig->width*2;
                cfig->fig->height = cfig->fig->height*2;
                for(i = 0;i < array_n(cfig->fig->pins);i++) {
                  fptr = array_fetch(comp_figpin_t *,cfig->fig->pins,i++);
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
            }
            break;
            case 4: {
                cfig->fig->width = cfig->fig->width*4;
                cfig->fig->height = cfig->fig->height*4;
                for(i = 0;i < array_n(cfig->fig->pins);i++) {
                  fptr = array_fetch(comp_figpin_t *,cfig->fig->pins,i++);
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
              }
              break;
            }
            ab = arch_AbcompIns(ab, cfig->fig, cptr->label, cptr->library, 
               cptr->x + xd - x1, cptr->y + yd - y1, cptr->io, cptr->pos);
   	    comp_Free(cfig);
        }
      }
      comp_LibFree(lib);
    }         
    if(ab->nseg > 0) {
      lsForeachItem(ab->seg, gen, sptr) {
        if(util2_InRect(sptr->x, sptr->y, x1, y1, x2 - x1, y2 - y1) ||
          util2_InRect(sptr->x, sptr->y, x1, y1, x2 - x1, y2 - y1)) {
          ab = arch_AbsegIns(ab, sptr->x + xd - x1, sptr->y + yd - y1, 
             sptr->x2 + xd - x1, sptr->y2 + yd - y1);
        }
      }
    }
  }
  else
    if((x1 == x2) && (y1 == y2)) {
      if(ab->ncomp > 0) {
        lsForeachItem(ab->comp, gen, cptr) {
          if(util2_InRect(x1,y1,cptr->x,cptr->y,cptr->fig->width,
            cptr->fig->height)) {
            lib = comp_LibLoad(cptr->library);
            c = comp_Find(lib, cptr->label);
            if(c == NIL(comp_data_t)) return ab;
            cfig = comp_FigCopy(c);
            switch(zf) {
              case 1: {
                cfig->fig->width = cfig->fig->width/2;
                cfig->fig->height = cfig->fig->height/2;
                for(i = 0;i < array_n(cfig->fig->pins);i++) {
                  fptr = array_fetch(comp_figpin_t *,cfig->fig->pins,i++);
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
                cfig->fig->width = cfig->fig->width*2;
                cfig->fig->height = cfig->fig->height*2;
                for(i = 0;i < array_n(cfig->fig->pins);i++) {
                  fptr = array_fetch(comp_figpin_t *,cfig->fig->pins,i++);
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
              } break;
              case 4: {
                cfig->fig->width = cfig->fig->width*4;
                cfig->fig->height = cfig->fig->height*4;
                for(i = 0;i < array_n(cfig->fig->pins);i++) {
                  fptr = array_fetch(comp_figpin_t *,cfig->fig->pins,i++);
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
            ab = arch_AbcompIns(ab, cfig->fig, cptr->label, cptr->library, 
              cptr->x + xd - x1, cptr->y + yd - y1, cptr->io, cptr->pos);
   	    comp_Free(cfig);
   	    comp_LibFree(lib);
   	    return ab;
          }
        }
      }      
      if(ab->nseg > 0) {
        lsForeachItem(ab->seg, gen, sptr) {        
          if(util2_InLine(x1,y1,sptr->x,sptr->y, sptr->x2, sptr->y2)) {
             ab = arch_AbsegIns(ab, sptr->x + xd - x1, sptr->y + yd - y1, 
               sptr->x2 + xd - x1, sptr->y2 + yd - y1);
             return ab;
           }
        }
      }
    }

  return ab;
}

/**Function********************************************************************

  Synopsis           [Move items from the architecture block, specified by
  the endpoints of a selection square.]

  Description        []

  SideEffects        []
  
  SeeAlso	     []
  
******************************************************************************/
arch_block_t *
arch_BlockMove(
  arch_block_t *ab,
  int x1,
  int y1,
  int x2,
  int y2,
  int xd,
  int yd,
  int zf)
{
arch_block_t *temp = NIL(arch_block_t);
lsGen gen;
arch_abcomp_t *cptr;
arch_abseg_t *sptr;
lsList lib;
comp_data_t *c, *cfig;
int status,t;
lsHandle handle;
char curlib[100] = "\0";
int i;
comp_figpin_t *fptr;

  if(x1 > x2) {
    t = x1;
    x1 = x2;
    x2 = t;
  }   
  if(y1 > y2) {
    t = y1;
    y1 = y2;
    y2 = t;
  }
      
  if((x1 != x2) || (y1 != y2)) {
    temp = arch_BlockAlloc(" ");     
    if(ab->ncomp > 0) {
      lib = lsCreate();
      gen = lsStart(ab->comp);
      status = lsNext(gen, (lsGeneric *) &cptr, &handle);
      while(status != LS_NOMORE) {
        if(util2_InRect(cptr->x, cptr->y, x1, y1, x2 - x1, y2 - y1) ||
           util2_InRect(cptr->fig->width + cptr->x,
             cptr->fig->height + cptr->y, x1, y1, x2 - x1, y2 - y1)) {
          if(strcmp(curlib, cptr->library)) {
            comp_LibFree(lib);
            lib = lsCreate();
            lib = comp_LibLoad(cptr->library);
            strcpy(curlib, cptr->library);          
          }
          c = comp_Find(lib, cptr->label);
          if(c == NIL(comp_data_t)) return ab;
          temp = arch_AbcompIns(temp, c->fig, cptr->label, cptr->library,
            cptr->x + xd - x1, cptr->y + yd - y1, cptr->io, cptr->pos);
          (void) lsRemoveItem(handle, (lsGeneric *) &cptr);
          AbcompFree(cptr);
          ab->ncomp--;                    
        }
        status = lsNext(gen, (lsGeneric *) &cptr, &handle);
      }    
      (void) lsFinish(gen);
      comp_LibFree(lib);
    }  
    if(ab->nseg > 0) {
      gen = lsStart(ab->seg);
      status = lsNext(gen, (lsGeneric *) &sptr, &handle);
      while(status != LS_NOMORE) {
        if(util2_InRect(sptr->x, sptr->y, x1, y1, x2 - x1, y2 - y1) ||
           util2_InRect(sptr->x, sptr->y, x1, y1, x2 - x1, y2 - y1)) {
          temp = arch_AbsegIns(temp, sptr->x + xd - x1, sptr->y + yd - y1, 
            sptr->x2 + xd - x1, sptr->y2 + yd - y1);
          (void) lsRemoveItem(handle, (lsGeneric *) &sptr);
          FREE(sptr);
          ab->nseg--;                      
        }
        status = lsNext(gen, (lsGeneric *) &sptr, &handle);
      }  
      (void) lsFinish(gen);
    }
  }
  else
    if((x1 == x2) && (y1 == y2)) {
      if(ab->ncomp > 0) {
        gen = lsStart(ab->comp);
        status = lsNext(gen, (lsGeneric *) &cptr, &handle);
        while(status != LS_NOMORE) {
          if(util2_InRect(x1, y1, cptr->x, cptr->y, cptr->fig->width,
            cptr->fig->height)) {
            lib = comp_LibLoad(cptr->library);
            c = comp_Find(lib, cptr->label);
            if(c == NIL(comp_data_t)) return ab;
            (void) lsRemoveItem(handle, (lsGeneric *) &cptr);
            cfig = comp_FigCopy(c);
            switch(zf) {
              case 1: {
                cfig->fig->width = cfig->fig->width/2;
                cfig->fig->height = cfig->fig->height/2;
                for(i = 0;i < array_n(cfig->fig->pins);i++) {
                  fptr = array_fetch(comp_figpin_t *,cfig->fig->pins,i++);
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
                cfig->fig->width = cfig->fig->width*2;
                cfig->fig->height = cfig->fig->height*2;
                for(i = 0;i < array_n(cfig->fig->pins);i++) {
                  fptr = array_fetch(comp_figpin_t *,cfig->fig->pins,i++);
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
              } break;
              case 4: {
                cfig->fig->width = cfig->fig->width*4;
                cfig->fig->height = cfig->fig->height*4;
                for(i = 0;i < array_n(cfig->fig->pins);i++) {
                  fptr = array_fetch(comp_figpin_t *,cfig->fig->pins,i++);
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
            ab = arch_AbcompIns(ab, cfig->fig, cptr->label, cptr->library,
              cptr->x + xd - x1, cptr->y + yd - y1, cptr->io, cptr->pos);
            AbcompFree(cptr);
            comp_Free(cfig);
            ab->ncomp--;
            return ab;
          }
          status = lsNext(gen, (lsGeneric *) &cptr, &handle);        
        }
      }
      
      if(ab->nseg > 0) {
        gen = lsStart(ab->seg);
        status = lsNext(gen, (lsGeneric *) &sptr, &handle);
        while(status != LS_NOMORE) {
          if(util2_InLine(x1, y1, sptr->x, sptr->y, sptr->x2, sptr->y2)) {
            (void) lsRemoveItem(handle, (lsGeneric *) &sptr);
            ab = arch_AbsegIns(ab, sptr->x + xd - x1, sptr->y + yd - y1,
              sptr->x2 + xd - x1, sptr->y2 + yd - y1);
            FREE(sptr);
            ab->nseg--;                      
            return ab;                          
          }
          status = lsNext(gen, (lsGeneric *) &sptr, &handle);
        }        
      }
    }

  if(temp != NIL(arch_block_t)) {
    if(temp->ncomp > 0) {
      lib = lsCreate();
      curlib[0] = '\0';
      lsForeachItem(temp->comp, gen, cptr) {
        if(strcmp(curlib, cptr->library)) {
          comp_LibFree(lib);
          lib = lsCreate();
          lib = comp_LibLoad(cptr->library);
          strcpy(curlib, cptr->library);                  
        }  
        c = comp_Find(lib, cptr->label);
        if(c == NIL(comp_data_t)) return ab;
        cfig = comp_FigCopy(c);
        switch(zf) {
          case 1: {
            cfig->fig->width = cfig->fig->width/2;
            cfig->fig->height = cfig->fig->height/2;
            for(i = 0;i < array_n(cfig->fig->pins);i++) {
              fptr = array_fetch(comp_figpin_t *,cfig->fig->pins,i++);
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
            cfig->fig->width = cfig->fig->width*2;
            cfig->fig->height = cfig->fig->height*2;
            for(i = 0;i < array_n(cfig->fig->pins);i++) {
              fptr = array_fetch(comp_figpin_t *,cfig->fig->pins,i++);
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
          } break;
          case 4: {
            cfig->fig->width = cfig->fig->width*4;
            cfig->fig->height = cfig->fig->height*4;
            for(i = 0;i < array_n(cfig->fig->pins);i++) {
              fptr = array_fetch(comp_figpin_t *,cfig->fig->pins,i++);
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
        ab = arch_AbcompIns(ab, cfig->fig, cptr->label, cptr->library, 
          cptr->x, cptr->y, cptr->io, cptr->pos);          
        comp_Free(cfig);
      }    
      comp_LibFree(lib);
    }  
    
    if(temp->nseg > 0) {
      lsForeachItem(temp->seg, gen, sptr) {
        ab = arch_AbsegIns(ab, sptr->x, sptr->y, sptr->x2, sptr->y2);            
      }
    }
    arch_BlockFree(temp);
  }      
  
  return ab;
}

/**Function********************************************************************

  Synopsis           [Save a block structure.]

  Description        []

  SideEffects        []
  
  SeeAlso	     []
  
******************************************************************************/
short
arch_BlockSave(
  arch_block_t *ab,
  char *filename)
{
FILE *fp;
lsGen gen;
arch_abcomp_t *cptr;
arch_abseg_t *sptr;
comp_figpin_t *fptr;
int i;

  fp = fopen(filename,"w");
  if(fp == NIL(FILE)) {
    return ARCH_NO_FILE;       
  }
  if(ab == NIL(arch_block_t)) {
    return ARCH_NO_ARCH_BLOCK;
  }
    
  fprintf(fp,"Architecture Block %s\n",ab->label);  
  if(ab->ncomp > 0) {
    fprintf(fp,"Components %d\n",ab->ncomp);
    lsForeachItem(ab->comp, gen, cptr) {
      fprintf(fp,"%d %d %s %s %d %d %d %d %d ",cptr->x,cptr->y,cptr->label,
        cptr->library, cptr->io, cptr->pos, cptr->fig->width, cptr->fig->height,
        array_n(cptr->fig->pins));
      
      for(i = 0;i < array_n(cptr->fig->pins);i++) {
        fptr = array_fetch(comp_figpin_t *,cptr->fig->pins,i);
        fprintf(fp,"%d %d %d %d %d ",fptr->dx1, fptr->dy1, fptr->dx2, fptr->dy2,
          fptr->number);
      }
    }   
  }
  if(ab->nseg > 0) {
    fprintf(fp,"\nSegments %d\n",ab->nseg);
    lsForeachItem(ab->seg, gen, sptr) {
      fprintf(fp,"%d %d %d %d ",sptr->x, sptr->y, sptr->x2, sptr->y2);      
    } 
  } 

  (void) fclose(fp);    
  return 0;
}

/**Function********************************************************************

  Synopsis           [Load a block structure.]

  Description        []

  SideEffects        []
  
  SeeAlso	     []
  
******************************************************************************/
arch_block_t * 
arch_BlockLoad(char *filename)
{
arch_block_t *ret = NULL;
char buffer1[80],buffer2[80];
FILE *fp;
int i, x, y, w, h, size, npins, j, dx1, dx2, dy1, dy2, number;
short io,pos;
comp_fig_t *fig;

  fp = fopen(filename,"r");
  if(fp == NIL(FILE)) return NIL(arch_block_t);
    
  (void) fscanf(fp,"%s %s",buffer1,buffer2);
  if(!strcmp(buffer1,"Architecture") && !strcmp(buffer2,"Block")) {
    (void) fscanf(fp,"%s",buffer2);
    ret = arch_BlockAlloc(buffer2);
    (void) fscanf(fp,"%s ",buffer1);
    if(strcmp(buffer1, "Components") == 0) {
      (void) fscanf(fp,"%d",&size);
      ret->ncomp = size;
      for(i = 0;(i < size) && !feof(fp);i++) {
        (void) fscanf(fp,"%d %d %s %s %d %d %d %d %d", &x, &y, buffer1, 
          buffer2, &io, &pos, &w, &h, &npins);
        fig = (comp_fig_t *) ALLOC(comp_fig_t, 1);
        fig->width = w;
        fig->height = h;
        fig->pins = array_alloc(comp_figpin_t *, 0);
        for(j = 0; j < npins; j++) {
          (void) fscanf(fp,"%d %d %d %d %d", &dx1, &dy1, &dx2, &dy2, &number);
          fig->pins = comp_FpinIns(fig->pins, dx1, dy1, dx2, dy2, number);         
        }
        ret = arch_AbcompIns(ret, fig, buffer1, buffer2, x, y, io, pos);
        comp_FpinFree(fig->pins);
        FREE(fig);
      }          
      if(feof(fp) == 0) {      
        (void) fscanf(fp,"%s %d", buffer1, &size);
        if(strcmp(buffer1,"Segments") == 0) {
          ret->nseg = size;
          for(i = 0;(i < size) && !feof(fp);i++) {
            (void) fscanf(fp,"%d %d %d %d",&dx1,&dy1,&dx2,&dy2);
            ret = arch_AbsegIns(ret, dx1, dy1, dx2, dy2);
          }         
        }
      }
    }
    else
      if(strcmp(buffer1, "Segments") == 0) {
        (void) fscanf(fp,"%d",&size);
        ret->nseg = size;
        for(i = 0;(i < size) && !feof(fp);i++) {
          (void) fscanf(fp,"%d %d %d %d",&dx1,&dy1,&dx2,&dy2);
          ret = arch_AbsegIns(ret, dx1, dy1, dx2, dy2);
        }             
      }
      else {
        fprintf(msgerr, "This file is not an architecture block file");      
      }
  }
  else {
    fprintf(msgerr,"This file is not an architecture block file");
  }
    
  (void) fclose(fp);  
  return ret;
}

/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis           [Print a block structure.]

  Description        []

  SideEffects        []
  
  SeeAlso	     []
  
******************************************************************************/
void
archBlockPrint(arch_block_t *ab)
{
arch_abcomp_t *cptr;
arch_abseg_t *sptr;
lsGen gen;

  if(ab == NIL(arch_block_t)) {
    return;
  }
  printf("Arch Block %s\n",ab->label);
  lsForeachItem(ab->comp, gen, cptr) {
    printf("Component %s (%d,%d)\n",cptr->label,cptr->x,cptr->y);  
  }
  
  lsForeachItem(ab->seg, gen, sptr) {
    printf("Segement (%d,%d) - (%d,%d)\n",sptr->x,sptr->y,sptr->x2,
      sptr->y2);  
  }
}

/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis           [Release the memory associated with an abcomp structure.]

  Description        []

  SideEffects        []
  
  SeeAlso	     []
  
******************************************************************************/
static void		/* destroy_ab_comp */
AbcompFree(arch_abcomp_t *c)
{
  if(c != NIL(arch_abcomp_t)) {
    comp_FpinFree(c->fig->pins);
    FREE(c->label);
    FREE(c->library);
    FREE(c);
  }
}

/**Function********************************************************************

  Synopsis           [Release the memory associated with an abseg structure.]

  Description        []

  SideEffects        []
  
  SeeAlso	     []
  
******************************************************************************/
static void
AbsegFree(arch_abseg_t *seg)
{
  if(seg != NIL(arch_abseg_t)) {
    FREE(seg); 
  }
}


/*
void
pinxy(clabel, plabel, a, x, y)
char *clabel;
char *plabel;
subarch *a;
int *x;
int *y;
{
layer *l;
element *data;
comp_lib *ltmp = NULL,*plib;
plist *ptmp;
int i = 1;
pinpos *pos;
char *pchar,ctmp[80];
int er, ln;

  l = a->l;
  while(l != NULL) {
    data = l->data;
    while((data != NULL) && (data->comp != NULL) &&
           strcmp(data->comp->label, clabel))
      data = data->next;    
    if(data != NULL) {    
      ltmp = load_library(data->comp->library, &er, &ln);
      plib = ltmp;
      strcpy(ctmp,data->comp->label);      
      pchar = strtok(ctmp,"_");
      while((plib != NULL) && strcmp(plib->component->label,pchar))
        plib = plib->next;
      if(plib != NULL) {
        ptmp = plib->component->pins;
        while((ptmp != NULL) && strcmp(ptmp->label, plabel)) {
          i++;
          ptmp = ptmp->next;
        }
        if(ptmp != NULL) {
          pos = data->comp->fig->pins;
          while((pos != NULL) && (i != pos->number)) {
            pos = pos->next;
          }     
          if(pos != NULL) {
            *x = data->x + pos->dx2;
            *y = data->y + pos->dy2;
          }
          break;
        }     
      }     
    }
    l = l->next;
  }
  if(ltmp != NULL) destroy_lib(ltmp);  
}

*/
