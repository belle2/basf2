#ifndef VIEWER_H
#define VIEWER_H

#include <TROOT.h>
#include <TRint.h>
#include <TCanvas.h>
//#include <TBox.h>
#include <TNtuple.h>
#include <TString.h>
#include <TRandom.h>
#include <TH2I.h>
#include <TH2F.h>
#include <TStyle.h>
#include <TColor.h>
#include <TMath.h>
#include <TGaxis.h>
#include <TCrown.h>

#include <TTree.h>
#include <TChain.h>

#include <ecl/modules/eclDisplay/EclData.h>

namespace Belle2 {
  // Changes, whether to display a count of events or energy.
  void SetMode(int i);
  int GetMode();

  float GetPhi(int chnum);
  int GetPhiId(int chnum);
  float GetTheta(int chnum);
  int GetThetaId(int chnum);

  int GetChannel(int theta_id, int phi_id);
}

#endif // VIEWER_H
