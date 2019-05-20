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
    virtual void initialize() override;

    /**
     * Called when entering a new run.
     * Set run dependent things like run header parameters, alignment, etc.
     */
    virtual void beginRun() override;

    /**
     * Event processor.
     */
    virtual void event() override;

    /**
     * End-of-run action.
     * Save run-related stuff, such as statistics.
     */
    virtual void endRun() override;

    /**
     * Termination action.
     * Clean-up, close files, summarize statistics, etc.
     */
    virtual void terminate() override;

  private:

    /**
     * Pack in format: c_Draft (tentative production format)
     * this format was never implemented in firmware!
     */
    void packProductionDraft();

    /**
     * Pack in format: c_Type0Ver16 (Feature-extracted data)
     * this format was never implemented in firmware!
     */
    void packType0Ver16();

    /**
     * Pack in format: Production Debugging Data Format 01
     */
    void packProductionDebug();

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

