/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>
#include <string>
#include <framework/datastore/StoreArray.h>
#include <rawdata/dataobjects/RawTOP.h>

#include "TFile.h"
#include "TTree.h"
#include "TClonesArray.h"

namespace Belle2 {

  /**
   * Event builder for PocketDAQ output root files of CRT tests of winter 2016.
   * These files are gigE packet based e.g. one basf2 event consists of 4 scrod packets.
   * Module opens the root file, reads 'tree' entries until it detects next event,
   * converts RawDataBlocks branches into RawTOP objects, fix the raw data format and
   * appends objects to StoreArray<RawTOP>. Format fix includes adding of header word,
   * removal of repeated word and byte swapping.
   */
  class TOPCRTEventBuilderModule : public Module {

  public:

    /**
     * Constructor
     */
    TOPCRTEventBuilderModule();

    /**
     * Destructor
     */
    virtual ~TOPCRTEventBuilderModule();

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
     * Deduce run number from file name
     * @param name file name
     * @return run number
     */
    int getRunNumber(const std::string& name);

    /**
     * Appends re-formated raw data to store array
     * @param rawTOP input raw data
     * @param rawData store array to append
     */
    void appendData(RawTOP& rawTOP, StoreArray<RawTOP>& rawData);

    /**
     * Swap bytes of an 32-bit integer
     */
    int swap32(int x)
    {
      return (((x << 24) & 0xFF000000) |
              ((x <<  8) & 0x00FF0000) |
              ((x >>  8) & 0x0000FF00) |
              ((x >> 24) & 0x000000FF));
    }

    std::string m_inputFileName; /**< input file name */
    TFile* m_file = 0; /**< root file ptr */
    TTree* m_tree = 0; /**< tree ptr */
    int m_numEntries = 0; /**< number of tree entries */
    int m_entryCount = 0; /**< tree entry counter */
    TClonesArray* m_cloneArray = 0; /**< tree branch */

    int m_evtNumber = 0; /**< current event number */
    int m_runNumber = 0; /**< run number */
    int m_expNumber = 0; /**< experiment number */

  };

} // Belle2 namespace

