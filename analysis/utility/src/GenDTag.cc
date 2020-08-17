/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Sourav Patra, Vishal Bhardwaj                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <analysis/utility/GenDTag.h>
using namespace Belle2;

int GenDTag::Array_CheckDecay(std::vector<int> MCDAU,
                              std::vector<int> genDAU)
{
  int iam_left = genDAU.size();
  //int mcDAUsize = MCDAU.size();

  if (MCDAU.size() == genDAU.size()) {
    for (int i = 0; i < int(MCDAU.size()); ++i) {
      for (int j = 0; j < int(genDAU.size()); ++j) {
        if (MCDAU[i] == genDAU[j]) {
          genDAU.erase(genDAU.begin() + j);
          iam_left--;
          // std::cout << left<< std::endl;
        }
      }
    }
    if (iam_left == 0) {return 1;}
    else { return -10;}
  } else {return -20;}
}


int GenDTag::CheckDecay(std::vector<int>MCDAU, int a0 = -987656789,
                        int a1 = -987656789,  int a2 = -987656789,
                        int a3 = -987656789,  int a4 = -987656789,
                        int a5 = -987656789,  int a6 = -987656789,
                        int a7 = -987656789,  int a8 = -987656789,
                        int a9 = -987656789,  int a10 = -987656789,
                        int a11 = -987656789,  int a12 = -987656789,
                        int a13 = -987656789,  int a14 = -987656789,
                        int a15 = -987656789,  int a16 = -987656789,
                        int a17 = -987656789,  int a18 = -987656789)
{

  std::vector<int>genDAU;
  if (a0 != -987656789)genDAU.push_back(a0);
  if (a1 != -987656789)genDAU.push_back(a1);
  if (a2 != -987656789)genDAU.push_back(a2);
  if (a3 != -987656789)genDAU.push_back(a3);
  if (a4 != -987656789)genDAU.push_back(a4);
  if (a5 != -987656789)genDAU.push_back(a5);
  if (a6 != -987656789)genDAU.push_back(a6);
  if (a7 != -987656789)genDAU.push_back(a7);
  if (a8 != -987656789)genDAU.push_back(a8);
  if (a9 != -987656789)genDAU.push_back(a9);
  if (a10 != -987656789)genDAU.push_back(a10);
  if (a11 != -987656789)genDAU.push_back(a11);
  if (a12 != -987656789)genDAU.push_back(a12);
  if (a13 != -987656789)genDAU.push_back(a13);
  if (a14 != -987656789)genDAU.push_back(a14);
  if (a15 != -987656789)genDAU.push_back(a15);
  if (a16 != -987656789)genDAU.push_back(a16);
  if (a17 != -987656789)genDAU.push_back(a17);
  if (a18 != -987656789)genDAU.push_back(a18);
  return  GenDTag::Array_CheckDecay(MCDAU, genDAU);
}


int GenDTag::DauSize(std::vector<int>genpart)
{
  return genpart.size();
}


int GenDTag::PcheckDecay(std::vector<int>gp, int da1, int da2)
{
  int ndau = DauSize(gp);
  if (ndau == 2) { nPhotos = 0; return CheckDecay(gp, da1, da2);}
  else if (ndau == 3) { nPhotos = 1; return CheckDecay(gp, da1, da2, 22);}
  else if (ndau == 4) { nPhotos = 2; return CheckDecay(gp, da1, da2, 22, 22);}
  else if (ndau == 5) { nPhotos = 3; return CheckDecay(gp, da1, da2, 22, 22, 22);}
  else if (ndau == 6) { nPhotos = 4; return CheckDecay(gp, da1, da2, 22, 22, 22, 22);}
  else return -999;
}


int GenDTag::PcheckDecay(std::vector<int>gp, int da1, int da2, int da3)
{
  int ndau = DauSize(gp);
  if (ndau == 3) { nPhotos = 0; return CheckDecay(gp, da1, da2, da3);}
  else if (ndau == 4) { nPhotos = 1; return CheckDecay(gp, da1, da2, da3, 22);}
  else if (ndau == 5) { nPhotos = 2; return CheckDecay(gp, da1, da2, da3, 22, 22);}
  else if (ndau == 6) { nPhotos = 3; return CheckDecay(gp, da1, da2, da3, 22, 22, 22);}
  else if (ndau == 7) { nPhotos = 4; return CheckDecay(gp, da1, da2, da3, 22, 22, 22, 22);}
  else return -999;
}


int GenDTag::PcheckDecay(std::vector<int>gp, int da1, int da2, int da3, int da4)
{
  int ndau = DauSize(gp);
  if (ndau == 4) { nPhotos = 0; return CheckDecay(gp, da1, da2, da3, da4);}
  else if (ndau == 5) { nPhotos = 1; return CheckDecay(gp, da1, da2, da3, da4, 22);}
  else if (ndau == 6) { nPhotos = 2; return CheckDecay(gp, da1, da2, da3, da4, 22, 22);}
  else if (ndau == 7) { nPhotos = 3; return CheckDecay(gp, da1, da2, da3, da4, 22, 22, 22);}
  else if (ndau == 8) { nPhotos = 4; return CheckDecay(gp, da1, da2, da3, da4, 22, 22, 22, 22);}
  else return -999;
}


int GenDTag::PcheckDecay(std::vector<int>gp, int da1, int da2, int da3, int da4, int da5)
{
  int ndau = DauSize(gp);
  if (ndau == 5) { nPhotos = 0; return CheckDecay(gp, da1, da2, da3, da4, da5);}
  else if (ndau == 6) { nPhotos = 1; return CheckDecay(gp, da1, da2, da3, da4, da5, 22);}
  else if (ndau == 7) { nPhotos = 2; return CheckDecay(gp, da1, da2, da3, da4, da5, 22, 22);}
  else if (ndau == 8) { nPhotos = 3; return CheckDecay(gp, da1, da2, da3, da4, da5, 22, 22, 22);}
  else if (ndau == 9) { nPhotos = 4; return CheckDecay(gp, da1, da2, da3, da4, da5, 22, 22, 22, 22);}
  else return -999;
}


int GenDTag::PcheckDecay(std::vector<int>gp, int da1, int da2, int da3, int da4, int da5,
                         int da6)
{
  int ndau = DauSize(gp);
  if (ndau == 6) { nPhotos = 0;  return CheckDecay(gp, da1, da2, da3, da4, da5, da6);}
  else if (ndau == 7) { nPhotos = 1; return CheckDecay(gp, da1, da2, da3, da4, da5, da6, 22);}
  else if (ndau == 8) { nPhotos = 2; return CheckDecay(gp, da1, da2, da3, da4, da5, da6, 22, 22);}
  else if (ndau == 9) { nPhotos = 3; return CheckDecay(gp, da1, da2, da3, da4, da5, da6, 22, 22, 22);}
  else if (ndau == 10) { nPhotos = 4; return CheckDecay(gp, da1, da2, da3, da4, da5, da6, 22, 22, 22, 22);}
  else return -999;
}


int GenDTag::PcheckDecay(std::vector<int>gp, int da1, int da2, int da3, int da4, int da5,
                         int da6, int da7)
{
  int ndau = DauSize(gp);
  if (ndau == 7) { nPhotos = 0;  return CheckDecay(gp, da1, da2, da3, da4, da5, da6, da7);}
  else if (ndau == 8) { nPhotos = 1; return CheckDecay(gp, da1, da2, da3, da4, da5, da6, da7, 22);}
  else if (ndau == 9) { nPhotos = 2; return CheckDecay(gp, da1, da2, da3, da4, da5, da6, da7, 22, 22);}
  else if (ndau == 10) { nPhotos = 3; return CheckDecay(gp, da1, da2, da3, da4, da5, da6, da7, 22, 22, 22);}
  else if (ndau == 11) { nPhotos = 4; return CheckDecay(gp, da1, da2, da3, da4, da5, da6, da7, 22, 22, 22, 22);}
  else return -999;
}


int GenDTag::PcheckDecay(std::vector<int>gp, int da1, int da2, int da3, int da4, int da5,
                         int da6, int da7, int da8)
{
  int ndau = DauSize(gp);
  if (ndau == 8) { nPhotos = 0; return CheckDecay(gp, da1, da2, da3, da4, da5, da6, da7, da8);}
  else if (ndau == 9) { nPhotos = 1; return CheckDecay(gp, da1, da2, da3, da4, da5, da6, da7, da8, 22);}
  else if (ndau == 10) { nPhotos = 2; return CheckDecay(gp, da1, da2, da3, da4, da5, da6, da7, da8, 22, 22);}
  else if (ndau == 11) { nPhotos = 3; return CheckDecay(gp, da1, da2, da3, da4, da5, da6, da7, da8, 22, 22, 22);}
  else if (ndau == 12) { nPhotos = 4; return CheckDecay(gp, da1, da2, da3, da4, da5, da6, da7, da8, 22, 22, 22, 22);}
  else return -999;
}

//All the tag number for various decay mode is defined here


int GenDTag::Mode_Dst_plus(std::vector<int>genpart)
{
  if (GenDTag::PcheckDecay(genpart, 421, 211) == 1) {
    return +1 * (100000 * nPhotos + 1001);
  }//D*+ decays to D0 pi+
  if (GenDTag::PcheckDecay(genpart, 411, 111) == 1) {
    return +1 * (100000 * nPhotos + 1002);
  }//D*+ decays to D+ pi0
  if (GenDTag::PcheckDecay(genpart, 411, 22) == 1) {
    return +1 * (100000 * nPhotos + 1003);
  }//D*+ decays to D+ gamma
  return +1 * (DauSize(genpart));
}// Rest of the D*+ decays


int GenDTag::Mode_Dst_minus(std::vector<int>genpart)
{
  if (GenDTag::PcheckDecay(genpart, -421, -211) == 1) {
    return -1 * (100000 * nPhotos + 1001);
  }//D*- decays to anti-D0 pi-
  if (GenDTag::PcheckDecay(genpart, -411, 111) == 1) {
    return -1 * (100000 * nPhotos + 1002);
  }//D*- decays to D- pi0
  if (GenDTag::PcheckDecay(genpart, -411, 22) == 1) {
    return -1 * (100000 * nPhotos + 1003);
  }//D*- decays to D- gamma
  return -1 * (DauSize(genpart));
}// Rest of the D*- decays


