#include "test.h"

#include "../entities/node.h"
#include "../helpers/logger.h"

#include <filesystem>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <string>

namespace SimpleGL {

void runLoggerSmokeTest() {
    const auto root = Node::create("LoggerTestRoot");
    Node::create("LeftChild")->setParent(root);
    Node::create("RightChild")->setParent(root);

    const auto outputPath = std::filesystem::temp_directory_path() / "simple_gl_logger_test.txt";

    Logger logger(outputPath);
    logger.logNode(root);

    std::ifstream stream(outputPath);
    if (!stream.is_open()) {
        throw std::runtime_error("Logger smoke test failed: unable to open output file.");
    }

    std::stringstream buffer;
    buffer << stream.rdbuf();
    const auto content = buffer.str();

    const std::string expected =
        "LoggerTestRoot [Transform]\n"
        "├─ LeftChild [Transform]\n"
        "└─ RightChild [Transform]\n";

    if (content != expected) {
        std::stringstream error;
        error << "Logger smoke test failed. Expected:\n"
              << expected
              << "Actual:\n"
              << content;
        throw std::runtime_error(error.str());
    }

    std::filesystem::remove(outputPath);
}

}
