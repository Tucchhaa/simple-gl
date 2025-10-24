#pragma once

#include <filesystem>
#include <fstream>
#include <memory>
#include <string>

namespace SimpleGL {

class Node;

class Logger {
public:
    explicit Logger(std::filesystem::path outputPath);

    void logNode(const std::shared_ptr<Node>& node) const;

private:
    void writeNode(
        const std::shared_ptr<Node>& node,
        std::ofstream& stream,
        const std::string& prefix,
        bool isLast,
        bool isRoot
    ) const;

    std::filesystem::path m_outputPath;
};

}
