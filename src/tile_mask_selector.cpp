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

#include "tile_mask_selector.hpp"

#include "SDL.h"

#include "video/drawing_context.hpp"
#include "video/renderer.hpp"
#include "video/window.hpp"

#include "tile_pairings.hpp"
#include "tile_selector.hpp"

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

Color
TileMaskSelector::get_col(short mask)
{
  return Color((mask & 0x1) ? .8f : .2f, (mask & 0x2) ? .8f : .2f, (mask & 0x4) ? .8f : .2f);
}

TileMaskSelector::TileMaskSelector(Window& window, const std::vector<Tile>& tiles, Texture& tiles_texture) :
  Scene(window),
  m_tiles(tiles),
  m_tiles_texture(tiles_texture),
  m_current_tile(0),
  m_btn_next_tile("Next tile", [this](int){ this->next_tile(); }, 0xff, true, 100, Rect(), theme_set, nullptr),
  m_btn_prev_tile("Prev. tile", [this](int){ this->prev_tile(); }, 0xff, true, 100, Rect(), theme_set, nullptr),
  m_btn_go_back("Go back", [this](int){ change_scene(std::make_unique<TileSelector>(this->m_window, this->m_tiles)); }, 0xff, true, 100, Rect(), theme_set, nullptr),
  m_btn_next_step("Next step", [this](int){ change_scene(std::make_unique<TilePairings>(this->m_window, this->m_tiles, this->m_tiles_texture)); }, 0xff, true, 100, Rect(), theme_set, nullptr)
{
  resize_elements();
}

void
TileMaskSelector::event(const SDL_Event& event)
{
  if (m_btn_next_tile.event(event) || m_btn_prev_tile.event(event) || m_btn_go_back.event(event) || m_btn_next_step.event(event))
    return;

  switch (event.type)
  {
    case SDL_QUIT:
      change_scene(nullptr);
      break;

    case SDL_MOUSEBUTTONUP:
    {
      Rect tile_rect = Rect(m_window.get_size().vector() / 2.f - Vector(16.f, 16.f), Size(32.f, 32.f));
      if (tile_rect.moved(Vector(32, 0)).contains(Vector(event.button.x, event.button.y)))
      {
        m_tiles[m_current_tile].mask_right %= 7;
        m_tiles[m_current_tile].mask_right++;
      }
      else if (tile_rect.moved(Vector(0, 32)).contains(Vector(event.button.x, event.button.y)))
      {
        m_tiles[m_current_tile].mask_down %= 7;
        m_tiles[m_current_tile].mask_down++;
      }
      else if (tile_rect.moved(Vector(-32, 0)).contains(Vector(event.button.x, event.button.y)))
      {
        m_tiles[m_current_tile].mask_left %= 7;
        m_tiles[m_current_tile].mask_left++;
      }
      else if (tile_rect.moved(Vector(0, -32)).contains(Vector(event.button.x, event.button.y)))
      {
        m_tiles[m_current_tile].mask_up %= 7;
        m_tiles[m_current_tile].mask_up++;
      }
      else if (tile_rect.contains(Vector(event.button.x, event.button.y)))
      {
        m_tiles[m_current_tile].non_solid = !m_tiles[m_current_tile].non_solid;
      }
    }
      break;

    default:
      break;
  }
}

void
TileMaskSelector::draw() const
{
  auto& r = m_window.get_renderer();
  DrawingContext dc(r);
  m_btn_next_tile.draw(dc);
  m_btn_prev_tile.draw(dc);
  m_btn_go_back.draw(dc);
  m_btn_next_step.draw(dc);
  dc.draw_text("red = empty       green = solid       blue = non-solid", Vector(r.get_window().get_size().w / 2.f, 8.f), Renderer::TextAlign::TOP_MID, "../data/fonts/SuperTux-Medium.ttf", 14, Color(1.f, 1.f, 1.f), Renderer::Blend::BLEND, 10);

  dc.draw_filled_rect(m_window.get_size(), Color(), Renderer::Blend::NONE, -100);

  Vector mid = m_window.get_size() / 2.f;
  Rect tile_rect = Rect(mid - Vector(16.f, 16.f), Size(32.f, 32.f));
  const auto& t = m_tiles_texture;
  const auto& src = m_tiles[m_current_tile].srcrect;
  dc.draw_texture(t, src, tile_rect, 0.f, m_tiles[m_current_tile].non_solid ? Color(1.f, .5f, .5f) : Color(1.f, 1.f, 1.f), Renderer::Blend::BLEND, 1);
  dc.draw_filled_rect(tile_rect.moved(Vector(32.f, 0.f)), get_col(m_tiles[m_current_tile].mask_right), Renderer::Blend::BLEND, 1);
  dc.draw_filled_rect(tile_rect.moved(Vector(0, -32.f)), get_col(m_tiles[m_current_tile].mask_up), Renderer::Blend::BLEND, 1);
  dc.draw_filled_rect(tile_rect.moved(Vector(-32.f, 0.f)), get_col(m_tiles[m_current_tile].mask_left), Renderer::Blend::BLEND, 1);
  dc.draw_filled_rect(tile_rect.moved(Vector(0, 32.f)), get_col(m_tiles[m_current_tile].mask_down), Renderer::Blend::BLEND, 1);

  dc.render();
}

void
TileMaskSelector::next_tile()
{
  if (m_current_tile < m_tiles.size() - 1)
  {
    m_current_tile++;
    m_btn_prev_tile.set_disabled(false);
    m_btn_next_tile.set_disabled(m_current_tile >= m_tiles.size() - 1);
  }
}

void
TileMaskSelector::prev_tile()
{
  if (m_current_tile > 0)
  {
    m_current_tile--;
    m_btn_prev_tile.set_disabled(m_current_tile <= 0);
    m_btn_next_tile.set_disabled(false);
  }
}

void
TileMaskSelector::resize_elements()
{
  m_btn_next_tile.get_rect() = Rect(0.f, m_window.get_size().h - 32.f, m_window.get_size().w / 4.f, m_window.get_size().h);
  m_btn_prev_tile.get_rect() = Rect(m_window.get_size().w / 4.f, m_window.get_size().h - 32.f, m_window.get_size().w / 2.f, m_window.get_size().h);
  m_btn_go_back.get_rect() = Rect(m_window.get_size().w / 2.f, m_window.get_size().h - 32.f, m_window.get_size().w * 3.f / 4.f, m_window.get_size().h);
  m_btn_next_step.get_rect() = Rect(m_window.get_size().w * 3.f / 4.f, m_window.get_size().h - 32.f, m_window.get_size().w, m_window.get_size().h);
}
