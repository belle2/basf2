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

//#include <CLHEP/Matrix/Vector.h>
//#include <CLHEP/Matrix/Matrix.h>
//#include <TMath.h>

//class HepVector ;
//class HepMatrix ;
//class TVector3;
//class BField ;
//class HepPoint ;

using namespace CLHEP;

namespace TreeFitter {
  class HelixUtils {
  public:
    enum VertexCoor {in_x = 0, in_y, in_z, in_px, in_py, in_pz} ;
    enum HelixCoor  {ex_d0 = 0, ex_phi0, ex_omega, ex_z0, ex_tanDip, ex_flt} ;

    static void vertexFromHelix(const HepVector& helixpar,
                                //const BField& fieldmap,
                                double Bz,
                                HepVector& vertexpar, int& charge) ;

    static void helixFromVertex(const HepVector& vertexpar, int charge,
                                //const BField& fieldmap,
                                double Bz,
                                HepVector& helixpar, HepMatrix& jacobian) ;

    static void helixFromVertexNumerical(const HepVector& vertexpar, int charge,
                                         //const BField& fieldmap,
                                         double Bz,
                                         HepVector& helixpar, HepMatrix& jacobian) ;

    static std::string helixParName(int i) ;
    static std::string vertexParName(int i) ;
    static void printHelixPar(const HepVector& helixpar) ;
    static void printVertexPar(const HepVector& vertexpar, int charge) ;

    static double helixPoca(const HepVector& helixpar1,
                            const HepVector& helixpar2,
                            double& flt1, double& flt2,
                            TVector3& v, bool parallel = false) ;
    static double helixPoca(const HepVector& helixpar, const TVector3& point,
                            double& flt) ;
    static double phidomain(const double phi) ;


    //FT: Added for testing:
    //    void HelixTest();
  } ;

}


#endif //HELIXUTILS_H

