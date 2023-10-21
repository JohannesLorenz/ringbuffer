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
cd $_
cmake ..
```

The typical options for `cmake` apply, e.g.:

```
-DCMAKE_BUILD_TYPE=Release
-DCMAKE_CXX_COMPILER=clang++
```

# 3 Running
You can execute tests now:
```sh
make test
```

# 4 Realtime check with stoat
Make sure stoat is (root-)installed on your disk.

For configuring, use
```sh
cmake -DCMAKE_BUILD_TYPE=Debug -DCMAKE_CXX_COMPILER=/usr/local/bin/stoat-compile++ ..
```

The output should prompt a total of 0 errors.

# 5 Debugging
TODO

