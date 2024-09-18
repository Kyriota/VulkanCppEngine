#include "lve/util/file_io.hpp"

namespace lve
{
    namespace io
    {
        void checkFileOpen(const std::ifstream &file, const std::string &filename)
        {
            if (!file.is_open())
            {
                throw std::runtime_error("Failed to open file: " + filename);
            }
        }

        void checkFileOpen(const std::ofstream &file, const std::string &filename)
        {
            if (!file.is_open())
            {
                throw std::runtime_error("Failed to open file: " + filename);
            }
        }

        void readTextFile(const std::string &filename, std::string &text)
        {
            std::ifstream file{filename};
            checkFileOpen(file, filename);

            std::string line;
            while (std::getline(file, line))
                text += line + "\n";
        }

        void readBinaryFile(const std::string &filename, std::vector<char> &buffer)
        {
            std::ifstream file{filename, std::ios::binary};
            checkFileOpen(file, filename);

            buffer = std::vector<char>(std::istreambuf_iterator<char>{file}, {});
        }

        void readBinaryFile(
            const std::string &filename,
            std::vector<uint32_t> &buffer,
            bool littleEndian
        )
        {
            std::ifstream file{filename, std::ios::binary};
            checkFileOpen(file, filename);

            file.seekg(0, std::ios::end);
            size_t fileSize = file.tellg();
            file.seekg(0, std::ios::beg);

            buffer = std::vector<uint32_t>(fileSize / sizeof(uint32_t));

            // Combine bytes into 32-bit integers
            if (littleEndian)
            {
                for (size_t i = 0; i < fileSize; i += sizeof(uint32_t))
                {
                    uint32_t word = 0;
                    for (size_t j = 0; j < sizeof(uint32_t); ++j)
                    {
                        word |= file.get() << (j * 8);
                    }
                    buffer[i / sizeof(uint32_t)] = word;
                }
            }
            else
            {
                for (size_t i = 0; i < fileSize; i += sizeof(uint32_t))
                {
                    uint32_t word = 0;
                    for (size_t j = 0; j < sizeof(uint32_t); ++j)
                    {
                        word = (file.get() << 24) | (word >> 8);
                    }
                    buffer[i / sizeof(uint32_t)] = word;
                }
            }
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

        void foreachFileInDirectory(
            const std::string &dir,
            std::function<void(const std::filesystem::__cxx11::directory_entry &)> callback
        )
        {
            if (!pathExists(dir))
            {
                throw std::runtime_error("Directory does not exist: " + dir);
            }

            for (const auto &entry : std::filesystem::directory_iterator(dir))
            {
                if (entry.is_regular_file())
                {
                    const std::filesystem::__cxx11::directory_entry &entryRef = entry;
                    callback(entryRef);
                }
            }
        }
    } // namespace io
} // namespace lve