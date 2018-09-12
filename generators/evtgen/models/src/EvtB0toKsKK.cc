#include <fstream>
#include <iomanip>
#include <cmath>
#include <string>

#include "EvtGenBase/EvtCPUtil.hh"
#include "EvtGenBase/EvtPDL.hh"
#include "EvtGenBase/EvtTensor4C.hh"

#include <generators/evtgen/EvtGenModelRegister.h>

#include "generators/evtgen/models/EvtB0toKsKK.h"

namespace Belle2 {

  B2_EVTGEN_REGISTER_MODEL(EvtB0toKsKK);

  EvtB0toKsKK::~EvtB0toKsKK() {}

  std::string EvtB0toKsKK::getName()
  {
    return "B0toKsKK";
  }

  EvtDecayBase* EvtB0toKsKK::clone()
  {
    return new EvtB0toKsKK;
  }

  void EvtB0toKsKK::init()
  {
    // Check number of arguments
    checkNArg(32);

    // Check number of daughters
    checkNDaug(3);

    // Check decay chain
    if ((getParentId() != EvtPDL::getId("B0") &&
         getParentId() != EvtPDL::getId("anti-B0")) ||
        (getDaug(0) != EvtPDL::getId("K_S0")) ||
        (getDaug(1) != EvtPDL::getId("K+") &&
         getDaug(1) != EvtPDL::getId("K-")) ||
        (getDaug(2) != EvtPDL::getId("K+") &&
         getDaug(2) != EvtPDL::getId("K-"))) {
      std::cout << "ERROR: Invalid decay" << std::endl;
      std::cout << "USAGE: K_S0 K+ K-" << std::endl;
      exit(1);
    }

    a_f0ks_ =
      (1.0 + getArg(3)) * EvtComplex(getArg(1) * cos(getArg(2) * M_PI / 180.0),
                                     getArg(1) * sin(getArg(2) * M_PI / 180.0));
    a_phiks_ =
      (1.0 + getArg(7)) * EvtComplex(getArg(5) * cos(getArg(6) * M_PI / 180.0),
                                     getArg(5) * sin(getArg(6) * M_PI / 180.0));
    a_fxks_ =
      (1.0 + getArg(11)) * EvtComplex(getArg(9) * cos(getArg(10) * M_PI / 180.0),
                                      getArg(9) * sin(getArg(10) * M_PI / 180.0));
    a_chic0ks_ =
      (1.0 + getArg(15)) * EvtComplex(getArg(13) * cos(getArg(14) * M_PI / 180.0),
                                      getArg(13) * sin(getArg(14) * M_PI / 180.0));
    a_kpkmnr_ =
      (1.0 + getArg(19)) * EvtComplex(getArg(17) * cos(getArg(18) * M_PI / 180.0),
                                      getArg(17) * sin(getArg(18) * M_PI / 180.0));
    a_kskpnr_ =
      (1.0 + getArg(24)) * EvtComplex(getArg(22) * cos(getArg(23) * M_PI / 180.0),
                                      getArg(22) * sin(getArg(23) * M_PI / 180.0));
    a_kskmnr_ =
      (1.0 + getArg(29)) * EvtComplex(getArg(27) * cos(getArg(28) * M_PI / 180.0),
                                      getArg(27) * sin(getArg(28) * M_PI / 180.0));

    abar_f0ks_ =
      (1.0 - getArg(3)) * EvtComplex(getArg(1) * cos(getArg(2) * M_PI / 180.0),
                                     getArg(1) * sin(getArg(2) * M_PI / 180.0));
    abar_phiks_ =
      (1.0 - getArg(7)) * EvtComplex(getArg(5) * cos(getArg(6) * M_PI / 180.0),
                                     getArg(5) * sin(getArg(6) * M_PI / 180.0));
    abar_fxks_ =
      (1.0 - getArg(11)) * EvtComplex(getArg(9) * cos(getArg(10) * M_PI / 180.0),
                                      getArg(9) * sin(getArg(10) * M_PI / 180.0));
    abar_chic0ks_ =
      (1.0 - getArg(15)) * EvtComplex(getArg(13) * cos(getArg(14) * M_PI / 180.0),
                                      getArg(13) * sin(getArg(14) * M_PI / 180.0));
    abar_kpkmnr_ =
      (1.0 - getArg(19)) * EvtComplex(getArg(17) * cos(getArg(18) * M_PI / 180.0),
                                      getArg(17) * sin(getArg(18) * M_PI / 180.0));
    abar_kskpnr_ =
      (1.0 - getArg(24)) * EvtComplex(getArg(22) * cos(getArg(23) * M_PI / 180.0),
                                      getArg(22) * sin(getArg(23) * M_PI / 180.0));
    abar_kskmnr_ =
      (1.0 - getArg(29)) * EvtComplex(getArg(27) * cos(getArg(28) * M_PI / 180.0),
                                      getArg(27) * sin(getArg(28) * M_PI / 180.0));

    alpha_kpkmnr = getArg(21);
    alpha_kskpnr = getArg(26);
    alpha_kskmnr = getArg(31);

    std::cout << setiosflags(std::ios::left) << std::endl
              << "B0 Channel  " << std::setw(20) << "Relative amplitude" << std::setw(20) << "Relative phase" << std::endl
              << "f0ks        " << std::setw(20) << real(a_f0ks_)        << std::setw(20) << imag(a_f0ks_)    << std::endl
              << "phiks       " << std::setw(20) << real(a_phiks_)       << std::setw(20) << imag(a_phiks_)   << std::endl
              << "fxks        " << std::setw(20) << real(a_fxks_)        << std::setw(20) << imag(a_fxks_)    << std::endl
              << "chic0ks     " << std::setw(20) << real(a_chic0ks_)     << std::setw(20) << imag(a_chic0ks_) << std::endl
              << "kpkmnr      " << std::setw(20) << real(a_kpkmnr_)      << std::setw(20) << imag(a_kpkmnr_)  << std::endl
              << "kskpnr      " << std::setw(20) << real(a_kskpnr_)      << std::setw(20) << imag(a_kskpnr_)  << std::endl
              << "kskmnr      " << std::setw(20) << real(a_kskmnr_)      << std::setw(20) << imag(a_kskmnr_)  << std::endl
              << std::endl;

    std::cout << setiosflags(std::ios::left) << std::endl
              << "B0B Channel " << std::setw(20) << "Relative amplitude" << std::setw(20) << "Relative phase"    << std::endl
              << "f0ks        " << std::setw(20) << real(abar_f0ks_)     << std::setw(20) << imag(abar_f0ks_)    << std::endl
              << "phiks       " << std::setw(20) << real(abar_phiks_)    << std::setw(20) << imag(abar_phiks_)   << std::endl
              << "fxks        " << std::setw(20) << real(abar_fxks_)     << std::setw(20) << imag(abar_fxks_)    << std::endl
              << "chic0ks     " << std::setw(20) << real(abar_chic0ks_)  << std::setw(20) << imag(abar_chic0ks_) << std::endl
              << "kpkmnr      " << std::setw(20) << real(abar_kpkmnr_)   << std::setw(20) << imag(abar_kpkmnr_)  << std::endl
              << "kskpnr      " << std::setw(20) << real(abar_kskpnr_)   << std::setw(20) << imag(abar_kskpnr_)  << std::endl
              << "kskmnr      " << std::setw(20) << real(abar_kskmnr_)   << std::setw(20) << imag(abar_kskmnr_)  << std::endl
              << std::endl;

    // Open debugging file
    debugfile_.open("debug.dat", std::ios::out);
  }

