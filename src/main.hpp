//  SuperTux Tile Manager - A utility for SuperTux to manage tiles
//  Copyright (C) 2021 Semphris <semphris@protonmail.com>
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

#ifndef _HEADER_STTILEMAN_MAIN_HPP
#define _HEADER_STTILEMAN_MAIN_HPP

#include <memory>
#include <string>
#include <vector>

#include "util/rect.hpp"

#include "scene.hpp"

void change_scene(std::unique_ptr<Scene> scene);

struct Tile
{
  // Step 1
  std::string texture_file;
  Rect srcrect;

  // Step 2
  short mask_up = 1;
  short mask_left = 1;
  short mask_down = 1;
  short mask_right = 1;
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

#endif
