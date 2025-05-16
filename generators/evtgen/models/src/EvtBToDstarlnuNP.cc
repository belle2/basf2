/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include "generators/evtgen/EvtGenModelRegister.h"
#include "generators/evtgen/models/EvtBToDstarlnuNP.h"

#include "EvtGenBase/EvtGenKine.hh"
#include "EvtGenBase/EvtPDL.hh"
#include "EvtGenBase/EvtParticle.hh"
#include "EvtGenBase/EvtDiracSpinor.hh"
#include "EvtGenBase/EvtTensor4C.hh"
#include "EvtGenBase/EvtVector4C.hh"
#include "EvtGenBase/EvtScalarParticle.hh"

/** register the model in EvtGen */
B2_EVTGEN_REGISTER_MODEL(EvtBToDstarlnuNP);

using std::endl;

std::string EvtBToDstarlnuNP::getName()
{
  return "BTODSTARLNUNP";
}

EvtDecayBase* EvtBToDstarlnuNP::clone()
{
  return new EvtBToDstarlnuNP;
}

void EvtBToDstarlnuNP::decay(EvtParticle* p)
{
  p->initializePhaseSpace(getNDaug(), getDaugs());
  CalcAmp(p, _amp2);
}

void EvtBToDstarlnuNP::initProbMax()
{
  setProbMax(CalcMaxProb());
}

void EvtBToDstarlnuNP::init()
{
  // First choose format of NP coefficients from the .DEC file
  // Cartesian(x,y)(0) or Polar(R,phase)(1)
  int n = getNArg();
  if (!(n == 0 || (n - 1) % 3 == 0)) {
    EvtGenReport(EVTGEN_ERROR, "EvtGen")
        << "Error in parameters in the BTODSTARLNUNP decay model." << endl;
    EvtGenReport(EVTGEN_ERROR, "EvtGen")
        << "Will terminate execution!" << endl;
    ::abort();
  }

  checkNDaug(3);

  // We expect the parent to be a scalar and
  // the daughters to be D* l+ nu_l

  checkSpinParent(EvtSpinType::SCALAR);
  checkSpinDaughter(1, EvtSpinType::DIRAC);
  checkSpinDaughter(2, EvtSpinType::NEUTRINO);

  EvtId mId = getDaug(0);
  EvtId IdDst0 = EvtPDL::getId("D*0"), IdaDst0 = EvtPDL::getId("anti-D*0"),
        IdDstp = EvtPDL::getId("D*+"), IdDstm = EvtPDL::getId("D*-");
  if (mId != IdDst0 && mId != IdaDst0 && mId != IdDstp && mId != IdDstm) {
    EvtGenReport(EVTGEN_ERROR, "EvtGen")
        << "EvtBToDstarlnuNP generator expected a D* 1st daughter, found: "
        << EvtPDL::name(getDaug(0))  << endl;
    EvtGenReport(EVTGEN_ERROR, "EvtGen")
        << "Will terminate execution!" << endl;
    ::abort();
  }

  auto getInteger = [this](int i) -> int {
    double a = getArg(i);
    if (a - static_cast<int>(a) != 0)
    {
      EvtGenReport(EVTGEN_ERROR, "EvtGen")
          << "Parameters is not integer in the BTODSTARLNUNP decay model: " << i << " " << a << endl;
      EvtGenReport(EVTGEN_ERROR, "EvtGen")
          << "Will terminate execution!" << endl;
      ::abort();
    }
    return static_cast<int>(a);
  };

  if (n > 0) { // parse arguments
    int i = 0, coordsyst = getInteger(i++);
    while (i < n) {
      int id =  getInteger(i++);   // New Physics cooeficient Id
      double a0 = getArg(i++);
      double a1 = getArg(i++);
      EvtComplex c = (coordsyst) ? EvtComplex(a0 * cos(a1), a0 * sin(a1)) : EvtComplex(a0, a1);
      if (id == 0) _Cvl = c;
      if (id == 1) _Cvr = c;
      if (id == 2) _Csl = c;
      if (id == 3) _Csr = c;
      if (id == 4) _cT  = c;
    }
  }
}

