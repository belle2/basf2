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
   *    m_assigned_hits_ids (< int >) = Assigned Hit IDs using this Cells, (outer hit  = Index 0, inner hit = Index 1)
   *  m_use_counter (int) = Countes the TCs using this cell (alive and connected)
   *  m_max_counter (int) = max. m_use_counter
   *    m_is_real (int)  0 = Particle is not real; 1 = Particle is real; 2 = part
   *    m_used_particles (vector pair (int, double) ) = Vector of Particles (pair: 1. int ParticleID, 2. purity)
   *
   * Important Methodes:
   *  isOverlappedByTC: returns true if = used more then one time by TCs = Cell is overlapped = m_use_counter > 1
   *
   */

  class CellTFInfo: public BaseTFInfo {
  public:

    /** Default constructor for the ROOT IO. */
    CellTFInfo() {
      m_state = 0;
      m_use_counter = 0;
      m_max_counter = 0;
      m_is_real = 0;
    };

    /** Standard constructor */
    CellTFInfo(int par_pass_index): BaseTFInfo(par_pass_index) {
      m_state = 0;
      m_neighbours.clear();
      m_assigned_hits_ids.clear();
      m_use_counter = 0;
      m_max_counter = 0;
      m_is_real = 0;
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
    std::vector<int>& getAssignedHits()  { return m_assigned_hits_ids; }

    /** add new int to Assigned Hits */
    void push_back_AssignedHits(int newMember) {
      m_assigned_hits_ids.push_back(newMember);
    }

    /** returns size of Assigned Hits */
    int sizeAssignedHits() { return m_assigned_hits_ids.size(); }


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

    // Cell is overlapped if it is used more then one time by a TC
    /** if the Cell is overlaped */
    bool isOverlappedByTC()  {
      if (m_use_counter > 1) {
        return true;
      } else {
        return false;
      }
    }

    /** getter - isReal */
    int getIsReal()  { return m_is_real; }

    /** setter - isReal */
    void setIsReal(int value) { m_is_real = value; }


    /** getter - used_particles*/
    std::vector<std::pair<int, double>>& getUsedParticles()  { return m_used_particles; }

    /** add new to used_particles */
    void push_back_UsedParticles(std::pair<int, double> newMember) {
      m_used_particles.push_back(newMember);
    }

    /** getter - Particle with highest purity*/
    std::pair<int, double> getMainParticle()  {

      int max_pos = 0;

      for (uint i = 0; i < m_used_particles.size(); i++) {
        if (m_used_particles.at(i).second > m_used_particles.at(max_pos).second) {
          max_pos = i;
        }
      }

      return m_used_particles.at(max_pos);
    }

    /** getter - Particle with particleID*/
    std::pair<int, double> getInfoParticle(int particleID)  {

      for (uint i = 0; i < m_used_particles.size(); i++) {
        if (m_used_particles.at(i).first == particleID) {
          return m_used_particles.at(i);
        }
      }

      return m_used_particles.at(0);
    }


    /** containsParticle - Particle with particleID */
    bool containsParticle(int particleID)  {

      for (uint i = 0; i < m_used_particles.size(); i++) {
        if (m_used_particles.at(i).first == particleID) {
          return true;
        }
      }

      return false;
    }


    /** returns size of used_particles */
    int sizeUsedParticles() { return m_used_particles.size(); }

    /** returns the String for the display - Information */
    TString getDisplayInformation() {

      // NOT FINAL !!!

      int outerHit = -1;
      if (m_assigned_hits_ids.size() > 0) { outerHit = m_assigned_hits_ids.at(0); }
      int innerHit = -1;
      if (m_assigned_hits_ids.size() > 1) { innerHit = m_assigned_hits_ids.at(1); }

      return TString::Format("OuterHit: %d, InnerHit: %d\n State: %d\n Died_ID: %d ", outerHit, innerHit, m_state, getDiedID());
    }

    /** returns the String for the display - AlternativeBox */
    TString getDisplayAlternativeBox() {

      std::pair<int, double> main_particle = getMainParticle();
      std::string died_at = getDiedAt();

      return TString::Format("State: %d, PassIndex: %d\n Died_ID: %s, IsReal: %d, ParticleID: %d, Purity: %.3f\n  Count NB-Cells: %d", m_state, getPassIndex(), died_at.c_str(), m_is_real, main_particle.first, main_particle.second, sizeNeighbours());
    }

  protected:

    int m_state; /**< State of the cell */

    std::vector<int> m_neighbours;  /**<Position of the Neighbour-Cells in the same Vector  */

    std::vector<int> m_assigned_hits_ids; /**< Assigned Hit IDs using this Cells, (outer hit  = Index 0, inner hit = Index 1) */

    int m_use_counter;  /**< Countes the TC (alive and connected) */
    int m_max_counter;  /**< Max. Counts of TC (max. m_use_counter)  */

    int m_is_real;  /**< 0 = Particle is not real; 1 = Particle is real */
    std::vector<std::pair<int, double>> m_used_particles;   /**< Vector of Particles (pair: 1. int ParticleID, 2. purity */

    ClassDef(CellTFInfo, 1)
  };
}
