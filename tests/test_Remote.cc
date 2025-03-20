/**
 * \file   test_Remote.cc
 * \author Lars Fröhlich
 * \date   Created on January 15, 2024
 * \brief  Test suite for the Remote class.
 *
 * \copyright Copyright 2024-2025 Deutsches Elektronen-Synchrotron (DESY), Hamburg
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

#include <catch2/catch_test_macros.hpp>
#include <git2.h>
#include <gul14/gul.h>

#include "libgit4cpp/Error.h"
#include "libgit4cpp/Repository.h"
#include "libgit4cpp/Remote.h"
#include "libgit4cpp/wrapper_functions.h"
#include "test_main.h"

using namespace git;
using namespace std::literals;
using gul14::cat;

static const auto working_dir = unit_test_folder() / "Remote_list_references";
static const auto remote_repo = unit_test_folder() / "Remote_list_references.remote";

TEST_CASE("Remote: Constructor", "[Remote]")
{
    const auto reporoot = unit_test_folder() / "Remote";

    const std::string repo_url{
        "https://gitlab.desy.de/jannik.woehnert/taskolib_remote_test.git" };

    Repository repo{ reporoot };

    auto remote_ptr = remote_create(repo.get_repo(), "origin", repo_url);
    REQUIRE(remote_ptr != nullptr);

    Remote remote{ std::move(remote_ptr) };

    REQUIRE(remote.get_name() == "origin"s);
    REQUIRE(remote.get_url() == repo_url);

    auto* re_ptr = remote.get();
    REQUIRE(re_ptr != nullptr);
    REQUIRE(git_remote_name(re_ptr) == "origin"s);
    REQUIRE(git_remote_url(re_ptr) == repo_url);
}

TEST_CASE("Remote: list_references()", "[Remote]")
{
    // Create a local repository and commit a single file
    auto repo = std::make_unique<Repository>(working_dir);

    std::ofstream f(working_dir / "test.txt");
    f << "Remote::list_references() test\n";
    f.close();

    repo->add();
    repo->commit("Add test.txt");

    // Create a bare remote repository
    repository_init(remote_repo, true);

    // Add the remote to the local repository
    auto remote = repo->add_remote(
        "origin", "file://" + std::filesystem::absolute(remote_repo).string());

    // The remote must still be empty
    auto refs = remote.list_references();
    REQUIRE(refs.empty());

    // Push the local repository to the remote (default: HEAD -> refs/heads/main)
    repo->push(remote);

    // The remote must now contain the main branch "refs/heads/main". Additionally, it
    // probably contains a reference for "HEAD".
    refs = remote.list_references();
    REQUIRE(refs.size() >= 1);
    REQUIRE(std::find(refs.begin(), refs.end(), "refs/heads/main"s) != refs.end());

    // Remove the "parent" repository and check if the remote still works
    repo.reset();
    refs = remote.list_references();
    REQUIRE(refs.size() >= 1);
    REQUIRE(std::find(refs.begin(), refs.end(), "refs/heads/main"s) != refs.end());

}

TEST_CASE("wrapper_functions: branch_remote_name()", "[Remote]")
{
    // Use the repo-with-a-remote from the previous test
    auto repo = std::make_unique<Repository>(working_dir);

    // The following test is modeled after the example code in Error.h class Error
    std::string name_str;
    try {
        name_str = git::branch_remote_name(repo->get_repo(), "refs/remotes/origin/main"s);
    }
    catch (const git::Error& e) {
        if (e.code().category() == git::git_category() and e.code().value() == git::git_error_code::GIT_EAMBIGUOUS) {
            REQUIRE(false); // will not happen
        }
        if (e.code() == git::git_error_code::GIT_EAMBIGUOUS) {
            REQUIRE(false); // will not happen
        }
        throw e;
    }
    REQUIRE(name_str == "origin"s);

    try {
        name_str = git::branch_remote_name(repo->get_repo(), "vanillavoid"s);
    }
    catch (const git::Error& e) {
        if (e.code() == git::git_error_code::GIT_EAMBIGUOUS) {
            REQUIRE(false); // must not happen
        }
    }
}
