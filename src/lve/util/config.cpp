#include "config.hpp"

// lve
#include "lve/path.hpp"
#include "lve/util/file_io.hpp"

namespace lve
{
bool YamlConfig::isKeyDefined(const std::string &key) const
{
    checkConfigDefined();
    return config[key].IsDefined();
}

void YamlConfig::saveConfig(const std::string &outputPath) const
{
    checkConfigDefined();
    io::writeFile(outputPath, YAML::Dump(config));
}

void YamlConfig::checkKeyDefined(const std::string &key) const
{
    if (!isKeyDefined(key))
    {
        throw std::runtime_error("key not found in config: " + key);
    }
}

void YamlConfig::checkConfigDefined() const
{
    if (!isConfigDefined())
    {
        throw std::runtime_error("config not initialized");
    }
}

const YamlConfig &ConfigManager::getConfig(const std::string &configReletivePath)
{
    static const ConfigManager instance;
    size_t hash = std::hash<std::string>{}(configReletivePath);
    return instance.configs.at(hash).config;
}

void ConfigManager::saveConfig(const std::string &configReletivePath, const std::string &outputPath)
{
    static const ConfigManager instance;
    size_t hash = std::hash<std::string>{}(configReletivePath);
    const YamlConfig &config = instance.configs.at(hash).config;
    config.saveConfig(outputPath);
}

void ConfigManager::reloadConfig(const std::string &configReletivePath)
{
    static ConfigManager instance;
    size_t hash = std::hash<std::string>{}(configReletivePath);
    instance.configs[hash].config.loadConfig(configReletivePath);
}

ConfigManager::ConfigManager()
{
    io::foreachFileInDirectory(
        lve::path::config::ROOT, [this](const std::filesystem::directory_entry &entry) {
            std::string configReletivePath = entry.path().string();
            size_t hash = std::hash<std::string>{}(configReletivePath);
            configs[hash] = ConfigWrapper{YamlConfig{configReletivePath}, configReletivePath};
        });
}
} // namespace lve