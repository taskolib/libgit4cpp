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

#include "libgit4cpp/LibGitPointer.h"

namespace git {

/**
 * Function wrappers for libgit2 functions which manipulate pointers.
 * (Which take double pointers as parameters)
 * \note for documentation see the associated libgit2 function
 *       name: "git_FUNCTIONNAME"
 * \defgroup lgptrfunc Group of wrapped libgit2 functions
 * \{
 */

 /**
  * Return an existing repository.
  * \param repo_path absolute or relative path from executable
  * \return Wrapper of a git_repository*
 */
LibGitPointer<git_repository>
repository_open(const std::string& repo_path);

/**
 * Return a fresh initialized repository.
 * \param repo_path absolute or relative path from executable
 * \param is_bare if true, a git repo is created at repo_path. Else, .git is created in repo_path.
 * \return Wrapper of a git_repository*
*/
LibGitPointer<git_repository>
repository_init(const std::string& repo_path, bool is_bare);

/**
 * Return the current index of a repository.
 * \param repo C-type repository
 * \return Wrapper of a git_index*
*/
LibGitPointer<git_index>
repository_index(git_repository* repo);

/**
 * Generate a signature from system values (already collected by the C-type repository).
 * \param repo_path absolute or relative path from executable
 * \return Wrapper of a git_signature*
*/
LibGitPointer<git_signature>
signature_default(git_repository* repo);

/**
 * Generate a signature from given parameters.
 * \param name User name who creates the commit
 * \param email E-mail of the user
 * \param time timestamp
 * \param offset timezone adjustment for the timezone
 * \return Wrapper of a git_signature*
*/
LibGitPointer<git_signature>
signature_new(const std::string& name, const std::string& email, time_t time, int offset);


/**
 * Collect the current tree of the git repository.
 * \param repo C-type repository
 * \param tree_id identity number of the active tree
 * \return Wrapper of a git_signature*
*/
LibGitPointer<git_tree>
tree_lookup(git_repository* repo, git_oid tree_id);

/**
 * Create a new status list which contains status_list_entry* elements
 * \param repo C-type repository
 * \param status_opt struct of status options
 * \return Wrapper of git_status_list*
*/
LibGitPointer<git_status_list>
status_list_new(git_repository* repo, const git_status_options& status_opt);

/**
 * Collect the reference to the repository head.
 * \param repo C-type repository
 * \return Wrapper of git_reference*
 */
LibGitPointer<git_reference>
repository_head(git_repository* repo);

/**
 * Create a new remote connection in the repository and returns it
 * \param repo C-type repository
 * \param remote_name typically 'origin'
 * \param url adress of remote connection, e.g https://github.com/...
 * \return Wrapper of a remote connection
 */
LibGitPointer<git_remote>
remote_create (git_repository* repo, const std::string& remote_name,
                const std::string& url);


/**
 * Collects the remove connection by name and returns it
 * \param repo C-type repository
 * \param remote_name typically 'origin'
 * \return Wrapper of a remote connection
 */
LibGitPointer<git_remote>
remote_lookup (git_repository* repo, const std::string& remote_name);



/**
 * Clone existing git repository into local filesystem
 * \param url adress of remote connection, e.g https://github.com/...
 *  \param repo_path absolute or relative path from executable
*/
LibGitPointer<git_repository>
clone (const std::string& url, const std::string& repo_path);

/** \}*/

} // namespace git

#endif
