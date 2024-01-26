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

#include "libgit4cpp/Error.h"
#include "libgit4cpp/Remote.h"
#include "libgit4cpp/wrapper_functions.h"
#include "credentials_callback.h"

using gul14::cat;

namespace git {

Remote::Remote(LibGitRemote&& remote_ptr)
    : remote_{ std::move(remote_ptr) }
{
    if (remote_ == nullptr)
        throw Error{ "Remote pointer may not be null" };
}

std::string Remote::get_name() const
{
    const char* name = git_remote_name(remote_.get());
    return name ? name : "";
}

std::string Remote::get_url() const
{
    const char* url = git_remote_url(remote_.get());
    return url ? url : "";
}

std::vector<std::string> Remote::list_references()
{
    if (!git_remote_connected(remote_.get()))
    {
        git_remote_callbacks callbacks = GIT_REMOTE_CALLBACKS_INIT;
        callbacks.credentials = get_dummy_credentials_callback();

        int error = git_remote_connect(remote_.get(), GIT_DIRECTION_FETCH, &callbacks,
            nullptr, nullptr);
        if (error < 0)
        {
            throw Error{ cat("Cannot connect to remote \"", get_name(), "\": ",
                git_error_last()->message) };
        }
    }

    const git_remote_head** out{ nullptr };
    size_t size{ 0 };
    auto error = git_remote_ls(&out, &size, remote_.get());
    if (error)
    {
        throw Error{ cat("Cannot list references on remote \"", get_name(), "\": ",
            git_error_last()->message) };
    }

    std::vector<std::string> refs;
    refs.reserve(size);
    for (size_t i = 0; i != size; ++i)
        refs.emplace_back(out[i]->name);

    return refs;
}

} // namespace git
