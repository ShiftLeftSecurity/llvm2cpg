# llvm2cpg

This project is intended to produce the code property graph out of a LLVM Bitcode files.

## Build

Requirements for the build:

 - clang
 - libzip-dev
 - libtinfo-dev
 - LLVM
 - cmake
 - ninja-build (optional, but recommended)

So far, it was only tested on Ubuntu against LLVM 8.
Recommended way to install LLVM is to download a precompiled binary from [LLVM Releases page](http://releases.llvm.org).
Example:

```
sudo mkdir /opt/llvm
sudo chown -R `whoami`:`whoami` /opt/llvm
cd /opt/llvm
wget http://releases.llvm.org/8.0.0/clang+llvm-8.0.0-x86_64-linux-gnu-ubuntu-18.04.tar.xz
tar xf clang+llvm-8.0.0-x86_64-linux-gnu-ubuntu-18.04.tar.xz
mv clang+llvm-8.0.0-x86_64-linux-gnu-ubuntu-18.04/ 8.0.0
```

Prepare build system:

```
git clone git@github.com:ShiftLeftSecurity/llvm2cpg.git --recursive
mkdir build
cd build
cmake -G Ninja \
    -DCMAKE_C_COMPILER=clang \
    -DCMAKE_CXX_COMPILER=clang++ \
    -DPATH_TO_LLVM=/opt/llvm/8.0.0 \
    ../llvm2cpg
```

#### Build and run tests:

```
ninja llvm2cpg-tests
./tests/CPGTests/llvm2cpg-tests
```

#### Build and run `cpg-proto-writer`

```
ninja cpg-proto-writer
./tools/cpg-proto-writer/cpg-proto-writer ./tests/fixtures/hello_world/hello_world.bc
```

You should see the following output:

```
Processing ./tests/fixtures/hello_world/hello_world.bc
CPG is successfully save on disk: ./cpg.bin.zip
```

Getting help from `cpg-proto-writer`:

```
./tools/cpg-proto-writer/cpg-proto-writer --help
```
