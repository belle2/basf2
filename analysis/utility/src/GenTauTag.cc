/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2020 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Sourav Patra, Vishal Bhardwaj                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <analysis/utility/GenTauTag.h>
#include <analysis/utility/CheckDecayUtils.h>

using namespace Belle2;

GenTauTag::GenTauTag() = default;

bool GenTauTag::PcheckDecay(std::vector<int> gp, int da1, int da2)
{
  int ndau = gp.size();
  if (ndau == 2) { m_nPhotos = 0; return CheckDecay::CheckDecay(gp, da1, da2);}
  else if (ndau == 3) { m_nPhotos = 1; return CheckDecay::CheckDecay(gp, da1, da2, 22);}
  else if (ndau == 4) { m_nPhotos = 2; return CheckDecay::CheckDecay(gp, da1, da2, 22, 22);}
  else if (ndau == 5) { m_nPhotos = 3; return CheckDecay::CheckDecay(gp, da1, da2, 22, 22, 22);}
  else if (ndau == 6) { m_nPhotos = 4; return CheckDecay::CheckDecay(gp, da1, da2, 22, 22, 22, 22);}
  else return false;
}

bool GenTauTag::PcheckDecay(std::vector<int> gp, int da1, int da2, int da3)
{
  int ndau = gp.size();
  if (ndau == 3) { m_nPhotos = 0; return CheckDecay::CheckDecay(gp, da1, da2, da3);}
  else if (ndau == 4) { m_nPhotos = 1; return CheckDecay::CheckDecay(gp, da1, da2, da3, 22);}
  else if (ndau == 5) { m_nPhotos = 2; return CheckDecay::CheckDecay(gp, da1, da2, da3, 22, 22);}
  else if (ndau == 6) { m_nPhotos = 3; return CheckDecay::CheckDecay(gp, da1, da2, da3, 22, 22, 22);}
  else if (ndau == 7) { m_nPhotos = 4; return CheckDecay::CheckDecay(gp, da1, da2, da3, 22, 22, 22, 22);}
  else return false;
}

bool GenTauTag::PcheckDecay(std::vector<int> gp, int da1, int da2, int da3, int da4)
{
  int ndau = gp.size();
  if (ndau == 4) { m_nPhotos = 0; return CheckDecay::CheckDecay(gp, da1, da2, da3, da4);}
  else if (ndau == 5) { m_nPhotos = 1; return CheckDecay::CheckDecay(gp, da1, da2, da3, da4, 22);}
  else if (ndau == 6) { m_nPhotos = 2; return CheckDecay::CheckDecay(gp, da1, da2, da3, da4, 22, 22);}
  else if (ndau == 7) { m_nPhotos = 3; return CheckDecay::CheckDecay(gp, da1, da2, da3, da4, 22, 22, 22);}
  else if (ndau == 8) { m_nPhotos = 4; return CheckDecay::CheckDecay(gp, da1, da2, da3, da4, 22, 22, 22, 22);}
  else return false;
}

bool GenTauTag::PcheckDecay(std::vector<int> gp, int da1, int da2, int da3, int da4, int da5)
{
  int ndau = gp.size();
  if (ndau == 5) { m_nPhotos = 0; return CheckDecay::CheckDecay(gp, da1, da2, da3, da4, da5);}
  else if (ndau == 6) { m_nPhotos = 1; return CheckDecay::CheckDecay(gp, da1, da2, da3, da4, da5, 22);}
  else if (ndau == 7) { m_nPhotos = 2; return CheckDecay::CheckDecay(gp, da1, da2, da3, da4, da5, 22, 22);}
  else if (ndau == 8) { m_nPhotos = 3; return CheckDecay::CheckDecay(gp, da1, da2, da3, da4, da5, 22, 22, 22);}
  else if (ndau == 9) { m_nPhotos = 4; return CheckDecay::CheckDecay(gp, da1, da2, da3, da4, da5, 22, 22, 22, 22);}
  else return false;
}

bool GenTauTag::PcheckDecay(std::vector<int> gp, int da1, int da2, int da3, int da4, int da5, int da6)
{
  int ndau = gp.size();
  if (ndau == 6) { m_nPhotos = 0;  return CheckDecay::CheckDecay(gp, da1, da2, da3, da4, da5, da6);}
  else if (ndau == 7) { m_nPhotos = 1; return CheckDecay::CheckDecay(gp, da1, da2, da3, da4, da5, da6, 22);}
  else if (ndau == 8) { m_nPhotos = 2; return CheckDecay::CheckDecay(gp, da1, da2, da3, da4, da5, da6, 22, 22);}
  else if (ndau == 9) { m_nPhotos = 3; return CheckDecay::CheckDecay(gp, da1, da2, da3, da4, da5, da6, 22, 22, 22);}
  else if (ndau == 10) { m_nPhotos = 4; return CheckDecay::CheckDecay(gp, da1, da2, da3, da4, da5, da6, 22, 22, 22, 22);}
  else return false;
}

bool GenTauTag::PcheckDecay(std::vector<int> gp, int da1, int da2, int da3, int da4, int da5, int da6, int da7)
{
  int ndau = gp.size();
  if (ndau == 7) { m_nPhotos = 0;  return CheckDecay::CheckDecay(gp, da1, da2, da3, da4, da5, da6, da7);}
  else if (ndau == 8) { m_nPhotos = 1; return CheckDecay::CheckDecay(gp, da1, da2, da3, da4, da5, da6, da7, 22);}
  else if (ndau == 9) { m_nPhotos = 2; return CheckDecay::CheckDecay(gp, da1, da2, da3, da4, da5, da6, da7, 22, 22);}
  else if (ndau == 10) { m_nPhotos = 3; return CheckDecay::CheckDecay(gp, da1, da2, da3, da4, da5, da6, da7, 22, 22, 22);}
  else if (ndau == 11) { m_nPhotos = 4; return CheckDecay::CheckDecay(gp, da1, da2, da3, da4, da5, da6, da7, 22, 22, 22, 22);}
  else return false;
}

