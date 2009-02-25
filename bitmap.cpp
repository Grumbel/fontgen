/*  $Id$
**   __      __ __             ___        __   __ __   __
**  /  \    /  \__| ____    __| _/_______/  |_|__|  | |  |   ____
**  \   \/\/   /  |/    \  / __ |/  ___/\   __\  |  | |  | _/ __ \
**   \        /|  |   |  \/ /_/ |\___ \  |  | |  |  |_|  |_\  ___/
**    \__/\  / |__|___|  /\____ /____  > |__| |__|____/____/\___  >
**         \/          \/      \/    \/                         \/
**  Copyright (C) 2005 Ingo Ruhnke <grumbel@gmx.de>
**
**  This program is free software; you can redistribute it and/or
**  modify it under the terms of the GNU General Public License
**  as published by the Free Software Foundation; either version 2
**  of the License, or (at your option) any later version.
**
**  This program is distributed in the hope that it will be useful,
**  but WITHOUT ANY WARRANTY; without even the implied warranty of
**  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**  GNU General Public License for more details.
** 
**  You should have received a copy of the GNU General Public License
**  along with this program; if not, write to the Free Software
**  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
**  02111-1307, USA.
*/

#include <stdexcept>
#include <sstream>
#include <assert.h>
#include <stdio.h>
#include <jpeglib.h>
#include <iostream>
#include <algorithm>
#include <fstream>
#include <string.h>
#include "bitmap.hpp"

Bitmap::Bitmap(int width_, int height_)
  : width(width_),
    height(height_),
    buffer(new unsigned char[width * height])
{
  memset(buffer, 255, width * height);
}

Bitmap::~Bitmap()
{
  delete[] buffer;
}

void
Bitmap::clear()
{
  memset(buffer, 255, width * height);
}

void
Bitmap::blit(const Bitmap& source, int x_pos, int y_pos)
{
  int start_x = std::max(0, -x_pos);
  int start_y = std::max(0, -y_pos);
  
  int end_x = std::min(source.width,  width  - x_pos);
  int end_y = std::min(source.height, height - y_pos);
 
  for(int y = start_y; y < end_y; ++y)
    for(int x = start_x; x < end_x; ++x)
      { // opaque blit, could use alpha/add blit instead
        buffer[(y + y_pos) * width + (x + x_pos)] = source.buffer[y * source.width + x];
      }
}

void 
Bitmap::write_pgm(const std::string& filename)
{
  std::ofstream out(filename.c_str());
  
  out << "P2" << std::endl;
  out << "# txt2png" << std::endl;
  out << get_width() << " " << get_height() << std::endl;
  out << "255" << std::endl;

  for(int y = 0; y < get_height(); ++y)
    for(int x = 0; x < get_width(); ++x)
      {
        out << int(at(x, y)) << " ";
      }
  out << std::endl;
}

void
Bitmap::write_jpg(const std::string& filename)
{
  struct jpeg_compress_struct cinfo;
  struct jpeg_error_mgr jerr;
  
  /* More stuff */
  FILE * outfile;		/* target file */

  cinfo.err = jpeg_std_error(&jerr);
  /* Now we can initialize the JPEG compression object. */
  jpeg_create_compress(&cinfo);

  if ((outfile = fopen(filename.c_str(), "wb")) == NULL)
    {
      std::cerr << "can't open "  << filename << std::endl;
      return;
    }
  jpeg_stdio_dest(&cinfo, outfile);

  cinfo.image_width  = get_width();
  cinfo.image_height = get_height();
  cinfo.input_components = 1;	//3	/* # of color components per pixel */
  cinfo.in_color_space = JCS_GRAYSCALE; /* colorspace of input image */

  jpeg_set_defaults(&cinfo);

  jpeg_set_quality(&cinfo, 85, TRUE /* limit to baseline-JPEG values */);

  /* TRUE ensures that we will write a complete interchange-JPEG file.
   * Pass TRUE unless you are very sure of what you're doing. */
  jpeg_start_compress(&cinfo, TRUE);

  JSAMPROW row_pointer[get_height()];	/* pointer to JSAMPLE row[s] */

  for(int y = 0; y < get_height(); ++y)
    row_pointer[y] = &buffer[y * get_width()];

  while (cinfo.next_scanline < cinfo.image_height) 
    {
      jpeg_write_scanlines(&cinfo, row_pointer, get_height());
    }

  jpeg_finish_compress(&cinfo);

  fclose(outfile);

  jpeg_destroy_compress(&cinfo);
}

unsigned char
Bitmap::at(int x, int y)
{
  return buffer[y * width + x];
}

void
Bitmap::invert(int x1, int y1, int x2, int y2)
{
  x1 = std::max(0, x1);
  y1 = std::max(0, y1);

  x2 = std::min(width,  x2);
  y2 = std::min(height, y2);

  for(int y = y1; y < y2; ++y)
    for(int x = x1; x < x2; ++x)
      {
        buffer[y * width + x] = 255 - buffer[y * width + x];
      }
}

void
Bitmap::fill(int x1, int y1, int x2, int y2, unsigned char c)
{  
  x1 = std::max(0, x1);
  y1 = std::max(0, y1);

  x2 = std::min(width,  x2);
  y2 = std::min(height, y2);

  for(int y = y1; y < y2; ++y)
    for(int x = x1; x < x2; ++x)
      {
        buffer[y * width + x] = c;
      }
}

void
Bitmap::truncate_height(int height_)
{
  if (height_ > height)
    {
      std::ostringstream str;
      str << "image height to small, increase it to " << height_;
      throw std::runtime_error(str.str());
    }

  height = height_;
}

/* EOF */
