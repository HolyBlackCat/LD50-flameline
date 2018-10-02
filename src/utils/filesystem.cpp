#include "filesystem.h"

#include <dirent.h>
#include <sys/stat.h>

#include "finally.h"
#include "program/errors.h"

namespace Filesystem
{
    ObjInfo GetObjectInfo(const std::string &entry_name)
    {
        struct stat info;
        if (stat(entry_name.c_str(), &info))
            Program::Error("Unable to access file or directory `", entry_name, "`.");

        ObjInfo ret;

        switch (info.st_mode & S_IFMT)
        {
          case S_IFREG:
            ret.category = file;
            break;
          case S_IFDIR:
            ret.category = directory;
            break;
          default:
            ret.category = other;
        }

        ret.time_modified = info.st_mtime; // `struct stat` also contains last access time and last parameter change time, but we don't really need those.

        return ret;
    }

    std::vector<std::string> GetDirectoryContents(const std::string &dir_name)
    {
        DIR *dir = opendir(dir_name.c_str());
        if (!dir)
            Program::Error("Unable to access directory `", dir_name, "`.");
        FINALLY( closedir(dir); )

        std::vector<std::string> ret;

        while (1)
        {
            dirent *entry = readdir(dir); // `entry` doesn't need to be free'd.
            if (!entry)
                break;

            ret.push_back(entry->d_name);
        }

        return ret;
    }

    static TreeNode GetObjectTreeLow(const std::string &name, const std::string &path, int max_depth)
    {
        TreeNode ret;
        ret.name = name;
        ret.path = path;
        ret.info = GetObjectInfo(path);
        ret.time_modified_recursive = ret.info.time_modified;
        if (ret.info.category == directory && max_depth != 0)
        {
            std::vector<std::string> contents;

            try
            {
                contents = GetDirectoryContents(path);
            }
            catch (...) {} // Silently ignore the error if we can no longer access the directory.

            for (const std::string &sub_name : contents)
            {
                if (sub_name == "." || sub_name == "..")
                    continue;

                try
                {
                    ret.contents.push_back(GetObjectTreeLow(sub_name, path + '/' + sub_name, max_depth-1));

                    std::time_t time_modified = ret.contents.back().time_modified_recursive;
                    if (time_modified > ret.time_modified_recursive)
                        ret.time_modified_recursive = time_modified;
                }
                catch (...) {} // Silently ignore the error if we can no longer access the directory.
            }
        }

        return ret;
    }

    TreeNode GetObjectTree(const std::string &entry_name, int max_depth)
    {
        return GetObjectTreeLow(entry_name, entry_name, max_depth);
    }
}
