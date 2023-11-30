/**
 * \file   GitRepository.cc
 * \author Sven-Jannik Wöhnert
 * \date   Created on March 20, 2023
 * \brief  Implementation of the GitRepository class.
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

#include <iostream>
#include <vector>

#include <git2.h>
#include <gul14/cat.h>

#include "libgit4cpp/Error.h"
#include "libgit4cpp/GitRepository.h"
#include "libgit4cpp/wrapper_functions.h"

namespace git {

GitRepository::GitRepository(const std::filesystem::path& file_path, const std::string& url)
    : repo_path_{ file_path }
    , url_{ url }
{
    git_libgit2_init();
    init(file_path);

    //check if remote already exists, else create new remote
    auto remote_ = remote_lookup(repo_.get(), "origin");
    if (remote_.get() == nullptr)
        remote_ = remote_create(repo_.get(), "origin", url.c_str());
}

GitRepository::~GitRepository()
{
    repo_.reset();
    my_signature_.reset();
    git_libgit2_shutdown();
}


void GitRepository::make_signature()
{
    my_signature_ = signature_default(repo_.get());
    if (my_signature_.get() == nullptr)
        my_signature_ = signature_new("Taskomat", "(none)", std::time(0), 0);
}

void GitRepository::reset_repo()
{
    repo_.reset();
    my_signature_.reset();

    // initialize repo and signature again
    init(repo_path_);
}

std::string GitRepository::get_last_commit_message()
{
    const auto commit = get_commit();
    return git_commit_message(commit.get());
}

std::filesystem::path GitRepository::get_path() const
{
    return repo_path_;
}

git_repository* GitRepository::get_repo()
{
    return repo_.get();
}

void GitRepository::update()
{
    auto index = repository_index(repo_.get());

    char* paths[1] = { const_cast<char*>("*") };
    git_strarray array = { paths, 1 };

    // update index to check for files
    git_index_update_all(index.get(), &array, nullptr, nullptr);
    git_index_write(index.get());
}

void GitRepository::init(const std::filesystem::path& file_path)
{
    repo_ = repository_open(repo_path_);

    if (repo_.get() == nullptr)
    {
        // create repository
        // 2nd argument: false so that .git folder is created in given path
        repo_ = repository_init(file_path, false);
        if (repo_.get() == nullptr)
            throw git::Error{ "Git init failed" };

        make_signature();
        update();
        commit_initial();
    }
    else
    {
        make_signature();
    }
}

void GitRepository::commit_initial()
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
        throw git::Error{ gul14::cat("Initial commit failed: ", git_error_last()->message, "\n") };
}

void GitRepository::commit(const std::string& commit_message)
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
        throw git::Error{ gul14::cat("Commit: ", git_error_last()->message, "\n") };
}

void GitRepository::add(const std::string& glob)
{
    auto gindex = repository_index(repo_.get());

    char *paths[] = { const_cast<char*>(glob.c_str()) };
    git_strarray array = { paths, 1 };

    int error = git_index_add_all(gindex.get(), &array, GIT_INDEX_ADD_DEFAULT, nullptr, nullptr);
    if (error)
        throw git::Error{ gul14::cat("Cannot stage files: ", git_error_last()->message, "\n") };

    git_index_write(gindex.get());
}

void GitRepository::remove_directory(const std::filesystem::path& directory)
{
    auto gindex = repository_index(repo_.get());

    int error = git_index_remove_directory(gindex.get(), directory.c_str(), 0);
    if (error)
        throw git::Error{ gul14::cat("Cannot remove directory: ", git_error_last()->message, "\n") };

    git_index_write(gindex.get());
}

void GitRepository::remove_files(const std::vector<std::filesystem::path>& filepaths)
{
    auto gindex = repository_index(repo_.get());

    //remove files from directory
    //TODO: Teste, ob oberer Teil ausreicht
    for (auto gfile: filepaths)
    {
        int error = git_index_remove_bypath(gindex.get(), gfile.c_str());
        if (error)
            throw git::Error{ gul14::cat("Cannot remove file: ", git_error_last()->message, "\n") };
    }

    git_index_write(gindex.get());
}

LibGitPointer<git_commit> GitRepository::get_commit(int count)
{
    auto ref = "HEAD~" + std::to_string(count);
    return get_commit(ref);
}

LibGitPointer<git_commit> GitRepository::get_commit(const std::string& ref)
{
    git_commit* commit;
    git_oid oid_parent_commit;

    // resolve HEAD into a SHA1
    int error = git_reference_name_to_id( &oid_parent_commit, repo_.get(), ref.c_str());
    if (error)
        throw git::Error{ gul14::cat("Cannot find ID from reference name: ", git_error_last()->message, "\n") };

    // find commit object by commit ID
    error = git_commit_lookup( &commit, repo_.get(), &oid_parent_commit);
    if (error)
        throw git::Error{ "Cannot find HEAD of branch." };

    return commit;
}

LibGitPointer<git_commit> GitRepository::get_commit()
{
    return get_commit(std::string{ "HEAD" });
}

bool GitRepository::is_unstaged(FileStatus& filestats, const git_status_entry* s)
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
        filestats.path_name = gul14::cat(old_path, " -> ", new_path);
    else
        filestats.path_name = old_path ? old_path : new_path;

    return true;
}

bool GitRepository::is_staged(FileStatus& filestats, const git_status_entry* s)
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
        filestats.path_name = gul14::cat(old_path, " -> ", new_path);
    else
        filestats.path_name = old_path ? old_path : new_path;

    return true;
}

std::vector<FileStatus> GitRepository::collect_status(LibGitPointer<git_status_list>& status) const
{
    // get number of submodules
    const size_t nr_entries = git_status_list_entrycount(status.get());

    // declare status holding vector for each submodule
    std::vector<FileStatus> return_array;
    FileStatus filestats;

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

std::vector<FileStatus> GitRepository::status()
{
    git_status_options status_opt = GIT_STATUS_OPTIONS_INIT;
    status_opt.flags =  GIT_STATUS_OPT_INCLUDE_UNTRACKED |          // untracked files
                        GIT_STATUS_OPT_RECURSE_UNTRACKED_DIRS |     // untracked directories
                        GIT_STATUS_OPT_INCLUDE_UNMODIFIED |         // unmodified files
                        GIT_STATUS_OPT_INCLUDE_IGNORED;             // ignored files

    auto my_status = status_list_new(repo_.get(), status_opt);
    if (my_status.get() == nullptr)
        throw git::Error{ "Cannot initialize status." };

    return collect_status(my_status);
}

std::vector<int> GitRepository::add_files(const std::vector<std::filesystem::path>& filepaths)
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

void GitRepository::reset(int nr_of_commits)
{
    const auto parent_commit = get_commit(nr_of_commits);

    int error = git_reset(repo_.get(), (git_object*) parent_commit.get(), GIT_RESET_HARD, nullptr);
    if (error)
        throw git::Error{ gul14::cat("Reset: ", git_error_last()->message, "\n") };
}

void GitRepository::push()
{
    // set options
    git_push_options gpush;
    int error = git_push_init_options(&gpush, GIT_PUSH_OPTIONS_VERSION);
    if (error)
        throw git::Error{ gul14::cat("Init push: ", git_error_last()->message, "\n") };

    // set remote
    /*
    auto remote = remote_lookup(repo_.get(), "origin");
    if (remote.get() == nullptr)
        throw git::Error{ "Cannot find remote object." };
    */

    // push to upstream
    error = git_remote_push(remote_.get(), nullptr, &gpush);
    if (error)
        throw git::Error{ gul14::cat("Push remote: ", git_error_last()->message, "\n") };
}


