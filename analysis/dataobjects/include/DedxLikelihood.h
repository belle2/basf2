#ifndef DEDXLIKELIHOOD_H
#define DEDXLIKELIHOOD_H

//only used for a few constants, no linking necessary
#include <analysis/modules/dedxPID/DedxConstants.h>

#include <TObject.h>

#include <string>
#include <cmath>

class TH1F;

namespace Belle2 {
  /** \addtogroup dataobjects
   * @{
   */

  /** Container for likelihoods obtained by the dE/dx PID (DedxPIDModule).
   */
  class DedxLikelihood : public TObject {
  public:
    /** default constructor */
    DedxLikelihood():
      TObject(),
      m_p(0.0) {
      //for all particles
      for (int i = 0; i < c_Dedx_num_particles; i++) {
        m_logl[i] = 0.0;
      }
    }

    /** actually const float (&logl)[c_Dedx_num_particles], but CINT complains. */
    DedxLikelihood(const float* logl, float p):
      TObject(),
      m_p(p) {
      //for all particles
      for (int i = 0; i < c_Dedx_num_particles; i++) {
        m_logl[i] = logl[i];
      }
    }


    /** returns unnormalised log-likelihood value for a particle hypothesis.
     *
     * This can be used for classifications using the ratio
     * \f$ \mathcal{L}_m / \mathcal{L}_n \f$ of the likelihoods for two
     * particle types m and n.
     *
     * Instead of comparing hypotheses in pairs, one can also use the probability
     * for a certain hypothesis provided by getProbability(), which takes into
     * account all hypotheses and their momentum distribution.
     *
     * @param type  The desired particle hypothesis.
     */

    float getLogLikelihood(DedxParticle type) const { return m_logl[type]; }

    /** returns exp(getLogLikelihood(type)) with sufficient precision. */
    double getLikelihood(DedxParticle type) const { return exp((double)m_logl[type]); }

    /** corresponding setter for m_logl. */
    void setLogLikelihood(DedxParticle type, float logl) { m_logl[type] = logl; }

    /** the associated track momentum at the origin*/
    float getMomentum() const { return m_p; }

    /** corresponding setter for m_p */
    void setMomentum(float p) { m_p = p; }


    /** returns a probability for a particle hypothesis.
     *
     * The probability for the hypothesis m is calculated as
     * \f[
       \frac { P_m(p) \mathcal{L}_m } { \sum_k P_k(p) \mathcal{L}_k },
       \f]
     * which is limited to values in the interval [0, 1] and guarant
     * Where k iterates over all hypotheses, including m. \f$ P_m(p) \f$ is the
     * prior probability for the hypothesis m, which will in general depend on
     * the particle momentum p.
     *
     * If momentum distributions have not been set using loadMomentumPriorsFromFile()
     * or setMomentumPriors() beforehand, equal prior probabilities will be assumed.
     *
     * @param type  The desired particle hypothesis.
     */
    double getProbability(DedxParticle type) const;

    /** set the prior distributions for the momentum.
     *
     * If you want getProbability() to return meaningful values, you should probably call this.
     * Since this is a static function, this only needs to be done once.
     *
     * @param momentumPrior Array of TH1F pointers with c_Dedx_num_particles entries.
     *                      DedxLikelihood does not take ownership of the associated memory.
     */
    static void setMomentumPriors(TH1F** momentumPrior) { s_momentumPrior = momentumPrior; }

    /** Load prior distributions from a file.
     *
     * For generic BB events, the PDF file in analysis/data/ contains suitable
     * prior distributions.
     */
    static void loadMomentumPriorsFromFile(const std::string& filename);

  private:
    float m_logl[c_Dedx_num_particles]; /**< log likelihood for each particle, not including momentum prior */
    float m_p; /**< track momentum used for PDF lookups */

    static TH1F** s_momentumPrior; /**< prior distributions for the momentum, an array with c_Dedx_num_particles elements */

    ClassDef(DedxLikelihood, 1);
  };
  /*! @} */
}
#endif
