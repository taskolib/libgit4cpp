/**
 * \file   Error.cc
 * \author Lars Fröhlich, Jannik Wöhnert
 * \date   Created on June 7, 2023
 * \brief  Definition of the Error exception class.
 *
 * \copyright Copyright 2023 Deutsches Elektronen-Synchrotron (DESY), Hamburg
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

#include "libgit4cpp/Error.h"

namespace git {

const char* git_category_impl::name() const noexcept
{
    return "git";
}

std::string git_category_impl::message(int ev) const
{
    switch (ev) {
        // Error::GIT_OK: return "GIT_OK";
        case Error::GIT_ERROR: return "GIT_ERROR";
        case Error::GIT_ENOTFOUND: return "GIT_ENOTFOUND";
        case Error::GIT_EEXISTS: return "GIT_EEXISTS";
        case Error::GIT_EAMBIGUOUS: return "GIT_EAMBIGUOUS";
        case Error::GIT_EBUFS: return "GIT_EBUFS";
        case Error::GIT_EUSER: return "GIT_EUSER";
        case Error::GIT_EBAREREPO: return "GIT_EBAREREPO";
        case Error::GIT_EUNBORNBRANCH: return "GIT_EUNBORNBRANCH";
        case Error::GIT_EUNMERGED: return "GIT_EUNMERGED";
        case Error::GIT_ENONFASTFORWARD: return "GIT_ENONFASTFORWARD";
        case Error::GIT_EINVALIDSPEC: return "GIT_EINVALIDSPEC";
        case Error::GIT_ECONFLICT: return "GIT_ECONFLICT";
        case Error::GIT_ELOCKED: return "GIT_ELOCKED";
        case Error::GIT_EMODIFIED: return "GIT_EMODIFIED";
        case Error::GIT_EAUTH: return "GIT_EAUTH";
        case Error::GIT_ECERTIFICATE: return "GIT_ECERTIFICATE";
        case Error::GIT_EAPPLIED: return "GIT_EAPPLIED";
        case Error::GIT_EPEEL: return "GIT_EPEEL";
        case Error::GIT_EEOF: return "GIT_EEOF";
        case Error::GIT_EINVALID: return "GIT_EINVALID";
        case Error::GIT_EUNCOMMITTED: return "GIT_EUNCOMMITTED";
        case Error::GIT_EDIRECTORY: return "GIT_EDIRECTORY";
        case Error::GIT_EMERGECONFLICT: return "GIT_EMERGECONFLICT";
        case Error::GIT_PASSTHROUGH: return "GIT_PASSTHROUGH";
        case Error::GIT_ITEROVER: return "GIT_ITEROVER";
        case Error::GIT_RETRY: return "GIT_RETRY";
        case Error::GIT_EMISMATCH: return "GIT_EMISMATCH";
        case Error::GIT_EINDEXDIRTY: return "GIT_EINDEXDIRTY";
        case Error::GIT_EAPPLYFAIL: return "GIT_EAPPLYFAIL";
        case Error::GIT_EOWNER: return "GIT_EOWNER";
        case Error::GIT_TIMEOUT: return "GIT_TIMEOUT";
        default: return "unknown GIT error";
    }
}

} // namespace git
