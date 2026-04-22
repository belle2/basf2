#include <EvtGenBase/EvtPatches.hh>
#include <EvtGenBase/EvtParticle.hh>
#include <EvtGenBase/EvtGenKine.hh>
#include <EvtGenBase/EvtPDL.hh>
#include <EvtGenBase/EvtReport.hh>
#include <EvtGenBase/EvtVector4R.hh>
#include <EvtGenBase/EvtComplex.hh>
#include <EvtGenBase/EvtDecayTable.hh>
#include <stdlib.h>

#include <generators/evtgen/EvtGenModelRegister.h>
#include <generators/evtgen/models/besiii/EvtD0TopipiEta.h>

namespace Belle2 {

  using namespace std;

  B2_EVTGEN_REGISTER_MODEL(EvtD0TopipiEta);

  EvtD0TopipiEta::~EvtD0TopipiEta() {}

  std::string EvtD0TopipiEta::getName()
  {
    return "D0TopipiEta";
  }

  EvtDecayBase* EvtD0TopipiEta::clone()
  {
    return new EvtD0TopipiEta;
  }

  void EvtD0TopipiEta::init()
  {
    // check that there are 0 arguments
    checkNArg(0);
    checkNDaug(3);
    checkSpinParent(EvtSpinType::SCALAR);
    checkSpinDaughter(0, EvtSpinType::SCALAR);
    checkSpinDaughter(1, EvtSpinType::SCALAR);
    checkSpinDaughter(2, EvtSpinType::SCALAR);

    phi[0] =  0;         rho[0] =  1;        //rho eta
    phi[1] = -0.98109;   rho[1] = -0.02447;  //omega eta (rho-omega mixing)
    phi[2] =  0.71358;   rho[2] =  1.0848;   //a0- pi+
    phi[3] = -0.83115;   rho[3] =  2.6444;   //a0+ pi-
    phi[4] = -0.058521;  rho[4] =  7.0274;   //(pi+ eta)_{2+} pi-

    mrho = 0.77511;
    ma0 = 0.99;
    Grho = 0.1491;
    Ga0 = 0.0756;

    const double mk0 = 0.497614;
    const double mass_Kaon = 0.49368;
    const double mass_Pion = 0.13957;
    const double mass_Pi0 = 0.1349766;
    const double meta = 0.547862;
    mpi = 0.13957;
    mD = 1.86483;
    sD = mD * mD;
    spi = mpi * mpi;
    snk = mk0 * mk0;
    sck = mass_Kaon * mass_Kaon;
    scpi = mass_Pion * mass_Pion;
    snpi = mass_Pi0 * mass_Pi0;
    seta = meta * meta;

    pi = 3.1415926;

    ci  = EvtComplex(0.0, 1.0);
    one = EvtComplex(1.0, 0.0);

    int GG[4][4] = { {1, 0, 0, 0}, {0, -1, 0, 0}, {0, 0, -1, 0}, {0, 0, 0, -1} };
    for (int i = 0; i < 4; i++) {
      for (int j = 0; j < 4; j++) {
        G[i][j] = GG[i][j];
      }
    }


  }

  void EvtD0TopipiEta::initProbMax()
  {
    setProbMax(476.5);
  }

