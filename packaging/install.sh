#!/bin/sh
set -e

USER_MODE=0

for arg in "$@"; do
    case "$arg" in
        --user)
            USER_MODE=1
            ;;
        --help|-h)
            echo "Usage: $0 [--user]"
            echo "  No arguments: install system-wide (/usr/local)"
            echo "  --user:       install for the current user only (~/.local)"
            exit 0
            ;;
    esac
done

if [ "$USER_MODE" -eq 1 ]; then
    echo "Installing for the current user (~/.local)..."
    make install-user
else
    if [ "$(id -u)" -ne 0 ]; then
        echo "Note: System-wide installation requires root privileges."
        echo "Running: sudo make install"
        sudo make install
    else
        make install
    fi
fi

echo "Installation completed successfully!"
echo "You can launch 'scarlett-control' from the terminal or your application launcher."
