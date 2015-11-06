/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef TOPRAWDATAINPUTMODULE_H
#define TOPRAWDATAINPUTMODULE_H

#include <framework/core/Module.h>
#include <testbeam/top/modules/TOPRawdataInput/Packet.h>
#include <string>

namespace Belle2 {

  /**
   * Raw data reader for TOP beam tests: converts data to standard format (RawTOP)
   */
  class TOPRawdataInputModule : public Module {

  public:

    /**
     * Constructor
     */
    TOPRawdataInputModule();

    /**
     * Destructor
     */
    virtual ~TOPRawdataInputModule();

    /**
     * Initialize the Module.
     * This method is called at the beginning of data processing.
     */
    virtual void initialize();

    /**
     * Called when entering a new run.
     * Set run dependent things like run header parameters, alignment, etc.
     */
    virtual void beginRun();

    /**
     * Event processor.
     */
    virtual void event();

    /**
     * End-of-run action.
     * Save run-related stuff, such as statistics.
     */
    virtual void endRun();

    /**
     * Termination action.
     * Clean-up, close files, summarize statistics, etc.
     */
    virtual void terminate();

  private:

    /**
     * Open raw-data file
     * @params fileName file name
     * @return true on success
     */
    bool openFile(const std::string& fileName);

    /**
     * Close raw-data file
     */
    void closeFile();

    /**
     * Set experiment and run numbers from file name
     * @return true on success
     */
    bool setExpRunNumbers(const std::string& fileName);

    /**
     * Read event.
     * Return status:
     * - 0 on success (event can be further processed)
     * - negative value for EOF or for corrupted file (file has to be closed)
     * - positive value for non-event or corrupted record (try with next data record)
     * @return error status
     */
    int readEvent();

    /**
     * Read event, IRS3B packet format (Kurtis)
     * Return status:
     * - 0 on success (event can be further processed)
     * - negative value for EOF or for corrupted file (file has to be closed)
     * - positive value for non-event or corrupted record (try with next data record)
     * @return error status
     */
    int readEventIRS3B();

    /**
     * Read event, IRSX gigE raw data format (Lynn)
     * Return status:
     * - 0 on success (event can be further processed)
     * - negative value for EOF or for corrupted file (file has to be closed)
     * - positive value for non-event or corrupted record (try with next data record)
     * @return error status
     */
    int readEventIRSXv1();


    /**
     * Read gigE raw data packet (https://belle2.cc.kek.jp/~twiki/pub/Detector/TOP/Module01Firmware/data_format_v1_5.xlsx)
     * @return true on success
     */
    bool readGigEPacket();

    /**
     * Read data of one SCROD
     * @return pointer to ScrodData (success) or NULL (failure)
     */
    TOPTB::ScrodData* readScrodData();

    std::string m_inputFileName; /**< input file name (raw data) */
    int m_dataFormat; /**< data format */
    std::vector<std::string> m_inputFileNames; /**< list of input file names (raw data) */
    unsigned int m_listIndex;  /**< current index of input file name list element */
    unsigned long m_evtNumber; /**< current event number */
    unsigned long m_runNumber; /**< run number */
    unsigned long m_expNumber; /**< experiment number */
    unsigned long long m_bytesRead;  /**< number of bytes read from file */
    unsigned long m_packetsRead;     /**< number of good packets read from file */
    unsigned long m_eventsRead;      /**< number of events read from file */

    std::ifstream m_stream;   /**< input stream for reading from a file */
    TOPTB::ScrodData* m_scrodData;   /**< storage for one SCROD data */
    bool m_err; /**< error status of last ScrodData::read */

    std::vector<unsigned> m_gigEPacket; /**< data of one gigE packet */

  };

} // Belle2 namespace

#endif