int GenDTag::Mode_Ds_plus(std::vector<int>genpart)
{
  if (GenDTag::PcheckDecay(genpart, 333, -11, 12) == 1) {
    return +1 * (100000 * nPhotos + 1001);
  }//D_s+ decays to phi e+ nu_e
  if (GenDTag::PcheckDecay(genpart, 221, -11, 12) == 1) {
    return +1 * (100000 * nPhotos + 1002);
  }//D_s+ decays to eta e+ nu_e
  if (GenDTag::PcheckDecay(genpart, 331, -11, 12) == 1) {
    return +1 * (100000 * nPhotos + 1003);
  }//D_s+ decays to eta' e+ nu_e
  if (GenDTag::PcheckDecay(genpart, -311, -11, 12) == 1) {
    return +1 * (100000 * nPhotos + 1004);
  }//D_s+ decays to anti-K0 e+ nu_e
  if (GenDTag::PcheckDecay(genpart, -313, -11, 12) == 1) {
    return +1 * (100000 * nPhotos + 1005);
  }//D_s+ decays to anti-K*0 e+ nu_e
  if (GenDTag::PcheckDecay(genpart, 333, -13, 14) == 1) {
    return +1 * (100000 * nPhotos + 1006);
  }//D_s+ decays to phi mu+ nu_mu
  if (GenDTag::PcheckDecay(genpart, 221, -13, 14) == 1) {
    return +1 * (100000 * nPhotos + 1007);
  }//D_s+ decays to eta mu+ nu_mu
  if (GenDTag::PcheckDecay(genpart, 331, -13, 14) == 1) {
    return +1 * (100000 * nPhotos + 1008);
  }//D_s+ decays to eta' mu+ nu_mu
  if (GenDTag::PcheckDecay(genpart, -311, -13, 14) == 1) {
    return +1 * (100000 * nPhotos + 1009);
  }//D_s+ decays to anti-K0 mu+ nu_mu
  if (GenDTag::PcheckDecay(genpart, -313, -13, 14) == 1) {
    return +1 * (100000 * nPhotos + 1010);
  }//D_s+ decays to anti-K*0 mu+ nu_mu
  if (GenDTag::PcheckDecay(genpart, -13, 14) == 1) {
    return +1 * (100000 * nPhotos + 1011);
  }//D_s+ decays to mu+ nu_mu
  if (GenDTag::PcheckDecay(genpart, -15, 16) == 1) {
    return +1 * (100000 * nPhotos + 1012);
  }//D_s+ decays to tau+ nu_tau
  if (GenDTag::PcheckDecay(genpart, 333, 211) == 1) {
    return +1 * (100000 * nPhotos + 1013);
  }//D_s+ decays to phi pi+
  if (GenDTag::PcheckDecay(genpart, 221, 211) == 1) {
    return +1 * (100000 * nPhotos + 1014);
  }//D_s+ decays to eta pi+
  if (GenDTag::PcheckDecay(genpart, 331, 211) == 1) {
    return +1 * (100000 * nPhotos + 1015);
  }//D_s+ decays to eta' pi+
  if (GenDTag::PcheckDecay(genpart, 223, 211) == 1) {
    return +1 * (100000 * nPhotos + 1016);
  }//D_s+ decays to omega pi+
  if (GenDTag::PcheckDecay(genpart, 113, 211) == 1) {
    return +1 * (100000 * nPhotos + 1017);
  }//D_s+ decays to rho0 pi+
  if (GenDTag::PcheckDecay(genpart, 213, 111) == 1) {
    return +1 * (100000 * nPhotos + 1018);
  }//D_s+ decays to rho+ pi0
  if (GenDTag::PcheckDecay(genpart, 211, 111) == 1) {
    return +1 * (100000 * nPhotos + 1019);
  }//D_s+ decays to pi+ pi0
  if (GenDTag::PcheckDecay(genpart, 10221, 211) == 1) {
    return +1 * (100000 * nPhotos + 1020);
  }//D_s+ decays to f_0 pi+
  if (GenDTag::PcheckDecay(genpart, 225, 211) == 1) {
    return +1 * (100000 * nPhotos + 1021);
  }//D_s+ decays to f_2 pi+
  if (GenDTag::PcheckDecay(genpart, 333, 213) == 1) {
    return +1 * (100000 * nPhotos + 1022);
  }//D_s+ decays to phi rho+
  if (GenDTag::PcheckDecay(genpart, 213, 221) == 1) {
    return +1 * (100000 * nPhotos + 1023);
  }//D_s+ decays to rho+ eta
  if (GenDTag::PcheckDecay(genpart, 213, 331) == 1) {
    return +1 * (100000 * nPhotos + 1024);
  }//D_s+ decays to rho+ eta'
  if (GenDTag::PcheckDecay(genpart, 211, -211, 211) == 1) {
    return +1 * (100000 * nPhotos + 1025);
  }//D_s+ decays to pi+ pi- pi+
  if (GenDTag::PcheckDecay(genpart, 211, 111, 111) == 1) {
    return +1 * (100000 * nPhotos + 1026);
  }//D_s+ decays to pi+ pi0 pi0
  if (GenDTag::PcheckDecay(genpart, 333, 211, 111) == 1) {
    return +1 * (100000 * nPhotos + 1027);
  }//D_s+ decays to phi pi+ pi0
  if (GenDTag::PcheckDecay(genpart, 221, 211, 111) == 1) {
    return +1 * (100000 * nPhotos + 1028);
  }//D_s+ decays to eta pi+ pi0
  if (GenDTag::PcheckDecay(genpart, 331, 211, 111) == 1) {
    return +1 * (100000 * nPhotos + 1029);
  }//D_s+ decays to eta' pi+ pi0
  if (GenDTag::PcheckDecay(genpart, 333, 211, -211, 211) == 1) {
    return +1 * (100000 * nPhotos + 1030);
  }//D_s+ decays to phi pi+ pi- pi+
  if (GenDTag::PcheckDecay(genpart, 333, 211, 111, 111) == 1) {
    return +1 * (100000 * nPhotos + 1031);
  }//D_s+ decays to phi pi+ pi0 pi0
  if (GenDTag::PcheckDecay(genpart, 221, 211, -211, 211) == 1) {
    return +1 * (100000 * nPhotos + 1032);
  }//D_s+ decays to eta pi+ pi- pi+
  if (GenDTag::PcheckDecay(genpart, 221, 211, 111, 111) == 1) {
    return +1 * (100000 * nPhotos + 1033);
  }//D_s+ decays to eta pi+ pi0 pi0
  if (GenDTag::PcheckDecay(genpart, 310, 321) == 1) {
    return +1 * (100000 * nPhotos + 1034);
  }//D_s+ decays to K_S0 K+
  if (GenDTag::PcheckDecay(genpart, 130, 321) == 1) {
    return +1 * (100000 * nPhotos + 1035);
  }//D_s+ decays to K_L0 K+
  if (GenDTag::PcheckDecay(genpart, -311, 321) == 1) {
    return +1 * (100000 * nPhotos + 1036);
  }//D_s+ decays to anti-K0 K+
  if (GenDTag::PcheckDecay(genpart, -313, 321) == 1) {
    return +1 * (100000 * nPhotos + 1037);
  }//D_s+ decays to anti-K*0 K+
  if (GenDTag::PcheckDecay(genpart, 323, -311) == 1) {
    return +1 * (100000 * nPhotos + 1038);
  }//D_s+ decays to K*+ anti-K0
  if (GenDTag::PcheckDecay(genpart, -313, 323) == 1) {
    return +1 * (100000 * nPhotos + 1039);
  }//D_s+ decays to anti-K*0 K*+
  if (GenDTag::PcheckDecay(genpart, -311, 321, 111) == 1) {
    return +1 * (100000 * nPhotos + 1040);
  }//D_s+ decays to anti-K0 K+ pi0
  if (GenDTag::PcheckDecay(genpart, -313, 321, 111) == 1) {
    return +1 * (100000 * nPhotos + 1041);
  }//D_s+ decays to anti-K*0 K+ pi0
  if (GenDTag::PcheckDecay(genpart, 323, -311, 111) == 1) {
    return +1 * (100000 * nPhotos + 1042);
  }//D_s+ decays to K*+ anti-K0 pi0
  if (GenDTag::PcheckDecay(genpart, -313, 323, 111) == 1) {
    return +1 * (100000 * nPhotos + 1043);
  }//D_s+ decays to anti-K*0 K*+ pi0
  if (GenDTag::PcheckDecay(genpart, 321, -321, 211) == 1) {
    return +1 * (100000 * nPhotos + 1044);
  }//D_s+ decays to K+ K- pi+
  if (GenDTag::PcheckDecay(genpart, 310, 321, 211, -211) == 1) {
    return +1 * (100000 * nPhotos + 1045);
  }//D_s+ decays to K_S0 K+ pi+ pi-
  if (GenDTag::PcheckDecay(genpart, 130, 321, 211, -211) == 1) {
    return +1 * (100000 * nPhotos + 1046);
  }//D_s+ decays to K_L0 K+ pi+ pi-
  if (GenDTag::PcheckDecay(genpart, -311, 321, 211, -211) == 1) {
    return +1 * (100000 * nPhotos + 1047);
  }//D_s+ decays to anti-K0 K+ pi+ pi-
  if (GenDTag::PcheckDecay(genpart, -311, 321, 111, 111) == 1) {
    return +1 * (100000 * nPhotos + 1048);
  }//D_s+ decays to anti-K0 K+ pi0 pi0
  if (GenDTag::PcheckDecay(genpart, 321, -321, 211, -211, 211) == 1) {
    return +1 * (100000 * nPhotos + 1049);
  }//D_s+ decays to K+ K- pi+ pi- pi+
  if (GenDTag::PcheckDecay(genpart, 333, 321) == 1) {
    return +1 * (100000 * nPhotos + 1050);
  }//D_s+ decays to phi K+
  if (GenDTag::PcheckDecay(genpart, 221, 321) == 1) {
    return +1 * (100000 * nPhotos + 1051);
  }//D_s+ decays to eta K+
  if (GenDTag::PcheckDecay(genpart, 331, 321) == 1) {
    return +1 * (100000 * nPhotos + 1052);
  }//D_s+ decays to eta' K+
  if (GenDTag::PcheckDecay(genpart, 221, 321, 111) == 1) {
    return +1 * (100000 * nPhotos + 1053);
  }//D_s+ decays to eta K+ pi0
  if (GenDTag::PcheckDecay(genpart, 221, 321, 211, -211) == 1) {
    return +1 * (100000 * nPhotos + 1054);
  }//D_s+ decays to eta K+ pi+ pi-
  if (GenDTag::PcheckDecay(genpart, 331, 321, 111) == 1) {
    return +1 * (100000 * nPhotos + 1055);
  }//D_s+ decays to eta' K+ pi0
  if (GenDTag::PcheckDecay(genpart, 331, 321, 211, -211) == 1) {
    return +1 * (100000 * nPhotos + 1056);
  }//D_s+ decays to eta' K+ pi+ pi-
  if (GenDTag::PcheckDecay(genpart, 321, -321, 321) == 1) {
    return +1 * (100000 * nPhotos + 1057);
  }//D_s+ decays to K+ K- K+
  if (GenDTag::PcheckDecay(genpart, 310, 211) == 1) {
    return +1 * (100000 * nPhotos + 1058);
  }//D_s+ decays to K_S0 pi+
  if (GenDTag::PcheckDecay(genpart, 130, 211) == 1) {
    return +1 * (100000 * nPhotos + 1059);
  }//D_s+ decays to K_L0 pi+
  if (GenDTag::PcheckDecay(genpart, 311, 211) == 1) {
    return +1 * (100000 * nPhotos + 1060);
  }//D_s+ decays to K0 pi+
  if (GenDTag::PcheckDecay(genpart, 213, 311) == 1) {
    return +1 * (100000 * nPhotos + 1061);
  }//D_s+ decays to rho+ K0
  if (GenDTag::PcheckDecay(genpart, 113, 321) == 1) {
    return +1 * (100000 * nPhotos + 1062);
  }//D_s+ decays to rho0 K+
  if (GenDTag::PcheckDecay(genpart, 311, 211, 111) == 1) {
    return +1 * (100000 * nPhotos + 1063);
  }//D_s+ decays to K0 pi+ pi0
  if (GenDTag::PcheckDecay(genpart, 20213, 311) == 1) {
    return +1 * (100000 * nPhotos + 1064);
  }//D_s+ decays to a_1+ K0
  if (GenDTag::PcheckDecay(genpart, 313, 211) == 1) {
    return +1 * (100000 * nPhotos + 1065);
  }//D_s+ decays to K*0 pi+
  if (GenDTag::PcheckDecay(genpart, 313, 213) == 1) {
    return +1 * (100000 * nPhotos + 1066);
  }//D_s+ decays to K*0 rho+
  if (GenDTag::PcheckDecay(genpart, 313, 211, 111) == 1) {
    return +1 * (100000 * nPhotos + 1067);
  }//D_s+ decays to K*0 pi+ pi0
  if (GenDTag::PcheckDecay(genpart, 321, 111) == 1) {
    return +1 * (100000 * nPhotos + 1068);
  }//D_s+ decays to K+ pi0
  if (GenDTag::PcheckDecay(genpart, 321, 211, -211) == 1) {
    return +1 * (100000 * nPhotos + 1069);
  }//D_s+ decays to K+ pi+ pi-
  if (GenDTag::PcheckDecay(genpart, 211, 211, 211, -211, -211) == 1) {
    return +1 * (100000 * nPhotos + 1070);
  }//D_s+ decays to pi+ pi+ pi+ pi- pi-
  if (GenDTag::PcheckDecay(genpart, 211, 211, 211, -211, -211, 111) == 1) {
    return +1 * (100000 * nPhotos + 1071);
  }//D_s+ decays to pi+ pi+ pi+ pi- pi- pi0
  if (GenDTag::PcheckDecay(genpart, 2212, -2112) == 1) {
    return +1 * (100000 * nPhotos + 1072);
  }//D_s+ decays to p+ anti-n0
  if (GenDTag::PcheckDecay(genpart, 310, 310, 211, 211, -211) == 1) {
    return +1 * (100000 * nPhotos + 1073);
  }//D_s+ decays to K_S0 K_S0 pi+ pi+ pi-
  if (GenDTag::PcheckDecay(genpart, 130, 130, 211, 211, -211) == 1) {
    return +1 * (100000 * nPhotos + 1074);
  }//D_s+ decays to K_L0 K_L0 pi+ pi+ pi-
  if (GenDTag::PcheckDecay(genpart, -311, 311, 211, 211, -211) == 1) {
    return +1 * (100000 * nPhotos + 1075);
  }//D_s+ decays to anti-K0 K0 pi+ pi+ pi-
  if (GenDTag::PcheckDecay(genpart, 310, 211, 211, -211) == 1) {
    return +1 * (100000 * nPhotos + 1076);
  }//D_s+ decays to K_S0 pi+ pi+ pi-
  if (GenDTag::PcheckDecay(genpart, 130, 211, 211, -211) == 1) {
    return +1 * (100000 * nPhotos + 1077);
  }//D_s+ decays to K_L0 pi+ pi+ pi-
  if (GenDTag::PcheckDecay(genpart, -311, 211, 211, -211) == 1) {
    return +1 * (100000 * nPhotos + 1078);
  }//D_s+ decays to anti-K0 pi+ pi+ pi-
  if (GenDTag::PcheckDecay(genpart, 321, 321, -211) == 1) {
    return +1 * (100000 * nPhotos + 1079);
  }//D_s+ decays to K+ K+ pi-
  if (GenDTag::PcheckDecay(genpart, 311, -11, 12) == 1) {
    return +1 * (100000 * nPhotos + 1080);
  }//D_s+ decays to K0 e+ nu_e
  if (GenDTag::PcheckDecay(genpart, 313, -11, 12) == 1) {
    return +1 * (100000 * nPhotos + 1081);
  }//D_s+ decays to K*0 e+ nu_e
  if (GenDTag::PcheckDecay(genpart, 321, -321, 211, 111) == 1) {
    return +1 * (100000 * nPhotos + 1082);
  }//D_s+ decays to K+ K- pi+ pi0
  if (GenDTag::PcheckDecay(genpart, 223, 211, 111) == 1) {
    return +1 * (100000 * nPhotos + 1083);
  }//D_s+ decays to omega pi+ pi0
  if (GenDTag::PcheckDecay(genpart, 223, 211, 211, -211) == 1) {
    return +1 * (100000 * nPhotos + 1084);
  }//D_s+ decays to omega pi+ pi+ pi-
  return +1 * (DauSize(genpart));
}// Rest of the D_s+ decays


