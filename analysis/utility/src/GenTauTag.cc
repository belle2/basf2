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
    return +1 * (100000 * m_nPhotos + 1001);
  }//tau+ decays to e+ nu_e anti-nu_tau
  if (GenTauTag::PcheckDecay(genpart, -13, 14, -16)) {
    return +1 * (100000 * m_nPhotos + 1002);
  }//tau+ decays to mu+ nu_mu anti-nu_tau
  if (GenTauTag::PcheckDecay(genpart, 211, -16)) {
    return +1 * (100000 * m_nPhotos + 1003);
  }//tau+ decays to pi+ anti-nu_tau
  if (GenTauTag::PcheckDecay(genpart, 321, -16)) {
    return +1 * (100000 * m_nPhotos + 1004);
  }//tau+ decays to K+ anti-nu_tau
  if (GenTauTag::PcheckDecay(genpart, 211, 111, -16)) {
    return +1 * (100000 * m_nPhotos + 1005);
  }//tau+ decays to pi+ pi0 anti-nu_tau
  if (GenTauTag::PcheckDecay(genpart, -16, 22, 211, 111)) {
    return +1 * (100000 * m_nPhotos + 1006);
  }//tau+ decays to anti-nu_tau gamma pi+ pi0
  if (GenTauTag::PcheckDecay(genpart, 111, 111, 211, -16)) {
    return +1 * (100000 * m_nPhotos + 1007);
  }//tau+ decays to pi0 pi0 pi+ anti-nu_tau
  if (GenTauTag::PcheckDecay(genpart, -16, 321, 111, 111)) {
    return +1 * (100000 * m_nPhotos + 1008);
  }//tau+ decays to anti-nu_tau K+ pi0 pi0
  if (GenTauTag::PcheckDecay(genpart, -16, 211, 111, 111, 111)) {
    return +1 * (100000 * m_nPhotos + 1009);
  }//tau+ decays to anti-nu_tau pi+ pi0 pi0 pi0
  if (GenTauTag::PcheckDecay(genpart, -16, 211, 111, 111, 111, 111)) {
    return +1 * (100000 * m_nPhotos + 1010);
  }//tau+ decays to anti-nu_tau pi+ pi0 pi0 pi0 pi0
  if (GenTauTag::PcheckDecay(genpart, -16, 211, 311)) {
    return +1 * (100000 * m_nPhotos + 1011);
  }//tau+ decays to anti-nu_tau pi+ K0
  if (GenTauTag::PcheckDecay(genpart, -16, 321, -311)) {
    return +1 * (100000 * m_nPhotos + 1012);
  }//tau+ decays to anti-nu_tau K+ anti-K0
  if (GenTauTag::PcheckDecay(genpart, -16, -311, 211, 311)) {
    return +1 * (100000 * m_nPhotos + 1013);
  }//tau+ decays to anti-nu_tau anti-K0 pi+ K0
  if (GenTauTag::PcheckDecay(genpart, -16, 321, 111, -311)) {
    return +1 * (100000 * m_nPhotos + 1014);
  }//tau+ decays to anti-nu_tau K+ pi0 anti-K0
  if (GenTauTag::PcheckDecay(genpart, -16, 211, 311, 111)) {
    return +1 * (100000 * m_nPhotos + 1015);
  }//tau+ decays to anti-nu_tau pi+ K0 pi0
  if (GenTauTag::PcheckDecay(genpart, 211, 211, -211, -16)) {
    return +1 * (100000 * m_nPhotos + 1016);
  }//tau+ decays to pi+ pi+ pi- anti-nu_tau
  if (GenTauTag::PcheckDecay(genpart, -16, 211, -211, 211, 111)) {
    return +1 * (100000 * m_nPhotos + 1017);
  }//tau+ decays to anti-nu_tau pi+ pi- pi+ pi0
  if (GenTauTag::PcheckDecay(genpart, -16, 211, 211, -211, 111, 111)) {
    return +1 * (100000 * m_nPhotos + 1018);
  }//tau+ decays to anti-nu_tau pi+ pi+ pi- pi0 pi0
  if (GenTauTag::PcheckDecay(genpart, -16, 211, 211, -211, 111, 111, 111)) {
    return +1 * (100000 * m_nPhotos + 1019);
  }//tau+ decays to anti-nu_tau pi+ pi+ pi- pi0 pi0 pi0
  if (GenTauTag::PcheckDecay(genpart, -16, 321, -211, 211)) {
    return +1 * (100000 * m_nPhotos + 1020);
  }//tau+ decays to anti-nu_tau K+ pi- pi+
  if (GenTauTag::PcheckDecay(genpart, -16, 321, -211, 211, 111)) {
    return +1 * (100000 * m_nPhotos + 1021);
  }//tau+ decays to anti-nu_tau K+ pi- pi+ pi0
  if (GenTauTag::PcheckDecay(genpart, -16, 321, 211, -321)) {
    return +1 * (100000 * m_nPhotos + 1022);
  }//tau+ decays to anti-nu_tau K+ pi+ K-
  if (GenTauTag::PcheckDecay(genpart, -16, 321, -321, 321)) {
    return +1 * (100000 * m_nPhotos + 1023);
  }//tau+ decays to anti-nu_tau K+ K- K+
  if (GenTauTag::PcheckDecay(genpart, -16, 211, 211, 211, -211, -211)) {
    return +1 * (100000 * m_nPhotos + 1024);
  }//tau+ decays to anti-nu_tau pi+ pi+ pi+ pi- pi-
  if (GenTauTag::PcheckDecay(genpart, -16, 211, 211, 211, -211, -211, 111)) {
    return +1 * (100000 * m_nPhotos + 1025);
  }//tau+ decays to anti-nu_tau pi+ pi+ pi+ pi- pi- pi0
  if (GenTauTag::PcheckDecay(genpart, 323, -16)) {
    return +1 * (100000 * m_nPhotos + 1026);
  }//tau+ decays to K*+ anti-nu_tau
  if (GenTauTag::PcheckDecay(genpart, -16, 221, 211, 111)) {
    return +1 * (100000 * m_nPhotos + 1027);
  }//tau+ decays to anti-nu_tau eta pi+ pi0
  if (GenTauTag::PcheckDecay(genpart, -16, 211, 223, 111)) {
    return +1 * (100000 * m_nPhotos + 1028);
  }//tau+ decays to anti-nu_tau pi+ omega pi0
  if (GenTauTag::PcheckDecay(genpart, -16, 321, 223)) {
    return +1 * (100000 * m_nPhotos + 1029);
  }//tau+ decays to anti-nu_tau K+ omega
  if (GenTauTag::PcheckDecay(genpart, 313, 211, -16)) {
    return +1 * (100000 * m_nPhotos + 1030);
  }//tau+ decays to K*0 pi+ anti-nu_tau
  if (GenTauTag::PcheckDecay(genpart, -313, 321, -16)) {
    return +1 * (100000 * m_nPhotos + 1031);
  }//tau+ decays to anti-K*0 K+ anti-nu_tau
  if (GenTauTag::PcheckDecay(genpart, 333, 211, -16)) {
    return +1 * (100000 * m_nPhotos + 1032);
  }//tau+ decays to phi pi+ anti-nu_tau
  if (GenTauTag::PcheckDecay(genpart, 333, 321, -16)) {
    return +1 * (100000 * m_nPhotos + 1033);
  }//tau+ decays to phi K+ anti-nu_tau
  if (GenTauTag::PcheckDecay(genpart, -13, 14, -16, 22)) {
    return +1 * (100000 * m_nPhotos + 1034);
  }//tau+ decays to mu+ nu_mu anti-nu_tau gamma
  if (GenTauTag::PcheckDecay(genpart, -11, 12, -16, 22)) {
    return +1 * (100000 * m_nPhotos + 1035);
  }//tau+ decays to e+ nu_e anti-nu_tau gamma
  if (GenTauTag::PcheckDecay(genpart, 321, 111, -16)) {
    return +1 * (100000 * m_nPhotos + 1036);
  }//tau+ decays to K+ pi0 anti-nu_tau
  if (GenTauTag::PcheckDecay(genpart, 311, 213, -16)) {
    return +1 * (100000 * m_nPhotos + 1037);
  }//tau+ decays to K0 rho+ anti-nu_tau
  if (GenTauTag::PcheckDecay(genpart, 211, 311, 111, 111, -16)) {
    return +1 * (100000 * m_nPhotos + 1038);
  }//tau+ decays to pi+ K0 pi0 pi0 anti-nu_tau
  if (GenTauTag::PcheckDecay(genpart, 211, -311, 311, 111, -16)) {
    return +1 * (100000 * m_nPhotos + 1039);
  }//tau+ decays to pi+ anti-K0 K0 pi0 anti-nu_tau
  if (GenTauTag::PcheckDecay(genpart, 321, -321, 211, 111, -16)) {
    return +1 * (100000 * m_nPhotos + 1040);
  }//tau+ decays to K+ K- pi+ pi0 anti-nu_tau
  if (GenTauTag::PcheckDecay(genpart, -11, -11, 11, 12, -16)) {
    return +1 * (100000 * m_nPhotos + 1041);
  }//tau+ decays to e+ e+ e- nu_e anti-nu_tau
  if (GenTauTag::PcheckDecay(genpart, 10323, -16)) {
    return +1 * (100000 * m_nPhotos + 1042);
  }//tau+ decays to K_1+ anti-nu_tau
  if (GenTauTag::PcheckDecay(genpart, 20323, -16)) {
    return +1 * (100000 * m_nPhotos + 1043);
  }//tau+ decays to K'_1+ anti-nu_tau
  if (GenTauTag::PcheckDecay(genpart, 100323, -16)) {
    return +1 * (100000 * m_nPhotos + 1044);
  }//tau+ decays to K'*+ anti-nu_tau
  if (GenTauTag::PcheckDecay(genpart, 221, 211, 111, 111, -16)) {
    return +1 * (100000 * m_nPhotos + 1045);
  }//tau+ decays to eta pi+ pi0 pi0 anti-nu_tau
  if (GenTauTag::PcheckDecay(genpart, 221, 321, -16)) {
    return +1 * (100000 * m_nPhotos + 1046);
  }//tau+ decays to eta K+ anti-nu_tau
  if (GenTauTag::PcheckDecay(genpart, 221, 323, -16)) {
    return +1 * (100000 * m_nPhotos + 1047);
  }//tau+ decays to eta K*+ anti-nu_tau
  if (GenTauTag::PcheckDecay(genpart, 221, 321, 111, -16)) {
    return +1 * (100000 * m_nPhotos + 1048);
  }//tau+ decays to eta K+ pi0 anti-nu_tau
  if (GenTauTag::PcheckDecay(genpart, 221, 311, 211, -16)) {
    return +1 * (100000 * m_nPhotos + 1049);
  }//tau+ decays to eta K0 pi+ anti-nu_tau
  if (GenTauTag::PcheckDecay(genpart, 20223, 211, -16)) {
    return +1 * (100000 * m_nPhotos + 1050);
  }//tau+ decays to f_1 pi+ anti-nu_tau
  return +1 * genpart.size();
}// Rest of the tau+ decays


