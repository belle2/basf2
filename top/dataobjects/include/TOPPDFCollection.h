/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Sam Cunliffe Jan Strube                                  *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <framework/datastore/RelationsObject.h>
#include <framework/gearbox/Const.h>
#include <map>
#include <vector>
#include <tuple>
#include <utility>

namespace Belle2 {

  /**
   * Class to store TOP log likelihoods (output of TOPReconstructor).
   * relation from Tracks
   * filled in top/modules/TOPReconstruction/src/TOPReconstructorModule.cc
   */

  class TOPPDFCollection : public RelationsObject {

  public:
    /**
     * typedef for parameters to describe a Gaussian
     * position, peak width, nphotons (area)
     */
    typedef std::tuple<float, float, float> gaussian_t; /**< representation of a Gaussian as the triplet mean, width, area*/
    typedef std::vector<gaussian_t> channelPDF_t; /**< the PDF in a given channel is a list of Gaussians*/
    typedef std::array<channelPDF_t, 512> modulePDF_t; /**< the PDF of the module is a list of 512 channel PDFs*/

    /**
     * default constructor
     */
    TOPPDFCollection() { }

    /**
     * adds the pdf for the given hypothesis (PDG code)
     */
    bool addHypothesisPDF(const modulePDF_t& pdf, const int hypothesis)
    {
      auto result = m_data.insert(std::make_pair(hypothesis, pdf));
      if (not result.second) {
        B2WARNING("PDF exists already for this track");
      }
      return result.second;
    }

    /**
     * returns the pdf for the given hypothesis (PDG code)
     */
    const modulePDF_t& getHypothesisPDF(const int hypothesis) const
    {
      return m_data.at(hypothesis);
    }

  private:
    std::map<int, modulePDF_t> m_data; /**< collection of samples of the pdf */
    ClassDef(TOPPDFCollection, 1); /**< ClassDef */
  };
} // end namespace Belle2