static EvtTensor4C asymProd(const EvtVector4R& a, const EvtVector4R& b)
{
  // t_{ij} = eps_{ijkl} a^{k} b^{l}
  // eps_{ijkl} is the Levi-Civita symbol -- antisymmetric tensor

  EvtTensor4C res;
  double t01 = a.get(2) * b.get(3) - a.get(3) * b.get(2);
  res.set(0, 1, t01);
  res.set(1, 0, -t01);
  double t02 = a.get(3) * b.get(1) - a.get(1) * b.get(3);
  res.set(0, 2, t02);
  res.set(2, 0, -t02);
  double t03 = a.get(1) * b.get(2) - a.get(2) * b.get(1);
  res.set(0, 3, t03);
  res.set(3, 0, -t03);
  double t12 = a.get(0) * b.get(3) - a.get(3) * b.get(0);
  res.set(1, 2, t12);
  res.set(2, 1, -t12);
  double t13 = a.get(2) * b.get(0) - a.get(0) * b.get(2);
  res.set(1, 3, t13);
  res.set(3, 1, -t13);
  double t23 = a.get(0) * b.get(1) - a.get(1) * b.get(0);
  res.set(2, 3, t23);
  res.set(3, 2, -t23);
  return res;
}

static EvtComplex asymProd(const EvtTensor4C& t1, const EvtTensor4C& t2)
{
  // returns eps_ijkl*t1(i,j)*t2(k,l);
  // eps_ijkl is the antisymmetric Levi-Civita tensor
  EvtComplex sum;
  sum += t1.get(0, 1) * t2.get(2, 3);
  sum -= t1.get(0, 1) * t2.get(3, 2);
  sum -= t1.get(0, 2) * t2.get(1, 3);
  sum += t1.get(0, 2) * t2.get(3, 1);
  sum += t1.get(0, 3) * t2.get(1, 2);
  sum -= t1.get(0, 3) * t2.get(2, 1);
  sum -= t1.get(1, 0) * t2.get(2, 3);
  sum += t1.get(1, 0) * t2.get(3, 2);
  sum += t1.get(1, 2) * t2.get(0, 3);
  sum -= t1.get(1, 2) * t2.get(3, 0);
  sum -= t1.get(1, 3) * t2.get(0, 2);
  sum += t1.get(1, 3) * t2.get(2, 0);
  sum += t1.get(2, 0) * t2.get(1, 3);
  sum -= t1.get(2, 0) * t2.get(3, 1);
  sum -= t1.get(2, 1) * t2.get(0, 3);
  sum += t1.get(2, 1) * t2.get(3, 0);
  sum += t1.get(2, 3) * t2.get(0, 1);
  sum -= t1.get(2, 3) * t2.get(1, 0);
  sum -= t1.get(3, 0) * t2.get(1, 2);
  sum += t1.get(3, 0) * t2.get(2, 1);
  sum += t1.get(3, 1) * t2.get(0, 2);
  sum -= t1.get(3, 1) * t2.get(2, 0);
  sum -= t1.get(3, 2) * t2.get(0, 1);
  sum += t1.get(3, 2) * t2.get(1, 0);
  return sum;
}

/************************************************************************************
 * Hadronic form factors evaluated according to the BGL parametrization
 */
