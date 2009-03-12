#include <set>
#include <sstream>
#include <stdexcept>
#include <algorithm>
#include <stdlib.h>
#include <string>
#include <stdexcept>
#include <vector>
#include <iostream>
#include <fstream>
#include "utf8.hpp"

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_GLYPH_H

#include "bitmap.hpp"

FT_Library library;

struct Glyph 
{
  Bitmap*  bitmap;
  FT_ULong charcode;
  int advance;
  int x_offset;
  int y_offset;
};

std::string unicode_to_utf8(FT_ULong v)
{
  if (v < 0x7f)
    {
      std::string str(1, ' ');
      str[0] = v;
      return str;
    }
  else if (v < 0x7ff)
    {
      std::string str(2, ' ');
      str[0] = 0xc0 | ((0x7c0 & v) >> 6);
      str[1] = 0x80 | ((0x03f & v) >> 0);
      return str;      
    }
  else if (v < 0xFFFF)
    {
      std::string str(3, ' ');
      str[0] = 0xe0 | ((0xf000 & v) >> 12);
      str[1] = 0x80 | ((0x0fc0 & v) >>  6);
      str[2] = 0x80 | ((0x003f & v) >>  0);
      return str;      
    }
  else if (v < 0x10FFFF)
    {
      std::string str(4, ' ');
      str[0] = 0xf0 | ((0x1c0000 & v) >> 18);
      str[1] = 0x80 | ((0x03f000 & v) >> 12);
      str[2] = 0x80 | ((0x000fc0 & v) >>  6);
      str[3] = 0x80 | ((0x00003f & v) >>  0);
      return str;      
    }
  else
    {
      std::ostringstream str;
      str << "unicode out of range: " << v;
      throw std::runtime_error(str.str());
    }
}

bool glyhp_height_sorter(const Glyph& lhs, const Glyph& rhs)
{
  return lhs.bitmap->get_height() > rhs.bitmap->get_height();
}

void generate_image(std::vector<Glyph>& glyphs, 
                    int font_height, 
                    int border, 
                    int image_width,
                    int image_height,
                    const std::string& pgm_filename,
                    const std::string& metadata_filename)
{
  std::ofstream metadata(metadata_filename.c_str());

  metadata << "(pingus-font" << std::endl;
  metadata << "  (size " << font_height << ")" << std::endl;
  metadata << "  (glyph-count " << glyphs.size() << ")" << std::endl;
  metadata << "  (glyphs " << std::endl;

  Bitmap image_bitmap(image_width, image_height); // FIXME: hardcoded height is evil 

  int x_pos = 0;
  int y_pos = 0;
  int row_height = 0;

  std::sort(glyphs.begin(), glyphs.end(), glyhp_height_sorter);

  for(std::vector<Glyph>::iterator i = glyphs.begin(); i != glyphs.end(); ++i)
    {
      Glyph& glyph = *i;

      if (x_pos + glyph.bitmap->get_width() + 2*border > image_bitmap.get_width())
        {
          x_pos = border;
          y_pos += row_height + 2*border;
          row_height = glyph.bitmap->get_height();

          image_bitmap.blit(*glyph.bitmap, x_pos+border, y_pos+border);
        }
      else
        {
          row_height = std::max(row_height, glyph.bitmap->get_height());
          
          image_bitmap.blit(*glyph.bitmap, x_pos+border, y_pos+border);
        }

      metadata << "    (glyph "
               << "(unicode " << glyph.charcode << ") "
               << "(offset " << glyph.x_offset << " " << glyph.y_offset << ") "
               << "(advance " << glyph.advance << ") "
               << "(rect "
               << x_pos << " " << y_pos << " " 
               << x_pos+glyph.bitmap->get_width()+border*2 << " " << y_pos+glyph.bitmap->get_height()+border*2 << ")"
               << ")" 
               << " ;; " << unicode_to_utf8(glyph.charcode)
               << std::endl;

      x_pos += glyph.bitmap->get_width() + 2*border;
    }

  metadata << "  ))" << std::endl;
  metadata << ";; EOF ;;" << std::endl;

  image_bitmap.truncate_height(y_pos + row_height + border);
  image_bitmap.write_pgm(pgm_filename);
  std::cout << "ImageSize: " << image_bitmap.get_width() << "x" << image_bitmap.get_height() << std::endl;
}

