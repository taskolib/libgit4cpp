/**
 * \file   Error.h
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

#ifndef LIBGIT4CPP_ERROR_H_
#define LIBGIT4CPP_ERROR_H_

#include <system_error>

#include <git2/errors.h>

namespace git {

using ::git_error_code; // from git2/errors.h

namespace detail {

class git_category_impl : public std::error_category {
    public:
        virtual const char* name() const noexcept;
        virtual std::string message(int ev) const;
};

} // namespace detail

const std::error_category& git_category();

/**
 * An exception class carrying an error code and message.
 *
 * Error is used as the standard exception class throughout libgit4cpp.
 * It can be used directly or inherited from.
 *
 * \code
 * try
 * {
 *     throw git::Error(errorcode, "An error has occurred");
 *     // or throw git::Error("Another error occurred");
 * }
 * catch (const git::Error& e)
 * {
 *     std::cerr << e.what() << "\n";
 * }
 * \endcode
 *
 * \code
 * try
 * {
 *     name_str = branch_remote_name(repo, branch_name);
 * }
 * catch (const git::Error& e)
 * {
 *     if (e.code().category() == git::git_category() and e.code().value() == git::git_error_code::GIT_EAMBIGUOUS)
 *         std::cerr << "The supplied branch name is ambiguous.\n";
 * }
 * \endcode
 *
 * If no error code is specified GIT_EUSER is used.
 *
 * \note
 * git::Error is derived from std::system_error. It can therefore be caught by
 * `catch (const std::exception&)`, `catch (const std::runtime_error&)`,
 * `catch (const std::system_error&)`, and `catch (const git::Error&)`.
 */
class Error : public std::system_error
{
public:
    using std::system_error::system_error;
    Error(int ev)
        : std::system_error(ev, git_category())
        { }
    Error(int ev, const std::string& what)
        : std::system_error(ev, git_category(), what)
        { }
    Error(int ev, const char* what)
        : std::system_error(ev, git_category(), what)
        { }
    Error(const std::string& what)
        : std::system_error(static_cast<int>(GIT_EUSER), git_category(), what)
        { }
    Error(const char* what)
        : std::system_error(static_cast<int>(GIT_EUSER), git_category(), what)
        { }

    /// The error category must be an unique object
    const std::error_category& git_category()
    {
        static detail::git_category_impl instance;
        return instance;
    }

};

} // namespace git

namespace std
{
    // register for implicit conversion to std::error_code
    template <>
    struct is_error_code_enum<git::git_error_code> : public true_type
    { };
}

std::error_code make_error_code(git::git_error_code e);

#endif
