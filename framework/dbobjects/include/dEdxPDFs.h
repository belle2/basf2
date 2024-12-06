/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

/* basf2 headers. */
#include <framework/gearbox/Const.h>

/* C++ headers. */
#include <array>
#include <vector>

/* ROOT headers. */
#include <TH2F.h>
#include <TObject.h>

namespace Belle2 {

  /**
   * Base class for holding the dE/dx PDFs.
   */

  class dEdxPDFs: public TObject {

  public:

    /**
     * Default constructor.
     */
    dEdxPDFs() = default;

    /**
     * Return the dE/dx PDF for the given particle hypothesis.
     * @param chargedStable Particle hypothesis
     * @param truncated If true, return the truncated dE/dx PDF
     */
    const TH2F* getPDF(const Const::ChargedStable& chargedStable, bool truncated) const
    {
      return getPDF(chargedStable.getIndex(), truncated);
    }

    /**
     * Return the dE/dx PDF for the given particle hypothesis.
     * @param hypothesis Particle hypothesis (as in Const::ChargedStable::c_SetSize)
     * @param truncated If true, return the truncated dE/dx PDF
     */
    const TH2F* getPDF(const unsigned int hypothesis, const bool truncated) const
    {
      return truncated ? &(m_dEdxPDFsTruncated.at(hypothesis)) : &(m_dEdxPDFs.at(hypothesis));
    }

    /**
     * Return all PDF's as a vector of histogram pointers
     * @param truncated If true, return the truncated PDF's
     * @return vector of histogram pointers
     */
    const std::vector<const TH2F*>& getPDFs(bool truncated) const;

    /**
     * Set the dE/dx PDF for the given particle hypothesis.
     * @param pdf dE/dx PDF as a 2D histogram (`TH2F`)
     * @param chargedStable Particle hypothesis
     * @param truncated If true, set the truncated dE/dx PDF
     */
    void setPDF(const TH2F& pdf, const Const::ChargedStable& chargedStable, bool truncated)
    {
      setPDF(pdf, chargedStable.getIndex(), truncated);
    }

    /**
     * Set the dE/dx PDF for the given particle hypothesis.
     * @param pdf dE/dx PDF as a 2D histogram (`TH2F`)
     * @param hypothesis Particle hypothesis (as in Const::ChargedStable::c_SetSize)
     * @param truncated If true, set the truncated dE/dx PDF
     */
    void setPDF(const TH2F& pdf, const unsigned int hypothesis, const bool truncated)
    {
      if (truncated)
        m_dEdxPDFsTruncated.at(hypothesis) = pdf;
      else
        m_dEdxPDFs.at(hypothesis) = pdf;
    }

    /**
     * Check PDF histograms if they are defined in the same range and with the same binning.
     * @param truncated if true check truncated histograms, otherwise check others
     * @return true if all histograms have the same range and binning.
     */
    bool checkPDFs(bool truncated) const;

  private:

    /** Array of dE/dx PDFs for each particle hypothesis. */
    std::array<TH2F, Const::ChargedStable::c_SetSize> m_dEdxPDFs;

    /** Array of truncated dE/dx PDFs for each particle hypothesis. */
    std::array<TH2F, Const::ChargedStable::c_SetSize> m_dEdxPDFsTruncated;

    /** cache for both vectors of pointers */
    mutable std::vector<const TH2F*> m_cachePDFs;  //! do not write out
    mutable std::vector<const TH2F*> m_cachePDFsTruncated;  //! do not write out

    /** Class version for the ROOT streamer. */
    ClassDef(dEdxPDFs, 1);

  };

}
