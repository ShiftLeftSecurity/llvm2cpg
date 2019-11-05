# Release Process

### MacOS

The list of required software:

 - libzip
 - protobuf
 - LLVM

Build and install `libzip`:

```
git clone https://github.com/nih-at/libzip.git
mkdir build.libzip.dir; cd build.libzip.dir
cmake \
	-DCMAKE_BUILD_TYPE=Release \
	-DBUILD_SHARED_LIBS=OFF \
	-DENABLE_BZIP2=OFF \
	../libzip/
make all -j8
sudo make install
```

Build and install `protobuf`:

```
wget https://github.com/protocolbuffers/protobuf/releases/download/v3.10.1/protobuf-all-3.10.1.zip
unzip protobuf-all-3.10.1.zip
cd protobuf-all-3.10.1
./configure --enable-static=yes --enable-shared=no
make -j8
```

Get LLVM:
```
mkdir /opt/llvm/
wget https://releases.llvm.org/9.0.0/clang+llvm-9.0.0-x86_64-darwin-apple.tar.xz
tar xzf clang+llvm-9.0.0-x86_64-darwin-apple.tar.xz
mv clang+llvm-9.0.0-x86_64-darwin-apple /opt/llvm/9.0.0
```

Build llvm2cpg:

```
git clone https://github.com/ShiftLeftSecurity/llvm2cpg.git --recursive
mkdir build.llvm2cpg.dir; cd build.llvm2cpg.dir
cmake -DCMAKE_BUILD_TYPE=Release -DPATH_TO_LLVM=/opt/llvm/9.0.0 ../llvm2cpg
make package -j8
```

Now the package is ready:
```
> ls *.zip
llvm2cpg-0.1.0-LLVM-9.0-macOS-10.14.6.zip
```
