/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oksana Brovchenko                                        *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef MCMATCHPARTICLE_H
#define MCMATCHPARTICLE_H

#include "TObject.h"
#include "TVector3.h"

#include <generators/dataobjects/MCParticle.h>

namespace Belle2 {

  /** MCMatchParticle is a utility class to store the results of MCMatching.
   *  This class is meant to be created for each MCParticle and to copy some key member variables from this particle.
   *  After the pattern recognition (or mctrackfinding or fitting or something else) information about the track candidate corresponding to this MCParticle is also stored within this class.
   *  For the moment mainly the different momenta are stored.
   */
  class MCMatchParticle : public TObject {
  public:

    /** Empty constructor. */
    MCMatchParticle();

    /** Constructor to create MCMatchParticle objects from MCParticles. */
    MCMatchParticle(MCParticle * mcp);

    /** Destructor. */
    ~MCMatchParticle();

    /** Returns the PDG value of the particle. */
    int getPDG() {return m_pdg;}

    /** Returns the mass of the particle. */
    float getMass() { return m_mass; }

    /** Returns the energy of the particle. */
    float getEnergy() {return m_energy;}

    /** Returns the momentum of the particle. */
    TVector3 getMomentum() { return m_momentum; }

    /** Returns the status of the particle: true if primary. */
    bool getPrimary() { return m_primary;}

    /** Returns the number of RecoHits produced by the particle.  */
    int getNHits() { return m_nHits; }

    /** Returns a vector of pairs: TrackCandId & Number of RecoHits from this MCP in this TrackCand. */
    std::vector<std::pair<int, int> > getTrackCandidates() { return m_trackCands; }

    /** Returns the ID of the TrackCandidate matched to the particle. */
    int getTrackCandId() { return m_trackCandId; }

    /** Returns the efficiency:  number of RecoHits collected in the matched TrackCandidate/m_nHits  * 100 */
    double getEfficiency() { return m_efficiency; }

    /** Sets the number of RecoHits produced by the particle. */
    void setNHits(int nHits);

    /** Adds a TrackCandidate to the collection of TrackCandidates in which the RecoHits produced by this MCP were collected.
     *  The Id of the TrackCandidate is added to the m_trackCands vector.
     *  If this TrackCandidates has already contributed to this MCP, the corresponding number of Hits is augmented by 1.
     */
    void addTrackCandidate(int id);

    /** Matches the particle with a TrackCandidate.
     *  This method selects the TrackCandidate with the largest contribution in m_trackCands and assigns the Id of the matched TrackCandidate and the efficiency.
     */
    void evaluateMC();

    /** Sets the momentum for matched TrackCandidate. */
    void setRecoMomentum(TVector3 momentum);

    /** Sets the fitted momentum with MC truth start values. */
    void setFitMCMomentum(TVector3 momentum);

    /** Sets the fitted momentum with start values from matched TrackCandidate. */
    void setFitRecoMomentum(TVector3 momentum);

    /** Sets the chi2 of the fit. */
    void setChi2(double chi2);


  private:


    int m_pdg;                                         /**< PDG-Code of the particle */
    float m_mass;                                      /**< Mass of the particle */
    float m_energy;                                    /**< Energy of the particle */
    TVector3 m_momentum;                               /**< Momentum of the particle */
    bool m_primary;                                    /**< Is true for primary particles */
    double m_momentumValue;                            /**< Absolute momentum value of the particle*/

    int m_nHits;                                       /**< Number of RecoHits produced by the particle */
    std::vector <std::pair<int, int> > m_trackCands;   /**< vector to store pairs <TrackCandId, Number of RecoHits from this MCP in this TrackCand> */

    int m_trackCandId;                                 /**< Id of the TrackCandidate which has the most RecoHits from the particle (-> is matched to the particle) */
    double m_efficiency;                               /**< Number of RecoHits collected in the matched TrackCandidate/m_nHits  * 100 */

    TVector3 m_recoMomentum;                           /**< Momentum of the matched TrackCandidate */
    TVector3 m_fitMCMomentum;                          /**< Fitted momentum with MC truth start values */
    TVector3 m_fitRecoMomentum;                        /**< Fitted momentum with start values from matched TrackCandidate */

    double m_fitChi2;                                  /**< Chi2 of the fit */

    ClassDef(MCMatchParticle, 1);
  };
}


#endif /* MCMATCHPARTICLE_H*/
