/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributor: Francesco Tenchini                                        *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <string>

#include <framework/dataobjects/Helix.h>
#include <framework/dataobjects/UncertainHelix.h>
#include <Eigen/Core>
namespace TreeFitter {

  /**  utility for helix<->x,p conversions
   * FIXME this is still a mess
   * FIXME we still use the numerical jacobian I think
   * FIXME FIX THIS
   * */
  class HelixUtils {

  public:

    /**  */
    enum VertexCoor {iX = 0, iY, iZ, iPx, iPy, iPz} ;

    /**  */
    enum HelixCoor  {iD0 = 0, iPhi0, iOmega, iZ0, iTanLambda, iArcLength2D} ;

    /** get helix from a vertex */
    static void helixFromVertex(const Eigen::Matrix<double, 3, 1>& position,
                                const Eigen::Matrix<double, 3, 1>& momentum,
                                int charge, double Bz,
                                Belle2::Helix& helix,
                                double& L,
                                Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic>& jacobian);

    /** get helix from a vertex */
    static void helixFromVertexNumerical(const Eigen::Matrix<double, 3, 1>& position,
                                         const Eigen::Matrix<double, 3, 1>& momentum,
                                         int charge, double Bz,
                                         Belle2::Helix& helix,
                                         double& flt,
                                         Eigen::Matrix<double, 5, 6>& jacobian);

    /**  */
    static void getHelixAndJacobianFromVertexNumerical(Eigen::Matrix<double, 1, 6>& positionAndMom,
                                                       int charge, double Bz,
                                                       Belle2::Helix& helix,
                                                       Eigen::Matrix<double, 5, 6>& jacobian);

    /**  */
    static void helixFromVertex(Eigen::Matrix<double, 1, 6>& positionAndMomentum ,
                                int charge, double Bz,
                                Belle2::Helix& helix,
                                double& L,
                                Eigen::Matrix<double, 5, 6>& jacobian);


    /**  */
    static void vertexFromHelix(const Belle2::Helix& helix,
                                double L, double Bz,
                                TVector3& position,
                                TVector3& momentum, int& charge);

    /**  */
    static std::string helixParName(int i) ;

    /**  */
    static std::string vertexParName(int i) ;

    /**  */
    static void printVertexPar(const TVector3& position, const TVector3& momentum, int charge) ;

    /**  */
    static double helixPoca(const Belle2::Helix& helix1,
                            const Belle2::Helix& helix2,
                            double& flt1, double& flt2,
                            TVector3& vertex, bool parallel = false) ;

    /**  */
    static double helixPoca(const Belle2::Helix& helix, const TVector3& point,
                            double& flt) ;

    /**  */
    static double phidomain(const double phi) ;

    /**  */
    static void helixTest();

  } ;

}
