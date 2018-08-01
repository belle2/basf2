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
   * FIXME we still use the numerical jacobian
   * */
  class HelixUtils {

  public:

    /** Parameters of the vertex  */
    enum VertexCoor {iX = 0, iY, iZ, iPx, iPy, iPz} ;

    /** Parameters of the helix */
    enum HelixCoor  {iD0 = 0, iPhi0, iOmega, iZ0, iTanLambda, iArcLength2D} ;

    /** get helix from a vertex */
    static void helixFromVertexNumerical(const Eigen::Matrix<double, 3, 1>& position,
                                         const Eigen::Matrix<double, 3, 1>& momentum,
                                         int charge, double Bz,
                                         Belle2::Helix& helix,
                                         double& flt,
                                         Eigen::Matrix<double, 5, 6>& jacobian);

    /** get helix and jacobian from a vertex */
    static void getHelixAndJacobianFromVertexNumerical(Eigen::Matrix<double, 1, 6>& positionAndMom,
                                                       int charge, double Bz,
                                                       Belle2::Helix& helix,
                                                       Eigen::Matrix<double, 5, 6>& jacobian);

    /** get jacobian from a vertex */
    static void getJacobianFromVertexNumerical(Eigen::Matrix<double, 1, 6>& positionAndMom,
                                               int charge, double Bz,
                                               Belle2::Helix& helix,
                                               Eigen::Matrix<double, 5, 6>& jacobian,
                                               double delta = 1e-5
                                              );

    /** vertex --> helix */
    static void helixFromVertex(Eigen::Matrix<double, 1, 6>& positionAndMomentum ,
                                int charge, double Bz,
                                Belle2::Helix& helix,
                                double& L,
                                Eigen::Matrix<double, 5, 6>& jacobian);


    /** helix --> vertex */
    static void vertexFromHelix(const Belle2::Helix& helix,
                                double L, double Bz,
                                TVector3& position,
                                TVector3& momentum, int& charge);

    /** map of the helix paramteres by list index */
    static std::string helixParName(int i) ;

    /** map of the vertex parameters by list index */
    static std::string vertexParName(int i) ;

    /** Print the vertex parameters */
    static void printVertexPar(const TVector3& position, const TVector3& momentum, int charge) ;

    /** POCA between two tracks */
    static double helixPoca(const Belle2::Helix& helix1,
                            const Belle2::Helix& helix2,
                            double& flt1, double& flt2,
                            TVector3& vertex, bool parallel = false) ;

    /** POCA between a track and a point */
    static double helixPoca(const Belle2::Helix& helix, const TVector3& point,
                            double& flt) ;

    /** the domain of phi */
    static double phidomain(const double phi) ;

    ///**  */
    //static void helixTest();

  } ;

}
