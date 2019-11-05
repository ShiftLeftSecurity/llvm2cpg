# Development Instructions

Various design considerations are collected [here](docs/design_considerations.md)

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

Next we need to figure out the path to llvm (find `LLVMConfig.cmake`) and, for validation tests, codepropertygraph (`cpgvalidator.sh`). For simplicity, we ume that these are in `/opt/llvm/` and `/opt/codepropertygraph` (if you want to copy-paste commands from here without thinking, then just place symlinks, like e.g. `ln -s /usr/lib/cmake/llvm/ /opt/llvm`).

Prepare build system:

```
$ git clone git@github.com:ShiftLeftSecurity/llvm2cpg.git --recursive
$ cd llvm2cpg
llvm2cpg$ (cd target; cmake -G Ninja     -DCMAKE_C_COMPILER=clang     -DCMAKE_CXX_COMPILER=clang++     -DPATH_TO_LLVM=/opt/llvm/  -DPATH_TO_CODEPROPERTYGRAPH=/opt/codepropertygraph ..)
```

We then want to build everything we need:
```
target$ ninja unit-tests
target$ ./tests/unit-tests/unit-tests
target$ ninja cpg-proto-writer

```
#### Build and run tests:

```
target$ ninja unit-tests
target$ ./tests/unit-tests/unit-tests
```

#### Build and run `cpg-proto-writer`

```
target$ ninja cpg-proto-writer
target$ ./tools/cpg-proto-writer/cpg-proto-writer ./tests/fixtures/basic_c_support/return_constant.bc
Processing ./tests/fixtures/basic_c_support/return_constant.bc
CPG is successfully save on disk: ./cpg.bin.zip
```

Getting help from `cpg-proto-writer`:

```
target$ ./tools/cpg-proto-writer/cpg-proto-writer --help-hidden
USAGE: cpg-proto-writer [options] Bitcode files
...
```
We can validate the file by
```
target$ /opt/codepropertygraph/cpgvalidator.sh ./cpg.bin.zip
CPG construction finished in 554ms.
Found 0 errors.
```

## Testing

LLVM IR is target specific. In order to write tests against C code, it is recommended
to compile it on the target (developer) machine. TODO: Fix build config to use fixed target triple by default.

### Unit Tests
We have already seen the unit tests:
```
target$ ninja unit-tests
target$ ./tests/unit-tests/unit-tests
```

#### Validation Tests

TODO: Fix validation tests.

Validation tests should be run via ninja

There is a shortcut for the fast iterations over `cpg-proto-writer`: validation tests. This only works if `-DPATH_TO_CODEPROPERTYGRAPH` was set.

You should specify CPG for which bitcode file you want to validate in the `tests/validation-tests/CMakeLists.txt`.
Example:
```
validate_cpg(hello_world_c_bc)
```

_`hello_world_c_bc` is a fixture name, read the `Fixtures` section to learn more._

Call to `validate_cpg` will generate a target you can use within the build system. Example:

```
> ninja validate-hello_world_c_bc
[ 35%] Built target CPG
[ 57%] Built target CPGProto
[ 78%] Built target CPGProtoWriter
[ 92%] Built target cpg-proto-writer
Validating /opt/llvm2cpg/cmake-build-debug/tests/validation-tests/hello_world_c_bc/cpg.bin.zip
on-disk overflow file: /tmp/mvstore4648208665182620618.bin
CPG construction finished in 301ms.
Validation error: Expected 1 to 1 outgoing AST edges from METHOD to BLOCK but found 0. METHOD details: id: 5, properties: Map(NAME -> hello_world, AST_PARENT_TYPE -> NAMESPACE_BLOCK, FULL_NAME -> hello_world, IS_EXTERNAL -> false, AST_PARENT_FULL_NAME -> /opt/llvm2cpg/tests/fixtures/hello_world/hello_world.c_global)
Found 1 errors.
```

#### Integration tests

Integration tests are implemented using Scala, which adds some complexity to the setup,
but it is still easy to use.

##### Running tests

There are two ways to run the tests. What follows is assuming that you have the build system ready.

Run tests using `sbt`:

```
target$ ninja prepare-integration-tests
tests/integration-tests$ sbt test
```

Run tests using `llvm2cpg` build system:

