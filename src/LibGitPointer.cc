/**
 * \file   LibGitPointer.cc
 * \author Sven-Jannik Wöhnert
 * \date   Created on March 20, 2023
 * \brief  Implementation of functions associated with the LibGitPointer class.
 *
 * \copyright Copyright 2023 Deutsches Elektronen-Synchrotron (DESY), Hamburg
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

#include "libgit4cpp/LibGitPointer.h"

namespace git {
namespace detail {

void free_libgit_ptr(git_tree* tree)
{
    git_tree_free(tree);
}

void free_libgit_ptr(git_signature* signature)
{
    git_signature_free(signature);
}

void free_libgit_ptr(git_index* index)
{
    git_index_free(index);
}

void free_libgit_ptr(git_repository* repo)
{
    git_repository_free(repo);
}

void free_libgit_ptr(git_remote* remote)
{
    git_remote_free(remote);
}

void free_libgit_ptr(git_commit* commit)
{
    git_commit_free(commit);
}

void free_libgit_ptr(git_status_list* status)
{
    git_status_list_free(status);
}

void free_libgit_ptr(git_reference* reference)
{
    git_reference_free(reference);
}

void free_libgit_ptr(git_buf* buf)
{
    git_buf_dispose(buf);
}

} // namespace detail
} // namespace git
