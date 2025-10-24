#include "logger.h"

#include "../entities/node.h"
#include "../entities/components/component.h"

#include <stdexcept>
#include <utility>
#include <vector>

namespace SimpleGL {

Logger::Logger(std::filesystem::path outputPath): m_outputPath(std::move(outputPath)) {}

void Logger::logNode(const std::shared_ptr<Node>& node) const {
    if (!node) {
        return;
    }

    std::ofstream stream(m_outputPath);

    if (!stream.is_open()) {
        throw std::runtime_error("Logger: Unable to open output file: " + m_outputPath.string());
    }

    writeNode(node, stream, "", true, true);
}

void Logger::writeNode(
    const std::shared_ptr<Node>& node,
    std::ofstream& stream,
    const std::string& prefix,
    bool isLast,
    bool isRoot
) const {
    if (!isRoot) {
        stream << prefix << (isLast ? "└─ " : "├─ ");
    }

    stream << node->name;

    const auto components = node->components();
    if (!components.empty()) {
        stream << " [";
        for (size_t i = 0; i < components.size(); ++i) {
            stream << components[i]->name;
            if (i + 1 < components.size()) {
                stream << ", ";
            }
        }
        stream << "]";
    }

    stream << '\n';

    const auto& children = node->children();
    if (children.empty()) {
        return;
    }

    const std::string childPrefix = isRoot
        ? ""
        : prefix + (isLast ? "    " : "│   ");

    for (size_t i = 0; i < children.size(); ++i) {
        const bool childIsLast = i + 1 == children.size();
        writeNode(children[i], stream, childPrefix, childIsLast, false);
    }
}

}
