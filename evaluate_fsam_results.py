
import os
import re
import glob
import time
from openai import OpenAI
import logging

# --- Configuration ---
FSAM_RESULTS_DIR = "/home/ConCodeQL/experimental_result/Fsam"
CVE_DATA_DIR = "/home/ConCodeQL/LinConVul"
OUTPUT_DIR = "/home/ConCodeQL/experimental_result/Fsam_result"
LLM_PROVIDER = "openai"
LLM_MODEL = "gpt-5-2025-08-07"
LLM_KEY = "sk-Y5PXhElM2NobgKPelwlHFaXPeQrSzm4WJYOnHYn0QafbVRoK"
BASE_URL = "https://jeniya.cn/v1"
TOOL_NAME = "Fsam"
MAX_RETRIES = 3
RETRY_DELAY = 10 # seconds

# --- Logging Setup ---
logging.basicConfig(level=logging.INFO, format='%(asctime)s - %(levelname)s - %(message)s')

# --- LLM Client Initialization ---
try:
    client = OpenAI(api_key=LLM_KEY, base_url=BASE_URL)
except Exception as e:
    logging.error(f"Failed to initialize OpenAI client: {e}")
    exit(1)

def build_prompt(tool_name, tool_result, cve_id, readme, source_code):
    """Builds the evaluation prompt string."""
    prompt = f"""作为一名专业的漏洞分析专家，请根据以下信息，专门评估工具“{tool_name}”的性能。

## 漏洞信息

### CVE ID: {cve_id}

### README 描述:
```
{readme}
```

### 漏洞源代码:
```c
{source_code}
```

--------------------------------

## “{tool_name}” 的检测结果:
```
{tool_result if tool_result.strip() else "未检测到任何问题。"}
```

--------------------------------

## 评估任务

请**只针对这一个工具**，从以下四个维度进行详细评估，并给出清晰的结论：

1.  **是否直接检测到漏洞？** (即检测结果是否直接指出了`README`中描述的最终漏洞，例如Use-After-Free, Double-Free等)
2.  **是否检测到作为Root Cause的数据竞争？** (如果未能直接检测到漏洞，评估其是否准确地识别出了导致该漏洞产生的根本原因——数据竞争)
3.  **数据竞争的隐蔽性如何？** (如果检测到了数据竞争，评估这个数据竞争是否容易被开发者忽略。例如，由于复杂的指针操作、间接的函数调用或是在看似无关的代码块之间发生的竞争)
4.  **误报率分析** (评估工具的误报情况。注意：良性的数据竞争也应被视为误报。请指出哪些是误报，哪些是恶性竞争)

请以清晰的、结构化的方式给出你的分析报告。"""
    return prompt

def read_file_content(path):
    """Reads and returns the content of a file."""
    if not os.path.exists(path):
        logging.warning(f"File not found: {path}")
        return ""
    try:
        with open(path, 'r', encoding='utf-8') as f:
            return f.read()
    except Exception as e:
        logging.error(f"Error reading file {path}: {e}")
        return ""

def main():
    """Main function to run the evaluation."""
    os.makedirs(OUTPUT_DIR, exist_ok=True)
    logging.info(f"Output directory is {OUTPUT_DIR}")

    fsam_files = glob.glob(os.path.join(FSAM_RESULTS_DIR, "mta-*.md"))
    if not fsam_files:
        logging.warning(f"No Fsam result files found in {FSAM_RESULTS_DIR}")
        return

    logging.info(f"Found {len(fsam_files)} Fsam result files to evaluate.")

    for fsam_file_path in fsam_files:
        filename = os.path.basename(fsam_file_path)
        match = re.search(r'(CVE-\d{4}-\d+)', filename)
        if not match:
            logging.warning(f"Could not extract CVE ID from filename: {filename}")
            continue

        cve_id = match.group(1)
        logging.info(f"--- Processing {cve_id} ---")

        output_path = os.path.join(OUTPUT_DIR, f"fsam_eval_{cve_id}.md")
        if os.path.exists(output_path):
            logging.info(f"Evaluation for {cve_id} already exists. Skipping.")
            continue

        # --- 1. Collect information ---
        cve_dir = os.path.join(CVE_DATA_DIR, cve_id)
        source_code_path = os.path.join(cve_dir, f"{cve_id}.c")
        if not os.path.exists(source_code_path):
             source_code_path = os.path.join(cve_dir, "main.c") # Fallback for common names

        readme_path = os.path.join(cve_dir, "README.md")

        source_code = read_file_content(source_code_path)
        readme_content = read_file_content(readme_path)
        fsam_result = read_file_content(fsam_file_path)

        if not source_code or not readme_content:
            logging.error(f"Missing source code or README for {cve_id}. Skipping.")
            continue

        # --- 2. Build prompt and evaluate with retries ---
        prompt = build_prompt(TOOL_NAME, fsam_result, cve_id, readme_content, source_code)
        
        llm_response = None
        last_exception = None
        for attempt in range(MAX_RETRIES):
            try:
                logging.info(f"Sending request to LLM for {cve_id} (Attempt {attempt + 1}/{MAX_RETRIES})...")
                response = client.chat.completions.create(
                    model=LLM_MODEL,
                    messages=[
                        {"role": "system", "content": "You are a professional vulnerability analysis expert."},
                        {"role": "user", "content": prompt}
                    ]
                )
                llm_response = response.choices[0].message.content
                logging.info(f"Successfully received LLM response for {cve_id}.")
                break  # Success, exit the retry loop
            except Exception as e:
                last_exception = e
                logging.warning(f"Attempt {attempt + 1} for {cve_id} failed: {e}")
                if attempt < MAX_RETRIES - 1:
                    logging.info(f"Retrying in {RETRY_DELAY} seconds...")
                    time.sleep(RETRY_DELAY)
        
        # --- 3. Save the report ---
        with open(output_path, 'w', encoding='utf-8') as f:
            if llm_response:
                f.write(llm_response)
                logging.info(f"Successfully generated and saved report to {output_path}")
            else:
                error_message = f"Failed to get response from LLM after {MAX_RETRIES} attempts. Last error: {last_exception}"
                f.write(error_message)
                logging.error(f"Failed to generate report for {cve_id}. Saved error message to {output_path}")


if __name__ == "__main__":
    main()
