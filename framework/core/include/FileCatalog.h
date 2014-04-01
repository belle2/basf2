/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Thomas Kuhr                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef FILECATALOG_H
#define FILECATALOG_H

#include <framework/dataobjects/FileMetaData.h>

#include <map>
#include <vector>


namespace Belle2 {

  /**
   * This class provides an interface to the file (metadata) catalog.
   */
  class FileCatalog {

  public:

    /**
     * Static method to get a reference to the FileCatalog instance.
     *
     * @return A reference to an instance of this class.
     */
    static FileCatalog& Instance();

    /**
     * Register a file in the (local) file catalog.
     *
     * @param fileName The name of the file to be registered.
     * @param metaData The meta data information of the file to be registered. Will be updated.
     * @return True if the registration succeeded.
     */
    virtual bool registerFile(std::string fileName, FileMetaData& metaData);

    /**
     * Get the metadata of a file with given ID.
     *
     * @param id The unique identifier of the file.
     * @param metaData The meta data information of the file.
     * @return True if the file was found in the catalog.
     */
    virtual bool getMetaData(int id, FileMetaData& metaData);

    /**
     * Get the metadata of a file with given LFN.
     *
     * @param lfn The logical file name of the file.
     * @param metaData The meta data information of the file.
     * @return True if the file was found in the catalog.
     */
    virtual bool getMetaData(std::string lfn, FileMetaData& metaData);

    /**
     * Structure for metadata information of parent files
     */
    struct ParentMetaDataEntry {
      FileMetaData metaData;   /**< The metadata of the parent. */
      std::vector<ParentMetaDataEntry> parents;   /**< The grandparents. */
    };
    typedef std::vector<ParentMetaDataEntry> ParentMetaData;   /**< Vector of parent metadata entries. */

    /**
     * Get the metadata of ancestors of a file with given metadata or ID.
     *
     * @param level The number of generations.
     * @param id The unique identifier of the file. It will only be used if no metadata is given.
     * @param metaData The meta data information of the file.
     * @param parentMetaData The meta data information of the ancestors.
     * @return True if the ancestors information could be obtained from the catalog.
     */
    virtual bool getParentMetaData(int level, int id, FileMetaData& metaData, ParentMetaData& parentMetaData);

  private:

    /**
     * Helper class for locking the file catalog
     */
    class Lock {
    public:

      /**
       * Construct a Lock object for the given file
       * @param fileName Name of the file to be locked
       */
      Lock(std::string fileName);

      /**
       * Destructor. Releases the lock
       */
      ~Lock();

      /**
       * Try to lock the file
       * @param timeout Time in seconds until it is tried to get a lock
       * @return True if the lock could be obtained
       */
      bool lock(int timeout = 20);

    private:
      int m_file;  /**< File descriptor of file to be locked */
    };

    /**
     * Constructor: locate local database file
     */
    FileCatalog();

    typedef std::map<int, FileMetaData> FileMap;   /**< Map with file catalog content. */

    /**
     * Read the file catalog from the local file.
     *
     * @param fileMap The map of file catalog entries.
     * @return True if the catalog was read successfully.
     */
    bool readCatalog(FileMap& fileMap);

    /**
     * Write the file catalog to the local file.
     *
     * @param fileMap The map of file catalog entries.
     * @return True if the catalog was written successfully.
     */
    bool writeCatalog(const FileMap& fileMap);

    /**
     * Get the metadata of parents of a file with given metadata.
     *
     * @param fileMap The map of file catalog entries.
     * @param level The number of generations.
     * @param metaData The meta data information of the file.
     * @param parentMetaData The meta data information of the ancestors.
     * @return True if the ancestors information could be obtained from the catalog.
     */
    bool getParents(const FileMap& fileMap, int level, const FileMetaData& metaData, ParentMetaData& parentMetaData);

    std::string m_fileName;   /**< Name of the file catalog file. */
  };

} //end of namespace Belle2

#endif /*FILECATALOG_H */