bool GenTauTag::PcheckDecay(std::vector<int> gp, int da1, int da2, int da3, int da4, int da5, int da6, int da7, int da8)
{
  int ndau = gp.size();
  if (ndau == 8) { m_nPhotos = 0; return CheckDecay::CheckDecay(gp, da1, da2, da3, da4, da5, da6, da7, da8);}
  else if (ndau == 9) { m_nPhotos = 1; return CheckDecay::CheckDecay(gp, da1, da2, da3, da4, da5, da6, da7, da8, 22);}
  else if (ndau == 10) { m_nPhotos = 2; return CheckDecay::CheckDecay(gp, da1, da2, da3, da4, da5, da6, da7, da8, 22, 22);}
  else if (ndau == 11) { m_nPhotos = 3; return CheckDecay::CheckDecay(gp, da1, da2, da3, da4, da5, da6, da7, da8, 22, 22, 22);}
  else if (ndau == 12) { m_nPhotos = 4; return CheckDecay::CheckDecay(gp, da1, da2, da3, da4, da5, da6, da7, da8, 22, 22, 22, 22);}
  else return false;
}

//All the tag number for various decay mode is defined here

int GenTauTag::Mode_tau_plus(std::vector<int> genpart)
{
  if (GenTauTag::PcheckDecay(genpart, -11, 12, -16)) {
    return 1000 * m_nPhotos + 1;
  }//tau+ decays to e+ nu_e anti-nu_tau
  if (GenTauTag::PcheckDecay(genpart, -13, 14, -16)) {
    return 1000 * m_nPhotos + 2;
  }//tau+ decays to mu+ nu_mu anti-nu_tau
  if (GenTauTag::PcheckDecay(genpart, 211, -16)) {
    return 1000 * m_nPhotos + 3;
  }//tau+ decays to pi+ anti-nu_tau
  if (GenTauTag::PcheckDecay(genpart, 213, -16)) {
    return 1000 * m_nPhotos + 4;
  }//tau+ decays to rho+ anti-nu_tau
  if (GenTauTag::PcheckDecay(genpart, 20213, -16)) {
    return 1000 * m_nPhotos + 5;
  }//tau+ decays to a_1+ anti-nu_tau
  if (GenTauTag::PcheckDecay(genpart, 321, -16)) {
    return 1000 * m_nPhotos + 6;
  }//tau+ decays to K+ anti-nu_tau
  if (GenTauTag::PcheckDecay(genpart, 323, -16)) {
    return 1000 * m_nPhotos + 7;
  }//tau- decays to K*+ anti-nu_tau
  if (GenTauTag::PcheckDecay(genpart, -16, 211, -211, 211, 111)) {
    return 1000 * m_nPhotos + 8;
  }//tau- decays to anti-nu_tau pi+ pi- pi+ pi0
  if (GenTauTag::PcheckDecay(genpart, -16, 211, 111, 111, 111)) {
    return 1000 * m_nPhotos + 9;
  }//tau- decays to anti-nu_tau pi+ pi0 pi0 pi0
  if (GenTauTag::PcheckDecay(genpart, -16, 211, 211, -211, 111, 111)) {
    return 1000 * m_nPhotos + 10;
  }//tau- decays to anti-nu_tau pi+ pi+ pi- pi0 pi0
  if (GenTauTag::PcheckDecay(genpart, -16, 211, 211, 211, -211, -211)) {
    return 1000 * m_nPhotos + 11;
  }//tau- decays to anti-nu_tau pi+ pi+ pi+ pi- pi-
  if (GenTauTag::PcheckDecay(genpart, -16, 211, 211, 211, -211, -211, 111)) {
    return 1000 * m_nPhotos + 12;
  }//tau- decays to anti-nu_tau pi+ pi+ pi+ pi- pi- pi0
  if (GenTauTag::PcheckDecay(genpart, -16, 211, 211, -211, 111, 111, 111)) {
    return 1000 * m_nPhotos + 13;
  }//tau- decays to anti-nu_tau pi+ pi+ pi- pi0 pi0 pi0
  if (GenTauTag::PcheckDecay(genpart, -16, 321, 211, -321)) {
    return 1000 * m_nPhotos + 14;
  }//tau- decays to anti-nu_tau K+ pi+ K-
  if (GenTauTag::PcheckDecay(genpart, -16, -10313, +211, 10313)) {
    return 1000 * m_nPhotos + 15;
  }//tau- decays to anti-nu_tau anti-K_10 pi+ K_10
  if (GenTauTag::PcheckDecay(genpart, -16, 311, 211, -311)) {
    return 1000 * m_nPhotos + 15;
  }//tau- decays to anti-nu_tau K0 pi+ anti-K0
  if (GenTauTag::PcheckDecay(genpart, -16, 321, -10313, 111)) {
    return 1000 * m_nPhotos + 16;
  }//tau- decays to anti-nu_tau K+ anti-K_10 pi0
  if (GenTauTag::PcheckDecay(genpart, -16, 321, 311, 111)) {
    return 1000 * m_nPhotos + 16;
  }//tau- decays to anti-nu_tau K+ K0 pi0
  if (GenTauTag::PcheckDecay(genpart, -16, 321, -311, 111)) {
    return 1000 * m_nPhotos + 16;
  }//tau- decays to anti-nu_tau K+ anti-K0 pi0
  if (GenTauTag::PcheckDecay(genpart, -16, 321, 111, 111)) {
    return 1000 * m_nPhotos + 17;
  }//tau- decays to anti-nu_tau K+ pi0 pi0
  if (GenTauTag::PcheckDecay(genpart, -16, 321, -211, 211)) {
    return 1000 * m_nPhotos + 18;
  }//tau- decays to anti-nu_tau K+ pi- pi+
  if (GenTauTag::PcheckDecay(genpart, -16, 10313, 211, 111)) {
    return 1000 * m_nPhotos + 19;
  }//tau- decays to anti-nu_tau K_10 pi+ pi0
  if (GenTauTag::PcheckDecay(genpart, -16, 311, 211, 111)) {
    return 1000 * m_nPhotos + 19;
  }//tau- decays to anti-nu_tau K0 pi+ pi0
  if (GenTauTag::PcheckDecay(genpart, -16, -311, 211, 111)) {
    return 1000 * m_nPhotos + 19;
  }//tau- decays to anti-nu_tau anti-K0 pi+ pi0
  if (GenTauTag::PcheckDecay(genpart, -16, 221, 211, 111)) {
    return 1000 * m_nPhotos + 20;
  }//tau- decays to anti-nu_tau eta pi+ pi0
  if (GenTauTag::PcheckDecay(genpart, 211, 111, -16)) {
    return 1000 * m_nPhotos + 21;
  }//tau+ decays to pi+ pi0 anti-nu_tau
  if (GenTauTag::PcheckDecay(genpart, -16, 321, -10313)) {
    return 1000 * m_nPhotos + 22;
  }//tau- decays to anti-nu_tau K+ anti-K_10
  if (GenTauTag::PcheckDecay(genpart, -16, 321, 311)) {
    return 1000 * m_nPhotos + 22;
  }//tau- decays to anti-nu_tau K+ K0
  if (GenTauTag::PcheckDecay(genpart, -16, 321, -311)) {
    return 1000 * m_nPhotos + 22;
  }//tau- decays to anti-nu_tau K+ anti-K0
  if (GenTauTag::PcheckDecay(genpart, -16, 211, 111, 111, 111, 111)) {
    return 1000 * m_nPhotos + 23;
  }//tau- decays to anti-nu_tau pi+ pi0 pi0 pi0 pi0
  if (GenTauTag::PcheckDecay(genpart, -16, 211, 223, 111)) {
    return 1000 * m_nPhotos + 24;
  }//tau- decays to anti-nu_tau pi+ omega pi0
  if (GenTauTag::PcheckDecay(genpart, 221, 211, -211, 211, -16)) {
    return 1000 * m_nPhotos + 25;
  }//tau- decays to eta pi+ pi- pi+ anti-nu_tau
  if (GenTauTag::PcheckDecay(genpart, 221, 211, 111, 111, -16)) {
    return 1000 * m_nPhotos + 26;
  }//tau- decays to eta pi+ pi0 pi0 anti-nu_tau
  if (GenTauTag::PcheckDecay(genpart, 221, 321, -16)) {
    return 1000 * m_nPhotos + 27;
  }//tau- decays to eta K+ anti-nu_tau
  if (GenTauTag::PcheckDecay(genpart, 221, 323, -16)) {
    return 1000 * m_nPhotos + 28;
  }//tau- decays to eta K*+ anti-nu_tau
  if (GenTauTag::PcheckDecay(genpart, -16, 321, -211, 211, 111)) {
    return 1000 * m_nPhotos + 29;
  }//tau- decays to anti-nu_tau K+ pi- pi+ pi0
  if (GenTauTag::PcheckDecay(genpart, 321, 111, 111, 111, -16)) {
    return 1000 * m_nPhotos + 30;
  }//tau- decays to K+ pi0 pi0 pi0 anti-nu_tau
  if (GenTauTag::PcheckDecay(genpart, -16, 211, -211, 211, -10313)) {
    return 1000 * m_nPhotos + 31;
  }//tau- decays to anti-nu_tau pi+ pi- pi+ anti-K_10
  if (GenTauTag::PcheckDecay(genpart, -16, 211, -211, 211, 311)) {
    return 1000 * m_nPhotos + 31;
  }//tau- decays to anti-nu_tau pi+ pi- pi+ K0
  if (GenTauTag::PcheckDecay(genpart, -16, 211, -211, 211, -311)) {
    return 1000 * m_nPhotos + 31;
  }//tau- decays to anti-nu_tau pi+ pi- pi+ anti-K0
  if (GenTauTag::PcheckDecay(genpart, -16, 211, 111, 111, 10313)) {
    return 1000 * m_nPhotos + 32;
  }//tau- decays to anti-nu_tau pi+ pi0 pi0 K_10
  if (GenTauTag::PcheckDecay(genpart, -16, 211, 111, 111, 311)) {
    return 1000 * m_nPhotos + 32;
  }//tau- decays to anti-nu_tau pi+ pi0 pi0 K0
  if (GenTauTag::PcheckDecay(genpart, -16, 211, 111, 111, -311)) {
    return 1000 * m_nPhotos + 32;
  }//tau- decays to anti-nu_tau pi+ pi0 pi0 anti-K0
  if (GenTauTag::PcheckDecay(genpart, 321, -321, 211, 111, -16)) {
    return 1000 * m_nPhotos + 33;
  }//tau- decays to K+ K- pi+ pi0 anti-nu_tau
  if (GenTauTag::PcheckDecay(genpart, -16, -10313, 211, 111, 10313)) {
    return 1000 * m_nPhotos + 34;
  }//tau- decays to anti-nu_tau anti-K_10 pi+ pi0 K_10
  if (GenTauTag::PcheckDecay(genpart, -16, 311, 211, 111, -311)) {
    return 1000 * m_nPhotos + 34;
  }//tau- decays to anti-nu_tau K0 pi+ pi0 anti-K0
  if (GenTauTag::PcheckDecay(genpart, -16, 211, 223, -211, 211)) {
    return 1000 * m_nPhotos + 35;
  }//tau- decays to anti-nu_tau pi+ omega pi- pi+
  if (GenTauTag::PcheckDecay(genpart, -16, 211, 223, 111, 111)) {
    return 1000 * m_nPhotos + 36;
  }//tau- decays to anti-nu_tau pi+ omega pi0 pi0
  if (GenTauTag::PcheckDecay(genpart, -11, -11, 11, 12, -16)) {
    return 1000 * m_nPhotos + 37;
  }//tau- decays to e+ e+ e- nu_e anti-nu_tau
  if (GenTauTag::PcheckDecay(genpart, 20223, 211, -16)) {
    return 1000 * m_nPhotos + 38;
  }//tau- decays to f_1 pi+ anti-nu_tau
  if (GenTauTag::PcheckDecay(genpart, -16, 321, 223)) {
    return 1000 * m_nPhotos + 39;
  }//tau- decays to anti-nu_tau K+ omega
  if (GenTauTag::PcheckDecay(genpart, -16, -10313, 211, -211, 321)) {
    return 1000 * m_nPhotos + 40;
  }//tau- decays to anti-nu_tau anti-K_10 pi+ pi- K+
  if (GenTauTag::PcheckDecay(genpart, -16, 311, 211, -211, 321)) {
    return 1000 * m_nPhotos + 40;
  }//tau- decays to anti-nu_tau K0 pi+ pi- K+
  if (GenTauTag::PcheckDecay(genpart, -16, -311, 211, -211, 321)) {
    return 1000 * m_nPhotos + 40;
  }//tau- decays to anti-nu_tau anti-K0 pi+ pi- K+
  if (GenTauTag::PcheckDecay(genpart, -16, 321, 111, 111, -10313)) {
    return 1000 * m_nPhotos + 41;
  }//tau- decays to anti-nu_tau K+ pi0 pi0 anti-K_10
  if (GenTauTag::PcheckDecay(genpart, -16, 321, 111, 111, 311)) {
    return 1000 * m_nPhotos + 41;
  }//tau- decays to anti-nu_tau K+ pi0 pi0 K0
  if (GenTauTag::PcheckDecay(genpart, -16, 321, 111, 111, -311)) {
    return 1000 * m_nPhotos + 41;
  }//tau- decays to anti-nu_tau K+ pi0 pi0 anti-K0
  if (GenTauTag::PcheckDecay(genpart, -16, 211, 211, -321, 10313)) {
    return 1000 * m_nPhotos + 42;
  }//tau- decays to anti-nu_tau pi+ pi+ K- K_10
  if (GenTauTag::PcheckDecay(genpart, -16, 211, 211, -321, 311)) {
    return 1000 * m_nPhotos + 42;
  }//tau- decays to anti-nu_tau pi+ pi+ K- K0
  if (GenTauTag::PcheckDecay(genpart, -16, 211, 211, -321, -311)) {
    return 1000 * m_nPhotos + 42;
  }//tau- decays to anti-nu_tau pi+ pi+ K- anti-K0
  if (GenTauTag::PcheckDecay(genpart, -16, 211, 311)) {
    return 1000 * m_nPhotos + 43;
  }//tau- decays to anti-nu_tau pi+ K0
  if (GenTauTag::PcheckDecay(genpart, -16, 211, -311)) {
    return 1000 * m_nPhotos + 43;
  }//tau- decays to anti-nu_tau pi+ anti-K0
  if (GenTauTag::PcheckDecay(genpart, -16, 22, 211, 111)) {
    return 1000 * m_nPhotos + 44;
  }//tau- decays to anti-nu_tau gamma pi+ pi0
  if (GenTauTag::PcheckDecay(genpart, 111, 111, 211, -16)) {
    return 1000 * m_nPhotos + 45;
  }//tau- decays to pi0 pi0 pi+ anti-nu_tau
  if (GenTauTag::PcheckDecay(genpart, 211, 211, -211, -16)) {
    return 1000 * m_nPhotos + 46;
  }//tau- decays to pi+ pi+ pi- anti-nu_tau
  if (GenTauTag::PcheckDecay(genpart, -16, 321, -321, 321)) {
    return 1000 * m_nPhotos + 47;
  }//tau- decays to anti-nu_tau K+ K- K+
  if (GenTauTag::PcheckDecay(genpart, 313, 211, -16)) {
    return 1000 * m_nPhotos + 48;
  }//tau- decays to K*0 pi+ anti-nu_tau
  if (GenTauTag::PcheckDecay(genpart, -313, 321, -16)) {
    return 1000 * m_nPhotos + 49;
  }//tau- decays to anti-K*0 K+ anti-nu_tau
  if (GenTauTag::PcheckDecay(genpart, 333, 211, -16)) {
    return 1000 * m_nPhotos + 50;
  }//tau- decays to phi pi+ anti-nu_tau
  if (GenTauTag::PcheckDecay(genpart, 333, 321, -16)) {
    return 1000 * m_nPhotos + 51;
  }//tau- decays to phi K+ anti-nu_tau
  if (GenTauTag::PcheckDecay(genpart, -13, 14, -16, 22)) {
    return 1000 * m_nPhotos + 52;
  }//tau- decays to mu+ nu_mu anti-nu_tau gamma
  if (GenTauTag::PcheckDecay(genpart, -11, 12, -16, 22)) {
    return 1000 * m_nPhotos + 53;
  }//tau- decays to e+ nu_e anti-nu_tau gamma
  if (GenTauTag::PcheckDecay(genpart, 321, 111, -16)) {
    return 1000 * m_nPhotos + 54;
  }//tau- decays to K+ pi0 anti-nu_tau
  if (GenTauTag::PcheckDecay(genpart, 311, 213, -16)) {
    return 1000 * m_nPhotos + 55;
  }//tau- decays to K0 rho+ anti-nu_tau
  if (GenTauTag::PcheckDecay(genpart, -311, 213, -16)) {
    return 1000 * m_nPhotos + 55;
  }//tau- decays to anti-K0 rho+ anti-nu_tau
  if (GenTauTag::PcheckDecay(genpart, 10323, -16)) {
    return 1000 * m_nPhotos + 56;
  }//tau- decays to K_1+ anti-nu_tau
  if (GenTauTag::PcheckDecay(genpart, 20323, -16)) {
    return 1000 * m_nPhotos + 57;
  }//tau- decays to K'_1+ anti-nu_tau
  if (GenTauTag::PcheckDecay(genpart, 100323, -16)) {
    return 1000 * m_nPhotos + 58;
  }//tau- decays to K'*+ anti-nu_tau
  if (GenTauTag::PcheckDecay(genpart, 221, 321, 111, -16)) {
    return 1000 * m_nPhotos + 59;
  }//tau- decays to eta K+ pi0 anti-nu_tau
  if (GenTauTag::PcheckDecay(genpart, 221, 311, 211, -16)) {
    return 1000 * m_nPhotos + 60;
  }//tau- decays to eta K0 pi+ anti-nu_tau
  if (GenTauTag::PcheckDecay(genpart, 221, -311, 211, -16)) {
    return 1000 * m_nPhotos + 60;
  }//tau- decays to eta anti-K0 pi+ anti-nu_tau
  if (GenTauTag::PcheckDecay(genpart, -16, 310, 310, 211)) {
    return 1000 * m_nPhotos + 20015;
  }//tau- decays to anti-nu_tau K_S0 K_S0 pi+
  if (GenTauTag::PcheckDecay(genpart, -16, 310, 130, 211)) {
    return 1000 * m_nPhotos + 110015;
  }//tau- decays to anti-nu_tau K_S0 K_L0 pi+
  if (GenTauTag::PcheckDecay(genpart, -16, 130, 130, 211)) {
    return 1000 * m_nPhotos + 200015;
  }//tau- decays to anti-nu_tau K_L0 K_L0 pi+
  if (GenTauTag::PcheckDecay(genpart, -16, 310, 321, 111)) {
    return 1000 * m_nPhotos + 10016;
  }//tau- decays to anti-nu_tau K_S0 K+ pi0
  if (GenTauTag::PcheckDecay(genpart, -16, 130, 321, 111)) {
    return 1000 * m_nPhotos + 100016;
  }//tau- decays to anti-nu_tau K_L0 K+ pi0
  if (GenTauTag::PcheckDecay(genpart, -16, 310, 211, 111)) {
    return 1000 * m_nPhotos + 10019;
  }//tau- decays to anti-nu_tau K_S0 pi+ pi0
  if (GenTauTag::PcheckDecay(genpart, -16, 130, 211, 111)) {
    return 1000 * m_nPhotos + 100019;
  }//tau- decays to anti-nu_tau K_L0 pi+ pi0
  if (GenTauTag::PcheckDecay(genpart, -16, 310, 321)) {
    return 1000 * m_nPhotos + 10022;
  }//tau- decays to anti-nu_tau K_S0 K+
  if (GenTauTag::PcheckDecay(genpart, -16, 130, 321)) {
    return 1000 * m_nPhotos + 100022;
  }//tau- decays to anti-nu_tau K_L0 K+
  if (GenTauTag::PcheckDecay(genpart, -16, 310, 211, -211, 211)) {
    return 1000 * m_nPhotos + 10031;
  }//tau- decays to anti-nu_tau K_S0 pi+ pi- pi+
  if (GenTauTag::PcheckDecay(genpart, -16, 130, 211, -211, 211)) {
    return 1000 * m_nPhotos + 100031;
  }//tau- decays to anti-nu_tau K_L0 pi+ pi- pi+
  if (GenTauTag::PcheckDecay(genpart, -16, 310, 211, 111, 111)) {
    return 1000 * m_nPhotos + 10032;
  }//tau- decays to anti-nu_tau K_S0 pi+ pi0 pi0
  if (GenTauTag::PcheckDecay(genpart, -16, 130, 211, 111, 111)) {
    return 1000 * m_nPhotos + 100032;
  }//tau- decays to anti-nu_tau K_L0 pi+ pi0 pi0
  if (GenTauTag::PcheckDecay(genpart, -16, 310, 310, 211, 111)) {
    return 1000 * m_nPhotos + 20034;
  }//tau- decays to anti-nu_tau K_S0 K_S0 pi+ pi0
  if (GenTauTag::PcheckDecay(genpart, -16, 310, 130, 211, 111)) {
    return 1000 * m_nPhotos + 110034;
  }//tau- decays to anti-nu_tau K_S0 K_L0 pi+ pi0
  if (GenTauTag::PcheckDecay(genpart, -16, 130, 130, 211, 111)) {
    return 1000 * m_nPhotos + 200034;
  }//tau- decays to anti-nu_tau K_L0 K_L0 pi+ pi0
  if (GenTauTag::PcheckDecay(genpart, -16, 310, 211, -211, 321)) {
    return 1000 * m_nPhotos + 10040;
  }//tau- decays to anti-nu_tau K_S0 pi+ pi- K+
  if (GenTauTag::PcheckDecay(genpart, -16, 130, 211, -211, 321)) {
    return 1000 * m_nPhotos + 100040;
  }//tau- decays to anti-nu_tau K_L0 pi+ pi- K+
  if (GenTauTag::PcheckDecay(genpart, -16, 321, 111, 111, 310)) {
    return 1000 * m_nPhotos + 10041;
  }//tau- decays to anti-nu_tau K+ pi0 pi0 K_S0
  if (GenTauTag::PcheckDecay(genpart, -16, 321, 111, 111, 130)) {
    return 1000 * m_nPhotos + 100041;
  }//tau- decays to anti-nu_tau K+ pi0 pi0 K_L0
  if (GenTauTag::PcheckDecay(genpart, -16, 211, 211, -321, 311)) {
    return 1000 * m_nPhotos + 10042;
  }//tau- decays to anti-nu_tau pi+ pi+ K- K_S0
  if (GenTauTag::PcheckDecay(genpart, -16, 211, 211, -321, 130)) {
    return 1000 * m_nPhotos + 100042;
  }//tau- decays to anti-nu_tau pi+ pi+ K- K_L0
  return 0;
}// not yet defined tau+ decay


