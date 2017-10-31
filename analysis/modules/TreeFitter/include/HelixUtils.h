/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributor: Francesco Tenchini                                        *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef HELIXUTILS_H
#define HELIXUTILS_H

#include <string>

#include <framework/dataobjects/Helix.h>
#include <framework/dataobjects/UncertainHelix.h>

namespace TreeFitter {
  class HelixUtils {
  public:
    enum VertexCoor {iX = 0, iY, iZ, iPx, iPy, iPz} ;
    enum HelixCoor  {iD0 = 0, iPhi0, iOmega, iZ0, iTanLambda, iArcLength2D} ;

    static void vertexFromHelix(const Belle2::Helix& helix,
                                double L, double Bz,
                                TVector3& position,
                                TVector3& momentum, int& charge);

    static void helixFromVertex(const TVector3& position,
                                const TVector3& momentum,
                                int charge, double Bz,
                                Belle2::Helix& helix,
                                double& L,
                                CLHEP::HepMatrix& jacobian);

    static void helixFromVertexNumerical(const TVector3& position,
                                         const TVector3& momentum,
                                         int charge, double Bz,
                                         Belle2::Helix& helix,
                                         double& flt,
                                         CLHEP::HepMatrix& jacobian) ;

    static std::string helixParName(int i) ;
    static std::string vertexParName(int i) ;
    static void printHelixPar(const CLHEP::HepVector& helixpar) ;
    static void printVertexPar(const TVector3& position, const TVector3& momentum, int charge) ;

    static double helixPoca(const Belle2::Helix& helix1,
                            const Belle2::Helix& helix2,
                            double& flt1, double& flt2,
                            TVector3& vertex, bool parallel = false) ;

    static double helixPoca(const Belle2::Helix& helix, const TVector3& point,
                            double& flt) ;

    static double phidomain(const double phi) ;

    static void helixTest();

  } ;

}


#endif //HELIXUTILS_H

