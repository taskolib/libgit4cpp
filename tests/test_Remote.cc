/**
 * \file   test_Remote.cc
 * \author Lars Fröhlich
 * \date   Created on January 15, 2024
 * \brief  Test suite for the Remote class.
 *
 * \copyright Copyright 2024 Deutsches Elektronen-Synchrotron (DESY), Hamburg
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

#include <git2.h>
#include <gul14/catch.h>
#include <gul14/gul.h>

#include "libgit4cpp/Error.h"
#include "libgit4cpp/GitRepository.h"
#include "libgit4cpp/Remote.h"
#include "libgit4cpp/wrapper_functions.h"
#include "test_main.h"

using namespace git;
using namespace std::literals;
using gul14::cat;

TEST_CASE("Remote: Constructor", "[Remote]")
{
    const auto reporoot = unit_test_folder() / "Remote";

    std::filesystem::remove_all(reporoot);

    const std::string repo_url{
        "https://gitlab.desy.de/jannik.woehnert/taskolib_remote_test.git" };

    GitRepository repo{ reporoot };

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
