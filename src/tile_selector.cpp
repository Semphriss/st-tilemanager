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

#include "tile_selector.hpp"

#include "SDL.h"
#include "portable-file-dialogs.h"

#include "ui/button_label.hpp"
#include "util/log.hpp"
#include "video/drawing_context.hpp"
#include "video/window.hpp"

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

static const Control::ThemeSet scrollbar_theme_set = ([]{
  Control::Theme t;
  t.fg_blend = Renderer::Blend::BLEND;
  t.fg_color = Color(.75f, .75f, .8f);
  t.bg_blend = Renderer::Blend::BLEND;
  t.bg_color = Color();
  t.font = "../data/fonts/SuperTux-Medium.ttf";
  t.fontsize = 18;

  Control::ThemeSet ts{t, t, t, t, t};
  ts.disabled.fg_color = Color(.5f, .5f, .55f);
  ts.disabled.bg_color = Color(.2f, .2f, .2f);
  ts.hover.fg_color = Color(.8f, .8f, .85f);
  ts.focus.fg_color = Color(.85f, .85f, .9f);
  ts.active.fg_color = Color(.9f, .9f, .95f);

  return ts;
})();

TileSelector::TileSelector(Window& window, const std::vector<Tile>& tiles) :
  Scene(window),
  m_mouse_pos(),
  m_textures(),
  m_current_texture(-1),
  m_tiles(tiles),
  m_tiles_scrollbar(nullptr, window.get_size().h, 0.f, false, 0xff, 100, Rect(),
                    scrollbar_theme_set, nullptr),
  m_btn_add_image("Open image", [this](int){ this->add_image(); }, 0xff, true, 100, Rect(), theme_set, nullptr),
  m_btn_next_step("Next step", [this](int){ change_scene(std::make_unique<TileMaskSelector>(this->m_window, this->m_tiles)); }, 0xff, true, 100, Rect(), theme_set, nullptr),
  m_dragging(false),
  m_camera(0.f, 0.f),
  m_last_folder()
{
  resize_elements();
}

void
TileSelector::event(const SDL_Event& event)
{
  if (m_tiles_scrollbar.is_valid() && m_tiles_scrollbar.event(event)
      || m_btn_add_image.event(event) || m_btn_next_step.event(event))
    return;

  switch (event.type)
  {
    case SDL_QUIT:
      change_scene(nullptr);
      break;

    case SDL_MOUSEMOTION:
      m_mouse_pos = Vector(event.motion.x, event.motion.y);
      if (m_dragging)
        m_camera += Vector(event.motion.xrel, event.motion.yrel);
      break;

    case SDL_MOUSEBUTTONDOWN:
      if (event.button.button == SDL_BUTTON_RIGHT &&
          Rect(0.f, 0.f, m_window.get_size().w - (m_tiles_scrollbar.is_valid() ? 37.f : 32.f), m_window.get_size().h - 32.f).contains(m_mouse_pos))
      {
        m_dragging = true;
      }
      break;

    case SDL_MOUSEBUTTONUP:
      switch(event.button.button)
      {
        case SDL_BUTTON_LEFT:
        {
          if (m_current_texture < 0)
            return;

          auto ws = (m_window.get_size().vector() - Vector(32, 32)).size();
          auto& t = m_window.load_texture(m_textures[m_current_texture]);
          auto s = t.get_size();
          Rect trect = Rect(s).move(Vector(ws) / 2 - Vector(s) / 2).move(m_camera);
          if (trect.contains(m_mouse_pos))
          {
            Vector tl = ((m_mouse_pos - trect.top_lft()) / 32.f).floor() * 32.f;

            std::string texture = m_textures[m_current_texture];
            Rect srcrect = Rect(tl, Size(32, 32));

            for (const auto& t : m_tiles)
              if (t.texture_file == texture && (t.srcrect.top_lft() - srcrect.top_lft()).length() < 0.1f)
                return;

            m_tiles.push_back(Tile{texture, srcrect, 0});
            m_tiles_scrollbar.set_total(m_tiles.size() * 32.f);
          }
        }
          break;

        case SDL_BUTTON_RIGHT:
        {
          if (m_dragging)
          {
            m_dragging = false;
          }
          else if (!Rect(0.f, 0.f, m_window.get_size().w - (m_tiles_scrollbar.is_valid() ? 37.f : 32.f), m_window.get_size().h - 32.f).contains(m_mouse_pos))
          {
            int tilenum = static_cast<int>(m_mouse_pos.y + m_tiles_scrollbar.get_progress()) / 32;
            if (m_tiles.size() > tilenum)
            {
              m_tiles.erase(m_tiles.begin() + tilenum);
              m_tiles_scrollbar.set_total(m_tiles.size() * 32.f);
            }
          }
        }
          break;

        default:
          break;
      }
      break;

    case SDL_WINDOWEVENT_RESIZED:
      resize_elements();
      break;

    case SDL_MOUSEWHEEL:
      if (m_mouse_pos.x >= m_window.get_size().w - 37.f)
        m_tiles_scrollbar.set_progress(m_tiles_scrollbar.get_progress() - event.wheel.y * 8.f);
      break;

    default:
      break;
  }
}

