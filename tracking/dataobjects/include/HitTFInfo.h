/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Stefan Ferstl                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once
#include <TObject.h>
#include <TVector3.h>
#include <string>
#include <algorithm>
#include <framework/logging/Logger.h>

#include "BaseTFInfo.h"

namespace Belle2 {
  /** Hit TF Info Class
   *
   *  This class is needed for information transport between the VXD Track Finder and the display.
   *
   * Members:
   *  m_assigned_cluster_IDs (< int >) = IDs of the assigned clusters
   *  m_use_counter (int) = Countes the Cells using this Hit (alive and connected)
   *  m_max_counter (int) = max. m_use_counter
   *  m_use_counter_cell (< int >) = hit counts for the inner or outer cell => (outer cell  = Index 0, inner cell = Index 1)
   *    m_use_TC_counter_IDs (< int >) = Assigned TCs IDs using this Hits
   *  m_sectorID (int) = Sector ID of the Hit
   *  m_hitPos (TVector3) = Position of the Hit
   *    m_is_real (int)  0 = Particle is not real; 1 = Particle is real; 2 = contaminated TC
   *    m_used_particles (vector pair (int, double) ) = Vector of Particles (pair: 1. int ParticleID, 2. purity)
   *
   * Important Methodes:
   *  changeUseCounterCell: returns counter of all Cell-Change-Counters, changes hit counts for the inner or outer cells
   *    isOverlappedByCellsOrTcs: returns true if Hit is overlapped due connected Cells and/or connected TCs
   *  push_back_UseCounterTCIDs: add new connected TC-ID (-1 = no new member)
   */


  class HitTFInfo: public BaseTFInfo {

  public:

    /** Default constructor for the ROOT IO. */
    HitTFInfo();

    // Standard constructor
    HitTFInfo(int par_pass_index, int par_sec_id, TVector3 par_position, TVector3 m_hitSigma);

    /** getter - getsectorID SectorID */
    int getSectorID()  { return m_sectorID; }

    /** setter - sectorID */
    void setSectorID(int value) { m_sectorID = value; }

    /** getter - getAssignedCluster*/
    std::vector<int>& getAssignedCluster()  { return m_assigned_cluster_IDs; }

    /** add new int to Assigned Hits */
    void push_back_AssignedCluster(int newMember) {
      m_assigned_cluster_IDs.push_back(newMember);
    }

    /** returns size of Assigned Hits */
    int sizeAssignedCluster() { return m_assigned_cluster_IDs.size(); }


    /** getter - UseCounter */
    int getUseCounter()  { return m_use_counter; }

    /** getMaxCounter */
    int getMaxCounter()  { return m_max_counter; }

    /** setter - UseCounter */
    void setUseCounter(int value) {
      m_use_counter = value;
      if (m_use_counter > m_max_counter) {
        m_max_counter = m_use_counter;
      }
    }

    /** UseCounte add / minus */
    void changeUseCounter(int value) {
      m_use_counter = m_use_counter + value;
      if (m_use_counter > m_max_counter) {
        m_max_counter = m_use_counter;
      }
    }

    /** getter - getUseCounterCell*/
    std::vector<int>& getUseCounterCell()  { return m_use_counter_cell; }

    /** add new int to UseCounterCell */
    void push_back_UseCounterCell(int newMember) {
      m_use_counter_cell.push_back(newMember);
    }


    /** UseCounte add / minus */
    // outer cell  = Index 0, inner cell = Index 1
    // hit counts for the inner or outer cell => counter for each one
    int changeUseCounterCell(std::vector<int> value);


    /** returns size of UseCounterCell */
    int sizeUseCounterCell() { return m_use_counter_cell.size(); }

    /** getter - getUseCounterTCIDs*/
    std::vector<int>& getUseCounterTCIDs()  { return m_use_TC_counter_IDs; }

    /** add new int to UseCounterTCIDs */
    void push_back_UseCounterTCIDs(int newMember);

    /** returns size of UseCounterTCIDs */
    int sizeUseCounterTCIDs() { return m_use_TC_counter_IDs.size(); }

    /** remove int from UseCounterTCIDs */
    void remove_UseCounterTCIDs(int oldMember);


    /** getter - getAllCounterTCIDs*/
    std::vector<int>& getAllCounterTCIDs() { return m_all_TC_counter_IDs; }

    /** add new int to UseCounterTCIDs */
    void push_back_AllCounterTCIDs(int newMember);

    /** returns size of UseCounterTCIDs */
    int sizeAllCounterTCIDs() { return m_all_TC_counter_IDs.size(); }



    /** getter - getPosition */
    TVector3 getPosition() { return m_hitPos; }

    /** setter - Position */
    void setPosition(TVector3 value) { m_hitPos = value; }


    /** getter - getHitSigma */
    TVector3 getHitSigma() { return m_hitSigma; }

    /** setter - setHitSigma */
    void setHitSigma(TVector3 value) { m_hitSigma = value; }



    /** if the Hit is overlaped */
    bool isOverlappedByCellsOrTCs();

    /** getter - isReal */
    int getIsReal() { return m_is_real; }

    /** setter - isReal */
    void setIsReal(int value) { m_is_real = value; }


    /** getter - used_particles*/
    std::vector<std::pair<int, double>>& getUsedParticles() { return m_used_particles; }

    /** getter - Particle with highest purity*/
    std::pair<int, double> getMainParticle() {

      int max_pos = 0;

      for (uint i = 0; i < m_used_particles.size(); i++) {
        if (m_used_particles[i].second > m_used_particles[max_pos].second) {
          max_pos = i;
        }
      }

      return m_used_particles[max_pos];
    }

    /** getter - Particle with particleID*/
    std::pair<int, double> getInfoParticle(int particleID) {

      for (uint i = 0; i < m_used_particles.size(); i++) {
        if (m_used_particles[i].first == particleID) {
          return m_used_particles[i];
        }
      }

      return m_used_particles[0];
    }


    /** containsParticle - Particle with particleID */
    bool containsParticle(int particleID) {

      for (uint i = 0; i < m_used_particles.size(); i++) {
        if (m_used_particles[i].first == particleID) {
          return true;
        }
      }

      return false;
    }





    /** add new to used_particles */
    void push_back_UsedParticles(std::pair<int, double> newMember) {
      m_used_particles.push_back(newMember);
    }

    /** returns size of used_particles */
    int sizeUsedParticles() { return m_used_particles.size(); }


  protected:
    int m_sectorID; /**< Sector ID */

    std::vector<int> m_assigned_cluster_IDs;      /**<  IDs of the assigned clusters, 1-2 x int */

    int m_use_counter;          /**<  Countes (alive and connected) */
    int m_max_counter;          /**<  Max. Counts (max. m_use_counter)  */

    std::vector<int> m_use_counter_cell;      /**<  Countes per Cell (alive and connected)  */
    std::vector<int> m_use_TC_counter_IDs;        /**<  TC IDs for counter */
    std::vector<int> m_all_TC_counter_IDs;        /**< All TC IDs for counter */

    TVector3 m_hitPos;    /**<  Position of the Hit */
    TVector3 m_hitSigma;  /**<  Hit Sigma */

    int m_is_real;  /**< 0 = Particle is not real; 1 = Particle is real */
    std::vector<std::pair<int, double>> m_used_particles;   /**< Vector of Particles (pair: 1. int ParticleID, 2. purity */

    ClassDef(HitTFInfo, 1)
  };
}
