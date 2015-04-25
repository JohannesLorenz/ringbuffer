# 0 Greetings
Welcome to the installation!

**Contents**

  1. Requirements
  2. Installation
  3. Running
  4. Realtime check with stoat
  5. Debugging

# 1 Requirements
You will need the following libraries, headers and tools:
  * at least g++ 4.8 or clang 3.3
  * cmake

# 2 Installation
In this directory, type:

```sh
mkdir build
cd build
# for a release build using clang (suggested), where /path/to/zynaddsubfx is
# the binary executable for zynaddsubfx
cmake -DCOMPILER=clang \
      -DCMAKE_BUILD_TYPE=Release \
      ..
```

# 3 Running
You can execute tests now:
```sh
make test
```

# 4 Realtime check with stoat
Make sure stoat is (root-)installed on your disk.

Instead of using the cmake code from above, use
```sh
cmake -DCOMPILER=stoat ..
```

Then, run
```sh
make stoat_ringbuffer
```

The output should prompt a total of 0 errors.

# 5 Debugging
TODO

