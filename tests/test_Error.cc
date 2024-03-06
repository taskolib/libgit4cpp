/**
 * \file   test_Error.cc
 * \date   Created on January 23, 2024
 * \brief  Test suite for the git::Error exception class.
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

#include <string>

#include <gul14/catch.h>

#include "libgit4cpp/Error.h"

using namespace std::literals;

TEST_CASE("Error: Constructor", "[exceptions]")
{
    // Default error code is -7 == GIT_EUSER
    git::Error e("Test");
    REQUIRE(e.what() == "Test: GIT_EUSER"s);
    REQUIRE(e.code().value() == git::git_error_code::GIT_EUSER);
    REQUIRE(e.code().value() == -7);
}

TEST_CASE("Error: Copy constructor", "[exceptions]")
{
    const git::Error e("Test");
    git::Error e2(e);

    REQUIRE(e.what() == std::string(e2.what()));
}

TEST_CASE("Error: Copy assignment", "[exceptions]")
{
    const git::Error e(1, "Test");
    git::Error e2(2, "Test2");

    e2 = e;

    REQUIRE(e2.what() == "Test: unknown GIT error"s);
}
