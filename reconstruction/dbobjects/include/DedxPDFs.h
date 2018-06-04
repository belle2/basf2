/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jake Bennett                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <framework/logging/Logger.h>
#include <framework/gearbox/Const.h>

#include <TFile.h>
#include <TObject.h>
#include <TH2F.h>

namespace Belle2 {

  /**
   *   dE/dx wire gain calibration constants
   */

  class DedxPDFs: public TObject {

  public:

    /**
     * Default constructor
     */
    DedxPDFs(): m_PXDpdfs() {};

    /**
     * Constructor
     */
    explicit DedxPDFs(TFile* pdffile)
    {
      for (unsigned int iPart = 0; iPart < Const::ChargedStable::c_SetSize; iPart++) {
        const int pdgCode = Const::chargedStableSet.at(iPart).getPDGCode();

        TH2F pxdhist;
        pdffile->Get(TString::Format("hist_d0_%d", pdgCode))->Copy(pxdhist);
        m_PXDpdfs.push_back(pxdhist);

        TH2F pxdhist_trunc;
        pdffile->Get(TString::Format("hist_d0_%d_trunc", pdgCode))->Copy(pxdhist_trunc);
        m_PXDpdfs_trunc.push_back(pxdhist_trunc);

        TH2F svdhist;
        pdffile->Get(TString::Format("hist_d1_%d", pdgCode))->Copy(svdhist);
        m_SVDpdfs.push_back(svdhist);

        TH2F svdhist_trunc;
        pdffile->Get(TString::Format("hist_d1_%d_trunc", pdgCode))->Copy(svdhist_trunc);
        m_SVDpdfs_trunc.push_back(svdhist_trunc);

        TH2F cdchist;
        pdffile->Get(TString::Format("hist_d2_%d", pdgCode))->Copy(cdchist);
        m_CDCpdfs.push_back(cdchist);

        TH2F cdchist_trunc;
        pdffile->Get(TString::Format("hist_d2_%d_trunc", pdgCode))->Copy(cdchist_trunc);
        m_CDCpdfs_trunc.push_back(cdchist_trunc);
      }
    };

    /**
     * Destructor
     */
    ~DedxPDFs() {};

    /** Return pdf for the PXD dE/dx for the given particle
     * @param particle number (as in Const::ChargedStable::c_SetSize)
     */
    TH2F* getPXDPDF(int part, bool truncated)
    {
      return truncated ? &m_PXDpdfs_trunc[part] : &m_PXDpdfs[part];
    }

    /** Return pdf for the PXD dE/dx for the given particle
     * @param particle number (as in Const::ChargedStable::c_SetSize)
     */
    TH2F* getSVDPDF(int part, bool truncated)
    {
      return truncated ? &m_SVDpdfs_trunc[part] : &m_SVDpdfs[part];
    }

    /** Return pdf for the PXD dE/dx for the given particle
     * @param particle number (as in Const::ChargedStable::c_SetSize)
     */
    TH2F* getCDCPDF(int part, bool truncated)
    {
      return truncated ? &m_CDCpdfs_trunc[part] : &m_CDCpdfs[part];
    }

  private:
    std::vector<TH2F> m_PXDpdfs; /**< 2D histograms of PXD dE/dx versus momentum */
    std::vector<TH2F> m_SVDpdfs; /**< 2D histograms of SVD dE/dx versus momentum */
    std::vector<TH2F> m_CDCpdfs; /**< 2D histograms of CDC dE/dx versus momentum */

    std::vector<TH2F> m_PXDpdfs_trunc; /**< 2D histograms of PXD dE/dx versus momentum */
    std::vector<TH2F> m_SVDpdfs_trunc; /**< 2D histograms of SVD dE/dx versus momentum */
    std::vector<TH2F> m_CDCpdfs_trunc; /**< 2D histograms of CDC dE/dx versus momentum */

    ClassDef(DedxPDFs, 2); /**< ClassDef */
  };
} // end namespace Belle2
