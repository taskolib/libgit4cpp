/**
 * \file   wrapper_functions.cc
 * \author Sven-Jannik Wöhnert
 * \date   Created on March 20, 2023
 * \brief  Implementation of wrappers for libgit2 functions.
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

#include <git2.h>

#include "libgit4cpp/wrapper_functions.h"

namespace git {

LibGitPointer<git_repository>
repository_open(const std::string& repo_path)
{
    git_repository *repo;
    if (git_repository_open(&repo, repo_path.c_str()))
        repo=nullptr;

    return LibGitPointer(repo);
}

LibGitPointer<git_repository>
repository_init(const std::string& repo_path, bool is_bare)
{
    git_repository *repo;
    if (git_repository_init(&repo, repo_path.c_str(), is_bare))
        repo = nullptr;
    return LibGitPointer(repo);
}

LibGitPointer<git_index>
repository_index(git_repository* repo)
{
    git_index *index;
    if (git_repository_index(&index, repo))
        index = nullptr;
    return LibGitPointer(index);
}

LibGitPointer<git_signature>
signature_default(git_repository* repo)
{
    git_signature *signature;
    if (git_signature_default(&signature, repo))
        signature = nullptr;
    return LibGitPointer(signature);
}

LibGitPointer<git_signature>
signature_new(const std::string& name, const std::string& email, time_t time, int offset)
{
    git_signature *signature;
    if (git_signature_new(&signature, name.c_str(), email.c_str(), time, offset))
        signature = nullptr;
    return LibGitPointer(signature);
}

LibGitPointer<git_tree>
tree_lookup(git_repository* repo, git_oid tree_id)
{
    git_tree *tree;
    if (git_tree_lookup(&tree, repo, &tree_id))
        tree = nullptr;
    return LibGitPointer(tree);
}

LibGitPointer<git_remote>
remote_create(git_repository* repo, const std::string& remote_name,
              const std::string& url)
{
    git_remote *remote;
    if (git_remote_create(&remote, repo, remote_name.c_str(), url.c_str()))
        remote = nullptr;
    return LibGitPointer(remote);
}

LibGitPointer<git_remote>
remote_lookup(git_repository* repo, const std::string& remote_name)
{
    git_remote *remote;
    if (git_remote_lookup(&remote, repo, remote_name.c_str()))
        remote = nullptr;
    return LibGitPointer(remote);
}

LibGitPointer<git_status_list>
status_list_new(git_repository* repo, const git_status_options& status_opt)
{
    git_status_list *status;
    if (git_status_list_new(&status, repo, &status_opt))
        status = nullptr;
    return LibGitPointer(status);
}

LibGitPointer<git_reference>
repository_head(git_repository* repo)
{
    git_reference *reference;
    if (git_repository_head(&reference, repo))
        reference = nullptr;
    return LibGitPointer(reference);
}

} // namespace git
