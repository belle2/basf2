/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <string>
#include <nlohmann/json.hpp>


namespace Belle2 {

  class FileMetaData;

  /**
   * This class provides a service for writing metadata about the basf2 execution
   * and about output files to a json file
   */
  class MetadataService {

  public:

    /**
     * Static method to get a reference to the MetadataService instance.
     *
     * @return A reference to an instance of this class.
     */
    static MetadataService& Instance();

    /**
     * Set the name of the json metadata file
     *
     * @param fileName name of the json file
     */
    void setJsonFileName(const std::string& fileName) {m_fileName = fileName; writeJson();};

    /**
     * Add the metadata of a root output file
     */
    void addRootOutputFile(const std::string& fileName, const FileMetaData* metaData = nullptr);

    /**
     * Add the metadata of a root ntuple file
     */
    void addRootNtupleFile(const std::string& fileName);

    /**
     * Add metadata of basf2 status
     */
    void addBasf2Status(const std::string& message = "");

    /**
     * Add the metadata of number of calls of all modules
     */
    void addModuleCallsCount();

    /**
     * Add metadata for basf2 completion
     */
    void finishBasf2(bool success = true);

  private:

    std::string    m_fileName;  /**< The name of the json file. */
    nlohmann::json m_json;      /**< The json object */
    double m_basf2StartTime;    /**< the start time of basf2 */

    /**
     *  Constructor
     */
    MetadataService();

    /**
     * Disable/Hide the copy constructor.
     */
    MetadataService(const MetadataService&) = delete;

    /**
     * Disable/Hide the copy assignment operator.
     */
    MetadataService& operator=(const MetadataService&) = delete;

    /**
     * Serialize the current json content to the json file
     */
    void writeJson();

  };

} //end of namespace Belle2
