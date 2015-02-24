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

  /**
   * This class stores the relevant information from the FlavorTagging. More concretely, it
   * stores the details of the most suitable particles used in the tagging.
   *
   * The way the information is stored is into vectors with the same length. Every position
   * in all vectors correspond to the same particle-category information, so that every category
   * is uniquely correlated to one, e.g, track. Variables stored:
   *
   * - Track probability of being the right target for every category
   * - Highest probability track's pointer
   * - Event probability to belong to a given category
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
    void setTracks(std::vector<Belle2::Track*>); // Save a vector of tracks
    void setCatProb(double); // Save the probability of belonging to a given category
    void setTargProb(double); // Save the probability of being the right target
    void setIsB(bool); // Save True if the MCTruth particle comes from the B
    void setCategories(std::string);


    /**
     * GETTERS
     */
    const std::vector<Belle2::Track*> getTracks() const; // Get the tracks corresponding to each cat
    std::vector<double> getCatProb(); // Get the probabilities of every particle to belong to its correspondant category
    std::vector<double> getTargProb(); // Get the probabilites of every partcile to be the target
    std::vector<Particle*> getParticle(); // Get a list of pointers to the particle of every category
    std::vector<bool> getIsB(); // Get True if MCTruth particles come from B
    std::vector<std::string> getCategories();


  private:

    // persistent data members
    std::vector<Belle2::Track*> m_tracks; // Most probable particle's track
    std::vector<Particle*> m_particle; // Most probable particle

    std::vector<double> m_categoryProb; // Probabilities for every category
    std::vector<double> m_targetProb; // Probabilities for every particle
    std::vector<bool> m_isB; // True if MCTruth particles come from B
    std::vector<std::string> m_categories;


    ClassDef(FlavorTagInfo, 3) /**< class definition */

  };


} // end namespace Belle2

#endif
