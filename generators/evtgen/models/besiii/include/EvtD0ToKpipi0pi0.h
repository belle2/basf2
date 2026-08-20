// Model: EvtD0ToKpipi0pi0
// This file is an amplitude model for D0 -> K- pi+ pi0 pi0.
// The model is from the BESIII Collaboration in PRD 99, 092008 (2019). DOI:&nbsp; https://doi.org/10.1103/PhysRevD.99.092008
//
// Permission to include these files in basf2 was generously granted by the BESIII Collaboration.
//
// Please cite the original reference for any public/published results where this model was used.

#pragma once

#include <EvtGenBase/EvtDecayProb.hh>

namespace Belle2 {

  class EvtD0ToKpipi0pi0: public EvtDecayProb  {

  public:
    EvtD0ToKpipi0pi0() {}
    virtual ~EvtD0ToKpipi0pi0();
    std::string getName();
    EvtDecayBase* clone();
    void init();
    void initProbMax();
    void decay(EvtParticle* p);

  private:
    /** K pi S-wave form factor  */
    EvtComplex KPiSFormfactor(const double sa, const double sb, const double sc, const double r);
    /** Probability distribution function of the decay */
    double PDF(double* Km, double* Pip, double* Pi01, double* Pi02);
    /** Amplitude modes */
    EvtComplex D2VV(const double* P1, const double* P2, const double* P3, const double* P4, int* g, const int flag);
    EvtComplex D2AP_A2VP(const double* P1, const double* P2, const double* P3, const double* P4, int* g, const int flag);
    EvtComplex D2AP_A2SP(const double* P1, const double* P2, const double* P3, const double* P4, const int flag);
    EvtComplex D2PP_P2VP(const double* P1, const double* P2, const double* P3, const double* P4, const int flag);
    EvtComplex D2VP_V2VP(const double* P1, const double* P2, const double* P3, const double* P4, const int flag);
    EvtComplex D2VS(const double* P1, const double* P2, const double* P3, const double* P4, int g, const int flag);
    EvtComplex D2TS(const double* P1, const double* P2, const double* P3, const double* P4, const int flag);
    EvtComplex PHSP(double* Km, double* Pip);

    /** Four-vector dot product */
    double LorentzDotProduct(const double* a1, const double* a2)const;
    /** Magnitudes of daughter particle momenta in the rest system of the mother particle  */
    double Qabcs(const double sa, const double sb, const double sc)const;
    /** Blatt-Weisskopf barrier factors  */
    double BWBarrierFactor(const double l, const double sa, const double sb, const double sc, const double r)const;
    /** Covariant Spin-1 Projector */
    void covariantTensor1(const double* daug1, const double* daug2, double* t1) const;
    /** Covariant Spin-2 Projector */
    void covariantTensor2(const double* daug1, const double* daug2, double (*t2)[4]) const;

    /** Relativistic Breit-Wigner lineshape function  */
    EvtComplex propagatorRBW(double mass, double width, const double sa, const double sb, const double sc, const double r,
                             const int l)const;
    /** Gounaris-Sakurai lineshape function  */
    EvtComplex propagatorGS(double mass, double width, const double sa, const double sb, const double sc, const double r,
                            const int l)const;

    /** Energy dependent width  */
    double energyDependentWidth(const double mass, const double sa, const double sb, const double sc, const double r,
                                const int l) const;
    /** h function in Gounaris-Sakurai lineshape  */
    double h(const double m, const double q) const;
    /** derivative h function in Gounaris-Sakurai lineshape  */
    double dh(double mass, const double q0) const;
    /** f function in Gounaris-Sakurai lineshape  */
    double f(double mass, const double sx, const double q0, const double q) const;
    /** d function in Gounaris-Sakurai lineshape  */
    double d(double mass, const double q0) const;

    /** Number of spacetime dimensions (t, x, y, z) */
    static constexpr int STDim = 4;
    double G[STDim][STDim], E[STDim][STDim][STDim][STDim];

    /** Number of resonances */
    static constexpr int numberOfResonances = 5;
    double mass[numberOfResonances];
    double width[numberOfResonances];

    /** Number of fit parameters */
    static constexpr int numOfParameters = 68;
    int    mod[numOfParameters];
    double rho[numOfParameters];
    double phi[numOfParameters];

    /** Fixed parameters */
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

} // Belle 2 Namespace

