#include "exe_path.h"

#include <algorithm>
#include <filesystem>

#include "program/platform.h"
#include "strings/format.h"

#if IMP_PLATFORM_IS(windows)
#include <windows.h>
#else
#include <unistd.h>
#endif

namespace Utils
{
    const std::string &ExeFilename()
    {
        static const std::string ret = []{
            IMP_PLATFORM_IF(windows)
            (
                std::wstring ret(MAX_PATH + 1, '\0');
            )
            IMP_PLATFORM_IF_NOT(windows)
            (
                std::string ret(PATH_MAX + 1, '\0');
            )

            while (true)
            {
                IMP_PLATFORM_IF(windows)
                (
                    auto len = GetModuleFileNameW(nullptr, ret.data(), ret.size());
                )
                IMP_PLATFORM_IF_NOT(windows)
                (
                    auto len = readlink("/proc/self/exe", ret.data(), ret.size());
                )
                if (len <= 0)
                    throw std::runtime_error("Unable to get the executable path from `/proc/self/exe`.");
                if (std::size_t(len) >= ret.size())
                {
                    // The buffer is too short.
                    ret.resize(ret.size() * 2);
                    continue;
                }

                ret.resize(len);
                IMP_PLATFORM_IF(windows)
                (
                    // Convert to UTF-8.
                    auto ret_converted = std::filesystem::path(std::move(ret)).string();
                    // Replace `\` with `/`.
                    std::replace(ret_converted.begin(), ret_converted.end(), '\\', '/');
                    return ret_converted;
                )
                IMP_PLATFORM_IF_NOT(windows)
                (
                    return ret;
                )
            }
        }();
        return ret;
    }

    const std::string &ExeDir()
    {
        static const std::string ret = []{
            const std::string &exe_path = ExeFilename();
            auto sep = exe_path.find_last_of('/');
            if (sep == exe_path.npos)
                throw std::runtime_error(FMT("Unable to determine the executable directory: No slash in the path returned by `/proc/self/exe`: `{}`.", exe_path));
            return exe_path.substr(0, sep);
        }();
        return ret;
    }
}
