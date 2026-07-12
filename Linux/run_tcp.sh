#!/usr/bin/env bash
set -euo pipefail

PROJECT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

cd "$PROJECT_DIR"

echo "Building project..."
cmake -S . -B build
cmake --build build

quote() {
    printf "%q" "$1"
}

make_command() {
    local program="$1"
    local project_dir_q

    project_dir_q="$(quote "$PROJECT_DIR")"
    printf 'cd %s && exec %s' "$project_dir_q" "$program"
}

open_wsl_tabs() {
    local server_command="$1"
    local client_command="$2"

    wt.exe -w new \
        new-tab --title "tcp_server" wsl.exe -d "$WSL_DISTRO_NAME" bash -lc "$server_command" \
        \; new-tab --title "tcp_client" wsl.exe -d "$WSL_DISTRO_NAME" bash -lc "$client_command" \
        >/dev/null 2>&1 &
}

open_terminal() {
    local title="$1"
    local terminal_command="$2"
    local terminal_command_q

    if command -v gnome-terminal >/dev/null 2>&1; then
        gnome-terminal --title="$title" -- bash -lc "$terminal_command" &
    elif command -v konsole >/dev/null 2>&1; then
        konsole --new-tab -p tabtitle="$title" -e bash -lc "$terminal_command" &
    elif command -v xfce4-terminal >/dev/null 2>&1; then
        terminal_command_q="$(quote "$terminal_command")"
        xfce4-terminal --title="$title" --command="bash -lc $terminal_command_q" &
    elif command -v xterm >/dev/null 2>&1; then
        xterm -T "$title" -e bash -lc "$terminal_command" &
    else
        echo "Could not find a supported terminal emulator."
        echo "Run these manually in two terminals:"
        echo "  ./build/bin/Tcp_Server"
        echo "  ./build/bin/Tcp_Client"
        return 1
    fi
}

SERVER_CMD="$(make_command "./build/bin/Tcp_Server")"
CLIENT_CMD="$(make_command "./build/bin/Tcp_Client")"

if [[ -n "${WSL_DISTRO_NAME:-}" ]] && command -v wt.exe >/dev/null 2>&1; then
    echo "Opening Ubuntu terminal tabs..."
    open_wsl_tabs "$SERVER_CMD" "$CLIENT_CMD"
    echo "Done."
    exit 0
fi

echo "Starting server..."
open_terminal "tcp_server" "$SERVER_CMD"

sleep 1

echo "Starting client..."
open_terminal "tcp_client" "$CLIENT_CMD"

echo "Done."
