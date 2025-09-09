# Lace: A Concurrency Vulnerability Detection Tool Based on Code Property Graphs

Lace is an advanced static analysis tool for C/C++, designed to automatically detect complex concurrency vulnerabilities. It deeply integrates Code Property Graphs (CPG), precise pointer analysis, and optional Large Language Model (LLM) assistance to deliver high-accuracy vulnerability analysis for large-scale, complex codebases.

## Core Features

-   **High-Precision Analysis**: Utilizes Joern to construct comprehensive Code Property Graphs (CPG) and integrates Phasar for precise pointer analysis, significantly enhancing the depth and accuracy of the analysis.
-   **Focus on Concurrency Vulnerabilities**: Features a built-in analysis model specifically for concurrency, enabling the effective detection of data races, use-after-frees (UAF), double frees, and other concurrency-related security flaws.
-   **Large Language Model (LLM) Integration**: The innovative `LLMUtil` module allows for interaction with large language models to assist in explaining vulnerability causes, generating analysis queries, or enabling natural language interaction with the analyzer.
-   **Extensive Vulnerability Benchmark**: Includes a comprehensive test benchmark (`ConVul/cve-benchmark`) with numerous real-world CVEs to continuously validate and evaluate the tool's effectiveness.
-   **Extensible Framework**: Built with C++ and CMake, offering a clear, modular structure (`src/`, `include/`) that allows researchers and developers to easily extend its functionality.
-   **Experimentation and Comparison**: The project contains a complete experimental evaluation framework (`experimental_result/`) that supports performance comparisons against other well-known analysis tools like Fsam and RacerF.

## Technical Architecture & Workflow

The core analysis workflow of Lace consists of the following key steps:

1.  **Compilation & LLVM Bitcode Generation**: The target C/C++ project is compiled into LLVM bitcode (`.bc` files) using `Clang-15`, which serves as the foundation for low-level analysis.
2.  **Code Property Graph (CPG) Construction**: **Joern** is invoked to parse the source code and generate an initial Code Property Graph, which contains rich information about the code's syntax, control flow, and data flow.
3.  **Pointer Analysis Enhancement**: **Phasar** performs high-precision pointer and alias analysis on the generated LLVM bitcode. The results are used to enhance the CPG with more accurate data dependency information.
4.  **Concurrency Vulnerability Analysis**: The core Lace analyzer loads the enhanced CPG and applies its built-in concurrency vulnerability detection models and query rules. It performs pattern matching and data-flow tracking on the graph to identify potential vulnerabilities.
5.  **Result Generation**: Analysis results, logs, and visualized data-flow graphs (e.g., `.dot` files) are saved in the `Lace-output/` or `CCPG_dump/` directory for further inspection.

## Environment & Dependencies

Before you begin, please ensure your system is properly configured.

### 1. System Dependencies (Ubuntu/Debian)
```bash
sudo apt-get update && sudo apt-get install -y \
    build-essential \
    cmake \
    g++ \
    python3 \
    libtinfo5 \
    libz3-dev \
    graphviz \
    libcurl4-openssl-dev
```

### 2. Joern
Please follow the official installation guide at [Joern's Documentation](https://joern.io/docs/installing).

### 3. Phasar & Clang-15
This project relies on Phasar for pointer analysis, which in turn requires **Clang-15**. The recommended way to install Phasar is from the source, as this will also build the correct LLVM/Clang version.

```bash
git clone https://github.com/secure-software-engineering/phasar.git
cd phasar
./utils/build.sh
```

**Important**: The Phasar build script will download and compile LLVM/Clang-15. Please ensure this does not conflict with other versions of Clang on your system. You may need to manage your `PATH` environment variable or use full paths to the executables to avoid version conflicts.

## Installation & Compilation

1.  **Clone the Repository**
    ```bash
    git clone <your-repository-url>
    cd Lace
    ```

2.  **Build using the script**
    The project includes a convenient build script.

    -   For a **Release build**:
        ```bash
        ./build.sh
        ```
        The executable will be located at `Release-build/llm_detector`.

    -   For a **Debug build**:
        ```bash
        ./build.sh debug
        ```
        The executable will be located at `Debug-build/llm_detector`.

## How to Use

Lace is operated via the command line.

**Basic Command Syntax:**

```bash
# Example for a release build
./Release-build/llm_detector [OPTIONS] <SOURCE_DIR> <BITCODE_FILE>
```

-   `<SOURCE_DIR>`: The absolute path to the target project's source directory.
-   `<BITCODE_FILE>`: The path to the LLVM bitcode file generated from the target project.

## Project Directory Structure
```
/
├── src/                   # Core analyzer C++ source code
├── include/               # Header files
│   ├── CCPG/              # Concurrency CPG definitions
│   ├── CPG/               # Base CPG structure
│   ├── LLMUtil/           # LLM interaction module
│   └── PhasarUtil/        # Phasar integration module
├── ConVul/                # Vulnerability benchmarks and test cases
│   └── cve-benchmark/
├── experimental_result/   # Experimental data and comparison tool results
├── Lace-output/      # Output directory for analysis results
├── cpg_dot/               # Generated DOT graph files
└── CMakeLists.txt         # CMake configuration file
```

## Evaluation

The `evaluate_fsam_results.py` script and the contents of the `experimental_result/` directory are used to evaluate the detection results of Lace and perform quantitative comparisons with other tools. For detailed evaluation methodologies and results, please refer to the relevant directory and the `report.md` file.