  void EvtD0TopipiEta::decay(EvtParticle* p)
  {
    /*
       // This piece of code could in principle be used to calculate maximum
       // probablity on fly. But as it uses high number of points and model
       // deals with single final state, we keep hardcoded number for now rather
       // than adapting code to work here.

       double maxprob = 0.0;
       for(int ir=0;ir<=60000000;ir++){
          p->initializePhaseSpace(getNDaug(),getDaugs());
          EvtVector4R D1 = p->getDaug(0)->getP4();
          EvtVector4R D2 = p->getDaug(1)->getP4();
          EvtVector4R D3 = p->getDaug(2)->getP4();

          double P1[4], P2[4], P3[4];
          P1[0] = D1.get(0); P1[1] = D1.get(1); P1[2] = D1.get(2); P1[3] = D1.get(3);
          P2[0] = D2.get(0); P2[1] = D2.get(1); P2[2] = D2.get(2); P2[3] = D2.get(3);
          P3[0] = D3.get(0); P3[1] = D3.get(1); P3[2] = D3.get(2); P3[3] = D3.get(3);

          double value;
          value = calDalEva(P1, P2, P3);
          if(value>maxprob) {
             maxprob=value;
          }
       }
    */
    p->initializePhaseSpace(getNDaug(), getDaugs());
    EvtVector4R D1 = p->getDaug(0)->getP4();
    EvtVector4R D2 = p->getDaug(1)->getP4();
    EvtVector4R D3 = p->getDaug(2)->getP4();

    double P1[4], P2[4], P3[4];
    P1[0] = D1.get(0); P1[1] = D1.get(1); P1[2] = D1.get(2); P1[3] = D1.get(3);
    P2[0] = D2.get(0); P2[1] = D2.get(1); P2[2] = D2.get(2); P2[3] = D2.get(3);
    P3[0] = D3.get(0); P3[1] = D3.get(1); P3[2] = D3.get(2); P3[3] = D3.get(3);

    double value;
    value = calDalEva(P1, P2, P3);
    setProb(value);

    return ;
  }

  double EvtD0TopipiEta::calDalEva(double P1[], double P2[], double P3[])
  {
    //pi- pi+ eta
    //0: non-resonance
    //1: resonance, RBW
    //2: resonance, GS
    //3: resonance, Flatte
    //4: rho-omega mxing for omega
    EvtComplex PDF[6];
    EvtComplex cof, pdf, module;
    double value;
    PDF[0] = Spin_factor(P1, P2, P3, 1, 2, mrho, Grho); // rho eta
    PDF[1] = Spin_factor(P1, P2, P3, 1, 4, mrho, Grho); // rho-omega mixing
    PDF[2] = Spin_factor(P1, P3, P2, 0, 3, ma0,  Ga0);  // a0- pi+
    PDF[3] = Spin_factor(P2, P3, P1, 0, 3, ma0,  Ga0);  // a0+ pi-
    PDF[4] = Spin_factor(P2, P3, P1, 2, 0, 1.698,  0.265);  // pi+ eta 2+ non-res

    pdf = EvtComplex(0.0, 0.0);
    for (int i = 0; i < 5; i++) {
      cof = EvtComplex(rho[i] * cos(phi[i]), rho[i] * sin(phi[i]));
      pdf = pdf + cof * PDF[i];
    }
    module = conj(pdf) * pdf;
    value = real(module);
    return (value <= 0) ? 1e-20 : value;
  }

