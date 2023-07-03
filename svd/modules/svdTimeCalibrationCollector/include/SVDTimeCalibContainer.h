/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once


#include <string>
#include <map>
#include <memory>

#include "TNamed.h"
#include "TH1F.h"
#include "TH2F.h"
#include "TString.h"
#include "TDirectory.h"


namespace Belle2 {

  /**
   * Collection of histogram to be used in the calibration process
   */
  class SVDTimeCalibContainer : public TNamed {
  public:
    /** Default Constructor */
    SVDTimeCalibContainer();
    ~SVDTimeCalibContainer() {}

    /** Setter for directory  */
    void SetDirectory(TDirectory* dir);

    bool AddDirectoryStatus() { return fgAddDirectory; }
    void AddDirectory(bool add) { fgAddDirectory = add; }

    void Reset();

    // void SetTH1FHistogram(TString name, TString title, int bins, double xmin, double xmax)
    // {
    //   m_TH1F[name] = new TH1F(name, title, bins, xmin, xmax);
    // }
    void SetTH1FHistogram(TH1F* hist)
    {
      somevalue = 0;
      histoNames["Surya"] = 5;
      // hist->SetDirectory(0);
      // histoNames.emplace(std::make_pair(hist->GetName(), 2));
      // histos.push_back(hist);
      m_TH1F[TString(hist->GetName())] = hist;
    }

    void SetTH2FHistogram(TH2F* hist)
    {
      // hist->SetDirectory(0);
      m_TH2F[hist->GetName()] = hist;
    }

    static bool fgAddDirectory;   ///<! Flag to add histograms to the directory

    /** All Histograms */
    std::map<TString, TH1F*> m_TH1F; /**< 1D Histograms */
    std::map<TString, TH2F*> m_TH2F; /**< 2D Histograms */

  private:
    TDirectory* fDirectory; /**< Pointer to directory holding this histogram */
    std::map<std::string, int> histoNames;
    std::vector<TH1F*> histos;
    int somevalue;

  };

} // end namespace Belle2