int GenDTag::Mode_Ds_minus(std::vector<int>genpart)
{
  if (GenDTag::PcheckDecay(genpart, 333, 11, -12) == 1) {
    return -1 * (100000 * nPhotos + 1001);
  }//D_s- decays to phi e- anti-nu_e
  if (GenDTag::PcheckDecay(genpart, 221, 11, -12) == 1) {
    return -1 * (100000 * nPhotos + 1002);
  }//D_s- decays to eta e- anti-nu_e
  if (GenDTag::PcheckDecay(genpart, 331, 11, -12) == 1) {
    return -1 * (100000 * nPhotos + 1003);
  }//D_s- decays to eta' e- anti-nu_e
  if (GenDTag::PcheckDecay(genpart, 311, 11, -12) == 1) {
    return -1 * (100000 * nPhotos + 1004);
  }//D_s- decays to K0 e- anti-nu_e
  if (GenDTag::PcheckDecay(genpart, 313, 11, -12) == 1) {
    return -1 * (100000 * nPhotos + 1005);
  }//D_s- decays to K*0 e- anti-nu_e
  if (GenDTag::PcheckDecay(genpart, 333, 13, -14) == 1) {
    return -1 * (100000 * nPhotos + 1006);
  }//D_s- decays to phi mu- anti-nu_mu
  if (GenDTag::PcheckDecay(genpart, 221, 13, -14) == 1) {
    return -1 * (100000 * nPhotos + 1007);
  }//D_s- decays to eta mu- anti-nu_mu
  if (GenDTag::PcheckDecay(genpart, 331, 13, -14) == 1) {
    return -1 * (100000 * nPhotos + 1008);
  }//D_s- decays to eta' mu- anti-nu_mu
  if (GenDTag::PcheckDecay(genpart, 311, 13, -14) == 1) {
    return -1 * (100000 * nPhotos + 1009);
  }//D_s- decays to K0 mu- anti-nu_mu
  if (GenDTag::PcheckDecay(genpart, 313, 13, -14) == 1) {
    return -1 * (100000 * nPhotos + 1010);
  }//D_s- decays to K*0 mu- anti-nu_mu
  if (GenDTag::PcheckDecay(genpart, 13, -14) == 1) {
    return -1 * (100000 * nPhotos + 1011);
  }//D_s- decays to mu- anti-nu_mu
  if (GenDTag::PcheckDecay(genpart, 15, -16) == 1) {
    return -1 * (100000 * nPhotos + 1012);
  }//D_s- decays to tau- anti-nu_tau
  if (GenDTag::PcheckDecay(genpart, 333, -211) == 1) {
    return -1 * (100000 * nPhotos + 1013);
  }//D_s- decays to phi pi-
  if (GenDTag::PcheckDecay(genpart, 221, -211) == 1) {
    return -1 * (100000 * nPhotos + 1014);
  }//D_s- decays to eta pi-
  if (GenDTag::PcheckDecay(genpart, 331, -211) == 1) {
    return -1 * (100000 * nPhotos + 1015);
  }//D_s- decays to eta' pi-
  if (GenDTag::PcheckDecay(genpart, 223, -211) == 1) {
    return -1 * (100000 * nPhotos + 1016);
  }//D_s- decays to omega pi-
  if (GenDTag::PcheckDecay(genpart, 113, -211) == 1) {
    return -1 * (100000 * nPhotos + 1017);
  }//D_s- decays to rho0 pi-
  if (GenDTag::PcheckDecay(genpart, -213, 111) == 1) {
    return -1 * (100000 * nPhotos + 1018);
  }//D_s- decays to rho- pi0
  if (GenDTag::PcheckDecay(genpart, -211, 111) == 1) {
    return -1 * (100000 * nPhotos + 1019);
  }//D_s- decays to pi- pi0
  if (GenDTag::PcheckDecay(genpart, 10221, -211) == 1) {
    return -1 * (100000 * nPhotos + 1020);
  }//D_s- decays to f_0 pi-
  if (GenDTag::PcheckDecay(genpart, 225, -211) == 1) {
    return -1 * (100000 * nPhotos + 1021);
  }//D_s- decays to f_2 pi-
  if (GenDTag::PcheckDecay(genpart, 333, -213) == 1) {
    return -1 * (100000 * nPhotos + 1022);
  }//D_s- decays to phi rho-
  if (GenDTag::PcheckDecay(genpart, -213, 221) == 1) {
    return -1 * (100000 * nPhotos + 1023);
  }//D_s- decays to rho- eta
  if (GenDTag::PcheckDecay(genpart, -213, 331) == 1) {
    return -1 * (100000 * nPhotos + 1024);
  }//D_s- decays to rho- eta'
  if (GenDTag::PcheckDecay(genpart, -211, -211, 211) == 1) {
    return -1 * (100000 * nPhotos + 1025);
  }//D_s- decays to pi- pi- pi+
  if (GenDTag::PcheckDecay(genpart, -211, 111, 111) == 1) {
    return -1 * (100000 * nPhotos + 1026);
  }//D_s- decays to pi- pi0 pi0
  if (GenDTag::PcheckDecay(genpart, 333, -211, 111) == 1) {
    return -1 * (100000 * nPhotos + 1027);
  }//D_s- decays to phi pi- pi0
  if (GenDTag::PcheckDecay(genpart, 221, -211, 111) == 1) {
    return -1 * (100000 * nPhotos + 1028);
  }//D_s- decays to eta pi- pi0
  if (GenDTag::PcheckDecay(genpart, 331, -211, 111) == 1) {
    return -1 * (100000 * nPhotos + 1029);
  }//D_s- decays to eta' pi- pi0
  if (GenDTag::PcheckDecay(genpart, 333, -211, -211, 211) == 1) {
    return -1 * (100000 * nPhotos + 1030);
  }//D_s- decays to phi pi- pi- pi+
  if (GenDTag::PcheckDecay(genpart, 333, -211, 111, 111) == 1) {
    return -1 * (100000 * nPhotos + 1031);
  }//D_s- decays to phi pi- pi0 pi0
  if (GenDTag::PcheckDecay(genpart, 221, -211, -211, 211) == 1) {
    return -1 * (100000 * nPhotos + 1032);
  }//D_s- decays to eta pi- pi- pi+
  if (GenDTag::PcheckDecay(genpart, 221, -211, 111, 111) == 1) {
    return -1 * (100000 * nPhotos + 1033);
  }//D_s- decays to eta pi- pi0 pi0
  if (GenDTag::PcheckDecay(genpart, 310, -321) == 1) {
    return -1 * (100000 * nPhotos + 1034);
  }//D_s- decays to K_S0 K-
  if (GenDTag::PcheckDecay(genpart, 130, -321) == 1) {
    return -1 * (100000 * nPhotos + 1035);
  }//D_s- decays to K_L0 K-
  if (GenDTag::PcheckDecay(genpart, 311, -321) == 1) {
    return -1 * (100000 * nPhotos + 1036);
  }//D_s- decays to K0 K-
  if (GenDTag::PcheckDecay(genpart, 313, -321) == 1) {
    return -1 * (100000 * nPhotos + 1037);
  }//D_s- decays to K*0 K-
  if (GenDTag::PcheckDecay(genpart, -323, 311) == 1) {
    return -1 * (100000 * nPhotos + 1038);
  }//D_s- decays to K*- K0
  if (GenDTag::PcheckDecay(genpart, 313, -323) == 1) {
    return -1 * (100000 * nPhotos + 1039);
  }//D_s- decays to K*0 K*-
  if (GenDTag::PcheckDecay(genpart, 311, -321, 111) == 1) {
    return -1 * (100000 * nPhotos + 1040);
  }//D_s- decays to K0 K- pi0
  if (GenDTag::PcheckDecay(genpart, 313, -321, 111) == 1) {
    return -1 * (100000 * nPhotos + 1041);
  }//D_s- decays to K*0 K- pi0
  if (GenDTag::PcheckDecay(genpart, -323, 311, 111) == 1) {
    return -1 * (100000 * nPhotos + 1042);
  }//D_s- decays to K*- K0 pi0
  if (GenDTag::PcheckDecay(genpart, 313, -323, 111) == 1) {
    return -1 * (100000 * nPhotos + 1043);
  }//D_s- decays to K*0 K*- pi0
  if (GenDTag::PcheckDecay(genpart, 321, -321, -211) == 1) {
    return -1 * (100000 * nPhotos + 1044);
  }//D_s- decays to K+ K- pi-
  if (GenDTag::PcheckDecay(genpart, 310, -321, 211, -211) == 1) {
    return -1 * (100000 * nPhotos + 1045);
  }//D_s- decays to K_S0 K- pi+ pi-
  if (GenDTag::PcheckDecay(genpart, 130, -321, 211, -211) == 1) {
    return -1 * (100000 * nPhotos + 1046);
  }//D_s- decays to K_L0 K- pi+ pi-
  if (GenDTag::PcheckDecay(genpart, 311, -321, 211, -211) == 1) {
    return -1 * (100000 * nPhotos + 1047);
  }//D_s- decays to K0 K- pi+ pi-
  if (GenDTag::PcheckDecay(genpart, 311, -321, 111, 111) == 1) {
    return -1 * (100000 * nPhotos + 1048);
  }//D_s- decays to K0 K- pi0 pi0
  if (GenDTag::PcheckDecay(genpart, -321, 321, -211, 211, -211) == 1) {
    return -1 * (100000 * nPhotos + 1049);
  }//D_s- decays to K- K+ pi- pi+ pi-
  if (GenDTag::PcheckDecay(genpart, 333, -321) == 1) {
    return -1 * (100000 * nPhotos + 1050);
  }//D_s- decays to phi K-
  if (GenDTag::PcheckDecay(genpart, 221, -321) == 1) {
    return -1 * (100000 * nPhotos + 1051);
  }//D_s- decays to eta K-
  if (GenDTag::PcheckDecay(genpart, 331, -321) == 1) {
    return -1 * (100000 * nPhotos + 1052);
  }//D_s- decays to eta' K-
  if (GenDTag::PcheckDecay(genpart, 221, -321, 111) == 1) {
    return -1 * (100000 * nPhotos + 1053);
  }//D_s- decays to eta K- pi0
  if (GenDTag::PcheckDecay(genpart, 221, -321, 211, -211) == 1) {
    return -1 * (100000 * nPhotos + 1054);
  }//D_s- decays to eta K- pi+ pi-
  if (GenDTag::PcheckDecay(genpart, 331, -321, 111) == 1) {
    return -1 * (100000 * nPhotos + 1055);
  }//D_s- decays to eta' K- pi0
  if (GenDTag::PcheckDecay(genpart, 331, -321, 211, -211) == 1) {
    return -1 * (100000 * nPhotos + 1056);
  }//D_s- decays to eta' K- pi+ pi-
  if (GenDTag::PcheckDecay(genpart, -321, -321, 321) == 1) {
    return -1 * (100000 * nPhotos + 1057);
  }//D_s- decays to K- K- K+
  if (GenDTag::PcheckDecay(genpart, 310, -211) == 1) {
    return -1 * (100000 * nPhotos + 1058);
  }//D_s- decays to K_S0 pi-
  if (GenDTag::PcheckDecay(genpart, 130, -211) == 1) {
    return -1 * (100000 * nPhotos + 1059);
  }//D_s- decays to K_L0 pi-
  if (GenDTag::PcheckDecay(genpart, -311, -211) == 1) {
    return -1 * (100000 * nPhotos + 1060);
  }//D_s- decays to anti-K0 pi-
  if (GenDTag::PcheckDecay(genpart, -213, -311) == 1) {
    return -1 * (100000 * nPhotos + 1061);
  }//D_s- decays to rho- anti-K0
  if (GenDTag::PcheckDecay(genpart, 113, -321) == 1) {
    return -1 * (100000 * nPhotos + 1062);
  }//D_s- decays to rho0 K-
  if (GenDTag::PcheckDecay(genpart, -311, -211, 111) == 1) {
    return -1 * (100000 * nPhotos + 1063);
  }//D_s- decays to anti-K0 pi- pi0
  if (GenDTag::PcheckDecay(genpart, -20213, -311) == 1) {
    return -1 * (100000 * nPhotos + 1064);
  }//D_s- decays to a_1- anti-K0
  if (GenDTag::PcheckDecay(genpart, -313, -211) == 1) {
    return -1 * (100000 * nPhotos + 1065);
  }//D_s- decays to anti-K*0 pi-
  if (GenDTag::PcheckDecay(genpart, -313, -213) == 1) {
    return -1 * (100000 * nPhotos + 1066);
  }//D_s- decays to anti-K*0 rho-
  if (GenDTag::PcheckDecay(genpart, -313, -211, 111) == 1) {
    return -1 * (100000 * nPhotos + 1067);
  }//D_s- decays to anti-K*0 pi- pi0
  if (GenDTag::PcheckDecay(genpart, -321, 111) == 1) {
    return -1 * (100000 * nPhotos + 1068);
  }//D_s- decays to K- pi0
  if (GenDTag::PcheckDecay(genpart, -321, 211, -211) == 1) {
    return -1 * (100000 * nPhotos + 1069);
  }//D_s- decays to K- pi+ pi-
  if (GenDTag::PcheckDecay(genpart, 211, 211, -211, -211, -211) == 1) {
    return -1 * (100000 * nPhotos + 1070);
  }//D_s- decays to pi+ pi+ pi- pi- pi-
  if (GenDTag::PcheckDecay(genpart, 211, 211, -211, -211, -211, 111) == 1) {
    return -1 * (100000 * nPhotos + 1071);
  }//D_s- decays to pi+ pi+ pi- pi- pi- pi0
  if (GenDTag::PcheckDecay(genpart, -2212, 2112) == 1) {
    return -1 * (100000 * nPhotos + 1072);
  }//D_s- decays to anti-p- n0
  if (GenDTag::PcheckDecay(genpart, 310, 310, 211, -211, -211) == 1) {
    return -1 * (100000 * nPhotos + 1073);
  }//D_s- decays to K_S0 K_S0 pi+ pi- pi-
  if (GenDTag::PcheckDecay(genpart, 130, 130, 211, -211, -211) == 1) {
    return -1 * (100000 * nPhotos + 1074);
  }//D_s- decays to K_L0 K_L0 pi+ pi- pi-
  if (GenDTag::PcheckDecay(genpart, -311, 311, 211, -211, -211) == 1) {
    return -1 * (100000 * nPhotos + 1075);
  }//D_s- decays to anti-K0 K0 pi+ pi- pi-
  if (GenDTag::PcheckDecay(genpart, 310, 211, -211, -211) == 1) {
    return -1 * (100000 * nPhotos + 1076);
  }//D_s- decays to K_S0 pi+ pi- pi-
  if (GenDTag::PcheckDecay(genpart, 130, 211, -211, -211) == 1) {
    return -1 * (100000 * nPhotos + 1077);
  }//D_s- decays to K_L0 pi+ pi- pi-
  if (GenDTag::PcheckDecay(genpart, 311, 211, -211, -211) == 1) {
    return -1 * (100000 * nPhotos + 1078);
  }//D_s- decays to K0 pi+ pi- pi-
  if (GenDTag::PcheckDecay(genpart, -321, -321, 211) == 1) {
    return -1 * (100000 * nPhotos + 1079);
  }//D_s- decays to K- K- pi+
  if (GenDTag::PcheckDecay(genpart, -311, 11, -12) == 1) {
    return -1 * (100000 * nPhotos + 1080);
  }//D_s- decays to anti-K0 e- anti-nu_e
  if (GenDTag::PcheckDecay(genpart, -313, 11, -12) == 1) {
    return -1 * (100000 * nPhotos + 1081);
  }//D_s- decays to anti-K*0 e- anti-nu_e
  if (GenDTag::PcheckDecay(genpart, -321, 321, -211, 111) == 1) {
    return -1 * (100000 * nPhotos + 1082);
  }//D_s- decays to K- K+ pi- pi0
  if (GenDTag::PcheckDecay(genpart, 223, -211, 111) == 1) {
    return -1 * (100000 * nPhotos + 1083);
  }//D_s- decays to omega pi- pi0
  if (GenDTag::PcheckDecay(genpart, 223, -211, -211, 211) == 1) {
    return -1 * (100000 * nPhotos + 1084);
  }//D_s- decays to omega pi- pi- pi+
  return -1 * (DauSize(genpart));
}// Rest of the D_s- decays


