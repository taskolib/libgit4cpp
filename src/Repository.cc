/**
 * \file   Repository.cc
 * \author Sven-Jannik Wöhnert
 * \date   Created on March 20, 2023
 * \brief  Implementation of the Repository class.
 *
 * \copyright Copyright 2023-2025 Deutsches Elektronen-Synchrotron (DESY), Hamburg
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

#include <cstring>
#include <iostream>
#include <vector>

#include <git2.h>
#include <gul17/cat.h>
#include <gul17/finalizer.h>

#include "libgit4cpp/Error.h"
#include "libgit4cpp/Repository.h"
#include "libgit4cpp/wrapper_functions.h"
#include "credentials_callback.h"

using gul17::cat;

namespace git {

Repository::Repository(const std::filesystem::path& file_path)
    : repo_path_{ file_path }
{
    git_libgit2_init();
    init(file_path);
}

Repository::~Repository()
{
    repo_.reset();
    my_signature_.reset();
    git_libgit2_shutdown();
}

void Repository::make_signature()
{
    my_signature_ = signature_default(repo_.get());
    if (not my_signature_)
        my_signature_ = signature_new("Taskomat", "(none)", std::time(0), 0);
}

void Repository::reset_repo()
{
    repo_.reset();
    my_signature_.reset();

    // initialize repo and signature again
    init(repo_path_);
}

std::string Repository::get_last_commit_message()
{
    const auto commit = get_commit();
    return git_commit_message(commit.get());
}

std::filesystem::path Repository::get_path() const
{
    return repo_path_;
}

git_repository* Repository::get_repo()
{
    return repo_.get();
}

void Repository::update(const std::string& glob)
{
    auto index = repository_index(repo_.get());

    char *paths[1] = {const_cast<char*>(glob.c_str())};
    git_strarray array = { paths, 1 };

    // update index to check for files
    git_index_update_all(index.get(), &array, nullptr, nullptr);
    git_index_write(index.get());
}

void Repository::init(const std::filesystem::path& file_path)
{
    repo_ = repository_open(repo_path_);

    if (not repo_)
    {
        // create repository
        // 2nd argument: false so that .git folder is created in given path
        repo_ = repository_init(file_path, false);
        if (not repo_)
            throw Error{ "Git init failed" };

        make_signature();
        update();
        commit_initial();
    }
    else
    {
        make_signature();
    }
}

void Repository::commit_initial()
{
    // prepare gitlib data types
    auto index = repository_index(repo_.get());
    git_oid tree_id, commit_id;

    git_index_write_tree(&tree_id, index.get());
    auto tree = tree_lookup(repo_.get(), tree_id);

    int error = git_commit_create(
        &commit_id,
        repo_.get(),
        "HEAD",
        my_signature_.get(),
        my_signature_.get(),
        "UTF-8",
        "Initial commit",
        tree.get(),
        0,
        nullptr
    );

    if (error)
        throw Error{ cat("Initial commit failed: ", git_error_last()->message) };
}

void Repository::commit(const std::string& commit_message)
{
    auto parent_commit = get_commit();
    const git_commit* raw_commit = parent_commit.get();

    //define types for commit call and get index
    auto index = repository_index(repo_.get());
    git_oid tree_id, commit_id;

    git_index_write_tree(&tree_id, index.get());
    auto tree = tree_lookup(repo_.get(), tree_id);

    int error = git_commit_create(
        &commit_id,
        repo_.get(),
        "HEAD",
        my_signature_.get(),
        my_signature_.get(),
        "UTF-8",
        commit_message.c_str(),
        tree.get(),
        1,
        &raw_commit
    );

    if (error)
        throw Error{ cat("Commit: ", git_error_last()->message) };
}

void Repository::add(const std::string& glob)
{
    auto gindex = repository_index(repo_.get());

    char *paths[] = { const_cast<char*>(glob.c_str()) };
    git_strarray array = { paths, 1 };

    int error = git_index_add_all(gindex.get(), &array, GIT_INDEX_ADD_DEFAULT, nullptr,
        nullptr);
    if (error)
        throw Error{ cat("Cannot stage files: ", git_error_last()->message) };

    git_index_write(gindex.get());
}

void Repository::remove_directory(const std::filesystem::path& directory)
{
    auto gindex = repository_index(repo_.get());

    int error = git_index_remove_directory(gindex.get(), directory.c_str(), 0);
    if (error)
        throw Error{ cat("Cannot remove directory: ", git_error_last()->message) };

    git_index_write(gindex.get());
}

void Repository::remove_files(const std::vector<std::filesystem::path>& filepaths)
{
    auto gindex = repository_index(repo_.get());

    //remove files from directory
    //TODO: Teste, ob oberer Teil ausreicht
    for (auto gfile: filepaths)
    {
        int error = git_index_remove_bypath(gindex.get(), gfile.c_str());
        if (error)
            throw Error{ cat("Cannot remove file: ", git_error_last()->message) };
    }

    git_index_write(gindex.get());
}

LibGitCommit Repository::get_commit(unsigned int count)
{
    git_commit* parent;
    auto err = git_commit_nth_gen_ancestor(&parent, get_commit("HEAD").get(), count);
    if (err)
        throw Error{ cat("Cannot find ", count, "th ancestor: ", git_error_last()->message) };
    return { parent, git_commit_free };
}

LibGitCommit Repository::get_commit(const std::string& ref)
{
    git_commit* commit;
    git_oid oid_parent_commit;

    // resolve HEAD into a SHA1
    int error = git_reference_name_to_id(&oid_parent_commit, repo_.get(), ref.c_str());
    if (error)
        throw Error{ cat("Cannot find ID from reference name: ", git_error_last()->message) };

    // find commit object by commit ID
    error = git_commit_lookup(&commit, repo_.get(), &oid_parent_commit);
    if (error)
        throw Error{ "Cannot find HEAD of branch" };

    return { commit, git_commit_free };
}

bool Repository::is_unstaged(FileStatus& filestats, const git_status_entry* s)
{
    std::string wstatus = "";

    if (s->status & GIT_STATUS_WT_MODIFIED)
        wstatus = "modified";
    if (s->status & GIT_STATUS_WT_DELETED)
        wstatus = "deleted";
    if (s->status & GIT_STATUS_WT_RENAMED)
        wstatus = "renamed";
    if (s->status & GIT_STATUS_WT_TYPECHANGE)
        wstatus = "typechange";

    if (wstatus.empty())
        return false;

    filestats.handling = "unstaged";
    filestats.changes = wstatus;

    const char* old_path = s->index_to_workdir->old_file.path;
    const char* new_path = s->index_to_workdir->new_file.path;

    if (old_path && new_path && strcmp(old_path, new_path))
        filestats.path_name = cat(old_path, " -> ", new_path);
    else
        filestats.path_name = old_path ? old_path : new_path;

    return true;
}

bool Repository::is_staged(FileStatus& filestats, const git_status_entry* s)
{
    std::string istatus = "";

    if (s->status & GIT_STATUS_INDEX_NEW)
        istatus = "new file";
    if (s->status & GIT_STATUS_INDEX_MODIFIED)
        istatus = "modified";
    if (s->status & GIT_STATUS_INDEX_DELETED)
        istatus = "deleted";
    if (s->status & GIT_STATUS_INDEX_RENAMED)
        istatus = "renamed";
    if (s->status & GIT_STATUS_INDEX_TYPECHANGE)
        istatus = "typechange";

    if (istatus.empty())
        return false;

    filestats.handling = "staged";
    filestats.changes = std::string{istatus};

    const char* old_path = s->head_to_index->old_file.path;
    const char* new_path = s->head_to_index->new_file.path;

    if (old_path && new_path && strcmp(old_path, new_path))
        filestats.path_name = cat(old_path, " -> ", new_path);
    else
        filestats.path_name = old_path ? old_path : new_path;

    return true;
}

RepoState Repository::collect_status(LibGitStatusList& status) const
{
    // get number of submodules
    const size_t nr_entries = git_status_list_entrycount(status.get());

    // declare status holding vector for each submodule
    RepoState return_array{ };
    FileStatus filestats{ };

    for (size_t i = 0; i < nr_entries; ++i)
    {
        const git_status_entry* s = git_status_byindex(status.get(), i);
        const char* old_path = nullptr;
        const char* new_path = nullptr;

        std::string istatus = "";
        std::string wstatus = "";

        // list files which exists but are untouched since last commit
        //############################################################
        if (s->status == GIT_STATUS_CURRENT)
        {
            filestats.handling = "unchanged";
            filestats.changes = "unchanged";

            old_path = s->head_to_index->old_file.path;
            new_path = s->head_to_index->new_file.path;

            filestats.path_name = old_path ? old_path : new_path;

            return_array.push_back(filestats);

            continue;
        }

        // list files which were touched but stil are unchanged
        //#####################################################

        if (is_unstaged(filestats, s))
        {
            return_array.push_back(filestats);
            continue;
        }

        // list files which are staged for next commit
        //############################################


        if (is_staged(filestats, s))
        {
            return_array.push_back(filestats);
            continue;
        }

        // list untracked files
        //######################
        if (s->status == GIT_STATUS_WT_NEW)
        {

            filestats.handling = "untracked";
            filestats.changes = "untracked";
            filestats.path_name = s->index_to_workdir->old_file.path;

            return_array.push_back(filestats);

            continue;
        }

        // list ignored files
        //####################
        if (s->status == GIT_STATUS_IGNORED) {

            filestats.handling = "ignored";
            filestats.changes = "ignored";
            filestats.path_name = s->index_to_workdir->old_file.path;

            return_array.push_back(filestats);

            continue;
        }
    }
    return return_array;
}

RepoState Repository::status()
{
    git_status_options status_opt = GIT_STATUS_OPTIONS_INIT;
    status_opt.flags =  GIT_STATUS_OPT_INCLUDE_UNTRACKED |          // untracked files
                        GIT_STATUS_OPT_RECURSE_UNTRACKED_DIRS |     // untracked directories
                        GIT_STATUS_OPT_INCLUDE_UNMODIFIED |         // unmodified files
                        GIT_STATUS_OPT_INCLUDE_IGNORED;             // ignored files

    auto my_status = status_list_new(repo_.get(), status_opt);
    if (not my_status)
        throw Error{ "Cannot initialize status" };

    return collect_status(my_status);
}

std::vector<int> Repository::add_files(const std::vector<std::filesystem::path>& filepaths)
{
    auto gindex = repository_index(repo_.get());

    size_t v_len = filepaths.size();

    std::vector<int> error_list;
    for (size_t i = 0; i < v_len; i++)
    {
        int error = git_index_add_bypath(gindex.get(), filepaths[i].c_str());
        if (error)
            error_list.push_back(i);
    }

    git_index_write(gindex.get());

    return error_list;
}

void Repository::reset(unsigned int nr_of_commits)
{
    auto parent_commit = get_commit(nr_of_commits);
    auto error = git_reset(repo_.get(), reinterpret_cast<git_object*>(parent_commit.get()), GIT_RESET_HARD, nullptr);
    if (error)
        throw Error{ cat("Reset: ", git_error_last()->message) };
}

Remote Repository::add_remote(const std::string& remote_name, const std::string& url)
{
    auto remote = remote_create(repo_.get(), remote_name, url);
    if (!remote)
    {
        throw Error{ cat("Cannot create remote \"", remote_name, "\": ",
            git_error_last()->message) };
    }
    return Remote{ std::move(remote) };
}

std::optional<Remote> Repository::get_remote(const std::string& remote_name) const
{
    auto remote = remote_lookup(repo_.get(), remote_name);
    if (!remote)
        return {};

    return Remote{ std::move(remote) };
}

std::vector<Remote> Repository::list_remotes() const
{
    auto remote_names = list_remote_names();

    std::vector<Remote> result;
    result.reserve(remote_names.size());

    for (const std::string& name : remote_names)
    {
        auto maybe_remote = get_remote(name);
        if (!maybe_remote)
        {
            throw Error(cat("Lookup failed for remote \"", name, "\": ",
                git_error_last()->message));
        }
        result.push_back(std::move(*maybe_remote));
    }

    return result;
}

std::vector<std::string> Repository::list_remote_names() const
{
    git_strarray remotes;
    int err = git_remote_list(&remotes, repo_.get());
    if (err)
        throw Error{ cat("Cannot list remotes: ", git_error_last()->message) };
    auto cleanup = gul17::finally([&remotes]() { git_strarray_free(&remotes); });

    std::vector<std::string> list;
    list.reserve(remotes.count);
    for (std::size_t i = 0; i != remotes.count; ++i)
    {
        const char* name = remotes.strings[i];
        list.emplace_back(name ? name : "");
    }

    return list;
}

void Repository::push(const Remote& remote, const std::string& refspec)
{
    git_remote_callbacks callbacks;
    int error = git_remote_init_callbacks(&callbacks, GIT_REMOTE_CALLBACKS_VERSION);
    if (error)
    {
        throw Error{ cat("Cannot initialize remote callbacks for push: ",
            git_error_last()->message) };
    }
    callbacks.credentials = get_dummy_credentials_callback();

    git_push_options push_options;
    error = git_push_init_options(&push_options, GIT_PUSH_OPTIONS_VERSION);
    if (error)
        throw Error{ cat("Init push: ", git_error_last()->message) };
    push_options.callbacks = callbacks;

    char* refspec_ptr = const_cast<char*>(refspec.c_str());
    const git_strarray refspec_array = {
        &refspec_ptr,
        1
    };

    error = git_remote_push(remote.get(), &refspec_array, &push_options);
    if (error)
        throw Error{ cat("Push remote: ", git_error_last()->message) };
}

#if 0
void Repository::pull()
{
    // define fetch options
    git_fetch_options options = GIT_FETCH_OPTIONS_INIT;

    // set remote
    /*
    auto remote = remote_lookup(repo_.get(), "origin");
    if (not remote)
        throw Error{ "Cannot find remote object" };
    */

    // fetch commits from remote connection
    int error = git_remote_fetch(remote_.get(), NULL, &options, NULL);
    if (error)
        throw Error{ cat("Pull: ", git_error_last()->message) };

    // git_apply, if failed git_merge
    // TODO

    //cleanup repository from remote connection and merge/fetch pointers
    git_repository_state_cleanup(repo_.get());


}