  void EvtB0toKsKK::initProbMax()
  {
    //setProbMax(50000.0);
    //setProbMax(100000.0);
    //setProbMax(200000.0);
    //setProbMax(400000.0);
    //setProbMax(600000.0);
    setProbMax(1100000.0);
  }

  void EvtB0toKsKK::decay(EvtParticle* p)
  {
    // Btag
    static EvtId B0  = EvtPDL::getId("B0");
    static EvtId B0B = EvtPDL::getId("anti-B0");

    double t;
    EvtId other_b;

    //std::cout << EvtCPUtil::getInstance()->getMixingType() << std::endl;
    //EvtCPUtil::getInstance()->setMixingType(0);
    EvtCPUtil::getInstance()->OtherB(p, t, other_b, 0.5);
    //EvtCPUtil::getInstance()->OtherB(p,t,other_b,0.4);
    //EvtCPUtil::getInstance()->OtherB(p,t,other_b);

    // Brec
    p->initializePhaseSpace(getNDaug(), getDaugs());
    EvtVector4R p4ks, p4kp, p4km;
    if (p->getId() == B0) {
      p4ks = p->getDaug(0)->getP4();
      p4kp = p->getDaug(1)->getP4();
      p4km = p->getDaug(2)->getP4();
      ///////EVTGEN BUG!!! Decays B0 --> Ks K- K+ instead of Ks K+ K-
      ///////Depends on EvtCPUtil::getInstance()->OtherB(p,t,other_b,0.5);
      /*p4ks = p->getDaug(0)->getP4();
      p4kp = p->getDaug(2)->getP4();
      p4km = p->getDaug(1)->getP4();*/
    } else {
      p4ks = p->getDaug(0)->getP4();
      p4kp = p->getDaug(2)->getP4();
      p4km = p->getDaug(1)->getP4();
      ///////EVTGEN BUG!!! Decays B0b --> Ks K+ K- instead of Ks K- K+
      /*p4ks = p->getDaug(0)->getP4();
      p4kp = p->getDaug(1)->getP4();
      p4km = p->getDaug(2)->getP4();*/
    }

    /*std::cout << (p4ks + p4kp).mass() << " " << (p4ks + p4km).mass() << " "
        << (p4kp + p4km).mass() << std::endl;
    if( p->getId() == other_b )
      std::cout << "same flavour" << std::endl;
    std::cout << p->getId() << " --> " << p->getDaug(0)->getId() << " "
        << p->getDaug(1)->getId() << " " << p->getDaug(2)->getId()
        << std::endl;
    std::cout << B0 << std::endl;
    std::cout << B0B << std::endl;
    std::cout << other_b << std::endl;*/
    /*std::cout << p->getLifetime() << std::endl;
    std::cout << p->get4Pos() << std::endl;
    std::cout << p->getP4() << std::endl;
    std::cout << p->getP4Lab() << std::endl;
    EvtParticle *parent=p->getParent();
    if (parent->getDaug(0)!=p){
      std::cout << parent->getDaug(0)->getLifetime() << std::endl;
      std::cout << parent->getDaug(0)->get4Pos() << std::endl;
      std::cout << parent->getDaug(0)->getP4() << std::endl;
      std::cout << parent->getDaug(0)->getP4Lab() << std::endl;
      std::cout << parent->getDaug(0)->getP4Lab()+p->getP4Lab() << std::endl;
    }
    else{
      std::cout << parent->getDaug(1)->getLifetime() << std::endl;
      std::cout << parent->getDaug(1)->get4Pos() << std::endl;
      std::cout << parent->getDaug(1)->getP4() << std::endl;
      std::cout << parent->getDaug(1)->getP4Lab() << std::endl;
      std::cout << parent->getDaug(1)->getP4Lab()+p->getP4Lab() << std::endl;
    }
    std::cout << parent->getP4() << std::endl;
    std::cout << t << std::endl;*/

    // Relative amplides and phases with direct CP violation
    a_f0ks_ =
      (1.0 + getArg(3)) * EvtComplex(getArg(1) * cos(getArg(2) * M_PI / 180.0),
                                     getArg(1) * sin(getArg(2) * M_PI / 180.0));
    a_phiks_ =
      (1.0 + getArg(7)) * EvtComplex(getArg(5) * cos(getArg(6) * M_PI / 180.0),
                                     getArg(5) * sin(getArg(6) * M_PI / 180.0));
    a_fxks_ =
      (1.0 + getArg(11)) * EvtComplex(getArg(9) * cos(getArg(10) * M_PI / 180.0),
                                      getArg(9) * sin(getArg(10) * M_PI / 180.0));
    a_chic0ks_ =
      (1.0 + getArg(15)) * EvtComplex(getArg(13) * cos(getArg(14) * M_PI / 180.0),
                                      getArg(13) * sin(getArg(14) * M_PI / 180.0));
    a_kpkmnr_ =
      (1.0 + getArg(19)) * EvtComplex(getArg(17) * cos(getArg(18) * M_PI / 180.0),
                                      getArg(17) * sin(getArg(18) * M_PI / 180.0));
    a_kskpnr_ =
      (1.0 + getArg(24)) * EvtComplex(getArg(22) * cos(getArg(23) * M_PI / 180.0),
                                      getArg(22) * sin(getArg(23) * M_PI / 180.0));
    a_kskmnr_ =
      (1.0 + getArg(29)) * EvtComplex(getArg(27) * cos(getArg(28) * M_PI / 180.0),
                                      getArg(27) * sin(getArg(28) * M_PI / 180.0));

    abar_f0ks_ =
      (1.0 - getArg(3)) * EvtComplex(getArg(1) * cos(getArg(2) * M_PI / 180.0),
                                     getArg(1) * sin(getArg(2) * M_PI / 180.0));
    abar_phiks_ =
      (1.0 - getArg(7)) * EvtComplex(getArg(5) * cos(getArg(6) * M_PI / 180.0),
                                     getArg(5) * sin(getArg(6) * M_PI / 180.0));
    abar_fxks_ =
      (1.0 - getArg(11)) * EvtComplex(getArg(9) * cos(getArg(10) * M_PI / 180.0),
                                      getArg(9) * sin(getArg(10) * M_PI / 180.0));
    abar_chic0ks_ =
      (1.0 - getArg(15)) * EvtComplex(getArg(13) * cos(getArg(14) * M_PI / 180.0),
                                      getArg(13) * sin(getArg(14) * M_PI / 180.0));
    abar_kpkmnr_ =
      (1.0 - getArg(19)) * EvtComplex(getArg(17) * cos(getArg(18) * M_PI / 180.0),
                                      getArg(17) * sin(getArg(18) * M_PI / 180.0));
    abar_kskpnr_ =
      (1.0 - getArg(24)) * EvtComplex(getArg(22) * cos(getArg(23) * M_PI / 180.0),
                                      getArg(22) * sin(getArg(23) * M_PI / 180.0));
    abar_kskmnr_ =
      (1.0 - getArg(29)) * EvtComplex(getArg(27) * cos(getArg(28) * M_PI / 180.0),
                                      getArg(27) * sin(getArg(28) * M_PI / 180.0));

    // Mixing-induced CP asymmetry
    const double pCP_f0ks    = getArg(4) * M_PI / 180.0;
    const double pCP_phiks   = getArg(8) * M_PI / 180.0;
    const double pCP_fxks    = getArg(12) * M_PI / 180.0;
    const double pCP_chic0ks = getArg(16) * M_PI / 180.0;
    const double pCP_kpkmnr  = getArg(20) * M_PI / 180.0;
    const double pCP_kskpnr  = getArg(25) * M_PI / 180.0;
    const double pCP_kskmnr  = getArg(30) * M_PI / 180.0;

    if (other_b == B0) {
      a_f0ks_ *=
        EvtComplex(cos(+2.0 * pCP_f0ks), sin(+2.0 * pCP_f0ks));
      a_phiks_ *=
        EvtComplex(cos(+2.0 * pCP_phiks), sin(+2.0 * pCP_phiks));
      a_fxks_ *=
        EvtComplex(cos(+2.0 * pCP_fxks), sin(+2.0 * pCP_fxks));
      a_chic0ks_ *=
        EvtComplex(cos(+2.0 * pCP_chic0ks), sin(+2.0 * pCP_chic0ks));
      a_kpkmnr_ *=
        EvtComplex(cos(+2.0 * pCP_kpkmnr), sin(+2.0 * pCP_kpkmnr));
      a_kskpnr_ *=
        EvtComplex(cos(+2.0 * pCP_kskpnr), sin(+2.0 * pCP_kskpnr));
      a_kskmnr_ *=
        EvtComplex(cos(+2.0 * pCP_kskmnr), sin(+2.0 * pCP_kskmnr));
    }
    if (other_b == B0B) {
      abar_f0ks_ *=
        EvtComplex(cos(-2.0 * pCP_f0ks), sin(-2.0 * pCP_f0ks));
      abar_phiks_ *=
        EvtComplex(cos(-2.0 * pCP_phiks), sin(-2.0 * pCP_phiks));
      abar_fxks_ *=
        EvtComplex(cos(-2.0 * pCP_fxks), sin(-2.0 * pCP_fxks));
      abar_chic0ks_ *=
        EvtComplex(cos(-2.0 * pCP_chic0ks), sin(-2.0 * pCP_chic0ks));
      abar_kpkmnr_ *=
        EvtComplex(cos(-2.0 * pCP_kpkmnr), sin(-2.0 * pCP_kpkmnr));
      abar_kskpnr_ *=
        EvtComplex(cos(-2.0 * pCP_kskpnr), sin(-2.0 * pCP_kskpnr));
      abar_kskmnr_ *=
        EvtComplex(cos(-2.0 * pCP_kskmnr), sin(-2.0 * pCP_kskmnr));
    }

    //Form Factors
    EvtComplex Amp_f0ks    = A_f0ks(p4ks, p4kp, p4km);
    EvtComplex Amp_phiks   = A_phiks(p4ks, p4kp, p4km);
    EvtComplex Amp_fxks    = A_fxks(p4ks, p4kp, p4km);
    EvtComplex Amp_chic0ks = A_chic0ks(p4ks, p4kp, p4km);
    EvtComplex Amp_kpkmnr  = A_kknr(p4kp, p4km, alpha_kpkmnr);
    EvtComplex Amp_kskpnr  = A_kknr(p4ks, p4kp, alpha_kskpnr);
    EvtComplex Amp_kskmnr  = A_kknr(p4ks, p4km, alpha_kskmnr);

    EvtComplex Ampbar_f0ks    = A_f0ks(p4ks, p4km, p4kp);
    EvtComplex Ampbar_phiks   = A_phiks(p4ks, p4km, p4kp);
    EvtComplex Ampbar_fxks    = A_fxks(p4ks, p4km, p4kp);
    EvtComplex Ampbar_chic0ks = A_chic0ks(p4ks, p4km, p4kp);
    EvtComplex Ampbar_kpkmnr  = A_kknr(p4km, p4kp, alpha_kpkmnr);
    EvtComplex Ampbar_kskpnr  = A_kknr(p4ks, p4km, alpha_kskpnr);
    EvtComplex Ampbar_kskmnr  = A_kknr(p4ks, p4kp, alpha_kskmnr);

    const EvtComplex A_B0toKsKK =
      (a_f0ks_   * Amp_f0ks)    +
      (a_phiks_  * Amp_phiks)   +
      (a_fxks_   * Amp_fxks)    +
      (a_chic0ks_ * Amp_chic0ks) +
      (a_kpkmnr_ * Amp_kpkmnr)  +
      (a_kskpnr_ * Amp_kskpnr)  +
      (a_kskmnr_ * Amp_kskmnr);

    const EvtComplex Abar_B0toKsKK =
      (abar_f0ks_   * Ampbar_f0ks)    +
      (abar_phiks_  * Ampbar_phiks)   +
      (abar_fxks_   * Ampbar_fxks)    +
      (abar_chic0ks_ * Ampbar_chic0ks) +
      (abar_kpkmnr_ * Ampbar_kpkmnr)  +
      (abar_kskpnr_ * Ampbar_kskpnr)  +
      (abar_kskmnr_ * Ampbar_kskmnr);

    // CP asymmetry
    const double dm = getArg(0);

    EvtComplex amp;
    if (other_b == B0B) {
      amp =
        A_B0toKsKK * cos(dm * t / (2.0 * EvtConst::c)) +
        EvtComplex(0.0, 1.0) * Abar_B0toKsKK * sin(dm * t / (2.0 * EvtConst::c));
    }
    if (other_b == B0) {
      amp =
        A_B0toKsKK *
        EvtComplex(0.0, 1.0) * sin(dm * t / (2.0 * EvtConst::c)) +
        Abar_B0toKsKK * cos(dm * t / (2.0 * EvtConst::c));
    }

    if (abs2(amp) > 50000.0)
      debugfile_ << abs2(amp) << std::endl;
    vertex(amp);

    return;
  }

