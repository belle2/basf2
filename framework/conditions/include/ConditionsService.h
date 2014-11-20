#ifndef ConditionsService_H
#define ConditionsService_H

#include <TList.h>
#include <TObject.h>
#include <string>
#include <map>

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


    /** Start defining a payload
     *  @param PayloadTag A name for the particular payload you are storing. A run name/number is a common example.
     *  @param PayloadTag A name for the type of payload you are storing. 'calibration' and 'configuration' are common types.
     *  @param SubsystemTag The tag for the subsystem to which your algorithm applies.  These tags are administratively controlled.
     *  @param AlgorithmName The name of the algorithm that generated the payload.
     *  @param AlgorithmVersion The version of the algorithm that generated the payload.
     *  @param InitialRun The earliest run in the interval of validity (IOV) for this payload. IOV can be changed.
     *  @param FinalRun The last run in the interval of validity (IOV) for this payload.  IOV can be changed.
     */
    Int_t StartPayload(std::string PayloadTag,
                       std::string PayloadType,
                       std::string SubsystemTag,
                       std::string AlgorithmName,
                       std::string AlgorithmVersion,
                       std::string Experiment,
                       std::string InitialRun,
                       std::string FinalRun);


    /** Write an object to your payload
     *  @param PayloadTag A name for the particular payload you are storing. A run name/number is a common example.
     *  @param SubsystemTag The tag for the subsystem to which your algorithm applies.  These tags are administratively controlled.
     *  @param AlgorithmName The name of the algorithm that generated the payload.
     *  @param AlgorithmVersion The version of the algorithm that generated the payload.
     *  @param InitialRun The earliest run in the interval of validity (IOV) for this payload. IOV can be changed.
     *  @param FinalRun The last run in the interval of validity (IOV) for this payload.  IOV can be changed.
     */
    Int_t WritePayloadObject(TObject* payload,
                             std::string PayloadTag,
                             std::string SubsystemTag,
                             std::string AlgorithmName,
                             std::string AlgorithmVersion,
                             std::string InitialRun,
                             std::string FinalRun);

    /** End the definition of the payload and commit it to the conditions database.
     *  @param PayloadTag A name for the particular payload you are storing. A run name/number is a common example.
     *  @param SubsystemTag The tag for the subsystem to which your algorithm applies.  These tags are administratively controlled.
     *  @param AlgorithmName The name of the algorithm that generated the payload.
     *  @param AlgorithmVersion The version of the algorithm that generated the payload.
     *  @param InitialRun The earliest run in the interval of validity (IOV) for this payload. IOV can be changed.
     *  @param FinalRun The last run in the interval of validity (IOV) for this payload.  IOV can be changed.
     */
    Int_t CommitPayload(std::string PayloadTag,
                        std::string SubsystemTag,
                        std::string AlgorithmName,
                        std::string AlgorithmVersion,
                        std::string InitialRun,
                        std::string FinalRun);


    /** Read back a payload associated with a particular global tag and run for a particular algorithm.
     *  @param GlobalTag A global tag is an approved set of payload tags.
     *  @param RunNumber The run number that you want your returned payload to be associated with.
     *  @param AlgorithmName The name of the algorithm that generated the payload.
     *  @param AlgorithmVersion The version of the algorithm that generated the payload.
     *
     *  @return A pointer to a TList of keys pointing to all the objects in the payload.
     */
    TList* GetPayloadList(std::string GlobalTag, std::string RunNumber,
                          std::string AlgorithmName, std::string AlgorithmVersion);




  private:
    /** Singleton, so control use of constructors */
    ConditionsService();

    /** Singleton, so control use of constructors */
    ConditionsService(ConditionsService const&);

    /** Singleton, so control use of copy operator */
    void operator=(ConditionsService const&);

    /** Singleton, so control use of destructor */
    ~ConditionsService();

    /** Function to generate consistent pointers to payload file directories */
    std::string GenerateDirectory(std::string PayloadTag,
                                  std::string SubsystemTag,
                                  std::string AlgorithmName,
                                  std::string AlgorithmVersion,
                                  std::string InitialRun,
                                  std::string FinalRun);

    /** Function to generate consistent names for payload files */
    std::string GenerateFilename(std::string PayloadTag,
                                 std::string SubsystemTag,
                                 std::string AlgorithmName,
                                 std::string AlgorithmVersion,
                                 std::string InitialRun,
                                 std::string FinalRun);

    /** The single instance of this class */
    static ConditionsService* m_Instance;

    /** A buffer to temporarily get rid of compiler warnings */
    std::string m_buffer;

    /** Map of payloads under construction **/
    std::map<std::string, TList*> m_current_payloads;

  };
}
#endif
