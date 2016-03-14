/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jeter Hall,                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef ConditionsService_H
#define ConditionsService_H

#include <TList.h>
#include <TObject.h>
#include <TXMLEngine.h>
#include <string>
#include <map>
#include <framework/core/Module.h>
#include <curl/curl.h>

namespace Belle2 {

  /** The conditionsPayload structure holds the information related to payloads. */
  struct conditionsPayload {

    /** The package name. */
    std::string package;

    /** The module name. */
    std::string module;

    /** The earliest run for the IOV. */
    std::string runInitial;

    /** The experiment for the earliest run in the IOV. */
    std::string expInitial;

    /** The last run for the IOV. */
    std::string runFinal;

    /** The experiment for the last run in the IOV. */
    std::string expFinal;

    /** The checksum for the payload */
    std::string md5Checksum;

    /** The relative path and file name for the payload */
    std::string logicalFileName;

  };


  /** The conditions service will store and retrieve conditions payloads.  This service is under construction and currently needs
   *  particular file directories and database permissions which will eventually be generalized for distributed use.  Currently
   *  must be used at hep.pnnl.gov.
   *
   * Details   Any TObject can be stored by the conditions service for later retrieval.  Storage is based on associating
   *           a payload with a Subsystem Tag, Algorithm Name, and Algorithm Version.  An example is the 'itop' subsystem,
   *           'pedestal' algorithm, and '1_0_0_1' is an example algorithm version.  This service is currently implemented as
   *           a singleton.
   *
   */
  class ConditionsService {

  public:

    /** Get a pointer to the ConditionsService instance */
    static ConditionsService* getInstance();



    /** Read back payloads associated with a particular global tag.
     *  @param GlobalTag A global tag is an approved set of payload tags.
     *
     *  @return Nothing yet
     */
    void getPayloads(std::string GlobalTag, std::string ExperimentName, std::string RunName);

    /** Sets the base name for the REST services.  Example is http://belle2db.hep.pnnl.gov/b2s/rest/v1/.
     *  @param RESTBaseName The base name for the REST services.  Example is http://belle2db.hep.pnnl.gov/b2s/rest/v1/.
     *
     *  @return Nothing yet
     */
    void setRESTbasename(std::string RESTBaseName) {m_RESTbase = RESTBaseName;};

    /** Sets the base name for the conditions files.  Example is http://belle2db.hep.pnnl.gov/.
     *  @param FILEBaseName The base name for the conditions files.  Example is http://belle2db.hep.pnnl.gov/.
     *
     *  @return Nothing yet
     */
    void setFILEbasename(std::string FILEBaseName) {m_FILEbase = FILEBaseName;};

    /** Sets the local directory name for copies of the conditions files.  Example is /tmp/.
     *  @param FILEBaseLocal The base name for the conditions files.  Example is http://belle2db.hep.pnnl.gov/.
     *
     *  @return Nothing yet
     */
    void setFILEbaselocal(std::string FILEBaseLocal) {m_FILElocal = FILEBaseLocal;};


    /** Check to see if a payload key is already registered in the payload map.
     *  @param PackageModuleName The concatenation of the package.name and module.name.
     *
     *  @return true if payload exists, false if not.
     */
    bool payloadExists(std::string PackageModuleName) {return (m_payloads.find(PackageModuleName) != m_payloads.end());}

    /** Adds a payload to the payload map.
     *  @param PackageModuleName The concatenation of the package.name and module.name.
     *  @param payload The payload information data structure.
     *
     *  @return Nothing yet
     */
    void addPayloadInfo(std::string PackageModuleName, conditionsPayload payload) {m_payloads[PackageModuleName] = payload;};

    /** Get payload information from the payload map.
     *
     *  @param packageName A string identifier for the payload type.  Does not necessarily need to be the BASF2 package.
     *  @param moduleName A string identifier for the payload type.  Does not necessarily need to be the BASF2 module.
     *
     *  @return conditionsPayload The payload information data structure. NULL if payload not found.
     */
    conditionsPayload getPayloadInfo(std::string packageName,
                                     std::string moduleName) {return getPayloadInfo(packageName + moduleName);};

