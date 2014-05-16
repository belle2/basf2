/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Pablo Goldenzweig                                        *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef CONTINUUMSUPPRESSION_H
#define CONTINUUMSUPPRESSION_H

#include <framework/datastore/RelationsObject.h>

#include <vector>
#include <set>

namespace Belle2 {

  /** \addtogroup dataobjects
   * @{
   */

  /**
   * This is a class for collecting variables used in continuum suppression. These variables
   * include thrust related quantities and the Super Fox Wolfram (KSFW) moments. The moments
   * are stored in separate vectors depending on whether the use_final_state_for_sig==0 (FS0)
   * or ==1 (FS1). For FS0 the moments are calculated using the B daughters, while for FS1
   * they are calculated using the B final state daughters.
   *
   * The ContinuumSuppression object is created for given existing Particle object by the
   * ContinuumSuppressionBuilder module and are related between each other with a BASF2 relation.
   *
   * Internally, the ContinuumSuppression class holds only floats and vectors of floats.
   */

  class ContinuumSuppression : public RelationsObject {

  public:

    /**
     * Default constructor.
     * All private members are set to 0 (all vectors are empty).
     */
    ContinuumSuppression() : m_thrustBm(0), m_thrustOm(0), m_cosTBTO(0), m_cosTBz(0) {};

    void addThrustBm(float thrustBm);

    float getThrustBm(void) const {
      return m_thrustBm;
    }

    void addThrustOm(float thrustOm);

    float getThrustOm(void) const {
      return m_thrustOm;
    }

    void addCosTBTO(float cosTBTO);

    float getCosTBTO(void) const {
      return m_cosTBTO;
    }

    void addCosTBz(float cosTBz);

    float getCosTBz(void) const {
      return m_cosTBz;
    }

    void addKsfwFS0(std::vector<float> ksfwFS0);

    std::vector<float> getKsfwFS0(void) const {
      return m_ksfwFS0;
    }

    void addKsfwFS1(std::vector<float> ksfwFS1);

    std::vector<float> getKsfwFS1(void) const {
      return m_ksfwFS1;
    }


  private:

    float m_thrustBm;
    float m_thrustOm;
    float m_cosTBTO;
    float m_cosTBz;

    std::vector<float> m_ksfwFS0;
    std::vector<float> m_ksfwFS1;

    ClassDef(ContinuumSuppression, 1) /**< class definition */

  };

  /** @}*/

} // end namespace Belle2

#endif
