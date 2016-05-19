#ifndef FITTER3DUTILITY_H
#define FITTER3DUTILITY_H

#include <TVectorD.h>
#include <TLorentzVector.h>
#include <TVector3.h>
#include <TVector2.h>
#include <map>
#include <string>
#include <tuple>
namespace Belle2{
  class TRGCDCJSignal;
  class TRGCDCJSignalData;
  class TRGCDCJLUT;
};


/// A class that holds functions for 3D tracking.
class Fitter3DUtility{

  public:

    /// 2D fitter functions
    static int findSign(double *phi2);

    /// A circle fitter
    static void rPhiFit(double *rr, double *phi2, double *phierror, double &rho, double &myphi0);

    /// A circle fitter with invPhiError without fit chi2 output.
    static void rPhiFitter(double *rr, double *phi2, double *invphierror, double &rho, double &myphi0);

    /// A circle fitter with invPhiError with fit chi2 output.
    static void rPhiFitter(double *rr, double *phi2, double *invphierror, double &rho, double &myphi0, double &chi2);

    /// A circle fitter
    static void rPhiFit2(double *rr, double *phi2, double *phierror, double &rho, double &myphi0, int nTS);

    /// Pre 3D fitter functions. rr is in cm scale. driftLength is in cm scale.
    static double calPhi(double wirePhi, double driftLength, double rr, int lr);

    /// Pre 3D fitter functions. 
    static double calPhi(double wirePhi, double driftTime, double eventTime, double rr, int lr);

    /// Pre 3D fitter functions
    static double calPhi(int localId, int nWires, double driftTime, double eventTime, double rr, int lr);

    /// Rotates to range [-pi, pi]
    static double rotatePhi(double value, double refPhi);

    /// Rotates to range [-pi, pi]. Use tsId as reference.
    static double rotatePhi(double value, int refId, int nTSs);

    /// Rotates to range [0, nTSs-1].
    static int rotateTsId(int value, int refId, int nTSs);

    /// Calculates the fitted axial phi for the stereo super layer.
    static double calStAxPhi(int charge, double anglest, double ztostraw, double rr, double rho, double phi0);

    /// Calculates the phi difference between fitted axial phi and stereo phi.
    static double calDeltaPhi(int charge, double anglest, double ztostraw, double rr, double phi, double rho, double phi0);

    /// Calculates z.
    static double calZ(int charge, double anglest, double ztostraw, double rr, double phi, double rho, double phi0);

    /// Calculates z using JSignal class.
    static void calZ(std::map<std::string, double> const & mConstants, std::map<std::string, Belle2::TRGCDCJSignal> & mSignalStorage, std::map<std::string, Belle2::TRGCDCJLUT * > & mLutStorage);

    /// Calculates z using JSignal and multiple constants.(This will be deprecated.)
    static void calZ(std::map<std::string, double> const & mConstD, std::map<std::string, std::vector<double> > const & mConstV, std::map<std::string, Belle2::TRGCDCJSignal> & mSignalStorage, std::map<std::string, Belle2::TRGCDCJLUT * > & mLutStorage);

    /// Calculates arc length.
    static double calS(double rho, double rr);

    /// Calculates arc length using JSingal class.
    static void calS(std::map<std::string, double> const & mConstants, std::map<std::string, Belle2::TRGCDCJSignal> & mSignalStorage, std::map<std::string, Belle2::TRGCDCJLUT * > & mLutStorage);

    /// Calculates arc length using JSignal and multiple constants. (This will be deprecated.)
    static void calS(std::map<std::string, double> const & mConstD, std::map<std::string, std::vector<double> > const & mConstV, std::map<std::string, Belle2::TRGCDCJSignal> & mSignalStorage, std::map<std::string, Belle2::TRGCDCJLUT * > & mLutStorage);

    /// Chooses and calculates inverse z error.
    static double calIZError2(int lr, double driftZError, double wireZError);

    /// Calculates the denominator for fitting z and arc s.
    static double calDen(double* arcS, double* zError);

    /// 3D fitter functions
    /// Fits z and arc S.
    static void rSFit(double *iezz2, double *arcS, double *zz, double &z0, double &cot,double &zchi2);

    /// Fits z and arc S. (Will be deprecated.)
    static void rSFit2(double *iezz21, double *iezz22, double *arcS, double *zz,int *lutv,  double &z0, double &cot,double &zchi2);

    /// Fits z and arc S using JSingal.
    static void rSFit(std::map<std::string, double> const & mConstants, std::map<std::string, Belle2::TRGCDCJSignal> & mSignalStorage, std::map<std::string, Belle2::TRGCDCJLUT * > & mLutStorage);

    /// Fits z and arc S using JSingal with multiple constants. (This will be deprecated.)
    static void rSFit(std::map<std::string, double> const & mConstD, std::map<std::string, std::vector<double> > const & mConstV, std::map<std::string, Belle2::TRGCDCJSignal> & mSignalStorage, std::map<std::string, Belle2::TRGCDCJLUT * > & mLutStorage);
    /// MC calculation functions
    /// Calculates the impact position of track.
    static void findImpactPosition(TVector3 * mcPosition, TLorentzVector * mcMomentum, int charge, TVector2 & helixCenter, TVector3 & impactPosition);

    /// HelixParameters: dR, phi0, keppa, dz, tanLambda
    /// Calculates the helix parameters of track.
    static void calHelixParameters(TVector3 position, TVector3 momentum, int charge, TVectorD& helixParameters); 

    /// Calculates position and momentum at a certain radius.
    static void calVectorsAtR(TVectorD& helixParameters, int charge, double radius, TVector3& position, TVector3& momentum);

    /// Firmware convert functions

    /// Old firmware functions. Currently used in Hough3DFinder.
    /// Calculates maximum for a given number of bits. (Will be deprecated.)
    static int bitSize(int numberBits, int mode);
    
    /// Changes float to integer value.
    static void changeInteger(int &integer, double real, double minValue, double maxValue, int bitSize);

    /// Changes integer to float value.
    static void changeReal(double &real, int integer, double minValue, double maxValue, int bitSize);

    /// Finds maximum and minium values.
    static void findExtreme(double &m_max, double &m_min, double value);
};

#endif
