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

#ifndef _HEADER_STTILEMAN_TILEMASKSELECTOR_HPP
#define _HEADER_STTILEMAN_TILEMASKSELECTOR_HPP

#include "scene.hpp"

#include <string>
#include <vector>

#include "ui/button_label.hpp"
#include "util/color.hpp"
#include "util/rect.hpp"

#include "main.hpp"

class TileMaskSelector :
  public Scene
{
private:
  static Color get_col(short mask);

public:
  TileMaskSelector() = delete;
  TileMaskSelector(Window& window, const std::vector<Tile>& tiles);
  virtual ~TileMaskSelector() = default;

  virtual void event(const SDL_Event& event) override;
  virtual void update(float dt_sec) override;
  virtual void draw() const override;

  void next_tile();
  void prev_tile();

private:
  void resize_elements();

private:
  std::vector<Tile> m_tiles;
  int m_current_tile;
  ButtonLabel m_btn_next_tile;
  ButtonLabel m_btn_prev_tile;
  ButtonLabel m_btn_go_back;
  ButtonLabel m_btn_next_step;

private:
  TileMaskSelector(const TileMaskSelector&) = delete;
  TileMaskSelector& operator=(const TileMaskSelector&) = delete;
};

#endif
