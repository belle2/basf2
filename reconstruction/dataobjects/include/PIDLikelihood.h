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
#include <top/dataobjects/TOPLikelihood.h>
#include <arich/dataobjects/ARICHLikelihoods.h>
#include <reconstruction/dataobjects/DedxLikelihood.h>


namespace Belle2 {
  /** \addtogroup dataobjects
   * @{
   */

  /*! Class to collect log likelihoods from TOP, ARICH and dEdx
   * aimed for output to mdst
   * includes a function to return combined likelihood probability (like Belle1 atc_pid)
   */

  class PIDLikelihood : public TObject {

  public:

    /*! particle type enumerators */
    enum EParticle {c_Electron, c_Muon, c_Pion, c_Kaon, c_Proton, c_NumofPart};

    /*! detector type enumerators */
    enum EDetector {c_Top, c_Arich, c_Dedx, c_NumofDet};

    /*! default constructor: log likelihoods and flags set to 0 */
    PIDLikelihood(): m_flags(0) {
      for (int i = 0; i < c_NumofDet; i++) {
        for (int k = 0; k < c_NumofPart; k++) m_logl[i][k] = 0.0;
      }
    }

    /*! set TOP likelihoods and corresponding reconstruction flag
     * @param logl TOPLikelihood pointer
     */
    void setLikelihoods(const TOPLikelihood* logl);

    /*! set TOP likelihoods and corresponding reconstruction flag
     * @param logl TOPLikelihood reference
     */
    void setLikelihoods(const TOPLikelihood& logl) {setLikelihoods(&logl);}

    /*! set ARICH likelihoods and corresponding reconstruction flag
     * @param logl ARICHLikelihoods pointer
     */
    void setLikelihoods(const ARICHLikelihoods* logl);

    /*! set ARICH likelihoods and corresponding reconstruction flag
     * @param logl ARICHLikelihoods reference
     */
    void setLikelihoods(const ARICHLikelihoods& logl) {setLikelihoods(&logl);}

    /*! set Dedx likelihoods and corresponding reconstruction flag
     * @param logl DedxLikelihood pointer
     */
    void setLikelihoods(const DedxLikelihood* logl);

    /*! set Dedx likelihoods and corresponding reconstruction flag
     * @param logl DedxLikelihood reference
     */
    void setLikelihoods(const DedxLikelihood& logl) {setLikelihoods(&logl);}

    /*! get all reconstruction flags as int
     * @return flags
     */
    int getFlags() const {return m_flags;}

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
    float getLogL(EDetector det, EParticle part) const {return m_logl[det][part];}

    /*! get log likelihood for given indices i, k
     * @param i detector index
     * @param k particle index
     * @return log likelihood (or 0 for invalid indices)
     */
    float getLogL(unsigned int i, unsigned int k) const {
      if (i < c_NumofDet && k < c_NumofPart) return m_logl[i][k];
      return 0;
    }

    /*! get combined likelihood probability for particle being p1 and not p2
     * @param p1 particle enumerator
     * @param p2 particle enumerator
     * @return likelihood probability P_{p1/p2} (a value btw. 0 and 1)
     */
    double getProbability(EParticle p1, EParticle p2) const;

    /*! get likelihood probability for particle being p1 and not p2 using single detector
     * @param p1 particle enumerator
     * @param p2 particle enumerator
     * @param det detector enumerator
     * @return likelihood probability P_{p1/p2} (a value btw. 0 and 1)
     */
    double getProbability(EParticle p1, EParticle p2, EDetector det) const {
      return probability(m_logl[det][p1], m_logl[det][p2]);
    }

  private:
    int m_flags;  /**< reconstruction flags (one bit for each detector)*/
    float m_logl[c_NumofDet][c_NumofPart]; /**< log likelihoods */

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
