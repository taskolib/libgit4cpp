/**
 * \file   Error.cc
 * \author Fini Jastrow
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

const std::error_category& git_category()
{
  static detail::git_category_impl instance{ };
  return instance;
}

namespace detail {

const char* git_category_impl::name() const noexcept
{
    return "git";
}

std::string git_category_impl::message(int ev) const
{
    switch (static_cast<git_error_code>(ev)) {
        case git_error_code::GIT_OK: return "GIT_OK";
        case git_error_code::GIT_ERROR: return "GIT_ERROR";
        case git_error_code::GIT_ENOTFOUND: return "GIT_ENOTFOUND";
        case git_error_code::GIT_EEXISTS: return "GIT_EEXISTS";
        case git_error_code::GIT_EAMBIGUOUS: return "GIT_EAMBIGUOUS";
        case git_error_code::GIT_EBUFS: return "GIT_EBUFS";

        /**
         * GIT_EUSER is a special error that is never generated by libgit2
         * code.  You can return it from a callback (e.g to stop an iteration)
         * to know that it was generated by the callback and not by libgit2.
         */
        case git_error_code::GIT_EUSER: return "GIT_EUSER";

        case git_error_code::GIT_EBAREREPO: return "GIT_EBAREREPO";
        case git_error_code::GIT_EUNBORNBRANCH: return "GIT_EUNBORNBRANCH";
        case git_error_code::GIT_EUNMERGED: return "GIT_EUNMERGED";
        case git_error_code::GIT_ENONFASTFORWARD: return "GIT_ENONFASTFORWARD";
        case git_error_code::GIT_EINVALIDSPEC: return "GIT_EINVALIDSPEC";
        case git_error_code::GIT_ECONFLICT: return "GIT_ECONFLICT";
        case git_error_code::GIT_ELOCKED: return "GIT_ELOCKED";
        case git_error_code::GIT_EMODIFIED: return "GIT_EMODIFIED";
        case git_error_code::GIT_EAUTH: return "GIT_EAUTH";
        case git_error_code::GIT_ECERTIFICATE: return "GIT_ECERTIFICATE";
        case git_error_code::GIT_EAPPLIED: return "GIT_EAPPLIED";
        case git_error_code::GIT_EPEEL: return "GIT_EPEEL";
        case git_error_code::GIT_EEOF: return "GIT_EEOF";
        case git_error_code::GIT_EINVALID: return "GIT_EINVALID";
        case git_error_code::GIT_EUNCOMMITTED: return "GIT_EUNCOMMITTED";
        case git_error_code::GIT_EDIRECTORY: return "GIT_EDIRECTORY";
        case git_error_code::GIT_EMERGECONFLICT: return "GIT_EMERGECONFLICT";
        case git_error_code::GIT_PASSTHROUGH: return "GIT_PASSTHROUGH";
        case git_error_code::GIT_ITEROVER: return "GIT_ITEROVER";
        case git_error_code::GIT_RETRY: return "GIT_RETRY";
        case git_error_code::GIT_EMISMATCH: return "GIT_EMISMATCH";
        case git_error_code::GIT_EINDEXDIRTY: return "GIT_EINDEXDIRTY";
        case git_error_code::GIT_EAPPLYFAIL: return "GIT_EAPPLYFAIL";
#ifndef ANCIENT_LIBGIT2
        case git_error_code::GIT_EOWNER: return "GIT_EOWNER";
        case git_error_code::GIT_TIMEOUT: return "GIT_TIMEOUT";
#endif
        default: return "unknown GIT error";
    }
}

} // namespace detail

} // namespace git

std::error_code make_error_code(git_error_code e)
{
    return std::error_code{ static_cast<int>(e), git::git_category() };
}
