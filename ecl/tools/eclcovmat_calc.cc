#include <TFile.h>
#include <TChain.h>
#include <TH2.h>
#include <TH1.h>
#include <TF1.h>
#include <TMath.h>
#include <stdio.h>
#include <vector>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <vector>
#include "TCL1.h"
#include <cassert>
#include <framework/utilities/FileSystem.h>
#include <ecl/digitization/WrapArray2D.h>

using namespace std;
using namespace Belle2;
using namespace ECL;

// function for inversion simmetric matrix
void sim(double* ss, int* N, double* aa, double* sb)
{
  int i, k, j;
  int n, n1, n2;

  double dbs;

  n = *N;
  n1 = n * (n + 1) / 2;
  n2 = n * n;

  vector<double> svec(n1, 0), rvec(n1, 0);
  double* s = & svec[0];
  double* r = & rvec[0];

  for (i = 0; i < n2; i++) {
    *(aa + i) = *(ss + i);
  }

  TCL::trpck(aa, s, *N);
  TCL::trsinv(s, r, *N);
  TCL::trupck(r, aa, *N);

  *sb = 0.;
  for (j = 0; j < n; j++) {
    for (i = 0; i < n; i++) {
      dbs = 0.;
      for (k = 0; k < n; k++) {
        dbs = dbs + *(ss + i + k * n) * (*(aa + n * j + k));
      }
      if (!(i == j) && fabs(dbs) > *sb) {*sb = fabs(dbs);}
      if (i == j && fabs(dbs - 1.) > *sb) {*sb = fabs(1. - dbs);}
    }
  }   // for j


  for (j = 0; j < n2; j++) {

    *(ss + j) = *(aa + j);

  }
}

