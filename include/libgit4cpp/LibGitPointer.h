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
     * The managed pointer is null.
     */
    LibGitPointer() = default;

    /// Construct a LibGitPointer from an "owning" C pointer from libgit2.
    LibGitPointer(T* val)
        : val_{val}
    {}

    /// Move a C-pointer from an existing LibGitPointer to a new one.
    LibGitPointer(LibGitPointer&& lg)
        : val_{std::exchange(lg.val_, nullptr)}
    {}

    /// Destruct the Object by freeing the C-type pointer with a libgit function.
    ~LibGitPointer()
    {
        detail::free_libgit_ptr(val_);
    }

    /// Move ownership of a pointer from another object to this object.
    LibGitPointer& operator=(LibGitPointer&& lg) noexcept
    {
        val_ = std::exchange(lg.val_, nullptr);
        return *this;
    }

    /// Copying is disabled to prevent double ownership of the managed pointer.
    LibGitPointer(const LibGitPointer&) = delete;

    /// Copying is disabled to prevent double ownership of the managed pointer.
    LibGitPointer& operator=(const LibGitPointer&) = delete;

    /// Free the managed pointer and reset it to null.
    void reset()
    {
        detail::free_libgit_ptr(val_);
        val_ = nullptr;
    }

    /**
     * Return a non-owning pointer to the managed object (or nullptr if no object is
     * managed).
     */
    T* get()
    {
        return val_;
    }

    /**
     * Return a constant non-owning pointer to the managed object (or nullptr if no object
     * is managed).
     */
    const T* get() const
    {
        return val_;
    }

private:
    T *val_ = nullptr;
};

} // namespace git

#endif
