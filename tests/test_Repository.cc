/**
 * \file   test_Repository.cc
 * \author Sven-Jannik Wöhnert
 * \date   Created on March 22, 2023
 * \brief  Test suite for the Repository class.
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

#include <filesystem>
#include <fstream>
#include <iostream>

#include <git2.h>
#include <gul14/catch.h>
#include <gul14/gul.h>

#include "libgit4cpp/Error.h"
#include "libgit4cpp/Repository.h"
#include "libgit4cpp/wrapper_functions.h"
#include "test_main.h"

using namespace git;
using namespace std::literals;
using gul14::cat;

namespace {

const auto reporoot = unit_test_folder() / "reporoot";

/**
 * Create a directory and store files in it.
 *
 * Filestructure:
 * \code
 * reporoot/
 *      $name$/
 *          file0.txt   << $msg$ /n file0
 *          file1.txt   << $msg$ /n file1
 *          ...
 * \endcode
 * \param name Name of the subdirectory
 * \param nr_files Number of files to be created
 * \param msg What to write to the file
 */
void create_testfiles(const std::filesystem::path& name, size_t nr_files,
    const std::string& msg)
{
    auto p = reporoot / name;
    std::filesystem::create_directories(p);
    for (size_t i = 0; i < nr_files; ++i)
    {
        // msg
        // file i
        std::ofstream f(p / cat("file", i, ".txt"));
        f << msg << cat("\nfile", i);
    }
}

} // anonymous namespace

