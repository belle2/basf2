#include <framework/database/DBImportObjPtr.h>
#include <ecl/dbobjects/ECLCrystalCalib.h>
#include <iostream>
#include <fstream>
#include <TFile.h>
#include <TH1F.h>

int main()
{

  /** Manually create a set of calibration constants for ECL crystals, class ECLCrystalCalib
   * Usage:
   *    - edit as needed, including specifying payload name and interval of validity
   *    - scons ecl
   *    - execute (eclWriteLocalDB) in appropriate working directory
   *    - payload is written to localdb subdirectory, and added to localdb/database.txt
   *    - you can then add payload to development global tag if desired:
   *        cd localdb
   *        conditionsdb upload development ./database.txt
   */

  std::vector<float> tempCalib;
  std::vector<float> tempCalibUnc;
  float calib, calibUnc;

  /** Sample code to fill payload contents with a constant value */
  calib = 1.;
  calibUnc = 0.;
  for (int cellID = 1; cellID <= 8736; cellID++) {
    tempCalib.push_back(calib);
    tempCalibUnc.push_back(calibUnc);
  }

  /** Sample code to read text file of values */
  /* std::ifstream calibFile("refref1.txt");
  float cellIDf, amplitude, toff;
  for(int cellID=1; cellID<=8736; cellID++) {
      calibFile >> cellIDf >> amplitude >> toff;
      if(cellID<=1152) {
          calib = 1.;
      } else {
          calib = 17800./amplitude;
      }
      calibUnc = 0.;
      tempCalib.push_back(calib);
      tempCalibUnc.push_back(calibUnc);
      if(cellID%100==0) {std::cout << cellID << " ampl = " << amplitude << " calib = " << calib << " " << calibUnc << std::endl;}
  } */

  /** Sample code to read in a histogram */
  /* TFile f("eclCosmicEAlgorithm.root");
  TH1F *CalibvsCrys = (TH1F*)f.Get("CalibvsCrys");
  for(int cellID=1; cellID<=8736; cellID++) {
      if(cellID<1153) {
          calib = 4.06902e-05;
          calibUnc = 0.;
      } else {
          calib = CalibvsCrys->GetBinContent(cellID);
          calibUnc = CalibvsCrys->GetBinError(cellID);
      }
      tempCalib.push_back(calib);
      tempCalibUnc.push_back(calibUnc);
      if(cellID%100==0) {std::cout << "cellID = " << cellID << " calib = " << calib << " +/- " << calibUnc << std::endl; }
  } */


  //..Write out to localdb
  std::cout << "Creating importer" << std::endl;
  Belle2::DBImportObjPtr<Belle2::ECLCrystalCalib> importer("ECLCrystalEnergy");
  importer.construct();
  importer->setCalibVector(tempCalib, tempCalibUnc);
  importer.import(Belle2::IntervalOfValidity(0, 0, -1, -1));
  std::cout << "Successfully wrote payload ECLCrystalEnergy" << std::endl;
}
