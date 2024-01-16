/**
 * \file   credentials_callback.h
 * \author Lars Fröhlich
 * \date   Created on January 16, 2024
 * \brief  Declaration of the credentials callback function.
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

#ifndef LIBGIT4CPP_CREDENTIALS_CALLBACK_H_
#define LIBGIT4CPP_CREDENTIALS_CALLBACK_H_

namespace git {
extern "C" {

/**
 * Credentials callback function for libgit2.
 *
 * This function is called by libgit2 when it needs credentials to access a remote, e.g.
 * over an HTTPS connection. Currently, this is just a dummy implementation that always
 * returns the same username and password.
 */
int credentials_callback(git_cred** out_credentials, const char* /*url*/,
    const char* /*username_from_url*/, unsigned int /*allowed_types*/, void* /*payload*/);

} // extern "C"
} // namespace git

#endif