int GenDTag::Mode_D_plus(std::vector<int>genpart)
{
  if (GenDTag::PcheckDecay(genpart, -313, -11, 12) == 1) {
    return +1 * (100000 * nPhotos + 1001);
  }//D+ decays to anti-K*0 e+ nu_e
  if (GenDTag::PcheckDecay(genpart, -311, -11, 12) == 1) {
    return +1 * (100000 * nPhotos + 1002);
  }//D+ decays to anti-K0 e+ nu_e
  if (GenDTag::PcheckDecay(genpart, -10313, -11, 12) == 1) {
    return +1 * (100000 * nPhotos + 1003);
  }//D+ decays to anti-K_10 e+ nu_e
  if (GenDTag::PcheckDecay(genpart, -315, -11, 12) == 1) {
    return +1 * (100000 * nPhotos + 1004);
  }//D+ decays to anti-K_2*0 e+ nu_e
  if (GenDTag::PcheckDecay(genpart, 111, -11, 12) == 1) {
    return +1 * (100000 * nPhotos + 1005);
  }//D+ decays to pi0 e+ nu_e
  if (GenDTag::PcheckDecay(genpart, 221, -11, 12) == 1) {
    return +1 * (100000 * nPhotos + 1006);
  }//D+ decays to eta e+ nu_e
  if (GenDTag::PcheckDecay(genpart, 331, -11, 12) == 1) {
    return +1 * (100000 * nPhotos + 1007);
  }//D+ decays to eta' e+ nu_e
  if (GenDTag::PcheckDecay(genpart, 113, -11, 12) == 1) {
    return +1 * (100000 * nPhotos + 1008);
  }//D+ decays to rho0 e+ nu_e
  if (GenDTag::PcheckDecay(genpart, 223, -11, 12) == 1) {
    return +1 * (100000 * nPhotos + 1009);
  }//D+ decays to omega e+ nu_e
  if (GenDTag::PcheckDecay(genpart, -321, 211, -11, 12) == 1) {
    return +1 * (100000 * nPhotos + 1010);
  }//D+ decays to K- pi+ e+ nu_e
  if (GenDTag::PcheckDecay(genpart, -311, 111, -11, 12) == 1) {
    return +1 * (100000 * nPhotos + 1011);
  }//D+ decays to anti-K0 pi0 e+ nu_e
  if (GenDTag::PcheckDecay(genpart, -313, -13, 14) == 1) {
    return +1 * (100000 * nPhotos + 1012);
  }//D+ decays to anti-K*0 mu+ nu_mu
  if (GenDTag::PcheckDecay(genpart, -311, -13, 14) == 1) {
    return +1 * (100000 * nPhotos + 1013);
  }//D+ decays to anti-K0 mu+ nu_mu
  if (GenDTag::PcheckDecay(genpart, -10313, -13, 14) == 1) {
    return +1 * (100000 * nPhotos + 1014);
  }//D+ decays to anti-K_10 mu+ nu_mu
  if (GenDTag::PcheckDecay(genpart, -315, -13, 14) == 1) {
    return +1 * (100000 * nPhotos + 1015);
  }//D+ decays to anti-K_2*0 mu+ nu_mu
  if (GenDTag::PcheckDecay(genpart, 111, -13, 14) == 1) {
    return +1 * (100000 * nPhotos + 1016);
  }//D+ decays to pi0 mu+ nu_mu
  if (GenDTag::PcheckDecay(genpart, 221, -13, 14) == 1) {
    return +1 * (100000 * nPhotos + 1017);
  }//D+ decays to eta mu+ nu_mu
  if (GenDTag::PcheckDecay(genpart, 331, -13, 14) == 1) {
    return +1 * (100000 * nPhotos + 1018);
  }//D+ decays to eta' mu+ nu_mu
  if (GenDTag::PcheckDecay(genpart, 113, -13, 14) == 1) {
    return +1 * (100000 * nPhotos + 1019);
  }//D+ decays to rho0 mu+ nu_mu
  if (GenDTag::PcheckDecay(genpart, 223, -13, 14) == 1) {
    return +1 * (100000 * nPhotos + 1020);
  }//D+ decays to omega mu+ nu_mu
  if (GenDTag::PcheckDecay(genpart, -321, 211, -13, 14) == 1) {
    return +1 * (100000 * nPhotos + 1021);
  }//D+ decays to K- pi+ mu+ nu_mu
  if (GenDTag::PcheckDecay(genpart, -311, 111, -13, 14) == 1) {
    return +1 * (100000 * nPhotos + 1022);
  }//D+ decays to anti-K0 pi0 mu+ nu_mu
  if (GenDTag::PcheckDecay(genpart, -13, 14) == 1) {
    return +1 * (100000 * nPhotos + 1023);
  }//D+ decays to mu+ nu_mu
  if (GenDTag::PcheckDecay(genpart, -15, 16) == 1) {
    return +1 * (100000 * nPhotos + 1024);
  }//D+ decays to tau+ nu_tau
  if (GenDTag::PcheckDecay(genpart, 310, 211) == 1) {
    return +1 * (100000 * nPhotos + 1025);
  }//D+ decays to K_S0 pi+
  if (GenDTag::PcheckDecay(genpart, 130, 211) == 1) {
    return +1 * (100000 * nPhotos + 1026);
  }//D+ decays to K_L0 pi+
  if (GenDTag::PcheckDecay(genpart, -311, 211) == 1) {
    return +1 * (100000 * nPhotos + 1027);
  }//D+ decays to anti-K0 pi+
  if (GenDTag::PcheckDecay(genpart, 20213, 310) == 1) {
    return +1 * (100000 * nPhotos + 1028);
  }//D+ decays to a_1+ K_S0
  if (GenDTag::PcheckDecay(genpart, 20213, 130) == 1) {
    return +1 * (100000 * nPhotos + 1029);
  }//D+ decays to a_1+ K_L0
  if (GenDTag::PcheckDecay(genpart, 20213, -311) == 1) {
    return +1 * (100000 * nPhotos + 1030);
  }//D+ decays to a_1+ anti-K0
  if (GenDTag::PcheckDecay(genpart, -20313, 211) == 1) {
    return +1 * (100000 * nPhotos + 1031);
  }//D+ decays to anti-K'_10 pi+
  if (GenDTag::PcheckDecay(genpart, -313, 213) == 1) {
    return +1 * (100000 * nPhotos + 1032);
  }//D+ decays to anti-K*0 rho+
  if (GenDTag::PcheckDecay(genpart, -321, 211, 211) == 1) {
    return +1 * (100000 * nPhotos + 1033);
  }//D+ decays to K- pi+ pi+
  if (GenDTag::PcheckDecay(genpart, 310, 211, 111) == 1) {
    return +1 * (100000 * nPhotos + 1034);
  }//D+ decays to K_S0 pi+ pi0
  if (GenDTag::PcheckDecay(genpart, 130, 211, 111) == 1) {
    return +1 * (100000 * nPhotos + 1035);
  }//D+ decays to K_L0 pi+ pi0
  if (GenDTag::PcheckDecay(genpart, -311, 211, 111) == 1) {
    return +1 * (100000 * nPhotos + 1036);
  }//D+ decays to anti-K0 pi+ pi0
  if (GenDTag::PcheckDecay(genpart, 310, 113, 211) == 1) {
    return +1 * (100000 * nPhotos + 1037);
  }//D+ decays to K_S0 rho0 pi+
  if (GenDTag::PcheckDecay(genpart, 130, 113, 211) == 1) {
    return +1 * (100000 * nPhotos + 1038);
  }//D+ decays to K_L0 rho0 pi+
  if (GenDTag::PcheckDecay(genpart, -311, 113, 211) == 1) {
    return +1 * (100000 * nPhotos + 1039);
  }//D+ decays to anti-K0 rho0 pi+
  if (GenDTag::PcheckDecay(genpart, -311, 223, 211) == 1) {
    return +1 * (100000 * nPhotos + 1040);
  }//D+ decays to anti-K0 omega pi+
  if (GenDTag::PcheckDecay(genpart, -321, 213, 211) == 1) {
    return +1 * (100000 * nPhotos + 1041);
  }//D+ decays to K- rho+ pi+
  if (GenDTag::PcheckDecay(genpart, -323, 211, 211) == 1) {
    return +1 * (100000 * nPhotos + 1042);
  }//D+ decays to K*- pi+ pi+
  if (GenDTag::PcheckDecay(genpart, -313, 111, 211) == 1) {
    return +1 * (100000 * nPhotos + 1043);
  }//D+ decays to anti-K*0 pi0 pi+
  if (GenDTag::PcheckDecay(genpart, -313, 113, 211) == 1) {
    return +1 * (100000 * nPhotos + 1044);
  }//D+ decays to anti-K*0 rho0 pi+
  if (GenDTag::PcheckDecay(genpart, -313, 223, 211) == 1) {
    return +1 * (100000 * nPhotos + 1045);
  }//D+ decays to anti-K*0 omega pi+
  if (GenDTag::PcheckDecay(genpart, -321, 211, 211, 111) == 1) {
    return +1 * (100000 * nPhotos + 1046);
  }//D+ decays to K- pi+ pi+ pi0
  if (GenDTag::PcheckDecay(genpart, 310, 211, 211, -211) == 1) {
    return +1 * (100000 * nPhotos + 1047);
  }//D+ decays to K_S0 pi+ pi+ pi-
  if (GenDTag::PcheckDecay(genpart, 130, 211, 211, -211) == 1) {
    return +1 * (100000 * nPhotos + 1048);
  }//D+ decays to K_L0 pi+ pi+ pi-
  if (GenDTag::PcheckDecay(genpart, -311, 211, 211, -211) == 1) {
    return +1 * (100000 * nPhotos + 1049);
  }//D+ decays to anti-K0 pi+ pi+ pi-
  if (GenDTag::PcheckDecay(genpart, -321, 211, 211, 211, -211) == 1) {
    return +1 * (100000 * nPhotos + 1050);
  }//D+ decays to K- pi+ pi+ pi+ pi-
  if (GenDTag::PcheckDecay(genpart, -321, 211, 211, 111, 111) == 1) {
    return +1 * (100000 * nPhotos + 1051);
  }//D+ decays to K- pi+ pi+ pi0 pi0
  if (GenDTag::PcheckDecay(genpart, -311, 211, 211, -211, 111) == 1) {
    return +1 * (100000 * nPhotos + 1052);
  }//D+ decays to anti-K0 pi+ pi+ pi- pi0
  if (GenDTag::PcheckDecay(genpart, -311, 211, 111, 111, 111) == 1) {
    return +1 * (100000 * nPhotos + 1053);
  }//D+ decays to anti-K0 pi+ pi0 pi0 pi0
  if (GenDTag::PcheckDecay(genpart, 310, 310, 321) == 1) {
    return +1 * (100000 * nPhotos + 1054);
  }//D+ decays to K_S0 K_S0 K+
  if (GenDTag::PcheckDecay(genpart, 130, 130, 321) == 1) {
    return +1 * (100000 * nPhotos + 1055);
  }//D+ decays to K_L0 K_L0 K+
  if (GenDTag::PcheckDecay(genpart, -311, -311, 321) == 1) {
    return +1 * (100000 * nPhotos + 1056);
  }//D+ decays to anti-K0 anti-K0 K+
  if (GenDTag::PcheckDecay(genpart, 333, 211) == 1) {
    return +1 * (100000 * nPhotos + 1057);
  }//D+ decays to phi pi+
  if (GenDTag::PcheckDecay(genpart, 333, 211, 111) == 1) {
    return +1 * (100000 * nPhotos + 1058);
  }//D+ decays to phi pi+ pi0
  if (GenDTag::PcheckDecay(genpart, 310, 321) == 1) {
    return +1 * (100000 * nPhotos + 1059);
  }//D+ decays to K_S0 K+
  if (GenDTag::PcheckDecay(genpart, 130, 321) == 1) {
    return +1 * (100000 * nPhotos + 1060);
  }//D+ decays to K_L0 K+
  if (GenDTag::PcheckDecay(genpart, -311, 321) == 1) {
    return +1 * (100000 * nPhotos + 1061);
  }//D+ decays to anti-K0 K+
  if (GenDTag::PcheckDecay(genpart, -313, 321) == 1) {
    return +1 * (100000 * nPhotos + 1062);
  }//D+ decays to anti-K*0 K+
  if (GenDTag::PcheckDecay(genpart, 323, 310) == 1) {
    return +1 * (100000 * nPhotos + 1063);
  }//D+ decays to K*+ K_S0
  if (GenDTag::PcheckDecay(genpart, 323, 130) == 1) {
    return +1 * (100000 * nPhotos + 1064);
  }//D+ decays to K*+ K_L0
  if (GenDTag::PcheckDecay(genpart, 323, -311) == 1) {
    return +1 * (100000 * nPhotos + 1065);
  }//D+ decays to K*+ anti-K0
  if (GenDTag::PcheckDecay(genpart, 321, -321, 211) == 1) {
    return +1 * (100000 * nPhotos + 1066);
  }//D+ decays to K+ K- pi+
  if (GenDTag::PcheckDecay(genpart, 321, -311, 111) == 1) {
    return +1 * (100000 * nPhotos + 1067);
  }//D+ decays to K+ anti-K0 pi0
  if (GenDTag::PcheckDecay(genpart, -311, 311, 211) == 1) {
    return +1 * (100000 * nPhotos + 1068);
  }//D+ decays to anti-K0 K0 pi+
  if (GenDTag::PcheckDecay(genpart, 323, -321, 211) == 1) {
    return +1 * (100000 * nPhotos + 1069);
  }//D+ decays to K*+ K- pi+
  if (GenDTag::PcheckDecay(genpart, 321, -323, 211) == 1) {
    return +1 * (100000 * nPhotos + 1070);
  }//D+ decays to K+ K*- pi+
  if (GenDTag::PcheckDecay(genpart, 323, -311, 111) == 1) {
    return +1 * (100000 * nPhotos + 1071);
  }//D+ decays to K*+ anti-K0 pi0
  if (GenDTag::PcheckDecay(genpart, 321, -313, 111) == 1) {
    return +1 * (100000 * nPhotos + 1072);
  }//D+ decays to K+ anti-K*0 pi0
  if (GenDTag::PcheckDecay(genpart, -313, 311, 211) == 1) {
    return +1 * (100000 * nPhotos + 1073);
  }//D+ decays to anti-K*0 K0 pi+
  if (GenDTag::PcheckDecay(genpart, -311, 313, 211) == 1) {
    return +1 * (100000 * nPhotos + 1074);
  }//D+ decays to anti-K0 K*0 pi+
  if (GenDTag::PcheckDecay(genpart, 111, 211) == 1) {
    return +1 * (100000 * nPhotos + 1075);
  }//D+ decays to pi0 pi+
  if (GenDTag::PcheckDecay(genpart, 113, 211) == 1) {
    return +1 * (100000 * nPhotos + 1076);
  }//D+ decays to rho0 pi+
  if (GenDTag::PcheckDecay(genpart, 211, 211, -211) == 1) {
    return +1 * (100000 * nPhotos + 1077);
  }//D+ decays to pi+ pi+ pi-
  if (GenDTag::PcheckDecay(genpart, 211, 111, 111) == 1) {
    return +1 * (100000 * nPhotos + 1078);
  }//D+ decays to pi+ pi0 pi0
  if (GenDTag::PcheckDecay(genpart, 211, 211, -211, 111) == 1) {
    return +1 * (100000 * nPhotos + 1079);
  }//D+ decays to pi+ pi+ pi- pi0
  if (GenDTag::PcheckDecay(genpart, 211, 111, 111, 111) == 1) {
    return +1 * (100000 * nPhotos + 1080);
  }//D+ decays to pi+ pi0 pi0 pi0
  if (GenDTag::PcheckDecay(genpart, 221, 211) == 1) {
    return +1 * (100000 * nPhotos + 1081);
  }//D+ decays to eta pi+
  if (GenDTag::PcheckDecay(genpart, 331, 211) == 1) {
    return +1 * (100000 * nPhotos + 1082);
  }//D+ decays to eta' pi+
  if (GenDTag::PcheckDecay(genpart, 221, 211, 111) == 1) {
    return +1 * (100000 * nPhotos + 1083);
  }//D+ decays to eta pi+ pi0
  if (GenDTag::PcheckDecay(genpart, 221, 211, 211, -211) == 1) {
    return +1 * (100000 * nPhotos + 1084);
  }//D+ decays to eta pi+ pi+ pi-
  if (GenDTag::PcheckDecay(genpart, 221, 211, 111, 111) == 1) {
    return +1 * (100000 * nPhotos + 1085);
  }//D+ decays to eta pi+ pi0 pi0
  if (GenDTag::PcheckDecay(genpart, 211, 211, 211, -211, -211) == 1) {
    return +1 * (100000 * nPhotos + 1086);
  }//D+ decays to pi+ pi+ pi+ pi- pi-
  if (GenDTag::PcheckDecay(genpart, 321, -321, 211, 111) == 1) {
    return +1 * (100000 * nPhotos + 1087);
  }//D+ decays to K+ K- pi+ pi0
  if (GenDTag::PcheckDecay(genpart, 321, 310, 211, -211) == 1) {
    return +1 * (100000 * nPhotos + 1088);
  }//D+ decays to K+ K_S0 pi+ pi-
  if (GenDTag::PcheckDecay(genpart, 321, 130, 211, -211) == 1) {
    return +1 * (100000 * nPhotos + 1089);
  }//D+ decays to K+ K_L0 pi+ pi-
  if (GenDTag::PcheckDecay(genpart, 321, -311, 211, -211) == 1) {
    return +1 * (100000 * nPhotos + 1090);
  }//D+ decays to K+ anti-K0 pi+ pi-
  if (GenDTag::PcheckDecay(genpart, 310, -321, 211, 211) == 1) {
    return +1 * (100000 * nPhotos + 1091);
  }//D+ decays to K_S0 K- pi+ pi+
  if (GenDTag::PcheckDecay(genpart, 130, -321, 211, 211) == 1) {
    return +1 * (100000 * nPhotos + 1092);
  }//D+ decays to K_L0 K- pi+ pi+
  if (GenDTag::PcheckDecay(genpart, 311, -321, 211, 211) == 1) {
    return +1 * (100000 * nPhotos + 1093);
  }//D+ decays to K0 K- pi+ pi+
  if (GenDTag::PcheckDecay(genpart, 321, -321, 211, 211, -211) == 1) {
    return +1 * (100000 * nPhotos + 1094);
  }//D+ decays to K+ K- pi+ pi+ pi-
  if (GenDTag::PcheckDecay(genpart, 321, -321, 310, 211) == 1) {
    return +1 * (100000 * nPhotos + 1095);
  }//D+ decays to K+ K- K_S0 pi+
  if (GenDTag::PcheckDecay(genpart, 321, -321, 130, 211) == 1) {
    return +1 * (100000 * nPhotos + 1096);
  }//D+ decays to K+ K- K_L0 pi+
  if (GenDTag::PcheckDecay(genpart, 321, -321, -311, 211) == 1) {
    return +1 * (100000 * nPhotos + 1097);
  }//D+ decays to K+ K- anti-K0 pi+
  if (GenDTag::PcheckDecay(genpart, -313, 20213) == 1) {
    return +1 * (100000 * nPhotos + 1098);
  }//D+ decays to anti-K*0 a_1+
  if (GenDTag::PcheckDecay(genpart, 321, 111) == 1) {
    return +1 * (100000 * nPhotos + 1099);
  }//D+ decays to K+ pi0
  if (GenDTag::PcheckDecay(genpart, 321, 211, -211) == 1) {
    return +1 * (100000 * nPhotos + 1100);
  }//D+ decays to K+ pi+ pi-
  if (GenDTag::PcheckDecay(genpart, 321, 321, -321) == 1) {
    return +1 * (100000 * nPhotos + 1101);
  }//D+ decays to K+ K+ K-
  if (GenDTag::PcheckDecay(genpart, -321, 113, 211, 211) == 1) {
    return +1 * (100000 * nPhotos + 1102);
  }//D+ decays to K- rho0 pi+ pi+
  if (GenDTag::PcheckDecay(genpart, 331, 211, 111) == 1) {
    return +1 * (100000 * nPhotos + 1103);
  }//D+ decays to eta' pi+ pi0
  if (GenDTag::PcheckDecay(genpart, 321, 113) == 1) {
    return +1 * (100000 * nPhotos + 1104);
  }//D+ decays to K+ rho0
  return +1 * (DauSize(genpart));
}// Rest of the D+ decays


