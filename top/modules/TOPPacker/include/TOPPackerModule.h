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
#include <top/geometry/TOPGeometryPar.h>
#include <top/RawDataTypes.h>
#include <string>

namespace Belle2 {

  /**
   * Raw data packer
   */
  class TOPPackerModule : public Module {

  public:

    /**
     * Constructor
     */
    TOPPackerModule();

    /**
     * Destructor
     */
    virtual ~TOPPackerModule();

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
     * Pack in format Production draft
     */
    void packProductionDraft();

    /**
     * Pack in format Feature-extracted data
     */
    void packType0Ver16();

    /**
     * sum both 16-bit words of 32-bit integer
     */
    unsigned short sumShorts(unsigned int x) const
    {
      return x + (x >> 16);
    }

    std::string m_inputDigitsName;      /**< name of TOPDigit store array */
    std::string m_inputRawDigitsName;   /**< name of TOPRawDigit store array */
    std::string m_outputRawDataName;    /**< name of RawTOP store array */
    std::string m_format;               /**< data format */
    TOP::RawDataType m_dataType = TOP::RawDataType::c_Draft; /**< enum for data format */

    TOP::TOPGeometryPar* m_topgp = TOP::TOPGeometryPar::Instance(); /**< geometry */

  };

} // Belle2 namespace