int GenTauTag::Mode_tau_minus(std::vector<int> genpart)
{
  if (GenTauTag::PcheckDecay(genpart, 11, -12, 16)) {
    return 1000 * m_nPhotos + 1;
  }//tau- decays to e- anti-nu_e nu_tau
  if (GenTauTag::PcheckDecay(genpart, 13, -14, 16)) {
    return 1000 * m_nPhotos + 2;
  }//tau- decays to mu- anti-nu_mu nu_tau
  if (GenTauTag::PcheckDecay(genpart, -211, 16)) {
    return 1000 * m_nPhotos + 3;
  }//tau- decays to pi- nu_tau
  if (GenTauTag::PcheckDecay(genpart, -213, 16)) {
    return 1000 * m_nPhotos + 4;
  }//tau- decays to rho- nu_tau
  if (GenTauTag::PcheckDecay(genpart, -20213, 16)) {
    return 1000 * m_nPhotos + 5;
  }//tau- decays to a_1- nu_tau
  if (GenTauTag::PcheckDecay(genpart, -321, 16)) {
    return 1000 * m_nPhotos + 6;
  }//tau- decays to K- nu_tau
  if (GenTauTag::PcheckDecay(genpart, -323, 16)) {
    return 1000 * m_nPhotos + 7;
  }//tau- decays to K*- nu_tau
  if (GenTauTag::PcheckDecay(genpart, 16, -211, 211, -211, 111)) {
    return 1000 * m_nPhotos + 8;
  }//tau- decays to nu_tau pi- pi+ pi- pi0
  if (GenTauTag::PcheckDecay(genpart, 16, -211, 111, 111, 111)) {
    return 1000 * m_nPhotos + 9;
  }//tau- decays to nu_tau pi- pi0 pi0 pi0
  if (GenTauTag::PcheckDecay(genpart, 16, -211, -211, 211, 111, 111)) {
    return 1000 * m_nPhotos + 10;
  }//tau- decays to nu_tau pi- pi- pi+ pi0 pi0
  if (GenTauTag::PcheckDecay(genpart, 16, -211, -211, -211, 211, 211)) {
    return 1000 * m_nPhotos + 11;
  }//tau- decays to nu_tau pi- pi- pi- pi+ pi+
  if (GenTauTag::PcheckDecay(genpart, 16, -211, -211, -211, 211, 211, 111)) {
    return 1000 * m_nPhotos + 12;
  }//tau- decays to nu_tau pi- pi- pi- pi+ pi+ pi0
  if (GenTauTag::PcheckDecay(genpart, 16, -211, -211, 211, 111, 111, 111)) {
    return 1000 * m_nPhotos + 13;
  }//tau- decays to nu_tau pi- pi- pi+ pi0 pi0 pi0
  if (GenTauTag::PcheckDecay(genpart, 16, -321, -211, 321)) {
    return 1000 * m_nPhotos + 14;
  }//tau- decays to nu_tau K- pi- K+
  if (GenTauTag::PcheckDecay(genpart, 16, -10313, -211, 10313)) {
    return 1000 * m_nPhotos + 15;
  }//tau- decays to nu_tau anti-K_10 pi- K_10
  if (GenTauTag::PcheckDecay(genpart, 16, 311, -211, -311)) {
    return 1000 * m_nPhotos + 15;
  }//tau- decays to nu_tau K0 pi- anti-K0
  if (GenTauTag::PcheckDecay(genpart, 16, -321, 10313, 111)) {
    return 1000 * m_nPhotos + 16;
  }//tau- decays to nu_tau K- K_10 pi0
  if (GenTauTag::PcheckDecay(genpart, 16, -321, 311, 111)) {
    return 1000 * m_nPhotos + 16;
  }//tau- decays to nu_tau K- K0 pi0
  if (GenTauTag::PcheckDecay(genpart, 16, -321, -311, 111)) {
    return 1000 * m_nPhotos + 16;
  }//tau- decays to nu_tau K- anti-K0 pi0
  if (GenTauTag::PcheckDecay(genpart, 16, -321, 111, 111)) {
    return 1000 * m_nPhotos + 17;
  }//tau- decays to nu_tau K- pi0 pi0
  if (GenTauTag::PcheckDecay(genpart, 16, -321, 211, -211)) {
    return 1000 * m_nPhotos + 18;
  }//tau- decays to nu_tau K- pi+ pi-
  if (GenTauTag::PcheckDecay(genpart, 16, -10313, -211, 111)) {
    return 1000 * m_nPhotos + 19;
  }//tau- decays to nu_tau anti-K_10 pi- pi0
  if (GenTauTag::PcheckDecay(genpart, 16, 311, -211, 111)) {
    return 1000 * m_nPhotos + 19;
  }//tau- decays to nu_tau K0 pi- pi0
  if (GenTauTag::PcheckDecay(genpart, 16, -311, -211, 111)) {
    return 1000 * m_nPhotos + 19;
  }//tau- decays to nu_tau anti-K0 pi- pi0
  if (GenTauTag::PcheckDecay(genpart, 16, 221, -211, 111)) {
    return 1000 * m_nPhotos + 20;
  }//tau- decays to nu_tau eta pi- pi0
  if (GenTauTag::PcheckDecay(genpart, -211, 111, 16)) {
    return 1000 * m_nPhotos + 21;
  }//tau+ decays to pi- pi0 nu_tau
  if (GenTauTag::PcheckDecay(genpart, 16, -321, 10313)) {
    return 1000 * m_nPhotos + 22;
  }//tau- decays to nu_tau K- K_10
  if (GenTauTag::PcheckDecay(genpart, 16, -321, 311)) {
    return 1000 * m_nPhotos + 22;
  }//tau- decays to nu_tau K- K0
  if (GenTauTag::PcheckDecay(genpart, 16, -321, -311)) {
    return 1000 * m_nPhotos + 22;
  }//tau- decays to nu_tau K- anti-K0
  if (GenTauTag::PcheckDecay(genpart, 16, -211, 111, 111, 111, 111)) {
    return 1000 * m_nPhotos + 23;
  }//tau- decays to nu_tau pi- pi0 pi0 pi0 pi0
  if (GenTauTag::PcheckDecay(genpart, 16, -211, 223, 111)) {
    return 1000 * m_nPhotos + 24;
  }//tau- decays to nu_tau pi- omega pi0
  if (GenTauTag::PcheckDecay(genpart, 221, -211, 211, -211, 16)) {
    return 1000 * m_nPhotos + 25;
  }//tau- decays to eta pi- pi+ pi- nu_tau
  if (GenTauTag::PcheckDecay(genpart, 221, -211, 111, 111, 16)) {
    return 1000 * m_nPhotos + 26;
  }//tau- decays to eta pi- pi0 pi0 nu_tau
  if (GenTauTag::PcheckDecay(genpart, 221, -321, 16)) {
    return 1000 * m_nPhotos + 27;
  }//tau- decays to eta K- nu_tau
  if (GenTauTag::PcheckDecay(genpart, 221, -323, 16)) {
    return 1000 * m_nPhotos + 28;
  }//tau- decays to eta K*- nu_tau
  if (GenTauTag::PcheckDecay(genpart, 16, -321, 211, -211, 111)) {
    return 1000 * m_nPhotos + 29;
  }//tau- decays to nu_tau K- pi+ pi- pi0
  if (GenTauTag::PcheckDecay(genpart, -321, 111, 111, 111, 16)) {
    return 1000 * m_nPhotos + 30;
  }//tau- decays to K- pi0 pi0 pi0 nu_tau
  if (GenTauTag::PcheckDecay(genpart, 16, -211, 211, -211, 10313)) {
    return 1000 * m_nPhotos + 31;
  }//tau- decays to nu_tau pi- pi+ pi- K_10
  if (GenTauTag::PcheckDecay(genpart, 16, -211, 211, -211, 311)) {
    return 1000 * m_nPhotos + 31;
  }//tau- decays to nu_tau pi- pi+ pi- K0
  if (GenTauTag::PcheckDecay(genpart, 16, -211, 211, -211, -311)) {
    return 1000 * m_nPhotos + 31;
  }//tau- decays to nu_tau pi- pi+ pi- anti-K0
  if (GenTauTag::PcheckDecay(genpart, 16, -211, 111, 111, -10313)) {
    return 1000 * m_nPhotos + 32;
  }//tau- decays to nu_tau pi- pi0 pi0 anti-K_10
  if (GenTauTag::PcheckDecay(genpart, 16, -211, 111, 111, 311)) {
    return 1000 * m_nPhotos + 32;
  }//tau- decays to nu_tau pi- pi0 pi0 K0
  if (GenTauTag::PcheckDecay(genpart, 16, -211, 111, 111, -311)) {
    return 1000 * m_nPhotos + 32;
  }//tau- decays to nu_tau pi- pi0 pi0 anti-K0
  if (GenTauTag::PcheckDecay(genpart, -321, 321, -211, 111, 16)) {
    return 1000 * m_nPhotos + 33;
  }//tau- decays to K- K+ pi- pi0 nu_tau
  if (GenTauTag::PcheckDecay(genpart, 16, -10313, -211, 111, 10313)) {
    return 1000 * m_nPhotos + 34;
  }//tau- decays to nu_tau anti-K_10 pi- pi0 K_10
  if (GenTauTag::PcheckDecay(genpart, 16, 311, -211, 111, -311)) {
    return 1000 * m_nPhotos + 34;
  }//tau- decays to nu_tau K0 pi- pi0 anti-K0
  if (GenTauTag::PcheckDecay(genpart, 16, -211, 223, 211, -211)) {
    return 1000 * m_nPhotos + 35;
  }//tau- decays to nu_tau pi- omega pi+ pi-
  if (GenTauTag::PcheckDecay(genpart, 16, -211, 223, 111, 111)) {
    return 1000 * m_nPhotos + 36;
  }//tau- decays to nu_tau pi- omega pi0 pi0
  if (GenTauTag::PcheckDecay(genpart, 11, 11, -11, -12, 16)) {
    return 1000 * m_nPhotos + 37;
  }//tau- decays to e- e- e+ anti-nu_e nu_tau
  if (GenTauTag::PcheckDecay(genpart, 20223, -211, 16)) {
    return 1000 * m_nPhotos + 38;
  }//tau- decays to f_1 pi- nu_tau
  if (GenTauTag::PcheckDecay(genpart, 16, -321, 223)) {
    return 1000 * m_nPhotos + 39;
  }//tau- decays to nu_tau K- omega
  if (GenTauTag::PcheckDecay(genpart, 16, 10313, -211, 211, -321)) {
    return 1000 * m_nPhotos + 40;
  }//tau- decays to nu_tau K_10 pi- pi+ K-
  if (GenTauTag::PcheckDecay(genpart, 16, 311, -211, 211, -321)) {
    return 1000 * m_nPhotos + 40;
  }//tau- decays to nu_tau K0 pi- pi+ K-
  if (GenTauTag::PcheckDecay(genpart, 16, -311, -211, 211, -321)) {
    return 1000 * m_nPhotos + 40;
  }//tau- decays to nu_tau anti-K0 pi- pi+ K-
  if (GenTauTag::PcheckDecay(genpart, 16, -321, 111, 111, 10313)) {
    return 1000 * m_nPhotos + 41;
  }//tau- decays to nu_tau K- pi0 pi0 K_10
  if (GenTauTag::PcheckDecay(genpart, 16, -321, 111, 111, 311)) {
    return 1000 * m_nPhotos + 41;
  }//tau- decays to nu_tau K- pi0 pi0 K0
  if (GenTauTag::PcheckDecay(genpart, 16, -321, 111, 111, -311)) {
    return 1000 * m_nPhotos + 41;
  }//tau- decays to nu_tau K- pi0 pi0 anti-K0
  if (GenTauTag::PcheckDecay(genpart, 16, -211, -211, 321, -10313)) {
    return 1000 * m_nPhotos + 42;
  }//tau- decays to nu_tau pi- pi- K+ anti-K_10
  if (GenTauTag::PcheckDecay(genpart, 16, -211, -211, 321, 311)) {
    return 1000 * m_nPhotos + 42;
  }//tau- decays to nu_tau pi- pi- K+ K0
  if (GenTauTag::PcheckDecay(genpart, 16, -211, -211, 321, -311)) {
    return 1000 * m_nPhotos + 42;
  }//tau- decays to nu_tau pi- pi- K+ anti-K0
  if (GenTauTag::PcheckDecay(genpart, 16, -211, 311)) {
    return 1000 * m_nPhotos + 43;
  }//tau- decays to nu_tau pi- K0
  if (GenTauTag::PcheckDecay(genpart, 16, -211, -311)) {
    return 1000 * m_nPhotos + 43;
  }//tau- decays to nu_tau pi- anti-K0
  if (GenTauTag::PcheckDecay(genpart, 16, 22, -211, 111)) {
    return 1000 * m_nPhotos + 44;
  }//tau- decays to nu_tau gamma pi- pi0
  if (GenTauTag::PcheckDecay(genpart, 111, 111, -211, 16)) {
    return 1000 * m_nPhotos + 45;
  }//tau- decays to pi0 pi0 pi- nu_tau
  if (GenTauTag::PcheckDecay(genpart, -211, -211, 211, 16)) {
    return 1000 * m_nPhotos + 46;
  }//tau- decays to pi- pi- pi+ nu_tau
  if (GenTauTag::PcheckDecay(genpart, 16, -321, 321, -321)) {
    return 1000 * m_nPhotos + 47;
  }//tau- decays to nu_tau K- K+ K-
  if (GenTauTag::PcheckDecay(genpart, -313, -211, 16)) {
    return 1000 * m_nPhotos + 48;
  }//tau- decays to anti-K*0 pi- nu_tau
  if (GenTauTag::PcheckDecay(genpart, 313, -321, 16)) {
    return 1000 * m_nPhotos + 49;
  }//tau- decays to K*0 K- nu_tau
  if (GenTauTag::PcheckDecay(genpart, 333, -211, 16)) {
    return 1000 * m_nPhotos + 50;
  }//tau- decays to phi pi- nu_tau
  if (GenTauTag::PcheckDecay(genpart, 333, -321, 16)) {
    return 1000 * m_nPhotos + 51;
  }//tau- decays to phi K- nu_tau
  if (GenTauTag::PcheckDecay(genpart, 13, -14, 16, 22)) {
    return 1000 * m_nPhotos + 52;
  }//tau- decays to mu- anti-nu_mu nu_tau gamma
  if (GenTauTag::PcheckDecay(genpart, 11, -12, 16, 22)) {
    return 1000 * m_nPhotos + 53;
  }//tau- decays to e- anti-nu_e nu_tau gamma
  if (GenTauTag::PcheckDecay(genpart, -321, 111, 16)) {
    return 1000 * m_nPhotos + 54;
  }//tau- decays to K- pi0 nu_tau
  if (GenTauTag::PcheckDecay(genpart, 311, -213, 16)) {
    return 1000 * m_nPhotos + 55;
  }//tau- decays to K0 rho- nu_tau
  if (GenTauTag::PcheckDecay(genpart, -311, -213, 16)) {
    return 1000 * m_nPhotos + 55;
  }//tau- decays to anti-K0 rho- nu_tau
  if (GenTauTag::PcheckDecay(genpart, -10323, 16)) {
    return 1000 * m_nPhotos + 56;
  }//tau- decays to K_1- nu_tau
  if (GenTauTag::PcheckDecay(genpart, -20323, 16)) {
    return 1000 * m_nPhotos + 57;
  }//tau- decays to K'_1- nu_tau
  if (GenTauTag::PcheckDecay(genpart, -100323, 16)) {
    return 1000 * m_nPhotos + 58;
  }//tau- decays to K'*- nu_tau
  if (GenTauTag::PcheckDecay(genpart, 221, -321, 111, 16)) {
    return 1000 * m_nPhotos + 59;
  }//tau- decays to eta K- pi0 nu_tau
  if (GenTauTag::PcheckDecay(genpart, 221, 311, -211, 16)) {
    return 1000 * m_nPhotos + 60;
  }//tau- decays to eta K0 pi- nu_tau
  if (GenTauTag::PcheckDecay(genpart, 221, -311, -211, 16)) {
    return 1000 * m_nPhotos + 60;
  }//tau- decays to eta anti-K0 pi- nu_tau
  if (GenTauTag::PcheckDecay(genpart, 16, 310, 310, -211)) {
    return 1000 * m_nPhotos + 20015;
  }//tau- decays to nu_tau K_S0 K_S0 pi-
  if (GenTauTag::PcheckDecay(genpart, 16, 310, 130, -211)) {
    return 1000 * m_nPhotos + 110015;
  }//tau- decays to nu_tau K_S0 K_L0 pi-
  if (GenTauTag::PcheckDecay(genpart, 16, 130, 130, -211)) {
    return 1000 * m_nPhotos + 200015;
  }//tau- decays to nu_tau K_L0 K_L0 pi-
  if (GenTauTag::PcheckDecay(genpart, 16, 310, -321, 111)) {
    return 1000 * m_nPhotos + 10016;
  }//tau- decays to nu_tau K_S0 K- pi0
  if (GenTauTag::PcheckDecay(genpart, 16, 130, -321, 111)) {
    return 1000 * m_nPhotos + 100016;
  }//tau- decays to nu_tau K_L0 K- pi0
  if (GenTauTag::PcheckDecay(genpart, 16, 310, -211, 111)) {
    return 1000 * m_nPhotos + 10019;
  }//tau- decays to nu_tau K_S0 pi- pi0
  if (GenTauTag::PcheckDecay(genpart, 16, 130, -211, 111)) {
    return 1000 * m_nPhotos + 100019;
  }//tau- decays to nu_tau K_L0 pi- pi0
  if (GenTauTag::PcheckDecay(genpart, 16, 310, -321)) {
    return 1000 * m_nPhotos + 10022;
  }//tau- decays to nu_tau K_S0 K-
  if (GenTauTag::PcheckDecay(genpart, 16, 130, -321)) {
    return 1000 * m_nPhotos + 100022;
  }//tau- decays to nu_tau K_L0 K-
  if (GenTauTag::PcheckDecay(genpart, 16, 310, -211, 211, -211)) {
    return 1000 * m_nPhotos + 10031;
  }//tau- decays to nu_tau K_S0 pi- pi+ pi-
  if (GenTauTag::PcheckDecay(genpart, 16, 130, -211, 211, -211)) {
    return 1000 * m_nPhotos + 100031;
  }//tau- decays to nu_tau K_L0 pi- pi+ pi-
  if (GenTauTag::PcheckDecay(genpart, 16, 310, -211, 111, 111)) {
    return 1000 * m_nPhotos + 10032;
  }//tau- decays to nu_tau K_S0 pi- pi0 pi0
  if (GenTauTag::PcheckDecay(genpart, 16, 130, -211, 111, 111)) {
    return 1000 * m_nPhotos + 100032;
  }//tau- decays to nu_tau K_L0 pi- pi0 pi0
  if (GenTauTag::PcheckDecay(genpart, 16, 310, 310, -211, 111)) {
    return 1000 * m_nPhotos + 20034;
  }//tau- decays to nu_tau K_S0 K_S0 pi- pi0
  if (GenTauTag::PcheckDecay(genpart, 16, 310, 130, -211, 111)) {
    return 1000 * m_nPhotos + 110034;
  }//tau- decays to nu_tau K_S0 K_L0 pi- pi0
  if (GenTauTag::PcheckDecay(genpart, 16, 130, 130, -211, 111)) {
    return 1000 * m_nPhotos + 200034;
  }//tau- decays to nu_tau K_L0 K_L0 pi- pi0
  if (GenTauTag::PcheckDecay(genpart, 16, 310, -211, 211, -321)) {
    return 1000 * m_nPhotos + 10040;
  }//tau- decays to nu_tau K_S0 pi- pi+ K-
  if (GenTauTag::PcheckDecay(genpart, 16, 130, -211, 211, -321)) {
    return 1000 * m_nPhotos + 100040;
  }//tau- decays to nu_tau K_L0 pi- pi+ K-
  if (GenTauTag::PcheckDecay(genpart, 16, -321, 111, 111, 310)) {
    return 1000 * m_nPhotos + 10041;
  }//tau- decays to nu_tau K- pi0 pi0 K_S0
  if (GenTauTag::PcheckDecay(genpart, 16, -321, 111, 111, 130)) {
    return 1000 * m_nPhotos + 100041;
  }//tau- decays to nu_tau K- pi0 pi0 K_L0
  if (GenTauTag::PcheckDecay(genpart, 16, -211, -211, 321, 310)) {
    return 1000 * m_nPhotos + 10042;
  }//tau- decays to nu_tau pi- pi- K+ K_S0
  if (GenTauTag::PcheckDecay(genpart, 16, -211, -211, 321, 130)) {
    return 1000 * m_nPhotos + 100042;
  }//tau- decays to nu_tau pi- pi- K+ K_L0
  return 0;
}// not yet defined tau- decay
