// Model: EvtDToKSpipipi
// This file is an amplitude model for D+ -> K_S0 pi+ pi+ pi-.
// The model is from the BESIII Collaboration in PRD 100, 072008 (2019). DOI:&nbsp; https://doi.org/10.1103/PhysRevD.100.072008
//
// Permission to include these files in basf2 was generously granted by the BESIII Collaboration.
//
// Please cite the original reference for any public/published results where this model was used.

#pragma once

#include <EvtGenBase/EvtDecayProb.hh>

namespace Belle2 {

  class EvtDToKSpipipi: public EvtDecayProb  {

  public:
    EvtDToKSpipipi() {}
    virtual ~EvtDToKSpipipi();
    std::string getName();
    EvtDecayBase* clone();
    void init();
    void initProbMax();
    void decay(EvtParticle* p);

  private:
    /** K pi S-wave form factor  */
    EvtComplex KPiSFormfactor(const double sa, const double sb, const double sc, const double r);
    /** Probability distribution function of the decay */
    double calPDF(double Km[], double Pip1[], double Pip2[], double Pim[]);
    /** Amplitude modes */
    EvtComplex D2AP_A2VP(double P1[], double P2[], double P3[], double P4[], int L);
    EvtComplex D2AP_A2SP(double P1[], double P2[], double P3[], double P4[]);
    EvtComplex D2PP_P2VP(double P1[], double P2[], double P3[], double P4[]);
    EvtComplex PHSP(double Km[], double Pip[]);

    /** Propagator Lineshapes */
    EvtComplex getprop(double daug1[], double daug2[], double mass, double width, int flag, int L);
    /** Two-body Phase-space Function */
    EvtComplex rhoab(const double sa, const double sb, const double sc);
    /** Two-body Phase-space Function (Two Pions) */
    EvtComplex rho4Pi(const double sa);

    /** Four-Vector Scalar Product */
    double dot(double* a1, double* a2)const;
    /** Magnitudes of daughter particle momenta in the rest system of the mother particle  */
    double Qabcs(const double sa, const double sb, const double sc)const;
    /** Blatt-Weisskopf barrier factors  */
    double barrier(const double l, const double sa, const double sb, const double sc, const double r)const;
    /** Covariant Spin-1 Projector */
    void calt1(double daug1[], double daug2[], double t1[]) const;
    /** Covariant Spin-2 Projector */
    void calt2(double daug1[], double daug2[], double t2[][4]) const;

    /** Relativistic Breit-Wigner Lineshape Function (Fixed Width) */
    EvtComplex propogator(const double mass, const double width, const double sx) const;
    /** Energy Dependent Width */
    double wid(const double mass, const double sa, const double sb, const double sc, const double r, const int l) const;
    /** h function in Gounaris-Sakurai lineshape  */
    double h(const double m, const double q) const;
    /** derivative h function in Gounaris-Sakurai lineshape  */
    double dh(const double mass, const double q0) const;
    /** f function in Gounaris-Sakurai lineshape  */
    double f(const double mass, const double sx, const double q0, const double q) const;
    /** d function in Gounaris-Sakurai lineshape  */
    double d(const double mass, const double q0) const;
    /** Relativistic Breit-Wigner Lineshape Function*/
    EvtComplex propagatorRBW(const double mass, const double width, const double sa, const double sb, const double sc, const double r,
                             const int l)const;
    /** Gounaris-Sakurai lineshape Function */
    EvtComplex propagatorGS(const double mass, const double width, const double sa, const double sb, const double sc, const double r,
                            const int l)const;

    /** Number of Spacetime Dimensions (t, x, y, z) */
    static constexpr int STDim = 4;
    double G[STDim][STDim], E[STDim][STDim][STDim][STDim];

    /** Number of fit parameters */
    static constexpr int numOfParameters = 24;
    double rho[numOfParameters];
    double phi[numOfParameters];
    double mrho, Grho, mKstr, GKstr, msigma, Gsigma;
    double ma1, Ga1, mK1270, GK1270, mK1400, GK1400, mK1460, GK1460, mK1650, GK1650;
    double rho_omega, phi_omega;

    /** Fixed parameters */
    double mD;
    double rD;
    double metap;
    double mkstr;
    double mk0;
    double mass_Kaon;
    double mass_Pion;
    double math_pi;

    double pi;
    double mpi;
    double g1;
    double g2;

  };

} // Belle 2 Namespace
