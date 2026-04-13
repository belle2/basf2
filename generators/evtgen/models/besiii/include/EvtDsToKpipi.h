#ifndef EVTDSTOKPIPI_HH
#define EVTDSTOKPIPI_HH

#include "EvtGenBase/EvtDecayProb.hh"

namespace Belle2 {

  class EvtDsToKpipi: public  EvtDecayProb  {

  public:
    EvtDsToKpipi() {}
    virtual ~EvtDsToKpipi();

    std::string getName();
    EvtDecayBase* clone();

    void init();
    void initProbMax();
    void decay(EvtParticle* p);

  private:

    void Com_Multi(double a1[2], double a2[2], double res[2]);
    void Com_Divide(double a1[2], double a2[2], double res[2]);
    void calt1(double daug1[4], double daug2[4], double t1[4]);
    void calt2(double daug1[4], double daug2[4], double t2[4][4]);
    void propagatorCBW(double mass, double width, double sx, double prop[2]);
    void propagatorFlatte(double mass, double width, double sa, double prop[2]);
    void propagatorGS(double mass, double width, double sa, double sb, double sc, double r2, double prop[2]);
    void calEva(double* K, double* Pi1, double* Pi2, double* mass1, double* width1, double* amp, double* phase, int* g0, int* spin,
                int* modetype, int nstates, double& Result);
    double SCADot(double a1[4], double a2[4]);
    double barrier(int l, double sa, double sb, double sc, double r, double mass);
    double wid(double mass2, double mass, double sa, double sb, double sc, double r2, int l);
    double widl1(double mass2, double mass, double sa, double sb, double sc, double r2);
    double DDalitz(double P1[4], double P2[4], double P3[4], int Ang, double mass);
    void propagatorRBW(double mass, double width, double sa, double sb, double sc, double r2, int l, double prop[2]);
    void propagatorKstr1430(double mass, double sx, double* sb, double* sc, double prop[2]);
    void Flatte_rhoab(double sa, double sb, double sc, double rho[2]);
    void propagatorsigma500(double sa, double sb, double sc, double prop[2]);
    void rhoab(double sa, double sb, double sc, double res[2]);
    void rho4Pi(double sa, double res[2]);


    int modetype[8];
    double mass[8];
    double width[8];
    double rho[8];
    double phi[8];
    double spin[8];
    double GK1410;
    double GKst0;
    double GKstm;
    double Grho;
    double Grho1450;
    double mK1410;
    double mKst0;
    double mKstm;
    double mrho;
    double mPi2 ;
    double mKa2 ;
    double mPi ;
    double mKa ;
    double mDsM;
    double mrho1450;
    double mD;
    double mDs;
    double rRes;
    double rD;
    double metap;
    double mkstr;
    double mk0;
    double mass_Kaon;
    double mass_Pion;
    double mass_Pi0;
    double math_pi;
    double meta;
    double ma0;
    double Ga0;
    double GS1;
    double GS2;
    double GS3;
    double GS4;
    int G[4][4];
  };

} // Belle2 namespace

#endif
