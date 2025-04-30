//********************************************************************
//  Copyright (c) 2025  Daniel D Miller
//  This class encapsulates the interface to the LodePNG library.
//********************************************************************

// #include <vector>
#include <gdiplus.h>

using namespace Gdiplus;

class gdi_plus {
private:   
   // std::vector<unsigned char> bmp ;
   // std::vector<unsigned char> img ;
   
   char *img_name ;
   // HBITMAP hBitmap ;
   Bitmap *gbitmap ;
   uint nWidth ;
   uint nHeight ;
   
   uint sprite_dx ;
   uint sprite_dy ;
   uint tiles_x ;
   uint tiles_y ;

   //  private functions
   // void encodeBMP(std::vector<unsigned char>& bmp, const unsigned char* image, int w, int h);
   // HBITMAP ConvertDibToHBitmap(void* bmpData);
   // HBITMAP load_png_to_bmp(void);

   //  disable the assignment operator and copy constructor
   gdi_plus &operator=(const gdi_plus &src) ;
   gdi_plus(const gdi_plus&);
   //  disable the default constructor
   gdi_plus() ;
   
public:   
   // gdi_plus();
   gdi_plus(TCHAR *new_img_name) ;
   gdi_plus(TCHAR *new_img_name, uint dx, uint dy) ;
   ~gdi_plus();
   void render_bitmap(HDC hdc, unsigned x, unsigned y);
   // void render_bitmap(HDC hdc, uint xdest, uint ydest, uint xsrc, uint ysrc);
   void render_bitmap(HDC hdc, uint xdest, uint ydest, uint sprite_col, uint sprite_row);
   void render_bitmap(HDC hdc, uint xdest, uint ydest, uint tile_index);
   uint img_width(void) const
      { return nWidth ; } ;
   uint img_height(void) const
      { return nHeight ; } ;
   uint horiz_tiles(void) const
      { return tiles_x ; } ;
   uint vert_tiles(void) const
      { return tiles_y ; } ;
} ;

