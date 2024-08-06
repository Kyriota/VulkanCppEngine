#pragma once

#include "lve/util/config_manager.hpp"

namespace lve
{
    template <typename T> T YamlConfig::get(const std::string &key) const
    {
        checkKeyDefined(key);
        return config[key].as<T>();
    }

    template <typename T> void YamlConfig::set(const std::string &key, const T &value)
    {
        checkConfigDefined();
        config[key] = value;
    }
} // namespace lve