TEST_CASE("Repository Wrapper Test all", "[Repository]")
{
    /**
     * Create files in a directory and then initialize the git repository within.
     * The initial commit must be empty as no staging has been done.
     * Check for the initialization of member variables
     * Check if a repository is created (HEAD exists)
     * Check if a comit was created (initial commit)
     *
    */
    SECTION("Construct Repository object")
    {
        std::filesystem::remove_all(reporoot);
        create_testfiles("unit_test_1", 2, "Construct");

        Repository gl{ reporoot };

        REQUIRE(not gl.get_path().empty());
        REQUIRE(gl.get_path() == reporoot);
        auto ref{ repository_head(gl.get_repo()) };
        REQUIRE(ref.get() != nullptr);

        // Test if repo_ got initialized
        REQUIRE(gl.get_last_commit_message() == "Initial commit");

        // We have no remote at this point
        REQUIRE_THROWS(branch_remote_name(gl.get_repo(), "master"));
    }


    /**
     * Check the general staging function "add".
     * 1) Load existing repository (in contrast to initialize in first section)
     * 2) Create files after repository loading (in contrast to first section)
     * 3) Check if files appear as untracked
     * 4) Stage all files. Now the status should show 4 staged new files
     */
    SECTION("Stage files (git add)")
    {
        create_testfiles("unit_test_2", 2, "Stage");

        Repository gl{ reporoot };

        auto stats = gl.status();
        REQUIRE(stats.size() != 0);
        for (const auto& elm: stats)
        {
            if (gul14::starts_with(elm.path_name, "unit_test_1") || gul14::starts_with(elm.path_name, "unit_test_2"))
            {
                REQUIRE(elm.handling == "untracked");
                REQUIRE(elm.changes == "untracked");
            }
        }

        gl.add();

        stats = gl.status();

        // new files from unit_test_2 should be in stage mode
        REQUIRE(stats.size() != 0);
        for (const auto& elm: stats)
        {
            if (gul14::starts_with(elm.path_name, "unit_test_1") || gul14::starts_with(elm.path_name, "unit_test_2"))
            {
                REQUIRE(elm.handling == "staged");
                REQUIRE(elm.changes == "new file");
            }
        }
    }

    /**
     * Commit the previous staged files.
     * 1) Load repository. 4 files should still be staged
     * 2) Last commit should be the initial commit
     * 3) commit staged files. files should now be in status "unchanged"
     * 4) Check if commit message was set successful
     */
    SECTION("Commit")
    {

        // Create Git Library
        Repository gl{ reporoot };

        auto stats = gl.status();

        // new files from unit_test_2 should be still in stage mode
        REQUIRE(stats.size() != 0);
        for (const auto& elm: stats)
        {
            if (gul14::starts_with(elm.path_name, "unit_test_1") || gul14::starts_with(elm.path_name, "unit_test_2"))
            {
                REQUIRE(elm.handling == "staged");
                REQUIRE(elm.changes == "new file");
            }
        }

        // Check if repo_ can be found again
        REQUIRE(gl.get_last_commit_message() == "Initial commit");

        gl.commit("Add files");

        stats = gl.status();
        REQUIRE(stats.size() != 0);
        for (const auto& elm: stats)
        {
            if (gul14::starts_with(elm.path_name, "unit_test_1") || gul14::starts_with(elm.path_name, "unit_test_2"))
            {
                REQUIRE(elm.handling == "unchanged");
                REQUIRE(elm.changes == "unchanged");
            }
        }

        REQUIRE(gl.get_last_commit_message() == "Add files");
    }


    /**
     * Change two files, but only stage one of them.
     * 1) Manipulate both files of unit_test_1
     * 2) check if status of them is modified, but unstaged
     * 3) stage file1 of unit_test_1
     * 4) file1 should be staged and file0 still be unstaged
    */
    SECTION("Add by path")
    {
        Repository gl{ reporoot };

        create_testfiles("unit_test_1", 2, "Add by path");

        auto stats = gl.status();
        REQUIRE(stats.size() != 0);
        for (const auto& elm: stats)
        {
            if (gul14::starts_with(elm.path_name, "unit_test_1/file"))
            {
                REQUIRE(elm.handling == "unstaged");
                REQUIRE(elm.changes == "modified");
            }
            else if (gul14::starts_with(elm.path_name, "unit_test_2/file"))
            {
                REQUIRE(elm.handling == "unchanged");
                REQUIRE(elm.changes == "unchanged");
            }
        }

        // Try the dump to stream operator with some 'interesting' state:
        std::stringstream ss{ };
        ss << stats;
        REQUIRE(gul14::trim(ss.str()) == "RepoState {\n" \
            "FileStatus{ \"unit_test_1/file0.txt\": unstaged; modified }\n" \
            "FileStatus{ \"unit_test_1/file1.txt\": unstaged; modified }\n" \
            "FileStatus{ \"unit_test_2/file0.txt\": unchanged; unchanged }\n" \
            "FileStatus{ \"unit_test_2/file1.txt\": unchanged; unchanged }\n" \
            "}");

        auto ret = gl.add_files({"unit_test_1/file1.txt"});

        // No errors should have occur
        REQUIRE(ret.size() == 0);

        stats = gl.status();
        REQUIRE(stats.size() != 0);
        for(const auto& elm: stats)
        {
            if (gul14::starts_with(elm.path_name, "unit_test_1/file0"))
            {
                REQUIRE(elm.handling == "unstaged");
                REQUIRE(elm.changes == "modified");
            }
            else if (gul14::starts_with(elm.path_name, "unit_test_1/file1"))
            {
                REQUIRE(elm.handling == "staged");
                REQUIRE(elm.changes == "modified");
            }
            else if (gul14::starts_with(elm.path_name, "unit_test_2/file"))
            {
                REQUIRE(elm.handling == "unchanged");
                REQUIRE(elm.changes == "unchanged");
            }
        }

        gl.commit("Add by path");
    }

    SECTION("Delete file")
    {
        // Create Git Library
        Repository gl{ reporoot };

        std::filesystem::path myfile = "unit_test_2/file1.txt";
        REQUIRE(std::filesystem::exists(gl.get_path() / myfile) == true);

        gl.remove_files({myfile});

        auto stats = gl.status();

        // every file in unit_test_2 should have the tag deleted
        REQUIRE(stats.size() != 0);
        for(const auto& elm: stats)
        {
            if (gul14::starts_with(elm.path_name, "unit_test_2/file1.txt"))
            {
                REQUIRE(elm.handling == "staged");
                REQUIRE(elm.changes == "deleted");
            }
        }

        gl.commit("remove file");

        REQUIRE(std::filesystem::exists(gl.get_path() / myfile) == true);
        std::filesystem::remove( reporoot / "unit_test_2/file1.txt");
        REQUIRE(std::filesystem::exists(gl.get_path() / myfile) == false);
    }

    SECTION("Get previous commit (git reset)")
    {
        Repository gl{ reporoot };

        std::stringstream ss{ };
        ss << gl.status();
        REQUIRE(gul14::trim(ss.str()) == "RepoState {\n" \
            "FileStatus{ \"unit_test_1/file0.txt\": unstaged; modified }\n" \
            "FileStatus{ \"unit_test_1/file1.txt\": unchanged; unchanged }\n" \
            "FileStatus{ \"unit_test_2/file0.txt\": unchanged; unchanged }\n" \
            "}");

        gl.reset(0); // `git reset --hard`: undo changes

        ss.str("");
        ss << gl.status();
        REQUIRE(gul14::trim(ss.str()) == "RepoState {\n" \
            "FileStatus{ \"unit_test_1/file0.txt\": unchanged; unchanged }\n" \
            "FileStatus{ \"unit_test_1/file1.txt\": unchanged; unchanged }\n" \
            "FileStatus{ \"unit_test_2/file0.txt\": unchanged; unchanged }\n" \
            "}");

        gl.reset(1); // `git reset --hard HEAD~1`: undo last commit

        ss.str("");
        ss << gl.status();
        REQUIRE(gul14::trim(ss.str()) == "RepoState {\n" \
            "FileStatus{ \"unit_test_1/file0.txt\": unchanged; unchanged }\n" \
            "FileStatus{ \"unit_test_1/file1.txt\": unchanged; unchanged }\n" \
            "FileStatus{ \"unit_test_2/file0.txt\": unchanged; unchanged }\n" \
            "FileStatus{ \"unit_test_2/file1.txt\": unchanged; unchanged }\n" \
            "}");

        REQUIRE_THROWS(gl.reset(3)); // We do not have so many ancestors
    }

    /**
     * remove a directory and check if the repository status notices
     * 1) Delete unit_test_2
     * 2) Files should be automatically staged for deletion
     * 3) Commit removal
     * 4) check if files are gone from status (should)
     * 5) check if files are gone from filesystem (should not)
     */
    SECTION("Delete Directory")
    {
        // Create Git Library
        Repository gl{ reporoot };

        std::filesystem::path mypath = "unit_test_2";
        REQUIRE(std::filesystem::exists(gl.get_path() / mypath) == true);

        gl.remove_directory(mypath);

        auto stats = gl.status();

        // every file in unit_test_2 should have the tag deleted
        REQUIRE(stats.size() != 0);
        for(const auto& elm: stats)
        {
            if (gul14::starts_with(elm.path_name, "unit_test_2"))
            {
                REQUIRE(elm.handling == "staged");
                REQUIRE(elm.changes == "deleted");
            }
        }

        gl.commit("remove files");

        stats = gl.status();

        // check if files are removed from repository status
        REQUIRE(stats.size() != 0);
        for(const auto& elm: stats)
        {
            if (gul14::starts_with(elm.path_name, "unit_test_2/file"))
                REQUIRE (elm.changes == "untracked");
        }

        REQUIRE(std::filesystem::exists(gl.get_path() / mypath) == true);
    }

    SECTION("Adding with glob (git add)")
    {
        Repository gl{ reporoot };
        auto ss = std::stringstream{ };
        ss << gl.status();
        REQUIRE(gul14::trim(ss.str()) == "RepoState {\n" \
            "FileStatus{ \"unit_test_1/file0.txt\": unchanged; unchanged }\n" \
            "FileStatus{ \"unit_test_1/file1.txt\": unchanged; unchanged }\n" \
            "FileStatus{ \"unit_test_2/file0.txt\": untracked; untracked }\n" \
            "FileStatus{ \"unit_test_2/file1.txt\": untracked; untracked }\n" \
            "}");

        gl.add("*le1*");

        ss.str("");
        ss << gl.status();
        REQUIRE(gul14::trim(ss.str()) == "RepoState {\n" \
            "FileStatus{ \"unit_test_1/file0.txt\": unchanged; unchanged }\n" \
            "FileStatus{ \"unit_test_1/file1.txt\": unchanged; unchanged }\n" \
            "FileStatus{ \"unit_test_2/file0.txt\": untracked; untracked }\n" \
            "FileStatus{ \"unit_test_2/file1.txt\": staged; new file }\n" \
            "}");
    }
}