int GenTauTag::Mode_tau_minus(std::vector<int> genpart)
{
  if (GenTauTag::PcheckDecay(genpart, 11, -12, 16)) {
    return -1 * (100000 * m_nPhotos + 1001);
  }//tau- decays to e- anti-nu_e nu_tau
  if (GenTauTag::PcheckDecay(genpart, 13, -14, 16)) {
    return -1 * (100000 * m_nPhotos + 1002);
  }//tau- decays to mu- anti-nu_mu nu_tau
  if (GenTauTag::PcheckDecay(genpart, -211, 16)) {
    return -1 * (100000 * m_nPhotos + 1003);
  }//tau- decays to pi- nu_tau
  if (GenTauTag::PcheckDecay(genpart, -321, 16)) {
    return -1 * (100000 * m_nPhotos + 1004);
  }//tau- decays to K- nu_tau
  if (GenTauTag::PcheckDecay(genpart, -211, 111, 16)) {
    return -1 * (100000 * m_nPhotos + 1005);
  }//tau- decays to pi- pi0 nu_tau
  if (GenTauTag::PcheckDecay(genpart, 16, 22, -211, 111)) {
    return -1 * (100000 * m_nPhotos + 1006);
  }//tau- decays to nu_tau gamma pi- pi0
  if (GenTauTag::PcheckDecay(genpart, 111, 111, -211, 16)) {
    return -1 * (100000 * m_nPhotos + 1007);
  }//tau- decays to pi0 pi0 pi- nu_tau
  if (GenTauTag::PcheckDecay(genpart, 16, -321, 111, 111)) {
    return -1 * (100000 * m_nPhotos + 1008);
  }//tau- decays to nu_tau K- pi0 pi0
  if (GenTauTag::PcheckDecay(genpart, 16, -211, 111, 111, 111)) {
    return -1 * (100000 * m_nPhotos + 1009);
  }//tau- decays to nu_tau pi- pi0 pi0 pi0
  if (GenTauTag::PcheckDecay(genpart, 16, -211, 111, 111, 111, 111)) {
    return -1 * (100000 * m_nPhotos + 1010);
  }//tau- decays to nu_tau pi- pi0 pi0 pi0 pi0
  if (GenTauTag::PcheckDecay(genpart, 16, -211, -311)) {
    return -1 * (100000 * m_nPhotos + 1011);
  }//tau- decays to nu_tau pi- anti-K0
  if (GenTauTag::PcheckDecay(genpart, 16, -321, 311)) {
    return -1 * (100000 * m_nPhotos + 1012);
  }//tau- decays to nu_tau K- K0
  if (GenTauTag::PcheckDecay(genpart, 16, 311, -211, -311)) {
    return -1 * (100000 * m_nPhotos + 1013);
  }//tau- decays to nu_tau K0 pi- anti-K0
  if (GenTauTag::PcheckDecay(genpart, 16, -321, 111, 311)) {
    return -1 * (100000 * m_nPhotos + 1014);
  }//tau- decays to nu_tau K- pi0 K0
  if (GenTauTag::PcheckDecay(genpart, 16, -211, -311, 111)) {
    return -1 * (100000 * m_nPhotos + 1015);
  }//tau- decays to nu_tau pi- anti-K0 pi0
  if (GenTauTag::PcheckDecay(genpart, -211, -211, 211, 16)) {
    return -1 * (100000 * m_nPhotos + 1016);
  }//tau- decays to pi- pi- pi+ nu_tau
  if (GenTauTag::PcheckDecay(genpart, 16, -211, 211, -211, 111)) {
    return -1 * (100000 * m_nPhotos + 1017);
  }//tau- decays to nu_tau pi- pi+ pi- pi0
  if (GenTauTag::PcheckDecay(genpart, 16, -211, -211, 211, 111, 111)) {
    return -1 * (100000 * m_nPhotos + 1018);
  }//tau- decays to nu_tau pi- pi- pi+ pi0 pi0
  if (GenTauTag::PcheckDecay(genpart, 16, -211, -211, 211, 111, 111, 111)) {
    return -1 * (100000 * m_nPhotos + 1019);
  }//tau- decays to nu_tau pi- pi- pi+ pi0 pi0 pi0
  if (GenTauTag::PcheckDecay(genpart, 16, -321, 211, -211)) {
    return -1 * (100000 * m_nPhotos + 1020);
  }//tau- decays to nu_tau K- pi+ pi-
  if (GenTauTag::PcheckDecay(genpart, 16, -321, 211, -211, 111)) {
    return -1 * (100000 * m_nPhotos + 1021);
  }//tau- decays to nu_tau K- pi+ pi- pi0
  if (GenTauTag::PcheckDecay(genpart, 16, -321, -211, 321)) {
    return -1 * (100000 * m_nPhotos + 1022);
  }//tau- decays to nu_tau K- pi- K+
  if (GenTauTag::PcheckDecay(genpart, 16, -321, 321, -321)) {
    return -1 * (100000 * m_nPhotos + 1023);
  }//tau- decays to nu_tau K- K+ K-
  if (GenTauTag::PcheckDecay(genpart, 16, -211, -211, -211, 211, 211)) {
    return -1 * (100000 * m_nPhotos + 1024);
  }//tau- decays to nu_tau pi- pi- pi- pi+ pi+
  if (GenTauTag::PcheckDecay(genpart, 16, -211, -211, -211, 211, 211, 111)) {
    return -1 * (100000 * m_nPhotos + 1025);
  }//tau- decays to nu_tau pi- pi- pi- pi+ pi+ pi0
  if (GenTauTag::PcheckDecay(genpart, -323, 16)) {
    return -1 * (100000 * m_nPhotos + 1026);
  }//tau- decays to K*- nu_tau
  if (GenTauTag::PcheckDecay(genpart, 16, 221, -211, 111)) {
    return -1 * (100000 * m_nPhotos + 1027);
  }//tau- decays to nu_tau eta pi- pi0
  if (GenTauTag::PcheckDecay(genpart, 16, -211, 223, 111)) {
    return -1 * (100000 * m_nPhotos + 1028);
  }//tau- decays to nu_tau pi- omega pi0
  if (GenTauTag::PcheckDecay(genpart, 16, -321, 223)) {
    return -1 * (100000 * m_nPhotos + 1029);
  }//tau- decays to nu_tau K- omega
  if (GenTauTag::PcheckDecay(genpart, -313, -211, 16)) {
    return -1 * (100000 * m_nPhotos + 1030);
  }//tau- decays to anti-K*0 pi- nu_tau
  if (GenTauTag::PcheckDecay(genpart, 313, -321, 16)) {
    return -1 * (100000 * m_nPhotos + 1031);
  }//tau- decays to K*0 K- nu_tau
  if (GenTauTag::PcheckDecay(genpart, 333, -211, 16)) {
    return -1 * (100000 * m_nPhotos + 1032);
  }//tau- decays to phi pi- nu_tau
  if (GenTauTag::PcheckDecay(genpart, 333, -321, 16)) {
    return -1 * (100000 * m_nPhotos + 1033);
  }//tau- decays to phi K- nu_tau
  if (GenTauTag::PcheckDecay(genpart, 13, -14, 16, 22)) {
    return -1 * (100000 * m_nPhotos + 1034);
  }//tau- decays to mu- anti-nu_mu nu_tau gamma
  if (GenTauTag::PcheckDecay(genpart, 11, -12, 16, 22)) {
    return -1 * (100000 * m_nPhotos + 1035);
  }//tau- decays to e- anti-nu_e nu_tau gamma
  if (GenTauTag::PcheckDecay(genpart, -321, 111, 16)) {
    return -1 * (100000 * m_nPhotos + 1036);
  }//tau- decays to K- pi0 nu_tau
  if (GenTauTag::PcheckDecay(genpart, -311, -213, 16)) {
    return -1 * (100000 * m_nPhotos + 1037);
  }//tau- decays to anti-K0 rho- nu_tau
  if (GenTauTag::PcheckDecay(genpart, -211, -311, 111, 111, 16)) {
    return -1 * (100000 * m_nPhotos + 1038);
  }//tau- decays to pi- anti-K0 pi0 pi0 nu_tau
  if (GenTauTag::PcheckDecay(genpart, -211, 311, -311, 111, 16)) {
    return -1 * (100000 * m_nPhotos + 1039);
  }//tau- decays to pi- K0 anti-K0 pi0 nu_tau
  if (GenTauTag::PcheckDecay(genpart, -321, 321, -211, 111, 16)) {
    return -1 * (100000 * m_nPhotos + 1040);
  }//tau- decays to K- K+ pi- pi0 nu_tau
  if (GenTauTag::PcheckDecay(genpart, 11, 11, -11, -12, 16)) {
    return -1 * (100000 * m_nPhotos + 1041);
  }//tau- decays to e- e- e+ anti-nu_e nu_tau
  if (GenTauTag::PcheckDecay(genpart, -10323, 16)) {
    return -1 * (100000 * m_nPhotos + 1042);
  }//tau- decays to K_1- nu_tau
  if (GenTauTag::PcheckDecay(genpart, -20323, 16)) {
    return -1 * (100000 * m_nPhotos + 1043);
  }//tau- decays to K'_1- nu_tau
  if (GenTauTag::PcheckDecay(genpart, -100323, 16)) {
    return -1 * (100000 * m_nPhotos + 1044);
  }//tau- decays to K'*- nu_tau
  if (GenTauTag::PcheckDecay(genpart, 221, -211, 111, 111, 16)) {
    return -1 * (100000 * m_nPhotos + 1045);
  }//tau- decays to eta pi- pi0 pi0 nu_tau
  if (GenTauTag::PcheckDecay(genpart, 221, -321, 16)) {
    return -1 * (100000 * m_nPhotos + 1046);
  }//tau- decays to eta K- nu_tau
  if (GenTauTag::PcheckDecay(genpart, 221, -323, 16)) {
    return -1 * (100000 * m_nPhotos + 1047);
  }//tau- decays to eta K*- nu_tau
  if (GenTauTag::PcheckDecay(genpart, 221, -321, 111, 16)) {
    return -1 * (100000 * m_nPhotos + 1048);
  }//tau- decays to eta K- pi0 nu_tau
  if (GenTauTag::PcheckDecay(genpart, 221, -311, -211, 16)) {
    return -1 * (100000 * m_nPhotos + 1049);
  }//tau- decays to eta anti-K0 pi- nu_tau
  if (GenTauTag::PcheckDecay(genpart, 20223, -211, 16)) {
    return -1 * (100000 * m_nPhotos + 1050);
  }//tau- decays to f_1 pi- nu_tau
  return -1 * (int)genpart.size();
}// Rest of the tau- decays
