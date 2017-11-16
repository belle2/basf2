/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Thomas Kuhr                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <map>
#include <string>

namespace Belle2 {
  class FileMetaData;

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
     * Get the metadata of a file with given (logical) file name.
     *
     * @param fileName The (logical) name of the file. Will be set to the physical file name.
     * @param metaData The meta data information of the file.
     * @return True if the file was found in the catalog.
     */
    virtual bool getMetaData(std::string& fileName, FileMetaData& metaData);

    /**
     * Get the physical file name for the LFN.
     *
     * @param lfn The logical file name.
     * @return the physical file name or an empty string if the lfn is not in the catalog.
     */
    virtual std::string getPhysicalFileName(std::string lfn);

  private:

    /**
     * Constructor: locate local database file
     */
    FileCatalog();

    typedef std::map<std::string, std::pair<std::string, FileMetaData>> FileMap;   /**< Map with file catalog content. */

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

    std::string m_fileName;   /**< Name of the file catalog file. */
  };

} //end of namespace Belle2
