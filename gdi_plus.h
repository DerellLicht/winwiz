//********************************************************************
//  Copyright (c) 2025  Daniel D Miller
//  This class encapsulates the interface to the LodePNG library.
//********************************************************************
//  WARNING about instantiating this class:
//  
//  It *CANNOT* be statically created like this:
//  The problem is that the gdiplus startup functions have not yet
//  been executed at this point, so the new() operations in this class will fail.
// static gdi_plus pngSprites(_T("tiles32.png"), 40, 26) ;
// static gdi_plus pngTiles  (_T("images.png"), 3, 1) ;
//  
//  Instead, the classes need to be created as pointers:
// static gdi_plus *pngSprites = NULL ;
// static gdi_plus *pngTiles = NULL;
//  Then, at runtime, after the gdiplus startup functions have been executed,
//  then these classes can be dynamically allocated:
//    pngSprites = new gdi_plus(_T("tiles32.png"), 40, 26) ;
//    pngTiles   = new gdi_plus(_T("images.png"), 3, 1) ;
//********************************************************************

#include <gdiplus.h>

//lint -esym(1714, gdi_plus::gdi_plus, gdi_plus::img_width, gdi_plus::img_height)
//lint -esym(1714, gdi_plus::horiz_tiles, gdi_plus::vert_tiles)
//lint -esym(1714, gdi_plus::get_sprite_dx, gdi_plus::get_sprite_dy)

using namespace Gdiplus;

class gdi_plus {
private:   
   TCHAR *img_name ;
   Bitmap *gbitmap ;
   uint nWidth ;
   uint nHeight ;
   
   uint sprite_dx ;
   uint sprite_dy ;
   uint tiles_x ;
   uint tiles_y ;

   //  private functions
   void copy_imagelist_item(Graphics& graphics, int xsrc, int ysrc, int dx, int dy, int xdest, int ydest);

   //  disable the assignment operator and copy constructor
   gdi_plus &operator=(const gdi_plus &src) ;
   gdi_plus(const gdi_plus&);
   //  disable the default constructor
   gdi_plus() ;
   
public:   
   // gdi_plus();
   gdi_plus(TCHAR *new_img_name) ;
   // gdi_plus(TCHAR *new_img_name, uint dx, uint dy) ;
   gdi_plus(TCHAR *new_img_name, uint icons_per_column, uint icon_rows);
   gdi_plus(TCHAR *new_img_name, uint icons_per_column, uint icon_rows, uint sprite_width, uint sprite_height);
   ~gdi_plus();
   // void render_bitmap(HDC hdc, unsigned x, unsigned y);
   void render_bitmap(HDC hdc, uint xdest, uint ydest, uint sprite_col, uint sprite_row);
   // void render_bitmap(HDC hdc, uint xdest, uint ydest, uint tile_index);
   uint img_width(void) const
      { return nWidth ; } ;
   uint img_height(void) const
      { return nHeight ; } ;
   uint horiz_tiles(void) const
      { return tiles_x ; } ;
   uint vert_tiles(void) const
      { return tiles_y ; } ;
   uint get_sprite_dx(void) const
      { return sprite_dx ; } ;
   uint get_sprite_dy(void) const
      { return sprite_dy ; } ;
} ;

