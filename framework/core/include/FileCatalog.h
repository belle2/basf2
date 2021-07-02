/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
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
     * @param oldLFN If not empty, update the file catalog: only register the
     *    file if the oldLFN is found in the catalog and if so, also remove any
     *    file with the old LFN from the catalog.
     * @return True if the registration succeeded.
     */
    virtual bool registerFile(const std::string& fileName, FileMetaData& metaData, const std::string& oldLFN = "");

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
    virtual std::string getPhysicalFileName(const std::string& lfn);

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
