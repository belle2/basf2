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
#include <ecl/dataobjects/ECLWaveformData.h>

using namespace std;
using namespace Belle2;
//using namespace ECL;

using matrix_16x16 = double[16][16];
using fmatrix_16x16 = float[16][16];
using vector_16 = double[16];

void fillCrystalCat(vector<int>& crystalCat, vector< vector<int> >& grmap)
{
  string crystalGroupsFilename = FileSystem::findFile("/data/ecl/CIdToEclData.txt");
  assert(! crystalGroupsFilename.empty());
  ifstream ctoecldatafile(crystalGroupsFilename);
  int cid, group;

  while (true) {
    ctoecldatafile >> cid >> group;
    if (ctoecldatafile.eof()) break;
    assert(ctoecldatafile) ;
    crystalCat[cid] = group;
    grmap[group].push_back(cid + 1);
  }
}

void writeOutput(const char* filename, const vector<matrix_16x16>& mat, const vector< vector< int> >& grmap, const vector<int>& N)
{
  TFile* f = new TFile(filename, "RECREATE");
  TTree* t =  new TTree("EclWF", "Waveform and covariance matrix");
  ECLWaveformData* data = new ECLWaveformData;
  Int_t ncellId;
  vector<Int_t> cellId(8736);
  t->Branch("CovarianceM", &data, 256000);
  t->Branch("ncellId", &ncellId, "ncellId/I");
  t->Branch("cellId", cellId.data(), "cellId[ncellId]/I");

  for (size_t n = 0; n < grmap.size(); n++) {
    if (N[n] > 50) {
      const vector<int>& v = grmap[n];
      ncellId = v.size();
      if (ncellId == 0) continue;
      cellId = v;
      //    fmatrix_16x16 float_mat;
      // for (auto i : mat[n] ) float_mat.push_back(float(*i)) ;
      data->storeMatrix(mat[n]);
      t->Fill();
    }
  }

  cout << "Done. Writing..." << endl;
  t->Write();
  f->Write();
  f->Close();
}


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


void matrix_cal(const char* inputFilename,
                const char* outputFilename)
{

  TChain fChain("m_tree");


  cout << "file for calibration:" << inputFilename << endl;
  fChain.Add(inputFilename);

  vector<int> crystalCat(8736, 0);
  const int mapmax = 252;
  vector< vector<int> >grmap(mapmax);
  fillCrystalCat(crystalCat, grmap);

  Int_t           nhits;
  using int_31 = int[31];
  vector<int_31> hitA(8736);
  vector<Int_t> cellId(8736);
  TBranch*        b_nhits;
  TBranch*        b_hitA;
  TBranch*        b_cellId;



  fChain.SetBranchAddress("nhits", &nhits, &b_nhits);
  fChain.SetBranchAddress("hitA", hitA.data(), &b_hitA);
  fChain.SetBranchAddress("cellID", cellId.data(), &b_cellId);


  vector<matrix_16x16> XiXj(252), covM(252), invcovM(252);
  memset(XiXj.data(), 0., sizeof(matrix_16x16) * XiXj.size());

  vector<vector_16> mu(grmap.size());
  memset(mu.data(), 0., sizeof(vector_16) * mu.size());
  vector<int> N(grmap.size(), 0);


  int nevent = fChain.GetEntries();
  for (Int_t i = 0; i < nevent; i++) {
    fChain.GetEntry(i);
    if (i % 100 == 0) cout << " nevent=" << i << endl;

    for (int ic = 0; ic < nhits; ic++) {

      int icell = cellId[ic];
      //      cout << "cat: " << crystalCat[icell]  << " cid: " << icell << endl;
      vector_16& mu_ref = mu[crystalCat[icell]];

      matrix_16x16& XiXj_ref = XiXj[crystalCat[icell]];
      vector<double> Y(16, 0);
      //      double sum = 0;
      for (int j = 0; j < 16; j++) {
        //cout << hitA[ic][j] << endl;
        Y[0] += hitA[ic][j];
      } // end of loop on first 16 adc channels
      Y[0] /= 16;
      mu_ref[0] += Y[0];
      for (int j = 16; j < 31; j++) {
        Y[j - 15] = hitA[ic][j];
        mu_ref[j - 15] += hitA[ic][j];
      }// end of loop on the 16-30 adc channels

      for (int i = 0; i < 31; i++)
        for (int j = 0; j < 31; j++)
          XiXj_ref[i][j] += Y[i] * Y[j];

      // cout << "average before = " << mu_ref << endl;

      N[crystalCat[icell]] ++;
      // cout << "average = " << mu_ref << " N: "<< N[crystalCat[icell]] <<endl;
    } // end of loop on crystals
  } // end of loop on root entries


  for (size_t icat = 0; icat < grmap.size(); icat++) {
    cout << "cat: " << icat << "N : " << N[icat] << endl;
    if (N[icat] > 50) {
      for (int i = 0; i < 31; i++)
        for (int j = 0; j < 31; j++) {
          covM[icat][i][j] = (XiXj[icat][i][j] - mu[icat][i] * mu[icat][j] / N[icat]) / N[icat] ;
          cout << "i: " << i << " j: " << j
               << "mu_j: " << mu[icat][j] / N[icat]
               << " Xij: " << XiXj[icat][i][j] / N[icat]
               << " cov: " << covM[icat][i][j]
               << endl;


        }
      int dims = 16;
      double dec = 0;

      sim(& covM[icat][0][0] , &dims, & invcovM[icat][0][0] , &dec);
    }
  }
  writeOutput(outputFilename, invcovM, grmap, N);
}


int main(int argc, char** argv)
{
  // first argument is cortype : what does it mean?
  // second argument is input root file name
  // third argument is (existing) directory to put the output files (cor matrices)
  // fourth argument is the cut file name
  assert(argc == 3);
  // matrix_cal(atoi(argv[1]), argv[2], argv[3], argv[4]);

  // first argument input file
  // second argument output file
  matrix_cal(argv[1] , argv[2]);
}
















