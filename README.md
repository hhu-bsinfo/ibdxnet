# Ibdxnet: InfiniBand subsystem and interface for DXNet/DXRAM

This project uses the ibverbs library to create a core InfiniBand subsystem in C++ as well as the JNI library Ibdxnet
which is used as a transport by [DXNet](https://github.com/hhu-bsinfo/dxnet) an event-driven messaging network
subsystem.

# Setup
You need hardware that supports the ibverbs library. How to setup your hardware is out of the scope of this document.
However, I took a few notes for myself that might help you as well [here](doc/Setup.md).

# Build instructions
This project supports Linux, only, and uses the ibverbs library to access InfiniBand hardware. CMake scripts are used
to generate build scripts (e.g. makefile). Just run the *build.sh* script which will execute the steps required to
compile everything.

The output files are located in *build/cmake* in their dedicated subdirs, e.g.
*build/cmake/JNIIbdxnet/libJNIIbdxnet.so*.

# Run instructions

## DXNet/DXRAM
To use Ibdxnet with [DXNet](https://github.com/hhu-bsinfo/dxnet) (or [DXRAM](https://github.com/hhu-bsinfo/dxram)),
follow the instructions in the respective repositories.

## MsgrcLoopback
This is a standalone application to test and benchmark the messaging engine which uses reliable verbs. Execute it
with the parameter *--help* to get a list of parameters with descriptions.

A simple example to run a full bi-directional test with two nodes (node65 and node66):
On node65:
```
./MsgrcLoopback -n 0 -c node65,node66 -d 1 -u 1000
```
On node66:
```
./MsgrcLoopback -n 1 -c node65,node66 -d 0 -u 1000
```

# Benchmark notes
When running benchmarks with Ibdxnet, ensure you compile with the log trace level removed (IBNET_LOG_TRACE_STRIP) and
statistics disabled (IBNET_DISABLE_STATISTICS). Both are heavily used for debugging and profiling. Disable both to get
the best performance possible.