TEST_CASE("Repository add() with glob", "[Repository]")
{
    std::filesystem::remove_all(reporoot);
    create_testfiles(".Atlantis", 1, "Atlantis");
    create_testfiles("Burundi", 3, "Gitega");
    create_testfiles("Honduras", 1, "Tegucigalpa");
    create_testfiles("Japan", 2, "Tokyo");
    create_testfiles("Japan/Hokkaido", 1, "Sapporo");
    create_testfiles("Japan/Hyogo", 2, "Kobe");
    create_testfiles("Malaysia", 1, "Kuala Lumpur");
    create_testfiles("Paraguay", 2, "Asuncion");
    create_testfiles("Peru", 2, "Lima");
    Repository gl{ reporoot };
    gl.reset(0);

    /**
     * Now we have:
     *
     * └── reporoot
     *     ├── .Atlantis
     *     │   └── file0.txt
     *     ├── Burundi
     *     │   ├── file0.txt
     *     │   ├── file1.txt
     *     │   └── file2.txt
     *     ├── Honduras
     *     │   └── file0.txt
     *     ├── Japan
     *     │   ├── file0.txt
     *     │   ├── file1.txt
     *     │   ├── Hokkaido
     *     │   │   └── file0.txt
     *     │   └── Hyogo
     *     │       ├── file0.txt
     *     │       └── file1.txt
     *     ├── Malaysia
     *     │   └── file0.txt
     *     ├── Paraguay
     *     │   ├── file0.txt
     *     │   └── file1.txt
     *     └── Peru
     *         ├── file0.txt
     *         └── file1.txt
     */

    SECTION("Star glob on files 1")
    {
        gl.add("file1*");
        // We expect to add no file at all, because the glob matches the full pathname
        // and no file starts with a pathname "file1..."
        auto stats = gl.status();
        auto new_end = std::remove_if(stats.begin(), stats.end(), [](FileStatus const& v) {
            return v.handling != "staged";
        });
        stats.erase(new_end, stats.end());
        std::stringstream ss{ };
        ss << stats;
        INFO(gul14::trim(ss.str()));
        REQUIRE(stats.size() == 0);
    }
    SECTION("Star glob on files 2")
    {
        gl.add("*/file1*");
        // Burundi/file1.txt
        // Japan/file1.txt
        // Japan/Hyogo/file1.txt
        // Paraguay/file1.txt
        // Peru/file1.txt
        auto stats = gl.status();
        auto new_end = std::remove_if(stats.begin(), stats.end(), [](FileStatus const& v) {
            return v.handling != "staged";
        });
        stats.erase(new_end, stats.end());
        std::stringstream ss{ };
        ss << stats;
        INFO(gul14::trim(ss.str()));
        REQUIRE(stats.size() == 5);
    }
    SECTION("Star glob on directories 1")
    {
        gl.add("*/H*");
        // Note: Not "Honduras/..."
        // This is the same: gl.add("*/H*/*");
        //
        // Japan/Hokkaido/file0.txt
        // Japan/Hyogo/file0.txt
        // Japan/Hyogo/file1.txt
        auto stats = gl.status();
        auto new_end = std::remove_if(stats.begin(), stats.end(), [](FileStatus const& v) {
            return v.handling != "staged";
        });
        stats.erase(new_end, stats.end());
        std::stringstream ss{ };
        ss << stats;
        INFO(gul14::trim(ss.str()));
        REQUIRE(stats.size() == 3);
    }
    SECTION("Star glob on directories 2")
    {
        gl.add("H*");
        // Note: Not Japan's prefectures
        // This is the same: gl.add("H*/*");
        //
        // Honduras/file0.txt
        auto stats = gl.status();
        auto new_end = std::remove_if(stats.begin(), stats.end(), [](FileStatus const& v) {
            return v.handling != "staged";
        });
        stats.erase(new_end, stats.end());
        std::stringstream ss{ };
        ss << stats;
        INFO(gul14::trim(ss.str()));
        REQUIRE(stats.size() == 1);
    }
    SECTION("Questionmark glob")
    {
        gl.add("*P??u*");
        // Peru/file0.txt
        // Peru/file1.txt
        auto stats = gl.status();
        auto new_end = std::remove_if(stats.begin(), stats.end(), [](FileStatus const& v) {
            return v.handling != "staged";
        });
        stats.erase(new_end, stats.end());
        std::stringstream ss{ };
        ss << stats;
        INFO(gul14::trim(ss.str()));
        REQUIRE(stats.size() == 2);
    }
    SECTION("Selection glob simple")
    {
        gl.add("*[aio]/*");
        // Note: This also matches subdirs
        //
        // Burundi/file0.txt
        // Burundi/file1.txt
        // Burundi/file2.txt
        // Japan/Hokkaido/file0.txt
        // Japan/Hyogo/file0.txt
        // Japan/Hyogo/file1.txt
        // Malaysia/file0.txt
        auto stats = gl.status();
        auto new_end = std::remove_if(stats.begin(), stats.end(), [](FileStatus const& v) {
            return v.handling != "staged";
        });
        stats.erase(new_end, stats.end());
        std::stringstream ss{ };
        ss << stats;
        INFO(gul14::trim(ss.str()));
        REQUIRE(stats.size() == 7);
    }
    SECTION("Selection glob range")
    {
        gl.add("*[1-3]*");
        // Burundi/file1.txt
        // Burundi/file2.txt
        // Japan/Hyogo/file1.txt
        // Japan/file1.txt
        // Paraguay/file1.txt
        // Peru/file1.txt
        auto stats = gl.status();
        auto new_end = std::remove_if(stats.begin(), stats.end(), [](FileStatus const& v) {
            return v.handling != "staged";
        });
        stats.erase(new_end, stats.end());
        std::stringstream ss{ };
        ss << stats;
        INFO(gul14::trim(ss.str()));
        REQUIRE(stats.size() == 6);
    }
    SECTION("Hidden files need extra globs")
    {
        gl.add(".*");
        // .Atlantis/file0.txt
        auto stats = gl.status();
        auto new_end = std::remove_if(stats.begin(), stats.end(), [](FileStatus const& v) {
            return v.handling != "staged";
        });
        stats.erase(new_end, stats.end());
        std::stringstream ss{ };
        ss << stats;
        INFO(gul14::trim(ss.str()));
        REQUIRE(stats.size() == 1);
    }
}

