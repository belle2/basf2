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
     * default constructor
     */
    TOPPDFCollection() { }
    void addHypothesisPDFSample(const std::vector<std::vector<double>>& pdfSamples, int hypot)
    {
      // TODO Check for existing hypotheses before overriding
      m_data.insert(std::make_pair(hypot, pdfSamples));
    }
    std::map<int, std::vector<std::vector<double>>> m_data; /**< collection of samples of the pdf */

  private:
    ClassDef(TOPPDFCollection, 1); /**< ClassDef */
  };
} // end namespace Belle2

