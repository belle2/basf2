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
#include "TTree.h"
#include "TH1F.h"
#include "TH2F.h"
#include "TList.h"
#include "TGaxis.h"
#include "TString.h"
#include "TCanvas.h"

#include <sstream>
#include <string>
#include <time.h>

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

    //! Run number of the current data set
    int m_runNumber;

    //! Input BKLMDigits
    StoreArray<BKLMDigit> m_digit;

    //! Input raw BKLMDigits
    StoreArray<BKLMDigitRaw> m_digitRaw;

    //!  Input BKLMDigits whose time is out of range
    StoreArray<BKLMDigitOutOfRange> m_digitOutOfRange;

    //! Output data array of analyzed BKLMDigit information
    StoreArray<BKLMDigitEventInfo> m_digitEventInfo;

    //! Output filename
    std::string m_outputRootName;

    //! Pointer to ROOT output file
    TFile* m_outputRootFile;

    //! Pointer to ROOT tree with extra info
    TTree* m_extraInfo;

    //! Pointer to ROOT list of histograms
    TList* m_histoList;

    //! Pointer to occupancy 2D histograms (numerator, denominator) of layer vs sector
    TH2F* m_histoLayerVsSector[2];

    //! Pointer to occupancy 2D histogram of layer vs sector for each view (=plane)
    TH2F* m_histoLayerVsSectorPerPlane[2][2];

    //! Pointer to occupancy 1D histogram of each layer for each sector
    TH1F* m_histoLayer[2][8];

    //! Pointer to occupancy 2D histogram of each channel
    TH2F* m_histoChannel[2][8][2];

    //! Pointer to occupancy 2D histogram of each strip
    TH2F* m_histoStrip[2][8][2];

    //! Pointer to TDC 1D histogram of each channel for each sector and view
    TH1F* m_histoTdc[2][8][2];

    //! Pointer to TDC-difference 1D histogram of each channel for each sector and view
    TH1F* m_histoCTimeDiff[2][8][2];

    //TH2F* m_histoCharge[2][8][2][2];

    //! Convert a number of type T into a string
    template <typename T>
    std::string toString(T val)
    {
      std::ostringstream stream;
      stream << val;
      return stream.str();
    }

    //! Create a ROOT 1D histogram
    TH1F* createTH1(const char* name, const char* title, Int_t nBinsX, Double_t minX, Double_t maxX, const char* titleX,
                    const char* titleY, bool drawStat, TList* histoList = NULL);

    //! Create a ROOT 2D histogram
    TH2F* createTH2(const char* name, const char* title, Int_t nBinsX, Double_t minX, Double_t maxX, const char* titleX, Int_t nBinsY,
                    Double_t minY, Double_t maxY, const char* titleY, bool drawStat, TList* histoList = NULL);
  };

}

