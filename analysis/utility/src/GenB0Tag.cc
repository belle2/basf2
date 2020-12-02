/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2020 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Sourav Patra, Vishal Bhardwaj                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <analysis/utility/GenB0Tag.h>
#include <analysis/utility/CheckDecayUtils.h>

using namespace Belle2;

GenB0Tag::GenB0Tag() = default;

bool GenB0Tag::PcheckDecay(std::vector<int> gp, int da1, int da2)
{
  int ndau = gp.size();
  if (ndau == 2) { m_nPhotos = 0; return CheckDecay::CheckDecay(gp, da1, da2);}
  else if (ndau == 3) { m_nPhotos = 1; return CheckDecay::CheckDecay(gp, da1, da2, 22);}
  else if (ndau == 4) { m_nPhotos = 2; return CheckDecay::CheckDecay(gp, da1, da2, 22, 22);}
  else if (ndau == 5) { m_nPhotos = 3; return CheckDecay::CheckDecay(gp, da1, da2, 22, 22, 22);}
  else if (ndau == 6) { m_nPhotos = 4; return CheckDecay::CheckDecay(gp, da1, da2, 22, 22, 22, 22);}
  else return false;
}

bool GenB0Tag::PcheckDecay(std::vector<int> gp, int da1, int da2, int da3)
{
  int ndau = gp.size();
  if (ndau == 3) { m_nPhotos = 0; return CheckDecay::CheckDecay(gp, da1, da2, da3);}
  else if (ndau == 4) { m_nPhotos = 1; return CheckDecay::CheckDecay(gp, da1, da2, da3, 22);}
  else if (ndau == 5) { m_nPhotos = 2; return CheckDecay::CheckDecay(gp, da1, da2, da3, 22, 22);}
  else if (ndau == 6) { m_nPhotos = 3; return CheckDecay::CheckDecay(gp, da1, da2, da3, 22, 22, 22);}
  else if (ndau == 7) { m_nPhotos = 4; return CheckDecay::CheckDecay(gp, da1, da2, da3, 22, 22, 22, 22);}
  else return false;
}

bool GenB0Tag::PcheckDecay(std::vector<int> gp, int da1, int da2, int da3, int da4)
{
  int ndau = gp.size();
  if (ndau == 4) { m_nPhotos = 0; return CheckDecay::CheckDecay(gp, da1, da2, da3, da4);}
  else if (ndau == 5) { m_nPhotos = 1; return CheckDecay::CheckDecay(gp, da1, da2, da3, da4, 22);}
  else if (ndau == 6) { m_nPhotos = 2; return CheckDecay::CheckDecay(gp, da1, da2, da3, da4, 22, 22);}
  else if (ndau == 7) { m_nPhotos = 3; return CheckDecay::CheckDecay(gp, da1, da2, da3, da4, 22, 22, 22);}
  else if (ndau == 8) { m_nPhotos = 4; return CheckDecay::CheckDecay(gp, da1, da2, da3, da4, 22, 22, 22, 22);}
  else return false;
}

bool GenB0Tag::PcheckDecay(std::vector<int> gp, int da1, int da2, int da3, int da4, int da5)
{
  int ndau = gp.size();
  if (ndau == 5) { m_nPhotos = 0; return CheckDecay::CheckDecay(gp, da1, da2, da3, da4, da5);}
  else if (ndau == 6) { m_nPhotos = 1; return CheckDecay::CheckDecay(gp, da1, da2, da3, da4, da5, 22);}
  else if (ndau == 7) { m_nPhotos = 2; return CheckDecay::CheckDecay(gp, da1, da2, da3, da4, da5, 22, 22);}
  else if (ndau == 8) { m_nPhotos = 3; return CheckDecay::CheckDecay(gp, da1, da2, da3, da4, da5, 22, 22, 22);}
  else if (ndau == 9) { m_nPhotos = 4; return CheckDecay::CheckDecay(gp, da1, da2, da3, da4, da5, 22, 22, 22, 22);}
  else return false;
}

bool GenB0Tag::PcheckDecay(std::vector<int> gp, int da1, int da2, int da3, int da4, int da5, int da6)
{
  int ndau = gp.size();
  if (ndau == 6) { m_nPhotos = 0;  return CheckDecay::CheckDecay(gp, da1, da2, da3, da4, da5, da6);}
  else if (ndau == 7) { m_nPhotos = 1; return CheckDecay::CheckDecay(gp, da1, da2, da3, da4, da5, da6, 22);}
  else if (ndau == 8) { m_nPhotos = 2; return CheckDecay::CheckDecay(gp, da1, da2, da3, da4, da5, da6, 22, 22);}
  else if (ndau == 9) { m_nPhotos = 3; return CheckDecay::CheckDecay(gp, da1, da2, da3, da4, da5, da6, 22, 22, 22);}
  else if (ndau == 10) { m_nPhotos = 4; return CheckDecay::CheckDecay(gp, da1, da2, da3, da4, da5, da6, 22, 22, 22, 22);}
  else return false;
}

bool GenB0Tag::PcheckDecay(std::vector<int> gp, int da1, int da2, int da3, int da4, int da5, int da6, int da7)
{
  int ndau = gp.size();
  if (ndau == 7) { m_nPhotos = 0;  return CheckDecay::CheckDecay(gp, da1, da2, da3, da4, da5, da6, da7);}
  else if (ndau == 8) { m_nPhotos = 1; return CheckDecay::CheckDecay(gp, da1, da2, da3, da4, da5, da6, da7, 22);}
  else if (ndau == 9) { m_nPhotos = 2; return CheckDecay::CheckDecay(gp, da1, da2, da3, da4, da5, da6, da7, 22, 22);}
  else if (ndau == 10) { m_nPhotos = 3; return CheckDecay::CheckDecay(gp, da1, da2, da3, da4, da5, da6, da7, 22, 22, 22);}
  else if (ndau == 11) { m_nPhotos = 4; return CheckDecay::CheckDecay(gp, da1, da2, da3, da4, da5, da6, da7, 22, 22, 22, 22);}
  else return false;
}

bool GenB0Tag::PcheckDecay(std::vector<int> gp, int da1, int da2, int da3, int da4, int da5, int da6, int da7, int da8)
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

int GenB0Tag::Mode_B0(std::vector<int> genpart)
{
  if (GenB0Tag::PcheckDecay(genpart, -413, -11, 12)) {
    return +1 * (100000 * m_nPhotos + 1001);
  }//B0 decays to D*- e+ nu_e
  if (GenB0Tag::PcheckDecay(genpart, -411, -11, 12)) {
    return +1 * (100000 * m_nPhotos + 1002);
  }//B0 decays to D- e+ nu_e
  if (GenB0Tag::PcheckDecay(genpart, -10413, -11, 12)) {
    return +1 * (100000 * m_nPhotos + 1003);
  }//B0 decays to D_1- e+ nu_e
  if (GenB0Tag::PcheckDecay(genpart, -10411, -11, 12)) {
    return +1 * (100000 * m_nPhotos + 1004);
  }//B0 decays to D_0*- e+ nu_e
  if (GenB0Tag::PcheckDecay(genpart, -20413, -11, 12)) {
    return +1 * (100000 * m_nPhotos + 1005);
  }//B0 decays to D'_1- e+ nu_e
  if (GenB0Tag::PcheckDecay(genpart, -415, -11, 12)) {
    return +1 * (100000 * m_nPhotos + 1006);
  }//B0 decays to D_2*- e+ nu_e
  if (GenB0Tag::PcheckDecay(genpart, -423, -211, -11, 12)) {
    return +1 * (100000 * m_nPhotos + 1007);
  }//B0 decays to anti-D*0 pi- e+ nu_e
  if (GenB0Tag::PcheckDecay(genpart, -413, 111, -11, 12)) {
    return +1 * (100000 * m_nPhotos + 1008);
  }//B0 decays to D*- pi0 e+ nu_e
  if (GenB0Tag::PcheckDecay(genpart, -421, -211, -11, 12)) {
    return +1 * (100000 * m_nPhotos + 1009);
  }//B0 decays to anti-D0 pi- e+ nu_e
  if (GenB0Tag::PcheckDecay(genpart, -411, 111, -11, 12)) {
    return +1 * (100000 * m_nPhotos + 1010);
  }//B0 decays to D- pi0 e+ nu_e
  if (GenB0Tag::PcheckDecay(genpart, -411, 211, -211, -11, 12)) {
    return +1 * (100000 * m_nPhotos + 1011);
  }//B0 decays to D- pi+ pi- e+ nu_e
  if (GenB0Tag::PcheckDecay(genpart, -421, -211, 111, -11, 12)) {
    return +1 * (100000 * m_nPhotos + 1012);
  }//B0 decays to anti-D0 pi- pi0 e+ nu_e
  if (GenB0Tag::PcheckDecay(genpart, -411, 111, 111, -11, 12)) {
    return +1 * (100000 * m_nPhotos + 1013);
  }//B0 decays to D- pi0 pi0 e+ nu_e
  if (GenB0Tag::PcheckDecay(genpart, -413, 211, -211, -11, 12)) {
    return +1 * (100000 * m_nPhotos + 1014);
  }//B0 decays to D*- pi+ pi- e+ nu_e
  if (GenB0Tag::PcheckDecay(genpart, -423, -211, 111, -11, 12)) {
    return +1 * (100000 * m_nPhotos + 1015);
  }//B0 decays to anti-D*0 pi- pi0 e+ nu_e
  if (GenB0Tag::PcheckDecay(genpart, -413, 111, 111, -11, 12)) {
    return +1 * (100000 * m_nPhotos + 1016);
  }//B0 decays to D*- pi0 pi0 e+ nu_e
  if (GenB0Tag::PcheckDecay(genpart, -411, 221, -11, 12)) {
    return +1 * (100000 * m_nPhotos + 1017);
  }//B0 decays to D- eta e+ nu_e
  if (GenB0Tag::PcheckDecay(genpart, -413, 221, -11, 12)) {
    return +1 * (100000 * m_nPhotos + 1018);
  }//B0 decays to D*- eta e+ nu_e
  if (GenB0Tag::PcheckDecay(genpart, -413, -13, 14)) {
    return +1 * (100000 * m_nPhotos + 1019);
  }//B0 decays to D*- mu+ nu_mu
  if (GenB0Tag::PcheckDecay(genpart, -411, -13, 14)) {
    return +1 * (100000 * m_nPhotos + 1020);
  }//B0 decays to D- mu+ nu_mu
  if (GenB0Tag::PcheckDecay(genpart, -10413, -13, 14)) {
    return +1 * (100000 * m_nPhotos + 1021);
  }//B0 decays to D_1- mu+ nu_mu
  if (GenB0Tag::PcheckDecay(genpart, -10411, -13, 14)) {
    return +1 * (100000 * m_nPhotos + 1022);
  }//B0 decays to D_0*- mu+ nu_mu
  if (GenB0Tag::PcheckDecay(genpart, -20413, -13, 14)) {
    return +1 * (100000 * m_nPhotos + 1023);
  }//B0 decays to D'_1- mu+ nu_mu
  if (GenB0Tag::PcheckDecay(genpart, -415, -13, 14)) {
    return +1 * (100000 * m_nPhotos + 1024);
  }//B0 decays to D_2*- mu+ nu_mu
  if (GenB0Tag::PcheckDecay(genpart, -423, -211, -13, 14)) {
    return +1 * (100000 * m_nPhotos + 1025);
  }//B0 decays to anti-D*0 pi- mu+ nu_mu
  if (GenB0Tag::PcheckDecay(genpart, -413, 111, -13, 14)) {
    return +1 * (100000 * m_nPhotos + 1026);
  }//B0 decays to D*- pi0 mu+ nu_mu
  if (GenB0Tag::PcheckDecay(genpart, -421, -211, -13, 14)) {
    return +1 * (100000 * m_nPhotos + 1027);
  }//B0 decays to anti-D0 pi- mu+ nu_mu
  if (GenB0Tag::PcheckDecay(genpart, -411, 111, -13, 14)) {
    return +1 * (100000 * m_nPhotos + 1028);
  }//B0 decays to D- pi0 mu+ nu_mu
  if (GenB0Tag::PcheckDecay(genpart, -411, 211, -211, -13, 14)) {
    return +1 * (100000 * m_nPhotos + 1029);
  }//B0 decays to D- pi+ pi- mu+ nu_mu
  if (GenB0Tag::PcheckDecay(genpart, -421, -211, 111, -13, 14)) {
    return +1 * (100000 * m_nPhotos + 1030);
  }//B0 decays to anti-D0 pi- pi0 mu+ nu_mu
  if (GenB0Tag::PcheckDecay(genpart, -411, 111, 111, -13, 14)) {
    return +1 * (100000 * m_nPhotos + 1031);
  }//B0 decays to D- pi0 pi0 mu+ nu_mu
  if (GenB0Tag::PcheckDecay(genpart, -413, 211, -211, -13, 14)) {
    return +1 * (100000 * m_nPhotos + 1032);
  }//B0 decays to D*- pi+ pi- mu+ nu_mu
  if (GenB0Tag::PcheckDecay(genpart, -423, -211, 111, -13, 14)) {
    return +1 * (100000 * m_nPhotos + 1033);
  }//B0 decays to anti-D*0 pi- pi0 mu+ nu_mu
  if (GenB0Tag::PcheckDecay(genpart, -413, 111, 111, -13, 14)) {
    return +1 * (100000 * m_nPhotos + 1034);
  }//B0 decays to D*- pi0 pi0 mu+ nu_mu
  if (GenB0Tag::PcheckDecay(genpart, -411, 221, -13, 14)) {
    return +1 * (100000 * m_nPhotos + 1035);
  }//B0 decays to D- eta mu+ nu_mu
  if (GenB0Tag::PcheckDecay(genpart, -413, 221, -13, 14)) {
    return +1 * (100000 * m_nPhotos + 1036);
  }//B0 decays to D*- eta mu+ nu_mu
  if (GenB0Tag::PcheckDecay(genpart, -413, -15, 16)) {
    return +1 * (100000 * m_nPhotos + 1037);
  }//B0 decays to D*- tau+ nu_tau
  if (GenB0Tag::PcheckDecay(genpart, -411, -15, 16)) {
    return +1 * (100000 * m_nPhotos + 1038);
  }//B0 decays to D- tau+ nu_tau
  if (GenB0Tag::PcheckDecay(genpart, -10413, -15, 16)) {
    return +1 * (100000 * m_nPhotos + 1039);
  }//B0 decays to D_1- tau+ nu_tau
  if (GenB0Tag::PcheckDecay(genpart, -10411, -15, 16)) {
    return +1 * (100000 * m_nPhotos + 1040);
  }//B0 decays to D_0*- tau+ nu_tau
  if (GenB0Tag::PcheckDecay(genpart, -20413, -15, 16)) {
    return +1 * (100000 * m_nPhotos + 1041);
  }//B0 decays to D'_1- tau+ nu_tau
  if (GenB0Tag::PcheckDecay(genpart, -415, -15, 16)) {
    return +1 * (100000 * m_nPhotos + 1042);
  }//B0 decays to D_2*- tau+ nu_tau
  if (GenB0Tag::PcheckDecay(genpart, -211, -11, 12)) {
    return +1 * (100000 * m_nPhotos + 1043);
  }//B0 decays to pi- e+ nu_e
  if (GenB0Tag::PcheckDecay(genpart, -213, -11, 12)) {
    return +1 * (100000 * m_nPhotos + 1044);
  }//B0 decays to rho- e+ nu_e
  if (GenB0Tag::PcheckDecay(genpart, -100411, -11, 12)) {
    return +1 * (100000 * m_nPhotos + 1045);
  }//B0 decays to D(2S)- e+ nu_e
  if (GenB0Tag::PcheckDecay(genpart, -100413, -11, 12)) {
    return +1 * (100000 * m_nPhotos + 1046);
  }//B0 decays to D*(2S)- e+ nu_e
  if (GenB0Tag::PcheckDecay(genpart, -62, -11, 12)) {
    return +1 * (100000 * m_nPhotos + 1047);
  }//B0 decays to Xu- e+ nu_e
  if (GenB0Tag::PcheckDecay(genpart, -211, -13, 14)) {
    return +1 * (100000 * m_nPhotos + 1048);
  }//B0 decays to pi- mu+ nu_mu
  if (GenB0Tag::PcheckDecay(genpart, -213, -13, 14)) {
    return +1 * (100000 * m_nPhotos + 1049);
  }//B0 decays to rho- mu+ nu_mu
  if (GenB0Tag::PcheckDecay(genpart, -100411, -13, 14)) {
    return +1 * (100000 * m_nPhotos + 1050);
  }//B0 decays to D(2S)- mu+ nu_mu
  if (GenB0Tag::PcheckDecay(genpart, -100413, -13, 14)) {
    return +1 * (100000 * m_nPhotos + 1051);
  }//B0 decays to D*(2S)- mu+ nu_mu
  if (GenB0Tag::PcheckDecay(genpart, -62, -13, 14)) {
    return +1 * (100000 * m_nPhotos + 1052);
  }//B0 decays to Xu- mu+ nu_mu
  if (GenB0Tag::PcheckDecay(genpart, -211, -15, 16)) {
    return +1 * (100000 * m_nPhotos + 1053);
  }//B0 decays to pi- tau+ nu_tau
  if (GenB0Tag::PcheckDecay(genpart, -213, -15, 16)) {
    return +1 * (100000 * m_nPhotos + 1054);
  }//B0 decays to rho- tau+ nu_tau
  if (GenB0Tag::PcheckDecay(genpart, -20213, -15, 16)) {
    return +1 * (100000 * m_nPhotos + 1055);
  }//B0 decays to a_1- tau+ nu_tau
  if (GenB0Tag::PcheckDecay(genpart, -10213, -15, 16)) {
    return +1 * (100000 * m_nPhotos + 1056);
  }//B0 decays to b_1- tau+ nu_tau
  if (GenB0Tag::PcheckDecay(genpart, -10211, -15, 16)) {
    return +1 * (100000 * m_nPhotos + 1057);
  }//B0 decays to a_0- tau+ nu_tau
  if (GenB0Tag::PcheckDecay(genpart, -215, -15, 16)) {
    return +1 * (100000 * m_nPhotos + 1058);
  }//B0 decays to a_2- tau+ nu_tau
  if (GenB0Tag::PcheckDecay(genpart, -100213, -15, 16)) {
    return +1 * (100000 * m_nPhotos + 1059);
  }//B0 decays to rho(2S)- tau+ nu_tau
  if (GenB0Tag::PcheckDecay(genpart, -100411, -15, 16)) {
    return +1 * (100000 * m_nPhotos + 1060);
  }//B0 decays to D(2S)- tau+ nu_tau
  if (GenB0Tag::PcheckDecay(genpart, -100413, -15, 16)) {
    return +1 * (100000 * m_nPhotos + 1061);
  }//B0 decays to D*(2S)- tau+ nu_tau
  if (GenB0Tag::PcheckDecay(genpart, 431, -211)) {
    return +1 * (100000 * m_nPhotos + 1062);
  }//B0 decays to D_s+ pi-
  if (GenB0Tag::PcheckDecay(genpart, -431, 321)) {
    return +1 * (100000 * m_nPhotos + 1063);
  }//B0 decays to D_s- K+
  if (GenB0Tag::PcheckDecay(genpart, 433, -211)) {
    return +1 * (100000 * m_nPhotos + 1064);
  }//B0 decays to D_s*+ pi-
  if (GenB0Tag::PcheckDecay(genpart, -433, 321)) {
    return +1 * (100000 * m_nPhotos + 1065);
  }//B0 decays to D_s*- K+
  if (GenB0Tag::PcheckDecay(genpart, -213, 431)) {
    return +1 * (100000 * m_nPhotos + 1066);
  }//B0 decays to rho- D_s+
  if (GenB0Tag::PcheckDecay(genpart, 323, -431)) {
    return +1 * (100000 * m_nPhotos + 1067);
  }//B0 decays to K*+ D_s-
  if (GenB0Tag::PcheckDecay(genpart, 433, -213)) {
    return +1 * (100000 * m_nPhotos + 1068);
  }//B0 decays to D_s*+ rho-
  if (GenB0Tag::PcheckDecay(genpart, -433, 323)) {
    return +1 * (100000 * m_nPhotos + 1069);
  }//B0 decays to D_s*- K*+
  if (GenB0Tag::PcheckDecay(genpart, 313, 22)) {
    return +1 * (100000 * m_nPhotos + 1070);
  }//B0 decays to K*0 gamma
  if (GenB0Tag::PcheckDecay(genpart, 223, 22)) {
    return +1 * (100000 * m_nPhotos + 1071);
  }//B0 decays to omega gamma
  if (GenB0Tag::PcheckDecay(genpart, 113, 22)) {
    return +1 * (100000 * m_nPhotos + 1072);
  }//B0 decays to rho0 gamma
  if (GenB0Tag::PcheckDecay(genpart, 10313, 22)) {
    return +1 * (100000 * m_nPhotos + 1073);
  }//B0 decays to K_10 gamma
  if (GenB0Tag::PcheckDecay(genpart, 20313, 22)) {
    return +1 * (100000 * m_nPhotos + 1074);
  }//B0 decays to K'_10 gamma
  if (GenB0Tag::PcheckDecay(genpart, 315, 22)) {
    return +1 * (100000 * m_nPhotos + 1075);
  }//B0 decays to K_2*0 gamma
  if (GenB0Tag::PcheckDecay(genpart, 30313, 22)) {
    return +1 * (100000 * m_nPhotos + 1076);
  }//B0 decays to K''*0 gamma
  if (GenB0Tag::PcheckDecay(genpart, 30343, 22)) {
    return +1 * (100000 * m_nPhotos + 1077);
  }//B0 decays to Xsd gamma
  if (GenB0Tag::PcheckDecay(genpart, 311, -11, 11)) {
    return +1 * (100000 * m_nPhotos + 1078);
  }//B0 decays to K0 e+ e-
  if (GenB0Tag::PcheckDecay(genpart, 313, -11, 11)) {
    return +1 * (100000 * m_nPhotos + 1079);
  }//B0 decays to K*0 e+ e-
  if (GenB0Tag::PcheckDecay(genpart, 30343, -11, 11)) {
    return +1 * (100000 * m_nPhotos + 1080);
  }//B0 decays to Xsd e+ e-
  if (GenB0Tag::PcheckDecay(genpart, 211, -211, -11, 11)) {
    return +1 * (100000 * m_nPhotos + 1081);
  }//B0 decays to pi+ pi- e+ e-
  if (GenB0Tag::PcheckDecay(genpart, 311, -13, 13)) {
    return +1 * (100000 * m_nPhotos + 1082);
  }//B0 decays to K0 mu+ mu-
  if (GenB0Tag::PcheckDecay(genpart, 313, -13, 13)) {
    return +1 * (100000 * m_nPhotos + 1083);
  }//B0 decays to K*0 mu+ mu-
  if (GenB0Tag::PcheckDecay(genpart, 211, -211, -13, 13)) {
    return +1 * (100000 * m_nPhotos + 1084);
  }//B0 decays to pi+ pi- mu+ mu-
  if (GenB0Tag::PcheckDecay(genpart, 30343, -13, 13)) {
    return +1 * (100000 * m_nPhotos + 1085);
  }//B0 decays to Xsd mu+ mu-
  if (GenB0Tag::PcheckDecay(genpart, 311, -15, 15)) {
    return +1 * (100000 * m_nPhotos + 1086);
  }//B0 decays to K0 tau+ tau-
  if (GenB0Tag::PcheckDecay(genpart, 313, -15, 15)) {
    return +1 * (100000 * m_nPhotos + 1087);
  }//B0 decays to K*0 tau+ tau-
  if (GenB0Tag::PcheckDecay(genpart, 30343, -15, 15)) {
    return +1 * (100000 * m_nPhotos + 1088);
  }//B0 decays to Xsd tau+ tau-
  if (GenB0Tag::PcheckDecay(genpart, 211, -211)) {
    return +1 * (100000 * m_nPhotos + 1089);
  }//B0 decays to pi+ pi-
  if (GenB0Tag::PcheckDecay(genpart, 111, 111)) {
    return +1 * (100000 * m_nPhotos + 1090);
  }//B0 decays to pi0 pi0
  if (GenB0Tag::PcheckDecay(genpart, 111, 221)) {
    return +1 * (100000 * m_nPhotos + 1091);
  }//B0 decays to pi0 eta
  if (GenB0Tag::PcheckDecay(genpart, 111, 331)) {
    return +1 * (100000 * m_nPhotos + 1092);
  }//B0 decays to pi0 eta'
  if (GenB0Tag::PcheckDecay(genpart, 111, 10111)) {
    return +1 * (100000 * m_nPhotos + 1093);
  }//B0 decays to pi0 a_00
  if (GenB0Tag::PcheckDecay(genpart, 111, 10221)) {
    return +1 * (100000 * m_nPhotos + 1094);
  }//B0 decays to pi0 f_0
  if (GenB0Tag::PcheckDecay(genpart, 111, 223)) {
    return +1 * (100000 * m_nPhotos + 1095);
  }//B0 decays to pi0 omega
  if (GenB0Tag::PcheckDecay(genpart, 111, 20113)) {
    return +1 * (100000 * m_nPhotos + 1096);
  }//B0 decays to pi0 a_10
  if (GenB0Tag::PcheckDecay(genpart, 111, 10113)) {
    return +1 * (100000 * m_nPhotos + 1097);
  }//B0 decays to pi0 b_10
  if (GenB0Tag::PcheckDecay(genpart, 221, 221)) {
    return +1 * (100000 * m_nPhotos + 1098);
  }//B0 decays to eta eta
  if (GenB0Tag::PcheckDecay(genpart, 221, 331)) {
    return +1 * (100000 * m_nPhotos + 1099);
  }//B0 decays to eta eta'
  if (GenB0Tag::PcheckDecay(genpart, 221, 10111)) {
    return +1 * (100000 * m_nPhotos + 1100);
  }//B0 decays to eta a_00
  if (GenB0Tag::PcheckDecay(genpart, 221, 10221)) {
    return +1 * (100000 * m_nPhotos + 1101);
  }//B0 decays to eta f_0
  if (GenB0Tag::PcheckDecay(genpart, 221, 113)) {
    return +1 * (100000 * m_nPhotos + 1102);
  }//B0 decays to eta rho0
  if (GenB0Tag::PcheckDecay(genpart, 221, 223)) {
    return +1 * (100000 * m_nPhotos + 1103);
  }//B0 decays to eta omega
  if (GenB0Tag::PcheckDecay(genpart, 221, 20113)) {
    return +1 * (100000 * m_nPhotos + 1104);
  }//B0 decays to eta a_10
  if (GenB0Tag::PcheckDecay(genpart, 221, 10113)) {
    return +1 * (100000 * m_nPhotos + 1105);
  }//B0 decays to eta b_10
  if (GenB0Tag::PcheckDecay(genpart, 331, 331)) {
    return +1 * (100000 * m_nPhotos + 1106);
  }//B0 decays to eta' eta'
  if (GenB0Tag::PcheckDecay(genpart, 331, 10111)) {
    return +1 * (100000 * m_nPhotos + 1107);
  }//B0 decays to eta' a_00
  if (GenB0Tag::PcheckDecay(genpart, 331, 10221)) {
    return +1 * (100000 * m_nPhotos + 1108);
  }//B0 decays to eta' f_0
  if (GenB0Tag::PcheckDecay(genpart, 331, 113)) {
    return +1 * (100000 * m_nPhotos + 1109);
  }//B0 decays to eta' rho0
  if (GenB0Tag::PcheckDecay(genpart, 331, 223)) {
    return +1 * (100000 * m_nPhotos + 1110);
  }//B0 decays to eta' omega
  if (GenB0Tag::PcheckDecay(genpart, 331, 20113)) {
    return +1 * (100000 * m_nPhotos + 1111);
  }//B0 decays to eta' a_10
  if (GenB0Tag::PcheckDecay(genpart, 331, 10113)) {
    return +1 * (100000 * m_nPhotos + 1112);
  }//B0 decays to eta' b_10
  if (GenB0Tag::PcheckDecay(genpart, 10111, 10111)) {
    return +1 * (100000 * m_nPhotos + 1113);
  }//B0 decays to a_00 a_00
  if (GenB0Tag::PcheckDecay(genpart, 10111, 10221)) {
    return +1 * (100000 * m_nPhotos + 1114);
  }//B0 decays to a_00 f_0
  if (GenB0Tag::PcheckDecay(genpart, 10111, 113)) {
    return +1 * (100000 * m_nPhotos + 1115);
  }//B0 decays to a_00 rho0
  if (GenB0Tag::PcheckDecay(genpart, 10111, 223)) {
    return +1 * (100000 * m_nPhotos + 1116);
  }//B0 decays to a_00 omega
  if (GenB0Tag::PcheckDecay(genpart, 10111, 20113)) {
    return +1 * (100000 * m_nPhotos + 1117);
  }//B0 decays to a_00 a_10
  if (GenB0Tag::PcheckDecay(genpart, 10111, 10113)) {
    return +1 * (100000 * m_nPhotos + 1118);
  }//B0 decays to a_00 b_10
  if (GenB0Tag::PcheckDecay(genpart, 10221, 10221)) {
    return +1 * (100000 * m_nPhotos + 1119);
  }//B0 decays to f_0 f_0
  if (GenB0Tag::PcheckDecay(genpart, 10221, 113)) {
    return +1 * (100000 * m_nPhotos + 1120);
  }//B0 decays to f_0 rho0
  if (GenB0Tag::PcheckDecay(genpart, 10221, 223)) {
    return +1 * (100000 * m_nPhotos + 1121);
  }//B0 decays to f_0 omega
  if (GenB0Tag::PcheckDecay(genpart, 10221, 20113)) {
    return +1 * (100000 * m_nPhotos + 1122);
  }//B0 decays to f_0 a_10
  if (GenB0Tag::PcheckDecay(genpart, 10221, 10113)) {
    return +1 * (100000 * m_nPhotos + 1123);
  }//B0 decays to f_0 b_10
  if (GenB0Tag::PcheckDecay(genpart, 333, 310)) {
    return +1 * (100000 * m_nPhotos + 1124);
  }//B0 decays to phi K_S0
  if (GenB0Tag::PcheckDecay(genpart, 333, 130)) {
    return +1 * (100000 * m_nPhotos + 1125);
  }//B0 decays to phi K_L0
  if (GenB0Tag::PcheckDecay(genpart, 221, 310)) {
    return +1 * (100000 * m_nPhotos + 1126);
  }//B0 decays to eta K_S0
  if (GenB0Tag::PcheckDecay(genpart, 221, 130)) {
    return +1 * (100000 * m_nPhotos + 1127);
  }//B0 decays to eta K_L0
  if (GenB0Tag::PcheckDecay(genpart, 331, 310)) {
    return +1 * (100000 * m_nPhotos + 1128);
  }//B0 decays to eta' K_S0
  if (GenB0Tag::PcheckDecay(genpart, 331, 130)) {
    return +1 * (100000 * m_nPhotos + 1129);
  }//B0 decays to eta' K_L0
  if (GenB0Tag::PcheckDecay(genpart, 223, 310)) {
    return +1 * (100000 * m_nPhotos + 1130);
  }//B0 decays to omega K_S0
  if (GenB0Tag::PcheckDecay(genpart, 223, 130)) {
    return +1 * (100000 * m_nPhotos + 1131);
  }//B0 decays to omega K_L0
  if (GenB0Tag::PcheckDecay(genpart, 333, 311)) {
    return +1 * (100000 * m_nPhotos + 1132);
  }//B0 decays to phi K0
  if (GenB0Tag::PcheckDecay(genpart, 333, -311)) {
    return +1 * (100000 * m_nPhotos + 1133);
  }//B0 decays to phi anti-K0
  if (GenB0Tag::PcheckDecay(genpart, 221, 311)) {
    return +1 * (100000 * m_nPhotos + 1134);
  }//B0 decays to eta K0
  if (GenB0Tag::PcheckDecay(genpart, 221, -311)) {
    return +1 * (100000 * m_nPhotos + 1135);
  }//B0 decays to eta anti-K0
  if (GenB0Tag::PcheckDecay(genpart, 331, 311)) {
    return +1 * (100000 * m_nPhotos + 1136);
  }//B0 decays to eta' K0
  if (GenB0Tag::PcheckDecay(genpart, 331, -311)) {
    return +1 * (100000 * m_nPhotos + 1137);
  }//B0 decays to eta' anti-K0
  if (GenB0Tag::PcheckDecay(genpart, 223, 311)) {
    return +1 * (100000 * m_nPhotos + 1138);
  }//B0 decays to omega K0
  if (GenB0Tag::PcheckDecay(genpart, 223, -311)) {
    return +1 * (100000 * m_nPhotos + 1139);
  }//B0 decays to omega anti-K0
  if (GenB0Tag::PcheckDecay(genpart, 333, 111)) {
    return +1 * (100000 * m_nPhotos + 1140);
  }//B0 decays to phi pi0
  if (GenB0Tag::PcheckDecay(genpart, 333, 221)) {
    return +1 * (100000 * m_nPhotos + 1141);
  }//B0 decays to phi eta
  if (GenB0Tag::PcheckDecay(genpart, 333, 331)) {
    return +1 * (100000 * m_nPhotos + 1142);
  }//B0 decays to phi eta'
  if (GenB0Tag::PcheckDecay(genpart, 313, 221)) {
    return +1 * (100000 * m_nPhotos + 1143);
  }//B0 decays to K*0 eta
  if (GenB0Tag::PcheckDecay(genpart, 313, 331)) {
    return +1 * (100000 * m_nPhotos + 1144);
  }//B0 decays to K*0 eta'
  if (GenB0Tag::PcheckDecay(genpart, 313, 223)) {
    return +1 * (100000 * m_nPhotos + 1145);
  }//B0 decays to K*0 omega
  if (GenB0Tag::PcheckDecay(genpart, 333, 223)) {
    return +1 * (100000 * m_nPhotos + 1146);
  }//B0 decays to phi omega
  if (GenB0Tag::PcheckDecay(genpart, 223, 223)) {
    return +1 * (100000 * m_nPhotos + 1147);
  }//B0 decays to omega omega
  if (GenB0Tag::PcheckDecay(genpart, 223, 113)) {
    return +1 * (100000 * m_nPhotos + 1148);
  }//B0 decays to omega rho0
  if (GenB0Tag::PcheckDecay(genpart, 321, -211)) {
    return +1 * (100000 * m_nPhotos + 1149);
  }//B0 decays to K+ pi-
  if (GenB0Tag::PcheckDecay(genpart, 321, -321)) {
    return +1 * (100000 * m_nPhotos + 1150);
  }//B0 decays to K+ K-
  if (GenB0Tag::PcheckDecay(genpart, 310, 310)) {
    return +1 * (100000 * m_nPhotos + 1151);
  }//B0 decays to K_S0 K_S0
  if (GenB0Tag::PcheckDecay(genpart, 310, 130)) {
    return +1 * (100000 * m_nPhotos + 1152);
  }//B0 decays to K_S0 K_L0
  if (GenB0Tag::PcheckDecay(genpart, 130, 130)) {
    return +1 * (100000 * m_nPhotos + 1153);
  }//B0 decays to K_L0 K_L0
  if (GenB0Tag::PcheckDecay(genpart, 311, 111)) {
    return +1 * (100000 * m_nPhotos + 1154);
  }//B0 decays to K0 pi0
  if (GenB0Tag::PcheckDecay(genpart, 310, 111)) {
    return +1 * (100000 * m_nPhotos + 1155);
  }//B0 decays to K_S0 pi0
  if (GenB0Tag::PcheckDecay(genpart, 130, 111)) {
    return +1 * (100000 * m_nPhotos + 1156);
  }//B0 decays to K_L0 pi0
  if (GenB0Tag::PcheckDecay(genpart, 311, -311)) {
    return +1 * (100000 * m_nPhotos + 1157);
  }//B0 decays to K0 anti-K0
  if (GenB0Tag::PcheckDecay(genpart, -211, 211, 111)) {
    return +1 * (100000 * m_nPhotos + 1158);
  }//B0 decays to pi- pi+ pi0
  if (GenB0Tag::PcheckDecay(genpart, 113, 111)) {
    return +1 * (100000 * m_nPhotos + 1159);
  }//B0 decays to rho0 pi0
  if (GenB0Tag::PcheckDecay(genpart, 213, -211)) {
    return +1 * (100000 * m_nPhotos + 1160);
  }//B0 decays to rho+ pi-
  if (GenB0Tag::PcheckDecay(genpart, -213, 211)) {
    return +1 * (100000 * m_nPhotos + 1161);
  }//B0 decays to rho- pi+
  if (GenB0Tag::PcheckDecay(genpart, 100213, -211)) {
    return +1 * (100000 * m_nPhotos + 1162);
  }//B0 decays to rho(2S)+ pi-
  if (GenB0Tag::PcheckDecay(genpart, -100213, 211)) {
    return +1 * (100000 * m_nPhotos + 1163);
  }//B0 decays to rho(2S)- pi+
  if (GenB0Tag::PcheckDecay(genpart, 30213, -211)) {
    return +1 * (100000 * m_nPhotos + 1164);
  }//B0 decays to rho(3S)+ pi-
  if (GenB0Tag::PcheckDecay(genpart, -30213, 211)) {
    return +1 * (100000 * m_nPhotos + 1165);
  }//B0 decays to rho(3S)- pi+
  if (GenB0Tag::PcheckDecay(genpart, 113, 310)) {
    return +1 * (100000 * m_nPhotos + 1166);
  }//B0 decays to rho0 K_S0
  if (GenB0Tag::PcheckDecay(genpart, 113, 130)) {
    return +1 * (100000 * m_nPhotos + 1167);
  }//B0 decays to rho0 K_L0
  if (GenB0Tag::PcheckDecay(genpart, -213, 321)) {
    return +1 * (100000 * m_nPhotos + 1168);
  }//B0 decays to rho- K+
  if (GenB0Tag::PcheckDecay(genpart, 100113, 310)) {
    return +1 * (100000 * m_nPhotos + 1169);
  }//B0 decays to rho(2S)0 K_S0
  if (GenB0Tag::PcheckDecay(genpart, 100113, 130)) {
    return +1 * (100000 * m_nPhotos + 1170);
  }//B0 decays to rho(2S)0 K_L0
  if (GenB0Tag::PcheckDecay(genpart, 10221, 311)) {
    return +1 * (100000 * m_nPhotos + 1171);
  }//B0 decays to f_0 K0
  if (GenB0Tag::PcheckDecay(genpart, 10111, 311)) {
    return +1 * (100000 * m_nPhotos + 1172);
  }//B0 decays to a_00 K0
  if (GenB0Tag::PcheckDecay(genpart, -10211, 321)) {
    return +1 * (100000 * m_nPhotos + 1173);
  }//B0 decays to a_0- K+
  if (GenB0Tag::PcheckDecay(genpart, 10211, -211)) {
    return +1 * (100000 * m_nPhotos + 1174);
  }//B0 decays to a_0+ pi-
  if (GenB0Tag::PcheckDecay(genpart, -10211, 211)) {
    return +1 * (100000 * m_nPhotos + 1175);
  }//B0 decays to a_0- pi+
  if (GenB0Tag::PcheckDecay(genpart, 313, 111)) {
    return +1 * (100000 * m_nPhotos + 1176);
  }//B0 decays to K*0 pi0
  if (GenB0Tag::PcheckDecay(genpart, 313, -311)) {
    return +1 * (100000 * m_nPhotos + 1177);
  }//B0 decays to K*0 anti-K0
  if (GenB0Tag::PcheckDecay(genpart, 10311, 111)) {
    return +1 * (100000 * m_nPhotos + 1178);
  }//B0 decays to K_0*0 pi0
  if (GenB0Tag::PcheckDecay(genpart, 10311, -311)) {
    return +1 * (100000 * m_nPhotos + 1179);
  }//B0 decays to K_0*0 anti-K0
  if (GenB0Tag::PcheckDecay(genpart, 30313, 111)) {
    return +1 * (100000 * m_nPhotos + 1180);
  }//B0 decays to K''*0 pi0
  if (GenB0Tag::PcheckDecay(genpart, 30313, 311)) {
    return +1 * (100000 * m_nPhotos + 1181);
  }//B0 decays to K''*0 K0
  if (GenB0Tag::PcheckDecay(genpart, 323, -211)) {
    return +1 * (100000 * m_nPhotos + 1182);
  }//B0 decays to K*+ pi-
  if (GenB0Tag::PcheckDecay(genpart, 323, -321)) {
    return +1 * (100000 * m_nPhotos + 1183);
  }//B0 decays to K*+ K-
  if (GenB0Tag::PcheckDecay(genpart, 10321, -211)) {
    return +1 * (100000 * m_nPhotos + 1184);
  }//B0 decays to K_0*+ pi-
  if (GenB0Tag::PcheckDecay(genpart, 10321, -321)) {
    return +1 * (100000 * m_nPhotos + 1185);
  }//B0 decays to K_0*+ K-
  if (GenB0Tag::PcheckDecay(genpart, 30323, -211)) {
    return +1 * (100000 * m_nPhotos + 1186);
  }//B0 decays to K''*+ pi-
  if (GenB0Tag::PcheckDecay(genpart, 30323, -321)) {
    return +1 * (100000 * m_nPhotos + 1187);
  }//B0 decays to K''*+ K-
  if (GenB0Tag::PcheckDecay(genpart, 211, -211, 221)) {
    return +1 * (100000 * m_nPhotos + 1188);
  }//B0 decays to pi+ pi- eta
  if (GenB0Tag::PcheckDecay(genpart, 211, -211, 331)) {
    return +1 * (100000 * m_nPhotos + 1189);
  }//B0 decays to pi+ pi- eta'
  if (GenB0Tag::PcheckDecay(genpart, 211, -211, 311)) {
    return +1 * (100000 * m_nPhotos + 1190);
  }//B0 decays to pi+ pi- K0
  if (GenB0Tag::PcheckDecay(genpart, 321, -211, 111)) {
    return +1 * (100000 * m_nPhotos + 1191);
  }//B0 decays to K+ pi- pi0
  if (GenB0Tag::PcheckDecay(genpart, 321, -211, 221)) {
    return +1 * (100000 * m_nPhotos + 1192);
  }//B0 decays to K+ pi- eta
  if (GenB0Tag::PcheckDecay(genpart, 321, -211, 331)) {
    return +1 * (100000 * m_nPhotos + 1193);
  }//B0 decays to K+ pi- eta'
  if (GenB0Tag::PcheckDecay(genpart, 211, -321, 311)) {
    return +1 * (100000 * m_nPhotos + 1194);
  }//B0 decays to pi+ K- K0
  if (GenB0Tag::PcheckDecay(genpart, -211, 321, -311)) {
    return +1 * (100000 * m_nPhotos + 1195);
  }//B0 decays to pi- K+ anti-K0
  if (GenB0Tag::PcheckDecay(genpart, 321, -321, 311)) {
    return +1 * (100000 * m_nPhotos + 1196);
  }//B0 decays to K+ K- K0
  if (GenB0Tag::PcheckDecay(genpart, 321, -321, 310)) {
    return +1 * (100000 * m_nPhotos + 1197);
  }//B0 decays to K+ K- K_S0
  if (GenB0Tag::PcheckDecay(genpart, 321, -321, 130)) {
    return +1 * (100000 * m_nPhotos + 1198);
  }//B0 decays to K+ K- K_L0
  if (GenB0Tag::PcheckDecay(genpart, 321, -321, 111)) {
    return +1 * (100000 * m_nPhotos + 1199);
  }//B0 decays to K+ K- pi0
  if (GenB0Tag::PcheckDecay(genpart, 321, -321, 221)) {
    return +1 * (100000 * m_nPhotos + 1200);
  }//B0 decays to K+ K- eta
  if (GenB0Tag::PcheckDecay(genpart, 321, -321, 331)) {
    return +1 * (100000 * m_nPhotos + 1201);
  }//B0 decays to K+ K- eta'
  if (GenB0Tag::PcheckDecay(genpart, 311, -311, 111)) {
    return +1 * (100000 * m_nPhotos + 1202);
  }//B0 decays to K0 anti-K0 pi0
  if (GenB0Tag::PcheckDecay(genpart, 311, -311, 221)) {
    return +1 * (100000 * m_nPhotos + 1203);
  }//B0 decays to K0 anti-K0 eta
  if (GenB0Tag::PcheckDecay(genpart, 311, -311, 331)) {
    return +1 * (100000 * m_nPhotos + 1204);
  }//B0 decays to K0 anti-K0 eta'
  if (GenB0Tag::PcheckDecay(genpart, 311, 111, 111)) {
    return +1 * (100000 * m_nPhotos + 1205);
  }//B0 decays to K0 pi0 pi0
  if (GenB0Tag::PcheckDecay(genpart, 311, 111, 221)) {
    return +1 * (100000 * m_nPhotos + 1206);
  }//B0 decays to K0 pi0 eta
  if (GenB0Tag::PcheckDecay(genpart, 311, 111, 331)) {
    return +1 * (100000 * m_nPhotos + 1207);
  }//B0 decays to K0 pi0 eta'
  if (GenB0Tag::PcheckDecay(genpart, 311, 221, 221)) {
    return +1 * (100000 * m_nPhotos + 1208);
  }//B0 decays to K0 eta eta
  if (GenB0Tag::PcheckDecay(genpart, 311, 221, 331)) {
    return +1 * (100000 * m_nPhotos + 1209);
  }//B0 decays to K0 eta eta'
  if (GenB0Tag::PcheckDecay(genpart, 311, 331, 331)) {
    return +1 * (100000 * m_nPhotos + 1210);
  }//B0 decays to K0 eta' eta'
  if (GenB0Tag::PcheckDecay(genpart, 311, -311, 311)) {
    return +1 * (100000 * m_nPhotos + 1211);
  }//B0 decays to K0 anti-K0 K0
  if (GenB0Tag::PcheckDecay(genpart, 111, 111, 111)) {
    return +1 * (100000 * m_nPhotos + 1212);
  }//B0 decays to pi0 pi0 pi0
  if (GenB0Tag::PcheckDecay(genpart, 111, 111, 221)) {
    return +1 * (100000 * m_nPhotos + 1213);
  }//B0 decays to pi0 pi0 eta
  if (GenB0Tag::PcheckDecay(genpart, 111, 111, 331)) {
    return +1 * (100000 * m_nPhotos + 1214);
  }//B0 decays to pi0 pi0 eta'
  if (GenB0Tag::PcheckDecay(genpart, 111, 221, 221)) {
    return +1 * (100000 * m_nPhotos + 1215);
  }//B0 decays to pi0 eta eta
  if (GenB0Tag::PcheckDecay(genpart, 111, 221, 331)) {
    return +1 * (100000 * m_nPhotos + 1216);
  }//B0 decays to pi0 eta eta'
  if (GenB0Tag::PcheckDecay(genpart, 313, 111, 111)) {
    return +1 * (100000 * m_nPhotos + 1217);
  }//B0 decays to K*0 pi0 pi0
  if (GenB0Tag::PcheckDecay(genpart, 113, 113)) {
    return +1 * (100000 * m_nPhotos + 1218);
  }//B0 decays to rho0 rho0
  if (GenB0Tag::PcheckDecay(genpart, 213, -213)) {
    return +1 * (100000 * m_nPhotos + 1219);
  }//B0 decays to rho+ rho-
  if (GenB0Tag::PcheckDecay(genpart, 113, 211, -211)) {
    return +1 * (100000 * m_nPhotos + 1220);
  }//B0 decays to rho0 pi+ pi-
  if (GenB0Tag::PcheckDecay(genpart, 223, 211, -211)) {
    return +1 * (100000 * m_nPhotos + 1221);
  }//B0 decays to omega pi+ pi-
  if (GenB0Tag::PcheckDecay(genpart, 113, 111, 111)) {
    return +1 * (100000 * m_nPhotos + 1222);
  }//B0 decays to rho0 pi0 pi0
  if (GenB0Tag::PcheckDecay(genpart, 223, 111, 111)) {
    return +1 * (100000 * m_nPhotos + 1223);
  }//B0 decays to omega pi0 pi0
  if (GenB0Tag::PcheckDecay(genpart, 113, 111, 221)) {
    return +1 * (100000 * m_nPhotos + 1224);
  }//B0 decays to rho0 pi0 eta
  if (GenB0Tag::PcheckDecay(genpart, 113, 221, 221)) {
    return +1 * (100000 * m_nPhotos + 1225);
  }//B0 decays to rho0 eta eta
  if (GenB0Tag::PcheckDecay(genpart, 113, 111, 331)) {
    return +1 * (100000 * m_nPhotos + 1226);
  }//B0 decays to rho0 pi0 eta'
  if (GenB0Tag::PcheckDecay(genpart, 113, 221, 331)) {
    return +1 * (100000 * m_nPhotos + 1227);
  }//B0 decays to rho0 eta eta'
  if (GenB0Tag::PcheckDecay(genpart, 223, 111, 221)) {
    return +1 * (100000 * m_nPhotos + 1228);
  }//B0 decays to omega pi0 eta
  if (GenB0Tag::PcheckDecay(genpart, 223, 221, 221)) {
    return +1 * (100000 * m_nPhotos + 1229);
  }//B0 decays to omega eta eta
  if (GenB0Tag::PcheckDecay(genpart, 223, 111, 331)) {
    return +1 * (100000 * m_nPhotos + 1230);
  }//B0 decays to omega pi0 eta'
  if (GenB0Tag::PcheckDecay(genpart, 223, 221, 331)) {
    return +1 * (100000 * m_nPhotos + 1231);
  }//B0 decays to omega eta eta'
  if (GenB0Tag::PcheckDecay(genpart, 213, -211, 111)) {
    return +1 * (100000 * m_nPhotos + 1232);
  }//B0 decays to rho+ pi- pi0
  if (GenB0Tag::PcheckDecay(genpart, 213, -211, 221)) {
    return +1 * (100000 * m_nPhotos + 1233);
  }//B0 decays to rho+ pi- eta
  if (GenB0Tag::PcheckDecay(genpart, -213, 211, 111)) {
    return +1 * (100000 * m_nPhotos + 1234);
  }//B0 decays to rho- pi+ pi0
  if (GenB0Tag::PcheckDecay(genpart, -213, 211, 221)) {
    return +1 * (100000 * m_nPhotos + 1235);
  }//B0 decays to rho- pi+ eta
  if (GenB0Tag::PcheckDecay(genpart, 211, -211, 211, -211)) {
    return +1 * (100000 * m_nPhotos + 1236);
  }//B0 decays to pi+ pi- pi+ pi-
  if (GenB0Tag::PcheckDecay(genpart, 211, -211, 111, 111)) {
    return +1 * (100000 * m_nPhotos + 1237);
  }//B0 decays to pi+ pi- pi0 pi0
  if (GenB0Tag::PcheckDecay(genpart, 211, -211, 221, 111)) {
    return +1 * (100000 * m_nPhotos + 1238);
  }//B0 decays to pi+ pi- eta pi0
  if (GenB0Tag::PcheckDecay(genpart, 211, -211, 221, 221)) {
    return +1 * (100000 * m_nPhotos + 1239);
  }//B0 decays to pi+ pi- eta eta
  if (GenB0Tag::PcheckDecay(genpart, 211, -211, 331, 111)) {
    return +1 * (100000 * m_nPhotos + 1240);
  }//B0 decays to pi+ pi- eta' pi0
  if (GenB0Tag::PcheckDecay(genpart, 211, -211, 331, 221)) {
    return +1 * (100000 * m_nPhotos + 1241);
  }//B0 decays to pi+ pi- eta' eta
  if (GenB0Tag::PcheckDecay(genpart, 20213, -211)) {
    return +1 * (100000 * m_nPhotos + 1242);
  }//B0 decays to a_1+ pi-
  if (GenB0Tag::PcheckDecay(genpart, -20213, 211)) {
    return +1 * (100000 * m_nPhotos + 1243);
  }//B0 decays to a_1- pi+
  if (GenB0Tag::PcheckDecay(genpart, 10213, -211)) {
    return +1 * (100000 * m_nPhotos + 1244);
  }//B0 decays to b_1+ pi-
  if (GenB0Tag::PcheckDecay(genpart, -10213, 211)) {
    return +1 * (100000 * m_nPhotos + 1245);
  }//B0 decays to b_1- pi+
  if (GenB0Tag::PcheckDecay(genpart, -213, 10211)) {
    return +1 * (100000 * m_nPhotos + 1246);
  }//B0 decays to rho- a_0+
  if (GenB0Tag::PcheckDecay(genpart, 213, -10211)) {
    return +1 * (100000 * m_nPhotos + 1247);
  }//B0 decays to rho+ a_0-
  if (GenB0Tag::PcheckDecay(genpart, 10221, 211, -211)) {
    return +1 * (100000 * m_nPhotos + 1248);
  }//B0 decays to f_0 pi+ pi-
  if (GenB0Tag::PcheckDecay(genpart, 10221, 111, 111)) {
    return +1 * (100000 * m_nPhotos + 1249);
  }//B0 decays to f_0 pi0 pi0
  if (GenB0Tag::PcheckDecay(genpart, -10211, 211, 111)) {
    return +1 * (100000 * m_nPhotos + 1250);
  }//B0 decays to a_0- pi+ pi0
  if (GenB0Tag::PcheckDecay(genpart, 10111, 111, 111)) {
    return +1 * (100000 * m_nPhotos + 1251);
  }//B0 decays to a_00 pi0 pi0
  if (GenB0Tag::PcheckDecay(genpart, 10211, -211, 111)) {
    return +1 * (100000 * m_nPhotos + 1252);
  }//B0 decays to a_0+ pi- pi0
  if (GenB0Tag::PcheckDecay(genpart, 113, 313)) {
    return +1 * (100000 * m_nPhotos + 1253);
  }//B0 decays to rho0 K*0
  if (GenB0Tag::PcheckDecay(genpart, -213, 323)) {
    return +1 * (100000 * m_nPhotos + 1254);
  }//B0 decays to rho- K*+
  if (GenB0Tag::PcheckDecay(genpart, -211, 211, 313)) {
    return +1 * (100000 * m_nPhotos + 1255);
  }//B0 decays to pi- pi+ K*0
  if (GenB0Tag::PcheckDecay(genpart, -211, 111, 323)) {
    return +1 * (100000 * m_nPhotos + 1256);
  }//B0 decays to pi- pi0 K*+
  if (GenB0Tag::PcheckDecay(genpart, -211, 221, 323)) {
    return +1 * (100000 * m_nPhotos + 1257);
  }//B0 decays to pi- eta K*+
  if (GenB0Tag::PcheckDecay(genpart, -211, 331, 323)) {
    return +1 * (100000 * m_nPhotos + 1258);
  }//B0 decays to pi- eta' K*+
  if (GenB0Tag::PcheckDecay(genpart, -213, 321, 111)) {
    return +1 * (100000 * m_nPhotos + 1259);
  }//B0 decays to rho- K+ pi0
  if (GenB0Tag::PcheckDecay(genpart, -213, 321, 221)) {
    return +1 * (100000 * m_nPhotos + 1260);
  }//B0 decays to rho- K+ eta
  if (GenB0Tag::PcheckDecay(genpart, -213, 321, 331)) {
    return +1 * (100000 * m_nPhotos + 1261);
  }//B0 decays to rho- K+ eta'
  if (GenB0Tag::PcheckDecay(genpart, -213, 311, 211)) {
    return +1 * (100000 * m_nPhotos + 1262);
  }//B0 decays to rho- K0 pi+
  if (GenB0Tag::PcheckDecay(genpart, 213, 311, -211)) {
    return +1 * (100000 * m_nPhotos + 1263);
  }//B0 decays to rho+ K0 pi-
  if (GenB0Tag::PcheckDecay(genpart, 113, 311, 111)) {
    return +1 * (100000 * m_nPhotos + 1264);
  }//B0 decays to rho0 K0 pi0
  if (GenB0Tag::PcheckDecay(genpart, 113, 311, 221)) {
    return +1 * (100000 * m_nPhotos + 1265);
  }//B0 decays to rho0 K0 eta
  if (GenB0Tag::PcheckDecay(genpart, 113, 311, 331)) {
    return +1 * (100000 * m_nPhotos + 1266);
  }//B0 decays to rho0 K0 eta'
  if (GenB0Tag::PcheckDecay(genpart, 113, 321, -211)) {
    return +1 * (100000 * m_nPhotos + 1267);
  }//B0 decays to rho0 K+ pi-
  if (GenB0Tag::PcheckDecay(genpart, 223, 311, 111)) {
    return +1 * (100000 * m_nPhotos + 1268);
  }//B0 decays to omega K0 pi0
  if (GenB0Tag::PcheckDecay(genpart, 223, 311, 221)) {
    return +1 * (100000 * m_nPhotos + 1269);
  }//B0 decays to omega K0 eta
  if (GenB0Tag::PcheckDecay(genpart, 223, 311, 331)) {
    return +1 * (100000 * m_nPhotos + 1270);
  }//B0 decays to omega K0 eta'
  if (GenB0Tag::PcheckDecay(genpart, 223, 321, -211)) {
    return +1 * (100000 * m_nPhotos + 1271);
  }//B0 decays to omega K+ pi-
  if (GenB0Tag::PcheckDecay(genpart, 211, 111, -211, 311)) {
    return +1 * (100000 * m_nPhotos + 1272);
  }//B0 decays to pi+ pi0 pi- K0
  if (GenB0Tag::PcheckDecay(genpart, 211, 221, -211, 311)) {
    return +1 * (100000 * m_nPhotos + 1273);
  }//B0 decays to pi+ eta pi- K0
  if (GenB0Tag::PcheckDecay(genpart, 111, 111, 111, 311)) {
    return +1 * (100000 * m_nPhotos + 1274);
  }//B0 decays to pi0 pi0 pi0 K0
  if (GenB0Tag::PcheckDecay(genpart, 111, 111, 221, 311)) {
    return +1 * (100000 * m_nPhotos + 1275);
  }//B0 decays to pi0 pi0 eta K0
  if (GenB0Tag::PcheckDecay(genpart, 111, 221, 221, 311)) {
    return +1 * (100000 * m_nPhotos + 1276);
  }//B0 decays to pi0 eta eta K0
  if (GenB0Tag::PcheckDecay(genpart, 111, 111, 331, 311)) {
    return +1 * (100000 * m_nPhotos + 1277);
  }//B0 decays to pi0 pi0 eta' K0
  if (GenB0Tag::PcheckDecay(genpart, 111, 221, 331, 311)) {
    return +1 * (100000 * m_nPhotos + 1278);
  }//B0 decays to pi0 eta eta' K0
  if (GenB0Tag::PcheckDecay(genpart, 211, -211, -211, 321)) {
    return +1 * (100000 * m_nPhotos + 1279);
  }//B0 decays to pi+ pi- pi- K+
  if (GenB0Tag::PcheckDecay(genpart, 111, 111, -211, 321)) {
    return +1 * (100000 * m_nPhotos + 1280);
  }//B0 decays to pi0 pi0 pi- K+
  if (GenB0Tag::PcheckDecay(genpart, 111, 221, -211, 321)) {
    return +1 * (100000 * m_nPhotos + 1281);
  }//B0 decays to pi0 eta pi- K+
  if (GenB0Tag::PcheckDecay(genpart, 221, 221, -211, 321)) {
    return +1 * (100000 * m_nPhotos + 1282);
  }//B0 decays to eta eta pi- K+
  if (GenB0Tag::PcheckDecay(genpart, 111, 331, -211, 321)) {
    return +1 * (100000 * m_nPhotos + 1283);
  }//B0 decays to pi0 eta' pi- K+
  if (GenB0Tag::PcheckDecay(genpart, 221, 331, -211, 321)) {
    return +1 * (100000 * m_nPhotos + 1284);
  }//B0 decays to eta eta' pi- K+
  if (GenB0Tag::PcheckDecay(genpart, 113, 10311)) {
    return +1 * (100000 * m_nPhotos + 1285);
  }//B0 decays to rho0 K_0*0
  if (GenB0Tag::PcheckDecay(genpart, -213, 10321)) {
    return +1 * (100000 * m_nPhotos + 1286);
  }//B0 decays to rho- K_0*+
  if (GenB0Tag::PcheckDecay(genpart, -211, 211, 10311)) {
    return +1 * (100000 * m_nPhotos + 1287);
  }//B0 decays to pi- pi+ K_0*0
  if (GenB0Tag::PcheckDecay(genpart, 111, 111, 10311)) {
    return +1 * (100000 * m_nPhotos + 1288);
  }//B0 decays to pi0 pi0 K_0*0
  if (GenB0Tag::PcheckDecay(genpart, -211, 111, 10321)) {
    return +1 * (100000 * m_nPhotos + 1289);
  }//B0 decays to pi- pi0 K_0*+
  if (GenB0Tag::PcheckDecay(genpart, 313, 10221)) {
    return +1 * (100000 * m_nPhotos + 1290);
  }//B0 decays to K*0 f_0
  if (GenB0Tag::PcheckDecay(genpart, 20113, 311)) {
    return +1 * (100000 * m_nPhotos + 1291);
  }//B0 decays to a_10 K0
  if (GenB0Tag::PcheckDecay(genpart, -20213, 321)) {
    return +1 * (100000 * m_nPhotos + 1292);
  }//B0 decays to a_1- K+
  if (GenB0Tag::PcheckDecay(genpart, 10113, 311)) {
    return +1 * (100000 * m_nPhotos + 1293);
  }//B0 decays to b_10 K0
  if (GenB0Tag::PcheckDecay(genpart, -10213, 321)) {
    return +1 * (100000 * m_nPhotos + 1294);
  }//B0 decays to b_1- K+
  if (GenB0Tag::PcheckDecay(genpart, 313, 10111)) {
    return +1 * (100000 * m_nPhotos + 1295);
  }//B0 decays to K*0 a_00
  if (GenB0Tag::PcheckDecay(genpart, 323, -10211)) {
    return +1 * (100000 * m_nPhotos + 1296);
  }//B0 decays to K*+ a_0-
  if (GenB0Tag::PcheckDecay(genpart, 313, -313)) {
    return +1 * (100000 * m_nPhotos + 1297);
  }//B0 decays to K*0 anti-K*0
  if (GenB0Tag::PcheckDecay(genpart, 323, -323)) {
    return +1 * (100000 * m_nPhotos + 1298);
  }//B0 decays to K*+ K*-
  if (GenB0Tag::PcheckDecay(genpart, 333, 113)) {
    return +1 * (100000 * m_nPhotos + 1299);
  }//B0 decays to phi rho0
  if (GenB0Tag::PcheckDecay(genpart, 333, -211, 211)) {
    return +1 * (100000 * m_nPhotos + 1300);
  }//B0 decays to phi pi- pi+
  if (GenB0Tag::PcheckDecay(genpart, 321, -321, 113)) {
    return +1 * (100000 * m_nPhotos + 1301);
  }//B0 decays to K+ K- rho0
  if (GenB0Tag::PcheckDecay(genpart, 311, -311, 113)) {
    return +1 * (100000 * m_nPhotos + 1302);
  }//B0 decays to K0 anti-K0 rho0
  if (GenB0Tag::PcheckDecay(genpart, 321, -321, 223)) {
    return +1 * (100000 * m_nPhotos + 1303);
  }//B0 decays to K+ K- omega
  if (GenB0Tag::PcheckDecay(genpart, 311, -311, 223)) {
    return +1 * (100000 * m_nPhotos + 1304);
  }//B0 decays to K0 anti-K0 omega
  if (GenB0Tag::PcheckDecay(genpart, 321, -311, -213)) {
    return +1 * (100000 * m_nPhotos + 1305);
  }//B0 decays to K+ anti-K0 rho-
  if (GenB0Tag::PcheckDecay(genpart, -321, 311, 213)) {
    return +1 * (100000 * m_nPhotos + 1306);
  }//B0 decays to K- K0 rho+
  if (GenB0Tag::PcheckDecay(genpart, 321, -321, -211, 211)) {
    return +1 * (100000 * m_nPhotos + 1307);
  }//B0 decays to K+ K- pi- pi+
  if (GenB0Tag::PcheckDecay(genpart, 321, -321, 111, 111)) {
    return +1 * (100000 * m_nPhotos + 1308);
  }//B0 decays to K+ K- pi0 pi0
  if (GenB0Tag::PcheckDecay(genpart, 321, -321, 111, 221)) {
    return +1 * (100000 * m_nPhotos + 1309);
  }//B0 decays to K+ K- pi0 eta
  if (GenB0Tag::PcheckDecay(genpart, 321, -321, 221, 221)) {
    return +1 * (100000 * m_nPhotos + 1310);
  }//B0 decays to K+ K- eta eta
  if (GenB0Tag::PcheckDecay(genpart, 321, -321, 111, 331)) {
    return +1 * (100000 * m_nPhotos + 1311);
  }//B0 decays to K+ K- pi0 eta'
  if (GenB0Tag::PcheckDecay(genpart, 321, -321, 221, 331)) {
    return +1 * (100000 * m_nPhotos + 1312);
  }//B0 decays to K+ K- eta eta'
  if (GenB0Tag::PcheckDecay(genpart, 311, -311, -211, 211)) {
    return +1 * (100000 * m_nPhotos + 1313);
  }//B0 decays to K0 anti-K0 pi- pi+
  if (GenB0Tag::PcheckDecay(genpart, 311, -311, 111, 111)) {
    return +1 * (100000 * m_nPhotos + 1314);
  }//B0 decays to K0 anti-K0 pi0 pi0
  if (GenB0Tag::PcheckDecay(genpart, 311, -311, 111, 221)) {
    return +1 * (100000 * m_nPhotos + 1315);
  }//B0 decays to K0 anti-K0 pi0 eta
  if (GenB0Tag::PcheckDecay(genpart, 311, -311, 221, 221)) {
    return +1 * (100000 * m_nPhotos + 1316);
  }//B0 decays to K0 anti-K0 eta eta
  if (GenB0Tag::PcheckDecay(genpart, 311, -311, 111, 331)) {
    return +1 * (100000 * m_nPhotos + 1317);
  }//B0 decays to K0 anti-K0 pi0 eta'
  if (GenB0Tag::PcheckDecay(genpart, 311, -311, 221, 331)) {
    return +1 * (100000 * m_nPhotos + 1318);
  }//B0 decays to K0 anti-K0 eta eta'
  if (GenB0Tag::PcheckDecay(genpart, 323, -321, 111)) {
    return +1 * (100000 * m_nPhotos + 1319);
  }//B0 decays to K*+ K- pi0
  if (GenB0Tag::PcheckDecay(genpart, -323, 321, 111)) {
    return +1 * (100000 * m_nPhotos + 1320);
  }//B0 decays to K*- K+ pi0
  if (GenB0Tag::PcheckDecay(genpart, 313, -311, 111)) {
    return +1 * (100000 * m_nPhotos + 1321);
  }//B0 decays to K*0 anti-K0 pi0
  if (GenB0Tag::PcheckDecay(genpart, 311, -313, 111)) {
    return +1 * (100000 * m_nPhotos + 1322);
  }//B0 decays to K0 anti-K*0 pi0
  if (GenB0Tag::PcheckDecay(genpart, 321, -313, -211)) {
    return +1 * (100000 * m_nPhotos + 1323);
  }//B0 decays to K+ anti-K*0 pi-
  if (GenB0Tag::PcheckDecay(genpart, 323, -311, -211)) {
    return +1 * (100000 * m_nPhotos + 1324);
  }//B0 decays to K*+ anti-K0 pi-
  if (GenB0Tag::PcheckDecay(genpart, -321, 313, 211)) {
    return +1 * (100000 * m_nPhotos + 1325);
  }//B0 decays to K- K*0 pi+
  if (GenB0Tag::PcheckDecay(genpart, -323, 311, 211)) {
    return +1 * (100000 * m_nPhotos + 1326);
  }//B0 decays to K*- K0 pi+
  if (GenB0Tag::PcheckDecay(genpart, 333, 313)) {
    return +1 * (100000 * m_nPhotos + 1327);
  }//B0 decays to phi K*0
  if (GenB0Tag::PcheckDecay(genpart, 333, 321, -211)) {
    return +1 * (100000 * m_nPhotos + 1328);
  }//B0 decays to phi K+ pi-
  if (GenB0Tag::PcheckDecay(genpart, 333, 311, 111)) {
    return +1 * (100000 * m_nPhotos + 1329);
  }//B0 decays to phi K0 pi0
  if (GenB0Tag::PcheckDecay(genpart, 333, 311, 221)) {
    return +1 * (100000 * m_nPhotos + 1330);
  }//B0 decays to phi K0 eta
  if (GenB0Tag::PcheckDecay(genpart, -321, 321, 313)) {
    return +1 * (100000 * m_nPhotos + 1331);
  }//B0 decays to K- K+ K*0
  if (GenB0Tag::PcheckDecay(genpart, -321, 323, 311)) {
    return +1 * (100000 * m_nPhotos + 1332);
  }//B0 decays to K- K*+ K0
  if (GenB0Tag::PcheckDecay(genpart, -323, 321, 311)) {
    return +1 * (100000 * m_nPhotos + 1333);
  }//B0 decays to K*- K+ K0
  if (GenB0Tag::PcheckDecay(genpart, 321, -321, 321, -211)) {
    return +1 * (100000 * m_nPhotos + 1334);
  }//B0 decays to K+ K- K+ pi-
  if (GenB0Tag::PcheckDecay(genpart, 321, -321, 311, 111)) {
    return +1 * (100000 * m_nPhotos + 1335);
  }//B0 decays to K+ K- K0 pi0
  if (GenB0Tag::PcheckDecay(genpart, 321, -321, 311, 221)) {
    return +1 * (100000 * m_nPhotos + 1336);
  }//B0 decays to K+ K- K0 eta
  if (GenB0Tag::PcheckDecay(genpart, 311, -311, 311, 111)) {
    return +1 * (100000 * m_nPhotos + 1337);
  }//B0 decays to K0 anti-K0 K0 pi0
  if (GenB0Tag::PcheckDecay(genpart, 311, -311, 311, 221)) {
    return +1 * (100000 * m_nPhotos + 1338);
  }//B0 decays to K0 anti-K0 K0 eta
  if (GenB0Tag::PcheckDecay(genpart, 333, 10311)) {
    return +1 * (100000 * m_nPhotos + 1339);
  }//B0 decays to phi K_0*0
  if (GenB0Tag::PcheckDecay(genpart, -321, 321, 10311)) {
    return +1 * (100000 * m_nPhotos + 1340);
  }//B0 decays to K- K+ K_0*0
  if (GenB0Tag::PcheckDecay(genpart, -321, 10321, 311)) {
    return +1 * (100000 * m_nPhotos + 1341);
  }//B0 decays to K- K_0*+ K0
  if (GenB0Tag::PcheckDecay(genpart, -10321, 321, 311)) {
    return +1 * (100000 * m_nPhotos + 1342);
  }//B0 decays to K_0*- K+ K0
  if (GenB0Tag::PcheckDecay(genpart, 333, 20313)) {
    return +1 * (100000 * m_nPhotos + 1343);
  }//B0 decays to phi K'_10
  if (GenB0Tag::PcheckDecay(genpart, -321, 321, 20313)) {
    return +1 * (100000 * m_nPhotos + 1344);
  }//B0 decays to K- K+ K'_10
  if (GenB0Tag::PcheckDecay(genpart, 333, 333)) {
    return +1 * (100000 * m_nPhotos + 1345);
  }//B0 decays to phi phi
  if (GenB0Tag::PcheckDecay(genpart, 333, 321, -321)) {
    return +1 * (100000 * m_nPhotos + 1346);
  }//B0 decays to phi K+ K-
  if (GenB0Tag::PcheckDecay(genpart, 333, 311, -311)) {
    return +1 * (100000 * m_nPhotos + 1347);
  }//B0 decays to phi K0 anti-K0
  if (GenB0Tag::PcheckDecay(genpart, 321, -321, 321, -321)) {
    return +1 * (100000 * m_nPhotos + 1348);
  }//B0 decays to K+ K- K+ K-
  if (GenB0Tag::PcheckDecay(genpart, 311, -311, 321, -321)) {
    return +1 * (100000 * m_nPhotos + 1349);
  }//B0 decays to K0 anti-K0 K+ K-
  if (GenB0Tag::PcheckDecay(genpart, 311, -311, 311, -311)) {
    return +1 * (100000 * m_nPhotos + 1350);
  }//B0 decays to K0 anti-K0 K0 anti-K0
  if (GenB0Tag::PcheckDecay(genpart, 113, 113, 111)) {
    return +1 * (100000 * m_nPhotos + 1351);
  }//B0 decays to rho0 rho0 pi0
  if (GenB0Tag::PcheckDecay(genpart, 113, 113, 221)) {
    return +1 * (100000 * m_nPhotos + 1352);
  }//B0 decays to rho0 rho0 eta
  if (GenB0Tag::PcheckDecay(genpart, 113, 113, 331)) {
    return +1 * (100000 * m_nPhotos + 1353);
  }//B0 decays to rho0 rho0 eta'
  if (GenB0Tag::PcheckDecay(genpart, 223, 113, 111)) {
    return +1 * (100000 * m_nPhotos + 1354);
  }//B0 decays to omega rho0 pi0
  if (GenB0Tag::PcheckDecay(genpart, 223, 113, 221)) {
    return +1 * (100000 * m_nPhotos + 1355);
  }//B0 decays to omega rho0 eta
  if (GenB0Tag::PcheckDecay(genpart, 223, 113, 331)) {
    return +1 * (100000 * m_nPhotos + 1356);
  }//B0 decays to omega rho0 eta'
  if (GenB0Tag::PcheckDecay(genpart, 213, -213, 111)) {
    return +1 * (100000 * m_nPhotos + 1357);
  }//B0 decays to rho+ rho- pi0
  if (GenB0Tag::PcheckDecay(genpart, 213, -213, 221)) {
    return +1 * (100000 * m_nPhotos + 1358);
  }//B0 decays to rho+ rho- eta
  if (GenB0Tag::PcheckDecay(genpart, 213, -213, 331)) {
    return +1 * (100000 * m_nPhotos + 1359);
  }//B0 decays to rho+ rho- eta'
  if (GenB0Tag::PcheckDecay(genpart, 113, -213, 211)) {
    return +1 * (100000 * m_nPhotos + 1360);
  }//B0 decays to rho0 rho- pi+
  if (GenB0Tag::PcheckDecay(genpart, 223, -213, 211)) {
    return +1 * (100000 * m_nPhotos + 1361);
  }//B0 decays to omega rho- pi+
  if (GenB0Tag::PcheckDecay(genpart, 113, 211, -211, 111)) {
    return +1 * (100000 * m_nPhotos + 1362);
  }//B0 decays to rho0 pi+ pi- pi0
  if (GenB0Tag::PcheckDecay(genpart, 113, 211, -211, 221)) {
    return +1 * (100000 * m_nPhotos + 1363);
  }//B0 decays to rho0 pi+ pi- eta
  if (GenB0Tag::PcheckDecay(genpart, 113, 211, -211, 331)) {
    return +1 * (100000 * m_nPhotos + 1364);
  }//B0 decays to rho0 pi+ pi- eta'
  if (GenB0Tag::PcheckDecay(genpart, 113, 111, 111, 111)) {
    return +1 * (100000 * m_nPhotos + 1365);
  }//B0 decays to rho0 pi0 pi0 pi0
  if (GenB0Tag::PcheckDecay(genpart, 113, 111, 221, 111)) {
    return +1 * (100000 * m_nPhotos + 1366);
  }//B0 decays to rho0 pi0 eta pi0
  if (GenB0Tag::PcheckDecay(genpart, 113, 111, 331, 111)) {
    return +1 * (100000 * m_nPhotos + 1367);
  }//B0 decays to rho0 pi0 eta' pi0
  if (GenB0Tag::PcheckDecay(genpart, 223, 211, -211, 111)) {
    return +1 * (100000 * m_nPhotos + 1368);
  }//B0 decays to omega pi+ pi- pi0
  if (GenB0Tag::PcheckDecay(genpart, 223, 211, -211, 221)) {
    return +1 * (100000 * m_nPhotos + 1369);
  }//B0 decays to omega pi+ pi- eta
  if (GenB0Tag::PcheckDecay(genpart, 223, 211, -211, 331)) {
    return +1 * (100000 * m_nPhotos + 1370);
  }//B0 decays to omega pi+ pi- eta'
  if (GenB0Tag::PcheckDecay(genpart, 223, 111, 111, 111)) {
    return +1 * (100000 * m_nPhotos + 1371);
  }//B0 decays to omega pi0 pi0 pi0
  if (GenB0Tag::PcheckDecay(genpart, 223, 111, 221, 111)) {
    return +1 * (100000 * m_nPhotos + 1372);
  }//B0 decays to omega pi0 eta pi0
  if (GenB0Tag::PcheckDecay(genpart, 223, 111, 331, 111)) {
    return +1 * (100000 * m_nPhotos + 1373);
  }//B0 decays to omega pi0 eta' pi0
  if (GenB0Tag::PcheckDecay(genpart, 213, -211, 211, -211)) {
    return +1 * (100000 * m_nPhotos + 1374);
  }//B0 decays to rho+ pi- pi+ pi-
  if (GenB0Tag::PcheckDecay(genpart, 213, -211, 111, 111)) {
    return +1 * (100000 * m_nPhotos + 1375);
  }//B0 decays to rho+ pi- pi0 pi0
  if (GenB0Tag::PcheckDecay(genpart, 213, -211, 111, 221)) {
    return +1 * (100000 * m_nPhotos + 1376);
  }//B0 decays to rho+ pi- pi0 eta
  if (GenB0Tag::PcheckDecay(genpart, 213, -211, 111, 331)) {
    return +1 * (100000 * m_nPhotos + 1377);
  }//B0 decays to rho+ pi- pi0 eta'
  if (GenB0Tag::PcheckDecay(genpart, -213, 211, 211, -211)) {
    return +1 * (100000 * m_nPhotos + 1378);
  }//B0 decays to rho- pi+ pi+ pi-
  if (GenB0Tag::PcheckDecay(genpart, -213, 211, 111, 111)) {
    return +1 * (100000 * m_nPhotos + 1379);
  }//B0 decays to rho- pi+ pi0 pi0
  if (GenB0Tag::PcheckDecay(genpart, -213, 211, 111, 221)) {
    return +1 * (100000 * m_nPhotos + 1380);
  }//B0 decays to rho- pi+ pi0 eta
  if (GenB0Tag::PcheckDecay(genpart, -213, 211, 111, 331)) {
    return +1 * (100000 * m_nPhotos + 1381);
  }//B0 decays to rho- pi+ pi0 eta'
  if (GenB0Tag::PcheckDecay(genpart, 211, -211, 211, -211, 111)) {
    return +1 * (100000 * m_nPhotos + 1382);
  }//B0 decays to pi+ pi- pi+ pi- pi0
  if (GenB0Tag::PcheckDecay(genpart, 211, -211, 211, -211, 221)) {
    return +1 * (100000 * m_nPhotos + 1383);
  }//B0 decays to pi+ pi- pi+ pi- eta
  if (GenB0Tag::PcheckDecay(genpart, 211, -211, 211, -211, 331)) {
    return +1 * (100000 * m_nPhotos + 1384);
  }//B0 decays to pi+ pi- pi+ pi- eta'
  if (GenB0Tag::PcheckDecay(genpart, 211, -211, 111, 111, 111)) {
    return +1 * (100000 * m_nPhotos + 1385);
  }//B0 decays to pi+ pi- pi0 pi0 pi0
  if (GenB0Tag::PcheckDecay(genpart, 211, -211, 111, 111, 221)) {
    return +1 * (100000 * m_nPhotos + 1386);
  }//B0 decays to pi+ pi- pi0 pi0 eta
  if (GenB0Tag::PcheckDecay(genpart, 211, -211, 111, 111, 331)) {
    return +1 * (100000 * m_nPhotos + 1387);
  }//B0 decays to pi+ pi- pi0 pi0 eta'
  if (GenB0Tag::PcheckDecay(genpart, 111, 111, 111, 111, 111)) {
    return +1 * (100000 * m_nPhotos + 1388);
  }//B0 decays to pi0 pi0 pi0 pi0 pi0
  if (GenB0Tag::PcheckDecay(genpart, 111, 111, 111, 111, 221)) {
    return +1 * (100000 * m_nPhotos + 1389);
  }//B0 decays to pi0 pi0 pi0 pi0 eta
  if (GenB0Tag::PcheckDecay(genpart, 111, 111, 111, 111, 331)) {
    return +1 * (100000 * m_nPhotos + 1390);
  }//B0 decays to pi0 pi0 pi0 pi0 eta'
  if (GenB0Tag::PcheckDecay(genpart, 20213, -213)) {
    return +1 * (100000 * m_nPhotos + 1391);
  }//B0 decays to a_1+ rho-
  if (GenB0Tag::PcheckDecay(genpart, -20213, 213)) {
    return +1 * (100000 * m_nPhotos + 1392);
  }//B0 decays to a_1- rho+
  if (GenB0Tag::PcheckDecay(genpart, 20113, 113)) {
    return +1 * (100000 * m_nPhotos + 1393);
  }//B0 decays to a_10 rho0
  if (GenB0Tag::PcheckDecay(genpart, 20113, 223)) {
    return +1 * (100000 * m_nPhotos + 1394);
  }//B0 decays to a_10 omega
  if (GenB0Tag::PcheckDecay(genpart, 20213, -211, 111)) {
    return +1 * (100000 * m_nPhotos + 1395);
  }//B0 decays to a_1+ pi- pi0
  if (GenB0Tag::PcheckDecay(genpart, 20213, -211, 221)) {
    return +1 * (100000 * m_nPhotos + 1396);
  }//B0 decays to a_1+ pi- eta
  if (GenB0Tag::PcheckDecay(genpart, 20213, -211, 331)) {
    return +1 * (100000 * m_nPhotos + 1397);
  }//B0 decays to a_1+ pi- eta'
  if (GenB0Tag::PcheckDecay(genpart, 20113, 211, -211)) {
    return +1 * (100000 * m_nPhotos + 1398);
  }//B0 decays to a_10 pi+ pi-
  if (GenB0Tag::PcheckDecay(genpart, 20113, 111, 111)) {
    return +1 * (100000 * m_nPhotos + 1399);
  }//B0 decays to a_10 pi0 pi0
  if (GenB0Tag::PcheckDecay(genpart, 20113, 111, 221)) {
    return +1 * (100000 * m_nPhotos + 1400);
  }//B0 decays to a_10 pi0 eta
  if (GenB0Tag::PcheckDecay(genpart, 20113, 111, 331)) {
    return +1 * (100000 * m_nPhotos + 1401);
  }//B0 decays to a_10 pi0 eta'
  if (GenB0Tag::PcheckDecay(genpart, -20213, 211, 111)) {
    return +1 * (100000 * m_nPhotos + 1402);
  }//B0 decays to a_1- pi+ pi0
  if (GenB0Tag::PcheckDecay(genpart, -20213, 211, 221)) {
    return +1 * (100000 * m_nPhotos + 1403);
  }//B0 decays to a_1- pi+ eta
  if (GenB0Tag::PcheckDecay(genpart, -20213, 211, 331)) {
    return +1 * (100000 * m_nPhotos + 1404);
  }//B0 decays to a_1- pi+ eta'
  if (GenB0Tag::PcheckDecay(genpart, 213, 10221, -211)) {
    return +1 * (100000 * m_nPhotos + 1405);
  }//B0 decays to rho+ f_0 pi-
  if (GenB0Tag::PcheckDecay(genpart, -213, 10221, 211)) {
    return +1 * (100000 * m_nPhotos + 1406);
  }//B0 decays to rho- f_0 pi+
  if (GenB0Tag::PcheckDecay(genpart, 113, 10221, 111)) {
    return +1 * (100000 * m_nPhotos + 1407);
  }//B0 decays to rho0 f_0 pi0
  if (GenB0Tag::PcheckDecay(genpart, 113, 10221, 221)) {
    return +1 * (100000 * m_nPhotos + 1408);
  }//B0 decays to rho0 f_0 eta
  if (GenB0Tag::PcheckDecay(genpart, 113, 10221, 331)) {
    return +1 * (100000 * m_nPhotos + 1409);
  }//B0 decays to rho0 f_0 eta'
  if (GenB0Tag::PcheckDecay(genpart, 223, 10221, 111)) {
    return +1 * (100000 * m_nPhotos + 1410);
  }//B0 decays to omega f_0 pi0
  if (GenB0Tag::PcheckDecay(genpart, 213, 10111, -211)) {
    return +1 * (100000 * m_nPhotos + 1411);
  }//B0 decays to rho+ a_00 pi-
  if (GenB0Tag::PcheckDecay(genpart, 213, -10211, 111)) {
    return +1 * (100000 * m_nPhotos + 1412);
  }//B0 decays to rho+ a_0- pi0
  if (GenB0Tag::PcheckDecay(genpart, 213, -10211, 221)) {
    return +1 * (100000 * m_nPhotos + 1413);
  }//B0 decays to rho+ a_0- eta
  if (GenB0Tag::PcheckDecay(genpart, 213, -10211, 331)) {
    return +1 * (100000 * m_nPhotos + 1414);
  }//B0 decays to rho+ a_0- eta'
  if (GenB0Tag::PcheckDecay(genpart, 113, 10211, -211)) {
    return +1 * (100000 * m_nPhotos + 1415);
  }//B0 decays to rho0 a_0+ pi-
  if (GenB0Tag::PcheckDecay(genpart, 113, -10211, 211)) {
    return +1 * (100000 * m_nPhotos + 1416);
  }//B0 decays to rho0 a_0- pi+
  if (GenB0Tag::PcheckDecay(genpart, 113, 10111, 111)) {
    return +1 * (100000 * m_nPhotos + 1417);
  }//B0 decays to rho0 a_00 pi0
  if (GenB0Tag::PcheckDecay(genpart, 113, 10111, 221)) {
    return +1 * (100000 * m_nPhotos + 1418);
  }//B0 decays to rho0 a_00 eta
  if (GenB0Tag::PcheckDecay(genpart, 113, 10111, 331)) {
    return +1 * (100000 * m_nPhotos + 1419);
  }//B0 decays to rho0 a_00 eta'
  if (GenB0Tag::PcheckDecay(genpart, -213, 10111, 211)) {
    return +1 * (100000 * m_nPhotos + 1420);
  }//B0 decays to rho- a_00 pi+
  if (GenB0Tag::PcheckDecay(genpart, -213, 10211, 111)) {
    return +1 * (100000 * m_nPhotos + 1421);
  }//B0 decays to rho- a_0+ pi0
  if (GenB0Tag::PcheckDecay(genpart, -213, 10211, 221)) {
    return +1 * (100000 * m_nPhotos + 1422);
  }//B0 decays to rho- a_0+ eta
  if (GenB0Tag::PcheckDecay(genpart, -213, 10211, 331)) {
    return +1 * (100000 * m_nPhotos + 1423);
  }//B0 decays to rho- a_0+ eta'
  if (GenB0Tag::PcheckDecay(genpart, 10221, 211, -211, 111)) {
    return +1 * (100000 * m_nPhotos + 1424);
  }//B0 decays to f_0 pi+ pi- pi0
  if (GenB0Tag::PcheckDecay(genpart, 10221, 211, -211, 221)) {
    return +1 * (100000 * m_nPhotos + 1425);
  }//B0 decays to f_0 pi+ pi- eta
  if (GenB0Tag::PcheckDecay(genpart, 10221, 211, -211, 331)) {
    return +1 * (100000 * m_nPhotos + 1426);
  }//B0 decays to f_0 pi+ pi- eta'
  if (GenB0Tag::PcheckDecay(genpart, 10221, 111, 111, 111)) {
    return +1 * (100000 * m_nPhotos + 1427);
  }//B0 decays to f_0 pi0 pi0 pi0
  if (GenB0Tag::PcheckDecay(genpart, 10221, 111, 111, 221)) {
    return +1 * (100000 * m_nPhotos + 1428);
  }//B0 decays to f_0 pi0 pi0 eta
  if (GenB0Tag::PcheckDecay(genpart, 10221, 111, 111, 331)) {
    return +1 * (100000 * m_nPhotos + 1429);
  }//B0 decays to f_0 pi0 pi0 eta'
  if (GenB0Tag::PcheckDecay(genpart, 10111, 211, -211, 111)) {
    return +1 * (100000 * m_nPhotos + 1430);
  }//B0 decays to a_00 pi+ pi- pi0
  if (GenB0Tag::PcheckDecay(genpart, 10111, 111, 111, 111)) {
    return +1 * (100000 * m_nPhotos + 1431);
  }//B0 decays to a_00 pi0 pi0 pi0
  if (GenB0Tag::PcheckDecay(genpart, 10211, -211, 211, -211)) {
    return +1 * (100000 * m_nPhotos + 1432);
  }//B0 decays to a_0+ pi- pi+ pi-
  if (GenB0Tag::PcheckDecay(genpart, 10211, -211, 111, 111)) {
    return +1 * (100000 * m_nPhotos + 1433);
  }//B0 decays to a_0+ pi- pi0 pi0
  if (GenB0Tag::PcheckDecay(genpart, -10211, 211, 211, -211)) {
    return +1 * (100000 * m_nPhotos + 1434);
  }//B0 decays to a_0- pi+ pi+ pi-
  if (GenB0Tag::PcheckDecay(genpart, -10211, 211, 111, 111)) {
    return +1 * (100000 * m_nPhotos + 1435);
  }//B0 decays to a_0- pi+ pi0 pi0
  if (GenB0Tag::PcheckDecay(genpart, 113, 323, -211)) {
    return +1 * (100000 * m_nPhotos + 1436);
  }//B0 decays to rho0 K*+ pi-
  if (GenB0Tag::PcheckDecay(genpart, -213, 323, 111)) {
    return +1 * (100000 * m_nPhotos + 1437);
  }//B0 decays to rho- K*+ pi0
  if (GenB0Tag::PcheckDecay(genpart, -213, 323, 221)) {
    return +1 * (100000 * m_nPhotos + 1438);
  }//B0 decays to rho- K*+ eta
  if (GenB0Tag::PcheckDecay(genpart, -213, 323, 331)) {
    return +1 * (100000 * m_nPhotos + 1439);
  }//B0 decays to rho- K*+ eta'
  if (GenB0Tag::PcheckDecay(genpart, 113, 313, 111)) {
    return +1 * (100000 * m_nPhotos + 1440);
  }//B0 decays to rho0 K*0 pi0
  if (GenB0Tag::PcheckDecay(genpart, 113, 313, 221)) {
    return +1 * (100000 * m_nPhotos + 1441);
  }//B0 decays to rho0 K*0 eta
  if (GenB0Tag::PcheckDecay(genpart, 113, 313, 331)) {
    return +1 * (100000 * m_nPhotos + 1442);
  }//B0 decays to rho0 K*0 eta'
  if (GenB0Tag::PcheckDecay(genpart, -213, 313, 211)) {
    return +1 * (100000 * m_nPhotos + 1443);
  }//B0 decays to rho- K*0 pi+
  if (GenB0Tag::PcheckDecay(genpart, 213, 313, -211)) {
    return +1 * (100000 * m_nPhotos + 1444);
  }//B0 decays to rho+ K*0 pi-
  if (GenB0Tag::PcheckDecay(genpart, 223, 323, -211)) {
    return +1 * (100000 * m_nPhotos + 1445);
  }//B0 decays to omega K*+ pi-
  if (GenB0Tag::PcheckDecay(genpart, 223, 313, 111)) {
    return +1 * (100000 * m_nPhotos + 1446);
  }//B0 decays to omega K*0 pi0
  if (GenB0Tag::PcheckDecay(genpart, 223, 313, 221)) {
    return +1 * (100000 * m_nPhotos + 1447);
  }//B0 decays to omega K*0 eta
  if (GenB0Tag::PcheckDecay(genpart, 223, 313, 331)) {
    return +1 * (100000 * m_nPhotos + 1448);
  }//B0 decays to omega K*0 eta'
  if (GenB0Tag::PcheckDecay(genpart, 211, -211, 323, -211)) {
    return +1 * (100000 * m_nPhotos + 1449);
  }//B0 decays to pi+ pi- K*+ pi-
  if (GenB0Tag::PcheckDecay(genpart, -211, 111, 323, 111)) {
    return +1 * (100000 * m_nPhotos + 1450);
  }//B0 decays to pi- pi0 K*+ pi0
  if (GenB0Tag::PcheckDecay(genpart, -211, 111, 323, 221)) {
    return +1 * (100000 * m_nPhotos + 1451);
  }//B0 decays to pi- pi0 K*+ eta
  if (GenB0Tag::PcheckDecay(genpart, -211, 111, 323, 331)) {
    return +1 * (100000 * m_nPhotos + 1452);
  }//B0 decays to pi- pi0 K*+ eta'
  if (GenB0Tag::PcheckDecay(genpart, 211, -211, 313, 111)) {
    return +1 * (100000 * m_nPhotos + 1453);
  }//B0 decays to pi+ pi- K*0 pi0
  if (GenB0Tag::PcheckDecay(genpart, 211, -211, 313, 221)) {
    return +1 * (100000 * m_nPhotos + 1454);
  }//B0 decays to pi+ pi- K*0 eta
  if (GenB0Tag::PcheckDecay(genpart, 211, -211, 313, 331)) {
    return +1 * (100000 * m_nPhotos + 1455);
  }//B0 decays to pi+ pi- K*0 eta'
  if (GenB0Tag::PcheckDecay(genpart, 111, 111, 313, 111)) {
    return +1 * (100000 * m_nPhotos + 1456);
  }//B0 decays to pi0 pi0 K*0 pi0
  if (GenB0Tag::PcheckDecay(genpart, 111, 111, 313, 221)) {
    return +1 * (100000 * m_nPhotos + 1457);
  }//B0 decays to pi0 pi0 K*0 eta
  if (GenB0Tag::PcheckDecay(genpart, 111, 111, 313, 331)) {
    return +1 * (100000 * m_nPhotos + 1458);
  }//B0 decays to pi0 pi0 K*0 eta'
  if (GenB0Tag::PcheckDecay(genpart, 213, 321, -211, -211)) {
    return +1 * (100000 * m_nPhotos + 1459);
  }//B0 decays to rho+ K+ pi- pi-
  if (GenB0Tag::PcheckDecay(genpart, 113, 321, -211, 111)) {
    return +1 * (100000 * m_nPhotos + 1460);
  }//B0 decays to rho0 K+ pi- pi0
  if (GenB0Tag::PcheckDecay(genpart, 113, 321, -211, 221)) {
    return +1 * (100000 * m_nPhotos + 1461);
  }//B0 decays to rho0 K+ pi- eta
  if (GenB0Tag::PcheckDecay(genpart, 113, 321, -211, 331)) {
    return +1 * (100000 * m_nPhotos + 1462);
  }//B0 decays to rho0 K+ pi- eta'
  if (GenB0Tag::PcheckDecay(genpart, -213, 321, 211, -211)) {
    return +1 * (100000 * m_nPhotos + 1463);
  }//B0 decays to rho- K+ pi+ pi-
  if (GenB0Tag::PcheckDecay(genpart, -213, 321, 111, 111)) {
    return +1 * (100000 * m_nPhotos + 1464);
  }//B0 decays to rho- K+ pi0 pi0
  if (GenB0Tag::PcheckDecay(genpart, -213, 321, 111, 221)) {
    return +1 * (100000 * m_nPhotos + 1465);
  }//B0 decays to rho- K+ pi0 eta
  if (GenB0Tag::PcheckDecay(genpart, -213, 321, 111, 331)) {
    return +1 * (100000 * m_nPhotos + 1466);
  }//B0 decays to rho- K+ pi0 eta'
  if (GenB0Tag::PcheckDecay(genpart, 113, 311, 211, -211)) {
    return +1 * (100000 * m_nPhotos + 1467);
  }//B0 decays to rho0 K0 pi+ pi-
  if (GenB0Tag::PcheckDecay(genpart, 113, 311, 111, 111)) {
    return +1 * (100000 * m_nPhotos + 1468);
  }//B0 decays to rho0 K0 pi0 pi0
  if (GenB0Tag::PcheckDecay(genpart, 113, 311, 111, 221)) {
    return +1 * (100000 * m_nPhotos + 1469);
  }//B0 decays to rho0 K0 pi0 eta
  if (GenB0Tag::PcheckDecay(genpart, 113, 311, 111, 331)) {
    return +1 * (100000 * m_nPhotos + 1470);
  }//B0 decays to rho0 K0 pi0 eta'
  if (GenB0Tag::PcheckDecay(genpart, 213, 311, -211, 111)) {
    return +1 * (100000 * m_nPhotos + 1471);
  }//B0 decays to rho+ K0 pi- pi0
  if (GenB0Tag::PcheckDecay(genpart, 213, 311, -211, 221)) {
    return +1 * (100000 * m_nPhotos + 1472);
  }//B0 decays to rho+ K0 pi- eta
  if (GenB0Tag::PcheckDecay(genpart, 213, 311, -211, 331)) {
    return +1 * (100000 * m_nPhotos + 1473);
  }//B0 decays to rho+ K0 pi- eta'
  if (GenB0Tag::PcheckDecay(genpart, -213, 311, 211, 111)) {
    return +1 * (100000 * m_nPhotos + 1474);
  }//B0 decays to rho- K0 pi+ pi0
  if (GenB0Tag::PcheckDecay(genpart, -213, 311, 211, 221)) {
    return +1 * (100000 * m_nPhotos + 1475);
  }//B0 decays to rho- K0 pi+ eta
  if (GenB0Tag::PcheckDecay(genpart, -213, 311, 211, 331)) {
    return +1 * (100000 * m_nPhotos + 1476);
  }//B0 decays to rho- K0 pi+ eta'
  if (GenB0Tag::PcheckDecay(genpart, 223, 321, -211, 111)) {
    return +1 * (100000 * m_nPhotos + 1477);
  }//B0 decays to omega K+ pi- pi0
  if (GenB0Tag::PcheckDecay(genpart, 223, 321, -211, 221)) {
    return +1 * (100000 * m_nPhotos + 1478);
  }//B0 decays to omega K+ pi- eta
  if (GenB0Tag::PcheckDecay(genpart, 223, 321, -211, 331)) {
    return +1 * (100000 * m_nPhotos + 1479);
  }//B0 decays to omega K+ pi- eta'
  if (GenB0Tag::PcheckDecay(genpart, 223, 311, 211, -211)) {
    return +1 * (100000 * m_nPhotos + 1480);
  }//B0 decays to omega K0 pi+ pi-
  if (GenB0Tag::PcheckDecay(genpart, 223, 311, 111, 111)) {
    return +1 * (100000 * m_nPhotos + 1481);
  }//B0 decays to omega K0 pi0 pi0
  if (GenB0Tag::PcheckDecay(genpart, 223, 311, 111, 221)) {
    return +1 * (100000 * m_nPhotos + 1482);
  }//B0 decays to omega K0 pi0 eta
  if (GenB0Tag::PcheckDecay(genpart, 223, 311, 111, 331)) {
    return +1 * (100000 * m_nPhotos + 1483);
  }//B0 decays to omega K0 pi0 eta'
  if (GenB0Tag::PcheckDecay(genpart, 211, -211, -211, 321, 111)) {
    return +1 * (100000 * m_nPhotos + 1484);
  }//B0 decays to pi+ pi- pi- K+ pi0
  if (GenB0Tag::PcheckDecay(genpart, 211, -211, -211, 321, 221)) {
    return +1 * (100000 * m_nPhotos + 1485);
  }//B0 decays to pi+ pi- pi- K+ eta
  if (GenB0Tag::PcheckDecay(genpart, 211, -211, -211, 321, 331)) {
    return +1 * (100000 * m_nPhotos + 1486);
  }//B0 decays to pi+ pi- pi- K+ eta'
  if (GenB0Tag::PcheckDecay(genpart, 111, 111, -211, 321, 111)) {
    return +1 * (100000 * m_nPhotos + 1487);
  }//B0 decays to pi0 pi0 pi- K+ pi0
  if (GenB0Tag::PcheckDecay(genpart, 111, 111, -211, 321, 221)) {
    return +1 * (100000 * m_nPhotos + 1488);
  }//B0 decays to pi0 pi0 pi- K+ eta
  if (GenB0Tag::PcheckDecay(genpart, 111, 111, -211, 321, 331)) {
    return +1 * (100000 * m_nPhotos + 1489);
  }//B0 decays to pi0 pi0 pi- K+ eta'
  if (GenB0Tag::PcheckDecay(genpart, 211, -211, 211, 311, -211)) {
    return +1 * (100000 * m_nPhotos + 1490);
  }//B0 decays to pi+ pi- pi+ K0 pi-
  if (GenB0Tag::PcheckDecay(genpart, 211, -211, 111, 311, 111)) {
    return +1 * (100000 * m_nPhotos + 1491);
  }//B0 decays to pi+ pi- pi0 K0 pi0
  if (GenB0Tag::PcheckDecay(genpart, 211, -211, 111, 311, 221)) {
    return +1 * (100000 * m_nPhotos + 1492);
  }//B0 decays to pi+ pi- pi0 K0 eta
  if (GenB0Tag::PcheckDecay(genpart, 211, -211, 111, 311, 331)) {
    return +1 * (100000 * m_nPhotos + 1493);
  }//B0 decays to pi+ pi- pi0 K0 eta'
  if (GenB0Tag::PcheckDecay(genpart, 111, 111, 111, 311, 111)) {
    return +1 * (100000 * m_nPhotos + 1494);
  }//B0 decays to pi0 pi0 pi0 K0 pi0
  if (GenB0Tag::PcheckDecay(genpart, 111, 111, 111, 311, 221)) {
    return +1 * (100000 * m_nPhotos + 1495);
  }//B0 decays to pi0 pi0 pi0 K0 eta
  if (GenB0Tag::PcheckDecay(genpart, 111, 111, 111, 311, 331)) {
    return +1 * (100000 * m_nPhotos + 1496);
  }//B0 decays to pi0 pi0 pi0 K0 eta'
  if (GenB0Tag::PcheckDecay(genpart, 213, 10311, -211)) {
    return +1 * (100000 * m_nPhotos + 1497);
  }//B0 decays to rho+ K_0*0 pi-
  if (GenB0Tag::PcheckDecay(genpart, 113, 10311, 111)) {
    return +1 * (100000 * m_nPhotos + 1498);
  }//B0 decays to rho0 K_0*0 pi0
  if (GenB0Tag::PcheckDecay(genpart, 113, 10311, 221)) {
    return +1 * (100000 * m_nPhotos + 1499);
  }//B0 decays to rho0 K_0*0 eta
  if (GenB0Tag::PcheckDecay(genpart, 113, 10311, 331)) {
    return +1 * (100000 * m_nPhotos + 1500);
  }//B0 decays to rho0 K_0*0 eta'
  if (GenB0Tag::PcheckDecay(genpart, -213, 10311, 211)) {
    return +1 * (100000 * m_nPhotos + 1501);
  }//B0 decays to rho- K_0*0 pi+
  if (GenB0Tag::PcheckDecay(genpart, -213, 10321, 111)) {
    return +1 * (100000 * m_nPhotos + 1502);
  }//B0 decays to rho- K_0*+ pi0
  if (GenB0Tag::PcheckDecay(genpart, -213, 10321, 221)) {
    return +1 * (100000 * m_nPhotos + 1503);
  }//B0 decays to rho- K_0*+ eta
  if (GenB0Tag::PcheckDecay(genpart, -213, 10321, 331)) {
    return +1 * (100000 * m_nPhotos + 1504);
  }//B0 decays to rho- K_0*+ eta'
  if (GenB0Tag::PcheckDecay(genpart, 113, 10321, -211)) {
    return +1 * (100000 * m_nPhotos + 1505);
  }//B0 decays to rho0 K_0*+ pi-
  if (GenB0Tag::PcheckDecay(genpart, 211, -211, 10321, -211)) {
    return +1 * (100000 * m_nPhotos + 1506);
  }//B0 decays to pi+ pi- K_0*+ pi-
  if (GenB0Tag::PcheckDecay(genpart, -211, 111, 10321, 111)) {
    return +1 * (100000 * m_nPhotos + 1507);
  }//B0 decays to pi- pi0 K_0*+ pi0
  if (GenB0Tag::PcheckDecay(genpart, -211, 111, 10321, 221)) {
    return +1 * (100000 * m_nPhotos + 1508);
  }//B0 decays to pi- pi0 K_0*+ eta
  if (GenB0Tag::PcheckDecay(genpart, -211, 111, 10321, 331)) {
    return +1 * (100000 * m_nPhotos + 1509);
  }//B0 decays to pi- pi0 K_0*+ eta'
  if (GenB0Tag::PcheckDecay(genpart, 211, -211, 10311, 111)) {
    return +1 * (100000 * m_nPhotos + 1510);
  }//B0 decays to pi+ pi- K_0*0 pi0
  if (GenB0Tag::PcheckDecay(genpart, 211, -211, 10311, 221)) {
    return +1 * (100000 * m_nPhotos + 1511);
  }//B0 decays to pi+ pi- K_0*0 eta
  if (GenB0Tag::PcheckDecay(genpart, 211, -211, 10311, 331)) {
    return +1 * (100000 * m_nPhotos + 1512);
  }//B0 decays to pi+ pi- K_0*0 eta'
  if (GenB0Tag::PcheckDecay(genpart, 111, 111, 10311, 111)) {
    return +1 * (100000 * m_nPhotos + 1513);
  }//B0 decays to pi0 pi0 K_0*0 pi0
  if (GenB0Tag::PcheckDecay(genpart, 111, 111, 10311, 221)) {
    return +1 * (100000 * m_nPhotos + 1514);
  }//B0 decays to pi0 pi0 K_0*0 eta
  if (GenB0Tag::PcheckDecay(genpart, 111, 111, 10311, 331)) {
    return +1 * (100000 * m_nPhotos + 1515);
  }//B0 decays to pi0 pi0 K_0*0 eta'
  if (GenB0Tag::PcheckDecay(genpart, 323, 10221, -211)) {
    return +1 * (100000 * m_nPhotos + 1516);
  }//B0 decays to K*+ f_0 pi-
  if (GenB0Tag::PcheckDecay(genpart, 313, 10221, 111)) {
    return +1 * (100000 * m_nPhotos + 1517);
  }//B0 decays to K*0 f_0 pi0
  if (GenB0Tag::PcheckDecay(genpart, 313, 10221, 221)) {
    return +1 * (100000 * m_nPhotos + 1518);
  }//B0 decays to K*0 f_0 eta
  if (GenB0Tag::PcheckDecay(genpart, 313, 10221, 331)) {
    return +1 * (100000 * m_nPhotos + 1519);
  }//B0 decays to K*0 f_0 eta'
  if (GenB0Tag::PcheckDecay(genpart, 20213, 311, -211)) {
    return +1 * (100000 * m_nPhotos + 1520);
  }//B0 decays to a_1+ K0 pi-
  if (GenB0Tag::PcheckDecay(genpart, 20113, 313)) {
    return +1 * (100000 * m_nPhotos + 1521);
  }//B0 decays to a_10 K*0
  if (GenB0Tag::PcheckDecay(genpart, 20113, 321, -211)) {
    return +1 * (100000 * m_nPhotos + 1522);
  }//B0 decays to a_10 K+ pi-
  if (GenB0Tag::PcheckDecay(genpart, 20113, 311, 111)) {
    return +1 * (100000 * m_nPhotos + 1523);
  }//B0 decays to a_10 K0 pi0
  if (GenB0Tag::PcheckDecay(genpart, -20213, 323)) {
    return +1 * (100000 * m_nPhotos + 1524);
  }//B0 decays to a_1- K*+
  if (GenB0Tag::PcheckDecay(genpart, -20213, 321, 111)) {
    return +1 * (100000 * m_nPhotos + 1525);
  }//B0 decays to a_1- K+ pi0
  if (GenB0Tag::PcheckDecay(genpart, -20213, 311, 211)) {
    return +1 * (100000 * m_nPhotos + 1526);
  }//B0 decays to a_1- K0 pi+
  if (GenB0Tag::PcheckDecay(genpart, 323, 10111, -211)) {
    return +1 * (100000 * m_nPhotos + 1527);
  }//B0 decays to K*+ a_00 pi-
  if (GenB0Tag::PcheckDecay(genpart, 323, -10211, 111)) {
    return +1 * (100000 * m_nPhotos + 1528);
  }//B0 decays to K*+ a_0- pi0
  if (GenB0Tag::PcheckDecay(genpart, 323, -10211, 221)) {
    return +1 * (100000 * m_nPhotos + 1529);
  }//B0 decays to K*+ a_0- eta
  if (GenB0Tag::PcheckDecay(genpart, 323, -10211, 331)) {
    return +1 * (100000 * m_nPhotos + 1530);
  }//B0 decays to K*+ a_0- eta'
  if (GenB0Tag::PcheckDecay(genpart, 313, 10211, -211)) {
    return +1 * (100000 * m_nPhotos + 1531);
  }//B0 decays to K*0 a_0+ pi-
  if (GenB0Tag::PcheckDecay(genpart, 313, 10111, 111)) {
    return +1 * (100000 * m_nPhotos + 1532);
  }//B0 decays to K*0 a_00 pi0
  if (GenB0Tag::PcheckDecay(genpart, 313, 10111, 221)) {
    return +1 * (100000 * m_nPhotos + 1533);
  }//B0 decays to K*0 a_00 eta
  if (GenB0Tag::PcheckDecay(genpart, 313, 10111, 331)) {
    return +1 * (100000 * m_nPhotos + 1534);
  }//B0 decays to K*0 a_00 eta'
  if (GenB0Tag::PcheckDecay(genpart, -323, 313, 211)) {
    return +1 * (100000 * m_nPhotos + 1535);
  }//B0 decays to K*- K*0 pi+
  if (GenB0Tag::PcheckDecay(genpart, 323, -313, -211)) {
    return +1 * (100000 * m_nPhotos + 1536);
  }//B0 decays to K*+ anti-K*0 pi-
  if (GenB0Tag::PcheckDecay(genpart, -323, 323, 111)) {
    return +1 * (100000 * m_nPhotos + 1537);
  }//B0 decays to K*- K*+ pi0
  if (GenB0Tag::PcheckDecay(genpart, -323, 323, 221)) {
    return +1 * (100000 * m_nPhotos + 1538);
  }//B0 decays to K*- K*+ eta
  if (GenB0Tag::PcheckDecay(genpart, -323, 323, 331)) {
    return +1 * (100000 * m_nPhotos + 1539);
  }//B0 decays to K*- K*+ eta'
  if (GenB0Tag::PcheckDecay(genpart, 313, -313, 111)) {
    return +1 * (100000 * m_nPhotos + 1540);
  }//B0 decays to K*0 anti-K*0 pi0
  if (GenB0Tag::PcheckDecay(genpart, 313, -313, 221)) {
    return +1 * (100000 * m_nPhotos + 1541);
  }//B0 decays to K*0 anti-K*0 eta
  if (GenB0Tag::PcheckDecay(genpart, 313, -313, 331)) {
    return +1 * (100000 * m_nPhotos + 1542);
  }//B0 decays to K*0 anti-K*0 eta'
  if (GenB0Tag::PcheckDecay(genpart, 333, 213, -211)) {
    return +1 * (100000 * m_nPhotos + 1543);
  }//B0 decays to phi rho+ pi-
  if (GenB0Tag::PcheckDecay(genpart, 333, -213, 211)) {
    return +1 * (100000 * m_nPhotos + 1544);
  }//B0 decays to phi rho- pi+
  if (GenB0Tag::PcheckDecay(genpart, 333, 113, 111)) {
    return +1 * (100000 * m_nPhotos + 1545);
  }//B0 decays to phi rho0 pi0
  if (GenB0Tag::PcheckDecay(genpart, 333, 113, 221)) {
    return +1 * (100000 * m_nPhotos + 1546);
  }//B0 decays to phi rho0 eta
  if (GenB0Tag::PcheckDecay(genpart, 333, 113, 331)) {
    return +1 * (100000 * m_nPhotos + 1547);
  }//B0 decays to phi rho0 eta'
  if (GenB0Tag::PcheckDecay(genpart, 333, 223, 111)) {
    return +1 * (100000 * m_nPhotos + 1548);
  }//B0 decays to phi omega pi0
  if (GenB0Tag::PcheckDecay(genpart, 333, 10221, 111)) {
    return +1 * (100000 * m_nPhotos + 1549);
  }//B0 decays to phi f_0 pi0
  if (GenB0Tag::PcheckDecay(genpart, 333, 211, -211, 111)) {
    return +1 * (100000 * m_nPhotos + 1550);
  }//B0 decays to phi pi+ pi- pi0
  if (GenB0Tag::PcheckDecay(genpart, 333, 211, -211, 221)) {
    return +1 * (100000 * m_nPhotos + 1551);
  }//B0 decays to phi pi+ pi- eta
  if (GenB0Tag::PcheckDecay(genpart, 333, 211, -211, 331)) {
    return +1 * (100000 * m_nPhotos + 1552);
  }//B0 decays to phi pi+ pi- eta'
  if (GenB0Tag::PcheckDecay(genpart, 333, 111, 111, 111)) {
    return +1 * (100000 * m_nPhotos + 1553);
  }//B0 decays to phi pi0 pi0 pi0
  if (GenB0Tag::PcheckDecay(genpart, 333, 111, 111, 221)) {
    return +1 * (100000 * m_nPhotos + 1554);
  }//B0 decays to phi pi0 pi0 eta
  if (GenB0Tag::PcheckDecay(genpart, 333, 111, 111, 331)) {
    return +1 * (100000 * m_nPhotos + 1555);
  }//B0 decays to phi pi0 pi0 eta'
  if (GenB0Tag::PcheckDecay(genpart, 311, -321, 113, 211)) {
    return +1 * (100000 * m_nPhotos + 1556);
  }//B0 decays to K0 K- rho0 pi+
  if (GenB0Tag::PcheckDecay(genpart, 311, -321, 213, 111)) {
    return +1 * (100000 * m_nPhotos + 1557);
  }//B0 decays to K0 K- rho+ pi0
  if (GenB0Tag::PcheckDecay(genpart, 311, -321, 213, 221)) {
    return +1 * (100000 * m_nPhotos + 1558);
  }//B0 decays to K0 K- rho+ eta
  if (GenB0Tag::PcheckDecay(genpart, 311, -321, 213, 331)) {
    return +1 * (100000 * m_nPhotos + 1559);
  }//B0 decays to K0 K- rho+ eta'
  if (GenB0Tag::PcheckDecay(genpart, 311, -321, 223, 211)) {
    return +1 * (100000 * m_nPhotos + 1560);
  }//B0 decays to K0 K- omega pi+
  if (GenB0Tag::PcheckDecay(genpart, 311, -321, 10221, 211)) {
    return +1 * (100000 * m_nPhotos + 1561);
  }//B0 decays to K0 K- f_0 pi+
  if (GenB0Tag::PcheckDecay(genpart, 321, -321, 213, -211)) {
    return +1 * (100000 * m_nPhotos + 1562);
  }//B0 decays to K+ K- rho+ pi-
  if (GenB0Tag::PcheckDecay(genpart, 321, -321, -213, 211)) {
    return +1 * (100000 * m_nPhotos + 1563);
  }//B0 decays to K+ K- rho- pi+
  if (GenB0Tag::PcheckDecay(genpart, 321, -321, 113, 111)) {
    return +1 * (100000 * m_nPhotos + 1564);
  }//B0 decays to K+ K- rho0 pi0
  if (GenB0Tag::PcheckDecay(genpart, 321, -321, 113, 221)) {
    return +1 * (100000 * m_nPhotos + 1565);
  }//B0 decays to K+ K- rho0 eta
  if (GenB0Tag::PcheckDecay(genpart, 321, -321, 113, 331)) {
    return +1 * (100000 * m_nPhotos + 1566);
  }//B0 decays to K+ K- rho0 eta'
  if (GenB0Tag::PcheckDecay(genpart, 321, -321, 223, 111)) {
    return +1 * (100000 * m_nPhotos + 1567);
  }//B0 decays to K+ K- omega pi0
  if (GenB0Tag::PcheckDecay(genpart, 321, -321, 223, 221)) {
    return +1 * (100000 * m_nPhotos + 1568);
  }//B0 decays to K+ K- omega eta
  if (GenB0Tag::PcheckDecay(genpart, 321, -321, 223, 331)) {
    return +1 * (100000 * m_nPhotos + 1569);
  }//B0 decays to K+ K- omega eta'
  if (GenB0Tag::PcheckDecay(genpart, 321, -321, 10221, 111)) {
    return +1 * (100000 * m_nPhotos + 1570);
  }//B0 decays to K+ K- f_0 pi0
  if (GenB0Tag::PcheckDecay(genpart, 311, -311, 213, -211)) {
    return +1 * (100000 * m_nPhotos + 1571);
  }//B0 decays to K0 anti-K0 rho+ pi-
  if (GenB0Tag::PcheckDecay(genpart, 311, -311, -213, 211)) {
    return +1 * (100000 * m_nPhotos + 1572);
  }//B0 decays to K0 anti-K0 rho- pi+
  if (GenB0Tag::PcheckDecay(genpart, 311, -311, 113, 111)) {
    return +1 * (100000 * m_nPhotos + 1573);
  }//B0 decays to K0 anti-K0 rho0 pi0
  if (GenB0Tag::PcheckDecay(genpart, 311, -311, 113, 221)) {
    return +1 * (100000 * m_nPhotos + 1574);
  }//B0 decays to K0 anti-K0 rho0 eta
  if (GenB0Tag::PcheckDecay(genpart, 311, -311, 113, 331)) {
    return +1 * (100000 * m_nPhotos + 1575);
  }//B0 decays to K0 anti-K0 rho0 eta'
  if (GenB0Tag::PcheckDecay(genpart, 311, -311, 223, 111)) {
    return +1 * (100000 * m_nPhotos + 1576);
  }//B0 decays to K0 anti-K0 omega pi0
  if (GenB0Tag::PcheckDecay(genpart, 311, -311, 10221, 111)) {
    return +1 * (100000 * m_nPhotos + 1577);
  }//B0 decays to K0 anti-K0 f_0 pi0
  if (GenB0Tag::PcheckDecay(genpart, -311, 321, 113, -211)) {
    return +1 * (100000 * m_nPhotos + 1578);
  }//B0 decays to anti-K0 K+ rho0 pi-
  if (GenB0Tag::PcheckDecay(genpart, -311, 321, -213, 111)) {
    return +1 * (100000 * m_nPhotos + 1579);
  }//B0 decays to anti-K0 K+ rho- pi0
  if (GenB0Tag::PcheckDecay(genpart, -311, 321, -213, 221)) {
    return +1 * (100000 * m_nPhotos + 1580);
  }//B0 decays to anti-K0 K+ rho- eta
  if (GenB0Tag::PcheckDecay(genpart, -311, 321, -213, 331)) {
    return +1 * (100000 * m_nPhotos + 1581);
  }//B0 decays to anti-K0 K+ rho- eta'
  if (GenB0Tag::PcheckDecay(genpart, -311, 321, 223, -211)) {
    return +1 * (100000 * m_nPhotos + 1582);
  }//B0 decays to anti-K0 K+ omega pi-
  if (GenB0Tag::PcheckDecay(genpart, -311, 321, 10221, -211)) {
    return +1 * (100000 * m_nPhotos + 1583);
  }//B0 decays to anti-K0 K+ f_0 pi-
  if (GenB0Tag::PcheckDecay(genpart, 311, -321, 211, 211, -211)) {
    return +1 * (100000 * m_nPhotos + 1584);
  }//B0 decays to K0 K- pi+ pi+ pi-
  if (GenB0Tag::PcheckDecay(genpart, 311, -321, 211, 111, 111)) {
    return +1 * (100000 * m_nPhotos + 1585);
  }//B0 decays to K0 K- pi+ pi0 pi0
  if (GenB0Tag::PcheckDecay(genpart, 311, -321, 211, 111, 221)) {
    return +1 * (100000 * m_nPhotos + 1586);
  }//B0 decays to K0 K- pi+ pi0 eta
  if (GenB0Tag::PcheckDecay(genpart, 311, -321, 211, 111, 331)) {
    return +1 * (100000 * m_nPhotos + 1587);
  }//B0 decays to K0 K- pi+ pi0 eta'
  if (GenB0Tag::PcheckDecay(genpart, 321, -321, 211, -211, 111)) {
    return +1 * (100000 * m_nPhotos + 1588);
  }//B0 decays to K+ K- pi+ pi- pi0
  if (GenB0Tag::PcheckDecay(genpart, 321, -321, 211, -211, 221)) {
    return +1 * (100000 * m_nPhotos + 1589);
  }//B0 decays to K+ K- pi+ pi- eta
  if (GenB0Tag::PcheckDecay(genpart, 321, -321, 211, -211, 331)) {
    return +1 * (100000 * m_nPhotos + 1590);
  }//B0 decays to K+ K- pi+ pi- eta'
  if (GenB0Tag::PcheckDecay(genpart, 321, -321, 111, 111, 111)) {
    return +1 * (100000 * m_nPhotos + 1591);
  }//B0 decays to K+ K- pi0 pi0 pi0
  if (GenB0Tag::PcheckDecay(genpart, 321, -321, 111, 111, 221)) {
    return +1 * (100000 * m_nPhotos + 1592);
  }//B0 decays to K+ K- pi0 pi0 eta
  if (GenB0Tag::PcheckDecay(genpart, 321, -321, 111, 111, 331)) {
    return +1 * (100000 * m_nPhotos + 1593);
  }//B0 decays to K+ K- pi0 pi0 eta'
  if (GenB0Tag::PcheckDecay(genpart, 311, -311, 211, -211, 111)) {
    return +1 * (100000 * m_nPhotos + 1594);
  }//B0 decays to K0 anti-K0 pi+ pi- pi0
  if (GenB0Tag::PcheckDecay(genpart, 311, -311, 211, -211, 221)) {
    return +1 * (100000 * m_nPhotos + 1595);
  }//B0 decays to K0 anti-K0 pi+ pi- eta
  if (GenB0Tag::PcheckDecay(genpart, 311, -311, 211, -211, 331)) {
    return +1 * (100000 * m_nPhotos + 1596);
  }//B0 decays to K0 anti-K0 pi+ pi- eta'
  if (GenB0Tag::PcheckDecay(genpart, 311, -311, 111, 111, 111)) {
    return +1 * (100000 * m_nPhotos + 1597);
  }//B0 decays to K0 anti-K0 pi0 pi0 pi0
  if (GenB0Tag::PcheckDecay(genpart, 311, -311, 111, 111, 221)) {
    return +1 * (100000 * m_nPhotos + 1598);
  }//B0 decays to K0 anti-K0 pi0 pi0 eta
  if (GenB0Tag::PcheckDecay(genpart, 311, -311, 111, 111, 331)) {
    return +1 * (100000 * m_nPhotos + 1599);
  }//B0 decays to K0 anti-K0 pi0 pi0 eta'
  if (GenB0Tag::PcheckDecay(genpart, -311, 321, -211, 211, -211)) {
    return +1 * (100000 * m_nPhotos + 1600);
  }//B0 decays to anti-K0 K+ pi- pi+ pi-
  if (GenB0Tag::PcheckDecay(genpart, -311, 321, -211, 111, 111)) {
    return +1 * (100000 * m_nPhotos + 1601);
  }//B0 decays to anti-K0 K+ pi- pi0 pi0
  if (GenB0Tag::PcheckDecay(genpart, -311, 321, -211, 111, 221)) {
    return +1 * (100000 * m_nPhotos + 1602);
  }//B0 decays to anti-K0 K+ pi- pi0 eta
  if (GenB0Tag::PcheckDecay(genpart, -311, 321, -211, 111, 331)) {
    return +1 * (100000 * m_nPhotos + 1603);
  }//B0 decays to anti-K0 K+ pi- pi0 eta'
  if (GenB0Tag::PcheckDecay(genpart, 313, -321, 211, 111)) {
    return +1 * (100000 * m_nPhotos + 1604);
  }//B0 decays to K*0 K- pi+ pi0
  if (GenB0Tag::PcheckDecay(genpart, 313, -321, 211, 221)) {
    return +1 * (100000 * m_nPhotos + 1605);
  }//B0 decays to K*0 K- pi+ eta
  if (GenB0Tag::PcheckDecay(genpart, 313, -321, 211, 331)) {
    return +1 * (100000 * m_nPhotos + 1606);
  }//B0 decays to K*0 K- pi+ eta'
  if (GenB0Tag::PcheckDecay(genpart, -323, 311, 211, 111)) {
    return +1 * (100000 * m_nPhotos + 1607);
  }//B0 decays to K*- K0 pi+ pi0
  if (GenB0Tag::PcheckDecay(genpart, -323, 311, 211, 221)) {
    return +1 * (100000 * m_nPhotos + 1608);
  }//B0 decays to K*- K0 pi+ eta
  if (GenB0Tag::PcheckDecay(genpart, -323, 311, 211, 331)) {
    return +1 * (100000 * m_nPhotos + 1609);
  }//B0 decays to K*- K0 pi+ eta'
  if (GenB0Tag::PcheckDecay(genpart, 323, -321, 211, -211)) {
    return +1 * (100000 * m_nPhotos + 1610);
  }//B0 decays to K*+ K- pi+ pi-
  if (GenB0Tag::PcheckDecay(genpart, 323, -321, 111, 111)) {
    return +1 * (100000 * m_nPhotos + 1611);
  }//B0 decays to K*+ K- pi0 pi0
  if (GenB0Tag::PcheckDecay(genpart, 323, -321, 111, 221)) {
    return +1 * (100000 * m_nPhotos + 1612);
  }//B0 decays to K*+ K- pi0 eta
  if (GenB0Tag::PcheckDecay(genpart, 323, -321, 111, 331)) {
    return +1 * (100000 * m_nPhotos + 1613);
  }//B0 decays to K*+ K- pi0 eta'
  if (GenB0Tag::PcheckDecay(genpart, -323, 321, 211, -211)) {
    return +1 * (100000 * m_nPhotos + 1614);
  }//B0 decays to K*- K+ pi+ pi-
  if (GenB0Tag::PcheckDecay(genpart, -323, 321, 111, 111)) {
    return +1 * (100000 * m_nPhotos + 1615);
  }//B0 decays to K*- K+ pi0 pi0
  if (GenB0Tag::PcheckDecay(genpart, -323, 321, 111, 221)) {
    return +1 * (100000 * m_nPhotos + 1616);
  }//B0 decays to K*- K+ pi0 eta
  if (GenB0Tag::PcheckDecay(genpart, -323, 321, 111, 331)) {
    return +1 * (100000 * m_nPhotos + 1617);
  }//B0 decays to K*- K+ pi0 eta'
  if (GenB0Tag::PcheckDecay(genpart, 313, -311, 211, -211)) {
    return +1 * (100000 * m_nPhotos + 1618);
  }//B0 decays to K*0 anti-K0 pi+ pi-
  if (GenB0Tag::PcheckDecay(genpart, 313, -311, 111, 111)) {
    return +1 * (100000 * m_nPhotos + 1619);
  }//B0 decays to K*0 anti-K0 pi0 pi0
  if (GenB0Tag::PcheckDecay(genpart, 313, -311, 111, 221)) {
    return +1 * (100000 * m_nPhotos + 1620);
  }//B0 decays to K*0 anti-K0 pi0 eta
  if (GenB0Tag::PcheckDecay(genpart, 313, -311, 111, 331)) {
    return +1 * (100000 * m_nPhotos + 1621);
  }//B0 decays to K*0 anti-K0 pi0 eta'
  if (GenB0Tag::PcheckDecay(genpart, 311, -313, 211, -211)) {
    return +1 * (100000 * m_nPhotos + 1622);
  }//B0 decays to K0 anti-K*0 pi+ pi-
  if (GenB0Tag::PcheckDecay(genpart, 311, -313, 111, 111)) {
    return +1 * (100000 * m_nPhotos + 1623);
  }//B0 decays to K0 anti-K*0 pi0 pi0
  if (GenB0Tag::PcheckDecay(genpart, 311, -313, 111, 221)) {
    return +1 * (100000 * m_nPhotos + 1624);
  }//B0 decays to K0 anti-K*0 pi0 eta
  if (GenB0Tag::PcheckDecay(genpart, 311, -313, 111, 331)) {
    return +1 * (100000 * m_nPhotos + 1625);
  }//B0 decays to K0 anti-K*0 pi0 eta'
  if (GenB0Tag::PcheckDecay(genpart, 321, -313, -211, 111)) {
    return +1 * (100000 * m_nPhotos + 1626);
  }//B0 decays to K+ anti-K*0 pi- pi0
  if (GenB0Tag::PcheckDecay(genpart, 321, -313, -211, 221)) {
    return +1 * (100000 * m_nPhotos + 1627);
  }//B0 decays to K+ anti-K*0 pi- eta
  if (GenB0Tag::PcheckDecay(genpart, 321, -313, -211, 331)) {
    return +1 * (100000 * m_nPhotos + 1628);
  }//B0 decays to K+ anti-K*0 pi- eta'
  if (GenB0Tag::PcheckDecay(genpart, 323, -311, -211, 111)) {
    return +1 * (100000 * m_nPhotos + 1629);
  }//B0 decays to K*+ anti-K0 pi- pi0
  if (GenB0Tag::PcheckDecay(genpart, 323, -311, -211, 221)) {
    return +1 * (100000 * m_nPhotos + 1630);
  }//B0 decays to K*+ anti-K0 pi- eta
  if (GenB0Tag::PcheckDecay(genpart, 323, -311, -211, 331)) {
    return +1 * (100000 * m_nPhotos + 1631);
  }//B0 decays to K*+ anti-K0 pi- eta'
  if (GenB0Tag::PcheckDecay(genpart, 333, 323, -211)) {
    return +1 * (100000 * m_nPhotos + 1632);
  }//B0 decays to phi K*+ pi-
  if (GenB0Tag::PcheckDecay(genpart, 333, 313, 111)) {
    return +1 * (100000 * m_nPhotos + 1633);
  }//B0 decays to phi K*0 pi0
  if (GenB0Tag::PcheckDecay(genpart, 333, 313, 221)) {
    return +1 * (100000 * m_nPhotos + 1634);
  }//B0 decays to phi K*0 eta
  if (GenB0Tag::PcheckDecay(genpart, 333, 313, 331)) {
    return +1 * (100000 * m_nPhotos + 1635);
  }//B0 decays to phi K*0 eta'
  if (GenB0Tag::PcheckDecay(genpart, 333, 321, -213)) {
    return +1 * (100000 * m_nPhotos + 1636);
  }//B0 decays to phi K+ rho-
  if (GenB0Tag::PcheckDecay(genpart, 333, 311, 113)) {
    return +1 * (100000 * m_nPhotos + 1637);
  }//B0 decays to phi K0 rho0
  if (GenB0Tag::PcheckDecay(genpart, 333, 311, 223)) {
    return +1 * (100000 * m_nPhotos + 1638);
  }//B0 decays to phi K0 omega
  if (GenB0Tag::PcheckDecay(genpart, 333, 311, 10221)) {
    return +1 * (100000 * m_nPhotos + 1639);
  }//B0 decays to phi K0 f_0
  if (GenB0Tag::PcheckDecay(genpart, 333, 311, 211, -211)) {
    return +1 * (100000 * m_nPhotos + 1640);
  }//B0 decays to phi K0 pi+ pi-
  if (GenB0Tag::PcheckDecay(genpart, 333, 311, 111, 111)) {
    return +1 * (100000 * m_nPhotos + 1641);
  }//B0 decays to phi K0 pi0 pi0
  if (GenB0Tag::PcheckDecay(genpart, 333, 311, 111, 221)) {
    return +1 * (100000 * m_nPhotos + 1642);
  }//B0 decays to phi K0 pi0 eta
  if (GenB0Tag::PcheckDecay(genpart, 333, 311, 111, 331)) {
    return +1 * (100000 * m_nPhotos + 1643);
  }//B0 decays to phi K0 pi0 eta'
  if (GenB0Tag::PcheckDecay(genpart, 333, 321, -211, 111)) {
    return +1 * (100000 * m_nPhotos + 1644);
  }//B0 decays to phi K+ pi- pi0
  if (GenB0Tag::PcheckDecay(genpart, 333, 321, -211, 221)) {
    return +1 * (100000 * m_nPhotos + 1645);
  }//B0 decays to phi K+ pi- eta
  if (GenB0Tag::PcheckDecay(genpart, 333, 321, -211, 331)) {
    return +1 * (100000 * m_nPhotos + 1646);
  }//B0 decays to phi K+ pi- eta'
  if (GenB0Tag::PcheckDecay(genpart, -321, 321, 323, -211)) {
    return +1 * (100000 * m_nPhotos + 1647);
  }//B0 decays to K- K+ K*+ pi-
  if (GenB0Tag::PcheckDecay(genpart, 321, -323, 321, -211)) {
    return +1 * (100000 * m_nPhotos + 1648);
  }//B0 decays to K+ K*- K+ pi-
  if (GenB0Tag::PcheckDecay(genpart, 321, 313, -311, -211)) {
    return +1 * (100000 * m_nPhotos + 1649);
  }//B0 decays to K+ K*0 anti-K0 pi-
  if (GenB0Tag::PcheckDecay(genpart, 321, 311, -313, -211)) {
    return +1 * (100000 * m_nPhotos + 1650);
  }//B0 decays to K+ K0 anti-K*0 pi-
  if (GenB0Tag::PcheckDecay(genpart, 311, 323, -311, -211)) {
    return +1 * (100000 * m_nPhotos + 1651);
  }//B0 decays to K0 K*+ anti-K0 pi-
  if (GenB0Tag::PcheckDecay(genpart, -321, 321, 321, -213)) {
    return +1 * (100000 * m_nPhotos + 1652);
  }//B0 decays to K- K+ K+ rho-
  if (GenB0Tag::PcheckDecay(genpart, -321, 321, 311, 113)) {
    return +1 * (100000 * m_nPhotos + 1653);
  }//B0 decays to K- K+ K0 rho0
  if (GenB0Tag::PcheckDecay(genpart, -321, 321, 311, 223)) {
    return +1 * (100000 * m_nPhotos + 1654);
  }//B0 decays to K- K+ K0 omega
  if (GenB0Tag::PcheckDecay(genpart, -321, 321, 311, 10221)) {
    return +1 * (100000 * m_nPhotos + 1655);
  }//B0 decays to K- K+ K0 f_0
  if (GenB0Tag::PcheckDecay(genpart, 321, -321, 321, -211, 111)) {
    return +1 * (100000 * m_nPhotos + 1656);
  }//B0 decays to K+ K- K+ pi- pi0
  if (GenB0Tag::PcheckDecay(genpart, 321, -321, 321, -211, 221)) {
    return +1 * (100000 * m_nPhotos + 1657);
  }//B0 decays to K+ K- K+ pi- eta
  if (GenB0Tag::PcheckDecay(genpart, 321, -321, 321, -211, 331)) {
    return +1 * (100000 * m_nPhotos + 1658);
  }//B0 decays to K+ K- K+ pi- eta'
  if (GenB0Tag::PcheckDecay(genpart, 321, -321, 311, 211, -211)) {
    return +1 * (100000 * m_nPhotos + 1659);
  }//B0 decays to K+ K- K0 pi+ pi-
  if (GenB0Tag::PcheckDecay(genpart, 321, -321, 311, 111, 111)) {
    return +1 * (100000 * m_nPhotos + 1660);
  }//B0 decays to K+ K- K0 pi0 pi0
  if (GenB0Tag::PcheckDecay(genpart, 321, -321, 311, 111, 221)) {
    return +1 * (100000 * m_nPhotos + 1661);
  }//B0 decays to K+ K- K0 pi0 eta
  if (GenB0Tag::PcheckDecay(genpart, 321, -321, 311, 111, 331)) {
    return +1 * (100000 * m_nPhotos + 1662);
  }//B0 decays to K+ K- K0 pi0 eta'
  if (GenB0Tag::PcheckDecay(genpart, 311, -311, 321, -211, 111)) {
    return +1 * (100000 * m_nPhotos + 1663);
  }//B0 decays to K0 anti-K0 K+ pi- pi0
  if (GenB0Tag::PcheckDecay(genpart, 311, -311, 321, -211, 221)) {
    return +1 * (100000 * m_nPhotos + 1664);
  }//B0 decays to K0 anti-K0 K+ pi- eta
  if (GenB0Tag::PcheckDecay(genpart, 311, -311, 321, -211, 331)) {
    return +1 * (100000 * m_nPhotos + 1665);
  }//B0 decays to K0 anti-K0 K+ pi- eta'
  if (GenB0Tag::PcheckDecay(genpart, 311, -311, 311, 211, -211)) {
    return +1 * (100000 * m_nPhotos + 1666);
  }//B0 decays to K0 anti-K0 K0 pi+ pi-
  if (GenB0Tag::PcheckDecay(genpart, 311, -311, 311, 111, 111)) {
    return +1 * (100000 * m_nPhotos + 1667);
  }//B0 decays to K0 anti-K0 K0 pi0 pi0
  if (GenB0Tag::PcheckDecay(genpart, 311, -311, 311, 111, 221)) {
    return +1 * (100000 * m_nPhotos + 1668);
  }//B0 decays to K0 anti-K0 K0 pi0 eta
  if (GenB0Tag::PcheckDecay(genpart, 311, -311, 311, 111, 331)) {
    return +1 * (100000 * m_nPhotos + 1669);
  }//B0 decays to K0 anti-K0 K0 pi0 eta'
  if (GenB0Tag::PcheckDecay(genpart, 333, 10321, -211)) {
    return +1 * (100000 * m_nPhotos + 1670);
  }//B0 decays to phi K_0*+ pi-
  if (GenB0Tag::PcheckDecay(genpart, 333, 10311, 111)) {
    return +1 * (100000 * m_nPhotos + 1671);
  }//B0 decays to phi K_0*0 pi0
  if (GenB0Tag::PcheckDecay(genpart, 321, -321, 10321, -211)) {
    return +1 * (100000 * m_nPhotos + 1672);
  }//B0 decays to K+ K- K_0*+ pi-
  if (GenB0Tag::PcheckDecay(genpart, 321, -321, 10311, 111)) {
    return +1 * (100000 * m_nPhotos + 1673);
  }//B0 decays to K+ K- K_0*0 pi0
  if (GenB0Tag::PcheckDecay(genpart, 321, -321, 10311, 221)) {
    return +1 * (100000 * m_nPhotos + 1674);
  }//B0 decays to K+ K- K_0*0 eta
  if (GenB0Tag::PcheckDecay(genpart, 321, -321, 10311, 331)) {
    return +1 * (100000 * m_nPhotos + 1675);
  }//B0 decays to K+ K- K_0*0 eta'
  if (GenB0Tag::PcheckDecay(genpart, 321, 321, -10321, -211)) {
    return +1 * (100000 * m_nPhotos + 1676);
  }//B0 decays to K+ K+ K_0*- pi-
  if (GenB0Tag::PcheckDecay(genpart, 321, -10311, 311, -211)) {
    return +1 * (100000 * m_nPhotos + 1677);
  }//B0 decays to K+ anti-K_0*0 K0 pi-
  if (GenB0Tag::PcheckDecay(genpart, 321, 10311, -311, -211)) {
    return +1 * (100000 * m_nPhotos + 1678);
  }//B0 decays to K+ K_0*0 anti-K0 pi-
  if (GenB0Tag::PcheckDecay(genpart, 311, -311, 10321, -211)) {
    return +1 * (100000 * m_nPhotos + 1679);
  }//B0 decays to K0 anti-K0 K_0*+ pi-
  if (GenB0Tag::PcheckDecay(genpart, 311, -311, 10311, 111)) {
    return +1 * (100000 * m_nPhotos + 1680);
  }//B0 decays to K0 anti-K0 K_0*0 pi0
  if (GenB0Tag::PcheckDecay(genpart, 311, -311, 10311, 221)) {
    return +1 * (100000 * m_nPhotos + 1681);
  }//B0 decays to K0 anti-K0 K_0*0 eta
  if (GenB0Tag::PcheckDecay(genpart, 311, -311, 10311, 331)) {
    return +1 * (100000 * m_nPhotos + 1682);
  }//B0 decays to K0 anti-K0 K_0*0 eta'
  if (GenB0Tag::PcheckDecay(genpart, 311, 311, -10311, 111)) {
    return +1 * (100000 * m_nPhotos + 1683);
  }//B0 decays to K0 K0 anti-K_0*0 pi0
  if (GenB0Tag::PcheckDecay(genpart, 311, 311, -10311, 221)) {
    return +1 * (100000 * m_nPhotos + 1684);
  }//B0 decays to K0 K0 anti-K_0*0 eta
  if (GenB0Tag::PcheckDecay(genpart, 311, 311, -10311, 331)) {
    return +1 * (100000 * m_nPhotos + 1685);
  }//B0 decays to K0 K0 anti-K_0*0 eta'
  if (GenB0Tag::PcheckDecay(genpart, 333, 20323, -211)) {
    return +1 * (100000 * m_nPhotos + 1686);
  }//B0 decays to phi K'_1+ pi-
  if (GenB0Tag::PcheckDecay(genpart, 333, 20313, 111)) {
    return +1 * (100000 * m_nPhotos + 1687);
  }//B0 decays to phi K'_10 pi0
  if (GenB0Tag::PcheckDecay(genpart, 333, 20313, 221)) {
    return +1 * (100000 * m_nPhotos + 1688);
  }//B0 decays to phi K'_10 eta
  if (GenB0Tag::PcheckDecay(genpart, 333, 20313, 331)) {
    return +1 * (100000 * m_nPhotos + 1689);
  }//B0 decays to phi K'_10 eta'
  if (GenB0Tag::PcheckDecay(genpart, -321, 321, 20323, -211)) {
    return +1 * (100000 * m_nPhotos + 1690);
  }//B0 decays to K- K+ K'_1+ pi-
  if (GenB0Tag::PcheckDecay(genpart, -321, 321, 20313, 111)) {
    return +1 * (100000 * m_nPhotos + 1691);
  }//B0 decays to K- K+ K'_10 pi0
  if (GenB0Tag::PcheckDecay(genpart, -321, 321, 20313, 221)) {
    return +1 * (100000 * m_nPhotos + 1692);
  }//B0 decays to K- K+ K'_10 eta
  if (GenB0Tag::PcheckDecay(genpart, -321, 321, 20313, 331)) {
    return +1 * (100000 * m_nPhotos + 1693);
  }//B0 decays to K- K+ K'_10 eta'
  if (GenB0Tag::PcheckDecay(genpart, 321, 321, -20323, -211)) {
    return +1 * (100000 * m_nPhotos + 1694);
  }//B0 decays to K+ K+ K'_1- pi-
  if (GenB0Tag::PcheckDecay(genpart, 311, -311, 20313, 111)) {
    return +1 * (100000 * m_nPhotos + 1695);
  }//B0 decays to K0 anti-K0 K'_10 pi0
  if (GenB0Tag::PcheckDecay(genpart, 311, -311, 20313, 221)) {
    return +1 * (100000 * m_nPhotos + 1696);
  }//B0 decays to K0 anti-K0 K'_10 eta
  if (GenB0Tag::PcheckDecay(genpart, 311, -311, 20313, 331)) {
    return +1 * (100000 * m_nPhotos + 1697);
  }//B0 decays to K0 anti-K0 K'_10 eta'
  if (GenB0Tag::PcheckDecay(genpart, 311, 311, -20313, 111)) {
    return +1 * (100000 * m_nPhotos + 1698);
  }//B0 decays to K0 K0 anti-K'_10 pi0
  if (GenB0Tag::PcheckDecay(genpart, 311, 311, -20313, 221)) {
    return +1 * (100000 * m_nPhotos + 1699);
  }//B0 decays to K0 K0 anti-K'_10 eta
  if (GenB0Tag::PcheckDecay(genpart, 311, 311, -20313, 331)) {
    return +1 * (100000 * m_nPhotos + 1700);
  }//B0 decays to K0 K0 anti-K'_10 eta'
  if (GenB0Tag::PcheckDecay(genpart, 333, 333, 311)) {
    return +1 * (100000 * m_nPhotos + 1701);
  }//B0 decays to phi phi K0
  if (GenB0Tag::PcheckDecay(genpart, 333, 333, 111)) {
    return +1 * (100000 * m_nPhotos + 1702);
  }//B0 decays to phi phi pi0
  if (GenB0Tag::PcheckDecay(genpart, 333, 321, -321, 311)) {
    return +1 * (100000 * m_nPhotos + 1703);
  }//B0 decays to phi K+ K- K0
  if (GenB0Tag::PcheckDecay(genpart, 333, 311, -311, 311)) {
    return +1 * (100000 * m_nPhotos + 1704);
  }//B0 decays to phi K0 anti-K0 K0
  if (GenB0Tag::PcheckDecay(genpart, 333, 321, -321, 111)) {
    return +1 * (100000 * m_nPhotos + 1705);
  }//B0 decays to phi K+ K- pi0
  if (GenB0Tag::PcheckDecay(genpart, 333, 311, -311, 111)) {
    return +1 * (100000 * m_nPhotos + 1706);
  }//B0 decays to phi K0 anti-K0 pi0
  if (GenB0Tag::PcheckDecay(genpart, 333, -311, 321, -211)) {
    return +1 * (100000 * m_nPhotos + 1707);
  }//B0 decays to phi anti-K0 K+ pi-
  if (GenB0Tag::PcheckDecay(genpart, 333, 311, -321, 211)) {
    return +1 * (100000 * m_nPhotos + 1708);
  }//B0 decays to phi K0 K- pi+
  if (GenB0Tag::PcheckDecay(genpart, 321, -321, 321, -321, 311)) {
    return +1 * (100000 * m_nPhotos + 1709);
  }//B0 decays to K+ K- K+ K- K0
  if (GenB0Tag::PcheckDecay(genpart, 311, -311, 321, -321, 311)) {
    return +1 * (100000 * m_nPhotos + 1710);
  }//B0 decays to K0 anti-K0 K+ K- K0
  if (GenB0Tag::PcheckDecay(genpart, 321, -321, 321, -321, 111)) {
    return +1 * (100000 * m_nPhotos + 1711);
  }//B0 decays to K+ K- K+ K- pi0
  if (GenB0Tag::PcheckDecay(genpart, 321, -321, 311, -311, 111)) {
    return +1 * (100000 * m_nPhotos + 1712);
  }//B0 decays to K+ K- K0 anti-K0 pi0
  if (GenB0Tag::PcheckDecay(genpart, 321, -321, 321, -311, -211)) {
    return +1 * (100000 * m_nPhotos + 1713);
  }//B0 decays to K+ K- K+ anti-K0 pi-
  if (GenB0Tag::PcheckDecay(genpart, 321, -321, 311, -321, 211)) {
    return +1 * (100000 * m_nPhotos + 1714);
  }//B0 decays to K+ K- K0 K- pi+
  if (GenB0Tag::PcheckDecay(genpart, 311, -311, 311, -311, 111)) {
    return +1 * (100000 * m_nPhotos + 1715);
  }//B0 decays to K0 anti-K0 K0 anti-K0 pi0
  if (GenB0Tag::PcheckDecay(genpart, 311, -311, -311, 321, -211)) {
    return +1 * (100000 * m_nPhotos + 1716);
  }//B0 decays to K0 anti-K0 anti-K0 K+ pi-
  if (GenB0Tag::PcheckDecay(genpart, 311, -311, 311, -321, 211)) {
    return +1 * (100000 * m_nPhotos + 1717);
  }//B0 decays to K0 anti-K0 K0 K- pi+
  if (GenB0Tag::PcheckDecay(genpart, 333, 333, 313)) {
    return +1 * (100000 * m_nPhotos + 1718);
  }//B0 decays to phi phi K*0
  if (GenB0Tag::PcheckDecay(genpart, 333, 321, -321, 313)) {
    return +1 * (100000 * m_nPhotos + 1719);
  }//B0 decays to phi K+ K- K*0
  if (GenB0Tag::PcheckDecay(genpart, 333, 311, -311, 313)) {
    return +1 * (100000 * m_nPhotos + 1720);
  }//B0 decays to phi K0 anti-K0 K*0
  if (GenB0Tag::PcheckDecay(genpart, 333, 311, 321, -323)) {
    return +1 * (100000 * m_nPhotos + 1721);
  }//B0 decays to phi K0 K+ K*-
  if (GenB0Tag::PcheckDecay(genpart, 333, 311, 311, -313)) {
    return +1 * (100000 * m_nPhotos + 1722);
  }//B0 decays to phi K0 K0 anti-K*0
  if (GenB0Tag::PcheckDecay(genpart, 333, 321, -321, 321, -211)) {
    return +1 * (100000 * m_nPhotos + 1723);
  }//B0 decays to phi K+ K- K+ pi-
  if (GenB0Tag::PcheckDecay(genpart, 333, 321, -321, 311, 111)) {
    return +1 * (100000 * m_nPhotos + 1724);
  }//B0 decays to phi K+ K- K0 pi0
  if (GenB0Tag::PcheckDecay(genpart, 333, 311, -311, 321, -211)) {
    return +1 * (100000 * m_nPhotos + 1725);
  }//B0 decays to phi K0 anti-K0 K+ pi-
  if (GenB0Tag::PcheckDecay(genpart, 333, 311, -311, 311, 111)) {
    return +1 * (100000 * m_nPhotos + 1726);
  }//B0 decays to phi K0 anti-K0 K0 pi0
  if (GenB0Tag::PcheckDecay(genpart, 20113, 20113)) {
    return +1 * (100000 * m_nPhotos + 1727);
  }//B0 decays to a_10 a_10
  if (GenB0Tag::PcheckDecay(genpart, 20213, -20213)) {
    return +1 * (100000 * m_nPhotos + 1728);
  }//B0 decays to a_1+ a_1-
  if (GenB0Tag::PcheckDecay(genpart, 443, 310)) {
    return +1 * (100000 * m_nPhotos + 1729);
  }//B0 decays to J/psi K_S0
  if (GenB0Tag::PcheckDecay(genpart, 443, 130)) {
    return +1 * (100000 * m_nPhotos + 1730);
  }//B0 decays to J/psi K_L0
  if (GenB0Tag::PcheckDecay(genpart, 443, 313)) {
    return +1 * (100000 * m_nPhotos + 1731);
  }//B0 decays to J/psi K*S
  if (GenB0Tag::PcheckDecay(genpart, 443, 313)) {
    return +1 * (100000 * m_nPhotos + 1732);
  }//B0 decays to J/psi K*L
  if (GenB0Tag::PcheckDecay(genpart, 443, 313)) {
    return +1 * (100000 * m_nPhotos + 1733);
  }//B0 decays to J/psi K*0T
  if (GenB0Tag::PcheckDecay(genpart, 443, 111)) {
    return +1 * (100000 * m_nPhotos + 1734);
  }//B0 decays to J/psi pi0
  if (GenB0Tag::PcheckDecay(genpart, 443, 113)) {
    return +1 * (100000 * m_nPhotos + 1735);
  }//B0 decays to J/psi rho0
  if (GenB0Tag::PcheckDecay(genpart, 443, 223)) {
    return +1 * (100000 * m_nPhotos + 1736);
  }//B0 decays to J/psi omega
  if (GenB0Tag::PcheckDecay(genpart, 443, 311, 111)) {
    return +1 * (100000 * m_nPhotos + 1737);
  }//B0 decays to J/psi K0 pi0
  if (GenB0Tag::PcheckDecay(genpart, 443, 311, -211, 211)) {
    return +1 * (100000 * m_nPhotos + 1738);
  }//B0 decays to J/psi K0 pi- pi+
  if (GenB0Tag::PcheckDecay(genpart, 443, 10313)) {
    return +1 * (100000 * m_nPhotos + 1739);
  }//B0 decays to J/psi K_10
  if (GenB0Tag::PcheckDecay(genpart, 443, 20313)) {
    return +1 * (100000 * m_nPhotos + 1740);
  }//B0 decays to J/psi K'_10
  if (GenB0Tag::PcheckDecay(genpart, 443, 315)) {
    return +1 * (100000 * m_nPhotos + 1741);
  }//B0 decays to J/psi K_2*0
  if (GenB0Tag::PcheckDecay(genpart, 443, 311, 333)) {
    return +1 * (100000 * m_nPhotos + 1742);
  }//B0 decays to J/psi K0 phi
  if (GenB0Tag::PcheckDecay(genpart, 443, 321, -211)) {
    return +1 * (100000 * m_nPhotos + 1743);
  }//B0 decays to J/psi K+ pi-
  if (GenB0Tag::PcheckDecay(genpart, 443, 221, 311)) {
    return +1 * (100000 * m_nPhotos + 1744);
  }//B0 decays to J/psi eta K0
  if (GenB0Tag::PcheckDecay(genpart, 443, 223, 311)) {
    return +1 * (100000 * m_nPhotos + 1745);
  }//B0 decays to J/psi omega K0
  if (GenB0Tag::PcheckDecay(genpart, 443, 221)) {
    return +1 * (100000 * m_nPhotos + 1746);
  }//B0 decays to J/psi eta
  if (GenB0Tag::PcheckDecay(genpart, 443, 211, -211)) {
    return +1 * (100000 * m_nPhotos + 1747);
  }//B0 decays to J/psi pi+ pi-
  if (GenB0Tag::PcheckDecay(genpart, 443, 225)) {
    return +1 * (100000 * m_nPhotos + 1748);
  }//B0 decays to J/psi f_2
  if (GenB0Tag::PcheckDecay(genpart, 443, 321, -321)) {
    return +1 * (100000 * m_nPhotos + 1749);
  }//B0 decays to J/psi K+ K-
  if (GenB0Tag::PcheckDecay(genpart, 443, 331)) {
    return +1 * (100000 * m_nPhotos + 1750);
  }//B0 decays to J/psi eta'
  if (GenB0Tag::PcheckDecay(genpart, 443, 311, 321, -321)) {
    return +1 * (100000 * m_nPhotos + 1751);
  }//B0 decays to J/psi K0 K+ K-
  if (GenB0Tag::PcheckDecay(genpart, 443, 311, 113)) {
    return +1 * (100000 * m_nPhotos + 1752);
  }//B0 decays to J/psi K0 rho0
  if (GenB0Tag::PcheckDecay(genpart, 443, 323, -211)) {
    return +1 * (100000 * m_nPhotos + 1753);
  }//B0 decays to J/psi K*+ pi-
  if (GenB0Tag::PcheckDecay(genpart, 443, 211, -211, 211, -211)) {
    return +1 * (100000 * m_nPhotos + 1754);
  }//B0 decays to J/psi pi+ pi- pi+ pi-
  if (GenB0Tag::PcheckDecay(genpart, 443, 20223)) {
    return +1 * (100000 * m_nPhotos + 1755);
  }//B0 decays to J/psi f_1
  if (GenB0Tag::PcheckDecay(genpart, 443, 313, 211, -211)) {
    return +1 * (100000 * m_nPhotos + 1756);
  }//B0 decays to J/psi K*0 pi+ pi-
  if (GenB0Tag::PcheckDecay(genpart, 100443, 310)) {
    return +1 * (100000 * m_nPhotos + 1757);
  }//B0 decays to psi(2S) K_S0
  if (GenB0Tag::PcheckDecay(genpart, 100443, 130)) {
    return +1 * (100000 * m_nPhotos + 1758);
  }//B0 decays to psi(2S) K_L0
  if (GenB0Tag::PcheckDecay(genpart, 100443, 313)) {
    return +1 * (100000 * m_nPhotos + 1759);
  }//B0 decays to psi(2S) K*S
  if (GenB0Tag::PcheckDecay(genpart, 100443, 313)) {
    return +1 * (100000 * m_nPhotos + 1760);
  }//B0 decays to psi(2S) K*L
  if (GenB0Tag::PcheckDecay(genpart, 100443, 313)) {
    return +1 * (100000 * m_nPhotos + 1761);
  }//B0 decays to psi(2S) K*0T
  if (GenB0Tag::PcheckDecay(genpart, 100443, 321, -211)) {
    return +1 * (100000 * m_nPhotos + 1762);
  }//B0 decays to psi(2S) K+ pi-
  if (GenB0Tag::PcheckDecay(genpart, 100443, 311, 111)) {
    return +1 * (100000 * m_nPhotos + 1763);
  }//B0 decays to psi(2S) K0 pi0
  if (GenB0Tag::PcheckDecay(genpart, 100443, 311, -211, 211)) {
    return +1 * (100000 * m_nPhotos + 1764);
  }//B0 decays to psi(2S) K0 pi- pi+
  if (GenB0Tag::PcheckDecay(genpart, 100443, 311, 111, 111)) {
    return +1 * (100000 * m_nPhotos + 1765);
  }//B0 decays to psi(2S) K0 pi0 pi0
  if (GenB0Tag::PcheckDecay(genpart, 100443, 321, -211, 111)) {
    return +1 * (100000 * m_nPhotos + 1766);
  }//B0 decays to psi(2S) K+ pi- pi0
  if (GenB0Tag::PcheckDecay(genpart, 100443, 10313)) {
    return +1 * (100000 * m_nPhotos + 1767);
  }//B0 decays to psi(2S) K_10
  if (GenB0Tag::PcheckDecay(genpart, 100443, 211, -211)) {
    return +1 * (100000 * m_nPhotos + 1768);
  }//B0 decays to psi(2S) pi+ pi-
  if (GenB0Tag::PcheckDecay(genpart, 100443, 111)) {
    return +1 * (100000 * m_nPhotos + 1769);
  }//B0 decays to psi(2S) pi0
  if (GenB0Tag::PcheckDecay(genpart, 441, 310)) {
    return +1 * (100000 * m_nPhotos + 1770);
  }//B0 decays to eta_c K_S0
  if (GenB0Tag::PcheckDecay(genpart, 441, 130)) {
    return +1 * (100000 * m_nPhotos + 1771);
  }//B0 decays to eta_c K_L0
  if (GenB0Tag::PcheckDecay(genpart, 313, 441)) {
    return +1 * (100000 * m_nPhotos + 1772);
  }//B0 decays to K*S eta_c
  if (GenB0Tag::PcheckDecay(genpart, 313, 441)) {
    return +1 * (100000 * m_nPhotos + 1773);
  }//B0 decays to K*L eta_c
  if (GenB0Tag::PcheckDecay(genpart, 313, 441)) {
    return +1 * (100000 * m_nPhotos + 1774);
  }//B0 decays to K*0T eta_c
  if (GenB0Tag::PcheckDecay(genpart, 441, 321, -211)) {
    return +1 * (100000 * m_nPhotos + 1775);
  }//B0 decays to eta_c K+ pi-
  if (GenB0Tag::PcheckDecay(genpart, 441, 311, 111)) {
    return +1 * (100000 * m_nPhotos + 1776);
  }//B0 decays to eta_c K0 pi0
  if (GenB0Tag::PcheckDecay(genpart, 441, 311, -211, 211)) {
    return +1 * (100000 * m_nPhotos + 1777);
  }//B0 decays to eta_c K0 pi- pi+
  if (GenB0Tag::PcheckDecay(genpart, 441, 311, 111, 111)) {
    return +1 * (100000 * m_nPhotos + 1778);
  }//B0 decays to eta_c K0 pi0 pi0
  if (GenB0Tag::PcheckDecay(genpart, 441, 321, -211, 111)) {
    return +1 * (100000 * m_nPhotos + 1779);
  }//B0 decays to eta_c K+ pi- pi0
  if (GenB0Tag::PcheckDecay(genpart, 100441, 310)) {
    return +1 * (100000 * m_nPhotos + 1780);
  }//B0 decays to eta_c(2S) K_S0
  if (GenB0Tag::PcheckDecay(genpart, 100441, 130)) {
    return +1 * (100000 * m_nPhotos + 1781);
  }//B0 decays to eta_c(2S) K_L0
  if (GenB0Tag::PcheckDecay(genpart, 313, 100441)) {
    return +1 * (100000 * m_nPhotos + 1782);
  }//B0 decays to K*S eta_c(2S)
  if (GenB0Tag::PcheckDecay(genpart, 313, 100441)) {
    return +1 * (100000 * m_nPhotos + 1783);
  }//B0 decays to K*L eta_c(2S)
  if (GenB0Tag::PcheckDecay(genpart, 313, 100441)) {
    return +1 * (100000 * m_nPhotos + 1784);
  }//B0 decays to K*0T eta_c(2S)
  if (GenB0Tag::PcheckDecay(genpart, 100441, 321, -211)) {
    return +1 * (100000 * m_nPhotos + 1785);
  }//B0 decays to eta_c(2S) K+ pi-
  if (GenB0Tag::PcheckDecay(genpart, 100441, 311, 111)) {
    return +1 * (100000 * m_nPhotos + 1786);
  }//B0 decays to eta_c(2S) K0 pi0
  if (GenB0Tag::PcheckDecay(genpart, 100441, 311, -211, 211)) {
    return +1 * (100000 * m_nPhotos + 1787);
  }//B0 decays to eta_c(2S) K0 pi- pi+
  if (GenB0Tag::PcheckDecay(genpart, 100441, 311, 111, 111)) {
    return +1 * (100000 * m_nPhotos + 1788);
  }//B0 decays to eta_c(2S) K0 pi0 pi0
  if (GenB0Tag::PcheckDecay(genpart, 100441, 321, -211, 111)) {
    return +1 * (100000 * m_nPhotos + 1789);
  }//B0 decays to eta_c(2S) K+ pi- pi0
  if (GenB0Tag::PcheckDecay(genpart, 10441, 310)) {
    return +1 * (100000 * m_nPhotos + 1790);
  }//B0 decays to chi_c0 K_S0
  if (GenB0Tag::PcheckDecay(genpart, 10441, 130)) {
    return +1 * (100000 * m_nPhotos + 1791);
  }//B0 decays to chi_c0 K_L0
  if (GenB0Tag::PcheckDecay(genpart, 313, 10441)) {
    return +1 * (100000 * m_nPhotos + 1792);
  }//B0 decays to K*S chi_c0
  if (GenB0Tag::PcheckDecay(genpart, 313, 10441)) {
    return +1 * (100000 * m_nPhotos + 1793);
  }//B0 decays to K*L chi_c0
  if (GenB0Tag::PcheckDecay(genpart, 313, 10441)) {
    return +1 * (100000 * m_nPhotos + 1794);
  }//B0 decays to K*0T chi_c0
  if (GenB0Tag::PcheckDecay(genpart, 10441, 321, -211)) {
    return +1 * (100000 * m_nPhotos + 1795);
  }//B0 decays to chi_c0 K+ pi-
  if (GenB0Tag::PcheckDecay(genpart, 10441, 311, 111)) {
    return +1 * (100000 * m_nPhotos + 1796);
  }//B0 decays to chi_c0 K0 pi0
  if (GenB0Tag::PcheckDecay(genpart, 10441, 311, -211, 211)) {
    return +1 * (100000 * m_nPhotos + 1797);
  }//B0 decays to chi_c0 K0 pi- pi+
  if (GenB0Tag::PcheckDecay(genpart, 10441, 311, 111, 111)) {
    return +1 * (100000 * m_nPhotos + 1798);
  }//B0 decays to chi_c0 K0 pi0 pi0
  if (GenB0Tag::PcheckDecay(genpart, 10441, 321, -211, 111)) {
    return +1 * (100000 * m_nPhotos + 1799);
  }//B0 decays to chi_c0 K+ pi- pi0
  if (GenB0Tag::PcheckDecay(genpart, 20443, 310)) {
    return +1 * (100000 * m_nPhotos + 1800);
  }//B0 decays to chi_c1 K_S0
  if (GenB0Tag::PcheckDecay(genpart, 20443, 130)) {
    return +1 * (100000 * m_nPhotos + 1801);
  }//B0 decays to chi_c1 K_L0
  if (GenB0Tag::PcheckDecay(genpart, 313, 20443)) {
    return +1 * (100000 * m_nPhotos + 1802);
  }//B0 decays to K*S chi_c1
  if (GenB0Tag::PcheckDecay(genpart, 313, 20443)) {
    return +1 * (100000 * m_nPhotos + 1803);
  }//B0 decays to K*L chi_c1
  if (GenB0Tag::PcheckDecay(genpart, 20443, 313)) {
    return +1 * (100000 * m_nPhotos + 1804);
  }//B0 decays to chi_c1 K*0T
  if (GenB0Tag::PcheckDecay(genpart, 20443, 321, -211)) {
    return +1 * (100000 * m_nPhotos + 1805);
  }//B0 decays to chi_c1 K+ pi-
  if (GenB0Tag::PcheckDecay(genpart, 20443, 311, 111)) {
    return +1 * (100000 * m_nPhotos + 1806);
  }//B0 decays to chi_c1 K0 pi0
  if (GenB0Tag::PcheckDecay(genpart, 20443, 311, -211, 211)) {
    return +1 * (100000 * m_nPhotos + 1807);
  }//B0 decays to chi_c1 K0 pi- pi+
  if (GenB0Tag::PcheckDecay(genpart, 20443, 311, 111, 111)) {
    return +1 * (100000 * m_nPhotos + 1808);
  }//B0 decays to chi_c1 K0 pi0 pi0
  if (GenB0Tag::PcheckDecay(genpart, 20443, 321, -211, 111)) {
    return +1 * (100000 * m_nPhotos + 1809);
  }//B0 decays to chi_c1 K+ pi- pi0
  if (GenB0Tag::PcheckDecay(genpart, 20443, 111)) {
    return +1 * (100000 * m_nPhotos + 1810);
  }//B0 decays to chi_c1 pi0
  if (GenB0Tag::PcheckDecay(genpart, 445, 310)) {
    return +1 * (100000 * m_nPhotos + 1811);
  }//B0 decays to chi_c2 K_S0
  if (GenB0Tag::PcheckDecay(genpart, 445, 130)) {
    return +1 * (100000 * m_nPhotos + 1812);
  }//B0 decays to chi_c2 K_L0
  if (GenB0Tag::PcheckDecay(genpart, 445, 313)) {
    return +1 * (100000 * m_nPhotos + 1813);
  }//B0 decays to chi_c2 K*0
  if (GenB0Tag::PcheckDecay(genpart, 445, 321, -211)) {
    return +1 * (100000 * m_nPhotos + 1814);
  }//B0 decays to chi_c2 K+ pi-
  if (GenB0Tag::PcheckDecay(genpart, 445, 311, 111)) {
    return +1 * (100000 * m_nPhotos + 1815);
  }//B0 decays to chi_c2 K0 pi0
  if (GenB0Tag::PcheckDecay(genpart, 445, 311, -211, 211)) {
    return +1 * (100000 * m_nPhotos + 1816);
  }//B0 decays to chi_c2 K0 pi- pi+
  if (GenB0Tag::PcheckDecay(genpart, 445, 321, -211, 111)) {
    return +1 * (100000 * m_nPhotos + 1817);
  }//B0 decays to chi_c2 K+ pi- pi0
  if (GenB0Tag::PcheckDecay(genpart, 445, 311, 111, 111)) {
    return +1 * (100000 * m_nPhotos + 1818);
  }//B0 decays to chi_c2 K0 pi0 pi0
  if (GenB0Tag::PcheckDecay(genpart, 30443, 310)) {
    return +1 * (100000 * m_nPhotos + 1819);
  }//B0 decays to psi(3770) K_S0
  if (GenB0Tag::PcheckDecay(genpart, 30443, 130)) {
    return +1 * (100000 * m_nPhotos + 1820);
  }//B0 decays to psi(3770) K_L0
  if (GenB0Tag::PcheckDecay(genpart, 30443, 313)) {
    return +1 * (100000 * m_nPhotos + 1821);
  }//B0 decays to psi(3770) K*S
  if (GenB0Tag::PcheckDecay(genpart, 30443, 313)) {
    return +1 * (100000 * m_nPhotos + 1822);
  }//B0 decays to psi(3770) K*L
  if (GenB0Tag::PcheckDecay(genpart, 30443, 313)) {
    return +1 * (100000 * m_nPhotos + 1823);
  }//B0 decays to psi(3770) K*0T
  if (GenB0Tag::PcheckDecay(genpart, 30443, 311, 111)) {
    return +1 * (100000 * m_nPhotos + 1824);
  }//B0 decays to psi(3770) K0 pi0
  if (GenB0Tag::PcheckDecay(genpart, 30443, 311, -211, 211)) {
    return +1 * (100000 * m_nPhotos + 1825);
  }//B0 decays to psi(3770) K0 pi- pi+
  if (GenB0Tag::PcheckDecay(genpart, 30443, 311, 111, 111)) {
    return +1 * (100000 * m_nPhotos + 1826);
  }//B0 decays to psi(3770) K0 pi0 pi0
  if (GenB0Tag::PcheckDecay(genpart, 30443, 321, -211, 111)) {
    return +1 * (100000 * m_nPhotos + 1827);
  }//B0 decays to psi(3770) K+ pi- pi0
  if (GenB0Tag::PcheckDecay(genpart, 30443, 10313)) {
    return +1 * (100000 * m_nPhotos + 1828);
  }//B0 decays to psi(3770) K_10
  if (GenB0Tag::PcheckDecay(genpart, -411, 411)) {
    return +1 * (100000 * m_nPhotos + 1829);
  }//B0 decays to D- D+
  if (GenB0Tag::PcheckDecay(genpart, -413, 411)) {
    return +1 * (100000 * m_nPhotos + 1830);
  }//B0 decays to D*- D+
  if (GenB0Tag::PcheckDecay(genpart, 413, -411)) {
    return +1 * (100000 * m_nPhotos + 1831);
  }//B0 decays to D*+ D-
  if (GenB0Tag::PcheckDecay(genpart, -413, 413)) {
    return +1 * (100000 * m_nPhotos + 1832);
  }//B0 decays to D*- D*+
  if (GenB0Tag::PcheckDecay(genpart, -411, 431)) {
    return +1 * (100000 * m_nPhotos + 1833);
  }//B0 decays to D- D_s+
  if (GenB0Tag::PcheckDecay(genpart, -413, 431)) {
    return +1 * (100000 * m_nPhotos + 1834);
  }//B0 decays to D*- D_s+
  if (GenB0Tag::PcheckDecay(genpart, 433, -411)) {
    return +1 * (100000 * m_nPhotos + 1835);
  }//B0 decays to D_s*+ D-
  if (GenB0Tag::PcheckDecay(genpart, 433, -413)) {
    return +1 * (100000 * m_nPhotos + 1836);
  }//B0 decays to D_s*+ D*-
  if (GenB0Tag::PcheckDecay(genpart, -20413, 431)) {
    return +1 * (100000 * m_nPhotos + 1837);
  }//B0 decays to D'_1- D_s+
  if (GenB0Tag::PcheckDecay(genpart, -20413, 433)) {
    return +1 * (100000 * m_nPhotos + 1838);
  }//B0 decays to D'_1- D_s*+
  if (GenB0Tag::PcheckDecay(genpart, -10413, 431)) {
    return +1 * (100000 * m_nPhotos + 1839);
  }//B0 decays to D_1- D_s+
  if (GenB0Tag::PcheckDecay(genpart, -10413, 433)) {
    return +1 * (100000 * m_nPhotos + 1840);
  }//B0 decays to D_1- D_s*+
  if (GenB0Tag::PcheckDecay(genpart, -415, 431)) {
    return +1 * (100000 * m_nPhotos + 1841);
  }//B0 decays to D_2*- D_s+
  if (GenB0Tag::PcheckDecay(genpart, -415, 433)) {
    return +1 * (100000 * m_nPhotos + 1842);
  }//B0 decays to D_2*- D_s*+
  if (GenB0Tag::PcheckDecay(genpart, 431, -411, 111)) {
    return +1 * (100000 * m_nPhotos + 1843);
  }//B0 decays to D_s+ D- pi0
  if (GenB0Tag::PcheckDecay(genpart, 431, -421, -211)) {
    return +1 * (100000 * m_nPhotos + 1844);
  }//B0 decays to D_s+ anti-D0 pi-
  if (GenB0Tag::PcheckDecay(genpart, 433, -411, 111)) {
    return +1 * (100000 * m_nPhotos + 1845);
  }//B0 decays to D_s*+ D- pi0
  if (GenB0Tag::PcheckDecay(genpart, 433, -421, -211)) {
    return +1 * (100000 * m_nPhotos + 1846);
  }//B0 decays to D_s*+ anti-D0 pi-
  if (GenB0Tag::PcheckDecay(genpart, 431, -411, -211, 211)) {
    return +1 * (100000 * m_nPhotos + 1847);
  }//B0 decays to D_s+ D- pi- pi+
  if (GenB0Tag::PcheckDecay(genpart, 431, -411, 111, 111)) {
    return +1 * (100000 * m_nPhotos + 1848);
  }//B0 decays to D_s+ D- pi0 pi0
  if (GenB0Tag::PcheckDecay(genpart, 431, -421, -211, 111)) {
    return +1 * (100000 * m_nPhotos + 1849);
  }//B0 decays to D_s+ anti-D0 pi- pi0
  if (GenB0Tag::PcheckDecay(genpart, 433, -411, -211, 211)) {
    return +1 * (100000 * m_nPhotos + 1850);
  }//B0 decays to D_s*+ D- pi- pi+
  if (GenB0Tag::PcheckDecay(genpart, 433, -411, 111, 111)) {
    return +1 * (100000 * m_nPhotos + 1851);
  }//B0 decays to D_s*+ D- pi0 pi0
  if (GenB0Tag::PcheckDecay(genpart, 433, -421, -211, 111)) {
    return +1 * (100000 * m_nPhotos + 1852);
  }//B0 decays to D_s*+ anti-D0 pi- pi0
  if (GenB0Tag::PcheckDecay(genpart, -411, 421, 321)) {
    return +1 * (100000 * m_nPhotos + 1853);
  }//B0 decays to D- D0 K+
  if (GenB0Tag::PcheckDecay(genpart, -411, 423, 321)) {
    return +1 * (100000 * m_nPhotos + 1854);
  }//B0 decays to D- D*0 K+
  if (GenB0Tag::PcheckDecay(genpart, -413, 421, 321)) {
    return +1 * (100000 * m_nPhotos + 1855);
  }//B0 decays to D*- D0 K+
  if (GenB0Tag::PcheckDecay(genpart, -413, 423, 321)) {
    return +1 * (100000 * m_nPhotos + 1856);
  }//B0 decays to D*- D*0 K+
  if (GenB0Tag::PcheckDecay(genpart, -411, 411, 311)) {
    return +1 * (100000 * m_nPhotos + 1857);
  }//B0 decays to D- D+ K0
  if (GenB0Tag::PcheckDecay(genpart, -413, 411, 311)) {
    return +1 * (100000 * m_nPhotos + 1858);
  }//B0 decays to D*- D+ K0
  if (GenB0Tag::PcheckDecay(genpart, -411, 413, 311)) {
    return +1 * (100000 * m_nPhotos + 1859);
  }//B0 decays to D- D*+ K0
  if (GenB0Tag::PcheckDecay(genpart, -413, 413, 311)) {
    return +1 * (100000 * m_nPhotos + 1860);
  }//B0 decays to D*- D*+ K0
  if (GenB0Tag::PcheckDecay(genpart, 421, -421, 311)) {
    return +1 * (100000 * m_nPhotos + 1861);
  }//B0 decays to D0 anti-D0 K0
  if (GenB0Tag::PcheckDecay(genpart, 423, -421, 311)) {
    return +1 * (100000 * m_nPhotos + 1862);
  }//B0 decays to D*0 anti-D0 K0
  if (GenB0Tag::PcheckDecay(genpart, 421, -423, 311)) {
    return +1 * (100000 * m_nPhotos + 1863);
  }//B0 decays to D0 anti-D*0 K0
  if (GenB0Tag::PcheckDecay(genpart, 423, -423, 311)) {
    return +1 * (100000 * m_nPhotos + 1864);
  }//B0 decays to D*0 anti-D*0 K0
  if (GenB0Tag::PcheckDecay(genpart, -411, 421, 323)) {
    return +1 * (100000 * m_nPhotos + 1865);
  }//B0 decays to D- D0 K*+
  if (GenB0Tag::PcheckDecay(genpart, -413, 421, 323)) {
    return +1 * (100000 * m_nPhotos + 1866);
  }//B0 decays to D*- D0 K*+
  if (GenB0Tag::PcheckDecay(genpart, -411, 423, 323)) {
    return +1 * (100000 * m_nPhotos + 1867);
  }//B0 decays to D- D*0 K*+
  if (GenB0Tag::PcheckDecay(genpart, -413, 423, 323)) {
    return +1 * (100000 * m_nPhotos + 1868);
  }//B0 decays to D*- D*0 K*+
  if (GenB0Tag::PcheckDecay(genpart, -411, 411, 313)) {
    return +1 * (100000 * m_nPhotos + 1869);
  }//B0 decays to D- D+ K*0
  if (GenB0Tag::PcheckDecay(genpart, -413, 411, 313)) {
    return +1 * (100000 * m_nPhotos + 1870);
  }//B0 decays to D*- D+ K*0
  if (GenB0Tag::PcheckDecay(genpart, -411, 413, 313)) {
    return +1 * (100000 * m_nPhotos + 1871);
  }//B0 decays to D- D*+ K*0
  if (GenB0Tag::PcheckDecay(genpart, -413, 413, 313)) {
    return +1 * (100000 * m_nPhotos + 1872);
  }//B0 decays to D*- D*+ K*0
  if (GenB0Tag::PcheckDecay(genpart, 421, -421, 313)) {
    return +1 * (100000 * m_nPhotos + 1873);
  }//B0 decays to D0 anti-D0 K*0
  if (GenB0Tag::PcheckDecay(genpart, 421, -423, 313)) {
    return +1 * (100000 * m_nPhotos + 1874);
  }//B0 decays to D0 anti-D*0 K*0
  if (GenB0Tag::PcheckDecay(genpart, 423, -421, 313)) {
    return +1 * (100000 * m_nPhotos + 1875);
  }//B0 decays to D*0 anti-D0 K*0
  if (GenB0Tag::PcheckDecay(genpart, 423, -423, 313)) {
    return +1 * (100000 * m_nPhotos + 1876);
  }//B0 decays to D*0 anti-D*0 K*0
  if (GenB0Tag::PcheckDecay(genpart, -413, 211)) {
    return +1 * (100000 * m_nPhotos + 1877);
  }//B0 decays to D*- pi+
  if (GenB0Tag::PcheckDecay(genpart, -411, 211)) {
    return +1 * (100000 * m_nPhotos + 1878);
  }//B0 decays to D- pi+
  if (GenB0Tag::PcheckDecay(genpart, 213, -411)) {
    return +1 * (100000 * m_nPhotos + 1879);
  }//B0 decays to rho+ D-
  if (GenB0Tag::PcheckDecay(genpart, 213, -413)) {
    return +1 * (100000 * m_nPhotos + 1880);
  }//B0 decays to rho+ D*-
  if (GenB0Tag::PcheckDecay(genpart, -411, 211, 111)) {
    return +1 * (100000 * m_nPhotos + 1881);
  }//B0 decays to D- pi+ pi0
  if (GenB0Tag::PcheckDecay(genpart, -413, 211, 111)) {
    return +1 * (100000 * m_nPhotos + 1882);
  }//B0 decays to D*- pi+ pi0
  if (GenB0Tag::PcheckDecay(genpart, -421, -211, 211)) {
    return +1 * (100000 * m_nPhotos + 1883);
  }//B0 decays to anti-D0 pi- pi+
  if (GenB0Tag::PcheckDecay(genpart, -423, -211, 211)) {
    return +1 * (100000 * m_nPhotos + 1884);
  }//B0 decays to anti-D*0 pi- pi+
  if (GenB0Tag::PcheckDecay(genpart, -423, 111, 111)) {
    return +1 * (100000 * m_nPhotos + 1885);
  }//B0 decays to anti-D*0 pi0 pi0
  if (GenB0Tag::PcheckDecay(genpart, 20213, -411)) {
    return +1 * (100000 * m_nPhotos + 1886);
  }//B0 decays to a_1+ D-
  if (GenB0Tag::PcheckDecay(genpart, -411, 113, 211)) {
    return +1 * (100000 * m_nPhotos + 1887);
  }//B0 decays to D- rho0 pi+
  if (GenB0Tag::PcheckDecay(genpart, -411, 213, 111)) {
    return +1 * (100000 * m_nPhotos + 1888);
  }//B0 decays to D- rho+ pi0
  if (GenB0Tag::PcheckDecay(genpart, -411, -211, 211, 211)) {
    return +1 * (100000 * m_nPhotos + 1889);
  }//B0 decays to D- pi- pi+ pi+
  if (GenB0Tag::PcheckDecay(genpart, -411, 111, 211, 111)) {
    return +1 * (100000 * m_nPhotos + 1890);
  }//B0 decays to D- pi0 pi+ pi0
  if (GenB0Tag::PcheckDecay(genpart, -421, -211, 211, 111)) {
    return +1 * (100000 * m_nPhotos + 1891);
  }//B0 decays to anti-D0 pi- pi+ pi0
  if (GenB0Tag::PcheckDecay(genpart, -421, 111, 111, 111)) {
    return +1 * (100000 * m_nPhotos + 1892);
  }//B0 decays to anti-D0 pi0 pi0 pi0
  if (GenB0Tag::PcheckDecay(genpart, -413, 20213)) {
    return +1 * (100000 * m_nPhotos + 1893);
  }//B0 decays to D*- a_1+
  if (GenB0Tag::PcheckDecay(genpart, -413, 113, 211)) {
    return +1 * (100000 * m_nPhotos + 1894);
  }//B0 decays to D*- rho0 pi+
  if (GenB0Tag::PcheckDecay(genpart, -413, 213, 111)) {
    return +1 * (100000 * m_nPhotos + 1895);
  }//B0 decays to D*- rho+ pi0
  if (GenB0Tag::PcheckDecay(genpart, -413, -211, 211, 211)) {
    return +1 * (100000 * m_nPhotos + 1896);
  }//B0 decays to D*- pi- pi+ pi+
  if (GenB0Tag::PcheckDecay(genpart, -413, 111, 211, 111)) {
    return +1 * (100000 * m_nPhotos + 1897);
  }//B0 decays to D*- pi0 pi+ pi0
  if (GenB0Tag::PcheckDecay(genpart, -423, -211, 211, 111)) {
    return +1 * (100000 * m_nPhotos + 1898);
  }//B0 decays to anti-D*0 pi- pi+ pi0
  if (GenB0Tag::PcheckDecay(genpart, -423, 111, 111, 111)) {
    return +1 * (100000 * m_nPhotos + 1899);
  }//B0 decays to anti-D*0 pi0 pi0 pi0
  if (GenB0Tag::PcheckDecay(genpart, -10413, 211)) {
    return +1 * (100000 * m_nPhotos + 1900);
  }//B0 decays to D_1- pi+
  if (GenB0Tag::PcheckDecay(genpart, -20413, 211)) {
    return +1 * (100000 * m_nPhotos + 1901);
  }//B0 decays to D'_1- pi+
  if (GenB0Tag::PcheckDecay(genpart, -10411, 211)) {
    return +1 * (100000 * m_nPhotos + 1902);
  }//B0 decays to D_0*- pi+
  if (GenB0Tag::PcheckDecay(genpart, -415, 211)) {
    return +1 * (100000 * m_nPhotos + 1903);
  }//B0 decays to D_2*- pi+
  if (GenB0Tag::PcheckDecay(genpart, -10413, 213)) {
    return +1 * (100000 * m_nPhotos + 1904);
  }//B0 decays to D_1- rho+
  if (GenB0Tag::PcheckDecay(genpart, -20413, 213)) {
    return +1 * (100000 * m_nPhotos + 1905);
  }//B0 decays to D'_1- rho+
  if (GenB0Tag::PcheckDecay(genpart, -415, 213)) {
    return +1 * (100000 * m_nPhotos + 1906);
  }//B0 decays to D_2*- rho+
  if (GenB0Tag::PcheckDecay(genpart, -413, 321)) {
    return +1 * (100000 * m_nPhotos + 1907);
  }//B0 decays to D*- K+
  if (GenB0Tag::PcheckDecay(genpart, -411, 321)) {
    return +1 * (100000 * m_nPhotos + 1908);
  }//B0 decays to D- K+
  if (GenB0Tag::PcheckDecay(genpart, -413, 323)) {
    return +1 * (100000 * m_nPhotos + 1909);
  }//B0 decays to D*- K*+
  if (GenB0Tag::PcheckDecay(genpart, 323, -411)) {
    return +1 * (100000 * m_nPhotos + 1910);
  }//B0 decays to K*+ D-
  if (GenB0Tag::PcheckDecay(genpart, -423, -311)) {
    return +1 * (100000 * m_nPhotos + 1911);
  }//B0 decays to anti-D*0 anti-K0
  if (GenB0Tag::PcheckDecay(genpart, -421, -311)) {
    return +1 * (100000 * m_nPhotos + 1912);
  }//B0 decays to anti-D0 anti-K0
  if (GenB0Tag::PcheckDecay(genpart, 313, -421)) {
    return +1 * (100000 * m_nPhotos + 1913);
  }//B0 decays to K*0 anti-D0
  if (GenB0Tag::PcheckDecay(genpart, 313, 421)) {
    return +1 * (100000 * m_nPhotos + 1914);
  }//B0 decays to K*0 D0
  if (GenB0Tag::PcheckDecay(genpart, -423, 313)) {
    return +1 * (100000 * m_nPhotos + 1915);
  }//B0 decays to anti-D*0 K*0
  if (GenB0Tag::PcheckDecay(genpart, 423, 313)) {
    return +1 * (100000 * m_nPhotos + 1916);
  }//B0 decays to D*0 K*0
  if (GenB0Tag::PcheckDecay(genpart, -421, 111)) {
    return +1 * (100000 * m_nPhotos + 1917);
  }//B0 decays to anti-D0 pi0
  if (GenB0Tag::PcheckDecay(genpart, -423, 111)) {
    return +1 * (100000 * m_nPhotos + 1918);
  }//B0 decays to anti-D*0 pi0
  if (GenB0Tag::PcheckDecay(genpart, 113, -421)) {
    return +1 * (100000 * m_nPhotos + 1919);
  }//B0 decays to rho0 anti-D0
  if (GenB0Tag::PcheckDecay(genpart, -423, 113)) {
    return +1 * (100000 * m_nPhotos + 1920);
  }//B0 decays to anti-D*0 rho0
  if (GenB0Tag::PcheckDecay(genpart, -421, 221)) {
    return +1 * (100000 * m_nPhotos + 1921);
  }//B0 decays to anti-D0 eta
  if (GenB0Tag::PcheckDecay(genpart, -423, 221)) {
    return +1 * (100000 * m_nPhotos + 1922);
  }//B0 decays to anti-D*0 eta
  if (GenB0Tag::PcheckDecay(genpart, -421, 331)) {
    return +1 * (100000 * m_nPhotos + 1923);
  }//B0 decays to anti-D0 eta'
  if (GenB0Tag::PcheckDecay(genpart, -423, 331)) {
    return +1 * (100000 * m_nPhotos + 1924);
  }//B0 decays to anti-D*0 eta'
  if (GenB0Tag::PcheckDecay(genpart, 223, -421)) {
    return +1 * (100000 * m_nPhotos + 1925);
  }//B0 decays to omega anti-D0
  if (GenB0Tag::PcheckDecay(genpart, -423, 223)) {
    return +1 * (100000 * m_nPhotos + 1926);
  }//B0 decays to anti-D*0 omega
  if (GenB0Tag::PcheckDecay(genpart, 2212, -2212)) {
    return +1 * (100000 * m_nPhotos + 1927);
  }//B0 decays to p+ anti-p-
  if (GenB0Tag::PcheckDecay(genpart, 10431, -411)) {
    return +1 * (100000 * m_nPhotos + 1928);
  }//B0 decays to D_s0*+ D-
  if (GenB0Tag::PcheckDecay(genpart, -413, 10431)) {
    return +1 * (100000 * m_nPhotos + 1929);
  }//B0 decays to D*- D_s0*+
  if (GenB0Tag::PcheckDecay(genpart, 10433, -411)) {
    return +1 * (100000 * m_nPhotos + 1930);
  }//B0 decays to D_s1+ D-
  if (GenB0Tag::PcheckDecay(genpart, -413, 10433)) {
    return +1 * (100000 * m_nPhotos + 1931);
  }//B0 decays to D*- D_s1+
  if (GenB0Tag::PcheckDecay(genpart, -411, 321, -313)) {
    return +1 * (100000 * m_nPhotos + 1932);
  }//B0 decays to D- K+ anti-K*0
  if (GenB0Tag::PcheckDecay(genpart, -413, 321, -313)) {
    return +1 * (100000 * m_nPhotos + 1933);
  }//B0 decays to D*- K+ anti-K*0
  if (GenB0Tag::PcheckDecay(genpart, -413, 311, 211)) {
    return +1 * (100000 * m_nPhotos + 1934);
  }//B0 decays to D*- K0 pi+
  if (GenB0Tag::PcheckDecay(genpart, 413, 311, -211)) {
    return +1 * (100000 * m_nPhotos + 1935);
  }//B0 decays to D*+ K0 pi-
  if (GenB0Tag::PcheckDecay(genpart, -411, 311, 211)) {
    return +1 * (100000 * m_nPhotos + 1936);
  }//B0 decays to D- K0 pi+
  if (GenB0Tag::PcheckDecay(genpart, -411, 223, 211)) {
    return +1 * (100000 * m_nPhotos + 1937);
  }//B0 decays to D- omega pi+
  if (GenB0Tag::PcheckDecay(genpart, -413, 223, 211)) {
    return +1 * (100000 * m_nPhotos + 1938);
  }//B0 decays to D*- omega pi+
  if (GenB0Tag::PcheckDecay(genpart, 20433, -411)) {
    return +1 * (100000 * m_nPhotos + 1939);
  }//B0 decays to D'_s1+ D-
  if (GenB0Tag::PcheckDecay(genpart, 20433, -413)) {
    return +1 * (100000 * m_nPhotos + 1940);
  }//B0 decays to D'_s1+ D*-
  if (GenB0Tag::PcheckDecay(genpart, -4101, 2101)) {
    return +1 * (100000 * m_nPhotos + 1941);
  }//B0 decays to anti-cd_0 ud_0
  if (GenB0Tag::PcheckDecay(genpart, -4103, 2103)) {
    return +1 * (100000 * m_nPhotos + 1942);
  }//B0 decays to anti-cd_1 ud_1
  if (GenB0Tag::PcheckDecay(genpart, -4301, 2101)) {
    return +1 * (100000 * m_nPhotos + 1943);
  }//B0 decays to anti-cs_0 ud_0
  if (GenB0Tag::PcheckDecay(genpart, -4303, 2103)) {
    return +1 * (100000 * m_nPhotos + 1944);
  }//B0 decays to anti-cs_1 ud_1
  if (GenB0Tag::PcheckDecay(genpart, 2, -1, -4, 1)) {
    return +1 * (100000 * m_nPhotos + 1945);
  }//B0 decays to u anti-d anti-c d
  if (GenB0Tag::PcheckDecay(genpart, 2, -1, -4, 1)) {
    return +1 * (100000 * m_nPhotos + 1946);
  }//B0 decays to u anti-d anti-c d
  if (GenB0Tag::PcheckDecay(genpart, 2, -3, -4, 1)) {
    return +1 * (100000 * m_nPhotos + 1947);
  }//B0 decays to u anti-s anti-c d
  if (GenB0Tag::PcheckDecay(genpart, 2, -4, -1, 1)) {
    return +1 * (100000 * m_nPhotos + 1948);
  }//B0 decays to u anti-c anti-d d
  if (GenB0Tag::PcheckDecay(genpart, 2, -4, -3, 1)) {
    return +1 * (100000 * m_nPhotos + 1949);
  }//B0 decays to u anti-c anti-s d
  if (GenB0Tag::PcheckDecay(genpart, 4, -3, -4, 1)) {
    return +1 * (100000 * m_nPhotos + 1950);
  }//B0 decays to c anti-s anti-c d
  if (GenB0Tag::PcheckDecay(genpart, 4, -1, -4, 1)) {
    return +1 * (100000 * m_nPhotos + 1951);
  }//B0 decays to c anti-d anti-c d
  if (GenB0Tag::PcheckDecay(genpart, 2, -1, -2, 1)) {
    return +1 * (100000 * m_nPhotos + 1952);
  }//B0 decays to u anti-d anti-u d
  if (GenB0Tag::PcheckDecay(genpart, 4, -3, -2, 1)) {
    return +1 * (100000 * m_nPhotos + 1953);
  }//B0 decays to c anti-s anti-u d
  if (GenB0Tag::PcheckDecay(genpart, 2, -2, -1, 1)) {
    return +1 * (100000 * m_nPhotos + 1954);
  }//B0 decays to u anti-u anti-d d
  if (GenB0Tag::PcheckDecay(genpart, 1, -1, -1, 1)) {
    return +1 * (100000 * m_nPhotos + 1955);
  }//B0 decays to d anti-d anti-d d
  if (GenB0Tag::PcheckDecay(genpart, 3, -3, -1, 1)) {
    return +1 * (100000 * m_nPhotos + 1956);
  }//B0 decays to s anti-s anti-d d
  if (GenB0Tag::PcheckDecay(genpart, 2, -2, -3, 1)) {
    return +1 * (100000 * m_nPhotos + 1957);
  }//B0 decays to u anti-u anti-s d
  if (GenB0Tag::PcheckDecay(genpart, 1, -1, -3, 1)) {
    return +1 * (100000 * m_nPhotos + 1958);
  }//B0 decays to d anti-d anti-s d
  if (GenB0Tag::PcheckDecay(genpart, 3, -3, -3, 1)) {
    return +1 * (100000 * m_nPhotos + 1959);
  }//B0 decays to s anti-s anti-s d
  if (GenB0Tag::PcheckDecay(genpart, -3, 1)) {
    return +1 * (100000 * m_nPhotos + 1960);
  }//B0 decays to anti-s d
  if (GenB0Tag::PcheckDecay(genpart, -421, 311)) {
    return +1 * (100000 * m_nPhotos + 1961);
  }//B0 decays to anti-D0 K0
  if (GenB0Tag::PcheckDecay(genpart, -421, 321, -211)) {
    return +1 * (100000 * m_nPhotos + 1962);
  }//B0 decays to anti-D0 K+ pi-
  if (GenB0Tag::PcheckDecay(genpart, 421, 321, -211)) {
    return +1 * (100000 * m_nPhotos + 1963);
  }//B0 decays to D0 K+ pi-
  if (GenB0Tag::PcheckDecay(genpart, -423, 311)) {
    return +1 * (100000 * m_nPhotos + 1964);
  }//B0 decays to anti-D*0 K0
  if (GenB0Tag::PcheckDecay(genpart, 423, 211, 211, -211, -211)) {
    return +1 * (100000 * m_nPhotos + 1965);
  }//B0 decays to D*0 pi+ pi+ pi- pi-
  if (GenB0Tag::PcheckDecay(genpart, 441, 311)) {
    return +1 * (100000 * m_nPhotos + 1966);
  }//B0 decays to eta_c K0
  if (GenB0Tag::PcheckDecay(genpart, 311, 421, -421, 111)) {
    return +1 * (100000 * m_nPhotos + 1967);
  }//B0 decays to K0 D0 anti-D0 pi0
  if (GenB0Tag::PcheckDecay(genpart, 100443, 311)) {
    return +1 * (100000 * m_nPhotos + 1968);
  }//B0 decays to psi(2S) K0
  if (GenB0Tag::PcheckDecay(genpart, 10441, 311)) {
    return +1 * (100000 * m_nPhotos + 1969);
  }//B0 decays to chi_c0 K0
  if (GenB0Tag::PcheckDecay(genpart, 20443, 311)) {
    return +1 * (100000 * m_nPhotos + 1970);
  }//B0 decays to chi_c1 K0
  if (GenB0Tag::PcheckDecay(genpart, 20443, -321, 211)) {
    return +1 * (100000 * m_nPhotos + 1971);
  }//B0 decays to chi_c1 K- pi+
  if (GenB0Tag::PcheckDecay(genpart, 311, 113)) {
    return +1 * (100000 * m_nPhotos + 1972);
  }//B0 decays to K0 rho0
  if (GenB0Tag::PcheckDecay(genpart, 221, 10311)) {
    return +1 * (100000 * m_nPhotos + 1973);
  }//B0 decays to eta K_0*0
  if (GenB0Tag::PcheckDecay(genpart, 221, 315)) {
    return +1 * (100000 * m_nPhotos + 1974);
  }//B0 decays to eta K_2*0
  if (GenB0Tag::PcheckDecay(genpart, 223, 10311)) {
    return +1 * (100000 * m_nPhotos + 1975);
  }//B0 decays to omega K_0*0
  if (GenB0Tag::PcheckDecay(genpart, 223, 315)) {
    return +1 * (100000 * m_nPhotos + 1976);
  }//B0 decays to omega K_2*0
  if (GenB0Tag::PcheckDecay(genpart, 225, 311)) {
    return +1 * (100000 * m_nPhotos + 1977);
  }//B0 decays to f_2 K0
  if (GenB0Tag::PcheckDecay(genpart, 10221, 321, -211)) {
    return +1 * (100000 * m_nPhotos + 1978);
  }//B0 decays to f_0 K+ pi-
  if (GenB0Tag::PcheckDecay(genpart, 315, 333)) {
    return +1 * (100000 * m_nPhotos + 1979);
  }//B0 decays to K_2*0 phi
  if (GenB0Tag::PcheckDecay(genpart, 221, 311, 22)) {
    return +1 * (100000 * m_nPhotos + 1980);
  }//B0 decays to eta K0 gamma
  if (GenB0Tag::PcheckDecay(genpart, 2212, -2212, 311)) {
    return +1 * (100000 * m_nPhotos + 1981);
  }//B0 decays to p+ anti-p- K0
  if (GenB0Tag::PcheckDecay(genpart, 2212, -2212, 313)) {
    return +1 * (100000 * m_nPhotos + 1982);
  }//B0 decays to p+ anti-p- K*0
  if (GenB0Tag::PcheckDecay(genpart, 2212, -3122, -211)) {
    return +1 * (100000 * m_nPhotos + 1983);
  }//B0 decays to p+ anti-Lambda0 pi-
  if (GenB0Tag::PcheckDecay(genpart, -3122, 3122, 311)) {
    return +1 * (100000 * m_nPhotos + 1984);
  }//B0 decays to anti-Lambda0 Lambda0 K0
  if (GenB0Tag::PcheckDecay(genpart, -3122, 3122, 313)) {
    return +1 * (100000 * m_nPhotos + 1985);
  }//B0 decays to anti-Lambda0 Lambda0 K*0
  if (GenB0Tag::PcheckDecay(genpart, -3122, 3122, 421)) {
    return +1 * (100000 * m_nPhotos + 1986);
  }//B0 decays to anti-Lambda0 Lambda0 D0
  if (GenB0Tag::PcheckDecay(genpart, -421, 2212, -2212)) {
    return +1 * (100000 * m_nPhotos + 1987);
  }//B0 decays to anti-D0 p+ anti-p-
  if (GenB0Tag::PcheckDecay(genpart, -431, -3122, 2212)) {
    return +1 * (100000 * m_nPhotos + 1988);
  }//B0 decays to D_s- anti-Lambda0 p+
  if (GenB0Tag::PcheckDecay(genpart, -423, 2212, -2212)) {
    return +1 * (100000 * m_nPhotos + 1989);
  }//B0 decays to anti-D*0 p+ anti-p-
  if (GenB0Tag::PcheckDecay(genpart, -413, 2212, -2112)) {
    return +1 * (100000 * m_nPhotos + 1990);
  }//B0 decays to D*- p+ anti-n0
  if (GenB0Tag::PcheckDecay(genpart, -411, 2212, -2212, 211)) {
    return +1 * (100000 * m_nPhotos + 1991);
  }//B0 decays to D- p+ anti-p- pi+
  if (GenB0Tag::PcheckDecay(genpart, -413, 2212, -2212, 211)) {
    return +1 * (100000 * m_nPhotos + 1992);
  }//B0 decays to D*- p+ anti-p- pi+
  if (GenB0Tag::PcheckDecay(genpart, -4122, 2212)) {
    return +1 * (100000 * m_nPhotos + 1993);
  }//B0 decays to anti-Lambda_c- p+
  if (GenB0Tag::PcheckDecay(genpart, -4122, 2212, 211, -211)) {
    return +1 * (100000 * m_nPhotos + 1994);
  }//B0 decays to anti-Lambda_c- p+ pi+ pi-
  if (GenB0Tag::PcheckDecay(genpart, -4224, 2212, 211)) {
    return +1 * (100000 * m_nPhotos + 1995);
  }//B0 decays to anti-Sigma_c*-- p+ pi+
  if (GenB0Tag::PcheckDecay(genpart, -4112, 2212, -211)) {
    return +1 * (100000 * m_nPhotos + 1996);
  }//B0 decays to anti-Sigma_c0 p+ pi-
  if (GenB0Tag::PcheckDecay(genpart, -4222, 2212, 211)) {
    return +1 * (100000 * m_nPhotos + 1997);
  }//B0 decays to anti-Sigma_c-- p+ pi+
  if (GenB0Tag::PcheckDecay(genpart, -413, 211, 211, -211, 111)) {
    return +1 * (100000 * m_nPhotos + 1998);
  }//B0 decays to D*- pi+ pi+ pi- pi0
  if (GenB0Tag::PcheckDecay(genpart, -413, 211, 211, 211, -211, -211)) {
    return +1 * (100000 * m_nPhotos + 1999);
  }//B0 decays to D*- pi+ pi+ pi+ pi- pi-
  if (GenB0Tag::PcheckDecay(genpart, -431, 211, 311)) {
    return +1 * (100000 * m_nPhotos + 2000);
  }//B0 decays to D_s- pi+ K0
  return +1 * genpart.size();
}// Rest of the B0 decays


int GenB0Tag::Mode_anti_B0(std::vector<int> genpart)
{
  if (GenB0Tag::PcheckDecay(genpart, 413, 11, -12)) {
    return -1 * (100000 * m_nPhotos + 1001);
  }//anti-B0 decays to D*+ e- anti-nu_e
  if (GenB0Tag::PcheckDecay(genpart, 411, 11, -12)) {
    return -1 * (100000 * m_nPhotos + 1002);
  }//anti-B0 decays to D+ e- anti-nu_e
  if (GenB0Tag::PcheckDecay(genpart, 10413, 11, -12)) {
    return -1 * (100000 * m_nPhotos + 1003);
  }//anti-B0 decays to D_1+ e- anti-nu_e
  if (GenB0Tag::PcheckDecay(genpart, 10411, 11, -12)) {
    return -1 * (100000 * m_nPhotos + 1004);
  }//anti-B0 decays to D_0*+ e- anti-nu_e
  if (GenB0Tag::PcheckDecay(genpart, 20413, 11, -12)) {
    return -1 * (100000 * m_nPhotos + 1005);
  }//anti-B0 decays to D'_1+ e- anti-nu_e
  if (GenB0Tag::PcheckDecay(genpart, 415, 11, -12)) {
    return -1 * (100000 * m_nPhotos + 1006);
  }//anti-B0 decays to D_2*+ e- anti-nu_e
  if (GenB0Tag::PcheckDecay(genpart, 423, 211, 11, -12)) {
    return -1 * (100000 * m_nPhotos + 1007);
  }//anti-B0 decays to D*0 pi+ e- anti-nu_e
  if (GenB0Tag::PcheckDecay(genpart, 413, 111, 11, -12)) {
    return -1 * (100000 * m_nPhotos + 1008);
  }//anti-B0 decays to D*+ pi0 e- anti-nu_e
  if (GenB0Tag::PcheckDecay(genpart, 421, 211, 11, -12)) {
    return -1 * (100000 * m_nPhotos + 1009);
  }//anti-B0 decays to D0 pi+ e- anti-nu_e
  if (GenB0Tag::PcheckDecay(genpart, 411, 111, 11, -12)) {
    return -1 * (100000 * m_nPhotos + 1010);
  }//anti-B0 decays to D+ pi0 e- anti-nu_e
  if (GenB0Tag::PcheckDecay(genpart, 411, 211, -211, 11, -12)) {
    return -1 * (100000 * m_nPhotos + 1011);
  }//anti-B0 decays to D+ pi+ pi- e- anti-nu_e
  if (GenB0Tag::PcheckDecay(genpart, 421, 211, 111, 11, -12)) {
    return -1 * (100000 * m_nPhotos + 1012);
  }//anti-B0 decays to D0 pi+ pi0 e- anti-nu_e
  if (GenB0Tag::PcheckDecay(genpart, 411, 111, 111, 11, -12)) {
    return -1 * (100000 * m_nPhotos + 1013);
  }//anti-B0 decays to D+ pi0 pi0 e- anti-nu_e
  if (GenB0Tag::PcheckDecay(genpart, 413, 211, -211, 11, -12)) {
    return -1 * (100000 * m_nPhotos + 1014);
  }//anti-B0 decays to D*+ pi+ pi- e- anti-nu_e
  if (GenB0Tag::PcheckDecay(genpart, 423, 211, 111, 11, -12)) {
    return -1 * (100000 * m_nPhotos + 1015);
  }//anti-B0 decays to D*0 pi+ pi0 e- anti-nu_e
  if (GenB0Tag::PcheckDecay(genpart, 413, 111, 111, 11, -12)) {
    return -1 * (100000 * m_nPhotos + 1016);
  }//anti-B0 decays to D*+ pi0 pi0 e- anti-nu_e
  if (GenB0Tag::PcheckDecay(genpart, 411, 221, 11, -12)) {
    return -1 * (100000 * m_nPhotos + 1017);
  }//anti-B0 decays to D+ eta e- anti-nu_e
  if (GenB0Tag::PcheckDecay(genpart, 413, 221, 11, -12)) {
    return -1 * (100000 * m_nPhotos + 1018);
  }//anti-B0 decays to D*+ eta e- anti-nu_e
  if (GenB0Tag::PcheckDecay(genpart, 413, 13, -14)) {
    return -1 * (100000 * m_nPhotos + 1019);
  }//anti-B0 decays to D*+ mu- anti-nu_mu
  if (GenB0Tag::PcheckDecay(genpart, 411, 13, -14)) {
    return -1 * (100000 * m_nPhotos + 1020);
  }//anti-B0 decays to D+ mu- anti-nu_mu
  if (GenB0Tag::PcheckDecay(genpart, 10413, 13, -14)) {
    return -1 * (100000 * m_nPhotos + 1021);
  }//anti-B0 decays to D_1+ mu- anti-nu_mu
  if (GenB0Tag::PcheckDecay(genpart, 10411, 13, -14)) {
    return -1 * (100000 * m_nPhotos + 1022);
  }//anti-B0 decays to D_0*+ mu- anti-nu_mu
  if (GenB0Tag::PcheckDecay(genpart, 20413, 13, -14)) {
    return -1 * (100000 * m_nPhotos + 1023);
  }//anti-B0 decays to D'_1+ mu- anti-nu_mu
  if (GenB0Tag::PcheckDecay(genpart, 415, 13, -14)) {
    return -1 * (100000 * m_nPhotos + 1024);
  }//anti-B0 decays to D_2*+ mu- anti-nu_mu
  if (GenB0Tag::PcheckDecay(genpart, 423, 211, 13, -14)) {
    return -1 * (100000 * m_nPhotos + 1025);
  }//anti-B0 decays to D*0 pi+ mu- anti-nu_mu
  if (GenB0Tag::PcheckDecay(genpart, 413, 111, 13, -14)) {
    return -1 * (100000 * m_nPhotos + 1026);
  }//anti-B0 decays to D*+ pi0 mu- anti-nu_mu
  if (GenB0Tag::PcheckDecay(genpart, 421, 211, 13, -14)) {
    return -1 * (100000 * m_nPhotos + 1027);
  }//anti-B0 decays to D0 pi+ mu- anti-nu_mu
  if (GenB0Tag::PcheckDecay(genpart, 411, 111, 13, -14)) {
    return -1 * (100000 * m_nPhotos + 1028);
  }//anti-B0 decays to D+ pi0 mu- anti-nu_mu
  if (GenB0Tag::PcheckDecay(genpart, 411, 211, -211, 13, -14)) {
    return -1 * (100000 * m_nPhotos + 1029);
  }//anti-B0 decays to D+ pi+ pi- mu- anti-nu_mu
  if (GenB0Tag::PcheckDecay(genpart, 421, 211, 111, 13, -14)) {
    return -1 * (100000 * m_nPhotos + 1030);
  }//anti-B0 decays to D0 pi+ pi0 mu- anti-nu_mu
  if (GenB0Tag::PcheckDecay(genpart, 411, 111, 111, 13, -14)) {
    return -1 * (100000 * m_nPhotos + 1031);
  }//anti-B0 decays to D+ pi0 pi0 mu- anti-nu_mu
  if (GenB0Tag::PcheckDecay(genpart, 413, 211, -211, 13, -14)) {
    return -1 * (100000 * m_nPhotos + 1032);
  }//anti-B0 decays to D*+ pi+ pi- mu- anti-nu_mu
  if (GenB0Tag::PcheckDecay(genpart, 423, 211, 111, 13, -14)) {
    return -1 * (100000 * m_nPhotos + 1033);
  }//anti-B0 decays to D*0 pi+ pi0 mu- anti-nu_mu
  if (GenB0Tag::PcheckDecay(genpart, 413, 111, 111, 13, -14)) {
    return -1 * (100000 * m_nPhotos + 1034);
  }//anti-B0 decays to D*+ pi0 pi0 mu- anti-nu_mu
  if (GenB0Tag::PcheckDecay(genpart, 411, 221, 13, -14)) {
    return -1 * (100000 * m_nPhotos + 1035);
  }//anti-B0 decays to D+ eta mu- anti-nu_mu
  if (GenB0Tag::PcheckDecay(genpart, 413, 221, 13, -14)) {
    return -1 * (100000 * m_nPhotos + 1036);
  }//anti-B0 decays to D*+ eta mu- anti-nu_mu
  if (GenB0Tag::PcheckDecay(genpart, 413, 15, -16)) {
    return -1 * (100000 * m_nPhotos + 1037);
  }//anti-B0 decays to D*+ tau- anti-nu_tau
  if (GenB0Tag::PcheckDecay(genpart, 411, 15, -16)) {
    return -1 * (100000 * m_nPhotos + 1038);
  }//anti-B0 decays to D+ tau- anti-nu_tau
  if (GenB0Tag::PcheckDecay(genpart, 10413, 15, -16)) {
    return -1 * (100000 * m_nPhotos + 1039);
  }//anti-B0 decays to D_1+ tau- anti-nu_tau
  if (GenB0Tag::PcheckDecay(genpart, 10411, 15, -16)) {
    return -1 * (100000 * m_nPhotos + 1040);
  }//anti-B0 decays to D_0*+ tau- anti-nu_tau
  if (GenB0Tag::PcheckDecay(genpart, 20413, 15, -16)) {
    return -1 * (100000 * m_nPhotos + 1041);
  }//anti-B0 decays to D'_1+ tau- anti-nu_tau
  if (GenB0Tag::PcheckDecay(genpart, 415, 15, -16)) {
    return -1 * (100000 * m_nPhotos + 1042);
  }//anti-B0 decays to D_2*+ tau- anti-nu_tau
  if (GenB0Tag::PcheckDecay(genpart, 211, 11, -12)) {
    return -1 * (100000 * m_nPhotos + 1043);
  }//anti-B0 decays to pi+ e- anti-nu_e
  if (GenB0Tag::PcheckDecay(genpart, 213, 11, -12)) {
    return -1 * (100000 * m_nPhotos + 1044);
  }//anti-B0 decays to rho+ e- anti-nu_e
  if (GenB0Tag::PcheckDecay(genpart, 100411, 11, -12)) {
    return -1 * (100000 * m_nPhotos + 1045);
  }//anti-B0 decays to D(2S)+ e- anti-nu_e
  if (GenB0Tag::PcheckDecay(genpart, 100413, 11, -12)) {
    return -1 * (100000 * m_nPhotos + 1046);
  }//anti-B0 decays to D*(2S)+ e- anti-nu_e
  if (GenB0Tag::PcheckDecay(genpart, 62, 11, -12)) {
    return -1 * (100000 * m_nPhotos + 1047);
  }//anti-B0 decays to Xu+ e- anti-nu_e
  if (GenB0Tag::PcheckDecay(genpart, 211, 13, -14)) {
    return -1 * (100000 * m_nPhotos + 1048);
  }//anti-B0 decays to pi+ mu- anti-nu_mu
  if (GenB0Tag::PcheckDecay(genpart, 213, 13, -14)) {
    return -1 * (100000 * m_nPhotos + 1049);
  }//anti-B0 decays to rho+ mu- anti-nu_mu
  if (GenB0Tag::PcheckDecay(genpart, 100411, 13, -14)) {
    return -1 * (100000 * m_nPhotos + 1050);
  }//anti-B0 decays to D(2S)+ mu- anti-nu_mu
  if (GenB0Tag::PcheckDecay(genpart, 100413, 13, -14)) {
    return -1 * (100000 * m_nPhotos + 1051);
  }//anti-B0 decays to D*(2S)+ mu- anti-nu_mu
  if (GenB0Tag::PcheckDecay(genpart, 62, 13, -14)) {
    return -1 * (100000 * m_nPhotos + 1052);
  }//anti-B0 decays to Xu+ mu- anti-nu_mu
  if (GenB0Tag::PcheckDecay(genpart, 211, 15, -16)) {
    return -1 * (100000 * m_nPhotos + 1053);
  }//anti-B0 decays to pi+ tau- anti-nu_tau
  if (GenB0Tag::PcheckDecay(genpart, 213, 15, -16)) {
    return -1 * (100000 * m_nPhotos + 1054);
  }//anti-B0 decays to rho+ tau- anti-nu_tau
  if (GenB0Tag::PcheckDecay(genpart, 20213, 15, -16)) {
    return -1 * (100000 * m_nPhotos + 1055);
  }//anti-B0 decays to a_1+ tau- anti-nu_tau
  if (GenB0Tag::PcheckDecay(genpart, 10213, 15, -16)) {
    return -1 * (100000 * m_nPhotos + 1056);
  }//anti-B0 decays to b_1+ tau- anti-nu_tau
  if (GenB0Tag::PcheckDecay(genpart, 10211, 15, -16)) {
    return -1 * (100000 * m_nPhotos + 1057);
  }//anti-B0 decays to a_0+ tau- anti-nu_tau
  if (GenB0Tag::PcheckDecay(genpart, 215, 15, -16)) {
    return -1 * (100000 * m_nPhotos + 1058);
  }//anti-B0 decays to a_2+ tau- anti-nu_tau
  if (GenB0Tag::PcheckDecay(genpart, 100213, 15, -16)) {
    return -1 * (100000 * m_nPhotos + 1059);
  }//anti-B0 decays to rho(2S)+ tau- anti-nu_tau
  if (GenB0Tag::PcheckDecay(genpart, 100411, 15, -16)) {
    return -1 * (100000 * m_nPhotos + 1060);
  }//anti-B0 decays to D(2S)+ tau- anti-nu_tau
  if (GenB0Tag::PcheckDecay(genpart, 100413, 15, -16)) {
    return -1 * (100000 * m_nPhotos + 1061);
  }//anti-B0 decays to D*(2S)+ tau- anti-nu_tau
  if (GenB0Tag::PcheckDecay(genpart, -431, 211)) {
    return -1 * (100000 * m_nPhotos + 1062);
  }//anti-B0 decays to D_s- pi+
  if (GenB0Tag::PcheckDecay(genpart, 431, -321)) {
    return -1 * (100000 * m_nPhotos + 1063);
  }//anti-B0 decays to D_s+ K-
  if (GenB0Tag::PcheckDecay(genpart, -433, 211)) {
    return -1 * (100000 * m_nPhotos + 1064);
  }//anti-B0 decays to D_s*- pi+
  if (GenB0Tag::PcheckDecay(genpart, 433, -321)) {
    return -1 * (100000 * m_nPhotos + 1065);
  }//anti-B0 decays to D_s*+ K-
  if (GenB0Tag::PcheckDecay(genpart, 213, -431)) {
    return -1 * (100000 * m_nPhotos + 1066);
  }//anti-B0 decays to rho+ D_s-
  if (GenB0Tag::PcheckDecay(genpart, -323, 431)) {
    return -1 * (100000 * m_nPhotos + 1067);
  }//anti-B0 decays to K*- D_s+
  if (GenB0Tag::PcheckDecay(genpart, -433, 213)) {
    return -1 * (100000 * m_nPhotos + 1068);
  }//anti-B0 decays to D_s*- rho+
  if (GenB0Tag::PcheckDecay(genpart, 433, -323)) {
    return -1 * (100000 * m_nPhotos + 1069);
  }//anti-B0 decays to D_s*+ K*-
  if (GenB0Tag::PcheckDecay(genpart, -313, 22)) {
    return -1 * (100000 * m_nPhotos + 1070);
  }//anti-B0 decays to anti-K*0 gamma
  if (GenB0Tag::PcheckDecay(genpart, 223, 22)) {
    return -1 * (100000 * m_nPhotos + 1071);
  }//anti-B0 decays to omega gamma
  if (GenB0Tag::PcheckDecay(genpart, 113, 22)) {
    return -1 * (100000 * m_nPhotos + 1072);
  }//anti-B0 decays to rho0 gamma
  if (GenB0Tag::PcheckDecay(genpart, -10313, 22)) {
    return -1 * (100000 * m_nPhotos + 1073);
  }//anti-B0 decays to anti-K_10 gamma
  if (GenB0Tag::PcheckDecay(genpart, -20313, 22)) {
    return -1 * (100000 * m_nPhotos + 1074);
  }//anti-B0 decays to anti-K'_10 gamma
  if (GenB0Tag::PcheckDecay(genpart, -315, 22)) {
    return -1 * (100000 * m_nPhotos + 1075);
  }//anti-B0 decays to anti-K_2*0 gamma
  if (GenB0Tag::PcheckDecay(genpart, -30313, 22)) {
    return -1 * (100000 * m_nPhotos + 1076);
  }//anti-B0 decays to anti-K''*0 gamma
  if (GenB0Tag::PcheckDecay(genpart, -30343, 22)) {
    return -1 * (100000 * m_nPhotos + 1077);
  }//anti-B0 decays to anti-Xsd gamma
  if (GenB0Tag::PcheckDecay(genpart, -311, -11, 11)) {
    return -1 * (100000 * m_nPhotos + 1078);
  }//anti-B0 decays to anti-K0 e+ e-
  if (GenB0Tag::PcheckDecay(genpart, -313, -11, 11)) {
    return -1 * (100000 * m_nPhotos + 1079);
  }//anti-B0 decays to anti-K*0 e+ e-
  if (GenB0Tag::PcheckDecay(genpart, -30343, -11, 11)) {
    return -1 * (100000 * m_nPhotos + 1080);
  }//anti-B0 decays to anti-Xsd e+ e-
  if (GenB0Tag::PcheckDecay(genpart, 211, -211, -11, 11)) {
    return -1 * (100000 * m_nPhotos + 1081);
  }//anti-B0 decays to pi+ pi- e+ e-
  if (GenB0Tag::PcheckDecay(genpart, -311, -13, 13)) {
    return -1 * (100000 * m_nPhotos + 1082);
  }//anti-B0 decays to anti-K0 mu+ mu-
  if (GenB0Tag::PcheckDecay(genpart, -313, -13, 13)) {
    return -1 * (100000 * m_nPhotos + 1083);
  }//anti-B0 decays to anti-K*0 mu+ mu-
  if (GenB0Tag::PcheckDecay(genpart, 211, -211, -13, 13)) {
    return -1 * (100000 * m_nPhotos + 1084);
  }//anti-B0 decays to pi+ pi- mu+ mu-
  if (GenB0Tag::PcheckDecay(genpart, -30343, -13, 13)) {
    return -1 * (100000 * m_nPhotos + 1085);
  }//anti-B0 decays to anti-Xsd mu+ mu-
  if (GenB0Tag::PcheckDecay(genpart, -311, -15, 15)) {
    return -1 * (100000 * m_nPhotos + 1086);
  }//anti-B0 decays to anti-K0 tau+ tau-
  if (GenB0Tag::PcheckDecay(genpart, -313, -15, 15)) {
    return -1 * (100000 * m_nPhotos + 1087);
  }//anti-B0 decays to anti-K*0 tau+ tau-
  if (GenB0Tag::PcheckDecay(genpart, -30343, -15, 15)) {
    return -1 * (100000 * m_nPhotos + 1088);
  }//anti-B0 decays to anti-Xsd tau+ tau-
  if (GenB0Tag::PcheckDecay(genpart, 211, -211)) {
    return -1 * (100000 * m_nPhotos + 1089);
  }//anti-B0 decays to pi+ pi-
  if (GenB0Tag::PcheckDecay(genpart, 111, 111)) {
    return -1 * (100000 * m_nPhotos + 1090);
  }//anti-B0 decays to pi0 pi0
  if (GenB0Tag::PcheckDecay(genpart, 111, 221)) {
    return -1 * (100000 * m_nPhotos + 1091);
  }//anti-B0 decays to pi0 eta
  if (GenB0Tag::PcheckDecay(genpart, 111, 331)) {
    return -1 * (100000 * m_nPhotos + 1092);
  }//anti-B0 decays to pi0 eta'
  if (GenB0Tag::PcheckDecay(genpart, 111, 10111)) {
    return -1 * (100000 * m_nPhotos + 1093);
  }//anti-B0 decays to pi0 a_00
  if (GenB0Tag::PcheckDecay(genpart, 111, 10221)) {
    return -1 * (100000 * m_nPhotos + 1094);
  }//anti-B0 decays to pi0 f_0
  if (GenB0Tag::PcheckDecay(genpart, 111, 223)) {
    return -1 * (100000 * m_nPhotos + 1095);
  }//anti-B0 decays to pi0 omega
  if (GenB0Tag::PcheckDecay(genpart, 111, 20113)) {
    return -1 * (100000 * m_nPhotos + 1096);
  }//anti-B0 decays to pi0 a_10
  if (GenB0Tag::PcheckDecay(genpart, 111, 10113)) {
    return -1 * (100000 * m_nPhotos + 1097);
  }//anti-B0 decays to pi0 b_10
  if (GenB0Tag::PcheckDecay(genpart, 221, 221)) {
    return -1 * (100000 * m_nPhotos + 1098);
  }//anti-B0 decays to eta eta
  if (GenB0Tag::PcheckDecay(genpart, 221, 331)) {
    return -1 * (100000 * m_nPhotos + 1099);
  }//anti-B0 decays to eta eta'
  if (GenB0Tag::PcheckDecay(genpart, 221, 10111)) {
    return -1 * (100000 * m_nPhotos + 1100);
  }//anti-B0 decays to eta a_00
  if (GenB0Tag::PcheckDecay(genpart, 221, 10221)) {
    return -1 * (100000 * m_nPhotos + 1101);
  }//anti-B0 decays to eta f_0
  if (GenB0Tag::PcheckDecay(genpart, 221, 113)) {
    return -1 * (100000 * m_nPhotos + 1102);
  }//anti-B0 decays to eta rho0
  if (GenB0Tag::PcheckDecay(genpart, 221, 223)) {
    return -1 * (100000 * m_nPhotos + 1103);
  }//anti-B0 decays to eta omega
  if (GenB0Tag::PcheckDecay(genpart, 221, 20113)) {
    return -1 * (100000 * m_nPhotos + 1104);
  }//anti-B0 decays to eta a_10
  if (GenB0Tag::PcheckDecay(genpart, 221, 10113)) {
    return -1 * (100000 * m_nPhotos + 1105);
  }//anti-B0 decays to eta b_10
  if (GenB0Tag::PcheckDecay(genpart, 331, 331)) {
    return -1 * (100000 * m_nPhotos + 1106);
  }//anti-B0 decays to eta' eta'
  if (GenB0Tag::PcheckDecay(genpart, 331, 10111)) {
    return -1 * (100000 * m_nPhotos + 1107);
  }//anti-B0 decays to eta' a_00
  if (GenB0Tag::PcheckDecay(genpart, 331, 10221)) {
    return -1 * (100000 * m_nPhotos + 1108);
  }//anti-B0 decays to eta' f_0
  if (GenB0Tag::PcheckDecay(genpart, 331, 113)) {
    return -1 * (100000 * m_nPhotos + 1109);
  }//anti-B0 decays to eta' rho0
  if (GenB0Tag::PcheckDecay(genpart, 331, 223)) {
    return -1 * (100000 * m_nPhotos + 1110);
  }//anti-B0 decays to eta' omega
  if (GenB0Tag::PcheckDecay(genpart, 331, 20113)) {
    return -1 * (100000 * m_nPhotos + 1111);
  }//anti-B0 decays to eta' a_10
  if (GenB0Tag::PcheckDecay(genpart, 331, 10113)) {
    return -1 * (100000 * m_nPhotos + 1112);
  }//anti-B0 decays to eta' b_10
  if (GenB0Tag::PcheckDecay(genpart, 10111, 10111)) {
    return -1 * (100000 * m_nPhotos + 1113);
  }//anti-B0 decays to a_00 a_00
  if (GenB0Tag::PcheckDecay(genpart, 10111, 10221)) {
    return -1 * (100000 * m_nPhotos + 1114);
  }//anti-B0 decays to a_00 f_0
  if (GenB0Tag::PcheckDecay(genpart, 10111, 113)) {
    return -1 * (100000 * m_nPhotos + 1115);
  }//anti-B0 decays to a_00 rho0
  if (GenB0Tag::PcheckDecay(genpart, 10111, 223)) {
    return -1 * (100000 * m_nPhotos + 1116);
  }//anti-B0 decays to a_00 omega
  if (GenB0Tag::PcheckDecay(genpart, 10111, 20113)) {
    return -1 * (100000 * m_nPhotos + 1117);
  }//anti-B0 decays to a_00 a_10
  if (GenB0Tag::PcheckDecay(genpart, 10111, 10113)) {
    return -1 * (100000 * m_nPhotos + 1118);
  }//anti-B0 decays to a_00 b_10
  if (GenB0Tag::PcheckDecay(genpart, 10221, 10221)) {
    return -1 * (100000 * m_nPhotos + 1119);
  }//anti-B0 decays to f_0 f_0
  if (GenB0Tag::PcheckDecay(genpart, 10221, 113)) {
    return -1 * (100000 * m_nPhotos + 1120);
  }//anti-B0 decays to f_0 rho0
  if (GenB0Tag::PcheckDecay(genpart, 10221, 223)) {
    return -1 * (100000 * m_nPhotos + 1121);
  }//anti-B0 decays to f_0 omega
  if (GenB0Tag::PcheckDecay(genpart, 10221, 20113)) {
    return -1 * (100000 * m_nPhotos + 1122);
  }//anti-B0 decays to f_0 a_10
  if (GenB0Tag::PcheckDecay(genpart, 10221, 10113)) {
    return -1 * (100000 * m_nPhotos + 1123);
  }//anti-B0 decays to f_0 b_10
  if (GenB0Tag::PcheckDecay(genpart, 333, 310)) {
    return -1 * (100000 * m_nPhotos + 1124);
  }//anti-B0 decays to phi K_S0
  if (GenB0Tag::PcheckDecay(genpart, 333, 130)) {
    return -1 * (100000 * m_nPhotos + 1125);
  }//anti-B0 decays to phi K_L0
  if (GenB0Tag::PcheckDecay(genpart, 221, 310)) {
    return -1 * (100000 * m_nPhotos + 1126);
  }//anti-B0 decays to eta K_S0
  if (GenB0Tag::PcheckDecay(genpart, 221, 130)) {
    return -1 * (100000 * m_nPhotos + 1127);
  }//anti-B0 decays to eta K_L0
  if (GenB0Tag::PcheckDecay(genpart, 331, 310)) {
    return -1 * (100000 * m_nPhotos + 1128);
  }//anti-B0 decays to eta' K_S0
  if (GenB0Tag::PcheckDecay(genpart, 331, 130)) {
    return -1 * (100000 * m_nPhotos + 1129);
  }//anti-B0 decays to eta' K_L0
  if (GenB0Tag::PcheckDecay(genpart, 223, 310)) {
    return -1 * (100000 * m_nPhotos + 1130);
  }//anti-B0 decays to omega K_S0
  if (GenB0Tag::PcheckDecay(genpart, 223, 130)) {
    return -1 * (100000 * m_nPhotos + 1131);
  }//anti-B0 decays to omega K_L0
  if (GenB0Tag::PcheckDecay(genpart, 333, 311)) {
    return -1 * (100000 * m_nPhotos + 1132);
  }//anti-B0 decays to phi K0
  if (GenB0Tag::PcheckDecay(genpart, 333, -311)) {
    return -1 * (100000 * m_nPhotos + 1133);
  }//anti-B0 decays to phi anti-K0
  if (GenB0Tag::PcheckDecay(genpart, 221, 311)) {
    return -1 * (100000 * m_nPhotos + 1134);
  }//anti-B0 decays to eta K0
  if (GenB0Tag::PcheckDecay(genpart, 221, -311)) {
    return -1 * (100000 * m_nPhotos + 1135);
  }//anti-B0 decays to eta anti-K0
  if (GenB0Tag::PcheckDecay(genpart, 331, 311)) {
    return -1 * (100000 * m_nPhotos + 1136);
  }//anti-B0 decays to eta' K0
  if (GenB0Tag::PcheckDecay(genpart, 331, -311)) {
    return -1 * (100000 * m_nPhotos + 1137);
  }//anti-B0 decays to eta' anti-K0
  if (GenB0Tag::PcheckDecay(genpart, 223, 311)) {
    return -1 * (100000 * m_nPhotos + 1138);
  }//anti-B0 decays to omega K0
  if (GenB0Tag::PcheckDecay(genpart, 223, -311)) {
    return -1 * (100000 * m_nPhotos + 1139);
  }//anti-B0 decays to omega anti-K0
  if (GenB0Tag::PcheckDecay(genpart, 333, 111)) {
    return -1 * (100000 * m_nPhotos + 1140);
  }//anti-B0 decays to phi pi0
  if (GenB0Tag::PcheckDecay(genpart, 333, 221)) {
    return -1 * (100000 * m_nPhotos + 1141);
  }//anti-B0 decays to phi eta
  if (GenB0Tag::PcheckDecay(genpart, 333, 331)) {
    return -1 * (100000 * m_nPhotos + 1142);
  }//anti-B0 decays to phi eta'
  if (GenB0Tag::PcheckDecay(genpart, -313, 221)) {
    return -1 * (100000 * m_nPhotos + 1143);
  }//anti-B0 decays to anti-K*0 eta
  if (GenB0Tag::PcheckDecay(genpart, -313, 331)) {
    return -1 * (100000 * m_nPhotos + 1144);
  }//anti-B0 decays to anti-K*0 eta'
  if (GenB0Tag::PcheckDecay(genpart, -313, 223)) {
    return -1 * (100000 * m_nPhotos + 1145);
  }//anti-B0 decays to anti-K*0 omega
  if (GenB0Tag::PcheckDecay(genpart, 333, 223)) {
    return -1 * (100000 * m_nPhotos + 1146);
  }//anti-B0 decays to phi omega
  if (GenB0Tag::PcheckDecay(genpart, 223, 223)) {
    return -1 * (100000 * m_nPhotos + 1147);
  }//anti-B0 decays to omega omega
  if (GenB0Tag::PcheckDecay(genpart, 223, 113)) {
    return -1 * (100000 * m_nPhotos + 1148);
  }//anti-B0 decays to omega rho0
  if (GenB0Tag::PcheckDecay(genpart, -321, 211)) {
    return -1 * (100000 * m_nPhotos + 1149);
  }//anti-B0 decays to K- pi+
  if (GenB0Tag::PcheckDecay(genpart, 321, -321)) {
    return -1 * (100000 * m_nPhotos + 1150);
  }//anti-B0 decays to K+ K-
  if (GenB0Tag::PcheckDecay(genpart, 310, 310)) {
    return -1 * (100000 * m_nPhotos + 1151);
  }//anti-B0 decays to K_S0 K_S0
  if (GenB0Tag::PcheckDecay(genpart, 310, 130)) {
    return -1 * (100000 * m_nPhotos + 1152);
  }//anti-B0 decays to K_S0 K_L0
  if (GenB0Tag::PcheckDecay(genpart, 130, 130)) {
    return -1 * (100000 * m_nPhotos + 1153);
  }//anti-B0 decays to K_L0 K_L0
  if (GenB0Tag::PcheckDecay(genpart, -311, 111)) {
    return -1 * (100000 * m_nPhotos + 1154);
  }//anti-B0 decays to anti-K0 pi0
  if (GenB0Tag::PcheckDecay(genpart, 310, 111)) {
    return -1 * (100000 * m_nPhotos + 1155);
  }//anti-B0 decays to K_S0 pi0
  if (GenB0Tag::PcheckDecay(genpart, 130, 111)) {
    return -1 * (100000 * m_nPhotos + 1156);
  }//anti-B0 decays to K_L0 pi0
  if (GenB0Tag::PcheckDecay(genpart, 311, -311)) {
    return -1 * (100000 * m_nPhotos + 1157);
  }//anti-B0 decays to K0 anti-K0
  if (GenB0Tag::PcheckDecay(genpart, -211, 211, 111)) {
    return -1 * (100000 * m_nPhotos + 1158);
  }//anti-B0 decays to pi- pi+ pi0
  if (GenB0Tag::PcheckDecay(genpart, 111, 113)) {
    return -1 * (100000 * m_nPhotos + 1159);
  }//anti-B0 decays to pi0 rho0
  if (GenB0Tag::PcheckDecay(genpart, -213, 211)) {
    return -1 * (100000 * m_nPhotos + 1160);
  }//anti-B0 decays to rho- pi+
  if (GenB0Tag::PcheckDecay(genpart, 213, -211)) {
    return -1 * (100000 * m_nPhotos + 1161);
  }//anti-B0 decays to rho+ pi-
  if (GenB0Tag::PcheckDecay(genpart, -100213, 211)) {
    return -1 * (100000 * m_nPhotos + 1162);
  }//anti-B0 decays to rho(2S)- pi+
  if (GenB0Tag::PcheckDecay(genpart, 100213, -211)) {
    return -1 * (100000 * m_nPhotos + 1163);
  }//anti-B0 decays to rho(2S)+ pi-
  if (GenB0Tag::PcheckDecay(genpart, -30213, 211)) {
    return -1 * (100000 * m_nPhotos + 1164);
  }//anti-B0 decays to rho(3S)- pi+
  if (GenB0Tag::PcheckDecay(genpart, 30213, -211)) {
    return -1 * (100000 * m_nPhotos + 1165);
  }//anti-B0 decays to rho(3S)+ pi-
  if (GenB0Tag::PcheckDecay(genpart, 113, 310)) {
    return -1 * (100000 * m_nPhotos + 1166);
  }//anti-B0 decays to rho0 K_S0
  if (GenB0Tag::PcheckDecay(genpart, 113, 130)) {
    return -1 * (100000 * m_nPhotos + 1167);
  }//anti-B0 decays to rho0 K_L0
  if (GenB0Tag::PcheckDecay(genpart, 213, -321)) {
    return -1 * (100000 * m_nPhotos + 1168);
  }//anti-B0 decays to rho+ K-
  if (GenB0Tag::PcheckDecay(genpart, 100113, 310)) {
    return -1 * (100000 * m_nPhotos + 1169);
  }//anti-B0 decays to rho(2S)0 K_S0
  if (GenB0Tag::PcheckDecay(genpart, 100113, 130)) {
    return -1 * (100000 * m_nPhotos + 1170);
  }//anti-B0 decays to rho(2S)0 K_L0
  if (GenB0Tag::PcheckDecay(genpart, 10221, -311)) {
    return -1 * (100000 * m_nPhotos + 1171);
  }//anti-B0 decays to f_0 anti-K0
  if (GenB0Tag::PcheckDecay(genpart, 10111, -311)) {
    return -1 * (100000 * m_nPhotos + 1172);
  }//anti-B0 decays to a_00 anti-K0
  if (GenB0Tag::PcheckDecay(genpart, 10211, -321)) {
    return -1 * (100000 * m_nPhotos + 1173);
  }//anti-B0 decays to a_0+ K-
  if (GenB0Tag::PcheckDecay(genpart, 10211, -211)) {
    return -1 * (100000 * m_nPhotos + 1174);
  }//anti-B0 decays to a_0+ pi-
  if (GenB0Tag::PcheckDecay(genpart, -10211, 211)) {
    return -1 * (100000 * m_nPhotos + 1175);
  }//anti-B0 decays to a_0- pi+
  if (GenB0Tag::PcheckDecay(genpart, -313, 111)) {
    return -1 * (100000 * m_nPhotos + 1176);
  }//anti-B0 decays to anti-K*0 pi0
  if (GenB0Tag::PcheckDecay(genpart, -313, 311)) {
    return -1 * (100000 * m_nPhotos + 1177);
  }//anti-B0 decays to anti-K*0 K0
  if (GenB0Tag::PcheckDecay(genpart, -10311, 111)) {
    return -1 * (100000 * m_nPhotos + 1178);
  }//anti-B0 decays to anti-K_0*0 pi0
  if (GenB0Tag::PcheckDecay(genpart, -10311, 311)) {
    return -1 * (100000 * m_nPhotos + 1179);
  }//anti-B0 decays to anti-K_0*0 K0
  if (GenB0Tag::PcheckDecay(genpart, -30313, 111)) {
    return -1 * (100000 * m_nPhotos + 1180);
  }//anti-B0 decays to anti-K''*0 pi0
  if (GenB0Tag::PcheckDecay(genpart, -30313, 311)) {
    return -1 * (100000 * m_nPhotos + 1181);
  }//anti-B0 decays to anti-K''*0 K0
  if (GenB0Tag::PcheckDecay(genpart, -323, 211)) {
    return -1 * (100000 * m_nPhotos + 1182);
  }//anti-B0 decays to K*- pi+
  if (GenB0Tag::PcheckDecay(genpart, -323, 321)) {
    return -1 * (100000 * m_nPhotos + 1183);
  }//anti-B0 decays to K*- K+
  if (GenB0Tag::PcheckDecay(genpart, -10321, 211)) {
    return -1 * (100000 * m_nPhotos + 1184);
  }//anti-B0 decays to K_0*- pi+
  if (GenB0Tag::PcheckDecay(genpart, -10321, 321)) {
    return -1 * (100000 * m_nPhotos + 1185);
  }//anti-B0 decays to K_0*- K+
  if (GenB0Tag::PcheckDecay(genpart, -30323, 211)) {
    return -1 * (100000 * m_nPhotos + 1186);
  }//anti-B0 decays to K''*- pi+
  if (GenB0Tag::PcheckDecay(genpart, -30323, 321)) {
    return -1 * (100000 * m_nPhotos + 1187);
  }//anti-B0 decays to K''*- K+
  if (GenB0Tag::PcheckDecay(genpart, -211, 211, 221)) {
    return -1 * (100000 * m_nPhotos + 1188);
  }//anti-B0 decays to pi- pi+ eta
  if (GenB0Tag::PcheckDecay(genpart, -211, 211, 331)) {
    return -1 * (100000 * m_nPhotos + 1189);
  }//anti-B0 decays to pi- pi+ eta'
  if (GenB0Tag::PcheckDecay(genpart, -211, 211, -311)) {
    return -1 * (100000 * m_nPhotos + 1190);
  }//anti-B0 decays to pi- pi+ anti-K0
  if (GenB0Tag::PcheckDecay(genpart, -321, 211, 111)) {
    return -1 * (100000 * m_nPhotos + 1191);
  }//anti-B0 decays to K- pi+ pi0
  if (GenB0Tag::PcheckDecay(genpart, -321, 211, 221)) {
    return -1 * (100000 * m_nPhotos + 1192);
  }//anti-B0 decays to K- pi+ eta
  if (GenB0Tag::PcheckDecay(genpart, -321, 211, 331)) {
    return -1 * (100000 * m_nPhotos + 1193);
  }//anti-B0 decays to K- pi+ eta'
  if (GenB0Tag::PcheckDecay(genpart, -211, 321, -311)) {
    return -1 * (100000 * m_nPhotos + 1194);
  }//anti-B0 decays to pi- K+ anti-K0
  if (GenB0Tag::PcheckDecay(genpart, 211, -321, 311)) {
    return -1 * (100000 * m_nPhotos + 1195);
  }//anti-B0 decays to pi+ K- K0
  if (GenB0Tag::PcheckDecay(genpart, -321, 321, -311)) {
    return -1 * (100000 * m_nPhotos + 1196);
  }//anti-B0 decays to K- K+ anti-K0
  if (GenB0Tag::PcheckDecay(genpart, -321, 321, 310)) {
    return -1 * (100000 * m_nPhotos + 1197);
  }//anti-B0 decays to K- K+ K_S0
  if (GenB0Tag::PcheckDecay(genpart, -321, 321, 130)) {
    return -1 * (100000 * m_nPhotos + 1198);
  }//anti-B0 decays to K- K+ K_L0
  if (GenB0Tag::PcheckDecay(genpart, -321, 321, 111)) {
    return -1 * (100000 * m_nPhotos + 1199);
  }//anti-B0 decays to K- K+ pi0
  if (GenB0Tag::PcheckDecay(genpart, -321, 321, 221)) {
    return -1 * (100000 * m_nPhotos + 1200);
  }//anti-B0 decays to K- K+ eta
  if (GenB0Tag::PcheckDecay(genpart, -321, 321, 331)) {
    return -1 * (100000 * m_nPhotos + 1201);
  }//anti-B0 decays to K- K+ eta'
  if (GenB0Tag::PcheckDecay(genpart, -311, 311, 111)) {
    return -1 * (100000 * m_nPhotos + 1202);
  }//anti-B0 decays to anti-K0 K0 pi0
  if (GenB0Tag::PcheckDecay(genpart, -311, 311, 221)) {
    return -1 * (100000 * m_nPhotos + 1203);
  }//anti-B0 decays to anti-K0 K0 eta
  if (GenB0Tag::PcheckDecay(genpart, -311, 311, 331)) {
    return -1 * (100000 * m_nPhotos + 1204);
  }//anti-B0 decays to anti-K0 K0 eta'
  if (GenB0Tag::PcheckDecay(genpart, -311, 111, 111)) {
    return -1 * (100000 * m_nPhotos + 1205);
  }//anti-B0 decays to anti-K0 pi0 pi0
  if (GenB0Tag::PcheckDecay(genpart, -311, 111, 221)) {
    return -1 * (100000 * m_nPhotos + 1206);
  }//anti-B0 decays to anti-K0 pi0 eta
  if (GenB0Tag::PcheckDecay(genpart, -311, 111, 331)) {
    return -1 * (100000 * m_nPhotos + 1207);
  }//anti-B0 decays to anti-K0 pi0 eta'
  if (GenB0Tag::PcheckDecay(genpart, -311, 221, 221)) {
    return -1 * (100000 * m_nPhotos + 1208);
  }//anti-B0 decays to anti-K0 eta eta
  if (GenB0Tag::PcheckDecay(genpart, -311, 221, 331)) {
    return -1 * (100000 * m_nPhotos + 1209);
  }//anti-B0 decays to anti-K0 eta eta'
  if (GenB0Tag::PcheckDecay(genpart, -311, 331, 331)) {
    return -1 * (100000 * m_nPhotos + 1210);
  }//anti-B0 decays to anti-K0 eta' eta'
  if (GenB0Tag::PcheckDecay(genpart, -311, 311, -311)) {
    return -1 * (100000 * m_nPhotos + 1211);
  }//anti-B0 decays to anti-K0 K0 anti-K0
  if (GenB0Tag::PcheckDecay(genpart, 111, 111, 111)) {
    return -1 * (100000 * m_nPhotos + 1212);
  }//anti-B0 decays to pi0 pi0 pi0
  if (GenB0Tag::PcheckDecay(genpart, 111, 111, 221)) {
    return -1 * (100000 * m_nPhotos + 1213);
  }//anti-B0 decays to pi0 pi0 eta
  if (GenB0Tag::PcheckDecay(genpart, 111, 111, 331)) {
    return -1 * (100000 * m_nPhotos + 1214);
  }//anti-B0 decays to pi0 pi0 eta'
  if (GenB0Tag::PcheckDecay(genpart, 111, 221, 221)) {
    return -1 * (100000 * m_nPhotos + 1215);
  }//anti-B0 decays to pi0 eta eta
  if (GenB0Tag::PcheckDecay(genpart, 111, 221, 331)) {
    return -1 * (100000 * m_nPhotos + 1216);
  }//anti-B0 decays to pi0 eta eta'
  if (GenB0Tag::PcheckDecay(genpart, -313, 111, 111)) {
    return -1 * (100000 * m_nPhotos + 1217);
  }//anti-B0 decays to anti-K*0 pi0 pi0
  if (GenB0Tag::PcheckDecay(genpart, 113, 113)) {
    return -1 * (100000 * m_nPhotos + 1218);
  }//anti-B0 decays to rho0 rho0
  if (GenB0Tag::PcheckDecay(genpart, 213, -213)) {
    return -1 * (100000 * m_nPhotos + 1219);
  }//anti-B0 decays to rho+ rho-
  if (GenB0Tag::PcheckDecay(genpart, 113, 211, -211)) {
    return -1 * (100000 * m_nPhotos + 1220);
  }//anti-B0 decays to rho0 pi+ pi-
  if (GenB0Tag::PcheckDecay(genpart, 223, 211, -211)) {
    return -1 * (100000 * m_nPhotos + 1221);
  }//anti-B0 decays to omega pi+ pi-
  if (GenB0Tag::PcheckDecay(genpart, 113, 111, 111)) {
    return -1 * (100000 * m_nPhotos + 1222);
  }//anti-B0 decays to rho0 pi0 pi0
  if (GenB0Tag::PcheckDecay(genpart, 223, 111, 111)) {
    return -1 * (100000 * m_nPhotos + 1223);
  }//anti-B0 decays to omega pi0 pi0
  if (GenB0Tag::PcheckDecay(genpart, 113, 111, 221)) {
    return -1 * (100000 * m_nPhotos + 1224);
  }//anti-B0 decays to rho0 pi0 eta
  if (GenB0Tag::PcheckDecay(genpart, 113, 221, 221)) {
    return -1 * (100000 * m_nPhotos + 1225);
  }//anti-B0 decays to rho0 eta eta
  if (GenB0Tag::PcheckDecay(genpart, 113, 111, 331)) {
    return -1 * (100000 * m_nPhotos + 1226);
  }//anti-B0 decays to rho0 pi0 eta'
  if (GenB0Tag::PcheckDecay(genpart, 113, 221, 331)) {
    return -1 * (100000 * m_nPhotos + 1227);
  }//anti-B0 decays to rho0 eta eta'
  if (GenB0Tag::PcheckDecay(genpart, 223, 111, 221)) {
    return -1 * (100000 * m_nPhotos + 1228);
  }//anti-B0 decays to omega pi0 eta
  if (GenB0Tag::PcheckDecay(genpart, 223, 221, 221)) {
    return -1 * (100000 * m_nPhotos + 1229);
  }//anti-B0 decays to omega eta eta
  if (GenB0Tag::PcheckDecay(genpart, 223, 111, 331)) {
    return -1 * (100000 * m_nPhotos + 1230);
  }//anti-B0 decays to omega pi0 eta'
  if (GenB0Tag::PcheckDecay(genpart, 223, 221, 331)) {
    return -1 * (100000 * m_nPhotos + 1231);
  }//anti-B0 decays to omega eta eta'
  if (GenB0Tag::PcheckDecay(genpart, 213, -211, 111)) {
    return -1 * (100000 * m_nPhotos + 1232);
  }//anti-B0 decays to rho+ pi- pi0
  if (GenB0Tag::PcheckDecay(genpart, 213, -211, 221)) {
    return -1 * (100000 * m_nPhotos + 1233);
  }//anti-B0 decays to rho+ pi- eta
  if (GenB0Tag::PcheckDecay(genpart, -213, 211, 111)) {
    return -1 * (100000 * m_nPhotos + 1234);
  }//anti-B0 decays to rho- pi+ pi0
  if (GenB0Tag::PcheckDecay(genpart, -213, 211, 221)) {
    return -1 * (100000 * m_nPhotos + 1235);
  }//anti-B0 decays to rho- pi+ eta
  if (GenB0Tag::PcheckDecay(genpart, 211, -211, 211, -211)) {
    return -1 * (100000 * m_nPhotos + 1236);
  }//anti-B0 decays to pi+ pi- pi+ pi-
  if (GenB0Tag::PcheckDecay(genpart, 211, -211, 111, 111)) {
    return -1 * (100000 * m_nPhotos + 1237);
  }//anti-B0 decays to pi+ pi- pi0 pi0
  if (GenB0Tag::PcheckDecay(genpart, 211, -211, 221, 111)) {
    return -1 * (100000 * m_nPhotos + 1238);
  }//anti-B0 decays to pi+ pi- eta pi0
  if (GenB0Tag::PcheckDecay(genpart, 211, -211, 221, 221)) {
    return -1 * (100000 * m_nPhotos + 1239);
  }//anti-B0 decays to pi+ pi- eta eta
  if (GenB0Tag::PcheckDecay(genpart, 211, -211, 331, 111)) {
    return -1 * (100000 * m_nPhotos + 1240);
  }//anti-B0 decays to pi+ pi- eta' pi0
  if (GenB0Tag::PcheckDecay(genpart, 211, -211, 331, 221)) {
    return -1 * (100000 * m_nPhotos + 1241);
  }//anti-B0 decays to pi+ pi- eta' eta
  if (GenB0Tag::PcheckDecay(genpart, 20213, -211)) {
    return -1 * (100000 * m_nPhotos + 1242);
  }//anti-B0 decays to a_1+ pi-
  if (GenB0Tag::PcheckDecay(genpart, -20213, 211)) {
    return -1 * (100000 * m_nPhotos + 1243);
  }//anti-B0 decays to a_1- pi+
  if (GenB0Tag::PcheckDecay(genpart, 10213, -211)) {
    return -1 * (100000 * m_nPhotos + 1244);
  }//anti-B0 decays to b_1+ pi-
  if (GenB0Tag::PcheckDecay(genpart, -10213, 211)) {
    return -1 * (100000 * m_nPhotos + 1245);
  }//anti-B0 decays to b_1- pi+
  if (GenB0Tag::PcheckDecay(genpart, -213, 10211)) {
    return -1 * (100000 * m_nPhotos + 1246);
  }//anti-B0 decays to rho- a_0+
  if (GenB0Tag::PcheckDecay(genpart, 213, -10211)) {
    return -1 * (100000 * m_nPhotos + 1247);
  }//anti-B0 decays to rho+ a_0-
  if (GenB0Tag::PcheckDecay(genpart, 10221, 211, -211)) {
    return -1 * (100000 * m_nPhotos + 1248);
  }//anti-B0 decays to f_0 pi+ pi-
  if (GenB0Tag::PcheckDecay(genpart, 10221, 111, 111)) {
    return -1 * (100000 * m_nPhotos + 1249);
  }//anti-B0 decays to f_0 pi0 pi0
  if (GenB0Tag::PcheckDecay(genpart, -10211, 211, 111)) {
    return -1 * (100000 * m_nPhotos + 1250);
  }//anti-B0 decays to a_0- pi+ pi0
  if (GenB0Tag::PcheckDecay(genpart, 10111, 111, 111)) {
    return -1 * (100000 * m_nPhotos + 1251);
  }//anti-B0 decays to a_00 pi0 pi0
  if (GenB0Tag::PcheckDecay(genpart, 10211, -211, 111)) {
    return -1 * (100000 * m_nPhotos + 1252);
  }//anti-B0 decays to a_0+ pi- pi0
  if (GenB0Tag::PcheckDecay(genpart, 113, -313)) {
    return -1 * (100000 * m_nPhotos + 1253);
  }//anti-B0 decays to rho0 anti-K*0
  if (GenB0Tag::PcheckDecay(genpart, 213, -323)) {
    return -1 * (100000 * m_nPhotos + 1254);
  }//anti-B0 decays to rho+ K*-
  if (GenB0Tag::PcheckDecay(genpart, -211, 211, -313)) {
    return -1 * (100000 * m_nPhotos + 1255);
  }//anti-B0 decays to pi- pi+ anti-K*0
  if (GenB0Tag::PcheckDecay(genpart, 211, 111, -323)) {
    return -1 * (100000 * m_nPhotos + 1256);
  }//anti-B0 decays to pi+ pi0 K*-
  if (GenB0Tag::PcheckDecay(genpart, 211, 221, -323)) {
    return -1 * (100000 * m_nPhotos + 1257);
  }//anti-B0 decays to pi+ eta K*-
  if (GenB0Tag::PcheckDecay(genpart, 211, 331, -323)) {
    return -1 * (100000 * m_nPhotos + 1258);
  }//anti-B0 decays to pi+ eta' K*-
  if (GenB0Tag::PcheckDecay(genpart, 213, -321, 111)) {
    return -1 * (100000 * m_nPhotos + 1259);
  }//anti-B0 decays to rho+ K- pi0
  if (GenB0Tag::PcheckDecay(genpart, 213, -321, 221)) {
    return -1 * (100000 * m_nPhotos + 1260);
  }//anti-B0 decays to rho+ K- eta
  if (GenB0Tag::PcheckDecay(genpart, 213, -321, 331)) {
    return -1 * (100000 * m_nPhotos + 1261);
  }//anti-B0 decays to rho+ K- eta'
  if (GenB0Tag::PcheckDecay(genpart, 213, -311, -211)) {
    return -1 * (100000 * m_nPhotos + 1262);
  }//anti-B0 decays to rho+ anti-K0 pi-
  if (GenB0Tag::PcheckDecay(genpart, -213, -311, 211)) {
    return -1 * (100000 * m_nPhotos + 1263);
  }//anti-B0 decays to rho- anti-K0 pi+
  if (GenB0Tag::PcheckDecay(genpart, 113, -311, 111)) {
    return -1 * (100000 * m_nPhotos + 1264);
  }//anti-B0 decays to rho0 anti-K0 pi0
  if (GenB0Tag::PcheckDecay(genpart, 113, -311, 221)) {
    return -1 * (100000 * m_nPhotos + 1265);
  }//anti-B0 decays to rho0 anti-K0 eta
  if (GenB0Tag::PcheckDecay(genpart, 113, -311, 331)) {
    return -1 * (100000 * m_nPhotos + 1266);
  }//anti-B0 decays to rho0 anti-K0 eta'
  if (GenB0Tag::PcheckDecay(genpart, 113, -321, 211)) {
    return -1 * (100000 * m_nPhotos + 1267);
  }//anti-B0 decays to rho0 K- pi+
  if (GenB0Tag::PcheckDecay(genpart, 223, -311, 111)) {
    return -1 * (100000 * m_nPhotos + 1268);
  }//anti-B0 decays to omega anti-K0 pi0
  if (GenB0Tag::PcheckDecay(genpart, 223, -311, 221)) {
    return -1 * (100000 * m_nPhotos + 1269);
  }//anti-B0 decays to omega anti-K0 eta
  if (GenB0Tag::PcheckDecay(genpart, 223, -311, 331)) {
    return -1 * (100000 * m_nPhotos + 1270);
  }//anti-B0 decays to omega anti-K0 eta'
  if (GenB0Tag::PcheckDecay(genpart, 223, -321, 211)) {
    return -1 * (100000 * m_nPhotos + 1271);
  }//anti-B0 decays to omega K- pi+
  if (GenB0Tag::PcheckDecay(genpart, 211, 111, -211, -311)) {
    return -1 * (100000 * m_nPhotos + 1272);
  }//anti-B0 decays to pi+ pi0 pi- anti-K0
  if (GenB0Tag::PcheckDecay(genpart, 211, 221, -211, -311)) {
    return -1 * (100000 * m_nPhotos + 1273);
  }//anti-B0 decays to pi+ eta pi- anti-K0
  if (GenB0Tag::PcheckDecay(genpart, 111, 111, 111, -311)) {
    return -1 * (100000 * m_nPhotos + 1274);
  }//anti-B0 decays to pi0 pi0 pi0 anti-K0
  if (GenB0Tag::PcheckDecay(genpart, 111, 111, 221, -311)) {
    return -1 * (100000 * m_nPhotos + 1275);
  }//anti-B0 decays to pi0 pi0 eta anti-K0
  if (GenB0Tag::PcheckDecay(genpart, 111, 221, 221, -311)) {
    return -1 * (100000 * m_nPhotos + 1276);
  }//anti-B0 decays to pi0 eta eta anti-K0
  if (GenB0Tag::PcheckDecay(genpart, 111, 111, 331, -311)) {
    return -1 * (100000 * m_nPhotos + 1277);
  }//anti-B0 decays to pi0 pi0 eta' anti-K0
  if (GenB0Tag::PcheckDecay(genpart, 111, 221, 331, -311)) {
    return -1 * (100000 * m_nPhotos + 1278);
  }//anti-B0 decays to pi0 eta eta' anti-K0
  if (GenB0Tag::PcheckDecay(genpart, 211, -211, 211, -321)) {
    return -1 * (100000 * m_nPhotos + 1279);
  }//anti-B0 decays to pi+ pi- pi+ K-
  if (GenB0Tag::PcheckDecay(genpart, 111, 111, 211, -321)) {
    return -1 * (100000 * m_nPhotos + 1280);
  }//anti-B0 decays to pi0 pi0 pi+ K-
  if (GenB0Tag::PcheckDecay(genpart, 111, 221, 211, -321)) {
    return -1 * (100000 * m_nPhotos + 1281);
  }//anti-B0 decays to pi0 eta pi+ K-
  if (GenB0Tag::PcheckDecay(genpart, 221, 221, 211, -321)) {
    return -1 * (100000 * m_nPhotos + 1282);
  }//anti-B0 decays to eta eta pi+ K-
  if (GenB0Tag::PcheckDecay(genpart, 111, 331, 211, -321)) {
    return -1 * (100000 * m_nPhotos + 1283);
  }//anti-B0 decays to pi0 eta' pi+ K-
  if (GenB0Tag::PcheckDecay(genpart, 221, 331, 211, -321)) {
    return -1 * (100000 * m_nPhotos + 1284);
  }//anti-B0 decays to eta eta' pi+ K-
  if (GenB0Tag::PcheckDecay(genpart, 113, -10311)) {
    return -1 * (100000 * m_nPhotos + 1285);
  }//anti-B0 decays to rho0 anti-K_0*0
  if (GenB0Tag::PcheckDecay(genpart, 213, -10321)) {
    return -1 * (100000 * m_nPhotos + 1286);
  }//anti-B0 decays to rho+ K_0*-
  if (GenB0Tag::PcheckDecay(genpart, -211, 211, -10311)) {
    return -1 * (100000 * m_nPhotos + 1287);
  }//anti-B0 decays to pi- pi+ anti-K_0*0
  if (GenB0Tag::PcheckDecay(genpart, 111, 111, -10311)) {
    return -1 * (100000 * m_nPhotos + 1288);
  }//anti-B0 decays to pi0 pi0 anti-K_0*0
  if (GenB0Tag::PcheckDecay(genpart, 211, 111, -10321)) {
    return -1 * (100000 * m_nPhotos + 1289);
  }//anti-B0 decays to pi+ pi0 K_0*-
  if (GenB0Tag::PcheckDecay(genpart, -313, 10221)) {
    return -1 * (100000 * m_nPhotos + 1290);
  }//anti-B0 decays to anti-K*0 f_0
  if (GenB0Tag::PcheckDecay(genpart, 20113, -311)) {
    return -1 * (100000 * m_nPhotos + 1291);
  }//anti-B0 decays to a_10 anti-K0
  if (GenB0Tag::PcheckDecay(genpart, 20213, -321)) {
    return -1 * (100000 * m_nPhotos + 1292);
  }//anti-B0 decays to a_1+ K-
  if (GenB0Tag::PcheckDecay(genpart, 10113, -311)) {
    return -1 * (100000 * m_nPhotos + 1293);
  }//anti-B0 decays to b_10 anti-K0
  if (GenB0Tag::PcheckDecay(genpart, 10213, -321)) {
    return -1 * (100000 * m_nPhotos + 1294);
  }//anti-B0 decays to b_1+ K-
  if (GenB0Tag::PcheckDecay(genpart, -313, 10111)) {
    return -1 * (100000 * m_nPhotos + 1295);
  }//anti-B0 decays to anti-K*0 a_00
  if (GenB0Tag::PcheckDecay(genpart, -323, 10211)) {
    return -1 * (100000 * m_nPhotos + 1296);
  }//anti-B0 decays to K*- a_0+
  if (GenB0Tag::PcheckDecay(genpart, 313, -313)) {
    return -1 * (100000 * m_nPhotos + 1297);
  }//anti-B0 decays to K*0 anti-K*0
  if (GenB0Tag::PcheckDecay(genpart, 323, -323)) {
    return -1 * (100000 * m_nPhotos + 1298);
  }//anti-B0 decays to K*+ K*-
  if (GenB0Tag::PcheckDecay(genpart, 333, 113)) {
    return -1 * (100000 * m_nPhotos + 1299);
  }//anti-B0 decays to phi rho0
  if (GenB0Tag::PcheckDecay(genpart, 333, -211, 211)) {
    return -1 * (100000 * m_nPhotos + 1300);
  }//anti-B0 decays to phi pi- pi+
  if (GenB0Tag::PcheckDecay(genpart, 321, -321, 113)) {
    return -1 * (100000 * m_nPhotos + 1301);
  }//anti-B0 decays to K+ K- rho0
  if (GenB0Tag::PcheckDecay(genpart, 311, -311, 113)) {
    return -1 * (100000 * m_nPhotos + 1302);
  }//anti-B0 decays to K0 anti-K0 rho0
  if (GenB0Tag::PcheckDecay(genpart, 321, -321, 223)) {
    return -1 * (100000 * m_nPhotos + 1303);
  }//anti-B0 decays to K+ K- omega
  if (GenB0Tag::PcheckDecay(genpart, 311, -311, 223)) {
    return -1 * (100000 * m_nPhotos + 1304);
  }//anti-B0 decays to K0 anti-K0 omega
  if (GenB0Tag::PcheckDecay(genpart, 321, -311, -213)) {
    return -1 * (100000 * m_nPhotos + 1305);
  }//anti-B0 decays to K+ anti-K0 rho-
  if (GenB0Tag::PcheckDecay(genpart, -321, 311, 213)) {
    return -1 * (100000 * m_nPhotos + 1306);
  }//anti-B0 decays to K- K0 rho+
  if (GenB0Tag::PcheckDecay(genpart, 321, -321, -211, 211)) {
    return -1 * (100000 * m_nPhotos + 1307);
  }//anti-B0 decays to K+ K- pi- pi+
  if (GenB0Tag::PcheckDecay(genpart, 321, -321, 111, 111)) {
    return -1 * (100000 * m_nPhotos + 1308);
  }//anti-B0 decays to K+ K- pi0 pi0
  if (GenB0Tag::PcheckDecay(genpart, 321, -321, 111, 221)) {
    return -1 * (100000 * m_nPhotos + 1309);
  }//anti-B0 decays to K+ K- pi0 eta
  if (GenB0Tag::PcheckDecay(genpart, 321, -321, 221, 221)) {
    return -1 * (100000 * m_nPhotos + 1310);
  }//anti-B0 decays to K+ K- eta eta
  if (GenB0Tag::PcheckDecay(genpart, 321, -321, 111, 331)) {
    return -1 * (100000 * m_nPhotos + 1311);
  }//anti-B0 decays to K+ K- pi0 eta'
  if (GenB0Tag::PcheckDecay(genpart, 321, -321, 221, 331)) {
    return -1 * (100000 * m_nPhotos + 1312);
  }//anti-B0 decays to K+ K- eta eta'
  if (GenB0Tag::PcheckDecay(genpart, 311, -311, -211, 211)) {
    return -1 * (100000 * m_nPhotos + 1313);
  }//anti-B0 decays to K0 anti-K0 pi- pi+
  if (GenB0Tag::PcheckDecay(genpart, 311, -311, 111, 111)) {
    return -1 * (100000 * m_nPhotos + 1314);
  }//anti-B0 decays to K0 anti-K0 pi0 pi0
  if (GenB0Tag::PcheckDecay(genpart, 311, -311, 111, 221)) {
    return -1 * (100000 * m_nPhotos + 1315);
  }//anti-B0 decays to K0 anti-K0 pi0 eta
  if (GenB0Tag::PcheckDecay(genpart, 311, -311, 221, 221)) {
    return -1 * (100000 * m_nPhotos + 1316);
  }//anti-B0 decays to K0 anti-K0 eta eta
  if (GenB0Tag::PcheckDecay(genpart, 311, -311, 111, 331)) {
    return -1 * (100000 * m_nPhotos + 1317);
  }//anti-B0 decays to K0 anti-K0 pi0 eta'
  if (GenB0Tag::PcheckDecay(genpart, 311, -311, 221, 331)) {
    return -1 * (100000 * m_nPhotos + 1318);
  }//anti-B0 decays to K0 anti-K0 eta eta'
  if (GenB0Tag::PcheckDecay(genpart, 323, -321, 111)) {
    return -1 * (100000 * m_nPhotos + 1319);
  }//anti-B0 decays to K*+ K- pi0
  if (GenB0Tag::PcheckDecay(genpart, -323, 321, 111)) {
    return -1 * (100000 * m_nPhotos + 1320);
  }//anti-B0 decays to K*- K+ pi0
  if (GenB0Tag::PcheckDecay(genpart, 313, -311, 111)) {
    return -1 * (100000 * m_nPhotos + 1321);
  }//anti-B0 decays to K*0 anti-K0 pi0
  if (GenB0Tag::PcheckDecay(genpart, 311, -313, 111)) {
    return -1 * (100000 * m_nPhotos + 1322);
  }//anti-B0 decays to K0 anti-K*0 pi0
  if (GenB0Tag::PcheckDecay(genpart, 321, -313, -211)) {
    return -1 * (100000 * m_nPhotos + 1323);
  }//anti-B0 decays to K+ anti-K*0 pi-
  if (GenB0Tag::PcheckDecay(genpart, 323, -311, -211)) {
    return -1 * (100000 * m_nPhotos + 1324);
  }//anti-B0 decays to K*+ anti-K0 pi-
  if (GenB0Tag::PcheckDecay(genpart, -321, 313, 211)) {
    return -1 * (100000 * m_nPhotos + 1325);
  }//anti-B0 decays to K- K*0 pi+
  if (GenB0Tag::PcheckDecay(genpart, -323, 311, 211)) {
    return -1 * (100000 * m_nPhotos + 1326);
  }//anti-B0 decays to K*- K0 pi+
  if (GenB0Tag::PcheckDecay(genpart, 333, -313)) {
    return -1 * (100000 * m_nPhotos + 1327);
  }//anti-B0 decays to phi anti-K*0
  if (GenB0Tag::PcheckDecay(genpart, 333, -321, 211)) {
    return -1 * (100000 * m_nPhotos + 1328);
  }//anti-B0 decays to phi K- pi+
  if (GenB0Tag::PcheckDecay(genpart, 333, -311, 111)) {
    return -1 * (100000 * m_nPhotos + 1329);
  }//anti-B0 decays to phi anti-K0 pi0
  if (GenB0Tag::PcheckDecay(genpart, 333, -311, 221)) {
    return -1 * (100000 * m_nPhotos + 1330);
  }//anti-B0 decays to phi anti-K0 eta
  if (GenB0Tag::PcheckDecay(genpart, -321, 321, -313)) {
    return -1 * (100000 * m_nPhotos + 1331);
  }//anti-B0 decays to K- K+ anti-K*0
  if (GenB0Tag::PcheckDecay(genpart, -321, 323, -311)) {
    return -1 * (100000 * m_nPhotos + 1332);
  }//anti-B0 decays to K- K*+ anti-K0
  if (GenB0Tag::PcheckDecay(genpart, -323, 321, -311)) {
    return -1 * (100000 * m_nPhotos + 1333);
  }//anti-B0 decays to K*- K+ anti-K0
  if (GenB0Tag::PcheckDecay(genpart, 321, -321, -321, 211)) {
    return -1 * (100000 * m_nPhotos + 1334);
  }//anti-B0 decays to K+ K- K- pi+
  if (GenB0Tag::PcheckDecay(genpart, 321, -321, -311, 111)) {
    return -1 * (100000 * m_nPhotos + 1335);
  }//anti-B0 decays to K+ K- anti-K0 pi0
  if (GenB0Tag::PcheckDecay(genpart, 321, -321, -311, 221)) {
    return -1 * (100000 * m_nPhotos + 1336);
  }//anti-B0 decays to K+ K- anti-K0 eta
  if (GenB0Tag::PcheckDecay(genpart, 311, -311, -311, 111)) {
    return -1 * (100000 * m_nPhotos + 1337);
  }//anti-B0 decays to K0 anti-K0 anti-K0 pi0
  if (GenB0Tag::PcheckDecay(genpart, 311, -311, -311, 221)) {
    return -1 * (100000 * m_nPhotos + 1338);
  }//anti-B0 decays to K0 anti-K0 anti-K0 eta
  if (GenB0Tag::PcheckDecay(genpart, 333, -10311)) {
    return -1 * (100000 * m_nPhotos + 1339);
  }//anti-B0 decays to phi anti-K_0*0
  if (GenB0Tag::PcheckDecay(genpart, -321, 321, -10311)) {
    return -1 * (100000 * m_nPhotos + 1340);
  }//anti-B0 decays to K- K+ anti-K_0*0
  if (GenB0Tag::PcheckDecay(genpart, -321, 10321, -311)) {
    return -1 * (100000 * m_nPhotos + 1341);
  }//anti-B0 decays to K- K_0*+ anti-K0
  if (GenB0Tag::PcheckDecay(genpart, -10321, 321, -311)) {
    return -1 * (100000 * m_nPhotos + 1342);
  }//anti-B0 decays to K_0*- K+ anti-K0
  if (GenB0Tag::PcheckDecay(genpart, 333, -20313)) {
    return -1 * (100000 * m_nPhotos + 1343);
  }//anti-B0 decays to phi anti-K'_10
  if (GenB0Tag::PcheckDecay(genpart, -321, 321, -20313)) {
    return -1 * (100000 * m_nPhotos + 1344);
  }//anti-B0 decays to K- K+ anti-K'_10
  if (GenB0Tag::PcheckDecay(genpart, 333, 333)) {
    return -1 * (100000 * m_nPhotos + 1345);
  }//anti-B0 decays to phi phi
  if (GenB0Tag::PcheckDecay(genpart, 333, 321, -321)) {
    return -1 * (100000 * m_nPhotos + 1346);
  }//anti-B0 decays to phi K+ K-
  if (GenB0Tag::PcheckDecay(genpart, 333, 311, -311)) {
    return -1 * (100000 * m_nPhotos + 1347);
  }//anti-B0 decays to phi K0 anti-K0
  if (GenB0Tag::PcheckDecay(genpart, 321, -321, 321, -321)) {
    return -1 * (100000 * m_nPhotos + 1348);
  }//anti-B0 decays to K+ K- K+ K-
  if (GenB0Tag::PcheckDecay(genpart, 311, -311, 321, -321)) {
    return -1 * (100000 * m_nPhotos + 1349);
  }//anti-B0 decays to K0 anti-K0 K+ K-
  if (GenB0Tag::PcheckDecay(genpart, 311, -311, 311, -311)) {
    return -1 * (100000 * m_nPhotos + 1350);
  }//anti-B0 decays to K0 anti-K0 K0 anti-K0
  if (GenB0Tag::PcheckDecay(genpart, 113, 113, 111)) {
    return -1 * (100000 * m_nPhotos + 1351);
  }//anti-B0 decays to rho0 rho0 pi0
  if (GenB0Tag::PcheckDecay(genpart, 113, 113, 221)) {
    return -1 * (100000 * m_nPhotos + 1352);
  }//anti-B0 decays to rho0 rho0 eta
  if (GenB0Tag::PcheckDecay(genpart, 113, 113, 331)) {
    return -1 * (100000 * m_nPhotos + 1353);
  }//anti-B0 decays to rho0 rho0 eta'
  if (GenB0Tag::PcheckDecay(genpart, 223, 113, 111)) {
    return -1 * (100000 * m_nPhotos + 1354);
  }//anti-B0 decays to omega rho0 pi0
  if (GenB0Tag::PcheckDecay(genpart, 223, 113, 221)) {
    return -1 * (100000 * m_nPhotos + 1355);
  }//anti-B0 decays to omega rho0 eta
  if (GenB0Tag::PcheckDecay(genpart, 223, 113, 331)) {
    return -1 * (100000 * m_nPhotos + 1356);
  }//anti-B0 decays to omega rho0 eta'
  if (GenB0Tag::PcheckDecay(genpart, 213, -213, 111)) {
    return -1 * (100000 * m_nPhotos + 1357);
  }//anti-B0 decays to rho+ rho- pi0
  if (GenB0Tag::PcheckDecay(genpart, 213, -213, 221)) {
    return -1 * (100000 * m_nPhotos + 1358);
  }//anti-B0 decays to rho+ rho- eta
  if (GenB0Tag::PcheckDecay(genpart, 213, -213, 331)) {
    return -1 * (100000 * m_nPhotos + 1359);
  }//anti-B0 decays to rho+ rho- eta'
  if (GenB0Tag::PcheckDecay(genpart, 113, 213, -211)) {
    return -1 * (100000 * m_nPhotos + 1360);
  }//anti-B0 decays to rho0 rho+ pi-
  if (GenB0Tag::PcheckDecay(genpart, 223, 213, -211)) {
    return -1 * (100000 * m_nPhotos + 1361);
  }//anti-B0 decays to omega rho+ pi-
  if (GenB0Tag::PcheckDecay(genpart, 113, 211, -211, 111)) {
    return -1 * (100000 * m_nPhotos + 1362);
  }//anti-B0 decays to rho0 pi+ pi- pi0
  if (GenB0Tag::PcheckDecay(genpart, 113, 211, -211, 221)) {
    return -1 * (100000 * m_nPhotos + 1363);
  }//anti-B0 decays to rho0 pi+ pi- eta
  if (GenB0Tag::PcheckDecay(genpart, 113, 211, -211, 331)) {
    return -1 * (100000 * m_nPhotos + 1364);
  }//anti-B0 decays to rho0 pi+ pi- eta'
  if (GenB0Tag::PcheckDecay(genpart, 113, 111, 111, 111)) {
    return -1 * (100000 * m_nPhotos + 1365);
  }//anti-B0 decays to rho0 pi0 pi0 pi0
  if (GenB0Tag::PcheckDecay(genpart, 113, 111, 221, 111)) {
    return -1 * (100000 * m_nPhotos + 1366);
  }//anti-B0 decays to rho0 pi0 eta pi0
  if (GenB0Tag::PcheckDecay(genpart, 113, 111, 331, 111)) {
    return -1 * (100000 * m_nPhotos + 1367);
  }//anti-B0 decays to rho0 pi0 eta' pi0
  if (GenB0Tag::PcheckDecay(genpart, 223, 211, -211, 111)) {
    return -1 * (100000 * m_nPhotos + 1368);
  }//anti-B0 decays to omega pi+ pi- pi0
  if (GenB0Tag::PcheckDecay(genpart, 223, 211, -211, 221)) {
    return -1 * (100000 * m_nPhotos + 1369);
  }//anti-B0 decays to omega pi+ pi- eta
  if (GenB0Tag::PcheckDecay(genpart, 223, 211, -211, 331)) {
    return -1 * (100000 * m_nPhotos + 1370);
  }//anti-B0 decays to omega pi+ pi- eta'
  if (GenB0Tag::PcheckDecay(genpart, 223, 111, 111, 111)) {
    return -1 * (100000 * m_nPhotos + 1371);
  }//anti-B0 decays to omega pi0 pi0 pi0
  if (GenB0Tag::PcheckDecay(genpart, 223, 111, 221, 111)) {
    return -1 * (100000 * m_nPhotos + 1372);
  }//anti-B0 decays to omega pi0 eta pi0
  if (GenB0Tag::PcheckDecay(genpart, 223, 111, 331, 111)) {
    return -1 * (100000 * m_nPhotos + 1373);
  }//anti-B0 decays to omega pi0 eta' pi0
  if (GenB0Tag::PcheckDecay(genpart, 213, -211, 211, -211)) {
    return -1 * (100000 * m_nPhotos + 1374);
  }//anti-B0 decays to rho+ pi- pi+ pi-
  if (GenB0Tag::PcheckDecay(genpart, 213, -211, 111, 111)) {
    return -1 * (100000 * m_nPhotos + 1375);
  }//anti-B0 decays to rho+ pi- pi0 pi0
  if (GenB0Tag::PcheckDecay(genpart, 213, -211, 111, 221)) {
    return -1 * (100000 * m_nPhotos + 1376);
  }//anti-B0 decays to rho+ pi- pi0 eta
  if (GenB0Tag::PcheckDecay(genpart, 213, -211, 111, 331)) {
    return -1 * (100000 * m_nPhotos + 1377);
  }//anti-B0 decays to rho+ pi- pi0 eta'
  if (GenB0Tag::PcheckDecay(genpart, -213, 211, 211, -211)) {
    return -1 * (100000 * m_nPhotos + 1378);
  }//anti-B0 decays to rho- pi+ pi+ pi-
  if (GenB0Tag::PcheckDecay(genpart, -213, 211, 111, 111)) {
    return -1 * (100000 * m_nPhotos + 1379);
  }//anti-B0 decays to rho- pi+ pi0 pi0
  if (GenB0Tag::PcheckDecay(genpart, -213, 211, 111, 221)) {
    return -1 * (100000 * m_nPhotos + 1380);
  }//anti-B0 decays to rho- pi+ pi0 eta
  if (GenB0Tag::PcheckDecay(genpart, -213, 211, 111, 331)) {
    return -1 * (100000 * m_nPhotos + 1381);
  }//anti-B0 decays to rho- pi+ pi0 eta'
  if (GenB0Tag::PcheckDecay(genpart, 211, -211, 211, -211, 111)) {
    return -1 * (100000 * m_nPhotos + 1382);
  }//anti-B0 decays to pi+ pi- pi+ pi- pi0
  if (GenB0Tag::PcheckDecay(genpart, 211, -211, 211, -211, 221)) {
    return -1 * (100000 * m_nPhotos + 1383);
  }//anti-B0 decays to pi+ pi- pi+ pi- eta
  if (GenB0Tag::PcheckDecay(genpart, 211, -211, 211, -211, 331)) {
    return -1 * (100000 * m_nPhotos + 1384);
  }//anti-B0 decays to pi+ pi- pi+ pi- eta'
  if (GenB0Tag::PcheckDecay(genpart, 211, -211, 111, 111, 111)) {
    return -1 * (100000 * m_nPhotos + 1385);
  }//anti-B0 decays to pi+ pi- pi0 pi0 pi0
  if (GenB0Tag::PcheckDecay(genpart, 211, -211, 111, 111, 221)) {
    return -1 * (100000 * m_nPhotos + 1386);
  }//anti-B0 decays to pi+ pi- pi0 pi0 eta
  if (GenB0Tag::PcheckDecay(genpart, 211, -211, 111, 111, 331)) {
    return -1 * (100000 * m_nPhotos + 1387);
  }//anti-B0 decays to pi+ pi- pi0 pi0 eta'
  if (GenB0Tag::PcheckDecay(genpart, 111, 111, 111, 111, 111)) {
    return -1 * (100000 * m_nPhotos + 1388);
  }//anti-B0 decays to pi0 pi0 pi0 pi0 pi0
  if (GenB0Tag::PcheckDecay(genpart, 111, 111, 111, 111, 221)) {
    return -1 * (100000 * m_nPhotos + 1389);
  }//anti-B0 decays to pi0 pi0 pi0 pi0 eta
  if (GenB0Tag::PcheckDecay(genpart, 111, 111, 111, 111, 331)) {
    return -1 * (100000 * m_nPhotos + 1390);
  }//anti-B0 decays to pi0 pi0 pi0 pi0 eta'
  if (GenB0Tag::PcheckDecay(genpart, 20213, -213)) {
    return -1 * (100000 * m_nPhotos + 1391);
  }//anti-B0 decays to a_1+ rho-
  if (GenB0Tag::PcheckDecay(genpart, -20213, 213)) {
    return -1 * (100000 * m_nPhotos + 1392);
  }//anti-B0 decays to a_1- rho+
  if (GenB0Tag::PcheckDecay(genpart, 20113, 113)) {
    return -1 * (100000 * m_nPhotos + 1393);
  }//anti-B0 decays to a_10 rho0
  if (GenB0Tag::PcheckDecay(genpart, 20113, 223)) {
    return -1 * (100000 * m_nPhotos + 1394);
  }//anti-B0 decays to a_10 omega
  if (GenB0Tag::PcheckDecay(genpart, 20213, -211, 111)) {
    return -1 * (100000 * m_nPhotos + 1395);
  }//anti-B0 decays to a_1+ pi- pi0
  if (GenB0Tag::PcheckDecay(genpart, 20213, -211, 221)) {
    return -1 * (100000 * m_nPhotos + 1396);
  }//anti-B0 decays to a_1+ pi- eta
  if (GenB0Tag::PcheckDecay(genpart, 20213, -211, 331)) {
    return -1 * (100000 * m_nPhotos + 1397);
  }//anti-B0 decays to a_1+ pi- eta'
  if (GenB0Tag::PcheckDecay(genpart, 20113, 211, -211)) {
    return -1 * (100000 * m_nPhotos + 1398);
  }//anti-B0 decays to a_10 pi+ pi-
  if (GenB0Tag::PcheckDecay(genpart, 20113, 111, 111)) {
    return -1 * (100000 * m_nPhotos + 1399);
  }//anti-B0 decays to a_10 pi0 pi0
  if (GenB0Tag::PcheckDecay(genpart, 20113, 111, 221)) {
    return -1 * (100000 * m_nPhotos + 1400);
  }//anti-B0 decays to a_10 pi0 eta
  if (GenB0Tag::PcheckDecay(genpart, 20113, 111, 331)) {
    return -1 * (100000 * m_nPhotos + 1401);
  }//anti-B0 decays to a_10 pi0 eta'
  if (GenB0Tag::PcheckDecay(genpart, -20213, 211, 111)) {
    return -1 * (100000 * m_nPhotos + 1402);
  }//anti-B0 decays to a_1- pi+ pi0
  if (GenB0Tag::PcheckDecay(genpart, -20213, 211, 221)) {
    return -1 * (100000 * m_nPhotos + 1403);
  }//anti-B0 decays to a_1- pi+ eta
  if (GenB0Tag::PcheckDecay(genpart, -20213, 211, 331)) {
    return -1 * (100000 * m_nPhotos + 1404);
  }//anti-B0 decays to a_1- pi+ eta'
  if (GenB0Tag::PcheckDecay(genpart, 213, 10221, -211)) {
    return -1 * (100000 * m_nPhotos + 1405);
  }//anti-B0 decays to rho+ f_0 pi-
  if (GenB0Tag::PcheckDecay(genpart, -213, 10221, 211)) {
    return -1 * (100000 * m_nPhotos + 1406);
  }//anti-B0 decays to rho- f_0 pi+
  if (GenB0Tag::PcheckDecay(genpart, 113, 10221, 111)) {
    return -1 * (100000 * m_nPhotos + 1407);
  }//anti-B0 decays to rho0 f_0 pi0
  if (GenB0Tag::PcheckDecay(genpart, 113, 10221, 221)) {
    return -1 * (100000 * m_nPhotos + 1408);
  }//anti-B0 decays to rho0 f_0 eta
  if (GenB0Tag::PcheckDecay(genpart, 113, 10221, 331)) {
    return -1 * (100000 * m_nPhotos + 1409);
  }//anti-B0 decays to rho0 f_0 eta'
  if (GenB0Tag::PcheckDecay(genpart, 223, 10221, 111)) {
    return -1 * (100000 * m_nPhotos + 1410);
  }//anti-B0 decays to omega f_0 pi0
  if (GenB0Tag::PcheckDecay(genpart, 213, 10111, -211)) {
    return -1 * (100000 * m_nPhotos + 1411);
  }//anti-B0 decays to rho+ a_00 pi-
  if (GenB0Tag::PcheckDecay(genpart, 213, -10211, 111)) {
    return -1 * (100000 * m_nPhotos + 1412);
  }//anti-B0 decays to rho+ a_0- pi0
  if (GenB0Tag::PcheckDecay(genpart, 213, -10211, 221)) {
    return -1 * (100000 * m_nPhotos + 1413);
  }//anti-B0 decays to rho+ a_0- eta
  if (GenB0Tag::PcheckDecay(genpart, 213, -10211, 331)) {
    return -1 * (100000 * m_nPhotos + 1414);
  }//anti-B0 decays to rho+ a_0- eta'
  if (GenB0Tag::PcheckDecay(genpart, 113, 10211, -211)) {
    return -1 * (100000 * m_nPhotos + 1415);
  }//anti-B0 decays to rho0 a_0+ pi-
  if (GenB0Tag::PcheckDecay(genpart, 113, -10211, 211)) {
    return -1 * (100000 * m_nPhotos + 1416);
  }//anti-B0 decays to rho0 a_0- pi+
  if (GenB0Tag::PcheckDecay(genpart, 113, 10111, 111)) {
    return -1 * (100000 * m_nPhotos + 1417);
  }//anti-B0 decays to rho0 a_00 pi0
  if (GenB0Tag::PcheckDecay(genpart, 113, 10111, 221)) {
    return -1 * (100000 * m_nPhotos + 1418);
  }//anti-B0 decays to rho0 a_00 eta
  if (GenB0Tag::PcheckDecay(genpart, 113, 10111, 331)) {
    return -1 * (100000 * m_nPhotos + 1419);
  }//anti-B0 decays to rho0 a_00 eta'
  if (GenB0Tag::PcheckDecay(genpart, -213, 10111, 211)) {
    return -1 * (100000 * m_nPhotos + 1420);
  }//anti-B0 decays to rho- a_00 pi+
  if (GenB0Tag::PcheckDecay(genpart, -213, 10211, 111)) {
    return -1 * (100000 * m_nPhotos + 1421);
  }//anti-B0 decays to rho- a_0+ pi0
  if (GenB0Tag::PcheckDecay(genpart, -213, 10211, 221)) {
    return -1 * (100000 * m_nPhotos + 1422);
  }//anti-B0 decays to rho- a_0+ eta
  if (GenB0Tag::PcheckDecay(genpart, -213, 10211, 331)) {
    return -1 * (100000 * m_nPhotos + 1423);
  }//anti-B0 decays to rho- a_0+ eta'
  if (GenB0Tag::PcheckDecay(genpart, 10221, 211, -211, 111)) {
    return -1 * (100000 * m_nPhotos + 1424);
  }//anti-B0 decays to f_0 pi+ pi- pi0
  if (GenB0Tag::PcheckDecay(genpart, 10221, 211, -211, 221)) {
    return -1 * (100000 * m_nPhotos + 1425);
  }//anti-B0 decays to f_0 pi+ pi- eta
  if (GenB0Tag::PcheckDecay(genpart, 10221, 211, -211, 331)) {
    return -1 * (100000 * m_nPhotos + 1426);
  }//anti-B0 decays to f_0 pi+ pi- eta'
  if (GenB0Tag::PcheckDecay(genpart, 10221, 111, 111, 111)) {
    return -1 * (100000 * m_nPhotos + 1427);
  }//anti-B0 decays to f_0 pi0 pi0 pi0
  if (GenB0Tag::PcheckDecay(genpart, 10221, 111, 111, 221)) {
    return -1 * (100000 * m_nPhotos + 1428);
  }//anti-B0 decays to f_0 pi0 pi0 eta
  if (GenB0Tag::PcheckDecay(genpart, 10221, 111, 111, 331)) {
    return -1 * (100000 * m_nPhotos + 1429);
  }//anti-B0 decays to f_0 pi0 pi0 eta'
  if (GenB0Tag::PcheckDecay(genpart, 10111, 211, -211, 111)) {
    return -1 * (100000 * m_nPhotos + 1430);
  }//anti-B0 decays to a_00 pi+ pi- pi0
  if (GenB0Tag::PcheckDecay(genpart, 10111, 111, 111, 111)) {
    return -1 * (100000 * m_nPhotos + 1431);
  }//anti-B0 decays to a_00 pi0 pi0 pi0
  if (GenB0Tag::PcheckDecay(genpart, 10211, -211, 211, -211)) {
    return -1 * (100000 * m_nPhotos + 1432);
  }//anti-B0 decays to a_0+ pi- pi+ pi-
  if (GenB0Tag::PcheckDecay(genpart, 10211, -211, 111, 111)) {
    return -1 * (100000 * m_nPhotos + 1433);
  }//anti-B0 decays to a_0+ pi- pi0 pi0
  if (GenB0Tag::PcheckDecay(genpart, -10211, 211, 211, -211)) {
    return -1 * (100000 * m_nPhotos + 1434);
  }//anti-B0 decays to a_0- pi+ pi+ pi-
  if (GenB0Tag::PcheckDecay(genpart, -10211, 211, 111, 111)) {
    return -1 * (100000 * m_nPhotos + 1435);
  }//anti-B0 decays to a_0- pi+ pi0 pi0
  if (GenB0Tag::PcheckDecay(genpart, 113, -323, 211)) {
    return -1 * (100000 * m_nPhotos + 1436);
  }//anti-B0 decays to rho0 K*- pi+
  if (GenB0Tag::PcheckDecay(genpart, 213, -323, 111)) {
    return -1 * (100000 * m_nPhotos + 1437);
  }//anti-B0 decays to rho+ K*- pi0
  if (GenB0Tag::PcheckDecay(genpart, 213, -323, 221)) {
    return -1 * (100000 * m_nPhotos + 1438);
  }//anti-B0 decays to rho+ K*- eta
  if (GenB0Tag::PcheckDecay(genpart, 213, -323, 331)) {
    return -1 * (100000 * m_nPhotos + 1439);
  }//anti-B0 decays to rho+ K*- eta'
  if (GenB0Tag::PcheckDecay(genpart, 113, -313, 111)) {
    return -1 * (100000 * m_nPhotos + 1440);
  }//anti-B0 decays to rho0 anti-K*0 pi0
  if (GenB0Tag::PcheckDecay(genpart, 113, -313, 221)) {
    return -1 * (100000 * m_nPhotos + 1441);
  }//anti-B0 decays to rho0 anti-K*0 eta
  if (GenB0Tag::PcheckDecay(genpart, 113, -313, 331)) {
    return -1 * (100000 * m_nPhotos + 1442);
  }//anti-B0 decays to rho0 anti-K*0 eta'
  if (GenB0Tag::PcheckDecay(genpart, -213, -313, 211)) {
    return -1 * (100000 * m_nPhotos + 1443);
  }//anti-B0 decays to rho- anti-K*0 pi+
  if (GenB0Tag::PcheckDecay(genpart, 213, -313, -211)) {
    return -1 * (100000 * m_nPhotos + 1444);
  }//anti-B0 decays to rho+ anti-K*0 pi-
  if (GenB0Tag::PcheckDecay(genpart, 223, -323, 211)) {
    return -1 * (100000 * m_nPhotos + 1445);
  }//anti-B0 decays to omega K*- pi+
  if (GenB0Tag::PcheckDecay(genpart, 223, -313, 111)) {
    return -1 * (100000 * m_nPhotos + 1446);
  }//anti-B0 decays to omega anti-K*0 pi0
  if (GenB0Tag::PcheckDecay(genpart, 223, -313, 221)) {
    return -1 * (100000 * m_nPhotos + 1447);
  }//anti-B0 decays to omega anti-K*0 eta
  if (GenB0Tag::PcheckDecay(genpart, 223, -313, 331)) {
    return -1 * (100000 * m_nPhotos + 1448);
  }//anti-B0 decays to omega anti-K*0 eta'
  if (GenB0Tag::PcheckDecay(genpart, 211, -211, -323, 211)) {
    return -1 * (100000 * m_nPhotos + 1449);
  }//anti-B0 decays to pi+ pi- K*- pi+
  if (GenB0Tag::PcheckDecay(genpart, 211, 111, -323, 111)) {
    return -1 * (100000 * m_nPhotos + 1450);
  }//anti-B0 decays to pi+ pi0 K*- pi0
  if (GenB0Tag::PcheckDecay(genpart, 211, 111, -323, 221)) {
    return -1 * (100000 * m_nPhotos + 1451);
  }//anti-B0 decays to pi+ pi0 K*- eta
  if (GenB0Tag::PcheckDecay(genpart, 211, 111, -323, 331)) {
    return -1 * (100000 * m_nPhotos + 1452);
  }//anti-B0 decays to pi+ pi0 K*- eta'
  if (GenB0Tag::PcheckDecay(genpart, 211, -211, -313, 111)) {
    return -1 * (100000 * m_nPhotos + 1453);
  }//anti-B0 decays to pi+ pi- anti-K*0 pi0
  if (GenB0Tag::PcheckDecay(genpart, 211, -211, -313, 221)) {
    return -1 * (100000 * m_nPhotos + 1454);
  }//anti-B0 decays to pi+ pi- anti-K*0 eta
  if (GenB0Tag::PcheckDecay(genpart, 211, -211, -313, 331)) {
    return -1 * (100000 * m_nPhotos + 1455);
  }//anti-B0 decays to pi+ pi- anti-K*0 eta'
  if (GenB0Tag::PcheckDecay(genpart, 111, 111, -313, 111)) {
    return -1 * (100000 * m_nPhotos + 1456);
  }//anti-B0 decays to pi0 pi0 anti-K*0 pi0
  if (GenB0Tag::PcheckDecay(genpart, 111, 111, -313, 221)) {
    return -1 * (100000 * m_nPhotos + 1457);
  }//anti-B0 decays to pi0 pi0 anti-K*0 eta
  if (GenB0Tag::PcheckDecay(genpart, 111, 111, -313, 331)) {
    return -1 * (100000 * m_nPhotos + 1458);
  }//anti-B0 decays to pi0 pi0 anti-K*0 eta'
  if (GenB0Tag::PcheckDecay(genpart, -213, -321, 211, 211)) {
    return -1 * (100000 * m_nPhotos + 1459);
  }//anti-B0 decays to rho- K- pi+ pi+
  if (GenB0Tag::PcheckDecay(genpart, 113, -321, 211, 111)) {
    return -1 * (100000 * m_nPhotos + 1460);
  }//anti-B0 decays to rho0 K- pi+ pi0
  if (GenB0Tag::PcheckDecay(genpart, 113, -321, 211, 221)) {
    return -1 * (100000 * m_nPhotos + 1461);
  }//anti-B0 decays to rho0 K- pi+ eta
  if (GenB0Tag::PcheckDecay(genpart, 113, -321, 211, 331)) {
    return -1 * (100000 * m_nPhotos + 1462);
  }//anti-B0 decays to rho0 K- pi+ eta'
  if (GenB0Tag::PcheckDecay(genpart, 213, -321, 211, -211)) {
    return -1 * (100000 * m_nPhotos + 1463);
  }//anti-B0 decays to rho+ K- pi+ pi-
  if (GenB0Tag::PcheckDecay(genpart, 213, -321, 111, 111)) {
    return -1 * (100000 * m_nPhotos + 1464);
  }//anti-B0 decays to rho+ K- pi0 pi0
  if (GenB0Tag::PcheckDecay(genpart, 213, -321, 111, 221)) {
    return -1 * (100000 * m_nPhotos + 1465);
  }//anti-B0 decays to rho+ K- pi0 eta
  if (GenB0Tag::PcheckDecay(genpart, 213, -321, 111, 331)) {
    return -1 * (100000 * m_nPhotos + 1466);
  }//anti-B0 decays to rho+ K- pi0 eta'
  if (GenB0Tag::PcheckDecay(genpart, 113, -311, 211, -211)) {
    return -1 * (100000 * m_nPhotos + 1467);
  }//anti-B0 decays to rho0 anti-K0 pi+ pi-
  if (GenB0Tag::PcheckDecay(genpart, 113, -311, 111, 111)) {
    return -1 * (100000 * m_nPhotos + 1468);
  }//anti-B0 decays to rho0 anti-K0 pi0 pi0
  if (GenB0Tag::PcheckDecay(genpart, 113, -311, 111, 221)) {
    return -1 * (100000 * m_nPhotos + 1469);
  }//anti-B0 decays to rho0 anti-K0 pi0 eta
  if (GenB0Tag::PcheckDecay(genpart, 113, -311, 111, 331)) {
    return -1 * (100000 * m_nPhotos + 1470);
  }//anti-B0 decays to rho0 anti-K0 pi0 eta'
  if (GenB0Tag::PcheckDecay(genpart, -213, -311, 211, 111)) {
    return -1 * (100000 * m_nPhotos + 1471);
  }//anti-B0 decays to rho- anti-K0 pi+ pi0
  if (GenB0Tag::PcheckDecay(genpart, -213, -311, 211, 221)) {
    return -1 * (100000 * m_nPhotos + 1472);
  }//anti-B0 decays to rho- anti-K0 pi+ eta
  if (GenB0Tag::PcheckDecay(genpart, -213, -311, 211, 331)) {
    return -1 * (100000 * m_nPhotos + 1473);
  }//anti-B0 decays to rho- anti-K0 pi+ eta'
  if (GenB0Tag::PcheckDecay(genpart, 213, -311, -211, 111)) {
    return -1 * (100000 * m_nPhotos + 1474);
  }//anti-B0 decays to rho+ anti-K0 pi- pi0
  if (GenB0Tag::PcheckDecay(genpart, 213, -311, -211, 221)) {
    return -1 * (100000 * m_nPhotos + 1475);
  }//anti-B0 decays to rho+ anti-K0 pi- eta
  if (GenB0Tag::PcheckDecay(genpart, 213, -311, -211, 331)) {
    return -1 * (100000 * m_nPhotos + 1476);
  }//anti-B0 decays to rho+ anti-K0 pi- eta'
  if (GenB0Tag::PcheckDecay(genpart, 223, -321, 211, 111)) {
    return -1 * (100000 * m_nPhotos + 1477);
  }//anti-B0 decays to omega K- pi+ pi0
  if (GenB0Tag::PcheckDecay(genpart, 223, -321, 211, 221)) {
    return -1 * (100000 * m_nPhotos + 1478);
  }//anti-B0 decays to omega K- pi+ eta
  if (GenB0Tag::PcheckDecay(genpart, 223, -321, 211, 331)) {
    return -1 * (100000 * m_nPhotos + 1479);
  }//anti-B0 decays to omega K- pi+ eta'
  if (GenB0Tag::PcheckDecay(genpart, 223, -311, 211, -211)) {
    return -1 * (100000 * m_nPhotos + 1480);
  }//anti-B0 decays to omega anti-K0 pi+ pi-
  if (GenB0Tag::PcheckDecay(genpart, 223, -311, 111, 111)) {
    return -1 * (100000 * m_nPhotos + 1481);
  }//anti-B0 decays to omega anti-K0 pi0 pi0
  if (GenB0Tag::PcheckDecay(genpart, 223, -311, 111, 221)) {
    return -1 * (100000 * m_nPhotos + 1482);
  }//anti-B0 decays to omega anti-K0 pi0 eta
  if (GenB0Tag::PcheckDecay(genpart, 223, -311, 111, 331)) {
    return -1 * (100000 * m_nPhotos + 1483);
  }//anti-B0 decays to omega anti-K0 pi0 eta'
  if (GenB0Tag::PcheckDecay(genpart, 211, -211, 211, -321, 111)) {
    return -1 * (100000 * m_nPhotos + 1484);
  }//anti-B0 decays to pi+ pi- pi+ K- pi0
  if (GenB0Tag::PcheckDecay(genpart, 211, -211, 211, -321, 221)) {
    return -1 * (100000 * m_nPhotos + 1485);
  }//anti-B0 decays to pi+ pi- pi+ K- eta
  if (GenB0Tag::PcheckDecay(genpart, 211, -211, 211, -321, 331)) {
    return -1 * (100000 * m_nPhotos + 1486);
  }//anti-B0 decays to pi+ pi- pi+ K- eta'
  if (GenB0Tag::PcheckDecay(genpart, 111, 111, 211, -321, 111)) {
    return -1 * (100000 * m_nPhotos + 1487);
  }//anti-B0 decays to pi0 pi0 pi+ K- pi0
  if (GenB0Tag::PcheckDecay(genpart, 111, 111, 211, -321, 221)) {
    return -1 * (100000 * m_nPhotos + 1488);
  }//anti-B0 decays to pi0 pi0 pi+ K- eta
  if (GenB0Tag::PcheckDecay(genpart, 111, 111, 211, -321, 331)) {
    return -1 * (100000 * m_nPhotos + 1489);
  }//anti-B0 decays to pi0 pi0 pi+ K- eta'
  if (GenB0Tag::PcheckDecay(genpart, 211, -211, 211, -311, -211)) {
    return -1 * (100000 * m_nPhotos + 1490);
  }//anti-B0 decays to pi+ pi- pi+ anti-K0 pi-
  if (GenB0Tag::PcheckDecay(genpart, 211, -211, 111, -311, 111)) {
    return -1 * (100000 * m_nPhotos + 1491);
  }//anti-B0 decays to pi+ pi- pi0 anti-K0 pi0
  if (GenB0Tag::PcheckDecay(genpart, 211, -211, 111, -311, 221)) {
    return -1 * (100000 * m_nPhotos + 1492);
  }//anti-B0 decays to pi+ pi- pi0 anti-K0 eta
  if (GenB0Tag::PcheckDecay(genpart, 211, -211, 111, -311, 331)) {
    return -1 * (100000 * m_nPhotos + 1493);
  }//anti-B0 decays to pi+ pi- pi0 anti-K0 eta'
  if (GenB0Tag::PcheckDecay(genpart, 111, 111, 111, -311, 111)) {
    return -1 * (100000 * m_nPhotos + 1494);
  }//anti-B0 decays to pi0 pi0 pi0 anti-K0 pi0
  if (GenB0Tag::PcheckDecay(genpart, 111, 111, 111, -311, 221)) {
    return -1 * (100000 * m_nPhotos + 1495);
  }//anti-B0 decays to pi0 pi0 pi0 anti-K0 eta
  if (GenB0Tag::PcheckDecay(genpart, 111, 111, 111, -311, 331)) {
    return -1 * (100000 * m_nPhotos + 1496);
  }//anti-B0 decays to pi0 pi0 pi0 anti-K0 eta'
  if (GenB0Tag::PcheckDecay(genpart, 213, -10311, -211)) {
    return -1 * (100000 * m_nPhotos + 1497);
  }//anti-B0 decays to rho+ anti-K_0*0 pi-
  if (GenB0Tag::PcheckDecay(genpart, 113, -10311, 111)) {
    return -1 * (100000 * m_nPhotos + 1498);
  }//anti-B0 decays to rho0 anti-K_0*0 pi0
  if (GenB0Tag::PcheckDecay(genpart, 113, -10311, 221)) {
    return -1 * (100000 * m_nPhotos + 1499);
  }//anti-B0 decays to rho0 anti-K_0*0 eta
  if (GenB0Tag::PcheckDecay(genpart, 113, -10311, 331)) {
    return -1 * (100000 * m_nPhotos + 1500);
  }//anti-B0 decays to rho0 anti-K_0*0 eta'
  if (GenB0Tag::PcheckDecay(genpart, -213, -10311, 211)) {
    return -1 * (100000 * m_nPhotos + 1501);
  }//anti-B0 decays to rho- anti-K_0*0 pi+
  if (GenB0Tag::PcheckDecay(genpart, 213, -10321, 111)) {
    return -1 * (100000 * m_nPhotos + 1502);
  }//anti-B0 decays to rho+ K_0*- pi0
  if (GenB0Tag::PcheckDecay(genpart, 213, -10321, 221)) {
    return -1 * (100000 * m_nPhotos + 1503);
  }//anti-B0 decays to rho+ K_0*- eta
  if (GenB0Tag::PcheckDecay(genpart, 213, -10321, 331)) {
    return -1 * (100000 * m_nPhotos + 1504);
  }//anti-B0 decays to rho+ K_0*- eta'
  if (GenB0Tag::PcheckDecay(genpart, 113, -10321, 211)) {
    return -1 * (100000 * m_nPhotos + 1505);
  }//anti-B0 decays to rho0 K_0*- pi+
  if (GenB0Tag::PcheckDecay(genpart, 211, -211, -10321, 211)) {
    return -1 * (100000 * m_nPhotos + 1506);
  }//anti-B0 decays to pi+ pi- K_0*- pi+
  if (GenB0Tag::PcheckDecay(genpart, 211, 111, -10321, 111)) {
    return -1 * (100000 * m_nPhotos + 1507);
  }//anti-B0 decays to pi+ pi0 K_0*- pi0
  if (GenB0Tag::PcheckDecay(genpart, 211, 111, -10321, 221)) {
    return -1 * (100000 * m_nPhotos + 1508);
  }//anti-B0 decays to pi+ pi0 K_0*- eta
  if (GenB0Tag::PcheckDecay(genpart, 211, 111, -10321, 331)) {
    return -1 * (100000 * m_nPhotos + 1509);
  }//anti-B0 decays to pi+ pi0 K_0*- eta'
  if (GenB0Tag::PcheckDecay(genpart, 211, -211, -10311, 111)) {
    return -1 * (100000 * m_nPhotos + 1510);
  }//anti-B0 decays to pi+ pi- anti-K_0*0 pi0
  if (GenB0Tag::PcheckDecay(genpart, 211, -211, -10311, 221)) {
    return -1 * (100000 * m_nPhotos + 1511);
  }//anti-B0 decays to pi+ pi- anti-K_0*0 eta
  if (GenB0Tag::PcheckDecay(genpart, 211, -211, -10311, 331)) {
    return -1 * (100000 * m_nPhotos + 1512);
  }//anti-B0 decays to pi+ pi- anti-K_0*0 eta'
  if (GenB0Tag::PcheckDecay(genpart, 111, 111, -10311, 111)) {
    return -1 * (100000 * m_nPhotos + 1513);
  }//anti-B0 decays to pi0 pi0 anti-K_0*0 pi0
  if (GenB0Tag::PcheckDecay(genpart, 111, 111, -10311, 221)) {
    return -1 * (100000 * m_nPhotos + 1514);
  }//anti-B0 decays to pi0 pi0 anti-K_0*0 eta
  if (GenB0Tag::PcheckDecay(genpart, 111, 111, -10311, 331)) {
    return -1 * (100000 * m_nPhotos + 1515);
  }//anti-B0 decays to pi0 pi0 anti-K_0*0 eta'
  if (GenB0Tag::PcheckDecay(genpart, -323, 10221, 211)) {
    return -1 * (100000 * m_nPhotos + 1516);
  }//anti-B0 decays to K*- f_0 pi+
  if (GenB0Tag::PcheckDecay(genpart, -313, 10221, 111)) {
    return -1 * (100000 * m_nPhotos + 1517);
  }//anti-B0 decays to anti-K*0 f_0 pi0
  if (GenB0Tag::PcheckDecay(genpart, -313, 10221, 221)) {
    return -1 * (100000 * m_nPhotos + 1518);
  }//anti-B0 decays to anti-K*0 f_0 eta
  if (GenB0Tag::PcheckDecay(genpart, -313, 10221, 331)) {
    return -1 * (100000 * m_nPhotos + 1519);
  }//anti-B0 decays to anti-K*0 f_0 eta'
  if (GenB0Tag::PcheckDecay(genpart, 20213, -311, -211)) {
    return -1 * (100000 * m_nPhotos + 1520);
  }//anti-B0 decays to a_1+ anti-K0 pi-
  if (GenB0Tag::PcheckDecay(genpart, 20113, -313)) {
    return -1 * (100000 * m_nPhotos + 1521);
  }//anti-B0 decays to a_10 anti-K*0
  if (GenB0Tag::PcheckDecay(genpart, 20113, -321, 211)) {
    return -1 * (100000 * m_nPhotos + 1522);
  }//anti-B0 decays to a_10 K- pi+
  if (GenB0Tag::PcheckDecay(genpart, 20113, -311, 111)) {
    return -1 * (100000 * m_nPhotos + 1523);
  }//anti-B0 decays to a_10 anti-K0 pi0
  if (GenB0Tag::PcheckDecay(genpart, 20213, -323)) {
    return -1 * (100000 * m_nPhotos + 1524);
  }//anti-B0 decays to a_1+ K*-
  if (GenB0Tag::PcheckDecay(genpart, 20213, -321, 111)) {
    return -1 * (100000 * m_nPhotos + 1525);
  }//anti-B0 decays to a_1+ K- pi0
  if (GenB0Tag::PcheckDecay(genpart, -20213, -311, 211)) {
    return -1 * (100000 * m_nPhotos + 1526);
  }//anti-B0 decays to a_1- anti-K0 pi+
  if (GenB0Tag::PcheckDecay(genpart, -323, 10111, 211)) {
    return -1 * (100000 * m_nPhotos + 1527);
  }//anti-B0 decays to K*- a_00 pi+
  if (GenB0Tag::PcheckDecay(genpart, -323, 10211, 111)) {
    return -1 * (100000 * m_nPhotos + 1528);
  }//anti-B0 decays to K*- a_0+ pi0
  if (GenB0Tag::PcheckDecay(genpart, -323, 10211, 221)) {
    return -1 * (100000 * m_nPhotos + 1529);
  }//anti-B0 decays to K*- a_0+ eta
  if (GenB0Tag::PcheckDecay(genpart, -323, 10211, 331)) {
    return -1 * (100000 * m_nPhotos + 1530);
  }//anti-B0 decays to K*- a_0+ eta'
  if (GenB0Tag::PcheckDecay(genpart, -313, 10211, -211)) {
    return -1 * (100000 * m_nPhotos + 1531);
  }//anti-B0 decays to anti-K*0 a_0+ pi-
  if (GenB0Tag::PcheckDecay(genpart, -313, 10111, 111)) {
    return -1 * (100000 * m_nPhotos + 1532);
  }//anti-B0 decays to anti-K*0 a_00 pi0
  if (GenB0Tag::PcheckDecay(genpart, -313, 10111, 221)) {
    return -1 * (100000 * m_nPhotos + 1533);
  }//anti-B0 decays to anti-K*0 a_00 eta
  if (GenB0Tag::PcheckDecay(genpart, -313, 10111, 331)) {
    return -1 * (100000 * m_nPhotos + 1534);
  }//anti-B0 decays to anti-K*0 a_00 eta'
  if (GenB0Tag::PcheckDecay(genpart, -323, 313, 211)) {
    return -1 * (100000 * m_nPhotos + 1535);
  }//anti-B0 decays to K*- K*0 pi+
  if (GenB0Tag::PcheckDecay(genpart, 323, -313, -211)) {
    return -1 * (100000 * m_nPhotos + 1536);
  }//anti-B0 decays to K*+ anti-K*0 pi-
  if (GenB0Tag::PcheckDecay(genpart, -323, 323, 111)) {
    return -1 * (100000 * m_nPhotos + 1537);
  }//anti-B0 decays to K*- K*+ pi0
  if (GenB0Tag::PcheckDecay(genpart, -323, 323, 221)) {
    return -1 * (100000 * m_nPhotos + 1538);
  }//anti-B0 decays to K*- K*+ eta
  if (GenB0Tag::PcheckDecay(genpart, -323, 323, 331)) {
    return -1 * (100000 * m_nPhotos + 1539);
  }//anti-B0 decays to K*- K*+ eta'
  if (GenB0Tag::PcheckDecay(genpart, 313, -313, 111)) {
    return -1 * (100000 * m_nPhotos + 1540);
  }//anti-B0 decays to K*0 anti-K*0 pi0
  if (GenB0Tag::PcheckDecay(genpart, 313, -313, 221)) {
    return -1 * (100000 * m_nPhotos + 1541);
  }//anti-B0 decays to K*0 anti-K*0 eta
  if (GenB0Tag::PcheckDecay(genpart, 313, -313, 331)) {
    return -1 * (100000 * m_nPhotos + 1542);
  }//anti-B0 decays to K*0 anti-K*0 eta'
  if (GenB0Tag::PcheckDecay(genpart, 333, 213, -211)) {
    return -1 * (100000 * m_nPhotos + 1543);
  }//anti-B0 decays to phi rho+ pi-
  if (GenB0Tag::PcheckDecay(genpart, 333, -213, 211)) {
    return -1 * (100000 * m_nPhotos + 1544);
  }//anti-B0 decays to phi rho- pi+
  if (GenB0Tag::PcheckDecay(genpart, 333, 113, 111)) {
    return -1 * (100000 * m_nPhotos + 1545);
  }//anti-B0 decays to phi rho0 pi0
  if (GenB0Tag::PcheckDecay(genpart, 333, 113, 221)) {
    return -1 * (100000 * m_nPhotos + 1546);
  }//anti-B0 decays to phi rho0 eta
  if (GenB0Tag::PcheckDecay(genpart, 333, 113, 331)) {
    return -1 * (100000 * m_nPhotos + 1547);
  }//anti-B0 decays to phi rho0 eta'
  if (GenB0Tag::PcheckDecay(genpart, 333, 223, 111)) {
    return -1 * (100000 * m_nPhotos + 1548);
  }//anti-B0 decays to phi omega pi0
  if (GenB0Tag::PcheckDecay(genpart, 333, 10221, 111)) {
    return -1 * (100000 * m_nPhotos + 1549);
  }//anti-B0 decays to phi f_0 pi0
  if (GenB0Tag::PcheckDecay(genpart, 333, 211, -211, 111)) {
    return -1 * (100000 * m_nPhotos + 1550);
  }//anti-B0 decays to phi pi+ pi- pi0
  if (GenB0Tag::PcheckDecay(genpart, 333, 211, -211, 221)) {
    return -1 * (100000 * m_nPhotos + 1551);
  }//anti-B0 decays to phi pi+ pi- eta
  if (GenB0Tag::PcheckDecay(genpart, 333, 211, -211, 331)) {
    return -1 * (100000 * m_nPhotos + 1552);
  }//anti-B0 decays to phi pi+ pi- eta'
  if (GenB0Tag::PcheckDecay(genpart, 333, 111, 111, 111)) {
    return -1 * (100000 * m_nPhotos + 1553);
  }//anti-B0 decays to phi pi0 pi0 pi0
  if (GenB0Tag::PcheckDecay(genpart, 333, 111, 111, 221)) {
    return -1 * (100000 * m_nPhotos + 1554);
  }//anti-B0 decays to phi pi0 pi0 eta
  if (GenB0Tag::PcheckDecay(genpart, 333, 111, 111, 331)) {
    return -1 * (100000 * m_nPhotos + 1555);
  }//anti-B0 decays to phi pi0 pi0 eta'
  if (GenB0Tag::PcheckDecay(genpart, 311, -321, 113, 211)) {
    return -1 * (100000 * m_nPhotos + 1556);
  }//anti-B0 decays to K0 K- rho0 pi+
  if (GenB0Tag::PcheckDecay(genpart, 311, -321, 213, 111)) {
    return -1 * (100000 * m_nPhotos + 1557);
  }//anti-B0 decays to K0 K- rho+ pi0
  if (GenB0Tag::PcheckDecay(genpart, 311, -321, 213, 221)) {
    return -1 * (100000 * m_nPhotos + 1558);
  }//anti-B0 decays to K0 K- rho+ eta
  if (GenB0Tag::PcheckDecay(genpart, 311, -321, 213, 331)) {
    return -1 * (100000 * m_nPhotos + 1559);
  }//anti-B0 decays to K0 K- rho+ eta'
  if (GenB0Tag::PcheckDecay(genpart, 311, -321, 223, 211)) {
    return -1 * (100000 * m_nPhotos + 1560);
  }//anti-B0 decays to K0 K- omega pi+
  if (GenB0Tag::PcheckDecay(genpart, 311, -321, 10221, 211)) {
    return -1 * (100000 * m_nPhotos + 1561);
  }//anti-B0 decays to K0 K- f_0 pi+
  if (GenB0Tag::PcheckDecay(genpart, 321, -321, 213, -211)) {
    return -1 * (100000 * m_nPhotos + 1562);
  }//anti-B0 decays to K+ K- rho+ pi-
  if (GenB0Tag::PcheckDecay(genpart, 321, -321, -213, 211)) {
    return -1 * (100000 * m_nPhotos + 1563);
  }//anti-B0 decays to K+ K- rho- pi+
  if (GenB0Tag::PcheckDecay(genpart, 321, -321, 113, 111)) {
    return -1 * (100000 * m_nPhotos + 1564);
  }//anti-B0 decays to K+ K- rho0 pi0
  if (GenB0Tag::PcheckDecay(genpart, 321, -321, 113, 221)) {
    return -1 * (100000 * m_nPhotos + 1565);
  }//anti-B0 decays to K+ K- rho0 eta
  if (GenB0Tag::PcheckDecay(genpart, 321, -321, 113, 331)) {
    return -1 * (100000 * m_nPhotos + 1566);
  }//anti-B0 decays to K+ K- rho0 eta'
  if (GenB0Tag::PcheckDecay(genpart, 321, -321, 223, 111)) {
    return -1 * (100000 * m_nPhotos + 1567);
  }//anti-B0 decays to K+ K- omega pi0
  if (GenB0Tag::PcheckDecay(genpart, 321, -321, 223, 221)) {
    return -1 * (100000 * m_nPhotos + 1568);
  }//anti-B0 decays to K+ K- omega eta
  if (GenB0Tag::PcheckDecay(genpart, 321, -321, 223, 331)) {
    return -1 * (100000 * m_nPhotos + 1569);
  }//anti-B0 decays to K+ K- omega eta'
  if (GenB0Tag::PcheckDecay(genpart, 321, -321, 10221, 111)) {
    return -1 * (100000 * m_nPhotos + 1570);
  }//anti-B0 decays to K+ K- f_0 pi0
  if (GenB0Tag::PcheckDecay(genpart, 311, -311, 213, -211)) {
    return -1 * (100000 * m_nPhotos + 1571);
  }//anti-B0 decays to K0 anti-K0 rho+ pi-
  if (GenB0Tag::PcheckDecay(genpart, 311, -311, -213, 211)) {
    return -1 * (100000 * m_nPhotos + 1572);
  }//anti-B0 decays to K0 anti-K0 rho- pi+
  if (GenB0Tag::PcheckDecay(genpart, 311, -311, 113, 111)) {
    return -1 * (100000 * m_nPhotos + 1573);
  }//anti-B0 decays to K0 anti-K0 rho0 pi0
  if (GenB0Tag::PcheckDecay(genpart, 311, -311, 113, 221)) {
    return -1 * (100000 * m_nPhotos + 1574);
  }//anti-B0 decays to K0 anti-K0 rho0 eta
  if (GenB0Tag::PcheckDecay(genpart, 311, -311, 113, 331)) {
    return -1 * (100000 * m_nPhotos + 1575);
  }//anti-B0 decays to K0 anti-K0 rho0 eta'
  if (GenB0Tag::PcheckDecay(genpart, 311, -311, 223, 111)) {
    return -1 * (100000 * m_nPhotos + 1576);
  }//anti-B0 decays to K0 anti-K0 omega pi0
  if (GenB0Tag::PcheckDecay(genpart, 311, -311, 10221, 111)) {
    return -1 * (100000 * m_nPhotos + 1577);
  }//anti-B0 decays to K0 anti-K0 f_0 pi0
  if (GenB0Tag::PcheckDecay(genpart, -311, 321, 113, -211)) {
    return -1 * (100000 * m_nPhotos + 1578);
  }//anti-B0 decays to anti-K0 K+ rho0 pi-
  if (GenB0Tag::PcheckDecay(genpart, -311, 321, -213, 111)) {
    return -1 * (100000 * m_nPhotos + 1579);
  }//anti-B0 decays to anti-K0 K+ rho- pi0
  if (GenB0Tag::PcheckDecay(genpart, -311, 321, -213, 221)) {
    return -1 * (100000 * m_nPhotos + 1580);
  }//anti-B0 decays to anti-K0 K+ rho- eta
  if (GenB0Tag::PcheckDecay(genpart, -311, 321, -213, 331)) {
    return -1 * (100000 * m_nPhotos + 1581);
  }//anti-B0 decays to anti-K0 K+ rho- eta'
  if (GenB0Tag::PcheckDecay(genpart, -311, 321, 223, -211)) {
    return -1 * (100000 * m_nPhotos + 1582);
  }//anti-B0 decays to anti-K0 K+ omega pi-
  if (GenB0Tag::PcheckDecay(genpart, -311, 321, 10221, -211)) {
    return -1 * (100000 * m_nPhotos + 1583);
  }//anti-B0 decays to anti-K0 K+ f_0 pi-
  if (GenB0Tag::PcheckDecay(genpart, 311, -321, 211, 211, -211)) {
    return -1 * (100000 * m_nPhotos + 1584);
  }//anti-B0 decays to K0 K- pi+ pi+ pi-
  if (GenB0Tag::PcheckDecay(genpart, 311, -321, 211, 111, 111)) {
    return -1 * (100000 * m_nPhotos + 1585);
  }//anti-B0 decays to K0 K- pi+ pi0 pi0
  if (GenB0Tag::PcheckDecay(genpart, 311, -321, 211, 111, 221)) {
    return -1 * (100000 * m_nPhotos + 1586);
  }//anti-B0 decays to K0 K- pi+ pi0 eta
  if (GenB0Tag::PcheckDecay(genpart, 311, -321, 211, 111, 331)) {
    return -1 * (100000 * m_nPhotos + 1587);
  }//anti-B0 decays to K0 K- pi+ pi0 eta'
  if (GenB0Tag::PcheckDecay(genpart, 321, -321, 211, -211, 111)) {
    return -1 * (100000 * m_nPhotos + 1588);
  }//anti-B0 decays to K+ K- pi+ pi- pi0
  if (GenB0Tag::PcheckDecay(genpart, 321, -321, 211, -211, 221)) {
    return -1 * (100000 * m_nPhotos + 1589);
  }//anti-B0 decays to K+ K- pi+ pi- eta
  if (GenB0Tag::PcheckDecay(genpart, 321, -321, 211, -211, 331)) {
    return -1 * (100000 * m_nPhotos + 1590);
  }//anti-B0 decays to K+ K- pi+ pi- eta'
  if (GenB0Tag::PcheckDecay(genpart, 321, -321, 111, 111, 111)) {
    return -1 * (100000 * m_nPhotos + 1591);
  }//anti-B0 decays to K+ K- pi0 pi0 pi0
  if (GenB0Tag::PcheckDecay(genpart, 321, -321, 111, 111, 221)) {
    return -1 * (100000 * m_nPhotos + 1592);
  }//anti-B0 decays to K+ K- pi0 pi0 eta
  if (GenB0Tag::PcheckDecay(genpart, 321, -321, 111, 111, 331)) {
    return -1 * (100000 * m_nPhotos + 1593);
  }//anti-B0 decays to K+ K- pi0 pi0 eta'
  if (GenB0Tag::PcheckDecay(genpart, 311, -311, 211, -211, 111)) {
    return -1 * (100000 * m_nPhotos + 1594);
  }//anti-B0 decays to K0 anti-K0 pi+ pi- pi0
  if (GenB0Tag::PcheckDecay(genpart, 311, -311, 211, -211, 221)) {
    return -1 * (100000 * m_nPhotos + 1595);
  }//anti-B0 decays to K0 anti-K0 pi+ pi- eta
  if (GenB0Tag::PcheckDecay(genpart, 311, -311, 211, -211, 331)) {
    return -1 * (100000 * m_nPhotos + 1596);
  }//anti-B0 decays to K0 anti-K0 pi+ pi- eta'
  if (GenB0Tag::PcheckDecay(genpart, 311, -311, 111, 111, 111)) {
    return -1 * (100000 * m_nPhotos + 1597);
  }//anti-B0 decays to K0 anti-K0 pi0 pi0 pi0
  if (GenB0Tag::PcheckDecay(genpart, 311, -311, 111, 111, 221)) {
    return -1 * (100000 * m_nPhotos + 1598);
  }//anti-B0 decays to K0 anti-K0 pi0 pi0 eta
  if (GenB0Tag::PcheckDecay(genpart, 311, -311, 111, 111, 331)) {
    return -1 * (100000 * m_nPhotos + 1599);
  }//anti-B0 decays to K0 anti-K0 pi0 pi0 eta'
  if (GenB0Tag::PcheckDecay(genpart, -311, 321, -211, 211, -211)) {
    return -1 * (100000 * m_nPhotos + 1600);
  }//anti-B0 decays to anti-K0 K+ pi- pi+ pi-
  if (GenB0Tag::PcheckDecay(genpart, -311, 321, -211, 111, 111)) {
    return -1 * (100000 * m_nPhotos + 1601);
  }//anti-B0 decays to anti-K0 K+ pi- pi0 pi0
  if (GenB0Tag::PcheckDecay(genpart, -311, 321, -211, 111, 221)) {
    return -1 * (100000 * m_nPhotos + 1602);
  }//anti-B0 decays to anti-K0 K+ pi- pi0 eta
  if (GenB0Tag::PcheckDecay(genpart, -311, 321, -211, 111, 331)) {
    return -1 * (100000 * m_nPhotos + 1603);
  }//anti-B0 decays to anti-K0 K+ pi- pi0 eta'
  if (GenB0Tag::PcheckDecay(genpart, 313, -321, 211, 111)) {
    return -1 * (100000 * m_nPhotos + 1604);
  }//anti-B0 decays to K*0 K- pi+ pi0
  if (GenB0Tag::PcheckDecay(genpart, 313, -321, 211, 221)) {
    return -1 * (100000 * m_nPhotos + 1605);
  }//anti-B0 decays to K*0 K- pi+ eta
  if (GenB0Tag::PcheckDecay(genpart, 313, -321, 211, 331)) {
    return -1 * (100000 * m_nPhotos + 1606);
  }//anti-B0 decays to K*0 K- pi+ eta'
  if (GenB0Tag::PcheckDecay(genpart, -323, 311, 211, 111)) {
    return -1 * (100000 * m_nPhotos + 1607);
  }//anti-B0 decays to K*- K0 pi+ pi0
  if (GenB0Tag::PcheckDecay(genpart, -323, 311, 211, 221)) {
    return -1 * (100000 * m_nPhotos + 1608);
  }//anti-B0 decays to K*- K0 pi+ eta
  if (GenB0Tag::PcheckDecay(genpart, -323, 311, 211, 331)) {
    return -1 * (100000 * m_nPhotos + 1609);
  }//anti-B0 decays to K*- K0 pi+ eta'
  if (GenB0Tag::PcheckDecay(genpart, 323, -321, 211, -211)) {
    return -1 * (100000 * m_nPhotos + 1610);
  }//anti-B0 decays to K*+ K- pi+ pi-
  if (GenB0Tag::PcheckDecay(genpart, 323, -321, 111, 111)) {
    return -1 * (100000 * m_nPhotos + 1611);
  }//anti-B0 decays to K*+ K- pi0 pi0
  if (GenB0Tag::PcheckDecay(genpart, 323, -321, 111, 221)) {
    return -1 * (100000 * m_nPhotos + 1612);
  }//anti-B0 decays to K*+ K- pi0 eta
  if (GenB0Tag::PcheckDecay(genpart, 323, -321, 111, 331)) {
    return -1 * (100000 * m_nPhotos + 1613);
  }//anti-B0 decays to K*+ K- pi0 eta'
  if (GenB0Tag::PcheckDecay(genpart, -323, 321, 211, -211)) {
    return -1 * (100000 * m_nPhotos + 1614);
  }//anti-B0 decays to K*- K+ pi+ pi-
  if (GenB0Tag::PcheckDecay(genpart, -323, 321, 111, 111)) {
    return -1 * (100000 * m_nPhotos + 1615);
  }//anti-B0 decays to K*- K+ pi0 pi0
  if (GenB0Tag::PcheckDecay(genpart, -323, 321, 111, 221)) {
    return -1 * (100000 * m_nPhotos + 1616);
  }//anti-B0 decays to K*- K+ pi0 eta
  if (GenB0Tag::PcheckDecay(genpart, -323, 321, 111, 331)) {
    return -1 * (100000 * m_nPhotos + 1617);
  }//anti-B0 decays to K*- K+ pi0 eta'
  if (GenB0Tag::PcheckDecay(genpart, 313, -311, 211, -211)) {
    return -1 * (100000 * m_nPhotos + 1618);
  }//anti-B0 decays to K*0 anti-K0 pi+ pi-
  if (GenB0Tag::PcheckDecay(genpart, 313, -311, 111, 111)) {
    return -1 * (100000 * m_nPhotos + 1619);
  }//anti-B0 decays to K*0 anti-K0 pi0 pi0
  if (GenB0Tag::PcheckDecay(genpart, 313, -311, 111, 221)) {
    return -1 * (100000 * m_nPhotos + 1620);
  }//anti-B0 decays to K*0 anti-K0 pi0 eta
  if (GenB0Tag::PcheckDecay(genpart, 313, -311, 111, 331)) {
    return -1 * (100000 * m_nPhotos + 1621);
  }//anti-B0 decays to K*0 anti-K0 pi0 eta'
  if (GenB0Tag::PcheckDecay(genpart, 311, -313, 211, -211)) {
    return -1 * (100000 * m_nPhotos + 1622);
  }//anti-B0 decays to K0 anti-K*0 pi+ pi-
  if (GenB0Tag::PcheckDecay(genpart, 311, -313, 111, 111)) {
    return -1 * (100000 * m_nPhotos + 1623);
  }//anti-B0 decays to K0 anti-K*0 pi0 pi0
  if (GenB0Tag::PcheckDecay(genpart, 311, -313, 111, 221)) {
    return -1 * (100000 * m_nPhotos + 1624);
  }//anti-B0 decays to K0 anti-K*0 pi0 eta
  if (GenB0Tag::PcheckDecay(genpart, 311, -313, 111, 331)) {
    return -1 * (100000 * m_nPhotos + 1625);
  }//anti-B0 decays to K0 anti-K*0 pi0 eta'
  if (GenB0Tag::PcheckDecay(genpart, 321, -313, -211, 111)) {
    return -1 * (100000 * m_nPhotos + 1626);
  }//anti-B0 decays to K+ anti-K*0 pi- pi0
  if (GenB0Tag::PcheckDecay(genpart, 321, -313, -211, 221)) {
    return -1 * (100000 * m_nPhotos + 1627);
  }//anti-B0 decays to K+ anti-K*0 pi- eta
  if (GenB0Tag::PcheckDecay(genpart, 321, -313, -211, 331)) {
    return -1 * (100000 * m_nPhotos + 1628);
  }//anti-B0 decays to K+ anti-K*0 pi- eta'
  if (GenB0Tag::PcheckDecay(genpart, 323, -311, -211, 111)) {
    return -1 * (100000 * m_nPhotos + 1629);
  }//anti-B0 decays to K*+ anti-K0 pi- pi0
  if (GenB0Tag::PcheckDecay(genpart, 323, -311, -211, 221)) {
    return -1 * (100000 * m_nPhotos + 1630);
  }//anti-B0 decays to K*+ anti-K0 pi- eta
  if (GenB0Tag::PcheckDecay(genpart, 323, -311, -211, 331)) {
    return -1 * (100000 * m_nPhotos + 1631);
  }//anti-B0 decays to K*+ anti-K0 pi- eta'
  if (GenB0Tag::PcheckDecay(genpart, 333, -323, 211)) {
    return -1 * (100000 * m_nPhotos + 1632);
  }//anti-B0 decays to phi K*- pi+
  if (GenB0Tag::PcheckDecay(genpart, 333, -313, 111)) {
    return -1 * (100000 * m_nPhotos + 1633);
  }//anti-B0 decays to phi anti-K*0 pi0
  if (GenB0Tag::PcheckDecay(genpart, 333, -313, 221)) {
    return -1 * (100000 * m_nPhotos + 1634);
  }//anti-B0 decays to phi anti-K*0 eta
  if (GenB0Tag::PcheckDecay(genpart, 333, -313, 331)) {
    return -1 * (100000 * m_nPhotos + 1635);
  }//anti-B0 decays to phi anti-K*0 eta'
  if (GenB0Tag::PcheckDecay(genpart, 333, -321, 213)) {
    return -1 * (100000 * m_nPhotos + 1636);
  }//anti-B0 decays to phi K- rho+
  if (GenB0Tag::PcheckDecay(genpart, 333, -311, 113)) {
    return -1 * (100000 * m_nPhotos + 1637);
  }//anti-B0 decays to phi anti-K0 rho0
  if (GenB0Tag::PcheckDecay(genpart, 333, -311, 223)) {
    return -1 * (100000 * m_nPhotos + 1638);
  }//anti-B0 decays to phi anti-K0 omega
  if (GenB0Tag::PcheckDecay(genpart, 333, -311, 10221)) {
    return -1 * (100000 * m_nPhotos + 1639);
  }//anti-B0 decays to phi anti-K0 f_0
  if (GenB0Tag::PcheckDecay(genpart, 333, -311, 211, -211)) {
    return -1 * (100000 * m_nPhotos + 1640);
  }//anti-B0 decays to phi anti-K0 pi+ pi-
  if (GenB0Tag::PcheckDecay(genpart, 333, -311, 111, 111)) {
    return -1 * (100000 * m_nPhotos + 1641);
  }//anti-B0 decays to phi anti-K0 pi0 pi0
  if (GenB0Tag::PcheckDecay(genpart, 333, -311, 111, 221)) {
    return -1 * (100000 * m_nPhotos + 1642);
  }//anti-B0 decays to phi anti-K0 pi0 eta
  if (GenB0Tag::PcheckDecay(genpart, 333, -311, 111, 331)) {
    return -1 * (100000 * m_nPhotos + 1643);
  }//anti-B0 decays to phi anti-K0 pi0 eta'
  if (GenB0Tag::PcheckDecay(genpart, 333, -321, 211, 111)) {
    return -1 * (100000 * m_nPhotos + 1644);
  }//anti-B0 decays to phi K- pi+ pi0
  if (GenB0Tag::PcheckDecay(genpart, 333, -321, 211, 221)) {
    return -1 * (100000 * m_nPhotos + 1645);
  }//anti-B0 decays to phi K- pi+ eta
  if (GenB0Tag::PcheckDecay(genpart, 333, -321, 211, 331)) {
    return -1 * (100000 * m_nPhotos + 1646);
  }//anti-B0 decays to phi K- pi+ eta'
  if (GenB0Tag::PcheckDecay(genpart, 321, -321, -323, 211)) {
    return -1 * (100000 * m_nPhotos + 1647);
  }//anti-B0 decays to K+ K- K*- pi+
  if (GenB0Tag::PcheckDecay(genpart, -321, 323, -321, 211)) {
    return -1 * (100000 * m_nPhotos + 1648);
  }//anti-B0 decays to K- K*+ K- pi+
  if (GenB0Tag::PcheckDecay(genpart, -321, 313, -311, 211)) {
    return -1 * (100000 * m_nPhotos + 1649);
  }//anti-B0 decays to K- K*0 anti-K0 pi+
  if (GenB0Tag::PcheckDecay(genpart, -321, 311, -313, 211)) {
    return -1 * (100000 * m_nPhotos + 1650);
  }//anti-B0 decays to K- K0 anti-K*0 pi+
  if (GenB0Tag::PcheckDecay(genpart, 311, -323, -311, 211)) {
    return -1 * (100000 * m_nPhotos + 1651);
  }//anti-B0 decays to K0 K*- anti-K0 pi+
  if (GenB0Tag::PcheckDecay(genpart, -321, 321, -321, 213)) {
    return -1 * (100000 * m_nPhotos + 1652);
  }//anti-B0 decays to K- K+ K- rho+
  if (GenB0Tag::PcheckDecay(genpart, -321, 321, -311, 113)) {
    return -1 * (100000 * m_nPhotos + 1653);
  }//anti-B0 decays to K- K+ anti-K0 rho0
  if (GenB0Tag::PcheckDecay(genpart, -321, 321, -311, 223)) {
    return -1 * (100000 * m_nPhotos + 1654);
  }//anti-B0 decays to K- K+ anti-K0 omega
  if (GenB0Tag::PcheckDecay(genpart, -321, 321, -311, 10221)) {
    return -1 * (100000 * m_nPhotos + 1655);
  }//anti-B0 decays to K- K+ anti-K0 f_0
  if (GenB0Tag::PcheckDecay(genpart, 321, -321, -321, 211, 111)) {
    return -1 * (100000 * m_nPhotos + 1656);
  }//anti-B0 decays to K+ K- K- pi+ pi0
  if (GenB0Tag::PcheckDecay(genpart, 321, -321, -321, 211, 221)) {
    return -1 * (100000 * m_nPhotos + 1657);
  }//anti-B0 decays to K+ K- K- pi+ eta
  if (GenB0Tag::PcheckDecay(genpart, 321, -321, -321, 211, 331)) {
    return -1 * (100000 * m_nPhotos + 1658);
  }//anti-B0 decays to K+ K- K- pi+ eta'
  if (GenB0Tag::PcheckDecay(genpart, 321, -321, -311, 211, -211)) {
    return -1 * (100000 * m_nPhotos + 1659);
  }//anti-B0 decays to K+ K- anti-K0 pi+ pi-
  if (GenB0Tag::PcheckDecay(genpart, 321, -321, -311, 111, 111)) {
    return -1 * (100000 * m_nPhotos + 1660);
  }//anti-B0 decays to K+ K- anti-K0 pi0 pi0
  if (GenB0Tag::PcheckDecay(genpart, 321, -321, -311, 111, 221)) {
    return -1 * (100000 * m_nPhotos + 1661);
  }//anti-B0 decays to K+ K- anti-K0 pi0 eta
  if (GenB0Tag::PcheckDecay(genpart, 321, -321, -311, 111, 331)) {
    return -1 * (100000 * m_nPhotos + 1662);
  }//anti-B0 decays to K+ K- anti-K0 pi0 eta'
  if (GenB0Tag::PcheckDecay(genpart, 311, -311, -321, 211, 111)) {
    return -1 * (100000 * m_nPhotos + 1663);
  }//anti-B0 decays to K0 anti-K0 K- pi+ pi0
  if (GenB0Tag::PcheckDecay(genpart, 311, -311, -321, 211, 221)) {
    return -1 * (100000 * m_nPhotos + 1664);
  }//anti-B0 decays to K0 anti-K0 K- pi+ eta
  if (GenB0Tag::PcheckDecay(genpart, 311, -311, -321, 211, 331)) {
    return -1 * (100000 * m_nPhotos + 1665);
  }//anti-B0 decays to K0 anti-K0 K- pi+ eta'
  if (GenB0Tag::PcheckDecay(genpart, 311, -311, -311, 211, -211)) {
    return -1 * (100000 * m_nPhotos + 1666);
  }//anti-B0 decays to K0 anti-K0 anti-K0 pi+ pi-
  if (GenB0Tag::PcheckDecay(genpart, 311, -311, -311, 111, 111)) {
    return -1 * (100000 * m_nPhotos + 1667);
  }//anti-B0 decays to K0 anti-K0 anti-K0 pi0 pi0
  if (GenB0Tag::PcheckDecay(genpart, 311, -311, -311, 111, 221)) {
    return -1 * (100000 * m_nPhotos + 1668);
  }//anti-B0 decays to K0 anti-K0 anti-K0 pi0 eta
  if (GenB0Tag::PcheckDecay(genpart, 311, -311, -311, 111, 331)) {
    return -1 * (100000 * m_nPhotos + 1669);
  }//anti-B0 decays to K0 anti-K0 anti-K0 pi0 eta'
  if (GenB0Tag::PcheckDecay(genpart, 333, -10321, 211)) {
    return -1 * (100000 * m_nPhotos + 1670);
  }//anti-B0 decays to phi K_0*- pi+
  if (GenB0Tag::PcheckDecay(genpart, 333, -10311, 111)) {
    return -1 * (100000 * m_nPhotos + 1671);
  }//anti-B0 decays to phi anti-K_0*0 pi0
  if (GenB0Tag::PcheckDecay(genpart, 321, -321, -10321, 211)) {
    return -1 * (100000 * m_nPhotos + 1672);
  }//anti-B0 decays to K+ K- K_0*- pi+
  if (GenB0Tag::PcheckDecay(genpart, 321, -321, -10311, 111)) {
    return -1 * (100000 * m_nPhotos + 1673);
  }//anti-B0 decays to K+ K- anti-K_0*0 pi0
  if (GenB0Tag::PcheckDecay(genpart, 321, -321, -10311, 221)) {
    return -1 * (100000 * m_nPhotos + 1674);
  }//anti-B0 decays to K+ K- anti-K_0*0 eta
  if (GenB0Tag::PcheckDecay(genpart, 321, -321, -10311, 331)) {
    return -1 * (100000 * m_nPhotos + 1675);
  }//anti-B0 decays to K+ K- anti-K_0*0 eta'
  if (GenB0Tag::PcheckDecay(genpart, -321, -321, 10321, 211)) {
    return -1 * (100000 * m_nPhotos + 1676);
  }//anti-B0 decays to K- K- K_0*+ pi+
  if (GenB0Tag::PcheckDecay(genpart, -321, -10311, 311, 211)) {
    return -1 * (100000 * m_nPhotos + 1677);
  }//anti-B0 decays to K- anti-K_0*0 K0 pi+
  if (GenB0Tag::PcheckDecay(genpart, -321, 10311, -311, 211)) {
    return -1 * (100000 * m_nPhotos + 1678);
  }//anti-B0 decays to K- K_0*0 anti-K0 pi+
  if (GenB0Tag::PcheckDecay(genpart, 311, -311, -10321, 211)) {
    return -1 * (100000 * m_nPhotos + 1679);
  }//anti-B0 decays to K0 anti-K0 K_0*- pi+
  if (GenB0Tag::PcheckDecay(genpart, 311, -311, -10311, 111)) {
    return -1 * (100000 * m_nPhotos + 1680);
  }//anti-B0 decays to K0 anti-K0 anti-K_0*0 pi0
  if (GenB0Tag::PcheckDecay(genpart, 311, -311, -10311, 221)) {
    return -1 * (100000 * m_nPhotos + 1681);
  }//anti-B0 decays to K0 anti-K0 anti-K_0*0 eta
  if (GenB0Tag::PcheckDecay(genpart, 311, -311, -10311, 331)) {
    return -1 * (100000 * m_nPhotos + 1682);
  }//anti-B0 decays to K0 anti-K0 anti-K_0*0 eta'
  if (GenB0Tag::PcheckDecay(genpart, -311, -311, 10311, 111)) {
    return -1 * (100000 * m_nPhotos + 1683);
  }//anti-B0 decays to anti-K0 anti-K0 K_0*0 pi0
  if (GenB0Tag::PcheckDecay(genpart, -311, -311, 10311, 221)) {
    return -1 * (100000 * m_nPhotos + 1684);
  }//anti-B0 decays to anti-K0 anti-K0 K_0*0 eta
  if (GenB0Tag::PcheckDecay(genpart, -311, -311, 10311, 331)) {
    return -1 * (100000 * m_nPhotos + 1685);
  }//anti-B0 decays to anti-K0 anti-K0 K_0*0 eta'
  if (GenB0Tag::PcheckDecay(genpart, 333, -20323, 211)) {
    return -1 * (100000 * m_nPhotos + 1686);
  }//anti-B0 decays to phi K'_1- pi+
  if (GenB0Tag::PcheckDecay(genpart, 333, -20313, 111)) {
    return -1 * (100000 * m_nPhotos + 1687);
  }//anti-B0 decays to phi anti-K'_10 pi0
  if (GenB0Tag::PcheckDecay(genpart, 333, -20313, 221)) {
    return -1 * (100000 * m_nPhotos + 1688);
  }//anti-B0 decays to phi anti-K'_10 eta
  if (GenB0Tag::PcheckDecay(genpart, 333, -20313, 331)) {
    return -1 * (100000 * m_nPhotos + 1689);
  }//anti-B0 decays to phi anti-K'_10 eta'
  if (GenB0Tag::PcheckDecay(genpart, -321, 321, -20323, 211)) {
    return -1 * (100000 * m_nPhotos + 1690);
  }//anti-B0 decays to K- K+ K'_1- pi+
  if (GenB0Tag::PcheckDecay(genpart, -321, 321, -20313, 111)) {
    return -1 * (100000 * m_nPhotos + 1691);
  }//anti-B0 decays to K- K+ anti-K'_10 pi0
  if (GenB0Tag::PcheckDecay(genpart, -321, 321, -20313, 221)) {
    return -1 * (100000 * m_nPhotos + 1692);
  }//anti-B0 decays to K- K+ anti-K'_10 eta
  if (GenB0Tag::PcheckDecay(genpart, -321, 321, -20313, 331)) {
    return -1 * (100000 * m_nPhotos + 1693);
  }//anti-B0 decays to K- K+ anti-K'_10 eta'
  if (GenB0Tag::PcheckDecay(genpart, -321, -321, 20323, 211)) {
    return -1 * (100000 * m_nPhotos + 1694);
  }//anti-B0 decays to K- K- K'_1+ pi+
  if (GenB0Tag::PcheckDecay(genpart, 311, -311, -20313, 111)) {
    return -1 * (100000 * m_nPhotos + 1695);
  }//anti-B0 decays to K0 anti-K0 anti-K'_10 pi0
  if (GenB0Tag::PcheckDecay(genpart, 311, -311, -20313, 221)) {
    return -1 * (100000 * m_nPhotos + 1696);
  }//anti-B0 decays to K0 anti-K0 anti-K'_10 eta
  if (GenB0Tag::PcheckDecay(genpart, 311, -311, -20313, 331)) {
    return -1 * (100000 * m_nPhotos + 1697);
  }//anti-B0 decays to K0 anti-K0 anti-K'_10 eta'
  if (GenB0Tag::PcheckDecay(genpart, -311, -311, 20313, 111)) {
    return -1 * (100000 * m_nPhotos + 1698);
  }//anti-B0 decays to anti-K0 anti-K0 K'_10 pi0
  if (GenB0Tag::PcheckDecay(genpart, -311, -311, 20313, 221)) {
    return -1 * (100000 * m_nPhotos + 1699);
  }//anti-B0 decays to anti-K0 anti-K0 K'_10 eta
  if (GenB0Tag::PcheckDecay(genpart, -311, -311, 20313, 331)) {
    return -1 * (100000 * m_nPhotos + 1700);
  }//anti-B0 decays to anti-K0 anti-K0 K'_10 eta'
  if (GenB0Tag::PcheckDecay(genpart, 333, 333, -311)) {
    return -1 * (100000 * m_nPhotos + 1701);
  }//anti-B0 decays to phi phi anti-K0
  if (GenB0Tag::PcheckDecay(genpart, 333, 333, 111)) {
    return -1 * (100000 * m_nPhotos + 1702);
  }//anti-B0 decays to phi phi pi0
  if (GenB0Tag::PcheckDecay(genpart, 333, 321, -321, -311)) {
    return -1 * (100000 * m_nPhotos + 1703);
  }//anti-B0 decays to phi K+ K- anti-K0
  if (GenB0Tag::PcheckDecay(genpart, 333, 311, -311, -311)) {
    return -1 * (100000 * m_nPhotos + 1704);
  }//anti-B0 decays to phi K0 anti-K0 anti-K0
  if (GenB0Tag::PcheckDecay(genpart, 333, 321, -321, 111)) {
    return -1 * (100000 * m_nPhotos + 1705);
  }//anti-B0 decays to phi K+ K- pi0
  if (GenB0Tag::PcheckDecay(genpart, 333, 311, -311, 111)) {
    return -1 * (100000 * m_nPhotos + 1706);
  }//anti-B0 decays to phi K0 anti-K0 pi0
  if (GenB0Tag::PcheckDecay(genpart, 333, -311, 321, -211)) {
    return -1 * (100000 * m_nPhotos + 1707);
  }//anti-B0 decays to phi anti-K0 K+ pi-
  if (GenB0Tag::PcheckDecay(genpart, 333, 311, -321, 211)) {
    return -1 * (100000 * m_nPhotos + 1708);
  }//anti-B0 decays to phi K0 K- pi+
  if (GenB0Tag::PcheckDecay(genpart, 321, -321, 321, -321, -311)) {
    return -1 * (100000 * m_nPhotos + 1709);
  }//anti-B0 decays to K+ K- K+ K- anti-K0
  if (GenB0Tag::PcheckDecay(genpart, 311, -311, 321, -321, -311)) {
    return -1 * (100000 * m_nPhotos + 1710);
  }//anti-B0 decays to K0 anti-K0 K+ K- anti-K0
  if (GenB0Tag::PcheckDecay(genpart, 321, -321, 321, -321, 111)) {
    return -1 * (100000 * m_nPhotos + 1711);
  }//anti-B0 decays to K+ K- K+ K- pi0
  if (GenB0Tag::PcheckDecay(genpart, 321, -321, 311, -311, 111)) {
    return -1 * (100000 * m_nPhotos + 1712);
  }//anti-B0 decays to K+ K- K0 anti-K0 pi0
  if (GenB0Tag::PcheckDecay(genpart, 321, -321, 321, -311, -211)) {
    return -1 * (100000 * m_nPhotos + 1713);
  }//anti-B0 decays to K+ K- K+ anti-K0 pi-
  if (GenB0Tag::PcheckDecay(genpart, 321, -321, 311, -321, 211)) {
    return -1 * (100000 * m_nPhotos + 1714);
  }//anti-B0 decays to K+ K- K0 K- pi+
  if (GenB0Tag::PcheckDecay(genpart, 311, -311, 311, -311, 111)) {
    return -1 * (100000 * m_nPhotos + 1715);
  }//anti-B0 decays to K0 anti-K0 K0 anti-K0 pi0
  if (GenB0Tag::PcheckDecay(genpart, 311, -311, -311, 321, -211)) {
    return -1 * (100000 * m_nPhotos + 1716);
  }//anti-B0 decays to K0 anti-K0 anti-K0 K+ pi-
  if (GenB0Tag::PcheckDecay(genpart, 311, -311, 311, -321, 211)) {
    return -1 * (100000 * m_nPhotos + 1717);
  }//anti-B0 decays to K0 anti-K0 K0 K- pi+
  if (GenB0Tag::PcheckDecay(genpart, 333, 333, -313)) {
    return -1 * (100000 * m_nPhotos + 1718);
  }//anti-B0 decays to phi phi anti-K*0
  if (GenB0Tag::PcheckDecay(genpart, 333, 321, -321, -313)) {
    return -1 * (100000 * m_nPhotos + 1719);
  }//anti-B0 decays to phi K+ K- anti-K*0
  if (GenB0Tag::PcheckDecay(genpart, 333, 311, -311, -313)) {
    return -1 * (100000 * m_nPhotos + 1720);
  }//anti-B0 decays to phi K0 anti-K0 anti-K*0
  if (GenB0Tag::PcheckDecay(genpart, 333, -311, -321, 323)) {
    return -1 * (100000 * m_nPhotos + 1721);
  }//anti-B0 decays to phi anti-K0 K- K*+
  if (GenB0Tag::PcheckDecay(genpart, 333, -311, -311, 313)) {
    return -1 * (100000 * m_nPhotos + 1722);
  }//anti-B0 decays to phi anti-K0 anti-K0 K*0
  if (GenB0Tag::PcheckDecay(genpart, 333, 321, -321, -321, 211)) {
    return -1 * (100000 * m_nPhotos + 1723);
  }//anti-B0 decays to phi K+ K- K- pi+
  if (GenB0Tag::PcheckDecay(genpart, 333, 321, -321, -311, 111)) {
    return -1 * (100000 * m_nPhotos + 1724);
  }//anti-B0 decays to phi K+ K- anti-K0 pi0
  if (GenB0Tag::PcheckDecay(genpart, 333, 311, -311, -321, 211)) {
    return -1 * (100000 * m_nPhotos + 1725);
  }//anti-B0 decays to phi K0 anti-K0 K- pi+
  if (GenB0Tag::PcheckDecay(genpart, 333, 311, -311, -311, 111)) {
    return -1 * (100000 * m_nPhotos + 1726);
  }//anti-B0 decays to phi K0 anti-K0 anti-K0 pi0
  if (GenB0Tag::PcheckDecay(genpart, 20113, 20113)) {
    return -1 * (100000 * m_nPhotos + 1727);
  }//anti-B0 decays to a_10 a_10
  if (GenB0Tag::PcheckDecay(genpart, 20213, -20213)) {
    return -1 * (100000 * m_nPhotos + 1728);
  }//anti-B0 decays to a_1+ a_1-
  if (GenB0Tag::PcheckDecay(genpart, 443, 310)) {
    return -1 * (100000 * m_nPhotos + 1729);
  }//anti-B0 decays to J/psi K_S0
  if (GenB0Tag::PcheckDecay(genpart, 443, 130)) {
    return -1 * (100000 * m_nPhotos + 1730);
  }//anti-B0 decays to J/psi K_L0
  if (GenB0Tag::PcheckDecay(genpart, 443, 313)) {
    return -1 * (100000 * m_nPhotos + 1731);
  }//anti-B0 decays to J/psi K*S
  if (GenB0Tag::PcheckDecay(genpart, 443, 313)) {
    return -1 * (100000 * m_nPhotos + 1732);
  }//anti-B0 decays to J/psi K*L
  if (GenB0Tag::PcheckDecay(genpart, 443, -313)) {
    return -1 * (100000 * m_nPhotos + 1733);
  }//anti-B0 decays to J/psi anti-K*0T
  if (GenB0Tag::PcheckDecay(genpart, 443, 111)) {
    return -1 * (100000 * m_nPhotos + 1734);
  }//anti-B0 decays to J/psi pi0
  if (GenB0Tag::PcheckDecay(genpart, 443, 113)) {
    return -1 * (100000 * m_nPhotos + 1735);
  }//anti-B0 decays to J/psi rho0
  if (GenB0Tag::PcheckDecay(genpart, 443, 223)) {
    return -1 * (100000 * m_nPhotos + 1736);
  }//anti-B0 decays to J/psi omega
  if (GenB0Tag::PcheckDecay(genpart, 443, -311, 111)) {
    return -1 * (100000 * m_nPhotos + 1737);
  }//anti-B0 decays to J/psi anti-K0 pi0
  if (GenB0Tag::PcheckDecay(genpart, 443, -311, 211, -211)) {
    return -1 * (100000 * m_nPhotos + 1738);
  }//anti-B0 decays to J/psi anti-K0 pi+ pi-
  if (GenB0Tag::PcheckDecay(genpart, 443, -10313)) {
    return -1 * (100000 * m_nPhotos + 1739);
  }//anti-B0 decays to J/psi anti-K_10
  if (GenB0Tag::PcheckDecay(genpart, 443, -20313)) {
    return -1 * (100000 * m_nPhotos + 1740);
  }//anti-B0 decays to J/psi anti-K'_10
  if (GenB0Tag::PcheckDecay(genpart, 443, -315)) {
    return -1 * (100000 * m_nPhotos + 1741);
  }//anti-B0 decays to J/psi anti-K_2*0
  if (GenB0Tag::PcheckDecay(genpart, 443, -311, 333)) {
    return -1 * (100000 * m_nPhotos + 1742);
  }//anti-B0 decays to J/psi anti-K0 phi
  if (GenB0Tag::PcheckDecay(genpart, 443, -321, 211)) {
    return -1 * (100000 * m_nPhotos + 1743);
  }//anti-B0 decays to J/psi K- pi+
  if (GenB0Tag::PcheckDecay(genpart, 443, 221, -311)) {
    return -1 * (100000 * m_nPhotos + 1744);
  }//anti-B0 decays to J/psi eta anti-K0
  if (GenB0Tag::PcheckDecay(genpart, 443, 223, -311)) {
    return -1 * (100000 * m_nPhotos + 1745);
  }//anti-B0 decays to J/psi omega anti-K0
  if (GenB0Tag::PcheckDecay(genpart, 443, 221)) {
    return -1 * (100000 * m_nPhotos + 1746);
  }//anti-B0 decays to J/psi eta
  if (GenB0Tag::PcheckDecay(genpart, 443, 211, -211)) {
    return -1 * (100000 * m_nPhotos + 1747);
  }//anti-B0 decays to J/psi pi+ pi-
  if (GenB0Tag::PcheckDecay(genpart, 443, 225)) {
    return -1 * (100000 * m_nPhotos + 1748);
  }//anti-B0 decays to J/psi f_2
  if (GenB0Tag::PcheckDecay(genpart, 443, 321, -321)) {
    return -1 * (100000 * m_nPhotos + 1749);
  }//anti-B0 decays to J/psi K+ K-
  if (GenB0Tag::PcheckDecay(genpart, 443, 331)) {
    return -1 * (100000 * m_nPhotos + 1750);
  }//anti-B0 decays to J/psi eta'
  if (GenB0Tag::PcheckDecay(genpart, 443, -311, 321, -321)) {
    return -1 * (100000 * m_nPhotos + 1751);
  }//anti-B0 decays to J/psi anti-K0 K+ K-
  if (GenB0Tag::PcheckDecay(genpart, 443, -311, 113)) {
    return -1 * (100000 * m_nPhotos + 1752);
  }//anti-B0 decays to J/psi anti-K0 rho0
  if (GenB0Tag::PcheckDecay(genpart, 443, -323, 211)) {
    return -1 * (100000 * m_nPhotos + 1753);
  }//anti-B0 decays to J/psi K*- pi+
  if (GenB0Tag::PcheckDecay(genpart, 443, 211, -211, 211, -211)) {
    return -1 * (100000 * m_nPhotos + 1754);
  }//anti-B0 decays to J/psi pi+ pi- pi+ pi-
  if (GenB0Tag::PcheckDecay(genpart, 443, 20223)) {
    return -1 * (100000 * m_nPhotos + 1755);
  }//anti-B0 decays to J/psi f_1
  if (GenB0Tag::PcheckDecay(genpart, 443, -313, 211, -211)) {
    return -1 * (100000 * m_nPhotos + 1756);
  }//anti-B0 decays to J/psi anti-K*0 pi+ pi-
  if (GenB0Tag::PcheckDecay(genpart, 100443, 310)) {
    return -1 * (100000 * m_nPhotos + 1757);
  }//anti-B0 decays to psi(2S) K_S0
  if (GenB0Tag::PcheckDecay(genpart, 100443, 130)) {
    return -1 * (100000 * m_nPhotos + 1758);
  }//anti-B0 decays to psi(2S) K_L0
  if (GenB0Tag::PcheckDecay(genpart, 100443, 313)) {
    return -1 * (100000 * m_nPhotos + 1759);
  }//anti-B0 decays to psi(2S) K*S
  if (GenB0Tag::PcheckDecay(genpart, 100443, 313)) {
    return -1 * (100000 * m_nPhotos + 1760);
  }//anti-B0 decays to psi(2S) K*L
  if (GenB0Tag::PcheckDecay(genpart, 100443, -313)) {
    return -1 * (100000 * m_nPhotos + 1761);
  }//anti-B0 decays to psi(2S) anti-K*0T
  if (GenB0Tag::PcheckDecay(genpart, 100443, -321, 211)) {
    return -1 * (100000 * m_nPhotos + 1762);
  }//anti-B0 decays to psi(2S) K- pi+
  if (GenB0Tag::PcheckDecay(genpart, 100443, -311, 111)) {
    return -1 * (100000 * m_nPhotos + 1763);
  }//anti-B0 decays to psi(2S) anti-K0 pi0
  if (GenB0Tag::PcheckDecay(genpart, 100443, -311, 211, -211)) {
    return -1 * (100000 * m_nPhotos + 1764);
  }//anti-B0 decays to psi(2S) anti-K0 pi+ pi-
  if (GenB0Tag::PcheckDecay(genpart, 100443, -311, 111, 111)) {
    return -1 * (100000 * m_nPhotos + 1765);
  }//anti-B0 decays to psi(2S) anti-K0 pi0 pi0
  if (GenB0Tag::PcheckDecay(genpart, 100443, -321, 211, 111)) {
    return -1 * (100000 * m_nPhotos + 1766);
  }//anti-B0 decays to psi(2S) K- pi+ pi0
  if (GenB0Tag::PcheckDecay(genpart, 100443, -10313)) {
    return -1 * (100000 * m_nPhotos + 1767);
  }//anti-B0 decays to psi(2S) anti-K_10
  if (GenB0Tag::PcheckDecay(genpart, 100443, 211, -211)) {
    return -1 * (100000 * m_nPhotos + 1768);
  }//anti-B0 decays to psi(2S) pi+ pi-
  if (GenB0Tag::PcheckDecay(genpart, 100443, 111)) {
    return -1 * (100000 * m_nPhotos + 1769);
  }//anti-B0 decays to psi(2S) pi0
  if (GenB0Tag::PcheckDecay(genpart, 441, 310)) {
    return -1 * (100000 * m_nPhotos + 1770);
  }//anti-B0 decays to eta_c K_S0
  if (GenB0Tag::PcheckDecay(genpart, 441, 130)) {
    return -1 * (100000 * m_nPhotos + 1771);
  }//anti-B0 decays to eta_c K_L0
  if (GenB0Tag::PcheckDecay(genpart, 313, 441)) {
    return -1 * (100000 * m_nPhotos + 1772);
  }//anti-B0 decays to K*S eta_c
  if (GenB0Tag::PcheckDecay(genpart, 313, 441)) {
    return -1 * (100000 * m_nPhotos + 1773);
  }//anti-B0 decays to K*L eta_c
  if (GenB0Tag::PcheckDecay(genpart, -313, 441)) {
    return -1 * (100000 * m_nPhotos + 1774);
  }//anti-B0 decays to anti-K*0T eta_c
  if (GenB0Tag::PcheckDecay(genpart, 441, -321, 211)) {
    return -1 * (100000 * m_nPhotos + 1775);
  }//anti-B0 decays to eta_c K- pi+
  if (GenB0Tag::PcheckDecay(genpart, 441, -311, 111)) {
    return -1 * (100000 * m_nPhotos + 1776);
  }//anti-B0 decays to eta_c anti-K0 pi0
  if (GenB0Tag::PcheckDecay(genpart, 441, -311, 211, -211)) {
    return -1 * (100000 * m_nPhotos + 1777);
  }//anti-B0 decays to eta_c anti-K0 pi+ pi-
  if (GenB0Tag::PcheckDecay(genpart, 441, -311, 111, 111)) {
    return -1 * (100000 * m_nPhotos + 1778);
  }//anti-B0 decays to eta_c anti-K0 pi0 pi0
  if (GenB0Tag::PcheckDecay(genpart, 441, -321, 211, 111)) {
    return -1 * (100000 * m_nPhotos + 1779);
  }//anti-B0 decays to eta_c K- pi+ pi0
  if (GenB0Tag::PcheckDecay(genpart, 100441, 310)) {
    return -1 * (100000 * m_nPhotos + 1780);
  }//anti-B0 decays to eta_c(2S) K_S0
  if (GenB0Tag::PcheckDecay(genpart, 100441, 130)) {
    return -1 * (100000 * m_nPhotos + 1781);
  }//anti-B0 decays to eta_c(2S) K_L0
  if (GenB0Tag::PcheckDecay(genpart, 313, 100441)) {
    return -1 * (100000 * m_nPhotos + 1782);
  }//anti-B0 decays to K*S eta_c(2S)
  if (GenB0Tag::PcheckDecay(genpart, 313, 100441)) {
    return -1 * (100000 * m_nPhotos + 1783);
  }//anti-B0 decays to K*L eta_c(2S)
  if (GenB0Tag::PcheckDecay(genpart, -313, 100441)) {
    return -1 * (100000 * m_nPhotos + 1784);
  }//anti-B0 decays to anti-K*0T eta_c(2S)
  if (GenB0Tag::PcheckDecay(genpart, 100441, -321, 211)) {
    return -1 * (100000 * m_nPhotos + 1785);
  }//anti-B0 decays to eta_c(2S) K- pi+
  if (GenB0Tag::PcheckDecay(genpart, 100441, -311, 111)) {
    return -1 * (100000 * m_nPhotos + 1786);
  }//anti-B0 decays to eta_c(2S) anti-K0 pi0
  if (GenB0Tag::PcheckDecay(genpart, 100441, -311, 211, -211)) {
    return -1 * (100000 * m_nPhotos + 1787);
  }//anti-B0 decays to eta_c(2S) anti-K0 pi+ pi-
  if (GenB0Tag::PcheckDecay(genpart, 100441, -311, 111, 111)) {
    return -1 * (100000 * m_nPhotos + 1788);
  }//anti-B0 decays to eta_c(2S) anti-K0 pi0 pi0
  if (GenB0Tag::PcheckDecay(genpart, 100441, -321, 211, 111)) {
    return -1 * (100000 * m_nPhotos + 1789);
  }//anti-B0 decays to eta_c(2S) K- pi+ pi0
  if (GenB0Tag::PcheckDecay(genpart, 10441, 310)) {
    return -1 * (100000 * m_nPhotos + 1790);
  }//anti-B0 decays to chi_c0 K_S0
  if (GenB0Tag::PcheckDecay(genpart, 10441, 130)) {
    return -1 * (100000 * m_nPhotos + 1791);
  }//anti-B0 decays to chi_c0 K_L0
  if (GenB0Tag::PcheckDecay(genpart, 313, 10441)) {
    return -1 * (100000 * m_nPhotos + 1792);
  }//anti-B0 decays to K*S chi_c0
  if (GenB0Tag::PcheckDecay(genpart, 313, 10441)) {
    return -1 * (100000 * m_nPhotos + 1793);
  }//anti-B0 decays to K*L chi_c0
  if (GenB0Tag::PcheckDecay(genpart, -313, 10441)) {
    return -1 * (100000 * m_nPhotos + 1794);
  }//anti-B0 decays to anti-K*0T chi_c0
  if (GenB0Tag::PcheckDecay(genpart, 10441, -321, 211)) {
    return -1 * (100000 * m_nPhotos + 1795);
  }//anti-B0 decays to chi_c0 K- pi+
  if (GenB0Tag::PcheckDecay(genpart, 10441, -311, 111)) {
    return -1 * (100000 * m_nPhotos + 1796);
  }//anti-B0 decays to chi_c0 anti-K0 pi0
  if (GenB0Tag::PcheckDecay(genpart, 10441, -311, 211, -211)) {
    return -1 * (100000 * m_nPhotos + 1797);
  }//anti-B0 decays to chi_c0 anti-K0 pi+ pi-
  if (GenB0Tag::PcheckDecay(genpart, 10441, -311, 111, 111)) {
    return -1 * (100000 * m_nPhotos + 1798);
  }//anti-B0 decays to chi_c0 anti-K0 pi0 pi0
  if (GenB0Tag::PcheckDecay(genpart, 10441, -321, 211, 111)) {
    return -1 * (100000 * m_nPhotos + 1799);
  }//anti-B0 decays to chi_c0 K- pi+ pi0
  if (GenB0Tag::PcheckDecay(genpart, 20443, 310)) {
    return -1 * (100000 * m_nPhotos + 1800);
  }//anti-B0 decays to chi_c1 K_S0
  if (GenB0Tag::PcheckDecay(genpart, 20443, 130)) {
    return -1 * (100000 * m_nPhotos + 1801);
  }//anti-B0 decays to chi_c1 K_L0
  if (GenB0Tag::PcheckDecay(genpart, 313, 20443)) {
    return -1 * (100000 * m_nPhotos + 1802);
  }//anti-B0 decays to K*S chi_c1
  if (GenB0Tag::PcheckDecay(genpart, 313, 20443)) {
    return -1 * (100000 * m_nPhotos + 1803);
  }//anti-B0 decays to K*L chi_c1
  if (GenB0Tag::PcheckDecay(genpart, 20443, -313)) {
    return -1 * (100000 * m_nPhotos + 1804);
  }//anti-B0 decays to chi_c1 anti-K*0T
  if (GenB0Tag::PcheckDecay(genpart, 20443, -321, 211)) {
    return -1 * (100000 * m_nPhotos + 1805);
  }//anti-B0 decays to chi_c1 K- pi+
  if (GenB0Tag::PcheckDecay(genpart, 20443, -311, 111)) {
    return -1 * (100000 * m_nPhotos + 1806);
  }//anti-B0 decays to chi_c1 anti-K0 pi0
  if (GenB0Tag::PcheckDecay(genpart, 20443, -311, 211, -211)) {
    return -1 * (100000 * m_nPhotos + 1807);
  }//anti-B0 decays to chi_c1 anti-K0 pi+ pi-
  if (GenB0Tag::PcheckDecay(genpart, 20443, -311, 111, 111)) {
    return -1 * (100000 * m_nPhotos + 1808);
  }//anti-B0 decays to chi_c1 anti-K0 pi0 pi0
  if (GenB0Tag::PcheckDecay(genpart, 20443, -321, 211, 111)) {
    return -1 * (100000 * m_nPhotos + 1809);
  }//anti-B0 decays to chi_c1 K- pi+ pi0
  if (GenB0Tag::PcheckDecay(genpart, 20443, 111)) {
    return -1 * (100000 * m_nPhotos + 1810);
  }//anti-B0 decays to chi_c1 pi0
  if (GenB0Tag::PcheckDecay(genpart, 445, 310)) {
    return -1 * (100000 * m_nPhotos + 1811);
  }//anti-B0 decays to chi_c2 K_S0
  if (GenB0Tag::PcheckDecay(genpart, 445, 130)) {
    return -1 * (100000 * m_nPhotos + 1812);
  }//anti-B0 decays to chi_c2 K_L0
  if (GenB0Tag::PcheckDecay(genpart, 445, -313)) {
    return -1 * (100000 * m_nPhotos + 1813);
  }//anti-B0 decays to chi_c2 anti-K*0
  if (GenB0Tag::PcheckDecay(genpart, 445, -321, 211)) {
    return -1 * (100000 * m_nPhotos + 1814);
  }//anti-B0 decays to chi_c2 K- pi+
  if (GenB0Tag::PcheckDecay(genpart, 445, -311, 111)) {
    return -1 * (100000 * m_nPhotos + 1815);
  }//anti-B0 decays to chi_c2 anti-K0 pi0
  if (GenB0Tag::PcheckDecay(genpart, 445, -311, 211, -211)) {
    return -1 * (100000 * m_nPhotos + 1816);
  }//anti-B0 decays to chi_c2 anti-K0 pi+ pi-
  if (GenB0Tag::PcheckDecay(genpart, 445, -321, 211, 111)) {
    return -1 * (100000 * m_nPhotos + 1817);
  }//anti-B0 decays to chi_c2 K- pi+ pi0
  if (GenB0Tag::PcheckDecay(genpart, 445, -311, 111, 111)) {
    return -1 * (100000 * m_nPhotos + 1818);
  }//anti-B0 decays to chi_c2 anti-K0 pi0 pi0
  if (GenB0Tag::PcheckDecay(genpart, 30443, 310)) {
    return -1 * (100000 * m_nPhotos + 1819);
  }//anti-B0 decays to psi(3770) K_S0
  if (GenB0Tag::PcheckDecay(genpart, 30443, 130)) {
    return -1 * (100000 * m_nPhotos + 1820);
  }//anti-B0 decays to psi(3770) K_L0
  if (GenB0Tag::PcheckDecay(genpart, 30443, 313)) {
    return -1 * (100000 * m_nPhotos + 1821);
  }//anti-B0 decays to psi(3770) K*S
  if (GenB0Tag::PcheckDecay(genpart, 30443, 313)) {
    return -1 * (100000 * m_nPhotos + 1822);
  }//anti-B0 decays to psi(3770) K*L
  if (GenB0Tag::PcheckDecay(genpart, 30443, -313)) {
    return -1 * (100000 * m_nPhotos + 1823);
  }//anti-B0 decays to psi(3770) anti-K*0T
  if (GenB0Tag::PcheckDecay(genpart, 30443, -311, 111)) {
    return -1 * (100000 * m_nPhotos + 1824);
  }//anti-B0 decays to psi(3770) anti-K0 pi0
  if (GenB0Tag::PcheckDecay(genpart, 30443, -311, 211, -211)) {
    return -1 * (100000 * m_nPhotos + 1825);
  }//anti-B0 decays to psi(3770) anti-K0 pi+ pi-
  if (GenB0Tag::PcheckDecay(genpart, 30443, -311, 111, 111)) {
    return -1 * (100000 * m_nPhotos + 1826);
  }//anti-B0 decays to psi(3770) anti-K0 pi0 pi0
  if (GenB0Tag::PcheckDecay(genpart, 30443, -321, 211, 111)) {
    return -1 * (100000 * m_nPhotos + 1827);
  }//anti-B0 decays to psi(3770) K- pi+ pi0
  if (GenB0Tag::PcheckDecay(genpart, 30443, -10313)) {
    return -1 * (100000 * m_nPhotos + 1828);
  }//anti-B0 decays to psi(3770) anti-K_10
  if (GenB0Tag::PcheckDecay(genpart, -411, 411)) {
    return -1 * (100000 * m_nPhotos + 1829);
  }//anti-B0 decays to D- D+
  if (GenB0Tag::PcheckDecay(genpart, 413, -411)) {
    return -1 * (100000 * m_nPhotos + 1830);
  }//anti-B0 decays to D*+ D-
  if (GenB0Tag::PcheckDecay(genpart, -413, 411)) {
    return -1 * (100000 * m_nPhotos + 1831);
  }//anti-B0 decays to D*- D+
  if (GenB0Tag::PcheckDecay(genpart, 413, -413)) {
    return -1 * (100000 * m_nPhotos + 1832);
  }//anti-B0 decays to D*+ D*-
  if (GenB0Tag::PcheckDecay(genpart, 411, -431)) {
    return -1 * (100000 * m_nPhotos + 1833);
  }//anti-B0 decays to D+ D_s-
  if (GenB0Tag::PcheckDecay(genpart, 413, -431)) {
    return -1 * (100000 * m_nPhotos + 1834);
  }//anti-B0 decays to D*+ D_s-
  if (GenB0Tag::PcheckDecay(genpart, -433, 411)) {
    return -1 * (100000 * m_nPhotos + 1835);
  }//anti-B0 decays to D_s*- D+
  if (GenB0Tag::PcheckDecay(genpart, 413, -433)) {
    return -1 * (100000 * m_nPhotos + 1836);
  }//anti-B0 decays to D*+ D_s*-
  if (GenB0Tag::PcheckDecay(genpart, 20413, -431)) {
    return -1 * (100000 * m_nPhotos + 1837);
  }//anti-B0 decays to D'_1+ D_s-
  if (GenB0Tag::PcheckDecay(genpart, 20413, -433)) {
    return -1 * (100000 * m_nPhotos + 1838);
  }//anti-B0 decays to D'_1+ D_s*-
  if (GenB0Tag::PcheckDecay(genpart, 10413, -431)) {
    return -1 * (100000 * m_nPhotos + 1839);
  }//anti-B0 decays to D_1+ D_s-
  if (GenB0Tag::PcheckDecay(genpart, 10413, -433)) {
    return -1 * (100000 * m_nPhotos + 1840);
  }//anti-B0 decays to D_1+ D_s*-
  if (GenB0Tag::PcheckDecay(genpart, 415, -431)) {
    return -1 * (100000 * m_nPhotos + 1841);
  }//anti-B0 decays to D_2*+ D_s-
  if (GenB0Tag::PcheckDecay(genpart, 415, -433)) {
    return -1 * (100000 * m_nPhotos + 1842);
  }//anti-B0 decays to D_2*+ D_s*-
  if (GenB0Tag::PcheckDecay(genpart, -431, 411, 111)) {
    return -1 * (100000 * m_nPhotos + 1843);
  }//anti-B0 decays to D_s- D+ pi0
  if (GenB0Tag::PcheckDecay(genpart, -431, 421, 211)) {
    return -1 * (100000 * m_nPhotos + 1844);
  }//anti-B0 decays to D_s- D0 pi+
  if (GenB0Tag::PcheckDecay(genpart, -433, 411, 111)) {
    return -1 * (100000 * m_nPhotos + 1845);
  }//anti-B0 decays to D_s*- D+ pi0
  if (GenB0Tag::PcheckDecay(genpart, -433, 421, 211)) {
    return -1 * (100000 * m_nPhotos + 1846);
  }//anti-B0 decays to D_s*- D0 pi+
  if (GenB0Tag::PcheckDecay(genpart, -431, 411, 211, -211)) {
    return -1 * (100000 * m_nPhotos + 1847);
  }//anti-B0 decays to D_s- D+ pi+ pi-
  if (GenB0Tag::PcheckDecay(genpart, -431, 411, 111, 111)) {
    return -1 * (100000 * m_nPhotos + 1848);
  }//anti-B0 decays to D_s- D+ pi0 pi0
  if (GenB0Tag::PcheckDecay(genpart, -431, 421, 211, 111)) {
    return -1 * (100000 * m_nPhotos + 1849);
  }//anti-B0 decays to D_s- D0 pi+ pi0
  if (GenB0Tag::PcheckDecay(genpart, -433, 411, 211, -211)) {
    return -1 * (100000 * m_nPhotos + 1850);
  }//anti-B0 decays to D_s*- D+ pi+ pi-
  if (GenB0Tag::PcheckDecay(genpart, -433, 411, 111, 111)) {
    return -1 * (100000 * m_nPhotos + 1851);
  }//anti-B0 decays to D_s*- D+ pi0 pi0
  if (GenB0Tag::PcheckDecay(genpart, -433, 421, 211, 111)) {
    return -1 * (100000 * m_nPhotos + 1852);
  }//anti-B0 decays to D_s*- D0 pi+ pi0
  if (GenB0Tag::PcheckDecay(genpart, 411, -421, -321)) {
    return -1 * (100000 * m_nPhotos + 1853);
  }//anti-B0 decays to D+ anti-D0 K-
  if (GenB0Tag::PcheckDecay(genpart, 411, -423, -321)) {
    return -1 * (100000 * m_nPhotos + 1854);
  }//anti-B0 decays to D+ anti-D*0 K-
  if (GenB0Tag::PcheckDecay(genpart, 413, -421, -321)) {
    return -1 * (100000 * m_nPhotos + 1855);
  }//anti-B0 decays to D*+ anti-D0 K-
  if (GenB0Tag::PcheckDecay(genpart, 413, -423, -321)) {
    return -1 * (100000 * m_nPhotos + 1856);
  }//anti-B0 decays to D*+ anti-D*0 K-
  if (GenB0Tag::PcheckDecay(genpart, 411, -411, -311)) {
    return -1 * (100000 * m_nPhotos + 1857);
  }//anti-B0 decays to D+ D- anti-K0
  if (GenB0Tag::PcheckDecay(genpart, 413, -411, -311)) {
    return -1 * (100000 * m_nPhotos + 1858);
  }//anti-B0 decays to D*+ D- anti-K0
  if (GenB0Tag::PcheckDecay(genpart, 411, -413, -311)) {
    return -1 * (100000 * m_nPhotos + 1859);
  }//anti-B0 decays to D+ D*- anti-K0
  if (GenB0Tag::PcheckDecay(genpart, 413, -413, -311)) {
    return -1 * (100000 * m_nPhotos + 1860);
  }//anti-B0 decays to D*+ D*- anti-K0
  if (GenB0Tag::PcheckDecay(genpart, 421, -421, -311)) {
    return -1 * (100000 * m_nPhotos + 1861);
  }//anti-B0 decays to D0 anti-D0 anti-K0
  if (GenB0Tag::PcheckDecay(genpart, 423, -421, -311)) {
    return -1 * (100000 * m_nPhotos + 1862);
  }//anti-B0 decays to D*0 anti-D0 anti-K0
  if (GenB0Tag::PcheckDecay(genpart, 421, -423, -311)) {
    return -1 * (100000 * m_nPhotos + 1863);
  }//anti-B0 decays to D0 anti-D*0 anti-K0
  if (GenB0Tag::PcheckDecay(genpart, 423, -423, -311)) {
    return -1 * (100000 * m_nPhotos + 1864);
  }//anti-B0 decays to D*0 anti-D*0 anti-K0
  if (GenB0Tag::PcheckDecay(genpart, 411, -421, -323)) {
    return -1 * (100000 * m_nPhotos + 1865);
  }//anti-B0 decays to D+ anti-D0 K*-
  if (GenB0Tag::PcheckDecay(genpart, 413, -421, -323)) {
    return -1 * (100000 * m_nPhotos + 1866);
  }//anti-B0 decays to D*+ anti-D0 K*-
  if (GenB0Tag::PcheckDecay(genpart, 411, -423, -323)) {
    return -1 * (100000 * m_nPhotos + 1867);
  }//anti-B0 decays to D+ anti-D*0 K*-
  if (GenB0Tag::PcheckDecay(genpart, 413, -423, -323)) {
    return -1 * (100000 * m_nPhotos + 1868);
  }//anti-B0 decays to D*+ anti-D*0 K*-
  if (GenB0Tag::PcheckDecay(genpart, 411, -411, -313)) {
    return -1 * (100000 * m_nPhotos + 1869);
  }//anti-B0 decays to D+ D- anti-K*0
  if (GenB0Tag::PcheckDecay(genpart, 413, -411, -313)) {
    return -1 * (100000 * m_nPhotos + 1870);
  }//anti-B0 decays to D*+ D- anti-K*0
  if (GenB0Tag::PcheckDecay(genpart, 411, -413, -313)) {
    return -1 * (100000 * m_nPhotos + 1871);
  }//anti-B0 decays to D+ D*- anti-K*0
  if (GenB0Tag::PcheckDecay(genpart, 413, -413, -313)) {
    return -1 * (100000 * m_nPhotos + 1872);
  }//anti-B0 decays to D*+ D*- anti-K*0
  if (GenB0Tag::PcheckDecay(genpart, -421, 421, -313)) {
    return -1 * (100000 * m_nPhotos + 1873);
  }//anti-B0 decays to anti-D0 D0 anti-K*0
  if (GenB0Tag::PcheckDecay(genpart, -421, 423, -313)) {
    return -1 * (100000 * m_nPhotos + 1874);
  }//anti-B0 decays to anti-D0 D*0 anti-K*0
  if (GenB0Tag::PcheckDecay(genpart, -423, 421, -313)) {
    return -1 * (100000 * m_nPhotos + 1875);
  }//anti-B0 decays to anti-D*0 D0 anti-K*0
  if (GenB0Tag::PcheckDecay(genpart, -423, 423, -313)) {
    return -1 * (100000 * m_nPhotos + 1876);
  }//anti-B0 decays to anti-D*0 D*0 anti-K*0
  if (GenB0Tag::PcheckDecay(genpart, 413, -211)) {
    return -1 * (100000 * m_nPhotos + 1877);
  }//anti-B0 decays to D*+ pi-
  if (GenB0Tag::PcheckDecay(genpart, 411, -211)) {
    return -1 * (100000 * m_nPhotos + 1878);
  }//anti-B0 decays to D+ pi-
  if (GenB0Tag::PcheckDecay(genpart, -213, 411)) {
    return -1 * (100000 * m_nPhotos + 1879);
  }//anti-B0 decays to rho- D+
  if (GenB0Tag::PcheckDecay(genpart, -213, 413)) {
    return -1 * (100000 * m_nPhotos + 1880);
  }//anti-B0 decays to rho- D*+
  if (GenB0Tag::PcheckDecay(genpart, 411, -211, 111)) {
    return -1 * (100000 * m_nPhotos + 1881);
  }//anti-B0 decays to D+ pi- pi0
  if (GenB0Tag::PcheckDecay(genpart, 413, -211, 111)) {
    return -1 * (100000 * m_nPhotos + 1882);
  }//anti-B0 decays to D*+ pi- pi0
  if (GenB0Tag::PcheckDecay(genpart, 421, 211, -211)) {
    return -1 * (100000 * m_nPhotos + 1883);
  }//anti-B0 decays to D0 pi+ pi-
  if (GenB0Tag::PcheckDecay(genpart, 423, 211, -211)) {
    return -1 * (100000 * m_nPhotos + 1884);
  }//anti-B0 decays to D*0 pi+ pi-
  if (GenB0Tag::PcheckDecay(genpart, 423, 111, 111)) {
    return -1 * (100000 * m_nPhotos + 1885);
  }//anti-B0 decays to D*0 pi0 pi0
  if (GenB0Tag::PcheckDecay(genpart, -20213, 411)) {
    return -1 * (100000 * m_nPhotos + 1886);
  }//anti-B0 decays to a_1- D+
  if (GenB0Tag::PcheckDecay(genpart, 411, 113, -211)) {
    return -1 * (100000 * m_nPhotos + 1887);
  }//anti-B0 decays to D+ rho0 pi-
  if (GenB0Tag::PcheckDecay(genpart, 411, -213, 111)) {
    return -1 * (100000 * m_nPhotos + 1888);
  }//anti-B0 decays to D+ rho- pi0
  if (GenB0Tag::PcheckDecay(genpart, 411, 211, -211, -211)) {
    return -1 * (100000 * m_nPhotos + 1889);
  }//anti-B0 decays to D+ pi+ pi- pi-
  if (GenB0Tag::PcheckDecay(genpart, 411, 111, -211, 111)) {
    return -1 * (100000 * m_nPhotos + 1890);
  }//anti-B0 decays to D+ pi0 pi- pi0
  if (GenB0Tag::PcheckDecay(genpart, 421, 211, -211, 111)) {
    return -1 * (100000 * m_nPhotos + 1891);
  }//anti-B0 decays to D0 pi+ pi- pi0
  if (GenB0Tag::PcheckDecay(genpart, 421, 111, 111, 111)) {
    return -1 * (100000 * m_nPhotos + 1892);
  }//anti-B0 decays to D0 pi0 pi0 pi0
  if (GenB0Tag::PcheckDecay(genpart, 413, -20213)) {
    return -1 * (100000 * m_nPhotos + 1893);
  }//anti-B0 decays to D*+ a_1-
  if (GenB0Tag::PcheckDecay(genpart, 413, 113, -211)) {
    return -1 * (100000 * m_nPhotos + 1894);
  }//anti-B0 decays to D*+ rho0 pi-
  if (GenB0Tag::PcheckDecay(genpart, 413, -213, 111)) {
    return -1 * (100000 * m_nPhotos + 1895);
  }//anti-B0 decays to D*+ rho- pi0
  if (GenB0Tag::PcheckDecay(genpart, 413, 211, -211, -211)) {
    return -1 * (100000 * m_nPhotos + 1896);
  }//anti-B0 decays to D*+ pi+ pi- pi-
  if (GenB0Tag::PcheckDecay(genpart, 413, 111, -211, 111)) {
    return -1 * (100000 * m_nPhotos + 1897);
  }//anti-B0 decays to D*+ pi0 pi- pi0
  if (GenB0Tag::PcheckDecay(genpart, 423, 211, -211, 111)) {
    return -1 * (100000 * m_nPhotos + 1898);
  }//anti-B0 decays to D*0 pi+ pi- pi0
  if (GenB0Tag::PcheckDecay(genpart, 423, 111, 111, 111)) {
    return -1 * (100000 * m_nPhotos + 1899);
  }//anti-B0 decays to D*0 pi0 pi0 pi0
  if (GenB0Tag::PcheckDecay(genpart, 10413, -211)) {
    return -1 * (100000 * m_nPhotos + 1900);
  }//anti-B0 decays to D_1+ pi-
  if (GenB0Tag::PcheckDecay(genpart, 20413, -211)) {
    return -1 * (100000 * m_nPhotos + 1901);
  }//anti-B0 decays to D'_1+ pi-
  if (GenB0Tag::PcheckDecay(genpart, 10411, -211)) {
    return -1 * (100000 * m_nPhotos + 1902);
  }//anti-B0 decays to D_0*+ pi-
  if (GenB0Tag::PcheckDecay(genpart, 415, -211)) {
    return -1 * (100000 * m_nPhotos + 1903);
  }//anti-B0 decays to D_2*+ pi-
  if (GenB0Tag::PcheckDecay(genpart, 10413, -213)) {
    return -1 * (100000 * m_nPhotos + 1904);
  }//anti-B0 decays to D_1+ rho-
  if (GenB0Tag::PcheckDecay(genpart, 20413, -213)) {
    return -1 * (100000 * m_nPhotos + 1905);
  }//anti-B0 decays to D'_1+ rho-
  if (GenB0Tag::PcheckDecay(genpart, 415, -213)) {
    return -1 * (100000 * m_nPhotos + 1906);
  }//anti-B0 decays to D_2*+ rho-
  if (GenB0Tag::PcheckDecay(genpart, 413, -321)) {
    return -1 * (100000 * m_nPhotos + 1907);
  }//anti-B0 decays to D*+ K-
  if (GenB0Tag::PcheckDecay(genpart, 411, -321)) {
    return -1 * (100000 * m_nPhotos + 1908);
  }//anti-B0 decays to D+ K-
  if (GenB0Tag::PcheckDecay(genpart, 413, -323)) {
    return -1 * (100000 * m_nPhotos + 1909);
  }//anti-B0 decays to D*+ K*-
  if (GenB0Tag::PcheckDecay(genpart, -323, 411)) {
    return -1 * (100000 * m_nPhotos + 1910);
  }//anti-B0 decays to K*- D+
  if (GenB0Tag::PcheckDecay(genpart, 423, 311)) {
    return -1 * (100000 * m_nPhotos + 1911);
  }//anti-B0 decays to D*0 K0
  if (GenB0Tag::PcheckDecay(genpart, 421, 311)) {
    return -1 * (100000 * m_nPhotos + 1912);
  }//anti-B0 decays to D0 K0
  if (GenB0Tag::PcheckDecay(genpart, -313, 421)) {
    return -1 * (100000 * m_nPhotos + 1913);
  }//anti-B0 decays to anti-K*0 D0
  if (GenB0Tag::PcheckDecay(genpart, -313, -421)) {
    return -1 * (100000 * m_nPhotos + 1914);
  }//anti-B0 decays to anti-K*0 anti-D0
  if (GenB0Tag::PcheckDecay(genpart, 423, -313)) {
    return -1 * (100000 * m_nPhotos + 1915);
  }//anti-B0 decays to D*0 anti-K*0
  if (GenB0Tag::PcheckDecay(genpart, -423, -313)) {
    return -1 * (100000 * m_nPhotos + 1916);
  }//anti-B0 decays to anti-D*0 anti-K*0
  if (GenB0Tag::PcheckDecay(genpart, 421, 111)) {
    return -1 * (100000 * m_nPhotos + 1917);
  }//anti-B0 decays to D0 pi0
  if (GenB0Tag::PcheckDecay(genpart, 423, 111)) {
    return -1 * (100000 * m_nPhotos + 1918);
  }//anti-B0 decays to D*0 pi0
  if (GenB0Tag::PcheckDecay(genpart, 113, 421)) {
    return -1 * (100000 * m_nPhotos + 1919);
  }//anti-B0 decays to rho0 D0
  if (GenB0Tag::PcheckDecay(genpart, 423, 113)) {
    return -1 * (100000 * m_nPhotos + 1920);
  }//anti-B0 decays to D*0 rho0
  if (GenB0Tag::PcheckDecay(genpart, 421, 221)) {
    return -1 * (100000 * m_nPhotos + 1921);
  }//anti-B0 decays to D0 eta
  if (GenB0Tag::PcheckDecay(genpart, 423, 221)) {
    return -1 * (100000 * m_nPhotos + 1922);
  }//anti-B0 decays to D*0 eta
  if (GenB0Tag::PcheckDecay(genpart, 421, 331)) {
    return -1 * (100000 * m_nPhotos + 1923);
  }//anti-B0 decays to D0 eta'
  if (GenB0Tag::PcheckDecay(genpart, 423, 331)) {
    return -1 * (100000 * m_nPhotos + 1924);
  }//anti-B0 decays to D*0 eta'
  if (GenB0Tag::PcheckDecay(genpart, 223, 421)) {
    return -1 * (100000 * m_nPhotos + 1925);
  }//anti-B0 decays to omega D0
  if (GenB0Tag::PcheckDecay(genpart, 423, 223)) {
    return -1 * (100000 * m_nPhotos + 1926);
  }//anti-B0 decays to D*0 omega
  if (GenB0Tag::PcheckDecay(genpart, 2212, -2212)) {
    return -1 * (100000 * m_nPhotos + 1927);
  }//anti-B0 decays to p+ anti-p-
  if (GenB0Tag::PcheckDecay(genpart, -10431, 411)) {
    return -1 * (100000 * m_nPhotos + 1928);
  }//anti-B0 decays to D_s0*- D+
  if (GenB0Tag::PcheckDecay(genpart, 413, -10431)) {
    return -1 * (100000 * m_nPhotos + 1929);
  }//anti-B0 decays to D*+ D_s0*-
  if (GenB0Tag::PcheckDecay(genpart, -10433, 411)) {
    return -1 * (100000 * m_nPhotos + 1930);
  }//anti-B0 decays to D_s1- D+
  if (GenB0Tag::PcheckDecay(genpart, 413, -10433)) {
    return -1 * (100000 * m_nPhotos + 1931);
  }//anti-B0 decays to D*+ D_s1-
  if (GenB0Tag::PcheckDecay(genpart, 411, -321, 313)) {
    return -1 * (100000 * m_nPhotos + 1932);
  }//anti-B0 decays to D+ K- K*0
  if (GenB0Tag::PcheckDecay(genpart, 413, -321, 313)) {
    return -1 * (100000 * m_nPhotos + 1933);
  }//anti-B0 decays to D*+ K- K*0
  if (GenB0Tag::PcheckDecay(genpart, 413, 311, -211)) {
    return -1 * (100000 * m_nPhotos + 1934);
  }//anti-B0 decays to D*+ K0 pi-
  if (GenB0Tag::PcheckDecay(genpart, -413, 311, 211)) {
    return -1 * (100000 * m_nPhotos + 1935);
  }//anti-B0 decays to D*- K0 pi+
  if (GenB0Tag::PcheckDecay(genpart, 411, -311, -211)) {
    return -1 * (100000 * m_nPhotos + 1936);
  }//anti-B0 decays to D+ anti-K0 pi-
  if (GenB0Tag::PcheckDecay(genpart, 411, 223, -211)) {
    return -1 * (100000 * m_nPhotos + 1937);
  }//anti-B0 decays to D+ omega pi-
  if (GenB0Tag::PcheckDecay(genpart, 413, 223, -211)) {
    return -1 * (100000 * m_nPhotos + 1938);
  }//anti-B0 decays to D*+ omega pi-
  if (GenB0Tag::PcheckDecay(genpart, -20433, 411)) {
    return -1 * (100000 * m_nPhotos + 1939);
  }//anti-B0 decays to D'_s1- D+
  if (GenB0Tag::PcheckDecay(genpart, -20433, 413)) {
    return -1 * (100000 * m_nPhotos + 1940);
  }//anti-B0 decays to D'_s1- D*+
  if (GenB0Tag::PcheckDecay(genpart, 4101, -2101)) {
    return -1 * (100000 * m_nPhotos + 1941);
  }//anti-B0 decays to cd_0 anti-ud_0
  if (GenB0Tag::PcheckDecay(genpart, 4103, -2103)) {
    return -1 * (100000 * m_nPhotos + 1942);
  }//anti-B0 decays to cd_1 anti-ud_1
  if (GenB0Tag::PcheckDecay(genpart, 4301, -2101)) {
    return -1 * (100000 * m_nPhotos + 1943);
  }//anti-B0 decays to cs_0 anti-ud_0
  if (GenB0Tag::PcheckDecay(genpart, 4303, -2103)) {
    return -1 * (100000 * m_nPhotos + 1944);
  }//anti-B0 decays to cs_1 anti-ud_1
  if (GenB0Tag::PcheckDecay(genpart, -2, 1, 4, -1)) {
    return -1 * (100000 * m_nPhotos + 1945);
  }//anti-B0 decays to anti-u d c anti-d
  if (GenB0Tag::PcheckDecay(genpart, -2, 1, 4, -1)) {
    return -1 * (100000 * m_nPhotos + 1946);
  }//anti-B0 decays to anti-u d c anti-d
  if (GenB0Tag::PcheckDecay(genpart, -2, 3, 4, -1)) {
    return -1 * (100000 * m_nPhotos + 1947);
  }//anti-B0 decays to anti-u s c anti-d
  if (GenB0Tag::PcheckDecay(genpart, -2, 4, 1, -1)) {
    return -1 * (100000 * m_nPhotos + 1948);
  }//anti-B0 decays to anti-u c d anti-d
  if (GenB0Tag::PcheckDecay(genpart, -2, 4, 3, -1)) {
    return -1 * (100000 * m_nPhotos + 1949);
  }//anti-B0 decays to anti-u c s anti-d
  if (GenB0Tag::PcheckDecay(genpart, -4, 3, 4, -1)) {
    return -1 * (100000 * m_nPhotos + 1950);
  }//anti-B0 decays to anti-c s c anti-d
  if (GenB0Tag::PcheckDecay(genpart, -4, 1, 4, -1)) {
    return -1 * (100000 * m_nPhotos + 1951);
  }//anti-B0 decays to anti-c d c anti-d
  if (GenB0Tag::PcheckDecay(genpart, -2, 1, 2, -1)) {
    return -1 * (100000 * m_nPhotos + 1952);
  }//anti-B0 decays to anti-u d u anti-d
  if (GenB0Tag::PcheckDecay(genpart, -4, 3, 2, -1)) {
    return -1 * (100000 * m_nPhotos + 1953);
  }//anti-B0 decays to anti-c s u anti-d
  if (GenB0Tag::PcheckDecay(genpart, -2, 2, 1, -1)) {
    return -1 * (100000 * m_nPhotos + 1954);
  }//anti-B0 decays to anti-u u d anti-d
  if (GenB0Tag::PcheckDecay(genpart, -1, 1, 1, -1)) {
    return -1 * (100000 * m_nPhotos + 1955);
  }//anti-B0 decays to anti-d d d anti-d
  if (GenB0Tag::PcheckDecay(genpart, -3, 3, 1, -1)) {
    return -1 * (100000 * m_nPhotos + 1956);
  }//anti-B0 decays to anti-s s d anti-d
  if (GenB0Tag::PcheckDecay(genpart, -2, 2, 3, -1)) {
    return -1 * (100000 * m_nPhotos + 1957);
  }//anti-B0 decays to anti-u u s anti-d
  if (GenB0Tag::PcheckDecay(genpart, -1, 1, 3, -1)) {
    return -1 * (100000 * m_nPhotos + 1958);
  }//anti-B0 decays to anti-d d s anti-d
  if (GenB0Tag::PcheckDecay(genpart, -3, 3, 3, -1)) {
    return -1 * (100000 * m_nPhotos + 1959);
  }//anti-B0 decays to anti-s s s anti-d
  if (GenB0Tag::PcheckDecay(genpart, 3, -1)) {
    return -1 * (100000 * m_nPhotos + 1960);
  }//anti-B0 decays to s anti-d
  if (GenB0Tag::PcheckDecay(genpart, 421, -311)) {
    return -1 * (100000 * m_nPhotos + 1961);
  }//anti-B0 decays to D0 anti-K0
  if (GenB0Tag::PcheckDecay(genpart, 421, -321, 211)) {
    return -1 * (100000 * m_nPhotos + 1962);
  }//anti-B0 decays to D0 K- pi+
  if (GenB0Tag::PcheckDecay(genpart, -421, -321, 211)) {
    return -1 * (100000 * m_nPhotos + 1963);
  }//anti-B0 decays to anti-D0 K- pi+
  if (GenB0Tag::PcheckDecay(genpart, 423, -311)) {
    return -1 * (100000 * m_nPhotos + 1964);
  }//anti-B0 decays to D*0 anti-K0
  if (GenB0Tag::PcheckDecay(genpart, -423, -211, -211, 211, 211)) {
    return -1 * (100000 * m_nPhotos + 1965);
  }//anti-B0 decays to anti-D*0 pi- pi- pi+ pi+
  if (GenB0Tag::PcheckDecay(genpart, 441, -311)) {
    return -1 * (100000 * m_nPhotos + 1966);
  }//anti-B0 decays to eta_c anti-K0
  if (GenB0Tag::PcheckDecay(genpart, -311, -421, 421, 111)) {
    return -1 * (100000 * m_nPhotos + 1967);
  }//anti-B0 decays to anti-K0 anti-D0 D0 pi0
  if (GenB0Tag::PcheckDecay(genpart, 100443, -311)) {
    return -1 * (100000 * m_nPhotos + 1968);
  }//anti-B0 decays to psi(2S) anti-K0
  if (GenB0Tag::PcheckDecay(genpart, 10441, -311)) {
    return -1 * (100000 * m_nPhotos + 1969);
  }//anti-B0 decays to chi_c0 anti-K0
  if (GenB0Tag::PcheckDecay(genpart, 20443, -311)) {
    return -1 * (100000 * m_nPhotos + 1970);
  }//anti-B0 decays to chi_c1 anti-K0
  if (GenB0Tag::PcheckDecay(genpart, 20443, 321, -211)) {
    return -1 * (100000 * m_nPhotos + 1971);
  }//anti-B0 decays to chi_c1 K+ pi-
  if (GenB0Tag::PcheckDecay(genpart, -311, 113)) {
    return -1 * (100000 * m_nPhotos + 1972);
  }//anti-B0 decays to anti-K0 rho0
  if (GenB0Tag::PcheckDecay(genpart, 221, -10311)) {
    return -1 * (100000 * m_nPhotos + 1973);
  }//anti-B0 decays to eta anti-K_0*0
  if (GenB0Tag::PcheckDecay(genpart, 221, -315)) {
    return -1 * (100000 * m_nPhotos + 1974);
  }//anti-B0 decays to eta anti-K_2*0
  if (GenB0Tag::PcheckDecay(genpart, 223, -10311)) {
    return -1 * (100000 * m_nPhotos + 1975);
  }//anti-B0 decays to omega anti-K_0*0
  if (GenB0Tag::PcheckDecay(genpart, 223, -315)) {
    return -1 * (100000 * m_nPhotos + 1976);
  }//anti-B0 decays to omega anti-K_2*0
  if (GenB0Tag::PcheckDecay(genpart, 225, -311)) {
    return -1 * (100000 * m_nPhotos + 1977);
  }//anti-B0 decays to f_2 anti-K0
  if (GenB0Tag::PcheckDecay(genpart, 10221, -321, 211)) {
    return -1 * (100000 * m_nPhotos + 1978);
  }//anti-B0 decays to f_0 K- pi+
  if (GenB0Tag::PcheckDecay(genpart, -315, 333)) {
    return -1 * (100000 * m_nPhotos + 1979);
  }//anti-B0 decays to anti-K_2*0 phi
  if (GenB0Tag::PcheckDecay(genpart, 221, -311, 22)) {
    return -1 * (100000 * m_nPhotos + 1980);
  }//anti-B0 decays to eta anti-K0 gamma
  if (GenB0Tag::PcheckDecay(genpart, -2212, 2212, -311)) {
    return -1 * (100000 * m_nPhotos + 1981);
  }//anti-B0 decays to anti-p- p+ anti-K0
  if (GenB0Tag::PcheckDecay(genpart, -2212, 2212, -313)) {
    return -1 * (100000 * m_nPhotos + 1982);
  }//anti-B0 decays to anti-p- p+ anti-K*0
  if (GenB0Tag::PcheckDecay(genpart, -2212, 3122, 211)) {
    return -1 * (100000 * m_nPhotos + 1983);
  }//anti-B0 decays to anti-p- Lambda0 pi+
  if (GenB0Tag::PcheckDecay(genpart, 3122, -3122, -311)) {
    return -1 * (100000 * m_nPhotos + 1984);
  }//anti-B0 decays to Lambda0 anti-Lambda0 anti-K0
  if (GenB0Tag::PcheckDecay(genpart, 3122, -3122, -313)) {
    return -1 * (100000 * m_nPhotos + 1985);
  }//anti-B0 decays to Lambda0 anti-Lambda0 anti-K*0
  if (GenB0Tag::PcheckDecay(genpart, 3122, -3122, -421)) {
    return -1 * (100000 * m_nPhotos + 1986);
  }//anti-B0 decays to Lambda0 anti-Lambda0 anti-D0
  if (GenB0Tag::PcheckDecay(genpart, 421, -2212, 2212)) {
    return -1 * (100000 * m_nPhotos + 1987);
  }//anti-B0 decays to D0 anti-p- p+
  if (GenB0Tag::PcheckDecay(genpart, 431, 3122, -2212)) {
    return -1 * (100000 * m_nPhotos + 1988);
  }//anti-B0 decays to D_s+ Lambda0 anti-p-
  if (GenB0Tag::PcheckDecay(genpart, 423, -2212, 2212)) {
    return -1 * (100000 * m_nPhotos + 1989);
  }//anti-B0 decays to D*0 anti-p- p+
  if (GenB0Tag::PcheckDecay(genpart, 413, -2212, 2112)) {
    return -1 * (100000 * m_nPhotos + 1990);
  }//anti-B0 decays to D*+ anti-p- n0
  if (GenB0Tag::PcheckDecay(genpart, 411, -2212, 2212, -211)) {
    return -1 * (100000 * m_nPhotos + 1991);
  }//anti-B0 decays to D+ anti-p- p+ pi-
  if (GenB0Tag::PcheckDecay(genpart, 413, -2212, 2212, -211)) {
    return -1 * (100000 * m_nPhotos + 1992);
  }//anti-B0 decays to D*+ anti-p- p+ pi-
  if (GenB0Tag::PcheckDecay(genpart, 4122, -2212)) {
    return -1 * (100000 * m_nPhotos + 1993);
  }//anti-B0 decays to Lambda_c+ anti-p-
  if (GenB0Tag::PcheckDecay(genpart, 4122, -2212, -211, 211)) {
    return -1 * (100000 * m_nPhotos + 1994);
  }//anti-B0 decays to Lambda_c+ anti-p- pi- pi+
  if (GenB0Tag::PcheckDecay(genpart, 4224, -2212, -211)) {
    return -1 * (100000 * m_nPhotos + 1995);
  }//anti-B0 decays to Sigma_c*++ anti-p- pi-
  if (GenB0Tag::PcheckDecay(genpart, 4112, -2212, 211)) {
    return -1 * (100000 * m_nPhotos + 1996);
  }//anti-B0 decays to Sigma_c0 anti-p- pi+
  if (GenB0Tag::PcheckDecay(genpart, 4222, -2212, -211)) {
    return -1 * (100000 * m_nPhotos + 1997);
  }//anti-B0 decays to Sigma_c++ anti-p- pi-
  if (GenB0Tag::PcheckDecay(genpart, 413, -211, -211, 211, 111)) {
    return -1 * (100000 * m_nPhotos + 1998);
  }//anti-B0 decays to D*+ pi- pi- pi+ pi0
  if (GenB0Tag::PcheckDecay(genpart, 413, -211, -211, -211, 211, 211)) {
    return -1 * (100000 * m_nPhotos + 1999);
  }//anti-B0 decays to D*+ pi- pi- pi- pi+ pi+
  if (GenB0Tag::PcheckDecay(genpart, 431, -211, -311)) {
    return -1 * (100000 * m_nPhotos + 2000);
  }//anti-B0 decays to D_s+ pi- anti-K0
  return -1 * (int)genpart.size();
}// Rest of the anti-B0 decays