static void getFF(double mB, double mV, double q2,
                  double& V, double& A0, double& A1, double& A2, double& T1, double& T2, double& T3)
{

  /* Quark masses from PDG */
  const double m_c = 1.27, // +- 0.02 GeV PDG
               m_b = 4.18; // +- 0.025 GeV PDG

  double rmB = 1 / mB, r = mV / mB, r2 = r * r;
  double c = 0.5 / sqrt(mB * mV), mBaV = mB + mV, mBsV = mB - mV, rmBaV = 1 / mBaV, rmBsV = 1 / mBsV;
  double w = (mB * mB + mV * mV - q2) / (2 * mB * mV), w2 = w * w, sqrtwa1 = sqrt(w + 1);
  double u = 1 + r2 - 2 * r * w;
  double z = (sqrtwa1 - sqrt(2)) / (sqrtwa1 + sqrt(2));

  /* FF parameter values are taken from Table I of arXiv:2111.01176 */
  const double
  a0f = 0.0123, // +- 0.0001
  a1f = 0.0222, // +- 0.0096
  a2f = -0.522, // +- 0.196

  a0g = 0.0318, // +- 0.0010
  a1g = -0.133, // +- 0.063
  a2g = -0.62,  // +- 1.46

  a0F2 = 0.0515, // +- 0.0021
  a1F2 = -0.149, // +- 0.059
  a2F2 = 0.987, // +- 0.932

  // Eq. (24) from kinematical constraint at w=1
  a0F1 = a0f * (1 - r) / (sqrt(2) * pow(1 + sqrt(r), 2)),
  a1F1 = 0.0021, // +- 0.0015
  // Eq. (25) from kinematical constraint at w = wmax = 1.5 after substituting all other constants
  a2F1 = -0.5162 * (104.455 * a0f + 34.7622 * a1F1 - 29.3161 * (a0F2 + 0.0557 * a1F2 + 0.0031 * a2F2)),

  nI = 2.6,
  chi1minT0 = 5.131e-4,
  chi1plusT0 = 3.894e-4,
  chi1plusL0 = 1.9421e-2;

  /* Blaschke factor calculation is based on Eq. (20) */
  auto BlaschkeFactor = [mB, mV](double zl, unsigned n, const double * m) -> double {
    double B = 1, t2 = sqrt(4 * mB * mV), mBV = mB + mV, mBV2 = mBV * mBV;
    for (unsigned i = 0; i < n; i++)
    {
      double t1 = sqrt(mBV2 - m[i] * m[i]), zp = (t1 - t2) / (t1 + t2); // zp is based on Eq. (21)
      B *= (zl - zp) / (1 - zl * zp);
    }
    return B;
  };

  static const double mP1plus[] = {6.739, 6.750, 7.145, 7.150};
  double P1plus = BlaschkeFactor(z, sizeof(mP1plus) / sizeof(mP1plus[0]), mP1plus);

  static const double mP1min[] = {6.329, 6.920, 7.020};
  double P1min = BlaschkeFactor(z, sizeof(mP1min) / sizeof(mP1min[0]), mP1min);

  static const double mP0min[] = {6.275, 6.842, 7.250};
  double P0min = BlaschkeFactor(z, sizeof(mP0min) / sizeof(mP0min[0]), mP0min);

  /* Eq. (22) phi_i(z) function evaluations */
  double zm = 1 - z, zp = 1 + z, szm = sqrt(zm), d1 = (1 + r) * zm + 2 * sqrt(r) * zp, d2 = d1 * d1, d4 = d2 * d2;
  double phif  =  4 * r / (mB * mB)    * sqrt(nI / (3 * M_PI * chi1plusT0)) * zp * (zm * szm) / d4;
  double phig  = 16 * r2           * sqrt(nI / (3 * M_PI * chi1minT0))  * zp * zp / (szm * d4);
  double phiF1 =  4 * r / (mB * mB * mB) * sqrt(nI / (6 * M_PI * chi1plusT0)) * zp * (zm * zm * szm) / (d4 * d1);
  double phiF2 =  8 * sqrt(2) * r2   * sqrt(nI / (M_PI * chi1plusL0))   * zp * zp / (szm * d4);

  /* Eq. (18) form factor evaluations */
  double ffunc  = (a0f  + z * (a1f  + z * (a2f))) / (P1plus * phif);
  double gfunc  = (a0g  + z * (a1g  + z * (a2g))) / (P1min  * phig);
  double F1func = (a0F1 + z * (a1F1 + z * (a2F1))) / (P1plus * phiF1);
  double F2func = (a0F2 + z * (a1F2 + z * (a2F2))) / (P0min  * phiF2);
  /* BGL Parametrization ends here. */

  /* Transform them into the output form factors */
  double hVw  = mB * sqrt(r) * gfunc;
  double hA1w = ffunc / (mB * sqrt(r) * (1 + w));
  double hA2w = hA1w / (1 - w) + (F1func * (w - r) - F2func * mB * mB * r * (w2 - 1)) / (mB * mB * sqrt(r) * u * (w2 - 1));
  double hA3w = (F1func * (r * w - 1) + mB * mB * sqrt(r) * (1 + w) * (F2func * r * sqrt(r) * (w - 1) + hA1w * w * u)) /
                (mB * mB * sqrt(r) * u * (w2 - 1));

  // arXiv:1309.0301 Eq 48b
  double cA1 = (m_b - m_c) * rmBsV * hA1w, cV = (m_b + m_c) * rmBaV * hVw;
  double hT1 = (0.5 * ((1 - r) * (1 - r) * (w + 1) * cA1 - (1 + r) * (1 + r) * (w - 1) * cV)) / u;
  double hT2 = (0.5 * (1 - r2) * (w + 1) * (cA1 - cV)) / u;
  double hT3 = (-0.5 * (2 * r * (w + 1) * cA1 + (m_b - m_c) * rmBsV * u * (hA3w - r * hA2w) - (1 + r) * (1 + r) * cV)) / (u *
               (1 + r));

  /************************************************************************************/

  // Form Factors :
  // arXiv:1309.0301 Eq 39
  V  = c * mBaV * hVw;
  A0 = c * ((mBaV * mBaV - q2) / (2 * mV) * hA1w - (mBaV * mBsV + q2) / (2 * mB) * hA2w - (mBaV * mBsV - q2) / (2 * mV) * hA3w);
  A1 = c * (mBaV * mBaV - q2) / mBaV * hA1w;
  A2 = c * mBaV * (hA3w + r * hA2w);
  T1 = c * (mBaV * hT1 - mBsV * hT2);
  T2 = c * ((mBaV - q2 * rmBaV) * hT1 - (mBsV - q2 * rmBsV) * hT2);
  T3 = c * (mBsV * hT1 - mBaV * hT2 - 2 * mBaV * mBsV * rmB * hT3);
}

