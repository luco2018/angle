//
// Copyright 2017 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// MemoryProgramCache: Stores compiled and linked programs in memory so they don't
//   always have to be re-compiled. Can be used in conjunction with the platform
//   layer to warm up the cache from disk.

#ifndef LIBANGLE_MEMORY_PROGRAM_CACHE_H_
#define LIBANGLE_MEMORY_PROGRAM_CACHE_H_

#include <array>

#include "common/MemoryBuffer.h"
#include "libANGLE/Error.h"
#include "libANGLE/SizedMRUCache.h"

namespace gl
{
// 160-bit SHA-1 hash key.
using ProgramHash = std::array<uint8_t, 20>;
}  // namespace gl

namespace std
{
template <>
struct hash<gl::ProgramHash>
{
    // Simple routine to hash four ints.
    size_t operator()(const gl::ProgramHash &programHash) const
    {
        unsigned int hash = 0;
        for (uint32_t num : programHash)
        {
            hash *= 37;
            hash += num;
        }
        return hash;
    }
};
}  // namespace std

namespace gl
{
class Context;
class InfoLog;
class Program;
class ProgramState;

class MemoryProgramCache final : angle::NonCopyable
{
  public:
    MemoryProgramCache(size_t maxCacheSizeBytes);
    ~MemoryProgramCache();

    // Writes a program's binary to the output memory buffer.
    static void Serialize(const Context *context,
                          const Program *program,
                          angle::MemoryBuffer *binaryOut);

    // Loads program state according to the specified binary blob.
    static LinkResult Deserialize(const Context *context,
                                  const Program *program,
                                  ProgramState *state,
                                  const uint8_t *binary,
                                  size_t length,
                                  InfoLog &infoLog);

    static void ComputeHash(const Context *context, const Program *program, ProgramHash *hashOut);

    // Check if the cache contains a binary matching the specified program.
    bool get(const ProgramHash &programHash, const angle::MemoryBuffer **programOut);

    // Evict a program from the binary cache.
    void remove(const ProgramHash &programHash);

    // Helper method that serializes a program.
    void putProgram(const ProgramHash &programHash, const Context *context, const Program *program);

    // Helper method that copies a user binary.
    void putBinary(const Context *context,
                   const Program *program,
                   const uint8_t *binary,
                   size_t length);

    // Check the cache, and deserialize and load the program if found. Evict existing hash if load
    // fails.
    LinkResult getProgram(const Context *context,
                          const Program *program,
                          ProgramState *state,
                          ProgramHash *hashOut);

    // Empty the cache.
    void clear();

  private:
    // Insert or update a binary program. Program contents are transferred.
    void put(const ProgramHash &programHash,
             const Context *context,
             angle::MemoryBuffer &&binaryProgram);

    angle::SizedMRUCache<ProgramHash, angle::MemoryBuffer> mProgramBinaryCache;
    unsigned int mIssuedWarnings;
};

}  // namespace gl

#endif  // LIBANGLE_MEMORY_PROGRAM_CACHE_H_
