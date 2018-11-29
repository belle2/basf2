/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Giacomo De Pietro                                        *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>
#include <framework/logging/Logger.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/dataobjects/EventMetaData.h>

#include <bklm/dataobjects/BKLMDigit.h>
#include <bklm/dataobjects/BKLMDigitRaw.h>
#include <bklm/dataobjects/BKLMDigitOutOfRange.h>
#include <bklm/dataobjects/BKLMDigitEventInfo.h>

#include "TFile.h"
#include "TH1F.h"
#include "TH2F.h"
#include "TList.h"
#include "TGaxis.h"
#include "TString.h"
#include "TCanvas.h"

#include <sstream>
#include <string>

namespace Belle2 {

  /**
   * Module useful to quickly analyze BKLM unpacked data.
   */
  class BKLMDigitAnalyzerModule : public Module {

  public:

    /**
     * Constructor
     */
    BKLMDigitAnalyzerModule();

    /**
     * Destructor
     */
    virtual ~BKLMDigitAnalyzerModule() override;

    virtual void initialize() override;

    virtual void beginRun() override;

    virtual void event() override;

    virtual void endRun() override;

    virtual void terminate() override;


  private:

    int m_runNumber;

    StoreArray<BKLMDigit> m_digit;

    StoreArray<BKLMDigitRaw> m_digitRaw;

    StoreArray<BKLMDigitOutOfRange> m_digitOutOfRange;

    StoreArray<BKLMDigitEventInfo> m_digitEventInfo;

    std::string m_outputRootName;  /**< Name of output .root file */

    TFile* m_outputRootFile;

    TList* m_histoList;

    // Histograms
    TH2F* m_histoLayerVsSector[2];

    TH2F* m_histoLayerVsSectorPerPlane[2][2];

    TH1F* m_histoLayer[2][8];

    TH2F* m_histoChannel[2][8][2];

    TH1F* m_histoTdc[2][8][2];

    TH1F* m_histoCTimeDiff[2][8][2];

    //TH2F* m_histoCharge[2][8][2][2];

    // Useful function to convert a number into a string
    template <typename T>
    std::string toString(T val)
    {
      std::ostringstream stream;
      stream << val;
      return stream.str();
    }

    // Functions to create the histograms
    TH1F* createTH1(const char* name, const char* title, Int_t nBinsX, Double_t minX, Double_t maxX, const char* titleX,
                    const char* titleY, bool drawStat, TList* histoList = NULL);

    TH2F* createTH2(const char* name, const char* title, Int_t nBinsX, Double_t minX, Double_t maxX, const char* titleX, Int_t nBinsY,
                    Double_t minY, Double_t maxY, const char* titleY, bool drawStat, TList* histoList = NULL);
  };

}

