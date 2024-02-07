//  SuperTux Tile Manager - A utility for SuperTux to manage tiles
//  Copyright (C) 2021 Semphris <semphris@protonmail.com>
//                2024 Vankata453
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

#ifndef _HEADER_STTILEMAN_TILE_HPP
#define _HEADER_STTILEMAN_TILE_HPP

#include <string>
#include <vector>
#include <cstdint>

#include "util/rect.hpp"

class Texture;
struct TileGroup;

struct Tile
{
  Tile(uint32_t id_, const Rect& src_rect) :
    id(id_),
    srcrect(src_rect),
    mask_up(1),
    mask_left(1),
    mask_down(1),
    mask_right(1),
    non_solid(false)
  {}

  // Step 1
  uint32_t id;
  Rect srcrect;

  // Step 2
  short mask_up;
  short mask_left;
  short mask_down;
  short mask_right;
  bool non_solid;

  // Step 3
  std::vector<Tile*> in_up;
  std::vector<Tile*> ex_up;
  std::vector<Tile*> in_left;
  std::vector<Tile*> ex_left;
  std::vector<Tile*> in_down;
  std::vector<Tile*> ex_down;
  std::vector<Tile*> in_right;
  std::vector<Tile*> ex_right;
};

struct TileGroup
{
  TileGroup(unsigned int w, unsigned int h,
            std::vector<Tile> tiles_, Texture* texture_,
            const Rect& region_) :
    width(w),
    height(h),
    tiles(std::move(tiles_)),
    texture(texture_),
    region(region_)
  {}

  const unsigned int width;
  const unsigned int height;
  const std::vector<Tile> tiles;

  Texture* const texture;
  const Rect region;
};

#endif
