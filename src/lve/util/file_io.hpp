#pragma once

// std
#include <filesystem>
#include <fstream>
#include <functional>
#include <stdexcept>
#include <string>
#include <vector>

namespace lve
{
    namespace io
    {
        void readTextFile(const std::string &filename, std::string &text);
        void readBinaryFile(const std::string &filename, std::vector<char> &buffer);
        void readBinaryFile(
            const std::string &filename,
            std::vector<uint32_t> &buffer,
            bool littleEndian = true
        );

        void writeFile(const std::string &filePath, const std::vector<char> &data);
        void writeFile(const std::string &filePath, const std::string &data);

        inline bool fileExists(const std::string &filePath)
        {
            return std::filesystem::is_regular_file(filePath);
        }
        inline bool pathExists(const std::string &path)
        {
            return std::filesystem::is_directory(path);
        }
        inline bool isFileOpen(const std::ifstream &file) { return file.is_open(); }
        inline bool isFileOpen(const std::ofstream &file) { return file.is_open(); }

        void foreachFileInDirectory(
            const std::string &dir,
            std::function<void(const std::filesystem::__cxx11::directory_entry &)> callback
        );
    } // namespace io
} // namespace lve