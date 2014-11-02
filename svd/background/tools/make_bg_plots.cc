#include <iostream>
#include <svd/background/HistogramFactory.h>
#include <vxd/dataobjects/VxdID.h>

#include <TROOT.h>
#include <TFile.h>
#include <TTree.h>
#include <TH1F.h>
#include <THStack.h>

using namespace std;
using namespace Belle2;

int main(int argc, char** argv)
{
  if (argc != 2) {
    cout << "Hello!" << endl;
    cout << "Usage: " << argv[0] << " filename.root" << endl;
    cout << "Make plots out of SVDBackground module results file filename.root" << endl;
    return 0;
  }

  TFile* f = new TFile(argv[1], "READ");
  if (!f->IsOpen()) {
    cout << "Input file not found. Exiting." << endl;
    return -1;
  } else {
    cout << "Opened " << argv[1] << ". Processing..." << endl;
  }

  // Now the plots one by one. Not sure what we are going to outuut. Canvases?
  // 1. Dose bars

  // 2. Neutron flux bars
  // 3. Fired strips bars
  // 4. Occupancy bars
  // 5. Fluence plots

  return 0;
}
