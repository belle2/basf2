/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <TObject.h>
#include <string>

namespace Belle2 {


  /** The payload containing all parameters for the VXD track candidate quality selection.
      Right now, it is used to configure the quality estimation method, associated
      scale factors like materialBudgetFactor and maxPt and the min_SPTC_quality requirement. */
  class VXDQualityEstimatorParameters: public TObject {
  public:
    /** Default constructor */
    VXDQualityEstimatorParameters() : m_estimationMethod("tripletFit"), m_materialBudgetFactor(1.2),  m_maxPt(0.01),
      m_minRequiredQuality(0.0) {}
    /** Destructor */
    ~VXDQualityEstimatorParameters() {}

    /** Set estimation method */
    void setEstimationMethod(const std::string& estimationMethod)
    {
      m_estimationMethod = estimationMethod;
    }

    /** Get estimation method */
    const std::string getEstimationMethod() const
    {
      return m_estimationMethod;
    }

    /** Set MaterialBudgetFactor for tripletFit */
    void setMaterialBudgetFactor(float materialBudgetFactor)
    {
      m_materialBudgetFactor =  materialBudgetFactor;
    }

    /** Get MaterialBudgetFactor for tripletFit */
    float getMaterialBudgetFactor() const
    {
      return m_materialBudgetFactor;
    }

    /** Set maximum Pt cutoff for tripletFit */
    void setMaxPt(float maxPt)
    {
      m_maxPt = maxPt;
    }

    /** Get maximum Pt cutoff for tripletFit */
    float getMaxPt() const
    {
      return m_maxPt;
    }

    /** Set VXD track candidate selection criteria */
    void setMinRequiredQuality(float minRequiredQuality)
    {
      m_minRequiredQuality = minRequiredQuality;
    }

    /** Get VXD track candidate selection criteria */
    float getMinRequiredQuality() const
    {
      return m_minRequiredQuality;
    }

  private:
    /** Identifier which estimation method to use. Valid identifiers are:
     * mcInfo
     * circleFit
     * tripletFit
     * helixFit
     * random
     */
    std::string m_estimationMethod;
    /** Only materialBudget factor used only for TripletFit method */
    float m_materialBudgetFactor;
    /** Only maxPt cutogg used only for TripletFit method */
    float m_maxPt;
    /** VXD track candidate selection criteria*/
    float m_minRequiredQuality;

    ClassDef(VXDQualityEstimatorParameters, 1);  /**< ClassDef, necessary for ROOT */
  };
}
