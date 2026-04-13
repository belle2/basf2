// Model: EvtD0ToKpipipi
// This file is an amplitude model for D0 -> K- pi+ pi+ pi-.
// The model is from the BESIII Collaboration in PRD 95, 072010 (2017). DOI:&nbsp; https://doi.org/10.1103/PhysRevD.95.072010
//
// Permission to include these files in basf2 was generously granted by the BESIII Collaboration.
//
// Please cite the original reference for any public/published results where this model was used.

#pragma once

#include "EvtGenBase/EvtDecayProb.hh"

namespace Belle2 {

  class EvtD0ToKpipipi: public EvtDecayProb  {

  public:

    EvtD0ToKpipipi() {}
    virtual ~EvtD0ToKpipipi();
    std::string getName();
    EvtDecayBase* clone();
    void init();
    void initProbMax();
    void decay(EvtParticle* p);

  private:
    /** K pi S-wave form factor  */
    EvtComplex KPiSFormfactor(const double sa, const double sb, const double sc, const double r);
    /** Probability distribution function of the decay */
    double calPDF(double* Km, double* Pip1, double* Pip2, double* Pim);
    /** Amplitude modes */
    EvtComplex D2VV(const double* P1, const double* P2, const double* P3, const double* P4, int* g);
    EvtComplex D2AP_A2VP(const double* P1, const double* P2, const double* P3, const double* P4, int* g, const int flag);
    EvtComplex D2AP_A2SP(const double* P1, const double* P2, const double* P3, const double* P4, const int flag);
    EvtComplex D2PP_P2VP(const double* P1, const double* P2, const double* P3, const double* P4, const int flag);
    EvtComplex D2VP_V2VP(const double* P1, const double* P2, const double* P3, const double* P4, const int flag);
    EvtComplex D2VS(const double* P1, const double* P2, const double* P3, const double* P4, int g, const int flag);
    EvtComplex D2TS(const double* P1, const double* P2, const double* P3, const double* P4, const int flag);
    EvtComplex PHSP(double* Km, double* Pip);

    /** Four-Vector Scalar Product */
    double dot(const double* a1, const double* a2)const;
    /** Magnitudes of daughter particle momenta in the rest system of the mother particle  */
    double Qabcs(const double sa, const double sb, const double sc)const;
    /** Blatt-Weisskopf barrier factors */
    double barrier(const double l, const double sa, const double sb, const double sc, const double r)const;
    /** Covariant Spin-1 Projector */
    void calt1(const double* daug1, const double* daug2, double* t1) const;
    /** Covariant Spin-2 Projector */
    void calt2(const double* daug1, const double* daug2, double (*t2)[4]) const;

    /** Relativistic Breit-Wigner Lineshape Function*/
    EvtComplex propagatorRBW(double mass, double width, const double sa, const double sb, const double sc, const double r,
                             const int l)const;
    /** Gounaris-Sakurai lineshape Function */
    EvtComplex propagatorGS(double mass, double width, const double sa, const double sb, const double sc, const double r,
                            const int l)const;
    /** Relativistic Breit-Wigner Lineshape Function (Fixed Width) */
    EvtComplex propogator(double mass, double width, const double sx) const;

    /** Energy dependent width  */
    double wid(double mass, const double sa, const double sb, const double sc, const double r, const int l) const;
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
    static constexpr int numberOfResonances = 4;
    double mass[numberOfResonances];
    double width[numberOfResonances];

    /** Number of fit parameters */
    static constexpr int numOfParameters = 24;
    double rho[numOfParameters];
    double phi[numOfParameters];

    /** Fixed parameters */
    double mD;
    double rRes;
    double rD;
    double metap;
    double mkstr;
    double mk0;
    double mass_Kaon;
    double mass_Pion;
    double mass_Pi0;
    double math_pi;

    double pi;
    double mpi;
    double g1;
    double g2;
  };

} // Belle 2 Namespace
