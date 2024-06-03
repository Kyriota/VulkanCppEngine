#pragma once

// libs
#include "include/yaml.hpp"

// std
#include <fstream>
#include <string>
#include <vector>
#include <stdexcept>

namespace lve
{
    namespace io
    {
        std::string readTextFile(const std::string &filename);
        std::vector<char> readBinaryFile(const std::string &filename);

        void writeFile(const std::string &filepath, const std::vector<char> &data);
        void writeFile(const std::string &filepath, const std::string &data);

        bool fileExists(const std::string &filepath);
        bool isFileOpen(const std::ifstream &file);
        bool isFileOpen(const std::ofstream &file);

        class YamlConfig
        {
        public:
            YamlConfig() = default;
            YamlConfig(const std::string &yamlFilePath) : config{YAML::LoadFile(yamlFilePath)} {}

            YamlConfig(const YamlConfig &) = delete;
            YamlConfig &operator=(const YamlConfig &) = delete;

            bool isConfigDefined() const { return config.IsDefined(); }
            bool isKeyDefined(const std::string &key) const;
            void loadConfig(const std::string &yamlFilePath) { config = YAML::LoadFile(yamlFilePath); }
            void saveConfig(const std::string &yamlFilePath);

            template <typename T>
            T get(const std::string &key) const;

            template <typename T>
            void set(const std::string &key, const T &value);

        private:
            YAML::Node config;

            void checkKeyDefined(const std::string &key) const;
            void checkConfigDefined() const;
        };
    } // namespace io
} // namespace lve

#include "lve/util/file_io.tpp"