#ifndef TELDATAREADERMODULE_H
#define TELDATAREADERMODULE_H

// include standard c++
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

// load some parts of the eudaq
#include <eutel/eudaq/FileReader.h>

namespace Belle2 {

  class TelDataReaderModule : public Module {
  public:
    /** Constructor, for setting module description and parameters. */
    TelDataReaderModule();

    /** Use to clean up anything you created in the constructor. */
    virtual ~TelDataReaderModule();

    // Standard Module Functions
    virtual void initialize();
    virtual void event();
    virtual void terminate();

    virtual void start_run();
    virtual void end_run();
  protected:

  private:

    // ==================================================
    // Private Member Functions

    /** Get list of input files, taking -i command line overrides into account. */
    std::vector<std::string> getInputFiles() const
    {
      std::vector<std::string> inputFiles = Environment::Instance().getInputFilesOverride();
      if (! inputFiles.empty()) {
        return inputFiles;
      }
      inputFiles = m_inputFileNames;
      if (! m_inputFileName.empty()) {
        inputFiles.push_back(m_inputFileName);
      }
      return inputFiles;
    }

    // ==================================================
    // Private Member Variables

    // Steering Variables

    /** File to read from. Cannot be used together with m_inputFileNames. */
    std::string m_inputFileName;

    /** Files to read from. */
    std::vector<std::string> m_inputFileNames;

    /** The maximum number of entries to be processed. If this is
              zero, don't place eny restrictions. '*/
    unsigned long long int m_ullMaxNumEntries;

    /** The number of entries that shall be skipped at the
        beginning. */
    unsigned long long int m_ullNumEntriesSkip;

    /** Used for output: for every n-th event there will be some
        output. n is given by this variable. */
    unsigned long long int m_numPrint;

    // Internal Variables

    // Event Meta Data
    StoreObjPtr<EventMetaData> m_eventMetaDataPtr;

    // Store the number of processed events
    unsigned long long int m_counter;
    // Store the current event number
    unsigned long long int m_eventNo;
    // Store the current run number
    unsigned long long int m_runNo;

    // store the current file id
    size_t m_fileID;

    // the eudaq data reader
    eudaq::FileReader* m_reader;

    // store the run numbers which were already processed
    std::set<unsigned long long int> m_runNumbers;

    unsigned int m_nData;
    unsigned int m_nDataLast;
    unsigned int m_nNonDet;
    unsigned int m_nBORE;
    unsigned int m_nEORE;
    unsigned int m_nNoTrig;

    // mapping of eutel plane numbers: EUDAQ numbers ordered in beam direction
    std::vector<int> m_eutelPlaneNrs;
    // conversion map between telescope sensor numbers and their VxdIDs
    std::map< int, VxdID > m_sensorID;

    // mapping of pxd plane numbers: PXD numbers ordered in beam direction
    std::vector<int> m_pxdPlaneNrs;
    // conversion map between pxd sensor numbers and their VxdIDs
    std::map< int, VxdID > m_PxdSensorID;
  };
}
#endif
