/*
 * Copyright (c) 2025 M.A.X. Port Team
 *
 * Reproduces the save game regression where SAVE7.DTA is missing.
 */

#include <gtest/gtest.h>
#include <filesystem>

#include "saveloadmenu.hpp"
#include "resource_manager.hpp"
#include "game_manager.hpp"
#include "units_manager.hpp"
#include "inifile.hpp"

class SaveRegressionTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create a unique temporary directory for this test run
        temp_dir = std::filesystem::temp_directory_path() / "max_test_save_regression";
        std::filesystem::create_directories(temp_dir);

        // Point ResourceManager to this directory
        ResourceManager_FilePathGamePref = temp_dir;
        ResourceManager_FilePathGameData = temp_dir;

        // Initialize minimal game state needed for saving.
        // Full initialization (ResourceManager_InitTeamInfo, ini_config.Init) is skipped 
        // as it requires a display environment and full resource files which are not present 
        // in this isolated test context. Defaults are sufficient for verifying file creation logic.
        
        ini_set_setting(INI_GAME_FILE_TYPE, GAME_TYPE_CUSTOM);
    }

    void TearDown() override {
        std::filesystem::remove_all(temp_dir);
    }

    std::filesystem::path temp_dir;
};

TEST_F(SaveRegressionTest, SaveSlot7_CreatesUppercaseFile) {
    const char* filename_input = "save7.dta"; // What SaveLoadMenu passes (derived from Sprintf)
    const char* save_name = "Test Save";
    
    // Execute the function under test
    SaveLoadMenu_Save(filename_input, save_name, false, false);
    
    // Verify expectation: Filesystem should contain SAVE7.DTA
    std::filesystem::path expected_path = temp_dir / "SAVE7.DTA";

    EXPECT_TRUE(std::filesystem::exists(expected_path)) << "File SAVE7.DTA should exist";
    
    // Also check that it's NOT lowercase "save7.dta" (unless filesystem is case insensitive, but standard behavior is requested)
    EXPECT_FALSE(std::filesystem::exists(temp_dir / "save7.dta")) << "File save7.dta (lowercase) should NOT exist";
}
