# Fonction pour compiler max-port proprement
max_recompile() {
    echo "Démarrage de la compilation propre de MAX-Port..."
    
    # Vérifie si le dossier source existe
    local SOURCE_DIR="/home/fred/Games/max-port"
    if [ ! -d "$SOURCE_DIR" ]; then
        echo "Erreur : Le dossier source $SOURCE_DIR n'existe pas."
        return 1
    fi
    
    # 1. Aller au répertoire racine
    cd "$SOURCE_DIR" || return 1
    
    # 2. Nettoyer et configurer
    rm -rf RelWithDebInfo
    mkdir RelWithDebInfo
    cd RelWithDebInfo || return 1
    
    # 3. Configuration CMake
    cmake -DCMAKE_TOOLCHAIN_FILE=../cmake/toolchain-linux-x86_64.cmake -DCMAKE_BUILD_TYPE=RelWithDebInfo -DBUILD_SHARED_LIBS=ON -DCMAKE_INSTALL_PREFIX=/usr/local -DCMAKE_CXX_STANDARD=20 ..
    
    # 4. Compilation
    echo "Compilation en cours..."
    cmake --build . --parallel || { echo "Erreur de compilation."; return 1; }

    echo "✅ Compilation terminée avec succès."
}

max_recompile
