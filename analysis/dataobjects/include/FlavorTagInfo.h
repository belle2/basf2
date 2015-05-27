/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Christian Roca                                           *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef FLAVORTAGINFO_H
#define FLAVORTAGINFO_H

#include <framework/datastore/RelationsObject.h>
#include <analysis/dataobjects/Particle.h>

#include <vector>
#include <set>

namespace Belle2 {

  // forward declarations

  class Track;
  class MCParticle;

  /**
   * This class stores the relevant information from the FlavorTagging. More concretely, it
   * stores the details of the most suitable tracks for the flavour tagging.
   *
   * The way the information is stored is into vectors with the same length. Every position
   * in all vectors correspond to the same particle-category information, so that every category
   * is uniquely correlated to one, e.g, track. Variables stored:
   *
   * - Track's probability of being a Btag daughter for every category
   * - Highest probability track's pointer
   * - Event probability to belong to a given category
   * - MC information obtained in the TagV module
   *
   * This class is still in an early phase, so lots of more stuff is expected soon enough.
   */

  class FlavorTagInfo : public RelationsObject {

  public:

    /**
     * Default constructor.
     * All private members are set to 0 (all vectors are empty).
     */
    FlavorTagInfo() {};

    /**
     * SETTERS
     */
    void setTrack(Belle2::Track* track); // Save the track of the most probable particle
    void setParticle(Particle*);  // Save the pointer of the most probable particle

    void setMCParticle(Belle2::MCParticle*);  // Save the pointer of the most probable particle
    void setMCParticleMother(Belle2::MCParticle*);  // Save the pointer of the most probable particle

    void setTracks(std::vector<Belle2::Track*>); // Save a vector of tracks
    void setCatProb(float); // Save the probability of belonging to a given category
    void setTargProb(float); // Save the probability of being the right target
    void setP(float); // Save the momentum of the track
    void setIsFromB(int); // Save True if the MCTruth particle comes from the B
    void setIsFromBGeneral(int);
    void setROEComesFromB(int);
    void setGoodTracksPurityFT(float);
    void setGoodBadTracksRatioFT(float);
    void setGoodTracksPurityROE(float);
    void setGoodBadTracksRatioROE(float);
    void setGoodTracksROE(int);
    void setBadTracksROE(int);
    void setGoodTracksFT(int);
    void setBadTracksFT(int);
    void setD0(double);
    void setZ0(double);
    void setFTandROEGoodTracksRatio(float);
    void setProdVertexMotherZ(float);
    void setCategories(std::string);



    /**
     * GETTERS
     */
    std::vector<Belle2::Track*> getTracks(); // Get the tracks corresponding to each cat
    Belle2::Track* getTrack(int);
    std::vector<float> getCatProb(); // Get the probabilities of every particle to belong to its correspondant category
    std::vector<float> getTargProb(); // Get the probabilites of every partcile to be the target
    std::vector<float> getP(); // Get the momentum of the track
    std::vector<Particle*> getParticle(); // Get a list of pointers to the particle of every category

    std::vector<Belle2::MCParticle*> getMCParticle(); // Get a list of pointers to the particle of every category
    std::vector<Belle2::MCParticle*> getMCParticleMother(); // Get a list of pointers to the particle of every category

    std::vector<float> getD0();
    std::vector<float> getZ0();

    std::vector<int> getIsFromB(); // Get True if MCTruth particles come from B
    int getIsFromBGeneral(); // Get True if MCTruth particles come from B
    int getROEComesFromB(); // Get True if MCTruth particles come from B
    float getGoodTracksPurityFT();
    float getGoodBadTracksRatioFT();

    float getGoodTracksPurityROE();
    float getGoodBadTracksRatioROE();
    int getBadTracksROE();
    int getGoodTracksROE();
    int getBadTracksFT();
    int getGoodTracksFT();
    float getFTandROEGoodTracksRatio();
    std::vector<std::string> getCategories();
    std::vector<float> getProdVertexMotherZ();


  private:

    // persistent data members
    std::vector<Belle2::Track*> m_tracks; // Most probable particle's track
    std::vector<Particle*> m_particle; // Most probable particle

    std::vector<Belle2::MCParticle*> m_MCparticle; // TEMPORARY
    std::vector<Belle2::MCParticle*> m_MCparticle_mother; // TEMPORARY

    std::vector<float> m_D0;
    std::vector<float> m_Z0;

    std::vector<float> m_categoryProb; // Probabilities for every category
    std::vector<float> m_targetProb; // Probabilities for every particle
    std::vector<float> m_P; // Absolute momentum of the track
    std::vector<int> m_isFromB; // True if MCTruth particles come from B
    int m_isFromBGeneral; // True if MCTruth particles come from B
    int m_ROEComesFromB;
    float m_goodTracksPurityFT;
    float m_goodBadTracksRatioFT;
    float m_goodTracksPurityROE;
    float m_goodBadTracksRatioROE;
    int m_badTracksROE;
    int m_goodTracksROE;
    int m_badTracksFT;
    int m_goodTracksFT;

    float m_FTandROEGoodTracksRatio;
    std::vector<float> m_prodVerZ;
    std::vector<std::string> m_categories;


    ClassDef(FlavorTagInfo, 3) /**< class definition */

  };


} // end namespace Belle2

#endif
