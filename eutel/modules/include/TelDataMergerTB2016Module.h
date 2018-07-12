#ifndef TELDATAMERGERTB2016MODULE_H
#define TELDATAMERGERTB2016MODULE_H

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

// load some parts of the PXD
#include <pxd/dataobjects/PXDDigit.h>

#include <framework/datastore/StoreArray.h>

#include <vxd/dataobjects/VxdID.h>
#include <rawdata/dataobjects/RawFTSW.h>

// load some parts of the eudaq
#include <eutel/eudaq/FileReader.h>
#include <eutel/merge/CircularTools.h>
#include <eutel/merge/BoundedSpaceMap.h>
#include <eutel/merge/EUDAQBuffer.h>

#ifdef debug_log
#include <fstream>
#endif

namespace Belle2 {
  /** A module template.
   *
   *  A detailed description of your module.
   */
  class TelDataMergerTB2016Module : public Module {
  public:

    typedef CIRC::tag_type tag_type;

    /** Constructor, for setting module description and parameters. */
    TelDataMergerTB2016Module();

    /** Use to clean up anything you created in the constructor. */
    virtual ~TelDataMergerTB2016Module();

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


    /** Compare digits of all eudaq data stream events to digits of one vxd data stream event
    * @param PXDdigitTuples pxddigits from one PXD subevent in the EUDAQEventBuffer
    * @param VXDStreamPXDDigits pxddigits from one event in the VXD data stream
    */
    short int CompareDigitsSingleEvent(StoreArray<PXDDigit> VXDStreamPXDDigits, std::vector<short_PXDdigit_type> PXDdigitTuples);

    /** Get data from the buffer and save them as digits in the DataStore.
     * @param currentTag current TLU tag from ftsw.
     * @param matchposition position of matched event in the EUDAQEventbuffer
     * @param Sensor ID of the PXD in the VXD data stream
     */
    virtual void saveDigits(int matchposition);

  private:


    // Steering Variables

    std::string m_inputFileName;                   /**< name of input telescope data file (*.raw) */
    std::string m_storeTELDigitsName;              /**< DataStore name of TelDigits */
    std::string m_storeOutput_PXDDigitsName;       /**< DataStore name of the OUtput PXDDigits */
    std::string m_storeInput_PXDDigitsName;        /**< DataStore name of the Input PXDDigits */
    std::string m_storeEventInfoName;              /**< DataStore name of Event Info */
    std::string m_storeRawFTSWsName;    /**< DataStore name of RawFTSWs */

    short int m_mergeswitch;             /**< Switch to choose the merging strategy: 1:triggernumber, 2:PXD digit matching */
    short int m_EUDAQTriggerOffset;      /**< Offset of the EUDAQ Trigger (-m_EUDAQTriggerOffset) number compared to the SVD Trigger number */


    unsigned short m_bufferSize;    /**< Size of the buffers */
    std::vector<int> m_eutelPlaneNrs; /**< Numbering of telescope planes in eudaq */
    std::vector<int> m_PXDPlaneNrs; /**< Numbering of telescope planes in eudaq */
    long m_nEventsProcess;          /**< Number of events to process */

    std::vector<int> m_VXDTriggerNumber;    /**< Vector of VXD Trigger numbers for events with matching digits */
    std::vector<int> m_EUDAQTriggerNumber;  /**< Vector of EUDAQ Trigger numbers for events with matching digits */
    std::vector<int> m_Iteration;           /**< Vector of VXD event numbers for events with matching digits */

    // Internal Variables

    eudaq::FileReader* m_reader; /**> EUDAQ data reader */

    // Telescope event data cache
    typedef std::tuple<unsigned short, short, short> short_digit_type;
    typedef unsigned long long int eudaq_timestamp_type;

    EUDAQBuffer m_EUDAQEventBuffer;

    bool m_digitmatch; /**< Bool that is true if for a given VXD data stream PXD event a EUDAQ PXD event was found */
    bool m_update; /**< Bool that is false once the buffer update didn't work */

    unsigned long int m_nVXDDataEvents;        /**< Number of processed VXD data events */
    unsigned long int m_nTelDataEvents;        /**< Number of processed Tel data events */
    unsigned long int m_nEUDAQPXDDataEvents;   /**< Number of processed EUDAQ PXD data events */
    unsigned long int m_nMapHits;              /**< Number of successful retrievals from buffer */
    unsigned int m_nBOREvents;                 /**< Number of BORE events */
    unsigned int m_nEOREvents;                 /**< Number of EORE events */
    unsigned int m_nNoTrigEvents;              /**< Number of data events without trigger tag */

    // The current Trigger ID and Event Number from the merged root file:
    unsigned long m_currentEventNumberFromEventInfo;  /**< TLU tag extracted from FTSW data in VXD DAQ */
    unsigned long m_currentTriggerIDFromEventInfo;    /**> TLU timestamp extracted from FTSW */

    // The current TLU ID (15 bits) from the FTSW:
    tag_type m_currentTLUTagFromFTSW;           /**< TLU tag extracted from FTSW data in VXD DAQ */
    unsigned long m_currentTimeStampFromFTSW;   /**> TLU timestamp extracted from FTSW */

    // conversion map between sensor numbers and their VxdIDs
    std::map< int, VxdID > m_sensorID;
    std::map< int, VxdID > m_PXDsensorID;

    unsigned short m_PXDEUDAQNumber;           /**< EUDAQ data PXD number*/

#ifdef debug_log
    std::ofstream m_debugLog;         /**< Debug log text file */
#endif
  };
}
#endif
