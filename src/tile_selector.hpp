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

#ifndef _HEADER_STTILEMAN_TILESELECTOR_HPP
#define _HEADER_STTILEMAN_TILESELECTOR_HPP

#include "scene.hpp"

#include <vector>

#include "ui/button_label.hpp"
#include "ui/listbox.hpp"
#include "ui/scrollbar.hpp"
#include "util/vector.hpp"
#include "video/texture.hpp"

#include "tile.hpp"

class TileSelector :
  public Scene
{
public:
  TileSelector() = delete;
  TileSelector(Window& window);
  virtual ~TileSelector() = default;

  virtual void event(const SDL_Event& event) override;
  virtual void update(float dt_sec) override;
  virtual void draw() const override;

  void add_tileset();

private:
  void resize_elements();

private:
  Vector m_mouse_pos;
  int m_current_tile;

  Listbox<TileGroup*> m_tilegroups_list;
  Scrollbar m_tiles_scrollbar;
  ButtonLabel m_btn_add_tileset;
  ButtonLabel m_btn_next_step;

  bool m_dragging;
  Vector m_camera;
  std::string m_last_folder;

private:
  TileSelector(const TileSelector&) = delete;
  TileSelector& operator=(const TileSelector&) = delete;
};

#endif
