/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <framework/logging/Logger.h>

#include <TObject.h>
#include <TString.h>

namespace Belle2 {
  /**
   * This class handle DQM plots configuration:  additional histograms (Charge, SNR, Time), 3Samples flag, etc...
   */

  class SVDDQMPlotsConfiguration: public TObject {
  public:
    /**
    * Default constructor
    */
    SVDDQMPlotsConfiguration(const TString& uniqueID = "");

    /**
     * Returns sensor list
     * @return  sensor list
     **/
    const std::vector<std::string> getListOfSensors() const { return m_svdListOfSensors; }

    /**
     * Add sensor to list
     * @param label: sensor to add (e.g.: "4.1.1" for L4_1_1)
     */
    void addSensorToList(std::string label) { m_svdListOfSensors.push_back(label); }

    /**
     * Returns 3samples flag
     * @return  3sample flag
     **/
    bool  isPlotsFor3SampleMonitoring() const { return m_3Samples; }

    /**
     * Enables 3sample flag
     */
    void enablePlotsFor3SampleMonitoring() { m_3Samples = true; }


    /**
     * Returns HLT skipped event flag
     * @return HLT skipped event flag
     **/
    bool  isSkipHLTRejectedEvents() const { return m_skipHLTRejectedEvents; }

    /**
     * Enables HLT skipped event flag
     */
    void enableSkipHLTRejectedEvents() { m_skipHLTRejectedEvents = true; }

    /**
     * Get the unique ID  of the payload
     */
    TString get_uniqueID() const {return m_uniqueID;}


    /** name of the payload */
    static std::string name;

  private:
    /** unique identifier of the SVD reconstruction configuration payload */
    TString m_uniqueID;

    /** flag for 3 samples histograns */
    bool m_3Samples = false;

    /** if true skip events rejected by HLT */
    bool m_skipHLTRejectedEvents = false;

    /** list of sensors */
    std::vector<std::string> m_svdListOfSensors;

    ClassDef(SVDDQMPlotsConfiguration, 2); /**< needed by root*/

  };

}
