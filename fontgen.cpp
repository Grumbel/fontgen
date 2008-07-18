#include <stdlib.h>
#include <string>
#include <stdexcept>
#include <vector>
#include <iostream>
#include <fstream>

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_GLYPH_H

#include "bitmap.hpp"

FT_Library library;

void generate_font(const std::string& filename, int px_size, int border, int img_width,
                   std::ostream& metadata_stream)
{
  // Read the TTF font file content into buffer
  std::ifstream fin(filename.c_str());
  std::istreambuf_iterator<char> first(fin), last;
  std::vector<char> buffer(first, last); 

  FT_Face face;
  if (FT_New_Memory_Face(library, 
                         reinterpret_cast<FT_Byte*>(&*buffer.begin()), buffer.size(), 
                         0, &face))
    {
      throw std::runtime_error("Couldn't load font: '" + filename + "'");
    }
      
  FT_Set_Pixel_Sizes(face, px_size, px_size);
  FT_Select_Charmap(face,  FT_ENCODING_UNICODE);

  std::cout << "BBox: " << px_size << " "
            << px_size * face->bbox.xMin/face->units_per_EM << " " 
            << px_size * face->bbox.yMin/face->units_per_EM << " " 
            << px_size * face->bbox.xMax/face->units_per_EM << " " 
            << px_size * face->bbox.yMax/face->units_per_EM << " " 
            << face->units_per_EM
            << std::endl;

  Bitmap image_bitmap(img_width, 4096);

  // We limit ourself to 256 characters for the momemnt
  int x_pos = 0;
  int y_pos = 0;

  //for(int glyph_index = 0; glyph_index < 256; glyph_index += 1)

  FT_ULong  charcode;                                              
  FT_UInt   glyph_index;                                                
  int row_height = 0;

  charcode = FT_Get_First_Char( face, &glyph_index );
  while ( glyph_index != 0 )                                            
    {                                                                                                      
      if (FT_Load_Char( face,  glyph_index, FT_LOAD_RENDER))//| FT_LOAD_FORCE_AUTOHINT))
        {
          std::cerr << "couldn't load char: " << glyph_index << " '" << char(glyph_index) << "'" << std::endl;
          //impl->characters.push_back(0);
        }
      else
        {
          FT_GlyphSlot glyph = face->glyph;;

          int x_offset =  glyph->bitmap_left;
          int y_offset = -glyph->bitmap_top;
          int advance = (glyph->advance.x >> 6);
          
          Bitmap glyph_bitmap(glyph->bitmap.width, glyph->bitmap.rows);

          for(int y = 0; y < glyph_bitmap.get_height(); ++y)
            for(int x = 0; x < glyph_bitmap.get_width(); ++x)
              {
                glyph_bitmap.get_data()[y * glyph_bitmap.get_width() + x] = 255 - glyph->bitmap.buffer[y * glyph->bitmap.pitch + x];
              }

          if (x_pos + glyph_bitmap.get_width() + 2*border > image_bitmap.get_width())
            {
              x_pos = border;
              y_pos += row_height + 2*border;
              row_height = glyph_bitmap.get_height();

              image_bitmap.blit(glyph_bitmap, x_pos+border, y_pos+border);
            }
          else
            {
              row_height = std::max(row_height, glyph_bitmap.get_height());
          
              image_bitmap.blit(glyph_bitmap, x_pos+border, y_pos+border);
            }

          std::cout << "(char "
                    << "(code " << charcode << ") "
                    << "(offset " << x_offset << " " << y_offset << ") "
                    << "(advance " << advance << ") "
                    << "(rect "
                    << x_pos << " " << y_pos << " " 
                    << x_pos+glyph_bitmap.get_width()+border*2 << " " << y_pos+glyph_bitmap.get_height()+border*2 << ")"
                    << ")" << std::endl;

          x_pos += glyph_bitmap.get_width() + 2*border;
        }

      charcode = FT_Get_Next_Char( face, charcode, &glyph_index );
    }

  image_bitmap.truncate_height(y_pos + row_height + border);
  image_bitmap.write_pgm("/tmp/out.pgm");

  FT_Done_Face(face);
}
  
int main(int argc, char** argv)
{
  if (argc != 5)
    {
      std::cout << "Usage: " << argv[0] << " TTFFILE SIZE BORDER WIDTH" << std::endl;
      return EXIT_FAILURE;
    }

  std::string ttf_filename = argv[1];
  int  pixel_size   = atoi(argv[2]);
  int  border       = atoi(argv[3]);
  int  image_width  = atoi(argv[4]);

  std::ofstream metadata_stream("/tmp/output.font");

  std::cout << "Generating image from " << ttf_filename << " with size " << pixel_size << " and width " << image_width << std::endl;

  try 
    {
      FT_Error   error;
  
      error = FT_Init_FreeType(&library);
      if (error)
        throw std::runtime_error("could not initialize FreeType");   
        
      generate_font(ttf_filename, pixel_size, border, image_width, 
                    std::cout);
      //metadata_stream);
        
      FT_Done_FreeType(library);
    } 
  catch(std::exception& err)
    {
      std::cout << "Error: " << err.what() << std::endl;
    }

  return 0;
}
  
/* EOF */