    /** Get payload information from the payload map.
     *  @param PackageModuleName The concatenation of the package.name and module.name.
     *
     *  @return conditionsPayload The payload information data structure. NULL if payload not found.
     */
    conditionsPayload getPayloadInfo(std::string PackageModuleName);

    /** Get payload information map.
     *  @param PackageModuleName The concatenation of the package.name and module.name.
     *
     *  @return std::map<std::string, conditionsPayload> The payload information map.  The key is the concatenation of package and module.  NULL if payload not found.
     */
    std::map<std::string, conditionsPayload> getPayloadInfoMap(void) {return m_payloads;};

    /** Adds a payload file to the conditions database.
     *  @param payloadFileName The file name of the payload.
     *  @param module The module requesting payload storage (used for the 'module' and 'package' payload tags).
     *
     *  @return Nothing yet
     */
    void writePayloadFile(std::string payloadFileName,
                          Module* module) {writePayloadFile(payloadFileName, module->getPackage(), module->getName());};

    /** Adds a string to the buffer (for REST returns.)
     *  @param buffer The string to be added to the buffer.
     *
     *  @return Nothing yet
     */
    void addReturn(std::string buffer) {m_buffer = m_buffer + buffer;};

    /** Adds a payload file to the conditions database.
     *  @param payloadFileName The file name of the payload.
     *  @param packageName A string identifier for the payload type.  Does not necessarily need to be the BASF2 package.
     *  @param moduleName A string identifier for the payload type.  Does not necessarily need to be the BASF2 module.
     *
     *  @return Nothing yet
     */
    void writePayloadFile(std::string payloadFileName,
                          std::string packageName,
                          std::string moduleName);

    /** Gets payload URL from run generated map and downloads conditions file if needed.  Note that this function will get the package and module from the module.
     *
     *  @return Returns a string with the payload URL.
     */
    std::string getPayloadFileURL(Module* module) {return getPayloadFileURL(module->getPackage(), module->getName());};

    /** Gets payload URL from run generated map and downloads conditions file if needed.
     *  @param packageName A string identifier for the payload type.  Does not necessarily need to be the BASF2 package.
     *  @param moduleName A string identifier for the payload type.  Does not necessarily need to be the BASF2 module.
     *
     *  @return Returns a string with the payload URL.
     */
    std::string getPayloadFileURL(std::string packageName, std::string moduleName);




  private:
    /** Singleton, so control use of constructors */
    ConditionsService();

    /** Singleton, so control use of constructors */
    ConditionsService(ConditionsService const&);

    /** Singleton, so control use of copy operator */
    void operator=(ConditionsService const&);

    /** Singleton, so control use of destructor */
    ~ConditionsService();


    /** Function to capture rest returns. **/
    static size_t capture_return(void* buffer, size_t size, size_t nmemb, void* userp);

    /** Function to parse payloads */
    void parse_payloads(std::string temp);

    /** Function to parse generic xml return and display */
    void parse_return(std::string temp);

    /** Function to facilitate downloading files */
    static size_t write_data(void* ptr, size_t size, size_t nmemb, FILE* stream);

    /** Function to display file transfer progress */
    static int progress_func(void* ptr, double TotalToDownload, double NowDownloaded,
                             double TotalToUpload, double NowUploaded);

    /** The single instance of this class */
    static ConditionsService* m_instance;

    /** The base name for the REST services */
    std::string m_RESTbase;

    /** The base name for the remote conditions files */
    std::string m_FILEbase;

    /** The base name for the local conditions files to be stored temporarily */
    std::string m_FILElocal;

    /** A buffer to temporarily get rid of compiler warnings */
    std::string m_buffer;

    /** Map of payloads for current experiment and run. */
    std::map<std::string, conditionsPayload> m_payloads;

  };
}
#endif
