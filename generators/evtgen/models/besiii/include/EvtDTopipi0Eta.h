#ifndef EVTDTOPIPI0ETA_HH
#define EVTDTOPIPI0ETA_HH

#include "EvtGenBase/EvtDecayProb.hh"

namespace Belle2 {

  class EvtDTopipi0Eta: public EvtDecayProb {

  public:
    EvtDTopipi0Eta() {}
    virtual ~EvtDTopipi0Eta();

    std::string getName();
    EvtDecayBase* clone();

    void init();
    void initProbMax();
    void decay(EvtParticle* p);

  private:
    double calDalEva(double P1[], double P2[], double P3[]);
    EvtComplex Spin_factor(double P1[], double P2[], double P3[], int spin, int flag, double mass_R, double width_R);

    double dot(double* a1, double* a2);
    double Qabcs(double sa, double sb, double sc);
    double barrier(double l, double sa, double sb, double sc, double r, double mass);
    void calt1(double daug1[], double daug2[], double t1[]);
    void calt2(double daug1[], double daug2[], double t2[][4]);

    double wid(double mass, double sa, double sb, double sc, double r, int l);
    EvtComplex propagatorRBW(double mass, double width, double sa, double sb, double sc, double r, int l);

    double h(double m, double q);
    double dh(double mass, double q0);
    double f(double mass, double sx, double q0, double q) ;
    double d(double mass, double q0);
    EvtComplex propagatorGS(double mass, double width, double sa, double sb, double sc, double r, int l);

    EvtComplex Flatte_rhoab(double sa, double sb, double sc);
    EvtComplex propagatorFlatte(double mass, double width, double sx, double* sb, double* sc);

    double rho[24];
    double phi[24];
    double mrho, ma0;
    double Grho, Ga0;
    double mpi, mD, sD, spi, snk, sck, scpi, snpi, seta;
    double pi;
    EvtComplex ci;
    EvtComplex one;
    int G[4][4];
  };

} // Belle2 namespace

#endif