void
TileSelector::update(float dt_sec)
{
}

void
TileSelector::draw() const
{
  Renderer& r = m_window.get_renderer();

  auto ctrls = m_window.create_texture(m_window.get_size());
  DrawingContext dc(r);
  if (m_tiles_scrollbar.is_valid())
    m_tiles_scrollbar.draw(dc);
  m_btn_next_step.draw(dc);
  m_btn_add_image.draw(dc);
  dc.render(ctrls.get());

  r.start_draw();

  auto ws = (m_window.get_size().vector() - Vector(32, 32)).size();
  r.draw_filled_rect(ws, Color(.15f, .15f, .15f), Renderer::Blend::NONE);

  if (m_current_texture > -1)
  {
    auto& t = m_window.load_texture(m_textures[m_current_texture]);
    auto s = t.get_size();

    Rect trect = Rect(s).move(Vector(ws) / 2 - Vector(s) / 2).move(m_camera);
    r.draw_filled_rect(trect, Color(0.f, 0.f, 0.f), Renderer::Blend::NONE);
    r.draw_texture(t, s, trect, 0.f, Color(1.f, 1.f, 1.f), Renderer::Blend::BLEND);
    if (trect.clipped(Rect(0.f, 0.f, ws.w - (m_tiles_scrollbar.is_valid() ? 37.f : 32.f), ws.h - 32.f)).contains(m_mouse_pos))
    {
      Vector tl = ((m_mouse_pos - trect.top_lft()) / 32.f).floor() * 32.f + trect.top_lft();
      r.draw_filled_rect(Rect(tl, Size(32.f, 32.f)), Color(1.f, 1.f, 1.f, .25f), Renderer::Blend::BLEND);
    }
  }

  r.draw_filled_rect(Rect(0.f, ws.h, ws.w, ws.h + 32.f), Color(.2f, .2f, .2f), Renderer::Blend::NONE);
  r.draw_filled_rect(Rect(ws.w, 0.f, ws.w + 32.f, ws.h + 32.f), Color(.2f, .2f, .2f), Renderer::Blend::NONE);

  Rect tile_rect(ws.w, 0.f - m_tiles_scrollbar.get_progress(), ws.w + 32.f, 32.f - m_tiles_scrollbar.get_progress());
  for (const auto& t : m_tiles)
  {
    r.draw_texture(m_window.load_texture(t.texture_file), t.srcrect, tile_rect, 0.f, Color(1.f, 1.f, 1.f), Renderer::Blend::BLEND);
    tile_rect.move(Vector(0, 32));
  }

  r.draw_texture(*ctrls, m_window.get_size(), m_window.get_size(), 0.f, Color(1.f, 1.f, 1.f), Renderer::Blend::BLEND);

  r.end_draw();
}

void
TileSelector::add_image()
{
  auto files = pfd::open_file("Select SuperTux version", m_last_folder, { "PNG", "*.png" }, pfd::opt::none).result();
  if (files.size() != 1)
    return;

  m_last_folder = files[0];

  auto& t = m_window.load_texture(files[0]);
  Size s = t.get_size();
  int width = static_cast<int>(s.w), height = static_cast<int>(s.h);

  if (width % 32 != 0 || height % 32 != 0)
  {
    SDL_ShowSimpleMessageBox(SDL_MessageBoxFlags::SDL_MESSAGEBOX_ERROR, "Error",
                    ("Image files must be based on 32x32 tiles. Image size: " +
                    std::to_string(width) + "x" + std::to_string(height)).c_str(),
                    nullptr);
    return;
  }

  m_textures.push_back(files[0]);
  m_current_texture = m_textures.size() - 1;
  m_camera = Vector();
}

void
TileSelector::resize_elements()
{
  m_tiles_scrollbar.get_rect() = Rect(m_window.get_size().w - 37.f, 0.f, m_window.get_size().w - 32.f, m_window.get_size().h);
  m_btn_add_image.get_rect() = Rect(0.f, m_window.get_size().h - 32.f, m_window.get_size().w / 2.f - 16.f, m_window.get_size().h);
  m_btn_next_step.get_rect() = Rect(m_window.get_size().w / 2.f - 16.f, m_window.get_size().h - 32.f, m_window.get_size().w - 32.f, m_window.get_size().h);
}