TEST_CASE("Repository: get_remote(), add_remote()", "[Repository]")
{
    std::filesystem::remove_all(reporoot);

    constexpr auto* repo_url
        = "https://gitlab.desy.de/jannik.woehnert/taskolib_remote_test.git";

    Repository repo{ reporoot };

    auto maybe_remote = repo.get_remote("origin");
    REQUIRE(maybe_remote.has_value() == false);

    auto remote = repo.add_remote("origin", repo_url);
    REQUIRE(remote.get() != nullptr);
    REQUIRE(remote.get_name() == "origin");
    REQUIRE(remote.get_url() == repo_url);

    maybe_remote = repo.get_remote("origin");
    REQUIRE(maybe_remote.has_value());
    REQUIRE(maybe_remote->get() != nullptr);
    REQUIRE(maybe_remote->get_name() == "origin");
    REQUIRE(maybe_remote->get_url() == repo_url);
}

TEST_CASE("Repository: list_remotes(), add_remote()", "[Repository]")
{
    std::filesystem::remove_all(reporoot);

    constexpr auto* repo_url
        = "https://gitlab.desy.de/jannik.woehnert/taskolib_remote_test.git";

    Repository repo{ reporoot };

    // Repo list must be empty
    auto remotes = repo.list_remotes();
    REQUIRE(remotes.empty());

    // Add a remote
    auto remote = repo.add_remote("origin", repo_url);
    REQUIRE(remote.get() != nullptr);
    REQUIRE(remote.get_name() == "origin");
    REQUIRE(remote.get_url() == repo_url);

    // Repo list must not be empty anymore
    remotes = repo.list_remotes();
    REQUIRE(remotes.size() == 1);
    REQUIRE(remotes[0].get() != nullptr);
    REQUIRE(remotes[0].get_name() == "origin");
    REQUIRE(remotes[0].get_url() == repo_url);

    // Adding the same remote again must fail
    REQUIRE_THROWS_AS(repo.add_remote("origin", repo_url), git::Error);
}

