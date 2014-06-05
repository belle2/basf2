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

#include "BaseTFInfo.h"

namespace Belle2 {
  /** Cell TF Info Class
   *
   *  This class is needed for information transport between the VXD Track Finder and the display.
   *
   * Members:
   *  state (int) = State of the cell
   *  m_neighbours (< int >) = Positions of the Neighbour-Cells in the same Vector
   *    m_assignedHitsIds (< int >) = Assigned Hit IDs using this Cells, (outer hit  = Index 0, inner hit = Index 1)
   * m_assignedHitsCoordinates < TVector3 > = Assigned Hit Coordinates using this Cells
   *  m_useCounter (int) = Countes the TCs using this cell (alive and connected)
   *  m_maxCounter (int) = max. m_useCounter
   *    m_isReal (int)  0 = Particle is not real; 1 = Particle is real; 2 = part
   *    m_usedParticles (vector pair (int, double) ) = Vector of Particles (pair: 1. int ParticleID, 2. purity)
   *
   * Important Methodes:
   *  isOverlappedByTC: returns true if = used more then one time by TCs = Cell is overlapped = m_useCounter > 1
   *
   */

  class CellTFInfo: public BaseTFInfo {
  public:

    /** Default constructor for the ROOT IO. */
    CellTFInfo() {
      m_state = 0;
      m_useCounter = 0;
      m_maxCounter = 0;
      m_isReal = 0;
    };

    /** Standard constructor */
    CellTFInfo(int parPassIndex): BaseTFInfo(parPassIndex) {
      m_state = 0;
      m_neighbours.clear();
      m_assignedHitsIds.clear();
      m_assignedHitsCoordinates.clear();
      m_useCounter = 0;
      m_maxCounter = 0;
      m_isReal = 0;
    };


    /** getter - getState State */
    int getState()  { return m_state; }

    /** setter - State */
    void setState(int value) { m_state = value; }

    /** setter - Change */
    void changeState(int value) { m_state = m_state + value; }


    /** getter - getneighbours*/
    std::vector<int>& getNeighbours()  { return m_neighbours; }

    /** add new int to Neighbours */
    void push_back_Neighbours(int newMember) {
      m_neighbours.push_back(newMember);
    }

    /** add new int to Neighbours */
    void insert_Neighbours(std::vector<int> newMember) {
      if (int(newMember.size()) != 0) {
        m_neighbours.insert(m_neighbours.end(), newMember.begin(), newMember.end());
      }
    }


    /** returns size of Neighbours */
    int sizeNeighbours() { return m_neighbours.size(); }



    /** getter - getAssignedHits*/
    std::vector<int>& getAssignedHits()  { return m_assignedHitsIds; }

    /** add new int to Assigned Hits */
    void push_back_AssignedHits(int newMember, TVector3 newCoordinates) {
      m_assignedHitsIds.push_back(newMember);
      m_assignedHitsCoordinates.push_back(newCoordinates);
    }

    /** returns size of Assigned Hits */
    int sizeAssignedHits() { return m_assignedHitsIds.size(); }


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

    // Cell is overlapped if it is used more then one time by a TC
    /** if the Cell is overlaped */
    bool isOverlappedByTC()  {
      if (m_useCounter > 1) {
        return true;
      } else {
        return false;
      }
    }

    /** getter - isReal */
    int getIsReal()  { return m_isReal; }

    /** setter - isReal */
    void setIsReal(int value) { m_isReal = value; }


    /** getter - used_particles*/
    std::vector<std::pair<int, double>>& getUsedParticles()  { return m_usedParticles; }

    /** add new to used_particles */
    void push_back_UsedParticles(std::pair<int, double> newMember) {
      m_usedParticles.push_back(newMember);
    }

    /** getter - Particle with highest purity*/
    std::pair<int, double> getMainParticle()  {

      int maxPos = 0;

      for (uint i = 0; i < m_usedParticles.size(); i++) {
        if (m_usedParticles.at(i).second > m_usedParticles.at(maxPos).second) {
          maxPos = i;
        }
      }

      return m_usedParticles.at(maxPos);
    }

    /** getter - Particle with particleID*/
    std::pair<int, double> getInfoParticle(int particleID)  {

      for (uint i = 0; i < m_usedParticles.size(); i++) {
        if (m_usedParticles.at(i).first == particleID) {
          return m_usedParticles.at(i);
        }
      }

      return m_usedParticles.at(0);
    }


    /** containsParticle - Particle with particleID */
    bool containsParticle(int particleID)  {

      for (uint i = 0; i < m_usedParticles.size(); i++) {
        if (m_usedParticles.at(i).first == particleID) {
          return true;
        }
      }

      return false;
    }


    /** returns size of used_particles */
    int sizeUsedParticles() { return m_usedParticles.size(); }

    /** returns the String for the display - Information */
    const TString getDisplayInformation() {

      int outerHit = -1;
      if (m_assignedHitsIds.size() > 0) { outerHit = m_assignedHitsIds.at(0); }
      int innerHit = -1;
      if (m_assignedHitsIds.size() > 1) { innerHit = m_assignedHitsIds.at(1); }

      return TString::Format("OuterHit: %d, InnerHit: %d\n State: %d\n Died_ID: %d ", outerHit, innerHit, m_state, getDiedID());
    }


    /** returns Coordinates of the Assigned Hits */
    const std::vector<TVector3> getCoordinates() {
      return m_assignedHitsCoordinates;
    }


    /** returns the String for the display - AlternativeBox */
    const TString getDisplayAlternativeBox() {

      std::pair<int, double> mainParticle = getMainParticle();
      std::string diedAt = getDiedAt();

      return TString::Format("State: %d, PassIndex: %d\n Died_ID: %s, IsReal: %d, ParticleID: %d, Purity: %.3f\n  Count NB-Cells: %d", m_state, getPassIndex(), diedAt.c_str(), m_isReal, mainParticle.first, mainParticle.second, sizeNeighbours());
    }

    /** returns the Color of the Object for the display */
    Color_t getColor() const {
      if (getActive()) {
        return (kRed - m_state); // different Red for different State
      } else {
        return (kRed - 10);
      }
    }

  protected:

    int m_state; /**< State of the cell */

    std::vector<int> m_neighbours;  /**<Position of the Neighbour-Cells in the same Vector  */

    std::vector<int> m_assignedHitsIds; /**< Assigned Hit IDs using this Cells, (outer hit  = Index 0, inner hit = Index 1) */
    std::vector<TVector3> m_assignedHitsCoordinates; /**< Assigned Hit Coordinates using this Cells, (outer hit  = Index 0, inner hit = Index 1) */

    int m_useCounter;  /**< Countes the TC (alive and connected) */
    int m_maxCounter;  /**< Max. Counts of TC (max. m_useCounter)  */

    int m_isReal;  /**< 0 = Particle is not real; 1 = Particle is real */
    std::vector<std::pair<int, double>> m_usedParticles;   /**< Vector of Particles (pair: 1. int ParticleID, 2. purity */

    ClassDef(CellTFInfo, 1)
  };
}
