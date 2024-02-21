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

#include "main.hpp"
#include "supertux/tile_set_parser.hpp"
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

  Control::ThemeSet ts{t, t, t, t, t};
  ts.disabled.fg_color = Color(.5f, .5f, .55f);
  ts.disabled.bg_color = Color(.2f, .2f, .2f);
  ts.hover.fg_color = Color(.8f, .8f, .85f);
  ts.active.fg_color = Color(.9f, .9f, .95f);

  return ts;
})();

static const Control::ThemeSet list_theme_set = ([]{
  Control::Theme t;
  t.bg_blend = Renderer::Blend::BLEND;
  t.bg_color = Color(.3f, .3f, .3f);
  t.fg_blend = Renderer::Blend::BLEND;
  t.fg_color = Color(1.f, 1.f, 1.f);
  t.font = "../data/fonts/SuperTux-Medium.ttf";
  t.fontsize = 13;

  Control::ThemeSet ts{t, t, t, t, t};
  ts.disabled.bg_color = Color(.5f, .5f, .55f);
  ts.disabled.fg_color = Color(.2f, .2f, .2f);
  ts.hover.bg_color = Color(.7f, .7f, .75f);
  ts.focus.bg_color = Color(.6f, .6f, .65f);
  ts.active.bg_color = Color(.9f, .9f, .95f);

  return ts;
})();

static const Control::ThemeSet list_scrollbar_theme_set = ([]{
  Control::Theme t;
  t.bg_blend = Renderer::Blend::BLEND;
  t.bg_color = Color(.3f, .3f, .3f);
  t.fg_blend = Renderer::Blend::BLEND;
  t.fg_color = Color(.6f, .6f, .6f);

  Control::ThemeSet ts{t, t, t, t, t};
  ts.disabled.fg_color = Color(.5f, .5f, .55f);
  ts.disabled.bg_color = Color(.2f, .2f, .2f);
  ts.hover.fg_color = Color(.8f, .8f, .8f);
  ts.active.fg_color = Color(.9f, .9f, .9f);

  return ts;
})();

TileSelector::TileSelector(Window& window) :
  Scene(window),
  m_mouse_pos(),
  m_current_tile(-1),
  m_tilegroups_list(30.f, list_scrollbar_theme_set, 100, Rect(), list_theme_set, nullptr),
  m_tiles_scrollbar(nullptr, window.get_size().h - 32.f, 0.f, false, 0xff, 100, Rect(),
                    scrollbar_theme_set, nullptr),
  m_btn_add_tileset("Open tileset", [this](int){ add_tileset(); }, 0xff, true, 100, Rect(), theme_set, nullptr),
  m_btn_next_step("Next step", [this](int)
    {
      if (!g_tilegroup || g_selected_tiles.empty())
      {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error", "Some tiles must exist for autotiles to be created", nullptr);
        return;
      }
      change_scene(std::make_unique<TileMaskSelector>(m_window));
    },
    0xff, true, 100, Rect(), theme_set, nullptr),
  m_dragging(false),
  m_camera(0.f, 0.f),
  m_last_folder()
{
  for (TileGroup& tilegroup : g_tilegroups)
    m_tilegroups_list.add_item(tilegroup.filename, &tilegroup);

  m_tilegroups_list.set_on_changed([this](int, TileGroup* const* tilegroup)
    {
      if (!tilegroup) return;

      g_tilegroup = *tilegroup;
      g_selected_tiles.clear();
      m_current_tile = g_tilegroup->tiles.size() - 1;
    });

  resize_elements();
}

