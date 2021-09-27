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

#ifndef _HEADER_STTILEMAN_SCENE_HPP
#define _HEADER_STTILEMAN_SCENE_HPP

class DrawingContext;
class GameManager;
union SDL_Event;
class Window;

class Scene
{
public:
  Scene() = delete;
  Scene(Window& window);
  virtual ~Scene() = default;

  virtual void event(const SDL_Event& event) = 0;
  virtual void update(float dt_sec) = 0;
  virtual void draw() const = 0;

protected:
  Window& m_window;

private:
  Scene(const Scene&) = delete;
  Scene& operator=(const Scene&) = delete;
};

#endif