  EvtComplex EvtD0TopipiEta::Spin_factor(double P1[], double P2[], double P3[], int spin, int flag, double mass_R, double width_R)
  {
    //D-> R P3, R->P1 P2, 0: non-resonance 1: resonance, RBW 2: resonance, GS 3: resonance, Flatte 4: rho-omega mxing for omega
    double R[4], s[3], sp2, B[2];
    double tmp;
    for (int i = 0; i < 4; i++) {
      R[i] = P1[i] + P2[i];
    }
    s[0] = dot(R, R);
    s[1] = dot(P1, P1);
    s[2] = dot(P2, P2);
    sp2 = dot(P3, P3);

    EvtComplex amp, prop, prop1, prop2;

    //-----------for prop-------------------------
    EvtComplex rhokk, rhopieta;
    if (spin == 0) {
      if (flag == 0) prop = one;
      if (flag == 1) prop = propagatorRBW(mass_R, width_R, s[0], s[1], s[2], 3.0, 0);
      if (flag == 3) {
        rhokk = Flatte_rhoab(s[0], snk, sck);
        rhopieta = Flatte_rhoab(s[0], scpi, seta);
        prop = 1.0 / (mass_R * mass_R - s[0] - ci * (0.341 * rhopieta + 0.341 * 0.892 * rhokk));
      }
      amp = prop;
    } else if (spin == 1) {
      if (flag == 0) {
        prop = EvtComplex(1.0, 0.0);
      }
      if (flag == 1) {
        prop = propagatorRBW(mass_R, width_R, s[0], s[1], s[2], 3.0, 1);
      }
      if (flag == 2) {
        prop = propagatorGS(mass_R, width_R, s[0], s[1], s[2], 3.0, 1);
      }
      if (flag == 4) {
        prop1 = propagatorGS(mass_R, width_R, s[0], s[1], s[2], 3.0, 1);
        prop2 = propagatorRBW(0.78266, 0.01358, s[0], s[1], s[2], 3.0, 1);
        prop = prop1 * prop2;
      }
      double T1[4], t1[4];
      calt1(R, P3, T1);
      calt1(P1, P2, t1);
      B[0] = barrier(1, s[0], s[1], s[2], 3.0, mass_R);
      B[1] = barrier(1, sD,  s[0], sp2, 5.0, mD);
      tmp = 0.0;
      for (int i = 0; i < 4; i++) {
        tmp += T1[i] * t1[i] * G[i][i];
      }
      amp = tmp * prop * B[0] * B[1];
    } else if (spin == 2) {
      double T2[4][4], t2[4][4];
      calt2(R, P3, T2);
      calt2(P1, P2, t2);
      B[0] = barrier(2, s[0], s[1], s[2], 3.0, mass_R);
      B[1] = barrier(2, sD,  s[0], sp2, 5.0, mD);
      tmp = 0.0;
      for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
          tmp += T2[i][j] * t2[j][i] * G[j][j] * G[i][i];
        }
      }
      if (flag == 0) prop = one;
      if (flag == 1) prop = propagatorRBW(mass_R, width_R, s[0], s[1], s[2], 3.0, 2);
      amp = tmp * prop * B[0] * B[1];
    }
    return amp;
  }

  double EvtD0TopipiEta::dot(double* a1, double* a2)
  {
    double Dot = 0;
    for (int i = 0; i != 4; i++) {
      Dot += a1[i] * a2[i] * G[i][i];
    }
    return Dot;
  }

  double EvtD0TopipiEta::Qabcs(double sa, double sb, double sc)
  {
    double Qabcs = (sa + sb - sc) * (sa + sb - sc) / (4 * sa) - sb;
    if (Qabcs < 0) Qabcs = 1e-16;
    return Qabcs;
  }

  double EvtD0TopipiEta::barrier(double l, double sa, double sb, double sc, double r, double mass)
  {
    double sa0 = mass * mass;
    double q0 = Qabcs(sa0, sb, sc);
    double z0 = q0 * r * r;
    double q = Qabcs(sa, sb, sc);
    q = sqrt(q);
    double z = q * r;
    z = z * z;
    double F = 1;
    if (l > 2) F = 0;
    if (l == 0) F = 1;
    if (l == 1) F = sqrt((1 + z0) / (1 + z));
    if (l == 2) F = sqrt((9 + 3 * z0 + z0 * z0) / (9 + 3 * z + z * z));
    return F;
  }

  void EvtD0TopipiEta::calt1(double daug1[], double daug2[], double t1[])
  {
    double p, pq;
    double pa[4], qa[4];
    for (int i = 0; i != 4; i++) {
      pa[i] = daug1[i] + daug2[i];
      qa[i] = daug1[i] - daug2[i];
    }
    p = dot(pa, pa);
    pq = dot(pa, qa);
    for (int i = 0; i != 4; i++) {
      t1[i] = qa[i] - pq / p * pa[i];
    }
  }

  void EvtD0TopipiEta::calt2(double daug1[], double daug2[], double t2[][4])
  {
    double p, r;
    double pa[4], t1[4];
    calt1(daug1, daug2, t1);
    r = dot(t1, t1);
    for (int i = 0; i != 4; i++) {
      pa[i] = daug1[i] + daug2[i];
    }
    p = dot(pa, pa);
    for (int i = 0; i != 4; i++) {
      for (int j = 0; j != 4; j++) {
        t2[i][j] = t1[i] * t1[j] - 1.0 / 3 * r * (G[i][j] - pa[i] * pa[j] / p);
      }
    }
  }

  double EvtD0TopipiEta::wid(double mass, double sa, double sb, double sc, double r, int l)
  {
    double widm(0.), q(0.), q0(0.);
    double sa0 = mass * mass;
    double m = sqrt(sa);
    q = Qabcs(sa, sb, sc);
    q0 = Qabcs(sa0, sb, sc);
    double z, z0;
    z = q * r * r;
    z0 = q0 * r * r;
    double t = q / q0;
    double F(0.);
    if (l == 0) F = 1;
    if (l == 1) F = sqrt((1 + z0) / (1 + z));
    if (l == 2) F = sqrt((9 + 3 * z0 + z0 * z0) / (9 + 3 * z + z * z));
    widm = pow(t, l + 0.5) * mass / m * F * F;
    return widm;
  }

  EvtComplex EvtD0TopipiEta::propagatorRBW(double mass, double width, double sa, double sb, double sc, double r, int l)
  {
    EvtComplex prop = 1.0 / (mass * mass - sa - ci * mass * width * wid(mass, sa, sb, sc, r, l));
    return prop;
  }

  double EvtD0TopipiEta::h(double m, double q)
  {
    double h(0.);
    h = 2 / pi * q / m * log((m + 2 * q) / (2 * mpi));
    return h;
  }

  double EvtD0TopipiEta::dh(double mass, double q0)
  {
    double dh = h(mass, q0) * (1.0 / (8 * q0 * q0) - 1.0 / (2 * mass * mass)) + 1.0 / (2 * pi * mass * mass);
    return dh;
  }

  double EvtD0TopipiEta::f(double mass, double sx, double q0, double q)
  {
    double m = sqrt(sx);
    double f = mass * mass / (pow(q0, 3)) * (q * q * (h(m, q) - h(mass, q0)) + (mass * mass - sx) * q0 * q0 * dh(mass, q0));
    return f;
  }

  double EvtD0TopipiEta::d(double mass, double q0)
  {
    double d = 3.0 / pi * spi / (q0 * q0) * log((mass + 2 * q0) / (2 * mpi)) + mass / (2 * pi * q0) - (spi * mass) / (pi * pow(q0, 3));
    return d;
  }

  EvtComplex EvtD0TopipiEta::propagatorGS(double mass, double width, double sa, double sb, double sc, double r, int l)
  {
    double q = Qabcs(sa, sb, sc);
    double sa0 = mass * mass;
    double q0 = Qabcs(sa0, sb, sc);
    q = sqrt(q);
    q0 = sqrt(q0);
    EvtComplex prop = (1 + d(mass, q0) * width / mass) / (mass * mass - sa + width * f(mass, sa, q0, q) - ci * mass * width * wid(mass,
                                                          sa, sb, sc, r, l));
    return prop;
  }

  EvtComplex EvtD0TopipiEta::Flatte_rhoab(double sa, double sb, double sc)
  {
    double q = (sa + sb - sc) * (sa + sb - sc) / (4 * sa) - sb;
    EvtComplex rhoo;
    if (q > 0) {
      rhoo = 2.0 * sqrt(q / sa) * one;
    }
    if (q < 0) {
      rhoo = 2.0 * sqrt(-q / sa) * ci;
    }
    return rhoo;
  }

  EvtComplex EvtD0TopipiEta::propagatorFlatte(double mass, double width, double sx, double* sb, double* sc)
  {
    (void)width;
    const double g1sq = 0.5468 * 0.5468;
    const double g2sq = 0.23 * 0.23;
    EvtComplex rho1 = Flatte_rhoab(sx, sb[0], sc[0]);
    EvtComplex rho2 = Flatte_rhoab(sx, sb[1], sc[1]);
    EvtComplex prop = 1.0 / (mass * mass - sx - ci * (g1sq * rho1 + g2sq * rho2));
    return prop;
  }
} // Belle 2 Namespace
