#ifndef FITTER3DUTILITY_H
#define FITTER3DUTILITY_H

#include <TVectorD.h>
#include <TLorentzVector.h>
#include <TVector3.h>
#include <TVector2.h>
#include <map>

class Fitter3DUtility{

  public:

    // 2D fitter functions
    static int findSign(double *phi2);

    static void rPhiFit(double *rr, double *phi2, double *phierror, double &rho, double &myphi0);

    static void rPhiFit2(double *rr, double *phi2, double *phierror, double &rho, double &myphi0, int nTS);

    // Pre 3D fitter functions
    static double calPhi(double wirePhi, double driftLength, double eventTime, double rr, int lr);

    static double calStAxPhi(int mysign, double anglest, double ztostraw, double rr, double rho, double myphi0);

    static double calDeltaPhi(int mysign, double anglest, double ztostraw, double rr, double phi2, double rho, double myphi0);

    static double calZ(int mysign, double anglest, double ztostraw, double rr, double phi2, double rho, double myphi0);

    static double calS(double rho, double rr);

    // 3D fitter functions
    static void rSFit(double *iezz2, double *arcS, double *zz, double &z0, double &cot,double &zchi2);

    static void rSFit2(double *iezz21, double *iezz22, double *arcS, double *zz,int *lutv,  double &z0, double &cot,double &zchi2);

    // MC calculation functions
    static void findImpactPosition(TVector3 * mcPosition, TLorentzVector * mcMomentum, int charge, TVector2 & helixCenter, TVector3 & impactPosition);

    // HelixParameters: dR, phi0, keppa, dz, tanLambda
    static void calHelixParameters(TVector3 position, TVector3 momentum, int charge, TVectorD& helixParameters); 

    static void calVectorsAtR(TVectorD& helixParameters, int charge, double radius, TVector3& position, TVector3& momentum);

    // Firmware convert functions
    static int convertToInt(double value, std::vector<double> convertInformation);

    static double convertToDouble(int value, std::vector<double> convertInformation);

    // Old firmware functions
    static int bitSize(int numberBits, int mode);

    static void changeInteger(int &integer, double real, double minValue, double maxValue, int bitSize);

    static void changeReal(double &real, int integer, double minValue, double maxValue, int bitSize);

    static void findExtreme(double &m_max, double &m_min, double value);
};

#endif
