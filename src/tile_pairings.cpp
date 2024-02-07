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

#include "tile_pairings.hpp"

#include <algorithm>

#include "SDL.h"

#include "util/log.hpp"
#include "video/drawing_context.hpp"
#include "video/renderer.hpp"
#include "video/window.hpp"

#include "main.hpp"
#include "tile_mask_selector.hpp"

static const Control::ThemeSet theme_set = ([]{
  Control::Theme t;
  t.bg_blend = Renderer::Blend::BLEND;
  t.bg_color = Color(.75f, .75f, .8f);
  t.fg_blend = Renderer::Blend::BLEND;
  t.fg_color = Color();
  t.font = "../data/fonts/SuperTux-Medium.ttf";
  t.fontsize = 18;

  Control::ThemeSet ts{t, t, t, t, t};
  ts.disabled.bg_color = Color(.5f, .5f, .55f);
  ts.disabled.fg_color = Color(.2f, .2f, .2f);
  ts.hover.bg_color = Color(.8f, .8f, .85f);
  ts.focus.bg_color = Color(.85f, .85f, .9f);
  ts.active.bg_color = Color(.9f, .9f, .95f);

  return ts;
})();

TilePairings::TilePairings(Window& window, const std::vector<Tile>& tiles, Texture& tiles_texture) :
  Scene(window),
  m_tiles(tiles),
  m_tiles_texture(tiles_texture),
  m_current_tile(0),
  m_current_match(0),
  m_match_direction(0),
  m_btn_yes("Yes", [this](int){ this->yes(); }, 0xff, true, 100, Rect(), theme_set, nullptr),
  m_btn_no("No", [this](int){ this->no(); }, 0xff, true, 100, Rect(), theme_set, nullptr),
  m_btn_prev("Go back", [this](int){ change_scene(std::make_unique<TileMaskSelector>(this->m_window, this->m_tiles, this->m_tiles_texture)); }, 0xff, true, 100, Rect(), theme_set, nullptr),
  m_btn_next("Next step", [this](int){  }, 0xff, true, 100, Rect(), theme_set, nullptr)
{
  resize_elements();
}

void
TilePairings::event(const SDL_Event& event)
{
  if (m_btn_yes.event(event) || m_btn_no.event(event) || m_btn_prev.event(event) || m_btn_next.event(event))
    return;

  switch (event.type)
  {
    case SDL_QUIT:
      change_scene(nullptr);
      break;

    default:
      break;
  }
}

void
TilePairings::update(float dt_sec)
{
}

void
TilePairings::draw() const
{
  auto& r = m_window.get_renderer();
  DrawingContext dc(r);

  m_btn_yes.draw(dc);
  m_btn_no.draw(dc);
  m_btn_prev.draw(dc);
  m_btn_next.draw(dc);

  dc.draw_text("Does this pairing tile properly?", Vector(r.get_window().get_size().w / 2.f, 8.f), Renderer::TextAlign::TOP_MID, "../data/fonts/SuperTux-Medium.ttf", 16, Color(1.f, 1.f, 1.f), Renderer::Blend::BLEND, 10);

  Vector mid = m_window.get_size() / 2.f;
  Rect tile_rect = Rect(mid - Vector(16.f, 16.f), Size(32.f, 32.f));

  Vector delta;
  switch(m_match_direction)
  {
    case 0:
      delta = Vector(0.f, 16.f);
      break;

    case 1:
      delta = Vector(0.f, -16.f);
      break;

    case 2:
      delta = Vector(16.f, 0.f);
      break;

    case 3:
      delta = Vector(-16.f, 0.f);
      break;

    default:
      break;
  }

  {
    const auto& src = m_tiles[m_current_tile].srcrect;
    dc.draw_texture(m_tiles_texture, src, tile_rect.moved(-delta), 0.f, Color(1.f, 1.f, 1.f), Renderer::Blend::BLEND, 1);
  }

  {
    const auto& src = m_tiles[m_current_match].srcrect;
    dc.draw_texture(m_tiles_texture, src, tile_rect.moved(delta), 0.f, Color(1.f, 1.f, 1.f), Renderer::Blend::BLEND, 1);
  }

  dc.render();
}

