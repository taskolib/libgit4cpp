/**
 * \file   Repository.h
 * \author Sven-Jannik Wöhnert
 * \date   Created on March 20, 2023
 * \brief  Wrapper for C-Package libgit2
 *
 * \copyright Copyright 2023-2024 Deutsches Elektronen-Synchrotron (DESY), Hamburg
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

#ifndef LIBGIT4CPP_REPOSITORY_H_
#define LIBGIT4CPP_REPOSITORY_H_

#include <filesystem>
#include <string>
#include <vector>

#include <git2.h>
#include <gul14/escape.h>

#include "libgit4cpp/Remote.h"
#include "libgit4cpp/types.h"

namespace git {

/**
 * Struct to express the git status for one file
 */
struct FileStatus
{
    std::string path_name; /// Relative path to file. If the path changed this value will have the shape "OLD_NAME -> NEW_NAME".
    std::string handling;  /// Handling status of file [unchanged, unstaged, staged, untracked, ignored]
    std::string changes;   /// Change status of file [new file, deleted, renamed, typechanged, modified, unchanged, ignored, untracked]

    friend std::ostream& operator<<(std::ostream& stream, FileStatus const& state) {
        stream << "FileStatus{ \"" << gul14::escape(state.path_name) << "\": " << state.handling << "; " << state.changes << " }";
        return stream;
    }
};

using RepoState = std::vector<FileStatus>; /// State of all files in the repo

inline std::ostream& operator<<(std::ostream& stream, const RepoState& repostate)
{
    stream << "RepoState {\n";
    for (auto& e : repostate)
        stream << e << '\n';
    stream << "}";
    return stream;
}


/**
 * A class to wrap used methods from C-Library libgit2.
 *
 * This class is necessary to handle C-pointers in a way that
 * C++ programmers using this class do not have to think about them.
 *
 * This class does not reflect the full libgit2 library.
 * git functions which are not implemented in this class will be
 * implemented in case of necessity
 */
class Repository
{
public:

    /**
     * Constructor which specifies the root dir of the git repository.
     * \param file_path  Path to git directory
     */
    explicit Repository(const std::filesystem::path& file_path);

    /**
     * Reset all knowledge this object knows about the repository and load the knowledge again.
     */
    void reset_repo();

    /// Returns member variable, which is the root dir of the git repository.
    std::filesystem::path get_path() const;

    /// Returns a non-owning raw pointer to the current repository.
    git_repository* get_repo();

    /**
     * Stage multiple new, changed, or removed files and folders in the repository directory.
     *
     * Instead of giving concrete paths to the files / objects to be added the function
     * takes a shell like glob/wildcard:
     * - \c * matches any number of any characters including none
     * - \c ? matches any single character
     * - \c [abc] matches one character given in the bracket (\c abc is example)
     * - \c [a-z] matches one character in the range of characters (\c a-z is example)
     * - \c \\* matches a star
     * - \c \\? matches a question mark
     * - \c \\\\ matches a backslash
     * - \c \\[ matches an opening square bracket
     *
     * The matching happens - unlike usual shells - on the whole pathname (inside the repository).
     * So `*` matches all files (including all subdirectories).
     *
     * Note: In fact you can never add a folder, you add files in a folder; i.e. it is
     * impossible to add an empty folder. Git just knows files, not folders as items.
     * If a file is in a folder the folder exists; if there is nothing in the folder the
     * folder is ignored. (File means also links etc.)
     *
     * Note: To write \c \? into a C++ string you probably need \c "\\?" etc.
     *
     * Note: Globs do not match hidden files in the form of Unix dotfiles; to match them the pattern
     * must explicitly start with \c . For example, \c * matches all visible files
     * while \c .* matches all hidden files.
     *
     * \param glob  The pattern with which files are selected to be added to the index
     *
     * \see add_files()
     */
    void add(const std::string& glob = "*");

    /**
     * Update the tracked files in the repository.
     *
     * The same as add() but only considers files that are already in the repository, never
     * adding just updating.
     *
     * \param glob  The pattern with which files are selected to be added to the index
     *
     * \see add() for an explanantion on the glob.
     */
    void update(const std::string& glob = "*");

    /**
     * Stage specific files listed in filepaths.
     * \param filepaths List of files. Either relative to repository root or absolute.
     * \return list of indices. An index from the filepaths vector is returned if
     *          the staging of the file failed. Returns an empty vector if all
     *          files were staged successfully.
     * \see add()
     */
   std::vector <int> add_files(const std::vector<std::filesystem::path>& filepaths);

    /**
     * Return the commit message of the HEAD commit.
     * \return message of last commit (=HEAD)
     */
    std::string get_last_commit_message();

    /**
     * Commit staged changes to the master branch of the git repository.
     * \param commit_message Customized message for the commit
     */
    void commit(const std::string& commit_message);

    /**
     * Hard reset of repository.
     * \param nr_of_commits number of commits to jump back
    */
    void reset(unsigned int nr_of_commits);

    /**
     * Add a new git remote with the specified name and URL to the repository.
     * \returns a Remote object representing the newly added remote
     * \exception Error is thrown if the remote cannot be added, for instance because it
     *            already exists.
     *
     * \code{.cpp}
     * Repository repo{ "/path/to/repo" };
     * repo.add_remote("origin", "https://gitlab.com/a/b.git");
     * repo.add_remote("upstream", "file:///path/to/upstream/repo");
     * \endcode
     */
    Remote add_remote(const std::string& remote_name, const std::string& url);

    /**
     * Look up a git remote by name in the repository.
     * \returns a Remote object if the remote exists, or an empty optional otherwise.
     */
    gul14::optional<Remote> get_remote(const std::string& remote_name) const;

