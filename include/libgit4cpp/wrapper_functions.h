/**
 * \file   wrapper_functions.h
 * \author Sven-Jannik Wöhnert
 * \date   Created on March 20, 2023
 * \brief  Declaration of wrappers for libgit2 functions.
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

#ifndef LIBGIT4CPP_WRAPPER_FUNCTIONS_H_
#define LIBGIT4CPP_WRAPPER_FUNCTIONS_H_

#include <string>
#include <typeinfo>
#include <utility>

#include <git2.h>

#include "libgit4cpp/types.h"

namespace git {

/**
 * Function wrappers for libgit2 functions which manipulate pointers.
 * (Which take a pointer to a pointer as output parameter)
 * \note For documentation see the associated libgit2 function
 *       name: "git_FUNCTIONNAME"
 * \defgroup lgptrfunc Group of wrapped libgit2 functions
 * \{
 */

/**
 * Open an existing repository.
 * \param repo_path Absolute or relative path to the repository root
 * \return new git_repository object for the opened repository
*/
LibGitRepository repository_open(const std::string& repo_path);

/**
 * Initialize a fresh repository.
 * \param repo_path Absolute or relative path to the repository root
 * \param is_bare If true, a git repo is created at repo_path. Else, .git is created in repo_path.
 * \return new git_repository object for the created repository
 */
LibGitRepository repository_init(const std::string& repo_path, bool is_bare);

/**
 * Return the current index of a repository.
 * \param repo Pointer to the repository object
 * \return new git_index object
 */
LibGitIndex repository_index(git_repository* repo);

/**
 * Generate a signature from system values.
 * Defaults deduced from existing repository object.
 * \param repo Pointer to repository object to use for defaults
 * \return new git_signature object
*/
LibGitSignature signature_default(git_repository* repo);

/**
 * Generate a signature from given parameters.
 * \param name User name who creates the commit
 * \param email E-mail of the user
 * \param time Unix time of the creation date
 * \param offset Timezone adjustment for the timestamp
 * \return new git_signature object
 */
LibGitSignature signature_new(const std::string& name, const std::string& email, time_t time, int offset);


/**
 * Collect the current tree of a git repository.
 * \param repo Pointer to repository object to examine
 * \param tree_id Identity number of the active tree
 * \return new git_tree object
 */
LibGitTree tree_lookup(git_repository* repo, git_oid tree_id);

/**
 * Create a new status list of the index.
 * The returned status object contains status_list_entry* elements.
 * \param repo Pointer to repository object
 * \param status_opt Struct of status options
 * \return new git_status_list object
 */
LibGitStatusList status_list_new(git_repository* repo, const git_status_options& status_opt);

/**
 * Collect the reference to the repository head.
 * \param repo Pointer to repository object
 * \return new git_reference object
 */
LibGitReference repository_head(git_repository* repo);

/**
 * Create a new remote connection in the repository.
 * \param repo Pointer to repository object
 * \param remote_name Name of the remote, e.g. "origin"
 * \param url Adress of remote connection, e.g https://github.com/...
 * \return new git_remote object
 */
LibGitRemote remote_create (git_repository* repo, const std::string& remote_name,
                const std::string& url);

/**
 * Find a remote repository by the name under which it is configured in the given
 * repository.
 * \param repo Pointer to the repository object to search in
 * \param remote_name Name of the remote repository, e.g. "origin"
 * \returns a pointer to a git_remote object (null if not found)
 */
LibGitRemote remote_lookup(git_repository* repo, const std::string& remote_name);

/**
 * Clone an existing git repository into the local filesystem.
 * \param url Address of remote connection, e.g https://github.com/...
 * \param repo_path Absolute or relative path to the repository root
 * \return new git_repository object
 */
LibGitRepository clone(const std::string& url, const std::string& repo_path);

/**
 * Find a named branch.
 * \param repo Pointer to repository object
 * \param branch_name The name to search for, e.g. 'main', 'fix-bugs'
 * \param branch_type Which branch type to find, enum with 1=GIT_BRANCH_LOCAL, 2=GIT_BRANCH_REMOTE, 3=GIT_BRANCH_ALL
 * \return new git_reference object
 */
LibGitReference branch_lookup(git_repository* repo, const std::string& branch_name, git_branch_t branch_type);


/**
 * Get the tree of a commit.
 * \param libgit2 git_commit pointer
 * \return git_tree wrapper
 */
LibGitTree commit_tree(git_commit* commit);


/**
 * Create a new branch.
 * \param repo Pointer to repository object
 * \param new_branch_name The name of the new branch
 * \param starting_commit reference object from where to branch out
 * \param force if True, it will force the creation even with uncommited changes
 * \return new git_reference object
 */
LibGitReference branch_create(git_repository* repo, const std::string& new_branch_name, const git_commit* starting_commit, int force);

/**
 * Find the name of a branch on the remote.
 * \param repo Pointer to repository object
 * \param branch_name Local branch name, e.g. 'main', 'fix-bugs'
 * \return name on remote , e.g. 'origin/main' or 'origin/fix-bugs'
 */
std::string branch_remote_name(git_repository* repo, const std::string& branch_name);

/**
 * Returns the human-readable name of a reference.
 * It is required for functions like \c git::branch_lookup() which requires a name instead of a reference object
 * \param ref constant git_reference object
 * \return readable name of reference (e.g. master, main, ...)
 */
std::string reference_shorthand(const git_reference* ref);

/**
 * Get full name of a reference.
 * \param ref libgit2 reference object
 * \return full name of reference (e.g. "refs/heads/master" == full name of "master")
 */
std::string reference_name(git_reference* ref);

/**
 * Find reference object from shortname, longname or id specification.
 * \param repo Pointer to repository object
 * \param name specification of reference (eg. refs/heads/master, main, e934a2, master@{2}, ...)
 * \return reference object
 */
LibGitReference parse_reference_from_name(git_repository* repo, const std::string& name);

/**
 * returns an iterator of the branches.
 * \param repo Pointer to repository object
 * \param flag bitwise flag libgit2 object.
 *      choices: GIT_BRANCH_LOCAL, GIT_BRANCH_REMOTE, GIT_BRANCH_ALL
 * \return iterator for existing branches
 */
LibGitBranchIterator branch_iterator(git_repository* repo, git_branch_t flag);

/**
 * Iterates branches over an iterator object.
 * \exception ref will be nullptr only if iteration object reached its end.
 *  otherwise exception will be thrown
 * \param branch_type defines if iterator is for local or remote branches
 * \param iter iterator object 
 */
LibGitReference branch_next(git_branch_t* branch_type, git_branch_iterator* iter);

/** \} */ // end of group lgptrfunc

} // namespace git

#endif
