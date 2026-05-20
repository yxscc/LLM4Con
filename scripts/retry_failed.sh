#!/bin/bash
# Retry only the failed CVEs with updated compile flags
set -o pipefail

FAILED_CVES="CVE-2025-38383 CVE-2025-37854 CVE-2024-56555 CVE-2024-53160 CVE-2024-46704 CVE-2024-43891 CVE-2024-42234 CVE-2024-41005"
# Skip: CVE-2024-49998 (file missing), CVE-2011-2183 (too old, no bounds.h)

LLM4CON_HOME="${LLM4CON_HOME:-/home/LLM4Con}"
SURVEY_FILE="${SURVEY_FILE:-/tmp/cve_survey.csv}"
EXPERIMENT_BASE="${EXPERIMENT_BASE:-${LLM4CON_HOME}/kernel_experiment}"
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"

# Remove old .ll so FORCE isn't needed
for cve in $FAILED_CVES; do
    rm -f "$EXPERIMENT_BASE/$cve"/*.ll "$EXPERIMENT_BASE/$cve"/merged.ll 2>/dev/null
done

# Create a temporary survey with only failed CVEs
TMP_SURVEY=$(mktemp)
head -1 "$SURVEY_FILE" > "$TMP_SURVEY"
for cve in $FAILED_CVES; do
    grep "^$cve," "$SURVEY_FILE" >> "$TMP_SURVEY"
done

echo "=== Retrying $(echo $FAILED_CVES | wc -w) failed CVEs ==="
cat "$TMP_SURVEY"
echo ""

SURVEY_FILE="$TMP_SURVEY" bash "$SCRIPT_DIR/batch_prepare.sh"

rm -f "$TMP_SURVEY"