    /**
     * Return a list of all configured remote repositories.
     * \exception Error is thrown if the list cannot be retrieved.
     */
    std::vector<Remote> list_remotes() const;

    /**
     * Return a list containing the names of all configured remote repositories (such as
     * "origin").
     * \exception Error is thrown if the remote list cannot be retrieved.
     */
    std::vector<std::string> list_remote_names() const;

    /**
     * Push something to the specified remote.
     *
     * In git parlance, this updates a remote ref from a local ref. The "refspec"
     * specifies which local ref and which remote ref is used. For instance, the default
     * "HEAD:refs/heads/main" pushes HEAD (whatever is currently checked out in the local
     * working directory) to the remote branch "main".
     *
     * \param remote   The git remote to push to (e.g. obtained by get_remote())
     * \param refspec  The refspec to push (e.g. "HEAD" or "refs/heads/main")
     */
    void push(const Remote& remote, const std::string& refspec = "HEAD:refs/heads/main");

#if 0
    /**
     * Pull changes from the remote repository.
     * \param addr Address of the git repository host
     */
    void pull();

    /**
     * Clone a repository.
     * \param addr Address of the git repository host
    */
    void clone_repo(const std::string& url, const std::filesystem::path& repo_path);

    /**
     * Check if remote and local branch are in same state.
     * \param branch_name
    */
    bool branch_up_to_date(const std::string& branch_name);

#endif


    /**
     * Create a new branch from the current branch.
     * \param branch_name name of the new branch
    */
    LibGitReference new_branch(const std::string& branch_name);

    /**
     * Create a new branch from a specified existing branch.
     * \param branch_name name of the new branch
     * \param origin_branch_name name of the existing branch to checkout from
    */
    LibGitReference new_branch(const std::string& branch_name, const std::string& origin_branch_name);

    /**
     * Checkout a branch with options.
     * The path parameter takes a list of files, directories. It supports pattern matching
     * \param branch_name The branch to checkout
     * \param paths specifies the files to checkout
    */
    void checkout(const std::string& branch_name, const std::vector<std::string>& paths);

    /**
     * Remove all entries from the index under a given directory.
     *
     * No files are removed from the filesystem.
     *
     * \param directory Directory to remove
     * \attention Directory is a relative path within repo_path_
     * \See remove_files() for hints about file removal
     */
    void remove_directory(const std::filesystem::path& directory);

    /**
     * Delete specific files from git index.
     *
     * No files are removed from the filesystem.
     *
     * They also do not need to exist on the filesystem to be removable from the index.
     *
     * To delete files from the repository you can eiter:
     *
     * - remove the files from the repository With remove_files()
     * - and remove the files from the filesystem with \c std::filesystem::remove()
     *
     * - remove the files from the filesystem with \c std::filesystem::remove()
     * - add the removal of the files with add_files() or with remove_files()
     *
     * \param filepaths list of files
     * \attention files in filepaths are relative to repo_path_
     * \see remove_directory()
    */
    void remove_files(const std::vector<std::filesystem::path>& filepaths);

    /**
     * Returns current git status.
     * This includes unchanged, untracked and ingnored files and directories.
     * Each files status is represented by
     * -- file name
     * -- change status (has the file changed)
     * -- handling status (what git will do with it)
     * \return vector of file status for each file.
     */
    RepoState status();

    /// Destructor
    ~Repository();

private:

    /// Path to the repository.
    std::filesystem::path repo_path_;

    /// Pointer which holds all infos of the active repository.
    LibGitRepository repo_{ nullptr, git_repository_free };

    /// Signature used in commits.
    LibGitSignature my_signature_{ nullptr, git_signature_free };

    /**
     * Initialize a new git repository and commit all files in its path.
     * \note This is a private member function because git repository init
     *       should be done by an LibGit Object Initialization
     */
    void init(const std::filesystem::path& file_path);

    /**
     * Make the first commit. Function is called by init.
     * \note slightly differs from commit because in this case,
     *       there is no previous commit to dock on.
     */
    void commit_initial();

    /**
     * Get a specific commit.
     * \param count Jump back count number of commits behind HEAD
     * \return C-type commit object
     */
    LibGitCommit get_commit(unsigned int count = 0);

    /**
     * Get a specific commit.
     * \param ref Use hex string identifier to find a commit
     * \return C-type commit object
     */
    LibGitCommit get_commit(const std::string& ref);

    /**
     * Load a git signature or create a default signature.
     */
    void make_signature();

    /**
     * Translate all status information for each submodule into String.
     * \param status C-type status of all submodules from libgit
     * \return A vector of dynamic length which contains a status struct
     */
    RepoState collect_status(LibGitStatusList& status) const;

    /**
     * Check if the file from the status entry is not staged and collect the status in filestats.
     * \note this function is solely called by collect_status. It has no other purpose.
     * \param filestats a structure which will be filled with the status of the file if the file is unstaged
     * \param s status of the file. A struct which contains flags about every possible state
     * \return true if file is unstaged (and filestats initialized), else false
     */
    static bool is_unstaged(FileStatus& filestats, const git_status_entry* s);

    /**
     * Check if the file from the status entry is not unstaged and collect the status in filestats.
     * \note this function is solely called by collect_status. It has no other purpose.
     * \param filestats a structure which will be filled with the status of the file if the file is staged
     * \param s status of the file. A struct which contains flags about every possible state
     * \return true if file is staged (and filestats initialized), else false
     */
    static bool is_staged(FileStatus& filestats, const git_status_entry* s);

};

} // namespace git

#endif

// vi:ts=4:sw=4:sts=4:et