  EvtVector4R EvtB0toKsKK::umu(const EvtVector4R& p4a, const EvtVector4R& p4b,
                               const EvtVector4R& p4c)
  {
    const double s = (p4a + p4b + p4c) * (p4a + p4b + p4c);

    const EvtVector4R umu = (p4a + p4b + p4c) / sqrt(s);

    return umu;
  }

  EvtVector4R EvtB0toKsKK::Smu(const EvtVector4R& p4a, const EvtVector4R& p4b,
                               const EvtVector4R& /*p4c*/)
  {
    const double ma = p4a.mass();
    const double mb = p4b.mass();
    const double mres = (p4a + p4b).mass();
    const double ma2 = ma * ma;
    const double mb2 = mb * mb;
    const double mres2 = mres * mres;

    const double N1 =
      mres /
      (sqrt(mres2 - ((ma + mb) * (ma + mb))) * sqrt(mres2 - ((ma - mb) * (ma - mb))));

    const EvtVector4R Smu =
      N1 * ((p4a - p4b) - ((p4a + p4b) * (ma2 - mb2) / mres2));

    return Smu;
  }

  EvtVector4R EvtB0toKsKK::Lmu(const EvtVector4R& p4a, const EvtVector4R& p4b,
                               const EvtVector4R& p4c)
  {
    const double mc = p4c.mass();
    const double mres = (p4a + p4b).mass();
    const double mc2 = mc * mc;
    const double mres2 = mres * mres;
    const double s = (p4a + p4b + p4c) * (p4a + p4b + p4c);

    const double N2 =
      sqrt(s) /
      (sqrt(s - ((mres + mc) * (mres + mc))) * sqrt(s - ((mres - mc) * (mres - mc))));

    const EvtVector4R Lmu =
      N2 * ((p4c - (p4a + p4b)) - ((p4c + (p4a + p4b)) * (mc2 - mres2) / s));

    return Lmu;
  }

