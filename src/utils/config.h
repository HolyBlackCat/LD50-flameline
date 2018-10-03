#pragma once

#include <exception>
#include <string>
#include <utility>

#include "program/errors.h"
#include "reflection/complete.h"
#include "utils/memory_file.h"

template <typename T> class Config
{
    T object;

  public:
    Config(decltype(nullptr)) : object() {}

    explicit Config(const T &object) : object(object) {}
    explicit Config(T &&object) : object(std::move(object)) {}

    Config(const std::string &file_name)
    {
        MemoryFile file;

        // Try loading the file;
        try
        {
            file = MemoryFile(file_name);
            // Loaded successfully, proceeding to parsing.
        }
        catch (std::exception &e)
        {
            // Failed to load the file. Make a new one.

            std::string obj_string = Refl::Interface(object).to_string(4);
            try
            {
                MemoryFile::Save(file_name, (uint8_t *)obj_string.data(), (uint8_t *)obj_string.data() + obj_string.size());
            }
            catch (...) {}

            return; // The new file is created, so we stop.
        }

        // The file was opened successfully, now we parse it.
        try
        {
            Refl::Interface(object).from_string(file.construct_string().c_str());
        }
        catch (std::exception &e)
        {
            Program::Error("Unable to parse config file `", file_name, "`\nDelete the file if you want it to be regenerated.\n", e.what());
        }
    }

    const T &operator*() const {return object;}
    const T *operator->() const {return &object;}
};