void GitRepository::pull()
{
    // define fetch options
    git_fetch_options options = GIT_FETCH_OPTIONS_INIT;

    // set remote
    /*
    auto remote = remote_lookup(repo_.get(), "origin");
    if (remote.get() == nullptr)
        throw git::Error{ gul14::cat("Cannot find remote object.") };
    */

    // fetch commits from remote connection
    int error = git_remote_fetch( remote_.get(), NULL, &options, NULL );
    if (error)
        throw git::Error{ gul14::cat("Pull: ", git_error_last()->message, "\n") };

    // git_apply, if failed git_merge
    // TODO

    //cleanup repository from remote connection and merge/fetch pointers
    git_repository_state_cleanup(repo_.get());


}

void GitRepository::clone_repo(const std::string& url, const std::filesystem::path& repo_path )
{
    auto repo = clone(url, repo_path);
    if (repo.get() == nullptr)
    {
        throw git::Error{ gul14::cat("Cannot clone repository.") };
    }
    else
    {
        url_ = url;
        remote_ = remote_lookup(repo.get(), "origin"); // TODO: origin korrekt?
        repo_path_ = repo_path;
        repo_=repo.get(); // move ownership: repo -> repo_ && nullptr -> repo

    }
}


bool GitRepository::branch_up_to_date(const std::string& branch_name)
{
    auto local_ref = branch_lookup(repo_.get(), "master", GIT_BRANCH_LOCAL);
    if (local_ref.get() == nullptr)
        throw git::Error{ gul14::cat("Branch lookup: ", git_error_last()->message, "\n") };

    // Get the name of the remote associated with the local branch
    //TODO: wrapper for buffer?
    auto remote_name = branch_remote_name(repo_.get(), branch_name.c_str());
    if (remote_name == "")
        throw git::Error{ gul14::cat("Failed to get remote name for the local branch.") };

    // Open the remote
    auto remote = remote_lookup(repo_.get(), remote_name);
    if (remote.get() == nullptr)
        throw git::Error{ gul14::cat("Cannot find remote object.") };

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


} //namespace task

// vi:ts=4:sw=4:sts=4:et
