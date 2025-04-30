//********************************************************************
//  Copyright (c) 2025  Daniel D Miller
//  This class encapsulates the interface to the gdi_plus library.
//********************************************************************

#include <windows.h>
#include <tchar.h>
#include <string>

#include "common.h"
#include "gdi_plus.h"

using namespace Gdiplus;

//***********************************************************************************
void gdi_plus::copy_imagelist_item(Graphics& graphics, int xsrc, int ysrc, int dx, int dy, int xdest, int ydest)
{
   Bitmap* clone = gbitmap->Clone(Rect(xsrc, ysrc, dx, dy), PixelFormatDontCare);
   graphics.DrawImage(clone, xdest, ydest);
   delete clone ;
}  //lint !e818

//***********************************************************************
//  this form of the renderer is for drawing single images
//***********************************************************************
// void gdi_plus::render_bitmap(HDC hdc, unsigned xdest, unsigned ydest)
// {
//    HDC hdcMem = CreateCompatibleDC (hdc);
//    SelectObject (hdcMem, (HGDIOBJ) hBitmap);
// 
//    if (!BitBlt (hdc, xdest, ydest, sprite_dx, sprite_dy, hdcMem, 0, 0, SRCCOPY)) {  //lint !e713
//       syslog(_T("BitBlt: %s") get_system_message()) ;
//    }
// 
//    DeleteDC (hdcMem);
// }

//********************************************************************
//  
//********************************************************************
void gdi_plus::render_bitmap(HDC hdc, uint xdest, uint ydest, uint sprite_col, uint sprite_row)
{
#ifdef  UNICODE
   Graphics graphics(hdc);
   // sprite_dx = nWidth / tiles_x ;
   // sprite_dy = nHeight / tiles_y ;
   uint iconx = sprite_col * sprite_dx ;
   uint icony = sprite_row * sprite_dy ;
   copy_imagelist_item(graphics, iconx, icony, sprite_dx, sprite_dy, xdest, ydest);

#else
   HDC hdcMem = CreateCompatibleDC (hdc);
   SelectObject (hdcMem, (HGDIOBJ) hBitmap);
   
   uint xsrc = sprite_col * sprite_dx  ;
   uint ysrc = sprite_row * sprite_dy ;
   if (!BitBlt (hdc, xdest, ydest, sprite_dx, sprite_dy, hdcMem, xsrc, ysrc, SRCCOPY)) {  //lint !e713
      syslog(_T("BitBlt: %s") get_system_message()) ;
   }
   DeleteDC (hdcMem);
#endif
}

//********************************************************************
// void gdi_plus::render_bitmap(HDC hdc, uint xdest, uint ydest, uint tile_index)
// {
//    HDC hdcMem = CreateCompatibleDC (hdc);
//    SelectObject (hdcMem, (HGDIOBJ) hBitmap);
//    uint x_idx = tile_index % tiles_x ;
//    uint y_idx = tile_index / tiles_x ;
//    uint xsrc = x_idx * sprite_dx ;
//    uint ysrc = y_idx * sprite_dy ;
//    
//    if (!BitBlt (hdc, xdest, ydest, sprite_dx, sprite_dy, hdcMem, xsrc, ysrc, SRCCOPY)) {  //lint !e713
//       syslog(_T("BitBlt: %s") get_system_message()) ;
//    }
//    
//    DeleteDC (hdcMem);
// }

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
   Bitmap *gbitmap = new Bitmap(new_img_name);
   nWidth  = gbitmap->GetWidth();
   nHeight = gbitmap->GetHeight();

}

//********************************************************************
gdi_plus::gdi_plus(TCHAR *new_img_name, uint icons_per_column, uint icon_rows) :
   // bmp(NULL),
   // img(NULL),
   img_name(NULL),
   gbitmap(NULL),
   nWidth(0),
   nHeight(0),
   sprite_dx(0),
   sprite_dy(0),
   tiles_x(icons_per_column),
   tiles_y(icon_rows)
{
   img_name = new TCHAR[_tcslen(new_img_name)+1] ;
   _tcscpy(img_name, new_img_name) ;
   
   // hBitmap = load_png_to_bmp() ; //  this will init bmp, img
   Bitmap *gbitmap = new Bitmap(new_img_name);
   nWidth  = gbitmap->GetWidth();
   nHeight = gbitmap->GetHeight();
   syslog(_T("open: %s\n"), new_img_name);
   syslog(_T("width: %u, height: %u\n"), nWidth, nHeight);
   sprite_dx = nWidth / tiles_x ;
   sprite_dy = nHeight / tiles_y ;
   syslog(_T("sprite size: %u x %u\n"), sprite_dx, sprite_dy) ;

}

//********************************************************************
gdi_plus::~gdi_plus()
{
   if (img_name != NULL)
      delete [] img_name ;
   delete gbitmap ;
   gbitmap = NULL ;
}

