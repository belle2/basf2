/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jakob Lettenbichler                                      *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef SharedFunctions_H
#define SharedFunctions_H

#include <algorithm>
#include <string>
#include <vector>
#include <TVector3.h>
// #include <TMatrixT.h>
#include <TMatrixD.h>
#include <Eigen/Dense>
#include <iostream>
#include <sstream> // stringstream

namespace Belle2 {


  /**
   * the following function retrieves the local error in u direction of each ladder and stores it after converting the info to a global value
   * (in x and y direction)
   */
  std::vector< std::vector< std::pair<double, double > > > getGlobalizedHitErrors();

  /**
   * the following function retrieves the local error in u direction of each layer (in u and v direction)
   */
  std::vector< std::pair<double, double> > getHitErrors();

  template<typename T_type1> void expandRootBranch(const T_type1& variable, const std::string& branchName, const std::string& treeName, const std::string& rootFileName); /**< can be used for exporting data into a branch of a rootFile */

  /** simple printFunction for root library Matrices */
  void printMyMatrix(TMatrixD& aMatrix, std::stringstream& ss); /*{
    //      std::stringstream printOut;
    for (int nRow = 0; nRow < aMatrix.GetNrows(); nRow++) {
      for (int nCol = 0; nCol < aMatrix.GetNcols(); nCol++) {
        ss << aMatrix(nRow, nCol) << '\t';
}
ss << std::endl;
}
//      return printOut;
}*/
  /** simple printFunction for Eigen library Matrices */
  void printMyMatrix(Eigen::MatrixXd& aMatrix, std::stringstream& ss);

  struct PositionInfo {
    TVector3 hitPosition; /**< contains global hitPosition */
    double sigmaX; /**< error in x-direction of hitPosition in global coordinates */
    double sigmaY; /**< error of y-direction of hitPosition in global coordinates */
  }; /**< will be used by VXDTF to store information about the position of the hit. It is also in use for faster import of hit information for circleFitter (part of trackletFilter) */
} //Belle2 namespace
#endif