void Repository::clone_repo(const std::string& url, const std::filesystem::path& repo_path)
{
    auto repo = clone(url, repo_path);
    if (not repo) {
        throw Error{ "Cannot clone repository" };
    }
    else
    {
        url_ = url;
        remote_ = remote_lookup(repo.get(), "origin"); // TODO: origin korrekt?
        repo_path_ = repo_path;
        repo_ = std::move(repo);

    }
}

bool Repository::branch_up_to_date(const std::string& branch_name)
{
    auto local_ref = branch_lookup(repo_.get(), "master", GIT_BRANCH_LOCAL);
    if (not local_ref)
        throw Error{ cat("Branch lookup: ", git_error_last()->message) };

    // Get the name of the remote associated with the local branch
    //TODO: wrapper for buffer?
    auto remote_name = branch_remote_name(repo_.get(), branch_name.c_str());
    if (remote_name == "")
        throw Error{ "Failed to get remote name for the local branch" };

    // Open the remote
    auto remote = remote_lookup(repo_.get(), remote_name);
    if (not remote)
        throw Error{ "Cannot find remote object" };

    // Get the upstream branch
    git_reference* upstream_ref = nullptr;
    if (git_branch_upstream(&upstream_ref, local_ref.get()) != 0) {
        std::cerr << "Failed to get upstream branch." << std::endl;
        return 1;
    }

    // Get the OIDs of the local and remote branches
    const git_oid* local_oid = git_reference_target(local_ref.get());
    const git_oid* remote_oid = git_reference_target(upstream_ref);

    // Compare OIDs to check if the branches are up to date
    return git_oid_equal(local_oid, remote_oid);
}
#endif


