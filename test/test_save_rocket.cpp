/* Copyright (c) 2023 M.A.X. Port Team
 *
 * Use of this source code is governed by an MIT-style license that can be
 * found in the LICENSE file.
 */

#include <gtest/gtest.h>
#include <SDL.h>
#include <filesystem>
#include <iostream>

#include "game_manager.hpp"
#include "resource_manager.hpp"
#include "saveloadmenu.hpp"
#include "units_manager.hpp"
#include "unitinfo.hpp"
#include "enums.hpp"
#include "access.hpp"

class SaveRocketTest : public ::testing::Test {
protected:
    std::filesystem::path temp_dir;

    void SetUp() override {
        if (SDL_Init(0) != 0) {
            FAIL() << "Failed to init SDL: " << SDL_GetError();
        }

        ResourceManager_InitPaths();
        ResourceManager_InitResources();

        // Create a temporary directory for test data
        temp_dir = std::filesystem::current_path() / "temp_rocket_test";
        std::filesystem::create_directories(temp_dir);

        // Redirect GamePref path to temp dir
        ResourceManager_FilePathGamePref = temp_dir;
    }

    void TearDown() override {
        std::filesystem::remove_all(temp_dir);
        SDL_Quit();
    }
};

TEST_F(SaveRocketTest, AnalyzeRocketLauncher) {
    std::cout << "Test Body Started: AnalyzeRocketLauncher" << std::endl;
    std::filesystem::path source_save = "/home/fred/Games/max-port/saves/SAVE2.DTA";

    if (!std::filesystem::exists(source_save)) {
        FAIL() << "Source save file not found at " << source_save;
    }

    // Copy to temp dir as valid save file for slot 2
    // SaveLoadMenu_Load expects "SAVE2.DTA" for slot 2, GAME_TYPE_CUSTOM (0)
    std::filesystem::path dest_save = temp_dir / "SAVE2.DTA";
    std::filesystem::copy_file(source_save, dest_save, std::filesystem::copy_options::overwrite_existing);

    // Load Game: Slot 2, Type 0 (Custom/.dta), ini_load_mode=true
    bool success = SaveLoadMenu_Load(2, 0, true);
    ASSERT_TRUE(success) << "Failed to load SAVE2.DTA";

    // Target coordinates from user report: 059-049
    // Note: User says 059-049. Internal coords are likely 0-indexed or 1-indexed.
    // Display usually shows 1-indexed coords? Let's assume 1-indexed and check both.
    // If user sees 059, it might be grid_x=58.
    
    int target_x = 59 - 1; 
    int target_y = 49 - 1;

    std::cout << "Searching for units at " << target_x << "," << target_y << std::endl;

    UnitInfo* found_unit = nullptr;

    // Search all units to find one at that location
    SmartList<UnitInfo>* lists[] = {
        &UnitsManager_MobileLandSeaUnits,
        &UnitsManager_StationaryUnits,
        &UnitsManager_MobileAirUnits,
        &UnitsManager_ParticleUnits, 
        &UnitsManager_GroundCoverUnits
    };

    for (int i = 0; i < 5; ++i) {
        for (auto it = lists[i]->Begin(); it != lists[i]->End(); ++it) {
            if ((*it).grid_x == target_x && (*it).grid_y == target_y) {
                 std::cout << "--- Found Unit at " << target_x << "," << target_y << " ---" << std::endl;
                 UnitInfo* unit = &(*it);
                 std::cout << "Unit Type: " << unit->GetUnitType() 
                           << " (" << UnitsManager_BaseUnits[unit->GetUnitType()].singular_name << ")" << std::endl;
                 std::cout << "Orders: " << (int)unit->GetOrder() << std::endl;
                 std::cout << "Order State: " << (int)unit->GetOrderState() << std::endl;
                 std::cout << "Target Grid: " << unit->target_grid_x << "," << unit->target_grid_y << std::endl;
                 std::cout << "Speed: " << (int)unit->speed << std::endl;
                 std::cout << "Base Speed: " << (int)unit->GetBaseValues()->GetAttribute(ATTRIB_SPEED) << std::endl;
                 std::cout << "In Transit: " << (unit->in_transit ? "YES" : "NO") << std::endl;
                 std::cout << "Path Info: " << (unit->path ? "EXISTS" : "NULL") << std::endl;
                 
                 found_unit = unit; 
            }
        }
    }

Found:
    if (!found_unit) {
        FAIL() << "No unit found at " << target_x << "," << target_y;
    }

    std::cout << "--- Unit Detailed State ---" << std::endl;
    std::cout << "Unit Type: " << found_unit->GetUnitType() << std::endl;
    std::cout << "Orders: " << (int)found_unit->GetOrder() << std::endl;
    std::cout << "Order State: " << (int)found_unit->GetOrderState() << std::endl;
    std::cout << "Target Grid: " << found_unit->target_grid_x << "," << found_unit->target_grid_y << std::endl;
    std::cout << "Speed: " << (int)found_unit->speed << std::endl;
    std::cout << "Base Speed: " << (int)found_unit->GetBaseValues()->GetAttribute(ATTRIB_SPEED) << std::endl;
    std::cout << "In Transit: " << (found_unit->in_transit ? "YES" : "NO") << std::endl;
    std::cout << "Enemy Unit: " << (found_unit->GetEnemy() ? "YES" : "NO") << std::endl;
    if (found_unit->GetEnemy()) {
        std::cout << "  Enemy Type: " << found_unit->GetEnemy()->GetUnitType() << std::endl;
        std::cout << "  Enemy Loc: " << found_unit->GetEnemy()->grid_x << "," << found_unit->GetEnemy()->grid_y << std::endl;
    }
    std::cout << "Path Info: " << (found_unit->path ? "EXISTS" : "NULL") << std::endl;
    
    // Check if it's considered mobile
    bool is_mobile = found_unit->GetBaseValues()->GetAttribute(ATTRIB_SPEED) > 0;
    std::cout << "Is Mobile (Base Speed > 0): " << (is_mobile ? "YES" : "NO") << std::endl;

    // Check mobility type
    // UnitsManager_BaseUnits[type].flags has mobile info?
    // Actually BaseValues stores attributes.
    
    // If it has a path, print start/end
    if (found_unit->path) {
        std::cout << "Path steps: ..." << std::endl;
        // Accessing path might need more headers or iterating carefully
    }
}
