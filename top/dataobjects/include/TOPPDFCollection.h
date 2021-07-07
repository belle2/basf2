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

namespace Belle2 {

  /**
   * Class to store analytical PDF
   * relation from Tracks
   * filled top/modules/TOPPDFDebugger/src/TOPPDFDebuggerModule.cc
   */

  class TOPPDFCollection : public RelationsObject {

  public:

    /**
     * parameters to describe a Gaussian
     */
    struct Gaussian {
      float mean = 0;  /**< position */
      float width = 0; /**< width (sigma) */
      float area = 0;  /**< area (number of photons) */
      /**
       * useful constructor
       */
      Gaussian(float m, float w, float a): mean(m), width(w), area(a)
      {}
    };

    typedef std::vector<Gaussian> channelPDF_t; /**< the PDF in a given channel is a list of Gaussians*/
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

    /**
     * sets the position and momentum of the exthit in local coordinates
     */
    void setLocalPositionMomentum(const TVector3& pos, const TVector3& mom, int moduleID)
    {
      m_localHitPosition.SetXYZ(pos.X(), pos.Y(), pos.Z());
      m_localHitMomentum.SetXYZ(mom.X(), mom.Y(), mom.Z());
      m_moduleID = moduleID;
    }

    /**
     * returns the local coordinates of the exthit associated with this PDF
     */
    const TVector3& getAssociatedLocalHit() const
    {
      return m_localHitPosition;
    }

    /**
     * returns the momentum of the associated exthit in local coordinates
     */
    const TVector3& getAssociatedLocalMomentum() const
    {
      return m_localHitMomentum;
    }

    /**
     * returns slot ID of the associated exthit
     */
    int getModuleID() const {return m_moduleID;}

  private:
    std::map<int, modulePDF_t> m_data; /**< collection of samples of the pdf */
    // The following two members are useful for python modules (with no access
    // to TOPGeometryPar)
    TVector3 m_localHitPosition; /**< position of the exthit in local coordinates */
    TVector3 m_localHitMomentum; /**< momentum of the exthit in local coordinates */
    int m_moduleID = 0; /**< slot ID of the exthit */
    ClassDef(TOPPDFCollection, 3); /**< ClassDef */
  };
} // end namespace Belle2

