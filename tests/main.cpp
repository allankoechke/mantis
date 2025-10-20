#include <gtest/gtest.h>
#include <iostream>
#include "test_fixure.h"

int main(int argc, char* argv[])
{
    // Setup directories for tests, for each, create a unique directory
    // in which we will use for current tests then later tear it down.
    const auto baseDir = getBaseDir();
    const auto scriptingDir = (fs::path(TEST_SOURCE_DIR) / "scripting").string();
    const auto dataDir = (baseDir / "data").string();
    const auto publicDir = (baseDir / "www").string();

    char* testArgs2[] = {
        "mantisapp_test", "--database", "SQLITE",
        "--dataDir", const_cast<char*>(dataDir.c_str()),
        "--publicDir", const_cast<char*>(publicDir.c_str()),
        "--scriptsDir", const_cast<char*>(scriptingDir.c_str()),
        "serve", "--port", "7075", "--host", "0.0.0.0"
    };

    // Catch::Session session;
    // session.configData().noCapture = true;

    // Setup Db, Server, etc.
    auto& tFix = TestFixture::instance(14, testArgs2);

    // Spawn a new thread to run the
    auto serverThread = std::thread([&tFix]()
    {
        try
        {
            auto& app = tFix.mantisApp();
            [[maybe_unused]] auto res = app.run();
        }
        catch (const std::exception& e)
        {
            std::cerr << "Error Starting Server: " << e.what() << '\n';
        }
    });

    // Wait or perform tests
    std::this_thread::sleep_for(std::chrono::seconds(2));

    ::testing::InitGoogleTest(&argc, argv);
    const int result = RUN_ALL_TESTS(); // Catch::Session().run(argc, argv);

    // Clean up files, data, etc.
    tFix.teardownOnce(baseDir);

    if (serverThread.joinable())
        serverThread.join();

    return result;
}
