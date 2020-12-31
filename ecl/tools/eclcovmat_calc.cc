/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2011 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Program for calculation and inversion cavariance matrices              *
 *                                                                        *
 * Contributors: Alexander Bobrov (a.v.bobrov@inp.nsk.su) ,               *
 * Guglielmo De Nardo                                                     *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <TChain.h>
#include <TF1.h>
#include <TMath.h>
#include <vector>
#include <iostream>
#include <iomanip>
#include <cassert>
#include <framework/utilities/FileSystem.h>
#include <ecl/digitization/WrapArray2D.h>
#include <Eigen/Dense>

using namespace std;
using namespace Belle2;
using namespace ECL;
using namespace Eigen;

void matrix_cal(int cortyp, const char* inputRootFilename,
                const char* corrDirSuffix, const char* cutFilename)
{

  TChain fChain("m_tree");
  Int_t poq;

  cout << "file for calibration:" << inputRootFilename << endl;
  fChain.Add(inputRootFilename);

  Int_t           nhits;
  WrapArray2D<Int_t> hitA(8736, 31);
  TBranch*        b_nhits;
  TBranch*        b_hitA;

  fChain.SetBranchAddress("nhits", &nhits, &b_nhits);
  fChain.SetBranchAddress("hitA", hitA, &b_hitA);


  // not used commented to silence warning
  // double SI[16][16];
  // double IS[16][16];

  double IM[16][16];
  double UU[16][16];
  double FF[16][16];



  constexpr int mapmax = 217;

  //Can't be std::array because those are constructed on the stack and then there's a compiler warning about a stack overflow
  std::vector <
  std::vector <
  std::vector<double >>> W(mapmax, std::vector<std::vector<double>>(16, std::vector<double>(1, 0.0)));

  std::vector <
  std::vector <
  std::vector<double >>> WW(mapmax, std::vector<std::vector<double>>(16, std::vector<double>(16, 0.0)));

  WrapArray2D<double> Q(mapmax, 16);
  vector<double> Mean(8736, 0);


  Eigen::MatrixXd m(16, 16);
  Eigen::MatrixXd u(16, 16);
  Eigen::MatrixXd y(16, 16);

  // not used commented to silence warning
  // double rms(8736,0);

  vector<double> sr(8736, 0);
  vector<double> sl(8736, 0);

  vector<double> dt(mapmax, 0);
  vector<double> dtn(mapmax, 0);


  vector<double> inmt(mapmax, 0);
  int it;
  double A0;
  // cortyp = 43;
  double Nsigcut;
  Nsigcut = 3.5;
  double delta = 0.;
  int icn, id;


  vector<int> DS(8736, 0);

  string crystalGroupsFilename = FileSystem::findFile("/data/ecl/CIdToEclData.txt");
  assert(! crystalGroupsFilename.empty());
  ifstream ctoecldatafile(crystalGroupsFilename);

  int cid, group;
  bool readerr = false;
  //vector<int> grmap[217];
  while (! ctoecldatafile.eof()) {
    ctoecldatafile >> cid >> group;

    if (ctoecldatafile.eof()) break;
    if (group <= -1 || group >= 217 || cid <= -1 || cid >= 8736) {
      cout << "Error cid="  << cid << " group=" << group << endl;
      readerr = true;
      break;
    }
    DS[cid] = group;
  }
  assert(!readerr);
  int ia, ib;

  int max = 8736;
  int j;
  int trun;

  double Y[16];

  // red mean and rms for channels
// sprintf(cutin,"/home/belle/avbobrov/ecl/covmat/chdatu005.txt");
  ifstream cutFile(cutFilename);

  int jk;
  double mn;
  double rs;
  double el;
  double er;
  double ch2;

  while (true) {
    cutFile >> jk >> mn >> rs >> el >> er >> ch2;
    if (cutFile.eof()) break;
    Mean[jk] = mn;
    // not used: commented to silence warnings
    //    rms[jk] = rs;
    sr[jk] = er;
    sl[jk] = el;
  }
  cutFile.close();

  for (icn = 0; icn < mapmax; icn++) {
    inmt[icn] = 0.;
    dt[icn] = 0.;
    dtn[icn] = 0.;
    for (it = 0; it < 16; it++) {
      W[icn][it][0] = 0.;
      Q[icn][it] = 0.;

      for (id = 0; id < 16; id++) {
        WW[icn][it][id] = 0.;

      }
    }
  }


  Int_t nevent = fChain.GetEntries();
  std::cout << "! nevent=" << nevent << std::endl;

  for (Int_t i = 0; i < nevent; i++) {
    fChain.GetEntry(i);
    if (i % 100 == 0) { cout << " nevent=" << i << endl; }
    for (icn = 0; icn < max; icn++) { //%%%%%%%%%%%%%%%%%%%%%%%%55555555555
      int index = 1;
      for (id = 0; id < 16; id++) {
        Y[id] = 0.; // was it, fixed 20151126.
      }

      A0 = 0;
      for (j = 0; j < 31; j++) {
        if (j < 16) {
          A0 = A0 + (double)hitA[icn][j] / 16.;
          if ((double)hitA[icn][j] - Mean[icn] < -Nsigcut * sl[icn]) {index = 0;}
          if ((double)hitA[icn][j] - Mean[icn] > Nsigcut * sr[icn]) {index = 0;}
        } else {
          if ((double)hitA[icn][j] - Mean[icn] > Nsigcut * sr[icn] || (double)hitA[icn][j] - Mean[icn] < -Nsigcut * sl[icn]) {index = 0;}
        }
      }  // points cicle for selection events

      // selection not applied because seems to cut all events
      // to be checked

      if (index == 1) {
        inmt[DS[icn]] = inmt[DS[icn]] + 1.;
        A0 = 0.;

        for (j = 0; j < 31; j++) {

          if (j < 16) {A0 = A0 + (double)hitA[icn][j] / 16.;}

          if (j == 15) {
            W[DS[icn]][0][0] = W[DS[icn]][0][0] + A0;
            Q[DS[icn]][0] = Q[DS[icn]][0] + A0;
            Y[0] = A0;

          }

          if (j > 15) {
            trun = j - 15;
            Y[trun] = (double)hitA[icn][j];
            W[DS[icn]][trun][0] = W[DS[icn]][trun][0] + (double)hitA[icn][j];
            Q[DS[icn]][trun] = Q[DS[icn]][trun] + (double)hitA[icn][j];

          }

        }  // points cicle for writing



        for (it = 0; it < 16; it++) {
          for (id = 0; id < 16; id++) {
            WW[DS[icn]][it][id] = WW[DS[icn]][it][id] + Y[it] * Y[id];
          }
        }

      }  // index

    }  //channels cilce

  } // events cicle





  // MATRICES INVERTION


  for (icn = 0; icn < mapmax; icn++) { //%%%%%%%%%%%%%%%%%%%%%%%%55555555555


    {
      // conventional comment
      if (icn % 10 == 0) {
        cout << "icn= " << icn << " imnt= " << inmt[icn] << endl;
        // printf("icn=%d inmt=%lf \n ", icn, inmt[icn]);
      }

      if (inmt[icn] == 0) {
        cout << "bad icn= " << icn << endl;
        // printf("bad icn=%d\n", icn);
      }

      // WRITE DATA INTO ARRAY
      for (ia = 0; ia < 16; ia++) {
        for (ib = 0; ib < 16; ib++) {
          if (ib < 16 && ia < 16) {
            m(ia, ib) = (WW[icn][ia][ib] - W[icn][ia][0] * W[icn][ib][0] / inmt[icn]) / inmt[icn];
            UU[ia][ib] = (WW[icn][ia][ib] - W[icn][ia][0] * W[icn][ib][0] / inmt[icn]) / inmt[icn];
            // not used commented to silence warning
            // SI[ia][ib] = (WW[icn][ia][ib] - W[icn][ia][0] * W[icn][ib][0] / inmt[icn]) / inmt[icn];



          }

        }
        // not used commented to silence warning
        // IS[ia][ib] = 0;

      }


      // INVERSION


      u = m.inverse();


//     u = y*y;
      int ic;

      // CALCULATION RESIDUAL
      dt[icn] = 0.;
      for (ia = 0; ia < 16; ia++) {
        for (ib = 0; ib < 16; ib++) {
          IM[ia][ib] = u(ia, ib);
          FF[ia][ib] = 0.;
          for (ic = 0; ic < 16; ic++) {
            FF[ia][ib] = FF[ia][ib] + UU[ia][ic] * u(ic, ib);
          }
          if (ia == ib && fabs(FF[ia][ib] - 1.) > delta) {delta = fabs(FF[ia][ib] - 1.);}
          if (ia != ib && fabs(FF[ia][ib]) > delta) {delta = fabs(FF[ia][ib]);}

          if (ia == ib && fabs(FF[ia][ib] - 1.) > dt[icn]) {dt[icn] = fabs(FF[ia][ib] - 1.);}
          if (ia != ib && fabs(FF[ia][ib]) > dt[icn]) {dt[icn] = fabs(FF[ia][ib]);}
        }
      }
      if (dt[icn] < 1.e-33 || dt[icn] > 1.e-9) {
        cout << "icn=" << icn <<  " inmt=" << inmt[icn] << " cut=??" << "delta=" << dt[icn] << endl;
        //  printf("icn=%d inmt=%lf cut=%lf delta= %e \n  ", icn, inmt[icn], dt[icn]);
      }


    } // largest conventional comment, but what is this for???



    // WRITE INVERS MATRICES


    {
      // conventional comment

      string mcorFilename(corrDirSuffix);
      mcorFilename += to_string(cortyp) + "/mcor" + to_string(icn) + "_L.dat";
      // sprintf(Min, "/hsm/belle/bdata2/users/avbobrov/belle2/corr%d/mcor%d_L.dat", cortyp, icn);
      ofstream mcorFile(mcorFilename);

      for (poq = 0; poq < 16; poq++) {
        mcorFile << setprecision(5)
                 << m(0, poq) << "\t" <<  m(1, poq) << "\t" <<   m(2, poq) << "\t"
                 <<  m(3, poq) << "\t" <<  m(4, poq) << "\t" <<   m(5, poq) << "\t"
                 <<  m(6, poq) << "\t" <<  m(7, poq) << "\t" <<   m(8, poq) << "\t"
                 <<  m(9, poq) << "\t" <<  m(10, poq)
                 <<  m(11, poq)  << "\t" <<  m(12, poq)  << "\t"
                 <<  m(13, poq)  << "\t" <<  m(14, poq) << "\t" <<   m(15, poq) << endl;
        // fprintf(McoIN,
        // "%.5e \t %.5e \t %.5e \t %.5e \t %.5e \t %.5e \t %.5e \t %.5e \t %.5e \t %.5e \t %.5e \t %.5e \t %.5e \t %.5e \t %.5e \t %.5e \n  ",
        //       UU[0][poq], UU[1][poq], UU[2][poq], UU[3][poq], UU[4][poq], UU[5][poq], UU[6][poq], UU[7][poq], UU[8][poq], UU[9][poq], UU[10][poq],
        //      UU[11][poq], UU[12][poq], UU[13][poq], UU[14][poq], UU[15][poq]);



      }
      mcorFile.close();

      string inmcorFilename(corrDirSuffix);
      inmcorFilename += to_string(cortyp) + "/inmcor" + to_string(icn) + "_L.dat";
      //      sprintf(Min, "/hsm/belle/bdata2/users/avbobrov/belle2/corr%d/inmcor%d_L.dat", cortyp, icn);

      ofstream inmcorFile(inmcorFilename);
      for (poq = 0; poq < 16; poq++) {
        inmcorFile << setprecision(3)
                   << u(0, poq) << "\t" <<  u(1, poq) << "\t" <<   u(2, poq) << "\t"
                   <<  u(3, poq) << "\t" <<  u(4, poq) << "\t" <<   u(5, poq) << "\t"
                   <<  u(6, poq) << "\t" <<  u(7, poq) << "\t" <<   u(8, poq) << "\t"
                   <<  u(9, poq) << "\t" <<  u(10, poq)
                   <<  u(11, poq)  << "\t" <<  u(12, poq)  << "\t"
                   <<  u(13, poq)  << "\t" <<  u(14, poq) << "\t" <<   u(15, poq) << endl;

        // fprintf(McoIN,
        //                "%.3e \t %.3e \t %.3e \t %.3e \t %.3e \t %.3e \t %.3e \t %.3e \t %.3e \t %.3e \t %.3e \t %.3e \t %.3e \t %.3e \t %.3e \t %.3e \n  ",
        //         IM[0][poq], IM[1][poq], IM[2][poq], IM[3][poq], IM[4][poq], IM[5][poq], IM[6][poq], IM[7][poq], IM[8][poq], IM[9][poq], IM[10][poq],
        //IM[11][poq], IM[12][poq], IM[13][poq], IM[14][poq], IM[15][poq]);
      }
      inmcorFile.close();

      string binmcorFilename(corrDirSuffix);
      binmcorFilename += to_string(cortyp) + "/Binmcor" + to_string(icn) + "_L.dat";
      //      sprintf(BMin, "/hsm/belle/bdata2/users/avbobrov/belle2/corr%d/Binmcor%d_L.dat", cortyp, icn);
      ofstream binmcorFile(binmcorFilename, ofstream::binary);
      binmcorFile.write(reinterpret_cast<char*>(IM), sizeof(double) * 16 * 16);
      binmcorFile.close();
    }  // converional cooment
    //%%%%%%%%%%%%%%%%%%%%%%%%55555555555

  }  // icn

  cout << endl;
//  cout << "  delta= " <<  delta << endl;
  cout <<  "  delta= " <<  delta << endl;




}

int main(int argc, char** argv)
{
  // first argument is cortype : what does it mean?
  // second argument is input root file name
  // third argument is (existing) directory to put the output files (cor matrices)
  // fourth argument is the cut file name
  assert(argc == 5);
  matrix_cal(atoi(argv[1]), argv[2], argv[3], argv[4]);
}
















