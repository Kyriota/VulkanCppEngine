#pragma once

// libs
#include "include/yaml.hpp"

// std
#include <string>
#include <unordered_map>

namespace lve
{
    class YamlConfig
    {
    public:
        YamlConfig() = default;
        YamlConfig(const std::string &yamlFilePath) : config{YAML::LoadFile(yamlFilePath)} {}

        bool isConfigDefined() const { return config.IsDefined(); }
        bool isKeyDefined(const std::string &key) const;
        void loadConfig(const std::string &inputFilePath)
        {
            config = YAML::LoadFile(inputFilePath);
        }
        void saveConfig(const std::string &outputPath) const;

        template <typename T> T get(const std::string &key) const;

        template <typename T> void set(const std::string &key, const T &value);

    private:
        YAML::Node config;

        inline void checkKeyDefined(const std::string &key) const
        {
            if (!isKeyDefined(key))
            {
                throw std::runtime_error("key not found in config: " + key);
            }
        }

        inline void checkConfigDefined() const
        {
            if (!isConfigDefined())
            {
                throw std::runtime_error("config not initialized");
            }
        }
    };

    class ConfigManager // Singleton class
    {
    public:
        struct ConfigWrapper
        {
            YamlConfig config;
            std::string path;
        };

        static const YamlConfig &getConfig(const std::string &configReletivePath);
        static void
        saveConfig(const std::string &configReletivePath, const std::string &outputPath);
        static void reloadConfig(const std::string &configReletivePath);

    private:
        ConfigManager();

        std::unordered_map<size_t, ConfigWrapper> configs;
    };
} // namespace lve

#include "lve/util/config_manager.tpp"