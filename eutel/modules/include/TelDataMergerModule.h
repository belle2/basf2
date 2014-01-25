#ifndef TELDATAMERGERMODULE_H
#define TELDATAMERGERMODULE_H

// include standard c++
#include <sys/time.h>
#include <memory>
#include <string>
#include <set>
#include <map>

// Include BASF2 Base stuff
#include <framework/core/Module.h>
#include <framework/core/Environment.h>

#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/dataobjects/EventMetaData.h>

#include <vxd/dataobjects/VxdID.h>
#include <rawdata/dataobjects/RawFTSW.h>

// load some parts of the eudaq
#include <eutel/eudaq/FileReader.h>

namespace Belle2 {
  /** A module template.
   *
   *  A detailed description of your module.
   */
  class TelDataMergerModule : public Module {
  public:
    /** Constructor, for setting module description and parameters. */
    TelDataMergerModule();

    /** Use to clean up anything you created in the constructor. */
    virtual ~TelDataMergerModule();

    // Standard Module Functions
    virtual void initialize();
    virtual void beginRun();
    virtual void event();
    virtual void endRun();
    virtual void terminate();

  protected:

    /** Stop processing of further events.
     * This just lets EventProcessor end processing of events, no errors are
     * thrown and processing ends normally.
     */
    void stopPeacefully();

    /** Perform specific actions when BORE event is encountered. Re-initialize
     * plugin manager. Don't interfere.
     * @param ev eudaq::Event to be processed
     * @return true
     */
    virtual bool processBOREvent(const eudaq::Event& ev);

    /** Perform specific actions when EORE event is encountered.
     * Don't interfere, we finish with the last event.
     * @param ev eudaq::Event to be processed
     * @return true
     */
    virtual bool processEOREvent(const eudaq::Event& ev);
    /** Get TLU tag from a telescope event
     * @param ev eudaq::Event from which the TLU tag has to be extracted.
     * @return TLU tag, or -1 if not found.
     */
    virtual int getTelTriggerID(const eudaq::Event& ev);
    /** Process normal data event: extract and save useful data.
     *
     */
    virtual bool processNormalEvent(const eudaq::Event& ev);

  private:

    // Steering Variables

    std::string m_inputFileName;    /**< name of input telescope data file (*.raw) */
    std::string m_storeDigitsName;  /**< DataStore name of TelDigits */
    std::string m_storeRawFTSWsName;    /**< DataStore name of RawFTSWs */

    // Internal Variables

    unsigned long int m_telEventNo; /**< stores the number of processed Tel events. */

    eudaq::FileReader* m_reader; /**> EUDAQ data reader */

    unsigned long int m_nDataEvents;
    unsigned int m_nBOREvents;
    unsigned int m_nEOREvents;
    unsigned int m_nNoTrigEvents;

    // The current TLU ID (15 bits) from the FTSW:
    int m_currentTLUTagFromFTSW; /**< TLU tag extracted from FTSW data in VXD DAQ */
    int m_currentTLUTagFromEUDAQ; /**< TLU tag extracted from EUDAQ telescope event */

    // conversion map between sensor numbers and their VxdIDs
    std::map< unsigned short, VxdID > m_sensorID;
  };
}
#endif
