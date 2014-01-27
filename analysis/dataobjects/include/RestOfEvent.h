/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Anze Zupanc                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef RESTOFEVENT_H
#define RESTOFEVENT_H

#include <framework/datastore/RelationsObject.h>

#include <vector>
#include <set>

namespace Belle2 {

  // forward declarations
  class ECLShower;
  class ECLGamma;
  class ECLPi0;
  class Track;
  // TODO: Add support for the MdstVee and MDSTKlong dataobjects when they become available.

  /** \addtogroup dataobjects
   * @{
   */

  /**
   * This is a general purpose class for collecting reconstructed MDST data objects
   * that are not used in reconstruction of given Particle -- referred also as
   * Rest Of the Event. From remaining charged tracks, energy deposits in the ECL, etc.,
   * we infer for example the decay vertex and flavor of the tagging B meson, which are
   * needed in measurements of t-dependent CP violation, or we determine whether or not
   * the rest of the event is consistent with some B decay that involves neutrinos after
   * we have reconstructed Btag hadronically or semileptonically. Remaining charged tracks
   * and unused energy deposits are for example also needed in calculation of continuum
   * suppression variables, like Kakuno-Super-Fox-Wolfram moments. The RestOfEvent class
   * should be used as an input in all these cases.
   *
   * The RestOfEvent object is created for given existing Particle object by the RestOfEventBuilder
   * module and are related between each other with the BASF2 relation.
   *
   * Internally, the RestOfEvent class holds only StoreArray indices of all unused MDST dataobjects:
   * Tracks, ECLPi0, ECLGamma, ECLShower, MDSTVee and MDSTKlong. Indices are stored in std::set
   * and not std::vector, since the former ensures uniqueness of all its elements.
   */

  class RestOfEvent : public RelationsObject {

  public:

    /**
     * Default constructor.
     * All private members are set to 0 (all vectors are empty).
     */
    RestOfEvent() {};

    // setters
    /**
     * Add StoreArray index of given Track to the list of unused tracks in the event.
     *
     * @param Pointer to the unused Track
     */
    void addTrack(const Track* track);

    /**
     * Add given StoreArray indices to the list of unused tracks in the event.
     *
     * @param vector of SoreArray indices of unused Tracks
     */
    void addTracks(const std::vector<int>& indices);

    /**
     * Add StoreArray index of given ECLShower to the list of unused ECL showers in the event.
     *
     * @param Pointer to the unused ECLShower
     */
    void addECLShower(const ECLShower* shower);

    /**
     * Add given StoreArray indices to the list of unused ECL Showers in the event.
     *
     * @param vector of SoreArray indices of unused Showers
     */
    void addECLShowers(const std::vector<int>& indices);

    /**
     * Add StoreArray index of given ECLGamma to the list of unused ECL gammas in the event.
     *
     * @param Pointer to the unused ECLGamma
     */
    void addECLGamma(const ECLGamma* gamma);

    /**
     * Add given StoreArray indices to the list of unused ECL Gammas in the event.
     *
     * @param vector of SoreArray indices of unused Gammas
     */
    void addECLGammas(const std::vector<int>& indices);

    /**
     * Add StoreArray index of given ECLPi0 to the list of unused ECL pi0s in the event.
     *
     * @param Pointer to the unused ECLPi0
     */
    void addECLPi0(const ECLPi0* pi0);

    /**
    * Add given StoreArray indices to the list of unused ECL Pi0s in the event.
    *
    * @param vector of SoreArray indices of unused Pi0s
    */
    void addECLPi0s(const std::vector<int>& indices);

    // getters
    /**
     * Get vector of all unused Tracks.
     *
     * @return vector of pointers to unused tracks
     */
    const std::vector<Belle2::Track*> getTracks() const;

    /**
     * Get vector of all unused ECLShowers.
     *
     * @return vector of pointers to unused ECLShowers
     */
    const std::vector<Belle2::ECLShower*> getECLShowers() const;

    /**
     * Get vector of all unused ECLGammas.
     *
     * @return vector of pointers to unused ECLGammas
     */
    const std::vector<Belle2::ECLGamma*> getECLGammas() const;

    /**
     * Get vector of all unused ECLPi0s.
     *
     * @return vector of pointers to unused ECLPi0s
     */
    const std::vector<Belle2::ECLPi0*> getECLPi0s() const;

    /**
     * Get number of all remaining tracks.
     *
     * @return number of all remaining tracks
     */
    int getNTracks(void) const {
      return int(m_trackIndices.size());
    }

    /**
     * Get number of all remaining ECL showers.
     *
     * @return number of all remaining ECL showers
     */
    int getNECLShowers(void) const {
      return int(m_eclShowerIndices.size());
    }

    /**
     * Get number of all remaining ECL gammas.
     *
     * @return number of all remaining ECL gammas
     */
    int getNECLGammas(void) const {
      return int(m_eclGammaIndices.size());
    }

    /**
     * Get number of all remaining ECL pi0s.
     *
     * @return number of all remaining ECL pi0s
     */
    int getNECLPi0s(void) const {
      return int(m_eclPi0Indices.size());
    }

    /**
     * Prints the contents of a RestOfEvent object to screen
     */
    void print() const;


  private:

    // persistent data members
    std::set<int> m_trackIndices;  /**< StoreArray indices to unused tracks */
    std::set<int> m_eclPi0Indices;  /**< StoreArray indices to unused ECLPi0s */
    std::set<int> m_eclGammaIndices;  /**< StoreArray indices to unused ECLGammas */
    std::set<int> m_eclShowerIndices;  /**< StoreArray indices to unused ECLShowers */
    // TODO: add support for vee and Klong

    /**
     * Prints indices in the given set in a single line
     */
    void printIndices(std::set<int> indices) const;

    /**
     * Copies indices (elements) from "from" vector to "to" set
     */
    void addIndices(const std::vector<int>& from, std::set<int>& to) {
      for (unsigned i = 0; i < from.size(); i++)
        to.insert(from[i]);
    }

    ClassDef(RestOfEvent, 1) /**< class definition */

  };

  /** @}*/

} // end namespace Belle2

#endif
