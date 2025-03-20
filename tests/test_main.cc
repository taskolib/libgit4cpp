/**
 * \file   test_main.cc
 * \author Lars Fröhlich
 * \date   Created on November 26, 2019
 * \brief  Test suite for libgit4cpp
 *
 * \copyright Copyright 2019-2025 Deutsches Elektronen-Synchrotron (DESY), Hamburg
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

#include <catch2/catch_session.hpp>

#include "test_main.h"

std::filesystem::path unit_test_folder()
{
    return "unit_test_files";
}

int main(int argc, char* argv[])
{
    std::filesystem::remove_all(unit_test_folder());
    return Catch::Session().run(argc, argv);
}
