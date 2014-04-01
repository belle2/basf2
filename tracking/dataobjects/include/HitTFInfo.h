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
   *  m_assignedClusterIDs (< int >) = IDs of the assigned clusters
   *  m_useCounter (int) = Countes the Cells using this Hit (alive and connected)
   *  m_maxCounter (int) = max. m_useCounter
   *  m_useCounterCell (< int >) = hit counts for the inner or outer cell => (outer cell  = Index 0, inner cell = Index 1)
   *    m_useTCCounterIDs (< int >) = Assigned TCs IDs using this Hits
   *  m_sectorID (int) = Sector ID of the Hit
   *  m_hitPos (TVector3) = Position of the Hit
   *    m_isReal (int)  0 = Particle is not real; 1 = Particle is real; 2 = contaminated TC
   *    m_usedParticles (vector pair (int, double) ) = Vector of Particles (pair: 1. int ParticleID, 2. purity)
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

    /** Standard constructor */
    HitTFInfo(int parPassIndex, int parSecId, TVector3 parPosition, TVector3 parHitsigma);

    /** getter - getsectorID SectorID */
    int getSectorID()  { return m_sectorID; }

    /** setter - sectorID */
    void setSectorID(int value) { m_sectorID = value; }

    /** getter - getAssignedCluster*/
    std::vector<int>& getAssignedCluster()  { return m_assignedClusterIDs; }

    /** add new int to Assigned Hits */
    void push_back_AssignedCluster(int newMember) {
      m_assignedClusterIDs.push_back(newMember);
    }

    /** returns size of Assigned Hits */
    int sizeAssignedCluster() { return m_assignedClusterIDs.size(); }


    /** getter - UseCounter */
    int getUseCounter()  { return m_useCounter; }

    /** getMaxCounter */
    int getMaxCounter()  { return m_maxCounter; }

    /** setter - UseCounter */
    void setUseCounter(int value) {
      m_useCounter = value;
      if (m_useCounter > m_maxCounter) {
        m_maxCounter = m_useCounter;
      }
    }

    /** UseCounte add / minus */
    void changeUseCounter(int value) {
      m_useCounter = m_useCounter + value;
      if (m_useCounter > m_maxCounter) {
        m_maxCounter = m_useCounter;
      }
    }

    /** getter - getUseCounterCell*/
    std::vector<int>& getUseCounterCell()  { return m_useCounterCell; }

    /** add new int to UseCounterCell */
    void push_back_UseCounterCell(int newMember) {
      m_useCounterCell.push_back(newMember);
    }


    /** UseCounte add / minus */
    // outer cell  = Index 0, inner cell = Index 1
    // hit counts for the inner or outer cell => counter for each one
    int changeUseCounterCell(std::vector<int> value);


    /** returns size of UseCounterCell */
    int sizeUseCounterCell() { return m_useCounterCell.size(); }

    /** getter - getUseCounterTCIDs*/
    std::vector<int>& getUseCounterTCIDs()  { return m_useTCCounterIDs; }

    /** add new int to UseCounterTCIDs */
    void push_back_UseCounterTCIDs(int newMember);

    /** returns size of UseCounterTCIDs */
    int sizeUseCounterTCIDs() { return m_useTCCounterIDs.size(); }

    /** remove int from UseCounterTCIDs */
    void remove_UseCounterTCIDs(int oldMember);


    /** getter - getAllCounterTCIDs*/
    std::vector<int>& getAllCounterTCIDs() { return m_allTCCounterIDs; }

    /** add new int to UseCounterTCIDs */
    void push_back_AllCounterTCIDs(int newMember);

    /** returns size of UseCounterTCIDs */
    int sizeAllCounterTCIDs() { return m_allTCCounterIDs.size(); }



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
    int getIsReal() { return m_isReal; }

    /** setter - isReal */
    void setIsReal(int value) { m_isReal = value; }


    /** getter - used_particles*/
    std::vector<std::pair<int, double>>& getUsedParticles() { return m_usedParticles; }

    /** getter - Particle with highest purity*/
    std::pair<int, double> getMainParticle() {

      int maxPos = 0;

      for (uint i = 0; i < m_usedParticles.size(); i++) {
        if (m_usedParticles.at(i).second > m_usedParticles.at(maxPos).second) {
          maxPos = i;
        }
      }

      return m_usedParticles.at(maxPos);
    }

    /** getter - Particle with particleID*/
    std::pair<int, double> getInfoParticle(int particleID) {

      for (uint i = 0; i < m_usedParticles.size(); i++) {
        if (m_usedParticles.at(i).first == particleID) {
          return m_usedParticles.at(i);
        }
      }

      return m_usedParticles.at(0);
    }


    /** containsParticle - Particle with particleID */
    bool containsParticle(int particleID) {

      for (uint i = 0; i < m_usedParticles.size(); i++) {
        if (m_usedParticles.at(i).first == particleID) {
          return true;
        }
      }

      return false;
    }





    /** add new to used_particles */
    void push_back_UsedParticles(std::pair<int, double> newMember) {
      m_usedParticles.push_back(newMember);
    }

    /** returns size of used_particles */
    int sizeUsedParticles() { return m_usedParticles.size(); }


  protected:
    int m_sectorID; /**< Sector ID */

    std::vector<int> m_assignedClusterIDs;      /**<  IDs of the assigned clusters, 1-2 x int */

    int m_useCounter;          /**<  Countes (alive and connected) */
    int m_maxCounter;          /**<  Max. Counts (max. m_useCounter)  */

    std::vector<int> m_useCounterCell;      /**<  Countes per Cell (alive and connected)  */
    std::vector<int> m_useTCCounterIDs;        /**<  TC IDs for counter */
    std::vector<int> m_allTCCounterIDs;        /**< All TC IDs for counter */

    TVector3 m_hitPos;    /**<  Position of the Hit */
    TVector3 m_hitSigma;  /**<  Hit Sigma */

    int m_isReal;  /**< 0 = Particle is not real; 1 = Particle is real */
    std::vector<std::pair<int, double>> m_usedParticles;   /**< Vector of Particles (pair: 1. int ParticleID, 2. purity */

    ClassDef(HitTFInfo, 1)
  };
}
