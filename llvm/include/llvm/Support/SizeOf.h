//===--- SizeOf.h - Helpers for calculating type size -----*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file defines the SizeOf and AlignOf classes.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_SUPPORT_SIZEOF_H
#define LLVM_SUPPORT_SIZEOF_H

#include <cstddef>

namespace llvm {

/// A sizeof operator stand-in which supports `sizeof(void) == 0`.
///
template <class T> struct SizeOf { static constexpr size_t value = sizeof(T); };
template <> struct SizeOf<void> { static constexpr size_t value = 0; };

/// An alignof operator stand-in which supports `alignof(void) == 1`.
///
template <class T> struct AlignOf {
  static constexpr size_t value = alignof(T);
};
template <> struct AlignOf<void> { static constexpr size_t value = 1; };

} // end namespace llvm

#endif // LLVM_SUPPORT_SIZEOF_H