int GenDTag::Mode_D_minus(std::vector<int>genpart)
{
  if (GenDTag::PcheckDecay(genpart, 313, 11, -12) == 1) {
    return -1 * (100000 * nPhotos + 1001);
  }//D- decays to K*0 e- anti-nu_e
  if (GenDTag::PcheckDecay(genpart, 311, 11, -12) == 1) {
    return -1 * (100000 * nPhotos + 1002);
  }//D- decays to K0 e- anti-nu_e
  if (GenDTag::PcheckDecay(genpart, 10313, 11, -12) == 1) {
    return -1 * (100000 * nPhotos + 1003);
  }//D- decays to K_10 e- anti-nu_e
  if (GenDTag::PcheckDecay(genpart, 315, 11, -12) == 1) {
    return -1 * (100000 * nPhotos + 1004);
  }//D- decays to K_2*0 e- anti-nu_e
  if (GenDTag::PcheckDecay(genpart, 111, 11, -12) == 1) {
    return -1 * (100000 * nPhotos + 1005);
  }//D- decays to pi0 e- anti-nu_e
  if (GenDTag::PcheckDecay(genpart, 221, 11, -12) == 1) {
    return -1 * (100000 * nPhotos + 1006);
  }//D- decays to eta e- anti-nu_e
  if (GenDTag::PcheckDecay(genpart, 331, 11, -12) == 1) {
    return -1 * (100000 * nPhotos + 1007);
  }//D- decays to eta' e- anti-nu_e
  if (GenDTag::PcheckDecay(genpart, 113, 11, -12) == 1) {
    return -1 * (100000 * nPhotos + 1008);
  }//D- decays to rho0 e- anti-nu_e
  if (GenDTag::PcheckDecay(genpart, 223, 11, -12) == 1) {
    return -1 * (100000 * nPhotos + 1009);
  }//D- decays to omega e- anti-nu_e
  if (GenDTag::PcheckDecay(genpart, 321, -211, 11, -12) == 1) {
    return -1 * (100000 * nPhotos + 1010);
  }//D- decays to K+ pi- e- anti-nu_e
  if (GenDTag::PcheckDecay(genpart, 311, 111, 11, -12) == 1) {
    return -1 * (100000 * nPhotos + 1011);
  }//D- decays to K0 pi0 e- anti-nu_e
  if (GenDTag::PcheckDecay(genpart, 313, 13, -14) == 1) {
    return -1 * (100000 * nPhotos + 1012);
  }//D- decays to K*0 mu- anti-nu_mu
  if (GenDTag::PcheckDecay(genpart, 311, 13, -14) == 1) {
    return -1 * (100000 * nPhotos + 1013);
  }//D- decays to K0 mu- anti-nu_mu
  if (GenDTag::PcheckDecay(genpart, 10313, 13, -14) == 1) {
    return -1 * (100000 * nPhotos + 1014);
  }//D- decays to K_10 mu- anti-nu_mu
  if (GenDTag::PcheckDecay(genpart, 315, 13, -14) == 1) {
    return -1 * (100000 * nPhotos + 1015);
  }//D- decays to K_2*0 mu- anti-nu_mu
  if (GenDTag::PcheckDecay(genpart, 111, 13, -14) == 1) {
    return -1 * (100000 * nPhotos + 1016);
  }//D- decays to pi0 mu- anti-nu_mu
  if (GenDTag::PcheckDecay(genpart, 221, 13, -14) == 1) {
    return -1 * (100000 * nPhotos + 1017);
  }//D- decays to eta mu- anti-nu_mu
  if (GenDTag::PcheckDecay(genpart, 331, 13, -14) == 1) {
    return -1 * (100000 * nPhotos + 1018);
  }//D- decays to eta' mu- anti-nu_mu
  if (GenDTag::PcheckDecay(genpart, 113, 13, -14) == 1) {
    return -1 * (100000 * nPhotos + 1019);
  }//D- decays to rho0 mu- anti-nu_mu
  if (GenDTag::PcheckDecay(genpart, 223, 13, -14) == 1) {
    return -1 * (100000 * nPhotos + 1020);
  }//D- decays to omega mu- anti-nu_mu
  if (GenDTag::PcheckDecay(genpart, 321, -211, 13, -14) == 1) {
    return -1 * (100000 * nPhotos + 1021);
  }//D- decays to K+ pi- mu- anti-nu_mu
  if (GenDTag::PcheckDecay(genpart, 311, 111, 13, -14) == 1) {
    return -1 * (100000 * nPhotos + 1022);
  }//D- decays to K0 pi0 mu- anti-nu_mu
  if (GenDTag::PcheckDecay(genpart, 13, -14) == 1) {
    return -1 * (100000 * nPhotos + 1023);
  }//D- decays to mu- anti-nu_mu
  if (GenDTag::PcheckDecay(genpart, 15, -16) == 1) {
    return -1 * (100000 * nPhotos + 1024);
  }//D- decays to tau- anti-nu_tau
  if (GenDTag::PcheckDecay(genpart, 310, -211) == 1) {
    return -1 * (100000 * nPhotos + 1025);
  }//D- decays to K_S0 pi-
  if (GenDTag::PcheckDecay(genpart, 130, -211) == 1) {
    return -1 * (100000 * nPhotos + 1026);
  }//D- decays to K_L0 pi-
  if (GenDTag::PcheckDecay(genpart, 311, -211) == 1) {
    return -1 * (100000 * nPhotos + 1027);
  }//D- decays to K0 pi-
  if (GenDTag::PcheckDecay(genpart, -20213, 310) == 1) {
    return -1 * (100000 * nPhotos + 1028);
  }//D- decays to a_1- K_S0
  if (GenDTag::PcheckDecay(genpart, -20213, 130) == 1) {
    return -1 * (100000 * nPhotos + 1029);
  }//D- decays to a_1- K_L0
  if (GenDTag::PcheckDecay(genpart, -20213, 311) == 1) {
    return -1 * (100000 * nPhotos + 1030);
  }//D- decays to a_1- K0
  if (GenDTag::PcheckDecay(genpart, 20313, -211) == 1) {
    return -1 * (100000 * nPhotos + 1031);
  }//D- decays to K'_10 pi-
  if (GenDTag::PcheckDecay(genpart, 313, -213) == 1) {
    return -1 * (100000 * nPhotos + 1032);
  }//D- decays to K*0 rho-
  if (GenDTag::PcheckDecay(genpart, 321, -211, -211) == 1) {
    return -1 * (100000 * nPhotos + 1033);
  }//D- decays to K+ pi- pi-
  if (GenDTag::PcheckDecay(genpart, 310, -211, 111) == 1) {
    return -1 * (100000 * nPhotos + 1034);
  }//D- decays to K_S0 pi- pi0
  if (GenDTag::PcheckDecay(genpart, 130, -211, 111) == 1) {
    return -1 * (100000 * nPhotos + 1035);
  }//D- decays to K_L0 pi- pi0
  if (GenDTag::PcheckDecay(genpart, 311, -211, 111) == 1) {
    return -1 * (100000 * nPhotos + 1036);
  }//D- decays to K0 pi- pi0
  if (GenDTag::PcheckDecay(genpart, 310, 113, -211) == 1) {
    return -1 * (100000 * nPhotos + 1037);
  }//D- decays to K_S0 rho0 pi-
  if (GenDTag::PcheckDecay(genpart, 130, 113, -211) == 1) {
    return -1 * (100000 * nPhotos + 1038);
  }//D- decays to K_L0 rho0 pi-
  if (GenDTag::PcheckDecay(genpart, 311, 113, -211) == 1) {
    return -1 * (100000 * nPhotos + 1039);
  }//D- decays to K0 rho0 pi-
  if (GenDTag::PcheckDecay(genpart, 311, 223, -211) == 1) {
    return -1 * (100000 * nPhotos + 1040);
  }//D- decays to K0 omega pi-
  if (GenDTag::PcheckDecay(genpart, 321, -213, -211) == 1) {
    return -1 * (100000 * nPhotos + 1041);
  }//D- decays to K+ rho- pi-
  if (GenDTag::PcheckDecay(genpart, 323, -211, -211) == 1) {
    return -1 * (100000 * nPhotos + 1042);
  }//D- decays to K*+ pi- pi-
  if (GenDTag::PcheckDecay(genpart, 313, 111, -211) == 1) {
    return -1 * (100000 * nPhotos + 1043);
  }//D- decays to K*0 pi0 pi-
  if (GenDTag::PcheckDecay(genpart, 313, 113, -211) == 1) {
    return -1 * (100000 * nPhotos + 1044);
  }//D- decays to K*0 rho0 pi-
  if (GenDTag::PcheckDecay(genpart, 313, 223, -211) == 1) {
    return -1 * (100000 * nPhotos + 1045);
  }//D- decays to K*0 omega pi-
  if (GenDTag::PcheckDecay(genpart, 321, -211, -211, 111) == 1) {
    return -1 * (100000 * nPhotos + 1046);
  }//D- decays to K+ pi- pi- pi0
  if (GenDTag::PcheckDecay(genpart, 310, -211, -211, 211) == 1) {
    return -1 * (100000 * nPhotos + 1047);
  }//D- decays to K_S0 pi- pi- pi+
  if (GenDTag::PcheckDecay(genpart, 130, -211, -211, 211) == 1) {
    return -1 * (100000 * nPhotos + 1048);
  }//D- decays to K_L0 pi- pi- pi+
  if (GenDTag::PcheckDecay(genpart, 311, -211, -211, 211) == 1) {
    return -1 * (100000 * nPhotos + 1049);
  }//D- decays to K0 pi- pi- pi+
  if (GenDTag::PcheckDecay(genpart, 321, -211, -211, -211, 211) == 1) {
    return -1 * (100000 * nPhotos + 1050);
  }//D- decays to K+ pi- pi- pi- pi+
  if (GenDTag::PcheckDecay(genpart, 321, -211, -211, 111, 111) == 1) {
    return -1 * (100000 * nPhotos + 1051);
  }//D- decays to K+ pi- pi- pi0 pi0
  if (GenDTag::PcheckDecay(genpart, 311, -211, -211, 211, 111) == 1) {
    return -1 * (100000 * nPhotos + 1052);
  }//D- decays to K0 pi- pi- pi+ pi0
  if (GenDTag::PcheckDecay(genpart, 311, -211, 111, 111, 111) == 1) {
    return -1 * (100000 * nPhotos + 1053);
  }//D- decays to K0 pi- pi0 pi0 pi0
  if (GenDTag::PcheckDecay(genpart, 310, 310, -321) == 1) {
    return -1 * (100000 * nPhotos + 1054);
  }//D- decays to K_S0 K_S0 K-
  if (GenDTag::PcheckDecay(genpart, 130, 130, -321) == 1) {
    return -1 * (100000 * nPhotos + 1055);
  }//D- decays to K_L0 K_L0 K-
  if (GenDTag::PcheckDecay(genpart, 311, 311, -321) == 1) {
    return -1 * (100000 * nPhotos + 1056);
  }//D- decays to K0 K0 K-
  if (GenDTag::PcheckDecay(genpart, 333, -211) == 1) {
    return -1 * (100000 * nPhotos + 1057);
  }//D- decays to phi pi-
  if (GenDTag::PcheckDecay(genpart, 333, -211, 111) == 1) {
    return -1 * (100000 * nPhotos + 1058);
  }//D- decays to phi pi- pi0
  if (GenDTag::PcheckDecay(genpart, 310, -321) == 1) {
    return -1 * (100000 * nPhotos + 1059);
  }//D- decays to K_S0 K-
  if (GenDTag::PcheckDecay(genpart, 130, -321) == 1) {
    return -1 * (100000 * nPhotos + 1060);
  }//D- decays to K_L0 K-
  if (GenDTag::PcheckDecay(genpart, 311, -321) == 1) {
    return -1 * (100000 * nPhotos + 1061);
  }//D- decays to K0 K-
  if (GenDTag::PcheckDecay(genpart, 313, -321) == 1) {
    return -1 * (100000 * nPhotos + 1062);
  }//D- decays to K*0 K-
  if (GenDTag::PcheckDecay(genpart, -323, 310) == 1) {
    return -1 * (100000 * nPhotos + 1063);
  }//D- decays to K*- K_S0
  if (GenDTag::PcheckDecay(genpart, -323, 130) == 1) {
    return -1 * (100000 * nPhotos + 1064);
  }//D- decays to K*- K_L0
  if (GenDTag::PcheckDecay(genpart, -323, 311) == 1) {
    return -1 * (100000 * nPhotos + 1065);
  }//D- decays to K*- K0
  if (GenDTag::PcheckDecay(genpart, -321, 321, -211) == 1) {
    return -1 * (100000 * nPhotos + 1066);
  }//D- decays to K- K+ pi-
  if (GenDTag::PcheckDecay(genpart, -321, 311, 111) == 1) {
    return -1 * (100000 * nPhotos + 1067);
  }//D- decays to K- K0 pi0
  if (GenDTag::PcheckDecay(genpart, -311, 311, -211) == 1) {
    return -1 * (100000 * nPhotos + 1068);
  }//D- decays to anti-K0 K0 pi-
  if (GenDTag::PcheckDecay(genpart, -323, 321, -211) == 1) {
    return -1 * (100000 * nPhotos + 1069);
  }//D- decays to K*- K+ pi-
  if (GenDTag::PcheckDecay(genpart, -321, 323, -211) == 1) {
    return -1 * (100000 * nPhotos + 1070);
  }//D- decays to K- K*+ pi-
  if (GenDTag::PcheckDecay(genpart, -323, 311, 111) == 1) {
    return -1 * (100000 * nPhotos + 1071);
  }//D- decays to K*- K0 pi0
  if (GenDTag::PcheckDecay(genpart, -321, 313, 111) == 1) {
    return -1 * (100000 * nPhotos + 1072);
  }//D- decays to K- K*0 pi0
  if (GenDTag::PcheckDecay(genpart, 313, -311, -211) == 1) {
    return -1 * (100000 * nPhotos + 1073);
  }//D- decays to K*0 anti-K0 pi-
  if (GenDTag::PcheckDecay(genpart, 311, -313, -211) == 1) {
    return -1 * (100000 * nPhotos + 1074);
  }//D- decays to K0 anti-K*0 pi-
  if (GenDTag::PcheckDecay(genpart, 111, -211) == 1) {
    return -1 * (100000 * nPhotos + 1075);
  }//D- decays to pi0 pi-
  if (GenDTag::PcheckDecay(genpart, 113, -211) == 1) {
    return -1 * (100000 * nPhotos + 1076);
  }//D- decays to rho0 pi-
  if (GenDTag::PcheckDecay(genpart, -211, 211, -211) == 1) {
    return -1 * (100000 * nPhotos + 1077);
  }//D- decays to pi- pi+ pi-
  if (GenDTag::PcheckDecay(genpart, -211, 111, 111) == 1) {
    return -1 * (100000 * nPhotos + 1078);
  }//D- decays to pi- pi0 pi0
  if (GenDTag::PcheckDecay(genpart, -211, 211, -211, 111) == 1) {
    return -1 * (100000 * nPhotos + 1079);
  }//D- decays to pi- pi+ pi- pi0
  if (GenDTag::PcheckDecay(genpart, -211, 111, 111, 111) == 1) {
    return -1 * (100000 * nPhotos + 1080);
  }//D- decays to pi- pi0 pi0 pi0
  if (GenDTag::PcheckDecay(genpart, 221, -211) == 1) {
    return -1 * (100000 * nPhotos + 1081);
  }//D- decays to eta pi-
  if (GenDTag::PcheckDecay(genpart, 331, -211) == 1) {
    return -1 * (100000 * nPhotos + 1082);
  }//D- decays to eta' pi-
  if (GenDTag::PcheckDecay(genpart, 221, -211, 111) == 1) {
    return -1 * (100000 * nPhotos + 1083);
  }//D- decays to eta pi- pi0
  if (GenDTag::PcheckDecay(genpart, 221, -211, 211, -211) == 1) {
    return -1 * (100000 * nPhotos + 1084);
  }//D- decays to eta pi- pi+ pi-
  if (GenDTag::PcheckDecay(genpart, 221, -211, 111, 111) == 1) {
    return -1 * (100000 * nPhotos + 1085);
  }//D- decays to eta pi- pi0 pi0
  if (GenDTag::PcheckDecay(genpart, 211, -211, 211, -211, -211) == 1) {
    return -1 * (100000 * nPhotos + 1086);
  }//D- decays to pi+ pi- pi+ pi- pi-
  if (GenDTag::PcheckDecay(genpart, 313, -20213) == 1) {
    return -1 * (100000 * nPhotos + 1087);
  }//D- decays to K*0 a_1-
  if (GenDTag::PcheckDecay(genpart, 321, -321, -211, 111) == 1) {
    return -1 * (100000 * nPhotos + 1088);
  }//D- decays to K+ K- pi- pi0
  if (GenDTag::PcheckDecay(genpart, -321, 310, 211, -211) == 1) {
    return -1 * (100000 * nPhotos + 1089);
  }//D- decays to K- K_S0 pi+ pi-
  if (GenDTag::PcheckDecay(genpart, -321, 130, 211, -211) == 1) {
    return -1 * (100000 * nPhotos + 1090);
  }//D- decays to K- K_L0 pi+ pi-
  if (GenDTag::PcheckDecay(genpart, -321, 311, 211, -211) == 1) {
    return -1 * (100000 * nPhotos + 1091);
  }//D- decays to K- K0 pi+ pi-
  if (GenDTag::PcheckDecay(genpart, 310, 321, -211, -211) == 1) {
    return -1 * (100000 * nPhotos + 1092);
  }//D- decays to K_S0 K+ pi- pi-
  if (GenDTag::PcheckDecay(genpart, 130, 321, -211, -211) == 1) {
    return -1 * (100000 * nPhotos + 1093);
  }//D- decays to K_L0 K+ pi- pi-
  if (GenDTag::PcheckDecay(genpart, -311, 321, -211, -211) == 1) {
    return -1 * (100000 * nPhotos + 1094);
  }//D- decays to anti-K0 K+ pi- pi-
  if (GenDTag::PcheckDecay(genpart, 321, -321, 211, -211, -211) == 1) {
    return -1 * (100000 * nPhotos + 1095);
  }//D- decays to K+ K- pi+ pi- pi-
  if (GenDTag::PcheckDecay(genpart, 321, -321, 310, -211) == 1) {
    return -1 * (100000 * nPhotos + 1096);
  }//D- decays to K+ K- K_S0 pi-
  if (GenDTag::PcheckDecay(genpart, 321, -321, 130, -211) == 1) {
    return -1 * (100000 * nPhotos + 1097);
  }//D- decays to K+ K- K_L0 pi-
  if (GenDTag::PcheckDecay(genpart, 321, -321, 311, -211) == 1) {
    return -1 * (100000 * nPhotos + 1098);
  }//D- decays to K+ K- K0 pi-
  if (GenDTag::PcheckDecay(genpart, -321, 111) == 1) {
    return -1 * (100000 * nPhotos + 1099);
  }//D- decays to K- pi0
  if (GenDTag::PcheckDecay(genpart, -321, 211, -211) == 1) {
    return -1 * (100000 * nPhotos + 1100);
  }//D- decays to K- pi+ pi-
  if (GenDTag::PcheckDecay(genpart, -321, 321, -321) == 1) {
    return -1 * (100000 * nPhotos + 1101);
  }//D- decays to K- K+ K-
  if (GenDTag::PcheckDecay(genpart, 321, 113, -211, -211) == 1) {
    return -1 * (100000 * nPhotos + 1102);
  }//D- decays to K+ rho0 pi- pi-
  if (GenDTag::PcheckDecay(genpart, 331, -211, 111) == 1) {
    return -1 * (100000 * nPhotos + 1103);
  }//D- decays to eta' pi- pi0
  if (GenDTag::PcheckDecay(genpart, -321, 113) == 1) {
    return -1 * (100000 * nPhotos + 1104);
  }//D- decays to K- rho0
  return -1 * (DauSize(genpart));
}// Rest of the D- decays


