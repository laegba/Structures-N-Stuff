/*

  Vector texture:

  Scalable Vector Graphics Parsing Test Code
  M. Salay 20150118

  Testing SVG rasterization with LibRsvg
    - currently sending to PNG
      - To use as texture it would be better to just store as a bitmap in memory 
        rather than writing to a file (and potentially compressing image).
  
  API Info: https://developer.gnome.org/rsvg/2.40/RsvgHandle.html
     How to use surface as texture in OpenGL?
     
    Scaling and its impact on image file size and image quality 
      has been demonstrated...
    
  Perhaps split up features on a separate layer so that these items can be added
    after the original draw.
     
 */

#include <iostream>
#include <librsvg/rsvg.h>
#include <librsvg/rsvg-cairo.h>
using namespace std;

int main(int argc, const char * argv[])
{
  // I'll leave this here...
  cout << "Hello World!\n";


  const char *filein ="tank3.svg";
  const char *fileout ="out.png";

  GError *error=NULL;
  RsvgHandle *handle;
  RsvgDimensionData dim;
  int width, height, scale;
  scale=2;

  cairo_surface_t *surface;
  cairo_t *cs;
//  cairo_status_t status;  // use to verify when needed

  g_type_init ();
  handle= rsvg_handle_new_from_file (filein, &error);

  rsvg_handle_get_dimensions (handle, &dim);
  width= dim.width;
  height= dim.height;

  surface= cairo_image_surface_create (CAIRO_FORMAT_ARGB32, scale*width, scale*height);
  cs= cairo_create (surface);
  cairo_scale (cs,scale,scale);
  rsvg_handle_render_cairo (handle, cs);
  cairo_surface_write_to_png (surface, fileout);

  cairo_destroy (cs);
  cairo_surface_destroy (surface);

  return 0;
}

