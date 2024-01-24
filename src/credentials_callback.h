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

#include <git2.h>

namespace git {

using CredentialsCallback
    = int (*)(git_cred**, const char*, const char*, unsigned int, void*);

/**
 * Return a dummy credentials callback function for libgit2.
 *
 * The returned function pointer may be called by libgit2 when it needs credentials to
 * access a remote, e.g. over an HTTPS connection. Currently, this is just a dummy
 * implementation that always returns the same username and password.
 */
CredentialsCallback get_dummy_credentials_callback();

} // namespace git

#endif
