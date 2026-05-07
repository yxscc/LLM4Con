# Lace: LLM-Enhanced Concurrency Vulnerability Detector

> **🚧 Picking up an in-progress experiment? Start here:**
> [`kernel_experiment/HANDOFF.md`](./kernel_experiment/HANDOFF.md)
> — TL;DR of current state, every pitfall the previous LLM/engineer hit, and exact next steps for the kernel-CVE detection experiment (M7).

Lace is a static analysis tool for C/C++ designed to detect complex concurrency vulnerabilities. It integrates Code Property Graphs (CPG), precise pointer analysis via Phasar, and Large Language Models (LLMs) to enhance traditional static analysis with semantic understanding.

## Core Features

-   **LLM-Enhanced Analysis**: Leverages Large Language Models to understand program semantics and identify complex concurrency patterns.
-   **High-Precision Pointer Analysis**: Integrates Phasar for precise pointer and alias analysis on LLVM bitcode.
-   **Code Property Graph (CPG)**: Utilizes Joern to construct comprehensive CPGs, providing rich structural and flow information.
-   **Multiple Analysis Modes**: Provides different executables for targeted analysis (`llm_detector`) and comparative studies (`llm_comparison`).

## Prerequisites

Before you begin, please ensure your system is properly configured.

### 1. System Dependencies (Ubuntu/Debian)

```bash
sudo apt-get update && sudo apt-get install -y \
    build-essential \
    cmake \
    g++ \
    clang-15 \
    libtinfo5 \
    libz3-dev \
    graphviz \
    libcurl4-openssl-dev \
    libssl-dev \
    libboost-system-dev \
    libboost-thread-dev \
    libcpprest-dev
```

### 2. Joern

Please follow the official installation guide at [Joern's Documentation](https://joern.io/docs/installing). Ensure the `joern-parse` and `joern-export` commands are available in your `PATH`.

### 3. Phasar

This project relies on Phasar for pointer analysis, which in turn requires **LLVM/Clang-15**. The recommended way to install Phasar is from the source.

```bash
git clone https://github.com/secure-software-engineering/phasar.git
cd phasar
# This script will manage dependencies and build Phasar along with LLVM-15
./bootstrap.sh
```

**Important**: The Phasar bootstrap script will download and compile LLVM/Clang-15. Please ensure this does not conflict with other versions of Clang on your system.

## Building the Project

The project includes a convenient build script.

1.  **Clone the Repository**
    ```bash
    git clone <your-repository-url>
    cd Lace
    ```

2.  **Build the executables**
    -   For a **Release build**:
        ```bash
        ./build.sh
        ```
        Executables will be located at `build/`.

    -   For a **Debug build**:
        ```bash
        ./build.sh debug
        ```
        Executables will be located at `Debug-build/`.

## How to Use

The project builds two main executables: `llm_detector` and `llm_comparison`. Both require similar arguments to run an analysis.

### Command-Line Arguments

Both executables share a common set of arguments for basic analysis and LLM configuration.

**Analysis Arguments:**

*   `--input-src <path>`: **(Required)** The absolute path to the target project's source directory or a single source file.
*   `--input-bc <path>`: **(Required)** The path to the pre-generated LLVM bitcode file (`.ll` or `.bc`).

**LLM Configuration:**

*   `--llm-provider <provider>`: Choose LLM provider: `openai` or `gemini` (default: `openai`).
*   `--llm-key <api_key>`: API key for the chosen LLM provider. Can also be set via `OPENAI_API_KEY` or `GEMINI_API_KEY` environment variables.
*   `--llm-model <model_name>`: Model name for the chosen LLM provider (e.g., `gpt-4o`).
*   `--llm-url <base_url>`: (Optional) Custom base URL for the LLM API.

### Running `llm_detector`

This is the primary tool for detecting concurrency bugs. It performs a detailed analysis and then uses an LLM to evaluate its own findings against other tools (Fsam, RacerF).

**Example:**

```bash
./Debug-build/llm_detector \
    --input-src /path/to/your/project \
    --input-bc /path/to/your/project.ll \
    --llm-provider openai \
    --llm-model gpt-4o
```

### Running `llm_comparison`

This tool is designed for comparative analysis. It runs the standard detection and then uses a powerful, separate LLM (hardcoded as GPT-5) to compare the results of its own analysis against a zero-shot LLM analysis.

**Example:**

```bash
./Debug-build/llm_comparison \
    --input-src /path/to/your/project \
    --input-bc /path/to/your/project.ll \
    --llm-provider gemini \
    --llm-model gemini-1.5-pro
```

### Provided Datasets

The project includes comprehensive datasets for testing and evaluation:

*   **`LinConVul/`**: A custom dataset of real-world concurrency vulnerabilities from the Linux kernel. Each subdirectory corresponds to a specific CVE and contains the vulnerable source code, a `README.md` with a detailed description of the vulnerability, and often a `run.sh` script to compile the code.
*   **`ConVul/`**: A collection of additional concurrency vulnerability benchmarks and test cases.

These datasets can be used as targets for analysis with `llm_detector` or `llm_comparison`.