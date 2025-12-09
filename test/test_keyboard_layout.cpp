/*
 * Test Keyboard Layout
 *
 * This test verifies that the keyboard layout can be set to French and that
 * key mappings are correct (AZERTY behavior).
 */

#include <gtest/gtest.h>
#include "kb.h"
#include "gnw.h"

class KeyboardLayoutTest : public ::testing::Test {
protected:
    void SetUp() override {
        GNW_kb_set();
    }

    void TearDown() override {
        GNW_kb_restore();
    }
};

TEST_F(KeyboardLayoutTest, SetLayoutFrench) {
    // Set layout to French
    kb_set_layout(french);

    // Verify layout was set
    EXPECT_EQ(kb_get_layout(), french);

    // In AZERTY, the physical 'Q' key (SDL_SCANCODE_Q -> GNW_KB_SCAN_Q) should map to 'a'
    // First we simulate the key press
    kb_simulate_key(GNW_KB_SCAN_Q);
    
    // Then we get the character
    int32_t char_code = kb_getch();
    
    // In QWERTY (default/broken behavior), this would be 'q'
    // In AZERTY (correct behavior), this should be 'a'
    // We expect this to fail initially if the mapping is not implemented
    EXPECT_EQ(char_code, 'a') << "Expected 'a' for GNW_KB_SCAN_Q in French layout, but got '" << (char)char_code << "'";
    
    // Similarly, physical 'A' (GNW_KB_SCAN_A) should be 'q' in AZERTY
    kb_simulate_key(GNW_KB_SCAN_A);
    char_code = kb_getch();
    EXPECT_EQ(char_code, 'q') << "Expected 'q' for GNW_KB_SCAN_A in French layout, but got '" << (char)char_code << "'";
}