int GenDTag::Mode_D0(std::vector<int>genpart)
{
  if (GenDTag::PcheckDecay(genpart, -323, -11, 12) == 1) {
    return +1 * (100000 * nPhotos + 1001);
  }//D0 decays to K*- e+ nu_e
  if (GenDTag::PcheckDecay(genpart, -321, -11, 12) == 1) {
    return +1 * (100000 * nPhotos + 1002);
  }//D0 decays to K- e+ nu_e
  if (GenDTag::PcheckDecay(genpart, -10323, -11, 12) == 1) {
    return +1 * (100000 * nPhotos + 1003);
  }//D0 decays to K_1- e+ nu_e
  if (GenDTag::PcheckDecay(genpart, -325, -11, 12) == 1) {
    return +1 * (100000 * nPhotos + 1004);
  }//D0 decays to K_2*- e+ nu_e
  if (GenDTag::PcheckDecay(genpart, -211, -11, 12) == 1) {
    return +1 * (100000 * nPhotos + 1005);
  }//D0 decays to pi- e+ nu_e
  if (GenDTag::PcheckDecay(genpart, -213, -11, 12) == 1) {
    return +1 * (100000 * nPhotos + 1006);
  }//D0 decays to rho- e+ nu_e
  if (GenDTag::PcheckDecay(genpart, -311, -211, -11, 12) == 1) {
    return +1 * (100000 * nPhotos + 1007);
  }//D0 decays to anti-K0 pi- e+ nu_e
  if (GenDTag::PcheckDecay(genpart, -321, 111, -11, 12) == 1) {
    return +1 * (100000 * nPhotos + 1008);
  }//D0 decays to K- pi0 e+ nu_e
  if (GenDTag::PcheckDecay(genpart, -323, -13, 14) == 1) {
    return +1 * (100000 * nPhotos + 1009);
  }//D0 decays to K*- mu+ nu_mu
  if (GenDTag::PcheckDecay(genpart, -321, -13, 14) == 1) {
    return +1 * (100000 * nPhotos + 1010);
  }//D0 decays to K- mu+ nu_mu
  if (GenDTag::PcheckDecay(genpart, -10323, -13, 14) == 1) {
    return +1 * (100000 * nPhotos + 1011);
  }//D0 decays to K_1- mu+ nu_mu
  if (GenDTag::PcheckDecay(genpart, -325, -13, 14) == 1) {
    return +1 * (100000 * nPhotos + 1012);
  }//D0 decays to K_2*- mu+ nu_mu
  if (GenDTag::PcheckDecay(genpart, -211, -13, 14) == 1) {
    return +1 * (100000 * nPhotos + 1013);
  }//D0 decays to pi- mu+ nu_mu
  if (GenDTag::PcheckDecay(genpart, -213, -13, 14) == 1) {
    return +1 * (100000 * nPhotos + 1014);
  }//D0 decays to rho- mu+ nu_mu
  if (GenDTag::PcheckDecay(genpart, -311, -211, -13, 14) == 1) {
    return +1 * (100000 * nPhotos + 1015);
  }//D0 decays to anti-K0 pi- mu+ nu_mu
  if (GenDTag::PcheckDecay(genpart, -321, 111, -13, 14) == 1) {
    return +1 * (100000 * nPhotos + 1016);
  }//D0 decays to K- pi0 mu+ nu_mu
  if (GenDTag::PcheckDecay(genpart, -321, 211) == 1) {
    return +1 * (100000 * nPhotos + 1017);
  }//D0 decays to K- pi+
  if (GenDTag::PcheckDecay(genpart, 310, 111) == 1) {
    return +1 * (100000 * nPhotos + 1018);
  }//D0 decays to K_S0 pi0
  if (GenDTag::PcheckDecay(genpart, 130, 111) == 1) {
    return +1 * (100000 * nPhotos + 1019);
  }//D0 decays to K_L0 pi0
  if (GenDTag::PcheckDecay(genpart, -311, 111) == 1) {
    return +1 * (100000 * nPhotos + 1020);
  }//D0 decays to anti-K0 pi0
  if (GenDTag::PcheckDecay(genpart, 310, 221) == 1) {
    return +1 * (100000 * nPhotos + 1021);
  }//D0 decays to K_S0 eta
  if (GenDTag::PcheckDecay(genpart, 130, 221) == 1) {
    return +1 * (100000 * nPhotos + 1022);
  }//D0 decays to K_L0 eta
  if (GenDTag::PcheckDecay(genpart, -311, 221) == 1) {
    return +1 * (100000 * nPhotos + 1023);
  }//D0 decays to anti-K0 eta
  if (GenDTag::PcheckDecay(genpart, 310, 331) == 1) {
    return +1 * (100000 * nPhotos + 1024);
  }//D0 decays to K_S0 eta'
  if (GenDTag::PcheckDecay(genpart, 130, 331) == 1) {
    return +1 * (100000 * nPhotos + 1025);
  }//D0 decays to K_L0 eta'
  if (GenDTag::PcheckDecay(genpart, -311, 331) == 1) {
    return +1 * (100000 * nPhotos + 1026);
  }//D0 decays to anti-K0 eta'
  if (GenDTag::PcheckDecay(genpart, 223, 310) == 1) {
    return +1 * (100000 * nPhotos + 1027);
  }//D0 decays to omega K_S0
  if (GenDTag::PcheckDecay(genpart, 223, 130) == 1) {
    return +1 * (100000 * nPhotos + 1028);
  }//D0 decays to omega K_L0
  if (GenDTag::PcheckDecay(genpart, 223, -311) == 1) {
    return +1 * (100000 * nPhotos + 1029);
  }//D0 decays to omega anti-K0
  if (GenDTag::PcheckDecay(genpart, -313, 221) == 1) {
    return +1 * (100000 * nPhotos + 1030);
  }//D0 decays to anti-K*0 eta
  if (GenDTag::PcheckDecay(genpart, -313, 331) == 1) {
    return +1 * (100000 * nPhotos + 1031);
  }//D0 decays to anti-K*0 eta'
  if (GenDTag::PcheckDecay(genpart, 20213, -321) == 1) {
    return +1 * (100000 * nPhotos + 1032);
  }//D0 decays to a_1+ K-
  if (GenDTag::PcheckDecay(genpart, -323, 213) == 1) {
    return +1 * (100000 * nPhotos + 1033);
  }//D0 decays to K*- rho+
  if (GenDTag::PcheckDecay(genpart, -313, 113) == 1) {
    return +1 * (100000 * nPhotos + 1034);
  }//D0 decays to anti-K*0 rho0
  if (GenDTag::PcheckDecay(genpart, -313, 223) == 1) {
    return +1 * (100000 * nPhotos + 1035);
  }//D0 decays to anti-K*0 omega
  if (GenDTag::PcheckDecay(genpart, -321, 211, 111) == 1) {
    return +1 * (100000 * nPhotos + 1036);
  }//D0 decays to K- pi+ pi0
  if (GenDTag::PcheckDecay(genpart, -313, 111) == 1) {
    return +1 * (100000 * nPhotos + 1037);
  }//D0 decays to K*BR pi0
  if (GenDTag::PcheckDecay(genpart, -10323, 211) == 1) {
    return +1 * (100000 * nPhotos + 1038);
  }//D0 decays to K_1- pi+
  if (GenDTag::PcheckDecay(genpart, -10313, 111) == 1) {
    return +1 * (100000 * nPhotos + 1039);
  }//D0 decays to anti-K_10 pi0
  if (GenDTag::PcheckDecay(genpart, 310, 211, -211) == 1) {
    return +1 * (100000 * nPhotos + 1040);
  }//D0 decays to K_S0 pi+ pi-
  if (GenDTag::PcheckDecay(genpart, 130, 211, -211) == 1) {
    return +1 * (100000 * nPhotos + 1041);
  }//D0 decays to K_L0 pi+ pi-
  if (GenDTag::PcheckDecay(genpart, -311, 211, -211) == 1) {
    return +1 * (100000 * nPhotos + 1042);
  }//D0 decays to anti-K0 pi+ pi-
  if (GenDTag::PcheckDecay(genpart, 310, 111, 111) == 1) {
    return +1 * (100000 * nPhotos + 1043);
  }//D0 decays to K_S0 pi0 pi0
  if (GenDTag::PcheckDecay(genpart, 130, 111, 111) == 1) {
    return +1 * (100000 * nPhotos + 1044);
  }//D0 decays to K_L0 pi0 pi0
  if (GenDTag::PcheckDecay(genpart, -311, 111, 111) == 1) {
    return +1 * (100000 * nPhotos + 1045);
  }//D0 decays to anti-K0 pi0 pi0
  if (GenDTag::PcheckDecay(genpart, -313, 211, -211) == 1) {
    return +1 * (100000 * nPhotos + 1046);
  }//D0 decays to anti-K*0 pi+ pi-
  if (GenDTag::PcheckDecay(genpart, -313, 111, 111) == 1) {
    return +1 * (100000 * nPhotos + 1047);
  }//D0 decays to anti-K*0 pi0 pi0
  if (GenDTag::PcheckDecay(genpart, -323, 211, 111) == 1) {
    return +1 * (100000 * nPhotos + 1048);
  }//D0 decays to K*- pi+ pi0
  if (GenDTag::PcheckDecay(genpart, -321, 213, 111) == 1) {
    return +1 * (100000 * nPhotos + 1049);
  }//D0 decays to K- rho+ pi0
  if (GenDTag::PcheckDecay(genpart, -321, 211, 113) == 1) {
    return +1 * (100000 * nPhotos + 1050);
  }//D0 decays to K- pi+ rho0
  if (GenDTag::PcheckDecay(genpart, -321, 211, 223) == 1) {
    return +1 * (100000 * nPhotos + 1051);
  }//D0 decays to K- pi+ omega
  if (GenDTag::PcheckDecay(genpart, -321, 211, 221) == 1) {
    return +1 * (100000 * nPhotos + 1052);
  }//D0 decays to K- pi+ eta
  if (GenDTag::PcheckDecay(genpart, -321, 211, 331) == 1) {
    return +1 * (100000 * nPhotos + 1053);
  }//D0 decays to K- pi+ eta'
  if (GenDTag::PcheckDecay(genpart, -321, 211, 211, -211) == 1) {
    return +1 * (100000 * nPhotos + 1054);
  }//D0 decays to K- pi+ pi+ pi-
  if (GenDTag::PcheckDecay(genpart, 310, 211, -211, 111) == 1) {
    return +1 * (100000 * nPhotos + 1055);
  }//D0 decays to K_S0 pi+ pi- pi0
  if (GenDTag::PcheckDecay(genpart, 130, 211, -211, 111) == 1) {
    return +1 * (100000 * nPhotos + 1056);
  }//D0 decays to K_L0 pi+ pi- pi0
  if (GenDTag::PcheckDecay(genpart, -311, 211, -211, 111) == 1) {
    return +1 * (100000 * nPhotos + 1057);
  }//D0 decays to anti-K0 pi+ pi- pi0
  if (GenDTag::PcheckDecay(genpart, -321, 211, 211, -211, 111) == 1) {
    return +1 * (100000 * nPhotos + 1058);
  }//D0 decays to K- pi+ pi+ pi- pi0
  if (GenDTag::PcheckDecay(genpart, -321, 211, 111, 111, 111) == 1) {
    return +1 * (100000 * nPhotos + 1059);
  }//D0 decays to K- pi+ pi0 pi0 pi0
  if (GenDTag::PcheckDecay(genpart, 310, 211, -211, 211, -211) == 1) {
    return +1 * (100000 * nPhotos + 1060);
  }//D0 decays to K_S0 pi+ pi- pi+ pi-
  if (GenDTag::PcheckDecay(genpart, 130, 211, -211, 211, -211) == 1) {
    return +1 * (100000 * nPhotos + 1061);
  }//D0 decays to K_L0 pi+ pi- pi+ pi-
  if (GenDTag::PcheckDecay(genpart, -311, 211, -211, 211, -211) == 1) {
    return +1 * (100000 * nPhotos + 1062);
  }//D0 decays to anti-K0 pi+ pi- pi+ pi-
  if (GenDTag::PcheckDecay(genpart, 333, 310) == 1) {
    return +1 * (100000 * nPhotos + 1063);
  }//D0 decays to phi K_S0
  if (GenDTag::PcheckDecay(genpart, 333, 130) == 1) {
    return +1 * (100000 * nPhotos + 1064);
  }//D0 decays to phi K_L0
  if (GenDTag::PcheckDecay(genpart, 333, -311) == 1) {
    return +1 * (100000 * nPhotos + 1065);
  }//D0 decays to phi anti-K0
  if (GenDTag::PcheckDecay(genpart, 310, 321, -321) == 1) {
    return +1 * (100000 * nPhotos + 1066);
  }//D0 decays to K_S0 K+ K-
  if (GenDTag::PcheckDecay(genpart, 130, 321, -321) == 1) {
    return +1 * (100000 * nPhotos + 1067);
  }//D0 decays to K_L0 K+ K-
  if (GenDTag::PcheckDecay(genpart, -311, 321, -321) == 1) {
    return +1 * (100000 * nPhotos + 1068);
  }//D0 decays to anti-K0 K+ K-
  if (GenDTag::PcheckDecay(genpart, 310, 310, 310) == 1) {
    return +1 * (100000 * nPhotos + 1069);
  }//D0 decays to K_S0 K_S0 K_S0
  if (GenDTag::PcheckDecay(genpart, 321, -321) == 1) {
    return +1 * (100000 * nPhotos + 1070);
  }//D0 decays to K+ K-
  if (GenDTag::PcheckDecay(genpart, 310, 310) == 1) {
    return +1 * (100000 * nPhotos + 1071);
  }//D0 decays to K_S0 K_S0
  if (GenDTag::PcheckDecay(genpart, 130, 130) == 1) {
    return +1 * (100000 * nPhotos + 1072);
  }//D0 decays to K_L0 K_L0
  if (GenDTag::PcheckDecay(genpart, 313, -311) == 1) {
    return +1 * (100000 * nPhotos + 1073);
  }//D0 decays to K*0 anti-K0
  if (GenDTag::PcheckDecay(genpart, -313, 310) == 1) {
    return +1 * (100000 * nPhotos + 1074);
  }//D0 decays to anti-K*0 K_S0
  if (GenDTag::PcheckDecay(genpart, -313, 130) == 1) {
    return +1 * (100000 * nPhotos + 1075);
  }//D0 decays to anti-K*0 K_L0
  if (GenDTag::PcheckDecay(genpart, -313, 311) == 1) {
    return +1 * (100000 * nPhotos + 1076);
  }//D0 decays to anti-K*0 K0
  if (GenDTag::PcheckDecay(genpart, -323, 321) == 1) {
    return +1 * (100000 * nPhotos + 1077);
  }//D0 decays to K*- K+
  if (GenDTag::PcheckDecay(genpart, 323, -321) == 1) {
    return +1 * (100000 * nPhotos + 1078);
  }//D0 decays to K*+ K-
  if (GenDTag::PcheckDecay(genpart, -313, 313) == 1) {
    return +1 * (100000 * nPhotos + 1079);
  }//D0 decays to anti-K*0 K*0
  if (GenDTag::PcheckDecay(genpart, 333, 111) == 1) {
    return +1 * (100000 * nPhotos + 1080);
  }//D0 decays to phi pi0
  if (GenDTag::PcheckDecay(genpart, 333, 211, -211) == 1) {
    return +1 * (100000 * nPhotos + 1081);
  }//D0 decays to phi pi+ pi-
  if (GenDTag::PcheckDecay(genpart, 321, -321, 211, -211) == 1) {
    return +1 * (100000 * nPhotos + 1082);
  }//D0 decays to K+ K- pi+ pi-
  if (GenDTag::PcheckDecay(genpart, 321, -321, 111, 111) == 1) {
    return +1 * (100000 * nPhotos + 1083);
  }//D0 decays to K+ K- pi0 pi0
  if (GenDTag::PcheckDecay(genpart, 310, 310, 211, -211) == 1) {
    return +1 * (100000 * nPhotos + 1084);
  }//D0 decays to K_S0 K_S0 pi+ pi-
  if (GenDTag::PcheckDecay(genpart, 130, 130, 211, -211) == 1) {
    return +1 * (100000 * nPhotos + 1085);
  }//D0 decays to K_L0 K_L0 pi+ pi-
  if (GenDTag::PcheckDecay(genpart, -311, 311, 211, -211) == 1) {
    return +1 * (100000 * nPhotos + 1086);
  }//D0 decays to anti-K0 K0 pi+ pi-
  if (GenDTag::PcheckDecay(genpart, -311, 311, 111, 111) == 1) {
    return +1 * (100000 * nPhotos + 1087);
  }//D0 decays to anti-K0 K0 pi0 pi0
  if (GenDTag::PcheckDecay(genpart, 211, -211) == 1) {
    return +1 * (100000 * nPhotos + 1088);
  }//D0 decays to pi+ pi-
  if (GenDTag::PcheckDecay(genpart, 111, 111) == 1) {
    return +1 * (100000 * nPhotos + 1089);
  }//D0 decays to pi0 pi0
  if (GenDTag::PcheckDecay(genpart, 221, 111) == 1) {
    return +1 * (100000 * nPhotos + 1090);
  }//D0 decays to eta pi0
  if (GenDTag::PcheckDecay(genpart, 331, 111) == 1) {
    return +1 * (100000 * nPhotos + 1091);
  }//D0 decays to eta' pi0
  if (GenDTag::PcheckDecay(genpart, 221, 221) == 1) {
    return +1 * (100000 * nPhotos + 1092);
  }//D0 decays to eta eta
  if (GenDTag::PcheckDecay(genpart, 213, -211) == 1) {
    return +1 * (100000 * nPhotos + 1093);
  }//D0 decays to rho+ pi-
  if (GenDTag::PcheckDecay(genpart, -213, 211) == 1) {
    return +1 * (100000 * nPhotos + 1094);
  }//D0 decays to rho- pi+
  if (GenDTag::PcheckDecay(genpart, 113, 111) == 1) {
    return +1 * (100000 * nPhotos + 1095);
  }//D0 decays to rho0 pi0
  if (GenDTag::PcheckDecay(genpart, 211, -211, 111) == 1) {
    return +1 * (100000 * nPhotos + 1096);
  }//D0 decays to pi+ pi- pi0
  if (GenDTag::PcheckDecay(genpart, 111, 111, 111) == 1) {
    return +1 * (100000 * nPhotos + 1097);
  }//D0 decays to pi0 pi0 pi0
  if (GenDTag::PcheckDecay(genpart, 211, 211, -211, -211) == 1) {
    return +1 * (100000 * nPhotos + 1098);
  }//D0 decays to pi+ pi+ pi- pi-
  if (GenDTag::PcheckDecay(genpart, 211, -211, 111, 111) == 1) {
    return +1 * (100000 * nPhotos + 1099);
  }//D0 decays to pi+ pi- pi0 pi0
  if (GenDTag::PcheckDecay(genpart, 211, -211, 211, -211, 111) == 1) {
    return +1 * (100000 * nPhotos + 1100);
  }//D0 decays to pi+ pi- pi+ pi- pi0
  if (GenDTag::PcheckDecay(genpart, 211, -211, 111, 111, 111) == 1) {
    return +1 * (100000 * nPhotos + 1101);
  }//D0 decays to pi+ pi- pi0 pi0 pi0
  if (GenDTag::PcheckDecay(genpart, 211, -211, 211, -211, 211, -211) == 1) {
    return +1 * (100000 * nPhotos + 1102);
  }//D0 decays to pi+ pi- pi+ pi- pi+ pi-
  if (GenDTag::PcheckDecay(genpart, -211, 321) == 1) {
    return +1 * (100000 * nPhotos + 1103);
  }//D0 decays to pi- K+
  if (GenDTag::PcheckDecay(genpart, -211, 323) == 1) {
    return +1 * (100000 * nPhotos + 1104);
  }//D0 decays to pi- K*+
  if (GenDTag::PcheckDecay(genpart, -211, 321, 111) == 1) {
    return +1 * (100000 * nPhotos + 1105);
  }//D0 decays to pi- K+ pi0
  if (GenDTag::PcheckDecay(genpart, 321, -211, 211, -211) == 1) {
    return +1 * (100000 * nPhotos + 1106);
  }//D0 decays to K+ pi- pi+ pi-
  if (GenDTag::PcheckDecay(genpart, -13, 13) == 1) {
    return +1 * (100000 * nPhotos + 1107);
  }//D0 decays to mu+ mu-
  if (GenDTag::PcheckDecay(genpart, 333, 221) == 1) {
    return +1 * (100000 * nPhotos + 1108);
  }//D0 decays to phi eta
  if (GenDTag::PcheckDecay(genpart, -313, 211, -211, 111) == 1) {
    return +1 * (100000 * nPhotos + 1109);
  }//D0 decays to anti-K*0 pi+ pi- pi0
  if (GenDTag::PcheckDecay(genpart, -321, 211, -211, 211, -211, 211) == 1) {
    return +1 * (100000 * nPhotos + 1110);
  }//D0 decays to K- pi+ pi- pi+ pi- pi+
  if (GenDTag::PcheckDecay(genpart, 321, -321, 111) == 1) {
    return +1 * (100000 * nPhotos + 1111);
  }//D0 decays to K+ K- pi0
  if (GenDTag::PcheckDecay(genpart, 321, -321, 211, -211, 111) == 1) {
    return +1 * (100000 * nPhotos + 1112);
  }//D0 decays to K+ K- pi+ pi- pi0
  if (GenDTag::PcheckDecay(genpart, 321, -321, -321, 211) == 1) {
    return +1 * (100000 * nPhotos + 1113);
  }//D0 decays to K+ K- K- pi+
  if (GenDTag::PcheckDecay(genpart, 310, 221, 111) == 1) {
    return +1 * (100000 * nPhotos + 1114);
  }//D0 decays to K_S0 eta pi0
  if (GenDTag::PcheckDecay(genpart, 130, 221, 111) == 1) {
    return +1 * (100000 * nPhotos + 1115);
  }//D0 decays to K_L0 eta pi0
  if (GenDTag::PcheckDecay(genpart, -311, 221, 111) == 1) {
    return +1 * (100000 * nPhotos + 1116);
  }//D0 decays to anti-K0 eta pi0
  if (GenDTag::PcheckDecay(genpart, 310, 321, -211) == 1) {
    return +1 * (100000 * nPhotos + 1117);
  }//D0 decays to K_S0 K+ pi-
  if (GenDTag::PcheckDecay(genpart, 130, 321, -211) == 1) {
    return +1 * (100000 * nPhotos + 1118);
  }//D0 decays to K_L0 K+ pi-
  if (GenDTag::PcheckDecay(genpart, -311, 321, -211) == 1) {
    return +1 * (100000 * nPhotos + 1119);
  }//D0 decays to anti-K0 K+ pi-
  if (GenDTag::PcheckDecay(genpart, 310, -321, 211) == 1) {
    return +1 * (100000 * nPhotos + 1120);
  }//D0 decays to K_S0 K- pi+
  if (GenDTag::PcheckDecay(genpart, 130, -321, 211) == 1) {
    return +1 * (100000 * nPhotos + 1121);
  }//D0 decays to K_L0 K- pi+
  if (GenDTag::PcheckDecay(genpart, 311, -321, 211) == 1) {
    return +1 * (100000 * nPhotos + 1122);
  }//D0 decays to K0 K- pi+
  if (GenDTag::PcheckDecay(genpart, 310, 310, 321, -211) == 1) {
    return +1 * (100000 * nPhotos + 1123);
  }//D0 decays to K_S0 K_S0 K+ pi-
  if (GenDTag::PcheckDecay(genpart, 130, 130, 321, -211) == 1) {
    return +1 * (100000 * nPhotos + 1124);
  }//D0 decays to K_L0 K_L0 K+ pi-
  if (GenDTag::PcheckDecay(genpart, -311, 311, 321, -211) == 1) {
    return +1 * (100000 * nPhotos + 1125);
  }//D0 decays to anti-K0 K0 K+ pi-
  if (GenDTag::PcheckDecay(genpart, 310, 310, -321, 211) == 1) {
    return +1 * (100000 * nPhotos + 1126);
  }//D0 decays to K_S0 K_S0 K- pi+
  if (GenDTag::PcheckDecay(genpart, 130, 130, -321, 211) == 1) {
    return +1 * (100000 * nPhotos + 1127);
  }//D0 decays to K_L0 K_L0 K- pi+
  if (GenDTag::PcheckDecay(genpart, -311, 311, -321, 211) == 1) {
    return +1 * (100000 * nPhotos + 1128);
  }//D0 decays to anti-K0 K0 K- pi+
  if (GenDTag::PcheckDecay(genpart, -321, 211, -211, -11, 12) == 1) {
    return +1 * (100000 * nPhotos + 1129);
  }//D0 decays to K- pi+ pi- e+ nu_e
  if (GenDTag::PcheckDecay(genpart, 113, 113) == 1) {
    return +1 * (100000 * nPhotos + 1130);
  }//D0 decays to rho0 rho0
  if (GenDTag::PcheckDecay(genpart, 221, 211, -211) == 1) {
    return +1 * (100000 * nPhotos + 1131);
  }//D0 decays to eta pi+ pi-
  if (GenDTag::PcheckDecay(genpart, 223, 211, -211) == 1) {
    return +1 * (100000 * nPhotos + 1132);
  }//D0 decays to omega pi+ pi-
  if (GenDTag::PcheckDecay(genpart, 331, 211, -211) == 1) {
    return +1 * (100000 * nPhotos + 1133);
  }//D0 decays to eta' pi+ pi-
  if (GenDTag::PcheckDecay(genpart, 221, 331) == 1) {
    return +1 * (100000 * nPhotos + 1134);
  }//D0 decays to eta eta'
  if (GenDTag::PcheckDecay(genpart, 333, 22) == 1) {
    return +1 * (100000 * nPhotos + 1135);
  }//D0 decays to phi gamma
  if (GenDTag::PcheckDecay(genpart, -313, 22) == 1) {
    return +1 * (100000 * nPhotos + 1136);
  }//D0 decays to anti-K*0 gamma
  return +1 * (DauSize(genpart));
}// Rest of the D0 decays


