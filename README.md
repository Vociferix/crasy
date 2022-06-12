# Crasy

Crasy - CoRoutine ASYnc

Crasy is a C++ async I/O library built on top of ASIO and based around
C++20 coroutines. Additionally, Crasy is inspired by Rust's popular
Tokio library. The intent is to enable developers to create "web scale"
C++ applications while writing straight-line code.

Currently, Crasy is an all-in-one library that includes common async
interfaces as well as a non-swappable runtime. The runtime runs async
tasks in a thread pool, and supports executing blocking tasks on a
separate thread pool.

See the `examples` directory for simple example applications.

[API Documentation](https://vociferix.github.io/docs/crasy/)

## Compiler Support

At this stage, GCC 10.3 is the only compiler that this library has been
built under, but it should support any version of GCC from 10 forward.
Clang may work if built with libc++ instead of libstdc++, but Clang's
coroutine support is incomplete at the time of writing, so there are
no guarantees. Windows builds have not yet been attempted, but it should
theoretically support MSVC 19.28 (VS 2019 16.8) or later.

Feel free to open issues for various compilers, but note that Clang will
not be officially supported until it has a complete coroutine
implementation. ICE reports are welcome for informational purposes, but
please also report ICEs to compiler developers.
