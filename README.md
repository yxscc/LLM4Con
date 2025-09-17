# Lace: Semantics-Guided Concurrency Defect Detection

Lace is an advanced static analysis tool for C/C++, designed to automatically detect complex concurrency vulnerabilities by bridging the gap between developer intent and program behavior. It deeply integrates Code Property Graphs (CPG), precise pointer analysis, and Large Language Model (LLM) assistance to provide semantic understanding that enhances traditional static analysis capabilities.

## Core Features

-   **Semantics-Guided Analysis**: Leverages Large Language Models to understand developer intent and program semantics, bridging the gap between static analysis and human understanding of concurrency patterns.
-   **Concurrency Contract Generation**: Automatically generates precise "Concurrency Contracts" for each thread, capturing semantic information about shared variables, synchronization primitives, and intended parallel behavior.
-   **High-Precision Analysis**: Utilizes Joern to construct comprehensive Code Property Graphs (CPG) and integrates Phasar for precise pointer analysis, significantly enhancing the depth and accuracy of the analysis.
-   **Focus on Concurrency Vulnerabilities**: Features a built-in analysis model specifically for concurrency, enabling the effective detection of data races, use-after-frees (UAF), double frees, and other concurrency-related security flaws.
-   **LLM-Enhanced Detection**: The innovative `LLMUtil` module uses LLM agents to analyze thread behavior, identify shared state, and detect concurrency violations that traditional static analysis might miss.
-   **Extensive Vulnerability Benchmark**: Includes a comprehensive test benchmark (`ConVul/cve-benchmark`) with numerous real-world CVEs to continuously validate and evaluate the tool's effectiveness.
-   **Custom Linux Dataset**: Features `LinConVul`, a self-constructed Linux kernel concurrency vulnerability dataset with 56 real-world CVEs for comprehensive evaluation.
-   **Extensible Framework**: Built with C++ and CMake, offering a clear, modular structure (`src/`, `include/`) that allows researchers and developers to easily extend its functionality.
-   **Experimentation and Comparison**: The project contains a complete experimental evaluation framework (`experimental_result/`) that supports performance comparisons against other well-known analysis tools like Fsam and RacerF.

## Technical Architecture & Workflow

The core analysis workflow of Lace consists of the following key steps:

1.  **Compilation & LLVM Bitcode Generation**: The target C/C++ project is compiled into LLVM bitcode (`.bc` files) using `Clang-15`, which serves as the foundation for low-level analysis.

2.  **Code Property Graph (CPG) Construction**: **Joern** is invoked to parse the source code and generate an initial Code Property Graph, which contains rich information about the code's syntax, control flow, and data flow.

3.  **Pointer Analysis Enhancement**: **Phasar** performs high-precision pointer and alias analysis on the generated LLVM bitcode. The results are used to enhance the CPG with more accurate data dependency information.

4.  **Concurrency Contract Generation**: **LLM Agents** analyze each thread's entry function to generate precise "Concurrency Contracts" that capture:
    - Thread role and purpose (semantic understanding)
    - Shared variables and their access patterns
    - Synchronization primitives and their usage
    - Intended parallel relationships between threads

5.  **Semantics-Guided Vulnerability Detection**: The core Lace analyzer combines traditional static analysis with LLM-generated semantic information to detect concurrency vulnerabilities that might be missed by purely syntactic approaches.

6.  **Result Generation**: Analysis results, logs, and visualized data-flow graphs (e.g., `.dot` files) are saved in the `Lace-output/` or `CCPG_dump/` directory for further inspection.

### Key Innovation: LLM-Enhanced Semantic Analysis

Lace's key innovation lies in its use of Large Language Models to bridge the gap between developer intent and program behavior:

- **Concurrency Contract Generation**: LLM agents analyze thread entry functions to understand their intended behavior and identify shared state that traditional static analysis might miss.

- **Semantic Understanding**: The tool uses LLM understanding of code semantics to identify complex concurrency patterns and potential race conditions that depend on program logic rather than just syntax.

- **Enhanced Detection Accuracy**: By combining traditional static analysis with semantic understanding, Lace achieves higher precision in detecting concurrency vulnerabilities while reducing false positives.

### LLM Integration Architecture

Lace employs a sophisticated multi-agent LLM architecture to enhance static analysis:

1. **ContractGeneratorAgent**: Analyzes thread entry functions to generate Concurrency Contracts that capture:
   - Thread role and semantic purpose
   - Shared variables and their access patterns (Read/Write/ReadWrite)
   - Synchronization primitives (mutexes, semaphores, condition variables)
   - Intended parallel relationships

2. **ParallelAnalysisAgent**: Uses generated contracts to detect concurrency violations:
   - Data races between threads accessing shared variables
   - Use-after-free and double-free vulnerabilities
   - Deadlock patterns and lock ordering violations
   - Time-of-check-time-of-use (TOCTOU) vulnerabilities

3. **FindingThreadEntryAgent**: Identifies thread creation points and entry functions in the codebase

