#!/bin/bash
# start_ghidra_mcp_server.sh — Start the Ghidra MCP Headless REST Server on localhost:8089

set -e

PORT=${GHIDRA_MCP_PORT:-8089}
BIND_ADDRESS=${GHIDRA_MCP_BIND_ADDRESS:-"127.0.0.1"}
JAVA_HOME="/usr/local/Cellar/openjdk@21/21.0.12.1/libexec/openjdk.jdk/Contents/Home"
GHIDRA_HOME="/usr/local/Cellar/ghidra/12.1.3/libexec"
EXTENSION_JAR="$HOME/Library/ghidra/ghidra_12.1.3_PUBLIC/Extensions/GhidraMCP/lib/GhidraMCP-6.0.0.jar"
PROJECT_PATH="$HOME/Library/ghidra/projects/SHO_R5900.gpr"

if [ ! -f "$EXTENSION_JAR" ]; then
    echo "[-] Error: Extension JAR not found at $EXTENSION_JAR"
    exit 1
fi

export PATH="$JAVA_HOME/bin:$PATH"
export JAVA_HOME

# Build classpath
CLASSPATH="$EXTENSION_JAR"

# Add Ghidra Framework, Features, Processors JARs
for jar in ${GHIDRA_HOME}/Ghidra/Framework/*/lib/*.jar; do
    [ -f "$jar" ] && CLASSPATH="${CLASSPATH}:${jar}"
done
for jar in ${GHIDRA_HOME}/Ghidra/Features/*/lib/*.jar; do
    [ -f "$jar" ] && CLASSPATH="${CLASSPATH}:${jar}"
done
for jar in ${GHIDRA_HOME}/Ghidra/Processors/*/lib/*.jar; do
    [ -f "$jar" ] && CLASSPATH="${CLASSPATH}:${jar}"
done

echo "[+] Starting GhidraMCP Headless Server on http://${BIND_ADDRESS}:${PORT}..."
echo "[+] Project: ${PROJECT_PATH}"

exec "$JAVA_HOME/bin/java" \
    -Xmx4g \
    -XX:+UseG1GC \
    -Dghidra.home="${GHIDRA_HOME}" \
    -Dapplication.name=GhidraMCP \
    -classpath "${CLASSPATH}" \
    com.xebyte.headless.GhidraMCPHeadlessServer \
    --port "${PORT}" \
    --bind "${BIND_ADDRESS}" \
    --project "${PROJECT_PATH}" \
    --program "${PROGRAM_NAME:-/SLES_551.47}" \
    "$@"