void
TileSelector::event(const SDL_Event& event)
{
  if (m_tilegroups_list.event(event) || (m_tiles_scrollbar.is_valid() && m_tiles_scrollbar.event(event))
      || m_btn_add_tileset.event(event) || m_btn_next_step.event(event))
    return;

  switch (event.type)
  {
    case SDL_QUIT:
      change_scene(nullptr);
      break;

    case SDL_MOUSEMOTION:
    {
      m_mouse_pos = Vector(event.motion.x, event.motion.y);
      if (m_dragging)
        m_camera += Vector(event.motion.xrel, event.motion.yrel);

      /** Determine hovered tile */
      if (g_tilegroup)
      {
        auto ws = (m_window.get_size().vector() - Vector(32 - m_window.get_size().w / 4, 32)).size();
        auto& t = *g_tilegroup->texture;
        auto s = g_tilegroup->region.size();
        const Rect trect = Rect(s).move(Vector(ws) / 2 - Vector(s) / 2).move(m_camera);
        if (trect.clipped(Rect(0.f, 0.f, ws.w - (m_tiles_scrollbar.is_valid() ? 37.f : 32.f), ws.h - 32.f)).contains(m_mouse_pos))
        {
          const Vector tile_pos = ((m_mouse_pos - trect.top_lft()) / 32.f).floor();
          m_current_tile = static_cast<int>(tile_pos.y) * static_cast<int>(s.w / 32.f) +
                           static_cast<int>(tile_pos.x);
        }
      }
    }
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
          if (!g_tilegroup || m_current_tile < 0 || !g_tilegroup->tiles[m_current_tile].id)
            return;

          auto ws = (m_window.get_size().vector() - Vector(32 - m_window.get_size().w / 4, 32)).size();
          auto& t = *g_tilegroup->texture;
          auto s = g_tilegroup->region.size();
          const Rect trect = Rect(s).move(Vector(ws) / 2 - Vector(s) / 2).move(m_camera);
          if (trect.contains(m_mouse_pos))
          {
            for (const auto& t : g_selected_tiles)
              if (t.id == g_tilegroup->tiles[m_current_tile].id)
                return;

            g_selected_tiles.push_back(g_tilegroup->tiles[m_current_tile]);
            m_tiles_scrollbar.set_total(g_selected_tiles.size() * 32.f);
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
            if (g_selected_tiles.size() > tilenum)
            {
              g_selected_tiles.erase(g_selected_tiles.begin() + tilenum);
              m_tiles_scrollbar.set_total(g_selected_tiles.size() * 32.f);
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
  m_tilegroups_list.draw(dc);
  if (m_tiles_scrollbar.is_valid())
    m_tiles_scrollbar.draw(dc);
  m_btn_next_step.draw(dc);
  m_btn_add_tileset.draw(dc);
  dc.render(ctrls.get());

  r.start_draw();

  auto ws = (m_window.get_size().vector() - Vector(32, 32)).size();
  r.draw_filled_rect(ws, Color(.15f, .15f, .15f), Renderer::Blend::NONE);

  if (g_tilegroup)
  {
    auto ws_tiles = m_window.get_size().vector() - Vector(32 - m_window.get_size().w / 4, 32);
    auto& t = *g_tilegroup->texture;
    auto s = g_tilegroup->region.size();

    const Rect trect = Rect(s).move(ws_tiles / 2 - Vector(s) / 2).move(m_camera);
    r.draw_filled_rect(trect, Color(0.f, 0.f, 0.f), Renderer::Blend::NONE);

    // Main tiles texture
    r.draw_texture(t, g_tilegroup->region, trect, 0.f, Color(1.f, 1.f, 1.f), Renderer::Blend::BLEND);

    // Tile hover
    if (g_tilegroup->tiles[m_current_tile].id &&
        trect.clipped(Rect(0.f, 0.f, ws.w - (m_tiles_scrollbar.is_valid() ? 37.f : 32.f), ws.h - 32.f)).contains(m_mouse_pos))
    {
      Vector tl = ((m_mouse_pos - trect.top_lft()) / 32.f).floor() * 32.f + trect.top_lft();
      r.draw_filled_rect(Rect(tl, Size(32.f, 32.f)), Color(1.f, 1.f, 1.f, .25f), Renderer::Blend::BLEND);
    }

    // Selected tiles bar
    r.draw_filled_rect(Rect(0.f, ws.h, ws.w, ws.h + 32.f), Color(.2f, .2f, .2f), Renderer::Blend::NONE);
    r.draw_filled_rect(Rect(ws.w, 0.f, ws.w + 32.f, ws.h + 32.f), Color(.2f, .2f, .2f), Renderer::Blend::NONE);

    Rect tile_rect(ws.w, 0.f - m_tiles_scrollbar.get_progress(), ws.w + 32.f, 32.f - m_tiles_scrollbar.get_progress());
    for (const auto& t : g_selected_tiles)
    {
      r.draw_texture(*g_tilegroup->texture, t.srcrect, tile_rect, 0.f, Color(1.f, 1.f, 1.f), Renderer::Blend::BLEND);
      tile_rect.move(Vector(0, 32));
    }
  }

  r.draw_texture(*ctrls, m_window.get_size(), m_window.get_size(), 0.f, Color(1.f, 1.f, 1.f), Renderer::Blend::BLEND);

  r.end_draw();
}

void
TileSelector::add_tileset()
{
  auto files = pfd::open_file("Select SuperTux tileset", m_last_folder, { "SuperTux Tileset", "*.strf" }, pfd::opt::none).result();
  if (files.size() != 1)
    return;


  g_selected_tiles.clear();
  g_tilegroups.clear();
  g_tilegroup = nullptr;
  m_tilegroups_list.clear_items();

  TileSetParser parser(g_tilegroups, m_last_folder, m_window);
  parser.parse();

  if (g_tilegroups.empty())
  {
    SDL_ShowSimpleMessageBox(SDL_MessageBoxFlags::SDL_MESSAGEBOX_INFORMATION, "Information",
                             "No tilegroups imported.", nullptr);
    return;
  }

  for (TileGroup& tilegroup : g_tilegroups)
    m_tilegroups_list.add_item(tilegroup.filename, &tilegroup);

  m_camera = Vector();
}

void
TileSelector::resize_elements()
{
  m_tilegroups_list.get_rect() = Rect(0.f, 0.f, m_window.get_size().w / 4.f, m_window.get_size().h - 32.f);
  m_tilegroups_list.update_scrollbar_rect();
  m_tiles_scrollbar.get_rect() = Rect(m_window.get_size().w - 37.f, 0.f, m_window.get_size().w - 32.f, m_window.get_size().h);
  m_btn_add_tileset.get_rect() = Rect(0.f, m_window.get_size().h - 32.f, m_window.get_size().w / 2.f - 16.f, m_window.get_size().h);
  m_btn_next_step.get_rect() = Rect(m_window.get_size().w / 2.f - 16.f, m_window.get_size().h - 32.f, m_window.get_size().w - 32.f, m_window.get_size().h);
}