class DoubleArray3D {
public:
  DoubleArray3D(size_t N1, size_t N2, size_t N3, double init)
  {
    m_N1 = N1;
    m_N2 = N2;
    m_N3 = N3;

    m_arr = new double** [m_N1];
    for (size_t i1 = 0; i1 < m_N1; i1++) {
      m_arr[i1] = new double * [m_N2];
      for (size_t i2 = 0; i2 < m_N2; i2++) {
        m_arr[i1][i2] = new double [m_N3];
        for (size_t i3 = 0; i3 < m_N3; i3++) {
          m_arr[i1][i2][i3] = init;
        }
      }
    }
  }
  ~DoubleArray3D()
  {
    for (size_t i1 = 0; i1 < m_N1; i1++) {
      for (size_t i2 = 0; i2 < m_N2; i2++) {
        delete [] m_arr[i1][i2];
      }
      delete [] m_arr[i1];
    }
    delete [] m_arr;
  }
  double** operator[](size_t idx)
  {
    return m_arr[idx];
  }

private:
  double*** m_arr;
  size_t m_N1, m_N2, m_N3;
};

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

  double dec;

  int N1;
  N1 = 16;
  double delta;
  delta = 0.;

  // not used commented to silence warning
  // double SI[16][16];
  // double IS[16][16];

  double MI[16][16];
  double IM[16][16];
  double UU[16][16];
  double FF[16][16];



  const int mapmax = 252;

  /*
  typedef boost::multi_array<double, 3> array3d;
  array3d W(boost::extents[mapmax][16][1]);
  array3d WW(boost::extents[mapmax][16][16]);
  */
  DoubleArray3D W(mapmax, 16, 1, 0.0);

  DoubleArray3D WW(mapmax, 16, 16, 0.0);

  WrapArray2D<double> Q(mapmax, 16);
  vector<double> Mean(8736, 0);

  // not used commented to silence warning
  // double rms(8736,0);

  vector<double> sr(8736, 0);
  vector<double> sl(8736, 0);

  vector<double> dt(mapmax, 0);


  vector<double> inmt(mapmax, 0);
  int it;
  double A0;
  cortyp = 43;
  double Nsigcut;
  Nsigcut = 3.5;
  delta = 0.;
  int index;
  index = 0;
  int icn, id;


  vector<int> DS(8736, 0);

  string crystalGroupsFilename = FileSystem::findFile("/data/ecl/CIdToEclData.txt");
  assert(! crystalGroupsFilename.empty());
  ifstream ctoecldatafile(crystalGroupsFilename);

  int cid, group;
  bool readerr = false;
  vector<int> grmap[252];
  while (! ctoecldatafile.eof()) {
    ctoecldatafile >> cid >> group;

    if (ctoecldatafile.eof()) break;
    if (group <= -1 || group >= 252 || cid <= -1 || cid >= 8736) {
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
    if (i % 1000 == 0) { cout << " nevent=" << i << endl; }
    for (icn = 0; icn < max; icn++) { //%%%%%%%%%%%%%%%%%%%%%%%%55555555555
      index = 1;
      for (id = 0; id < 16; id++) {
        Y[it] = 0.;
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

      if (index == 1 || index == 0) {
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

  //             for(icn=0;icn<max;icn++){  //%%%%%%%%%%%%%%%%%%%%%%%%55555555555

//                  for(icn=0;icn<mapmax;icn++){  //%%%%%%%%%%%%%%%%%%%%%%%%55555555555

  for (icn = 0; icn < 12; icn++) { //%%%%%%%%%%%%%%%%%%%%%%%%55555555555


    if (0 == 0) { // conventional comment
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
            MI[ia][ib] = (WW[icn][ia][ib] - W[icn][ia][0] * W[icn][ib][0] / inmt[icn]) / inmt[icn];

            UU[ia][ib] = (WW[icn][ia][ib] - W[icn][ia][0] * W[icn][ib][0] / inmt[icn]) / inmt[icn];
            // not used commented to silence warning
            // SI[ia][ib] = (WW[icn][ia][ib] - W[icn][ia][0] * W[icn][ib][0] / inmt[icn]) / inmt[icn];



          }

        }
        // not used commented to silence warning
        // IS[ia][ib] = 0;

      }


      // INVERSION
      N1 = 16;

      sim(*MI, &N1, *IM, &dec);



      int ic;

      // CALCULATION RESIDUAL
      dt[icn] = 0.;
      for (ia = 0; ia < 16; ia++) {
        for (ib = 0; ib < 16; ib++) {
          FF[ia][ib] = 0.;
          for (ic = 0; ic < 16; ic++) {
            FF[ia][ib] = FF[ia][ib] + UU[ia][ic] * IM[ic][ib];
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

      // can be improved....
      if (0 == 1) { //convetrinal commnet
        cout << endl;

        for (ia = 0; ia < 16; ia++) {
          cout << endl;
          for (ib = 0; ib < 16; ib++) {
            cout << setprecision(2) << FF[ia][ib] << endl;
            // printf("%.2e ", FF[ia][ib]);
          }
        }


      }  //conventinal comment

    } // largest conventional comment, but what is this for???

    // WRITE INVERS MATRICES

    if (0 == 0) { // conventional comment

      string mcorFilename(corrDirSuffix);
      mcorFilename += to_string(cortyp) + "/mcor" + to_string(icn) + "_L.dat";
      // sprintf(Min, "/hsm/belle/bdata2/users/avbobrov/belle2/corr%d/mcor%d_L.dat", cortyp, icn);
      ofstream mcorFile(mcorFilename);

      for (poq = 0; poq < 16; poq++) {
        mcorFile << setprecision(5)
                 << UU[0][poq] << "\t" << UU[1][poq] << "\t" <<  UU[2][poq] << "\t"
                 << UU[3][poq] << "\t" << UU[4][poq] << "\t" <<  UU[5][poq] << "\t"
                 << UU[6][poq] << "\t" << UU[7][poq] << "\t" <<  UU[8][poq] << "\t"
                 << UU[9][poq] << "\t" << UU[10][poq]
                 << UU[11][poq]  << "\t" << UU[12][poq]  << "\t"
                 << UU[13][poq]  << "\t" << UU[14][poq] << "\t" <<  UU[15][poq] << endl;
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
                   << IM[0][poq] << "\t" << IM[1][poq] << "\t" <<  IM[2][poq] << "\t"
                   << IM[3][poq] << "\t" << IM[4][poq] << "\t" <<  IM[5][poq] << "\t"
                   << IM[6][poq] << "\t" << IM[7][poq] << "\t" <<  IM[8][poq] << "\t"
                   << IM[9][poq] << "\t" << IM[10][poq]
                   << IM[11][poq]  << "\t" << IM[12][poq]  << "\t"
                   << IM[13][poq]  << "\t" << IM[14][poq] << "\t" <<  IM[15][poq] << endl;

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
  cout << "delta= " <<  delta << endl;

}

int main(int argc, char** argv)
{
  // first argument is cortype : what does it mean?
  // second argument is input root file name
  // third argument is (existing) directory to put the output files (cor matrices)
  // fourth argument is the cut file name
  assert(argc == 4);
  matrix_cal(10, argv[1], argv[2], argv[3]);
}
















