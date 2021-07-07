/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

/* KLM headers. */
#include <klm/dataobjects/KLMDigit.h>
#include <klm/dataobjects/KLMDigitEventInfo.h>
#include <klm/dataobjects/KLMDigitRaw.h>

/* Belle 2 headers. */
#include <framework/core/Module.h>
#include <framework/datastore/StoreArray.h>

/* ROOT headers. */
#include <TFile.h>
#include <TH1F.h>
#include <TH2F.h>
#include <TList.h>
#include <TTree.h>

/* C++ headers. */
#include <ctime>
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
    ~BKLMDigitAnalyzerModule() override;

    /**
     * Initializer.
     */
    void initialize() override;

    /**
     * Called when entering a new run.
     */
    void beginRun() override;

    /**
     * This method is called for each event.
     */
    void event() override;

    /**
     * This method is called if the current run ends.
     */
    void endRun() override;

    /**
     * This method is called at the end of the event processing.
     */
    void terminate() override;


  private:

    //! Run number of the current data set
    int m_runNumber;

    //! Input KLMDigits
    StoreArray<KLMDigit> m_digit;

    //! Input raw BLMDigits
    StoreArray<KLMDigitRaw> m_digitRaw;

    //!  Input BKLMDigits whose time is out of range
    StoreArray<KLMDigit> m_digitOutOfRange;

    //! Output data array of analyzed KLMDigit information
    StoreArray<KLMDigitEventInfo> m_digitEventInfo;

    //! Output filename
    std::string m_outputRootName;

    //! Pointer to ROOT output file
    TFile* m_outputRootFile;

    //! Pointer to ROOT tree with extra info
    TTree* m_extraInfo = nullptr;

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
                    const char* titleY, bool drawStat, TList* histoList = nullptr);

    //! Create a ROOT 2D histogram
    TH2F* createTH2(const char* name, const char* title, Int_t nBinsX, Double_t minX, Double_t maxX, const char* titleX, Int_t nBinsY,
                    Double_t minY, Double_t maxY, const char* titleY, bool drawStat, TList* histoList = nullptr);
  };

}
