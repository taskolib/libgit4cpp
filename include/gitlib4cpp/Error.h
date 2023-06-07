/**
 * \file   Error.h
 * \author Lars Froehlich, Jannik Woehnert
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

#include <stdexcept>

namespace git {

/**
 * An exception class carrying an error message.
 *
 * Error is used as the standard exception class throughout gitlib4cpp.
 * It can be used directly or inherited from.
 *
 * \code
 * try
 * {
 *     throw git::Error("An error has occurred");
 * }
 * catch (const git::Error& e)
 * {
 *     std::cerr << e.what() << "\n";
 * }
 * \endcode
 *
 * \note
 * git::Error is derived from std::runtime_error. It can therefore be caught by
 * `catch (const std::exception&)`, `catch (const std::runtime_error&)`, and
 * `catch (const git::Error&)`.
 */
class Error : public std::runtime_error
{
public:
    using std::runtime_error::runtime_error;
};

} // namespace git

#endif
