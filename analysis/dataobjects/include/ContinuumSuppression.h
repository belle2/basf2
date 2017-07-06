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

#include <TVector3.h>
#include <vector>
#include <set>

namespace Belle2 {


  /**
   * This is a class for collecting variables used in continuum suppression. These variables
   * include: thrust related quantities; the reduced Fox-Wolfram moment R2; and the Super
   * Fox Wolfram (KSFW) moments.
   *
   * The KSFW moments are stored in separate vectors depending on whether the
   * use_final_state_for_sig==0 (FS0) or ==1 (FS1). For FS0 the moments are calculated
   * using the B primary daughters, while for FS1 they are calculated using the B final state daughters.
   *
   * The CleoCones are stored in separate vectors depending on whether they are calculated
   * using all final state particles (default method), or whether they are calculated using only particles
   * from the ROE.
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
    ContinuumSuppression() : m_thrustO(0.0, 0.0, 0.0), m_thrustBm(0.0), m_thrustOm(0.0), m_cosTBTO(0.0), m_cosTBz(0.0), m_R2(0.0) {};

    // setters
    /**
     * Add ROE thrust axis.
     *
     * @param TVector3 B thrust axis
     */
    void addThrustB(TVector3 thrustB);

    /**
     * Add ROE thrust axis.
     *
     * @param TVector3 ROE thrust axis
     */
    void addThrustO(TVector3 thrustO);

    /**
     * Add magnitude of B thrust axis.
     *
     * @param Float magnitude of B thrust axis
     */
    void addThrustBm(float thrustBm);

    /**
     * Add magnitude of ROE thrust axis.
     *
     * @param Float magnitude of ROE thrust axis
     */
    void addThrustOm(float thrustOm);

    /**
     * Add cosine of the angle between the thrust axis of the B and the thrust axis of the ROE.
     *
     * @param Float cosine of the angle between the thrust axis of the B and the thrust axis of the ROE
     */
    void addCosTBTO(float cosTBTO);

    /**
     * Add cosine of the angle between the thrust axis of the B and the z-axis.
     *
     * @param Float cosine of the angle between the thrust axis of the B and the z-axis
     */
    void addCosTBz(float cosTBz);

    /**
     * Add reduced Fox-Wolfram moment R2.
     *
     * @param Float reduced Fox-Wolfram moment R2
     */
    void addR2(float R2);

    /**
     * Add vector of KSFW moments, Et, and mm2 for final state = 0.
     *
     * @param vector of KSFW moments, Et, and mm2 for final state = 0
     */
    void addKsfwFS0(std::vector<float> ksfwFS0);

    /**
     * Add vector of KSFW moments, Et, and mm2 for final state = 1.
     *
     * @param vector of KSFW moments, Et, and mm2 for final state = 1
     */
    void addKsfwFS1(std::vector<float> ksfwFS1);

    /**
     * Add vector of Cleo Cones constructed of all final state particles
     *
     * @param vector of Cleo Cones constructed of all final state particles
     */
    void addCleoConesALL(std::vector<float> cleoConesALL);

    /**
     * Add vector of Cleo Cones constructed of only ROE particles
     *
     * @param vector of Cleo Cones constructed of only ROE particles
     */
    void addCleoConesROE(std::vector<float> cleoConesROE);

    // getters
    /**
     * Get B thrust axis.
     *
     * @return TVector3 ROE thrust axis
     */
    TVector3 getThrustB(void) const
    {
      return m_thrustB;
    }

    /**
     * Get ROE thrust axis.
     *
     * @return TVector3 ROE thrust axis
     */
    TVector3 getThrustO(void) const
    {
      return m_thrustO;
    }

    /**
     * Get magnitude of B thrust axis.
     *
     * @return Float magnitude of B thrust axis
     */
    float getThrustBm(void) const
    {
      return m_thrustBm;
    }

    /**
     * Get magnitude of ROE thrust axis.
     *
     * @return Float magnitude of ROE thrust axis
     */
    float getThrustOm(void) const
    {
      return m_thrustOm;
    }

    /**
     * Get cosine of the angle between the thrust axis of the B and the thrust axis of the ROE.
     *
     * @return Float cosine of the angle between the thrust axis of the B and the thrust axis of the ROE
     */
    float getCosTBTO(void) const
    {
      return m_cosTBTO;
    }

    /**
     * Get cosine of the angle between the thrust axis of the B and the z-axis.
     *
     * @return Float cosine of the angle between the thrust axis of the B and the z-axis
     */
    float getCosTBz(void) const
    {
      return m_cosTBz;
    }

    /**
     * Get reduced Fox-Wolfram moment R2.
     *
     * @return Float reduced Fox-Wolfram moment R2
     */
    float getR2(void) const
    {
      return m_R2;
    }

    /**
     * Get vector of KSFW moments, Et, and mm2 for final state = 0.
     *
     * @return vector of KSFW moments, Et, and mm2 for final state = 0
     */
    std::vector<float> getKsfwFS0(void) const
    {
      return m_ksfwFS0;
    }

    /**
     * Get vector of KSFW moments, Et, and mm2 for final state = 1.
     *
     * @return vector of KSFW moments, Et, and mm2 for final state = 1
     */
    std::vector<float> getKsfwFS1(void) const
    {
      return m_ksfwFS1;
    }

    /**
     * Get vector of Cleo Cones constructed of all final state particles.
     *
     * @return vector of Cleo Cones constructed of all final state particles
     */
    std::vector<float> getCleoConesALL(void) const
    {
      return m_cleoConesALL;
    }

    /**
     * Get vector of Cleo Cones constructed of only ROE particles.
     *
     * @return vector of Cleo Cones constructed of only ROE particles
     */
    std::vector<float> getCleoConesROE(void) const
    {
      return m_cleoConesROE;
    }


  private:

    // persistent data members
    TVector3 m_thrustO; /**< ROE thrust axis */
    TVector3 m_thrustB; /**< B thrust axis */

    float m_thrustBm;   /**< magnitude of B thrust axis */
    float m_thrustOm;   /**< magnitude of ROE thrust axis */
    float m_cosTBTO;    /**< cosine of the angle between the thrust axis of the B and the thrust axis of the ROE */
    float m_cosTBz;     /**< cosine of the angle between the thrust axis of the B and the z-axis */
    float m_R2;         /**< reduced Fox-Wolfram moment R2 */

    std::vector<float> m_ksfwFS0;  /**< vector of KSFW moments, Et, and mm2 for final state = 0 */
    std::vector<float> m_ksfwFS1;  /**< vector of KSFW moments, Et, and mm2 for final state = 1 */

    std::vector<float> m_cleoConesALL;  /**< vector of Cleo Cones constructed from all final state particles */
    std::vector<float> m_cleoConesROE;  /**< vector of Cleo Cones constructed from only ROE particles */

    ClassDef(ContinuumSuppression, 1) /**< class definition */

  };


} // end namespace Belle2

#endif
