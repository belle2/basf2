/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <framework/datastore/RelationsObject.h>
#include <framework/gearbox/Const.h>
#include <framework/logging/Logger.h>

#include <map>
#include <vector>
#include <tuple>
#include <utility>
#include <Math/Vector3D.h>
#include <Math/Point3D.h>

namespace Belle2 {

  /**
   * Class to store analytical PDF
   * relation from Tracks
   * filled top/modules/TOPPDFDebugger/src/TOPPDFDebuggerModule.cc
   */

  class TOPPDFCollection : public RelationsObject {

  public:

    /**
     * Parameters to describe a Gaussian
     */
    struct Gaussian {
      float mean = 0;  /**< position */
      float width = 0; /**< width (sigma) */
      float area = 0;  /**< area (number of photons) */
      /**
       * Useful constructor
       * @param m mean
       * @param w width
       * @param a area
       */
      Gaussian(float m, float w, float a): mean(m), width(w), area(a)
      {}
    };

    typedef std::vector<Gaussian> channelPDF_t; /**< the PDF in a given channel is a list of Gaussians*/
    typedef std::array<channelPDF_t, 512> modulePDF_t; /**< the PDF of the module is a list of 512 channel PDFs*/

    /**
     * Default constructor
     */
    TOPPDFCollection() { }

    /**
     * Adds the pdf for the given hypothesis (PDG code)
     * @param pdf pdf of a given hypothesis
     * @param hypothesis PDG code
     * @return true on success
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
     * Returns the pdf for the given hypothesis (PDG code)
     * @param hypothesis PDG code
     * @return pdf
     */
    const modulePDF_t& getHypothesisPDF(const int hypothesis) const
    {
      return m_data.at(hypothesis);
    }

    /**
     * Sets the position and momentum of the exthit in local coordinates
     * @param pos position
     * @param mom momentum
     * @param moduleID slot ID
     */
    void setLocalPositionMomentum(const ROOT::Math::XYZPoint& pos, const ROOT::Math::XYZVector& mom, int moduleID)
    {
      m_localHitPosition = pos;
      m_localHitMomentum = mom;
      m_moduleID = moduleID;
    }

    /**
     * Returns the local coordinates of the exthit associated with this PDF
     * @return local coordinates of the exthit
     */
    const ROOT::Math::XYZPoint& getAssociatedLocalHit() const
    {
      return m_localHitPosition;
    }

    /**
     * Returns the momentum of the associated exthit in local coordinates
     * @return local momentum of the associated exthit
     */
    const ROOT::Math::XYZVector& getAssociatedLocalMomentum() const
    {
      return m_localHitMomentum;
    }

    /**
     * Returns slot ID of the associated exthit
     * @return slot ID
     */
    int getModuleID() const {return m_moduleID;}

  private:
    std::map<int, modulePDF_t> m_data; /**< collection of samples of the pdf */
    // The following two members are useful for python modules (with no access
    // to TOPGeometryPar)
    ROOT::Math::XYZPoint m_localHitPosition; /**< position of the exthit in local coordinates */
    ROOT::Math::XYZVector m_localHitMomentum; /**< momentum of the exthit in local coordinates */
    int m_moduleID = 0; /**< slot ID of the exthit */
    ClassDef(TOPPDFCollection, 4); /**< ClassDef */
  };
} // end namespace Belle2