  EvtTensor4C EvtB0toKsKK::gmunu_tilde(const EvtVector4R& p4a,
                                       const EvtVector4R& p4b,
                                       const EvtVector4R& p4c)
  {
    const double s = (p4a + p4b + p4c) * (p4a + p4b + p4c);

    const EvtVector4R umu = (p4a + p4b + p4c) / sqrt(s);

    const EvtTensor4C gmunu_tilde =
      EvtTensor4C::g() - EvtGenFunctions::directProd(umu, umu);

    return gmunu_tilde;
  }

  EvtTensor4C EvtB0toKsKK::Tmunu(const EvtVector4R& p4a, const EvtVector4R& p4b,
                                 const EvtVector4R& p4c)
  {
    const double mres = (p4a + p4b).mass();
    const EvtVector4R wmu = (p4a + p4b) / mres;

    const EvtTensor4C Tmunu =
      sqrt(3.0 / 2.0) *
      (EvtGenFunctions::directProd(Smu(p4a, p4b, p4c), Smu(p4a, p4b, p4c)) +
       (1.0 / 3.0 * (EvtTensor4C::g() - EvtGenFunctions::directProd(wmu, wmu))));

    return Tmunu;
  }

  EvtTensor4C EvtB0toKsKK::Multiply(const EvtTensor4C& t1,
                                    const EvtTensor4C& t2)
  {
    EvtTensor4C t;
    for (unsigned int i = 0; i < 4; i++)
      for (unsigned int j = 0; j < 4; j++) {
        const EvtComplex element =
          t1.get(i, 0) * t2.get(0, j) +
          t1.get(i, 1) * t2.get(1, j) +
          t1.get(i, 2) * t2.get(2, j) +
          t1.get(i, 3) * t2.get(3, j);
        t.set(i, j, element);
      }
    return t;
  }

