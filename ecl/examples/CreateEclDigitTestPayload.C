/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 *  Create dummy file for energy and time digit calibration.              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Torben Ferber                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

int CreateEclDigitTestPayload(const std::string& outfile = "out.root", const std::string& type = "time"){
  
  // Open output file
  TFile *file = new TFile(outfile.c_str(), "RECREATE");
  
  // Constants
  const int c_nCrystals = 8736;
                          
  // Make calibration histograms
  if(type == "energy" ){
    TH1F* energy = new TH1F("energy", "energy", c_nCrystals, 0, c_nCrystals);
    TH1F* amplitude = new TH1F("amplitude", "amplitude", c_nCrystals, 0, c_nCrystals);
    for(int i=1; i<=c_nCrystals; ++i) {
      energy->SetBinContent(i, 1.0);
      amplitude->SetBinContent(i, 20000.0);
    }
  } // end energy
  else if (type == "time"){
    TH1F* offset = new TH1F("offset", "offset", c_nCrystals, 0, c_nCrystals);
    for(int i=1; i<=c_nCrystals; ++i) {
      offset->SetBinContent(i, 79.0);
    }
  } //end time
  else return -1;
  
  file->Write();
  file->Close();
  if(file) delete file;

  return 1;
}
