# FastKV

[![CI](https://github.com/IsokGta7/FastKV/actions/workflows/ci.yml/badge.svg)](https://github.com/IsokGta7/FastKV/actions/workflows/ci.yml)

FastKV is a small, thread-safe, in-memory key-value store written in C++20. It is a
library (`fastkv`) with a string-to-string map behind a sharded locking scheme, a test
suite that runs under AddressSanitizer, UndefinedBehaviorSanitizer and ThreadSanitizer,
and a Google Benchmark target that measures how the shard count affects throughput
under concurrent load.

It is a learning and portfolio project. There is no network server and no persistence
yet; see [Roadmap](#roadmap).

## Design

The store is split into a fixed number of shards (16 by default). Each shard is an
`std::unordered_map<std::string, std::string>` guarded by its own `std::shared_mutex`:

- `get` takes a shared lock on one shard, so any number of readers proceed in parallel.
- `put` and `erase` take an exclusive lock on one shard, so a writer only blocks
  operations whose keys hash to the same shard.

A key's shard is chosen from `std::hash<std::string>` after a splitmix64 mixing step.
The mixing matters: each shard's map hashes the key again with the same function, and
standard libraries that index power-of-two bucket arrays by the low bits of the hash
(MSVC's does) would otherwise see every key in a shard share those bits and use only a
fraction of the buckets. Before the fix, 16 shards were about three times slower than one
shard on a single thread under MSVC.

Shards are aligned to 64 bytes so neighbouring locks do not share a cache line.

## API

```cpp
#include "fastkv/fastkv.hpp"

fastkv::FastKV kv;          // 16 shards
fastkv::FastKV single(1);   // one shard: a single map behind one shared_mutex

kv.put("user:1", "isok");                       // insert or overwrite
std::optional<std::string> v = kv.get("user:1"); // "isok"
kv.get("missing");                              // std::nullopt
kv.put("empty", "");
kv.get("empty");                                // "" (present, empty), not nullopt
bool removed = kv.erase("user:1");              // true; false if the key was absent
kv.shard_count();                               // 16
```

`FastKV(0)` throws `std::invalid_argument`. The store is neither copyable nor movable.
All member functions are safe to call concurrently from any number of threads. `get`
returns a copy of the value, so the result stays valid after the lock is released.

## Building

Requirements: CMake 3.25 or newer, a C++20 compiler, and
[vcpkg](https://github.com/microsoft/vcpkg) for GoogleTest and Google Benchmark.
`vcpkg.json` lists the dependencies and pins their versions with `builtin-baseline`, so
CMake installs them automatically through the vcpkg toolchain file (manifest mode).

Linux and macOS:

```bash
git clone https://github.com/microsoft/vcpkg.git ~/vcpkg
~/vcpkg/bootstrap-vcpkg.sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release \
      -DCMAKE_TOOLCHAIN_FILE=$HOME/vcpkg/scripts/buildsystems/vcpkg.cmake
cmake --build build --parallel
```

Windows (Visual Studio 2022 or Build Tools, from a Developer PowerShell):

```powershell
git clone https://github.com/microsoft/vcpkg.git C:\vcpkg
C:\vcpkg\bootstrap-vcpkg.bat
cmake -S . -B build -G "Visual Studio 17 2022" -A x64 `
      -DCMAKE_TOOLCHAIN_FILE=C:/vcpkg/scripts/buildsystems/vcpkg.cmake
cmake --build build --config Release --parallel
```

The vcpkg copy bundled with Visual Studio (`%VCINSTALLDIR%\vcpkg`) also works, because
the manifest names a baseline.

CI builds and tests on Ubuntu with GCC and on Windows with MSVC. macOS is not covered by
CI; the code uses only the standard library, but that build has not been verified.

## Tests

```bash
ctest --test-dir build --output-on-failure            # add -C Release on Windows
```

The suite covers put, get, erase, overwrite, absent keys versus empty values, identical
behaviour across shard counts, and two multi-threaded cases (disjoint writers with
concurrent readers, and threads racing put and erase on the same keys).

Sanitizer builds (GCC or Clang):

```bash
cmake -S . -B build-asan -DFASTKV_SANITIZE=address,undefined \
      -DCMAKE_TOOLCHAIN_FILE=$HOME/vcpkg/scripts/buildsystems/vcpkg.cmake
cmake -S . -B build-tsan -DFASTKV_SANITIZE=thread \
      -DCMAKE_TOOLCHAIN_FILE=$HOME/vcpkg/scripts/buildsystems/vcpkg.cmake
```

On kernels with high mmap randomisation (Ubuntu 24.04, for example) GCC 13's
ThreadSanitizer aborts with "unexpected memory mapping"; run
`sudo sysctl vm.mmap_rnd_bits=28` first, or wrap the build and test commands in
`setarch $(uname -m) -R`.

## Benchmark

```bash
./build/fastkv_bench                       # build/Release/fastkv_bench.exe on Windows
./build/fastkv_bench --benchmark_filter='threads:8'
```

`BM_MixedWorkload` preloads 65,536 keys into one shared store and has 1, 2, 4 or 8
threads issue uniformly random operations, 10% or 50% of them writes, against a store
with 1 or 16 shards.

Measured on an Intel Core Ultra 7 155H laptop (16 cores, 22 threads), Windows 11, MSVC
19.44 (Visual Studio Build Tools 2022 17.14), Release build, median of 5 repetitions of
at least 1 s each. Figures are total operations per second across all threads. Run to
run variation on this machine was up to about 30%, so read them as shape, not as exact
values.

| Writes | Threads | 1 shard | 16 shards |
|-------:|--------:|--------:|----------:|
| 10%    | 1       | 6.7 M/s | 7.3 M/s   |
| 10%    | 2       | 6.5 M/s | 9.8 M/s   |
| 10%    | 4       | 4.8 M/s | 11.5 M/s  |
| 10%    | 8       | 1.7 M/s | 15.4 M/s  |
| 50%    | 1       | 5.1 M/s | 6.6 M/s   |
| 50%    | 2       | 5.9 M/s | 7.3 M/s   |
| 50%    | 4       | 5.5 M/s | 7.7 M/s   |
| 50%    | 8       | 3.1 M/s | 8.7 M/s   |

With one shard, adding threads lowers total throughput: every operation contends for the
same lock, even when only 10% of them write. With 16 shards, throughput rises with the
thread count. Write-heavy load scales less, because writers still exclude everything else
on their shard.

## Roadmap

None of the following is implemented.

- A TCP server built on Asio, speaking a small text protocol (`SET`, `GET`, `DEL`).
- Persistence through an append-only log replayed at startup.
- Running clang-tidy in CI (the `.clang-tidy` and `.clang-format` files are in the
  repository but are not enforced yet).

## Layout

```
include/fastkv/fastkv.hpp   public header
src/fastkv.cpp              implementation
src/main.cpp                tiny demo executable (fastkv_main)
tests/test_fastkv.cpp       GoogleTest suite (fastkv_tests)
benchmarks/bench_fastkv.cpp Google Benchmark target (fastkv_bench)
```
