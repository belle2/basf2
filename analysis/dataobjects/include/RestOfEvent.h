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
#include <framework/gearbox/Const.h>
#include <TLorentzVector.h>

#include <vector>
#include <set>
#include <map>

namespace Belle2 {

  // forward declarations
  class ECLCluster;
  class KLMCluster;
  class Track;

  // TODO: Add support for the MdstVee dataobjects when they become available.


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
   * Tracks, ECLCluster, MDSTVee and KLMCluster. Indices are stored in std::set
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
     * Add StoreArray index of given ECLCluster to the list of unused ECL showers in the event.
     *
     * @param Pointer to the unused ECLCluster
     */
    void addECLCluster(const ECLCluster* shower);

    /**
     * Add given StoreArray indices to the list of unused ECL Showers in the event.
     *
     * @param vector of SoreArray indices of unused Showers
     */
    void addECLClusters(const std::vector<int>& indices);

    /**
     * Add StoreArray index of given KLMCluster to the list of unused KLM clusters in the event.
     *
     * @param Pointer to the unused KLMCluster
     */
    void addKLMCluster(const KLMCluster* cluster);

    /**
     * Set the probabilities of ChargedStable particles. This is used in ROEVariables, where mass hypotheses are needed.
     * Default is pion-mass always.
     *
     * @param vector of probabilities of ChargedStable particles
     */
    void setChargedStableFractions(const std::vector<double>& fractions);

    /**
     * Sets the map of masks for Tracks
     */
    void setTrackMasks(std::map<int, bool> trackMasks);

    /**
     * Sets the map of masks for ECLClusters
     */
    void setECLClusterMasks(std::map<int, bool> clusterMasks);

    /**
     * Add given StoreArray indices to the list of unused KLM Clusters in the event.
     *
     * @param vector of SoreArray indices of unused Clusters
     */
    void addKLMClusters(const std::vector<int>& indices);

    // getters
    /**
     * Get vector of all unused Tracks.
     *
     * @return vector of pointers to unused tracks
     */
    const std::vector<Belle2::Track*> getTracks() const;

    /**
     * Get vector of all unused ECLClusters.
     *
     * @return vector of pointers to unused ECLClusters
     */
    const std::vector<Belle2::ECLCluster*> getECLClusters() const;

    /**
     * Get vector of all unused KLMClusters.
     *
     * @return vector of pointers to unused KLMClusters
     */
    const std::vector<Belle2::KLMCluster*> getKLMClusters() const;

    /**
     * Get 4-momentum vector in lab system of all tracks and clusters in ROE
     *
     * @return total TLorentzVector in lab system of all tracks and clusters in ROE
     */
    TLorentzVector getROE4Vector() const;

    /**
     * Get number of all remaining tracks.
     *
     * @return number of all remaining tracks
     */
    int getNTracks(void) const
    {
      return int(m_trackIndices.size());
    }

    /**
     * Get number of all remaining ECL showers.
     *
     * @return number of all remaining ECL showers
     */
    int getNECLClusters(void) const
    {
      return int(m_eclClusterIndices.size());
    }

    /**
     * Get number of all remaining KLM clusters.
     *
     * @return number of all remaining KLM clusters
     */
    int getNKLMClusters(void) const
    {
      return int(m_klmClusterIndices.size());
    }

    /**
     * Prints the contents of a RestOfEvent object to screen
     */
    void print() const;

    /**
     * Getter for the map of masks for Tracks
     */
    std::map<int, bool> getTrackMasks(void) const;

    /**
     * Getter for the map of masks for ECLClusters
     */
    std::map<int, bool> getECLClusterMasks(void) const;


  private:

    // persistent data members
    std::set<int> m_trackIndices;      /**< StoreArray indices to unused tracks */
    std::set<int> m_eclClusterIndices; /**< StoreArray indices to unused ECLClusters */
    std::set<int> m_klmClusterIndices;  /**< StoreArray indices to unused KLMClusters */

    /**
     * A set of probabilities of the ChargedStable particles in the process (to be used in ROE variables).
     * If fractions = {-1}, the true particle mass based on MC information will be used, if available
     * Default is pion always.
     */
    double m_fractions[Const::ChargedStable::c_SetSize] = {0, 0, 1, 0, 0, 0};

    bool m_useTrueMassHypothesis = false;

    std::map<int, bool> m_trackMasks; /**< Map of mask values for each track to be used in ROE or not */
    std::map<int, bool> m_eclClusterMasks; /**< Map of mask values for each ECLCluster to be used in ROE or not */
    // TODO: add support for vee

    /**
     * Prints indices in the given set in a single line
     */
    void printIndices(std::set<int> indices) const;

    /**
     * Copies indices (elements) from "from" vector to "to" set
     */
    void addIndices(const std::vector<int>& from, std::set<int>& to)
    {
      for (unsigned i = 0; i < from.size(); i++)
        to.insert(from[i]);
    }

    ClassDef(RestOfEvent, 4) /**< class definition */

  };


} // end namespace Belle2

#endif
