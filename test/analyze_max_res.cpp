/* Copyright (c) 2023 M.A.X. Port Team
 *
 * Analysis tool for MAX.RES file structure.
 */

#include <iostream>
#include <fstream>
#include <vector>
#include <cstring>
#include <iomanip>
#include <filesystem>
#include <algorithm>

// Structures from resource_manager.cpp/hpp
struct res_index {
    char tag[8];
    int32_t data_offset;
    int32_t data_size;
};

// Ensure packing
static_assert(sizeof(struct res_index) == 16, "The structure needs to be packed.");

struct res_header {
    char id[4];
    int32_t offset;
    int32_t size;
};

static_assert(sizeof(struct res_header) == 12, "The structure needs to be packed.");

std::string clean_tag(const char* tag) {
    std::string s;
    for (int i = 0; i < 8; ++i) {
        if (tag[i] != 0) {
            s += tag[i];
        }
    }
    return s;
}

const char* get_type(const std::string& tag) {
    if (tag.rfind("FONT_", 0) == 0) return "Font";
    if (tag.rfind("SC_", 0) == 0) return "Script";
    if (tag.rfind("ILOGO", 0) == 0) return "Image";
    if (tag.rfind("V_", 0) == 0) return "Voice";
    if (tag.rfind("S_", 0) == 0) return "Sound (Sfx)";
    if (tag.rfind("F_", 0) == 0) return "Flic (Anim)";
    if (tag.rfind("I_", 0) == 0) return "Icon";
    if (tag.rfind("P_", 0) == 0) return "Picture";
    if (tag.rfind("A_", 0) == 0) return "Audio/Anim"; // Ambiguous
    if (tag.find("FLC") != std::string::npos) return "Flic (Movie)";
    if (tag.find("PIC") != std::string::npos) return "Picture";
    if (tag.find("MSC") != std::string::npos) return "Music";
    if (tag == "HELP_ENG" || tag == "HELP_FRE" || tag == "TIPS" || tag == "CLANATRB" || tag == "ATTRIBS") return "Text/Data";
    if (tag.rfind("SNOW", 0) == 0) return "Map Data";
    if (tag.rfind("CRATER", 0) == 0) return "Map Data";
    if (tag.rfind("GREEN", 0) == 0) return "Map Data";
    if (tag.rfind("DESERT", 0) == 0) return "Map Data";
    
    return "Unknown";
}

// Helper to trim strings
std::string trim(const std::string& str) {
    size_t first = str.find_first_not_of(" \t\r\n");
    if (std::string::npos == first) {
        return str;
    }
    size_t last = str.find_last_not_of(" \t\r\n");
    return str.substr(first, (last - first + 1));
}

// Function to find settings.ini and extract game_data path
std::filesystem::path resolve_game_data_path() {
    std::vector<std::filesystem::path> setting_paths;

    // 1. Check XDG_DATA_HOME/max-port/settings.ini
    const char* xdg_data_home = std::getenv("XDG_DATA_HOME");
    if (xdg_data_home) {
        setting_paths.push_back(std::filesystem::path(xdg_data_home) / "max-port/settings.ini");
    } else {
        const char* home = std::getenv("HOME");
        if (home) {
            setting_paths.push_back(std::filesystem::path(home) / ".local/share/max-port/settings.ini");
        }
    }

    // 2. Fallback to local paths
    setting_paths.push_back("assets/settings.ini");
    setting_paths.push_back("settings.ini");
    // Also check parent dir (common in build/test envs)
    setting_paths.push_back("../assets/settings.ini");

    for (const auto& path : setting_paths) {
        if (std::filesystem::exists(path)) {
            // Found settings.ini, parse it
            std::ifstream ini_file(path);
            std::string line;
            bool in_setup = false;
            while (std::getline(ini_file, line)) {
                line = trim(line);
                if (line.empty() || line[0] == ';' || line[0] == '#') continue;

                if (line[0] == '[' && line.back() == ']') {
                    in_setup = (line == "[SETUP]");
                    continue;
                }

                if (in_setup) {
                    size_t eq_pos = line.find('=');
                    if (eq_pos != std::string::npos) {
                        std::string key = trim(line.substr(0, eq_pos));
                        if (key == "game_data") {
                            std::string value = trim(line.substr(eq_pos + 1));
                            // Handle "." relative path
                            if (value == ".") {
                                // If ".", it means the executable dir or current dir? 
                                // Usually means where the binary is, OR where settings.ini is?
                                // Let's assume where settings.ini is, or fallback to current.
                                // But for safety in this tool, let's treat it as the parent of settings.ini if absolute,
                                // or just current dir.
                                return std::filesystem::current_path();
                            }
                            return std::filesystem::path(value);
                        }
                    }
                }
            }
        }
    }
    
    return "";
}

