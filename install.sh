# Fonction pour installer max-port proprement
max_install() {
    echo "Installation du jeu max-port (nécessite sudo)..."
   
    # 1. Vérifie si le dossier source existe
    local SOURCE_DIR="/home/fred/Games/max-port"
    if [ ! -d "$SOURCE_DIR" ]; then
        echo "Erreur : Le dossier source $SOURCE_DIR n'existe pas."
        return 1
    fi

    # 2. Aller dnas le répertoire de build
    cd "$SOURCE_DIR/RelWithDebInfo" || return 1

    # 3. Lancer l'installation proprement dite
    sudo cmake --install . || { echo "Erreur d'installation."; return 1; }

    echo "✅ Installation terminée avec succès."
}

max_install
