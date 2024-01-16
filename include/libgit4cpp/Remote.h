/**
 * \file   Remote.h
 * \author Lars Fröhlich
 * \date   Created on January 15, 2024
 * \brief  Declaration of the Remote class.
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

#ifndef LIBGIT4CPP_REMOTE_H_
#define LIBGIT4CPP_REMOTE_H_

#include <string>

#include <git2.h>
#include <gul14/optional.h>
#include <gul14/SmallVector.h>

#include "libgit4cpp/types.h"

namespace git {

/**
 * An abstraction of a git remote (such as "origin").
 *
 * A remote has a name and a URL which can be accessed via get_name() and get_url(),
 * respectively.
 */
class Remote
{
public:
    /**
     * Construct a Remote object by taking the ownership of a git_remote unique pointer.
     * The constructor queries the name and URL of the remote and caches them for later
     * access via get_name() and get_url().
     * \exception Error is thrown if the given pointer is null.
     */
    Remote(LibGitRemote&& remote_ptr);

    /// Return a non-owning pointer to the underlying git remote object.
    git_remote* get() const { return remote_.get(); }

    /// Return the name of the remote (e.g. "origin"). May be empty for in-memory remotes.
    const std::string& get_name() const { return name_; }

    /// Return the URL of the remote (e.g. "https://gitlab.com/a/b.git").
    const std::string& get_url() const { return url_; }

    /**
     * Retrieve a list of references available on this remote repository
     * ("git ls-remote").
     *
     * \note
     * The API for this function is preliminary. It would be more useful to return not
     * just the names of the references, but also additional details like the commit IDs.
     */
    std::vector<std::string> list_references();

private:
    std::string name_;
    std::string url_;
    LibGitRemote remote_{ nullptr, git_remote_free };
};

} // namespace git

#endif
