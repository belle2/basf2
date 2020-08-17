/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Sourav Patra, Vishal Bhardwaj                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <analysis/utility/GenTauTag.h>
using namespace Belle2;

int GenTauTag::Array_CheckDecay(std::vector<int> MCDAU,
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


int GenTauTag::CheckDecay(std::vector<int>MCDAU, int a0 = -987656789,
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
  return  GenTauTag::Array_CheckDecay(MCDAU, genDAU);
}


int GenTauTag::DauSize(std::vector<int>genpart)
{
  return genpart.size();
}


int GenTauTag::PcheckDecay(std::vector<int>gp, int da1, int da2)
{
  int ndau = DauSize(gp);
  if (ndau == 2) { nPhotos = 0; return CheckDecay(gp, da1, da2);}
  else if (ndau == 3) { nPhotos = 1; return CheckDecay(gp, da1, da2, 22);}
  else if (ndau == 4) { nPhotos = 2; return CheckDecay(gp, da1, da2, 22, 22);}
  else if (ndau == 5) { nPhotos = 3; return CheckDecay(gp, da1, da2, 22, 22, 22);}
  else if (ndau == 6) { nPhotos = 4; return CheckDecay(gp, da1, da2, 22, 22, 22, 22);}
  else return -999;
}


int GenTauTag::PcheckDecay(std::vector<int>gp, int da1, int da2, int da3)
{
  int ndau = DauSize(gp);
  if (ndau == 3) { nPhotos = 0; return CheckDecay(gp, da1, da2, da3);}
  else if (ndau == 4) { nPhotos = 1; return CheckDecay(gp, da1, da2, da3, 22);}
  else if (ndau == 5) { nPhotos = 2; return CheckDecay(gp, da1, da2, da3, 22, 22);}
  else if (ndau == 6) { nPhotos = 3; return CheckDecay(gp, da1, da2, da3, 22, 22, 22);}
  else if (ndau == 7) { nPhotos = 4; return CheckDecay(gp, da1, da2, da3, 22, 22, 22, 22);}
  else return -999;
}


int GenTauTag::PcheckDecay(std::vector<int>gp, int da1, int da2, int da3, int da4)
{
  int ndau = DauSize(gp);
  if (ndau == 4) { nPhotos = 0; return CheckDecay(gp, da1, da2, da3, da4);}
  else if (ndau == 5) { nPhotos = 1; return CheckDecay(gp, da1, da2, da3, da4, 22);}
  else if (ndau == 6) { nPhotos = 2; return CheckDecay(gp, da1, da2, da3, da4, 22, 22);}
  else if (ndau == 7) { nPhotos = 3; return CheckDecay(gp, da1, da2, da3, da4, 22, 22, 22);}
  else if (ndau == 8) { nPhotos = 4; return CheckDecay(gp, da1, da2, da3, da4, 22, 22, 22, 22);}
  else return -999;
}


int GenTauTag::PcheckDecay(std::vector<int>gp, int da1, int da2, int da3, int da4, int da5)
{
  int ndau = DauSize(gp);
  if (ndau == 5) { nPhotos = 0; return CheckDecay(gp, da1, da2, da3, da4, da5);}
  else if (ndau == 6) { nPhotos = 1; return CheckDecay(gp, da1, da2, da3, da4, da5, 22);}
  else if (ndau == 7) { nPhotos = 2; return CheckDecay(gp, da1, da2, da3, da4, da5, 22, 22);}
  else if (ndau == 8) { nPhotos = 3; return CheckDecay(gp, da1, da2, da3, da4, da5, 22, 22, 22);}
  else if (ndau == 9) { nPhotos = 4; return CheckDecay(gp, da1, da2, da3, da4, da5, 22, 22, 22, 22);}
  else return -999;
}


int GenTauTag::PcheckDecay(std::vector<int>gp, int da1, int da2, int da3, int da4, int da5,
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


int GenTauTag::PcheckDecay(std::vector<int>gp, int da1, int da2, int da3, int da4, int da5,
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


int GenTauTag::PcheckDecay(std::vector<int>gp, int da1, int da2, int da3, int da4, int da5,
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


int GenTauTag::Mode_tau_plus(std::vector<int>genpart)
{
  if (GenTauTag::PcheckDecay(genpart, -11, 12, -16) == 1) {
    return +1 * (100000 * nPhotos + 1001);
  }//tau+ decays to e+ nu_e anti-nu_tau
  if (GenTauTag::PcheckDecay(genpart, -13, 14, -16) == 1) {
    return +1 * (100000 * nPhotos + 1002);
  }//tau+ decays to mu+ nu_mu anti-nu_tau
  if (GenTauTag::PcheckDecay(genpart, 211, -16) == 1) {
    return +1 * (100000 * nPhotos + 1003);
  }//tau+ decays to pi+ anti-nu_tau
  if (GenTauTag::PcheckDecay(genpart, 321, -16) == 1) {
    return +1 * (100000 * nPhotos + 1004);
  }//tau+ decays to K+ anti-nu_tau
  if (GenTauTag::PcheckDecay(genpart, 211, 111, -16) == 1) {
    return +1 * (100000 * nPhotos + 1005);
  }//tau+ decays to pi+ pi0 anti-nu_tau
  if (GenTauTag::PcheckDecay(genpart, -16, 22, 211, 111) == 1) {
    return +1 * (100000 * nPhotos + 1006);
  }//tau+ decays to anti-nu_tau gamma pi+ pi0
  if (GenTauTag::PcheckDecay(genpart, 111, 111, 211, -16) == 1) {
    return +1 * (100000 * nPhotos + 1007);
  }//tau+ decays to pi0 pi0 pi+ anti-nu_tau
  if (GenTauTag::PcheckDecay(genpart, -16, 321, 111, 111) == 1) {
    return +1 * (100000 * nPhotos + 1008);
  }//tau+ decays to anti-nu_tau K+ pi0 pi0
  if (GenTauTag::PcheckDecay(genpart, -16, 211, 111, 111, 111) == 1) {
    return +1 * (100000 * nPhotos + 1009);
  }//tau+ decays to anti-nu_tau pi+ pi0 pi0 pi0
  if (GenTauTag::PcheckDecay(genpart, -16, 211, 111, 111, 111, 111) == 1) {
    return +1 * (100000 * nPhotos + 1010);
  }//tau+ decays to anti-nu_tau pi+ pi0 pi0 pi0 pi0
  if (GenTauTag::PcheckDecay(genpart, -16, 211, 311) == 1) {
    return +1 * (100000 * nPhotos + 1011);
  }//tau+ decays to anti-nu_tau pi+ K0
  if (GenTauTag::PcheckDecay(genpart, -16, 321, -311) == 1) {
    return +1 * (100000 * nPhotos + 1012);
  }//tau+ decays to anti-nu_tau K+ anti-K0
  if (GenTauTag::PcheckDecay(genpart, -16, -311, 211, 311) == 1) {
    return +1 * (100000 * nPhotos + 1013);
  }//tau+ decays to anti-nu_tau anti-K0 pi+ K0
  if (GenTauTag::PcheckDecay(genpart, -16, 321, 111, -311) == 1) {
    return +1 * (100000 * nPhotos + 1014);
  }//tau+ decays to anti-nu_tau K+ pi0 anti-K0
  if (GenTauTag::PcheckDecay(genpart, -16, 211, 311, 111) == 1) {
    return +1 * (100000 * nPhotos + 1015);
  }//tau+ decays to anti-nu_tau pi+ K0 pi0
  if (GenTauTag::PcheckDecay(genpart, 211, 211, -211, -16) == 1) {
    return +1 * (100000 * nPhotos + 1016);
  }//tau+ decays to pi+ pi+ pi- anti-nu_tau
  if (GenTauTag::PcheckDecay(genpart, -16, 211, -211, 211, 111) == 1) {
    return +1 * (100000 * nPhotos + 1017);
  }//tau+ decays to anti-nu_tau pi+ pi- pi+ pi0
  if (GenTauTag::PcheckDecay(genpart, -16, 211, 211, -211, 111, 111) == 1) {
    return +1 * (100000 * nPhotos + 1018);
  }//tau+ decays to anti-nu_tau pi+ pi+ pi- pi0 pi0
  if (GenTauTag::PcheckDecay(genpart, -16, 211, 211, -211, 111, 111, 111) == 1) {
    return +1 * (100000 * nPhotos + 1019);
  }//tau+ decays to anti-nu_tau pi+ pi+ pi- pi0 pi0 pi0
  if (GenTauTag::PcheckDecay(genpart, -16, 321, -211, 211) == 1) {
    return +1 * (100000 * nPhotos + 1020);
  }//tau+ decays to anti-nu_tau K+ pi- pi+
  if (GenTauTag::PcheckDecay(genpart, -16, 321, -211, 211, 111) == 1) {
    return +1 * (100000 * nPhotos + 1021);
  }//tau+ decays to anti-nu_tau K+ pi- pi+ pi0
  if (GenTauTag::PcheckDecay(genpart, -16, 321, 211, -321) == 1) {
    return +1 * (100000 * nPhotos + 1022);
  }//tau+ decays to anti-nu_tau K+ pi+ K-
  if (GenTauTag::PcheckDecay(genpart, -16, 321, -321, 321) == 1) {
    return +1 * (100000 * nPhotos + 1023);
  }//tau+ decays to anti-nu_tau K+ K- K+
  if (GenTauTag::PcheckDecay(genpart, -16, 211, 211, 211, -211, -211) == 1) {
    return +1 * (100000 * nPhotos + 1024);
  }//tau+ decays to anti-nu_tau pi+ pi+ pi+ pi- pi-
  if (GenTauTag::PcheckDecay(genpart, -16, 211, 211, 211, -211, -211, 111) == 1) {
    return +1 * (100000 * nPhotos + 1025);
  }//tau+ decays to anti-nu_tau pi+ pi+ pi+ pi- pi- pi0
  if (GenTauTag::PcheckDecay(genpart, 323, -16) == 1) {
    return +1 * (100000 * nPhotos + 1026);
  }//tau+ decays to K*+ anti-nu_tau
  if (GenTauTag::PcheckDecay(genpart, -16, 221, 211, 111) == 1) {
    return +1 * (100000 * nPhotos + 1027);
  }//tau+ decays to anti-nu_tau eta pi+ pi0
  if (GenTauTag::PcheckDecay(genpart, -16, 211, 223, 111) == 1) {
    return +1 * (100000 * nPhotos + 1028);
  }//tau+ decays to anti-nu_tau pi+ omega pi0
  if (GenTauTag::PcheckDecay(genpart, -16, 321, 223) == 1) {
    return +1 * (100000 * nPhotos + 1029);
  }//tau+ decays to anti-nu_tau K+ omega
  if (GenTauTag::PcheckDecay(genpart, 313, 211, -16) == 1) {
    return +1 * (100000 * nPhotos + 1030);
  }//tau+ decays to K*0 pi+ anti-nu_tau
  if (GenTauTag::PcheckDecay(genpart, -313, 321, -16) == 1) {
    return +1 * (100000 * nPhotos + 1031);
  }//tau+ decays to anti-K*0 K+ anti-nu_tau
  if (GenTauTag::PcheckDecay(genpart, 333, 211, -16) == 1) {
    return +1 * (100000 * nPhotos + 1032);
  }//tau+ decays to phi pi+ anti-nu_tau
  if (GenTauTag::PcheckDecay(genpart, 333, 321, -16) == 1) {
    return +1 * (100000 * nPhotos + 1033);
  }//tau+ decays to phi K+ anti-nu_tau
  if (GenTauTag::PcheckDecay(genpart, -13, 14, -16, 22) == 1) {
    return +1 * (100000 * nPhotos + 1034);
  }//tau+ decays to mu+ nu_mu anti-nu_tau gamma
  if (GenTauTag::PcheckDecay(genpart, -11, 12, -16, 22) == 1) {
    return +1 * (100000 * nPhotos + 1035);
  }//tau+ decays to e+ nu_e anti-nu_tau gamma
  if (GenTauTag::PcheckDecay(genpart, 321, 111, -16) == 1) {
    return +1 * (100000 * nPhotos + 1036);
  }//tau+ decays to K+ pi0 anti-nu_tau
  if (GenTauTag::PcheckDecay(genpart, 311, 213, -16) == 1) {
    return +1 * (100000 * nPhotos + 1037);
  }//tau+ decays to K0 rho+ anti-nu_tau
  if (GenTauTag::PcheckDecay(genpart, 211, 311, 111, 111, -16) == 1) {
    return +1 * (100000 * nPhotos + 1038);
  }//tau+ decays to pi+ K0 pi0 pi0 anti-nu_tau
  if (GenTauTag::PcheckDecay(genpart, 211, -311, 311, 111, -16) == 1) {
    return +1 * (100000 * nPhotos + 1039);
  }//tau+ decays to pi+ anti-K0 K0 pi0 anti-nu_tau
  if (GenTauTag::PcheckDecay(genpart, 321, -321, 211, 111, -16) == 1) {
    return +1 * (100000 * nPhotos + 1040);
  }//tau+ decays to K+ K- pi+ pi0 anti-nu_tau
  if (GenTauTag::PcheckDecay(genpart, -11, -11, 11, 12, -16) == 1) {
    return +1 * (100000 * nPhotos + 1041);
  }//tau+ decays to e+ e+ e- nu_e anti-nu_tau
  if (GenTauTag::PcheckDecay(genpart, 10323, -16) == 1) {
    return +1 * (100000 * nPhotos + 1042);
  }//tau+ decays to K_1+ anti-nu_tau
  if (GenTauTag::PcheckDecay(genpart, 20323, -16) == 1) {
    return +1 * (100000 * nPhotos + 1043);
  }//tau+ decays to K'_1+ anti-nu_tau
  if (GenTauTag::PcheckDecay(genpart, 100323, -16) == 1) {
    return +1 * (100000 * nPhotos + 1044);
  }//tau+ decays to K'*+ anti-nu_tau
  if (GenTauTag::PcheckDecay(genpart, 221, 211, 111, 111, -16) == 1) {
    return +1 * (100000 * nPhotos + 1045);
  }//tau+ decays to eta pi+ pi0 pi0 anti-nu_tau
  if (GenTauTag::PcheckDecay(genpart, 221, 321, -16) == 1) {
    return +1 * (100000 * nPhotos + 1046);
  }//tau+ decays to eta K+ anti-nu_tau
  if (GenTauTag::PcheckDecay(genpart, 221, 323, -16) == 1) {
    return +1 * (100000 * nPhotos + 1047);
  }//tau+ decays to eta K*+ anti-nu_tau
  if (GenTauTag::PcheckDecay(genpart, 221, 321, 111, -16) == 1) {
    return +1 * (100000 * nPhotos + 1048);
  }//tau+ decays to eta K+ pi0 anti-nu_tau
  if (GenTauTag::PcheckDecay(genpart, 221, 311, 211, -16) == 1) {
    return +1 * (100000 * nPhotos + 1049);
  }//tau+ decays to eta K0 pi+ anti-nu_tau
  if (GenTauTag::PcheckDecay(genpart, 20223, 211, -16) == 1) {
    return +1 * (100000 * nPhotos + 1050);
  }//tau+ decays to f_1 pi+ anti-nu_tau
  return +1 * (DauSize(genpart));
}// Rest of the tau+ decays


int GenTauTag::Mode_tau_minus(std::vector<int>genpart)
{
  if (GenTauTag::PcheckDecay(genpart, 11, -12, 16) == 1) {
    return -1 * (100000 * nPhotos + 1001);
  }//tau- decays to e- anti-nu_e nu_tau
  if (GenTauTag::PcheckDecay(genpart, 13, -14, 16) == 1) {
    return -1 * (100000 * nPhotos + 1002);
  }//tau- decays to mu- anti-nu_mu nu_tau
  if (GenTauTag::PcheckDecay(genpart, -211, 16) == 1) {
    return -1 * (100000 * nPhotos + 1003);
  }//tau- decays to pi- nu_tau
  if (GenTauTag::PcheckDecay(genpart, -321, 16) == 1) {
    return -1 * (100000 * nPhotos + 1004);
  }//tau- decays to K- nu_tau
  if (GenTauTag::PcheckDecay(genpart, -211, 111, 16) == 1) {
    return -1 * (100000 * nPhotos + 1005);
  }//tau- decays to pi- pi0 nu_tau
  if (GenTauTag::PcheckDecay(genpart, 16, 22, -211, 111) == 1) {
    return -1 * (100000 * nPhotos + 1006);
  }//tau- decays to nu_tau gamma pi- pi0
  if (GenTauTag::PcheckDecay(genpart, 111, 111, -211, 16) == 1) {
    return -1 * (100000 * nPhotos + 1007);
  }//tau- decays to pi0 pi0 pi- nu_tau
  if (GenTauTag::PcheckDecay(genpart, 16, -321, 111, 111) == 1) {
    return -1 * (100000 * nPhotos + 1008);
  }//tau- decays to nu_tau K- pi0 pi0
  if (GenTauTag::PcheckDecay(genpart, 16, -211, 111, 111, 111) == 1) {
    return -1 * (100000 * nPhotos + 1009);
  }//tau- decays to nu_tau pi- pi0 pi0 pi0
  if (GenTauTag::PcheckDecay(genpart, 16, -211, 111, 111, 111, 111) == 1) {
    return -1 * (100000 * nPhotos + 1010);
  }//tau- decays to nu_tau pi- pi0 pi0 pi0 pi0
  if (GenTauTag::PcheckDecay(genpart, 16, -211, -311) == 1) {
    return -1 * (100000 * nPhotos + 1011);
  }//tau- decays to nu_tau pi- anti-K0
  if (GenTauTag::PcheckDecay(genpart, 16, -321, 311) == 1) {
    return -1 * (100000 * nPhotos + 1012);
  }//tau- decays to nu_tau K- K0
  if (GenTauTag::PcheckDecay(genpart, 16, 311, -211, -311) == 1) {
    return -1 * (100000 * nPhotos + 1013);
  }//tau- decays to nu_tau K0 pi- anti-K0
  if (GenTauTag::PcheckDecay(genpart, 16, -321, 111, 311) == 1) {
    return -1 * (100000 * nPhotos + 1014);
  }//tau- decays to nu_tau K- pi0 K0
  if (GenTauTag::PcheckDecay(genpart, 16, -211, -311, 111) == 1) {
    return -1 * (100000 * nPhotos + 1015);
  }//tau- decays to nu_tau pi- anti-K0 pi0
  if (GenTauTag::PcheckDecay(genpart, -211, -211, 211, 16) == 1) {
    return -1 * (100000 * nPhotos + 1016);
  }//tau- decays to pi- pi- pi+ nu_tau
  if (GenTauTag::PcheckDecay(genpart, 16, -211, 211, -211, 111) == 1) {
    return -1 * (100000 * nPhotos + 1017);
  }//tau- decays to nu_tau pi- pi+ pi- pi0
  if (GenTauTag::PcheckDecay(genpart, 16, -211, -211, 211, 111, 111) == 1) {
    return -1 * (100000 * nPhotos + 1018);
  }//tau- decays to nu_tau pi- pi- pi+ pi0 pi0
  if (GenTauTag::PcheckDecay(genpart, 16, -211, -211, 211, 111, 111, 111) == 1) {
    return -1 * (100000 * nPhotos + 1019);
  }//tau- decays to nu_tau pi- pi- pi+ pi0 pi0 pi0
  if (GenTauTag::PcheckDecay(genpart, 16, -321, 211, -211) == 1) {
    return -1 * (100000 * nPhotos + 1020);
  }//tau- decays to nu_tau K- pi+ pi-
  if (GenTauTag::PcheckDecay(genpart, 16, -321, 211, -211, 111) == 1) {
    return -1 * (100000 * nPhotos + 1021);
  }//tau- decays to nu_tau K- pi+ pi- pi0
  if (GenTauTag::PcheckDecay(genpart, 16, -321, -211, 321) == 1) {
    return -1 * (100000 * nPhotos + 1022);
  }//tau- decays to nu_tau K- pi- K+
  if (GenTauTag::PcheckDecay(genpart, 16, -321, 321, -321) == 1) {
    return -1 * (100000 * nPhotos + 1023);
  }//tau- decays to nu_tau K- K+ K-
  if (GenTauTag::PcheckDecay(genpart, 16, -211, -211, -211, 211, 211) == 1) {
    return -1 * (100000 * nPhotos + 1024);
  }//tau- decays to nu_tau pi- pi- pi- pi+ pi+
  if (GenTauTag::PcheckDecay(genpart, 16, -211, -211, -211, 211, 211, 111) == 1) {
    return -1 * (100000 * nPhotos + 1025);
  }//tau- decays to nu_tau pi- pi- pi- pi+ pi+ pi0
  if (GenTauTag::PcheckDecay(genpart, -323, 16) == 1) {
    return -1 * (100000 * nPhotos + 1026);
  }//tau- decays to K*- nu_tau
  if (GenTauTag::PcheckDecay(genpart, 16, 221, -211, 111) == 1) {
    return -1 * (100000 * nPhotos + 1027);
  }//tau- decays to nu_tau eta pi- pi0
  if (GenTauTag::PcheckDecay(genpart, 16, -211, 223, 111) == 1) {
    return -1 * (100000 * nPhotos + 1028);
  }//tau- decays to nu_tau pi- omega pi0
  if (GenTauTag::PcheckDecay(genpart, 16, -321, 223) == 1) {
    return -1 * (100000 * nPhotos + 1029);
  }//tau- decays to nu_tau K- omega
  if (GenTauTag::PcheckDecay(genpart, -313, -211, 16) == 1) {
    return -1 * (100000 * nPhotos + 1030);
  }//tau- decays to anti-K*0 pi- nu_tau
  if (GenTauTag::PcheckDecay(genpart, 313, -321, 16) == 1) {
    return -1 * (100000 * nPhotos + 1031);
  }//tau- decays to K*0 K- nu_tau
  if (GenTauTag::PcheckDecay(genpart, 333, -211, 16) == 1) {
    return -1 * (100000 * nPhotos + 1032);
  }//tau- decays to phi pi- nu_tau
  if (GenTauTag::PcheckDecay(genpart, 333, -321, 16) == 1) {
    return -1 * (100000 * nPhotos + 1033);
  }//tau- decays to phi K- nu_tau
  if (GenTauTag::PcheckDecay(genpart, 13, -14, 16, 22) == 1) {
    return -1 * (100000 * nPhotos + 1034);
  }//tau- decays to mu- anti-nu_mu nu_tau gamma
  if (GenTauTag::PcheckDecay(genpart, 11, -12, 16, 22) == 1) {
    return -1 * (100000 * nPhotos + 1035);
  }//tau- decays to e- anti-nu_e nu_tau gamma
  if (GenTauTag::PcheckDecay(genpart, -321, 111, 16) == 1) {
    return -1 * (100000 * nPhotos + 1036);
  }//tau- decays to K- pi0 nu_tau
  if (GenTauTag::PcheckDecay(genpart, -311, -213, 16) == 1) {
    return -1 * (100000 * nPhotos + 1037);
  }//tau- decays to anti-K0 rho- nu_tau
  if (GenTauTag::PcheckDecay(genpart, -211, -311, 111, 111, 16) == 1) {
    return -1 * (100000 * nPhotos + 1038);
  }//tau- decays to pi- anti-K0 pi0 pi0 nu_tau
  if (GenTauTag::PcheckDecay(genpart, -211, 311, -311, 111, 16) == 1) {
    return -1 * (100000 * nPhotos + 1039);
  }//tau- decays to pi- K0 anti-K0 pi0 nu_tau
  if (GenTauTag::PcheckDecay(genpart, -321, 321, -211, 111, 16) == 1) {
    return -1 * (100000 * nPhotos + 1040);
  }//tau- decays to K- K+ pi- pi0 nu_tau
  if (GenTauTag::PcheckDecay(genpart, 11, 11, -11, -12, 16) == 1) {
    return -1 * (100000 * nPhotos + 1041);
  }//tau- decays to e- e- e+ anti-nu_e nu_tau
  if (GenTauTag::PcheckDecay(genpart, -10323, 16) == 1) {
    return -1 * (100000 * nPhotos + 1042);
  }//tau- decays to K_1- nu_tau
  if (GenTauTag::PcheckDecay(genpart, -20323, 16) == 1) {
    return -1 * (100000 * nPhotos + 1043);
  }//tau- decays to K'_1- nu_tau
  if (GenTauTag::PcheckDecay(genpart, -100323, 16) == 1) {
    return -1 * (100000 * nPhotos + 1044);
  }//tau- decays to K'*- nu_tau
  if (GenTauTag::PcheckDecay(genpart, 221, -211, 111, 111, 16) == 1) {
    return -1 * (100000 * nPhotos + 1045);
  }//tau- decays to eta pi- pi0 pi0 nu_tau
  if (GenTauTag::PcheckDecay(genpart, 221, -321, 16) == 1) {
    return -1 * (100000 * nPhotos + 1046);
  }//tau- decays to eta K- nu_tau
  if (GenTauTag::PcheckDecay(genpart, 221, -323, 16) == 1) {
    return -1 * (100000 * nPhotos + 1047);
  }//tau- decays to eta K*- nu_tau
  if (GenTauTag::PcheckDecay(genpart, 221, -321, 111, 16) == 1) {
    return -1 * (100000 * nPhotos + 1048);
  }//tau- decays to eta K- pi0 nu_tau
  if (GenTauTag::PcheckDecay(genpart, 221, -311, -211, 16) == 1) {
    return -1 * (100000 * nPhotos + 1049);
  }//tau- decays to eta anti-K0 pi- nu_tau
  if (GenTauTag::PcheckDecay(genpart, 20223, -211, 16) == 1) {
    return -1 * (100000 * nPhotos + 1050);
  }//tau- decays to f_1 pi- nu_tau
  return -1 * (DauSize(genpart));
}// Rest of the tau- decays