static EvtId EM, MUM, TAUM, EP, MUP, TAUP, D0, D0B, DP, DM, DSM, DSP;
static bool cafirst = true;
void EvtBToDstarlnuNP::CalcAmp(EvtParticle* parent, EvtAmp& amp)
{
  if (cafirst) {
    cafirst = false;
    EM = EvtPDL::getId("e-"); EP = EvtPDL::getId("e+");
    MUM = EvtPDL::getId("mu-"); MUP = EvtPDL::getId("mu+");
    TAUM = EvtPDL::getId("tau-"); TAUP = EvtPDL::getId("tau+");

    D0 = EvtPDL::getId("D0"); D0B = EvtPDL::getId("anti-D0");
    DP = EvtPDL::getId("D+"); DM = EvtPDL::getId("D-");
    DSP = EvtPDL::getId("D_s+"); DSM = EvtPDL::getId("D_s-");
  }

  EvtParticle
  *pvm = parent->getDaug(0),   // vector meson
   *plp = parent->getDaug(1),   // charged lepton
    *pnu = parent->getDaug(2);   // neutrino
  EvtId pId = parent->getId(), lId = plp->getId();

  //Add the lepton and neutrino 4 momenta to find q2
  EvtVector4R q = plp->getP4() + pnu->getP4();
  double q2 = q.mass2(), m_V = pvm->mass(), m_B = parent->mass();
  double m_BaV = m_B + m_V, m_BsV = m_B - m_V;

  double a1f, a2f, vf, a0f, a3f;
  /*  Tensor form factors */
  double T1, T2, T3;
  getFF(m_B, m_V, q2, vf, a0f, a1f, a2f, T1, T2, T3);
  a3f = (m_BaV * a1f - m_BsV * a2f) / (2.0 * m_V);

  double c1 = (m_BaV * m_BsV * (T1 - T2)) / q2;
  double c2 = (T1 - T2 - T3 * q2 / (m_BaV * m_BsV)) * 2 / q2;

  if (pId == D0 || pId == D0B || pId == DP || pId == DM || pId == DSP || pId == DSM)
    vf = -vf;

  EvtVector4R p(m_B, 0., 0., 0.);
  const EvtVector4R& k = pvm->getP4();
  EvtVector4R pak = p + k;

  EvtDiracSpinor
  nu = pnu->spParentNeutrino(), lp0 = plp->spParent(0), lp1 = plp->spParent(1);

  /* Various lepton currents */
  EvtComplex s1, s2, p1, p2;
  EvtVector4C l1, l2, a1, a2;
  EvtTensor4C z1, z2, w1, w2;
  if (lId == EM || lId == MUM || lId == TAUM) {
    l1 = EvtLeptonVCurrent(nu, lp0);
    l2 = EvtLeptonVCurrent(nu, lp1);
    a1 = EvtLeptonACurrent(nu, lp0);
    a2 = EvtLeptonACurrent(nu, lp1);
    s1 = EvtLeptonSCurrent(nu, lp0);
    s2 = EvtLeptonSCurrent(nu, lp1);
    p1 = EvtLeptonPCurrent(nu, lp0);
    p2 = EvtLeptonPCurrent(nu, lp1);
    z1 = EvtLeptonTCurrent(nu, lp0);
    z2 = EvtLeptonTCurrent(nu, lp1);
  } else if (lId == EP || lId == MUP || lId == TAUP) {
    vf = -vf; /*This takes care of sign flip in vector hadronic current when you conjugate it*/
    l1 = EvtLeptonVCurrent(lp0, nu);
    l2 = EvtLeptonVCurrent(lp1, nu);
    a1 = EvtLeptonACurrent(lp0, nu);
    a2 = EvtLeptonACurrent(lp1, nu);
    s1 = EvtLeptonSCurrent(lp0, nu);
    s2 = EvtLeptonSCurrent(lp1, nu);
    p1 = EvtLeptonPCurrent(lp0, nu);
    p2 = EvtLeptonPCurrent(lp1, nu);
    z1 = EvtLeptonTCurrent(lp0, nu);
    z2 = EvtLeptonTCurrent(lp1, nu);
  }

  /* Quark masses from PDG */
  double m_c = 1.27, m_b = 4.18;

  /* New Physics */
  EvtComplex C_V_L = 1, //Standard Model
             C_V_R = _Cvr,
             C_S_L = _Csl,
             C_S_R = _Csr,
             C_T = _cT;

  C_V_L += _Cvl;

  /*Eq 12 Axial vector hadronic tensor*/
  EvtTensor4C tA = (a1f * m_BaV) * EvtTensor4C::g();
  tA.addDirProd(-(a2f / m_BaV) * p, pak);
  tA.addDirProd(-2 * m_V / q2 * (a3f - a0f) * p, q);

  /*Eq 11 Vector hadronic tensor*/
  EvtTensor4C tV = EvtComplex(0.0, (-2 * vf / m_BaV)) * asymProd(p, k);

  EvtTensor4C tVA = 0.5 * (C_V_L + C_V_R) * tV + 0.5 * (C_V_R - C_V_L) * tA;

  EvtComplex tS = 0.5 * (C_S_R - C_S_L) * 2 * m_V / (m_b + m_c) * a0f;

  pak *= T1;
  EvtVector4C qc1(q); qc1 *= c1;
  EvtTensor4C tt, tt5;
  EvtComplex sum1, sum2, TT1, TT2;
  for (int i = 0; i < 3; i++) {
    EvtVector4C eps = pvm->epsParent(i).conj(), epsTVA = tVA.cont1(eps);
    EvtComplex epsq = eps * q, epsqc2 = epsq * c2;

    /*Eq 14 Tensor component */
    for (int i2 = 0; i2 < 4; i2++) {
      for (int i3 = 0; i3 < 4; i3++) {
        tt.set(i2, i3, eps.get(i2) * (-pak.get(i3) + qc1.get(i3)) + p.get(i2)*k.get(i3)*epsqc2);
      }
    }
    if (lId == EM || lId == MUM || lId == TAUM) {
      TT1 = EvtComplex(0.0, 1.0) * C_T * (asymProd(z1, tt));
      TT2 = EvtComplex(0.0, 1.0) * C_T * (asymProd(z2, tt));
      sum1 = l1.cont(epsTVA) - a1.cont(epsTVA) - epsq * tS * p1 + TT1;
      sum2 = l2.cont(epsTVA) - a2.cont(epsTVA) - epsq * tS * p2 + TT2;
    } else if (lId == EP || lId == MUP || lId == TAUP) {
      TT1 = EvtComplex(0.0, -1.0) * conj(C_T) * (asymProd(z1, tt));
      TT2 = EvtComplex(0.0, -1.0) * conj(C_T) * (asymProd(z2, tt));
      sum1 = l1.cont(epsTVA) - a1.cont(epsTVA) - epsq * tS * p1 + TT1;
      sum2 = l2.cont(epsTVA) - a2.cont(epsTVA) - epsq * tS * p2 + TT2;
    }

    amp.vertex(i, 0, sum1);
    amp.vertex(i, 1, sum2);
  }
}

