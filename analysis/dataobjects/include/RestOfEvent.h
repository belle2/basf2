/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Anze Zupanc, Matic Lubej                                 *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef RESTOFEVENT_H
#define RESTOFEVENT_H

#include <framework/datastore/RelationsObject.h>
#include <mdst/dataobjects/PIDLikelihood.h>
#include <framework/gearbox/Const.h>

#include <vector>
#include <string>
#include <set>
#include <map>

class TLorentzVector;

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
     * Add given StoreArray indices to the list of unused Tracks in the event.
     *
     * @param vector of StoreArray indices of unused Tracks
     */
    void addTracks(std::vector<int>& indices);

    /**
     * Add StoreArray index of given ECLCluster to the list of unused ECLClusters in the event.
     *
     * @param Pointer to the unused ECLClusters
     */
    void addECLCluster(const ECLCluster* cluster);

    /**
     * Add given StoreArray indices to the list of unused ECLClusters in the event.
     *
     * @param vector of StoreArray indices of unused ECLClusters
     */
    void addECLClusters(std::vector<int>& indices);

    /**
     * Add StoreArray index of given KLMCluster to the list of unused KLM clusters in the event.
     *
     * @param Pointer to the unused KLMCluster
     */
    void addKLMCluster(const KLMCluster* cluster);

    /**
     * Add given StoreArray indices to the list of unused KLM Clusters in the event.
     *
     * @param vector of StoreArray indices of unused Clusters
     */
    void addKLMClusters(std::vector<int>& indices);

    /**
     * Append the map of a priori fractions of ChargedStable particles to the ROE object. This is used whenever mass hypotheses are needed.
     * Default is pion-mass always.
     *
     * @param map of mask names and a priori fractions for each mask
     */
    void appendChargedStableFractionsSet(std::map<std::string, std::vector<double>> fractionsSet);

    /**
     * Update or add a priori ChargedStable fractions for a specific mask name in the ROE object.
     *
     * @param name of mask
     * @param a priori fractions
     */
    void updateChargedStableFractions(std::string maskName, std::vector<double> fractions);

    /**
     * Append the Track mask (set of rules for tracks) to the ROE object.
     *
     * @param map of mask names and masks for Tracks
     */
    void appendTrackMasks(std::map<std::string, std::map<unsigned int, bool>> trackMasks);

    /**
     * Update or add a new Track mask (set of rules for tracks) with a specific mask name in the ROE object.
     *
     * @param name of mask
     * @param masks for Tracks
     */
    void updateTrackMask(std::string maskName, std::map<unsigned int, bool> trackMask);

    /**
     * Append the ECLCluster mask (set of rules for clusters) to the ROE object.
     *
     * @param map of mask names and masks for ECLClusters
     */
    void appendECLClusterMasks(std::map<std::string, std::map<unsigned int, bool>> eclClusterMasks);

    /**
     * Update or add a new ECLCluster mask (set of rules for eclClusters) with a specific mask name in the ROE object.
     *
     * @param name of mask
     * @param masks for ECLClusters
     */
    void updateECLClusterMask(std::string maskName, std::map<unsigned int, bool> eclClusterMask);

    /**
     * Append the vector of V0 array indices from ROE to the map
     */
    void appendV0IDList(std::string maskName, std::vector<unsigned int>);

    // getters
    /**
     * Get vector of all (no mask) or a subset (use mask) of all Tracks in ROE.
     *
     * @param name of mask
     * @return vector of pointers to unused Tracks
     */
    std::vector<const Track*> getTracks(std::string maskName = "") const;

    /**
     * Get vector of all (no mask) or a subset (use mask) of all ECLClusters in ROE.
     *
     * @param name of mask
     * @return vector of pointers to unused ECLClusters
     */
    std::vector<const ECLCluster*> getECLClusters(std::string maskName = "") const;

    /**
     * Get vector of all unused KLMClusters.
     *
     * @return vector of pointers to unused KLMClusters
     */
    std::vector<const KLMCluster*> getKLMClusters() const;

    /**
     * Get 4-momentum vector all (no mask) or a subset (use mask) of all Tracks and ECLClusters in ROE.
     *
     * @param name of mask
     * @return 4-momentum of unused Tracks and ECLClusters in ROE
     */
    TLorentzVector get4Vector(std::string maskName = "") const;

    /**
     * Get 4-momentum vector all (no mask) or a subset (use mask) of all Tracks in ROE.
     *
     * @param name of mask
     * @return 4-momentum of unused Tracks and ECLClusters in ROE
     */
    TLorentzVector get4VectorTracks(std::string maskName = "") const;

    /**
     * Get 4-momentum vector all (no mask) or a subset (use mask) of all ECLClusters in ROE.
     *
     * @param name of mask
     * @return 4-momentum of unused Tracks and ECLClusters in ROE
     */
    TLorentzVector get4VectorNeutralECLClusters(std::string maskName = "") const;

    /**
     * Get number of all (no mask) or a subset (use mask) of all Tracks in ROE.
     *
     * @param name of mask
     * @return number of all remaining tracks
     */
    int getNTracks(std::string maskName = "") const;

    /**
     * Get number of all (no mask) or a subset (use mask) of all ECLclusters in ROE.
     *
     * @param name of mask
     * @return number of all remaining ECL clusters
     */
    int getNECLClusters(std::string maskName = "") const;

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
     * Get Track mask with specific a mask name
     *
     * @param name of mask
     * @return mask
     */
    std::map<unsigned int, bool> getTrackMask(std::string maskName) const;

    /**
     * Get ECLCluster mask with a specific mask name
     *
     * @param name of mask
     * @return mask
     */
    std::map<unsigned int, bool> getECLClusterMask(std::string maskName) const;

    /**
     * Get charged stable fractions with a specific mask name
     *
     * @param name of mask
     * @return fractions
     */
    std::vector<double> getChargedStableFractions(std::string maskName) const;

    /**
     * Get list of V0 array indices used to replace tracks in ROE with a specific mask name
     *
     * @param name of mask
     * @return list ov V0 array indices
     */
    std::vector<unsigned int> getV0IDList(std::string maskName) const;

    /**
     * Fill input parameter with a priori ChargedStable fractions with a specific mask name
     *
     * @param a priori fractions container
     * @param name of mask
     */
    void fillFractions(double fractions[], std::string maskName) const;

    /**
     * Get vector of all mask names of the ROE object
     */
    std::vector<std::string> getMaskNames() const;

    /**
     * Added helper function so creation of temporary particles and setting pid relations is not needed
     */
    double atcPIDBelleKpiFromPID(const PIDLikelihood* pid) const;

    /**
     * Prints the contents of a RestOfEvent object to screen
     */
    void print() const;


  private:

    // persistent data members
    std::set<int> m_trackIndices;      /**< StoreArray indices to unused tracks */
    std::set<int> m_eclClusterIndices; /**< StoreArray indices to unused ECLClusters */
    std::set<int> m_klmClusterIndices;  /**< StoreArray indices to unused KLMClusters */

    std::map<std::string, std::vector<double>>
                                            m_fractionsSet; /**< Map of a-priori charged FSP probabilities to be used whenever most-likely hypothesis is determined */
    std::map<std::string, std::map<unsigned int, bool>>
                                                     m_trackMasks; /**< Map of Track masks, where each mask is another map that contains track indices and boolean values based on selection criteria for each track */
    std::map<std::string, std::map<unsigned int, bool>>
                                                     m_eclClusterMasks; /**< Map of ECLCluster masks, where each mask is another map that contains cluster indices and boolean values based on selection criteria for each cluster */    // TODO: add support for vee

    std::map<std::string, std::vector<unsigned int>>
                                                  m_v0IDMap; /**< map of V0 array indices from ROE for each ROE mask to be used to update the ROE 4 momentum */

    /**
     * Prints indices in the given set in a single line
     */
    void printIndices(std::set<int> indices) const;

    /**
     * Copies indices (elements) from "from" vector to "to" set
     */
    void addIndices(std::vector<int>& from, std::set<int>& to)
    {
      for (unsigned i = 0; i < from.size(); i++)
        to.insert(from[i]);
    }

    ClassDef(RestOfEvent, 5) /**< class definition */

  };


} // end namespace Belle2

#endif
