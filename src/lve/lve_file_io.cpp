#include "lve_file_io.hpp"

// std
#include <stdexcept>

namespace lve
{
    void checkFileOpen(const std::ifstream &file, const std::string &filename)
    {
        if (!file.is_open())
            throw std::runtime_error("failed to open file: " + filename);
    }

    void checkFileOpen(const std::ofstream &file, const std::string &filename)
    {
        if (!file.is_open())
            throw std::runtime_error("failed to open file: " + filename);
    }

    std::string readTextFile(const std::string &filename)
    {
        std::ifstream file{filename};
        checkFileOpen(file, filename);

        std::string text;
        std::string line;
        while (std::getline(file, line))
            text += line + "\n";

        return text;
    }

    std::vector<char> readBinaryFile(const std::string &filename)
    {
        std::ifstream file{filename, std::ios::binary};
        checkFileOpen(file, filename);

        std::vector<char> buffer(std::istreambuf_iterator<char>{file}, {});
        return buffer;
    }

    void writeFile(const std::string &filepath, const std::vector<char> &data)
    {
        std::ofstream file{filepath, std::ios::binary};
        checkFileOpen(file, filepath);

        file.write(data.data(), data.size());
        file.close();
    }

    bool fileExists(const std::string &filepath)
    {
        std::ifstream file{filepath};
        return file.good();
    }

    bool isFileOpen(const std::ifstream &file)
    {
        return file.is_open();
    }

    bool isFileOpen(const std::ofstream &file)
    {
        return file.is_open();
    }

    bool LveYamlConfig::isKeyDefined(const std::string &key) const
    {
        checkConfigDefined();
        return config[key].IsDefined();
    }

    void LveYamlConfig::saveConfig(const std::string &yamlFilePath)
    {
        checkConfigDefined();
    }

    void LveYamlConfig::checkKeyDefined(const std::string &key) const
    {
        if (!isKeyDefined(key))
            throw std::runtime_error("key not found in config: " + key);
    }

    void LveYamlConfig::checkConfigDefined() const
    {
        if (!isConfigDefined())
            throw std::runtime_error("config not initialized");
    }
} // namespace lve