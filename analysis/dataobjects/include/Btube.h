/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2019 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Sourav Dey, Abi Soffer                                   *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <framework/datastore/RelationsObject.h>

#include <TVector3.h>
#include <TMatrixFSym.h>

#include <vector>
#include <set>
#include <Eigen/Core>

namespace Belle2 {
  /** For each MCParticle with hits in the CDC, this class stores some summarising information on those hits. */
  class Btube : public RelationsObject {
  public:
    /** default constructor.
     *
     *  Note that you should always provide a constructor with no arguments (or with default values for
     *  all parameters).
     *  You must not allocate heap memory here (with 'new'), otherwise it will be lost when reading objects from file.
     */
    Btube()
    {
      m_tubecenter(0, 0) = 0; m_tubecenter(1, 0) = 0; m_tubecenter(2, 0) = 0;
      m_tubedirection(0, 0) = 0; m_tubedirection(1, 0) = 0; m_tubedirection(2, 0) = 0;
      resetTubeMatrix();
    }

    /**
     * Returns Btube center
     */
    Eigen::Matrix<double, 3, 1> getTubeCenter() const;
    /**
     * Returns Btube direction
     */
    Eigen::Matrix<double, 3, 1> getTubeDirection() const;
    /**
     * Returns Btube matrix
     */
    TMatrixFSym getTubeMatrix() const;
    /**
     * Sets Btube Center
     */
    void setTubeCenter(const Eigen::Matrix<double, 3, 1>& tubecenter);
    /**
     * Sets Btube Direction
     */
    void setTubeDirection(const Eigen::Matrix<double, 3, 1>& tubedirection);
    /**
     * Sets Btube Error Matrix
     */
    void setTubeMatrix(const TMatrixFSym& tubematrix);

    /** destructor, empty because we don't allocate memory anywhere. */
    ~Btube() { }


  private:
    Eigen::Matrix<double, 3, 1> m_tubecenter;   /**< Btube center */
    Eigen::Matrix<double, 3, 1> m_tubedirection;   /**< Btube center */
    TMatrixFSym m_tubematrix;   /**< Btube (3x3)  matrix */

    void resetTubeMatrix(); /**< resets the tube  matrix */
    // Note: values >=1 are versions, and should be incremented when anything about the class members is changed.
    // 0 is used to disable I/O capabilities, so always make sure this starts at 1.
    // comments after the Classdef macro are also used by ROOT to provide online documentation of a class, e.g. with the auto-generated Print() function
    ClassDef(Btube, 1); /**< For each MCParticle with hits in the CDC, this class stores some summarising information on those hits .*/
  };
}



