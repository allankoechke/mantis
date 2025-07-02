//
// Created by allan on 19/06/2025.
//

#include <gtest/gtest.h>
#include "test_environment.h"

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);

    // Register the global test environment
    ::testing::AddGlobalTestEnvironment(new MantisTestEnvironment);

    return RUN_ALL_TESTS();
}