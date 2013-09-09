#pragma once

#include <string>

namespace Belle2 {
  /** Utility functions related to filename validation and filesystem access */
  namespace FileSystem {
    /** Check if a given filename exits */
    bool fileExists(const std::string& filename);
    /** Check if the dir containing the filename exists */
    bool fileDirExists(const std::string& filename);
    /** Check if filename points to an existing file */
    bool isFile(const std::string& filename);
    /** Check if filename points to an existing directory */
    bool isDir(const std::string& filename);

    /**
     * Load a shared library
     * @param library  Name of the library
     * @param fullname If false, the Library name is interpreted as short name
     *                 like given to the compiler with -l. It will be expanded
     *                 to lib<library>.so
     */
    bool loadLibrary(std::string library, bool fullname = true);

    /**
     * Search for given file in local or central release directory and return
     * absolute path if found.
     *
     * @param path path to file/directory, assuming it's installed locally
     *             (e.g. /data/geometry/Belle2.xml). Leading slash is not
     *             strictly required.
     * @return absolute path to file in local directory, if it exists,
     *         otherwise abs. path to file in central release directory,
     *         or empty if file wasn't found.
     */
    std::string findFile(const std::string& path);
  }

}
