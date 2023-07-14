/**
 * \file   test_Error.cc
 * \author Lars Fröhlich
 * \date   Created on December 10, 2021
 * \brief  Test suite for the Error exception class.
 *
 * \copyright Copyright 2021-2023 Deutsches Elektronen-Synchrotron (DESY), Hamburg
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
using namespace task;

TEST_CASE("Error: Constructor", "[exceptions]")
{
    Error e("Test");
    REQUIRE(e.what() == "Test"s);
}

TEST_CASE("Error: Copy constructor", "[exceptions]")
{
    const Error e("Test");
    Error e2(e);

    REQUIRE(e.what() == std::string(e2.what()));
}

TEST_CASE("Error: Copy assignment", "[exceptions]")
{
    const Error e("Test", 1);
    Error e2("Test2", 2);

    e2 = e;

    REQUIRE(e2.what() == "Test"s);
}

TEST_CASE("Error: operator==", "[exceptions]")
{
    REQUIRE(Error("Test") == Error("Test"));
    REQUIRE(Error("") == Error(""));

    REQUIRE_FALSE(Error("test") == Error("TEST"));
}

TEST_CASE("Error: operator!=", "[exceptions]")
{
    REQUIRE_FALSE(Error("Test") != Error("Test"));
    REQUIRE_FALSE(Error("") != Error(""));

    REQUIRE(Error("test") != Error("TEST"));
}
