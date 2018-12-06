#pragma once

#include <exception>
#include <string>
#include <utility>

#include "interface/messagebox.h"
#include "program/errors.h"
#include "reflection/complete.h"
#include "utils/memory_file.h"

template <typename T> class Config
{
    T object = {};

    std::string file_name;

  public:
    Config(decltype(nullptr)) {}

    explicit Config(const T &object) : object(object) {}
    explicit Config(T &&object) : object(std::move(object)) {}

    Config(const std::string &file_name) : file_name(file_name)
    {
        MemoryFile file;

        // Try loading the file;
        try
        {
            file = MemoryFile(file_name);
        }
        catch (...) {}

        if (file)
        {
            // The file was opened successfully, now we parse it.
            try
            {
                // Try parsing normally.
                auto refl = Refl::Interface(object);
                refl.from_string(file.construct_string().c_str());

                // Success, we're done.
                return;
            }
            catch (std::exception &e)
            {
                // Normal parsing failed, disabling completeness check and trying again.
                // We probably don't need to reset `object` to the default state.

                bool partial_parsing_ok = 0;
                try
                {
                    auto refl = Refl::Interface(object);
                    refl.from_string(file.construct_string().c_str(), Refl::partial);
                    partial_parsing_ok = 1;
                }
                catch (...) {}

                if (!partial_parsing_ok)
                {
                    // Parsing without completeness check failed as well. Throwing the original exception.
                    Program::Error("Unable to parse config file `", file_name, "`.\nDelete the file if you want it to be regenerated.\n", e.what());
                }

                // The file is parsed successfully, but it's incomplete. Proceeding to making a new file.
            }
        }

        // We can't open the file, unable to parse the file, or file is incomplete.
        // Creating a new file.

        auto refl = Refl::Interface(object);
        std::string obj_string = refl.to_string(4);
        try
        {
            MemoryFile::Save(file_name, (uint8_t *)obj_string.data(), (uint8_t *)obj_string.data() + obj_string.size());
        }
        catch (...) {}
    }

    const T &operator*() const {return object;}
    const T *operator->() const {return &object;}

    void Reload()
    {
        try
        {
            *this = Config(file_name);
        }
        catch (std::exception &e)
        {
            Interface::MessageBox(Interface::MessageBoxType::warning, "Unable to load config", e.what());
        }
    }
};
