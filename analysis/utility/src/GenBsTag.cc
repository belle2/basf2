/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Sourav Patra, Vishal Bhardwaj                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <analysis/utility/GenBsTag.h>
using namespace Belle2;

int GenBsTag::Array_CheckDecay(std::vector<int> MCDAU,
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


int GenBsTag::CheckDecay(std::vector<int>MCDAU, int a0 = -987656789,
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
  return  GenBsTag::Array_CheckDecay(MCDAU, genDAU);
}


int GenBsTag::DauSize(std::vector<int>genpart)
{
  return genpart.size();
}


int GenBsTag::PcheckDecay(std::vector<int>gp, int da1, int da2)
{
  int ndau = DauSize(gp);
  if (ndau == 2) { nPhotos = 0; return CheckDecay(gp, da1, da2);}
  else if (ndau == 3) { nPhotos = 1; return CheckDecay(gp, da1, da2, 22);}
  else if (ndau == 4) { nPhotos = 2; return CheckDecay(gp, da1, da2, 22, 22);}
  else if (ndau == 5) { nPhotos = 3; return CheckDecay(gp, da1, da2, 22, 22, 22);}
  else if (ndau == 6) { nPhotos = 4; return CheckDecay(gp, da1, da2, 22, 22, 22, 22);}
  else return -999;
}


int GenBsTag::PcheckDecay(std::vector<int>gp, int da1, int da2, int da3)
{
  int ndau = DauSize(gp);
  if (ndau == 3) { nPhotos = 0; return CheckDecay(gp, da1, da2, da3);}
  else if (ndau == 4) { nPhotos = 1; return CheckDecay(gp, da1, da2, da3, 22);}
  else if (ndau == 5) { nPhotos = 2; return CheckDecay(gp, da1, da2, da3, 22, 22);}
  else if (ndau == 6) { nPhotos = 3; return CheckDecay(gp, da1, da2, da3, 22, 22, 22);}
  else if (ndau == 7) { nPhotos = 4; return CheckDecay(gp, da1, da2, da3, 22, 22, 22, 22);}
  else return -999;
}


int GenBsTag::PcheckDecay(std::vector<int>gp, int da1, int da2, int da3, int da4)
{
  int ndau = DauSize(gp);
  if (ndau == 4) { nPhotos = 0; return CheckDecay(gp, da1, da2, da3, da4);}
  else if (ndau == 5) { nPhotos = 1; return CheckDecay(gp, da1, da2, da3, da4, 22);}
  else if (ndau == 6) { nPhotos = 2; return CheckDecay(gp, da1, da2, da3, da4, 22, 22);}
  else if (ndau == 7) { nPhotos = 3; return CheckDecay(gp, da1, da2, da3, da4, 22, 22, 22);}
  else if (ndau == 8) { nPhotos = 4; return CheckDecay(gp, da1, da2, da3, da4, 22, 22, 22, 22);}
  else return -999;
}


int GenBsTag::PcheckDecay(std::vector<int>gp, int da1, int da2, int da3, int da4, int da5)
{
  int ndau = DauSize(gp);
  if (ndau == 5) { nPhotos = 0; return CheckDecay(gp, da1, da2, da3, da4, da5);}
  else if (ndau == 6) { nPhotos = 1; return CheckDecay(gp, da1, da2, da3, da4, da5, 22);}
  else if (ndau == 7) { nPhotos = 2; return CheckDecay(gp, da1, da2, da3, da4, da5, 22, 22);}
  else if (ndau == 8) { nPhotos = 3; return CheckDecay(gp, da1, da2, da3, da4, da5, 22, 22, 22);}
  else if (ndau == 9) { nPhotos = 4; return CheckDecay(gp, da1, da2, da3, da4, da5, 22, 22, 22, 22);}
  else return -999;
}


int GenBsTag::PcheckDecay(std::vector<int>gp, int da1, int da2, int da3, int da4, int da5,
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


int GenBsTag::PcheckDecay(std::vector<int>gp, int da1, int da2, int da3, int da4, int da5,
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


int GenBsTag::PcheckDecay(std::vector<int>gp, int da1, int da2, int da3, int da4, int da5,
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


int GenBsTag::Mode_Bs0(std::vector<int>genpart)
{
  if (GenBsTag::PcheckDecay(genpart, -431, -11, 12) == 1) {
    return +1 * (100000 * nPhotos + 1001);
  }//B_s0 decays to D_s- e+ nu_e
  if (GenBsTag::PcheckDecay(genpart, -433, -11, 12) == 1) {
    return +1 * (100000 * nPhotos + 1002);
  }//B_s0 decays to D_s*- e+ nu_e
  if (GenBsTag::PcheckDecay(genpart, -10433, -11, 12) == 1) {
    return +1 * (100000 * nPhotos + 1003);
  }//B_s0 decays to D_s1- e+ nu_e
  if (GenBsTag::PcheckDecay(genpart, -10431, -11, 12) == 1) {
    return +1 * (100000 * nPhotos + 1004);
  }//B_s0 decays to D_s0*- e+ nu_e
  if (GenBsTag::PcheckDecay(genpart, -20433, -11, 12) == 1) {
    return +1 * (100000 * nPhotos + 1005);
  }//B_s0 decays to D'_s1- e+ nu_e
  if (GenBsTag::PcheckDecay(genpart, -435, -11, 12) == 1) {
    return +1 * (100000 * nPhotos + 1006);
  }//B_s0 decays to D_s2*- e+ nu_e
  if (GenBsTag::PcheckDecay(genpart, -431, -13, 14) == 1) {
    return +1 * (100000 * nPhotos + 1007);
  }//B_s0 decays to D_s- mu+ nu_mu
  if (GenBsTag::PcheckDecay(genpart, -433, -13, 14) == 1) {
    return +1 * (100000 * nPhotos + 1008);
  }//B_s0 decays to D_s*- mu+ nu_mu
  if (GenBsTag::PcheckDecay(genpart, -10433, -13, 14) == 1) {
    return +1 * (100000 * nPhotos + 1009);
  }//B_s0 decays to D_s1- mu+ nu_mu
  if (GenBsTag::PcheckDecay(genpart, -10431, -13, 14) == 1) {
    return +1 * (100000 * nPhotos + 1010);
  }//B_s0 decays to D_s0*- mu+ nu_mu
  if (GenBsTag::PcheckDecay(genpart, -20433, -13, 14) == 1) {
    return +1 * (100000 * nPhotos + 1011);
  }//B_s0 decays to D'_s1- mu+ nu_mu
  if (GenBsTag::PcheckDecay(genpart, -435, -13, 14) == 1) {
    return +1 * (100000 * nPhotos + 1012);
  }//B_s0 decays to D_s2*- mu+ nu_mu
  if (GenBsTag::PcheckDecay(genpart, -431, -15, 16) == 1) {
    return +1 * (100000 * nPhotos + 1013);
  }//B_s0 decays to D_s- tau+ nu_tau
  if (GenBsTag::PcheckDecay(genpart, -433, -15, 16) == 1) {
    return +1 * (100000 * nPhotos + 1014);
  }//B_s0 decays to D_s*- tau+ nu_tau
  if (GenBsTag::PcheckDecay(genpart, -10433, -15, 16) == 1) {
    return +1 * (100000 * nPhotos + 1015);
  }//B_s0 decays to D_s1- tau+ nu_tau
  if (GenBsTag::PcheckDecay(genpart, -10431, -15, 16) == 1) {
    return +1 * (100000 * nPhotos + 1016);
  }//B_s0 decays to D_s0*- tau+ nu_tau
  if (GenBsTag::PcheckDecay(genpart, -20433, -15, 16) == 1) {
    return +1 * (100000 * nPhotos + 1017);
  }//B_s0 decays to D'_s1- tau+ nu_tau
  if (GenBsTag::PcheckDecay(genpart, -435, -15, 16) == 1) {
    return +1 * (100000 * nPhotos + 1018);
  }//B_s0 decays to D_s2*- tau+ nu_tau
  if (GenBsTag::PcheckDecay(genpart, -431, 431) == 1) {
    return +1 * (100000 * nPhotos + 1019);
  }//B_s0 decays to D_s- D_s+
  if (GenBsTag::PcheckDecay(genpart, 433, -431) == 1) {
    return +1 * (100000 * nPhotos + 1020);
  }//B_s0 decays to D_s*+ D_s-
  if (GenBsTag::PcheckDecay(genpart, -433, 431) == 1) {
    return +1 * (100000 * nPhotos + 1021);
  }//B_s0 decays to D_s*- D_s+
  if (GenBsTag::PcheckDecay(genpart, -433, 433) == 1) {
    return +1 * (100000 * nPhotos + 1022);
  }//B_s0 decays to D_s*- D_s*+
  if (GenBsTag::PcheckDecay(genpart, 431, -411, -311) == 1) {
    return +1 * (100000 * nPhotos + 1023);
  }//B_s0 decays to D_s+ D- anti-K0
  if (GenBsTag::PcheckDecay(genpart, 431, 421, -321) == 1) {
    return +1 * (100000 * nPhotos + 1024);
  }//B_s0 decays to D_s+ D0 K-
  if (GenBsTag::PcheckDecay(genpart, 433, -411, -311) == 1) {
    return +1 * (100000 * nPhotos + 1025);
  }//B_s0 decays to D_s*+ D- anti-K0
  if (GenBsTag::PcheckDecay(genpart, 433, -421, -321) == 1) {
    return +1 * (100000 * nPhotos + 1026);
  }//B_s0 decays to D_s*+ anti-D0 K-
  if (GenBsTag::PcheckDecay(genpart, 431, -411, 111, -311) == 1) {
    return +1 * (100000 * nPhotos + 1027);
  }//B_s0 decays to D_s+ D- pi0 anti-K0
  if (GenBsTag::PcheckDecay(genpart, 431, -421, -211, -311) == 1) {
    return +1 * (100000 * nPhotos + 1028);
  }//B_s0 decays to D_s+ anti-D0 pi- anti-K0
  if (GenBsTag::PcheckDecay(genpart, 431, -411, 211, -321) == 1) {
    return +1 * (100000 * nPhotos + 1029);
  }//B_s0 decays to D_s+ D- pi+ K-
  if (GenBsTag::PcheckDecay(genpart, 431, -421, 111, -321) == 1) {
    return +1 * (100000 * nPhotos + 1030);
  }//B_s0 decays to D_s+ anti-D0 pi0 K-
  if (GenBsTag::PcheckDecay(genpart, 433, -411, 111, -311) == 1) {
    return +1 * (100000 * nPhotos + 1031);
  }//B_s0 decays to D_s*+ D- pi0 anti-K0
  if (GenBsTag::PcheckDecay(genpart, 433, -421, -211, -311) == 1) {
    return +1 * (100000 * nPhotos + 1032);
  }//B_s0 decays to D_s*+ anti-D0 pi- anti-K0
  if (GenBsTag::PcheckDecay(genpart, 433, -411, 211, -321) == 1) {
    return +1 * (100000 * nPhotos + 1033);
  }//B_s0 decays to D_s*+ D- pi+ K-
  if (GenBsTag::PcheckDecay(genpart, 433, -421, 111, -321) == 1) {
    return +1 * (100000 * nPhotos + 1034);
  }//B_s0 decays to D_s*+ anti-D0 pi0 K-
  if (GenBsTag::PcheckDecay(genpart, -433, 423, 321) == 1) {
    return +1 * (100000 * nPhotos + 1035);
  }//B_s0 decays to D_s*- D*0 K+
  if (GenBsTag::PcheckDecay(genpart, -433, 413, 311) == 1) {
    return +1 * (100000 * nPhotos + 1036);
  }//B_s0 decays to D_s*- D*+ K0
  if (GenBsTag::PcheckDecay(genpart, -433, 421, 321) == 1) {
    return +1 * (100000 * nPhotos + 1037);
  }//B_s0 decays to D_s*- D0 K+
  if (GenBsTag::PcheckDecay(genpart, -433, 411, 311) == 1) {
    return +1 * (100000 * nPhotos + 1038);
  }//B_s0 decays to D_s*- D+ K0
  if (GenBsTag::PcheckDecay(genpart, -431, 423, 321) == 1) {
    return +1 * (100000 * nPhotos + 1039);
  }//B_s0 decays to D_s- D*0 K+
  if (GenBsTag::PcheckDecay(genpart, -431, 413, 311) == 1) {
    return +1 * (100000 * nPhotos + 1040);
  }//B_s0 decays to D_s- D*+ K0
  if (GenBsTag::PcheckDecay(genpart, -431, 421, 321) == 1) {
    return +1 * (100000 * nPhotos + 1041);
  }//B_s0 decays to D_s- D0 K+
  if (GenBsTag::PcheckDecay(genpart, -431, 411, 311) == 1) {
    return +1 * (100000 * nPhotos + 1042);
  }//B_s0 decays to D_s- D+ K0
  if (GenBsTag::PcheckDecay(genpart, -433, 423, 323) == 1) {
    return +1 * (100000 * nPhotos + 1043);
  }//B_s0 decays to D_s*- D*0 K*+
  if (GenBsTag::PcheckDecay(genpart, -433, 413, 313) == 1) {
    return +1 * (100000 * nPhotos + 1044);
  }//B_s0 decays to D_s*- D*+ K*0
  if (GenBsTag::PcheckDecay(genpart, -433, 421, 323) == 1) {
    return +1 * (100000 * nPhotos + 1045);
  }//B_s0 decays to D_s*- D0 K*+
  if (GenBsTag::PcheckDecay(genpart, -433, 411, 313) == 1) {
    return +1 * (100000 * nPhotos + 1046);
  }//B_s0 decays to D_s*- D+ K*0
  if (GenBsTag::PcheckDecay(genpart, -431, 423, 323) == 1) {
    return +1 * (100000 * nPhotos + 1047);
  }//B_s0 decays to D_s- D*0 K*+
  if (GenBsTag::PcheckDecay(genpart, -431, 413, 313) == 1) {
    return +1 * (100000 * nPhotos + 1048);
  }//B_s0 decays to D_s- D*+ K*0
  if (GenBsTag::PcheckDecay(genpart, -431, 421, 323) == 1) {
    return +1 * (100000 * nPhotos + 1049);
  }//B_s0 decays to D_s- D0 K*+
  if (GenBsTag::PcheckDecay(genpart, -431, 411, 313) == 1) {
    return +1 * (100000 * nPhotos + 1050);
  }//B_s0 decays to D_s- D+ K*0
  if (GenBsTag::PcheckDecay(genpart, 431, -411) == 1) {
    return +1 * (100000 * nPhotos + 1051);
  }//B_s0 decays to D_s+ D-
  if (GenBsTag::PcheckDecay(genpart, -413, 431) == 1) {
    return +1 * (100000 * nPhotos + 1052);
  }//B_s0 decays to D*- D_s+
  if (GenBsTag::PcheckDecay(genpart, 433, -411) == 1) {
    return +1 * (100000 * nPhotos + 1053);
  }//B_s0 decays to D_s*+ D-
  if (GenBsTag::PcheckDecay(genpart, 433, -413) == 1) {
    return +1 * (100000 * nPhotos + 1054);
  }//B_s0 decays to D_s*+ D*-
  if (GenBsTag::PcheckDecay(genpart, 411, -411, -311) == 1) {
    return +1 * (100000 * nPhotos + 1055);
  }//B_s0 decays to D+ D- anti-K0
  if (GenBsTag::PcheckDecay(genpart, 411, 421, -321) == 1) {
    return +1 * (100000 * nPhotos + 1056);
  }//B_s0 decays to D+ D0 K-
  if (GenBsTag::PcheckDecay(genpart, 413, -411, -311) == 1) {
    return +1 * (100000 * nPhotos + 1057);
  }//B_s0 decays to D*+ D- anti-K0
  if (GenBsTag::PcheckDecay(genpart, 413, -421, -321) == 1) {
    return +1 * (100000 * nPhotos + 1058);
  }//B_s0 decays to D*+ anti-D0 K-
  if (GenBsTag::PcheckDecay(genpart, 411, -411, 111, -311) == 1) {
    return +1 * (100000 * nPhotos + 1059);
  }//B_s0 decays to D+ D- pi0 anti-K0
  if (GenBsTag::PcheckDecay(genpart, 411, -421, -211, -311) == 1) {
    return +1 * (100000 * nPhotos + 1060);
  }//B_s0 decays to D+ anti-D0 pi- anti-K0
  if (GenBsTag::PcheckDecay(genpart, 411, -411, 211, -321) == 1) {
    return +1 * (100000 * nPhotos + 1061);
  }//B_s0 decays to D+ D- pi+ K-
  if (GenBsTag::PcheckDecay(genpart, 411, -421, 111, -321) == 1) {
    return +1 * (100000 * nPhotos + 1062);
  }//B_s0 decays to D+ anti-D0 pi0 K-
  if (GenBsTag::PcheckDecay(genpart, 413, -411, 111, -311) == 1) {
    return +1 * (100000 * nPhotos + 1063);
  }//B_s0 decays to D*+ D- pi0 anti-K0
  if (GenBsTag::PcheckDecay(genpart, 413, -421, -211, -311) == 1) {
    return +1 * (100000 * nPhotos + 1064);
  }//B_s0 decays to D*+ anti-D0 pi- anti-K0
  if (GenBsTag::PcheckDecay(genpart, 413, -411, 211, -321) == 1) {
    return +1 * (100000 * nPhotos + 1065);
  }//B_s0 decays to D*+ D- pi+ K-
  if (GenBsTag::PcheckDecay(genpart, 413, -421, 111, -321) == 1) {
    return +1 * (100000 * nPhotos + 1066);
  }//B_s0 decays to D*+ anti-D0 pi0 K-
  if (GenBsTag::PcheckDecay(genpart, -433, 321) == 1) {
    return +1 * (100000 * nPhotos + 1067);
  }//B_s0 decays to D_s*- K+
  if (GenBsTag::PcheckDecay(genpart, -431, 321) == 1) {
    return +1 * (100000 * nPhotos + 1068);
  }//B_s0 decays to D_s- K+
  if (GenBsTag::PcheckDecay(genpart, -433, 323) == 1) {
    return +1 * (100000 * nPhotos + 1069);
  }//B_s0 decays to D_s*- K*+
  if (GenBsTag::PcheckDecay(genpart, 323, -431) == 1) {
    return +1 * (100000 * nPhotos + 1070);
  }//B_s0 decays to K*+ D_s-
  if (GenBsTag::PcheckDecay(genpart, 443, 331) == 1) {
    return +1 * (100000 * nPhotos + 1071);
  }//B_s0 decays to J/psi eta'
  if (GenBsTag::PcheckDecay(genpart, 443, 221) == 1) {
    return +1 * (100000 * nPhotos + 1072);
  }//B_s0 decays to J/psi eta
  if (GenBsTag::PcheckDecay(genpart, 443, 333) == 1) {
    return +1 * (100000 * nPhotos + 1073);
  }//B_s0 decays to J/psi phi
  if (GenBsTag::PcheckDecay(genpart, 443, 311) == 1) {
    return +1 * (100000 * nPhotos + 1074);
  }//B_s0 decays to J/psi K0
  if (GenBsTag::PcheckDecay(genpart, 443, -321, 321) == 1) {
    return +1 * (100000 * nPhotos + 1075);
  }//B_s0 decays to J/psi K- K+
  if (GenBsTag::PcheckDecay(genpart, 443, -311, 311) == 1) {
    return +1 * (100000 * nPhotos + 1076);
  }//B_s0 decays to J/psi anti-K0 K0
  if (GenBsTag::PcheckDecay(genpart, 443, 311, -321, 211) == 1) {
    return +1 * (100000 * nPhotos + 1077);
  }//B_s0 decays to J/psi K0 K- pi+
  if (GenBsTag::PcheckDecay(genpart, 443, -311, 321, -211) == 1) {
    return +1 * (100000 * nPhotos + 1078);
  }//B_s0 decays to J/psi anti-K0 K+ pi-
  if (GenBsTag::PcheckDecay(genpart, 443, -311, 311, 111) == 1) {
    return +1 * (100000 * nPhotos + 1079);
  }//B_s0 decays to J/psi anti-K0 K0 pi0
  if (GenBsTag::PcheckDecay(genpart, 443, -321, 321, 111) == 1) {
    return +1 * (100000 * nPhotos + 1080);
  }//B_s0 decays to J/psi K- K+ pi0
  if (GenBsTag::PcheckDecay(genpart, 443, 333, 211, -211) == 1) {
    return +1 * (100000 * nPhotos + 1081);
  }//B_s0 decays to J/psi phi pi+ pi-
  if (GenBsTag::PcheckDecay(genpart, 443, 333, 111, 111) == 1) {
    return +1 * (100000 * nPhotos + 1082);
  }//B_s0 decays to J/psi phi pi0 pi0
  if (GenBsTag::PcheckDecay(genpart, 443, 221, 211, -211) == 1) {
    return +1 * (100000 * nPhotos + 1083);
  }//B_s0 decays to J/psi eta pi+ pi-
  if (GenBsTag::PcheckDecay(genpart, 443, 221, 111, 111) == 1) {
    return +1 * (100000 * nPhotos + 1084);
  }//B_s0 decays to J/psi eta pi0 pi0
  if (GenBsTag::PcheckDecay(genpart, 443, 331, 211, -211) == 1) {
    return +1 * (100000 * nPhotos + 1085);
  }//B_s0 decays to J/psi eta' pi+ pi-
  if (GenBsTag::PcheckDecay(genpart, 443, 331, 111, 111) == 1) {
    return +1 * (100000 * nPhotos + 1086);
  }//B_s0 decays to J/psi eta' pi0 pi0
  if (GenBsTag::PcheckDecay(genpart, 443, 211, -211) == 1) {
    return +1 * (100000 * nPhotos + 1087);
  }//B_s0 decays to J/psi pi+ pi-
  if (GenBsTag::PcheckDecay(genpart, 443, 111, 111) == 1) {
    return +1 * (100000 * nPhotos + 1088);
  }//B_s0 decays to J/psi pi0 pi0
  if (GenBsTag::PcheckDecay(genpart, 100443, 331) == 1) {
    return +1 * (100000 * nPhotos + 1089);
  }//B_s0 decays to psi(2S) eta'
  if (GenBsTag::PcheckDecay(genpart, 100443, 221) == 1) {
    return +1 * (100000 * nPhotos + 1090);
  }//B_s0 decays to psi(2S) eta
  if (GenBsTag::PcheckDecay(genpart, 100443, 333) == 1) {
    return +1 * (100000 * nPhotos + 1091);
  }//B_s0 decays to psi(2S) phi
  if (GenBsTag::PcheckDecay(genpart, 100443, -321, 321) == 1) {
    return +1 * (100000 * nPhotos + 1092);
  }//B_s0 decays to psi(2S) K- K+
  if (GenBsTag::PcheckDecay(genpart, 100443, -311, 311) == 1) {
    return +1 * (100000 * nPhotos + 1093);
  }//B_s0 decays to psi(2S) anti-K0 K0
  if (GenBsTag::PcheckDecay(genpart, 100443, 311, -321, 211) == 1) {
    return +1 * (100000 * nPhotos + 1094);
  }//B_s0 decays to psi(2S) K0 K- pi+
  if (GenBsTag::PcheckDecay(genpart, 100443, -311, 311, 111) == 1) {
    return +1 * (100000 * nPhotos + 1095);
  }//B_s0 decays to psi(2S) anti-K0 K0 pi0
  if (GenBsTag::PcheckDecay(genpart, 100443, -321, 321, 111) == 1) {
    return +1 * (100000 * nPhotos + 1096);
  }//B_s0 decays to psi(2S) K- K+ pi0
  if (GenBsTag::PcheckDecay(genpart, 100443, 333, 211, -211) == 1) {
    return +1 * (100000 * nPhotos + 1097);
  }//B_s0 decays to psi(2S) phi pi+ pi-
  if (GenBsTag::PcheckDecay(genpart, 100443, 333, 111, 111) == 1) {
    return +1 * (100000 * nPhotos + 1098);
  }//B_s0 decays to psi(2S) phi pi0 pi0
  if (GenBsTag::PcheckDecay(genpart, 100443, 221, 211, -211) == 1) {
    return +1 * (100000 * nPhotos + 1099);
  }//B_s0 decays to psi(2S) eta pi+ pi-
  if (GenBsTag::PcheckDecay(genpart, 100443, 221, 111, 111) == 1) {
    return +1 * (100000 * nPhotos + 1100);
  }//B_s0 decays to psi(2S) eta pi0 pi0
  if (GenBsTag::PcheckDecay(genpart, 100443, 331, 211, -211) == 1) {
    return +1 * (100000 * nPhotos + 1101);
  }//B_s0 decays to psi(2S) eta' pi+ pi-
  if (GenBsTag::PcheckDecay(genpart, 100443, 331, 111, 111) == 1) {
    return +1 * (100000 * nPhotos + 1102);
  }//B_s0 decays to psi(2S) eta' pi0 pi0
  if (GenBsTag::PcheckDecay(genpart, 100443, 211, -211) == 1) {
    return +1 * (100000 * nPhotos + 1103);
  }//B_s0 decays to psi(2S) pi+ pi-
  if (GenBsTag::PcheckDecay(genpart, 100443, 111, 111) == 1) {
    return +1 * (100000 * nPhotos + 1104);
  }//B_s0 decays to psi(2S) pi0 pi0
  if (GenBsTag::PcheckDecay(genpart, 10441, 331) == 1) {
    return +1 * (100000 * nPhotos + 1105);
  }//B_s0 decays to chi_c0 eta'
  if (GenBsTag::PcheckDecay(genpart, 10441, 221) == 1) {
    return +1 * (100000 * nPhotos + 1106);
  }//B_s0 decays to chi_c0 eta
  if (GenBsTag::PcheckDecay(genpart, 333, 10441) == 1) {
    return +1 * (100000 * nPhotos + 1107);
  }//B_s0 decays to phi chi_c0
  if (GenBsTag::PcheckDecay(genpart, 10441, -321, 321) == 1) {
    return +1 * (100000 * nPhotos + 1108);
  }//B_s0 decays to chi_c0 K- K+
  if (GenBsTag::PcheckDecay(genpart, 10441, -311, 311) == 1) {
    return +1 * (100000 * nPhotos + 1109);
  }//B_s0 decays to chi_c0 anti-K0 K0
  if (GenBsTag::PcheckDecay(genpart, 10441, 311, -321, 211) == 1) {
    return +1 * (100000 * nPhotos + 1110);
  }//B_s0 decays to chi_c0 K0 K- pi+
  if (GenBsTag::PcheckDecay(genpart, 10441, -311, 311, 111) == 1) {
    return +1 * (100000 * nPhotos + 1111);
  }//B_s0 decays to chi_c0 anti-K0 K0 pi0
  if (GenBsTag::PcheckDecay(genpart, 10441, -321, 321, 111) == 1) {
    return +1 * (100000 * nPhotos + 1112);
  }//B_s0 decays to chi_c0 K- K+ pi0
  if (GenBsTag::PcheckDecay(genpart, 20443, 331) == 1) {
    return +1 * (100000 * nPhotos + 1113);
  }//B_s0 decays to chi_c1 eta'
  if (GenBsTag::PcheckDecay(genpart, 20443, 221) == 1) {
    return +1 * (100000 * nPhotos + 1114);
  }//B_s0 decays to chi_c1 eta
  if (GenBsTag::PcheckDecay(genpart, 20443, 333) == 1) {
    return +1 * (100000 * nPhotos + 1115);
  }//B_s0 decays to chi_c1 phi
  if (GenBsTag::PcheckDecay(genpart, 20443, -321, 321) == 1) {
    return +1 * (100000 * nPhotos + 1116);
  }//B_s0 decays to chi_c1 K- K+
  if (GenBsTag::PcheckDecay(genpart, 20443, -311, 311) == 1) {
    return +1 * (100000 * nPhotos + 1117);
  }//B_s0 decays to chi_c1 anti-K0 K0
  if (GenBsTag::PcheckDecay(genpart, 20443, 311, -321, 211) == 1) {
    return +1 * (100000 * nPhotos + 1118);
  }//B_s0 decays to chi_c1 K0 K- pi+
  if (GenBsTag::PcheckDecay(genpart, 20443, -311, 311, 111) == 1) {
    return +1 * (100000 * nPhotos + 1119);
  }//B_s0 decays to chi_c1 anti-K0 K0 pi0
  if (GenBsTag::PcheckDecay(genpart, 20443, -321, 321, 111) == 1) {
    return +1 * (100000 * nPhotos + 1120);
  }//B_s0 decays to chi_c1 K- K+ pi0
  if (GenBsTag::PcheckDecay(genpart, 20443, 333, 211, -211) == 1) {
    return +1 * (100000 * nPhotos + 1121);
  }//B_s0 decays to chi_c1 phi pi+ pi-
  if (GenBsTag::PcheckDecay(genpart, 20443, 333, 111, 111) == 1) {
    return +1 * (100000 * nPhotos + 1122);
  }//B_s0 decays to chi_c1 phi pi0 pi0
  if (GenBsTag::PcheckDecay(genpart, 20443, 221, 211, -211) == 1) {
    return +1 * (100000 * nPhotos + 1123);
  }//B_s0 decays to chi_c1 eta pi+ pi-
  if (GenBsTag::PcheckDecay(genpart, 20443, 221, 111, 111) == 1) {
    return +1 * (100000 * nPhotos + 1124);
  }//B_s0 decays to chi_c1 eta pi0 pi0
  if (GenBsTag::PcheckDecay(genpart, 20443, 331, 211, -211) == 1) {
    return +1 * (100000 * nPhotos + 1125);
  }//B_s0 decays to chi_c1 eta' pi+ pi-
  if (GenBsTag::PcheckDecay(genpart, 20443, 331, 111, 111) == 1) {
    return +1 * (100000 * nPhotos + 1126);
  }//B_s0 decays to chi_c1 eta' pi0 pi0
  if (GenBsTag::PcheckDecay(genpart, 445, 331) == 1) {
    return +1 * (100000 * nPhotos + 1127);
  }//B_s0 decays to chi_c2 eta'
  if (GenBsTag::PcheckDecay(genpart, 445, 221) == 1) {
    return +1 * (100000 * nPhotos + 1128);
  }//B_s0 decays to chi_c2 eta
  if (GenBsTag::PcheckDecay(genpart, 445, -321, 321) == 1) {
    return +1 * (100000 * nPhotos + 1129);
  }//B_s0 decays to chi_c2 K- K+
  if (GenBsTag::PcheckDecay(genpart, 445, -311, 311) == 1) {
    return +1 * (100000 * nPhotos + 1130);
  }//B_s0 decays to chi_c2 anti-K0 K0
  if (GenBsTag::PcheckDecay(genpart, 445, 311, -321, 211) == 1) {
    return +1 * (100000 * nPhotos + 1131);
  }//B_s0 decays to chi_c2 K0 K- pi+
  if (GenBsTag::PcheckDecay(genpart, 445, -311, 311, 111) == 1) {
    return +1 * (100000 * nPhotos + 1132);
  }//B_s0 decays to chi_c2 anti-K0 K0 pi0
  if (GenBsTag::PcheckDecay(genpart, 445, -321, 321, 111) == 1) {
    return +1 * (100000 * nPhotos + 1133);
  }//B_s0 decays to chi_c2 K- K+ pi0
  if (GenBsTag::PcheckDecay(genpart, 441, 331) == 1) {
    return +1 * (100000 * nPhotos + 1134);
  }//B_s0 decays to eta_c eta'
  if (GenBsTag::PcheckDecay(genpart, 441, 221) == 1) {
    return +1 * (100000 * nPhotos + 1135);
  }//B_s0 decays to eta_c eta
  if (GenBsTag::PcheckDecay(genpart, 333, 441) == 1) {
    return +1 * (100000 * nPhotos + 1136);
  }//B_s0 decays to phi eta_c
  if (GenBsTag::PcheckDecay(genpart, 441, -321, 321) == 1) {
    return +1 * (100000 * nPhotos + 1137);
  }//B_s0 decays to eta_c K- K+
  if (GenBsTag::PcheckDecay(genpart, 441, 311, -311) == 1) {
    return +1 * (100000 * nPhotos + 1138);
  }//B_s0 decays to eta_c K0 anti-K0
  if (GenBsTag::PcheckDecay(genpart, 441, 311, -321, 211) == 1) {
    return +1 * (100000 * nPhotos + 1139);
  }//B_s0 decays to eta_c K0 K- pi+
  if (GenBsTag::PcheckDecay(genpart, 441, 311, -311, 111) == 1) {
    return +1 * (100000 * nPhotos + 1140);
  }//B_s0 decays to eta_c K0 anti-K0 pi0
  if (GenBsTag::PcheckDecay(genpart, 441, -321, 321, 111) == 1) {
    return +1 * (100000 * nPhotos + 1141);
  }//B_s0 decays to eta_c K- K+ pi0
  if (GenBsTag::PcheckDecay(genpart, 441, 333, 211, -211) == 1) {
    return +1 * (100000 * nPhotos + 1142);
  }//B_s0 decays to eta_c phi pi+ pi-
  if (GenBsTag::PcheckDecay(genpart, 441, 333, 111, 111) == 1) {
    return +1 * (100000 * nPhotos + 1143);
  }//B_s0 decays to eta_c phi pi0 pi0
  if (GenBsTag::PcheckDecay(genpart, 441, 221, 211, -211) == 1) {
    return +1 * (100000 * nPhotos + 1144);
  }//B_s0 decays to eta_c eta pi+ pi-
  if (GenBsTag::PcheckDecay(genpart, 441, 221, 111, 111) == 1) {
    return +1 * (100000 * nPhotos + 1145);
  }//B_s0 decays to eta_c eta pi0 pi0
  if (GenBsTag::PcheckDecay(genpart, 441, 331, 211, -211) == 1) {
    return +1 * (100000 * nPhotos + 1146);
  }//B_s0 decays to eta_c eta' pi+ pi-
  if (GenBsTag::PcheckDecay(genpart, 441, 331, 111, 111) == 1) {
    return +1 * (100000 * nPhotos + 1147);
  }//B_s0 decays to eta_c eta' pi0 pi0
  if (GenBsTag::PcheckDecay(genpart, 100441, 331) == 1) {
    return +1 * (100000 * nPhotos + 1148);
  }//B_s0 decays to eta_c(2S) eta'
  if (GenBsTag::PcheckDecay(genpart, 100441, 221) == 1) {
    return +1 * (100000 * nPhotos + 1149);
  }//B_s0 decays to eta_c(2S) eta
  if (GenBsTag::PcheckDecay(genpart, 333, 100441) == 1) {
    return +1 * (100000 * nPhotos + 1150);
  }//B_s0 decays to phi eta_c(2S)
  if (GenBsTag::PcheckDecay(genpart, 100441, -321, 321) == 1) {
    return +1 * (100000 * nPhotos + 1151);
  }//B_s0 decays to eta_c(2S) K- K+
  if (GenBsTag::PcheckDecay(genpart, 100441, -311, 311) == 1) {
    return +1 * (100000 * nPhotos + 1152);
  }//B_s0 decays to eta_c(2S) anti-K0 K0
  if (GenBsTag::PcheckDecay(genpart, 100441, 311, -321, 211) == 1) {
    return +1 * (100000 * nPhotos + 1153);
  }//B_s0 decays to eta_c(2S) K0 K- pi+
  if (GenBsTag::PcheckDecay(genpart, 100441, -311, 311, 111) == 1) {
    return +1 * (100000 * nPhotos + 1154);
  }//B_s0 decays to eta_c(2S) anti-K0 K0 pi0
  if (GenBsTag::PcheckDecay(genpart, 100441, -321, 321, 111) == 1) {
    return +1 * (100000 * nPhotos + 1155);
  }//B_s0 decays to eta_c(2S) K- K+ pi0
  if (GenBsTag::PcheckDecay(genpart, 10443, 331) == 1) {
    return +1 * (100000 * nPhotos + 1156);
  }//B_s0 decays to h_c eta'
  if (GenBsTag::PcheckDecay(genpart, 10443, 221) == 1) {
    return +1 * (100000 * nPhotos + 1157);
  }//B_s0 decays to h_c eta
  if (GenBsTag::PcheckDecay(genpart, 10443, 333) == 1) {
    return +1 * (100000 * nPhotos + 1158);
  }//B_s0 decays to h_c phi
  if (GenBsTag::PcheckDecay(genpart, 10443, -321, 321) == 1) {
    return +1 * (100000 * nPhotos + 1159);
  }//B_s0 decays to h_c K- K+
  if (GenBsTag::PcheckDecay(genpart, 10443, -311, 311) == 1) {
    return +1 * (100000 * nPhotos + 1160);
  }//B_s0 decays to h_c anti-K0 K0
  if (GenBsTag::PcheckDecay(genpart, 10443, 311, -321, 211) == 1) {
    return +1 * (100000 * nPhotos + 1161);
  }//B_s0 decays to h_c K0 K- pi+
  if (GenBsTag::PcheckDecay(genpart, 10443, -311, 311, 111) == 1) {
    return +1 * (100000 * nPhotos + 1162);
  }//B_s0 decays to h_c anti-K0 K0 pi0
  if (GenBsTag::PcheckDecay(genpart, 10443, -321, 321, 111) == 1) {
    return +1 * (100000 * nPhotos + 1163);
  }//B_s0 decays to h_c K- K+ pi0
  if (GenBsTag::PcheckDecay(genpart, 443, 333, 333) == 1) {
    return +1 * (100000 * nPhotos + 1164);
  }//B_s0 decays to J/psi phi phi
  if (GenBsTag::PcheckDecay(genpart, 443, 310) == 1) {
    return +1 * (100000 * nPhotos + 1165);
  }//B_s0 decays to J/psi K_S0
  if (GenBsTag::PcheckDecay(genpart, 443, 313) == 1) {
    return +1 * (100000 * nPhotos + 1166);
  }//B_s0 decays to J/psi K*0
  if (GenBsTag::PcheckDecay(genpart, 443, 211, -211, 211, -211) == 1) {
    return +1 * (100000 * nPhotos + 1167);
  }//B_s0 decays to J/psi pi+ pi- pi+ pi-
  if (GenBsTag::PcheckDecay(genpart, 443, 20223) == 1) {
    return +1 * (100000 * nPhotos + 1168);
  }//B_s0 decays to J/psi f_1
  if (GenBsTag::PcheckDecay(genpart, 100443, 321, -211) == 1) {
    return +1 * (100000 * nPhotos + 1169);
  }//B_s0 decays to psi(2S) K+ pi-
  if (GenBsTag::PcheckDecay(genpart, 100443, 313) == 1) {
    return +1 * (100000 * nPhotos + 1170);
  }//B_s0 decays to psi(2S) K*0
  if (GenBsTag::PcheckDecay(genpart, -10433, 211) == 1) {
    return +1 * (100000 * nPhotos + 1171);
  }//B_s0 decays to D_s1- pi+
  if (GenBsTag::PcheckDecay(genpart, -10433, 213) == 1) {
    return +1 * (100000 * nPhotos + 1172);
  }//B_s0 decays to D_s1- rho+
  if (GenBsTag::PcheckDecay(genpart, -435, 211) == 1) {
    return +1 * (100000 * nPhotos + 1173);
  }//B_s0 decays to D_s2*- pi+
  if (GenBsTag::PcheckDecay(genpart, -433, 211) == 1) {
    return +1 * (100000 * nPhotos + 1174);
  }//B_s0 decays to D_s*- pi+
  if (GenBsTag::PcheckDecay(genpart, -431, 211) == 1) {
    return +1 * (100000 * nPhotos + 1175);
  }//B_s0 decays to D_s- pi+
  if (GenBsTag::PcheckDecay(genpart, 213, -431) == 1) {
    return +1 * (100000 * nPhotos + 1176);
  }//B_s0 decays to rho+ D_s-
  if (GenBsTag::PcheckDecay(genpart, -433, 213) == 1) {
    return +1 * (100000 * nPhotos + 1177);
  }//B_s0 decays to D_s*- rho+
  if (GenBsTag::PcheckDecay(genpart, 20213, -431) == 1) {
    return +1 * (100000 * nPhotos + 1178);
  }//B_s0 decays to a_1+ D_s-
  if (GenBsTag::PcheckDecay(genpart, -431, 113, 211) == 1) {
    return +1 * (100000 * nPhotos + 1179);
  }//B_s0 decays to D_s- rho0 pi+
  if (GenBsTag::PcheckDecay(genpart, -431, 213, 111) == 1) {
    return +1 * (100000 * nPhotos + 1180);
  }//B_s0 decays to D_s- rho+ pi0
  if (GenBsTag::PcheckDecay(genpart, -431, -211, 211, 211) == 1) {
    return +1 * (100000 * nPhotos + 1181);
  }//B_s0 decays to D_s- pi- pi+ pi+
  if (GenBsTag::PcheckDecay(genpart, -431, 111, 211, 111) == 1) {
    return +1 * (100000 * nPhotos + 1182);
  }//B_s0 decays to D_s- pi0 pi+ pi0
  if (GenBsTag::PcheckDecay(genpart, -433, 20213) == 1) {
    return +1 * (100000 * nPhotos + 1183);
  }//B_s0 decays to D_s*- a_1+
  if (GenBsTag::PcheckDecay(genpart, -433, 113, 211) == 1) {
    return +1 * (100000 * nPhotos + 1184);
  }//B_s0 decays to D_s*- rho0 pi+
  if (GenBsTag::PcheckDecay(genpart, -433, 213, 111) == 1) {
    return +1 * (100000 * nPhotos + 1185);
  }//B_s0 decays to D_s*- rho+ pi0
  if (GenBsTag::PcheckDecay(genpart, -433, -211, 211, 211) == 1) {
    return +1 * (100000 * nPhotos + 1186);
  }//B_s0 decays to D_s*- pi- pi+ pi+
  if (GenBsTag::PcheckDecay(genpart, -433, 111, 211, 111) == 1) {
    return +1 * (100000 * nPhotos + 1187);
  }//B_s0 decays to D_s*- pi0 pi+ pi0
  if (GenBsTag::PcheckDecay(genpart, -423, -311) == 1) {
    return +1 * (100000 * nPhotos + 1188);
  }//B_s0 decays to anti-D*0 anti-K0
  if (GenBsTag::PcheckDecay(genpart, -421, -311) == 1) {
    return +1 * (100000 * nPhotos + 1189);
  }//B_s0 decays to anti-D0 anti-K0
  if (GenBsTag::PcheckDecay(genpart, -313, -421) == 1) {
    return +1 * (100000 * nPhotos + 1190);
  }//B_s0 decays to anti-K*0 anti-D0
  if (GenBsTag::PcheckDecay(genpart, -423, -313) == 1) {
    return +1 * (100000 * nPhotos + 1191);
  }//B_s0 decays to anti-D*0 anti-K*0
  if (GenBsTag::PcheckDecay(genpart, 2212, -2212) == 1) {
    return +1 * (100000 * nPhotos + 1192);
  }//B_s0 decays to p+ anti-p-
  if (GenBsTag::PcheckDecay(genpart, -4301, 2101) == 1) {
    return +1 * (100000 * nPhotos + 1193);
  }//B_s0 decays to anti-cs_0 ud_0
  if (GenBsTag::PcheckDecay(genpart, -4303, 2103) == 1) {
    return +1 * (100000 * nPhotos + 1194);
  }//B_s0 decays to anti-cs_1 ud_1
  if (GenBsTag::PcheckDecay(genpart, 2, -1, -4, 3) == 1) {
    return +1 * (100000 * nPhotos + 1195);
  }//B_s0 decays to u anti-d anti-c s
  if (GenBsTag::PcheckDecay(genpart, 2, -1, -4, 3) == 1) {
    return +1 * (100000 * nPhotos + 1196);
  }//B_s0 decays to u anti-d anti-c s
  if (GenBsTag::PcheckDecay(genpart, 2, -4, -1, 3) == 1) {
    return +1 * (100000 * nPhotos + 1197);
  }//B_s0 decays to u anti-c anti-d s
  if (GenBsTag::PcheckDecay(genpart, 4, -3, -2, 3) == 1) {
    return +1 * (100000 * nPhotos + 1198);
  }//B_s0 decays to c anti-s anti-u s
  if (GenBsTag::PcheckDecay(genpart, 2, -3, -4, 3) == 1) {
    return +1 * (100000 * nPhotos + 1199);
  }//B_s0 decays to u anti-s anti-c s
  if (GenBsTag::PcheckDecay(genpart, 2, -3, -4, 3) == 1) {
    return +1 * (100000 * nPhotos + 1200);
  }//B_s0 decays to u anti-s anti-c s
  if (GenBsTag::PcheckDecay(genpart, 2, -4, -3, 3) == 1) {
    return +1 * (100000 * nPhotos + 1201);
  }//B_s0 decays to u anti-c anti-s s
  if (GenBsTag::PcheckDecay(genpart, 4, -1, -4, 3) == 1) {
    return +1 * (100000 * nPhotos + 1202);
  }//B_s0 decays to c anti-d anti-c s
  if (GenBsTag::PcheckDecay(genpart, 4, -1, -4, 3) == 1) {
    return +1 * (100000 * nPhotos + 1203);
  }//B_s0 decays to c anti-d anti-c s
  if (GenBsTag::PcheckDecay(genpart, 2, -2, -1, 3) == 1) {
    return +1 * (100000 * nPhotos + 1204);
  }//B_s0 decays to u anti-u anti-d s
  if (GenBsTag::PcheckDecay(genpart, 1, -1, -1, 3) == 1) {
    return +1 * (100000 * nPhotos + 1205);
  }//B_s0 decays to d anti-d anti-d s
  if (GenBsTag::PcheckDecay(genpart, 3, -3, -1, 3) == 1) {
    return +1 * (100000 * nPhotos + 1206);
  }//B_s0 decays to s anti-s anti-d s
  if (GenBsTag::PcheckDecay(genpart, 2, -2, -3, 3) == 1) {
    return +1 * (100000 * nPhotos + 1207);
  }//B_s0 decays to u anti-u anti-s s
  if (GenBsTag::PcheckDecay(genpart, 1, -1, -3, 3) == 1) {
    return +1 * (100000 * nPhotos + 1208);
  }//B_s0 decays to d anti-d anti-s s
  if (GenBsTag::PcheckDecay(genpart, 3, -3, -3, 3) == 1) {
    return +1 * (100000 * nPhotos + 1209);
  }//B_s0 decays to s anti-s anti-s s
  if (GenBsTag::PcheckDecay(genpart, -3, 3) == 1) {
    return +1 * (100000 * nPhotos + 1210);
  }//B_s0 decays to anti-s s
  if (GenBsTag::PcheckDecay(genpart, -321, -11, 12) == 1) {
    return +1 * (100000 * nPhotos + 1211);
  }//B_s0 decays to K- e+ nu_e
  if (GenBsTag::PcheckDecay(genpart, -323, -11, 12) == 1) {
    return +1 * (100000 * nPhotos + 1212);
  }//B_s0 decays to K*- e+ nu_e
  if (GenBsTag::PcheckDecay(genpart, -10323, -11, 12) == 1) {
    return +1 * (100000 * nPhotos + 1213);
  }//B_s0 decays to K_1- e+ nu_e
  if (GenBsTag::PcheckDecay(genpart, -20323, -11, 12) == 1) {
    return +1 * (100000 * nPhotos + 1214);
  }//B_s0 decays to K'_1- e+ nu_e
  if (GenBsTag::PcheckDecay(genpart, -321, -13, 14) == 1) {
    return +1 * (100000 * nPhotos + 1215);
  }//B_s0 decays to K- mu+ nu_mu
  if (GenBsTag::PcheckDecay(genpart, -323, -13, 14) == 1) {
    return +1 * (100000 * nPhotos + 1216);
  }//B_s0 decays to K*- mu+ nu_mu
  if (GenBsTag::PcheckDecay(genpart, -10323, -13, 14) == 1) {
    return +1 * (100000 * nPhotos + 1217);
  }//B_s0 decays to K_1- mu+ nu_mu
  if (GenBsTag::PcheckDecay(genpart, -20323, -13, 14) == 1) {
    return +1 * (100000 * nPhotos + 1218);
  }//B_s0 decays to K'_1- mu+ nu_mu
  if (GenBsTag::PcheckDecay(genpart, -321, 321) == 1) {
    return +1 * (100000 * nPhotos + 1219);
  }//B_s0 decays to K- K+
  if (GenBsTag::PcheckDecay(genpart, -311, 311) == 1) {
    return +1 * (100000 * nPhotos + 1220);
  }//B_s0 decays to anti-K0 K0
  if (GenBsTag::PcheckDecay(genpart, 310, 310) == 1) {
    return +1 * (100000 * nPhotos + 1221);
  }//B_s0 decays to K_S0 K_S0
  if (GenBsTag::PcheckDecay(genpart, 130, 130) == 1) {
    return +1 * (100000 * nPhotos + 1222);
  }//B_s0 decays to K_L0 K_L0
  if (GenBsTag::PcheckDecay(genpart, 211, -321) == 1) {
    return +1 * (100000 * nPhotos + 1223);
  }//B_s0 decays to pi+ K-
  if (GenBsTag::PcheckDecay(genpart, 111, -311) == 1) {
    return +1 * (100000 * nPhotos + 1224);
  }//B_s0 decays to pi0 anti-K0
  if (GenBsTag::PcheckDecay(genpart, 211, -211) == 1) {
    return +1 * (100000 * nPhotos + 1225);
  }//B_s0 decays to pi+ pi-
  if (GenBsTag::PcheckDecay(genpart, 223, 221) == 1) {
    return +1 * (100000 * nPhotos + 1226);
  }//B_s0 decays to omega eta
  if (GenBsTag::PcheckDecay(genpart, 223, 331) == 1) {
    return +1 * (100000 * nPhotos + 1227);
  }//B_s0 decays to omega eta'
  if (GenBsTag::PcheckDecay(genpart, 333, 221) == 1) {
    return +1 * (100000 * nPhotos + 1228);
  }//B_s0 decays to phi eta
  if (GenBsTag::PcheckDecay(genpart, 333, 331) == 1) {
    return +1 * (100000 * nPhotos + 1229);
  }//B_s0 decays to phi eta'
  if (GenBsTag::PcheckDecay(genpart, 223, 311) == 1) {
    return +1 * (100000 * nPhotos + 1230);
  }//B_s0 decays to omega K0
  if (GenBsTag::PcheckDecay(genpart, 333, 311) == 1) {
    return +1 * (100000 * nPhotos + 1231);
  }//B_s0 decays to phi K0
  if (GenBsTag::PcheckDecay(genpart, -323, 211) == 1) {
    return +1 * (100000 * nPhotos + 1232);
  }//B_s0 decays to K*- pi+
  if (GenBsTag::PcheckDecay(genpart, 213, -321) == 1) {
    return +1 * (100000 * nPhotos + 1233);
  }//B_s0 decays to rho+ K-
  if (GenBsTag::PcheckDecay(genpart, 113, 311) == 1) {
    return +1 * (100000 * nPhotos + 1234);
  }//B_s0 decays to rho0 K0
  if (GenBsTag::PcheckDecay(genpart, -313, 111) == 1) {
    return +1 * (100000 * nPhotos + 1235);
  }//B_s0 decays to anti-K*0 pi0
  if (GenBsTag::PcheckDecay(genpart, 323, -321) == 1) {
    return +1 * (100000 * nPhotos + 1236);
  }//B_s0 decays to K*+ K-
  if (GenBsTag::PcheckDecay(genpart, 313, -311) == 1) {
    return +1 * (100000 * nPhotos + 1237);
  }//B_s0 decays to K*0 anti-K0
  if (GenBsTag::PcheckDecay(genpart, 313, 310) == 1) {
    return +1 * (100000 * nPhotos + 1238);
  }//B_s0 decays to K*0 K_S0
  if (GenBsTag::PcheckDecay(genpart, 313, 130) == 1) {
    return +1 * (100000 * nPhotos + 1239);
  }//B_s0 decays to K*0 K_L0
  if (GenBsTag::PcheckDecay(genpart, -323, 321) == 1) {
    return +1 * (100000 * nPhotos + 1240);
  }//B_s0 decays to K*- K+
  if (GenBsTag::PcheckDecay(genpart, -313, 311) == 1) {
    return +1 * (100000 * nPhotos + 1241);
  }//B_s0 decays to anti-K*0 K0
  if (GenBsTag::PcheckDecay(genpart, -313, 310) == 1) {
    return +1 * (100000 * nPhotos + 1242);
  }//B_s0 decays to anti-K*0 K_S0
  if (GenBsTag::PcheckDecay(genpart, -313, 130) == 1) {
    return +1 * (100000 * nPhotos + 1243);
  }//B_s0 decays to anti-K*0 K_L0
  if (GenBsTag::PcheckDecay(genpart, 331, 331) == 1) {
    return +1 * (100000 * nPhotos + 1244);
  }//B_s0 decays to eta' eta'
  if (GenBsTag::PcheckDecay(genpart, 331, 221) == 1) {
    return +1 * (100000 * nPhotos + 1245);
  }//B_s0 decays to eta' eta
  if (GenBsTag::PcheckDecay(genpart, 221, 221) == 1) {
    return +1 * (100000 * nPhotos + 1246);
  }//B_s0 decays to eta eta
  if (GenBsTag::PcheckDecay(genpart, -311, 331) == 1) {
    return +1 * (100000 * nPhotos + 1247);
  }//B_s0 decays to anti-K0 eta'
  if (GenBsTag::PcheckDecay(genpart, -311, 221) == 1) {
    return +1 * (100000 * nPhotos + 1248);
  }//B_s0 decays to anti-K0 eta
  if (GenBsTag::PcheckDecay(genpart, -313, 331) == 1) {
    return +1 * (100000 * nPhotos + 1249);
  }//B_s0 decays to anti-K*0 eta'
  if (GenBsTag::PcheckDecay(genpart, -313, 221) == 1) {
    return +1 * (100000 * nPhotos + 1250);
  }//B_s0 decays to anti-K*0 eta
  if (GenBsTag::PcheckDecay(genpart, 213, -323) == 1) {
    return +1 * (100000 * nPhotos + 1251);
  }//B_s0 decays to rho+ K*-
  if (GenBsTag::PcheckDecay(genpart, 113, -313) == 1) {
    return +1 * (100000 * nPhotos + 1252);
  }//B_s0 decays to rho0 anti-K*0
  if (GenBsTag::PcheckDecay(genpart, 223, -313) == 1) {
    return +1 * (100000 * nPhotos + 1253);
  }//B_s0 decays to omega anti-K*0
  if (GenBsTag::PcheckDecay(genpart, -323, 323) == 1) {
    return +1 * (100000 * nPhotos + 1254);
  }//B_s0 decays to K*- K*+
  if (GenBsTag::PcheckDecay(genpart, -313, 313) == 1) {
    return +1 * (100000 * nPhotos + 1255);
  }//B_s0 decays to anti-K*0 K*0
  if (GenBsTag::PcheckDecay(genpart, 333, 333) == 1) {
    return +1 * (100000 * nPhotos + 1256);
  }//B_s0 decays to phi phi
  if (GenBsTag::PcheckDecay(genpart, 333, -313) == 1) {
    return +1 * (100000 * nPhotos + 1257);
  }//B_s0 decays to phi anti-K*0
  if (GenBsTag::PcheckDecay(genpart, -13, 13) == 1) {
    return +1 * (100000 * nPhotos + 1258);
  }//B_s0 decays to mu+ mu-
  if (GenBsTag::PcheckDecay(genpart, -15, 15) == 1) {
    return +1 * (100000 * nPhotos + 1259);
  }//B_s0 decays to tau+ tau-
  if (GenBsTag::PcheckDecay(genpart, 333, 22) == 1) {
    return +1 * (100000 * nPhotos + 1260);
  }//B_s0 decays to phi gamma
  if (GenBsTag::PcheckDecay(genpart, 333, -11, 11) == 1) {
    return +1 * (100000 * nPhotos + 1261);
  }//B_s0 decays to phi e+ e-
  if (GenBsTag::PcheckDecay(genpart, 333, -13, 13) == 1) {
    return +1 * (100000 * nPhotos + 1262);
  }//B_s0 decays to phi mu+ mu-
  if (GenBsTag::PcheckDecay(genpart, 22, 22) == 1) {
    return +1 * (100000 * nPhotos + 1263);
  }//B_s0 decays to gamma gamma
  if (GenBsTag::PcheckDecay(genpart, 431, -321) == 1) {
    return +1 * (100000 * nPhotos + 1264);
  }//B_s0 decays to D_s+ K-
  return +1 * (DauSize(genpart));
}// Rest of the B_s0 decays


int GenBsTag::Mode_anti_Bs0(std::vector<int>genpart)
{
  if (GenBsTag::PcheckDecay(genpart, 431, 11, -12) == 1) {
    return -1 * (100000 * nPhotos + 1001);
  }//anti-B_s0 decays to D_s+ e- anti-nu_e
  if (GenBsTag::PcheckDecay(genpart, 433, 11, -12) == 1) {
    return -1 * (100000 * nPhotos + 1002);
  }//anti-B_s0 decays to D_s*+ e- anti-nu_e
  if (GenBsTag::PcheckDecay(genpart, 10433, 11, -12) == 1) {
    return -1 * (100000 * nPhotos + 1003);
  }//anti-B_s0 decays to D_s1+ e- anti-nu_e
  if (GenBsTag::PcheckDecay(genpart, 10431, 11, -12) == 1) {
    return -1 * (100000 * nPhotos + 1004);
  }//anti-B_s0 decays to D_s0*+ e- anti-nu_e
  if (GenBsTag::PcheckDecay(genpart, 20433, 11, -12) == 1) {
    return -1 * (100000 * nPhotos + 1005);
  }//anti-B_s0 decays to D'_s1+ e- anti-nu_e
  if (GenBsTag::PcheckDecay(genpart, 435, 11, -12) == 1) {
    return -1 * (100000 * nPhotos + 1006);
  }//anti-B_s0 decays to D_s2*+ e- anti-nu_e
  if (GenBsTag::PcheckDecay(genpart, 431, 13, -14) == 1) {
    return -1 * (100000 * nPhotos + 1007);
  }//anti-B_s0 decays to D_s+ mu- anti-nu_mu
  if (GenBsTag::PcheckDecay(genpart, 433, 13, -14) == 1) {
    return -1 * (100000 * nPhotos + 1008);
  }//anti-B_s0 decays to D_s*+ mu- anti-nu_mu
  if (GenBsTag::PcheckDecay(genpart, 10433, 13, -14) == 1) {
    return -1 * (100000 * nPhotos + 1009);
  }//anti-B_s0 decays to D_s1+ mu- anti-nu_mu
  if (GenBsTag::PcheckDecay(genpart, 10431, 13, -14) == 1) {
    return -1 * (100000 * nPhotos + 1010);
  }//anti-B_s0 decays to D_s0*+ mu- anti-nu_mu
  if (GenBsTag::PcheckDecay(genpart, 20433, 13, -14) == 1) {
    return -1 * (100000 * nPhotos + 1011);
  }//anti-B_s0 decays to D'_s1+ mu- anti-nu_mu
  if (GenBsTag::PcheckDecay(genpart, 435, 13, -14) == 1) {
    return -1 * (100000 * nPhotos + 1012);
  }//anti-B_s0 decays to D_s2*+ mu- anti-nu_mu
  if (GenBsTag::PcheckDecay(genpart, 431, 15, -16) == 1) {
    return -1 * (100000 * nPhotos + 1013);
  }//anti-B_s0 decays to D_s+ tau- anti-nu_tau
  if (GenBsTag::PcheckDecay(genpart, 433, 15, -16) == 1) {
    return -1 * (100000 * nPhotos + 1014);
  }//anti-B_s0 decays to D_s*+ tau- anti-nu_tau
  if (GenBsTag::PcheckDecay(genpart, 10433, 15, -16) == 1) {
    return -1 * (100000 * nPhotos + 1015);
  }//anti-B_s0 decays to D_s1+ tau- anti-nu_tau
  if (GenBsTag::PcheckDecay(genpart, 10431, 15, -16) == 1) {
    return -1 * (100000 * nPhotos + 1016);
  }//anti-B_s0 decays to D_s0*+ tau- anti-nu_tau
  if (GenBsTag::PcheckDecay(genpart, 20433, 15, -16) == 1) {
    return -1 * (100000 * nPhotos + 1017);
  }//anti-B_s0 decays to D'_s1+ tau- anti-nu_tau
  if (GenBsTag::PcheckDecay(genpart, 435, 15, -16) == 1) {
    return -1 * (100000 * nPhotos + 1018);
  }//anti-B_s0 decays to D_s2*+ tau- anti-nu_tau
  if (GenBsTag::PcheckDecay(genpart, -431, 431) == 1) {
    return -1 * (100000 * nPhotos + 1019);
  }//anti-B_s0 decays to D_s- D_s+
  if (GenBsTag::PcheckDecay(genpart, 433, -431) == 1) {
    return -1 * (100000 * nPhotos + 1020);
  }//anti-B_s0 decays to D_s*+ D_s-
  if (GenBsTag::PcheckDecay(genpart, -433, 431) == 1) {
    return -1 * (100000 * nPhotos + 1021);
  }//anti-B_s0 decays to D_s*- D_s+
  if (GenBsTag::PcheckDecay(genpart, -433, 433) == 1) {
    return -1 * (100000 * nPhotos + 1022);
  }//anti-B_s0 decays to D_s*- D_s*+
  if (GenBsTag::PcheckDecay(genpart, -431, 411, 311) == 1) {
    return -1 * (100000 * nPhotos + 1023);
  }//anti-B_s0 decays to D_s- D+ K0
  if (GenBsTag::PcheckDecay(genpart, -431, -421, 321) == 1) {
    return -1 * (100000 * nPhotos + 1024);
  }//anti-B_s0 decays to D_s- anti-D0 K+
  if (GenBsTag::PcheckDecay(genpart, -433, 411, 311) == 1) {
    return -1 * (100000 * nPhotos + 1025);
  }//anti-B_s0 decays to D_s*- D+ K0
  if (GenBsTag::PcheckDecay(genpart, -433, 421, 321) == 1) {
    return -1 * (100000 * nPhotos + 1026);
  }//anti-B_s0 decays to D_s*- D0 K+
  if (GenBsTag::PcheckDecay(genpart, -431, 411, 111, 311) == 1) {
    return -1 * (100000 * nPhotos + 1027);
  }//anti-B_s0 decays to D_s- D+ pi0 K0
  if (GenBsTag::PcheckDecay(genpart, -431, 421, 211, 311) == 1) {
    return -1 * (100000 * nPhotos + 1028);
  }//anti-B_s0 decays to D_s- D0 pi+ K0
  if (GenBsTag::PcheckDecay(genpart, -431, 411, -211, 321) == 1) {
    return -1 * (100000 * nPhotos + 1029);
  }//anti-B_s0 decays to D_s- D+ pi- K+
  if (GenBsTag::PcheckDecay(genpart, -431, 421, 111, 321) == 1) {
    return -1 * (100000 * nPhotos + 1030);
  }//anti-B_s0 decays to D_s- D0 pi0 K+
  if (GenBsTag::PcheckDecay(genpart, -433, 411, 111, 311) == 1) {
    return -1 * (100000 * nPhotos + 1031);
  }//anti-B_s0 decays to D_s*- D+ pi0 K0
  if (GenBsTag::PcheckDecay(genpart, -433, 421, 211, 311) == 1) {
    return -1 * (100000 * nPhotos + 1032);
  }//anti-B_s0 decays to D_s*- D0 pi+ K0
  if (GenBsTag::PcheckDecay(genpart, -433, 411, -211, 321) == 1) {
    return -1 * (100000 * nPhotos + 1033);
  }//anti-B_s0 decays to D_s*- D+ pi- K+
  if (GenBsTag::PcheckDecay(genpart, -433, 421, 111, 321) == 1) {
    return -1 * (100000 * nPhotos + 1034);
  }//anti-B_s0 decays to D_s*- D0 pi0 K+
  if (GenBsTag::PcheckDecay(genpart, 433, -423, -321) == 1) {
    return -1 * (100000 * nPhotos + 1035);
  }//anti-B_s0 decays to D_s*+ anti-D*0 K-
  if (GenBsTag::PcheckDecay(genpart, 433, -413, -311) == 1) {
    return -1 * (100000 * nPhotos + 1036);
  }//anti-B_s0 decays to D_s*+ D*- anti-K0
  if (GenBsTag::PcheckDecay(genpart, 433, -421, -321) == 1) {
    return -1 * (100000 * nPhotos + 1037);
  }//anti-B_s0 decays to D_s*+ anti-D0 K-
  if (GenBsTag::PcheckDecay(genpart, 433, -411, -311) == 1) {
    return -1 * (100000 * nPhotos + 1038);
  }//anti-B_s0 decays to D_s*+ D- anti-K0
  if (GenBsTag::PcheckDecay(genpart, 431, -423, -321) == 1) {
    return -1 * (100000 * nPhotos + 1039);
  }//anti-B_s0 decays to D_s+ anti-D*0 K-
  if (GenBsTag::PcheckDecay(genpart, 431, -413, -311) == 1) {
    return -1 * (100000 * nPhotos + 1040);
  }//anti-B_s0 decays to D_s+ D*- anti-K0
  if (GenBsTag::PcheckDecay(genpart, 431, -421, -321) == 1) {
    return -1 * (100000 * nPhotos + 1041);
  }//anti-B_s0 decays to D_s+ anti-D0 K-
  if (GenBsTag::PcheckDecay(genpart, 431, -411, -311) == 1) {
    return -1 * (100000 * nPhotos + 1042);
  }//anti-B_s0 decays to D_s+ D- anti-K0
  if (GenBsTag::PcheckDecay(genpart, 433, -423, -323) == 1) {
    return -1 * (100000 * nPhotos + 1043);
  }//anti-B_s0 decays to D_s*+ anti-D*0 K*-
  if (GenBsTag::PcheckDecay(genpart, 433, -413, -313) == 1) {
    return -1 * (100000 * nPhotos + 1044);
  }//anti-B_s0 decays to D_s*+ D*- anti-K*0
  if (GenBsTag::PcheckDecay(genpart, 433, -421, -323) == 1) {
    return -1 * (100000 * nPhotos + 1045);
  }//anti-B_s0 decays to D_s*+ anti-D0 K*-
  if (GenBsTag::PcheckDecay(genpart, 433, -411, -313) == 1) {
    return -1 * (100000 * nPhotos + 1046);
  }//anti-B_s0 decays to D_s*+ D- anti-K*0
  if (GenBsTag::PcheckDecay(genpart, 431, -423, -323) == 1) {
    return -1 * (100000 * nPhotos + 1047);
  }//anti-B_s0 decays to D_s+ anti-D*0 K*-
  if (GenBsTag::PcheckDecay(genpart, 431, -413, -313) == 1) {
    return -1 * (100000 * nPhotos + 1048);
  }//anti-B_s0 decays to D_s+ D*- anti-K*0
  if (GenBsTag::PcheckDecay(genpart, 431, -421, -323) == 1) {
    return -1 * (100000 * nPhotos + 1049);
  }//anti-B_s0 decays to D_s+ anti-D0 K*-
  if (GenBsTag::PcheckDecay(genpart, 431, -411, -313) == 1) {
    return -1 * (100000 * nPhotos + 1050);
  }//anti-B_s0 decays to D_s+ D- anti-K*0
  if (GenBsTag::PcheckDecay(genpart, -431, 411) == 1) {
    return -1 * (100000 * nPhotos + 1051);
  }//anti-B_s0 decays to D_s- D+
  if (GenBsTag::PcheckDecay(genpart, 413, -431) == 1) {
    return -1 * (100000 * nPhotos + 1052);
  }//anti-B_s0 decays to D*+ D_s-
  if (GenBsTag::PcheckDecay(genpart, -433, 411) == 1) {
    return -1 * (100000 * nPhotos + 1053);
  }//anti-B_s0 decays to D_s*- D+
  if (GenBsTag::PcheckDecay(genpart, -433, 413) == 1) {
    return -1 * (100000 * nPhotos + 1054);
  }//anti-B_s0 decays to D_s*- D*+
  if (GenBsTag::PcheckDecay(genpart, -411, 411, 311) == 1) {
    return -1 * (100000 * nPhotos + 1055);
  }//anti-B_s0 decays to D- D+ K0
  if (GenBsTag::PcheckDecay(genpart, -411, -421, 321) == 1) {
    return -1 * (100000 * nPhotos + 1056);
  }//anti-B_s0 decays to D- anti-D0 K+
  if (GenBsTag::PcheckDecay(genpart, -413, 411, 311) == 1) {
    return -1 * (100000 * nPhotos + 1057);
  }//anti-B_s0 decays to D*- D+ K0
  if (GenBsTag::PcheckDecay(genpart, -413, 421, 321) == 1) {
    return -1 * (100000 * nPhotos + 1058);
  }//anti-B_s0 decays to D*- D0 K+
  if (GenBsTag::PcheckDecay(genpart, -411, 411, 111, 311) == 1) {
    return -1 * (100000 * nPhotos + 1059);
  }//anti-B_s0 decays to D- D+ pi0 K0
  if (GenBsTag::PcheckDecay(genpart, -411, 421, 211, 311) == 1) {
    return -1 * (100000 * nPhotos + 1060);
  }//anti-B_s0 decays to D- D0 pi+ K0
  if (GenBsTag::PcheckDecay(genpart, -411, 411, -211, 321) == 1) {
    return -1 * (100000 * nPhotos + 1061);
  }//anti-B_s0 decays to D- D+ pi- K+
  if (GenBsTag::PcheckDecay(genpart, -411, 421, 111, 321) == 1) {
    return -1 * (100000 * nPhotos + 1062);
  }//anti-B_s0 decays to D- D0 pi0 K+
  if (GenBsTag::PcheckDecay(genpart, -413, 411, 111, 311) == 1) {
    return -1 * (100000 * nPhotos + 1063);
  }//anti-B_s0 decays to D*- D+ pi0 K0
  if (GenBsTag::PcheckDecay(genpart, -413, 421, 211, 311) == 1) {
    return -1 * (100000 * nPhotos + 1064);
  }//anti-B_s0 decays to D*- D0 pi+ K0
  if (GenBsTag::PcheckDecay(genpart, -413, 411, -211, 321) == 1) {
    return -1 * (100000 * nPhotos + 1065);
  }//anti-B_s0 decays to D*- D+ pi- K+
  if (GenBsTag::PcheckDecay(genpart, -413, 421, 111, 321) == 1) {
    return -1 * (100000 * nPhotos + 1066);
  }//anti-B_s0 decays to D*- D0 pi0 K+
  if (GenBsTag::PcheckDecay(genpart, 433, -321) == 1) {
    return -1 * (100000 * nPhotos + 1067);
  }//anti-B_s0 decays to D_s*+ K-
  if (GenBsTag::PcheckDecay(genpart, 431, -321) == 1) {
    return -1 * (100000 * nPhotos + 1068);
  }//anti-B_s0 decays to D_s+ K-
  if (GenBsTag::PcheckDecay(genpart, 433, -323) == 1) {
    return -1 * (100000 * nPhotos + 1069);
  }//anti-B_s0 decays to D_s*+ K*-
  if (GenBsTag::PcheckDecay(genpart, -323, 431) == 1) {
    return -1 * (100000 * nPhotos + 1070);
  }//anti-B_s0 decays to K*- D_s+
  if (GenBsTag::PcheckDecay(genpart, 443, 331) == 1) {
    return -1 * (100000 * nPhotos + 1071);
  }//anti-B_s0 decays to J/psi eta'
  if (GenBsTag::PcheckDecay(genpart, 443, 221) == 1) {
    return -1 * (100000 * nPhotos + 1072);
  }//anti-B_s0 decays to J/psi eta
  if (GenBsTag::PcheckDecay(genpart, 443, 333) == 1) {
    return -1 * (100000 * nPhotos + 1073);
  }//anti-B_s0 decays to J/psi phi
  if (GenBsTag::PcheckDecay(genpart, 443, 311) == 1) {
    return -1 * (100000 * nPhotos + 1074);
  }//anti-B_s0 decays to J/psi K0
  if (GenBsTag::PcheckDecay(genpart, 443, -321, 321) == 1) {
    return -1 * (100000 * nPhotos + 1075);
  }//anti-B_s0 decays to J/psi K- K+
  if (GenBsTag::PcheckDecay(genpart, 443, -311, 311) == 1) {
    return -1 * (100000 * nPhotos + 1076);
  }//anti-B_s0 decays to J/psi anti-K0 K0
  if (GenBsTag::PcheckDecay(genpart, 443, 311, -321, 211) == 1) {
    return -1 * (100000 * nPhotos + 1077);
  }//anti-B_s0 decays to J/psi K0 K- pi+
  if (GenBsTag::PcheckDecay(genpart, 443, -311, 321, -211) == 1) {
    return -1 * (100000 * nPhotos + 1078);
  }//anti-B_s0 decays to J/psi anti-K0 K+ pi-
  if (GenBsTag::PcheckDecay(genpart, 443, -311, 311, 111) == 1) {
    return -1 * (100000 * nPhotos + 1079);
  }//anti-B_s0 decays to J/psi anti-K0 K0 pi0
  if (GenBsTag::PcheckDecay(genpart, 443, -321, 321, 111) == 1) {
    return -1 * (100000 * nPhotos + 1080);
  }//anti-B_s0 decays to J/psi K- K+ pi0
  if (GenBsTag::PcheckDecay(genpart, 443, 333, 211, -211) == 1) {
    return -1 * (100000 * nPhotos + 1081);
  }//anti-B_s0 decays to J/psi phi pi+ pi-
  if (GenBsTag::PcheckDecay(genpart, 443, 333, 111, 111) == 1) {
    return -1 * (100000 * nPhotos + 1082);
  }//anti-B_s0 decays to J/psi phi pi0 pi0
  if (GenBsTag::PcheckDecay(genpart, 443, 221, 211, -211) == 1) {
    return -1 * (100000 * nPhotos + 1083);
  }//anti-B_s0 decays to J/psi eta pi+ pi-
  if (GenBsTag::PcheckDecay(genpart, 443, 221, 111, 111) == 1) {
    return -1 * (100000 * nPhotos + 1084);
  }//anti-B_s0 decays to J/psi eta pi0 pi0
  if (GenBsTag::PcheckDecay(genpart, 443, 331, 211, -211) == 1) {
    return -1 * (100000 * nPhotos + 1085);
  }//anti-B_s0 decays to J/psi eta' pi+ pi-
  if (GenBsTag::PcheckDecay(genpart, 443, 331, 111, 111) == 1) {
    return -1 * (100000 * nPhotos + 1086);
  }//anti-B_s0 decays to J/psi eta' pi0 pi0
  if (GenBsTag::PcheckDecay(genpart, 443, 211, -211) == 1) {
    return -1 * (100000 * nPhotos + 1087);
  }//anti-B_s0 decays to J/psi pi+ pi-
  if (GenBsTag::PcheckDecay(genpart, 443, 111, 111) == 1) {
    return -1 * (100000 * nPhotos + 1088);
  }//anti-B_s0 decays to J/psi pi0 pi0
  if (GenBsTag::PcheckDecay(genpart, 100443, 331) == 1) {
    return -1 * (100000 * nPhotos + 1089);
  }//anti-B_s0 decays to psi(2S) eta'
  if (GenBsTag::PcheckDecay(genpart, 100443, 221) == 1) {
    return -1 * (100000 * nPhotos + 1090);
  }//anti-B_s0 decays to psi(2S) eta
  if (GenBsTag::PcheckDecay(genpart, 100443, 333) == 1) {
    return -1 * (100000 * nPhotos + 1091);
  }//anti-B_s0 decays to psi(2S) phi
  if (GenBsTag::PcheckDecay(genpart, 100443, -321, 321) == 1) {
    return -1 * (100000 * nPhotos + 1092);
  }//anti-B_s0 decays to psi(2S) K- K+
  if (GenBsTag::PcheckDecay(genpart, 100443, -311, 311) == 1) {
    return -1 * (100000 * nPhotos + 1093);
  }//anti-B_s0 decays to psi(2S) anti-K0 K0
  if (GenBsTag::PcheckDecay(genpart, 100443, -311, 321, -211) == 1) {
    return -1 * (100000 * nPhotos + 1094);
  }//anti-B_s0 decays to psi(2S) anti-K0 K+ pi-
  if (GenBsTag::PcheckDecay(genpart, 100443, -311, 311, 111) == 1) {
    return -1 * (100000 * nPhotos + 1095);
  }//anti-B_s0 decays to psi(2S) anti-K0 K0 pi0
  if (GenBsTag::PcheckDecay(genpart, 100443, -321, 321, 111) == 1) {
    return -1 * (100000 * nPhotos + 1096);
  }//anti-B_s0 decays to psi(2S) K- K+ pi0
  if (GenBsTag::PcheckDecay(genpart, 100443, 333, 211, -211) == 1) {
    return -1 * (100000 * nPhotos + 1097);
  }//anti-B_s0 decays to psi(2S) phi pi+ pi-
  if (GenBsTag::PcheckDecay(genpart, 100443, 333, 111, 111) == 1) {
    return -1 * (100000 * nPhotos + 1098);
  }//anti-B_s0 decays to psi(2S) phi pi0 pi0
  if (GenBsTag::PcheckDecay(genpart, 100443, 221, 211, -211) == 1) {
    return -1 * (100000 * nPhotos + 1099);
  }//anti-B_s0 decays to psi(2S) eta pi+ pi-
  if (GenBsTag::PcheckDecay(genpart, 100443, 221, 111, 111) == 1) {
    return -1 * (100000 * nPhotos + 1100);
  }//anti-B_s0 decays to psi(2S) eta pi0 pi0
  if (GenBsTag::PcheckDecay(genpart, 100443, 331, 211, -211) == 1) {
    return -1 * (100000 * nPhotos + 1101);
  }//anti-B_s0 decays to psi(2S) eta' pi+ pi-
  if (GenBsTag::PcheckDecay(genpart, 100443, 331, 111, 111) == 1) {
    return -1 * (100000 * nPhotos + 1102);
  }//anti-B_s0 decays to psi(2S) eta' pi0 pi0
  if (GenBsTag::PcheckDecay(genpart, 100443, 211, -211) == 1) {
    return -1 * (100000 * nPhotos + 1103);
  }//anti-B_s0 decays to psi(2S) pi+ pi-
  if (GenBsTag::PcheckDecay(genpart, 100443, 111, 111) == 1) {
    return -1 * (100000 * nPhotos + 1104);
  }//anti-B_s0 decays to psi(2S) pi0 pi0
  if (GenBsTag::PcheckDecay(genpart, 10441, 331) == 1) {
    return -1 * (100000 * nPhotos + 1105);
  }//anti-B_s0 decays to chi_c0 eta'
  if (GenBsTag::PcheckDecay(genpart, 10441, 221) == 1) {
    return -1 * (100000 * nPhotos + 1106);
  }//anti-B_s0 decays to chi_c0 eta
  if (GenBsTag::PcheckDecay(genpart, 333, 10441) == 1) {
    return -1 * (100000 * nPhotos + 1107);
  }//anti-B_s0 decays to phi chi_c0
  if (GenBsTag::PcheckDecay(genpart, 10441, -321, 321) == 1) {
    return -1 * (100000 * nPhotos + 1108);
  }//anti-B_s0 decays to chi_c0 K- K+
  if (GenBsTag::PcheckDecay(genpart, 10441, -311, 311) == 1) {
    return -1 * (100000 * nPhotos + 1109);
  }//anti-B_s0 decays to chi_c0 anti-K0 K0
  if (GenBsTag::PcheckDecay(genpart, 10441, -311, 321, -211) == 1) {
    return -1 * (100000 * nPhotos + 1110);
  }//anti-B_s0 decays to chi_c0 anti-K0 K+ pi-
  if (GenBsTag::PcheckDecay(genpart, 10441, -311, 311, 111) == 1) {
    return -1 * (100000 * nPhotos + 1111);
  }//anti-B_s0 decays to chi_c0 anti-K0 K0 pi0
  if (GenBsTag::PcheckDecay(genpart, 10441, -321, 321, 111) == 1) {
    return -1 * (100000 * nPhotos + 1112);
  }//anti-B_s0 decays to chi_c0 K- K+ pi0
  if (GenBsTag::PcheckDecay(genpart, 20443, 331) == 1) {
    return -1 * (100000 * nPhotos + 1113);
  }//anti-B_s0 decays to chi_c1 eta'
  if (GenBsTag::PcheckDecay(genpart, 20443, 221) == 1) {
    return -1 * (100000 * nPhotos + 1114);
  }//anti-B_s0 decays to chi_c1 eta
  if (GenBsTag::PcheckDecay(genpart, 20443, 333) == 1) {
    return -1 * (100000 * nPhotos + 1115);
  }//anti-B_s0 decays to chi_c1 phi
  if (GenBsTag::PcheckDecay(genpart, 20443, -321, 321) == 1) {
    return -1 * (100000 * nPhotos + 1116);
  }//anti-B_s0 decays to chi_c1 K- K+
  if (GenBsTag::PcheckDecay(genpart, 20443, -311, 311) == 1) {
    return -1 * (100000 * nPhotos + 1117);
  }//anti-B_s0 decays to chi_c1 anti-K0 K0
  if (GenBsTag::PcheckDecay(genpart, 20443, -311, 321, -211) == 1) {
    return -1 * (100000 * nPhotos + 1118);
  }//anti-B_s0 decays to chi_c1 anti-K0 K+ pi-
  if (GenBsTag::PcheckDecay(genpart, 20443, -311, 311, 111) == 1) {
    return -1 * (100000 * nPhotos + 1119);
  }//anti-B_s0 decays to chi_c1 anti-K0 K0 pi0
  if (GenBsTag::PcheckDecay(genpart, 20443, -321, 321, 111) == 1) {
    return -1 * (100000 * nPhotos + 1120);
  }//anti-B_s0 decays to chi_c1 K- K+ pi0
  if (GenBsTag::PcheckDecay(genpart, 20443, 333, 211, -211) == 1) {
    return -1 * (100000 * nPhotos + 1121);
  }//anti-B_s0 decays to chi_c1 phi pi+ pi-
  if (GenBsTag::PcheckDecay(genpart, 20443, 333, 111, 111) == 1) {
    return -1 * (100000 * nPhotos + 1122);
  }//anti-B_s0 decays to chi_c1 phi pi0 pi0
  if (GenBsTag::PcheckDecay(genpart, 20443, 221, 211, -211) == 1) {
    return -1 * (100000 * nPhotos + 1123);
  }//anti-B_s0 decays to chi_c1 eta pi+ pi-
  if (GenBsTag::PcheckDecay(genpart, 20443, 221, 111, 111) == 1) {
    return -1 * (100000 * nPhotos + 1124);
  }//anti-B_s0 decays to chi_c1 eta pi0 pi0
  if (GenBsTag::PcheckDecay(genpart, 20443, 331, 211, -211) == 1) {
    return -1 * (100000 * nPhotos + 1125);
  }//anti-B_s0 decays to chi_c1 eta' pi+ pi-
  if (GenBsTag::PcheckDecay(genpart, 20443, 331, 111, 111) == 1) {
    return -1 * (100000 * nPhotos + 1126);
  }//anti-B_s0 decays to chi_c1 eta' pi0 pi0
  if (GenBsTag::PcheckDecay(genpart, 445, 331) == 1) {
    return -1 * (100000 * nPhotos + 1127);
  }//anti-B_s0 decays to chi_c2 eta'
  if (GenBsTag::PcheckDecay(genpart, 445, 221) == 1) {
    return -1 * (100000 * nPhotos + 1128);
  }//anti-B_s0 decays to chi_c2 eta
  if (GenBsTag::PcheckDecay(genpart, 445, -321, 321) == 1) {
    return -1 * (100000 * nPhotos + 1129);
  }//anti-B_s0 decays to chi_c2 K- K+
  if (GenBsTag::PcheckDecay(genpart, 445, -311, 311) == 1) {
    return -1 * (100000 * nPhotos + 1130);
  }//anti-B_s0 decays to chi_c2 anti-K0 K0
  if (GenBsTag::PcheckDecay(genpart, 445, -311, 321, -211) == 1) {
    return -1 * (100000 * nPhotos + 1131);
  }//anti-B_s0 decays to chi_c2 anti-K0 K+ pi-
  if (GenBsTag::PcheckDecay(genpart, 445, -311, 311, 111) == 1) {
    return -1 * (100000 * nPhotos + 1132);
  }//anti-B_s0 decays to chi_c2 anti-K0 K0 pi0
  if (GenBsTag::PcheckDecay(genpart, 445, -321, 321, 111) == 1) {
    return -1 * (100000 * nPhotos + 1133);
  }//anti-B_s0 decays to chi_c2 K- K+ pi0
  if (GenBsTag::PcheckDecay(genpart, 441, 331) == 1) {
    return -1 * (100000 * nPhotos + 1134);
  }//anti-B_s0 decays to eta_c eta'
  if (GenBsTag::PcheckDecay(genpart, 441, 221) == 1) {
    return -1 * (100000 * nPhotos + 1135);
  }//anti-B_s0 decays to eta_c eta
  if (GenBsTag::PcheckDecay(genpart, 333, 441) == 1) {
    return -1 * (100000 * nPhotos + 1136);
  }//anti-B_s0 decays to phi eta_c
  if (GenBsTag::PcheckDecay(genpart, 441, -321, 321) == 1) {
    return -1 * (100000 * nPhotos + 1137);
  }//anti-B_s0 decays to eta_c K- K+
  if (GenBsTag::PcheckDecay(genpart, 441, -311, 311) == 1) {
    return -1 * (100000 * nPhotos + 1138);
  }//anti-B_s0 decays to eta_c anti-K0 K0
  if (GenBsTag::PcheckDecay(genpart, 441, -311, 321, -211) == 1) {
    return -1 * (100000 * nPhotos + 1139);
  }//anti-B_s0 decays to eta_c anti-K0 K+ pi-
  if (GenBsTag::PcheckDecay(genpart, 441, -311, 311, 111) == 1) {
    return -1 * (100000 * nPhotos + 1140);
  }//anti-B_s0 decays to eta_c anti-K0 K0 pi0
  if (GenBsTag::PcheckDecay(genpart, 441, -321, 321, 111) == 1) {
    return -1 * (100000 * nPhotos + 1141);
  }//anti-B_s0 decays to eta_c K- K+ pi0
  if (GenBsTag::PcheckDecay(genpart, 441, 333, 211, -211) == 1) {
    return -1 * (100000 * nPhotos + 1142);
  }//anti-B_s0 decays to eta_c phi pi+ pi-
  if (GenBsTag::PcheckDecay(genpart, 441, 333, 111, 111) == 1) {
    return -1 * (100000 * nPhotos + 1143);
  }//anti-B_s0 decays to eta_c phi pi0 pi0
  if (GenBsTag::PcheckDecay(genpart, 441, 221, 211, -211) == 1) {
    return -1 * (100000 * nPhotos + 1144);
  }//anti-B_s0 decays to eta_c eta pi+ pi-
  if (GenBsTag::PcheckDecay(genpart, 441, 221, 111, 111) == 1) {
    return -1 * (100000 * nPhotos + 1145);
  }//anti-B_s0 decays to eta_c eta pi0 pi0
  if (GenBsTag::PcheckDecay(genpart, 441, 331, 211, -211) == 1) {
    return -1 * (100000 * nPhotos + 1146);
  }//anti-B_s0 decays to eta_c eta' pi+ pi-
  if (GenBsTag::PcheckDecay(genpart, 441, 331, 111, 111) == 1) {
    return -1 * (100000 * nPhotos + 1147);
  }//anti-B_s0 decays to eta_c eta' pi0 pi0
  if (GenBsTag::PcheckDecay(genpart, 100441, 331) == 1) {
    return -1 * (100000 * nPhotos + 1148);
  }//anti-B_s0 decays to eta_c(2S) eta'
  if (GenBsTag::PcheckDecay(genpart, 100441, 221) == 1) {
    return -1 * (100000 * nPhotos + 1149);
  }//anti-B_s0 decays to eta_c(2S) eta
  if (GenBsTag::PcheckDecay(genpart, 333, 100441) == 1) {
    return -1 * (100000 * nPhotos + 1150);
  }//anti-B_s0 decays to phi eta_c(2S)
  if (GenBsTag::PcheckDecay(genpart, 100441, -321, 321) == 1) {
    return -1 * (100000 * nPhotos + 1151);
  }//anti-B_s0 decays to eta_c(2S) K- K+
  if (GenBsTag::PcheckDecay(genpart, 100441, -311, 311) == 1) {
    return -1 * (100000 * nPhotos + 1152);
  }//anti-B_s0 decays to eta_c(2S) anti-K0 K0
  if (GenBsTag::PcheckDecay(genpart, 100441, -311, 321, -211) == 1) {
    return -1 * (100000 * nPhotos + 1153);
  }//anti-B_s0 decays to eta_c(2S) anti-K0 K+ pi-
  if (GenBsTag::PcheckDecay(genpart, 100441, -311, 311, 111) == 1) {
    return -1 * (100000 * nPhotos + 1154);
  }//anti-B_s0 decays to eta_c(2S) anti-K0 K0 pi0
  if (GenBsTag::PcheckDecay(genpart, 100441, -321, 321, 111) == 1) {
    return -1 * (100000 * nPhotos + 1155);
  }//anti-B_s0 decays to eta_c(2S) K- K+ pi0
  if (GenBsTag::PcheckDecay(genpart, 10443, 331) == 1) {
    return -1 * (100000 * nPhotos + 1156);
  }//anti-B_s0 decays to h_c eta'
  if (GenBsTag::PcheckDecay(genpart, 10443, 221) == 1) {
    return -1 * (100000 * nPhotos + 1157);
  }//anti-B_s0 decays to h_c eta
  if (GenBsTag::PcheckDecay(genpart, 10443, 333) == 1) {
    return -1 * (100000 * nPhotos + 1158);
  }//anti-B_s0 decays to h_c phi
  if (GenBsTag::PcheckDecay(genpart, 10443, -321, 321) == 1) {
    return -1 * (100000 * nPhotos + 1159);
  }//anti-B_s0 decays to h_c K- K+
  if (GenBsTag::PcheckDecay(genpart, 10443, -311, 311) == 1) {
    return -1 * (100000 * nPhotos + 1160);
  }//anti-B_s0 decays to h_c anti-K0 K0
  if (GenBsTag::PcheckDecay(genpart, 10443, -311, 321, -211) == 1) {
    return -1 * (100000 * nPhotos + 1161);
  }//anti-B_s0 decays to h_c anti-K0 K+ pi-
  if (GenBsTag::PcheckDecay(genpart, 10443, -311, 311, 111) == 1) {
    return -1 * (100000 * nPhotos + 1162);
  }//anti-B_s0 decays to h_c anti-K0 K0 pi0
  if (GenBsTag::PcheckDecay(genpart, 10443, -321, 321, 111) == 1) {
    return -1 * (100000 * nPhotos + 1163);
  }//anti-B_s0 decays to h_c K- K+ pi0
  if (GenBsTag::PcheckDecay(genpart, 443, 333, 333) == 1) {
    return -1 * (100000 * nPhotos + 1164);
  }//anti-B_s0 decays to J/psi phi phi
  if (GenBsTag::PcheckDecay(genpart, 443, 310) == 1) {
    return -1 * (100000 * nPhotos + 1165);
  }//anti-B_s0 decays to J/psi K_S0
  if (GenBsTag::PcheckDecay(genpart, 443, -313) == 1) {
    return -1 * (100000 * nPhotos + 1166);
  }//anti-B_s0 decays to J/psi anti-K*0
  if (GenBsTag::PcheckDecay(genpart, 443, 211, -211, 211, -211) == 1) {
    return -1 * (100000 * nPhotos + 1167);
  }//anti-B_s0 decays to J/psi pi+ pi- pi+ pi-
  if (GenBsTag::PcheckDecay(genpart, 443, 20223) == 1) {
    return -1 * (100000 * nPhotos + 1168);
  }//anti-B_s0 decays to J/psi f_1
  if (GenBsTag::PcheckDecay(genpart, 100443, -321, 211) == 1) {
    return -1 * (100000 * nPhotos + 1169);
  }//anti-B_s0 decays to psi(2S) K- pi+
  if (GenBsTag::PcheckDecay(genpart, 100443, -313) == 1) {
    return -1 * (100000 * nPhotos + 1170);
  }//anti-B_s0 decays to psi(2S) anti-K*0
  if (GenBsTag::PcheckDecay(genpart, 10433, -211) == 1) {
    return -1 * (100000 * nPhotos + 1171);
  }//anti-B_s0 decays to D_s1+ pi-
  if (GenBsTag::PcheckDecay(genpart, 10433, -213) == 1) {
    return -1 * (100000 * nPhotos + 1172);
  }//anti-B_s0 decays to D_s1+ rho-
  if (GenBsTag::PcheckDecay(genpart, 435, -211) == 1) {
    return -1 * (100000 * nPhotos + 1173);
  }//anti-B_s0 decays to D_s2*+ pi-
  if (GenBsTag::PcheckDecay(genpart, 433, -211) == 1) {
    return -1 * (100000 * nPhotos + 1174);
  }//anti-B_s0 decays to D_s*+ pi-
  if (GenBsTag::PcheckDecay(genpart, 431, -211) == 1) {
    return -1 * (100000 * nPhotos + 1175);
  }//anti-B_s0 decays to D_s+ pi-
  if (GenBsTag::PcheckDecay(genpart, -213, 431) == 1) {
    return -1 * (100000 * nPhotos + 1176);
  }//anti-B_s0 decays to rho- D_s+
  if (GenBsTag::PcheckDecay(genpart, 433, -213) == 1) {
    return -1 * (100000 * nPhotos + 1177);
  }//anti-B_s0 decays to D_s*+ rho-
  if (GenBsTag::PcheckDecay(genpart, -20213, 431) == 1) {
    return -1 * (100000 * nPhotos + 1178);
  }//anti-B_s0 decays to a_1- D_s+
  if (GenBsTag::PcheckDecay(genpart, 431, 113, -211) == 1) {
    return -1 * (100000 * nPhotos + 1179);
  }//anti-B_s0 decays to D_s+ rho0 pi-
  if (GenBsTag::PcheckDecay(genpart, 431, -213, 111) == 1) {
    return -1 * (100000 * nPhotos + 1180);
  }//anti-B_s0 decays to D_s+ rho- pi0
  if (GenBsTag::PcheckDecay(genpart, 431, 211, -211, -211) == 1) {
    return -1 * (100000 * nPhotos + 1181);
  }//anti-B_s0 decays to D_s+ pi+ pi- pi-
  if (GenBsTag::PcheckDecay(genpart, 431, 111, -211, 111) == 1) {
    return -1 * (100000 * nPhotos + 1182);
  }//anti-B_s0 decays to D_s+ pi0 pi- pi0
  if (GenBsTag::PcheckDecay(genpart, 433, -20213) == 1) {
    return -1 * (100000 * nPhotos + 1183);
  }//anti-B_s0 decays to D_s*+ a_1-
  if (GenBsTag::PcheckDecay(genpart, 433, 113, -211) == 1) {
    return -1 * (100000 * nPhotos + 1184);
  }//anti-B_s0 decays to D_s*+ rho0 pi-
  if (GenBsTag::PcheckDecay(genpart, 433, -213, 111) == 1) {
    return -1 * (100000 * nPhotos + 1185);
  }//anti-B_s0 decays to D_s*+ rho- pi0
  if (GenBsTag::PcheckDecay(genpart, 433, 211, -211, -211) == 1) {
    return -1 * (100000 * nPhotos + 1186);
  }//anti-B_s0 decays to D_s*+ pi+ pi- pi-
  if (GenBsTag::PcheckDecay(genpart, 433, 111, -211, 111) == 1) {
    return -1 * (100000 * nPhotos + 1187);
  }//anti-B_s0 decays to D_s*+ pi0 pi- pi0
  if (GenBsTag::PcheckDecay(genpart, 423, 311) == 1) {
    return -1 * (100000 * nPhotos + 1188);
  }//anti-B_s0 decays to D*0 K0
  if (GenBsTag::PcheckDecay(genpart, 421, 311) == 1) {
    return -1 * (100000 * nPhotos + 1189);
  }//anti-B_s0 decays to D0 K0
  if (GenBsTag::PcheckDecay(genpart, 313, 421) == 1) {
    return -1 * (100000 * nPhotos + 1190);
  }//anti-B_s0 decays to K*0 D0
  if (GenBsTag::PcheckDecay(genpart, 423, 313) == 1) {
    return -1 * (100000 * nPhotos + 1191);
  }//anti-B_s0 decays to D*0 K*0
  if (GenBsTag::PcheckDecay(genpart, 2212, -2212) == 1) {
    return -1 * (100000 * nPhotos + 1192);
  }//anti-B_s0 decays to p+ anti-p-
  if (GenBsTag::PcheckDecay(genpart, 4301, -2101) == 1) {
    return -1 * (100000 * nPhotos + 1193);
  }//anti-B_s0 decays to cs_0 anti-ud_0
  if (GenBsTag::PcheckDecay(genpart, 4303, -2103) == 1) {
    return -1 * (100000 * nPhotos + 1194);
  }//anti-B_s0 decays to cs_1 anti-ud_1
  if (GenBsTag::PcheckDecay(genpart, -2, 1, 4, -3) == 1) {
    return -1 * (100000 * nPhotos + 1195);
  }//anti-B_s0 decays to anti-u d c anti-s
  if (GenBsTag::PcheckDecay(genpart, -2, 1, 4, -3) == 1) {
    return -1 * (100000 * nPhotos + 1196);
  }//anti-B_s0 decays to anti-u d c anti-s
  if (GenBsTag::PcheckDecay(genpart, -2, 4, 1, -3) == 1) {
    return -1 * (100000 * nPhotos + 1197);
  }//anti-B_s0 decays to anti-u c d anti-s
  if (GenBsTag::PcheckDecay(genpart, -4, 3, 2, -3) == 1) {
    return -1 * (100000 * nPhotos + 1198);
  }//anti-B_s0 decays to anti-c s u anti-s
  if (GenBsTag::PcheckDecay(genpart, -2, 3, 4, -3) == 1) {
    return -1 * (100000 * nPhotos + 1199);
  }//anti-B_s0 decays to anti-u s c anti-s
  if (GenBsTag::PcheckDecay(genpart, -2, 3, 4, -3) == 1) {
    return -1 * (100000 * nPhotos + 1200);
  }//anti-B_s0 decays to anti-u s c anti-s
  if (GenBsTag::PcheckDecay(genpart, -2, 4, 3, -3) == 1) {
    return -1 * (100000 * nPhotos + 1201);
  }//anti-B_s0 decays to anti-u c s anti-s
  if (GenBsTag::PcheckDecay(genpart, -4, 1, 4, -3) == 1) {
    return -1 * (100000 * nPhotos + 1202);
  }//anti-B_s0 decays to anti-c d c anti-s
  if (GenBsTag::PcheckDecay(genpart, -4, 1, 4, -3) == 1) {
    return -1 * (100000 * nPhotos + 1203);
  }//anti-B_s0 decays to anti-c d c anti-s
  if (GenBsTag::PcheckDecay(genpart, -2, 2, 1, -3) == 1) {
    return -1 * (100000 * nPhotos + 1204);
  }//anti-B_s0 decays to anti-u u d anti-s
  if (GenBsTag::PcheckDecay(genpart, -1, 1, 1, -3) == 1) {
    return -1 * (100000 * nPhotos + 1205);
  }//anti-B_s0 decays to anti-d d d anti-s
  if (GenBsTag::PcheckDecay(genpart, -3, 3, 1, -3) == 1) {
    return -1 * (100000 * nPhotos + 1206);
  }//anti-B_s0 decays to anti-s s d anti-s
  if (GenBsTag::PcheckDecay(genpart, -2, 2, 3, -3) == 1) {
    return -1 * (100000 * nPhotos + 1207);
  }//anti-B_s0 decays to anti-u u s anti-s
  if (GenBsTag::PcheckDecay(genpart, -1, 1, 3, -3) == 1) {
    return -1 * (100000 * nPhotos + 1208);
  }//anti-B_s0 decays to anti-d d s anti-s
  if (GenBsTag::PcheckDecay(genpart, -3, 3, 3, -3) == 1) {
    return -1 * (100000 * nPhotos + 1209);
  }//anti-B_s0 decays to anti-s s s anti-s
  if (GenBsTag::PcheckDecay(genpart, 3, -3) == 1) {
    return -1 * (100000 * nPhotos + 1210);
  }//anti-B_s0 decays to s anti-s
  if (GenBsTag::PcheckDecay(genpart, 321, 11, -12) == 1) {
    return -1 * (100000 * nPhotos + 1211);
  }//anti-B_s0 decays to K+ e- anti-nu_e
  if (GenBsTag::PcheckDecay(genpart, 323, 11, -12) == 1) {
    return -1 * (100000 * nPhotos + 1212);
  }//anti-B_s0 decays to K*+ e- anti-nu_e
  if (GenBsTag::PcheckDecay(genpart, 10323, 11, -12) == 1) {
    return -1 * (100000 * nPhotos + 1213);
  }//anti-B_s0 decays to K_1+ e- anti-nu_e
  if (GenBsTag::PcheckDecay(genpart, 20323, 11, -12) == 1) {
    return -1 * (100000 * nPhotos + 1214);
  }//anti-B_s0 decays to K'_1+ e- anti-nu_e
  if (GenBsTag::PcheckDecay(genpart, 321, 13, -14) == 1) {
    return -1 * (100000 * nPhotos + 1215);
  }//anti-B_s0 decays to K+ mu- anti-nu_mu
  if (GenBsTag::PcheckDecay(genpart, 323, 13, -14) == 1) {
    return -1 * (100000 * nPhotos + 1216);
  }//anti-B_s0 decays to K*+ mu- anti-nu_mu
  if (GenBsTag::PcheckDecay(genpart, 10323, 13, -14) == 1) {
    return -1 * (100000 * nPhotos + 1217);
  }//anti-B_s0 decays to K_1+ mu- anti-nu_mu
  if (GenBsTag::PcheckDecay(genpart, 20323, 13, -14) == 1) {
    return -1 * (100000 * nPhotos + 1218);
  }//anti-B_s0 decays to K'_1+ mu- anti-nu_mu
  if (GenBsTag::PcheckDecay(genpart, -321, 321) == 1) {
    return -1 * (100000 * nPhotos + 1219);
  }//anti-B_s0 decays to K- K+
  if (GenBsTag::PcheckDecay(genpart, -311, 311) == 1) {
    return -1 * (100000 * nPhotos + 1220);
  }//anti-B_s0 decays to anti-K0 K0
  if (GenBsTag::PcheckDecay(genpart, 310, 310) == 1) {
    return -1 * (100000 * nPhotos + 1221);
  }//anti-B_s0 decays to K_S0 K_S0
  if (GenBsTag::PcheckDecay(genpart, 130, 130) == 1) {
    return -1 * (100000 * nPhotos + 1222);
  }//anti-B_s0 decays to K_L0 K_L0
  if (GenBsTag::PcheckDecay(genpart, -211, 321) == 1) {
    return -1 * (100000 * nPhotos + 1223);
  }//anti-B_s0 decays to pi- K+
  if (GenBsTag::PcheckDecay(genpart, 111, 311) == 1) {
    return -1 * (100000 * nPhotos + 1224);
  }//anti-B_s0 decays to pi0 K0
  if (GenBsTag::PcheckDecay(genpart, 211, -211) == 1) {
    return -1 * (100000 * nPhotos + 1225);
  }//anti-B_s0 decays to pi+ pi-
  if (GenBsTag::PcheckDecay(genpart, 223, 221) == 1) {
    return -1 * (100000 * nPhotos + 1226);
  }//anti-B_s0 decays to omega eta
  if (GenBsTag::PcheckDecay(genpart, 223, 331) == 1) {
    return -1 * (100000 * nPhotos + 1227);
  }//anti-B_s0 decays to omega eta'
  if (GenBsTag::PcheckDecay(genpart, 333, 221) == 1) {
    return -1 * (100000 * nPhotos + 1228);
  }//anti-B_s0 decays to phi eta
  if (GenBsTag::PcheckDecay(genpart, 333, 331) == 1) {
    return -1 * (100000 * nPhotos + 1229);
  }//anti-B_s0 decays to phi eta'
  if (GenBsTag::PcheckDecay(genpart, 223, -311) == 1) {
    return -1 * (100000 * nPhotos + 1230);
  }//anti-B_s0 decays to omega anti-K0
  if (GenBsTag::PcheckDecay(genpart, 333, -311) == 1) {
    return -1 * (100000 * nPhotos + 1231);
  }//anti-B_s0 decays to phi anti-K0
  if (GenBsTag::PcheckDecay(genpart, 323, -211) == 1) {
    return -1 * (100000 * nPhotos + 1232);
  }//anti-B_s0 decays to K*+ pi-
  if (GenBsTag::PcheckDecay(genpart, -213, 321) == 1) {
    return -1 * (100000 * nPhotos + 1233);
  }//anti-B_s0 decays to rho- K+
  if (GenBsTag::PcheckDecay(genpart, 113, 311) == 1) {
    return -1 * (100000 * nPhotos + 1234);
  }//anti-B_s0 decays to rho0 K0
  if (GenBsTag::PcheckDecay(genpart, 313, 111) == 1) {
    return -1 * (100000 * nPhotos + 1235);
  }//anti-B_s0 decays to K*0 pi0
  if (GenBsTag::PcheckDecay(genpart, -323, 321) == 1) {
    return -1 * (100000 * nPhotos + 1236);
  }//anti-B_s0 decays to K*- K+
  if (GenBsTag::PcheckDecay(genpart, -313, 311) == 1) {
    return -1 * (100000 * nPhotos + 1237);
  }//anti-B_s0 decays to anti-K*0 K0
  if (GenBsTag::PcheckDecay(genpart, -313, 310) == 1) {
    return -1 * (100000 * nPhotos + 1238);
  }//anti-B_s0 decays to anti-K*0 K_S0
  if (GenBsTag::PcheckDecay(genpart, -313, 130) == 1) {
    return -1 * (100000 * nPhotos + 1239);
  }//anti-B_s0 decays to anti-K*0 K_L0
  if (GenBsTag::PcheckDecay(genpart, 323, -321) == 1) {
    return -1 * (100000 * nPhotos + 1240);
  }//anti-B_s0 decays to K*+ K-
  if (GenBsTag::PcheckDecay(genpart, 313, -311) == 1) {
    return -1 * (100000 * nPhotos + 1241);
  }//anti-B_s0 decays to K*0 anti-K0
  if (GenBsTag::PcheckDecay(genpart, 313, 310) == 1) {
    return -1 * (100000 * nPhotos + 1242);
  }//anti-B_s0 decays to K*0 K_S0
  if (GenBsTag::PcheckDecay(genpart, 313, 130) == 1) {
    return -1 * (100000 * nPhotos + 1243);
  }//anti-B_s0 decays to K*0 K_L0
  if (GenBsTag::PcheckDecay(genpart, 331, 331) == 1) {
    return -1 * (100000 * nPhotos + 1244);
  }//anti-B_s0 decays to eta' eta'
  if (GenBsTag::PcheckDecay(genpart, 331, 221) == 1) {
    return -1 * (100000 * nPhotos + 1245);
  }//anti-B_s0 decays to eta' eta
  if (GenBsTag::PcheckDecay(genpart, 221, 221) == 1) {
    return -1 * (100000 * nPhotos + 1246);
  }//anti-B_s0 decays to eta eta
  if (GenBsTag::PcheckDecay(genpart, 311, 331) == 1) {
    return -1 * (100000 * nPhotos + 1247);
  }//anti-B_s0 decays to K0 eta'
  if (GenBsTag::PcheckDecay(genpart, 311, 221) == 1) {
    return -1 * (100000 * nPhotos + 1248);
  }//anti-B_s0 decays to K0 eta
  if (GenBsTag::PcheckDecay(genpart, 313, 331) == 1) {
    return -1 * (100000 * nPhotos + 1249);
  }//anti-B_s0 decays to K*0 eta'
  if (GenBsTag::PcheckDecay(genpart, 313, 221) == 1) {
    return -1 * (100000 * nPhotos + 1250);
  }//anti-B_s0 decays to K*0 eta
  if (GenBsTag::PcheckDecay(genpart, -213, 323) == 1) {
    return -1 * (100000 * nPhotos + 1251);
  }//anti-B_s0 decays to rho- K*+
  if (GenBsTag::PcheckDecay(genpart, 113, 313) == 1) {
    return -1 * (100000 * nPhotos + 1252);
  }//anti-B_s0 decays to rho0 K*0
  if (GenBsTag::PcheckDecay(genpart, 223, 313) == 1) {
    return -1 * (100000 * nPhotos + 1253);
  }//anti-B_s0 decays to omega K*0
  if (GenBsTag::PcheckDecay(genpart, -323, 323) == 1) {
    return -1 * (100000 * nPhotos + 1254);
  }//anti-B_s0 decays to K*- K*+
  if (GenBsTag::PcheckDecay(genpart, -313, 313) == 1) {
    return -1 * (100000 * nPhotos + 1255);
  }//anti-B_s0 decays to anti-K*0 K*0
  if (GenBsTag::PcheckDecay(genpart, 333, 333) == 1) {
    return -1 * (100000 * nPhotos + 1256);
  }//anti-B_s0 decays to phi phi
  if (GenBsTag::PcheckDecay(genpart, 333, 313) == 1) {
    return -1 * (100000 * nPhotos + 1257);
  }//anti-B_s0 decays to phi K*0
  if (GenBsTag::PcheckDecay(genpart, 13, -13) == 1) {
    return -1 * (100000 * nPhotos + 1258);
  }//anti-B_s0 decays to mu- mu+
  if (GenBsTag::PcheckDecay(genpart, 15, -15) == 1) {
    return -1 * (100000 * nPhotos + 1259);
  }//anti-B_s0 decays to tau- tau+
  if (GenBsTag::PcheckDecay(genpart, 333, 22) == 1) {
    return -1 * (100000 * nPhotos + 1260);
  }//anti-B_s0 decays to phi gamma
  if (GenBsTag::PcheckDecay(genpart, 333, 11, -11) == 1) {
    return -1 * (100000 * nPhotos + 1261);
  }//anti-B_s0 decays to phi e- e+
  if (GenBsTag::PcheckDecay(genpart, 333, 13, -13) == 1) {
    return -1 * (100000 * nPhotos + 1262);
  }//anti-B_s0 decays to phi mu- mu+
  if (GenBsTag::PcheckDecay(genpart, 22, 22) == 1) {
    return -1 * (100000 * nPhotos + 1263);
  }//anti-B_s0 decays to gamma gamma
  if (GenBsTag::PcheckDecay(genpart, -431, 321) == 1) {
    return -1 * (100000 * nPhotos + 1264);
  }//anti-B_s0 decays to D_s- K+
  return -1 * (DauSize(genpart));
}// Rest of the anti-B_s0 decays
