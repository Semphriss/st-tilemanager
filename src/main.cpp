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

#include "main.hpp"

#include "SDL.h"
#include "SDL_image.h"
#include "SDL_ttf.h"

#include "util/log.hpp"
#include "video/sdl/sdl_window.hpp"
#include "video/drawing_context.hpp"
#include "video/font.hpp"

#include "tile_selector.hpp"

std::unique_ptr<Scene> g_scene;

void change_scene(std::unique_ptr<Scene> scene)
{
  g_scene = std::move(scene);
}

void run_loops(SDLWindow& w)
{
  while (g_scene)
  {
    SDL_Event e;
    while (g_scene && SDL_PollEvent(&e))
    {
      g_scene->event(e);
    }

    if (g_scene)
      g_scene->update(1.f / 64.f);

    if (g_scene)
      g_scene->draw();

    SDL_Delay(15);
  }
}

int main()
{
  SDL_Init(SDL_INIT_VIDEO);
  IMG_Init(IMG_INIT_PNG);
  TTF_Init();

  try
  {
    SDLWindow w;
    w.set_title("SuperTux Tile Manager");
    w.set_resizable(true);

    g_scene = std::make_unique<TileSelector>(w);

    run_loops(w);
  }
  catch (const std::exception& e)
  {
    log_fatal << "Unexpected exception: " << e.what() << std::endl;
  }
  catch (...)
  {
    log_fatal << "Unexpected error" << std::endl;
  }

  g_scene.reset();

  Font::flush_fonts();

  TTF_Quit();
  IMG_Quit();
  SDL_Quit();

  return 0;
}
