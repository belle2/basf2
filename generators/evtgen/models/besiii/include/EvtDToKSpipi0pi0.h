// Model: EvtDToKSpipi0pi0
// This file is an amplitude model for D+ -> K_S0 pi- pi0 pi0.
// The model is from the BESIII Collaboration in JHEP09 (2023) 077. DOI:&nbsp; https://doi.org/10.1007/JHEP09(2023)077
//
// Permission to include these files in basf2 was generously granted by the BESIII Collaboration.
//
// Please cite the original reference for any public/published results where this model was used.

#pragma once

#include "EvtGenBase/EvtDecayProb.hh"

namespace Belle2 {

  class EvtDToKSpipi0pi0: public EvtDecayProb  {

  public:
    EvtDToKSpipi0pi0() {}
    virtual ~EvtDToKSpipi0pi0();
    std::string getName();
    EvtDecayBase* clone();
    void init();
    void initProbMax();
    void decay(EvtParticle* p);

  private:
    /** Probability distribution function of the decay */
    double calPDF(const double Ks[], const double Pip[], const double Pi01[], const double Pi02[], double& Result);

    /** Complex Multiplication */
    void Com_Multi(const double a1[2], const double a2[2], double res[2]);
    /** Complex Division */
    void Com_Divide(const double a1[2], const double a2[2], double res[2]);
    /** Four-Vector Scalar Product */
    double SCADot(const double a1[4], const double a2[4]);

    /** Blatt-Weisskopf barrier factors */
    double Barrier(const double mass2, const int l, const double sa, const double sb, const double sc, const double r2);
    /** Covariant Spin-1 Projector */
    void calt1(const double daug1[4], const double daug2[4], double t1[4]);
    /** Covariant Spin-2 Projector */
    void calt2(const double daug1[4], const double daug2[4], double t2[4][4]);
    /** Energy Dependent Width */
    double wid(const double mass2, const double mass, const double sa, const double sb, const double sc, const double r2, const int l);
    /** Energy Dependent Width (L=1) */
    double widl1(const double mass2, const double mass, const double sa, const double sb, const double sc, const double r2);

    /** Relativistic Breit-Wigner Lineshape Function*/
    void propagatorRBW(const double mass2, const double mass, const double width, const double sa, const double sb, const double sc,
                       const double r2, const int l, double prop[2]);
    /** Relativistic Breit-Wigner Lineshape Function (L=1) */
    void propagatorRBWl1(const double mass2, const double mass, const double width, const double sa, const double sb, const double sc,
                         const double r2, double prop[2]);
    /** Relativistic Breit-Wigner Lineshape Function (Specific to a_1) */
    void propagatorRBW_a1(const double mass2, const double mass, const double width, const double sa, const double sb, const double sc,
                          const double r2, const int l, double prop[2]);
    /** Gounaris-Sakurai lineshape Function */
    void propagatorGS(const double mass2, const double mass, const double width, const double sa, const double sb, const double sc,
                      const double r2, double prop[2]);
    /** sigma(500) Lineshape Function */
    void propagatorsigma500(const double sa, const double sb, const double sc, double prop[2]);

    /** Two-body Phase-space Function */
    void rhoab(const double sa, const double sb, const double sc, double res[2]);
    /** Two-body Phase-space Function (Two Pions) */
    void rho4Pi(const double sa, double res[2]);
    /** LASS Parameterization Kpi S-Wave Function */
    void KPiSLASS(const double sa, const double sb, const double sc, double prop[2]);

    /** Mother PDG code */
    int mother_c;

    /** Number of Spacetime Dimensions (t, x, y, z) */
    static constexpr int STDim = 4;
    double G[STDim][STDim], E[STDim][STDim][STDim][STDim];

    /** Number of Fit Parameters */
    static constexpr int numOfParameters = 24;
    double rho[numOfParameters];
    double phi[numOfParameters];

    /** Number of Decay Topologies */
    static constexpr int topologies = 20;
    double modetype[topologies];

    /** Resonance Mass and Width */
    double mrho, Grho, mKstr0, GKstr0, msigma, Gsigma;
    double ma1, Ga1, mK1270, GK1270, mK1400, GK1400;

    /** Fixed parameters */
    double mD;
    double math_pi;
  };

} // Belle 2 Namespace
