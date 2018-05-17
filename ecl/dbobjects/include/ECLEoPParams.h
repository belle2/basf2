/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * Parameters of the E/p PDFs for charged particles.                      *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marco Milesi (marco.milesi@unimelb.edu.au)               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <framework/logging/Logger.h>
#include <framework/gearbox/Const.h>

#include <ecl/electronId/ParameterMap.h>

#include <TObject.h>

namespace Belle2 {

  /** Class to hold the information for a charged particle's E/p PDF parameters in a ROOT DB payload */

  class ECLEoPParams: public TObject {

  public:

    /** Default constructor */
    ECLEoPParams() {};

    /** Constructor */
    ECLEoPParams(const std::string& parametersFileName)
    {

      // Here need to fill the DB object w/ the info that is contained in the .dat file
      ParameterMap map(parametersFileName.c_str());
      ECLAbsPdf::init(map);



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

    /** Destructor */
    ~ECLEoPParams() {};

    /** Return pdf for the PXD dE/dx for the given particle
     * @param particle number (as in Const::ChargedStable::c_SetSize)
     */
    TH2F getPXDPDF(int part)
    {
      return m_PXDpdfs[part];
    }

    /** Return pdf for the PXD dE/dx for the given particle
     * @param particle number (as in Const::ChargedStable::c_SetSize)
     */
    TH2F getSVDPDF(int part)
    {
      return m_SVDpdfs[part];
    }

    /** Return pdf for the PXD dE/dx for the given particle
     * @param particle number (as in Const::ChargedStable::c_SetSize)
     */
    TH2F getCDCPDF(int part)
    {
      return m_CDCpdfs[part];
    }

    /** Return pdf for the PXD dE/dx for the given particle
     * @param particle number (as in Const::ChargedStable::c_SetSize)
     */
    TH2F getPXDTruncatedPDF(int part)
    {
      return m_PXDpdfs_trunc[part];
    }

    /** Return pdf for the PXD dE/dx for the given particle
     * @param particle number (as in Const::ChargedStable::c_SetSize)
     */
    TH2F getSVDTruncatedPDF(int part)
    {
      return m_SVDpdfs_trunc[part];
    }

    /** Return pdf for the PXD dE/dx for the given particle
     * @param particle number (as in Const::ChargedStable::c_SetSize)
     */
    TH2F getCDCTruncatedPDF(int part)
    {
      return m_CDCpdfs_trunc[part];
    }

  private:
    std::vector<TH2F> m_PXDpdfs; /**< 2D histograms of PXD dE/dx versus momentum */
    std::vector<TH2F> m_SVDpdfs; /**< 2D histograms of SVD dE/dx versus momentum */
    std::vector<TH2F> m_CDCpdfs; /**< 2D histograms of CDC dE/dx versus momentum */

    std::vector<TH2F> m_PXDpdfs_trunc; /**< 2D histograms of PXD dE/dx versus momentum */
    std::vector<TH2F> m_SVDpdfs_trunc; /**< 2D histograms of SVD dE/dx versus momentum */
    std::vector<TH2F> m_CDCpdfs_trunc; /**< 2D histograms of CDC dE/dx versus momentum */

    ClassDef(ECLEoPParams, 1); /**< ClassDef */
  };
} // end namespace Belle2
