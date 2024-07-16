#pragma once

#include "lve/util/file_io.hpp"

namespace lve
{
    namespace io
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
    } // namespace io
} // namespace lve