  EvtTensor4C EvtB0toKsKK::RaiseIndices(const EvtTensor4C& t)
  {
    const EvtTensor4C tmp = Multiply(t, EvtTensor4C::g());
    const EvtTensor4C t_raised = Multiply(EvtTensor4C::g(), tmp);

    return t_raised;
  }

  void EvtB0toKsKK::RaiseIndex(EvtVector4R& vector)
  {
    vector.set(1, -vector.get(1));
    vector.set(2, -vector.get(2));
    vector.set(3, -vector.get(3));
  }

  EvtTensor4C EvtB0toKsKK::Mmunu(const EvtVector4R& p4a, const EvtVector4R& p4b,
                                 const EvtVector4R& p4c)
  {
    const EvtTensor4C Mmunu =
      sqrt(3.0 / 2.0) *
      (EvtGenFunctions::directProd(Lmu(p4a, p4b, p4c), Lmu(p4a, p4b, p4c)) +
       (1.0 / 3.0 * gmunu_tilde(p4a, p4b, p4c)));

    return Mmunu;
  }

  double EvtB0toKsKK::BWBF(const double& q, const unsigned int& L)
  {
    //Meson radius
    const double d = 1.0;

    const double z = q * q * d * d;

    double bwbf = 1.0;

    if (L == 1)
      bwbf = sqrt(2.0 * z / (1.0 + z));
    if (L == 2)
      bwbf = sqrt(13.0 * z * z / (((z - 3.0) * (z - 3.0)) + (9.0 * z)));
    if (L > 2) {
      std::cout << "ERROR: BWBF not implemented for L>2" << std::endl;
      exit(1);
    }
    return bwbf;
  }

