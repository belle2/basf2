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

  /** TrackCandidate Info Class
   *
   *  This class is needed for information transport between the VXD Track Finder and the display.
   *
   *  Members:
   *  m_own_id (int) = Track Candidate ID
   *  m_assigned_cell_ids (< int >) =  IDs of the connected Cells
   *
   *  m_fitSuccessful = is true, when fit was successfull
   *  m_probValue = probability-value calculated by kalman fit (probability that his TC is real track)
   *  m_assignedGFTC = index number of assigned genfit::TrackCand for unique identification
   *
   *   m_is_real (int)  0 = Particle is not real; 1 = Particle is real; 2 = contaminated TC
   *   m_used_particles (vector pair (int, double) ) = Vector of Particles (pair: 1. int ParticleID, 2. purity)
   */

  class TrackCandidateTFInfo: public BaseTFInfo {
  public:

    /** Default constructor for the ROOT IO. */
    TrackCandidateTFInfo() {
      m_own_id = -1;
      m_assigned_cell_ids.clear();

      m_fitSuccessful = false;
      m_probValue = 0;
      m_assignedGFTC = -1;

      m_is_real = 0;
    }

    /** Standard constructor */
    TrackCandidateTFInfo(int par_pass_index, int par_own_id): BaseTFInfo(par_pass_index) {
      m_own_id = par_own_id;
      m_assigned_cell_ids.clear();

      m_fitSuccessful = false;
      m_probValue = 0;
      m_assignedGFTC = -1;

      m_is_real = 0;
    }

    /** getter - own_id */
    int getOwnID()  { return m_own_id; }

    /** setter - own_id */
    void setOwnID(int value) { m_own_id = value; }

    /** getter - getAssignedCell*/
    std::vector<int>& getAssignedCell()  { return m_assigned_cell_ids; }

    /** add new int to AssignedCell */
    void push_back_AssignedCell(int newMember) {
      m_assigned_cell_ids.push_back(newMember);
    }

    /** returns size of AssignedCell */
    int sizeAssignedCell() { return m_assigned_cell_ids.size(); }


    /** getter - isFitPossible returns whether fit using GenFit within the VXDTFModule was possible or not */
    bool isFitPossible()  { return m_fitSuccessful; }

    /** getter - returns index number of assigned genfit::TrackCand */
    int getAssignedGFTC()  { return m_assignedGFTC; }

    /** getter - returns calculated probability that assigned GFTC is real track */
    double getProbValue()  { return m_probValue; }

    /** setter - submits indexnumber of  assigned GFTC for unique identification */
    void assignGFTC(int index) { m_assignedGFTC = index; }

    /** setter - set true, if fit was possible*/
    void fitIsPossible(bool yesNo) { m_fitSuccessful = yesNo; }

    /** setter - submit probability value that assigned GFTC is real track */
    void setProbValue(double value) { m_probValue = value; }

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
        if (m_used_particles[i].second > m_used_particles[max_pos].second) {
          max_pos = i;
        }
      }

      return m_used_particles[max_pos];
    }

    /** getter - gets Particle with particleID*/
    std::pair<int, double> getInfoParticle(int particleID)  {

      for (uint i = 0; i < m_used_particles.size(); i++) {
        if (m_used_particles[i].first == particleID) {
          return m_used_particles[i];
        }
      }

      return m_used_particles[0];
    }


    /** containsParticle - if Particle with particleID here */
    bool containsParticle(int particleID)  {

      for (uint i = 0; i < m_used_particles.size(); i++) {
        if (m_used_particles[i].first == particleID) {
          return true;
        }
      }

      return false;
    }

    /** returns size of used_particles */
    int sizeUsedParticles() { return m_used_particles.size(); }

  protected:
    int m_own_id; /**< Track Candidate ID */

    std::vector<int> m_assigned_cell_ids;  /** IDs of the connected Cells */

    bool m_fitSuccessful; /**< is true, when fit was successfull */
    double m_probValue; /**< probability-value calculated by kalman fit (probability that his TC is real track) */
    int m_assignedGFTC; /**< index number of assigned genfit::TrackCand for unique identification */

    int m_is_real;  /**< 0 = Particle is not real; 1 = Particle is real; 2 = contaminated TC */
    std::vector<std::pair<int, double>> m_used_particles;   /**< Vector of Particles (pair: 1. int ParticleID, 2. purity */

    ClassDef(TrackCandidateTFInfo, 1)
  };
}
