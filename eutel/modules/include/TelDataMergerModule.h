#ifndef TELDATAMERGERMODULE_H
#define TELDATAMERGERMODULE_H

#undef debug_log
// include standard c++
#include <sys/time.h>
#include <memory>
#include <string>
#include <set>
#include <map>
#include <tuple>

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
#include <eutel/merge/CircularTools.h>
#include <eutel/merge/BoundedSpaceMap.h>

#ifdef debug_log
#include <fstream>
#endif

namespace Belle2 {
  /** A module template.
   *
   *  A detailed description of your module.
   */
  class TelDataMergerModule : public Module {
  public:

    typedef CIRC::tag_type tag_type;

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

    /** Process normal data event: extract and push useful data into the buffer.
     * @param ev eudaq::Event to process
     * @return bool if useful data were saved.
     */
    virtual bool processNormalEvent(const eudaq::Event& ev);

    /** Extract TLU tag from a normal EUDAQ event. .
     * @param ev eudaq::Event to process
     * @return the extracted TLU tag, 0 if tag not present.
     */
    virtual short int getTLUTagFromEUDAQ(const eudaq::Event& ev);


    /** get the next telescope event to buffer
     * @return true, if a data event was retrieved successfully, otherwise false.
     */
    virtual bool addTelEventToBuffer();

    /** Advance buffer by max(number of free positions, given advance)
     * @param advance forced buffer advance - the buffer will advance by at least this
     * @return actual number of advanced positions
     */
    virtual std::size_t advanceBuffer();

    /** Get data from the buffer and save them as digits in the DataStore.
     * @param currentTag current TLU tag from ftsw.
     */
    virtual void saveDigits(tag_type currentTag);

  private:


    // Steering Variables

    std::string m_inputFileName;    /**< name of input telescope data file (*.raw) */
    std::string m_storeDigitsName;  /**< DataStore name of TelDigits */
    std::string m_storeRawFTSWsName;    /**< DataStore name of RawFTSWs */

    unsigned short m_bufferSize;    /**< Size of the buffers */
    std::vector<int> m_eutelPlaneNrs; /**< Numbering of telescope planes in eudaq */
    long m_nEventsProcess;          /**< Number of events to process */

    // Internal Variables

    eudaq::FileReader* m_reader; /**> EUDAQ data reader */

    // Telescope event data cache
    typedef std::tuple<unsigned short, short, short> short_digit_type;
    typedef unsigned long long int eudaq_timestamp_type;
    BoundedSpaceMap<eudaq_timestamp_type, short_digit_type> m_buffer;
    BoundedSpaceSet m_bufferVXD;

    unsigned long int m_nVXDDataEvents; /**< Number of processed VXD data events */
    unsigned long int m_nTelDataEvents; /**< Number of processed Tel data events */
    unsigned long int m_nMapHits;       /**< Number of successful retrievals from buffer */
    unsigned int m_nBOREvents;            /**< Number of BORE events */
    unsigned int m_nEOREvents;            /**< Number of EORE events */
    unsigned int m_nNoTrigEvents;         /**< Number of data events without trigger tag */

    // The current TLU ID (15 bits) from the FTSW:
    tag_type m_currentTLUTagFromFTSW; /**< TLU tag extracted from FTSW data in VXD DAQ */
    unsigned long m_currentTimeStampFromFTSW; /**> TLU timestamp extracted from FTSW */

    // Data for sync sanity check
    tag_type m_referenceTLUTag;                 /** TLU tag of a previous matched event. */
    long long int m_referenceTimeFromFTSW;     /**< FTSW timestamp of a previous matched pair */
    long long int m_referenceTimeFromEUDAQ; /**< EUDAQ timestamp of a previous matched pair */

    // conversion map between sensor numbers and their VxdIDs
    std::map< int, VxdID > m_sensorID;

#ifdef debug_log
    std::ofstream m_debugLog;         /**< Debug log text file */
#endif
  };
}
#endif