LibGitReference Repository::new_branch(const std::string& branch_name)
{
    return new_branch(branch_name, get_current_branch_name());
}

LibGitReference Repository::new_branch(const std::string& branch_name,
    const std::string& origin_branch_name)
{
    // checkout origin branch
    auto ref = branch_lookup(repo_.get(), origin_branch_name, GIT_BRANCH_LOCAL);

    // get latest commit
    auto commit = get_commit(reference_name(ref.get()));

    // create new branch
    return branch_create(repo_.get(), branch_name, commit.get(), 0);
}

std::string Repository::get_current_branch_name()
{
    // get current HEAD
    auto head = repository_head(repo_.get());

    // get branch name from HEAD
    return reference_shorthand(head.get());
}

std::vector<std::string> Repository::list_branches(BranchType type_flag)
{
    // transform libgit4cpp enum into libgit2 object
    git_branch_t flag;
    if (type_flag == BranchType::all)
        flag = GIT_BRANCH_ALL;
    else if (type_flag == BranchType::local)
        flag = GIT_BRANCH_LOCAL;
    else if (type_flag == BranchType::remote)
        flag = GIT_BRANCH_REMOTE;
    else
        throw Error{"list_branches: unknown type_flag"};

    std::vector<std::string> ret;

    LibGitBranchIterator iter = branch_iterator(repo_.get(), flag);

    LibGitReference ref = branch_next(&flag, iter.get());
    while(ref != nullptr)
    {
        ret.push_back(reference_name(ref.get()));
        ref = branch_next(&flag, iter.get());
    }
    return ret;
}

