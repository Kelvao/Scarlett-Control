#!/bin/sh
set -e

USER_MODE=0

for arg in "$@"; do
    case "$arg" in
        --user)
            USER_MODE=1
            ;;
        --help|-h)
            echo "Uso: $0 [--user]"
            echo "  Sem argumentos: instala no sistema (/usr/local)"
            echo "  --user:        instala apenas para o usuário atual (~/.local)"
            exit 0
            ;;
    esac
done

if [ "$USER_MODE" -eq 1 ]; then
    echo "Instalando para o usuário atual (~/.local)..."
    make install-user
else
    if [ "$(id -u)" -ne 0 ]; then
        echo "Aviso: Instalação no sistema requer permissões de root."
        echo "Executando: sudo make install"
        sudo make install
    else
        make install
    fi
fi

echo "Instalação concluída com sucesso!"
echo "Você pode executar 'scarlett-control' no terminal ou abrir pelo menu de aplicativos."

