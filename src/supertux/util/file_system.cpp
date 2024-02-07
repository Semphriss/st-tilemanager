//  SuperTux
//  Copyright (C) 2006 Matthias Braun <matze@braunis.de>
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

#include "supertux/util/file_system.hpp"

namespace FileSystem {

std::string dirname(const std::string& filename)
{
  std::string::size_type p = filename.find_last_of('/');
  if (p == std::string::npos)
    p = filename.find_last_of('\\');
  if (p == std::string::npos)
    return "./";

  return filename.substr(0, p+1);
}

std::string join(const std::string& lhs, const std::string& rhs)
{
  if (lhs.empty())
  {
    return rhs;
  }
  else if (rhs.empty())
  {
    return lhs + "/";
  }
  else if (lhs.back() == '/' && rhs.front() != '/')
  {
    return lhs + rhs;
  }
  else if (lhs.back() != '/' && rhs.front() == '/')
  {
    return lhs + rhs;
  }
  else if (lhs.back() == '/' && rhs.front() == '/')
  {
    return lhs + rhs.substr(1);
  }
  else
  {
    return lhs + "/" + rhs;
  }
}

} // namespace FileSystem

/* EOF */
