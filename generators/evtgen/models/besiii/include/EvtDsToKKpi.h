// Model: EvtDsToKKpi
// This file is an amplitude model for Ds+ -> K- K+ pi+.
// The model is from the BESIII Collaboration in PRD 104, 012016 (2021). DOI: https://doi.org/10.1103/PhysRevD.104.012016
//
// Permission to use these files in basf2 was generously granted by the BESIII Collaboration.
//
// Please cite the original reference for any public/published results where this model was used.

#pragma once
#include <EvtGenBase/EvtDecayProb.hh>
#include <EvtGenBase/EvtDecayAmp.hh>
#include <EvtGenBase/EvtParticle.hh>
#include <fstream>

namespace Belle2 {
  class EvtDsToKKpi: public EvtDecayProb  {

  public:

    EvtDsToKKpi() {}
    virtual ~EvtDsToKKpi();
    std::string getName();
    EvtDecayBase* clone();
    void init();
    void initProbMax();
    void decay(EvtParticle* p);

  private:
    void MIP_LineShape(double sa, double pro[2]);
    void calEvaMy(double* pKm, double* pKp, double* pPi, double* mass1, double* width1, double* amp, double* phase, int* g0,
                  int* modetype, int nstates, double& Result);
    void Com_Multi(double a1[2], double a2[2], double res[2]);
    void Com_Divide(double a1[2], double a2[2], double res[2]);
    double SCADot(double a1[4], double a2[4]);
    double barrier(int l, double sa, double sb, double sc, double r);
    double barrierNeo(int l, double sa, double sb, double sc, double r, double mR);
    double barrierNeoDs(int l, double sa, double sb, double sc, double r, double mR, double mb);
    void calt1(double daug1[4], double daug2[4], double t1[4]);
    void calt2(double daug1[4], double daug2[4], double t2[4][4]);
    void propagator(double mass, double width, double sx, double prop[2]);
    double wid(double mass, double sa, double sb, double sc, double r, int l);
    void Flatte_rhoab(double sa, double sb, double sc, double rho[2]);
    void propagatorFlatte(double mass, double width, double sx, double* sb, double* sc, double prop[2]);
    void propagator980(double mass, double sx, double* sb, double* sc, double prop[2]);
    void propagatora0980(double mass, double sx, double* sb, double* sc, double prop[2]);
    void propagatorKstr1430(double mass, double sx, double* sb, double* sc, double prop[2]);
    void propagatorRBW(double mass, double width, double sa, double sb, double sc, double r, int l, double prop[2]);
    void propagatorRBWNeo(double mass, double width, double sa, double sb, double sc, double r, int l, double prop[2]);
    void propagatorRBWNeoKstr892(double mass, double width, double sa, double sb, double sc, double r, int l, double prop[2]);
    double h(double m, double q);
    double dh(double mass, double q0);
    double f(double mass, double sx, double q0, double q);
    double d(double mass, double q0);
    void propagatorGS(double mass, double width, double sa, double sb, double sc, double r, int l, double prop[2]);

    // Parameters
    double phi[6], rho[6], mass[6], width[6];
    int modetype[6];
    double mD, mDs, rRes, rD, mkstr, mk0, mass_Kaon, mass_Pion, mass_Pi0, mass_EtaP, mass_Eta, math_pi, afRatio;
    int G[4][4], E[4][4][4][4];

  };
} // Belle2 namespace
