/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2021 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Ludovico Massaccesi                                      *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <framework/core/HistoModule.h>
#include <framework/datastore/StoreArray.h>
#include <rawdata/dataobjects/RawFTSW.h>
#include <svd/dataobjects/SVDShaperDigit.h>
#include <svd/calibration/SVDFADCMaskedStrips.h>
#include <TH1.h>
#include <TH2.h>
#include <string>

namespace Belle2 {
  namespace SVD {
    /** The SVD dose-monitoring DQM module.
     *
     * Fills histograms of the SVD's occupancy, occupancy vs time since
     * last injection and time in beam cycle, occupancy by layer, by
     * sensor type, by trigger type, by injection type, by presence of
     * injection veto, by ... (TODO).
     *
     * @sa https://agira.desy.de/browse/BII-7853
     */
    class SVDDQMDoseModule : public HistoModule {
    public:
      SVDDQMDoseModule(); /**< Default constructor, defines parameters. */

      void initialize() override final; /**< Overrides HistoModule::initialize */
      void beginRun() override final; /**< Overrides HistoModule::beginRun */
      void event() override final; /**< Overrides HistoModule::event */
      void defineHisto() override final; /**< Overrides HistoModule::defineHisto */

    private:
      // Steerable data members (parameters)
      std::string m_histogramDirectoryName; /**< Name of the histograms' directory in the ROOT file. */
      std::string m_SVDShaperDigitsName; /**< Name of the StoreArray of SVDShaperDigit to use. */
      // TODO ...

      // Inputs
      StoreArray<RawFTSW> m_rawTTD; /**< Input: DAQ status. */
      StoreArray<SVDShaperDigit> m_digits; /**< Input: raw hits. */
      SVDFADCMaskedStrips m_maskedStrips; /**< Input: masked strips. */

      // Outputs (histograms)
      // TODO ...

      // Other stuff
      unsigned int m_nActiveStripsU = 0;
      unsigned int m_nActiveStripsV = 0;

    };
  }
}
