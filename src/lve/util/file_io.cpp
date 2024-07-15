#include "lve/util/file_io.hpp"

// std
#include <stdexcept>

namespace lve
{
    namespace io
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

        void writeFile(const std::string &filePath, const std::vector<char> &data)
        {
            std::ofstream file{filePath, std::ios::binary};
            checkFileOpen(file, filePath);

            file.write(data.data(), data.size());
            file.close();
        }

        void writeFile(const std::string &filePath, const std::string &data)
        {
            std::ofstream file{filePath};
            checkFileOpen(file, filePath);

            file << data;
            file.close();
        }

        bool fileExists(const std::string &filePath)
        {
            std::ifstream file{filePath};
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

        bool YamlConfig::isKeyDefined(const std::string &key) const
        {
            checkConfigDefined();
            return config[key].IsDefined();
        }

        void YamlConfig::saveConfig(const std::string &yamlFilePath)
        {
            checkConfigDefined();
            writeFile(yamlFilePath, YAML::Dump(config));
        }

        void YamlConfig::checkKeyDefined(const std::string &key) const
        {
            if (!isKeyDefined(key))
                throw std::runtime_error("key not found in config: " + key);
        }

        void YamlConfig::checkConfigDefined() const
        {
            if (!isConfigDefined())
                throw std::runtime_error("config not initialized");
        }
    } // namespace io
} // namespace lve