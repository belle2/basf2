/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Luka Santelj, Manca Mrvar                                *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <vector>
#include <map>
#include <TObject.h>
#include <iostream>

//number of channels per HAPD
#define N_MERGERS 72
#define N_FEB2MERGER 6

namespace Belle2 {
  //! The Class for ARICH mapping of modules to merger boards
  /*! This class provides ARICH mapping of HAPD modules to merger boards, and their FEB slots
    It is assumed merger ID going from 1:73, febSlot goes from 1:6
    change to std::map if merger IDs are "non-trivial"
  */
  class ARICHMergerMapping : public TObject {

  public:

    //! Default constructor
    ARICHMergerMapping();

    /**
     * Get ID of module connected to merger at FEB slot
     * @param mergerID merger ID number
     * @param febSlot  FEB slot
     * @return module ID number
     */
    unsigned getModuleID(unsigned mergerID, unsigned febSlot) const;

    /**
     * Get ID of merger to which module is connected
     * @param moduleID module ID number
     * @return merger ID number
     */
    unsigned getMergerID(unsigned moduleID) const;

    /**
     * Get FEB slot to which module is connected
     * @param moduleID module ID number
     * @return merger FEB slot
     */
    unsigned getFEBSlot(unsigned moduleID) const;

    /**
     * Get ID of merger from serial number
     * @param mergerSN merger serial number
     * @return mergerID merger ID number
     */
    unsigned getMergerIDfromSN(unsigned mergerSN) const;

    /**
     * Get serial number of marger from its ID
     * @param mergerID merger ID number
     * @return mergerSN merger serial number
     */
    unsigned getMergerSN(unsigned mergerID) const;

    /**
     * Add mapping entry
     * @param moduleID module ID number
     * @param mergerID merger ID number
     * @param mergerSN merger serial number
     * @param febSlot FEB slot
     */
    void addMapping(unsigned moduleID, unsigned mergerID, unsigned febSlot, unsigned mergerSN);

    /**
     * Pring mapping
     */
    void print() const;

  private:

    std::vector<uint16_t> m_merger2module;  /**< map of mergers to modules */
    std::vector<uint16_t> m_module2merger;  /**< map of modules to mergers */

    std::vector<bool> m_isMapped;           /**< vector of mapped modules */
    std::map<unsigned, unsigned> m_mergerID; /**< map mergers ID to SN */

    ClassDef(ARICHMergerMapping, 2);  /**< ClassDef, must be the last term before the closing {}*/

  };

} // namespace
