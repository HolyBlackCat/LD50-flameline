#include "filesystem.h"

#include <dirent.h>
#include <sys/stat.h>

#include "finally.h"
#include "program/errors.h"

namespace Filesystem
{
    std::vector<std::string> GetDirectoryContents(const std::string &dir_name)
    {
        DIR *dir = opendir(dir_name.c_str());
        if (!dir)
            Program::Error("Unable to access directory `", dir_name, "`.");
        FINALLY( closedir(dir); )

        std::vector<std::string> ret;

        while (1)
        {
            dirent *entry = readdir(dir);
            if (!entry)
                break;

            ret.push_back(entry->d_name);
        }

        return ret;
    }

    EntryInfo GetEntryInfo(const std::string &entry_name)
    {
        struct stat info;
        if (stat(entry_name.c_str(), &info))
            Program::Error("Unable to access file `", entry_name, "`.");

        EntryInfo ret;

        switch (info.st_mode & S_IFMT)
        {
          case S_IFREG:
            ret.category = EntryCategory::file;
            break;
          case S_IFDIR:
            ret.category = EntryCategory::directory;
            break;
          default:
            ret.category = EntryCategory::other;
        }

        ret.time_modified = info.st_mtime; // `struct stat` also contains last access time and last parameter change time, but we don't really need those.

        return ret;
    }

    static TreeNode GetEntryTreeLow(const std::string &name, const std::string &path, int max_depth)
    {
        TreeNode ret;
        ret.name = name;
        ret.path = path;
        ret.info = GetEntryInfo(path);
        ret.time_modified_recursive = ret.info.time_modified;
        if (ret.info.category == directory && max_depth != 0)
        {
            std::vector<std::string> contents;

            try
            {
                contents = GetDirectoryContents(path);
            }
            catch (...) {} // Silently ignore the fact that we can no longer access the directory.

            for (const std::string &sub_name : contents)
            {
                if (sub_name == "." || sub_name == "..")
                    continue;

                try
                {
                    auto &last = ret.contents.emplace_back(GetEntryTreeLow(sub_name, path + '/' + sub_name, max_depth-1)); // We use `emplace_back` instead of `push_back` because it returns a reference.
                    if (ret.time_modified_recursive < last.time_modified_recursive)
                        ret.time_modified_recursive = last.time_modified_recursive;
                }
                catch (...) {} // Silently ignore the fact that we can no longer access the directory.
            }
        }

        return ret;
    }

    TreeNode GetEntryTree(const std::string &entry_name, int max_depth)
    {
        return GetEntryTreeLow(entry_name, entry_name, max_depth);
    }
}