void generate_font(const std::string& filename, 
                   int px_size, 
                   const std::set<FT_ULong>& unicodes,
                   std::vector<Glyph>& glyphs)
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

  {
    FT_Error   error;
    error = FT_Select_Charmap(face,  FT_ENCODING_UNICODE);
    if (error)
      {
        std::cout << "Error: Couldn't set Unicode charmap" << std::endl;
        exit(EXIT_FAILURE);
      }
  }

  std::cout << "BBox: " << px_size << " "
            << px_size * face->bbox.xMin/face->units_per_EM << " " 
            << px_size * face->bbox.yMin/face->units_per_EM << " " 
            << px_size * face->bbox.xMax/face->units_per_EM << " " 
            << px_size * face->bbox.yMax/face->units_per_EM << " " 
            << face->units_per_EM
            << std::endl;

  FT_UInt   glyph_index = 0;                                                
  FT_ULong  charcode = FT_Get_First_Char( face, &glyph_index );
  while ( glyph_index != 0 )                                            
    {                                                                                    
      if (unicodes.empty() || unicodes.find(charcode) != unicodes.end())
        {
          if (FT_Load_Glyph( face,  glyph_index, FT_LOAD_RENDER))//| FT_LOAD_FORCE_AUTOHINT))
            {
              std::cerr << "couldn't load char: " << glyph_index << " '" << char(glyph_index) << "'" << std::endl;
              //impl->characters.push_back(0);
            }
          else
            {
              Bitmap* glyph_bitmap = new Bitmap(face->glyph->bitmap.width, face->glyph->bitmap.rows);
              {
                for(int y = 0; y < glyph_bitmap->get_height(); ++y)
                  for(int x = 0; x < glyph_bitmap->get_width(); ++x)
                    {
                      glyph_bitmap->get_data()[y * glyph_bitmap->get_width() + x]
                        = 255 - face->glyph->bitmap.buffer[y * face->glyph->bitmap.pitch + x];
                    }
              }

              Glyph glyph;
              glyph.bitmap   = glyph_bitmap;
              glyph.charcode = charcode;
              glyph.advance  = (face->glyph->advance.x >> 6);
              glyph.x_offset = face->glyph->bitmap_left;
              glyph.y_offset = -face->glyph->bitmap_top;
              glyphs.push_back(glyph);
            }
        }

      charcode = FT_Get_Next_Char( face, charcode, &glyph_index );
    }

  std::cout << "Glyphs(intern):   " << face->num_glyphs << std::endl;
  std::cout << "Glyphs(exported): " << glyphs.size() << std::endl;
  
  FT_Done_Face(face);
}
  
void print_usage(const char* arg0)
{
  std::cout << "Usage: " << arg0 << " generate TTFFILE SIZE BORDER IMAGEWIDTH IMAGEHEIGHT [UNICODES]\n"
            << "       " << arg0 << " listchars TTFFILE" << std::endl;
  exit(EXIT_FAILURE);
}

void list_chars(const std::string& filename)
{
  try 
    {
      FT_Error   error;
  
      error = FT_Init_FreeType(&library);
      if (error)
        {
          throw std::runtime_error("could not initialize FreeType");   
        }
      else
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
      
          FT_Set_Pixel_Sizes(face, 12, 12);

          {
            FT_Error   error;
            error = FT_Select_Charmap(face,  FT_ENCODING_UNICODE);
            if (error)
              {
                std::cout << "Error: Couldn't set Unicode charmap" << std::endl;
                exit(EXIT_FAILURE);
              }
          }

          FT_UInt   glyph_index = 0;                                                
          FT_ULong  charcode = FT_Get_First_Char( face, &glyph_index );
          while ( glyph_index != 0 )                                            
            {
              std::cout << unicode_to_utf8(charcode) << std::endl;
              charcode = FT_Get_Next_Char( face, charcode, &glyph_index );
            }
  
          FT_Done_Face(face);
        
          FT_Done_FreeType(library);
        }
    } 
  catch(std::exception& err)
    {
      std::cout << "Error: " << err.what() << std::endl;
    }      
}

int main(int argc, char** argv)
{
  if (argc == 3 && strcmp(argv[1], "listchars") == 0)
    {
      list_chars(argv[2]);
    }
  else if ((argc == 7 || argc == 8) &&
           strcmp(argv[1], "generate") == 0)
    {
      std::string ttf_filename = argv[2];
      int  pixel_size   = atoi(argv[3]);
      int  border       = atoi(argv[4]);
      int  image_width  = atoi(argv[5]);
      int  image_height = atoi(argv[6]);

      std::set<FT_ULong> unicodes;

      if (argc == 8)
        {
          std::string codes = argv[7];
          UTF8::iterator i(codes);
          while(i.next())
            {
              unicodes.insert(*i);
            }
        }

      std::cout << "Generating image from " << ttf_filename << " with size " << pixel_size << " and width " << image_width << std::endl;

      try 
        {
          FT_Error   error;
  
          error = FT_Init_FreeType(&library);
          if (error)
            throw std::runtime_error("could not initialize FreeType");   

          std::vector<Glyph> glyphs;

          generate_font(ttf_filename, pixel_size, unicodes, glyphs);
          generate_image(glyphs, pixel_size, border, image_width, image_height, "/tmp/out.pgm", "/tmp/out.font");
        
          FT_Done_FreeType(library);
        } 
      catch(std::exception& err)
        {
          std::cout << "Error: " << err.what() << std::endl;
        }    
    }
  else
    {
      print_usage(argv[0]);
    }

  return 0;
}
  
/* EOF */
