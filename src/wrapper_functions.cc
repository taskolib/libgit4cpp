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
#include <gul14/cat.h>
#include <gul14/finalizer.h>

#include "libgit4cpp/wrapper_functions.h"
#include "libgit4cpp/Error.h"

namespace git {

LibGitRepository repository_open(const std::string& repo_path)
{
    git_repository* repo;
    if (git_repository_open(&repo, repo_path.c_str()))
    {
        // gul14::cat("repository_open: ", git_error_last()->message);
        repo = nullptr;
    }

    return { repo, git_repository_free };
}

LibGitRepository repository_init(const std::string& repo_path, bool is_bare)
{
    git_repository_init_options opts;
    git_repository_init_init_options(&opts, GIT_REPOSITORY_INIT_OPTIONS_VERSION);
    if (is_bare)
        opts.flags |= GIT_REPOSITORY_INIT_BARE;
    opts.flags |= GIT_REPOSITORY_INIT_MKPATH;
    opts.initial_head = "main";
    git_repository* repo;
    int error = git_repository_init_ext(&repo, repo_path.c_str(), &opts);
    if (error)
    {
        // gul14::cat("repository_init: ", git_error_last()->message);
        repo = nullptr;
    }
    return { repo, git_repository_free };
}

LibGitIndex repository_index(git_repository* repo)
{
    git_index* index;
    if (git_repository_index(&index, repo))
    {
        // gul14::cat("repository_index: ", git_error_last()->message);
        index = nullptr;
    }
    return { index, git_index_free };
}

LibGitSignature signature_default(git_repository* repo)
{
    git_signature* signature;
    if (git_signature_default(&signature, repo))
    {
        // gul14::cat("signature_default: ", git_error_last()->message);
        signature = nullptr;
    }
    return { signature, git_signature_free };
}

LibGitSignature signature_new(const std::string& name, const std::string& email, time_t time, int offset)
{
    git_signature* signature;
    if (git_signature_new(&signature, name.c_str(), email.c_str(), time, offset))
    {
        // gul14::cat("signature_new: ", git_error_last()->message);
        signature = nullptr;
    }
    return { signature, git_signature_free };
}

LibGitTree tree_lookup(git_repository* repo, git_oid tree_id)
{
    git_tree* tree;
    if (git_tree_lookup(&tree, repo, &tree_id))
    {
        // gul14::cat("tree_lookup: ", git_error_last()->message);
        tree = nullptr;
    }
    return { tree, git_tree_free };
}

LibGitRemote remote_create(git_repository* repo, const std::string& remote_name,
              const std::string& url)
{
    git_remote* remote;
    if (git_remote_create(&remote, repo, remote_name.c_str(), url.c_str()))
    {
        // gul14::cat("remote_create: ", git_error_last()->message);
        remote = nullptr;
    }
    return { remote, git_remote_free };
}

LibGitRemote remote_lookup(git_repository* repo, const std::string& remote_name)
{
    git_remote* remote = nullptr;
    if (repo)
        git_remote_lookup(&remote, repo, remote_name.c_str());
    return { remote, git_remote_free };
}

LibGitStatusList status_list_new(git_repository* repo, const git_status_options& status_opt)
{
    git_status_list* status;
    if (git_status_list_new(&status, repo, &status_opt))
    {
        // gul14::cat("status_list_new: ", git_error_last()->message);
        status = nullptr;
    }
    return { status, git_status_list_free };
}

LibGitReference repository_head(git_repository* repo)
{
    git_reference* reference;
    if (git_repository_head(&reference, repo))
    {
        // gul14::cat("reposiotry_head: ", git_error_last()->message);
        reference = nullptr;
    }
    return { reference, git_reference_free };
}

LibGitRepository clone(const std::string& url, const std::string& repo_path)
{
    git_repository* repo;
    if (git_clone(&repo, url.c_str(), repo_path.c_str(), nullptr))
    {
        // gul14::cat("branch_remote_name: ", git_error_last()->message);
        repo = nullptr;
    }
    return { repo, git_repository_free };

}

LibGitReference branch_lookup(git_repository* repo, const std::string& branch_name, git_branch_t branch_type)
{
    git_reference* ref;
    if (git_branch_lookup(&ref, repo, branch_name.c_str(), branch_type))
    {
        // gul14::cat("branch_lookup: ", git_error_last()->message);
        ref = nullptr;
    }
    return { ref, git_reference_free };
}

LibGitTree commit_tree(git_commit* commit)
{
    git_tree* tree;
    if (git_commit_tree(&tree, commit))
    {
        tree = nullptr;
    }
    return { tree, git_tree_free };
}

LibGitReference branch_create(git_repository* repo, std::string& new_branch_name, const git_commit* starting_commit, int force)
{
    git_reference* ref;
    if(git_branch_create(&ref, repo, new_branch_name.c_str(), starting_commit, force))
        ref = nullptr;
    return {ref, git_reference_free};
}

std::string branch_remote_name(git_repository* repo, const std::string& branch_name)
{
    git_buf buf{ };
    auto _ = gul14::finally([buf_addr = &buf]() { git_buf_dispose(buf_addr); });
    auto error = git_branch_remote_name(&buf, repo, branch_name.c_str());
    if (error) {
        throw Error{ error, gul14::cat("branch_remote_name: ", git_error_last()->message) };
    }
    auto ret = std::string{ buf.ptr };
    return ret;
}

std::string reference_shorthand(const git_reference* ref)
{
    const char* name_cstr = git_reference_shorthand(ref);
    if (name_cstr == nullptr)
        throw Error{gul14::cat("reference_shorthand: ", git_error_last()->message) };
    return std::string(name_cstr);
}

} // namespace git
