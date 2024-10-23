#pragma once

// std
#include <filesystem>
#include <functional>
#include <string>
#include <vector>

namespace lve::io
{
void readTextFile(const std::string &filename, std::string &text);
void readBinaryFile(const std::string &filename, std::vector<char> &buffer);
void readBinaryFile(
    const std::string &filename, std::vector<uint32_t> &buffer, bool littleEndian = true);

void writeFile(const std::string &filePath, const std::vector<char> &data);
void writeFile(const std::string &filePath, const std::string &data);

void foreachFileInDirectory(
    const std::string &dir,
    std::function<void(const std::filesystem::directory_entry &)> callback);
} // namespace lve::io