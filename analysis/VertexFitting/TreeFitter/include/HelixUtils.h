/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 * External Contributor: Wouter Hulsbergen                                *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <string>

#include <framework/dataobjects/Helix.h>
#include <Eigen/Core>
namespace TreeFitter {

  /**  utility for helix<->x,p conversions
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

    /** get the jacobian dh={helix pars}/dx={x,y,z,px,py,pz} for the implementation of the framework helix.
     * WARNING only valid right after initialisation!
     * */
    static void getJacobianToCartesianFrameworkHelix(Eigen::Matrix<double, 5, 6>& jacobian,
                                                     const double x,
                                                     const double y,
                                                     const double z,
                                                     const double px,
                                                     const double py,
                                                     const double pz,
                                                     const double bfield,
                                                     const double charge
                                                    );

    /** get helix and jacobian from a vertex */
    static void getHelixAndJacobianFromVertexNumerical(const Eigen::Matrix<double, 1, 6>& positionAndMom,
                                                       int charge, double Bz,
                                                       Belle2::Helix& helix,
                                                       Eigen::Matrix<double, 5, 6>& jacobian);

    /** get jacobian from a vertex */
    static void getJacobianFromVertexNumerical(const Eigen::Matrix<double, 1, 6>& positionAndMom,
                                               int charge, double Bz,
                                               const Belle2::Helix& helix,
                                               Eigen::Matrix<double, 5, 6>& jacobian,
                                               double delta = 1e-5
                                              );

    /** vertex --> helix */
    static void helixFromVertex(const Eigen::Matrix<double, 1, 6>& positionAndMomentum ,
                                int charge, double Bz,
                                Belle2::Helix& helix,
                                double& L,
                                Eigen::Matrix<double, 5, 6>& jacobian);


    /** helix --> vertex */
    static void vertexFromHelix(const Belle2::Helix& helix,
                                double L, double Bz,
                                TVector3& position,
                                TVector3& momentum, int& charge);

    /** map of the helix parameters by list index */
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

  } ;

}
