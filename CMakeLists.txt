# SPDX-FileCopyrightText: 2024 Alexey Andreyev <aa13q@ya.ru>
#
# SPDX-License-Identifier: BSD-3-Clause

cmake_minimum_required(VERSION 3.16)

project(b4-le-me LANGUAGES CXX)

add_subdirectory(src)

# codestyle stuff from ECM:
find_package(ECM REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})
include(KDEClangFormat)
include(KDEGitCommitHooks)
file(GLOB_RECURSE ALL_CLANG_FORMAT_SOURCE_FILES src/*.cpp src/*.h)
kde_clang_format(${ALL_CLANG_FORMAT_SOURCE_FILES})
kde_configure_git_pre_commit_hook(CHECKS CLANG_FORMAT)
