
# ConCord Installation and Usage Guide

## 1. Environment Setup

### 1.1 Install LLVM Toolchain

download [clang+llvm-16](https://github.com/llvm/llvm-project/releases?page=5) and install.

### 1.2 Install Joern

download [joern](https://github.com/joernio/joern/releases) and install.

### 1.3 System Dependencies
```shell
# For Ubuntu/Debian systems
sudo apt-get update && sudo apt-get install -y \
    cmake \
    gcc-10 g++-10 \
    libtinfo5 \
    libz3-dev \
    graphviz \
    libcurl4-openssl-dev
```

## 2. Build Process

### 2.1 Repository Setup
```shell
download ConCord-0FD6.zip
unzip ConCord-0FD6.zip
cd ConCord-0FD6
```

### 2.2 Compilation
```shell
# Build with debug symbols
source ./build.sh debug
```

## 3. Usage Documentation

### 3.1 Command Syntax
```shell
./Debug-build/ccql [OPTIONS] <SOURCE_DIR> <BITCODE_FILE>
```

### 3.2 Core Arguments
| Argument          | Format                   | Example                    |
|-------------------|--------------------------|----------------------------|
| `<SOURCE_DIR>`    | POSIX absolute path      | `/mnt/projects/openssl`    |
| `<BITCODE_FILE>`  | LLVM bitcode file        | `/build/obj/main.bc`       |

## 4. Technical Reference

### 4.1 File Specifications
| Extension | Purpose                  | Generation Command        |
|-----------|--------------------------|---------------------------|
| `.bc`     | LLVM bitcode             | `clang -c -emit-llvm`     |
| `.ll`     | Human-readable IR        | `llvm-dis input.bc`       |

## 5. Output

The detection results are output in the folder ConCord-0FD6/CCPG_dump/targetDir_YY_MM_DD_HH_MM_SS.

Each output result folder contains the following subfolders and files:

Subfolders:

| Folders         | Purpose                            |
|-----------------|------------------------------------|
| `functions`     | Identified functions               |
| `threads`       | Identified threads                 |
| `dataraces`     | Results of data race detection     |
| `useAfterFrees` | Results of UAF detection           |
| `nullReference` | Results of NPD detection           |
| `doubleFrees`   | Results of DF detection            |

Files:

CCPG.dot

execution-time.txt

thread-creation-tree.dot