  double EvtB0toKsKK::BWBF(const double& q, const double& q0,
                           const unsigned int& L)
  {
    //Meson radius
    const double d = 1.0;

    const double z  = q * q * d * d;
    const double z0 = q0 * q0 * d * d;

    double bwbf = 1.0;

    if (L == 1)
      bwbf = sqrt((1.0 + z0) / (1.0 + z));
    if (L == 2)
      bwbf = sqrt((((z0 - 3.0) * (z0 - 3.0)) + (9.0 * z0)) / (((z - 3.0) * (z - 3.0)) + (9.0 * z)));
    if (L > 2) {
      std::cout << "ERROR: BWBF not implemented for L>2" << std::endl;
      exit(1);
    }
    return bwbf;
  }

  EvtComplex EvtB0toKsKK::BreitWigner(const double& m, const double& m0,
                                      const double& Gamma0,
                                      const double& q, const double& q0,
                                      const unsigned int& L)
  {
    const double s = m * m;
    const double s0 = m0 * m0;

    const double Gamma =
      Gamma0 * m0 / m * pow(q / q0, (2 * L) + 1) *
      BWBF(q, q0, L) * BWBF(q, q0, L);

    const EvtComplex BreitWigner = 1.0 / EvtComplex(s0 - s, -m0 * Gamma);

    return BreitWigner;
  }

  EvtVector4R EvtB0toKsKK::Boost(const EvtVector4R& p4,
                                 const EvtVector4R& boost)
  {
    const double bx = boost.get(1) / boost.get(0);
    const double by = boost.get(2) / boost.get(0);
    const double bz = boost.get(3) / boost.get(0);

    const double bx2 = bx * bx;
    const double by2 = by * by;
    const double bz2 = bz * bz;

    const double b2 = bx2 + by2 + bz2;
    if (b2 == 0.0)
      return p4;
    assert(b2 < 1.0);

    const double gamma = 1.0 / sqrt(1 - b2);

    const double gb2 = (gamma - 1.0) / b2;

    const double gb2xy = gb2 * bx * by;
    const double gb2xz = gb2 * bx * bz;
    const double gb2yz = gb2 * by * bz;

    const double gbx = gamma * bx;
    const double gby = gamma * by;
    const double gbz = gamma * bz;

    const double e_b =
      (gamma * p4.get(0)) - (gbx * p4.get(1)) - (gby * p4.get(2)) - (gbz * p4.get(3));
    const double x_b =
      (-gbx * p4.get(0)) + ((1.0 + (gb2 * bx2)) * p4.get(1)) +
      (gb2xy * p4.get(2)) + (gb2xz * p4.get(3));
    const double y_b =
      (-gby * p4.get(0)) + (gb2xy * p4.get(1)) +
      ((1.0 + (gb2 * by2)) * p4.get(2)) + (gb2yz * p4.get(3));
    const double z_b =
      (-gbz * p4.get(0)) + (gb2xz * p4.get(1)) +
      (gb2yz * p4.get(2)) + ((1.0 + (gb2 * bz2)) * p4.get(3));

    const EvtVector4R p4_b(e_b, x_b, y_b, z_b);

    return p4_b;
  }

  double EvtB0toKsKK::p(const double& mab, const double& M, const double& mc)
  {
    const double sab = mab * mab;
    const double M_p_mc2 = (M + mc) * (M + mc);
    const double M_m_mc2 = (M - mc) * (M - mc);

    const double p = sqrt((sab - M_p_mc2) * (sab - M_m_mc2)) / (2.0 * mab);

    return p;
  }

  double EvtB0toKsKK::q(const double& mab, const double& ma, const double& mb)
  {
    const double mab2 = mab * mab;
    const double ma_p_mb2 = (ma + mb) * (ma + mb);
    const double ma_m_mb2 = (ma - mb) * (ma - mb);

    const double q = sqrt((mab2 - ma_p_mb2) * (mab2 - ma_m_mb2)) / (2.0 * mab);

    return q;
  }

