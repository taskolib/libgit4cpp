/**
 * \file   LibGitPointer.h
 * \author Sven-Jannik Wöhnert, Lars Fröhlich
 * \date   Created on March 20, 2023
 * \brief  Definition of the LibGitPointer class template and declarations of associated
 *         functions.
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

#ifndef LIBGIT4CPP_LIBGITPOINTER_H
#define LIBGIT4CPP_LIBGITPOINTER_H

#include <string>
#include <typeinfo>
#include <utility>

#include <git2.h>

#include "libgit4cpp/Error.h"

namespace git {

namespace detail {

// An overload set of functions to free raw C pointers from libgit2.
void free_libgit_ptr(git_tree* tree);
void free_libgit_ptr(git_signature* signature);
void free_libgit_ptr(git_index* index);
void free_libgit_ptr(git_repository* repo);
void free_libgit_ptr(git_remote* remote);
void free_libgit_ptr(git_commit* commit);
void free_libgit_ptr(git_status_list* status);
void free_libgit_ptr(git_reference* reference);
void free_libgit_ptr(git_buf* buf);

} // namespace detail

/**
 * RAII wrapper for all libgit2 pointers that require a cleanup call of some free_*
 * function.
 *
 * Copy methods are excluded to prevent double ownership of the pointer.
 *
 * \tparam T: A libgit2 type like git_remote, git_tree, git_commit, ...
*/
template <class T>
class LibGitPointer
{
public:
    /**
     * Default-construct a LibGitPointer.
     * Contains a default error message.
     */
    LibGitPointer() = default;

    /// Construct a LibGitPointer from an owning raw pointer from libgit2.
    LibGitPointer(T* val)
        : val_{ val }
        , err_msg_{ "" }
    {}

    /// Construct a LibGitPointer from an error message.
    LibGitPointer(std::string err)
        : val_{ nullptr }
        , err_msg_{ std::move(err) }
    {}

    /// Move constructor
    LibGitPointer(LibGitPointer&& lg)
        : val_{ std::exchange(lg.val_, nullptr) }
        , err_msg_{ std::exchange(lg.err_msg_, default_err_) }
    {}

    /// Move assignment
    LibGitPointer& operator=(LibGitPointer&& lg) noexcept
    {
        val_ = std::exchange(lg.val_, nullptr);
        err_msg_ = std::exchange(lg.err_msg_, default_err_);
        return *this;
    }

    /// Copying is disabled to prevent double ownership of the managed pointer.
    LibGitPointer(const LibGitPointer&) = delete;

    /// Copying is disabled to prevent double ownership of the managed pointer.
    LibGitPointer& operator=(const LibGitPointer&) = delete;

    /// Destruct and free the owned libgit object.
    ~LibGitPointer()
    {
        detail::free_libgit_ptr(val_);
    }

    /// Free the managed object and reset to the empty state.
    void reset()
    {
        detail::free_libgit_ptr(val_);
        val_ = nullptr;
        err_msg_ = default_err_;
    }

    /**
     * Get a pointer to the contained managed object.
     *
     * \return A non-owning raw pointer to the managed object
     * \exception Throws a git::Error when no managed object exists
     */
    T* get()
    {
        if (not err_msg_.empty())
            throw Error{ err_msg_ };
        return val_;
    }

    /**
     * Get a (const) pointer to the contained managed object.
     *
     * \return A constant non-owning raw pointer to the managed object
     * \exception Throws a git::Error when no managed object exists
     */
    const T* get() const
    {
        if (not err_msg_.empty())
            throw Error{ err_msg_ };
        return val_;
    }

    /**
     * Return the explanation why no managed object is contained.
     *
     * \return Explanation string; or empty string if there is a managed object
     */
    std::string error() const
    {
        return err_msg_;
    }

    /**
     * Check if a managed object is contained.
     *
     * \return True if an object is contained
     */
    explicit operator bool() const
    {
        return err_msg_.empty();
    }

private:
    static constexpr const char* default_err_ = "LibGitPointer uninit'ed";
    T* val_ = nullptr;
    std::string err_msg_{ default_err_ };
};

} // namespace git

#endif
