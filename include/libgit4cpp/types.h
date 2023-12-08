/**
 * \file   types.h
 * \date   Created on 13 December 2023
 * \brief  Definition of owner types for raw pointers.
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

#ifndef LIBGIT4CPP_TYPES_H
#define LIBGIT4CPP_TYPES_H

#include <memory>

#include <git2.h>

namespace git {

using LibGitTree = std::unique_ptr<git_tree, void(*)(git_tree*)>;
using LibGitSignature = std::unique_ptr<git_signature, void(*)(git_signature*)>;
using LibGitIndex = std::unique_ptr<git_index, void(*)(git_index*)>;
using LibGitRepository = std::unique_ptr<git_repository, void(*)(git_repository*)>;
using LibGitRemote = std::unique_ptr<git_remote, void(*)(git_remote*)>;
using LibGitCommit = std::unique_ptr<git_commit, void(*)(git_commit*)>;
using LibGitStatusList = std::unique_ptr<git_status_list, void(*)(git_status_list*)>;
using LibGitReference = std::unique_ptr<git_reference, void(*)(git_reference*)>;
using LibGitBuf = std::unique_ptr<git_buf, void(*)(git_buf*)>;

} // namespace git

#endif // LIBGIT4CPP_TYPES_H
