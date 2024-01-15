/**
 * \file   Remote.cc
 * \author Lars Fröhlich
 * \date   Created on January 15, 2024
 * \brief  Implementation of the Remote class.
 *
 * \copyright Copyright 2024 Deutsches Elektronen-Synchrotron (DESY), Hamburg
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published
 * by the Free Software Foundation, either version 2.1 of the license, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

// SPDX-License-Identifier: LGPL-2.1-or-later

#include <git2.h>
#include <gul14/cat.h>
#include <gul14/string_util.h>

#include "libgit4cpp/Error.h"
#include "libgit4cpp/Remote.h"
#include "libgit4cpp/wrapper_functions.h"

using gul14::cat;

namespace git {

Remote::Remote(LibGitRemote&& remote_ptr)
    : remote_{ std::move(remote_ptr) }
{
    if (remote_ == nullptr)
        throw Error{ "Remote pointer may not be null" };

    name_ = gul14::safe_string(git_remote_name(remote_.get()), 512);
    url_ = gul14::safe_string(git_remote_url(remote_.get()), 512);
}

} // namespace git
