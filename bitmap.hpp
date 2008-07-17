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

#ifndef HEADER_BITMAP_HPP
#define HEADER_BITMAP_HPP

/** */
class Bitmap
{
private:
  int width;
  int height;
  unsigned char* buffer;

public:
  Bitmap(int width, int height);
  ~Bitmap();

  int get_width() { return width; }
  int get_height() { return height; }

  unsigned char at(int x, int y);

  void blit(const Bitmap& source, int x, int y);
  unsigned char* get_data() { return buffer; }
  void clear();
  void write_pgm(const std::string& filename);
  void write_jpg(const std::string& filename);

  void truncate_height(int height);

  /** Invert the given region */
  void invert(int x1, int y1, int x2, int y2);

  /** Fill the given region with color \a c */
  void fill(int x1, int y1, int x2, int y2, unsigned char c);
private:
  Bitmap (const Bitmap&);
  Bitmap& operator= (const Bitmap&);
};

#endif

/* EOF */
