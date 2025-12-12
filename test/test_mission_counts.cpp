#include <gtest/gtest.h>
#include <SDL.h>
#include "resource_manager.hpp"
#include "missionregistry.hpp"
#include "enums.hpp"
#include "game_manager.hpp"
#include "gnw.h"

class MissionCountsTest : public ::testing::Test {
protected:
    void SetUp() override {
        if (SDL_Init(0) != 0) {
            FAIL() << "Failed to init SDL: " << SDL_GetError();
        }
        ResourceManager_InitPaths();
        ResourceManager_InitResources();
    }
    
    void TearDown() override {
        SDL_Quit();
    }
};

TEST_F(MissionCountsTest, PrintMissionCounts) {
    auto* mgr = ResourceManager_GetMissionManager();
    ASSERT_NE(mgr, nullptr) << "MissionManager should be initialized";

    printf("Category Counts:\n");
    printf("TRAINING (0): %zu\n", mgr->GetMissions(MISSION_CATEGORY_TRAINING).size());
    printf("CAMPAIGN (1): %zu\n", mgr->GetMissions(MISSION_CATEGORY_CAMPAIGN).size());
    printf("DEMO (2): %zu\n", mgr->GetMissions(MISSION_CATEGORY_DEMO).size());
    printf("SCENARIO (3): %zu\n", mgr->GetMissions(MISSION_CATEGORY_SCENARIO).size());
    printf("MULTI (4): %zu\n", mgr->GetMissions(MISSION_CATEGORY_MULTI_PLAYER_SCENARIO).size());
    
    printf("GAME_TYPE_CAMPAIGN (2) -> MissionCategory(2) [DEMO]: %zu\n", mgr->GetMissions((MissionCategory)2).size());
}
