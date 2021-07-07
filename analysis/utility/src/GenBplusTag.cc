/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <analysis/utility/GenBplusTag.h>
#include <analysis/utility/CheckDecayUtils.h>

using namespace Belle2;

GenBplusTag::GenBplusTag() = default;

bool GenBplusTag::PcheckDecay(std::vector<int>gp, int da1, int da2)
{
  int ndau = gp.size();
  if (ndau == 2) { m_nPhotos = 0; return CheckDecay::CheckDecay(gp, da1, da2);}
  else if (ndau == 3) { m_nPhotos = 1; return CheckDecay::CheckDecay(gp, da1, da2, 22);}
  else if (ndau == 4) { m_nPhotos = 2; return CheckDecay::CheckDecay(gp, da1, da2, 22, 22);}
  else if (ndau == 5) { m_nPhotos = 3; return CheckDecay::CheckDecay(gp, da1, da2, 22, 22, 22);}
  else if (ndau == 6) { m_nPhotos = 4; return CheckDecay::CheckDecay(gp, da1, da2, 22, 22, 22, 22);}
  else return false;
}

bool GenBplusTag::PcheckDecay(std::vector<int>gp, int da1, int da2, int da3)
{
  int ndau = gp.size();
  if (ndau == 3) { m_nPhotos = 0; return CheckDecay::CheckDecay(gp, da1, da2, da3);}
  else if (ndau == 4) { m_nPhotos = 1; return CheckDecay::CheckDecay(gp, da1, da2, da3, 22);}
  else if (ndau == 5) { m_nPhotos = 2; return CheckDecay::CheckDecay(gp, da1, da2, da3, 22, 22);}
  else if (ndau == 6) { m_nPhotos = 3; return CheckDecay::CheckDecay(gp, da1, da2, da3, 22, 22, 22);}
  else if (ndau == 7) { m_nPhotos = 4; return CheckDecay::CheckDecay(gp, da1, da2, da3, 22, 22, 22, 22);}
  else return false;
}

bool GenBplusTag::PcheckDecay(std::vector<int>gp, int da1, int da2, int da3, int da4)
{
  int ndau = gp.size();
  if (ndau == 4) { m_nPhotos = 0; return CheckDecay::CheckDecay(gp, da1, da2, da3, da4);}
  else if (ndau == 5) { m_nPhotos = 1; return CheckDecay::CheckDecay(gp, da1, da2, da3, da4, 22);}
  else if (ndau == 6) { m_nPhotos = 2; return CheckDecay::CheckDecay(gp, da1, da2, da3, da4, 22, 22);}
  else if (ndau == 7) { m_nPhotos = 3; return CheckDecay::CheckDecay(gp, da1, da2, da3, da4, 22, 22, 22);}
  else if (ndau == 8) { m_nPhotos = 4; return CheckDecay::CheckDecay(gp, da1, da2, da3, da4, 22, 22, 22, 22);}
  else return false;
}

bool GenBplusTag::PcheckDecay(std::vector<int>gp, int da1, int da2, int da3, int da4, int da5)
{
  int ndau = gp.size();
  if (ndau == 5) { m_nPhotos = 0; return CheckDecay::CheckDecay(gp, da1, da2, da3, da4, da5);}
  else if (ndau == 6) { m_nPhotos = 1; return CheckDecay::CheckDecay(gp, da1, da2, da3, da4, da5, 22);}
  else if (ndau == 7) { m_nPhotos = 2; return CheckDecay::CheckDecay(gp, da1, da2, da3, da4, da5, 22, 22);}
  else if (ndau == 8) { m_nPhotos = 3; return CheckDecay::CheckDecay(gp, da1, da2, da3, da4, da5, 22, 22, 22);}
  else if (ndau == 9) { m_nPhotos = 4; return CheckDecay::CheckDecay(gp, da1, da2, da3, da4, da5, 22, 22, 22, 22);}
  else return false;
}

bool GenBplusTag::PcheckDecay(std::vector<int>gp, int da1, int da2, int da3, int da4, int da5, int da6)
{
  int ndau = gp.size();
  if (ndau == 6) { m_nPhotos = 0;  return CheckDecay::CheckDecay(gp, da1, da2, da3, da4, da5, da6);}
  else if (ndau == 7) { m_nPhotos = 1; return CheckDecay::CheckDecay(gp, da1, da2, da3, da4, da5, da6, 22);}
  else if (ndau == 8) { m_nPhotos = 2; return CheckDecay::CheckDecay(gp, da1, da2, da3, da4, da5, da6, 22, 22);}
  else if (ndau == 9) { m_nPhotos = 3; return CheckDecay::CheckDecay(gp, da1, da2, da3, da4, da5, da6, 22, 22, 22);}
  else if (ndau == 10) { m_nPhotos = 4; return CheckDecay::CheckDecay(gp, da1, da2, da3, da4, da5, da6, 22, 22, 22, 22);}
  else return false;
}

bool GenBplusTag::PcheckDecay(std::vector<int>gp, int da1, int da2, int da3, int da4, int da5, int da6, int da7)
{
  int ndau = gp.size();
  if (ndau == 7) { m_nPhotos = 0;  return CheckDecay::CheckDecay(gp, da1, da2, da3, da4, da5, da6, da7);}
  else if (ndau == 8) { m_nPhotos = 1; return CheckDecay::CheckDecay(gp, da1, da2, da3, da4, da5, da6, da7, 22);}
  else if (ndau == 9) { m_nPhotos = 2; return CheckDecay::CheckDecay(gp, da1, da2, da3, da4, da5, da6, da7, 22, 22);}
  else if (ndau == 10) { m_nPhotos = 3; return CheckDecay::CheckDecay(gp, da1, da2, da3, da4, da5, da6, da7, 22, 22, 22);}
  else if (ndau == 11) { m_nPhotos = 4; return CheckDecay::CheckDecay(gp, da1, da2, da3, da4, da5, da6, da7, 22, 22, 22, 22);}
  else return false;
}

bool GenBplusTag::PcheckDecay(std::vector<int>gp, int da1, int da2, int da3, int da4, int da5, int da6, int da7, int da8)
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

int GenBplusTag::Mode_B_plus(std::vector<int>genpart)
{
  if (GenBplusTag::PcheckDecay(genpart, -423, -11, 12)) {
    return +1 * (100000 * m_nPhotos + 1001);
  }//B+ decays to anti-D*0 e+ nu_e
  if (GenBplusTag::PcheckDecay(genpart, -421, -11, 12)) {
    return +1 * (100000 * m_nPhotos + 1002);
  }//B+ decays to anti-D0 e+ nu_e
  if (GenBplusTag::PcheckDecay(genpart, -10423, -11, 12)) {
    return +1 * (100000 * m_nPhotos + 1003);
  }//B+ decays to anti-D_10 e+ nu_e
  if (GenBplusTag::PcheckDecay(genpart, -10421, -11, 12)) {
    return +1 * (100000 * m_nPhotos + 1004);
  }//B+ decays to anti-D_0*0 e+ nu_e
  if (GenBplusTag::PcheckDecay(genpart, -20423, -11, 12)) {
    return +1 * (100000 * m_nPhotos + 1005);
  }//B+ decays to anti-D'_10 e+ nu_e
  if (GenBplusTag::PcheckDecay(genpart, -425, -11, 12)) {
    return +1 * (100000 * m_nPhotos + 1006);
  }//B+ decays to anti-D_2*0 e+ nu_e
  if (GenBplusTag::PcheckDecay(genpart, -413, 211, -11, 12)) {
    return +1 * (100000 * m_nPhotos + 1007);
  }//B+ decays to D*- pi+ e+ nu_e
  if (GenBplusTag::PcheckDecay(genpart, -423, 111, -11, 12)) {
    return +1 * (100000 * m_nPhotos + 1008);
  }//B+ decays to anti-D*0 pi0 e+ nu_e
  if (GenBplusTag::PcheckDecay(genpart, -411, 211, -11, 12)) {
    return +1 * (100000 * m_nPhotos + 1009);
  }//B+ decays to D- pi+ e+ nu_e
  if (GenBplusTag::PcheckDecay(genpart, -421, 111, -11, 12)) {
    return +1 * (100000 * m_nPhotos + 1010);
  }//B+ decays to anti-D0 pi0 e+ nu_e
  if (GenBplusTag::PcheckDecay(genpart, -421, 211, -211, -11, 12)) {
    return +1 * (100000 * m_nPhotos + 1011);
  }//B+ decays to anti-D0 pi+ pi- e+ nu_e
  if (GenBplusTag::PcheckDecay(genpart, -411, 211, 111, -11, 12)) {
    return +1 * (100000 * m_nPhotos + 1012);
  }//B+ decays to D- pi+ pi0 e+ nu_e
  if (GenBplusTag::PcheckDecay(genpart, -421, 111, 111, -11, 12)) {
    return +1 * (100000 * m_nPhotos + 1013);
  }//B+ decays to anti-D0 pi0 pi0 e+ nu_e
  if (GenBplusTag::PcheckDecay(genpart, -423, 211, -211, -11, 12)) {
    return +1 * (100000 * m_nPhotos + 1014);
  }//B+ decays to anti-D*0 pi+ pi- e+ nu_e
  if (GenBplusTag::PcheckDecay(genpart, -413, 211, 111, -11, 12)) {
    return +1 * (100000 * m_nPhotos + 1015);
  }//B+ decays to D*- pi+ pi0 e+ nu_e
  if (GenBplusTag::PcheckDecay(genpart, -423, 111, 111, -11, 12)) {
    return +1 * (100000 * m_nPhotos + 1016);
  }//B+ decays to anti-D*0 pi0 pi0 e+ nu_e
  if (GenBplusTag::PcheckDecay(genpart, -433, 321, -11, 12)) {
    return +1 * (100000 * m_nPhotos + 1017);
  }//B+ decays to D_s*- K+ e+ nu_e
  if (GenBplusTag::PcheckDecay(genpart, -431, 321, -11, 12)) {
    return +1 * (100000 * m_nPhotos + 1018);
  }//B+ decays to D_s- K+ e+ nu_e
  if (GenBplusTag::PcheckDecay(genpart, -421, 221, -11, 12)) {
    return +1 * (100000 * m_nPhotos + 1019);
  }//B+ decays to anti-D0 eta e+ nu_e
  if (GenBplusTag::PcheckDecay(genpart, -423, 221, -11, 12)) {
    return +1 * (100000 * m_nPhotos + 1020);
  }//B+ decays to anti-D*0 eta e+ nu_e
  if (GenBplusTag::PcheckDecay(genpart, -423, -13, 14)) {
    return +1 * (100000 * m_nPhotos + 1021);
  }//B+ decays to anti-D*0 mu+ nu_mu
  if (GenBplusTag::PcheckDecay(genpart, -421, -13, 14)) {
    return +1 * (100000 * m_nPhotos + 1022);
  }//B+ decays to anti-D0 mu+ nu_mu
  if (GenBplusTag::PcheckDecay(genpart, -10423, -13, 14)) {
    return +1 * (100000 * m_nPhotos + 1023);
  }//B+ decays to anti-D_10 mu+ nu_mu
  if (GenBplusTag::PcheckDecay(genpart, -10421, -13, 14)) {
    return +1 * (100000 * m_nPhotos + 1024);
  }//B+ decays to anti-D_0*0 mu+ nu_mu
  if (GenBplusTag::PcheckDecay(genpart, -20423, -13, 14)) {
    return +1 * (100000 * m_nPhotos + 1025);
  }//B+ decays to anti-D'_10 mu+ nu_mu
  if (GenBplusTag::PcheckDecay(genpart, -425, -13, 14)) {
    return +1 * (100000 * m_nPhotos + 1026);
  }//B+ decays to anti-D_2*0 mu+ nu_mu
  if (GenBplusTag::PcheckDecay(genpart, -413, 211, -13, 14)) {
    return +1 * (100000 * m_nPhotos + 1027);
  }//B+ decays to D*- pi+ mu+ nu_mu
  if (GenBplusTag::PcheckDecay(genpart, -423, 111, -13, 14)) {
    return +1 * (100000 * m_nPhotos + 1028);
  }//B+ decays to anti-D*0 pi0 mu+ nu_mu
  if (GenBplusTag::PcheckDecay(genpart, -411, 211, -13, 14)) {
    return +1 * (100000 * m_nPhotos + 1029);
  }//B+ decays to D- pi+ mu+ nu_mu
  if (GenBplusTag::PcheckDecay(genpart, -421, 111, -13, 14)) {
    return +1 * (100000 * m_nPhotos + 1030);
  }//B+ decays to anti-D0 pi0 mu+ nu_mu
  if (GenBplusTag::PcheckDecay(genpart, -421, 211, -211, -13, 14)) {
    return +1 * (100000 * m_nPhotos + 1031);
  }//B+ decays to anti-D0 pi+ pi- mu+ nu_mu
  if (GenBplusTag::PcheckDecay(genpart, -411, 211, 111, -13, 14)) {
    return +1 * (100000 * m_nPhotos + 1032);
  }//B+ decays to D- pi+ pi0 mu+ nu_mu
  if (GenBplusTag::PcheckDecay(genpart, -421, 111, 111, -13, 14)) {
    return +1 * (100000 * m_nPhotos + 1033);
  }//B+ decays to anti-D0 pi0 pi0 mu+ nu_mu
  if (GenBplusTag::PcheckDecay(genpart, -423, 211, -211, -13, 14)) {
    return +1 * (100000 * m_nPhotos + 1034);
  }//B+ decays to anti-D*0 pi+ pi- mu+ nu_mu
  if (GenBplusTag::PcheckDecay(genpart, -413, 211, 111, -13, 14)) {
    return +1 * (100000 * m_nPhotos + 1035);
  }//B+ decays to D*- pi+ pi0 mu+ nu_mu
  if (GenBplusTag::PcheckDecay(genpart, -423, 111, 111, -13, 14)) {
    return +1 * (100000 * m_nPhotos + 1036);
  }//B+ decays to anti-D*0 pi0 pi0 mu+ nu_mu
  if (GenBplusTag::PcheckDecay(genpart, -433, 321, -13, 14)) {
    return +1 * (100000 * m_nPhotos + 1037);
  }//B+ decays to D_s*- K+ mu+ nu_mu
  if (GenBplusTag::PcheckDecay(genpart, -431, 321, -13, 14)) {
    return +1 * (100000 * m_nPhotos + 1038);
  }//B+ decays to D_s- K+ mu+ nu_mu
  if (GenBplusTag::PcheckDecay(genpart, -421, 221, -13, 14)) {
    return +1 * (100000 * m_nPhotos + 1039);
  }//B+ decays to anti-D0 eta mu+ nu_mu
  if (GenBplusTag::PcheckDecay(genpart, -423, 221, -13, 14)) {
    return +1 * (100000 * m_nPhotos + 1040);
  }//B+ decays to anti-D*0 eta mu+ nu_mu
  if (GenBplusTag::PcheckDecay(genpart, -423, -15, 16)) {
    return +1 * (100000 * m_nPhotos + 1041);
  }//B+ decays to anti-D*0 tau+ nu_tau
  if (GenBplusTag::PcheckDecay(genpart, -421, -15, 16)) {
    return +1 * (100000 * m_nPhotos + 1042);
  }//B+ decays to anti-D0 tau+ nu_tau
  if (GenBplusTag::PcheckDecay(genpart, -10423, -15, 16)) {
    return +1 * (100000 * m_nPhotos + 1043);
  }//B+ decays to anti-D_10 tau+ nu_tau
  if (GenBplusTag::PcheckDecay(genpart, -10421, -15, 16)) {
    return +1 * (100000 * m_nPhotos + 1044);
  }//B+ decays to anti-D_0*0 tau+ nu_tau
  if (GenBplusTag::PcheckDecay(genpart, -20423, -15, 16)) {
    return +1 * (100000 * m_nPhotos + 1045);
  }//B+ decays to anti-D'_10 tau+ nu_tau
  if (GenBplusTag::PcheckDecay(genpart, -425, -15, 16)) {
    return +1 * (100000 * m_nPhotos + 1046);
  }//B+ decays to anti-D_2*0 tau+ nu_tau
  if (GenBplusTag::PcheckDecay(genpart, 111, -11, 12)) {
    return +1 * (100000 * m_nPhotos + 1047);
  }//B+ decays to pi0 e+ nu_e
  if (GenBplusTag::PcheckDecay(genpart, 221, -11, 12)) {
    return +1 * (100000 * m_nPhotos + 1048);
  }//B+ decays to eta e+ nu_e
  if (GenBplusTag::PcheckDecay(genpart, 113, -11, 12)) {
    return +1 * (100000 * m_nPhotos + 1049);
  }//B+ decays to rho0 e+ nu_e
  if (GenBplusTag::PcheckDecay(genpart, 223, -11, 12)) {
    return +1 * (100000 * m_nPhotos + 1050);
  }//B+ decays to omega e+ nu_e
  if (GenBplusTag::PcheckDecay(genpart, 331, -11, 12)) {
    return +1 * (100000 * m_nPhotos + 1051);
  }//B+ decays to eta' e+ nu_e
  if (GenBplusTag::PcheckDecay(genpart, -100421, -11, 12)) {
    return +1 * (100000 * m_nPhotos + 1052);
  }//B+ decays to anti-D(2S)0 e+ nu_e
  if (GenBplusTag::PcheckDecay(genpart, -100423, -11, 12)) {
    return +1 * (100000 * m_nPhotos + 1053);
  }//B+ decays to anti-D*(2S)0 e+ nu_e
  if (GenBplusTag::PcheckDecay(genpart, 61, -11, 12)) {
    return +1 * (100000 * m_nPhotos + 1054);
  }//B+ decays to Xu0 e+ nu_e
  if (GenBplusTag::PcheckDecay(genpart, 111, -13, 14)) {
    return +1 * (100000 * m_nPhotos + 1055);
  }//B+ decays to pi0 mu+ nu_mu
  if (GenBplusTag::PcheckDecay(genpart, 221, -13, 14)) {
    return +1 * (100000 * m_nPhotos + 1056);
  }//B+ decays to eta mu+ nu_mu
  if (GenBplusTag::PcheckDecay(genpart, 113, -13, 14)) {
    return +1 * (100000 * m_nPhotos + 1057);
  }//B+ decays to rho0 mu+ nu_mu
  if (GenBplusTag::PcheckDecay(genpart, 223, -13, 14)) {
    return +1 * (100000 * m_nPhotos + 1058);
  }//B+ decays to omega mu+ nu_mu
  if (GenBplusTag::PcheckDecay(genpart, 331, -13, 14)) {
    return +1 * (100000 * m_nPhotos + 1059);
  }//B+ decays to eta' mu+ nu_mu
  if (GenBplusTag::PcheckDecay(genpart, -100421, -13, 14)) {
    return +1 * (100000 * m_nPhotos + 1060);
  }//B+ decays to anti-D(2S)0 mu+ nu_mu
  if (GenBplusTag::PcheckDecay(genpart, -100423, -13, 14)) {
    return +1 * (100000 * m_nPhotos + 1061);
  }//B+ decays to anti-D*(2S)0 mu+ nu_mu
  if (GenBplusTag::PcheckDecay(genpart, 61, -13, 14)) {
    return +1 * (100000 * m_nPhotos + 1062);
  }//B+ decays to Xu0 mu+ nu_mu
  if (GenBplusTag::PcheckDecay(genpart, 111, -15, 16)) {
    return +1 * (100000 * m_nPhotos + 1063);
  }//B+ decays to pi0 tau+ nu_tau
  if (GenBplusTag::PcheckDecay(genpart, 221, -15, 16)) {
    return +1 * (100000 * m_nPhotos + 1064);
  }//B+ decays to eta tau+ nu_tau
  if (GenBplusTag::PcheckDecay(genpart, 113, -15, 16)) {
    return +1 * (100000 * m_nPhotos + 1065);
  }//B+ decays to rho0 tau+ nu_tau
  if (GenBplusTag::PcheckDecay(genpart, 223, -15, 16)) {
    return +1 * (100000 * m_nPhotos + 1066);
  }//B+ decays to omega tau+ nu_tau
  if (GenBplusTag::PcheckDecay(genpart, 331, -15, 16)) {
    return +1 * (100000 * m_nPhotos + 1067);
  }//B+ decays to eta' tau+ nu_tau
  if (GenBplusTag::PcheckDecay(genpart, 20113, -15, 16)) {
    return +1 * (100000 * m_nPhotos + 1068);
  }//B+ decays to a_10 tau+ nu_tau
  if (GenBplusTag::PcheckDecay(genpart, 10113, -15, 16)) {
    return +1 * (100000 * m_nPhotos + 1069);
  }//B+ decays to b_10 tau+ nu_tau
  if (GenBplusTag::PcheckDecay(genpart, 10111, -15, 16)) {
    return +1 * (100000 * m_nPhotos + 1070);
  }//B+ decays to a_00 tau+ nu_tau
  if (GenBplusTag::PcheckDecay(genpart, 10221, -15, 16)) {
    return +1 * (100000 * m_nPhotos + 1071);
  }//B+ decays to f_0 tau+ nu_tau
  if (GenBplusTag::PcheckDecay(genpart, 10331, -15, 16)) {
    return +1 * (100000 * m_nPhotos + 1072);
  }//B+ decays to f'_0 tau+ nu_tau
  if (GenBplusTag::PcheckDecay(genpart, 20223, -15, 16)) {
    return +1 * (100000 * m_nPhotos + 1073);
  }//B+ decays to f_1 tau+ nu_tau
  if (GenBplusTag::PcheckDecay(genpart, 20333, -15, 16)) {
    return +1 * (100000 * m_nPhotos + 1074);
  }//B+ decays to f'_1 tau+ nu_tau
  if (GenBplusTag::PcheckDecay(genpart, 10223, -15, 16)) {
    return +1 * (100000 * m_nPhotos + 1075);
  }//B+ decays to h_1 tau+ nu_tau
  if (GenBplusTag::PcheckDecay(genpart, 10333, -15, 16)) {
    return +1 * (100000 * m_nPhotos + 1076);
  }//B+ decays to h'_1 tau+ nu_tau
  if (GenBplusTag::PcheckDecay(genpart, 225, -15, 16)) {
    return +1 * (100000 * m_nPhotos + 1077);
  }//B+ decays to f_2 tau+ nu_tau
  if (GenBplusTag::PcheckDecay(genpart, 335, -15, 16)) {
    return +1 * (100000 * m_nPhotos + 1078);
  }//B+ decays to f'_2 tau+ nu_tau
  if (GenBplusTag::PcheckDecay(genpart, -100421, -15, 16)) {
    return +1 * (100000 * m_nPhotos + 1079);
  }//B+ decays to anti-D(2S)0 tau+ nu_tau
  if (GenBplusTag::PcheckDecay(genpart, -100423, -15, 16)) {
    return +1 * (100000 * m_nPhotos + 1080);
  }//B+ decays to anti-D*(2S)0 tau+ nu_tau
  if (GenBplusTag::PcheckDecay(genpart, 431, 111)) {
    return +1 * (100000 * m_nPhotos + 1081);
  }//B+ decays to D_s+ pi0
  if (GenBplusTag::PcheckDecay(genpart, 433, 111)) {
    return +1 * (100000 * m_nPhotos + 1082);
  }//B+ decays to D_s*+ pi0
  if (GenBplusTag::PcheckDecay(genpart, 113, 431)) {
    return +1 * (100000 * m_nPhotos + 1083);
  }//B+ decays to rho0 D_s+
  if (GenBplusTag::PcheckDecay(genpart, 433, 113)) {
    return +1 * (100000 * m_nPhotos + 1084);
  }//B+ decays to D_s*+ rho0
  if (GenBplusTag::PcheckDecay(genpart, 323, 22)) {
    return +1 * (100000 * m_nPhotos + 1085);
  }//B+ decays to K*+ gamma
  if (GenBplusTag::PcheckDecay(genpart, 10323, 22)) {
    return +1 * (100000 * m_nPhotos + 1086);
  }//B+ decays to K_1+ gamma
  if (GenBplusTag::PcheckDecay(genpart, 325, 22)) {
    return +1 * (100000 * m_nPhotos + 1087);
  }//B+ decays to K_2*+ gamma
  if (GenBplusTag::PcheckDecay(genpart, 213, 22)) {
    return +1 * (100000 * m_nPhotos + 1088);
  }//B+ decays to rho+ gamma
  if (GenBplusTag::PcheckDecay(genpart, 20323, 22)) {
    return +1 * (100000 * m_nPhotos + 1089);
  }//B+ decays to K'_1+ gamma
  if (GenBplusTag::PcheckDecay(genpart, 100323, 22)) {
    return +1 * (100000 * m_nPhotos + 1090);
  }//B+ decays to K'*+ gamma
  if (GenBplusTag::PcheckDecay(genpart, 30323, 22)) {
    return +1 * (100000 * m_nPhotos + 1091);
  }//B+ decays to K''*+ gamma
  if (GenBplusTag::PcheckDecay(genpart, 30353, 22)) {
    return +1 * (100000 * m_nPhotos + 1092);
  }//B+ decays to Xsu gamma
  if (GenBplusTag::PcheckDecay(genpart, 321, -11, 11)) {
    return +1 * (100000 * m_nPhotos + 1093);
  }//B+ decays to K+ e+ e-
  if (GenBplusTag::PcheckDecay(genpart, 211, -11, 11)) {
    return +1 * (100000 * m_nPhotos + 1094);
  }//B+ decays to pi+ e+ e-
  if (GenBplusTag::PcheckDecay(genpart, 323, -11, 11)) {
    return +1 * (100000 * m_nPhotos + 1095);
  }//B+ decays to K*+ e+ e-
  if (GenBplusTag::PcheckDecay(genpart, 321, 211, -211, -11, 11)) {
    return +1 * (100000 * m_nPhotos + 1096);
  }//B+ decays to K+ pi+ pi- e+ e-
  if (GenBplusTag::PcheckDecay(genpart, 321, 111, 111, -11, 11)) {
    return +1 * (100000 * m_nPhotos + 1097);
  }//B+ decays to K+ pi0 pi0 e+ e-
  if (GenBplusTag::PcheckDecay(genpart, 321, 333, -11, 11)) {
    return +1 * (100000 * m_nPhotos + 1098);
  }//B+ decays to K+ phi e+ e-
  if (GenBplusTag::PcheckDecay(genpart, 30353, -11, 11)) {
    return +1 * (100000 * m_nPhotos + 1099);
  }//B+ decays to Xsu e+ e-
  if (GenBplusTag::PcheckDecay(genpart, 321, -13, 13)) {
    return +1 * (100000 * m_nPhotos + 1100);
  }//B+ decays to K+ mu+ mu-
  if (GenBplusTag::PcheckDecay(genpart, 211, -13, 13)) {
    return +1 * (100000 * m_nPhotos + 1101);
  }//B+ decays to pi+ mu+ mu-
  if (GenBplusTag::PcheckDecay(genpart, 323, -13, 13)) {
    return +1 * (100000 * m_nPhotos + 1102);
  }//B+ decays to K*+ mu+ mu-
  if (GenBplusTag::PcheckDecay(genpart, 321, 211, -211, -13, 13)) {
    return +1 * (100000 * m_nPhotos + 1103);
  }//B+ decays to K+ pi+ pi- mu+ mu-
  if (GenBplusTag::PcheckDecay(genpart, 321, 111, 111, -13, 13)) {
    return +1 * (100000 * m_nPhotos + 1104);
  }//B+ decays to K+ pi0 pi0 mu+ mu-
  if (GenBplusTag::PcheckDecay(genpart, 321, 333, -13, 13)) {
    return +1 * (100000 * m_nPhotos + 1105);
  }//B+ decays to K+ phi mu+ mu-
  if (GenBplusTag::PcheckDecay(genpart, 30353, -13, 13)) {
    return +1 * (100000 * m_nPhotos + 1106);
  }//B+ decays to Xsu mu+ mu-
  if (GenBplusTag::PcheckDecay(genpart, 321, -15, 15)) {
    return +1 * (100000 * m_nPhotos + 1107);
  }//B+ decays to K+ tau+ tau-
  if (GenBplusTag::PcheckDecay(genpart, 323, -15, 15)) {
    return +1 * (100000 * m_nPhotos + 1108);
  }//B+ decays to K*+ tau+ tau-
  if (GenBplusTag::PcheckDecay(genpart, 30353, -15, 15)) {
    return +1 * (100000 * m_nPhotos + 1109);
  }//B+ decays to Xsu tau+ tau-
  if (GenBplusTag::PcheckDecay(genpart, -15, 16)) {
    return +1 * (100000 * m_nPhotos + 1110);
  }//B+ decays to tau+ nu_tau
  if (GenBplusTag::PcheckDecay(genpart, -13, 14)) {
    return +1 * (100000 * m_nPhotos + 1111);
  }//B+ decays to mu+ nu_mu
  if (GenBplusTag::PcheckDecay(genpart, -11, 12)) {
    return +1 * (100000 * m_nPhotos + 1112);
  }//B+ decays to e+ nu_e
  if (GenBplusTag::PcheckDecay(genpart, 211, 111, 111)) {
    return +1 * (100000 * m_nPhotos + 1113);
  }//B+ decays to pi+ pi0 pi0
  if (GenBplusTag::PcheckDecay(genpart, 211, 211, -211)) {
    return +1 * (100000 * m_nPhotos + 1114);
  }//B+ decays to pi+ pi+ pi-
  if (GenBplusTag::PcheckDecay(genpart, 221, 211)) {
    return +1 * (100000 * m_nPhotos + 1115);
  }//B+ decays to eta pi+
  if (GenBplusTag::PcheckDecay(genpart, 221, 321)) {
    return +1 * (100000 * m_nPhotos + 1116);
  }//B+ decays to eta K+
  if (GenBplusTag::PcheckDecay(genpart, 323, 221)) {
    return +1 * (100000 * m_nPhotos + 1117);
  }//B+ decays to K*+ eta
  if (GenBplusTag::PcheckDecay(genpart, 213, 221)) {
    return +1 * (100000 * m_nPhotos + 1118);
  }//B+ decays to rho+ eta
  if (GenBplusTag::PcheckDecay(genpart, 331, 211)) {
    return +1 * (100000 * m_nPhotos + 1119);
  }//B+ decays to eta' pi+
  if (GenBplusTag::PcheckDecay(genpart, 331, 321)) {
    return +1 * (100000 * m_nPhotos + 1120);
  }//B+ decays to eta' K+
  if (GenBplusTag::PcheckDecay(genpart, 323, 331)) {
    return +1 * (100000 * m_nPhotos + 1121);
  }//B+ decays to K*+ eta'
  if (GenBplusTag::PcheckDecay(genpart, 213, 331)) {
    return +1 * (100000 * m_nPhotos + 1122);
  }//B+ decays to rho+ eta'
  if (GenBplusTag::PcheckDecay(genpart, 223, 211)) {
    return +1 * (100000 * m_nPhotos + 1123);
  }//B+ decays to omega pi+
  if (GenBplusTag::PcheckDecay(genpart, 223, 321)) {
    return +1 * (100000 * m_nPhotos + 1124);
  }//B+ decays to omega K+
  if (GenBplusTag::PcheckDecay(genpart, 223, 323)) {
    return +1 * (100000 * m_nPhotos + 1125);
  }//B+ decays to omega K*+
  if (GenBplusTag::PcheckDecay(genpart, 223, 213)) {
    return +1 * (100000 * m_nPhotos + 1126);
  }//B+ decays to omega rho+
  if (GenBplusTag::PcheckDecay(genpart, 333, 211)) {
    return +1 * (100000 * m_nPhotos + 1127);
  }//B+ decays to phi pi+
  if (GenBplusTag::PcheckDecay(genpart, 333, 321)) {
    return +1 * (100000 * m_nPhotos + 1128);
  }//B+ decays to phi K+
  if (GenBplusTag::PcheckDecay(genpart, 311, 211)) {
    return +1 * (100000 * m_nPhotos + 1129);
  }//B+ decays to K0 pi+
  if (GenBplusTag::PcheckDecay(genpart, 310, 211)) {
    return +1 * (100000 * m_nPhotos + 1130);
  }//B+ decays to K_S0 pi+
  if (GenBplusTag::PcheckDecay(genpart, 130, 211)) {
    return +1 * (100000 * m_nPhotos + 1131);
  }//B+ decays to K_L0 pi+
  if (GenBplusTag::PcheckDecay(genpart, -311, 321)) {
    return +1 * (100000 * m_nPhotos + 1132);
  }//B+ decays to anti-K0 K+
  if (GenBplusTag::PcheckDecay(genpart, 211, 111)) {
    return +1 * (100000 * m_nPhotos + 1133);
  }//B+ decays to pi+ pi0
  if (GenBplusTag::PcheckDecay(genpart, 321, 111)) {
    return +1 * (100000 * m_nPhotos + 1134);
  }//B+ decays to K+ pi0
  if (GenBplusTag::PcheckDecay(genpart, 113, 211)) {
    return +1 * (100000 * m_nPhotos + 1135);
  }//B+ decays to rho0 pi+
  if (GenBplusTag::PcheckDecay(genpart, 113, 321)) {
    return +1 * (100000 * m_nPhotos + 1136);
  }//B+ decays to rho0 K+
  if (GenBplusTag::PcheckDecay(genpart, 213, 111)) {
    return +1 * (100000 * m_nPhotos + 1137);
  }//B+ decays to rho+ pi0
  if (GenBplusTag::PcheckDecay(genpart, 213, 311)) {
    return +1 * (100000 * m_nPhotos + 1138);
  }//B+ decays to rho+ K0
  if (GenBplusTag::PcheckDecay(genpart, 100113, 211)) {
    return +1 * (100000 * m_nPhotos + 1139);
  }//B+ decays to rho(2S)0 pi+
  if (GenBplusTag::PcheckDecay(genpart, 100113, 321)) {
    return +1 * (100000 * m_nPhotos + 1140);
  }//B+ decays to rho(2S)0 K+
  if (GenBplusTag::PcheckDecay(genpart, 10221, 211)) {
    return +1 * (100000 * m_nPhotos + 1141);
  }//B+ decays to f_0 pi+
  if (GenBplusTag::PcheckDecay(genpart, 10221, 321)) {
    return +1 * (100000 * m_nPhotos + 1142);
  }//B+ decays to f_0 K+
  if (GenBplusTag::PcheckDecay(genpart, 10211, 311)) {
    return +1 * (100000 * m_nPhotos + 1143);
  }//B+ decays to a_0+ K0
  if (GenBplusTag::PcheckDecay(genpart, 10111, 321)) {
    return +1 * (100000 * m_nPhotos + 1144);
  }//B+ decays to a_00 K+
  if (GenBplusTag::PcheckDecay(genpart, 10111, 211)) {
    return +1 * (100000 * m_nPhotos + 1145);
  }//B+ decays to a_00 pi+
  if (GenBplusTag::PcheckDecay(genpart, 10211, 111)) {
    return +1 * (100000 * m_nPhotos + 1146);
  }//B+ decays to a_0+ pi0
  if (GenBplusTag::PcheckDecay(genpart, 225, 211)) {
    return +1 * (100000 * m_nPhotos + 1147);
  }//B+ decays to f_2 pi+
  if (GenBplusTag::PcheckDecay(genpart, 10311, 211)) {
    return +1 * (100000 * m_nPhotos + 1148);
  }//B+ decays to K_0*0 pi+
  if (GenBplusTag::PcheckDecay(genpart, -10311, 321)) {
    return +1 * (100000 * m_nPhotos + 1149);
  }//B+ decays to anti-K_0*0 K+
  if (GenBplusTag::PcheckDecay(genpart, 10321, 111)) {
    return +1 * (100000 * m_nPhotos + 1150);
  }//B+ decays to K_0*+ pi0
  if (GenBplusTag::PcheckDecay(genpart, 10321, -311)) {
    return +1 * (100000 * m_nPhotos + 1151);
  }//B+ decays to K_0*+ anti-K0
  if (GenBplusTag::PcheckDecay(genpart, 323, 111)) {
    return +1 * (100000 * m_nPhotos + 1152);
  }//B+ decays to K*+ pi0
  if (GenBplusTag::PcheckDecay(genpart, 323, -311)) {
    return +1 * (100000 * m_nPhotos + 1153);
  }//B+ decays to K*+ anti-K0
  if (GenBplusTag::PcheckDecay(genpart, 321, -211, 211)) {
    return +1 * (100000 * m_nPhotos + 1154);
  }//B+ decays to K+ pi- pi+
  if (GenBplusTag::PcheckDecay(genpart, 321, -321, 211)) {
    return +1 * (100000 * m_nPhotos + 1155);
  }//B+ decays to K+ K- pi+
  if (GenBplusTag::PcheckDecay(genpart, 321, -321, 321)) {
    return +1 * (100000 * m_nPhotos + 1156);
  }//B+ decays to K+ K- K+
  if (GenBplusTag::PcheckDecay(genpart, 321, 321, -211)) {
    return +1 * (100000 * m_nPhotos + 1157);
  }//B+ decays to K+ K+ pi-
  if (GenBplusTag::PcheckDecay(genpart, -321, 211, 211)) {
    return +1 * (100000 * m_nPhotos + 1158);
  }//B+ decays to K- pi+ pi+
  if (GenBplusTag::PcheckDecay(genpart, 321, -311, 111)) {
    return +1 * (100000 * m_nPhotos + 1159);
  }//B+ decays to K+ anti-K0 pi0
  if (GenBplusTag::PcheckDecay(genpart, 321, -311, 221)) {
    return +1 * (100000 * m_nPhotos + 1160);
  }//B+ decays to K+ anti-K0 eta
  if (GenBplusTag::PcheckDecay(genpart, 321, -311, 331)) {
    return +1 * (100000 * m_nPhotos + 1161);
  }//B+ decays to K+ anti-K0 eta'
  if (GenBplusTag::PcheckDecay(genpart, 211, 311, 111)) {
    return +1 * (100000 * m_nPhotos + 1162);
  }//B+ decays to pi+ K0 pi0
  if (GenBplusTag::PcheckDecay(genpart, 211, 311, 221)) {
    return +1 * (100000 * m_nPhotos + 1163);
  }//B+ decays to pi+ K0 eta
  if (GenBplusTag::PcheckDecay(genpart, 211, 311, 331)) {
    return +1 * (100000 * m_nPhotos + 1164);
  }//B+ decays to pi+ K0 eta'
  if (GenBplusTag::PcheckDecay(genpart, 211, 111, 221)) {
    return +1 * (100000 * m_nPhotos + 1165);
  }//B+ decays to pi+ pi0 eta
  if (GenBplusTag::PcheckDecay(genpart, 211, 111, 331)) {
    return +1 * (100000 * m_nPhotos + 1166);
  }//B+ decays to pi+ pi0 eta'
  if (GenBplusTag::PcheckDecay(genpart, 211, 221, 221)) {
    return +1 * (100000 * m_nPhotos + 1167);
  }//B+ decays to pi+ eta eta
  if (GenBplusTag::PcheckDecay(genpart, 211, 221, 331)) {
    return +1 * (100000 * m_nPhotos + 1168);
  }//B+ decays to pi+ eta eta'
  if (GenBplusTag::PcheckDecay(genpart, 321, 111, 111)) {
    return +1 * (100000 * m_nPhotos + 1169);
  }//B+ decays to K+ pi0 pi0
  if (GenBplusTag::PcheckDecay(genpart, 321, 111, 221)) {
    return +1 * (100000 * m_nPhotos + 1170);
  }//B+ decays to K+ pi0 eta
  if (GenBplusTag::PcheckDecay(genpart, 321, 111, 331)) {
    return +1 * (100000 * m_nPhotos + 1171);
  }//B+ decays to K+ pi0 eta'
  if (GenBplusTag::PcheckDecay(genpart, 321, 221, 221)) {
    return +1 * (100000 * m_nPhotos + 1172);
  }//B+ decays to K+ eta eta
  if (GenBplusTag::PcheckDecay(genpart, 321, 221, 331)) {
    return +1 * (100000 * m_nPhotos + 1173);
  }//B+ decays to K+ eta eta'
  if (GenBplusTag::PcheckDecay(genpart, 321, 331, 331)) {
    return +1 * (100000 * m_nPhotos + 1174);
  }//B+ decays to K+ eta' eta'
  if (GenBplusTag::PcheckDecay(genpart, 321, 311, -311)) {
    return +1 * (100000 * m_nPhotos + 1175);
  }//B+ decays to K+ K0 anti-K0
  if (GenBplusTag::PcheckDecay(genpart, 211, 311, -311)) {
    return +1 * (100000 * m_nPhotos + 1176);
  }//B+ decays to pi+ K0 anti-K0
  if (GenBplusTag::PcheckDecay(genpart, 213, 113)) {
    return +1 * (100000 * m_nPhotos + 1177);
  }//B+ decays to rho+ rho0
  if (GenBplusTag::PcheckDecay(genpart, 113, 211, 111)) {
    return +1 * (100000 * m_nPhotos + 1178);
  }//B+ decays to rho0 pi+ pi0
  if (GenBplusTag::PcheckDecay(genpart, 113, 211, 221)) {
    return +1 * (100000 * m_nPhotos + 1179);
  }//B+ decays to rho0 pi+ eta
  if (GenBplusTag::PcheckDecay(genpart, 113, 211, 331)) {
    return +1 * (100000 * m_nPhotos + 1180);
  }//B+ decays to rho0 pi+ eta'
  if (GenBplusTag::PcheckDecay(genpart, 223, 211, 111)) {
    return +1 * (100000 * m_nPhotos + 1181);
  }//B+ decays to omega pi+ pi0
  if (GenBplusTag::PcheckDecay(genpart, 223, 211, 221)) {
    return +1 * (100000 * m_nPhotos + 1182);
  }//B+ decays to omega pi+ eta
  if (GenBplusTag::PcheckDecay(genpart, 223, 211, 331)) {
    return +1 * (100000 * m_nPhotos + 1183);
  }//B+ decays to omega pi+ eta'
  if (GenBplusTag::PcheckDecay(genpart, 213, 211, -211)) {
    return +1 * (100000 * m_nPhotos + 1184);
  }//B+ decays to rho+ pi+ pi-
  if (GenBplusTag::PcheckDecay(genpart, -213, 211, 211)) {
    return +1 * (100000 * m_nPhotos + 1185);
  }//B+ decays to rho- pi+ pi+
  if (GenBplusTag::PcheckDecay(genpart, 213, 111, 111)) {
    return +1 * (100000 * m_nPhotos + 1186);
  }//B+ decays to rho+ pi0 pi0
  if (GenBplusTag::PcheckDecay(genpart, 213, 111, 221)) {
    return +1 * (100000 * m_nPhotos + 1187);
  }//B+ decays to rho+ pi0 eta
  if (GenBplusTag::PcheckDecay(genpart, 213, 221, 221)) {
    return +1 * (100000 * m_nPhotos + 1188);
  }//B+ decays to rho+ eta eta
  if (GenBplusTag::PcheckDecay(genpart, 213, 111, 331)) {
    return +1 * (100000 * m_nPhotos + 1189);
  }//B+ decays to rho+ pi0 eta'
  if (GenBplusTag::PcheckDecay(genpart, -211, 211, 211, 111)) {
    return +1 * (100000 * m_nPhotos + 1190);
  }//B+ decays to pi- pi+ pi+ pi0
  if (GenBplusTag::PcheckDecay(genpart, -211, 211, 211, 221)) {
    return +1 * (100000 * m_nPhotos + 1191);
  }//B+ decays to pi- pi+ pi+ eta
  if (GenBplusTag::PcheckDecay(genpart, -211, 211, 211, 331)) {
    return +1 * (100000 * m_nPhotos + 1192);
  }//B+ decays to pi- pi+ pi+ eta'
  if (GenBplusTag::PcheckDecay(genpart, 211, 111, 111, 111)) {
    return +1 * (100000 * m_nPhotos + 1193);
  }//B+ decays to pi+ pi0 pi0 pi0
  if (GenBplusTag::PcheckDecay(genpart, 211, 221, 111, 111)) {
    return +1 * (100000 * m_nPhotos + 1194);
  }//B+ decays to pi+ eta pi0 pi0
  if (GenBplusTag::PcheckDecay(genpart, 211, 221, 221, 111)) {
    return +1 * (100000 * m_nPhotos + 1195);
  }//B+ decays to pi+ eta eta pi0
  if (GenBplusTag::PcheckDecay(genpart, 211, 331, 111, 111)) {
    return +1 * (100000 * m_nPhotos + 1196);
  }//B+ decays to pi+ eta' pi0 pi0
  if (GenBplusTag::PcheckDecay(genpart, 211, 331, 221, 111)) {
    return +1 * (100000 * m_nPhotos + 1197);
  }//B+ decays to pi+ eta' eta pi0
  if (GenBplusTag::PcheckDecay(genpart, 20113, 211)) {
    return +1 * (100000 * m_nPhotos + 1198);
  }//B+ decays to a_10 pi+
  if (GenBplusTag::PcheckDecay(genpart, 20213, 111)) {
    return +1 * (100000 * m_nPhotos + 1199);
  }//B+ decays to a_1+ pi0
  if (GenBplusTag::PcheckDecay(genpart, 10113, 211)) {
    return +1 * (100000 * m_nPhotos + 1200);
  }//B+ decays to b_10 pi+
  if (GenBplusTag::PcheckDecay(genpart, 10213, 111)) {
    return +1 * (100000 * m_nPhotos + 1201);
  }//B+ decays to b_1+ pi0
  if (GenBplusTag::PcheckDecay(genpart, 213, 10221)) {
    return +1 * (100000 * m_nPhotos + 1202);
  }//B+ decays to rho+ f_0
  if (GenBplusTag::PcheckDecay(genpart, 213, 10111)) {
    return +1 * (100000 * m_nPhotos + 1203);
  }//B+ decays to rho+ a_00
  if (GenBplusTag::PcheckDecay(genpart, 113, 10211)) {
    return +1 * (100000 * m_nPhotos + 1204);
  }//B+ decays to rho0 a_0+
  if (GenBplusTag::PcheckDecay(genpart, 10221, 211, 111)) {
    return +1 * (100000 * m_nPhotos + 1205);
  }//B+ decays to f_0 pi+ pi0
  if (GenBplusTag::PcheckDecay(genpart, 10111, 211, 111)) {
    return +1 * (100000 * m_nPhotos + 1206);
  }//B+ decays to a_00 pi+ pi0
  if (GenBplusTag::PcheckDecay(genpart, 10211, 111, 111)) {
    return +1 * (100000 * m_nPhotos + 1207);
  }//B+ decays to a_0+ pi0 pi0
  if (GenBplusTag::PcheckDecay(genpart, -10211, 211, 211)) {
    return +1 * (100000 * m_nPhotos + 1208);
  }//B+ decays to a_0- pi+ pi+
  if (GenBplusTag::PcheckDecay(genpart, 213, 313)) {
    return +1 * (100000 * m_nPhotos + 1209);
  }//B+ decays to rho+ K*0
  if (GenBplusTag::PcheckDecay(genpart, 113, 323)) {
    return +1 * (100000 * m_nPhotos + 1210);
  }//B+ decays to rho0 K*+
  if (GenBplusTag::PcheckDecay(genpart, 211, 111, 313)) {
    return +1 * (100000 * m_nPhotos + 1211);
  }//B+ decays to pi+ pi0 K*0
  if (GenBplusTag::PcheckDecay(genpart, 211, 221, 313)) {
    return +1 * (100000 * m_nPhotos + 1212);
  }//B+ decays to pi+ eta K*0
  if (GenBplusTag::PcheckDecay(genpart, -211, 211, 323)) {
    return +1 * (100000 * m_nPhotos + 1213);
  }//B+ decays to pi- pi+ K*+
  if (GenBplusTag::PcheckDecay(genpart, 111, 111, 323)) {
    return +1 * (100000 * m_nPhotos + 1214);
  }//B+ decays to pi0 pi0 K*+
  if (GenBplusTag::PcheckDecay(genpart, 111, 221, 323)) {
    return +1 * (100000 * m_nPhotos + 1215);
  }//B+ decays to pi0 eta K*+
  if (GenBplusTag::PcheckDecay(genpart, 221, 221, 323)) {
    return +1 * (100000 * m_nPhotos + 1216);
  }//B+ decays to eta eta K*+
  if (GenBplusTag::PcheckDecay(genpart, 111, 331, 323)) {
    return +1 * (100000 * m_nPhotos + 1217);
  }//B+ decays to pi0 eta' K*+
  if (GenBplusTag::PcheckDecay(genpart, 221, 331, 323)) {
    return +1 * (100000 * m_nPhotos + 1218);
  }//B+ decays to eta eta' K*+
  if (GenBplusTag::PcheckDecay(genpart, 213, 311, 111)) {
    return +1 * (100000 * m_nPhotos + 1219);
  }//B+ decays to rho+ K0 pi0
  if (GenBplusTag::PcheckDecay(genpart, 213, 311, 221)) {
    return +1 * (100000 * m_nPhotos + 1220);
  }//B+ decays to rho+ K0 eta
  if (GenBplusTag::PcheckDecay(genpart, 213, 311, 331)) {
    return +1 * (100000 * m_nPhotos + 1221);
  }//B+ decays to rho+ K0 eta'
  if (GenBplusTag::PcheckDecay(genpart, 213, 321, -211)) {
    return +1 * (100000 * m_nPhotos + 1222);
  }//B+ decays to rho+ K+ pi-
  if (GenBplusTag::PcheckDecay(genpart, -213, 321, 211)) {
    return +1 * (100000 * m_nPhotos + 1223);
  }//B+ decays to rho- K+ pi+
  if (GenBplusTag::PcheckDecay(genpart, 113, 311, 211)) {
    return +1 * (100000 * m_nPhotos + 1224);
  }//B+ decays to rho0 K0 pi+
  if (GenBplusTag::PcheckDecay(genpart, 113, 321, 111)) {
    return +1 * (100000 * m_nPhotos + 1225);
  }//B+ decays to rho0 K+ pi0
  if (GenBplusTag::PcheckDecay(genpart, 113, 321, 221)) {
    return +1 * (100000 * m_nPhotos + 1226);
  }//B+ decays to rho0 K+ eta
  if (GenBplusTag::PcheckDecay(genpart, 113, 321, 331)) {
    return +1 * (100000 * m_nPhotos + 1227);
  }//B+ decays to rho0 K+ eta'
  if (GenBplusTag::PcheckDecay(genpart, 223, 311, 211)) {
    return +1 * (100000 * m_nPhotos + 1228);
  }//B+ decays to omega K0 pi+
  if (GenBplusTag::PcheckDecay(genpart, 223, 321, 111)) {
    return +1 * (100000 * m_nPhotos + 1229);
  }//B+ decays to omega K+ pi0
  if (GenBplusTag::PcheckDecay(genpart, 223, 321, 221)) {
    return +1 * (100000 * m_nPhotos + 1230);
  }//B+ decays to omega K+ eta
  if (GenBplusTag::PcheckDecay(genpart, 223, 321, 331)) {
    return +1 * (100000 * m_nPhotos + 1231);
  }//B+ decays to omega K+ eta'
  if (GenBplusTag::PcheckDecay(genpart, 211, 211, -211, 311)) {
    return +1 * (100000 * m_nPhotos + 1232);
  }//B+ decays to pi+ pi+ pi- K0
  if (GenBplusTag::PcheckDecay(genpart, 211, 111, 111, 311)) {
    return +1 * (100000 * m_nPhotos + 1233);
  }//B+ decays to pi+ pi0 pi0 K0
  if (GenBplusTag::PcheckDecay(genpart, 211, 221, 111, 311)) {
    return +1 * (100000 * m_nPhotos + 1234);
  }//B+ decays to pi+ eta pi0 K0
  if (GenBplusTag::PcheckDecay(genpart, 211, 221, 221, 311)) {
    return +1 * (100000 * m_nPhotos + 1235);
  }//B+ decays to pi+ eta eta K0
  if (GenBplusTag::PcheckDecay(genpart, 211, -211, 111, 321)) {
    return +1 * (100000 * m_nPhotos + 1236);
  }//B+ decays to pi+ pi- pi0 K+
  if (GenBplusTag::PcheckDecay(genpart, 211, -211, 221, 321)) {
    return +1 * (100000 * m_nPhotos + 1237);
  }//B+ decays to pi+ pi- eta K+
  if (GenBplusTag::PcheckDecay(genpart, 111, 111, 111, 321)) {
    return +1 * (100000 * m_nPhotos + 1238);
  }//B+ decays to pi0 pi0 pi0 K+
  if (GenBplusTag::PcheckDecay(genpart, 111, 111, 221, 321)) {
    return +1 * (100000 * m_nPhotos + 1239);
  }//B+ decays to pi0 pi0 eta K+
  if (GenBplusTag::PcheckDecay(genpart, 111, 221, 221, 321)) {
    return +1 * (100000 * m_nPhotos + 1240);
  }//B+ decays to pi0 eta eta K+
  if (GenBplusTag::PcheckDecay(genpart, 111, 111, 331, 321)) {
    return +1 * (100000 * m_nPhotos + 1241);
  }//B+ decays to pi0 pi0 eta' K+
  if (GenBplusTag::PcheckDecay(genpart, 111, 221, 331, 321)) {
    return +1 * (100000 * m_nPhotos + 1242);
  }//B+ decays to pi0 eta eta' K+
  if (GenBplusTag::PcheckDecay(genpart, 213, 10311)) {
    return +1 * (100000 * m_nPhotos + 1243);
  }//B+ decays to rho+ K_0*0
  if (GenBplusTag::PcheckDecay(genpart, 113, 10321)) {
    return +1 * (100000 * m_nPhotos + 1244);
  }//B+ decays to rho0 K_0*+
  if (GenBplusTag::PcheckDecay(genpart, 211, 111, 10311)) {
    return +1 * (100000 * m_nPhotos + 1245);
  }//B+ decays to pi+ pi0 K_0*0
  if (GenBplusTag::PcheckDecay(genpart, 111, 111, 10321)) {
    return +1 * (100000 * m_nPhotos + 1246);
  }//B+ decays to pi0 pi0 K_0*+
  if (GenBplusTag::PcheckDecay(genpart, 323, 10221)) {
    return +1 * (100000 * m_nPhotos + 1247);
  }//B+ decays to K*+ f_0
  if (GenBplusTag::PcheckDecay(genpart, 20113, 321)) {
    return +1 * (100000 * m_nPhotos + 1248);
  }//B+ decays to a_10 K+
  if (GenBplusTag::PcheckDecay(genpart, 20213, 311)) {
    return +1 * (100000 * m_nPhotos + 1249);
  }//B+ decays to a_1+ K0
  if (GenBplusTag::PcheckDecay(genpart, 10113, 321)) {
    return +1 * (100000 * m_nPhotos + 1250);
  }//B+ decays to b_10 K+
  if (GenBplusTag::PcheckDecay(genpart, 10213, 311)) {
    return +1 * (100000 * m_nPhotos + 1251);
  }//B+ decays to b_1+ K0
  if (GenBplusTag::PcheckDecay(genpart, 323, 10111)) {
    return +1 * (100000 * m_nPhotos + 1252);
  }//B+ decays to K*+ a_00
  if (GenBplusTag::PcheckDecay(genpart, 313, 10211)) {
    return +1 * (100000 * m_nPhotos + 1253);
  }//B+ decays to K*0 a_0+
  if (GenBplusTag::PcheckDecay(genpart, -313, 323)) {
    return +1 * (100000 * m_nPhotos + 1254);
  }//B+ decays to anti-K*0 K*+
  if (GenBplusTag::PcheckDecay(genpart, 333, 213)) {
    return +1 * (100000 * m_nPhotos + 1255);
  }//B+ decays to phi rho+
  if (GenBplusTag::PcheckDecay(genpart, 333, 211, 111)) {
    return +1 * (100000 * m_nPhotos + 1256);
  }//B+ decays to phi pi+ pi0
  if (GenBplusTag::PcheckDecay(genpart, 333, 211, 221)) {
    return +1 * (100000 * m_nPhotos + 1257);
  }//B+ decays to phi pi+ eta
  if (GenBplusTag::PcheckDecay(genpart, 333, 211, 331)) {
    return +1 * (100000 * m_nPhotos + 1258);
  }//B+ decays to phi pi+ eta'
  if (GenBplusTag::PcheckDecay(genpart, 321, -321, 213)) {
    return +1 * (100000 * m_nPhotos + 1259);
  }//B+ decays to K+ K- rho+
  if (GenBplusTag::PcheckDecay(genpart, 311, -311, 213)) {
    return +1 * (100000 * m_nPhotos + 1260);
  }//B+ decays to K0 anti-K0 rho+
  if (GenBplusTag::PcheckDecay(genpart, -311, 321, 113)) {
    return +1 * (100000 * m_nPhotos + 1261);
  }//B+ decays to anti-K0 K+ rho0
  if (GenBplusTag::PcheckDecay(genpart, -311, 321, 223)) {
    return +1 * (100000 * m_nPhotos + 1262);
  }//B+ decays to anti-K0 K+ omega
  if (GenBplusTag::PcheckDecay(genpart, 321, -321, 211, 111)) {
    return +1 * (100000 * m_nPhotos + 1263);
  }//B+ decays to K+ K- pi+ pi0
  if (GenBplusTag::PcheckDecay(genpart, 321, -321, 211, 221)) {
    return +1 * (100000 * m_nPhotos + 1264);
  }//B+ decays to K+ K- pi+ eta
  if (GenBplusTag::PcheckDecay(genpart, 321, -321, 211, 331)) {
    return +1 * (100000 * m_nPhotos + 1265);
  }//B+ decays to K+ K- pi+ eta'
  if (GenBplusTag::PcheckDecay(genpart, 311, -311, 211, 111)) {
    return +1 * (100000 * m_nPhotos + 1266);
  }//B+ decays to K0 anti-K0 pi+ pi0
  if (GenBplusTag::PcheckDecay(genpart, 311, -311, 211, 221)) {
    return +1 * (100000 * m_nPhotos + 1267);
  }//B+ decays to K0 anti-K0 pi+ eta
  if (GenBplusTag::PcheckDecay(genpart, 311, -311, 211, 331)) {
    return +1 * (100000 * m_nPhotos + 1268);
  }//B+ decays to K0 anti-K0 pi+ eta'
  if (GenBplusTag::PcheckDecay(genpart, -311, 321, 211, -211)) {
    return +1 * (100000 * m_nPhotos + 1269);
  }//B+ decays to anti-K0 K+ pi+ pi-
  if (GenBplusTag::PcheckDecay(genpart, -311, 321, 111, 111)) {
    return +1 * (100000 * m_nPhotos + 1270);
  }//B+ decays to anti-K0 K+ pi0 pi0
  if (GenBplusTag::PcheckDecay(genpart, -311, 321, 111, 221)) {
    return +1 * (100000 * m_nPhotos + 1271);
  }//B+ decays to anti-K0 K+ pi0 eta
  if (GenBplusTag::PcheckDecay(genpart, -311, 321, 221, 221)) {
    return +1 * (100000 * m_nPhotos + 1272);
  }//B+ decays to anti-K0 K+ eta eta
  if (GenBplusTag::PcheckDecay(genpart, -311, 321, 111, 331)) {
    return +1 * (100000 * m_nPhotos + 1273);
  }//B+ decays to anti-K0 K+ pi0 eta'
  if (GenBplusTag::PcheckDecay(genpart, -311, 321, 221, 331)) {
    return +1 * (100000 * m_nPhotos + 1274);
  }//B+ decays to anti-K0 K+ eta eta'
  if (GenBplusTag::PcheckDecay(genpart, 323, -321, 211)) {
    return +1 * (100000 * m_nPhotos + 1275);
  }//B+ decays to K*+ K- pi+
  if (GenBplusTag::PcheckDecay(genpart, -323, 321, 211)) {
    return +1 * (100000 * m_nPhotos + 1276);
  }//B+ decays to K*- K+ pi+
  if (GenBplusTag::PcheckDecay(genpart, 313, -311, 211)) {
    return +1 * (100000 * m_nPhotos + 1277);
  }//B+ decays to K*0 anti-K0 pi+
  if (GenBplusTag::PcheckDecay(genpart, 311, -313, 211)) {
    return +1 * (100000 * m_nPhotos + 1278);
  }//B+ decays to K0 anti-K*0 pi+
  if (GenBplusTag::PcheckDecay(genpart, 321, -313, 111)) {
    return +1 * (100000 * m_nPhotos + 1279);
  }//B+ decays to K+ anti-K*0 pi0
  if (GenBplusTag::PcheckDecay(genpart, 321, -313, 221)) {
    return +1 * (100000 * m_nPhotos + 1280);
  }//B+ decays to K+ anti-K*0 eta
  if (GenBplusTag::PcheckDecay(genpart, 321, -313, 331)) {
    return +1 * (100000 * m_nPhotos + 1281);
  }//B+ decays to K+ anti-K*0 eta'
  if (GenBplusTag::PcheckDecay(genpart, 323, -311, 111)) {
    return +1 * (100000 * m_nPhotos + 1282);
  }//B+ decays to K*+ anti-K0 pi0
  if (GenBplusTag::PcheckDecay(genpart, 323, -311, 221)) {
    return +1 * (100000 * m_nPhotos + 1283);
  }//B+ decays to K*+ anti-K0 eta
  if (GenBplusTag::PcheckDecay(genpart, 323, -311, 331)) {
    return +1 * (100000 * m_nPhotos + 1284);
  }//B+ decays to K*+ anti-K0 eta'
  if (GenBplusTag::PcheckDecay(genpart, 333, 323)) {
    return +1 * (100000 * m_nPhotos + 1285);
  }//B+ decays to phi K*+
  if (GenBplusTag::PcheckDecay(genpart, 333, 311, 211)) {
    return +1 * (100000 * m_nPhotos + 1286);
  }//B+ decays to phi K0 pi+
  if (GenBplusTag::PcheckDecay(genpart, 333, 321, 111)) {
    return +1 * (100000 * m_nPhotos + 1287);
  }//B+ decays to phi K+ pi0
  if (GenBplusTag::PcheckDecay(genpart, 333, 321, 221)) {
    return +1 * (100000 * m_nPhotos + 1288);
  }//B+ decays to phi K+ eta
  if (GenBplusTag::PcheckDecay(genpart, 333, 321, 331)) {
    return +1 * (100000 * m_nPhotos + 1289);
  }//B+ decays to phi K+ eta'
  if (GenBplusTag::PcheckDecay(genpart, -321, 321, 323)) {
    return +1 * (100000 * m_nPhotos + 1290);
  }//B+ decays to K- K+ K*+
  if (GenBplusTag::PcheckDecay(genpart, 321, -323, 321)) {
    return +1 * (100000 * m_nPhotos + 1291);
  }//B+ decays to K+ K*- K+
  if (GenBplusTag::PcheckDecay(genpart, 321, 313, -311)) {
    return +1 * (100000 * m_nPhotos + 1292);
  }//B+ decays to K+ K*0 anti-K0
  if (GenBplusTag::PcheckDecay(genpart, 321, 311, -313)) {
    return +1 * (100000 * m_nPhotos + 1293);
  }//B+ decays to K+ K0 anti-K*0
  if (GenBplusTag::PcheckDecay(genpart, 311, 323, -311)) {
    return +1 * (100000 * m_nPhotos + 1294);
  }//B+ decays to K0 K*+ anti-K0
  if (GenBplusTag::PcheckDecay(genpart, 321, -321, 321, 111)) {
    return +1 * (100000 * m_nPhotos + 1295);
  }//B+ decays to K+ K- K+ pi0
  if (GenBplusTag::PcheckDecay(genpart, 321, -321, 321, 221)) {
    return +1 * (100000 * m_nPhotos + 1296);
  }//B+ decays to K+ K- K+ eta
  if (GenBplusTag::PcheckDecay(genpart, 321, -321, 321, 331)) {
    return +1 * (100000 * m_nPhotos + 1297);
  }//B+ decays to K+ K- K+ eta'
  if (GenBplusTag::PcheckDecay(genpart, 321, -321, 311, 211)) {
    return +1 * (100000 * m_nPhotos + 1298);
  }//B+ decays to K+ K- K0 pi+
  if (GenBplusTag::PcheckDecay(genpart, 311, -311, 321, 111)) {
    return +1 * (100000 * m_nPhotos + 1299);
  }//B+ decays to K0 anti-K0 K+ pi0
  if (GenBplusTag::PcheckDecay(genpart, 311, -311, 321, 221)) {
    return +1 * (100000 * m_nPhotos + 1300);
  }//B+ decays to K0 anti-K0 K+ eta
  if (GenBplusTag::PcheckDecay(genpart, 311, -311, 321, 331)) {
    return +1 * (100000 * m_nPhotos + 1301);
  }//B+ decays to K0 anti-K0 K+ eta'
  if (GenBplusTag::PcheckDecay(genpart, 311, -311, 311, 211)) {
    return +1 * (100000 * m_nPhotos + 1302);
  }//B+ decays to K0 anti-K0 K0 pi+
  if (GenBplusTag::PcheckDecay(genpart, 333, 10321)) {
    return +1 * (100000 * m_nPhotos + 1303);
  }//B+ decays to phi K_0*+
  if (GenBplusTag::PcheckDecay(genpart, -321, 321, 10321)) {
    return +1 * (100000 * m_nPhotos + 1304);
  }//B+ decays to K- K+ K_0*+
  if (GenBplusTag::PcheckDecay(genpart, 321, 321, -10321)) {
    return +1 * (100000 * m_nPhotos + 1305);
  }//B+ decays to K+ K+ K_0*-
  if (GenBplusTag::PcheckDecay(genpart, 321, -10311, 311)) {
    return +1 * (100000 * m_nPhotos + 1306);
  }//B+ decays to K+ anti-K_0*0 K0
  if (GenBplusTag::PcheckDecay(genpart, 321, 10311, -311)) {
    return +1 * (100000 * m_nPhotos + 1307);
  }//B+ decays to K+ K_0*0 anti-K0
  if (GenBplusTag::PcheckDecay(genpart, 333, 20323)) {
    return +1 * (100000 * m_nPhotos + 1308);
  }//B+ decays to phi K'_1+
  if (GenBplusTag::PcheckDecay(genpart, -321, 321, 20323)) {
    return +1 * (100000 * m_nPhotos + 1309);
  }//B+ decays to K- K+ K'_1+
  if (GenBplusTag::PcheckDecay(genpart, 333, -311, 321)) {
    return +1 * (100000 * m_nPhotos + 1310);
  }//B+ decays to phi anti-K0 K+
  if (GenBplusTag::PcheckDecay(genpart, 321, -321, 321, -311)) {
    return +1 * (100000 * m_nPhotos + 1311);
  }//B+ decays to K+ K- K+ anti-K0
  if (GenBplusTag::PcheckDecay(genpart, 311, -311, -311, 321)) {
    return +1 * (100000 * m_nPhotos + 1312);
  }//B+ decays to K0 anti-K0 anti-K0 K+
  if (GenBplusTag::PcheckDecay(genpart, 113, 113, 211)) {
    return +1 * (100000 * m_nPhotos + 1313);
  }//B+ decays to rho0 rho0 pi+
  if (GenBplusTag::PcheckDecay(genpart, 213, 113, 111)) {
    return +1 * (100000 * m_nPhotos + 1314);
  }//B+ decays to rho+ rho0 pi0
  if (GenBplusTag::PcheckDecay(genpart, 213, 113, 221)) {
    return +1 * (100000 * m_nPhotos + 1315);
  }//B+ decays to rho+ rho0 eta
  if (GenBplusTag::PcheckDecay(genpart, 213, 113, 331)) {
    return +1 * (100000 * m_nPhotos + 1316);
  }//B+ decays to rho+ rho0 eta'
  if (GenBplusTag::PcheckDecay(genpart, 213, -213, 211)) {
    return +1 * (100000 * m_nPhotos + 1317);
  }//B+ decays to rho+ rho- pi+
  if (GenBplusTag::PcheckDecay(genpart, 223, 113, 211)) {
    return +1 * (100000 * m_nPhotos + 1318);
  }//B+ decays to omega rho0 pi+
  if (GenBplusTag::PcheckDecay(genpart, 223, 213, 111)) {
    return +1 * (100000 * m_nPhotos + 1319);
  }//B+ decays to omega rho+ pi0
  if (GenBplusTag::PcheckDecay(genpart, 223, 213, 221)) {
    return +1 * (100000 * m_nPhotos + 1320);
  }//B+ decays to omega rho+ eta
  if (GenBplusTag::PcheckDecay(genpart, 223, 213, 331)) {
    return +1 * (100000 * m_nPhotos + 1321);
  }//B+ decays to omega rho+ eta'
  if (GenBplusTag::PcheckDecay(genpart, 113, 211, -211, 211)) {
    return +1 * (100000 * m_nPhotos + 1322);
  }//B+ decays to rho0 pi+ pi- pi+
  if (GenBplusTag::PcheckDecay(genpart, 113, 211, 111, 111)) {
    return +1 * (100000 * m_nPhotos + 1323);
  }//B+ decays to rho0 pi+ pi0 pi0
  if (GenBplusTag::PcheckDecay(genpart, 113, 211, 221, 111)) {
    return +1 * (100000 * m_nPhotos + 1324);
  }//B+ decays to rho0 pi+ eta pi0
  if (GenBplusTag::PcheckDecay(genpart, 113, 211, 331, 111)) {
    return +1 * (100000 * m_nPhotos + 1325);
  }//B+ decays to rho0 pi+ eta' pi0
  if (GenBplusTag::PcheckDecay(genpart, 223, 211, 111, 111)) {
    return +1 * (100000 * m_nPhotos + 1326);
  }//B+ decays to omega pi+ pi0 pi0
  if (GenBplusTag::PcheckDecay(genpart, 223, 211, 221, 111)) {
    return +1 * (100000 * m_nPhotos + 1327);
  }//B+ decays to omega pi+ eta pi0
  if (GenBplusTag::PcheckDecay(genpart, 223, 211, 331, 111)) {
    return +1 * (100000 * m_nPhotos + 1328);
  }//B+ decays to omega pi+ eta' pi0
  if (GenBplusTag::PcheckDecay(genpart, -213, 211, 211, 111)) {
    return +1 * (100000 * m_nPhotos + 1329);
  }//B+ decays to rho- pi+ pi+ pi0
  if (GenBplusTag::PcheckDecay(genpart, -213, 211, 211, 221)) {
    return +1 * (100000 * m_nPhotos + 1330);
  }//B+ decays to rho- pi+ pi+ eta
  if (GenBplusTag::PcheckDecay(genpart, -213, 211, 211, 331)) {
    return +1 * (100000 * m_nPhotos + 1331);
  }//B+ decays to rho- pi+ pi+ eta'
  if (GenBplusTag::PcheckDecay(genpart, 213, 211, -211, 111)) {
    return +1 * (100000 * m_nPhotos + 1332);
  }//B+ decays to rho+ pi+ pi- pi0
  if (GenBplusTag::PcheckDecay(genpart, 213, 211, -211, 221)) {
    return +1 * (100000 * m_nPhotos + 1333);
  }//B+ decays to rho+ pi+ pi- eta
  if (GenBplusTag::PcheckDecay(genpart, 213, 211, -211, 331)) {
    return +1 * (100000 * m_nPhotos + 1334);
  }//B+ decays to rho+ pi+ pi- eta'
  if (GenBplusTag::PcheckDecay(genpart, 213, 111, 111, 111)) {
    return +1 * (100000 * m_nPhotos + 1335);
  }//B+ decays to rho+ pi0 pi0 pi0
  if (GenBplusTag::PcheckDecay(genpart, 213, 111, 221, 111)) {
    return +1 * (100000 * m_nPhotos + 1336);
  }//B+ decays to rho+ pi0 eta pi0
  if (GenBplusTag::PcheckDecay(genpart, 213, 221, 221, 111)) {
    return +1 * (100000 * m_nPhotos + 1337);
  }//B+ decays to rho+ eta eta pi0
  if (GenBplusTag::PcheckDecay(genpart, 213, 111, 331, 111)) {
    return +1 * (100000 * m_nPhotos + 1338);
  }//B+ decays to rho+ pi0 eta' pi0
  if (GenBplusTag::PcheckDecay(genpart, -211, 211, 211, 111, 111)) {
    return +1 * (100000 * m_nPhotos + 1339);
  }//B+ decays to pi- pi+ pi+ pi0 pi0
  if (GenBplusTag::PcheckDecay(genpart, -211, 211, 211, 221, 111)) {
    return +1 * (100000 * m_nPhotos + 1340);
  }//B+ decays to pi- pi+ pi+ eta pi0
  if (GenBplusTag::PcheckDecay(genpart, -211, 211, 211, 331, 111)) {
    return +1 * (100000 * m_nPhotos + 1341);
  }//B+ decays to pi- pi+ pi+ eta' pi0
  if (GenBplusTag::PcheckDecay(genpart, 211, 111, 111, 111, 111)) {
    return +1 * (100000 * m_nPhotos + 1342);
  }//B+ decays to pi+ pi0 pi0 pi0 pi0
  if (GenBplusTag::PcheckDecay(genpart, 211, 221, 111, 111, 111)) {
    return +1 * (100000 * m_nPhotos + 1343);
  }//B+ decays to pi+ eta pi0 pi0 pi0
  if (GenBplusTag::PcheckDecay(genpart, 211, 331, 111, 111, 111)) {
    return +1 * (100000 * m_nPhotos + 1344);
  }//B+ decays to pi+ eta' pi0 pi0 pi0
  if (GenBplusTag::PcheckDecay(genpart, 20213, 113)) {
    return +1 * (100000 * m_nPhotos + 1345);
  }//B+ decays to a_1+ rho0
  if (GenBplusTag::PcheckDecay(genpart, 20213, 223)) {
    return +1 * (100000 * m_nPhotos + 1346);
  }//B+ decays to a_1+ omega
  if (GenBplusTag::PcheckDecay(genpart, 20213, 10221)) {
    return +1 * (100000 * m_nPhotos + 1347);
  }//B+ decays to a_1+ f_0
  if (GenBplusTag::PcheckDecay(genpart, 20113, 211, 111)) {
    return +1 * (100000 * m_nPhotos + 1348);
  }//B+ decays to a_10 pi+ pi0
  if (GenBplusTag::PcheckDecay(genpart, 20113, 211, 221)) {
    return +1 * (100000 * m_nPhotos + 1349);
  }//B+ decays to a_10 pi+ eta
  if (GenBplusTag::PcheckDecay(genpart, 20113, 211, 331)) {
    return +1 * (100000 * m_nPhotos + 1350);
  }//B+ decays to a_10 pi+ eta'
  if (GenBplusTag::PcheckDecay(genpart, -20213, 211, 211)) {
    return +1 * (100000 * m_nPhotos + 1351);
  }//B+ decays to a_1- pi+ pi+
  if (GenBplusTag::PcheckDecay(genpart, 20213, 211, -211)) {
    return +1 * (100000 * m_nPhotos + 1352);
  }//B+ decays to a_1+ pi+ pi-
  if (GenBplusTag::PcheckDecay(genpart, 20213, 111, 111)) {
    return +1 * (100000 * m_nPhotos + 1353);
  }//B+ decays to a_1+ pi0 pi0
  if (GenBplusTag::PcheckDecay(genpart, 20213, 111, 221)) {
    return +1 * (100000 * m_nPhotos + 1354);
  }//B+ decays to a_1+ pi0 eta
  if (GenBplusTag::PcheckDecay(genpart, 20213, 111, 331)) {
    return +1 * (100000 * m_nPhotos + 1355);
  }//B+ decays to a_1+ pi0 eta'
  if (GenBplusTag::PcheckDecay(genpart, 213, 10221, 111)) {
    return +1 * (100000 * m_nPhotos + 1356);
  }//B+ decays to rho+ f_0 pi0
  if (GenBplusTag::PcheckDecay(genpart, 213, 10221, 221)) {
    return +1 * (100000 * m_nPhotos + 1357);
  }//B+ decays to rho+ f_0 eta
  if (GenBplusTag::PcheckDecay(genpart, 213, 10221, 331)) {
    return +1 * (100000 * m_nPhotos + 1358);
  }//B+ decays to rho+ f_0 eta'
  if (GenBplusTag::PcheckDecay(genpart, 113, 10221, 211)) {
    return +1 * (100000 * m_nPhotos + 1359);
  }//B+ decays to rho0 f_0 pi+
  if (GenBplusTag::PcheckDecay(genpart, 223, 10221, 211)) {
    return +1 * (100000 * m_nPhotos + 1360);
  }//B+ decays to omega f_0 pi+
  if (GenBplusTag::PcheckDecay(genpart, 213, 10111, 111)) {
    return +1 * (100000 * m_nPhotos + 1361);
  }//B+ decays to rho+ a_00 pi0
  if (GenBplusTag::PcheckDecay(genpart, 213, 10111, 221)) {
    return +1 * (100000 * m_nPhotos + 1362);
  }//B+ decays to rho+ a_00 eta
  if (GenBplusTag::PcheckDecay(genpart, 213, 10111, 331)) {
    return +1 * (100000 * m_nPhotos + 1363);
  }//B+ decays to rho+ a_00 eta'
  if (GenBplusTag::PcheckDecay(genpart, -213, 10211, 211)) {
    return +1 * (100000 * m_nPhotos + 1364);
  }//B+ decays to rho- a_0+ pi+
  if (GenBplusTag::PcheckDecay(genpart, 113, 10211, 111)) {
    return +1 * (100000 * m_nPhotos + 1365);
  }//B+ decays to rho0 a_0+ pi0
  if (GenBplusTag::PcheckDecay(genpart, 113, 10211, 221)) {
    return +1 * (100000 * m_nPhotos + 1366);
  }//B+ decays to rho0 a_0+ eta
  if (GenBplusTag::PcheckDecay(genpart, 113, 10211, 331)) {
    return +1 * (100000 * m_nPhotos + 1367);
  }//B+ decays to rho0 a_0+ eta'
  if (GenBplusTag::PcheckDecay(genpart, 10221, 211, -211, 211)) {
    return +1 * (100000 * m_nPhotos + 1368);
  }//B+ decays to f_0 pi+ pi- pi+
  if (GenBplusTag::PcheckDecay(genpart, 10221, 211, 111, 111)) {
    return +1 * (100000 * m_nPhotos + 1369);
  }//B+ decays to f_0 pi+ pi0 pi0
  if (GenBplusTag::PcheckDecay(genpart, 10221, 211, 111, 221)) {
    return +1 * (100000 * m_nPhotos + 1370);
  }//B+ decays to f_0 pi+ pi0 eta
  if (GenBplusTag::PcheckDecay(genpart, 10221, 211, 111, 331)) {
    return +1 * (100000 * m_nPhotos + 1371);
  }//B+ decays to f_0 pi+ pi0 eta'
  if (GenBplusTag::PcheckDecay(genpart, 10111, 211, 111, 111)) {
    return +1 * (100000 * m_nPhotos + 1372);
  }//B+ decays to a_00 pi+ pi0 pi0
  if (GenBplusTag::PcheckDecay(genpart, 10211, 111, 111, 111)) {
    return +1 * (100000 * m_nPhotos + 1373);
  }//B+ decays to a_0+ pi0 pi0 pi0
  if (GenBplusTag::PcheckDecay(genpart, -10211, 211, 211, 111)) {
    return +1 * (100000 * m_nPhotos + 1374);
  }//B+ decays to a_0- pi+ pi+ pi0
  if (GenBplusTag::PcheckDecay(genpart, 213, 323, -211)) {
    return +1 * (100000 * m_nPhotos + 1375);
  }//B+ decays to rho+ K*+ pi-
  if (GenBplusTag::PcheckDecay(genpart, -213, 323, 211)) {
    return +1 * (100000 * m_nPhotos + 1376);
  }//B+ decays to rho- K*+ pi+
  if (GenBplusTag::PcheckDecay(genpart, 213, 313, 111)) {
    return +1 * (100000 * m_nPhotos + 1377);
  }//B+ decays to rho+ K*0 pi0
  if (GenBplusTag::PcheckDecay(genpart, 213, 313, 221)) {
    return +1 * (100000 * m_nPhotos + 1378);
  }//B+ decays to rho+ K*0 eta
  if (GenBplusTag::PcheckDecay(genpart, 213, 313, 331)) {
    return +1 * (100000 * m_nPhotos + 1379);
  }//B+ decays to rho+ K*0 eta'
  if (GenBplusTag::PcheckDecay(genpart, 113, 313, 211)) {
    return +1 * (100000 * m_nPhotos + 1380);
  }//B+ decays to rho0 K*0 pi+
  if (GenBplusTag::PcheckDecay(genpart, 113, 323, 111)) {
    return +1 * (100000 * m_nPhotos + 1381);
  }//B+ decays to rho0 K*+ pi0
  if (GenBplusTag::PcheckDecay(genpart, 113, 323, 221)) {
    return +1 * (100000 * m_nPhotos + 1382);
  }//B+ decays to rho0 K*+ eta
  if (GenBplusTag::PcheckDecay(genpart, 113, 323, 331)) {
    return +1 * (100000 * m_nPhotos + 1383);
  }//B+ decays to rho0 K*+ eta'
  if (GenBplusTag::PcheckDecay(genpart, 223, 313, 211)) {
    return +1 * (100000 * m_nPhotos + 1384);
  }//B+ decays to omega K*0 pi+
  if (GenBplusTag::PcheckDecay(genpart, 223, 323, 111)) {
    return +1 * (100000 * m_nPhotos + 1385);
  }//B+ decays to omega K*+ pi0
  if (GenBplusTag::PcheckDecay(genpart, 223, 323, 221)) {
    return +1 * (100000 * m_nPhotos + 1386);
  }//B+ decays to omega K*+ eta
  if (GenBplusTag::PcheckDecay(genpart, 223, 323, 331)) {
    return +1 * (100000 * m_nPhotos + 1387);
  }//B+ decays to omega K*+ eta'
  if (GenBplusTag::PcheckDecay(genpart, 211, -211, 313, 211)) {
    return +1 * (100000 * m_nPhotos + 1388);
  }//B+ decays to pi+ pi- K*0 pi+
  if (GenBplusTag::PcheckDecay(genpart, 211, 111, 313, 111)) {
    return +1 * (100000 * m_nPhotos + 1389);
  }//B+ decays to pi+ pi0 K*0 pi0
  if (GenBplusTag::PcheckDecay(genpart, 211, 221, 313, 111)) {
    return +1 * (100000 * m_nPhotos + 1390);
  }//B+ decays to pi+ eta K*0 pi0
  if (GenBplusTag::PcheckDecay(genpart, 211, 331, 313, 111)) {
    return +1 * (100000 * m_nPhotos + 1391);
  }//B+ decays to pi+ eta' K*0 pi0
  if (GenBplusTag::PcheckDecay(genpart, -211, 211, 323, 111)) {
    return +1 * (100000 * m_nPhotos + 1392);
  }//B+ decays to pi- pi+ K*+ pi0
  if (GenBplusTag::PcheckDecay(genpart, -211, 211, 323, 221)) {
    return +1 * (100000 * m_nPhotos + 1393);
  }//B+ decays to pi- pi+ K*+ eta
  if (GenBplusTag::PcheckDecay(genpart, -211, 211, 323, 331)) {
    return +1 * (100000 * m_nPhotos + 1394);
  }//B+ decays to pi- pi+ K*+ eta'
  if (GenBplusTag::PcheckDecay(genpart, 111, 111, 323, 111)) {
    return +1 * (100000 * m_nPhotos + 1395);
  }//B+ decays to pi0 pi0 K*+ pi0
  if (GenBplusTag::PcheckDecay(genpart, 111, 221, 323, 111)) {
    return +1 * (100000 * m_nPhotos + 1396);
  }//B+ decays to pi0 eta K*+ pi0
  if (GenBplusTag::PcheckDecay(genpart, 221, 221, 323, 111)) {
    return +1 * (100000 * m_nPhotos + 1397);
  }//B+ decays to eta eta K*+ pi0
  if (GenBplusTag::PcheckDecay(genpart, 111, 331, 323, 111)) {
    return +1 * (100000 * m_nPhotos + 1398);
  }//B+ decays to pi0 eta' K*+ pi0
  if (GenBplusTag::PcheckDecay(genpart, 221, 331, 323, 111)) {
    return +1 * (100000 * m_nPhotos + 1399);
  }//B+ decays to eta eta' K*+ pi0
  if (GenBplusTag::PcheckDecay(genpart, 213, 311, 211, -211)) {
    return +1 * (100000 * m_nPhotos + 1400);
  }//B+ decays to rho+ K0 pi+ pi-
  if (GenBplusTag::PcheckDecay(genpart, 213, 311, 111, 111)) {
    return +1 * (100000 * m_nPhotos + 1401);
  }//B+ decays to rho+ K0 pi0 pi0
  if (GenBplusTag::PcheckDecay(genpart, 213, 311, 221, 111)) {
    return +1 * (100000 * m_nPhotos + 1402);
  }//B+ decays to rho+ K0 eta pi0
  if (GenBplusTag::PcheckDecay(genpart, 213, 311, 331, 111)) {
    return +1 * (100000 * m_nPhotos + 1403);
  }//B+ decays to rho+ K0 eta' pi0
  if (GenBplusTag::PcheckDecay(genpart, 213, 321, -211, 111)) {
    return +1 * (100000 * m_nPhotos + 1404);
  }//B+ decays to rho+ K+ pi- pi0
  if (GenBplusTag::PcheckDecay(genpart, -213, 321, 211, 111)) {
    return +1 * (100000 * m_nPhotos + 1405);
  }//B+ decays to rho- K+ pi+ pi0
  if (GenBplusTag::PcheckDecay(genpart, -213, 311, 211, 211)) {
    return +1 * (100000 * m_nPhotos + 1406);
  }//B+ decays to rho- K0 pi+ pi+
  if (GenBplusTag::PcheckDecay(genpart, 113, 311, 211, 111)) {
    return +1 * (100000 * m_nPhotos + 1407);
  }//B+ decays to rho0 K0 pi+ pi0
  if (GenBplusTag::PcheckDecay(genpart, 113, 321, 111, 111)) {
    return +1 * (100000 * m_nPhotos + 1408);
  }//B+ decays to rho0 K+ pi0 pi0
  if (GenBplusTag::PcheckDecay(genpart, 113, 321, 221, 111)) {
    return +1 * (100000 * m_nPhotos + 1409);
  }//B+ decays to rho0 K+ eta pi0
  if (GenBplusTag::PcheckDecay(genpart, 113, 321, 331, 111)) {
    return +1 * (100000 * m_nPhotos + 1410);
  }//B+ decays to rho0 K+ eta' pi0
  if (GenBplusTag::PcheckDecay(genpart, 223, 311, 211, 111)) {
    return +1 * (100000 * m_nPhotos + 1411);
  }//B+ decays to omega K0 pi+ pi0
  if (GenBplusTag::PcheckDecay(genpart, 223, 321, 111, 111)) {
    return +1 * (100000 * m_nPhotos + 1412);
  }//B+ decays to omega K+ pi0 pi0
  if (GenBplusTag::PcheckDecay(genpart, 223, 321, 221, 111)) {
    return +1 * (100000 * m_nPhotos + 1413);
  }//B+ decays to omega K+ eta pi0
  if (GenBplusTag::PcheckDecay(genpart, 223, 321, 331, 111)) {
    return +1 * (100000 * m_nPhotos + 1414);
  }//B+ decays to omega K+ eta' pi0
  if (GenBplusTag::PcheckDecay(genpart, 211, 211, -211, 311, 111)) {
    return +1 * (100000 * m_nPhotos + 1415);
  }//B+ decays to pi+ pi+ pi- K0 pi0
  if (GenBplusTag::PcheckDecay(genpart, 211, 211, -211, 311, 221)) {
    return +1 * (100000 * m_nPhotos + 1416);
  }//B+ decays to pi+ pi+ pi- K0 eta
  if (GenBplusTag::PcheckDecay(genpart, 211, 211, -211, 311, 331)) {
    return +1 * (100000 * m_nPhotos + 1417);
  }//B+ decays to pi+ pi+ pi- K0 eta'
  if (GenBplusTag::PcheckDecay(genpart, 211, 111, 111, 311, 111)) {
    return +1 * (100000 * m_nPhotos + 1418);
  }//B+ decays to pi+ pi0 pi0 K0 pi0
  if (GenBplusTag::PcheckDecay(genpart, 211, 221, 111, 311, 111)) {
    return +1 * (100000 * m_nPhotos + 1419);
  }//B+ decays to pi+ eta pi0 K0 pi0
  if (GenBplusTag::PcheckDecay(genpart, 211, 221, 221, 311, 111)) {
    return +1 * (100000 * m_nPhotos + 1420);
  }//B+ decays to pi+ eta eta K0 pi0
  if (GenBplusTag::PcheckDecay(genpart, 211, -211, 211, 321, -211)) {
    return +1 * (100000 * m_nPhotos + 1421);
  }//B+ decays to pi+ pi- pi+ K+ pi-
  if (GenBplusTag::PcheckDecay(genpart, 211, -211, 111, 321, 111)) {
    return +1 * (100000 * m_nPhotos + 1422);
  }//B+ decays to pi+ pi- pi0 K+ pi0
  if (GenBplusTag::PcheckDecay(genpart, 211, -211, 221, 321, 111)) {
    return +1 * (100000 * m_nPhotos + 1423);
  }//B+ decays to pi+ pi- eta K+ pi0
  if (GenBplusTag::PcheckDecay(genpart, 211, -211, 331, 321, 111)) {
    return +1 * (100000 * m_nPhotos + 1424);
  }//B+ decays to pi+ pi- eta' K+ pi0
  if (GenBplusTag::PcheckDecay(genpart, 111, 111, 111, 321, 111)) {
    return +1 * (100000 * m_nPhotos + 1425);
  }//B+ decays to pi0 pi0 pi0 K+ pi0
  if (GenBplusTag::PcheckDecay(genpart, 111, 111, 221, 321, 111)) {
    return +1 * (100000 * m_nPhotos + 1426);
  }//B+ decays to pi0 pi0 eta K+ pi0
  if (GenBplusTag::PcheckDecay(genpart, 111, 221, 221, 321, 111)) {
    return +1 * (100000 * m_nPhotos + 1427);
  }//B+ decays to pi0 eta eta K+ pi0
  if (GenBplusTag::PcheckDecay(genpart, 111, 111, 331, 321, 111)) {
    return +1 * (100000 * m_nPhotos + 1428);
  }//B+ decays to pi0 pi0 eta' K+ pi0
  if (GenBplusTag::PcheckDecay(genpart, 111, 221, 331, 321, 111)) {
    return +1 * (100000 * m_nPhotos + 1429);
  }//B+ decays to pi0 eta eta' K+ pi0
  if (GenBplusTag::PcheckDecay(genpart, 113, 10311, 211)) {
    return +1 * (100000 * m_nPhotos + 1430);
  }//B+ decays to rho0 K_0*0 pi+
  if (GenBplusTag::PcheckDecay(genpart, 213, 10311, 111)) {
    return +1 * (100000 * m_nPhotos + 1431);
  }//B+ decays to rho+ K_0*0 pi0
  if (GenBplusTag::PcheckDecay(genpart, 213, 10311, 221)) {
    return +1 * (100000 * m_nPhotos + 1432);
  }//B+ decays to rho+ K_0*0 eta
  if (GenBplusTag::PcheckDecay(genpart, 213, 10311, 331)) {
    return +1 * (100000 * m_nPhotos + 1433);
  }//B+ decays to rho+ K_0*0 eta'
  if (GenBplusTag::PcheckDecay(genpart, 213, 10321, -211)) {
    return +1 * (100000 * m_nPhotos + 1434);
  }//B+ decays to rho+ K_0*+ pi-
  if (GenBplusTag::PcheckDecay(genpart, 113, 10321, 111)) {
    return +1 * (100000 * m_nPhotos + 1435);
  }//B+ decays to rho0 K_0*+ pi0
  if (GenBplusTag::PcheckDecay(genpart, 113, 10321, 221)) {
    return +1 * (100000 * m_nPhotos + 1436);
  }//B+ decays to rho0 K_0*+ eta
  if (GenBplusTag::PcheckDecay(genpart, 113, 10321, 331)) {
    return +1 * (100000 * m_nPhotos + 1437);
  }//B+ decays to rho0 K_0*+ eta'
  if (GenBplusTag::PcheckDecay(genpart, 211, 211, 10311, -211)) {
    return +1 * (100000 * m_nPhotos + 1438);
  }//B+ decays to pi+ pi+ K_0*0 pi-
  if (GenBplusTag::PcheckDecay(genpart, 211, 111, 10311, 111)) {
    return +1 * (100000 * m_nPhotos + 1439);
  }//B+ decays to pi+ pi0 K_0*0 pi0
  if (GenBplusTag::PcheckDecay(genpart, 211, 111, 10311, 221)) {
    return +1 * (100000 * m_nPhotos + 1440);
  }//B+ decays to pi+ pi0 K_0*0 eta
  if (GenBplusTag::PcheckDecay(genpart, 211, 111, 10311, 331)) {
    return +1 * (100000 * m_nPhotos + 1441);
  }//B+ decays to pi+ pi0 K_0*0 eta'
  if (GenBplusTag::PcheckDecay(genpart, 111, 111, 10321, 111)) {
    return +1 * (100000 * m_nPhotos + 1442);
  }//B+ decays to pi0 pi0 K_0*+ pi0
  if (GenBplusTag::PcheckDecay(genpart, 111, 111, 10321, 221)) {
    return +1 * (100000 * m_nPhotos + 1443);
  }//B+ decays to pi0 pi0 K_0*+ eta
  if (GenBplusTag::PcheckDecay(genpart, 111, 111, 10321, 331)) {
    return +1 * (100000 * m_nPhotos + 1444);
  }//B+ decays to pi0 pi0 K_0*+ eta'
  if (GenBplusTag::PcheckDecay(genpart, 313, 10221, 211)) {
    return +1 * (100000 * m_nPhotos + 1445);
  }//B+ decays to K*0 f_0 pi+
  if (GenBplusTag::PcheckDecay(genpart, 323, 10221, 111)) {
    return +1 * (100000 * m_nPhotos + 1446);
  }//B+ decays to K*+ f_0 pi0
  if (GenBplusTag::PcheckDecay(genpart, 323, 10221, 221)) {
    return +1 * (100000 * m_nPhotos + 1447);
  }//B+ decays to K*+ f_0 eta
  if (GenBplusTag::PcheckDecay(genpart, 323, 10221, 331)) {
    return +1 * (100000 * m_nPhotos + 1448);
  }//B+ decays to K*+ f_0 eta'
  if (GenBplusTag::PcheckDecay(genpart, 20113, 323)) {
    return +1 * (100000 * m_nPhotos + 1449);
  }//B+ decays to a_10 K*+
  if (GenBplusTag::PcheckDecay(genpart, 20113, 321, 111)) {
    return +1 * (100000 * m_nPhotos + 1450);
  }//B+ decays to a_10 K+ pi0
  if (GenBplusTag::PcheckDecay(genpart, 20113, 311, 211)) {
    return +1 * (100000 * m_nPhotos + 1451);
  }//B+ decays to a_10 K0 pi+
  if (GenBplusTag::PcheckDecay(genpart, 20213, 313)) {
    return +1 * (100000 * m_nPhotos + 1452);
  }//B+ decays to a_1+ K*0
  if (GenBplusTag::PcheckDecay(genpart, 20213, 321, -211)) {
    return +1 * (100000 * m_nPhotos + 1453);
  }//B+ decays to a_1+ K+ pi-
  if (GenBplusTag::PcheckDecay(genpart, 20213, 311, 111)) {
    return +1 * (100000 * m_nPhotos + 1454);
  }//B+ decays to a_1+ K0 pi0
  if (GenBplusTag::PcheckDecay(genpart, 323, 10111, 111)) {
    return +1 * (100000 * m_nPhotos + 1455);
  }//B+ decays to K*+ a_00 pi0
  if (GenBplusTag::PcheckDecay(genpart, 313, 10211, 111)) {
    return +1 * (100000 * m_nPhotos + 1456);
  }//B+ decays to K*0 a_0+ pi0
  if (GenBplusTag::PcheckDecay(genpart, 313, -313, 211)) {
    return +1 * (100000 * m_nPhotos + 1457);
  }//B+ decays to K*0 anti-K*0 pi+
  if (GenBplusTag::PcheckDecay(genpart, -313, 323, 111)) {
    return +1 * (100000 * m_nPhotos + 1458);
  }//B+ decays to anti-K*0 K*+ pi0
  if (GenBplusTag::PcheckDecay(genpart, -313, 323, 221)) {
    return +1 * (100000 * m_nPhotos + 1459);
  }//B+ decays to anti-K*0 K*+ eta
  if (GenBplusTag::PcheckDecay(genpart, -313, 323, 331)) {
    return +1 * (100000 * m_nPhotos + 1460);
  }//B+ decays to anti-K*0 K*+ eta'
  if (GenBplusTag::PcheckDecay(genpart, 333, 113, 211)) {
    return +1 * (100000 * m_nPhotos + 1461);
  }//B+ decays to phi rho0 pi+
  if (GenBplusTag::PcheckDecay(genpart, 333, 223, 211)) {
    return +1 * (100000 * m_nPhotos + 1462);
  }//B+ decays to phi omega pi+
  if (GenBplusTag::PcheckDecay(genpart, 333, 10221, 211)) {
    return +1 * (100000 * m_nPhotos + 1463);
  }//B+ decays to phi f_0 pi+
  if (GenBplusTag::PcheckDecay(genpart, 333, 213, 111)) {
    return +1 * (100000 * m_nPhotos + 1464);
  }//B+ decays to phi rho+ pi0
  if (GenBplusTag::PcheckDecay(genpart, 333, 213, 221)) {
    return +1 * (100000 * m_nPhotos + 1465);
  }//B+ decays to phi rho+ eta
  if (GenBplusTag::PcheckDecay(genpart, 333, 213, 331)) {
    return +1 * (100000 * m_nPhotos + 1466);
  }//B+ decays to phi rho+ eta'
  if (GenBplusTag::PcheckDecay(genpart, 333, 211, -211, 211)) {
    return +1 * (100000 * m_nPhotos + 1467);
  }//B+ decays to phi pi+ pi- pi+
  if (GenBplusTag::PcheckDecay(genpart, 333, 211, 111, 111)) {
    return +1 * (100000 * m_nPhotos + 1468);
  }//B+ decays to phi pi+ pi0 pi0
  if (GenBplusTag::PcheckDecay(genpart, 333, 211, 221, 111)) {
    return +1 * (100000 * m_nPhotos + 1469);
  }//B+ decays to phi pi+ eta pi0
  if (GenBplusTag::PcheckDecay(genpart, 333, 211, 331, 111)) {
    return +1 * (100000 * m_nPhotos + 1470);
  }//B+ decays to phi pi+ eta' pi0
  if (GenBplusTag::PcheckDecay(genpart, -311, 321, 213, -211)) {
    return +1 * (100000 * m_nPhotos + 1471);
  }//B+ decays to anti-K0 K+ rho+ pi-
  if (GenBplusTag::PcheckDecay(genpart, -311, 321, -213, 211)) {
    return +1 * (100000 * m_nPhotos + 1472);
  }//B+ decays to anti-K0 K+ rho- pi+
  if (GenBplusTag::PcheckDecay(genpart, -311, 321, 113, 111)) {
    return +1 * (100000 * m_nPhotos + 1473);
  }//B+ decays to anti-K0 K+ rho0 pi0
  if (GenBplusTag::PcheckDecay(genpart, -311, 321, 113, 221)) {
    return +1 * (100000 * m_nPhotos + 1474);
  }//B+ decays to anti-K0 K+ rho0 eta
  if (GenBplusTag::PcheckDecay(genpart, -311, 321, 113, 331)) {
    return +1 * (100000 * m_nPhotos + 1475);
  }//B+ decays to anti-K0 K+ rho0 eta'
  if (GenBplusTag::PcheckDecay(genpart, -311, 321, 223, 111)) {
    return +1 * (100000 * m_nPhotos + 1476);
  }//B+ decays to anti-K0 K+ omega pi0
  if (GenBplusTag::PcheckDecay(genpart, -311, 321, 10221, 111)) {
    return +1 * (100000 * m_nPhotos + 1477);
  }//B+ decays to anti-K0 K+ f_0 pi0
  if (GenBplusTag::PcheckDecay(genpart, 321, -321, 113, 211)) {
    return +1 * (100000 * m_nPhotos + 1478);
  }//B+ decays to K+ K- rho0 pi+
  if (GenBplusTag::PcheckDecay(genpart, 321, -321, 213, 111)) {
    return +1 * (100000 * m_nPhotos + 1479);
  }//B+ decays to K+ K- rho+ pi0
  if (GenBplusTag::PcheckDecay(genpart, 321, -321, 213, 221)) {
    return +1 * (100000 * m_nPhotos + 1480);
  }//B+ decays to K+ K- rho+ eta
  if (GenBplusTag::PcheckDecay(genpart, 321, -321, 213, 331)) {
    return +1 * (100000 * m_nPhotos + 1481);
  }//B+ decays to K+ K- rho+ eta'
  if (GenBplusTag::PcheckDecay(genpart, 321, -321, 223, 211)) {
    return +1 * (100000 * m_nPhotos + 1482);
  }//B+ decays to K+ K- omega pi+
  if (GenBplusTag::PcheckDecay(genpart, 311, -311, 113, 211)) {
    return +1 * (100000 * m_nPhotos + 1483);
  }//B+ decays to K0 anti-K0 rho0 pi+
  if (GenBplusTag::PcheckDecay(genpart, 311, -311, 223, 211)) {
    return +1 * (100000 * m_nPhotos + 1484);
  }//B+ decays to K0 anti-K0 omega pi+
  if (GenBplusTag::PcheckDecay(genpart, 311, -311, 10221, 211)) {
    return +1 * (100000 * m_nPhotos + 1485);
  }//B+ decays to K0 anti-K0 f_0 pi+
  if (GenBplusTag::PcheckDecay(genpart, 311, -311, 213, 111)) {
    return +1 * (100000 * m_nPhotos + 1486);
  }//B+ decays to K0 anti-K0 rho+ pi0
  if (GenBplusTag::PcheckDecay(genpart, 311, -311, 213, 221)) {
    return +1 * (100000 * m_nPhotos + 1487);
  }//B+ decays to K0 anti-K0 rho+ eta
  if (GenBplusTag::PcheckDecay(genpart, 311, -311, 213, 331)) {
    return +1 * (100000 * m_nPhotos + 1488);
  }//B+ decays to K0 anti-K0 rho+ eta'
  if (GenBplusTag::PcheckDecay(genpart, 311, -321, 213, 211)) {
    return +1 * (100000 * m_nPhotos + 1489);
  }//B+ decays to K0 K- rho+ pi+
  if (GenBplusTag::PcheckDecay(genpart, 311, -321, 211, 211, 111)) {
    return +1 * (100000 * m_nPhotos + 1490);
  }//B+ decays to K0 K- pi+ pi+ pi0
  if (GenBplusTag::PcheckDecay(genpart, 311, -321, 211, 211, 221)) {
    return +1 * (100000 * m_nPhotos + 1491);
  }//B+ decays to K0 K- pi+ pi+ eta
  if (GenBplusTag::PcheckDecay(genpart, 311, -321, 211, 211, 331)) {
    return +1 * (100000 * m_nPhotos + 1492);
  }//B+ decays to K0 K- pi+ pi+ eta'
  if (GenBplusTag::PcheckDecay(genpart, 321, -321, 211, -211, 211)) {
    return +1 * (100000 * m_nPhotos + 1493);
  }//B+ decays to K+ K- pi+ pi- pi+
  if (GenBplusTag::PcheckDecay(genpart, 321, -321, 211, 111, 111)) {
    return +1 * (100000 * m_nPhotos + 1494);
  }//B+ decays to K+ K- pi+ pi0 pi0
  if (GenBplusTag::PcheckDecay(genpart, 321, -321, 211, 221, 111)) {
    return +1 * (100000 * m_nPhotos + 1495);
  }//B+ decays to K+ K- pi+ eta pi0
  if (GenBplusTag::PcheckDecay(genpart, 321, -321, 211, 331, 111)) {
    return +1 * (100000 * m_nPhotos + 1496);
  }//B+ decays to K+ K- pi+ eta' pi0
  if (GenBplusTag::PcheckDecay(genpart, 311, -311, 211, -211, 211)) {
    return +1 * (100000 * m_nPhotos + 1497);
  }//B+ decays to K0 anti-K0 pi+ pi- pi+
  if (GenBplusTag::PcheckDecay(genpart, 311, -311, 211, 111, 111)) {
    return +1 * (100000 * m_nPhotos + 1498);
  }//B+ decays to K0 anti-K0 pi+ pi0 pi0
  if (GenBplusTag::PcheckDecay(genpart, 311, -311, 211, 221, 111)) {
    return +1 * (100000 * m_nPhotos + 1499);
  }//B+ decays to K0 anti-K0 pi+ eta pi0
  if (GenBplusTag::PcheckDecay(genpart, 311, -311, 211, 331, 111)) {
    return +1 * (100000 * m_nPhotos + 1500);
  }//B+ decays to K0 anti-K0 pi+ eta' pi0
  if (GenBplusTag::PcheckDecay(genpart, -311, 321, 211, -211, 111)) {
    return +1 * (100000 * m_nPhotos + 1501);
  }//B+ decays to anti-K0 K+ pi+ pi- pi0
  if (GenBplusTag::PcheckDecay(genpart, -311, 321, 211, -211, 221)) {
    return +1 * (100000 * m_nPhotos + 1502);
  }//B+ decays to anti-K0 K+ pi+ pi- eta
  if (GenBplusTag::PcheckDecay(genpart, -311, 321, 211, -211, 331)) {
    return +1 * (100000 * m_nPhotos + 1503);
  }//B+ decays to anti-K0 K+ pi+ pi- eta'
  if (GenBplusTag::PcheckDecay(genpart, -311, 321, 111, 111, 111)) {
    return +1 * (100000 * m_nPhotos + 1504);
  }//B+ decays to anti-K0 K+ pi0 pi0 pi0
  if (GenBplusTag::PcheckDecay(genpart, -311, 321, 111, 221, 111)) {
    return +1 * (100000 * m_nPhotos + 1505);
  }//B+ decays to anti-K0 K+ pi0 eta pi0
  if (GenBplusTag::PcheckDecay(genpart, -311, 321, 111, 331, 111)) {
    return +1 * (100000 * m_nPhotos + 1506);
  }//B+ decays to anti-K0 K+ pi0 eta' pi0
  if (GenBplusTag::PcheckDecay(genpart, 313, -321, 211, 211)) {
    return +1 * (100000 * m_nPhotos + 1507);
  }//B+ decays to K*0 K- pi+ pi+
  if (GenBplusTag::PcheckDecay(genpart, -323, 311, 211, 211)) {
    return +1 * (100000 * m_nPhotos + 1508);
  }//B+ decays to K*- K0 pi+ pi+
  if (GenBplusTag::PcheckDecay(genpart, 323, -321, 211, 111)) {
    return +1 * (100000 * m_nPhotos + 1509);
  }//B+ decays to K*+ K- pi+ pi0
  if (GenBplusTag::PcheckDecay(genpart, -323, 321, 211, 111)) {
    return +1 * (100000 * m_nPhotos + 1510);
  }//B+ decays to K*- K+ pi+ pi0
  if (GenBplusTag::PcheckDecay(genpart, 313, -311, 211, 111)) {
    return +1 * (100000 * m_nPhotos + 1511);
  }//B+ decays to K*0 anti-K0 pi+ pi0
  if (GenBplusTag::PcheckDecay(genpart, 311, -313, 211, 111)) {
    return +1 * (100000 * m_nPhotos + 1512);
  }//B+ decays to K0 anti-K*0 pi+ pi0
  if (GenBplusTag::PcheckDecay(genpart, 321, -313, 111, 111)) {
    return +1 * (100000 * m_nPhotos + 1513);
  }//B+ decays to K+ anti-K*0 pi0 pi0
  if (GenBplusTag::PcheckDecay(genpart, 321, -313, 221, 111)) {
    return +1 * (100000 * m_nPhotos + 1514);
  }//B+ decays to K+ anti-K*0 eta pi0
  if (GenBplusTag::PcheckDecay(genpart, 321, -313, 331, 111)) {
    return +1 * (100000 * m_nPhotos + 1515);
  }//B+ decays to K+ anti-K*0 eta' pi0
  if (GenBplusTag::PcheckDecay(genpart, 323, -311, 111, 111)) {
    return +1 * (100000 * m_nPhotos + 1516);
  }//B+ decays to K*+ anti-K0 pi0 pi0
  if (GenBplusTag::PcheckDecay(genpart, 323, -311, 221, 111)) {
    return +1 * (100000 * m_nPhotos + 1517);
  }//B+ decays to K*+ anti-K0 eta pi0
  if (GenBplusTag::PcheckDecay(genpart, 323, -311, 331, 111)) {
    return +1 * (100000 * m_nPhotos + 1518);
  }//B+ decays to K*+ anti-K0 eta' pi0
  if (GenBplusTag::PcheckDecay(genpart, 333, 323, 111)) {
    return +1 * (100000 * m_nPhotos + 1519);
  }//B+ decays to phi K*+ pi0
  if (GenBplusTag::PcheckDecay(genpart, 333, 321, 113)) {
    return +1 * (100000 * m_nPhotos + 1520);
  }//B+ decays to phi K+ rho0
  if (GenBplusTag::PcheckDecay(genpart, 333, 321, 223)) {
    return +1 * (100000 * m_nPhotos + 1521);
  }//B+ decays to phi K+ omega
  if (GenBplusTag::PcheckDecay(genpart, 333, 321, 10221)) {
    return +1 * (100000 * m_nPhotos + 1522);
  }//B+ decays to phi K+ f_0
  if (GenBplusTag::PcheckDecay(genpart, 333, 311, 213)) {
    return +1 * (100000 * m_nPhotos + 1523);
  }//B+ decays to phi K0 rho+
  if (GenBplusTag::PcheckDecay(genpart, 333, 311, 211, 111)) {
    return +1 * (100000 * m_nPhotos + 1524);
  }//B+ decays to phi K0 pi+ pi0
  if (GenBplusTag::PcheckDecay(genpart, 333, 321, 211, -211)) {
    return +1 * (100000 * m_nPhotos + 1525);
  }//B+ decays to phi K+ pi+ pi-
  if (GenBplusTag::PcheckDecay(genpart, 333, 321, 111, 111)) {
    return +1 * (100000 * m_nPhotos + 1526);
  }//B+ decays to phi K+ pi0 pi0
  if (GenBplusTag::PcheckDecay(genpart, 333, 321, 221, 111)) {
    return +1 * (100000 * m_nPhotos + 1527);
  }//B+ decays to phi K+ eta pi0
  if (GenBplusTag::PcheckDecay(genpart, 333, 321, 331, 111)) {
    return +1 * (100000 * m_nPhotos + 1528);
  }//B+ decays to phi K+ eta' pi0
  if (GenBplusTag::PcheckDecay(genpart, -321, 321, 323, 111)) {
    return +1 * (100000 * m_nPhotos + 1529);
  }//B+ decays to K- K+ K*+ pi0
  if (GenBplusTag::PcheckDecay(genpart, 321, -323, 321, 111)) {
    return +1 * (100000 * m_nPhotos + 1530);
  }//B+ decays to K+ K*- K+ pi0
  if (GenBplusTag::PcheckDecay(genpart, 321, 313, -311, 111)) {
    return +1 * (100000 * m_nPhotos + 1531);
  }//B+ decays to K+ K*0 anti-K0 pi0
  if (GenBplusTag::PcheckDecay(genpart, 321, 311, -313, 111)) {
    return +1 * (100000 * m_nPhotos + 1532);
  }//B+ decays to K+ K0 anti-K*0 pi0
  if (GenBplusTag::PcheckDecay(genpart, 311, 323, -311, 111)) {
    return +1 * (100000 * m_nPhotos + 1533);
  }//B+ decays to K0 K*+ anti-K0 pi0
  if (GenBplusTag::PcheckDecay(genpart, -321, 321, 321, 113)) {
    return +1 * (100000 * m_nPhotos + 1534);
  }//B+ decays to K- K+ K+ rho0
  if (GenBplusTag::PcheckDecay(genpart, -321, 321, 321, 223)) {
    return +1 * (100000 * m_nPhotos + 1535);
  }//B+ decays to K- K+ K+ omega
  if (GenBplusTag::PcheckDecay(genpart, -321, 321, 321, 10221)) {
    return +1 * (100000 * m_nPhotos + 1536);
  }//B+ decays to K- K+ K+ f_0
  if (GenBplusTag::PcheckDecay(genpart, -321, 321, 311, 213)) {
    return +1 * (100000 * m_nPhotos + 1537);
  }//B+ decays to K- K+ K0 rho+
  if (GenBplusTag::PcheckDecay(genpart, 321, -321, 321, 111, 111)) {
    return +1 * (100000 * m_nPhotos + 1538);
  }//B+ decays to K+ K- K+ pi0 pi0
  if (GenBplusTag::PcheckDecay(genpart, 321, -321, 321, 221, 111)) {
    return +1 * (100000 * m_nPhotos + 1539);
  }//B+ decays to K+ K- K+ eta pi0
  if (GenBplusTag::PcheckDecay(genpart, 321, -321, 321, 331, 111)) {
    return +1 * (100000 * m_nPhotos + 1540);
  }//B+ decays to K+ K- K+ eta' pi0
  if (GenBplusTag::PcheckDecay(genpart, 321, -321, 311, 211, 111)) {
    return +1 * (100000 * m_nPhotos + 1541);
  }//B+ decays to K+ K- K0 pi+ pi0
  if (GenBplusTag::PcheckDecay(genpart, 311, -311, 321, 111, 111)) {
    return +1 * (100000 * m_nPhotos + 1542);
  }//B+ decays to K0 anti-K0 K+ pi0 pi0
  if (GenBplusTag::PcheckDecay(genpart, 311, -311, 321, 221, 111)) {
    return +1 * (100000 * m_nPhotos + 1543);
  }//B+ decays to K0 anti-K0 K+ eta pi0
  if (GenBplusTag::PcheckDecay(genpart, 311, -311, 321, 331, 111)) {
    return +1 * (100000 * m_nPhotos + 1544);
  }//B+ decays to K0 anti-K0 K+ eta' pi0
  if (GenBplusTag::PcheckDecay(genpart, 311, -311, 311, 211, 111)) {
    return +1 * (100000 * m_nPhotos + 1545);
  }//B+ decays to K0 anti-K0 K0 pi+ pi0
  if (GenBplusTag::PcheckDecay(genpart, 333, 10311, 211)) {
    return +1 * (100000 * m_nPhotos + 1546);
  }//B+ decays to phi K_0*0 pi+
  if (GenBplusTag::PcheckDecay(genpart, 333, 10321, 111)) {
    return +1 * (100000 * m_nPhotos + 1547);
  }//B+ decays to phi K_0*+ pi0
  if (GenBplusTag::PcheckDecay(genpart, 333, 10321, 221)) {
    return +1 * (100000 * m_nPhotos + 1548);
  }//B+ decays to phi K_0*+ eta
  if (GenBplusTag::PcheckDecay(genpart, 333, 10321, 331)) {
    return +1 * (100000 * m_nPhotos + 1549);
  }//B+ decays to phi K_0*+ eta'
  if (GenBplusTag::PcheckDecay(genpart, -321, 321, 10321, 111)) {
    return +1 * (100000 * m_nPhotos + 1550);
  }//B+ decays to K- K+ K_0*+ pi0
  if (GenBplusTag::PcheckDecay(genpart, -321, 321, 10321, 221)) {
    return +1 * (100000 * m_nPhotos + 1551);
  }//B+ decays to K- K+ K_0*+ eta
  if (GenBplusTag::PcheckDecay(genpart, -321, 321, 10321, 331)) {
    return +1 * (100000 * m_nPhotos + 1552);
  }//B+ decays to K- K+ K_0*+ eta'
  if (GenBplusTag::PcheckDecay(genpart, 321, 321, -10321, 111)) {
    return +1 * (100000 * m_nPhotos + 1553);
  }//B+ decays to K+ K+ K_0*- pi0
  if (GenBplusTag::PcheckDecay(genpart, 321, 321, -10321, 221)) {
    return +1 * (100000 * m_nPhotos + 1554);
  }//B+ decays to K+ K+ K_0*- eta
  if (GenBplusTag::PcheckDecay(genpart, 321, 321, -10321, 331)) {
    return +1 * (100000 * m_nPhotos + 1555);
  }//B+ decays to K+ K+ K_0*- eta'
  if (GenBplusTag::PcheckDecay(genpart, -321, 321, 10311, 211)) {
    return +1 * (100000 * m_nPhotos + 1556);
  }//B+ decays to K- K+ K_0*0 pi+
  if (GenBplusTag::PcheckDecay(genpart, 321, 321, -10311, -211)) {
    return +1 * (100000 * m_nPhotos + 1557);
  }//B+ decays to K+ K+ anti-K_0*0 pi-
  if (GenBplusTag::PcheckDecay(genpart, 311, 311, -10311, 211)) {
    return +1 * (100000 * m_nPhotos + 1558);
  }//B+ decays to K0 K0 anti-K_0*0 pi+
  if (GenBplusTag::PcheckDecay(genpart, 321, -10311, 311, 111)) {
    return +1 * (100000 * m_nPhotos + 1559);
  }//B+ decays to K+ anti-K_0*0 K0 pi0
  if (GenBplusTag::PcheckDecay(genpart, 321, -10311, 311, 221)) {
    return +1 * (100000 * m_nPhotos + 1560);
  }//B+ decays to K+ anti-K_0*0 K0 eta
  if (GenBplusTag::PcheckDecay(genpart, 321, -10311, 311, 331)) {
    return +1 * (100000 * m_nPhotos + 1561);
  }//B+ decays to K+ anti-K_0*0 K0 eta'
  if (GenBplusTag::PcheckDecay(genpart, 321, 10311, -311, 111)) {
    return +1 * (100000 * m_nPhotos + 1562);
  }//B+ decays to K+ K_0*0 anti-K0 pi0
  if (GenBplusTag::PcheckDecay(genpart, 321, 10311, -311, 221)) {
    return +1 * (100000 * m_nPhotos + 1563);
  }//B+ decays to K+ K_0*0 anti-K0 eta
  if (GenBplusTag::PcheckDecay(genpart, 321, 10311, -311, 331)) {
    return +1 * (100000 * m_nPhotos + 1564);
  }//B+ decays to K+ K_0*0 anti-K0 eta'
  if (GenBplusTag::PcheckDecay(genpart, 333, 20313, 211)) {
    return +1 * (100000 * m_nPhotos + 1565);
  }//B+ decays to phi K'_10 pi+
  if (GenBplusTag::PcheckDecay(genpart, 333, 20323, 111)) {
    return +1 * (100000 * m_nPhotos + 1566);
  }//B+ decays to phi K'_1+ pi0
  if (GenBplusTag::PcheckDecay(genpart, 333, 20323, 221)) {
    return +1 * (100000 * m_nPhotos + 1567);
  }//B+ decays to phi K'_1+ eta
  if (GenBplusTag::PcheckDecay(genpart, 333, 20323, 331)) {
    return +1 * (100000 * m_nPhotos + 1568);
  }//B+ decays to phi K'_1+ eta'
  if (GenBplusTag::PcheckDecay(genpart, -321, 321, 20313, 211)) {
    return +1 * (100000 * m_nPhotos + 1569);
  }//B+ decays to K- K+ K'_10 pi+
  if (GenBplusTag::PcheckDecay(genpart, -321, 321, 20323, 111)) {
    return +1 * (100000 * m_nPhotos + 1570);
  }//B+ decays to K- K+ K'_1+ pi0
  if (GenBplusTag::PcheckDecay(genpart, -321, 321, 20323, 221)) {
    return +1 * (100000 * m_nPhotos + 1571);
  }//B+ decays to K- K+ K'_1+ eta
  if (GenBplusTag::PcheckDecay(genpart, -321, 321, 20323, 331)) {
    return +1 * (100000 * m_nPhotos + 1572);
  }//B+ decays to K- K+ K'_1+ eta'
  if (GenBplusTag::PcheckDecay(genpart, 321, 321, -20323, 111)) {
    return +1 * (100000 * m_nPhotos + 1573);
  }//B+ decays to K+ K+ K'_1- pi0
  if (GenBplusTag::PcheckDecay(genpart, 321, 321, -20313, -211)) {
    return +1 * (100000 * m_nPhotos + 1574);
  }//B+ decays to K+ K+ anti-K'_10 pi-
  if (GenBplusTag::PcheckDecay(genpart, 311, -311, 20313, 211)) {
    return +1 * (100000 * m_nPhotos + 1575);
  }//B+ decays to K0 anti-K0 K'_10 pi+
  if (GenBplusTag::PcheckDecay(genpart, 311, -311, 20323, 111)) {
    return +1 * (100000 * m_nPhotos + 1576);
  }//B+ decays to K0 anti-K0 K'_1+ pi0
  if (GenBplusTag::PcheckDecay(genpart, 311, -311, 20323, 221)) {
    return +1 * (100000 * m_nPhotos + 1577);
  }//B+ decays to K0 anti-K0 K'_1+ eta
  if (GenBplusTag::PcheckDecay(genpart, 311, -311, 20323, 331)) {
    return +1 * (100000 * m_nPhotos + 1578);
  }//B+ decays to K0 anti-K0 K'_1+ eta'
  if (GenBplusTag::PcheckDecay(genpart, 311, 311, -20313, 211)) {
    return +1 * (100000 * m_nPhotos + 1579);
  }//B+ decays to K0 K0 anti-K'_10 pi+
  if (GenBplusTag::PcheckDecay(genpart, -311, -311, 20323, 111)) {
    return +1 * (100000 * m_nPhotos + 1580);
  }//B+ decays to anti-K0 anti-K0 K'_1+ pi0
  if (GenBplusTag::PcheckDecay(genpart, -311, -311, 20323, 221)) {
    return +1 * (100000 * m_nPhotos + 1581);
  }//B+ decays to anti-K0 anti-K0 K'_1+ eta
  if (GenBplusTag::PcheckDecay(genpart, -311, -311, 20323, 331)) {
    return +1 * (100000 * m_nPhotos + 1582);
  }//B+ decays to anti-K0 anti-K0 K'_1+ eta'
  if (GenBplusTag::PcheckDecay(genpart, 321, 311, -20313, 111)) {
    return +1 * (100000 * m_nPhotos + 1583);
  }//B+ decays to K+ K0 anti-K'_10 pi0
  if (GenBplusTag::PcheckDecay(genpart, 321, 311, -20313, 221)) {
    return +1 * (100000 * m_nPhotos + 1584);
  }//B+ decays to K+ K0 anti-K'_10 eta
  if (GenBplusTag::PcheckDecay(genpart, 321, 311, -20313, 331)) {
    return +1 * (100000 * m_nPhotos + 1585);
  }//B+ decays to K+ K0 anti-K'_10 eta'
  if (GenBplusTag::PcheckDecay(genpart, 321, 20313, -311, 111)) {
    return +1 * (100000 * m_nPhotos + 1586);
  }//B+ decays to K+ K'_10 anti-K0 pi0
  if (GenBplusTag::PcheckDecay(genpart, 321, 20313, -311, 221)) {
    return +1 * (100000 * m_nPhotos + 1587);
  }//B+ decays to K+ K'_10 anti-K0 eta
  if (GenBplusTag::PcheckDecay(genpart, 321, 20313, -311, 331)) {
    return +1 * (100000 * m_nPhotos + 1588);
  }//B+ decays to K+ K'_10 anti-K0 eta'
  if (GenBplusTag::PcheckDecay(genpart, 333, 333, 321)) {
    return +1 * (100000 * m_nPhotos + 1589);
  }//B+ decays to phi phi K+
  if (GenBplusTag::PcheckDecay(genpart, 333, 333, 211)) {
    return +1 * (100000 * m_nPhotos + 1590);
  }//B+ decays to phi phi pi+
  if (GenBplusTag::PcheckDecay(genpart, 333, 321, -321, 321)) {
    return +1 * (100000 * m_nPhotos + 1591);
  }//B+ decays to phi K+ K- K+
  if (GenBplusTag::PcheckDecay(genpart, 333, 311, -311, 321)) {
    return +1 * (100000 * m_nPhotos + 1592);
  }//B+ decays to phi K0 anti-K0 K+
  if (GenBplusTag::PcheckDecay(genpart, 333, 311, -311, 211)) {
    return +1 * (100000 * m_nPhotos + 1593);
  }//B+ decays to phi K0 anti-K0 pi+
  if (GenBplusTag::PcheckDecay(genpart, 333, -311, 321, 111)) {
    return +1 * (100000 * m_nPhotos + 1594);
  }//B+ decays to phi anti-K0 K+ pi0
  if (GenBplusTag::PcheckDecay(genpart, 321, -321, 321, -321, 211)) {
    return +1 * (100000 * m_nPhotos + 1595);
  }//B+ decays to K+ K- K+ K- pi+
  if (GenBplusTag::PcheckDecay(genpart, 321, -321, 311, -311, 211)) {
    return +1 * (100000 * m_nPhotos + 1596);
  }//B+ decays to K+ K- K0 anti-K0 pi+
  if (GenBplusTag::PcheckDecay(genpart, 321, -321, 321, -311, 111)) {
    return +1 * (100000 * m_nPhotos + 1597);
  }//B+ decays to K+ K- K+ anti-K0 pi0
  if (GenBplusTag::PcheckDecay(genpart, 311, -311, -311, 321, 111)) {
    return +1 * (100000 * m_nPhotos + 1598);
  }//B+ decays to K0 anti-K0 anti-K0 K+ pi0
  if (GenBplusTag::PcheckDecay(genpart, 333, 333, 323)) {
    return +1 * (100000 * m_nPhotos + 1599);
  }//B+ decays to phi phi K*+
  if (GenBplusTag::PcheckDecay(genpart, 333, 321, -321, 323)) {
    return +1 * (100000 * m_nPhotos + 1600);
  }//B+ decays to phi K+ K- K*+
  if (GenBplusTag::PcheckDecay(genpart, 333, 311, -311, 323)) {
    return +1 * (100000 * m_nPhotos + 1601);
  }//B+ decays to phi K0 anti-K0 K*+
  if (GenBplusTag::PcheckDecay(genpart, 333, -311, 321, 313)) {
    return +1 * (100000 * m_nPhotos + 1602);
  }//B+ decays to phi anti-K0 K+ K*0
  if (GenBplusTag::PcheckDecay(genpart, 333, 321, -321, 321, 111)) {
    return +1 * (100000 * m_nPhotos + 1603);
  }//B+ decays to phi K+ K- K+ pi0
  if (GenBplusTag::PcheckDecay(genpart, 333, 321, -321, 311, 211)) {
    return +1 * (100000 * m_nPhotos + 1604);
  }//B+ decays to phi K+ K- K0 pi+
  if (GenBplusTag::PcheckDecay(genpart, 333, 311, -311, 321, 111)) {
    return +1 * (100000 * m_nPhotos + 1605);
  }//B+ decays to phi K0 anti-K0 K+ pi0
  if (GenBplusTag::PcheckDecay(genpart, 333, 311, -311, 311, 211)) {
    return +1 * (100000 * m_nPhotos + 1606);
  }//B+ decays to phi K0 anti-K0 K0 pi+
  if (GenBplusTag::PcheckDecay(genpart, 333, -311, 321, 321, -211)) {
    return +1 * (100000 * m_nPhotos + 1607);
  }//B+ decays to phi anti-K0 K+ K+ pi-
  if (GenBplusTag::PcheckDecay(genpart, 20213, 20113)) {
    return +1 * (100000 * m_nPhotos + 1608);
  }//B+ decays to a_1+ a_10
  if (GenBplusTag::PcheckDecay(genpart, 443, 321)) {
    return +1 * (100000 * m_nPhotos + 1609);
  }//B+ decays to J/psi K+
  if (GenBplusTag::PcheckDecay(genpart, 443, 323)) {
    return +1 * (100000 * m_nPhotos + 1610);
  }//B+ decays to J/psi K*+
  if (GenBplusTag::PcheckDecay(genpart, 443, 211)) {
    return +1 * (100000 * m_nPhotos + 1611);
  }//B+ decays to J/psi pi+
  if (GenBplusTag::PcheckDecay(genpart, 443, 213)) {
    return +1 * (100000 * m_nPhotos + 1612);
  }//B+ decays to J/psi rho+
  if (GenBplusTag::PcheckDecay(genpart, 443, 311, 211)) {
    return +1 * (100000 * m_nPhotos + 1613);
  }//B+ decays to J/psi K0 pi+
  if (GenBplusTag::PcheckDecay(genpart, 443, 321, 111)) {
    return +1 * (100000 * m_nPhotos + 1614);
  }//B+ decays to J/psi K+ pi0
  if (GenBplusTag::PcheckDecay(genpart, 443, 20323)) {
    return +1 * (100000 * m_nPhotos + 1615);
  }//B+ decays to J/psi K'_1+
  if (GenBplusTag::PcheckDecay(genpart, 443, 325)) {
    return +1 * (100000 * m_nPhotos + 1616);
  }//B+ decays to J/psi K_2*+
  if (GenBplusTag::PcheckDecay(genpart, 443, 10323)) {
    return +1 * (100000 * m_nPhotos + 1617);
  }//B+ decays to J/psi K_1+
  if (GenBplusTag::PcheckDecay(genpart, 443, 321, 211, -211)) {
    return +1 * (100000 * m_nPhotos + 1618);
  }//B+ decays to J/psi K+ pi+ pi-
  if (GenBplusTag::PcheckDecay(genpart, 443, 321, 321, -321)) {
    return +1 * (100000 * m_nPhotos + 1619);
  }//B+ decays to J/psi K+ K+ K-
  if (GenBplusTag::PcheckDecay(genpart, 443, 221, 321)) {
    return +1 * (100000 * m_nPhotos + 1620);
  }//B+ decays to J/psi eta K+
  if (GenBplusTag::PcheckDecay(genpart, 443, 333, 321)) {
    return +1 * (100000 * m_nPhotos + 1621);
  }//B+ decays to J/psi phi K+
  if (GenBplusTag::PcheckDecay(genpart, 443, 223, 321)) {
    return +1 * (100000 * m_nPhotos + 1622);
  }//B+ decays to J/psi omega K+
  if (GenBplusTag::PcheckDecay(genpart, 443, -211, -211, 211, 211, 211)) {
    return +1 * (100000 * m_nPhotos + 1623);
  }//B+ decays to J/psi pi- pi- pi+ pi+ pi+
  if (GenBplusTag::PcheckDecay(genpart, 443, 2212, -3122)) {
    return +1 * (100000 * m_nPhotos + 1624);
  }//B+ decays to J/psi p+ anti-Lambda0
  if (GenBplusTag::PcheckDecay(genpart, 100443, 321)) {
    return +1 * (100000 * m_nPhotos + 1625);
  }//B+ decays to psi(2S) K+
  if (GenBplusTag::PcheckDecay(genpart, 100443, 323)) {
    return +1 * (100000 * m_nPhotos + 1626);
  }//B+ decays to psi(2S) K*+
  if (GenBplusTag::PcheckDecay(genpart, 100443, 311, 211)) {
    return +1 * (100000 * m_nPhotos + 1627);
  }//B+ decays to psi(2S) K0 pi+
  if (GenBplusTag::PcheckDecay(genpart, 100443, 321, 111)) {
    return +1 * (100000 * m_nPhotos + 1628);
  }//B+ decays to psi(2S) K+ pi0
  if (GenBplusTag::PcheckDecay(genpart, 100443, 321, 211, -211)) {
    return +1 * (100000 * m_nPhotos + 1629);
  }//B+ decays to psi(2S) K+ pi+ pi-
  if (GenBplusTag::PcheckDecay(genpart, 100443, 321, 111, 111)) {
    return +1 * (100000 * m_nPhotos + 1630);
  }//B+ decays to psi(2S) K+ pi0 pi0
  if (GenBplusTag::PcheckDecay(genpart, 100443, 311, 211, 111)) {
    return +1 * (100000 * m_nPhotos + 1631);
  }//B+ decays to psi(2S) K0 pi+ pi0
  if (GenBplusTag::PcheckDecay(genpart, 100443, 10323)) {
    return +1 * (100000 * m_nPhotos + 1632);
  }//B+ decays to psi(2S) K_1+
  if (GenBplusTag::PcheckDecay(genpart, 100443, 211)) {
    return +1 * (100000 * m_nPhotos + 1633);
  }//B+ decays to psi(2S) pi+
  if (GenBplusTag::PcheckDecay(genpart, 100443, 333, 321)) {
    return +1 * (100000 * m_nPhotos + 1634);
  }//B+ decays to psi(2S) phi K+
  if (GenBplusTag::PcheckDecay(genpart, 441, 321)) {
    return +1 * (100000 * m_nPhotos + 1635);
  }//B+ decays to eta_c K+
  if (GenBplusTag::PcheckDecay(genpart, 323, 441)) {
    return +1 * (100000 * m_nPhotos + 1636);
  }//B+ decays to K*+ eta_c
  if (GenBplusTag::PcheckDecay(genpart, 441, 311, 211)) {
    return +1 * (100000 * m_nPhotos + 1637);
  }//B+ decays to eta_c K0 pi+
  if (GenBplusTag::PcheckDecay(genpart, 441, 321, 111)) {
    return +1 * (100000 * m_nPhotos + 1638);
  }//B+ decays to eta_c K+ pi0
  if (GenBplusTag::PcheckDecay(genpart, 441, 321, 211, -211)) {
    return +1 * (100000 * m_nPhotos + 1639);
  }//B+ decays to eta_c K+ pi+ pi-
  if (GenBplusTag::PcheckDecay(genpart, 441, 321, 111, 111)) {
    return +1 * (100000 * m_nPhotos + 1640);
  }//B+ decays to eta_c K+ pi0 pi0
  if (GenBplusTag::PcheckDecay(genpart, 441, 311, 211, 111)) {
    return +1 * (100000 * m_nPhotos + 1641);
  }//B+ decays to eta_c K0 pi+ pi0
  if (GenBplusTag::PcheckDecay(genpart, 100441, 321)) {
    return +1 * (100000 * m_nPhotos + 1642);
  }//B+ decays to eta_c(2S) K+
  if (GenBplusTag::PcheckDecay(genpart, 323, 100441)) {
    return +1 * (100000 * m_nPhotos + 1643);
  }//B+ decays to K*+ eta_c(2S)
  if (GenBplusTag::PcheckDecay(genpart, 100441, 311, 211)) {
    return +1 * (100000 * m_nPhotos + 1644);
  }//B+ decays to eta_c(2S) K0 pi+
  if (GenBplusTag::PcheckDecay(genpart, 100441, 321, 111)) {
    return +1 * (100000 * m_nPhotos + 1645);
  }//B+ decays to eta_c(2S) K+ pi0
  if (GenBplusTag::PcheckDecay(genpart, 100441, 321, 211, -211)) {
    return +1 * (100000 * m_nPhotos + 1646);
  }//B+ decays to eta_c(2S) K+ pi+ pi-
  if (GenBplusTag::PcheckDecay(genpart, 100441, 321, 111, 111)) {
    return +1 * (100000 * m_nPhotos + 1647);
  }//B+ decays to eta_c(2S) K+ pi0 pi0
  if (GenBplusTag::PcheckDecay(genpart, 100441, 311, 211, 111)) {
    return +1 * (100000 * m_nPhotos + 1648);
  }//B+ decays to eta_c(2S) K0 pi+ pi0
  if (GenBplusTag::PcheckDecay(genpart, 10441, 321)) {
    return +1 * (100000 * m_nPhotos + 1649);
  }//B+ decays to chi_c0 K+
  if (GenBplusTag::PcheckDecay(genpart, 323, 10441)) {
    return +1 * (100000 * m_nPhotos + 1650);
  }//B+ decays to K*+ chi_c0
  if (GenBplusTag::PcheckDecay(genpart, 10441, 311, 211)) {
    return +1 * (100000 * m_nPhotos + 1651);
  }//B+ decays to chi_c0 K0 pi+
  if (GenBplusTag::PcheckDecay(genpart, 10441, 321, 111)) {
    return +1 * (100000 * m_nPhotos + 1652);
  }//B+ decays to chi_c0 K+ pi0
  if (GenBplusTag::PcheckDecay(genpart, 10441, 321, 211, -211)) {
    return +1 * (100000 * m_nPhotos + 1653);
  }//B+ decays to chi_c0 K+ pi+ pi-
  if (GenBplusTag::PcheckDecay(genpart, 10441, 321, 111, 111)) {
    return +1 * (100000 * m_nPhotos + 1654);
  }//B+ decays to chi_c0 K+ pi0 pi0
  if (GenBplusTag::PcheckDecay(genpart, 10441, 311, 211, 111)) {
    return +1 * (100000 * m_nPhotos + 1655);
  }//B+ decays to chi_c0 K0 pi+ pi0
  if (GenBplusTag::PcheckDecay(genpart, 20443, 321)) {
    return +1 * (100000 * m_nPhotos + 1656);
  }//B+ decays to chi_c1 K+
  if (GenBplusTag::PcheckDecay(genpart, 20443, 323)) {
    return +1 * (100000 * m_nPhotos + 1657);
  }//B+ decays to chi_c1 K*+
  if (GenBplusTag::PcheckDecay(genpart, 20443, 311, 211)) {
    return +1 * (100000 * m_nPhotos + 1658);
  }//B+ decays to chi_c1 K0 pi+
  if (GenBplusTag::PcheckDecay(genpart, 20443, 321, 111)) {
    return +1 * (100000 * m_nPhotos + 1659);
  }//B+ decays to chi_c1 K+ pi0
  if (GenBplusTag::PcheckDecay(genpart, 20443, 321, 211, -211)) {
    return +1 * (100000 * m_nPhotos + 1660);
  }//B+ decays to chi_c1 K+ pi+ pi-
  if (GenBplusTag::PcheckDecay(genpart, 20443, 321, 111, 111)) {
    return +1 * (100000 * m_nPhotos + 1661);
  }//B+ decays to chi_c1 K+ pi0 pi0
  if (GenBplusTag::PcheckDecay(genpart, 20443, 311, 211, 111)) {
    return +1 * (100000 * m_nPhotos + 1662);
  }//B+ decays to chi_c1 K0 pi+ pi0
  if (GenBplusTag::PcheckDecay(genpart, 20443, 211)) {
    return +1 * (100000 * m_nPhotos + 1663);
  }//B+ decays to chi_c1 pi+
  if (GenBplusTag::PcheckDecay(genpart, 445, 321)) {
    return +1 * (100000 * m_nPhotos + 1664);
  }//B+ decays to chi_c2 K+
  if (GenBplusTag::PcheckDecay(genpart, 445, 323)) {
    return +1 * (100000 * m_nPhotos + 1665);
  }//B+ decays to chi_c2 K*+
  if (GenBplusTag::PcheckDecay(genpart, 445, 311, 211)) {
    return +1 * (100000 * m_nPhotos + 1666);
  }//B+ decays to chi_c2 K0 pi+
  if (GenBplusTag::PcheckDecay(genpart, 445, 321, 111)) {
    return +1 * (100000 * m_nPhotos + 1667);
  }//B+ decays to chi_c2 K+ pi0
  if (GenBplusTag::PcheckDecay(genpart, 445, 321, 211, -211)) {
    return +1 * (100000 * m_nPhotos + 1668);
  }//B+ decays to chi_c2 K+ pi+ pi-
  if (GenBplusTag::PcheckDecay(genpart, 445, 321, 111, 111)) {
    return +1 * (100000 * m_nPhotos + 1669);
  }//B+ decays to chi_c2 K+ pi0 pi0
  if (GenBplusTag::PcheckDecay(genpart, 445, 311, 211, 111)) {
    return +1 * (100000 * m_nPhotos + 1670);
  }//B+ decays to chi_c2 K0 pi+ pi0
  if (GenBplusTag::PcheckDecay(genpart, 30443, 321)) {
    return +1 * (100000 * m_nPhotos + 1671);
  }//B+ decays to psi(3770) K+
  if (GenBplusTag::PcheckDecay(genpart, 30443, 323)) {
    return +1 * (100000 * m_nPhotos + 1672);
  }//B+ decays to psi(3770) K*+
  if (GenBplusTag::PcheckDecay(genpart, 30443, 311, 211)) {
    return +1 * (100000 * m_nPhotos + 1673);
  }//B+ decays to psi(3770) K0 pi+
  if (GenBplusTag::PcheckDecay(genpart, 30443, 321, 111)) {
    return +1 * (100000 * m_nPhotos + 1674);
  }//B+ decays to psi(3770) K+ pi0
  if (GenBplusTag::PcheckDecay(genpart, 30443, 321, 211, -211)) {
    return +1 * (100000 * m_nPhotos + 1675);
  }//B+ decays to psi(3770) K+ pi+ pi-
  if (GenBplusTag::PcheckDecay(genpart, 30443, 321, 111, 111)) {
    return +1 * (100000 * m_nPhotos + 1676);
  }//B+ decays to psi(3770) K+ pi0 pi0
  if (GenBplusTag::PcheckDecay(genpart, 30443, 311, 211, 111)) {
    return +1 * (100000 * m_nPhotos + 1677);
  }//B+ decays to psi(3770) K0 pi+ pi0
  if (GenBplusTag::PcheckDecay(genpart, 30443, 10323)) {
    return +1 * (100000 * m_nPhotos + 1678);
  }//B+ decays to psi(3770) K_1+
  if (GenBplusTag::PcheckDecay(genpart, 9010443, 321)) {
    return +1 * (100000 * m_nPhotos + 1679);
  }//B+ decays to psi(4160) K+
  if (GenBplusTag::PcheckDecay(genpart, 10443, 321)) {
    return +1 * (100000 * m_nPhotos + 1680);
  }//B+ decays to h_c K+
  if (GenBplusTag::PcheckDecay(genpart, -421, 431)) {
    return +1 * (100000 * m_nPhotos + 1681);
  }//B+ decays to anti-D0 D_s+
  if (GenBplusTag::PcheckDecay(genpart, -423, 431)) {
    return +1 * (100000 * m_nPhotos + 1682);
  }//B+ decays to anti-D*0 D_s+
  if (GenBplusTag::PcheckDecay(genpart, 433, -421)) {
    return +1 * (100000 * m_nPhotos + 1683);
  }//B+ decays to D_s*+ anti-D0
  if (GenBplusTag::PcheckDecay(genpart, 433, -423)) {
    return +1 * (100000 * m_nPhotos + 1684);
  }//B+ decays to D_s*+ anti-D*0
  if (GenBplusTag::PcheckDecay(genpart, -20423, 431)) {
    return +1 * (100000 * m_nPhotos + 1685);
  }//B+ decays to anti-D'_10 D_s+
  if (GenBplusTag::PcheckDecay(genpart, -20423, 433)) {
    return +1 * (100000 * m_nPhotos + 1686);
  }//B+ decays to anti-D'_10 D_s*+
  if (GenBplusTag::PcheckDecay(genpart, -10423, 431)) {
    return +1 * (100000 * m_nPhotos + 1687);
  }//B+ decays to anti-D_10 D_s+
  if (GenBplusTag::PcheckDecay(genpart, -10423, 433)) {
    return +1 * (100000 * m_nPhotos + 1688);
  }//B+ decays to anti-D_10 D_s*+
  if (GenBplusTag::PcheckDecay(genpart, -425, 431)) {
    return +1 * (100000 * m_nPhotos + 1689);
  }//B+ decays to anti-D_2*0 D_s+
  if (GenBplusTag::PcheckDecay(genpart, -425, 433)) {
    return +1 * (100000 * m_nPhotos + 1690);
  }//B+ decays to anti-D_2*0 D_s*+
  if (GenBplusTag::PcheckDecay(genpart, 431, -411, 211)) {
    return +1 * (100000 * m_nPhotos + 1691);
  }//B+ decays to D_s+ D- pi+
  if (GenBplusTag::PcheckDecay(genpart, 431, -421, 111)) {
    return +1 * (100000 * m_nPhotos + 1692);
  }//B+ decays to D_s+ anti-D0 pi0
  if (GenBplusTag::PcheckDecay(genpart, 433, -411, 211)) {
    return +1 * (100000 * m_nPhotos + 1693);
  }//B+ decays to D_s*+ D- pi+
  if (GenBplusTag::PcheckDecay(genpart, 433, -421, 111)) {
    return +1 * (100000 * m_nPhotos + 1694);
  }//B+ decays to D_s*+ anti-D0 pi0
  if (GenBplusTag::PcheckDecay(genpart, 431, -411, 211, 111)) {
    return +1 * (100000 * m_nPhotos + 1695);
  }//B+ decays to D_s+ D- pi+ pi0
  if (GenBplusTag::PcheckDecay(genpart, 431, -421, 211, -211)) {
    return +1 * (100000 * m_nPhotos + 1696);
  }//B+ decays to D_s+ anti-D0 pi+ pi-
  if (GenBplusTag::PcheckDecay(genpart, 431, -421, 111, 111)) {
    return +1 * (100000 * m_nPhotos + 1697);
  }//B+ decays to D_s+ anti-D0 pi0 pi0
  if (GenBplusTag::PcheckDecay(genpart, 433, -411, 211, 111)) {
    return +1 * (100000 * m_nPhotos + 1698);
  }//B+ decays to D_s*+ D- pi+ pi0
  if (GenBplusTag::PcheckDecay(genpart, 433, -421, 211, -211)) {
    return +1 * (100000 * m_nPhotos + 1699);
  }//B+ decays to D_s*+ anti-D0 pi+ pi-
  if (GenBplusTag::PcheckDecay(genpart, 433, -421, 111, 111)) {
    return +1 * (100000 * m_nPhotos + 1700);
  }//B+ decays to D_s*+ anti-D0 pi0 pi0
  if (GenBplusTag::PcheckDecay(genpart, -421, 411, 311)) {
    return +1 * (100000 * m_nPhotos + 1701);
  }//B+ decays to anti-D0 D+ K0
  if (GenBplusTag::PcheckDecay(genpart, -421, 413, 311)) {
    return +1 * (100000 * m_nPhotos + 1702);
  }//B+ decays to anti-D0 D*+ K0
  if (GenBplusTag::PcheckDecay(genpart, -423, 411, 311)) {
    return +1 * (100000 * m_nPhotos + 1703);
  }//B+ decays to anti-D*0 D+ K0
  if (GenBplusTag::PcheckDecay(genpart, -423, 413, 311)) {
    return +1 * (100000 * m_nPhotos + 1704);
  }//B+ decays to anti-D*0 D*+ K0
  if (GenBplusTag::PcheckDecay(genpart, -421, 421, 321)) {
    return +1 * (100000 * m_nPhotos + 1705);
  }//B+ decays to anti-D0 D0 K+
  if (GenBplusTag::PcheckDecay(genpart, -423, 421, 321)) {
    return +1 * (100000 * m_nPhotos + 1706);
  }//B+ decays to anti-D*0 D0 K+
  if (GenBplusTag::PcheckDecay(genpart, -421, 423, 321)) {
    return +1 * (100000 * m_nPhotos + 1707);
  }//B+ decays to anti-D0 D*0 K+
  if (GenBplusTag::PcheckDecay(genpart, -423, 423, 321)) {
    return +1 * (100000 * m_nPhotos + 1708);
  }//B+ decays to anti-D*0 D*0 K+
  if (GenBplusTag::PcheckDecay(genpart, 411, -411, 321)) {
    return +1 * (100000 * m_nPhotos + 1709);
  }//B+ decays to D+ D- K+
  if (GenBplusTag::PcheckDecay(genpart, 413, -411, 321)) {
    return +1 * (100000 * m_nPhotos + 1710);
  }//B+ decays to D*+ D- K+
  if (GenBplusTag::PcheckDecay(genpart, 411, -413, 321)) {
    return +1 * (100000 * m_nPhotos + 1711);
  }//B+ decays to D+ D*- K+
  if (GenBplusTag::PcheckDecay(genpart, 413, -413, 321)) {
    return +1 * (100000 * m_nPhotos + 1712);
  }//B+ decays to D*+ D*- K+
  if (GenBplusTag::PcheckDecay(genpart, -421, 411, 313)) {
    return +1 * (100000 * m_nPhotos + 1713);
  }//B+ decays to anti-D0 D+ K*0
  if (GenBplusTag::PcheckDecay(genpart, -423, 411, 313)) {
    return +1 * (100000 * m_nPhotos + 1714);
  }//B+ decays to anti-D*0 D+ K*0
  if (GenBplusTag::PcheckDecay(genpart, -421, 413, 313)) {
    return +1 * (100000 * m_nPhotos + 1715);
  }//B+ decays to anti-D0 D*+ K*0
  if (GenBplusTag::PcheckDecay(genpart, -423, 413, 313)) {
    return +1 * (100000 * m_nPhotos + 1716);
  }//B+ decays to anti-D*0 D*+ K*0
  if (GenBplusTag::PcheckDecay(genpart, -421, 421, 323)) {
    return +1 * (100000 * m_nPhotos + 1717);
  }//B+ decays to anti-D0 D0 K*+
  if (GenBplusTag::PcheckDecay(genpart, -423, 421, 323)) {
    return +1 * (100000 * m_nPhotos + 1718);
  }//B+ decays to anti-D*0 D0 K*+
  if (GenBplusTag::PcheckDecay(genpart, -421, 423, 323)) {
    return +1 * (100000 * m_nPhotos + 1719);
  }//B+ decays to anti-D0 D*0 K*+
  if (GenBplusTag::PcheckDecay(genpart, -423, 423, 323)) {
    return +1 * (100000 * m_nPhotos + 1720);
  }//B+ decays to anti-D*0 D*0 K*+
  if (GenBplusTag::PcheckDecay(genpart, 411, -411, 323)) {
    return +1 * (100000 * m_nPhotos + 1721);
  }//B+ decays to D+ D- K*+
  if (GenBplusTag::PcheckDecay(genpart, 413, -411, 323)) {
    return +1 * (100000 * m_nPhotos + 1722);
  }//B+ decays to D*+ D- K*+
  if (GenBplusTag::PcheckDecay(genpart, 411, -413, 323)) {
    return +1 * (100000 * m_nPhotos + 1723);
  }//B+ decays to D+ D*- K*+
  if (GenBplusTag::PcheckDecay(genpart, 413, -413, 323)) {
    return +1 * (100000 * m_nPhotos + 1724);
  }//B+ decays to D*+ D*- K*+
  if (GenBplusTag::PcheckDecay(genpart, 411, -421)) {
    return +1 * (100000 * m_nPhotos + 1725);
  }//B+ decays to D+ anti-D0
  if (GenBplusTag::PcheckDecay(genpart, 413, -421)) {
    return +1 * (100000 * m_nPhotos + 1726);
  }//B+ decays to D*+ anti-D0
  if (GenBplusTag::PcheckDecay(genpart, -423, 411)) {
    return +1 * (100000 * m_nPhotos + 1727);
  }//B+ decays to anti-D*0 D+
  if (GenBplusTag::PcheckDecay(genpart, -423, 413)) {
    return +1 * (100000 * m_nPhotos + 1728);
  }//B+ decays to anti-D*0 D*+
  if (GenBplusTag::PcheckDecay(genpart, -423, 211)) {
    return +1 * (100000 * m_nPhotos + 1729);
  }//B+ decays to anti-D*0 pi+
  if (GenBplusTag::PcheckDecay(genpart, -421, 211)) {
    return +1 * (100000 * m_nPhotos + 1730);
  }//B+ decays to anti-D0 pi+
  if (GenBplusTag::PcheckDecay(genpart, 213, -421)) {
    return +1 * (100000 * m_nPhotos + 1731);
  }//B+ decays to rho+ anti-D0
  if (GenBplusTag::PcheckDecay(genpart, -423, 213)) {
    return +1 * (100000 * m_nPhotos + 1732);
  }//B+ decays to anti-D*0 rho+
  if (GenBplusTag::PcheckDecay(genpart, -421, 111, 211)) {
    return +1 * (100000 * m_nPhotos + 1733);
  }//B+ decays to anti-D0 pi0 pi+
  if (GenBplusTag::PcheckDecay(genpart, -423, 111, 211)) {
    return +1 * (100000 * m_nPhotos + 1734);
  }//B+ decays to anti-D*0 pi0 pi+
  if (GenBplusTag::PcheckDecay(genpart, -411, 211, 211)) {
    return +1 * (100000 * m_nPhotos + 1735);
  }//B+ decays to D- pi+ pi+
  if (GenBplusTag::PcheckDecay(genpart, -413, 211, 211)) {
    return +1 * (100000 * m_nPhotos + 1736);
  }//B+ decays to D*- pi+ pi+
  if (GenBplusTag::PcheckDecay(genpart, 20213, -421)) {
    return +1 * (100000 * m_nPhotos + 1737);
  }//B+ decays to a_1+ anti-D0
  if (GenBplusTag::PcheckDecay(genpart, -421, 113, 211)) {
    return +1 * (100000 * m_nPhotos + 1738);
  }//B+ decays to anti-D0 rho0 pi+
  if (GenBplusTag::PcheckDecay(genpart, -421, -211, 211, 211)) {
    return +1 * (100000 * m_nPhotos + 1739);
  }//B+ decays to anti-D0 pi- pi+ pi+
  if (GenBplusTag::PcheckDecay(genpart, -423, 20213)) {
    return +1 * (100000 * m_nPhotos + 1740);
  }//B+ decays to anti-D*0 a_1+
  if (GenBplusTag::PcheckDecay(genpart, -423, 113, 211)) {
    return +1 * (100000 * m_nPhotos + 1741);
  }//B+ decays to anti-D*0 rho0 pi+
  if (GenBplusTag::PcheckDecay(genpart, -423, -211, 211, 211)) {
    return +1 * (100000 * m_nPhotos + 1742);
  }//B+ decays to anti-D*0 pi- pi+ pi+
  if (GenBplusTag::PcheckDecay(genpart, -411, 213, 211)) {
    return +1 * (100000 * m_nPhotos + 1743);
  }//B+ decays to D- rho+ pi+
  if (GenBplusTag::PcheckDecay(genpart, -411, 111, 211, 211)) {
    return +1 * (100000 * m_nPhotos + 1744);
  }//B+ decays to D- pi0 pi+ pi+
  if (GenBplusTag::PcheckDecay(genpart, -413, 213, 211)) {
    return +1 * (100000 * m_nPhotos + 1745);
  }//B+ decays to D*- rho+ pi+
  if (GenBplusTag::PcheckDecay(genpart, -413, 111, 211, 211)) {
    return +1 * (100000 * m_nPhotos + 1746);
  }//B+ decays to D*- pi0 pi+ pi+
  if (GenBplusTag::PcheckDecay(genpart, -423, 213, 111)) {
    return +1 * (100000 * m_nPhotos + 1747);
  }//B+ decays to anti-D*0 rho+ pi0
  if (GenBplusTag::PcheckDecay(genpart, -423, 211, 111, 111)) {
    return +1 * (100000 * m_nPhotos + 1748);
  }//B+ decays to anti-D*0 pi+ pi0 pi0
  if (GenBplusTag::PcheckDecay(genpart, -10423, 211)) {
    return +1 * (100000 * m_nPhotos + 1749);
  }//B+ decays to anti-D_10 pi+
  if (GenBplusTag::PcheckDecay(genpart, -20423, 211)) {
    return +1 * (100000 * m_nPhotos + 1750);
  }//B+ decays to anti-D'_10 pi+
  if (GenBplusTag::PcheckDecay(genpart, -425, 211)) {
    return +1 * (100000 * m_nPhotos + 1751);
  }//B+ decays to anti-D_2*0 pi+
  if (GenBplusTag::PcheckDecay(genpart, -10423, 213)) {
    return +1 * (100000 * m_nPhotos + 1752);
  }//B+ decays to anti-D_10 rho+
  if (GenBplusTag::PcheckDecay(genpart, -20423, 213)) {
    return +1 * (100000 * m_nPhotos + 1753);
  }//B+ decays to anti-D'_10 rho+
  if (GenBplusTag::PcheckDecay(genpart, -425, 213)) {
    return +1 * (100000 * m_nPhotos + 1754);
  }//B+ decays to anti-D_2*0 rho+
  if (GenBplusTag::PcheckDecay(genpart, -10421, 211)) {
    return +1 * (100000 * m_nPhotos + 1755);
  }//B+ decays to anti-D_0*0 pi+
  if (GenBplusTag::PcheckDecay(genpart, -421, 321)) {
    return +1 * (100000 * m_nPhotos + 1756);
  }//B+ decays to anti-D0 K+
  if (GenBplusTag::PcheckDecay(genpart, -423, 321)) {
    return +1 * (100000 * m_nPhotos + 1757);
  }//B+ decays to anti-D*0 K+
  if (GenBplusTag::PcheckDecay(genpart, 323, -421)) {
    return +1 * (100000 * m_nPhotos + 1758);
  }//B+ decays to K*+ anti-D0
  if (GenBplusTag::PcheckDecay(genpart, -423, 323)) {
    return +1 * (100000 * m_nPhotos + 1759);
  }//B+ decays to anti-D*0 K*+
  if (GenBplusTag::PcheckDecay(genpart, 411, 111)) {
    return +1 * (100000 * m_nPhotos + 1760);
  }//B+ decays to D+ pi0
  if (GenBplusTag::PcheckDecay(genpart, 413, 111)) {
    return +1 * (100000 * m_nPhotos + 1761);
  }//B+ decays to D*+ pi0
  if (GenBplusTag::PcheckDecay(genpart, 411, -311)) {
    return +1 * (100000 * m_nPhotos + 1762);
  }//B+ decays to D+ anti-K0
  if (GenBplusTag::PcheckDecay(genpart, 413, -311)) {
    return +1 * (100000 * m_nPhotos + 1763);
  }//B+ decays to D*+ anti-K0
  if (GenBplusTag::PcheckDecay(genpart, -421, 10431)) {
    return +1 * (100000 * m_nPhotos + 1764);
  }//B+ decays to anti-D0 D_s0*+
  if (GenBplusTag::PcheckDecay(genpart, -423, 10431)) {
    return +1 * (100000 * m_nPhotos + 1765);
  }//B+ decays to anti-D*0 D_s0*+
  if (GenBplusTag::PcheckDecay(genpart, 10433, -421)) {
    return +1 * (100000 * m_nPhotos + 1766);
  }//B+ decays to D_s1+ anti-D0
  if (GenBplusTag::PcheckDecay(genpart, -423, 10433)) {
    return +1 * (100000 * m_nPhotos + 1767);
  }//B+ decays to anti-D*0 D_s1+
  if (GenBplusTag::PcheckDecay(genpart, -421, 321, -311)) {
    return +1 * (100000 * m_nPhotos + 1768);
  }//B+ decays to anti-D0 K+ anti-K0
  if (GenBplusTag::PcheckDecay(genpart, -421, 321, -313)) {
    return +1 * (100000 * m_nPhotos + 1769);
  }//B+ decays to anti-D0 K+ anti-K*0
  if (GenBplusTag::PcheckDecay(genpart, -423, 321, -313)) {
    return +1 * (100000 * m_nPhotos + 1770);
  }//B+ decays to anti-D*0 K+ anti-K*0
  if (GenBplusTag::PcheckDecay(genpart, -421, 223, 211)) {
    return +1 * (100000 * m_nPhotos + 1771);
  }//B+ decays to anti-D0 omega pi+
  if (GenBplusTag::PcheckDecay(genpart, -423, 223, 211)) {
    return +1 * (100000 * m_nPhotos + 1772);
  }//B+ decays to anti-D*0 omega pi+
  if (GenBplusTag::PcheckDecay(genpart, -421, 20433)) {
    return +1 * (100000 * m_nPhotos + 1773);
  }//B+ decays to anti-D0 D'_s1+
  if (GenBplusTag::PcheckDecay(genpart, -423, 20433)) {
    return +1 * (100000 * m_nPhotos + 1774);
  }//B+ decays to anti-D*0 D'_s1+
  if (GenBplusTag::PcheckDecay(genpart, -4103, 2203)) {
    return +1 * (100000 * m_nPhotos + 1775);
  }//B+ decays to anti-cd_1 uu_1
  if (GenBplusTag::PcheckDecay(genpart, -4303, 2203)) {
    return +1 * (100000 * m_nPhotos + 1776);
  }//B+ decays to anti-cs_1 uu_1
  if (GenBplusTag::PcheckDecay(genpart, 2, -1, -4, 2)) {
    return +1 * (100000 * m_nPhotos + 1777);
  }//B+ decays to u anti-d anti-c u
  if (GenBplusTag::PcheckDecay(genpart, 2, -1, -4, 2)) {
    return +1 * (100000 * m_nPhotos + 1778);
  }//B+ decays to u anti-d anti-c u
  if (GenBplusTag::PcheckDecay(genpart, 2, -3, -4, 2)) {
    return +1 * (100000 * m_nPhotos + 1779);
  }//B+ decays to u anti-s anti-c u
  if (GenBplusTag::PcheckDecay(genpart, 4, -3, -4, 2)) {
    return +1 * (100000 * m_nPhotos + 1780);
  }//B+ decays to c anti-s anti-c u
  if (GenBplusTag::PcheckDecay(genpart, 4, -1, -4, 2)) {
    return +1 * (100000 * m_nPhotos + 1781);
  }//B+ decays to c anti-d anti-c u
  if (GenBplusTag::PcheckDecay(genpart, 2, -1, -2, 2)) {
    return +1 * (100000 * m_nPhotos + 1782);
  }//B+ decays to u anti-d anti-u u
  if (GenBplusTag::PcheckDecay(genpart, 4, -3, -2, 2)) {
    return +1 * (100000 * m_nPhotos + 1783);
  }//B+ decays to c anti-s anti-u u
  if (GenBplusTag::PcheckDecay(genpart, 2, -2, -1, 2)) {
    return +1 * (100000 * m_nPhotos + 1784);
  }//B+ decays to u anti-u anti-d u
  if (GenBplusTag::PcheckDecay(genpart, 1, -1, -1, 2)) {
    return +1 * (100000 * m_nPhotos + 1785);
  }//B+ decays to d anti-d anti-d u
  if (GenBplusTag::PcheckDecay(genpart, 3, -3, -1, 2)) {
    return +1 * (100000 * m_nPhotos + 1786);
  }//B+ decays to s anti-s anti-d u
  if (GenBplusTag::PcheckDecay(genpart, 2, -2, -3, 2)) {
    return +1 * (100000 * m_nPhotos + 1787);
  }//B+ decays to u anti-u anti-s u
  if (GenBplusTag::PcheckDecay(genpart, 1, -1, -3, 2)) {
    return +1 * (100000 * m_nPhotos + 1788);
  }//B+ decays to d anti-d anti-s u
  if (GenBplusTag::PcheckDecay(genpart, 3, -3, -3, 2)) {
    return +1 * (100000 * m_nPhotos + 1789);
  }//B+ decays to s anti-s anti-s u
  if (GenBplusTag::PcheckDecay(genpart, -3, 2)) {
    return +1 * (100000 * m_nPhotos + 1790);
  }//B+ decays to anti-s u
  if (GenBplusTag::PcheckDecay(genpart, -423, 321, 313)) {
    return +1 * (100000 * m_nPhotos + 1791);
  }//B+ decays to anti-D*0 K+ K*0
  if (GenBplusTag::PcheckDecay(genpart, -423, -211, 211, 211, 111)) {
    return +1 * (100000 * m_nPhotos + 1792);
  }//B+ decays to anti-D*0 pi- pi+ pi+ pi0
  if (GenBplusTag::PcheckDecay(genpart, -423, 211, 211, 211, -211, -211)) {
    return +1 * (100000 * m_nPhotos + 1793);
  }//B+ decays to anti-D*0 pi+ pi+ pi+ pi- pi-
  if (GenBplusTag::PcheckDecay(genpart, -413, 211, 211, 211, -211)) {
    return +1 * (100000 * m_nPhotos + 1794);
  }//B+ decays to D*- pi+ pi+ pi+ pi-
  if (GenBplusTag::PcheckDecay(genpart, -431, 211, 321)) {
    return +1 * (100000 * m_nPhotos + 1795);
  }//B+ decays to D_s- pi+ K+
  if (GenBplusTag::PcheckDecay(genpart, -433, 211, 321)) {
    return +1 * (100000 * m_nPhotos + 1796);
  }//B+ decays to D_s*- pi+ K+
  if (GenBplusTag::PcheckDecay(genpart, -431, 321, 321)) {
    return +1 * (100000 * m_nPhotos + 1797);
  }//B+ decays to D_s- K+ K+
  if (GenBplusTag::PcheckDecay(genpart, 221, 10321)) {
    return +1 * (100000 * m_nPhotos + 1798);
  }//B+ decays to eta K_0*+
  if (GenBplusTag::PcheckDecay(genpart, 221, 325)) {
    return +1 * (100000 * m_nPhotos + 1799);
  }//B+ decays to eta K_2*+
  if (GenBplusTag::PcheckDecay(genpart, 223, 10321)) {
    return +1 * (100000 * m_nPhotos + 1800);
  }//B+ decays to omega K_0*+
  if (GenBplusTag::PcheckDecay(genpart, 223, 325)) {
    return +1 * (100000 * m_nPhotos + 1801);
  }//B+ decays to omega K_2*+
  if (GenBplusTag::PcheckDecay(genpart, 313, 211)) {
    return +1 * (100000 * m_nPhotos + 1802);
  }//B+ decays to K*0 pi+
  if (GenBplusTag::PcheckDecay(genpart, 225, 321)) {
    return +1 * (100000 * m_nPhotos + 1803);
  }//B+ decays to f_2 K+
  if (GenBplusTag::PcheckDecay(genpart, 315, 211)) {
    return +1 * (100000 * m_nPhotos + 1804);
  }//B+ decays to K_2*0 pi+
  if (GenBplusTag::PcheckDecay(genpart, 323, 313)) {
    return +1 * (100000 * m_nPhotos + 1805);
  }//B+ decays to K*+ K*0
  if (GenBplusTag::PcheckDecay(genpart, 333, 10323)) {
    return +1 * (100000 * m_nPhotos + 1806);
  }//B+ decays to phi K_1+
  if (GenBplusTag::PcheckDecay(genpart, 333, 325)) {
    return +1 * (100000 * m_nPhotos + 1807);
  }//B+ decays to phi K_2*+
  if (GenBplusTag::PcheckDecay(genpart, 221, 321, 22)) {
    return +1 * (100000 * m_nPhotos + 1808);
  }//B+ decays to eta K+ gamma
  if (GenBplusTag::PcheckDecay(genpart, 331, 321, 22)) {
    return +1 * (100000 * m_nPhotos + 1809);
  }//B+ decays to eta' K+ gamma
  if (GenBplusTag::PcheckDecay(genpart, 333, 321, 22)) {
    return +1 * (100000 * m_nPhotos + 1810);
  }//B+ decays to phi K+ gamma
  if (GenBplusTag::PcheckDecay(genpart, 2212, -2212, 211)) {
    return +1 * (100000 * m_nPhotos + 1811);
  }//B+ decays to p+ anti-p- pi+
  if (GenBplusTag::PcheckDecay(genpart, 2212, -2212, 321)) {
    return +1 * (100000 * m_nPhotos + 1812);
  }//B+ decays to p+ anti-p- K+
  if (GenBplusTag::PcheckDecay(genpart, 2212, -2212, 323)) {
    return +1 * (100000 * m_nPhotos + 1813);
  }//B+ decays to p+ anti-p- K*+
  if (GenBplusTag::PcheckDecay(genpart, 2212, -3122, 22)) {
    return +1 * (100000 * m_nPhotos + 1814);
  }//B+ decays to p+ anti-Lambda0 gamma
  if (GenBplusTag::PcheckDecay(genpart, 2212, -3122, 111)) {
    return +1 * (100000 * m_nPhotos + 1815);
  }//B+ decays to p+ anti-Lambda0 pi0
  if (GenBplusTag::PcheckDecay(genpart, 2212, -3122, 211, -211)) {
    return +1 * (100000 * m_nPhotos + 1816);
  }//B+ decays to p+ anti-Lambda0 pi+ pi-
  if (GenBplusTag::PcheckDecay(genpart, 2212, -3122, 113)) {
    return +1 * (100000 * m_nPhotos + 1817);
  }//B+ decays to p+ anti-Lambda0 rho0
  if (GenBplusTag::PcheckDecay(genpart, 2212, -3122, 225)) {
    return +1 * (100000 * m_nPhotos + 1818);
  }//B+ decays to p+ anti-Lambda0 f_2
  if (GenBplusTag::PcheckDecay(genpart, 3122, -3122, 321)) {
    return +1 * (100000 * m_nPhotos + 1819);
  }//B+ decays to Lambda0 anti-Lambda0 K+
  if (GenBplusTag::PcheckDecay(genpart, 3122, -3122, 323)) {
    return +1 * (100000 * m_nPhotos + 1820);
  }//B+ decays to Lambda0 anti-Lambda0 K*+
  if (GenBplusTag::PcheckDecay(genpart, -4122, 2212, 211)) {
    return +1 * (100000 * m_nPhotos + 1821);
  }//B+ decays to anti-Lambda_c- p+ pi+
  if (GenBplusTag::PcheckDecay(genpart, -4122, 2212, 211, 111)) {
    return +1 * (100000 * m_nPhotos + 1822);
  }//B+ decays to anti-Lambda_c- p+ pi+ pi0
  if (GenBplusTag::PcheckDecay(genpart, -4122, 2212, 211, 211, -211)) {
    return +1 * (100000 * m_nPhotos + 1823);
  }//B+ decays to anti-Lambda_c- p+ pi+ pi+ pi-
  if (GenBplusTag::PcheckDecay(genpart, -4112, 2212)) {
    return +1 * (100000 * m_nPhotos + 1824);
  }//B+ decays to anti-Sigma_c0 p+
  if (GenBplusTag::PcheckDecay(genpart, -4112, 2212, 111)) {
    return +1 * (100000 * m_nPhotos + 1825);
  }//B+ decays to anti-Sigma_c0 p+ pi0
  if (GenBplusTag::PcheckDecay(genpart, -4112, 2212, -211, 211)) {
    return +1 * (100000 * m_nPhotos + 1826);
  }//B+ decays to anti-Sigma_c0 p+ pi- pi+
  if (GenBplusTag::PcheckDecay(genpart, -4222, 2212, 211, 211)) {
    return +1 * (100000 * m_nPhotos + 1827);
  }//B+ decays to anti-Sigma_c-- p+ pi+ pi+
  return +1 * genpart.size();
}// Rest of the B+ decays


int GenBplusTag::Mode_B_minus(std::vector<int>genpart)
{
  if (GenBplusTag::PcheckDecay(genpart, 423, 11, -12)) {
    return -1 * (100000 * m_nPhotos + 1001);
  }//B- decays to D*0 e- anti-nu_e
  if (GenBplusTag::PcheckDecay(genpart, 421, 11, -12)) {
    return -1 * (100000 * m_nPhotos + 1002);
  }//B- decays to D0 e- anti-nu_e
  if (GenBplusTag::PcheckDecay(genpart, 10423, 11, -12)) {
    return -1 * (100000 * m_nPhotos + 1003);
  }//B- decays to D_10 e- anti-nu_e
  if (GenBplusTag::PcheckDecay(genpart, 10421, 11, -12)) {
    return -1 * (100000 * m_nPhotos + 1004);
  }//B- decays to D_0*0 e- anti-nu_e
  if (GenBplusTag::PcheckDecay(genpart, 20423, 11, -12)) {
    return -1 * (100000 * m_nPhotos + 1005);
  }//B- decays to D'_10 e- anti-nu_e
  if (GenBplusTag::PcheckDecay(genpart, 425, 11, -12)) {
    return -1 * (100000 * m_nPhotos + 1006);
  }//B- decays to D_2*0 e- anti-nu_e
  if (GenBplusTag::PcheckDecay(genpart, 413, -211, 11, -12)) {
    return -1 * (100000 * m_nPhotos + 1007);
  }//B- decays to D*+ pi- e- anti-nu_e
  if (GenBplusTag::PcheckDecay(genpart, 423, 111, 11, -12)) {
    return -1 * (100000 * m_nPhotos + 1008);
  }//B- decays to D*0 pi0 e- anti-nu_e
  if (GenBplusTag::PcheckDecay(genpart, 411, -211, 11, -12)) {
    return -1 * (100000 * m_nPhotos + 1009);
  }//B- decays to D+ pi- e- anti-nu_e
  if (GenBplusTag::PcheckDecay(genpart, 421, 111, 11, -12)) {
    return -1 * (100000 * m_nPhotos + 1010);
  }//B- decays to D0 pi0 e- anti-nu_e
  if (GenBplusTag::PcheckDecay(genpart, 421, 211, -211, 11, -12)) {
    return -1 * (100000 * m_nPhotos + 1011);
  }//B- decays to D0 pi+ pi- e- anti-nu_e
  if (GenBplusTag::PcheckDecay(genpart, 411, -211, 111, 11, -12)) {
    return -1 * (100000 * m_nPhotos + 1012);
  }//B- decays to D+ pi- pi0 e- anti-nu_e
  if (GenBplusTag::PcheckDecay(genpart, 421, 111, 111, 11, -12)) {
    return -1 * (100000 * m_nPhotos + 1013);
  }//B- decays to D0 pi0 pi0 e- anti-nu_e
  if (GenBplusTag::PcheckDecay(genpart, 423, 211, -211, 11, -12)) {
    return -1 * (100000 * m_nPhotos + 1014);
  }//B- decays to D*0 pi+ pi- e- anti-nu_e
  if (GenBplusTag::PcheckDecay(genpart, 413, -211, 111, 11, -12)) {
    return -1 * (100000 * m_nPhotos + 1015);
  }//B- decays to D*+ pi- pi0 e- anti-nu_e
  if (GenBplusTag::PcheckDecay(genpart, 423, 111, 111, 11, -12)) {
    return -1 * (100000 * m_nPhotos + 1016);
  }//B- decays to D*0 pi0 pi0 e- anti-nu_e
  if (GenBplusTag::PcheckDecay(genpart, 433, -321, 11, -12)) {
    return -1 * (100000 * m_nPhotos + 1017);
  }//B- decays to D_s*+ K- e- anti-nu_e
  if (GenBplusTag::PcheckDecay(genpart, 431, -321, 11, -12)) {
    return -1 * (100000 * m_nPhotos + 1018);
  }//B- decays to D_s+ K- e- anti-nu_e
  if (GenBplusTag::PcheckDecay(genpart, 421, 221, 11, -12)) {
    return -1 * (100000 * m_nPhotos + 1019);
  }//B- decays to D0 eta e- anti-nu_e
  if (GenBplusTag::PcheckDecay(genpart, 423, 221, 11, -12)) {
    return -1 * (100000 * m_nPhotos + 1020);
  }//B- decays to D*0 eta e- anti-nu_e
  if (GenBplusTag::PcheckDecay(genpart, 423, 13, -14)) {
    return -1 * (100000 * m_nPhotos + 1021);
  }//B- decays to D*0 mu- anti-nu_mu
  if (GenBplusTag::PcheckDecay(genpart, 421, 13, -14)) {
    return -1 * (100000 * m_nPhotos + 1022);
  }//B- decays to D0 mu- anti-nu_mu
  if (GenBplusTag::PcheckDecay(genpart, 10423, 13, -14)) {
    return -1 * (100000 * m_nPhotos + 1023);
  }//B- decays to D_10 mu- anti-nu_mu
  if (GenBplusTag::PcheckDecay(genpart, 10421, 13, -14)) {
    return -1 * (100000 * m_nPhotos + 1024);
  }//B- decays to D_0*0 mu- anti-nu_mu
  if (GenBplusTag::PcheckDecay(genpart, 20423, 13, -14)) {
    return -1 * (100000 * m_nPhotos + 1025);
  }//B- decays to D'_10 mu- anti-nu_mu
  if (GenBplusTag::PcheckDecay(genpart, 425, 13, -14)) {
    return -1 * (100000 * m_nPhotos + 1026);
  }//B- decays to D_2*0 mu- anti-nu_mu
  if (GenBplusTag::PcheckDecay(genpart, 413, -211, 13, -14)) {
    return -1 * (100000 * m_nPhotos + 1027);
  }//B- decays to D*+ pi- mu- anti-nu_mu
  if (GenBplusTag::PcheckDecay(genpart, 423, 111, 13, -14)) {
    return -1 * (100000 * m_nPhotos + 1028);
  }//B- decays to D*0 pi0 mu- anti-nu_mu
  if (GenBplusTag::PcheckDecay(genpart, 411, -211, 13, -14)) {
    return -1 * (100000 * m_nPhotos + 1029);
  }//B- decays to D+ pi- mu- anti-nu_mu
  if (GenBplusTag::PcheckDecay(genpart, 421, 111, 13, -14)) {
    return -1 * (100000 * m_nPhotos + 1030);
  }//B- decays to D0 pi0 mu- anti-nu_mu
  if (GenBplusTag::PcheckDecay(genpart, 421, 211, -211, 13, -14)) {
    return -1 * (100000 * m_nPhotos + 1031);
  }//B- decays to D0 pi+ pi- mu- anti-nu_mu
  if (GenBplusTag::PcheckDecay(genpart, 411, -211, 111, 13, -14)) {
    return -1 * (100000 * m_nPhotos + 1032);
  }//B- decays to D+ pi- pi0 mu- anti-nu_mu
  if (GenBplusTag::PcheckDecay(genpart, 421, 111, 111, 13, -14)) {
    return -1 * (100000 * m_nPhotos + 1033);
  }//B- decays to D0 pi0 pi0 mu- anti-nu_mu
  if (GenBplusTag::PcheckDecay(genpart, 423, 211, -211, 13, -14)) {
    return -1 * (100000 * m_nPhotos + 1034);
  }//B- decays to D*0 pi+ pi- mu- anti-nu_mu
  if (GenBplusTag::PcheckDecay(genpart, 413, -211, 111, 13, -14)) {
    return -1 * (100000 * m_nPhotos + 1035);
  }//B- decays to D*+ pi- pi0 mu- anti-nu_mu
  if (GenBplusTag::PcheckDecay(genpart, 423, 111, 111, 13, -14)) {
    return -1 * (100000 * m_nPhotos + 1036);
  }//B- decays to D*0 pi0 pi0 mu- anti-nu_mu
  if (GenBplusTag::PcheckDecay(genpart, 433, -321, 13, -14)) {
    return -1 * (100000 * m_nPhotos + 1037);
  }//B- decays to D_s*+ K- mu- anti-nu_mu
  if (GenBplusTag::PcheckDecay(genpart, 431, -321, 13, -14)) {
    return -1 * (100000 * m_nPhotos + 1038);
  }//B- decays to D_s+ K- mu- anti-nu_mu
  if (GenBplusTag::PcheckDecay(genpart, 421, 221, 13, -14)) {
    return -1 * (100000 * m_nPhotos + 1039);
  }//B- decays to D0 eta mu- anti-nu_mu
  if (GenBplusTag::PcheckDecay(genpart, 423, 221, 13, -14)) {
    return -1 * (100000 * m_nPhotos + 1040);
  }//B- decays to D*0 eta mu- anti-nu_mu
  if (GenBplusTag::PcheckDecay(genpart, 423, 15, -16)) {
    return -1 * (100000 * m_nPhotos + 1041);
  }//B- decays to D*0 tau- anti-nu_tau
  if (GenBplusTag::PcheckDecay(genpart, 421, 15, -16)) {
    return -1 * (100000 * m_nPhotos + 1042);
  }//B- decays to D0 tau- anti-nu_tau
  if (GenBplusTag::PcheckDecay(genpart, 10423, 15, -16)) {
    return -1 * (100000 * m_nPhotos + 1043);
  }//B- decays to D_10 tau- anti-nu_tau
  if (GenBplusTag::PcheckDecay(genpart, 10421, 15, -16)) {
    return -1 * (100000 * m_nPhotos + 1044);
  }//B- decays to D_0*0 tau- anti-nu_tau
  if (GenBplusTag::PcheckDecay(genpart, 20423, 15, -16)) {
    return -1 * (100000 * m_nPhotos + 1045);
  }//B- decays to D'_10 tau- anti-nu_tau
  if (GenBplusTag::PcheckDecay(genpart, 425, 15, -16)) {
    return -1 * (100000 * m_nPhotos + 1046);
  }//B- decays to D_2*0 tau- anti-nu_tau
  if (GenBplusTag::PcheckDecay(genpart, 111, 11, -12)) {
    return -1 * (100000 * m_nPhotos + 1047);
  }//B- decays to pi0 e- anti-nu_e
  if (GenBplusTag::PcheckDecay(genpart, 221, 11, -12)) {
    return -1 * (100000 * m_nPhotos + 1048);
  }//B- decays to eta e- anti-nu_e
  if (GenBplusTag::PcheckDecay(genpart, 113, 11, -12)) {
    return -1 * (100000 * m_nPhotos + 1049);
  }//B- decays to rho0 e- anti-nu_e
  if (GenBplusTag::PcheckDecay(genpart, 223, 11, -12)) {
    return -1 * (100000 * m_nPhotos + 1050);
  }//B- decays to omega e- anti-nu_e
  if (GenBplusTag::PcheckDecay(genpart, 331, 11, -12)) {
    return -1 * (100000 * m_nPhotos + 1051);
  }//B- decays to eta' e- anti-nu_e
  if (GenBplusTag::PcheckDecay(genpart, 100421, 11, -12)) {
    return -1 * (100000 * m_nPhotos + 1052);
  }//B- decays to D(2S)0 e- anti-nu_e
  if (GenBplusTag::PcheckDecay(genpart, 100423, 11, -12)) {
    return -1 * (100000 * m_nPhotos + 1053);
  }//B- decays to D*(2S)0 e- anti-nu_e
  if (GenBplusTag::PcheckDecay(genpart, 61, 11, -12)) {
    return -1 * (100000 * m_nPhotos + 1054);
  }//B- decays to Xu0 e- anti-nu_e
  if (GenBplusTag::PcheckDecay(genpart, 111, 13, -14)) {
    return -1 * (100000 * m_nPhotos + 1055);
  }//B- decays to pi0 mu- anti-nu_mu
  if (GenBplusTag::PcheckDecay(genpart, 221, 13, -14)) {
    return -1 * (100000 * m_nPhotos + 1056);
  }//B- decays to eta mu- anti-nu_mu
  if (GenBplusTag::PcheckDecay(genpart, 113, 13, -14)) {
    return -1 * (100000 * m_nPhotos + 1057);
  }//B- decays to rho0 mu- anti-nu_mu
  if (GenBplusTag::PcheckDecay(genpart, 223, 13, -14)) {
    return -1 * (100000 * m_nPhotos + 1058);
  }//B- decays to omega mu- anti-nu_mu
  if (GenBplusTag::PcheckDecay(genpart, 331, 13, -14)) {
    return -1 * (100000 * m_nPhotos + 1059);
  }//B- decays to eta' mu- anti-nu_mu
  if (GenBplusTag::PcheckDecay(genpart, 100421, 13, -14)) {
    return -1 * (100000 * m_nPhotos + 1060);
  }//B- decays to D(2S)0 mu- anti-nu_mu
  if (GenBplusTag::PcheckDecay(genpart, 100423, 13, -14)) {
    return -1 * (100000 * m_nPhotos + 1061);
  }//B- decays to D*(2S)0 mu- anti-nu_mu
  if (GenBplusTag::PcheckDecay(genpart, 61, 13, -14)) {
    return -1 * (100000 * m_nPhotos + 1062);
  }//B- decays to Xu0 mu- anti-nu_mu
  if (GenBplusTag::PcheckDecay(genpart, 111, 15, -16)) {
    return -1 * (100000 * m_nPhotos + 1063);
  }//B- decays to pi0 tau- anti-nu_tau
  if (GenBplusTag::PcheckDecay(genpart, 221, 15, -16)) {
    return -1 * (100000 * m_nPhotos + 1064);
  }//B- decays to eta tau- anti-nu_tau
  if (GenBplusTag::PcheckDecay(genpart, 113, 15, -16)) {
    return -1 * (100000 * m_nPhotos + 1065);
  }//B- decays to rho0 tau- anti-nu_tau
  if (GenBplusTag::PcheckDecay(genpart, 223, 15, -16)) {
    return -1 * (100000 * m_nPhotos + 1066);
  }//B- decays to omega tau- anti-nu_tau
  if (GenBplusTag::PcheckDecay(genpart, 331, 15, -16)) {
    return -1 * (100000 * m_nPhotos + 1067);
  }//B- decays to eta' tau- anti-nu_tau
  if (GenBplusTag::PcheckDecay(genpart, 20113, 15, -16)) {
    return -1 * (100000 * m_nPhotos + 1068);
  }//B- decays to a_10 tau- anti-nu_tau
  if (GenBplusTag::PcheckDecay(genpart, 10113, 15, -16)) {
    return -1 * (100000 * m_nPhotos + 1069);
  }//B- decays to b_10 tau- anti-nu_tau
  if (GenBplusTag::PcheckDecay(genpart, 10111, 15, -16)) {
    return -1 * (100000 * m_nPhotos + 1070);
  }//B- decays to a_00 tau- anti-nu_tau
  if (GenBplusTag::PcheckDecay(genpart, 10221, 15, -16)) {
    return -1 * (100000 * m_nPhotos + 1071);
  }//B- decays to f_0 tau- anti-nu_tau
  if (GenBplusTag::PcheckDecay(genpart, 10331, 15, -16)) {
    return -1 * (100000 * m_nPhotos + 1072);
  }//B- decays to f'_0 tau- anti-nu_tau
  if (GenBplusTag::PcheckDecay(genpart, 20223, 15, -16)) {
    return -1 * (100000 * m_nPhotos + 1073);
  }//B- decays to f_1 tau- anti-nu_tau
  if (GenBplusTag::PcheckDecay(genpart, 20333, 15, -16)) {
    return -1 * (100000 * m_nPhotos + 1074);
  }//B- decays to f'_1 tau- anti-nu_tau
  if (GenBplusTag::PcheckDecay(genpart, 10223, 15, -16)) {
    return -1 * (100000 * m_nPhotos + 1075);
  }//B- decays to h_1 tau- anti-nu_tau
  if (GenBplusTag::PcheckDecay(genpart, 10333, 15, -16)) {
    return -1 * (100000 * m_nPhotos + 1076);
  }//B- decays to h'_1 tau- anti-nu_tau
  if (GenBplusTag::PcheckDecay(genpart, 225, 15, -16)) {
    return -1 * (100000 * m_nPhotos + 1077);
  }//B- decays to f_2 tau- anti-nu_tau
  if (GenBplusTag::PcheckDecay(genpart, 335, 15, -16)) {
    return -1 * (100000 * m_nPhotos + 1078);
  }//B- decays to f'_2 tau- anti-nu_tau
  if (GenBplusTag::PcheckDecay(genpart, 100421, 15, -16)) {
    return -1 * (100000 * m_nPhotos + 1079);
  }//B- decays to D(2S)0 tau- anti-nu_tau
  if (GenBplusTag::PcheckDecay(genpart, 100423, 15, -16)) {
    return -1 * (100000 * m_nPhotos + 1080);
  }//B- decays to D*(2S)0 tau- anti-nu_tau
  if (GenBplusTag::PcheckDecay(genpart, -431, 111)) {
    return -1 * (100000 * m_nPhotos + 1081);
  }//B- decays to D_s- pi0
  if (GenBplusTag::PcheckDecay(genpart, -433, 111)) {
    return -1 * (100000 * m_nPhotos + 1082);
  }//B- decays to D_s*- pi0
  if (GenBplusTag::PcheckDecay(genpart, 113, -431)) {
    return -1 * (100000 * m_nPhotos + 1083);
  }//B- decays to rho0 D_s-
  if (GenBplusTag::PcheckDecay(genpart, -433, 113)) {
    return -1 * (100000 * m_nPhotos + 1084);
  }//B- decays to D_s*- rho0
  if (GenBplusTag::PcheckDecay(genpart, -323, 22)) {
    return -1 * (100000 * m_nPhotos + 1085);
  }//B- decays to K*- gamma
  if (GenBplusTag::PcheckDecay(genpart, -10323, 22)) {
    return -1 * (100000 * m_nPhotos + 1086);
  }//B- decays to K_1- gamma
  if (GenBplusTag::PcheckDecay(genpart, -325, 22)) {
    return -1 * (100000 * m_nPhotos + 1087);
  }//B- decays to K_2*- gamma
  if (GenBplusTag::PcheckDecay(genpart, -213, 22)) {
    return -1 * (100000 * m_nPhotos + 1088);
  }//B- decays to rho- gamma
  if (GenBplusTag::PcheckDecay(genpart, -20323, 22)) {
    return -1 * (100000 * m_nPhotos + 1089);
  }//B- decays to K'_1- gamma
  if (GenBplusTag::PcheckDecay(genpart, -100323, 22)) {
    return -1 * (100000 * m_nPhotos + 1090);
  }//B- decays to K'*- gamma
  if (GenBplusTag::PcheckDecay(genpart, -30323, 22)) {
    return -1 * (100000 * m_nPhotos + 1091);
  }//B- decays to K''*- gamma
  if (GenBplusTag::PcheckDecay(genpart, -30353, 22)) {
    return -1 * (100000 * m_nPhotos + 1092);
  }//B- decays to anti-Xsu gamma
  if (GenBplusTag::PcheckDecay(genpart, -321, -11, 11)) {
    return -1 * (100000 * m_nPhotos + 1093);
  }//B- decays to K- e+ e-
  if (GenBplusTag::PcheckDecay(genpart, -211, -11, 11)) {
    return -1 * (100000 * m_nPhotos + 1094);
  }//B- decays to pi- e+ e-
  if (GenBplusTag::PcheckDecay(genpart, -323, -11, 11)) {
    return -1 * (100000 * m_nPhotos + 1095);
  }//B- decays to K*- e+ e-
  if (GenBplusTag::PcheckDecay(genpart, -30353, -11, 11)) {
    return -1 * (100000 * m_nPhotos + 1096);
  }//B- decays to anti-Xsu e+ e-
  if (GenBplusTag::PcheckDecay(genpart, -321, 211, -211, -11, 11)) {
    return -1 * (100000 * m_nPhotos + 1097);
  }//B- decays to K- pi+ pi- e+ e-
  if (GenBplusTag::PcheckDecay(genpart, -321, 111, 111, -11, 11)) {
    return -1 * (100000 * m_nPhotos + 1098);
  }//B- decays to K- pi0 pi0 e+ e-
  if (GenBplusTag::PcheckDecay(genpart, -321, 333, -11, 11)) {
    return -1 * (100000 * m_nPhotos + 1099);
  }//B- decays to K- phi e+ e-
  if (GenBplusTag::PcheckDecay(genpart, -321, -13, 13)) {
    return -1 * (100000 * m_nPhotos + 1100);
  }//B- decays to K- mu+ mu-
  if (GenBplusTag::PcheckDecay(genpart, -211, -13, 13)) {
    return -1 * (100000 * m_nPhotos + 1101);
  }//B- decays to pi- mu+ mu-
  if (GenBplusTag::PcheckDecay(genpart, -323, -13, 13)) {
    return -1 * (100000 * m_nPhotos + 1102);
  }//B- decays to K*- mu+ mu-
  if (GenBplusTag::PcheckDecay(genpart, -321, 211, -211, -13, 13)) {
    return -1 * (100000 * m_nPhotos + 1103);
  }//B- decays to K- pi+ pi- mu+ mu-
  if (GenBplusTag::PcheckDecay(genpart, -321, 111, 111, -13, 13)) {
    return -1 * (100000 * m_nPhotos + 1104);
  }//B- decays to K- pi0 pi0 mu+ mu-
  if (GenBplusTag::PcheckDecay(genpart, -321, 333, -13, 13)) {
    return -1 * (100000 * m_nPhotos + 1105);
  }//B- decays to K- phi mu+ mu-
  if (GenBplusTag::PcheckDecay(genpart, -30353, -13, 13)) {
    return -1 * (100000 * m_nPhotos + 1106);
  }//B- decays to anti-Xsu mu+ mu-
  if (GenBplusTag::PcheckDecay(genpart, -321, -15, 15)) {
    return -1 * (100000 * m_nPhotos + 1107);
  }//B- decays to K- tau+ tau-
  if (GenBplusTag::PcheckDecay(genpart, -323, -15, 15)) {
    return -1 * (100000 * m_nPhotos + 1108);
  }//B- decays to K*- tau+ tau-
  if (GenBplusTag::PcheckDecay(genpart, -30353, -15, 15)) {
    return -1 * (100000 * m_nPhotos + 1109);
  }//B- decays to anti-Xsu tau+ tau-
  if (GenBplusTag::PcheckDecay(genpart, 15, -16)) {
    return -1 * (100000 * m_nPhotos + 1110);
  }//B- decays to tau- anti-nu_tau
  if (GenBplusTag::PcheckDecay(genpart, 13, -14)) {
    return -1 * (100000 * m_nPhotos + 1111);
  }//B- decays to mu- anti-nu_mu
  if (GenBplusTag::PcheckDecay(genpart, 11, -12)) {
    return -1 * (100000 * m_nPhotos + 1112);
  }//B- decays to e- anti-nu_e
  if (GenBplusTag::PcheckDecay(genpart, -211, 111, 111)) {
    return -1 * (100000 * m_nPhotos + 1113);
  }//B- decays to pi- pi0 pi0
  if (GenBplusTag::PcheckDecay(genpart, -211, -211, 211)) {
    return -1 * (100000 * m_nPhotos + 1114);
  }//B- decays to pi- pi- pi+
  if (GenBplusTag::PcheckDecay(genpart, 221, -211)) {
    return -1 * (100000 * m_nPhotos + 1115);
  }//B- decays to eta pi-
  if (GenBplusTag::PcheckDecay(genpart, 221, -321)) {
    return -1 * (100000 * m_nPhotos + 1116);
  }//B- decays to eta K-
  if (GenBplusTag::PcheckDecay(genpart, -323, 221)) {
    return -1 * (100000 * m_nPhotos + 1117);
  }//B- decays to K*- eta
  if (GenBplusTag::PcheckDecay(genpart, -213, 221)) {
    return -1 * (100000 * m_nPhotos + 1118);
  }//B- decays to rho- eta
  if (GenBplusTag::PcheckDecay(genpart, 331, -211)) {
    return -1 * (100000 * m_nPhotos + 1119);
  }//B- decays to eta' pi-
  if (GenBplusTag::PcheckDecay(genpart, 331, -321)) {
    return -1 * (100000 * m_nPhotos + 1120);
  }//B- decays to eta' K-
  if (GenBplusTag::PcheckDecay(genpart, -323, 331)) {
    return -1 * (100000 * m_nPhotos + 1121);
  }//B- decays to K*- eta'
  if (GenBplusTag::PcheckDecay(genpart, -213, 331)) {
    return -1 * (100000 * m_nPhotos + 1122);
  }//B- decays to rho- eta'
  if (GenBplusTag::PcheckDecay(genpart, 223, -211)) {
    return -1 * (100000 * m_nPhotos + 1123);
  }//B- decays to omega pi-
  if (GenBplusTag::PcheckDecay(genpart, 223, -321)) {
    return -1 * (100000 * m_nPhotos + 1124);
  }//B- decays to omega K-
  if (GenBplusTag::PcheckDecay(genpart, 223, -323)) {
    return -1 * (100000 * m_nPhotos + 1125);
  }//B- decays to omega K*-
  if (GenBplusTag::PcheckDecay(genpart, 223, -213)) {
    return -1 * (100000 * m_nPhotos + 1126);
  }//B- decays to omega rho-
  if (GenBplusTag::PcheckDecay(genpart, 333, -211)) {
    return -1 * (100000 * m_nPhotos + 1127);
  }//B- decays to phi pi-
  if (GenBplusTag::PcheckDecay(genpart, 333, -321)) {
    return -1 * (100000 * m_nPhotos + 1128);
  }//B- decays to phi K-
  if (GenBplusTag::PcheckDecay(genpart, -311, -211)) {
    return -1 * (100000 * m_nPhotos + 1129);
  }//B- decays to anti-K0 pi-
  if (GenBplusTag::PcheckDecay(genpart, 310, -211)) {
    return -1 * (100000 * m_nPhotos + 1130);
  }//B- decays to K_S0 pi-
  if (GenBplusTag::PcheckDecay(genpart, 130, -211)) {
    return -1 * (100000 * m_nPhotos + 1131);
  }//B- decays to K_L0 pi-
  if (GenBplusTag::PcheckDecay(genpart, 311, -321)) {
    return -1 * (100000 * m_nPhotos + 1132);
  }//B- decays to K0 K-
  if (GenBplusTag::PcheckDecay(genpart, -211, 111)) {
    return -1 * (100000 * m_nPhotos + 1133);
  }//B- decays to pi- pi0
  if (GenBplusTag::PcheckDecay(genpart, -321, 111)) {
    return -1 * (100000 * m_nPhotos + 1134);
  }//B- decays to K- pi0
  if (GenBplusTag::PcheckDecay(genpart, 113, -211)) {
    return -1 * (100000 * m_nPhotos + 1135);
  }//B- decays to rho0 pi-
  if (GenBplusTag::PcheckDecay(genpart, 113, -321)) {
    return -1 * (100000 * m_nPhotos + 1136);
  }//B- decays to rho0 K-
  if (GenBplusTag::PcheckDecay(genpart, -213, -311)) {
    return -1 * (100000 * m_nPhotos + 1137);
  }//B- decays to rho- anti-K0
  if (GenBplusTag::PcheckDecay(genpart, -213, 111)) {
    return -1 * (100000 * m_nPhotos + 1138);
  }//B- decays to rho- pi0
  if (GenBplusTag::PcheckDecay(genpart, 100113, -211)) {
    return -1 * (100000 * m_nPhotos + 1139);
  }//B- decays to rho(2S)0 pi-
  if (GenBplusTag::PcheckDecay(genpart, 100113, -321)) {
    return -1 * (100000 * m_nPhotos + 1140);
  }//B- decays to rho(2S)0 K-
  if (GenBplusTag::PcheckDecay(genpart, 10221, -211)) {
    return -1 * (100000 * m_nPhotos + 1141);
  }//B- decays to f_0 pi-
  if (GenBplusTag::PcheckDecay(genpart, 10221, -321)) {
    return -1 * (100000 * m_nPhotos + 1142);
  }//B- decays to f_0 K-
  if (GenBplusTag::PcheckDecay(genpart, -10211, -311)) {
    return -1 * (100000 * m_nPhotos + 1143);
  }//B- decays to a_0- anti-K0
  if (GenBplusTag::PcheckDecay(genpart, 10111, -321)) {
    return -1 * (100000 * m_nPhotos + 1144);
  }//B- decays to a_00 K-
  if (GenBplusTag::PcheckDecay(genpart, 10111, -211)) {
    return -1 * (100000 * m_nPhotos + 1145);
  }//B- decays to a_00 pi-
  if (GenBplusTag::PcheckDecay(genpart, -10211, 111)) {
    return -1 * (100000 * m_nPhotos + 1146);
  }//B- decays to a_0- pi0
  if (GenBplusTag::PcheckDecay(genpart, 225, -211)) {
    return -1 * (100000 * m_nPhotos + 1147);
  }//B- decays to f_2 pi-
  if (GenBplusTag::PcheckDecay(genpart, -10311, -211)) {
    return -1 * (100000 * m_nPhotos + 1148);
  }//B- decays to anti-K_0*0 pi-
  if (GenBplusTag::PcheckDecay(genpart, 10311, -321)) {
    return -1 * (100000 * m_nPhotos + 1149);
  }//B- decays to K_0*0 K-
  if (GenBplusTag::PcheckDecay(genpart, -10321, 111)) {
    return -1 * (100000 * m_nPhotos + 1150);
  }//B- decays to K_0*- pi0
  if (GenBplusTag::PcheckDecay(genpart, -10321, 311)) {
    return -1 * (100000 * m_nPhotos + 1151);
  }//B- decays to K_0*- K0
  if (GenBplusTag::PcheckDecay(genpart, -323, 111)) {
    return -1 * (100000 * m_nPhotos + 1152);
  }//B- decays to K*- pi0
  if (GenBplusTag::PcheckDecay(genpart, -323, 311)) {
    return -1 * (100000 * m_nPhotos + 1153);
  }//B- decays to K*- K0
  if (GenBplusTag::PcheckDecay(genpart, -321, 211, -211)) {
    return -1 * (100000 * m_nPhotos + 1154);
  }//B- decays to K- pi+ pi-
  if (GenBplusTag::PcheckDecay(genpart, -321, 321, -211)) {
    return -1 * (100000 * m_nPhotos + 1155);
  }//B- decays to K- K+ pi-
  if (GenBplusTag::PcheckDecay(genpart, -321, 321, -321)) {
    return -1 * (100000 * m_nPhotos + 1156);
  }//B- decays to K- K+ K-
  if (GenBplusTag::PcheckDecay(genpart, -321, -321, 211)) {
    return -1 * (100000 * m_nPhotos + 1157);
  }//B- decays to K- K- pi+
  if (GenBplusTag::PcheckDecay(genpart, 321, -211, -211)) {
    return -1 * (100000 * m_nPhotos + 1158);
  }//B- decays to K+ pi- pi-
  if (GenBplusTag::PcheckDecay(genpart, -321, 311, 111)) {
    return -1 * (100000 * m_nPhotos + 1159);
  }//B- decays to K- K0 pi0
  if (GenBplusTag::PcheckDecay(genpart, -321, 311, 221)) {
    return -1 * (100000 * m_nPhotos + 1160);
  }//B- decays to K- K0 eta
  if (GenBplusTag::PcheckDecay(genpart, -321, 311, 331)) {
    return -1 * (100000 * m_nPhotos + 1161);
  }//B- decays to K- K0 eta'
  if (GenBplusTag::PcheckDecay(genpart, -211, -311, 111)) {
    return -1 * (100000 * m_nPhotos + 1162);
  }//B- decays to pi- anti-K0 pi0
  if (GenBplusTag::PcheckDecay(genpart, -211, -311, 221)) {
    return -1 * (100000 * m_nPhotos + 1163);
  }//B- decays to pi- anti-K0 eta
  if (GenBplusTag::PcheckDecay(genpart, -211, -311, 331)) {
    return -1 * (100000 * m_nPhotos + 1164);
  }//B- decays to pi- anti-K0 eta'
  if (GenBplusTag::PcheckDecay(genpart, -211, 111, 221)) {
    return -1 * (100000 * m_nPhotos + 1165);
  }//B- decays to pi- pi0 eta
  if (GenBplusTag::PcheckDecay(genpart, -211, 111, 331)) {
    return -1 * (100000 * m_nPhotos + 1166);
  }//B- decays to pi- pi0 eta'
  if (GenBplusTag::PcheckDecay(genpart, -211, 221, 221)) {
    return -1 * (100000 * m_nPhotos + 1167);
  }//B- decays to pi- eta eta
  if (GenBplusTag::PcheckDecay(genpart, -211, 221, 331)) {
    return -1 * (100000 * m_nPhotos + 1168);
  }//B- decays to pi- eta eta'
  if (GenBplusTag::PcheckDecay(genpart, -321, 111, 111)) {
    return -1 * (100000 * m_nPhotos + 1169);
  }//B- decays to K- pi0 pi0
  if (GenBplusTag::PcheckDecay(genpart, -321, 111, 221)) {
    return -1 * (100000 * m_nPhotos + 1170);
  }//B- decays to K- pi0 eta
  if (GenBplusTag::PcheckDecay(genpart, -321, 111, 331)) {
    return -1 * (100000 * m_nPhotos + 1171);
  }//B- decays to K- pi0 eta'
  if (GenBplusTag::PcheckDecay(genpart, -321, 221, 221)) {
    return -1 * (100000 * m_nPhotos + 1172);
  }//B- decays to K- eta eta
  if (GenBplusTag::PcheckDecay(genpart, -321, 221, 331)) {
    return -1 * (100000 * m_nPhotos + 1173);
  }//B- decays to K- eta eta'
  if (GenBplusTag::PcheckDecay(genpart, -321, 331, 331)) {
    return -1 * (100000 * m_nPhotos + 1174);
  }//B- decays to K- eta' eta'
  if (GenBplusTag::PcheckDecay(genpart, -321, -311, 311)) {
    return -1 * (100000 * m_nPhotos + 1175);
  }//B- decays to K- anti-K0 K0
  if (GenBplusTag::PcheckDecay(genpart, -211, -311, 311)) {
    return -1 * (100000 * m_nPhotos + 1176);
  }//B- decays to pi- anti-K0 K0
  if (GenBplusTag::PcheckDecay(genpart, -213, 113)) {
    return -1 * (100000 * m_nPhotos + 1177);
  }//B- decays to rho- rho0
  if (GenBplusTag::PcheckDecay(genpart, 113, -211, 111)) {
    return -1 * (100000 * m_nPhotos + 1178);
  }//B- decays to rho0 pi- pi0
  if (GenBplusTag::PcheckDecay(genpart, 113, -211, 221)) {
    return -1 * (100000 * m_nPhotos + 1179);
  }//B- decays to rho0 pi- eta
  if (GenBplusTag::PcheckDecay(genpart, 113, -211, 331)) {
    return -1 * (100000 * m_nPhotos + 1180);
  }//B- decays to rho0 pi- eta'
  if (GenBplusTag::PcheckDecay(genpart, 223, -211, 111)) {
    return -1 * (100000 * m_nPhotos + 1181);
  }//B- decays to omega pi- pi0
  if (GenBplusTag::PcheckDecay(genpart, 223, -211, 221)) {
    return -1 * (100000 * m_nPhotos + 1182);
  }//B- decays to omega pi- eta
  if (GenBplusTag::PcheckDecay(genpart, 223, -211, 331)) {
    return -1 * (100000 * m_nPhotos + 1183);
  }//B- decays to omega pi- eta'
  if (GenBplusTag::PcheckDecay(genpart, -213, -211, 211)) {
    return -1 * (100000 * m_nPhotos + 1184);
  }//B- decays to rho- pi- pi+
  if (GenBplusTag::PcheckDecay(genpart, 213, -211, -211)) {
    return -1 * (100000 * m_nPhotos + 1185);
  }//B- decays to rho+ pi- pi-
  if (GenBplusTag::PcheckDecay(genpart, -213, 111, 111)) {
    return -1 * (100000 * m_nPhotos + 1186);
  }//B- decays to rho- pi0 pi0
  if (GenBplusTag::PcheckDecay(genpart, -213, 111, 221)) {
    return -1 * (100000 * m_nPhotos + 1187);
  }//B- decays to rho- pi0 eta
  if (GenBplusTag::PcheckDecay(genpart, -213, 221, 221)) {
    return -1 * (100000 * m_nPhotos + 1188);
  }//B- decays to rho- eta eta
  if (GenBplusTag::PcheckDecay(genpart, -213, 111, 331)) {
    return -1 * (100000 * m_nPhotos + 1189);
  }//B- decays to rho- pi0 eta'
  if (GenBplusTag::PcheckDecay(genpart, 211, -211, -211, 111)) {
    return -1 * (100000 * m_nPhotos + 1190);
  }//B- decays to pi+ pi- pi- pi0
  if (GenBplusTag::PcheckDecay(genpart, 211, -211, -211, 221)) {
    return -1 * (100000 * m_nPhotos + 1191);
  }//B- decays to pi+ pi- pi- eta
  if (GenBplusTag::PcheckDecay(genpart, 211, -211, -211, 331)) {
    return -1 * (100000 * m_nPhotos + 1192);
  }//B- decays to pi+ pi- pi- eta'
  if (GenBplusTag::PcheckDecay(genpart, -211, 111, 111, 111)) {
    return -1 * (100000 * m_nPhotos + 1193);
  }//B- decays to pi- pi0 pi0 pi0
  if (GenBplusTag::PcheckDecay(genpart, -211, 221, 111, 111)) {
    return -1 * (100000 * m_nPhotos + 1194);
  }//B- decays to pi- eta pi0 pi0
  if (GenBplusTag::PcheckDecay(genpart, -211, 221, 221, 111)) {
    return -1 * (100000 * m_nPhotos + 1195);
  }//B- decays to pi- eta eta pi0
  if (GenBplusTag::PcheckDecay(genpart, -211, 331, 111, 111)) {
    return -1 * (100000 * m_nPhotos + 1196);
  }//B- decays to pi- eta' pi0 pi0
  if (GenBplusTag::PcheckDecay(genpart, -211, 331, 221, 111)) {
    return -1 * (100000 * m_nPhotos + 1197);
  }//B- decays to pi- eta' eta pi0
  if (GenBplusTag::PcheckDecay(genpart, 20113, -211)) {
    return -1 * (100000 * m_nPhotos + 1198);
  }//B- decays to a_10 pi-
  if (GenBplusTag::PcheckDecay(genpart, -20213, 111)) {
    return -1 * (100000 * m_nPhotos + 1199);
  }//B- decays to a_1- pi0
  if (GenBplusTag::PcheckDecay(genpart, 10113, -211)) {
    return -1 * (100000 * m_nPhotos + 1200);
  }//B- decays to b_10 pi-
  if (GenBplusTag::PcheckDecay(genpart, -10213, 111)) {
    return -1 * (100000 * m_nPhotos + 1201);
  }//B- decays to b_1- pi0
  if (GenBplusTag::PcheckDecay(genpart, -213, 10221)) {
    return -1 * (100000 * m_nPhotos + 1202);
  }//B- decays to rho- f_0
  if (GenBplusTag::PcheckDecay(genpart, -213, 10111)) {
    return -1 * (100000 * m_nPhotos + 1203);
  }//B- decays to rho- a_00
  if (GenBplusTag::PcheckDecay(genpart, 113, -10211)) {
    return -1 * (100000 * m_nPhotos + 1204);
  }//B- decays to rho0 a_0-
  if (GenBplusTag::PcheckDecay(genpart, 10221, -211, 111)) {
    return -1 * (100000 * m_nPhotos + 1205);
  }//B- decays to f_0 pi- pi0
  if (GenBplusTag::PcheckDecay(genpart, 10111, -211, 111)) {
    return -1 * (100000 * m_nPhotos + 1206);
  }//B- decays to a_00 pi- pi0
  if (GenBplusTag::PcheckDecay(genpart, -10211, 111, 111)) {
    return -1 * (100000 * m_nPhotos + 1207);
  }//B- decays to a_0- pi0 pi0
  if (GenBplusTag::PcheckDecay(genpart, 10211, -211, -211)) {
    return -1 * (100000 * m_nPhotos + 1208);
  }//B- decays to a_0+ pi- pi-
  if (GenBplusTag::PcheckDecay(genpart, -213, -313)) {
    return -1 * (100000 * m_nPhotos + 1209);
  }//B- decays to rho- anti-K*0
  if (GenBplusTag::PcheckDecay(genpart, 113, -323)) {
    return -1 * (100000 * m_nPhotos + 1210);
  }//B- decays to rho0 K*-
  if (GenBplusTag::PcheckDecay(genpart, -211, 111, -313)) {
    return -1 * (100000 * m_nPhotos + 1211);
  }//B- decays to pi- pi0 anti-K*0
  if (GenBplusTag::PcheckDecay(genpart, -211, 221, -313)) {
    return -1 * (100000 * m_nPhotos + 1212);
  }//B- decays to pi- eta anti-K*0
  if (GenBplusTag::PcheckDecay(genpart, 211, -211, -323)) {
    return -1 * (100000 * m_nPhotos + 1213);
  }//B- decays to pi+ pi- K*-
  if (GenBplusTag::PcheckDecay(genpart, 111, 111, -323)) {
    return -1 * (100000 * m_nPhotos + 1214);
  }//B- decays to pi0 pi0 K*-
  if (GenBplusTag::PcheckDecay(genpart, 111, 221, -323)) {
    return -1 * (100000 * m_nPhotos + 1215);
  }//B- decays to pi0 eta K*-
  if (GenBplusTag::PcheckDecay(genpart, 221, 221, -323)) {
    return -1 * (100000 * m_nPhotos + 1216);
  }//B- decays to eta eta K*-
  if (GenBplusTag::PcheckDecay(genpart, 111, 331, -323)) {
    return -1 * (100000 * m_nPhotos + 1217);
  }//B- decays to pi0 eta' K*-
  if (GenBplusTag::PcheckDecay(genpart, 221, 331, -323)) {
    return -1 * (100000 * m_nPhotos + 1218);
  }//B- decays to eta eta' K*-
  if (GenBplusTag::PcheckDecay(genpart, -213, -311, 111)) {
    return -1 * (100000 * m_nPhotos + 1219);
  }//B- decays to rho- anti-K0 pi0
  if (GenBplusTag::PcheckDecay(genpart, -213, -311, 221)) {
    return -1 * (100000 * m_nPhotos + 1220);
  }//B- decays to rho- anti-K0 eta
  if (GenBplusTag::PcheckDecay(genpart, -213, -311, 331)) {
    return -1 * (100000 * m_nPhotos + 1221);
  }//B- decays to rho- anti-K0 eta'
  if (GenBplusTag::PcheckDecay(genpart, -213, -321, 211)) {
    return -1 * (100000 * m_nPhotos + 1222);
  }//B- decays to rho- K- pi+
  if (GenBplusTag::PcheckDecay(genpart, 213, -321, -211)) {
    return -1 * (100000 * m_nPhotos + 1223);
  }//B- decays to rho+ K- pi-
  if (GenBplusTag::PcheckDecay(genpart, 113, -311, -211)) {
    return -1 * (100000 * m_nPhotos + 1224);
  }//B- decays to rho0 anti-K0 pi-
  if (GenBplusTag::PcheckDecay(genpart, 113, -321, 111)) {
    return -1 * (100000 * m_nPhotos + 1225);
  }//B- decays to rho0 K- pi0
  if (GenBplusTag::PcheckDecay(genpart, 113, -321, 221)) {
    return -1 * (100000 * m_nPhotos + 1226);
  }//B- decays to rho0 K- eta
  if (GenBplusTag::PcheckDecay(genpart, 113, -321, 331)) {
    return -1 * (100000 * m_nPhotos + 1227);
  }//B- decays to rho0 K- eta'
  if (GenBplusTag::PcheckDecay(genpart, 223, -311, -211)) {
    return -1 * (100000 * m_nPhotos + 1228);
  }//B- decays to omega anti-K0 pi-
  if (GenBplusTag::PcheckDecay(genpart, 223, -321, 111)) {
    return -1 * (100000 * m_nPhotos + 1229);
  }//B- decays to omega K- pi0
  if (GenBplusTag::PcheckDecay(genpart, 223, -321, 221)) {
    return -1 * (100000 * m_nPhotos + 1230);
  }//B- decays to omega K- eta
  if (GenBplusTag::PcheckDecay(genpart, 223, -321, 331)) {
    return -1 * (100000 * m_nPhotos + 1231);
  }//B- decays to omega K- eta'
  if (GenBplusTag::PcheckDecay(genpart, 211, -211, -211, -311)) {
    return -1 * (100000 * m_nPhotos + 1232);
  }//B- decays to pi+ pi- pi- anti-K0
  if (GenBplusTag::PcheckDecay(genpart, -211, 111, 111, -311)) {
    return -1 * (100000 * m_nPhotos + 1233);
  }//B- decays to pi- pi0 pi0 anti-K0
  if (GenBplusTag::PcheckDecay(genpart, -211, 221, 111, -311)) {
    return -1 * (100000 * m_nPhotos + 1234);
  }//B- decays to pi- eta pi0 anti-K0
  if (GenBplusTag::PcheckDecay(genpart, -211, 221, 221, -311)) {
    return -1 * (100000 * m_nPhotos + 1235);
  }//B- decays to pi- eta eta anti-K0
  if (GenBplusTag::PcheckDecay(genpart, 211, -211, 111, -321)) {
    return -1 * (100000 * m_nPhotos + 1236);
  }//B- decays to pi+ pi- pi0 K-
  if (GenBplusTag::PcheckDecay(genpart, 211, -211, 221, -321)) {
    return -1 * (100000 * m_nPhotos + 1237);
  }//B- decays to pi+ pi- eta K-
  if (GenBplusTag::PcheckDecay(genpart, 111, 111, 111, -321)) {
    return -1 * (100000 * m_nPhotos + 1238);
  }//B- decays to pi0 pi0 pi0 K-
  if (GenBplusTag::PcheckDecay(genpart, 111, 111, 221, -321)) {
    return -1 * (100000 * m_nPhotos + 1239);
  }//B- decays to pi0 pi0 eta K-
  if (GenBplusTag::PcheckDecay(genpart, 111, 221, 221, -321)) {
    return -1 * (100000 * m_nPhotos + 1240);
  }//B- decays to pi0 eta eta K-
  if (GenBplusTag::PcheckDecay(genpart, 111, 111, 331, -321)) {
    return -1 * (100000 * m_nPhotos + 1241);
  }//B- decays to pi0 pi0 eta' K-
  if (GenBplusTag::PcheckDecay(genpart, 111, 221, 331, -321)) {
    return -1 * (100000 * m_nPhotos + 1242);
  }//B- decays to pi0 eta eta' K-
  if (GenBplusTag::PcheckDecay(genpart, -213, -10311)) {
    return -1 * (100000 * m_nPhotos + 1243);
  }//B- decays to rho- anti-K_0*0
  if (GenBplusTag::PcheckDecay(genpart, 113, -10321)) {
    return -1 * (100000 * m_nPhotos + 1244);
  }//B- decays to rho0 K_0*-
  if (GenBplusTag::PcheckDecay(genpart, -211, 111, -10311)) {
    return -1 * (100000 * m_nPhotos + 1245);
  }//B- decays to pi- pi0 anti-K_0*0
  if (GenBplusTag::PcheckDecay(genpart, 111, 111, -10321)) {
    return -1 * (100000 * m_nPhotos + 1246);
  }//B- decays to pi0 pi0 K_0*-
  if (GenBplusTag::PcheckDecay(genpart, -323, 10221)) {
    return -1 * (100000 * m_nPhotos + 1247);
  }//B- decays to K*- f_0
  if (GenBplusTag::PcheckDecay(genpart, 20113, -321)) {
    return -1 * (100000 * m_nPhotos + 1248);
  }//B- decays to a_10 K-
  if (GenBplusTag::PcheckDecay(genpart, -20213, -311)) {
    return -1 * (100000 * m_nPhotos + 1249);
  }//B- decays to a_1- anti-K0
  if (GenBplusTag::PcheckDecay(genpart, 10113, -321)) {
    return -1 * (100000 * m_nPhotos + 1250);
  }//B- decays to b_10 K-
  if (GenBplusTag::PcheckDecay(genpart, -10213, -311)) {
    return -1 * (100000 * m_nPhotos + 1251);
  }//B- decays to b_1- anti-K0
  if (GenBplusTag::PcheckDecay(genpart, -323, 10111)) {
    return -1 * (100000 * m_nPhotos + 1252);
  }//B- decays to K*- a_00
  if (GenBplusTag::PcheckDecay(genpart, -313, -10211)) {
    return -1 * (100000 * m_nPhotos + 1253);
  }//B- decays to anti-K*0 a_0-
  if (GenBplusTag::PcheckDecay(genpart, 313, -323)) {
    return -1 * (100000 * m_nPhotos + 1254);
  }//B- decays to K*0 K*-
  if (GenBplusTag::PcheckDecay(genpart, 333, -213)) {
    return -1 * (100000 * m_nPhotos + 1255);
  }//B- decays to phi rho-
  if (GenBplusTag::PcheckDecay(genpart, 333, -211, 111)) {
    return -1 * (100000 * m_nPhotos + 1256);
  }//B- decays to phi pi- pi0
  if (GenBplusTag::PcheckDecay(genpart, 333, -211, 221)) {
    return -1 * (100000 * m_nPhotos + 1257);
  }//B- decays to phi pi- eta
  if (GenBplusTag::PcheckDecay(genpart, 333, -211, 331)) {
    return -1 * (100000 * m_nPhotos + 1258);
  }//B- decays to phi pi- eta'
  if (GenBplusTag::PcheckDecay(genpart, 321, -321, -213)) {
    return -1 * (100000 * m_nPhotos + 1259);
  }//B- decays to K+ K- rho-
  if (GenBplusTag::PcheckDecay(genpart, 311, -311, -213)) {
    return -1 * (100000 * m_nPhotos + 1260);
  }//B- decays to K0 anti-K0 rho-
  if (GenBplusTag::PcheckDecay(genpart, 311, -321, 113)) {
    return -1 * (100000 * m_nPhotos + 1261);
  }//B- decays to K0 K- rho0
  if (GenBplusTag::PcheckDecay(genpart, 311, -321, 223)) {
    return -1 * (100000 * m_nPhotos + 1262);
  }//B- decays to K0 K- omega
  if (GenBplusTag::PcheckDecay(genpart, 321, -321, -211, 111)) {
    return -1 * (100000 * m_nPhotos + 1263);
  }//B- decays to K+ K- pi- pi0
  if (GenBplusTag::PcheckDecay(genpart, 321, -321, -211, 221)) {
    return -1 * (100000 * m_nPhotos + 1264);
  }//B- decays to K+ K- pi- eta
  if (GenBplusTag::PcheckDecay(genpart, 321, -321, -211, 331)) {
    return -1 * (100000 * m_nPhotos + 1265);
  }//B- decays to K+ K- pi- eta'
  if (GenBplusTag::PcheckDecay(genpart, 311, -311, -211, 111)) {
    return -1 * (100000 * m_nPhotos + 1266);
  }//B- decays to K0 anti-K0 pi- pi0
  if (GenBplusTag::PcheckDecay(genpart, 311, -311, -211, 221)) {
    return -1 * (100000 * m_nPhotos + 1267);
  }//B- decays to K0 anti-K0 pi- eta
  if (GenBplusTag::PcheckDecay(genpart, 311, -311, -211, 331)) {
    return -1 * (100000 * m_nPhotos + 1268);
  }//B- decays to K0 anti-K0 pi- eta'
  if (GenBplusTag::PcheckDecay(genpart, 311, -321, 211, -211)) {
    return -1 * (100000 * m_nPhotos + 1269);
  }//B- decays to K0 K- pi+ pi-
  if (GenBplusTag::PcheckDecay(genpart, 311, -321, 111, 111)) {
    return -1 * (100000 * m_nPhotos + 1270);
  }//B- decays to K0 K- pi0 pi0
  if (GenBplusTag::PcheckDecay(genpart, 311, -321, 111, 221)) {
    return -1 * (100000 * m_nPhotos + 1271);
  }//B- decays to K0 K- pi0 eta
  if (GenBplusTag::PcheckDecay(genpart, 311, -321, 221, 221)) {
    return -1 * (100000 * m_nPhotos + 1272);
  }//B- decays to K0 K- eta eta
  if (GenBplusTag::PcheckDecay(genpart, 311, -321, 111, 331)) {
    return -1 * (100000 * m_nPhotos + 1273);
  }//B- decays to K0 K- pi0 eta'
  if (GenBplusTag::PcheckDecay(genpart, 311, -321, 221, 331)) {
    return -1 * (100000 * m_nPhotos + 1274);
  }//B- decays to K0 K- eta eta'
  if (GenBplusTag::PcheckDecay(genpart, 323, -321, -211)) {
    return -1 * (100000 * m_nPhotos + 1275);
  }//B- decays to K*+ K- pi-
  if (GenBplusTag::PcheckDecay(genpart, -323, 321, -211)) {
    return -1 * (100000 * m_nPhotos + 1276);
  }//B- decays to K*- K+ pi-
  if (GenBplusTag::PcheckDecay(genpart, 313, -311, -211)) {
    return -1 * (100000 * m_nPhotos + 1277);
  }//B- decays to K*0 anti-K0 pi-
  if (GenBplusTag::PcheckDecay(genpart, 311, -313, -211)) {
    return -1 * (100000 * m_nPhotos + 1278);
  }//B- decays to K0 anti-K*0 pi-
  if (GenBplusTag::PcheckDecay(genpart, -321, 313, 111)) {
    return -1 * (100000 * m_nPhotos + 1279);
  }//B- decays to K- K*0 pi0
  if (GenBplusTag::PcheckDecay(genpart, -321, 313, 221)) {
    return -1 * (100000 * m_nPhotos + 1280);
  }//B- decays to K- K*0 eta
  if (GenBplusTag::PcheckDecay(genpart, -321, 313, 331)) {
    return -1 * (100000 * m_nPhotos + 1281);
  }//B- decays to K- K*0 eta'
  if (GenBplusTag::PcheckDecay(genpart, -323, 311, 111)) {
    return -1 * (100000 * m_nPhotos + 1282);
  }//B- decays to K*- K0 pi0
  if (GenBplusTag::PcheckDecay(genpart, -323, 311, 221)) {
    return -1 * (100000 * m_nPhotos + 1283);
  }//B- decays to K*- K0 eta
  if (GenBplusTag::PcheckDecay(genpart, -323, 311, 331)) {
    return -1 * (100000 * m_nPhotos + 1284);
  }//B- decays to K*- K0 eta'
  if (GenBplusTag::PcheckDecay(genpart, 333, -323)) {
    return -1 * (100000 * m_nPhotos + 1285);
  }//B- decays to phi K*-
  if (GenBplusTag::PcheckDecay(genpart, 333, -311, -211)) {
    return -1 * (100000 * m_nPhotos + 1286);
  }//B- decays to phi anti-K0 pi-
  if (GenBplusTag::PcheckDecay(genpart, 333, -321, 111)) {
    return -1 * (100000 * m_nPhotos + 1287);
  }//B- decays to phi K- pi0
  if (GenBplusTag::PcheckDecay(genpart, 333, -321, 221)) {
    return -1 * (100000 * m_nPhotos + 1288);
  }//B- decays to phi K- eta
  if (GenBplusTag::PcheckDecay(genpart, 333, -321, 331)) {
    return -1 * (100000 * m_nPhotos + 1289);
  }//B- decays to phi K- eta'
  if (GenBplusTag::PcheckDecay(genpart, -321, 321, -323)) {
    return -1 * (100000 * m_nPhotos + 1290);
  }//B- decays to K- K+ K*-
  if (GenBplusTag::PcheckDecay(genpart, -321, 323, -321)) {
    return -1 * (100000 * m_nPhotos + 1291);
  }//B- decays to K- K*+ K-
  if (GenBplusTag::PcheckDecay(genpart, -321, 313, -311)) {
    return -1 * (100000 * m_nPhotos + 1292);
  }//B- decays to K- K*0 anti-K0
  if (GenBplusTag::PcheckDecay(genpart, -321, 311, -313)) {
    return -1 * (100000 * m_nPhotos + 1293);
  }//B- decays to K- K0 anti-K*0
  if (GenBplusTag::PcheckDecay(genpart, 311, -323, -311)) {
    return -1 * (100000 * m_nPhotos + 1294);
  }//B- decays to K0 K*- anti-K0
  if (GenBplusTag::PcheckDecay(genpart, 321, -321, -321, 111)) {
    return -1 * (100000 * m_nPhotos + 1295);
  }//B- decays to K+ K- K- pi0
  if (GenBplusTag::PcheckDecay(genpart, 321, -321, -321, 221)) {
    return -1 * (100000 * m_nPhotos + 1296);
  }//B- decays to K+ K- K- eta
  if (GenBplusTag::PcheckDecay(genpart, 321, -321, -321, 331)) {
    return -1 * (100000 * m_nPhotos + 1297);
  }//B- decays to K+ K- K- eta'
  if (GenBplusTag::PcheckDecay(genpart, 321, -321, -311, -211)) {
    return -1 * (100000 * m_nPhotos + 1298);
  }//B- decays to K+ K- anti-K0 pi-
  if (GenBplusTag::PcheckDecay(genpart, 311, -311, -321, 111)) {
    return -1 * (100000 * m_nPhotos + 1299);
  }//B- decays to K0 anti-K0 K- pi0
  if (GenBplusTag::PcheckDecay(genpart, 311, -311, -321, 221)) {
    return -1 * (100000 * m_nPhotos + 1300);
  }//B- decays to K0 anti-K0 K- eta
  if (GenBplusTag::PcheckDecay(genpart, 311, -311, -321, 331)) {
    return -1 * (100000 * m_nPhotos + 1301);
  }//B- decays to K0 anti-K0 K- eta'
  if (GenBplusTag::PcheckDecay(genpart, 311, -311, -311, -211)) {
    return -1 * (100000 * m_nPhotos + 1302);
  }//B- decays to K0 anti-K0 anti-K0 pi-
  if (GenBplusTag::PcheckDecay(genpart, 333, -10321)) {
    return -1 * (100000 * m_nPhotos + 1303);
  }//B- decays to phi K_0*-
  if (GenBplusTag::PcheckDecay(genpart, -321, 321, -10321)) {
    return -1 * (100000 * m_nPhotos + 1304);
  }//B- decays to K- K+ K_0*-
  if (GenBplusTag::PcheckDecay(genpart, -321, -321, 10321)) {
    return -1 * (100000 * m_nPhotos + 1305);
  }//B- decays to K- K- K_0*+
  if (GenBplusTag::PcheckDecay(genpart, -321, -10311, 311)) {
    return -1 * (100000 * m_nPhotos + 1306);
  }//B- decays to K- anti-K_0*0 K0
  if (GenBplusTag::PcheckDecay(genpart, -321, 10311, -311)) {
    return -1 * (100000 * m_nPhotos + 1307);
  }//B- decays to K- K_0*0 anti-K0
  if (GenBplusTag::PcheckDecay(genpart, 333, -20323)) {
    return -1 * (100000 * m_nPhotos + 1308);
  }//B- decays to phi K'_1-
  if (GenBplusTag::PcheckDecay(genpart, -321, 321, -20323)) {
    return -1 * (100000 * m_nPhotos + 1309);
  }//B- decays to K- K+ K'_1-
  if (GenBplusTag::PcheckDecay(genpart, 333, 311, -321)) {
    return -1 * (100000 * m_nPhotos + 1310);
  }//B- decays to phi K0 K-
  if (GenBplusTag::PcheckDecay(genpart, 321, -321, -321, 311)) {
    return -1 * (100000 * m_nPhotos + 1311);
  }//B- decays to K+ K- K- K0
  if (GenBplusTag::PcheckDecay(genpart, 311, -311, 311, -321)) {
    return -1 * (100000 * m_nPhotos + 1312);
  }//B- decays to K0 anti-K0 K0 K-
  if (GenBplusTag::PcheckDecay(genpart, 113, 113, -211)) {
    return -1 * (100000 * m_nPhotos + 1313);
  }//B- decays to rho0 rho0 pi-
  if (GenBplusTag::PcheckDecay(genpart, -213, 113, 111)) {
    return -1 * (100000 * m_nPhotos + 1314);
  }//B- decays to rho- rho0 pi0
  if (GenBplusTag::PcheckDecay(genpart, -213, 113, 221)) {
    return -1 * (100000 * m_nPhotos + 1315);
  }//B- decays to rho- rho0 eta
  if (GenBplusTag::PcheckDecay(genpart, -213, 113, 331)) {
    return -1 * (100000 * m_nPhotos + 1316);
  }//B- decays to rho- rho0 eta'
  if (GenBplusTag::PcheckDecay(genpart, 213, -213, -211)) {
    return -1 * (100000 * m_nPhotos + 1317);
  }//B- decays to rho+ rho- pi-
  if (GenBplusTag::PcheckDecay(genpart, 223, 113, -211)) {
    return -1 * (100000 * m_nPhotos + 1318);
  }//B- decays to omega rho0 pi-
  if (GenBplusTag::PcheckDecay(genpart, 223, -213, 111)) {
    return -1 * (100000 * m_nPhotos + 1319);
  }//B- decays to omega rho- pi0
  if (GenBplusTag::PcheckDecay(genpart, 223, -213, 221)) {
    return -1 * (100000 * m_nPhotos + 1320);
  }//B- decays to omega rho- eta
  if (GenBplusTag::PcheckDecay(genpart, 223, -213, 331)) {
    return -1 * (100000 * m_nPhotos + 1321);
  }//B- decays to omega rho- eta'
  if (GenBplusTag::PcheckDecay(genpart, 113, 211, -211, -211)) {
    return -1 * (100000 * m_nPhotos + 1322);
  }//B- decays to rho0 pi+ pi- pi-
  if (GenBplusTag::PcheckDecay(genpart, 113, -211, 111, 111)) {
    return -1 * (100000 * m_nPhotos + 1323);
  }//B- decays to rho0 pi- pi0 pi0
  if (GenBplusTag::PcheckDecay(genpart, 113, -211, 221, 111)) {
    return -1 * (100000 * m_nPhotos + 1324);
  }//B- decays to rho0 pi- eta pi0
  if (GenBplusTag::PcheckDecay(genpart, 113, -211, 331, 111)) {
    return -1 * (100000 * m_nPhotos + 1325);
  }//B- decays to rho0 pi- eta' pi0
  if (GenBplusTag::PcheckDecay(genpart, 223, -211, 111, 111)) {
    return -1 * (100000 * m_nPhotos + 1326);
  }//B- decays to omega pi- pi0 pi0
  if (GenBplusTag::PcheckDecay(genpart, 223, -211, 221, 111)) {
    return -1 * (100000 * m_nPhotos + 1327);
  }//B- decays to omega pi- eta pi0
  if (GenBplusTag::PcheckDecay(genpart, 223, -211, 331, 111)) {
    return -1 * (100000 * m_nPhotos + 1328);
  }//B- decays to omega pi- eta' pi0
  if (GenBplusTag::PcheckDecay(genpart, 213, -211, -211, 111)) {
    return -1 * (100000 * m_nPhotos + 1329);
  }//B- decays to rho+ pi- pi- pi0
  if (GenBplusTag::PcheckDecay(genpart, 213, -211, -211, 221)) {
    return -1 * (100000 * m_nPhotos + 1330);
  }//B- decays to rho+ pi- pi- eta
  if (GenBplusTag::PcheckDecay(genpart, 213, -211, -211, 331)) {
    return -1 * (100000 * m_nPhotos + 1331);
  }//B- decays to rho+ pi- pi- eta'
  if (GenBplusTag::PcheckDecay(genpart, -213, 211, -211, 111)) {
    return -1 * (100000 * m_nPhotos + 1332);
  }//B- decays to rho- pi+ pi- pi0
  if (GenBplusTag::PcheckDecay(genpart, -213, 211, -211, 221)) {
    return -1 * (100000 * m_nPhotos + 1333);
  }//B- decays to rho- pi+ pi- eta
  if (GenBplusTag::PcheckDecay(genpart, -213, 211, -211, 331)) {
    return -1 * (100000 * m_nPhotos + 1334);
  }//B- decays to rho- pi+ pi- eta'
  if (GenBplusTag::PcheckDecay(genpart, -213, 111, 111, 111)) {
    return -1 * (100000 * m_nPhotos + 1335);
  }//B- decays to rho- pi0 pi0 pi0
  if (GenBplusTag::PcheckDecay(genpart, -213, 111, 221, 111)) {
    return -1 * (100000 * m_nPhotos + 1336);
  }//B- decays to rho- pi0 eta pi0
  if (GenBplusTag::PcheckDecay(genpart, -213, 221, 221, 111)) {
    return -1 * (100000 * m_nPhotos + 1337);
  }//B- decays to rho- eta eta pi0
  if (GenBplusTag::PcheckDecay(genpart, -213, 111, 331, 111)) {
    return -1 * (100000 * m_nPhotos + 1338);
  }//B- decays to rho- pi0 eta' pi0
  if (GenBplusTag::PcheckDecay(genpart, 211, -211, -211, 111, 111)) {
    return -1 * (100000 * m_nPhotos + 1339);
  }//B- decays to pi+ pi- pi- pi0 pi0
  if (GenBplusTag::PcheckDecay(genpart, 211, -211, -211, 221, 111)) {
    return -1 * (100000 * m_nPhotos + 1340);
  }//B- decays to pi+ pi- pi- eta pi0
  if (GenBplusTag::PcheckDecay(genpart, 211, -211, -211, 331, 111)) {
    return -1 * (100000 * m_nPhotos + 1341);
  }//B- decays to pi+ pi- pi- eta' pi0
  if (GenBplusTag::PcheckDecay(genpart, -211, 111, 111, 111, 111)) {
    return -1 * (100000 * m_nPhotos + 1342);
  }//B- decays to pi- pi0 pi0 pi0 pi0
  if (GenBplusTag::PcheckDecay(genpart, -211, 221, 111, 111, 111)) {
    return -1 * (100000 * m_nPhotos + 1343);
  }//B- decays to pi- eta pi0 pi0 pi0
  if (GenBplusTag::PcheckDecay(genpart, -211, 331, 111, 111, 111)) {
    return -1 * (100000 * m_nPhotos + 1344);
  }//B- decays to pi- eta' pi0 pi0 pi0
  if (GenBplusTag::PcheckDecay(genpart, -20213, 113)) {
    return -1 * (100000 * m_nPhotos + 1345);
  }//B- decays to a_1- rho0
  if (GenBplusTag::PcheckDecay(genpart, -20213, 223)) {
    return -1 * (100000 * m_nPhotos + 1346);
  }//B- decays to a_1- omega
  if (GenBplusTag::PcheckDecay(genpart, -20213, 10221)) {
    return -1 * (100000 * m_nPhotos + 1347);
  }//B- decays to a_1- f_0
  if (GenBplusTag::PcheckDecay(genpart, 20113, -211, 111)) {
    return -1 * (100000 * m_nPhotos + 1348);
  }//B- decays to a_10 pi- pi0
  if (GenBplusTag::PcheckDecay(genpart, 20113, -211, 221)) {
    return -1 * (100000 * m_nPhotos + 1349);
  }//B- decays to a_10 pi- eta
  if (GenBplusTag::PcheckDecay(genpart, 20113, -211, 331)) {
    return -1 * (100000 * m_nPhotos + 1350);
  }//B- decays to a_10 pi- eta'
  if (GenBplusTag::PcheckDecay(genpart, 20213, -211, -211)) {
    return -1 * (100000 * m_nPhotos + 1351);
  }//B- decays to a_1+ pi- pi-
  if (GenBplusTag::PcheckDecay(genpart, -20213, 211, -211)) {
    return -1 * (100000 * m_nPhotos + 1352);
  }//B- decays to a_1- pi+ pi-
  if (GenBplusTag::PcheckDecay(genpart, -20213, 111, 111)) {
    return -1 * (100000 * m_nPhotos + 1353);
  }//B- decays to a_1- pi0 pi0
  if (GenBplusTag::PcheckDecay(genpart, -20213, 111, 221)) {
    return -1 * (100000 * m_nPhotos + 1354);
  }//B- decays to a_1- pi0 eta
  if (GenBplusTag::PcheckDecay(genpart, -20213, 111, 331)) {
    return -1 * (100000 * m_nPhotos + 1355);
  }//B- decays to a_1- pi0 eta'
  if (GenBplusTag::PcheckDecay(genpart, -213, 10221, 111)) {
    return -1 * (100000 * m_nPhotos + 1356);
  }//B- decays to rho- f_0 pi0
  if (GenBplusTag::PcheckDecay(genpart, -213, 10221, 221)) {
    return -1 * (100000 * m_nPhotos + 1357);
  }//B- decays to rho- f_0 eta
  if (GenBplusTag::PcheckDecay(genpart, -213, 10221, 331)) {
    return -1 * (100000 * m_nPhotos + 1358);
  }//B- decays to rho- f_0 eta'
  if (GenBplusTag::PcheckDecay(genpart, 113, 10221, -211)) {
    return -1 * (100000 * m_nPhotos + 1359);
  }//B- decays to rho0 f_0 pi-
  if (GenBplusTag::PcheckDecay(genpart, 223, 10221, -211)) {
    return -1 * (100000 * m_nPhotos + 1360);
  }//B- decays to omega f_0 pi-
  if (GenBplusTag::PcheckDecay(genpart, -213, 10111, 111)) {
    return -1 * (100000 * m_nPhotos + 1361);
  }//B- decays to rho- a_00 pi0
  if (GenBplusTag::PcheckDecay(genpart, -213, 10111, 221)) {
    return -1 * (100000 * m_nPhotos + 1362);
  }//B- decays to rho- a_00 eta
  if (GenBplusTag::PcheckDecay(genpart, -213, 10111, 331)) {
    return -1 * (100000 * m_nPhotos + 1363);
  }//B- decays to rho- a_00 eta'
  if (GenBplusTag::PcheckDecay(genpart, 213, -10211, -211)) {
    return -1 * (100000 * m_nPhotos + 1364);
  }//B- decays to rho+ a_0- pi-
  if (GenBplusTag::PcheckDecay(genpart, 113, -10211, 111)) {
    return -1 * (100000 * m_nPhotos + 1365);
  }//B- decays to rho0 a_0- pi0
  if (GenBplusTag::PcheckDecay(genpart, 113, -10211, 221)) {
    return -1 * (100000 * m_nPhotos + 1366);
  }//B- decays to rho0 a_0- eta
  if (GenBplusTag::PcheckDecay(genpart, 113, -10211, 331)) {
    return -1 * (100000 * m_nPhotos + 1367);
  }//B- decays to rho0 a_0- eta'
  if (GenBplusTag::PcheckDecay(genpart, 10221, 211, -211, -211)) {
    return -1 * (100000 * m_nPhotos + 1368);
  }//B- decays to f_0 pi+ pi- pi-
  if (GenBplusTag::PcheckDecay(genpart, 10221, -211, 111, 111)) {
    return -1 * (100000 * m_nPhotos + 1369);
  }//B- decays to f_0 pi- pi0 pi0
  if (GenBplusTag::PcheckDecay(genpart, 10221, -211, 111, 221)) {
    return -1 * (100000 * m_nPhotos + 1370);
  }//B- decays to f_0 pi- pi0 eta
  if (GenBplusTag::PcheckDecay(genpart, 10221, -211, 111, 331)) {
    return -1 * (100000 * m_nPhotos + 1371);
  }//B- decays to f_0 pi- pi0 eta'
  if (GenBplusTag::PcheckDecay(genpart, 10111, -211, 111, 111)) {
    return -1 * (100000 * m_nPhotos + 1372);
  }//B- decays to a_00 pi- pi0 pi0
  if (GenBplusTag::PcheckDecay(genpart, -10211, 111, 111, 111)) {
    return -1 * (100000 * m_nPhotos + 1373);
  }//B- decays to a_0- pi0 pi0 pi0
  if (GenBplusTag::PcheckDecay(genpart, 10211, -211, -211, 111)) {
    return -1 * (100000 * m_nPhotos + 1374);
  }//B- decays to a_0+ pi- pi- pi0
  if (GenBplusTag::PcheckDecay(genpart, -213, -323, 211)) {
    return -1 * (100000 * m_nPhotos + 1375);
  }//B- decays to rho- K*- pi+
  if (GenBplusTag::PcheckDecay(genpart, 213, -323, -211)) {
    return -1 * (100000 * m_nPhotos + 1376);
  }//B- decays to rho+ K*- pi-
  if (GenBplusTag::PcheckDecay(genpart, -213, -313, 111)) {
    return -1 * (100000 * m_nPhotos + 1377);
  }//B- decays to rho- anti-K*0 pi0
  if (GenBplusTag::PcheckDecay(genpart, -213, -313, 221)) {
    return -1 * (100000 * m_nPhotos + 1378);
  }//B- decays to rho- anti-K*0 eta
  if (GenBplusTag::PcheckDecay(genpart, -213, -313, 331)) {
    return -1 * (100000 * m_nPhotos + 1379);
  }//B- decays to rho- anti-K*0 eta'
  if (GenBplusTag::PcheckDecay(genpart, 113, -313, -211)) {
    return -1 * (100000 * m_nPhotos + 1380);
  }//B- decays to rho0 anti-K*0 pi-
  if (GenBplusTag::PcheckDecay(genpart, 113, -323, 111)) {
    return -1 * (100000 * m_nPhotos + 1381);
  }//B- decays to rho0 K*- pi0
  if (GenBplusTag::PcheckDecay(genpart, 113, -323, 221)) {
    return -1 * (100000 * m_nPhotos + 1382);
  }//B- decays to rho0 K*- eta
  if (GenBplusTag::PcheckDecay(genpart, 113, -323, 331)) {
    return -1 * (100000 * m_nPhotos + 1383);
  }//B- decays to rho0 K*- eta'
  if (GenBplusTag::PcheckDecay(genpart, 223, -313, -211)) {
    return -1 * (100000 * m_nPhotos + 1384);
  }//B- decays to omega anti-K*0 pi-
  if (GenBplusTag::PcheckDecay(genpart, 223, -323, 111)) {
    return -1 * (100000 * m_nPhotos + 1385);
  }//B- decays to omega K*- pi0
  if (GenBplusTag::PcheckDecay(genpart, 223, -323, 221)) {
    return -1 * (100000 * m_nPhotos + 1386);
  }//B- decays to omega K*- eta
  if (GenBplusTag::PcheckDecay(genpart, 223, -323, 331)) {
    return -1 * (100000 * m_nPhotos + 1387);
  }//B- decays to omega K*- eta'
  if (GenBplusTag::PcheckDecay(genpart, 211, -211, -313, -211)) {
    return -1 * (100000 * m_nPhotos + 1388);
  }//B- decays to pi+ pi- anti-K*0 pi-
  if (GenBplusTag::PcheckDecay(genpart, -211, 111, -313, 111)) {
    return -1 * (100000 * m_nPhotos + 1389);
  }//B- decays to pi- pi0 anti-K*0 pi0
  if (GenBplusTag::PcheckDecay(genpart, -211, 221, -313, 111)) {
    return -1 * (100000 * m_nPhotos + 1390);
  }//B- decays to pi- eta anti-K*0 pi0
  if (GenBplusTag::PcheckDecay(genpart, -211, 331, -313, 111)) {
    return -1 * (100000 * m_nPhotos + 1391);
  }//B- decays to pi- eta' anti-K*0 pi0
  if (GenBplusTag::PcheckDecay(genpart, -211, 211, -323, 111)) {
    return -1 * (100000 * m_nPhotos + 1392);
  }//B- decays to pi- pi+ K*- pi0
  if (GenBplusTag::PcheckDecay(genpart, -211, 211, -323, 221)) {
    return -1 * (100000 * m_nPhotos + 1393);
  }//B- decays to pi- pi+ K*- eta
  if (GenBplusTag::PcheckDecay(genpart, -211, 211, -323, 331)) {
    return -1 * (100000 * m_nPhotos + 1394);
  }//B- decays to pi- pi+ K*- eta'
  if (GenBplusTag::PcheckDecay(genpart, 111, 111, -323, 111)) {
    return -1 * (100000 * m_nPhotos + 1395);
  }//B- decays to pi0 pi0 K*- pi0
  if (GenBplusTag::PcheckDecay(genpart, 111, 221, -323, 111)) {
    return -1 * (100000 * m_nPhotos + 1396);
  }//B- decays to pi0 eta K*- pi0
  if (GenBplusTag::PcheckDecay(genpart, 221, 221, -323, 111)) {
    return -1 * (100000 * m_nPhotos + 1397);
  }//B- decays to eta eta K*- pi0
  if (GenBplusTag::PcheckDecay(genpart, 111, 331, -323, 111)) {
    return -1 * (100000 * m_nPhotos + 1398);
  }//B- decays to pi0 eta' K*- pi0
  if (GenBplusTag::PcheckDecay(genpart, 221, 331, -323, 111)) {
    return -1 * (100000 * m_nPhotos + 1399);
  }//B- decays to eta eta' K*- pi0
  if (GenBplusTag::PcheckDecay(genpart, -213, -311, 211, -211)) {
    return -1 * (100000 * m_nPhotos + 1400);
  }//B- decays to rho- anti-K0 pi+ pi-
  if (GenBplusTag::PcheckDecay(genpart, -213, -311, 111, 111)) {
    return -1 * (100000 * m_nPhotos + 1401);
  }//B- decays to rho- anti-K0 pi0 pi0
  if (GenBplusTag::PcheckDecay(genpart, -213, -311, 221, 111)) {
    return -1 * (100000 * m_nPhotos + 1402);
  }//B- decays to rho- anti-K0 eta pi0
  if (GenBplusTag::PcheckDecay(genpart, -213, -311, 331, 111)) {
    return -1 * (100000 * m_nPhotos + 1403);
  }//B- decays to rho- anti-K0 eta' pi0
  if (GenBplusTag::PcheckDecay(genpart, -213, -321, 211, 111)) {
    return -1 * (100000 * m_nPhotos + 1404);
  }//B- decays to rho- K- pi+ pi0
  if (GenBplusTag::PcheckDecay(genpart, 213, -321, -211, 111)) {
    return -1 * (100000 * m_nPhotos + 1405);
  }//B- decays to rho+ K- pi- pi0
  if (GenBplusTag::PcheckDecay(genpart, 213, -311, -211, -211)) {
    return -1 * (100000 * m_nPhotos + 1406);
  }//B- decays to rho+ anti-K0 pi- pi-
  if (GenBplusTag::PcheckDecay(genpart, 113, -311, -211, 111)) {
    return -1 * (100000 * m_nPhotos + 1407);
  }//B- decays to rho0 anti-K0 pi- pi0
  if (GenBplusTag::PcheckDecay(genpart, 113, -321, 111, 111)) {
    return -1 * (100000 * m_nPhotos + 1408);
  }//B- decays to rho0 K- pi0 pi0
  if (GenBplusTag::PcheckDecay(genpart, 113, -321, 221, 111)) {
    return -1 * (100000 * m_nPhotos + 1409);
  }//B- decays to rho0 K- eta pi0
  if (GenBplusTag::PcheckDecay(genpart, 113, -321, 331, 111)) {
    return -1 * (100000 * m_nPhotos + 1410);
  }//B- decays to rho0 K- eta' pi0
  if (GenBplusTag::PcheckDecay(genpart, 223, -311, -211, 111)) {
    return -1 * (100000 * m_nPhotos + 1411);
  }//B- decays to omega anti-K0 pi- pi0
  if (GenBplusTag::PcheckDecay(genpart, 223, -321, 111, 111)) {
    return -1 * (100000 * m_nPhotos + 1412);
  }//B- decays to omega K- pi0 pi0
  if (GenBplusTag::PcheckDecay(genpart, 223, -321, 221, 111)) {
    return -1 * (100000 * m_nPhotos + 1413);
  }//B- decays to omega K- eta pi0
  if (GenBplusTag::PcheckDecay(genpart, 223, -321, 331, 111)) {
    return -1 * (100000 * m_nPhotos + 1414);
  }//B- decays to omega K- eta' pi0
  if (GenBplusTag::PcheckDecay(genpart, -211, -211, 211, -311, 111)) {
    return -1 * (100000 * m_nPhotos + 1415);
  }//B- decays to pi- pi- pi+ anti-K0 pi0
  if (GenBplusTag::PcheckDecay(genpart, -211, -211, 211, -311, 221)) {
    return -1 * (100000 * m_nPhotos + 1416);
  }//B- decays to pi- pi- pi+ anti-K0 eta
  if (GenBplusTag::PcheckDecay(genpart, -211, -211, 211, -311, 331)) {
    return -1 * (100000 * m_nPhotos + 1417);
  }//B- decays to pi- pi- pi+ anti-K0 eta'
  if (GenBplusTag::PcheckDecay(genpart, -211, 111, 111, -311, 111)) {
    return -1 * (100000 * m_nPhotos + 1418);
  }//B- decays to pi- pi0 pi0 anti-K0 pi0
  if (GenBplusTag::PcheckDecay(genpart, -211, 221, 111, -311, 111)) {
    return -1 * (100000 * m_nPhotos + 1419);
  }//B- decays to pi- eta pi0 anti-K0 pi0
  if (GenBplusTag::PcheckDecay(genpart, -211, 221, 221, -311, 111)) {
    return -1 * (100000 * m_nPhotos + 1420);
  }//B- decays to pi- eta eta anti-K0 pi0
  if (GenBplusTag::PcheckDecay(genpart, 211, -211, 211, -321, -211)) {
    return -1 * (100000 * m_nPhotos + 1421);
  }//B- decays to pi+ pi- pi+ K- pi-
  if (GenBplusTag::PcheckDecay(genpart, 211, -211, 111, -321, 111)) {
    return -1 * (100000 * m_nPhotos + 1422);
  }//B- decays to pi+ pi- pi0 K- pi0
  if (GenBplusTag::PcheckDecay(genpart, 211, -211, 221, -321, 111)) {
    return -1 * (100000 * m_nPhotos + 1423);
  }//B- decays to pi+ pi- eta K- pi0
  if (GenBplusTag::PcheckDecay(genpart, 211, -211, 331, -321, 111)) {
    return -1 * (100000 * m_nPhotos + 1424);
  }//B- decays to pi+ pi- eta' K- pi0
  if (GenBplusTag::PcheckDecay(genpart, 111, 111, 111, -321, 111)) {
    return -1 * (100000 * m_nPhotos + 1425);
  }//B- decays to pi0 pi0 pi0 K- pi0
  if (GenBplusTag::PcheckDecay(genpart, 111, 111, 221, -321, 111)) {
    return -1 * (100000 * m_nPhotos + 1426);
  }//B- decays to pi0 pi0 eta K- pi0
  if (GenBplusTag::PcheckDecay(genpart, 111, 221, 221, -321, 111)) {
    return -1 * (100000 * m_nPhotos + 1427);
  }//B- decays to pi0 eta eta K- pi0
  if (GenBplusTag::PcheckDecay(genpart, 111, 111, 331, -321, 111)) {
    return -1 * (100000 * m_nPhotos + 1428);
  }//B- decays to pi0 pi0 eta' K- pi0
  if (GenBplusTag::PcheckDecay(genpart, 111, 221, 331, -321, 111)) {
    return -1 * (100000 * m_nPhotos + 1429);
  }//B- decays to pi0 eta eta' K- pi0
  if (GenBplusTag::PcheckDecay(genpart, 113, -10311, -211)) {
    return -1 * (100000 * m_nPhotos + 1430);
  }//B- decays to rho0 anti-K_0*0 pi-
  if (GenBplusTag::PcheckDecay(genpart, -213, -10311, 111)) {
    return -1 * (100000 * m_nPhotos + 1431);
  }//B- decays to rho- anti-K_0*0 pi0
  if (GenBplusTag::PcheckDecay(genpart, -213, -10311, 221)) {
    return -1 * (100000 * m_nPhotos + 1432);
  }//B- decays to rho- anti-K_0*0 eta
  if (GenBplusTag::PcheckDecay(genpart, -213, -10311, 331)) {
    return -1 * (100000 * m_nPhotos + 1433);
  }//B- decays to rho- anti-K_0*0 eta'
  if (GenBplusTag::PcheckDecay(genpart, -213, -10321, 211)) {
    return -1 * (100000 * m_nPhotos + 1434);
  }//B- decays to rho- K_0*- pi+
  if (GenBplusTag::PcheckDecay(genpart, 113, -10321, 111)) {
    return -1 * (100000 * m_nPhotos + 1435);
  }//B- decays to rho0 K_0*- pi0
  if (GenBplusTag::PcheckDecay(genpart, 113, -10321, 221)) {
    return -1 * (100000 * m_nPhotos + 1436);
  }//B- decays to rho0 K_0*- eta
  if (GenBplusTag::PcheckDecay(genpart, 113, -10321, 331)) {
    return -1 * (100000 * m_nPhotos + 1437);
  }//B- decays to rho0 K_0*- eta'
  if (GenBplusTag::PcheckDecay(genpart, -211, -211, -10311, 211)) {
    return -1 * (100000 * m_nPhotos + 1438);
  }//B- decays to pi- pi- anti-K_0*0 pi+
  if (GenBplusTag::PcheckDecay(genpart, -211, 111, -10311, 111)) {
    return -1 * (100000 * m_nPhotos + 1439);
  }//B- decays to pi- pi0 anti-K_0*0 pi0
  if (GenBplusTag::PcheckDecay(genpart, -211, 111, -10311, 221)) {
    return -1 * (100000 * m_nPhotos + 1440);
  }//B- decays to pi- pi0 anti-K_0*0 eta
  if (GenBplusTag::PcheckDecay(genpart, -211, 111, -10311, 331)) {
    return -1 * (100000 * m_nPhotos + 1441);
  }//B- decays to pi- pi0 anti-K_0*0 eta'
  if (GenBplusTag::PcheckDecay(genpart, 111, 111, -10321, 111)) {
    return -1 * (100000 * m_nPhotos + 1442);
  }//B- decays to pi0 pi0 K_0*- pi0
  if (GenBplusTag::PcheckDecay(genpart, 111, 111, -10321, 221)) {
    return -1 * (100000 * m_nPhotos + 1443);
  }//B- decays to pi0 pi0 K_0*- eta
  if (GenBplusTag::PcheckDecay(genpart, 111, 111, -10321, 331)) {
    return -1 * (100000 * m_nPhotos + 1444);
  }//B- decays to pi0 pi0 K_0*- eta'
  if (GenBplusTag::PcheckDecay(genpart, -313, 10221, -211)) {
    return -1 * (100000 * m_nPhotos + 1445);
  }//B- decays to anti-K*0 f_0 pi-
  if (GenBplusTag::PcheckDecay(genpart, -323, 10221, 111)) {
    return -1 * (100000 * m_nPhotos + 1446);
  }//B- decays to K*- f_0 pi0
  if (GenBplusTag::PcheckDecay(genpart, -323, 10221, 221)) {
    return -1 * (100000 * m_nPhotos + 1447);
  }//B- decays to K*- f_0 eta
  if (GenBplusTag::PcheckDecay(genpart, -323, 10221, 331)) {
    return -1 * (100000 * m_nPhotos + 1448);
  }//B- decays to K*- f_0 eta'
  if (GenBplusTag::PcheckDecay(genpart, 20113, -323)) {
    return -1 * (100000 * m_nPhotos + 1449);
  }//B- decays to a_10 K*-
  if (GenBplusTag::PcheckDecay(genpart, 20113, -321, 111)) {
    return -1 * (100000 * m_nPhotos + 1450);
  }//B- decays to a_10 K- pi0
  if (GenBplusTag::PcheckDecay(genpart, 20113, -311, -211)) {
    return -1 * (100000 * m_nPhotos + 1451);
  }//B- decays to a_10 anti-K0 pi-
  if (GenBplusTag::PcheckDecay(genpart, -20213, -313)) {
    return -1 * (100000 * m_nPhotos + 1452);
  }//B- decays to a_1- anti-K*0
  if (GenBplusTag::PcheckDecay(genpart, -20213, -321, 211)) {
    return -1 * (100000 * m_nPhotos + 1453);
  }//B- decays to a_1- K- pi+
  if (GenBplusTag::PcheckDecay(genpart, -20213, -311, 111)) {
    return -1 * (100000 * m_nPhotos + 1454);
  }//B- decays to a_1- anti-K0 pi0
  if (GenBplusTag::PcheckDecay(genpart, -323, 10111, 111)) {
    return -1 * (100000 * m_nPhotos + 1455);
  }//B- decays to K*- a_00 pi0
  if (GenBplusTag::PcheckDecay(genpart, -313, -10211, 111)) {
    return -1 * (100000 * m_nPhotos + 1456);
  }//B- decays to anti-K*0 a_0- pi0
  if (GenBplusTag::PcheckDecay(genpart, 313, -313, -211)) {
    return -1 * (100000 * m_nPhotos + 1457);
  }//B- decays to K*0 anti-K*0 pi-
  if (GenBplusTag::PcheckDecay(genpart, 313, -323, 111)) {
    return -1 * (100000 * m_nPhotos + 1458);
  }//B- decays to K*0 K*- pi0
  if (GenBplusTag::PcheckDecay(genpart, 313, -323, 221)) {
    return -1 * (100000 * m_nPhotos + 1459);
  }//B- decays to K*0 K*- eta
  if (GenBplusTag::PcheckDecay(genpart, 313, -323, 331)) {
    return -1 * (100000 * m_nPhotos + 1460);
  }//B- decays to K*0 K*- eta'
  if (GenBplusTag::PcheckDecay(genpart, 333, 113, -211)) {
    return -1 * (100000 * m_nPhotos + 1461);
  }//B- decays to phi rho0 pi-
  if (GenBplusTag::PcheckDecay(genpart, 333, 223, -211)) {
    return -1 * (100000 * m_nPhotos + 1462);
  }//B- decays to phi omega pi-
  if (GenBplusTag::PcheckDecay(genpart, 333, 10221, -211)) {
    return -1 * (100000 * m_nPhotos + 1463);
  }//B- decays to phi f_0 pi-
  if (GenBplusTag::PcheckDecay(genpart, 333, -213, 111)) {
    return -1 * (100000 * m_nPhotos + 1464);
  }//B- decays to phi rho- pi0
  if (GenBplusTag::PcheckDecay(genpart, 333, -213, 221)) {
    return -1 * (100000 * m_nPhotos + 1465);
  }//B- decays to phi rho- eta
  if (GenBplusTag::PcheckDecay(genpart, 333, -213, 331)) {
    return -1 * (100000 * m_nPhotos + 1466);
  }//B- decays to phi rho- eta'
  if (GenBplusTag::PcheckDecay(genpart, 333, 211, -211, -211)) {
    return -1 * (100000 * m_nPhotos + 1467);
  }//B- decays to phi pi+ pi- pi-
  if (GenBplusTag::PcheckDecay(genpart, 333, -211, 111, 111)) {
    return -1 * (100000 * m_nPhotos + 1468);
  }//B- decays to phi pi- pi0 pi0
  if (GenBplusTag::PcheckDecay(genpart, 333, -211, 221, 111)) {
    return -1 * (100000 * m_nPhotos + 1469);
  }//B- decays to phi pi- eta pi0
  if (GenBplusTag::PcheckDecay(genpart, 333, -211, 331, 111)) {
    return -1 * (100000 * m_nPhotos + 1470);
  }//B- decays to phi pi- eta' pi0
  if (GenBplusTag::PcheckDecay(genpart, 311, -321, 213, -211)) {
    return -1 * (100000 * m_nPhotos + 1471);
  }//B- decays to K0 K- rho+ pi-
  if (GenBplusTag::PcheckDecay(genpart, 311, -321, -213, 211)) {
    return -1 * (100000 * m_nPhotos + 1472);
  }//B- decays to K0 K- rho- pi+
  if (GenBplusTag::PcheckDecay(genpart, 311, -321, 113, 111)) {
    return -1 * (100000 * m_nPhotos + 1473);
  }//B- decays to K0 K- rho0 pi0
  if (GenBplusTag::PcheckDecay(genpart, 311, -321, 113, 221)) {
    return -1 * (100000 * m_nPhotos + 1474);
  }//B- decays to K0 K- rho0 eta
  if (GenBplusTag::PcheckDecay(genpart, 311, -321, 113, 331)) {
    return -1 * (100000 * m_nPhotos + 1475);
  }//B- decays to K0 K- rho0 eta'
  if (GenBplusTag::PcheckDecay(genpart, 311, -321, 223, 111)) {
    return -1 * (100000 * m_nPhotos + 1476);
  }//B- decays to K0 K- omega pi0
  if (GenBplusTag::PcheckDecay(genpart, 311, -321, 10221, 111)) {
    return -1 * (100000 * m_nPhotos + 1477);
  }//B- decays to K0 K- f_0 pi0
  if (GenBplusTag::PcheckDecay(genpart, 321, -321, 113, -211)) {
    return -1 * (100000 * m_nPhotos + 1478);
  }//B- decays to K+ K- rho0 pi-
  if (GenBplusTag::PcheckDecay(genpart, 321, -321, -213, 111)) {
    return -1 * (100000 * m_nPhotos + 1479);
  }//B- decays to K+ K- rho- pi0
  if (GenBplusTag::PcheckDecay(genpart, 321, -321, -213, 221)) {
    return -1 * (100000 * m_nPhotos + 1480);
  }//B- decays to K+ K- rho- eta
  if (GenBplusTag::PcheckDecay(genpart, 321, -321, -213, 331)) {
    return -1 * (100000 * m_nPhotos + 1481);
  }//B- decays to K+ K- rho- eta'
  if (GenBplusTag::PcheckDecay(genpart, 321, -321, 223, -211)) {
    return -1 * (100000 * m_nPhotos + 1482);
  }//B- decays to K+ K- omega pi-
  if (GenBplusTag::PcheckDecay(genpart, 311, -311, 113, -211)) {
    return -1 * (100000 * m_nPhotos + 1483);
  }//B- decays to K0 anti-K0 rho0 pi-
  if (GenBplusTag::PcheckDecay(genpart, 311, -311, 223, -211)) {
    return -1 * (100000 * m_nPhotos + 1484);
  }//B- decays to K0 anti-K0 omega pi-
  if (GenBplusTag::PcheckDecay(genpart, 311, -311, 10221, -211)) {
    return -1 * (100000 * m_nPhotos + 1485);
  }//B- decays to K0 anti-K0 f_0 pi-
  if (GenBplusTag::PcheckDecay(genpart, 311, -311, -213, 111)) {
    return -1 * (100000 * m_nPhotos + 1486);
  }//B- decays to K0 anti-K0 rho- pi0
  if (GenBplusTag::PcheckDecay(genpart, 311, -311, -213, 221)) {
    return -1 * (100000 * m_nPhotos + 1487);
  }//B- decays to K0 anti-K0 rho- eta
  if (GenBplusTag::PcheckDecay(genpart, 311, -311, -213, 331)) {
    return -1 * (100000 * m_nPhotos + 1488);
  }//B- decays to K0 anti-K0 rho- eta'
  if (GenBplusTag::PcheckDecay(genpart, -311, 321, -213, -211)) {
    return -1 * (100000 * m_nPhotos + 1489);
  }//B- decays to anti-K0 K+ rho- pi-
  if (GenBplusTag::PcheckDecay(genpart, -311, 321, -211, -211, 111)) {
    return -1 * (100000 * m_nPhotos + 1490);
  }//B- decays to anti-K0 K+ pi- pi- pi0
  if (GenBplusTag::PcheckDecay(genpart, -311, 321, -211, -211, 221)) {
    return -1 * (100000 * m_nPhotos + 1491);
  }//B- decays to anti-K0 K+ pi- pi- eta
  if (GenBplusTag::PcheckDecay(genpart, -311, 321, -211, -211, 331)) {
    return -1 * (100000 * m_nPhotos + 1492);
  }//B- decays to anti-K0 K+ pi- pi- eta'
  if (GenBplusTag::PcheckDecay(genpart, 321, -321, 211, -211, -211)) {
    return -1 * (100000 * m_nPhotos + 1493);
  }//B- decays to K+ K- pi+ pi- pi-
  if (GenBplusTag::PcheckDecay(genpart, 321, -321, -211, 111, 111)) {
    return -1 * (100000 * m_nPhotos + 1494);
  }//B- decays to K+ K- pi- pi0 pi0
  if (GenBplusTag::PcheckDecay(genpart, 321, -321, -211, 221, 111)) {
    return -1 * (100000 * m_nPhotos + 1495);
  }//B- decays to K+ K- pi- eta pi0
  if (GenBplusTag::PcheckDecay(genpart, 321, -321, -211, 331, 111)) {
    return -1 * (100000 * m_nPhotos + 1496);
  }//B- decays to K+ K- pi- eta' pi0
  if (GenBplusTag::PcheckDecay(genpart, 311, -311, 211, -211, -211)) {
    return -1 * (100000 * m_nPhotos + 1497);
  }//B- decays to K0 anti-K0 pi+ pi- pi-
  if (GenBplusTag::PcheckDecay(genpart, 311, -311, -211, 111, 111)) {
    return -1 * (100000 * m_nPhotos + 1498);
  }//B- decays to K0 anti-K0 pi- pi0 pi0
  if (GenBplusTag::PcheckDecay(genpart, 311, -311, -211, 221, 111)) {
    return -1 * (100000 * m_nPhotos + 1499);
  }//B- decays to K0 anti-K0 pi- eta pi0
  if (GenBplusTag::PcheckDecay(genpart, 311, -311, -211, 331, 111)) {
    return -1 * (100000 * m_nPhotos + 1500);
  }//B- decays to K0 anti-K0 pi- eta' pi0
  if (GenBplusTag::PcheckDecay(genpart, 311, -321, 211, -211, 111)) {
    return -1 * (100000 * m_nPhotos + 1501);
  }//B- decays to K0 K- pi+ pi- pi0
  if (GenBplusTag::PcheckDecay(genpart, 311, -321, 211, -211, 221)) {
    return -1 * (100000 * m_nPhotos + 1502);
  }//B- decays to K0 K- pi+ pi- eta
  if (GenBplusTag::PcheckDecay(genpart, 311, -321, 211, -211, 331)) {
    return -1 * (100000 * m_nPhotos + 1503);
  }//B- decays to K0 K- pi+ pi- eta'
  if (GenBplusTag::PcheckDecay(genpart, 311, -321, 111, 111, 111)) {
    return -1 * (100000 * m_nPhotos + 1504);
  }//B- decays to K0 K- pi0 pi0 pi0
  if (GenBplusTag::PcheckDecay(genpart, 311, -321, 111, 221, 111)) {
    return -1 * (100000 * m_nPhotos + 1505);
  }//B- decays to K0 K- pi0 eta pi0
  if (GenBplusTag::PcheckDecay(genpart, 311, -321, 111, 331, 111)) {
    return -1 * (100000 * m_nPhotos + 1506);
  }//B- decays to K0 K- pi0 eta' pi0
  if (GenBplusTag::PcheckDecay(genpart, -313, 321, -211, -211)) {
    return -1 * (100000 * m_nPhotos + 1507);
  }//B- decays to anti-K*0 K+ pi- pi-
  if (GenBplusTag::PcheckDecay(genpart, 323, -311, -211, -211)) {
    return -1 * (100000 * m_nPhotos + 1508);
  }//B- decays to K*+ anti-K0 pi- pi-
  if (GenBplusTag::PcheckDecay(genpart, -323, 321, -211, 111)) {
    return -1 * (100000 * m_nPhotos + 1509);
  }//B- decays to K*- K+ pi- pi0
  if (GenBplusTag::PcheckDecay(genpart, 323, -321, -211, 111)) {
    return -1 * (100000 * m_nPhotos + 1510);
  }//B- decays to K*+ K- pi- pi0
  if (GenBplusTag::PcheckDecay(genpart, 313, -311, -211, 111)) {
    return -1 * (100000 * m_nPhotos + 1511);
  }//B- decays to K*0 anti-K0 pi- pi0
  if (GenBplusTag::PcheckDecay(genpart, 311, -313, -211, 111)) {
    return -1 * (100000 * m_nPhotos + 1512);
  }//B- decays to K0 anti-K*0 pi- pi0
  if (GenBplusTag::PcheckDecay(genpart, -321, 313, 111, 111)) {
    return -1 * (100000 * m_nPhotos + 1513);
  }//B- decays to K- K*0 pi0 pi0
  if (GenBplusTag::PcheckDecay(genpart, -321, 313, 221, 111)) {
    return -1 * (100000 * m_nPhotos + 1514);
  }//B- decays to K- K*0 eta pi0
  if (GenBplusTag::PcheckDecay(genpart, -321, 313, 331, 111)) {
    return -1 * (100000 * m_nPhotos + 1515);
  }//B- decays to K- K*0 eta' pi0
  if (GenBplusTag::PcheckDecay(genpart, -323, 311, 111, 111)) {
    return -1 * (100000 * m_nPhotos + 1516);
  }//B- decays to K*- K0 pi0 pi0
  if (GenBplusTag::PcheckDecay(genpart, -323, 311, 221, 111)) {
    return -1 * (100000 * m_nPhotos + 1517);
  }//B- decays to K*- K0 eta pi0
  if (GenBplusTag::PcheckDecay(genpart, -323, 311, 331, 111)) {
    return -1 * (100000 * m_nPhotos + 1518);
  }//B- decays to K*- K0 eta' pi0
  if (GenBplusTag::PcheckDecay(genpart, 333, -323, 111)) {
    return -1 * (100000 * m_nPhotos + 1519);
  }//B- decays to phi K*- pi0
  if (GenBplusTag::PcheckDecay(genpart, 333, -321, 113)) {
    return -1 * (100000 * m_nPhotos + 1520);
  }//B- decays to phi K- rho0
  if (GenBplusTag::PcheckDecay(genpart, 333, -321, 223)) {
    return -1 * (100000 * m_nPhotos + 1521);
  }//B- decays to phi K- omega
  if (GenBplusTag::PcheckDecay(genpart, 333, -321, 10221)) {
    return -1 * (100000 * m_nPhotos + 1522);
  }//B- decays to phi K- f_0
  if (GenBplusTag::PcheckDecay(genpart, 333, -311, -213)) {
    return -1 * (100000 * m_nPhotos + 1523);
  }//B- decays to phi anti-K0 rho-
  if (GenBplusTag::PcheckDecay(genpart, 333, -311, -211, 111)) {
    return -1 * (100000 * m_nPhotos + 1524);
  }//B- decays to phi anti-K0 pi- pi0
  if (GenBplusTag::PcheckDecay(genpart, 333, -321, 211, -211)) {
    return -1 * (100000 * m_nPhotos + 1525);
  }//B- decays to phi K- pi+ pi-
  if (GenBplusTag::PcheckDecay(genpart, 333, -321, 111, 111)) {
    return -1 * (100000 * m_nPhotos + 1526);
  }//B- decays to phi K- pi0 pi0
  if (GenBplusTag::PcheckDecay(genpart, 333, -321, 221, 111)) {
    return -1 * (100000 * m_nPhotos + 1527);
  }//B- decays to phi K- eta pi0
  if (GenBplusTag::PcheckDecay(genpart, 333, -321, 331, 111)) {
    return -1 * (100000 * m_nPhotos + 1528);
  }//B- decays to phi K- eta' pi0
  if (GenBplusTag::PcheckDecay(genpart, 321, -321, -323, 111)) {
    return -1 * (100000 * m_nPhotos + 1529);
  }//B- decays to K+ K- K*- pi0
  if (GenBplusTag::PcheckDecay(genpart, -321, 323, -321, 111)) {
    return -1 * (100000 * m_nPhotos + 1530);
  }//B- decays to K- K*+ K- pi0
  if (GenBplusTag::PcheckDecay(genpart, -321, 313, -311, 111)) {
    return -1 * (100000 * m_nPhotos + 1531);
  }//B- decays to K- K*0 anti-K0 pi0
  if (GenBplusTag::PcheckDecay(genpart, -321, 311, -313, 111)) {
    return -1 * (100000 * m_nPhotos + 1532);
  }//B- decays to K- K0 anti-K*0 pi0
  if (GenBplusTag::PcheckDecay(genpart, 311, -323, -311, 111)) {
    return -1 * (100000 * m_nPhotos + 1533);
  }//B- decays to K0 K*- anti-K0 pi0
  if (GenBplusTag::PcheckDecay(genpart, 321, -321, -321, 113)) {
    return -1 * (100000 * m_nPhotos + 1534);
  }//B- decays to K+ K- K- rho0
  if (GenBplusTag::PcheckDecay(genpart, 321, -321, -321, 223)) {
    return -1 * (100000 * m_nPhotos + 1535);
  }//B- decays to K+ K- K- omega
  if (GenBplusTag::PcheckDecay(genpart, 321, -321, -321, 10221)) {
    return -1 * (100000 * m_nPhotos + 1536);
  }//B- decays to K+ K- K- f_0
  if (GenBplusTag::PcheckDecay(genpart, -321, 321, -311, -213)) {
    return -1 * (100000 * m_nPhotos + 1537);
  }//B- decays to K- K+ anti-K0 rho-
  if (GenBplusTag::PcheckDecay(genpart, -321, -321, 321, 111, 111)) {
    return -1 * (100000 * m_nPhotos + 1538);
  }//B- decays to K- K- K+ pi0 pi0
  if (GenBplusTag::PcheckDecay(genpart, -321, -321, 321, 221, 111)) {
    return -1 * (100000 * m_nPhotos + 1539);
  }//B- decays to K- K- K+ eta pi0
  if (GenBplusTag::PcheckDecay(genpart, -321, -321, 321, 331, 111)) {
    return -1 * (100000 * m_nPhotos + 1540);
  }//B- decays to K- K- K+ eta' pi0
  if (GenBplusTag::PcheckDecay(genpart, 321, -321, -311, -211, 111)) {
    return -1 * (100000 * m_nPhotos + 1541);
  }//B- decays to K+ K- anti-K0 pi- pi0
  if (GenBplusTag::PcheckDecay(genpart, 311, -311, -321, 111, 111)) {
    return -1 * (100000 * m_nPhotos + 1542);
  }//B- decays to K0 anti-K0 K- pi0 pi0
  if (GenBplusTag::PcheckDecay(genpart, 311, -311, -321, 221, 111)) {
    return -1 * (100000 * m_nPhotos + 1543);
  }//B- decays to K0 anti-K0 K- eta pi0
  if (GenBplusTag::PcheckDecay(genpart, 311, -311, -321, 331, 111)) {
    return -1 * (100000 * m_nPhotos + 1544);
  }//B- decays to K0 anti-K0 K- eta' pi0
  if (GenBplusTag::PcheckDecay(genpart, 311, -311, -311, -211, 111)) {
    return -1 * (100000 * m_nPhotos + 1545);
  }//B- decays to K0 anti-K0 anti-K0 pi- pi0
  if (GenBplusTag::PcheckDecay(genpart, 333, -10311, -211)) {
    return -1 * (100000 * m_nPhotos + 1546);
  }//B- decays to phi anti-K_0*0 pi-
  if (GenBplusTag::PcheckDecay(genpart, 333, -10321, 111)) {
    return -1 * (100000 * m_nPhotos + 1547);
  }//B- decays to phi K_0*- pi0
  if (GenBplusTag::PcheckDecay(genpart, 333, -10321, 221)) {
    return -1 * (100000 * m_nPhotos + 1548);
  }//B- decays to phi K_0*- eta
  if (GenBplusTag::PcheckDecay(genpart, 333, -10321, 331)) {
    return -1 * (100000 * m_nPhotos + 1549);
  }//B- decays to phi K_0*- eta'
  if (GenBplusTag::PcheckDecay(genpart, -321, 321, -10321, 111)) {
    return -1 * (100000 * m_nPhotos + 1550);
  }//B- decays to K- K+ K_0*- pi0
  if (GenBplusTag::PcheckDecay(genpart, -321, 321, -10321, 221)) {
    return -1 * (100000 * m_nPhotos + 1551);
  }//B- decays to K- K+ K_0*- eta
  if (GenBplusTag::PcheckDecay(genpart, -321, 321, -10321, 331)) {
    return -1 * (100000 * m_nPhotos + 1552);
  }//B- decays to K- K+ K_0*- eta'
  if (GenBplusTag::PcheckDecay(genpart, -321, -321, 10321, 111)) {
    return -1 * (100000 * m_nPhotos + 1553);
  }//B- decays to K- K- K_0*+ pi0
  if (GenBplusTag::PcheckDecay(genpart, -321, -321, 10321, 221)) {
    return -1 * (100000 * m_nPhotos + 1554);
  }//B- decays to K- K- K_0*+ eta
  if (GenBplusTag::PcheckDecay(genpart, -321, -321, 10321, 331)) {
    return -1 * (100000 * m_nPhotos + 1555);
  }//B- decays to K- K- K_0*+ eta'
  if (GenBplusTag::PcheckDecay(genpart, -321, 321, -10311, -211)) {
    return -1 * (100000 * m_nPhotos + 1556);
  }//B- decays to K- K+ anti-K_0*0 pi-
  if (GenBplusTag::PcheckDecay(genpart, -321, -321, 10311, 211)) {
    return -1 * (100000 * m_nPhotos + 1557);
  }//B- decays to K- K- K_0*0 pi+
  if (GenBplusTag::PcheckDecay(genpart, -311, -311, 10311, -211)) {
    return -1 * (100000 * m_nPhotos + 1558);
  }//B- decays to anti-K0 anti-K0 K_0*0 pi-
  if (GenBplusTag::PcheckDecay(genpart, -321, -10311, 311, 111)) {
    return -1 * (100000 * m_nPhotos + 1559);
  }//B- decays to K- anti-K_0*0 K0 pi0
  if (GenBplusTag::PcheckDecay(genpart, -321, -10311, 311, 221)) {
    return -1 * (100000 * m_nPhotos + 1560);
  }//B- decays to K- anti-K_0*0 K0 eta
  if (GenBplusTag::PcheckDecay(genpart, -321, -10311, 311, 331)) {
    return -1 * (100000 * m_nPhotos + 1561);
  }//B- decays to K- anti-K_0*0 K0 eta'
  if (GenBplusTag::PcheckDecay(genpart, -321, 10311, -311, 111)) {
    return -1 * (100000 * m_nPhotos + 1562);
  }//B- decays to K- K_0*0 anti-K0 pi0
  if (GenBplusTag::PcheckDecay(genpart, -321, 10311, -311, 221)) {
    return -1 * (100000 * m_nPhotos + 1563);
  }//B- decays to K- K_0*0 anti-K0 eta
  if (GenBplusTag::PcheckDecay(genpart, -321, 10311, -311, 331)) {
    return -1 * (100000 * m_nPhotos + 1564);
  }//B- decays to K- K_0*0 anti-K0 eta'
  if (GenBplusTag::PcheckDecay(genpart, 333, -20313, -211)) {
    return -1 * (100000 * m_nPhotos + 1565);
  }//B- decays to phi anti-K'_10 pi-
  if (GenBplusTag::PcheckDecay(genpart, 333, -20323, 111)) {
    return -1 * (100000 * m_nPhotos + 1566);
  }//B- decays to phi K'_1- pi0
  if (GenBplusTag::PcheckDecay(genpart, 333, -20323, 221)) {
    return -1 * (100000 * m_nPhotos + 1567);
  }//B- decays to phi K'_1- eta
  if (GenBplusTag::PcheckDecay(genpart, 333, -20323, 331)) {
    return -1 * (100000 * m_nPhotos + 1568);
  }//B- decays to phi K'_1- eta'
  if (GenBplusTag::PcheckDecay(genpart, -321, 321, -20313, -211)) {
    return -1 * (100000 * m_nPhotos + 1569);
  }//B- decays to K- K+ anti-K'_10 pi-
  if (GenBplusTag::PcheckDecay(genpart, -321, 321, -20323, 111)) {
    return -1 * (100000 * m_nPhotos + 1570);
  }//B- decays to K- K+ K'_1- pi0
  if (GenBplusTag::PcheckDecay(genpart, -321, 321, -20323, 221)) {
    return -1 * (100000 * m_nPhotos + 1571);
  }//B- decays to K- K+ K'_1- eta
  if (GenBplusTag::PcheckDecay(genpart, -321, 321, -20323, 331)) {
    return -1 * (100000 * m_nPhotos + 1572);
  }//B- decays to K- K+ K'_1- eta'
  if (GenBplusTag::PcheckDecay(genpart, -321, -321, 20323, 111)) {
    return -1 * (100000 * m_nPhotos + 1573);
  }//B- decays to K- K- K'_1+ pi0
  if (GenBplusTag::PcheckDecay(genpart, -321, -321, -20313, 211)) {
    return -1 * (100000 * m_nPhotos + 1574);
  }//B- decays to K- K- anti-K'_10 pi+
  if (GenBplusTag::PcheckDecay(genpart, 311, -311, -20313, -211)) {
    return -1 * (100000 * m_nPhotos + 1575);
  }//B- decays to K0 anti-K0 anti-K'_10 pi-
  if (GenBplusTag::PcheckDecay(genpart, 311, -311, -20323, 111)) {
    return -1 * (100000 * m_nPhotos + 1576);
  }//B- decays to K0 anti-K0 K'_1- pi0
  if (GenBplusTag::PcheckDecay(genpart, 311, -311, -20323, 221)) {
    return -1 * (100000 * m_nPhotos + 1577);
  }//B- decays to K0 anti-K0 K'_1- eta
  if (GenBplusTag::PcheckDecay(genpart, 311, -311, -20323, 331)) {
    return -1 * (100000 * m_nPhotos + 1578);
  }//B- decays to K0 anti-K0 K'_1- eta'
  if (GenBplusTag::PcheckDecay(genpart, -311, -311, 20313, -211)) {
    return -1 * (100000 * m_nPhotos + 1579);
  }//B- decays to anti-K0 anti-K0 K'_10 pi-
  if (GenBplusTag::PcheckDecay(genpart, 311, 311, -20323, 111)) {
    return -1 * (100000 * m_nPhotos + 1580);
  }//B- decays to K0 K0 K'_1- pi0
  if (GenBplusTag::PcheckDecay(genpart, 311, 311, -20323, 221)) {
    return -1 * (100000 * m_nPhotos + 1581);
  }//B- decays to K0 K0 K'_1- eta
  if (GenBplusTag::PcheckDecay(genpart, 311, 311, -20323, 331)) {
    return -1 * (100000 * m_nPhotos + 1582);
  }//B- decays to K0 K0 K'_1- eta'
  if (GenBplusTag::PcheckDecay(genpart, -321, 311, -20313, 111)) {
    return -1 * (100000 * m_nPhotos + 1583);
  }//B- decays to K- K0 anti-K'_10 pi0
  if (GenBplusTag::PcheckDecay(genpart, -321, 311, -20313, 221)) {
    return -1 * (100000 * m_nPhotos + 1584);
  }//B- decays to K- K0 anti-K'_10 eta
  if (GenBplusTag::PcheckDecay(genpart, -321, 311, -20313, 331)) {
    return -1 * (100000 * m_nPhotos + 1585);
  }//B- decays to K- K0 anti-K'_10 eta'
  if (GenBplusTag::PcheckDecay(genpart, -321, 20313, -311, 111)) {
    return -1 * (100000 * m_nPhotos + 1586);
  }//B- decays to K- K'_10 anti-K0 pi0
  if (GenBplusTag::PcheckDecay(genpart, -321, 20313, -311, 221)) {
    return -1 * (100000 * m_nPhotos + 1587);
  }//B- decays to K- K'_10 anti-K0 eta
  if (GenBplusTag::PcheckDecay(genpart, -321, 20313, -311, 331)) {
    return -1 * (100000 * m_nPhotos + 1588);
  }//B- decays to K- K'_10 anti-K0 eta'
  if (GenBplusTag::PcheckDecay(genpart, 333, 333, -321)) {
    return -1 * (100000 * m_nPhotos + 1589);
  }//B- decays to phi phi K-
  if (GenBplusTag::PcheckDecay(genpart, 333, 333, -211)) {
    return -1 * (100000 * m_nPhotos + 1590);
  }//B- decays to phi phi pi-
  if (GenBplusTag::PcheckDecay(genpart, 333, 321, -321, -321)) {
    return -1 * (100000 * m_nPhotos + 1591);
  }//B- decays to phi K+ K- K-
  if (GenBplusTag::PcheckDecay(genpart, 333, 311, -311, -321)) {
    return -1 * (100000 * m_nPhotos + 1592);
  }//B- decays to phi K0 anti-K0 K-
  if (GenBplusTag::PcheckDecay(genpart, 333, 311, -311, -211)) {
    return -1 * (100000 * m_nPhotos + 1593);
  }//B- decays to phi K0 anti-K0 pi-
  if (GenBplusTag::PcheckDecay(genpart, 333, 311, -321, 111)) {
    return -1 * (100000 * m_nPhotos + 1594);
  }//B- decays to phi K0 K- pi0
  if (GenBplusTag::PcheckDecay(genpart, 321, -321, 321, -321, -211)) {
    return -1 * (100000 * m_nPhotos + 1595);
  }//B- decays to K+ K- K+ K- pi-
  if (GenBplusTag::PcheckDecay(genpart, 321, -321, 311, -311, -211)) {
    return -1 * (100000 * m_nPhotos + 1596);
  }//B- decays to K+ K- K0 anti-K0 pi-
  if (GenBplusTag::PcheckDecay(genpart, 321, -321, -321, 311, 111)) {
    return -1 * (100000 * m_nPhotos + 1597);
  }//B- decays to K+ K- K- K0 pi0
  if (GenBplusTag::PcheckDecay(genpart, 311, -311, 311, -321, 111)) {
    return -1 * (100000 * m_nPhotos + 1598);
  }//B- decays to K0 anti-K0 K0 K- pi0
  if (GenBplusTag::PcheckDecay(genpart, 333, 333, -323)) {
    return -1 * (100000 * m_nPhotos + 1599);
  }//B- decays to phi phi K*-
  if (GenBplusTag::PcheckDecay(genpart, 333, 321, -321, -323)) {
    return -1 * (100000 * m_nPhotos + 1600);
  }//B- decays to phi K+ K- K*-
  if (GenBplusTag::PcheckDecay(genpart, 333, 311, -311, -323)) {
    return -1 * (100000 * m_nPhotos + 1601);
  }//B- decays to phi K0 anti-K0 K*-
  if (GenBplusTag::PcheckDecay(genpart, 333, 311, -321, 313)) {
    return -1 * (100000 * m_nPhotos + 1602);
  }//B- decays to phi K0 K- K*0
  if (GenBplusTag::PcheckDecay(genpart, 333, 321, -321, -321, 111)) {
    return -1 * (100000 * m_nPhotos + 1603);
  }//B- decays to phi K+ K- K- pi0
  if (GenBplusTag::PcheckDecay(genpart, 333, 321, -321, -311, -211)) {
    return -1 * (100000 * m_nPhotos + 1604);
  }//B- decays to phi K+ K- anti-K0 pi-
  if (GenBplusTag::PcheckDecay(genpart, 333, 311, -311, -321, 111)) {
    return -1 * (100000 * m_nPhotos + 1605);
  }//B- decays to phi K0 anti-K0 K- pi0
  if (GenBplusTag::PcheckDecay(genpart, 333, 311, -311, -311, -211)) {
    return -1 * (100000 * m_nPhotos + 1606);
  }//B- decays to phi K0 anti-K0 anti-K0 pi-
  if (GenBplusTag::PcheckDecay(genpart, 333, 311, -321, -321, 211)) {
    return -1 * (100000 * m_nPhotos + 1607);
  }//B- decays to phi K0 K- K- pi+
  if (GenBplusTag::PcheckDecay(genpart, -20213, 20113)) {
    return -1 * (100000 * m_nPhotos + 1608);
  }//B- decays to a_1- a_10
  if (GenBplusTag::PcheckDecay(genpart, 443, -321)) {
    return -1 * (100000 * m_nPhotos + 1609);
  }//B- decays to J/psi K-
  if (GenBplusTag::PcheckDecay(genpart, 443, -323)) {
    return -1 * (100000 * m_nPhotos + 1610);
  }//B- decays to J/psi K*-
  if (GenBplusTag::PcheckDecay(genpart, 443, -211)) {
    return -1 * (100000 * m_nPhotos + 1611);
  }//B- decays to J/psi pi-
  if (GenBplusTag::PcheckDecay(genpart, 443, -213)) {
    return -1 * (100000 * m_nPhotos + 1612);
  }//B- decays to J/psi rho-
  if (GenBplusTag::PcheckDecay(genpart, 443, -311, -211)) {
    return -1 * (100000 * m_nPhotos + 1613);
  }//B- decays to J/psi anti-K0 pi-
  if (GenBplusTag::PcheckDecay(genpart, 443, -321, 111)) {
    return -1 * (100000 * m_nPhotos + 1614);
  }//B- decays to J/psi K- pi0
  if (GenBplusTag::PcheckDecay(genpart, 443, -20323)) {
    return -1 * (100000 * m_nPhotos + 1615);
  }//B- decays to J/psi K'_1-
  if (GenBplusTag::PcheckDecay(genpart, 443, -325)) {
    return -1 * (100000 * m_nPhotos + 1616);
  }//B- decays to J/psi K_2*-
  if (GenBplusTag::PcheckDecay(genpart, 443, -10323)) {
    return -1 * (100000 * m_nPhotos + 1617);
  }//B- decays to J/psi K_1-
  if (GenBplusTag::PcheckDecay(genpart, 443, -321, 211, -211)) {
    return -1 * (100000 * m_nPhotos + 1618);
  }//B- decays to J/psi K- pi+ pi-
  if (GenBplusTag::PcheckDecay(genpart, 443, -321, 321, -321)) {
    return -1 * (100000 * m_nPhotos + 1619);
  }//B- decays to J/psi K- K+ K-
  if (GenBplusTag::PcheckDecay(genpart, 443, 221, -321)) {
    return -1 * (100000 * m_nPhotos + 1620);
  }//B- decays to J/psi eta K-
  if (GenBplusTag::PcheckDecay(genpart, 443, 333, -321)) {
    return -1 * (100000 * m_nPhotos + 1621);
  }//B- decays to J/psi phi K-
  if (GenBplusTag::PcheckDecay(genpart, 443, 223, -321)) {
    return -1 * (100000 * m_nPhotos + 1622);
  }//B- decays to J/psi omega K-
  if (GenBplusTag::PcheckDecay(genpart, 443, -211, -211, -211, 211, 211)) {
    return -1 * (100000 * m_nPhotos + 1623);
  }//B- decays to J/psi pi- pi- pi- pi+ pi+
  if (GenBplusTag::PcheckDecay(genpart, 443, -2212, 3122)) {
    return -1 * (100000 * m_nPhotos + 1624);
  }//B- decays to J/psi anti-p- Lambda0
  if (GenBplusTag::PcheckDecay(genpart, 100443, -321)) {
    return -1 * (100000 * m_nPhotos + 1625);
  }//B- decays to psi(2S) K-
  if (GenBplusTag::PcheckDecay(genpart, 100443, -323)) {
    return -1 * (100000 * m_nPhotos + 1626);
  }//B- decays to psi(2S) K*-
  if (GenBplusTag::PcheckDecay(genpart, 100443, -311, -211)) {
    return -1 * (100000 * m_nPhotos + 1627);
  }//B- decays to psi(2S) anti-K0 pi-
  if (GenBplusTag::PcheckDecay(genpart, 100443, -321, 111)) {
    return -1 * (100000 * m_nPhotos + 1628);
  }//B- decays to psi(2S) K- pi0
  if (GenBplusTag::PcheckDecay(genpart, 100443, -321, 211, -211)) {
    return -1 * (100000 * m_nPhotos + 1629);
  }//B- decays to psi(2S) K- pi+ pi-
  if (GenBplusTag::PcheckDecay(genpart, 100443, -321, 111, 111)) {
    return -1 * (100000 * m_nPhotos + 1630);
  }//B- decays to psi(2S) K- pi0 pi0
  if (GenBplusTag::PcheckDecay(genpart, 100443, -311, -211, 111)) {
    return -1 * (100000 * m_nPhotos + 1631);
  }//B- decays to psi(2S) anti-K0 pi- pi0
  if (GenBplusTag::PcheckDecay(genpart, 100443, -10323)) {
    return -1 * (100000 * m_nPhotos + 1632);
  }//B- decays to psi(2S) K_1-
  if (GenBplusTag::PcheckDecay(genpart, 100443, -211)) {
    return -1 * (100000 * m_nPhotos + 1633);
  }//B- decays to psi(2S) pi-
  if (GenBplusTag::PcheckDecay(genpart, 100443, 333, -321)) {
    return -1 * (100000 * m_nPhotos + 1634);
  }//B- decays to psi(2S) phi K-
  if (GenBplusTag::PcheckDecay(genpart, 441, -321)) {
    return -1 * (100000 * m_nPhotos + 1635);
  }//B- decays to eta_c K-
  if (GenBplusTag::PcheckDecay(genpart, -323, 441)) {
    return -1 * (100000 * m_nPhotos + 1636);
  }//B- decays to K*- eta_c
  if (GenBplusTag::PcheckDecay(genpart, 441, -311, -211)) {
    return -1 * (100000 * m_nPhotos + 1637);
  }//B- decays to eta_c anti-K0 pi-
  if (GenBplusTag::PcheckDecay(genpart, 441, -321, 111)) {
    return -1 * (100000 * m_nPhotos + 1638);
  }//B- decays to eta_c K- pi0
  if (GenBplusTag::PcheckDecay(genpart, 441, -321, 211, -211)) {
    return -1 * (100000 * m_nPhotos + 1639);
  }//B- decays to eta_c K- pi+ pi-
  if (GenBplusTag::PcheckDecay(genpart, 441, -321, 111, 111)) {
    return -1 * (100000 * m_nPhotos + 1640);
  }//B- decays to eta_c K- pi0 pi0
  if (GenBplusTag::PcheckDecay(genpart, 441, -311, -211, 111)) {
    return -1 * (100000 * m_nPhotos + 1641);
  }//B- decays to eta_c anti-K0 pi- pi0
  if (GenBplusTag::PcheckDecay(genpart, 100441, -321)) {
    return -1 * (100000 * m_nPhotos + 1642);
  }//B- decays to eta_c(2S) K-
  if (GenBplusTag::PcheckDecay(genpart, -323, 100441)) {
    return -1 * (100000 * m_nPhotos + 1643);
  }//B- decays to K*- eta_c(2S)
  if (GenBplusTag::PcheckDecay(genpart, 100441, -311, -211)) {
    return -1 * (100000 * m_nPhotos + 1644);
  }//B- decays to eta_c(2S) anti-K0 pi-
  if (GenBplusTag::PcheckDecay(genpart, 100441, -321, 111)) {
    return -1 * (100000 * m_nPhotos + 1645);
  }//B- decays to eta_c(2S) K- pi0
  if (GenBplusTag::PcheckDecay(genpart, 100441, -321, 211, -211)) {
    return -1 * (100000 * m_nPhotos + 1646);
  }//B- decays to eta_c(2S) K- pi+ pi-
  if (GenBplusTag::PcheckDecay(genpart, 100441, -321, 111, 111)) {
    return -1 * (100000 * m_nPhotos + 1647);
  }//B- decays to eta_c(2S) K- pi0 pi0
  if (GenBplusTag::PcheckDecay(genpart, 100441, -311, -211, 111)) {
    return -1 * (100000 * m_nPhotos + 1648);
  }//B- decays to eta_c(2S) anti-K0 pi- pi0
  if (GenBplusTag::PcheckDecay(genpart, 10441, -321)) {
    return -1 * (100000 * m_nPhotos + 1649);
  }//B- decays to chi_c0 K-
  if (GenBplusTag::PcheckDecay(genpart, -323, 10441)) {
    return -1 * (100000 * m_nPhotos + 1650);
  }//B- decays to K*- chi_c0
  if (GenBplusTag::PcheckDecay(genpart, 10441, -311, -211)) {
    return -1 * (100000 * m_nPhotos + 1651);
  }//B- decays to chi_c0 anti-K0 pi-
  if (GenBplusTag::PcheckDecay(genpart, 10441, -321, 111)) {
    return -1 * (100000 * m_nPhotos + 1652);
  }//B- decays to chi_c0 K- pi0
  if (GenBplusTag::PcheckDecay(genpart, 10441, -321, 211, -211)) {
    return -1 * (100000 * m_nPhotos + 1653);
  }//B- decays to chi_c0 K- pi+ pi-
  if (GenBplusTag::PcheckDecay(genpart, 10441, -321, 111, 111)) {
    return -1 * (100000 * m_nPhotos + 1654);
  }//B- decays to chi_c0 K- pi0 pi0
  if (GenBplusTag::PcheckDecay(genpart, 10441, -311, -211, 111)) {
    return -1 * (100000 * m_nPhotos + 1655);
  }//B- decays to chi_c0 anti-K0 pi- pi0
  if (GenBplusTag::PcheckDecay(genpart, 20443, -321)) {
    return -1 * (100000 * m_nPhotos + 1656);
  }//B- decays to chi_c1 K-
  if (GenBplusTag::PcheckDecay(genpart, 20443, -323)) {
    return -1 * (100000 * m_nPhotos + 1657);
  }//B- decays to chi_c1 K*-
  if (GenBplusTag::PcheckDecay(genpart, 20443, -311, -211)) {
    return -1 * (100000 * m_nPhotos + 1658);
  }//B- decays to chi_c1 anti-K0 pi-
  if (GenBplusTag::PcheckDecay(genpart, 20443, -321, 111)) {
    return -1 * (100000 * m_nPhotos + 1659);
  }//B- decays to chi_c1 K- pi0
  if (GenBplusTag::PcheckDecay(genpart, 20443, -321, 211, -211)) {
    return -1 * (100000 * m_nPhotos + 1660);
  }//B- decays to chi_c1 K- pi+ pi-
  if (GenBplusTag::PcheckDecay(genpart, 20443, -321, 111, 111)) {
    return -1 * (100000 * m_nPhotos + 1661);
  }//B- decays to chi_c1 K- pi0 pi0
  if (GenBplusTag::PcheckDecay(genpart, 20443, -311, -211, 111)) {
    return -1 * (100000 * m_nPhotos + 1662);
  }//B- decays to chi_c1 anti-K0 pi- pi0
  if (GenBplusTag::PcheckDecay(genpart, 20443, -211)) {
    return -1 * (100000 * m_nPhotos + 1663);
  }//B- decays to chi_c1 pi-
  if (GenBplusTag::PcheckDecay(genpart, 445, -321)) {
    return -1 * (100000 * m_nPhotos + 1664);
  }//B- decays to chi_c2 K-
  if (GenBplusTag::PcheckDecay(genpart, 445, -323)) {
    return -1 * (100000 * m_nPhotos + 1665);
  }//B- decays to chi_c2 K*-
  if (GenBplusTag::PcheckDecay(genpart, 445, -311, -211)) {
    return -1 * (100000 * m_nPhotos + 1666);
  }//B- decays to chi_c2 anti-K0 pi-
  if (GenBplusTag::PcheckDecay(genpart, 445, -321, 111)) {
    return -1 * (100000 * m_nPhotos + 1667);
  }//B- decays to chi_c2 K- pi0
  if (GenBplusTag::PcheckDecay(genpart, 445, -321, 211, -211)) {
    return -1 * (100000 * m_nPhotos + 1668);
  }//B- decays to chi_c2 K- pi+ pi-
  if (GenBplusTag::PcheckDecay(genpart, 445, -321, 111, 111)) {
    return -1 * (100000 * m_nPhotos + 1669);
  }//B- decays to chi_c2 K- pi0 pi0
  if (GenBplusTag::PcheckDecay(genpart, 445, -311, -211, 111)) {
    return -1 * (100000 * m_nPhotos + 1670);
  }//B- decays to chi_c2 anti-K0 pi- pi0
  if (GenBplusTag::PcheckDecay(genpart, 30443, -321)) {
    return -1 * (100000 * m_nPhotos + 1671);
  }//B- decays to psi(3770) K-
  if (GenBplusTag::PcheckDecay(genpart, 30443, -323)) {
    return -1 * (100000 * m_nPhotos + 1672);
  }//B- decays to psi(3770) K*-
  if (GenBplusTag::PcheckDecay(genpart, 30443, -311, -211)) {
    return -1 * (100000 * m_nPhotos + 1673);
  }//B- decays to psi(3770) anti-K0 pi-
  if (GenBplusTag::PcheckDecay(genpart, 30443, -321, 111)) {
    return -1 * (100000 * m_nPhotos + 1674);
  }//B- decays to psi(3770) K- pi0
  if (GenBplusTag::PcheckDecay(genpart, 30443, -321, 211, -211)) {
    return -1 * (100000 * m_nPhotos + 1675);
  }//B- decays to psi(3770) K- pi+ pi-
  if (GenBplusTag::PcheckDecay(genpart, 30443, -321, 111, 111)) {
    return -1 * (100000 * m_nPhotos + 1676);
  }//B- decays to psi(3770) K- pi0 pi0
  if (GenBplusTag::PcheckDecay(genpart, 30443, -311, -211, 111)) {
    return -1 * (100000 * m_nPhotos + 1677);
  }//B- decays to psi(3770) anti-K0 pi- pi0
  if (GenBplusTag::PcheckDecay(genpart, 30443, -10323)) {
    return -1 * (100000 * m_nPhotos + 1678);
  }//B- decays to psi(3770) K_1-
  if (GenBplusTag::PcheckDecay(genpart, 9010443, -321)) {
    return -1 * (100000 * m_nPhotos + 1679);
  }//B- decays to psi(4160) K-
  if (GenBplusTag::PcheckDecay(genpart, 10443, -321)) {
    return -1 * (100000 * m_nPhotos + 1680);
  }//B- decays to h_c K-
  if (GenBplusTag::PcheckDecay(genpart, 421, -431)) {
    return -1 * (100000 * m_nPhotos + 1681);
  }//B- decays to D0 D_s-
  if (GenBplusTag::PcheckDecay(genpart, 423, -431)) {
    return -1 * (100000 * m_nPhotos + 1682);
  }//B- decays to D*0 D_s-
  if (GenBplusTag::PcheckDecay(genpart, -433, 421)) {
    return -1 * (100000 * m_nPhotos + 1683);
  }//B- decays to D_s*- D0
  if (GenBplusTag::PcheckDecay(genpart, -433, 423)) {
    return -1 * (100000 * m_nPhotos + 1684);
  }//B- decays to D_s*- D*0
  if (GenBplusTag::PcheckDecay(genpart, 20423, -431)) {
    return -1 * (100000 * m_nPhotos + 1685);
  }//B- decays to D'_10 D_s-
  if (GenBplusTag::PcheckDecay(genpart, 20423, -433)) {
    return -1 * (100000 * m_nPhotos + 1686);
  }//B- decays to D'_10 D_s*-
  if (GenBplusTag::PcheckDecay(genpart, 10423, -431)) {
    return -1 * (100000 * m_nPhotos + 1687);
  }//B- decays to D_10 D_s-
  if (GenBplusTag::PcheckDecay(genpart, 10423, -433)) {
    return -1 * (100000 * m_nPhotos + 1688);
  }//B- decays to D_10 D_s*-
  if (GenBplusTag::PcheckDecay(genpart, 425, -431)) {
    return -1 * (100000 * m_nPhotos + 1689);
  }//B- decays to D_2*0 D_s-
  if (GenBplusTag::PcheckDecay(genpart, 425, -433)) {
    return -1 * (100000 * m_nPhotos + 1690);
  }//B- decays to D_2*0 D_s*-
  if (GenBplusTag::PcheckDecay(genpart, -431, 411, -211)) {
    return -1 * (100000 * m_nPhotos + 1691);
  }//B- decays to D_s- D+ pi-
  if (GenBplusTag::PcheckDecay(genpart, -431, 421, 111)) {
    return -1 * (100000 * m_nPhotos + 1692);
  }//B- decays to D_s- D0 pi0
  if (GenBplusTag::PcheckDecay(genpart, -433, 411, -211)) {
    return -1 * (100000 * m_nPhotos + 1693);
  }//B- decays to D_s*- D+ pi-
  if (GenBplusTag::PcheckDecay(genpart, -433, 421, 111)) {
    return -1 * (100000 * m_nPhotos + 1694);
  }//B- decays to D_s*- D0 pi0
  if (GenBplusTag::PcheckDecay(genpart, -431, 411, -211, 111)) {
    return -1 * (100000 * m_nPhotos + 1695);
  }//B- decays to D_s- D+ pi- pi0
  if (GenBplusTag::PcheckDecay(genpart, -431, 421, -211, 211)) {
    return -1 * (100000 * m_nPhotos + 1696);
  }//B- decays to D_s- D0 pi- pi+
  if (GenBplusTag::PcheckDecay(genpart, -431, 421, 111, 111)) {
    return -1 * (100000 * m_nPhotos + 1697);
  }//B- decays to D_s- D0 pi0 pi0
  if (GenBplusTag::PcheckDecay(genpart, -433, 411, -211, 111)) {
    return -1 * (100000 * m_nPhotos + 1698);
  }//B- decays to D_s*- D+ pi- pi0
  if (GenBplusTag::PcheckDecay(genpart, -433, 421, -211, 211)) {
    return -1 * (100000 * m_nPhotos + 1699);
  }//B- decays to D_s*- D0 pi- pi+
  if (GenBplusTag::PcheckDecay(genpart, -433, 421, 111, 111)) {
    return -1 * (100000 * m_nPhotos + 1700);
  }//B- decays to D_s*- D0 pi0 pi0
  if (GenBplusTag::PcheckDecay(genpart, 421, -411, -311)) {
    return -1 * (100000 * m_nPhotos + 1701);
  }//B- decays to D0 D- anti-K0
  if (GenBplusTag::PcheckDecay(genpart, 421, -413, -311)) {
    return -1 * (100000 * m_nPhotos + 1702);
  }//B- decays to D0 D*- anti-K0
  if (GenBplusTag::PcheckDecay(genpart, 423, -411, -311)) {
    return -1 * (100000 * m_nPhotos + 1703);
  }//B- decays to D*0 D- anti-K0
  if (GenBplusTag::PcheckDecay(genpart, 423, -413, -311)) {
    return -1 * (100000 * m_nPhotos + 1704);
  }//B- decays to D*0 D*- anti-K0
  if (GenBplusTag::PcheckDecay(genpart, 421, -421, -321)) {
    return -1 * (100000 * m_nPhotos + 1705);
  }//B- decays to D0 anti-D0 K-
  if (GenBplusTag::PcheckDecay(genpart, 423, -421, -321)) {
    return -1 * (100000 * m_nPhotos + 1706);
  }//B- decays to D*0 anti-D0 K-
  if (GenBplusTag::PcheckDecay(genpart, 421, -423, -321)) {
    return -1 * (100000 * m_nPhotos + 1707);
  }//B- decays to D0 anti-D*0 K-
  if (GenBplusTag::PcheckDecay(genpart, 423, -423, -321)) {
    return -1 * (100000 * m_nPhotos + 1708);
  }//B- decays to D*0 anti-D*0 K-
  if (GenBplusTag::PcheckDecay(genpart, -411, 411, -321)) {
    return -1 * (100000 * m_nPhotos + 1709);
  }//B- decays to D- D+ K-
  if (GenBplusTag::PcheckDecay(genpart, -413, 411, -321)) {
    return -1 * (100000 * m_nPhotos + 1710);
  }//B- decays to D*- D+ K-
  if (GenBplusTag::PcheckDecay(genpart, -411, 413, -321)) {
    return -1 * (100000 * m_nPhotos + 1711);
  }//B- decays to D- D*+ K-
  if (GenBplusTag::PcheckDecay(genpart, -413, 413, -321)) {
    return -1 * (100000 * m_nPhotos + 1712);
  }//B- decays to D*- D*+ K-
  if (GenBplusTag::PcheckDecay(genpart, 421, -411, -313)) {
    return -1 * (100000 * m_nPhotos + 1713);
  }//B- decays to D0 D- anti-K*0
  if (GenBplusTag::PcheckDecay(genpart, 423, -411, -313)) {
    return -1 * (100000 * m_nPhotos + 1714);
  }//B- decays to D*0 D- anti-K*0
  if (GenBplusTag::PcheckDecay(genpart, 421, -413, -313)) {
    return -1 * (100000 * m_nPhotos + 1715);
  }//B- decays to D0 D*- anti-K*0
  if (GenBplusTag::PcheckDecay(genpart, 423, -413, -313)) {
    return -1 * (100000 * m_nPhotos + 1716);
  }//B- decays to D*0 D*- anti-K*0
  if (GenBplusTag::PcheckDecay(genpart, 421, -421, -323)) {
    return -1 * (100000 * m_nPhotos + 1717);
  }//B- decays to D0 anti-D0 K*-
  if (GenBplusTag::PcheckDecay(genpart, 423, -421, -323)) {
    return -1 * (100000 * m_nPhotos + 1718);
  }//B- decays to D*0 anti-D0 K*-
  if (GenBplusTag::PcheckDecay(genpart, 421, -423, -323)) {
    return -1 * (100000 * m_nPhotos + 1719);
  }//B- decays to D0 anti-D*0 K*-
  if (GenBplusTag::PcheckDecay(genpart, 423, -423, -323)) {
    return -1 * (100000 * m_nPhotos + 1720);
  }//B- decays to D*0 anti-D*0 K*-
  if (GenBplusTag::PcheckDecay(genpart, -411, 411, -323)) {
    return -1 * (100000 * m_nPhotos + 1721);
  }//B- decays to D- D+ K*-
  if (GenBplusTag::PcheckDecay(genpart, -413, 411, -323)) {
    return -1 * (100000 * m_nPhotos + 1722);
  }//B- decays to D*- D+ K*-
  if (GenBplusTag::PcheckDecay(genpart, -411, 413, -323)) {
    return -1 * (100000 * m_nPhotos + 1723);
  }//B- decays to D- D*+ K*-
  if (GenBplusTag::PcheckDecay(genpart, -413, 413, -323)) {
    return -1 * (100000 * m_nPhotos + 1724);
  }//B- decays to D*- D*+ K*-
  if (GenBplusTag::PcheckDecay(genpart, -411, 421)) {
    return -1 * (100000 * m_nPhotos + 1725);
  }//B- decays to D- D0
  if (GenBplusTag::PcheckDecay(genpart, -413, 421)) {
    return -1 * (100000 * m_nPhotos + 1726);
  }//B- decays to D*- D0
  if (GenBplusTag::PcheckDecay(genpart, 423, -411)) {
    return -1 * (100000 * m_nPhotos + 1727);
  }//B- decays to D*0 D-
  if (GenBplusTag::PcheckDecay(genpart, 423, -413)) {
    return -1 * (100000 * m_nPhotos + 1728);
  }//B- decays to D*0 D*-
  if (GenBplusTag::PcheckDecay(genpart, 423, -211)) {
    return -1 * (100000 * m_nPhotos + 1729);
  }//B- decays to D*0 pi-
  if (GenBplusTag::PcheckDecay(genpart, 421, -211)) {
    return -1 * (100000 * m_nPhotos + 1730);
  }//B- decays to D0 pi-
  if (GenBplusTag::PcheckDecay(genpart, -213, 421)) {
    return -1 * (100000 * m_nPhotos + 1731);
  }//B- decays to rho- D0
  if (GenBplusTag::PcheckDecay(genpart, 423, -213)) {
    return -1 * (100000 * m_nPhotos + 1732);
  }//B- decays to D*0 rho-
  if (GenBplusTag::PcheckDecay(genpart, 421, 111, -211)) {
    return -1 * (100000 * m_nPhotos + 1733);
  }//B- decays to D0 pi0 pi-
  if (GenBplusTag::PcheckDecay(genpart, 423, 111, -211)) {
    return -1 * (100000 * m_nPhotos + 1734);
  }//B- decays to D*0 pi0 pi-
  if (GenBplusTag::PcheckDecay(genpart, 411, -211, -211)) {
    return -1 * (100000 * m_nPhotos + 1735);
  }//B- decays to D+ pi- pi-
  if (GenBplusTag::PcheckDecay(genpart, 413, -211, -211)) {
    return -1 * (100000 * m_nPhotos + 1736);
  }//B- decays to D*+ pi- pi-
  if (GenBplusTag::PcheckDecay(genpart, -20213, 421)) {
    return -1 * (100000 * m_nPhotos + 1737);
  }//B- decays to a_1- D0
  if (GenBplusTag::PcheckDecay(genpart, 421, 113, -211)) {
    return -1 * (100000 * m_nPhotos + 1738);
  }//B- decays to D0 rho0 pi-
  if (GenBplusTag::PcheckDecay(genpart, 421, 211, -211, -211)) {
    return -1 * (100000 * m_nPhotos + 1739);
  }//B- decays to D0 pi+ pi- pi-
  if (GenBplusTag::PcheckDecay(genpart, 423, -20213)) {
    return -1 * (100000 * m_nPhotos + 1740);
  }//B- decays to D*0 a_1-
  if (GenBplusTag::PcheckDecay(genpart, 423, 113, -211)) {
    return -1 * (100000 * m_nPhotos + 1741);
  }//B- decays to D*0 rho0 pi-
  if (GenBplusTag::PcheckDecay(genpart, 423, 211, -211, -211)) {
    return -1 * (100000 * m_nPhotos + 1742);
  }//B- decays to D*0 pi+ pi- pi-
  if (GenBplusTag::PcheckDecay(genpart, 411, -213, -211)) {
    return -1 * (100000 * m_nPhotos + 1743);
  }//B- decays to D+ rho- pi-
  if (GenBplusTag::PcheckDecay(genpart, 411, 111, -211, -211)) {
    return -1 * (100000 * m_nPhotos + 1744);
  }//B- decays to D+ pi0 pi- pi-
  if (GenBplusTag::PcheckDecay(genpart, 413, -213, -211)) {
    return -1 * (100000 * m_nPhotos + 1745);
  }//B- decays to D*+ rho- pi-
  if (GenBplusTag::PcheckDecay(genpart, 413, 111, -211, -211)) {
    return -1 * (100000 * m_nPhotos + 1746);
  }//B- decays to D*+ pi0 pi- pi-
  if (GenBplusTag::PcheckDecay(genpart, 423, -213, 111)) {
    return -1 * (100000 * m_nPhotos + 1747);
  }//B- decays to D*0 rho- pi0
  if (GenBplusTag::PcheckDecay(genpart, 423, -211, 111, 111)) {
    return -1 * (100000 * m_nPhotos + 1748);
  }//B- decays to D*0 pi- pi0 pi0
  if (GenBplusTag::PcheckDecay(genpart, 10423, -211)) {
    return -1 * (100000 * m_nPhotos + 1749);
  }//B- decays to D_10 pi-
  if (GenBplusTag::PcheckDecay(genpart, 20423, -211)) {
    return -1 * (100000 * m_nPhotos + 1750);
  }//B- decays to D'_10 pi-
  if (GenBplusTag::PcheckDecay(genpart, 425, -211)) {
    return -1 * (100000 * m_nPhotos + 1751);
  }//B- decays to D_2*0 pi-
  if (GenBplusTag::PcheckDecay(genpart, 10423, -213)) {
    return -1 * (100000 * m_nPhotos + 1752);
  }//B- decays to D_10 rho-
  if (GenBplusTag::PcheckDecay(genpart, 20423, -213)) {
    return -1 * (100000 * m_nPhotos + 1753);
  }//B- decays to D'_10 rho-
  if (GenBplusTag::PcheckDecay(genpart, 425, -213)) {
    return -1 * (100000 * m_nPhotos + 1754);
  }//B- decays to D_2*0 rho-
  if (GenBplusTag::PcheckDecay(genpart, 10421, -211)) {
    return -1 * (100000 * m_nPhotos + 1755);
  }//B- decays to D_0*0 pi-
  if (GenBplusTag::PcheckDecay(genpart, 421, -321)) {
    return -1 * (100000 * m_nPhotos + 1756);
  }//B- decays to D0 K-
  if (GenBplusTag::PcheckDecay(genpart, 423, -321)) {
    return -1 * (100000 * m_nPhotos + 1757);
  }//B- decays to D*0 K-
  if (GenBplusTag::PcheckDecay(genpart, -323, 421)) {
    return -1 * (100000 * m_nPhotos + 1758);
  }//B- decays to K*- D0
  if (GenBplusTag::PcheckDecay(genpart, 423, -323)) {
    return -1 * (100000 * m_nPhotos + 1759);
  }//B- decays to D*0 K*-
  if (GenBplusTag::PcheckDecay(genpart, -411, 111)) {
    return -1 * (100000 * m_nPhotos + 1760);
  }//B- decays to D- pi0
  if (GenBplusTag::PcheckDecay(genpart, -413, 111)) {
    return -1 * (100000 * m_nPhotos + 1761);
  }//B- decays to D*- pi0
  if (GenBplusTag::PcheckDecay(genpart, -411, -311)) {
    return -1 * (100000 * m_nPhotos + 1762);
  }//B- decays to D- anti-K0
  if (GenBplusTag::PcheckDecay(genpart, -413, -311)) {
    return -1 * (100000 * m_nPhotos + 1763);
  }//B- decays to D*- anti-K0
  if (GenBplusTag::PcheckDecay(genpart, 421, -10431)) {
    return -1 * (100000 * m_nPhotos + 1764);
  }//B- decays to D0 D_s0*-
  if (GenBplusTag::PcheckDecay(genpart, 423, -10431)) {
    return -1 * (100000 * m_nPhotos + 1765);
  }//B- decays to D*0 D_s0*-
  if (GenBplusTag::PcheckDecay(genpart, -10433, 421)) {
    return -1 * (100000 * m_nPhotos + 1766);
  }//B- decays to D_s1- D0
  if (GenBplusTag::PcheckDecay(genpart, 423, -10433)) {
    return -1 * (100000 * m_nPhotos + 1767);
  }//B- decays to D*0 D_s1-
  if (GenBplusTag::PcheckDecay(genpart, 421, -321, -311)) {
    return -1 * (100000 * m_nPhotos + 1768);
  }//B- decays to D0 K- anti-K0
  if (GenBplusTag::PcheckDecay(genpart, 421, -321, -313)) {
    return -1 * (100000 * m_nPhotos + 1769);
  }//B- decays to D0 K- anti-K*0
  if (GenBplusTag::PcheckDecay(genpart, 423, -321, -313)) {
    return -1 * (100000 * m_nPhotos + 1770);
  }//B- decays to D*0 K- anti-K*0
  if (GenBplusTag::PcheckDecay(genpart, 421, 223, -211)) {
    return -1 * (100000 * m_nPhotos + 1771);
  }//B- decays to D0 omega pi-
  if (GenBplusTag::PcheckDecay(genpart, 423, 223, -211)) {
    return -1 * (100000 * m_nPhotos + 1772);
  }//B- decays to D*0 omega pi-
  if (GenBplusTag::PcheckDecay(genpart, 421, -20433)) {
    return -1 * (100000 * m_nPhotos + 1773);
  }//B- decays to D0 D'_s1-
  if (GenBplusTag::PcheckDecay(genpart, 423, -20433)) {
    return -1 * (100000 * m_nPhotos + 1774);
  }//B- decays to D*0 D'_s1-
  if (GenBplusTag::PcheckDecay(genpart, 4103, -2203)) {
    return -1 * (100000 * m_nPhotos + 1775);
  }//B- decays to cd_1 anti-uu_1
  if (GenBplusTag::PcheckDecay(genpart, 4303, -2203)) {
    return -1 * (100000 * m_nPhotos + 1776);
  }//B- decays to cs_1 anti-uu_1
  if (GenBplusTag::PcheckDecay(genpart, -2, 1, 4, -2)) {
    return -1 * (100000 * m_nPhotos + 1777);
  }//B- decays to anti-u d c anti-u
  if (GenBplusTag::PcheckDecay(genpart, -2, 1, 4, -2)) {
    return -1 * (100000 * m_nPhotos + 1778);
  }//B- decays to anti-u d c anti-u
  if (GenBplusTag::PcheckDecay(genpart, -2, 3, 4, -2)) {
    return -1 * (100000 * m_nPhotos + 1779);
  }//B- decays to anti-u s c anti-u
  if (GenBplusTag::PcheckDecay(genpart, -4, 3, 4, -2)) {
    return -1 * (100000 * m_nPhotos + 1780);
  }//B- decays to anti-c s c anti-u
  if (GenBplusTag::PcheckDecay(genpart, -4, 1, 4, -2)) {
    return -1 * (100000 * m_nPhotos + 1781);
  }//B- decays to anti-c d c anti-u
  if (GenBplusTag::PcheckDecay(genpart, -2, 1, 2, -2)) {
    return -1 * (100000 * m_nPhotos + 1782);
  }//B- decays to anti-u d u anti-u
  if (GenBplusTag::PcheckDecay(genpart, -4, 3, 2, -2)) {
    return -1 * (100000 * m_nPhotos + 1783);
  }//B- decays to anti-c s u anti-u
  if (GenBplusTag::PcheckDecay(genpart, -2, 2, 1, -2)) {
    return -1 * (100000 * m_nPhotos + 1784);
  }//B- decays to anti-u u d anti-u
  if (GenBplusTag::PcheckDecay(genpart, -1, 1, 1, -2)) {
    return -1 * (100000 * m_nPhotos + 1785);
  }//B- decays to anti-d d d anti-u
  if (GenBplusTag::PcheckDecay(genpart, -3, 3, 1, -2)) {
    return -1 * (100000 * m_nPhotos + 1786);
  }//B- decays to anti-s s d anti-u
  if (GenBplusTag::PcheckDecay(genpart, -2, 2, 3, -2)) {
    return -1 * (100000 * m_nPhotos + 1787);
  }//B- decays to anti-u u s anti-u
  if (GenBplusTag::PcheckDecay(genpart, -1, 1, 3, -2)) {
    return -1 * (100000 * m_nPhotos + 1788);
  }//B- decays to anti-d d s anti-u
  if (GenBplusTag::PcheckDecay(genpart, -3, 3, 3, -2)) {
    return -1 * (100000 * m_nPhotos + 1789);
  }//B- decays to anti-s s s anti-u
  if (GenBplusTag::PcheckDecay(genpart, 3, -2)) {
    return -1 * (100000 * m_nPhotos + 1790);
  }//B- decays to s anti-u
  if (GenBplusTag::PcheckDecay(genpart, 421, -321, 311)) {
    return -1 * (100000 * m_nPhotos + 1791);
  }//B- decays to D0 K- K0
  if (GenBplusTag::PcheckDecay(genpart, 421, -321, 313)) {
    return -1 * (100000 * m_nPhotos + 1792);
  }//B- decays to D0 K- K*0
  if (GenBplusTag::PcheckDecay(genpart, 423, 211, -211, -211, 111)) {
    return -1 * (100000 * m_nPhotos + 1793);
  }//B- decays to D*0 pi+ pi- pi- pi0
  if (GenBplusTag::PcheckDecay(genpart, 423, -211, -211, -211, 211, 211)) {
    return -1 * (100000 * m_nPhotos + 1794);
  }//B- decays to D*0 pi- pi- pi- pi+ pi+
  if (GenBplusTag::PcheckDecay(genpart, 413, -211, -211, -211, 211)) {
    return -1 * (100000 * m_nPhotos + 1795);
  }//B- decays to D*+ pi- pi- pi- pi+
  if (GenBplusTag::PcheckDecay(genpart, 431, -211, -321)) {
    return -1 * (100000 * m_nPhotos + 1796);
  }//B- decays to D_s+ pi- K-
  if (GenBplusTag::PcheckDecay(genpart, 433, -211, -321)) {
    return -1 * (100000 * m_nPhotos + 1797);
  }//B- decays to D_s*+ pi- K-
  if (GenBplusTag::PcheckDecay(genpart, 431, -321, -321)) {
    return -1 * (100000 * m_nPhotos + 1798);
  }//B- decays to D_s+ K- K-
  if (GenBplusTag::PcheckDecay(genpart, 221, -10321)) {
    return -1 * (100000 * m_nPhotos + 1799);
  }//B- decays to eta K_0*-
  if (GenBplusTag::PcheckDecay(genpart, 221, -325)) {
    return -1 * (100000 * m_nPhotos + 1800);
  }//B- decays to eta K_2*-
  if (GenBplusTag::PcheckDecay(genpart, 223, -10321)) {
    return -1 * (100000 * m_nPhotos + 1801);
  }//B- decays to omega K_0*-
  if (GenBplusTag::PcheckDecay(genpart, 223, -325)) {
    return -1 * (100000 * m_nPhotos + 1802);
  }//B- decays to omega K_2*-
  if (GenBplusTag::PcheckDecay(genpart, -313, -211)) {
    return -1 * (100000 * m_nPhotos + 1803);
  }//B- decays to anti-K*0 pi-
  if (GenBplusTag::PcheckDecay(genpart, 225, -321)) {
    return -1 * (100000 * m_nPhotos + 1804);
  }//B- decays to f_2 K-
  if (GenBplusTag::PcheckDecay(genpart, -315, -211)) {
    return -1 * (100000 * m_nPhotos + 1805);
  }//B- decays to anti-K_2*0 pi-
  if (GenBplusTag::PcheckDecay(genpart, -323, -313)) {
    return -1 * (100000 * m_nPhotos + 1806);
  }//B- decays to K*- anti-K*0
  if (GenBplusTag::PcheckDecay(genpart, 333, -10323)) {
    return -1 * (100000 * m_nPhotos + 1807);
  }//B- decays to phi K_1-
  if (GenBplusTag::PcheckDecay(genpart, 333, -325)) {
    return -1 * (100000 * m_nPhotos + 1808);
  }//B- decays to phi K_2*-
  if (GenBplusTag::PcheckDecay(genpart, 221, -321, 22)) {
    return -1 * (100000 * m_nPhotos + 1809);
  }//B- decays to eta K- gamma
  if (GenBplusTag::PcheckDecay(genpart, 331, -321, 22)) {
    return -1 * (100000 * m_nPhotos + 1810);
  }//B- decays to eta' K- gamma
  if (GenBplusTag::PcheckDecay(genpart, 333, -321, 22)) {
    return -1 * (100000 * m_nPhotos + 1811);
  }//B- decays to phi K- gamma
  if (GenBplusTag::PcheckDecay(genpart, -2212, 2212, -211)) {
    return -1 * (100000 * m_nPhotos + 1812);
  }//B- decays to anti-p- p+ pi-
  if (GenBplusTag::PcheckDecay(genpart, -2212, 2212, -321)) {
    return -1 * (100000 * m_nPhotos + 1813);
  }//B- decays to anti-p- p+ K-
  if (GenBplusTag::PcheckDecay(genpart, -2212, 2212, -323)) {
    return -1 * (100000 * m_nPhotos + 1814);
  }//B- decays to anti-p- p+ K*-
  if (GenBplusTag::PcheckDecay(genpart, -2212, 3122, 22)) {
    return -1 * (100000 * m_nPhotos + 1815);
  }//B- decays to anti-p- Lambda0 gamma
  if (GenBplusTag::PcheckDecay(genpart, -2212, 3122, 111)) {
    return -1 * (100000 * m_nPhotos + 1816);
  }//B- decays to anti-p- Lambda0 pi0
  if (GenBplusTag::PcheckDecay(genpart, -2212, 3122, -211, 211)) {
    return -1 * (100000 * m_nPhotos + 1817);
  }//B- decays to anti-p- Lambda0 pi- pi+
  if (GenBplusTag::PcheckDecay(genpart, -2212, 3122, 113)) {
    return -1 * (100000 * m_nPhotos + 1818);
  }//B- decays to anti-p- Lambda0 rho0
  if (GenBplusTag::PcheckDecay(genpart, -2212, 3122, 225)) {
    return -1 * (100000 * m_nPhotos + 1819);
  }//B- decays to anti-p- Lambda0 f_2
  if (GenBplusTag::PcheckDecay(genpart, -3122, 3122, -321)) {
    return -1 * (100000 * m_nPhotos + 1820);
  }//B- decays to anti-Lambda0 Lambda0 K-
  if (GenBplusTag::PcheckDecay(genpart, -3122, 3122, -323)) {
    return -1 * (100000 * m_nPhotos + 1821);
  }//B- decays to anti-Lambda0 Lambda0 K*-
  if (GenBplusTag::PcheckDecay(genpart, 4122, -2212, -211)) {
    return -1 * (100000 * m_nPhotos + 1822);
  }//B- decays to Lambda_c+ anti-p- pi-
  if (GenBplusTag::PcheckDecay(genpart, 4122, -2212, -211, 111)) {
    return -1 * (100000 * m_nPhotos + 1823);
  }//B- decays to Lambda_c+ anti-p- pi- pi0
  if (GenBplusTag::PcheckDecay(genpart, 4122, -2212, -211, -211, 211)) {
    return -1 * (100000 * m_nPhotos + 1824);
  }//B- decays to Lambda_c+ anti-p- pi- pi- pi+
  if (GenBplusTag::PcheckDecay(genpart, 4112, -2212)) {
    return -1 * (100000 * m_nPhotos + 1825);
  }//B- decays to Sigma_c0 anti-p-
  if (GenBplusTag::PcheckDecay(genpart, 4112, -2212, 111)) {
    return -1 * (100000 * m_nPhotos + 1826);
  }//B- decays to Sigma_c0 anti-p- pi0
  if (GenBplusTag::PcheckDecay(genpart, 4112, -2212, 211, -211)) {
    return -1 * (100000 * m_nPhotos + 1827);
  }//B- decays to Sigma_c0 anti-p- pi+ pi-
  if (GenBplusTag::PcheckDecay(genpart, 4222, -2212, -211, -211)) {
    return -1 * (100000 * m_nPhotos + 1828);
  }//B- decays to Sigma_c++ anti-p- pi- pi-
  return -1 * (int)genpart.size();
}// Rest of the B- decays
