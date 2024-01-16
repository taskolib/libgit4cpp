/**
 * \file   credentials_callback.cc
 * \author Lars Fröhlich
 * \date   Created on January 16, 2024
 * \brief  Implementation of the credentials callback function.
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

#include <git2.h>

namespace git {
extern "C" {

int credentials_callback(git_cred** out_credentials, const char* /*url*/,
    const char* /*username_from_url*/, unsigned int /*allowed_types*/, void* /*payload*/)
{
    // Dummy implementation: Always return the same username and password
    return git_cred_userpass_plaintext_new(out_credentials, "user", "pwd");
}

} // extern "C"
} // namespace git