double EvtBToDstarlnuNP::CalcMaxProb()
{
  //This routine takes the arguements parent, meson, and lepton
  //number, and a form factor model, and returns a maximum
  //probability for this semileptonic form factor model.  A
  //brute force method is used.  The 2D cos theta lepton and
  //q2 phase space is probed.

  //Start by declaring a particle at rest.

  //It only makes sense to have a scalar parent.  For now.
  //This should be generalized later.

  EvtId pId = getParentId(), mId = getDaug(0), l1Id = getDaug(1), l2Id = getDaug(2);

  EvtScalarParticle* scalar_part;
  EvtParticle* root_part;

  scalar_part = new EvtScalarParticle;

  //cludge to avoid generating random numbers!
  scalar_part->noLifeTime();

  EvtVector4R p_init;

  p_init.set(EvtPDL::getMass(pId), 0.0, 0.0, 0.0);
  scalar_part->init(pId, p_init);
  root_part = (EvtParticle*)scalar_part;
  root_part->setDiagonalSpinDensity();

  EvtId listdaug[3];
  listdaug[0] = mId;
  listdaug[1] = l1Id;
  listdaug[2] = l2Id;

  EvtAmp amp;
  amp.init(pId, 3, listdaug);

  root_part->makeDaughters(3, listdaug);
  EvtParticle* daughter, *lep, *trino;
  daughter = root_part->getDaug(0);
  lep = root_part->getDaug(1);
  trino = root_part->getDaug(2);

  //cludge to avoid generating random numbers!
  daughter->noLifeTime();
  lep->noLifeTime();
  trino->noLifeTime();

  //Initial particle is unpolarized, well it is a scalar so it is
  //trivial
  EvtSpinDensity rho;
  rho.setDiag(root_part->getSpinStates());

  EvtVector4R p4meson, p4lepton, p4nu, p4w;
  double mass[3], m = root_part->mass(), maxfoundprob = 0.0, prob = -10.0;

  for (int massiter = 0; massiter < 3; massiter++) {
    mass[0] = EvtPDL::getMeanMass(mId);
    mass[1] = EvtPDL::getMeanMass(l1Id);
    mass[2] = EvtPDL::getMeanMass(l2Id);
    if (massiter == 1) {
      mass[0] = EvtPDL::getMinMass(mId);
    }
    if (massiter == 2) {
      mass[0] = EvtPDL::getMaxMass(mId);
      if ((mass[0] + mass[1] + mass[2]) > m)
        mass[0] = m - mass[1] - mass[2] - 0.00001;
    }

    double q2min = mass[1] * mass[1], q2max = (m - mass[0]) * (m - mass[0]);

    for (int imax = 40, i = 0; i < imax; i++) {
      double q2 = q2min + ((i + 0.5) * (q2max - q2min)) / imax;
      double erho = (m * m + mass[0] * mass[0] - q2) / (2.0 * m);
      double prho = sqrt(erho * erho - mass[0] * mass[0]);
      p4meson.set(erho, 0.0, 0.0, -1.0 * prho);
      p4w.set(m - erho, 0.0, 0.0, prho);

      //This is in the W rest frame
      double elepton = (q2 + mass[1] * mass[1]) / (2.0 * sqrt(q2));
      double plepton = sqrt(elepton * elepton - mass[1] * mass[1]);

      double probctl[3];
      for (int j = 0; j < 3; j++) {
        double costl = 0.99 * (j - 1.0);

        //These are in the W rest frame. Need to boost out into
        //the B frame.
        p4lepton.set(elepton, 0.0, plepton * sqrt(1.0 - costl * costl),
                     plepton * costl);
        p4nu.set(plepton, 0.0,
                 -1.0 * plepton * sqrt(1.0 - costl * costl),
                 -1.0 * plepton * costl);

        EvtVector4R boost((m - erho), 0.0, 0.0, 1.0 * prho);
        p4lepton = boostTo(p4lepton, boost);
        p4nu = boostTo(p4nu, boost);

        //Now initialize the daughters...

        daughter->init(mId, p4meson);
        lep->init(l1Id, p4lepton);
        trino->init(l2Id, p4nu);

        CalcAmp(root_part, amp);

        //Now find the probability at this q2 and cos theta lepton point
        //and compare to maxfoundprob.

        //Do a little magic to get the probability!!
        prob = rho.normalizedProb(amp.getSpinDensity());

        probctl[j] = prob;
      }

      //probclt contains prob at ctl=-1,0,1.
      //prob=a+b*ctl+c*ctl^2

      double a = probctl[1];
      double b = 0.5 * (probctl[2] - probctl[0]);
      double c = 0.5 * (probctl[2] + probctl[0]) - probctl[1];

      prob = probctl[0];
      if (probctl[1] > prob)
        prob = probctl[1];
      if (probctl[2] > prob)
        prob = probctl[2];

      if (fabs(c) > 1e-20) {
        double ctlx = -0.5 * b / c;
        if (fabs(ctlx) < 1.0) {
          double probtmp = a + b * ctlx + c * ctlx * ctlx;
          if (probtmp > prob)
            prob = probtmp;
        }
      }

      if (prob > maxfoundprob) {
        maxfoundprob = prob;
      }
    }
    if (EvtPDL::getWidth(mId) <= 0.0) {
      // if the particle is narrow don't bother with changing the mass.
      massiter = 4;
    }
  }
  root_part->deleteTree();

  maxfoundprob *= 1.1;
  return maxfoundprob;
}
