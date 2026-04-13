// Model: EvtD0Topippim2pi0
// This file is an amplitude model for D0 -> pi+ pi- pi0 pi0.
// The model is from the BESIII Collaboration in Chin. Phys. C 48, 083001 (2024). DOI:&nbsp; https://doi.org/10.1088/1674-1137/ad3d4d
//
// Permission to include these files in basf2 was generously granted by the BESIII Collaboration.
//
// Please cite the original reference for any public/published results where this model was used.

#pragma once

#include "EvtGenBase/EvtDecayProb.hh"
#include <vector>
#include <complex>

#include "EvtGenBase/EvtParticle.hh"

namespace Belle2 {
  class EvtD0Topippim2pi0: public EvtDecayProb {

  public:

    EvtD0Topippim2pi0() {}
    virtual ~EvtD0Topippim2pi0();

    std::string getName();
    EvtDecayBase* clone();

    void init();
    void initProbMax();
    void decay(EvtParticle* p);

  private:

    double AmplitudeSquare(int charm, int tagmode);

    EvtVector4R GetDaugMomLab(int i) {return _p4Lab[i];}
    EvtVector4R GetDaugMomCM(int i) {return _p4CM[i];}
    EvtVector4R GetDaugMomHel(int i) {return _p4Hel[i];}

    EvtVector4R _p4Lab[10], _p4CM[10], _p4Hel[10];
    int _nd;

    int charm;
    int tagmode;

    void setInput(double* pip, double* pim, double* pi01, double* pi02);
    int CalAmp();

    std::complex<double> Get_AmpD0() {return m_AmpD0;}
    std::complex<double> Get_AmpDb() {return m_AmpDb;}

    double Get_strongPhase();
    std::complex<double> Amp(std::vector<double> Pip1, std::vector<double> Pim1, std::vector<double> Pip2, std::vector<double> Pim2);
    double arg(std::complex<double> x);
    double mag2(std::complex<double> x);

    std::complex<double> CalD0Amp();
    std::complex<double> CalDbAmp();

    std::vector<double> sum_tensor(std::vector<double> pa, std::vector<double> pb);
    double contract_11_0(std::vector<double> pa, std::vector<double> pb);
    double contract_22_0(std::vector<double> pa, std::vector<double> pb);
    std::vector<double> contract_21_1(std::vector<double> pa, std::vector<double> pb);
    std::vector<double> contract_22_2(std::vector<double> pa, std::vector<double> pb);
    std::vector<double> contract_31_2(std::vector<double> pa, std::vector<double> pb);
    std::vector<double> contract_41_3(std::vector<double> pa, std::vector<double> pb);
    std::vector<double> contract_42_2(std::vector<double> pa, std::vector<double> pb);

    std::vector<double> OrbitalTensors(std::vector<double> pa, std::vector<double> pb, std::vector<double> pc, double r, int rank);
    std::vector<double> ProjectionTensors(std::vector<double> pa, int rank);

    double fundecaymomentum(double mr2, double m1_2, double m2_2);
    double fundecaymomentum2(double mr2, double m1_2, double m2_2);
    std::complex<double>  breitwigner(double mx2, double mr, double wr);
    double h(double m, double q);
    double dh(double m0, double q0);
    double f(double m0, double sx, double q0, double q);
    double d(double m0, double q0);
    double wid(double mass, double sa, double sb, double sc, double r, int l);
    double widT1260(int i, double g1, double g2);
    double widT1300(int i);
    double widT1640(int i);
    double widT1170(int i);
    double anywid1260(double sc, double g1, double g2);
    double anywid1300(double sc);
    double anywid1640(double sc);
    double anywid1170(double sc);

    std::complex<double> irho(double mr2, double m1_2, double m2_2);
    std::complex<double> Flatte(double mx2, double mr, double g1, double g2, double m1a, double m1b, double m2a, double m2b);

    std::complex<double> GS(double mx2, double mr, double wr, double m1_2, double m2_2, double r, int l);
    std::complex<double> RBW(double mx2, double mr, double wr, double m1_2, double m2_2, double r, int l);
    std::complex<double> RBWa1260(double mx2, double mr, double g1, double g2);
    std::complex<double> RBWpi1300(double mx2, double mr, double wr);
    std::complex<double> RBWa1640(double mx2, double mr, double wr);
    std::complex<double> RBWh11170(double mx2, double mr, double wr);
    double rho22(double sc);
    std::complex<double> rhoMTX(int i, int j, double s);
    std::complex<double> KMTX(int i, int j, double s);
    std::complex<double> IMTX(int i, int j);
    std::complex<double> FMTX(double Kijx, double Kijy, double rhojjx, double rhojjy, int i, int j);
    double FINVMTX(double s, double* FINVx, double* FINVy);
    std::complex<double> PVTR(int ID, double s);
    std::complex<double> Fvector(double sa, double s0, int l);

    std::vector<double> m_Pip;
    std::vector<double> m_Pim;
    std::vector<double> m_Pi01;
    std::vector<double> m_Pi02;

    std::vector<double> g_uv;
    std::vector<double> epsilon_uvmn;
    std::vector< std::complex<double> > fitpara;

    double math_pi;
    double mass_Pion;

    double rRes;
    double rD;
    double m_Pi;
    double m2_Pi;
    double m_Pi0;
    double m2_Pi0;
    double m_Ka;
    double m2_Ka;

    double m0_f0980;
    double g1_f0980;
    double g2_f0980;

    double m0_rho7700;
    double w0_rho7700;

    double m0_rho770p;
    double w0_rho770p;

    double m0_rho1450;
    double w0_rho1450;

    double m0_f21270;
    double w0_f21270;

    double m0_a11260;
    double g1_a11260;
    double g2_a11260;

    double m0_pi1300;
    double w0_pi1300;

    double m0_a11420;
    double w0_a11420;

    double m0_a11640;
    double w0_a11640;

    double m0_a21320;
    double w0_a21320;

    double m0_pi11400;
    double w0_pi11400;

    double m0_h11170;
    double w0_h11170;

    double m0_pi21670;
    double w0_pi21670;

    double m0_omega;
    double w0_omega;

    double m0_phi;
    double w0_phi;

    double s0_prod;

    std::complex<double> m_AmpD0;
    std::complex<double> m_AmpDb;
    std::complex<double> m_RAmp;

    double delta;

  };

}