  EvtComplex EvtB0toKsKK::Flatte_k(const double& s, const double& m_h)
  {
    const double k2 = 1.0 - (4.0 * m_h * m_h / s);
    EvtComplex k;
    if (k2 < 0.0)
      k = EvtComplex(0.0, sqrt(fabs(k2)));
    else
      k = sqrt(k2);

    return k;
  }

  EvtComplex EvtB0toKsKK::Flatte(const double& m, const double& m0)
  {
    const double g_pipi = 0.165;
    const double g_kk = 4.21 * g_pipi;

    static EvtId pip = EvtPDL::getId("pi+");
    static EvtId kp  = EvtPDL::getId("K+");

    const double s = m * m;
    const double s0 = m0 * m0;

    const EvtComplex rho_pipi = 2.0 * Flatte_k(s, EvtPDL::getMeanMass(pip)) / m;
    const EvtComplex rho_kk = 2.0 * Flatte_k(s, EvtPDL::getMeanMass(kp)) / m;

    const EvtComplex Gamma =
      EvtComplex(0.0, 1.0) * ((g_pipi * rho_pipi) + (g_kk * rho_kk));

    const EvtComplex Flatte = 1.0 / (s0 - s - Gamma);

    return Flatte;
  }

  EvtComplex EvtB0toKsKK::A_f0ks(const EvtVector4R& /*p4ks*/,
                                 const EvtVector4R& p4kp,
                                 const EvtVector4R& p4km)
  {
    static EvtId f0 = EvtPDL::getId("f_0");

    const double f0_m = EvtPDL::getMeanMass(f0);

    const EvtVector4R p4kpkm = p4kp + p4km;
    const double mkpkm = p4kpkm.mass();

    //Angular distribution
    const EvtComplex H_f0ks = 1.0;

    //Barrier factors
    const EvtComplex D_f0ks = 1.0;

    //Line shape
    const EvtComplex L_f0ks = Flatte(mkpkm, f0_m);

    //Amplitude
    const EvtComplex A_f0ks = D_f0ks * H_f0ks * L_f0ks;

    return A_f0ks;
  }

  double s13_min(const double& s12, const double& M,
                 const double& m1, const double& m2, const double& m3)
  {
    const double E2star = (s12 - (m2 * m2) + (m1 * m1)) / 2.0 / sqrt(s12);
    const double E3star = (M * M - s12 - (m3 * m3)) / 2.0 / sqrt(s12);

    const double s23_min =
      ((E2star + E3star) * (E2star + E3star)) -
      ((sqrt((E2star * E2star) - (m1 * m1)) + sqrt((E3star * E3star) - (m3 * m3))) *
       (sqrt((E2star * E2star) - (m1 * m1)) + sqrt((E3star * E3star) - (m3 * m3))));

    return s23_min;
  }

  double s13_max(const double& s12, const double& M,
                 const double& m1, const double& m2, const double& m3)
  {
    const double E2star = (s12 - (m2 * m2) + (m1 * m1)) / 2.0 / sqrt(s12);
    const double E3star = (M * M - s12 - (m3 * m3)) / 2.0 / sqrt(s12);

    const double s23_max =
      ((E2star + E3star) * (E2star + E3star)) -
      ((sqrt((E2star * E2star) - (m1 * m1)) - sqrt((E3star * E3star) - (m3 * m3))) *
       (sqrt((E2star * E2star) - (m1 * m1)) - sqrt((E3star * E3star) - (m3 * m3))));

    return s23_max;
  }

