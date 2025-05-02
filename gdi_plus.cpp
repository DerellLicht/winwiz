//********************************************************************
//  Copyright (c) 2025  Daniel D Miller
//  This class encapsulates the interface to the gdi_plus library.
//********************************************************************

#include <windows.h>
#include <tchar.h>

#include "common.h"
#include "gdi_plus.h"

//***********************************************************************************
void gdi_plus::copy_imagelist_item(Graphics& graphics, int xsrc, int ysrc, int dx, int dy, int xdest, int ydest)
{
   // cii: src: 0x0, dxy: 359x362, dest: 0x0
   // actually drawn 116x116
   // syslog(_T("cii: src: %ux%u, dxy: %ux%u, dest: %ux%u\n"),
   //    xsrc, ysrc, dx, dy, xdest, ydest);
   Bitmap* clone = gbitmap->Clone(xsrc, ysrc, dx, dy, PixelFormatDontCare);
   //  use the five-arg form of DrawImage(), in order to disable auto-scaling
   graphics.DrawImage(clone, xdest, ydest, dx, dy);
   delete clone ;
}  //lint !e818

//********************************************************************
//  
//********************************************************************
void gdi_plus::render_bitmap(HDC hdc, uint xdest, uint ydest, uint sprite_col, uint sprite_row)
{
   Graphics graphics(hdc);
   // sprite_dx = nWidth / tiles_x ;
   // sprite_dy = nHeight / tiles_y ;
   uint srcx = sprite_col * sprite_dx ;
   uint srcy = sprite_row * sprite_dy ;
   copy_imagelist_item(graphics, srcx, srcy, sprite_dx, sprite_dy, xdest, ydest);
}

//********************************************************************
gdi_plus::gdi_plus(TCHAR *new_img_name) :
   // bmp(NULL),
   // img(NULL),
   img_name(NULL),
   gbitmap(NULL),
   nWidth(0),
   nHeight(0),
   sprite_dx(0),
   sprite_dy(0),
   tiles_x(1),
   tiles_y(1)
{
   img_name = new TCHAR[_tcslen(new_img_name)+1] ;
   _tcscpy(img_name, new_img_name) ;
   
   // hBitmap = load_png_to_bmp() ; //  this will init bmp, img
   gbitmap = new Bitmap(new_img_name);
   nWidth  = gbitmap->GetWidth();
   nHeight = gbitmap->GetHeight();

}

//********************************************************************
gdi_plus::gdi_plus(TCHAR *new_img_name, uint icons_per_column, uint icon_rows) :
   img_name(NULL),
   gbitmap(NULL),
   nWidth(0),
   nHeight(0),
   sprite_dx(0),
   sprite_dy(0),
   tiles_x(icons_per_column),
   tiles_y(icon_rows)
{
   img_name = new TCHAR[_tcslen(new_img_name) +1] ;
   _tcscpy(img_name, new_img_name) ;
   
   gbitmap = new Bitmap(new_img_name);
   nWidth  = gbitmap->GetWidth();
   nHeight = gbitmap->GetHeight();
   sprite_dx = nWidth / tiles_x ;
   sprite_dy = nHeight / tiles_y ;
   // syslog(_T("open: %s, width: %u, height: %u, sprite size: %u x %u\n"), 
   //    new_img_name, nWidth, nHeight, sprite_dx, sprite_dy) ;
}

//********************************************************************
gdi_plus::gdi_plus(TCHAR *new_img_name, uint icons_per_column, uint icon_rows, uint sprite_width, uint sprite_height) :
   img_name(NULL),
   gbitmap(NULL),
   nWidth(0),
   nHeight(0),
   sprite_dx(sprite_width),
   sprite_dy(sprite_height),
   tiles_x(icons_per_column),
   tiles_y(icon_rows)
{
   img_name = new TCHAR[_tcslen(new_img_name) +1] ;
   _tcscpy(img_name, new_img_name) ;
   
   gbitmap = new Bitmap(new_img_name);
   nWidth  = gbitmap->GetWidth();
   nHeight = gbitmap->GetHeight();
   // sprite_dx = nWidth / tiles_x ;
   // sprite_dy = nHeight / tiles_y ;
   // syslog(_T("open: %s, width: %u, height: %u, sprite size: %u x %u\n"), 
   //    new_img_name, nWidth, nHeight, sprite_dx, sprite_dy) ;
}

//********************************************************************
gdi_plus::~gdi_plus()
{
   if (img_name != NULL) {
      delete [] img_name ;
   }
   delete gbitmap ;  //lint !e1551
   gbitmap = NULL ;
}

