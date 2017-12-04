# Ibnet: InfiniBand core subsystem and Ibdxnet interface for DXRAM

This project uses the ibverbs library to create a core InfiniBand subsystem
in C++ as well as the JNI library Ibdxnet which is used as a transport by 
[DXNet](https://github.com/hhu-bsinfo/dxnet) an event-driven message
passing network subsystem. 

# Setup
You need hardware that supports the ibverbs library. How to setup your hardware
is not part of this project. However, I took a few notes for myself that might
help you [here](doc/Setup.md).

# Build instructions
This project supports Linux, only, and uses the ibverbs library to access 
InfiniBand hardware. CMake scripts are used to generate build scripts 
(e.g. makefile). Just run the *build.sh* script which will execute the
steps required to compile everything.

The output files are located in *build/cmake* in their dedicated subdirs,
e.g. *build/cmake/JNIIbdxnet/libJNIIbdxnet.so*.

# Run instructions
To use Ibdxnet with DXNet (or DXRAM), follow the instructions in the
respective repositories.
