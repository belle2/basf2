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
    static ConditionsService* GetInstance();



    /** Read back payloads associated with a particular global tag.
     *  @param GlobalTag A global tag is an approved set of payload tags.
     *
     *  @return Nothing yet
     */
    void GetPayloads(std::string GlobalTag, std::string ExperimentName, std::string RunName);

    /** Sets the base name for the REST services.  Example is http://belle2db.hep.pnnl.gov/b2s/rest/v1/.
     *  @param RESTBaseName The base name for the REST services.  Example is http://belle2db.hep.pnnl.gov/b2s/rest/v1/.
     *
     *  @return Nothing yet
     */
    void SetRESTbasename(std::string RESTBaseName) {m_RESTbase = RESTBaseName;};

    /** Sets the base name for the conditions files.  Example is http://belle2db.hep.pnnl.gov/.
     *  @param FILEBaseName The base name for the conditions files.  Example is http://belle2db.hep.pnnl.gov/.
     *
     *  @return Nothing yet
     */
    void SetFILEbasename(std::string FILEBaseName) {m_FILEbase = FILEBaseName;};

    /** Sets the local directory name for copies of the conditions files.  Example is /tmp/.
     *  @param FILEBaseLocal The base name for the conditions files.  Example is http://belle2db.hep.pnnl.gov/.
     *
     *  @return Nothing yet
     */
    void SetFILEbaselocal(std::string FILEBaseLocal) {m_FILElocal = FILEBaseLocal;};


    /** Adds a payload URL to the payload map.
     *  @param PackageModuleName The concatenation of the package.name and module.name.
     *  @param PayloadURL The payload URL.
     *
     *  @return Nothing yet
     */
    void AddPayloadURL(std::string PackageModuleName, std::string PayloadURL) {m_run_payloads[PackageModuleName] = PayloadURL;};
    /** Check to see if a payload key is already registered in the payload map.
     *  @param PackageModuleName The concatenation of the package.name and module.name.
     *
     *  @return true if payload exists, false if not.
     */
    bool PayloadExists(std::string PackageModuleName) {return (m_run_payloads.find(PackageModuleName) != m_run_payloads.end());}

    /** Adds a payload checksum to the payload map.
     *  @param PackageModuleName The concatenation of the package.name and module.name.
     *  @param Checksum The payload MD5 checksum.
     *
     *  @return Nothing yet
     */
    void AddChecksum(std::string PackageModuleName, std::string Checksum) {m_run_checksums[PackageModuleName] = Checksum;};

    /** Adds a payload file to the conditions database.
     *  @param payloadFileName The file name of the payload.
     *  @param module The module requesting payload storage (used for the 'module' and 'package' payload tags).
     *
     *  @return Nothing yet
     */
    void WritePayloadFile(std::string payloadFileName,
                          Module* module) {WritePayloadFile(payloadFileName, module->getPackage(), module->getName());};

    /** Adds a string to the buffer (for REST returns.)
     *  @param buffer The string to be added to the buffer.
     *
     *  @return Nothing yet
     */
    void AddReturn(std::string buffer) {m_buffer = m_buffer + buffer;};

    /** Adds a payload file to the conditions database.
     *  @param payloadFileName The file name of the payload.
     *  @param packageName A string identifier for the payload type.  Does not necessarily need to be the BASF2 package.
     *  @param moduleName A string identifier for the payload type.  Does not necessarily need to be the BASF2 module.
     *
     *  @return Nothing yet
     */
    void WritePayloadFile(std::string payloadFileName,
                          std::string packageName,
                          std::string moduleName);

    /** Gets payload URL from run generated map and downloads conditions file if needed.  Note that this function will get the package and module from the module.
     *
     *  @return Returns a string with the payload URL.
     */
    std::string GetPayloadFileURL(Module* module) {return GetPayloadFileURL(module->getPackage(), module->getName());};

    /** Gets payload URL from run generated map and downloads conditions file if needed.
     *  @param packageName A string identifier for the payload type.  Does not necessarily need to be the BASF2 package.
     *  @param moduleName A string identifier for the payload type.  Does not necessarily need to be the BASF2 module.
     *
     *  @return Returns a string with the payload URL.
     */
    std::string GetPayloadFileURL(std::string packageName, std::string moduleName);




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
    static ConditionsService* m_Instance;

    /** The base name for the REST services */
    std::string m_RESTbase;

    /** The base name for the remote conditions files */
    std::string m_FILEbase;

    /** The base name for the local conditions files to be stored temporarily */
    std::string m_FILElocal;

    /** A buffer to temporarily get rid of compiler warnings */
    std::string m_buffer;

    /** Map of payloads under construction **/
    std::map<std::string, TList*> m_current_payloads;

    /** Map of payloads in DB for a particular run, <PackageNameModuleName, PayloadURL> **/
    std::map<std::string, std::string> m_run_payloads;
    /** Map of checksums from DB for a particular run, <PackageNameModuleName, Checksum> **/
    std::map<std::string, std::string> m_run_checksums;

  };
}
#endif
