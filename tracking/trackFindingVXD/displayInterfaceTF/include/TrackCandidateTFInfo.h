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
   *  m_ownId (int) = Track Candidate ID
   *  m_assignedCellIds (< int >) =  IDs of the connected Cells
   *
   *  m_fitSuccessful = is true, when fit was successfull
   *  m_probValue = probability-value calculated by kalman fit (probability that his TC is real track)
   *  m_assignedGFTC = index number of assigned genfit::TrackCand for unique identification
   *
   *   m_isReal (int)  0 = Particle is not real; 1 = Particle is real; 2 = contaminated TC
   *   m_usedParticles (vector pair (int, double) ) = Vector of Particles (pair: 1. int ParticleID, 2. purity)
   */

  class TrackCandidateTFInfo: public BaseTFInfo {
  public:

    /** Default constructor for the ROOT IO. */
    TrackCandidateTFInfo() {
      m_ownId = -1;
      m_assignedCellIds.clear();
      m_assignedCellCoordinates.clear();

      m_fitSuccessful = false;
      m_probValue = 0;
      m_assignedGFTC = -1;

      m_isReal = 0;
    }

    /** Standard constructor */
    TrackCandidateTFInfo(int parPassIndex, int parOwnId): BaseTFInfo(parPassIndex) {
      m_ownId = parOwnId;
      m_assignedCellIds.clear();
      m_assignedCellCoordinates.clear();

      m_fitSuccessful = false;
      m_probValue = 0;
      m_assignedGFTC = -1;

      m_isReal = 0;
    }

    /** getter - own_id */
    int getOwnID()  { return m_ownId; }

    /** setter - own_id */
    void setOwnID(int value) { m_ownId = value; }

    /** getter - getAssignedCell*/
    std::vector<int>& getAssignedCell()  { return m_assignedCellIds; }

    /** add new int to AssignedCell */
    void push_back_AssignedCell(int newMember, std::vector<TVector3> newCoordinates) {
      m_assignedCellIds.push_back(newMember);
      m_assignedCellCoordinates.insert(m_assignedCellCoordinates.end(), newCoordinates.begin(), newCoordinates.end());
    }

    /** returns size of AssignedCell */
    int sizeAssignedCell() { return m_assignedCellIds.size(); }


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

    /** getter - gets Particle with particleID*/
    std::pair<int, double> getInfoParticle(int particleID)  {

      for (uint i = 0; i < m_usedParticles.size(); i++) {
        if (m_usedParticles.at(i).first == particleID) {
          return m_usedParticles.at(i);
        }
      }

      return m_usedParticles.at(0);
    }


    /** containsParticle - if Particle with particleID here */
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

    /** returns Coordinates of the Assigned Cell */
    const std::vector<TVector3> getCoordinates() {
      return m_assignedCellCoordinates;
    }

    /** returns the String for the display - Information */
    const TString getDisplayInformation() {

      int cell1 = -1;
      if (m_assignedCellIds.size() > 0) { cell1 = m_assignedCellIds.at(0); }
      int cell2 = -1;
      if (m_assignedCellIds.size() > 1) { cell2 = m_assignedCellIds.at(1); }

      return TString::Format("Cell 1: %d, Cell 2: %d\n IsReal: %d\n Died_ID: %d ", cell1, cell2, m_isReal, getDiedID());
    }

    /** returns the String for the display - AlternativeBox */
    const TString getDisplayAlternativeBox() {

      std::pair<int, double> mainParticle = getMainParticle();
      std::string diedAt = getDiedAt();

      return TString::Format("PassIndex: %d\n Died_ID: %s, IsReal: %d, ParticleID: %d, Purity: %.3f\n  Probability: %.3f, GTFC: %d", getPassIndex(), diedAt.c_str(), m_isReal, mainParticle.first, mainParticle.second, m_probValue, m_assignedGFTC);
    }

  protected:
    int m_ownId; /**< Track Canrdidate ID */

    std::vector<int> m_assignedCellIds;  /**< IDs of the connected Cells */
    std::vector<TVector3> m_assignedCellCoordinates; /**< Connected Cell Coordinates using this TCand */


    bool m_fitSuccessful; /**< is true, when fit was successfull */
    double m_probValue; /**< probability-value calculated by kalman fit (probability that his TC is real track) */
    int m_assignedGFTC; /**< index number of assigned genfit::TrackCand for unique identification */

    int m_isReal;  /**< 0 = Particle is not real; 1 = Particle is real; 2 = contaminated TC */
    std::vector<std::pair<int, double>> m_usedParticles;   /**< Vector of Particles (pair: 1. int ParticleID, 2. purity */

    ClassDef(TrackCandidateTFInfo, 1)
  };
}
