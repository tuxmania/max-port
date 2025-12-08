/* Copyright (c) 2023 M.A.X. Port Team
 *
 * Use of this source code is governed by an MIT-style license that can be
 * found in the LICENSE file.
 */

#include <gtest/gtest.h>
#include <SDL.h>
#include <filesystem>
#include <fstream>
#include <iostream>

#include "resource_manager.hpp"
#include "saveloadmenu.hpp"
#include "winloss.hpp"
#include "game_manager.hpp"
#include "units_manager.hpp"
#include "enums.hpp"

class SaveVictoryTest : public ::testing::Test {
protected:
    std::filesystem::path temp_dir;
    std::filesystem::path original_game_data_path;

    void SetUp() override {
        if (SDL_Init(0) != 0) {
            FAIL() << "Failed to init SDL: " << SDL_GetError();
        }

        ResourceManager_InitPaths();
        ResourceManager_InitResources();
        original_game_data_path = ResourceManager_FilePathGameData;

        // Create a temporary directory for test data
        temp_dir = std::filesystem::current_path() / "temp_victory_test";
        std::filesystem::create_directories(temp_dir);

        // Redirect GamePref path to temp dir so we can control the save files (for Custom game type)
        ResourceManager_FilePathGamePref = temp_dir;
    }

    void TearDown() override {
        // Restore paths - actually GamePref global doesn't need restore if process exits, but good practice
        // ResourceManager_FilePathGamePref = ...;
        
        // Cleanup temp dir
        std::filesystem::remove_all(temp_dir);
        
        SDL_Quit();
    }
};

TEST_F(SaveVictoryTest, AnalyzeSave1DAT) {
    std::cout << "Test Body Started" << std::endl;
    std::filesystem::path source_save = "/home/fred/Games/max-port/saves/SAVE1.DTA";
    
    if (!std::filesystem::exists(source_save)) {
        FAIL() << "Source save file not found at " << source_save;
    }

    // Copy to temp dir as valid save file for slot 1
    // The game expects "SAVE1.DTA" (uppercase) for standard/training/campaign saves usually (type 0, 1, 2 etc map to .dta ?)
    // Wait, type 0 maps to .dta. 
    std::filesystem::path dest_save = temp_dir / "SAVE1.DTA";
    std::filesystem::copy_file(source_save, dest_save, std::filesystem::copy_options::overwrite_existing);

    // Load the game
    // Slot 1, GameType 0 (Custom - .dta), ini_load_mode true
    // Note: SaveLoadMenu_Load takes int32_t save_slot, int32_t game_file_type, bool ini_load_mode
    // And it constructs filename: save%i.%s
    // SaveLoadMenu_SaveFileTypes[0] is "dta".
    
    // We try to load as GAME_TYPE_CUSTOM (0) which maps to "dta"
    bool success = SaveLoadMenu_Load(1, 0, true);
    
    if (!success) {
        // Try renaming to .DAT just in case? Or .dta lowercase?
        // On Linux file systems, case matters.
        // The code calls .Toupper(), so it looks for SAVE1.DTA if it uses standard path construction.
        // If it failed, maybe the save type inside the file mismatch?
        // But headers are read.
        FAIL() << "Failed to load save file SAVE1.DTA";
    }

    // Now analyze victory status
    WinLoss_Status status = WinLoss_EvaluateStatus(GameManager_TurnCounter);

    std::cout << "--- Analysis of SAVE1.DAT ---" << std::endl;
    std::cout << "Mission Type: " << (int)status.mission_type << std::endl;
    std::cout << "Turn Counter: " << GameManager_TurnCounter << std::endl;
    std::cout << "Player Team: " << (int)GameManager_PlayerTeam << std::endl;

    for (int team = PLAYER_TEAM_RED; team < PLAYER_TEAM_MAX - 1; ++team) {
        std::string teamName = "Unknown";
        if (team == PLAYER_TEAM_RED) teamName = "Red";
        else if (team == PLAYER_TEAM_GREEN) teamName = "Green";
        else if (team == PLAYER_TEAM_BLUE) teamName = "Blue";
        else if (team == PLAYER_TEAM_GRAY) teamName = "Gray";

        std::string statusStr = "Unknown";
        switch (status.team_status[team]) {
            case VICTORY_STATE_WON: statusStr = "WON"; break;
            case VICTORY_STATE_LOST: statusStr = "LOST"; break;
            case VICTORY_STATE_PENDING: statusStr = "PENDING"; break;
            case VICTORY_STATE_GENERIC: statusStr = "GENERIC"; break;
        }

        std::cout << "Team " << teamName << " Status: " << statusStr << std::endl;
        
        // Count units for this team
        int unitCount = 0;
        for (SmartList<UnitInfo>::Iterator it = UnitsManager_MobileLandSeaUnits.Begin(); it != UnitsManager_MobileLandSeaUnits.End(); ++it) {
            if ((*it).team == team) unitCount++;
        }
        for (SmartList<UnitInfo>::Iterator it = UnitsManager_StationaryUnits.Begin(); it != UnitsManager_StationaryUnits.End(); ++it) {
            if ((*it).team == team) unitCount++;
        }
        for (SmartList<UnitInfo>::Iterator it = UnitsManager_MobileAirUnits.Begin(); it != UnitsManager_MobileAirUnits.End(); ++it) {
            if ((*it).team == team) unitCount++;
        }
        
        std::cout << "  Active Units: " << unitCount << std::endl;
        
        // Print resource/materials if relevant
        // WinLoss checks material counts often.
        // We can't easily call static WinLoss_HasMaterials, but we can iterate units or check `UnitsManager_TeamInfo` if it has stats?
        // UnitsManager_TeamInfo has `stats_...` but WinLoss usually summing up storage.
    }
    
    // Check specific conditions for Campaign Mission 8 mentioned in WinLoss? 
    // Or just generic elimination.
    // The user says "eliminated all opponents".
    // This usually means `VICTORY_STATE_WON` if opponent counts are 0.
    
    // List details of active opponent units to help user find them
    if (GameManager_PlayerTeam == PLAYER_TEAM_RED) {
        for (int team = PLAYER_TEAM_GREEN; team <= PLAYER_TEAM_GRAY; ++team) {
             // Focus on MobileLandSea units which seem to be the cause (HasAttackPower=1)
             std::cout << "--- MobileLandSea Units for Team " << team << " ---" << std::endl;
             SmartList<UnitInfo>& list = UnitsManager_MobileLandSeaUnits;
             for (auto it = list.Begin(); it != list.End(); ++it) {
                 if ((*it).team == team) {
                     const char* unitName = UnitsManager_BaseUnits[(*it).GetUnitType()].singular_name;
                     int ammo = (*it).ammo;
                     int hits = (*it).hits;
                     ResourceID type = (*it).GetUnitType();
                     
                     bool contributesToAttackPower = (ammo > 0 && type != SUBMARNE && type != COMMANDO);
                     
                     std::cout << "  [Mobile] " << unitName << " (Type: " << type << ")"
                               << " at (" << ((*it).grid_x + 1) << "," << ((*it).grid_y + 1) << ")"
                               << " Hits: " << hits << " Ammo: " << ammo
                               << " Contributes: " << (contributesToAttackPower ? "YES" : "NO")
                               << std::endl;
                 }
             }
        }
    }

    std::cout << "-----------------------------" << std::endl;
}
