/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef PIDLIKELIHOOD_H
#define PIDLIKELIHOOD_H

#include <TObject.h>

#include <framework/gearbox/Const.h>


namespace Belle2 {

  class TOPLikelihood;
  class ARICHLikelihoods;
  class DedxLikelihood;

  /** \addtogroup dataobjects
   * @{
   */

  /*! Class to collect log likelihoods from TOP, ARICH and dEdx
   * aimed for output to mdst
   * includes a function to return combined likelihood probability (like Belle1 atc_pid)
   */

  class PIDLikelihood : public TObject {

  public:

    /*! detector type enumerators, FIXME: to be replaced by the constants from Const.h */
    enum EDetector {c_Top, c_Arich, c_Dedx, c_NumofDet};

    /*! default constructor: log likelihoods and flags set to 0 */
    PIDLikelihood(): m_flags(0) {
      for (int i = 0; i < c_NumofDet; i++) {
        for (Const::ParticleType k = Const::chargedStable.begin(); k != Const::chargedStable.end(); k++) m_logl[i][k.getIndex()] = 0.0;
      }
    }

    /*! set TOP likelihoods and corresponding reconstruction flag
     * @param logl TOPLikelihood pointer
     */
    void setLikelihoods(const TOPLikelihood* logl);

    /*! set ARICH likelihoods and corresponding reconstruction flag
     * @param logl ARICHLikelihoods pointer
     */
    void setLikelihoods(const ARICHLikelihoods* logl);

    /*! set Dedx likelihoods and corresponding reconstruction flag
     * @param logl DedxLikelihood pointer
     */
    void setLikelihoods(const DedxLikelihood* logl);

    /*! get reconstruction flag for a given detector
     * @param det detector enumerator
     * @return flag
     */
    bool getFlag(EDetector det) const {return (m_flags >> det & 1);}

    /*! get log likelihood for a given detector and particle
     * @param det detector enumerator
     * @param part particle enumerator
     * @return log likelihood
     */
    float getLogL(EDetector det, const Const::ChargedStable& part) const {return m_logl[det][part.getIndex()];}

    /*! get combined likelihood probability for particle being p1 and not p2
     * @param p1 particle enumerator
     * @param p2 particle enumerator
     * @return likelihood probability P_{p1/p2} (a value btw. 0 and 1)
     */
    double getProbability(const Const::ChargedStable& p1, const Const::ChargedStable& p2) const;

    /*! get likelihood probability for particle being p1 and not p2 using single detector
     * @param p1 particle enumerator
     * @param p2 particle enumerator
     * @param det detector enumerator
     * @return likelihood probability P_{p1/p2} (a value btw. 0 and 1)
     */
    double getProbability(const Const::ChargedStable& p1, const Const::ChargedStable& p2, EDetector det) const {
      return probability(m_logl[det][p1.getIndex()], m_logl[det][p2.getIndex()]);
    }

  private:
    int m_flags;  /**< reconstruction flags (one bit for each detector)*/
    float m_logl[c_NumofDet][5]; /**< log likelihoods, FIXME: replace numerical constant */

    /*! Set flag for a given detector
     * @param det detector enumerator
     */
    void setFlag(EDetector det) { m_flags = m_flags | (1 << det);}

    /*! Calculate likelihood probability from log likelihood difference logl1-logl2
     * @param logl1 log likelihood
     * @param logl2 log likelihood
     * @return likelihood probability (a value btw. 0 and 1)
     */
    double probability(float logl1, float logl2) const;

    ClassDef(PIDLikelihood, 1); /**< ClassDef */

  };

  /** @}*/

} // end namespace Belle2

#endif