  EvtComplex EvtB0toKsKK::A_phiks(const EvtVector4R& p4ks,
                                  const EvtVector4R& p4kp,
                                  const EvtVector4R& p4km)
  {
    static EvtId phi = EvtPDL::getId("phi");

    const double phi_m = EvtPDL::getMeanMass(phi);
    const double phi_w = EvtPDL::getWidth(phi);

    const EvtVector4R p4kpkm = p4kp + p4km;
    const double mkpkm = p4kpkm.mass();

    //Angular distribution
    const EvtVector4R S_mu = Smu(p4kp, p4km, p4ks);
    const EvtVector4R L_mu = Lmu(p4kp, p4km, p4ks);

    const EvtTensor4C g_munu_tilde = gmunu_tilde(p4kp, p4km, p4ks);
    const EvtTensor4C g__munu_tilde = RaiseIndices(g_munu_tilde);

    EvtComplex H_phiks = 0.0;
    for (unsigned int i = 0; i < 4; i++)
      for (unsigned int j = 0; j < 4; j++)
        H_phiks += g__munu_tilde.get(i, j) * S_mu.get(i) * L_mu.get(j);

    //Barrier factors
    const EvtVector4R p4kp_kpkm = Boost(p4kp, p4kpkm);

    const EvtComplex D_phiks = BWBF(p4kp_kpkm.d3mag(), 1);

    //Line shape
    const double q0_phi = q(phi_m, p4kp.mass(), p4km.mass());
    const EvtComplex L_phiks =
      BreitWigner(mkpkm, phi_m, phi_w, p4kp_kpkm.d3mag(), q0_phi, 1);

    //Amplitude
    const EvtComplex A_phiks = D_phiks * H_phiks * L_phiks;

    ////
    /*const EvtVector4R p4ks_kpkm = Boost(p4ks, p4kpkm);
    const double p4ks_kpkm_alt = p(mkpkm, (p4ks+p4kp+p4km).mass(), p4ks.mass());
    const double p4kp_kpkm_alt = q(mkpkm, p4kp.mass(), p4km.mass());
    //std::cout << p4ks_kpkm.d3mag() << std::endl;
    //std::cout << p4ks_kpkm_alt << std::endl;
    //std::cout << p4kp_kpkm.d3mag() << std::endl;
    //std::cout << p4kp_kpkm_alt << std::endl;
    const double costheta =
      p4ks_kpkm.dot(p4kp_kpkm)/p4ks_kpkm.d3mag()/p4kp_kpkm.d3mag();
    //const double z = p4ks_kpkm.d3mag()/(p4ks+p4kp+p4km).mass();
    //const double H_phiks_alt = -sqrt(1.0+(z*z))*costheta;
    const double sp_min =
      s13_min( mkpkm*mkpkm, (p4ks+p4kp+p4km).mass(),
         p4kp.mass(), p4km.mass(), p4ks.mass() );
    const double sp_max =
      s13_max( mkpkm*mkpkm, (p4ks+p4kp+p4km).mass(),
         p4kp.mass(), p4km.mass(), p4ks.mass() );
    const double sp = (p4ks+p4kp).mass2();
    const double costheta_alt = (sp_max + sp_min - (2.0*sp))/(sp_max - sp_min);
    //std::cout << costheta << std::endl;
    //std::cout << costheta_alt << std::endl;
    //exit(1);
    const double z_alt = p4ks_kpkm_alt/(p4ks+p4kp+p4km).mass();
    const double H_phiks_alt = -sqrt(1.0+(z_alt*z_alt))*costheta_alt;
    //std::cout << H_phiks << std::endl;
    //std::cout << H_phiks_alt << std::endl;
    if( real(H_phiks) != H_phiks_alt )
      {
        std::cout << H_phiks << std::endl;
        std::cout << H_phiks_alt << std::endl;
      }
      const EvtComplex A_phiks = D_phiks*H_phiks_alt*L_phiks;*/
    ////

    return A_phiks;
  }

  EvtComplex EvtB0toKsKK::A_fxks(const EvtVector4R& /*p4ks*/,
                                 const EvtVector4R& p4kp,
                                 const EvtVector4R& p4km)
  {
    static EvtId fx = EvtPDL::getId("f_0(1500)");

    const double fx_m = EvtPDL::getMeanMass(fx);
    const double fx_w = EvtPDL::getWidth(fx);

    const EvtVector4R p4kpkm = p4kp + p4km;
    const double mkpkm = p4kpkm.mass();

    //Angular distribution
    const EvtComplex H_fxks = 1.0;

    //Barrier factors
    const EvtComplex D_fxks = 1.0;

    //Line shape
    const EvtVector4R p4kp_kpkm = Boost(p4kp, p4kpkm);
    const double q0_fx = q(fx_m, p4kp.mass(), p4km.mass());
    const EvtComplex L_fxks =
      BreitWigner(mkpkm, fx_m, fx_w, p4kp_kpkm.d3mag(), q0_fx, 0);

    //Amplitude
    const EvtComplex A_fxks = D_fxks * H_fxks * L_fxks;

    return A_fxks;
  }

  EvtComplex EvtB0toKsKK::A_chic0ks(const EvtVector4R& /*p4ks*/,
                                    const EvtVector4R& p4kp,
                                    const EvtVector4R& p4km)
  {
    static EvtId chic0 = EvtPDL::getId("chi_c0");

    const double chic0_m = EvtPDL::getMeanMass(chic0);
    const double chic0_w = EvtPDL::getWidth(chic0);

    const EvtVector4R p4kpkm = p4kp + p4km;
    const double mkpkm = p4kpkm.mass();

    //Angular distribution
    const EvtComplex H_chic0ks = 1.0;

    //Barrier factors
    const EvtComplex D_chic0ks = 1.0;

    //Line shape
    const EvtVector4R p4kp_kpkm = Boost(p4kp, p4kpkm);
    const double q0_chic0 = q(chic0_m, p4kp.mass(), p4km.mass());
    const EvtComplex L_chic0ks =
      BreitWigner(mkpkm, chic0_m, chic0_w, p4kp_kpkm.d3mag(), q0_chic0, 0);

    //Amplitude
    const EvtComplex A_chic0ks = D_chic0ks * H_chic0ks * L_chic0ks;

    return A_chic0ks;
  }

  EvtComplex EvtB0toKsKK::A_kknr(const EvtVector4R& p4k1,
                                 const EvtVector4R& p4k2,
                                 const double& alpha_kk)
  {
    const EvtVector4R p4kk = p4k1 + p4k2;
    const double m2kk = p4kk.mass2();

    const EvtComplex A_kknr = exp(-alpha_kk * m2kk);

    return A_kknr;
  }

} // Belle 2 Namespace
