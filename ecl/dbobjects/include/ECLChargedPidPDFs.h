/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * Parameters of the ECL PDFs for charged particles.                      *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marco Milesi (marco.milesi@unimelb.edu.au)               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <framework/logging/Logger.h>
#include <framework/gearbox/Unit.h>
#include <framework/gearbox/Const.h>

#include <map>

#include <TObject.h>
#include <TH2F.h>
#include <TF1.h>

namespace Belle2 {

  /** Class representing the DB payload w/ information about ECL PDF parameters for a set of particle hypotheses. */
  class ECLChargedPidPDFs: public TObject {

    typedef std::map<int, TH2F> BinsHistoByParticle; /**< Typedef */
    typedef std::map<int, TF1> PdfsByBinIdxs; /**< Typedef */
    typedef std::map<int, PdfsByBinIdxs > PdfsMapByParticle; /**< Typedef */

  public:

    /** Default constructor */
    ECLChargedPidPDFs() {};

    /** Destructor */
    ~ECLChargedPidPDFs() {};

    /** Set the energy unit to be consistent w/ the one used in the fit.
     */
    void setEnergyUnit(const double& unit) { m_energy_unit = unit; }

    /** Set the angular unit to be consistent w/ the one used in the fit.
     */
    void setAngularUnit(const double& unit) { m_ang_unit = unit; }

    /** Set the 2D grid (now we use (P,theta)) w/ the binning of the PDF:
    @param hypo the particle hypothesis ( +/-*Const::ChargedStableSet.at(i)).
    @param binhist the 2D histogram w/ the bin grid.
     */
    void setBinsHist(const int hypo, const TH2F& binhist)
    {
      m_binshisto.emplace(hypo, binhist);
    };

    /** Return 2D bins grid histogram for the given particle and charge hypothesis.
    @param hypo the particle hypothesis ( +/-*Const::ChargedStableSet.at(i)).
     */
    TH2F* getBinsHist(const int hypo)
    {
      return &m_binshisto[hypo];
    }

    /** Set the PDF map :
    @param hypo the particle hypothesis ( +/-*Const::ChargedStableSet.at(i)).
    @param i the index along the 2D grid X axis.
    @param j the index along the 2D grid Y axis.
    @param pdf the pdf object.
     */
    void setPDFsMap(const int hypo, const unsigned int i, const unsigned int j, const TF1& pdf)
    {
      auto ij = getBinsHist(hypo)->GetBin(i, j);

      PdfsByBinIdxs pdfsbybinidxs = { {ij, pdf} };

      m_pdfsmap.emplace(hypo, pdfsbybinidxs);
    };

    /** Return the PDF for the given particle hypothesis, for this reconstructed p, theta.
    @param hypo the particle hypothesis ( +/-*Const::ChargedStableSet.at(i)).
    @param theta the reconstructed polar angle of the particle's track.
    @param p the reconstructed momentum of the particle's track.
     */
    TF1* getPdf(const int hypo, const double& theta, const double& p)
    {
      //PdfsByBinIdxs pdfsbybinidxs = m_pdfsmap[hypo];

      TH2F* binhist = getBinsHist(hypo);

      auto ij = binhist->FindBin(theta * m_ang_unit, p * m_energy_unit);

      return &(m_pdfsmap[hypo][ij]);
    }

  private:

    double m_ang_unit = Unit::GeV; /**< The angular unit used for the binning. */
    double m_energy_unit = Unit::rad; /**< The energy unit used for the binning. */

    BinsHistoByParticle
    m_binshisto; /**< This map contains the 2D grid histograms describing the PDF binning for each particle hypothesis.
          The key corresponds to the particle hypothesis index (+/-*Const::ChargedStableSet.at(i)).
          The mapped value is the 2D bin grid as a histogram.
            */
    PdfsMapByParticle m_pdfsmap;  /**< This map contains the actual PDFs, for each particle hypothesis and 2D bin indexes.
             The key corresponds to the particle hypothesis index (+/-*Const::ChargedStableSet.at(i)).
             The mapped value is a map whose key is the global bin index in the 2D grid, and the mapped value
             is a TF1 representing the normalised PDF for that particle in that bin.
          */

    ClassDef(ECLChargedPidPDFs, 1); /**< ClassDef */
  };
} // end namespace Belle2