void Repository::checkout(const std::string& branch_name,
    const std::vector<std::string>& paths)
{

    git_checkout_options checkout_opts = GIT_CHECKOUT_OPTIONS_INIT;
    // define the paths of files to checkout
    checkout_opts.paths.count = paths.size();

    // transform std::string input into readaable data for libgit2
    std::vector<const char*> paths_as_cstr;
    for(const auto& path: paths)
        paths_as_cstr.push_back(path.c_str());
    checkout_opts.paths.strings = const_cast<char **>(paths_as_cstr.data());

    checkout_opts.checkout_strategy = GIT_CHECKOUT_FORCE;

    // find latest commit of said branch
    auto full_branch_name = reference_name(
        parse_reference_from_name(repo_.get(), branch_name).get());
    auto last_commit = get_commit(full_branch_name);

    // checkout (the cast is necessary because libgit2 simulates inheritance by having a
    // struct git_object as the first member of the opaque struct git_commit)
    auto error = git_checkout_tree(repo_.get(),
        reinterpret_cast<const git_object*>(last_commit.get()), &checkout_opts);
    if (error)
        throw Error{ cat("Checkout: ", git_error_last()->message) };
}

void Repository::switch_branch(const std::string& branch_name)
{
    // get full name from branch identifier
    auto branch_ref = parse_reference_from_name(repo_.get(), branch_name);
    auto branch_full_name = reference_name(branch_ref.get());

    // switch HEAD
    int error = git_repository_set_head(repo_.get(), branch_full_name.c_str());
    if (error)
        throw Error{ cat("switch_branch: ", git_error_last()->message) };

    // go back to original state on this branch
    reset(0);
}



} // namespace git

// vi:ts=4:sw=4:sts=4:et