TEST_CASE("Repository: list_remote_names(), add_remote()", "[Repository]")
{
    const auto folder = unit_test_folder() / "list_remote_names";

    constexpr auto* repo_url
        = "https://gitlab.desy.de/jannik.woehnert/taskolib_remote_test.git";

    Repository repo{ folder };

    // Repo list must be empty
    auto remotes = repo.list_remote_names();
    REQUIRE(remotes.empty());

    // Add a remote
    auto remote = repo.add_remote("origin", repo_url);
    REQUIRE(remote.get() != nullptr);
    REQUIRE(remote.get_name() == "origin");
    REQUIRE(remote.get_url() == repo_url);

    // Repo list must not be empty anymore
    remotes = repo.list_remote_names();
    REQUIRE(remotes.size() == 1);
    REQUIRE(remotes[0] == "origin");

    // Adding the same remote again must fail
    REQUIRE_THROWS_AS(repo.add_remote("origin", repo_url), git::Error);
}

TEST_CASE("Repository: push()", "[Repository]")
{
    const auto working_dir = unit_test_folder() / "push_test";
    const auto remote_repo = unit_test_folder() / "push_test_remote";

    std::filesystem::remove_all(working_dir);
    std::filesystem::remove_all(remote_repo);

    // Create a local repository
    Repository repo{ working_dir };

    // Commit a single file
    std::ofstream f(working_dir / "test.txt");
    f << "push() test\n";
    f.close();

    repo.add();
    repo.commit("Add test.txt");

    // Create a bare remote repository
    repository_init(remote_repo, true);

    // Add the remote to the local repository
    auto remote = repo.add_remote(
        "origin", "file://" + std::filesystem::absolute(remote_repo).string());

    // The remote must still be empty
    auto refs = remote.list_references();
    REQUIRE(refs.empty());

    // Push the local repository to the remote
    repo.push(remote);

    // The remote must now contain the main branch "refs/heads/main". Additionally, it
    // probably contains a reference for "HEAD".
    refs = remote.list_references();
    REQUIRE(refs.size() >= 1);
    REQUIRE(std::find(refs.begin(), refs.end(), "refs/heads/main"s) != refs.end());
}