```
target$ ninja run-integration-tests
```

##### Adding new test

Let's add a test called `FooTest`.

 - Create a file `tests/integration-tests/src/test/scala/io/shiftleft/llvm2cpgintegration/FooTest.scala`
with the following content:

```
package io.shiftleft.llvm2cpgintegration

import io.shiftleft.codepropertygraph.cpgloading.CpgLoader
import org.scalatest.{Matchers, WordSpec}
import io.shiftleft.semanticcpg.language._

class FooTest extends WordSpec with Matchers {
  val cpg = CpgLoader.load(TestCpgPaths.fooCPG)
  "test" in {
    cpg.file.toList.size shouldBe 42
  }
}
```

 - Add an entry for the CPG file path to `tests/integration-tests/src/test/scala/io/shiftleft/llvm2cpgintegration/TestCpgPaths.scala.in`, i.e.:

```
object TestCpgPaths {
  val helloWorldCPG = "@HelloWorldTest@"
  val fooCPG = "@FooTest@"
}
```

The variable (`fooCPG`) can have any name, but the value should follow the `"@TestName@"` format!

- Register the test within the build system (`tests/integration-tests/CMakeLists.txt`), e.g.:

```
add_integration_test(FooTest hello_world_c_bc hello_world_cpp_bc)

enable_integration_tests()
```

The parameter to `add_integration_test` is the name of the test, all the rest are names of bitcode fixtures.
You can pass as many fixtures as you like.
Important note, `add_integration_test` should go strictly before calling `enable_integration_tests`!

#### Fixtures

Test harness contains a number of utilities to generate and to use fixtures.
You can find implementation details under `build-system/tests/fixtures.cmake`.

##### Generation of fixtures

From a developer perspective, you only need to know one function: `compile_fixture`.

Here is an example:

```
compile_fixture(
  COMPILER ${CMAKE_C_COMPILER}
  INPUT ${CMAKE_CURRENT_LIST_DIR}/hello_world.c
  OUTPUT_EXTENSION ll
  FLAGS -c -emit-llvm
)
```

This function expands into the following command:

```
${CMAKE_C_COMPILER} -c -emit-llvm ${CMAKE_CURRENT_LIST_DIR}/hello_world.c -o hello_world.ll
```

The fixture will be generated when it is needed by the test target.
The fixture will be re-generated when the contents of the input file changes.

For more examples, please look at `tests/fixtures`.

##### Usage of fixtures

Besides that, it does some bookkeeping under the hood:

 - adds the fixture `INPUT` into the global list of fixtures
 - stores input and output files for the given fixture
 - generates a C++ header file containing all the fixture inputs and outputs

The file looks like this:

```
// !!! Autogenerated file !!!

namespace llvm2cpg {
namespace fixtures {

static __attribute__((used)) const char *hello_world_c_bc_output_path() {
  return "/opt/llvm2cpg/cmake-build-debug/tests/fixtures/hello_world/hello_world.bc";
}

static __attribute__((used)) const char *hello_world_c_bc_input_path() {
  return "/opt/llvm2cpg/tests/fixtures/hello_world/hello_world.c";
}

} // namespace fixtures
} // namespace llvm2cpg
```

There are two methods for each fixtures: `<fixture_name>_input_path` and `<fixture_name>_output_path`.
They can be used as follows:

```
llvm::LLVMContext context;
BitcodeLoader loader;
auto bitcode = loader.loadBitcode(fixtures::hello_world_c_bc_output_path(), context);
ASSERT_NE(bitcode.get(), nullptr);
```

##### Fixture naming

The fixture name is a concatenation of the `INPUT` file name and the `OUTPUT_EXTENSION` with all
the dots replaced by underscores. Examples:

 ```
compile_fixture(
  INPUT foo/bar.buzz.c
  OUTPUT_EXTENSION bc
)
=> bar_buzz_c_bc

compile_fixture(
  INPUT foo/bar.buzz.c
  OUTPUT_EXTENSION opt.bc
)
=> bar_buzz_c_opt_bc
```

The fixture names must be unique across the project. Otherwise, you will get an error from CMake.


# Viewing generated cpgs

For easy visualization of the output, we have some scripts in the `devtools` folder. These require python3, graphviz, and the feh image viewer.
