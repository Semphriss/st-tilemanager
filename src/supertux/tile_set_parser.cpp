//  SuperTux
//  Copyright (C) 2008 Matthias Braun <matze@braunis.de>
//                     Ingo Ruhnke <grumbel@gmail.com>
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

#include "supertux/tile_set_parser.hpp"

#include <sstream>
#include <sexp/value.hpp>
#include <sexp/io.hpp>

#include "util/log.hpp"
#include "util/vector.hpp"
#include "video/texture.hpp"
#include "video/window.hpp"

#include "supertux/util/reader_document.hpp"
#include "supertux/util/reader_mapping.hpp"
#include "supertux/util/file_system.hpp"

TileSetParser::TileSetParser(std::vector<TileGroup>& tilegroups, const std::string& filename, Window& window) :
  m_window(window),
  m_tilegroups(tilegroups),
  m_filename(filename),
  m_tiles_path()
{
}

void
TileSetParser::parse()
{
  m_tiles_path = FileSystem::dirname(m_filename);

  auto doc = ReaderDocument::from_file(m_filename);
  auto root = doc.get_root();

  if (root.get_name() != "supertux-tiles")
    throw std::runtime_error("file is not a supertux tiles file.");

  auto iter = root.get_mapping().get_iter();
  while (iter.next())
  {  
    if (iter.get_key() == "tiles")
    {
      ReaderMapping tiles_mapping = iter.as_mapping();
      parse_tiles(tiles_mapping);
    }
  }
}

void
TileSetParser::parse_tiles(const ReaderMapping& reader)
{
  // List of ids (use 0 if the tile should be ignored)
  std::vector<uint32_t> ids;
  // List of tile objects
  std::vector<Tile> tiles;

  // width and height of the image in tile units, this is used for two
  // purposes:
  //  a) so we don't have to load the image here to know its dimensions
  //  b) so that the resulting 'tiles' entry is more robust,
  //  ie. enlarging the image won't break the tile id mapping
  // FIXME: height is actually not used, since width might be enough for
  // all purposes, still feels somewhat more natural this way
  unsigned int width  = 0;
  unsigned int height = 0;

  bool has_ids = reader.get("ids", ids);

  reader.get("width", width);
  reader.get("height", height);

  // Allow specifying additional offset to tiles
  int32_t offset = 0;
  reader.get("offset", offset);

  bool deprecated = false;
  reader.get("deprecated", deprecated);
  if (deprecated)
    return; // Do not create autotiles for deprecated tiles.

  if (ids.empty() || !has_ids)
  {
    throw std::runtime_error("No IDs specified.");
  }
  if (width == 0)
  {
    throw std::runtime_error("Width is zero.");
  }
  else if (height == 0)
  {
    throw std::runtime_error("Height is zero.");
  }
  else if (ids.size() != width*height)
  {
    std::ostringstream err;
    err << "Number of ids (" << ids.size() <<  ") and "
      "dimensions of image (" << width << "x" << height << " = " << width*height << ") "
      "differ";
    throw std::runtime_error(err.str());
  }
  else
  {
    Texture* texture;
    std::string file;
    Rect region;
    std::optional<ReaderMapping> textures_mapping;
    if (reader.get("image", textures_mapping) ||
        reader.get("images", textures_mapping))
      texture = parse_imagespecs(*textures_mapping, file, region);

    if (!texture)
      return;

    // Region should not exceed texture size
    region.x2 = region.x1 + std::min(region.width(), texture->get_size().w - region.x1);
    region.y2 = region.y1 + std::min(region.height(), texture->get_size().h - region.y1);

    // Tilegroup size should allow for maximum possible 32x32 squares in region.
    // Region size should not exceed provided tilegroup size.
    while (region.width() + 32.f < static_cast<float>(width) * 32.f && width > 0)
      width--;
    region.x2 = region.x1 + static_cast<float>(width) * 32.f;

    while (region.height() + 32.f < static_cast<float>(height) * 32.f && height > 0)
      height--;
    region.y2 = region.y1 + static_cast<float>(height) * 32.f;

    // Create tiles from IDs
    for (size_t i = 0; i < ids.size(); ++i)
    {
      ids[i] += offset;

      const Vector pos = Vector(i % width, i / width) * 32.f + region.top_lft();
      tiles.push_back(Tile(ids[i], Rect(pos, Size(32.f, 32.f))));
    }

    m_tilegroups.push_back(TileGroup(FileSystem::basename(file), width, height,
                                     std::move(tiles), texture, region));
  }
}

Texture*
TileSetParser::parse_imagespecs(const ReaderMapping& images_mapping,
                                std::string& file, Rect& region) const
{
  // (images "foo.png" "foo.bar" ...)
  // (images (region "foo.png" 0 0 32 32))
  auto iter = images_mapping.get_iter();
  while (iter.next())
  {
    if (iter.is_string())
    {
      file = iter.as_string_item();
      Texture& texture = m_window.load_texture(FileSystem::join(m_tiles_path, file));
      region = Rect(Vector(0.f, 0.f), texture.get_size());
      return &texture;
    }
    else if (iter.is_pair() && iter.get_key() == "surface")
    {
      log_warn << "Surfaces are not supported." << std::endl;
    }
    else if (iter.is_pair() && iter.get_key() == "region")
    {
      auto const& sx = iter.as_mapping().get_sexp();
      auto const& arr = sx.as_array();
      if (arr.size() != 6)
      {
        log_warn << "(region X Y WIDTH HEIGHT) tag malformed: " << sx << std::endl;
      }
      else
      {
        file = arr[1].as_string();
        const int x = arr[2].as_int();
        const int y = arr[3].as_int();
        const int w = arr[4].as_int();
        const int h = arr[5].as_int();

        region = Rect(Vector(x, y), Size(w, h));
        return &m_window.load_texture(FileSystem::join(m_tiles_path, file));
      }
    }
    else
    {
      log_warn << "Expected string or list in images tag" << std::endl;
    }
  }
  return nullptr;
}

/* EOF */
