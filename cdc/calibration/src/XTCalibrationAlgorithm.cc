#include <cdc/calibration/XTCalibrationAlgorithm.h>
#include <cdc/geometry/CDCGeometryPar.h>

#include <cdc/dataobjects/WireID.h>
#include <cdc/dbobjects/CDCXtRelations.h>

#include <TError.h>
#include <TROOT.h>
#include <TH1D.h>
#include <TGraphErrors.h>
#include <TProfile.h>
#include <TF1.h>
#include <TFile.h>
#include <TChain.h>
#include <TTree.h>
#include <TSystem.h>
#include <iostream>
#include <iomanip>
#include <cdc/calibration/CDCDatabaseImporter.h>

#include <boost/iostreams/filtering_stream.hpp>
#include <boost/iostreams/device/file.hpp>
#include <boost/iostreams/filter/gzip.hpp>


using namespace std;
using namespace Belle2;
using namespace CDC;
typedef std::array<float, 3> array3; /**< angle bin info. */
XTCalibrationAlgorithm::XTCalibrationAlgorithm() :  CalibrationAlgorithm("CDCCalibrationCollector")
{
  //  setDescription(
  //     " -------------------------- XT Calibration Algoritm -------------------------\n"
  //     );
}

void XTCalibrationAlgorithm::createHisto()
{
  readXT();
  auto tree = getObjectPtr<TTree>("tree");

  int lay, trighit;
  double dt;
  double dx;
  double Pval, alpha, theta;
  double ndf;

  tree->SetBranchAddress("lay", &lay);
  tree->SetBranchAddress("t", &dt);
  tree->SetBranchAddress("x_u", &dx);
  tree->SetBranchAddress("alpha", &alpha);
  tree->SetBranchAddress("theta", &theta);
  tree->SetBranchAddress("Pval", &Pval);
  tree->SetBranchAddress("trighit", &trighit);
  tree->SetBranchAddress("ndf", &ndf);

  /*Create histogram*/
  for (int i = 0; i < 56; ++i) {
    for (int lr = 0; lr < 2; ++lr) {
      for (int al = 0; al < m_nAlpha; ++al) {
        for (int th = 0; th < m_nTheta; ++th) {
          hprof[i][lr][al][th] = new TProfile(Form("hprof%d_%d_%d_%d", i, lr, al, th),
                                              Form("(L=%d)-(lr=%d)-(#alpha=%3.0f)-(#theta=%3.0f); Drift time (ns);Drift Length (cm)",
                                                   i, lr, ialpha[al], itheta[th]), 210, -20, 600, 0, 1.2, "i");
          hist2d[i][lr][al][th] = new TH2D(Form("h%d_%d_%d_%d", i, lr, al, th),
                                           Form("(L=%d)-(lr=%d)-(#alpha=%3.0f)-(#theta=%3.0f); Drift time (ns);Drift Length (cm)",
                                                i, lr, ialpha[al], itheta[th]), 210, -20, 600, 110, 0, 1.2);
          if (lr == 1)
            hist2d_draw[i][al][th] = new TH2D(Form("h_draw%d_%d_%d", i, al, th),
                                              Form("(L=%d)-(#alpha=%3.0f)-(#theta=%3.0f); Drift time (ns);Drift Length (cm)",
                                                   i, ialpha[al], itheta[th]), 210, -20, 600, 2200, -1.2, 1.2);
        }
      }
    }
  }

  /*Now read data and make histo*/
  int al = 0;
  int th = 0;
  int lr = 0;
  Long64_t nbytes = 0;
  int nEntries = tree->GetEntries();
  cout << "Number of Hit: " << nEntries << endl;

  for (int i = 0; i < nEntries; ++i) {
    nbytes += tree->GetEntry(i);
    /* protect in case |alpha|>90*/
    if (fabs(alpha > 90)) {
      if (alpha < 0) alpha += 180;
      if (alpha > 0) alpha -= 180;
    }

    if (Pval < m_Pvalmin) continue;
    if (ndf < m_ndfmin) continue;

    for (int k = 0; k < m_nAlpha; ++k) {
      if (alpha < u_alpha[k]) {al = k; break;}
    }
    for (int j = 0; j < m_nTheta; ++j) {
      if (theta < u_theta[j]) {th = j; break;}
    }
    if (dx > 0)
      lr = c_Right;
    else
      lr = c_Left;
    if (m_LRseparate) {
      hprof[lay][lr][al][th]->Fill(dt, abs(dx));
      hist2d[lay][lr][al][th]->Fill(dt, abs(dx));
    } else {
      hprof[lay][0][al][th]->Fill(dt, abs(dx));
      hist2d[lay][0][al][th]->Fill(dt, abs(dx));
      hprof[lay][1][al][th]->Fill(dt, abs(dx));
      hist2d[lay][1][al][th]->Fill(dt, abs(dx));
    }
    hist2d_draw[lay][al][th]->Fill(dt, dx);
  }
}

CalibrationAlgorithm::EResult XTCalibrationAlgorithm::calibrate()
{
  B2INFO("Start calibration");
  return c_OK;
}
void XTCalibrationAlgorithm::write()
{
}

void XTCalibrationAlgorithm::storeHisto()
{
}

void XTCalibrationAlgorithm::readXT()
{
  DBObjPtr<CDCXtRelations> dbXT_old;
}