void
TilePairings::yes()
{
  switch (m_match_direction)
  {
    case 0:
      m_tiles[m_current_tile].in_down.push_back(&m_tiles[m_current_match]);
      m_tiles[m_current_match].in_up.push_back(&m_tiles[m_current_tile]);
      break;

    case 1:
      m_tiles[m_current_tile].in_up.push_back(&m_tiles[m_current_match]);
      m_tiles[m_current_match].in_down.push_back(&m_tiles[m_current_tile]);
      break;

    case 2:
      m_tiles[m_current_tile].in_right.push_back(&m_tiles[m_current_match]);
      m_tiles[m_current_match].in_left.push_back(&m_tiles[m_current_tile]);
      break;

    case 3:
      m_tiles[m_current_tile].in_left.push_back(&m_tiles[m_current_match]);
      m_tiles[m_current_match].in_right.push_back(&m_tiles[m_current_tile]);
      break;
  }

  next();
}

void
TilePairings::no()
{
  switch (m_match_direction)
  {
    case 0:
      m_tiles[m_current_tile].ex_down.push_back(&m_tiles[m_current_match]);
      m_tiles[m_current_match].ex_up.push_back(&m_tiles[m_current_tile]);
      break;

    case 1:
      m_tiles[m_current_tile].ex_up.push_back(&m_tiles[m_current_match]);
      m_tiles[m_current_match].ex_down.push_back(&m_tiles[m_current_tile]);
      break;

    case 2:
      m_tiles[m_current_tile].ex_right.push_back(&m_tiles[m_current_match]);
      m_tiles[m_current_match].ex_left.push_back(&m_tiles[m_current_tile]);
      break;

    case 3:
      m_tiles[m_current_tile].ex_left.push_back(&m_tiles[m_current_match]);
      m_tiles[m_current_match].ex_right.push_back(&m_tiles[m_current_tile]);
      break;
  }

  next();
}

void
TilePairings::next()
{
  m_current_match++;

  if (m_current_match >= m_tiles.size())
  {
    m_current_match = 0;
    m_current_tile++;

    if (m_current_tile >= m_tiles.size())
    {
      m_current_tile = 0;

      if (++m_match_direction > 3)
      {
        log_warn << "Done" << std::endl;
        return;
      }
    }
  }

  auto has = [](std::vector<Tile*>& array, Tile* element) {
    return std::find(array.begin(), array.end(), element) != array.end();
  };

  bool skip = false;
  auto& tile = m_tiles[m_current_tile];
  auto& match = m_tiles[m_current_match];
  switch(m_match_direction)
  {
    case 0:
      if (tile.mask_down != match.non_solid + 1)
      {
        skip = true;
        break;
      }

      if (has(tile.ex_down, &match) || has(tile.in_down, &match))
        skip = true;
      break;

    case 1:
      if (tile.mask_up != match.non_solid + 1)
      {
        skip = true;
        break;
      }

      if (has(tile.ex_up, &match) || has(tile.in_up, &match))
        skip = true;
      break;

    case 2:
      if (tile.mask_right != match.non_solid + 1)
      {
        skip = true;
        break;
      }

      if (has(tile.ex_right, &match) || has(tile.in_right, &match))
        skip = true;
      break;

    case 3:
      if (tile.mask_left != match.non_solid + 1)
      {
        skip = true;
        break;
      }

      if (has(tile.ex_left, &match) || has(tile.in_left, &match))
        skip = true;
      break;

  }

  if (skip)
    next();
}

void
TilePairings::resize_elements()
{
  m_btn_yes.get_rect() = Rect(0.f, m_window.get_size().h - 32.f, m_window.get_size().w / 4.f, m_window.get_size().h);
  m_btn_no.get_rect() = Rect(m_window.get_size().w / 4.f, m_window.get_size().h - 32.f, m_window.get_size().w / 2.f, m_window.get_size().h);
  m_btn_prev.get_rect() = Rect(m_window.get_size().w / 2.f, m_window.get_size().h - 32.f, m_window.get_size().w * 3.f / 4.f, m_window.get_size().h);
  m_btn_next.get_rect() = Rect(m_window.get_size().w * 3.f / 4.f, m_window.get_size().h - 32.f, m_window.get_size().w, m_window.get_size().h);
}