int GenDTag::Mode_anti_D0(std::vector<int>genpart)
{
  if (GenDTag::PcheckDecay(genpart, 323, 11, -12) == 1) {
    return -1 * (100000 * nPhotos + 1001);
  }//anti-D0 decays to K*+ e- anti-nu_e
  if (GenDTag::PcheckDecay(genpart, 321, 11, -12) == 1) {
    return -1 * (100000 * nPhotos + 1002);
  }//anti-D0 decays to K+ e- anti-nu_e
  if (GenDTag::PcheckDecay(genpart, 10323, 11, -12) == 1) {
    return -1 * (100000 * nPhotos + 1003);
  }//anti-D0 decays to K_1+ e- anti-nu_e
  if (GenDTag::PcheckDecay(genpart, 325, 11, -12) == 1) {
    return -1 * (100000 * nPhotos + 1004);
  }//anti-D0 decays to K_2*+ e- anti-nu_e
  if (GenDTag::PcheckDecay(genpart, 211, 11, -12) == 1) {
    return -1 * (100000 * nPhotos + 1005);
  }//anti-D0 decays to pi+ e- anti-nu_e
  if (GenDTag::PcheckDecay(genpart, 213, 11, -12) == 1) {
    return -1 * (100000 * nPhotos + 1006);
  }//anti-D0 decays to rho+ e- anti-nu_e
  if (GenDTag::PcheckDecay(genpart, 311, 211, 11, -12) == 1) {
    return -1 * (100000 * nPhotos + 1007);
  }//anti-D0 decays to K0 pi+ e- anti-nu_e
  if (GenDTag::PcheckDecay(genpart, 321, 111, 11, -12) == 1) {
    return -1 * (100000 * nPhotos + 1008);
  }//anti-D0 decays to K+ pi0 e- anti-nu_e
  if (GenDTag::PcheckDecay(genpart, 323, 13, -14) == 1) {
    return -1 * (100000 * nPhotos + 1009);
  }//anti-D0 decays to K*+ mu- anti-nu_mu
  if (GenDTag::PcheckDecay(genpart, 321, 13, -14) == 1) {
    return -1 * (100000 * nPhotos + 1010);
  }//anti-D0 decays to K+ mu- anti-nu_mu
  if (GenDTag::PcheckDecay(genpart, 10323, 13, -14) == 1) {
    return -1 * (100000 * nPhotos + 1011);
  }//anti-D0 decays to K_1+ mu- anti-nu_mu
  if (GenDTag::PcheckDecay(genpart, 325, 13, -14) == 1) {
    return -1 * (100000 * nPhotos + 1012);
  }//anti-D0 decays to K_2*+ mu- anti-nu_mu
  if (GenDTag::PcheckDecay(genpart, 211, 13, -14) == 1) {
    return -1 * (100000 * nPhotos + 1013);
  }//anti-D0 decays to pi+ mu- anti-nu_mu
  if (GenDTag::PcheckDecay(genpart, 213, 13, -14) == 1) {
    return -1 * (100000 * nPhotos + 1014);
  }//anti-D0 decays to rho+ mu- anti-nu_mu
  if (GenDTag::PcheckDecay(genpart, 311, 211, 13, -14) == 1) {
    return -1 * (100000 * nPhotos + 1015);
  }//anti-D0 decays to K0 pi+ mu- anti-nu_mu
  if (GenDTag::PcheckDecay(genpart, 321, 111, 13, -14) == 1) {
    return -1 * (100000 * nPhotos + 1016);
  }//anti-D0 decays to K+ pi0 mu- anti-nu_mu
  if (GenDTag::PcheckDecay(genpart, 321, -211) == 1) {
    return -1 * (100000 * nPhotos + 1017);
  }//anti-D0 decays to K+ pi-
  if (GenDTag::PcheckDecay(genpart, 310, 111) == 1) {
    return -1 * (100000 * nPhotos + 1018);
  }//anti-D0 decays to K_S0 pi0
  if (GenDTag::PcheckDecay(genpart, 130, 111) == 1) {
    return -1 * (100000 * nPhotos + 1019);
  }//anti-D0 decays to K_L0 pi0
  if (GenDTag::PcheckDecay(genpart, 311, 111) == 1) {
    return -1 * (100000 * nPhotos + 1020);
  }//anti-D0 decays to K0 pi0
  if (GenDTag::PcheckDecay(genpart, 310, 221) == 1) {
    return -1 * (100000 * nPhotos + 1021);
  }//anti-D0 decays to K_S0 eta
  if (GenDTag::PcheckDecay(genpart, 130, 221) == 1) {
    return -1 * (100000 * nPhotos + 1022);
  }//anti-D0 decays to K_L0 eta
  if (GenDTag::PcheckDecay(genpart, 311, 221) == 1) {
    return -1 * (100000 * nPhotos + 1023);
  }//anti-D0 decays to K0 eta
  if (GenDTag::PcheckDecay(genpart, 310, 331) == 1) {
    return -1 * (100000 * nPhotos + 1024);
  }//anti-D0 decays to K_S0 eta'
  if (GenDTag::PcheckDecay(genpart, 130, 331) == 1) {
    return -1 * (100000 * nPhotos + 1025);
  }//anti-D0 decays to K_L0 eta'
  if (GenDTag::PcheckDecay(genpart, 311, 331) == 1) {
    return -1 * (100000 * nPhotos + 1026);
  }//anti-D0 decays to K0 eta'
  if (GenDTag::PcheckDecay(genpart, 223, 310) == 1) {
    return -1 * (100000 * nPhotos + 1027);
  }//anti-D0 decays to omega K_S0
  if (GenDTag::PcheckDecay(genpart, 223, 130) == 1) {
    return -1 * (100000 * nPhotos + 1028);
  }//anti-D0 decays to omega K_L0
  if (GenDTag::PcheckDecay(genpart, 223, 311) == 1) {
    return -1 * (100000 * nPhotos + 1029);
  }//anti-D0 decays to omega K0
  if (GenDTag::PcheckDecay(genpart, 313, 221) == 1) {
    return -1 * (100000 * nPhotos + 1030);
  }//anti-D0 decays to K*0 eta
  if (GenDTag::PcheckDecay(genpart, 313, 331) == 1) {
    return -1 * (100000 * nPhotos + 1031);
  }//anti-D0 decays to K*0 eta'
  if (GenDTag::PcheckDecay(genpart, -20213, 321) == 1) {
    return -1 * (100000 * nPhotos + 1032);
  }//anti-D0 decays to a_1- K+
  if (GenDTag::PcheckDecay(genpart, 323, -213) == 1) {
    return -1 * (100000 * nPhotos + 1033);
  }//anti-D0 decays to K*+ rho-
  if (GenDTag::PcheckDecay(genpart, 313, 113) == 1) {
    return -1 * (100000 * nPhotos + 1034);
  }//anti-D0 decays to K*0 rho0
  if (GenDTag::PcheckDecay(genpart, 313, 223) == 1) {
    return -1 * (100000 * nPhotos + 1035);
  }//anti-D0 decays to K*0 omega
  if (GenDTag::PcheckDecay(genpart, 321, -211, 111) == 1) {
    return -1 * (100000 * nPhotos + 1036);
  }//anti-D0 decays to K+ pi- pi0
  if (GenDTag::PcheckDecay(genpart, 313, 111) == 1) {
    return -1 * (100000 * nPhotos + 1037);
  }//anti-D0 decays to K*0R pi0
  if (GenDTag::PcheckDecay(genpart, 10323, -211) == 1) {
    return -1 * (100000 * nPhotos + 1038);
  }//anti-D0 decays to K_1+ pi-
  if (GenDTag::PcheckDecay(genpart, 10313, 111) == 1) {
    return -1 * (100000 * nPhotos + 1039);
  }//anti-D0 decays to K_10 pi0
  if (GenDTag::PcheckDecay(genpart, 311, 211, -211) == 1) {
    return -1 * (100000 * nPhotos + 1040);
  }//anti-D0 decays to K0 pi+ pi-
  if (GenDTag::PcheckDecay(genpart, 310, 211, -211) == 1) {
    return -1 * (100000 * nPhotos + 1041);
  }//anti-D0 decays to K_S0 pi+ pi-
  if (GenDTag::PcheckDecay(genpart, 130, 211, -211) == 1) {
    return -1 * (100000 * nPhotos + 1042);
  }//anti-D0 decays to K_L0 pi+ pi-
  if (GenDTag::PcheckDecay(genpart, 310, 111, 111) == 1) {
    return -1 * (100000 * nPhotos + 1043);
  }//anti-D0 decays to K_S0 pi0 pi0
  if (GenDTag::PcheckDecay(genpart, 130, 111, 111) == 1) {
    return -1 * (100000 * nPhotos + 1044);
  }//anti-D0 decays to K_L0 pi0 pi0
  if (GenDTag::PcheckDecay(genpart, 311, 111, 111) == 1) {
    return -1 * (100000 * nPhotos + 1045);
  }//anti-D0 decays to K0 pi0 pi0
  if (GenDTag::PcheckDecay(genpart, 313, 211, -211) == 1) {
    return -1 * (100000 * nPhotos + 1046);
  }//anti-D0 decays to K*0 pi+ pi-
  if (GenDTag::PcheckDecay(genpart, 313, 111, 111) == 1) {
    return -1 * (100000 * nPhotos + 1047);
  }//anti-D0 decays to K*0 pi0 pi0
  if (GenDTag::PcheckDecay(genpart, 323, -211, 111) == 1) {
    return -1 * (100000 * nPhotos + 1048);
  }//anti-D0 decays to K*+ pi- pi0
  if (GenDTag::PcheckDecay(genpart, 321, -213, 111) == 1) {
    return -1 * (100000 * nPhotos + 1049);
  }//anti-D0 decays to K+ rho- pi0
  if (GenDTag::PcheckDecay(genpart, 321, -211, 113) == 1) {
    return -1 * (100000 * nPhotos + 1050);
  }//anti-D0 decays to K+ pi- rho0
  if (GenDTag::PcheckDecay(genpart, 321, -211, 223) == 1) {
    return -1 * (100000 * nPhotos + 1051);
  }//anti-D0 decays to K+ pi- omega
  if (GenDTag::PcheckDecay(genpart, 321, -211, 221) == 1) {
    return -1 * (100000 * nPhotos + 1052);
  }//anti-D0 decays to K+ pi- eta
  if (GenDTag::PcheckDecay(genpart, 321, -211, 331) == 1) {
    return -1 * (100000 * nPhotos + 1053);
  }//anti-D0 decays to K+ pi- eta'
  if (GenDTag::PcheckDecay(genpart, 321, -211, 211, -211) == 1) {
    return -1 * (100000 * nPhotos + 1054);
  }//anti-D0 decays to K+ pi- pi+ pi-
  if (GenDTag::PcheckDecay(genpart, 310, 211, -211, 111) == 1) {
    return -1 * (100000 * nPhotos + 1055);
  }//anti-D0 decays to K_S0 pi+ pi- pi0
  if (GenDTag::PcheckDecay(genpart, 130, 211, -211, 111) == 1) {
    return -1 * (100000 * nPhotos + 1056);
  }//anti-D0 decays to K_L0 pi+ pi- pi0
  if (GenDTag::PcheckDecay(genpart, 311, 211, -211, 111) == 1) {
    return -1 * (100000 * nPhotos + 1057);
  }//anti-D0 decays to K0 pi+ pi- pi0
  if (GenDTag::PcheckDecay(genpart, 321, -211, 211, -211, 111) == 1) {
    return -1 * (100000 * nPhotos + 1058);
  }//anti-D0 decays to K+ pi- pi+ pi- pi0
  if (GenDTag::PcheckDecay(genpart, 321, -211, 111, 111, 111) == 1) {
    return -1 * (100000 * nPhotos + 1059);
  }//anti-D0 decays to K+ pi- pi0 pi0 pi0
  if (GenDTag::PcheckDecay(genpart, 310, 211, -211, 211, -211) == 1) {
    return -1 * (100000 * nPhotos + 1060);
  }//anti-D0 decays to K_S0 pi+ pi- pi+ pi-
  if (GenDTag::PcheckDecay(genpart, 130, 211, -211, 211, -211) == 1) {
    return -1 * (100000 * nPhotos + 1061);
  }//anti-D0 decays to K_L0 pi+ pi- pi+ pi-
  if (GenDTag::PcheckDecay(genpart, 311, 211, -211, 211, -211) == 1) {
    return -1 * (100000 * nPhotos + 1062);
  }//anti-D0 decays to K0 pi+ pi- pi+ pi-
  if (GenDTag::PcheckDecay(genpart, 333, 310) == 1) {
    return -1 * (100000 * nPhotos + 1063);
  }//anti-D0 decays to phi K_S0
  if (GenDTag::PcheckDecay(genpart, 333, 130) == 1) {
    return -1 * (100000 * nPhotos + 1064);
  }//anti-D0 decays to phi K_L0
  if (GenDTag::PcheckDecay(genpart, 333, 311) == 1) {
    return -1 * (100000 * nPhotos + 1065);
  }//anti-D0 decays to phi K0
  if (GenDTag::PcheckDecay(genpart, 310, 321, -321) == 1) {
    return -1 * (100000 * nPhotos + 1066);
  }//anti-D0 decays to K_S0 K+ K-
  if (GenDTag::PcheckDecay(genpart, 130, 321, -321) == 1) {
    return -1 * (100000 * nPhotos + 1067);
  }//anti-D0 decays to K_L0 K+ K-
  if (GenDTag::PcheckDecay(genpart, 311, 321, -321) == 1) {
    return -1 * (100000 * nPhotos + 1068);
  }//anti-D0 decays to K0 K+ K-
  if (GenDTag::PcheckDecay(genpart, 310, 310, 310) == 1) {
    return -1 * (100000 * nPhotos + 1069);
  }//anti-D0 decays to K_S0 K_S0 K_S0
  if (GenDTag::PcheckDecay(genpart, 321, -321) == 1) {
    return -1 * (100000 * nPhotos + 1070);
  }//anti-D0 decays to K+ K-
  if (GenDTag::PcheckDecay(genpart, 310, 310) == 1) {
    return -1 * (100000 * nPhotos + 1071);
  }//anti-D0 decays to K_S0 K_S0
  if (GenDTag::PcheckDecay(genpart, 130, 130) == 1) {
    return -1 * (100000 * nPhotos + 1072);
  }//anti-D0 decays to K_L0 K_L0
  if (GenDTag::PcheckDecay(genpart, -313, 311) == 1) {
    return -1 * (100000 * nPhotos + 1073);
  }//anti-D0 decays to anti-K*0 K0
  if (GenDTag::PcheckDecay(genpart, 313, 310) == 1) {
    return -1 * (100000 * nPhotos + 1074);
  }//anti-D0 decays to K*0 K_S0
  if (GenDTag::PcheckDecay(genpart, 313, 130) == 1) {
    return -1 * (100000 * nPhotos + 1075);
  }//anti-D0 decays to K*0 K_L0
  if (GenDTag::PcheckDecay(genpart, 313, -311) == 1) {
    return -1 * (100000 * nPhotos + 1076);
  }//anti-D0 decays to K*0 anti-K0
  if (GenDTag::PcheckDecay(genpart, 323, -321) == 1) {
    return -1 * (100000 * nPhotos + 1077);
  }//anti-D0 decays to K*+ K-
  if (GenDTag::PcheckDecay(genpart, -323, 321) == 1) {
    return -1 * (100000 * nPhotos + 1078);
  }//anti-D0 decays to K*- K+
  if (GenDTag::PcheckDecay(genpart, 313, -313) == 1) {
    return -1 * (100000 * nPhotos + 1079);
  }//anti-D0 decays to K*0 anti-K*0
  if (GenDTag::PcheckDecay(genpart, 333, 111) == 1) {
    return -1 * (100000 * nPhotos + 1080);
  }//anti-D0 decays to phi pi0
  if (GenDTag::PcheckDecay(genpart, 333, 211, -211) == 1) {
    return -1 * (100000 * nPhotos + 1081);
  }//anti-D0 decays to phi pi+ pi-
  if (GenDTag::PcheckDecay(genpart, 321, -321, 211, -211) == 1) {
    return -1 * (100000 * nPhotos + 1082);
  }//anti-D0 decays to K+ K- pi+ pi-
  if (GenDTag::PcheckDecay(genpart, 321, -321, 111, 111) == 1) {
    return -1 * (100000 * nPhotos + 1083);
  }//anti-D0 decays to K+ K- pi0 pi0
  if (GenDTag::PcheckDecay(genpart, 310, 310, 211, -211) == 1) {
    return -1 * (100000 * nPhotos + 1084);
  }//anti-D0 decays to K_S0 K_S0 pi+ pi-
  if (GenDTag::PcheckDecay(genpart, 130, 130, 211, -211) == 1) {
    return -1 * (100000 * nPhotos + 1085);
  }//anti-D0 decays to K_L0 K_L0 pi+ pi-
  if (GenDTag::PcheckDecay(genpart, -311, 311, 211, -211) == 1) {
    return -1 * (100000 * nPhotos + 1086);
  }//anti-D0 decays to anti-K0 K0 pi+ pi-
  if (GenDTag::PcheckDecay(genpart, -311, 311, 111, 111) == 1) {
    return -1 * (100000 * nPhotos + 1087);
  }//anti-D0 decays to anti-K0 K0 pi0 pi0
  if (GenDTag::PcheckDecay(genpart, 211, -211) == 1) {
    return -1 * (100000 * nPhotos + 1088);
  }//anti-D0 decays to pi+ pi-
  if (GenDTag::PcheckDecay(genpart, 111, 111) == 1) {
    return -1 * (100000 * nPhotos + 1089);
  }//anti-D0 decays to pi0 pi0
  if (GenDTag::PcheckDecay(genpart, 221, 111) == 1) {
    return -1 * (100000 * nPhotos + 1090);
  }//anti-D0 decays to eta pi0
  if (GenDTag::PcheckDecay(genpart, 331, 111) == 1) {
    return -1 * (100000 * nPhotos + 1091);
  }//anti-D0 decays to eta' pi0
  if (GenDTag::PcheckDecay(genpart, 221, 221) == 1) {
    return -1 * (100000 * nPhotos + 1092);
  }//anti-D0 decays to eta eta
  if (GenDTag::PcheckDecay(genpart, 213, -211) == 1) {
    return -1 * (100000 * nPhotos + 1093);
  }//anti-D0 decays to rho+ pi-
  if (GenDTag::PcheckDecay(genpart, -213, 211) == 1) {
    return -1 * (100000 * nPhotos + 1094);
  }//anti-D0 decays to rho- pi+
  if (GenDTag::PcheckDecay(genpart, 113, 111) == 1) {
    return -1 * (100000 * nPhotos + 1095);
  }//anti-D0 decays to rho0 pi0
  if (GenDTag::PcheckDecay(genpart, 211, -211, 111) == 1) {
    return -1 * (100000 * nPhotos + 1096);
  }//anti-D0 decays to pi+ pi- pi0
  if (GenDTag::PcheckDecay(genpart, 111, 111, 111) == 1) {
    return -1 * (100000 * nPhotos + 1097);
  }//anti-D0 decays to pi0 pi0 pi0
  if (GenDTag::PcheckDecay(genpart, 211, 211, -211, -211) == 1) {
    return -1 * (100000 * nPhotos + 1098);
  }//anti-D0 decays to pi+ pi+ pi- pi-
  if (GenDTag::PcheckDecay(genpart, 211, -211, 111, 111) == 1) {
    return -1 * (100000 * nPhotos + 1099);
  }//anti-D0 decays to pi+ pi- pi0 pi0
  if (GenDTag::PcheckDecay(genpart, 211, -211, 211, -211, 111) == 1) {
    return -1 * (100000 * nPhotos + 1100);
  }//anti-D0 decays to pi+ pi- pi+ pi- pi0
  if (GenDTag::PcheckDecay(genpart, 211, -211, 111, 111, 111) == 1) {
    return -1 * (100000 * nPhotos + 1101);
  }//anti-D0 decays to pi+ pi- pi0 pi0 pi0
  if (GenDTag::PcheckDecay(genpart, 211, -211, 211, -211, 211, -211) == 1) {
    return -1 * (100000 * nPhotos + 1102);
  }//anti-D0 decays to pi+ pi- pi+ pi- pi+ pi-
  if (GenDTag::PcheckDecay(genpart, 211, -321) == 1) {
    return -1 * (100000 * nPhotos + 1103);
  }//anti-D0 decays to pi+ K-
  if (GenDTag::PcheckDecay(genpart, 211, -323) == 1) {
    return -1 * (100000 * nPhotos + 1104);
  }//anti-D0 decays to pi+ K*-
  if (GenDTag::PcheckDecay(genpart, 211, -321, 111) == 1) {
    return -1 * (100000 * nPhotos + 1105);
  }//anti-D0 decays to pi+ K- pi0
  if (GenDTag::PcheckDecay(genpart, -321, 211, 211, -211) == 1) {
    return -1 * (100000 * nPhotos + 1106);
  }//anti-D0 decays to K- pi+ pi+ pi-
  if (GenDTag::PcheckDecay(genpart, 13, -13) == 1) {
    return -1 * (100000 * nPhotos + 1107);
  }//anti-D0 decays to mu- mu+
  if (GenDTag::PcheckDecay(genpart, 333, 221) == 1) {
    return -1 * (100000 * nPhotos + 1108);
  }//anti-D0 decays to phi eta
  if (GenDTag::PcheckDecay(genpart, 313, 211, -211, 111) == 1) {
    return -1 * (100000 * nPhotos + 1109);
  }//anti-D0 decays to K*0 pi+ pi- pi0
  if (GenDTag::PcheckDecay(genpart, 321, -211, -211, 211, -211, 211) == 1) {
    return -1 * (100000 * nPhotos + 1110);
  }//anti-D0 decays to K+ pi- pi- pi+ pi- pi+
  if (GenDTag::PcheckDecay(genpart, 321, -321, 111) == 1) {
    return -1 * (100000 * nPhotos + 1111);
  }//anti-D0 decays to K+ K- pi0
  if (GenDTag::PcheckDecay(genpart, 321, -321, 211, -211, 111) == 1) {
    return -1 * (100000 * nPhotos + 1112);
  }//anti-D0 decays to K+ K- pi+ pi- pi0
  if (GenDTag::PcheckDecay(genpart, 321, 321, -321, -211) == 1) {
    return -1 * (100000 * nPhotos + 1113);
  }//anti-D0 decays to K+ K+ K- pi-
  if (GenDTag::PcheckDecay(genpart, 310, 221, 111) == 1) {
    return -1 * (100000 * nPhotos + 1114);
  }//anti-D0 decays to K_S0 eta pi0
  if (GenDTag::PcheckDecay(genpart, 130, 221, 111) == 1) {
    return -1 * (100000 * nPhotos + 1115);
  }//anti-D0 decays to K_L0 eta pi0
  if (GenDTag::PcheckDecay(genpart, 311, 221, 111) == 1) {
    return -1 * (100000 * nPhotos + 1116);
  }//anti-D0 decays to K0 eta pi0
  if (GenDTag::PcheckDecay(genpart, 310, -321, 211) == 1) {
    return -1 * (100000 * nPhotos + 1117);
  }//anti-D0 decays to K_S0 K- pi+
  if (GenDTag::PcheckDecay(genpart, 130, -321, 211) == 1) {
    return -1 * (100000 * nPhotos + 1118);
  }//anti-D0 decays to K_L0 K- pi+
  if (GenDTag::PcheckDecay(genpart, 311, -321, 211) == 1) {
    return -1 * (100000 * nPhotos + 1119);
  }//anti-D0 decays to K0 K- pi+
  if (GenDTag::PcheckDecay(genpart, 310, 321, -211) == 1) {
    return -1 * (100000 * nPhotos + 1120);
  }//anti-D0 decays to K_S0 K+ pi-
  if (GenDTag::PcheckDecay(genpart, 130, 321, -211) == 1) {
    return -1 * (100000 * nPhotos + 1121);
  }//anti-D0 decays to K_L0 K+ pi-
  if (GenDTag::PcheckDecay(genpart, -311, 321, -211) == 1) {
    return -1 * (100000 * nPhotos + 1122);
  }//anti-D0 decays to anti-K0 K+ pi-
  if (GenDTag::PcheckDecay(genpart, 310, 310, 321, -211) == 1) {
    return -1 * (100000 * nPhotos + 1123);
  }//anti-D0 decays to K_S0 K_S0 K+ pi-
  if (GenDTag::PcheckDecay(genpart, 130, 130, 321, -211) == 1) {
    return -1 * (100000 * nPhotos + 1124);
  }//anti-D0 decays to K_L0 K_L0 K+ pi-
  if (GenDTag::PcheckDecay(genpart, -311, 311, 321, -211) == 1) {
    return -1 * (100000 * nPhotos + 1125);
  }//anti-D0 decays to anti-K0 K0 K+ pi-
  if (GenDTag::PcheckDecay(genpart, 310, 310, -321, 211) == 1) {
    return -1 * (100000 * nPhotos + 1126);
  }//anti-D0 decays to K_S0 K_S0 K- pi+
  if (GenDTag::PcheckDecay(genpart, 130, 130, -321, 211) == 1) {
    return -1 * (100000 * nPhotos + 1127);
  }//anti-D0 decays to K_L0 K_L0 K- pi+
  if (GenDTag::PcheckDecay(genpart, -311, 311, -321, 211) == 1) {
    return -1 * (100000 * nPhotos + 1128);
  }//anti-D0 decays to anti-K0 K0 K- pi+
  if (GenDTag::PcheckDecay(genpart, 321, -211, 211, 11, -12) == 1) {
    return -1 * (100000 * nPhotos + 1129);
  }//anti-D0 decays to K+ pi- pi+ e- anti-nu_e
  if (GenDTag::PcheckDecay(genpart, 113, 113) == 1) {
    return -1 * (100000 * nPhotos + 1130);
  }//anti-D0 decays to rho0 rho0
  if (GenDTag::PcheckDecay(genpart, 221, -211, 211) == 1) {
    return -1 * (100000 * nPhotos + 1131);
  }//anti-D0 decays to eta pi- pi+
  if (GenDTag::PcheckDecay(genpart, 223, -211, 211) == 1) {
    return -1 * (100000 * nPhotos + 1132);
  }//anti-D0 decays to omega pi- pi+
  if (GenDTag::PcheckDecay(genpart, 331, -211, 211) == 1) {
    return -1 * (100000 * nPhotos + 1133);
  }//anti-D0 decays to eta' pi- pi+
  if (GenDTag::PcheckDecay(genpart, 221, 331) == 1) {
    return -1 * (100000 * nPhotos + 1134);
  }//anti-D0 decays to eta eta'
  if (GenDTag::PcheckDecay(genpart, 333, 22) == 1) {
    return -1 * (100000 * nPhotos + 1135);
  }//anti-D0 decays to phi gamma
  if (GenDTag::PcheckDecay(genpart, 313, 22) == 1) {
    return -1 * (100000 * nPhotos + 1136);
  }//anti-D0 decays to K*0 gamma
  return -1 * (DauSize(genpart));
}// Rest of the anti-D0 decays
