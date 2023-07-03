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
// #include "TH1F.h"
// #include "TH2F.h"
// #include "TString.h"

class TDirectory;
class TH1F;
class TH2F;
class TString;

namespace Belle2 {

  /**
   * Collection of histogram to be used in the calibration process
   */
  class SVDTimeCalibContainer : public TNamed {
  public:
    /** Default Constructor */
    SVDTimeCalibContainer(TString name, TString title);
    ~SVDTimeCalibContainer();

    /** Setter for directory  */
    void SetDirectory(TDirectory* dir);

    bool AddDirectoryStatus() { return fgAddDirectory; }
    void AddDirectory(bool add) { fgAddDirectory = add; }

    void Reset();

    TObject* Clone(const char* newname = "") const override;
    void     Copy(TObject& hnew) const override;

    void SetTH1FHistogram(TH1F* hist);
    void SetTH2FHistogram(TH2F* hist);

    static bool fgAddDirectory;   ///<! Flag to add histograms to the directory

    /** All Histograms */
    std::map<TString, TH1F*> m_TH1F; /**< 1D Histograms */
    std::map<TString, TH2F*> m_TH2F; /**< 2D Histograms */

  private:
    TDirectory* fDirectory; /**< Pointer to directory holding this */
    std::map<std::string, int> histoNames;
    std::vector<TH1F*> histos;
    int somevalue;

    SVDTimeCalibContainer(const SVDTimeCalibContainer&) = delete;
    SVDTimeCalibContainer& operator=(const SVDTimeCalibContainer&) = delete;
  };

} // end namespace Belle2
