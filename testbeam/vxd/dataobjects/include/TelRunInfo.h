#ifndef TELRUNINFO_H
#define TELRUNINFO_H

#include <framework/datastore/RelationsObject.h>

#include <string>

namespace Belle2 {
  /** Store event number, run number, and trigger ID.
   */
  class TelRunInfo : public RelationsObject {
  public:

    /** Constructor. */
    explicit TelRunInfo(ULong64_t run = 0,
                        ULong64_t numEvents = 0,
                        const std::string& fileName = "",
                        const std::string& detectorName = "");

    /** Destructor. */
    virtual ~TelRunInfo() {}

    /** Run Setter.
     *
     *  @param run The number of the current run.
     */
    void setRunNumber(ULong64_t run) {
      m_run = run;
    }

    /** Number of Events Setter.
     *
     *  @param numEvents The number of events of the current run.
     */
    void setNumEvents(ULong64_t numEvents) {
      m_numEvents = numEvents;
    }

    /** File Name Setter.
     *
     *  @param fileName The current file name.
     */
    void setFileName(const std::string& fileName) {
      m_fileName = fileName;
    }

    /** Detector Name Setter.
     *
     *  @param detectorName The current detector's name.
     */
    void setDetectorName(const std::string& detectorName) {
      m_detectorName = detectorName;
    }

    /** Run Getter.
     *
     *  @return The number of the current run.
     */
    ULong64_t getRunNumber() const {
      return m_run;
    }


    /** Number of Events Getter.
     *
     *  @return The number of events of the current run.
     */
    ULong64_t getNumEvents() const {
      return m_numEvents;
    }

    /** File Name Getter.
     *
     *  @return The current file name:
     */
    std::string getFileName() const {
      return m_fileName;
    }

    /** Detector Name Getter.
    *
    *  @return The current detector name:
    */
    std::string getDetectorName() const {
      return m_detectorName;
    }

    /** Comparison Operator.
     *
     *  @param telRunInfo The telescope run data to compare with.
     *  @return True if run number, number of events, filename, and detector name are
     *  the same
     */
    bool operator==(const TelRunInfo& telRunInfo) const;

    /** Comparison Operator.
     *
     *  @param telRunInfo The event meta data to compare with.
     *  @return True if run number, number of events, filename and detector name are
     *  different
     */
    bool operator!=(const TelRunInfo& telRunInfo) const;

  private:

    /** Run number.
     */
    ULong64_t m_run;


    /** Number of events number.
     */
    ULong64_t m_numEvents;

    /** Input File Name.
     */
    std::string m_fileName;

    /** Detector Name.
     */
    std::string m_detectorName;

    ClassDef(TelRunInfo, 2) /**< Store event number, run number, and trigger ID. */
  }; //class
} // namespace Belle2
#endif // TELRUNINFO
