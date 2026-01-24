// Model: EvtD0ToKpipi0pi0
// This file is an amplitude model for D0 -> K- pi+ pi0 pi0.
// The model is from the BESIII Collaboration in PRD 99, 092008 (2019). DOI:&nbsp;https://doi.org/10.1103/PhysRevD.99.092008
//
// Permission to include these files in basf2 was generously granted by the BESIII Collaboration.
//
// Please cite the original reference for any public/published results where this model was used.

//#ifndef EVTD0TOKPIPI0PI0_HH  // Commented out
//#define EVTD0TOKPIPI0PI0_HH  // Commented out
#pragma once // Added

#include "EvtGenBase/EvtDecayProb.hh"

// class EvtParticle;  // Commented out

namespace Belle2 { // Added

  class EvtD0ToKpipi0pi0: public EvtDecayProb  {

  public:
    EvtD0ToKpipi0pi0() {}
    virtual ~EvtD0ToKpipi0pi0();
    //void getName(std::string& name); // Commented out
    std::string getName(); // Added
    EvtDecayBase* clone();
    void init();
    void initProbMax();
    void decay(EvtParticle* p);

  private:
    EvtComplex KPiSFormfactor(double sa, double sb, double sc, double r);
    double calPDF(double Km[], double Pip[], double Pi01[], double Pi02[]);
    EvtComplex D2VV(double P1[], double P2[], double P3[], double P4[], int g[], int flag);
    EvtComplex D2AP_A2VP(double P1[], double P2[], double P3[], double P4[], int g[], int flag);
    //flag = 1, V = K*; flag = 2, V = rho
    EvtComplex D2AP_A2SP(double P1[], double P2[], double P3[], double P4[], int flag);
    //flag = 1, S = K*; flag = 2, S = rho
    EvtComplex D2PP_P2VP(double P1[], double P2[], double P3[], double P4[], int flag);
    //flag = 1, V = K*; flag = 2, V = rho
    EvtComplex D2VP_V2VP(double P1[], double P2[], double P3[], double P4[], int flag);
    //flag = 1, (K*Pi)V; flag = 2, (rhoK)V
    EvtComplex D2VS(double P1[], double P2[], double P3[], double P4[], int g, int flag);
    //flag = 1, V = K*; flag = 2, V = rho
    EvtComplex D2TS(double P1[], double P2[], double P3[], double P4[], int flag);
    //flag = 1, T = K*; flag = 2, T = rho
    EvtComplex PHSP(double Km[], double Pip[]);

    double calDalEva(double P1[], double P2[], double P3[]);
    EvtComplex Spin_factor(double P1[], double P2[], double P3[], int spin);
    EvtComplex getProp(double s[], int flag);
    EvtComplex rhofactor(double sx, double sdau);

    double dot(double* a1, double* a2)const;
    double Qabcs(double sa, double sb, double sc)const;
    double barrier(double l, double sa, double sb, double sc, double r)const;
    void calt1(double daug1[], double daug2[], double t1[]) const;
    void calt2(double daug1[], double daug2[], double t2[][4]) const;

    EvtComplex propogator(double mass, double width, double sx) const;
    double wid(double mass, double sa, double sb, double sc, double r, int l) const;
    double h(double m, double q) const;
    double dh(double mass, double q0) const;
    double f(double mass, double sx, double q0, double q) const;
    double d(double mass, double q0) const;
    double Flatte_rhoab(double sa, double sb, double sc) const;
    double rhoab_L(double sa, double sb, double sc, int L) const;
    EvtComplex propagatorRBW(double mass, double width, double sa, double sb, double sc, double r, int l)const;
    EvtComplex propagatorGS(double mass, double width, double sa, double sb, double sc, double r, int l)const;
    EvtComplex propagatorFlatte(double mass, double width, double sx, double* sb, double* sc)const;
    EvtComplex propagatorK1270(double mass, double sx, double* sb, double* sc, double* g)const;
    EvtComplex propagator980(double mass, double sx, double* sb, double* sc, double* g)const;
    EvtComplex propagatorf600(double mass, double width, double sx)const;
    EvtComplex propagatorsigma500(double mass, double width, double sx, double* g)const;

    double G[4][4], E[4][4][4][4];
    double mass[5];
    double width[5];
    int    mod[68];
    double rho[68];
    double phi[68];

    double mD;
    double rRes;
    double rD;
    double metap;
    double mk0;
    double mass_Kaon;
    double mass_Pion;
    double math_pi;
    double mass_Pi0;
    double mkstrm;
    double mkstr0;

    double pi;
    double mpi;
    double g1;
    double g2;
  };

} // Belle 2 Namespace // Added

//#endif // Commented out