// Sanitize string for filename
std::string sanitize_filename(const std::string& name) {
    std::string safe = name;
    for (char& c : safe) {
        if (!isalnum(c) && c != '_' && c != '-') {
            c = '_';
        }
    }
    return safe;
}

// Hex dump function
void dump_hex(std::ostream& out, const std::vector<uint8_t>& data) {
    out << "```text" << std::endl;
    size_t size = data.size();
    const size_t bytes_per_line = 16;
    
    for (size_t i = 0; i < size; i += bytes_per_line) {
        out << std::hex << std::setw(8) << std::setfill('0') << i << "  ";
        
        // Hex
        for (size_t j = 0; j < bytes_per_line; ++j) {
            if (i + j < size) {
                out << std::hex << std::setw(2) << std::setfill('0') << (int)data[i + j] << " ";
            } else {
                out << "   ";
            }
            if (j == 7) out << " "; // Split 8/8
        }
        
        out << " |";
        
        // ASCII
        for (size_t j = 0; j < bytes_per_line; ++j) {
            if (i + j < size) {
                char c = static_cast<char>(data[i + j]);
                out << (isprint(c) ? c : '.');
            }
        }
        out << "|" << std::endl;
    }
    out << "```" << std::endl;
}

int main(int argc, char** argv) {
    std::filesystem::path max_res_path;
    bool dump_mode = false;
    std::filesystem::path output_dir = "doc/specs";

    // Simple arg parsing
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--dump") {
            dump_mode = true;
        } else {
            max_res_path = arg;
        }
    }

    if (max_res_path.empty()) {
       // Logic to find automatically...
       // Reuse existing logic but need to refactor main slightly to accommodate arg parsing above
       // For now, let's keep the existing auto-discovery if path is empty after loop
    }
    
    // ... [Previous auto-discovery logic here, simplified/merged] ...
    // To minimize code churn, I will adapt the variable `max_res_path`.
    
    if (max_res_path.empty()) {
         std::filesystem::path game_data = resolve_game_data_path();
         if (!game_data.empty()) {
            std::vector<std::filesystem::path> candidates = {
                game_data / "MAX.RES",
                game_data / "assets/MAX.RES"
            };
            for (const auto& candidate : candidates) {
                if (std::filesystem::exists(candidate)) {
                    max_res_path = candidate;
                    break;
                }
            }
         }
         if (max_res_path.empty()) {
             // Fallbacks
              std::vector<std::filesystem::path> paths = {
                "MAX.RES",
                "../MAX.RES",
                "../assets/MAX.RES",
                "/usr/local/share/max-port/MAX.RES"
            };
            for (const auto& p : paths) {
                if (std::filesystem::exists(p)) {
                    max_res_path = p;
                    break;
                }
            }
         }
    }

    if (max_res_path.empty() || !std::filesystem::exists(max_res_path)) {
        std::cerr << "Error: MAX.RES not found or invalid path." << std::endl;
        return 1;
    }

    std::ifstream file(max_res_path, std::ios::binary);
    if (!file) {
        std::cerr << "Error: Could not open file." << std::endl;
        return 1;
    }

    // Read Header
    res_header header;
    if (!file.read(reinterpret_cast<char*>(&header), sizeof(header))) {
        std::cerr << "Error: Could not read header." << std::endl;
        return 1;
    }

    int item_count = header.size / sizeof(res_index);

    std::filesystem::path specs_dir = "doc/specs";
    std::filesystem::path structure_dir = specs_dir / "RES_Structure";
    std::filesystem::path dump_dir = specs_dir / "RES_Dump";
    std::filesystem::path txt_dir = specs_dir / "RES_TXT";

    // Create directories
    std::filesystem::create_directories(structure_dir);
    if (dump_mode) {
        std::filesystem::create_directories(dump_dir);
        std::filesystem::create_directories(txt_dir);
    }

    // Determine base filename for output
    std::string base_name = max_res_path.filename().string();
    std::replace(base_name.begin(), base_name.end(), '.', '_');
    
    // Prepare structure output file
    std::filesystem::path structure_file = structure_dir / (base_name + "_Structure.md");
    std::ofstream struct_out(structure_file);
    if (!struct_out) {
        std::cerr << "Error: Could not create structure file " << structure_file << std::endl;
        return 1;
    }
    
    // Use a pointer to verify if we write to stdout or file? 
    // User requested "Code should save...". So we write to file.
    // We can also print a message to stdout saying where it was saved.
    std::ostream& out = struct_out;

    out << "# Structure du fichier " << max_res_path.filename().string() << std::endl << std::endl;
    out << "**Fichier**: `" << max_res_path.string() << "`" << std::endl << std::endl;
    
    out << "## En-tête (Header)" << std::endl << std::endl;
    out << "*   **ID**: `" << std::string(header.id, 4) << "`" << std::endl;
    out << "*   **Index Offset**: `" << header.offset << "` (0x" << std::hex << header.offset << ")" << std::dec << std::endl;
    out << "*   **Index Size**: `" << header.size << "` bytes" << std::endl;
    out << "*   **Nombre d'éléments**: `" << item_count << "`" << std::endl << std::endl;

    // Read Index Table
    std::vector<res_index> indices(item_count);
    file.seekg(header.offset, std::ios::beg);
    if (!file.read(reinterpret_cast<char*>(indices.data()), header.size)) {
        std::cerr << "Error: Could not read index table." << std::endl;
        return 1;
    }

    out << "## Contenu (Table des Index)" << std::endl << std::endl;
    out << "| Tag | Offset | Taille (Bytes) | Fin | Type |" << std::endl;
    out << "| :--- | :--- | :--- | :--- | :--- |" << std::endl;

    for (int i = 0; i < item_count; ++i) {
        std::string tag = clean_tag(indices[i].tag);
        std::string type = get_type(tag);
        
        out << "| `" << tag << "`"
            << " | " << indices[i].data_offset 
            << " | " << indices[i].data_size 
            << " | " << (indices[i].data_offset + indices[i].data_size) 
            << " | " << type << " |" << std::endl;
    }
    out << std::endl;

    std::cout << "Generated structure file: " << structure_file << std::endl;

    if (dump_mode) {
        std::cout << "Dumping " << item_count << " items to " << dump_dir << "..." << std::endl;

        for (int i = 0; i < item_count; ++i) {
            std::string tag = clean_tag(indices[i].tag);
            std::string type = get_type(tag);
            std::string safe_tag = sanitize_filename(tag);
            
            std::string filename = base_name + "_" + safe_tag + "_Dump.md";
            std::filesystem::path out_path = dump_dir / filename;
            
            std::ofstream dump_out(out_path);
            if (!dump_out) {
                std::cerr << "Error creating " << out_path << std::endl;
                continue;
            }
            
            dump_out << "# Dump of " << tag << std::endl << std::endl;
            dump_out << "*   **File**: " << max_res_path.filename().string() << std::endl;
            dump_out << "*   **Tag**: `" << tag << "`" << std::endl;
            dump_out << "*   **Type**: " << type << std::endl;
            dump_out << "*   **Offset**: " << indices[i].data_offset << std::endl;
            dump_out << "*   **Size**: " << indices[i].data_size << " bytes" << std::endl << std::endl;
            
            // Read data
            std::vector<uint8_t> buffer(indices[i].data_size);
            file.seekg(indices[i].data_offset, std::ios::beg);
            if (file.read(reinterpret_cast<char*>(buffer.data()), indices[i].data_size)) {
                dump_out << "## Hex Dump" << std::endl << std::endl;
                dump_hex(dump_out, buffer);

                // Text Dump Extraction
                if (tag == "ATTRIBS" || tag == "HELP_ENG" || tag == "TIPS" || tag == "CLANATRB") {
                    std::string txt_filename = base_name + "_" + safe_tag + ".txt";
                    std::filesystem::path txt_path = txt_dir / txt_filename;
                    
                    std::ofstream txt_out(txt_path, std::ios::binary);
                    if (txt_out) {
                        txt_out.write(reinterpret_cast<const char*>(buffer.data()), buffer.size());
                        std::cout << "Extracted text to " << txt_path << std::endl;
                    }
                }

            } else {
                dump_out << "**Error reading data**" << std::endl;
            }
            // Optional: reduce console spam for large files
            // std::cout << "Generated " << out_path << std::endl;
        }
        std::cout << "Dump complete." << std::endl;
    }

    return 0;
}