TEST_CASE("Repository: checkout new branch", "[Repository]")
{
    // create test files

    // create repository

    // create new branch

    // check reference shortname

    // create new test files

    // check last commit

    // checkout to original branch

    // check last commit
    
}

/**
 * To test a remote repository, the following steps are executed
 * 1) Create a Repository with a link to a remote repository
 * 2) Commit and push files to remote repository (2x)
 * 3) Reset local repo to first commit and pull 2nd commit from remote
 * 4) Delete local repository and clone from remote
 *
 * Tidy up) Reset remote, delete all local files

TEST_CASE("Repository Wrapper Test Remote", "[GitWrapper]")
{


    SECTION("Init empty Repository with remote connection")
    {
        std::filesystem::remove_all(reporoot);

        Repository gl{ reporoot, "https://gitlab.desy.de/jannik.woehnert/taskolib_remote_test.git" };

        REQUIRE(not gl.get_path().empty());
        REQUIRE(gl.get_path() == reporoot);
        LibGitReference ref{ repository_head(gl.get_repo()) };
        REQUIRE(ref.get() != nullptr);


        // Test if repo_ got initialized
        REQUIRE(gl.get_last_commit_message() == "Initial commit");

    }

    SECTION("make commit and push to remote repository")
    {
        Repository gl{ reporoot, "https://gitlab.desy.de/jannik.woehnert/taskolib_remote_test.git" };

        create_testfiles("unit_test_1", 2, "commit and push");

        gl.add();
        gl.commit("Test push");


        auto stats = gl.status();
        // every file in unit_test_2 should have the tag deleted
        REQUIRE(stats.size() != 0);
        for(const auto& elm: stats)
        {
            if (gul14::starts_with(elm.path_name, "unit_test_1"))
            {
                REQUIRE(elm.handling == "unchanged");
                REQUIRE(elm.changes == "unchanged");
            }
        }

        // check if remote and local repo are not in the same state
        REQUIRE( ! gl.branch_up_to_date("master"));

        gl.push();

        // check if remote and local repo are in same state
        REQUIRE( gl.branch_up_to_date("master"));

    }

    SECTION("Reset repository to former commit and pull current commit from remote repository")
    {
        Repository gl{ reporoot, "https://gitlab.desy.de/jannik.woehnert/taskolib_remote_test.git" };

        //second commit
        create_testfiles("unit_test_2", 1, "commit and push 2");
        gl.add();
        gl.commit("Second commit");
        gl.push();

        REQUIRE( gl.branch_up_to_date("master"));

        // reset local repository
        gl.reset(1);

        REQUIRE_FALSE( gl.branch_up_to_date("master"));

        gl.pull();

        REQUIRE( gl.branch_up_to_date("master"));
    }


    SECTION("Clone from remote connection")
    {
        // Delete local repository
        std::filesystem::remove_all(reporoot);


        //clone reposiotry from remote
        Repository gl{ reporoot };
        gl.clone_repo("https://gitlab.desy.de/jannik.woehnert/taskolib_remote_test.git", reporoot);

        REQUIRE(gl.get_last_commit_message() == "Second commit");
    }


}
*/

// vi:ts=4:sw=4:sts=4:et
