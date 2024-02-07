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

#ifndef _HEADER_STTILEMAN_TILEPAIRINGS_HPP
#define _HEADER_STTILEMAN_TILEPAIRINGS_HPP

#include "scene.hpp"

#include <vector>

#include "ui/button_label.hpp"

#include "tile.hpp"

class TilePairings :
  public Scene
{
public:
  TilePairings() = delete;
  TilePairings(Window& window);
  virtual ~TilePairings() = default;

  virtual void event(const SDL_Event& event) override;
  virtual void update(float dt_sec) override;
  virtual void draw() const override;

  void yes();
  void no();

private:
  void next();

private:
  void resize_elements();

private:
  int m_current_tile;
  int m_current_match;
  int m_match_direction;
  ButtonLabel m_btn_yes;
  ButtonLabel m_btn_no;
  ButtonLabel m_btn_prev;
  ButtonLabel m_btn_next;

private:
  TilePairings(const TilePairings&) = delete;
  TilePairings& operator=(const TilePairings&) = delete;
};

#endif
