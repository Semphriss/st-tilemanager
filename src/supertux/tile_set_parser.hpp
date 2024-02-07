//  SuperTux
//  Copyright (C) 2008 Matthias Braun <matze@braunis.de>,
//                     Ingo Ruhnke <grumbel@gmail.com>
//
//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program.  If not, see <http://www.gnu.org/licenses/>.

#ifndef HEADER_SUPERTUX_SUPERTUX_TILE_SET_PARSER_HPP
#define HEADER_SUPERTUX_SUPERTUX_TILE_SET_PARSER_HPP

#include <string>
#include <vector>

#include "tile.hpp"
#include "util/rect.hpp"

class ReaderMapping;
class Texture;
class Window;

class TileSetParser final
{
private:
  Window& m_window;
  std::vector<TileGroup>& m_tilegroups;
  std::string m_filename;
  std::string m_tiles_path;

public:
  TileSetParser(std::vector<TileGroup>& tilegroups, const std::string& filename, Window& window);

  void parse();

private:
  void parse_tiles(const ReaderMapping& reader);
  Texture* parse_imagespecs(const ReaderMapping& images_mapping, Rect& region) const;

private:
  TileSetParser(const TileSetParser&) = delete;
  TileSetParser& operator=(const TileSetParser&) = delete;
};

#endif

/* EOF */