4. **AgentManager**: Orchestrates the multi-agent analysis workflow, coordinating between different LLM agents and traditional static analysis components

The LLM agents communicate through a structured tool-calling interface, allowing them to:
- Query the Code Property Graph for program structure information
- Access pointer analysis results from Phasar
- Generate structured analysis results that integrate with traditional static analysis

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

### Using the LinConVul Dataset

The project includes a comprehensive Linux kernel concurrency vulnerability dataset (`LinConVul/`) with 56 real-world CVEs. Each CVE directory contains:

- `CVE-XXXX-XXXX.c`: The vulnerable source code
- `README.md`: Detailed description of the vulnerability
- `run.sh`: Automated compilation and analysis script
- `poc/`: Proof-of-concept files (if available)

**To analyze a specific CVE:**

```bash
cd LinConVul/CVE-XXXX-XXXX
./run.sh
```

This will automatically compile the code to LLVM bitcode and run Lace analysis.

## Project Directory Structure
```
Lace/
├── src/                           # Core analyzer C++ source code
├── include/                       # Header files
│   ├── CCPG/                      # Concurrency CPG definitions
│   ├── CPG/                       # Base CPG structure
│   ├── LLMUtil/                   # LLM interaction module
│   └── PhasarUtil/                # Phasar integration module
├── ConVul/                        # Vulnerability benchmarks and test cases
│   └── cve-benchmark/             # Standard CVE benchmark dataset
├── LinConVul/                     # Custom Linux kernel concurrency vulnerability dataset
│   ├── CVE-2009-3547/             # Individual CVE directories (56 total)
│   │   ├── CVE-2009-3547.c       # Vulnerable source code
│   │   ├── README.md              # Vulnerability description
│   │   ├── run.sh                 # Automated analysis script
│   │   └── poc/                   # Proof-of-concept files
│   └── ...                        # More CVE directories
├── Lace-organized/               # Organized experimental results (by CVE)
│   └── CVE-XXXX-XXXX/            # Results organized by CVE
│       └── run_01/               # Individual run results
├── Lace-comparison-organized/    # LLM comparison experimental results
│   ├── deepseek-v3.1/            # DeepSeek v3.1 model results
│   │   └── CVE-XXXX-XXXX/        # Results by CVE
│   └── gemini-2.5-pro/           # Gemini 2.5 Pro model results
│       └── CVE-XXXX-XXXX/        # Results by CVE
├── experimental_result/           # Experimental data and comparison tool results
├── CCPG_dump/                    # Dumped Code Property Graph files
├── cpg_dot/                      # Generated DOT graph files for visualization
├── LLM_dump/                     # LLM interaction logs and outputs
├── llvmbc/                       # LLVM bitcode files
├── build/                        # Build directory
├── Debug-build/                  # Debug build output
├── build.sh                      # Build script
├── CMakeLists.txt                # CMake configuration file
└── README.md                     # This file
```

## Evaluation

### Dataset Evaluation

The project includes comprehensive evaluation capabilities:

1. **LinConVul Dataset**: A custom Linux kernel concurrency vulnerability dataset with 56 real-world CVEs, each containing:
   - Vulnerable source code
   - Detailed vulnerability descriptions
   - Automated analysis scripts
   - Proof-of-concept files where available

2. **Standard CVE Benchmark**: The `ConVul/cve-benchmark/` directory contains additional vulnerability test cases for broader evaluation.

3. **LLM Comparison**: The `Lace-comparison-organized/` directory contains results from different LLM models (DeepSeek v3.1 and Gemini 2.5 Pro) for comparative analysis.

### Experimental Results

- **Lace-organized/**: Contains organized experimental results grouped by CVE
- **experimental_result/**: Contains detailed experimental data and comparison results with other analysis tools
- **Lace-comparison-organized/**: Contains LLM comparison results for evaluating different model performances

For detailed evaluation methodologies and results, please refer to the relevant directories and analysis reports.

## Compilation and Build Process

### Prerequisites

1. **System Dependencies**: Install required packages as listed in the Environment & Dependencies section
2. **Joern**: Install following the official documentation
3. **Phasar & Clang-15**: Build from source to ensure compatibility

### Build Steps

1. **Clone the repository**:
   ```bash
   git clone <repository-url>
   cd Lace
   ```

2. **Build the project**:
   ```bash
   # For release build
   ./build.sh
   
   # For debug build
   ./build.sh debug
   ```

3. **Verify installation**:
   ```bash
   # Check if executable exists
   ls -la Release-build/llm_detector
   # or for debug build
   ls -la Debug-build/llm_detector
   ```

### Running Analysis

1. **Using LinConVul dataset**:
   ```bash
   cd LinConVul/CVE-XXXX-XXXX
   ./run.sh
   ```

2. **Manual analysis**:
   ```bash
   ./Release-build/llm_detector [OPTIONS] <SOURCE_DIR> <BITCODE_FILE>
   ```

The tool will generate analysis results, logs, and visualization files in the appropriate output directories.