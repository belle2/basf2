/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013-2018 Belle II Collaboration                          *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Christian Pulvermacher, Thomas Kuhr, Martin Ritter       *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <string>
#include <fstream>

namespace Belle2 {
  /** Utility functions related to filename validation and filesystem access */
  class FileSystem {
  public:
    /**
     * Search for given file or directory in local or central release directory,
     * and return absolute path if found.
     * If the file isn't found in either of these, absolute paths and paths
     * relative to the current working directory are also accepted.
     *
     * It is recommended to replace any relative paths you have with the
     * return value of this function during initialization.
     *
     * @param path path to file/directory, assuming it's installed locally
     *             (e.g. /data/geometry/Belle2.xml). Leading slash is not
     *             strictly required.
     * @param silent If true, no error message is printed when file could
     *             not be found.
     * @return absolute path to file in local directory, if it exists,
     *         otherwise abs. path to file in central release directory,
     *         or empty string if file wasn't found.
     */
    static std::string findFile(const std::string& path, bool silent = false);

    /** Check if the file with given filename exists */
    static bool fileExists(const std::string& filename);
    /** Check if the dir containing the filename exists */
    static bool fileDirExists(const std::string& filename);
    /** Check if filename points to an existing file */
    static bool isFile(const std::string& filename);
    /** Check if filename points to an existing directory */
    static bool isDir(const std::string& filename);

    /** calculate the MD5 checksum of a given file */
    static std::string calculateMD5(const std::string& filename);

    /**
     * Load a shared library.
     *
     * Symbols are resolved only when used, slightly reducing loading time (RTLD_LAZY, see dlopen(3))
     *
     * @param library  Name of the library
     * @param fullname If false, the Library name is interpreted as short name
     *                 like given to the compiler with -l. It will be expanded
     *                 to lib<library>.so
     */
    static bool loadLibrary(std::string library, bool fullname = true);

    /**
     * Helper class for locking a file
     */
    class Lock {
    public:

      /**
       * Construct a Lock object for the given file
       * @param fileName  Name of the file to be locked (if it does not exist, it will be created)
       * @param readonly  create a read-only lock (multiple processes can hold one)
       */
      explicit Lock(std::string fileName, bool readonly = false);

      /**
       * Destructor. Releases the lock
       */
      ~Lock();

      /**
       * Try to lock the file
       * @note Locks are not exclusive inside the same process, i.e. lock() will succeed even if a lock
       *       is already held by the current process.
       * @param timeout  Time in seconds to wait for a lock (default is rather high to deal with slow FS at KEKCC)
       * @param ignoreErrors if true just return if locking was unsuccessful but don't print an error
       * @return  True if the lock could be obtained, false if file could not be opened or timeout is reached
       */
      bool lock(int timeout = 300, bool ignoreErrors = false);

    private:
      int m_file;  /**< File descriptor of file to be locked */
      bool m_readOnly; /**< if this is a read-only lock (multiple processes can hold one). */
    };

    /** Helper file to create a temporary file and ensure deletion if object goes out of scope */
    class TemporaryFile: public std::fstream {
    public:
      /** construct a new temporary file */
      explicit TemporaryFile(std::ios_base::openmode mode = std::ios_base::trunc | std::ios_base::out);
      /** prevent assignment */
      TemporaryFile(const TemporaryFile&) = delete;
      /** but allow move construction */
      TemporaryFile(TemporaryFile&&) = default;
      /** prevent copy constructor */
      TemporaryFile& operator=(const TemporaryFile&) = delete;
      /** close file and delete on destruction */
      ~TemporaryFile();
      /** get filename of the temporary file */
      std::string getName() const { return m_filename; }
    private:
      /** filename of the temporary file */
      std::string m_filename;
    };

  private:
    /** no instances. */
    FileSystem() = delete;
  };
}
