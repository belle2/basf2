// Model: EvtDToKSKpi0
// This file is an amplitude model for D+ -> K_S0 K+ pi0.
// The model is from the BESIII Collaboration in PRD 104, 012006 (2021). DOI:&nbsp; https://doi.org/10.1103/PhysRevD.104.012006
//
// Permission to include these files in basf2 was generously granted by the BESIII Collaboration.
//
// Please cite the original reference for any public/published results where this model was used.

#pragma once

#include <EvtGenBase/EvtDecayProb.hh>

namespace Belle2 {

  class EvtDToKSKpi0: public EvtDecayProb  {

  public:

    EvtDToKSKpi0() {}
    virtual ~EvtDToKSKpi0();

    std::string getName();
    EvtDecayBase* clone();

    void init();
    void initProbMax();
    void decay(EvtParticle* p);

  private:

    double AmplitudeSquare();

    EvtVector4R GetDaugMomLab(int i) {return _p4Lab[i];}
    EvtVector4R GetDaugMomCM(int i) {return _p4CM[i];}
    EvtVector4R GetDaugMomHel(int i) {return _p4Hel[i];}

    EvtVector4R _p4Lab[10], _p4CM[10], _p4Hel[10];
    int _nd;
    EvtVector4R _pd[3];

    double twoBodyCMmom(double rMassSq, double d1m, double d2m);
    double dampingFactorSquare(const double& cmmom, const int& spin, const double& mRadius);
    double spinFactor(int spin, double motherMass, double daug1Mass, double daug2Mass, double daug3Mass, double m12, double m13,
                      double m23);
    double getM23(double massPZ, double massPM) { return (_mDp2 + KsMass * KsMass + KpMass * KpMass + pi0Mass * pi0Mass - massPZ - massPM); }
    EvtComplex RBW(int id, double resmass, double reswidth, int spin);
    EvtComplex LASS(int id, double resmass, double reswidth);
    EvtComplex Flatte(int id, double resmass, double g1, double rg2og1);

    double _mDp;
    double c_motherMass;
    double _mDp2;
    double _mDp2inv;
    double KsMass;
    double KpMass;
    double pi0Mass;
    double etamass;
    double pipMass;
    double c_meson_radius_inter;
    double c_meson_radius_Dp;

  };

} // Belle2 namespace
