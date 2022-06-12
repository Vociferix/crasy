# Crasy

Crasy is a C++ async I/O library built on top of ASIO and based around
C++20 coroutines. Additionally, Crasy is inspired by Rust's popular
Tokio library. The intent is to enable developers to create "web scale"
C++ applications while writing straight-line code.

Currently, Crasy is an all-in-one library that includes common async
interfaces as well as a non-swappable runtime. The runtime runs async
tasks in a thread pool, and supports executing blocking tasks on a
separate thread pool.

See the `examples` directory for simple example applications.
