/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <ecl/dbobjects/ECLClusteringParameters.h>
#include <framework/database/DBImportObjPtr.h>
#include <iostream>


using namespace Belle2;


//------------------------------------------------------------------------
//  eclWriteClusteringParameters creates an eclClusteringParameters payload
//  from the arguments of the function.
//
// usage:
//  eclWriteClusteringParameters LM_energy_cut
//
//  where the 10 arguments are:
//  LM_energy_cut [ECLLocalMaximumFinder]: Seed energy cut [MeV]
//  CRF_energy_cut0 [ECLCRFinder]: seed energy cut [MeV]
//  CRF_energy_cut1 [ECLCRFinder]: growth energy cut [MeV]
//  CRF_energy_cut2 [ECLCRFinder]: digit energy cut [MeV]
//  CRF_time_cut0 [ECLCRFinder]: seed time cut [ns]
//  CRF_time_cut1 [ECLCRFinder]: growth time cut [ns]
//  CRF_time_cut2 [ECLCRFinder]: digit time cut [ns]
//  CRF_timeCut_maxEnergy0 [ECLCRFinder]: max energy for seed time cut [MeV]
//  CRF_timeCut_maxEnergy1 [ECLCRFinder]: max energy for growth time cut [MeV]
//  CRF_timeCut_maxEnergy2 [ECLCRFinder]: max energy for digit time cut [MeV]


//------------------------------------------------------------------------
int main(int argc, char** argv)
{
  if (argc != 11) {
    std::cout << "eclWriteClusteringParameters must be called with 10 arguments:" << std::endl;
    std::cout << "  LM_energy_cut [ECLLocalMaximumFinder]: Seed energy cut [MeV]" << std::endl;
    std::cout << "  CRF_energy_cut0 [ECLCRFinder]: seed energy cut [MeV]" << std::endl;
    std::cout << "  CRF_energy_cut1 [ECLCRFinder]: growth energy cut [MeV]" << std::endl;
    std::cout << "  CRF_energy_cut2 [ECLCRFinder]: digit energy cut [MeV]" << std::endl;
    std::cout << "  CRF_time_cut0 [ECLCRFinder]: seed time cut [ns]" << std::endl;
    std::cout << "  CRF_time_cut1 [ECLCRFinder]: growth time cut [ns]" << std::endl;
    std::cout << "  CRF_time_cut2 [ECLCRFinder]: digit time cut [ns]" << std::endl;
    std::cout << "  CRF_timeCut_maxEnergy0 [ECLCRFinder]: max E for seed time cut [MeV]" << std::endl;
    std::cout << "  CRF_timeCut_maxEnergy1 [ECLCRFinder]: max E for growth time cut [MeV]" << std::endl;
    std::cout << "  CRF_timeCut_maxEnergy2 [ECLCRFinder]: max E for digit time cut [MeV]" << std::endl;
    return -1;
  }

  //..Parameter values
  double LM_energy_cut = std::stod(argv[1]);

  std::vector<double> CRF_energy_cut;
  CRF_energy_cut.push_back(std::stod(argv[2]));
  CRF_energy_cut.push_back(std::stod(argv[3]));
  CRF_energy_cut.push_back(std::stod(argv[4]));

  std::vector<double> CRF_time_cut;
  CRF_time_cut.push_back(std::stod(argv[5]));
  CRF_time_cut.push_back(std::stod(argv[6]));
  CRF_time_cut.push_back(std::stod(argv[7]));

  std::vector<double> CRF_timeCut_maxEnergy;
  CRF_timeCut_maxEnergy.push_back(std::stod(argv[8]));
  CRF_timeCut_maxEnergy.push_back(std::stod(argv[9]));
  CRF_timeCut_maxEnergy.push_back(std::stod(argv[10]));

  //..Write out arguments, just to be sure
  std::cout << std::endl << "values to be written in payload eclClusteringParameters: " << std::endl;
  std::cout << " LM_energy_cut [ECLLocalMaximumFinder]: " << LM_energy_cut << std::endl;
  std::cout << " CRF_energy_cut0 [ECLCRFinder]: " << CRF_energy_cut[0] << std::endl;
  std::cout << " CRF_energy_cut1 [ECLCRFinder]: " << CRF_energy_cut[1] << std::endl;
  std::cout << " CRF_energy_cut2 [ECLCRFinder]: " << CRF_energy_cut[2] << std::endl;
  std::cout << " CRF_time_cut0 [ECLCRFinder]: " << CRF_time_cut[0] << std::endl;
  std::cout << " CRF_time_cut1 [ECLCRFinder]: " << CRF_time_cut[1] << std::endl;
  std::cout << " CRF_time_cut2 [ECLCRFinder]: " << CRF_time_cut[2] << std::endl;
  std::cout << " CRF_timeCut_maxEnergy0 [ECLCRFinder]: " << CRF_timeCut_maxEnergy[0] << std::endl;
  std::cout << " CRF_timeCut_maxEnergy1 [ECLCRFinder]: " << CRF_timeCut_maxEnergy[1] << std::endl;
  std::cout << " CRF_timeCut_maxEnergy2 [ECLCRFinder]: " << CRF_timeCut_maxEnergy[2] << std::endl;
  std::cout << std::endl;

  //..Create new payload
  Belle2::DBImportObjPtr<Belle2::ECLClusteringParameters> importer("ECLClusteringParameters");
  importer.construct();
  importer->setLMEnergyCut(LM_energy_cut);
  importer->setCRFEnergyCut(CRF_energy_cut);
  importer->setCRFTimeCut(CRF_time_cut);
  importer->setCRFTimeCutMaxEnergy(CRF_timeCut_maxEnergy);
  importer.import(Belle2::IntervalOfValidity(0, 0, -1, -1));

  std::cout << std::endl << "Successfully wrote payload ECLClusteringParameters with iov " <<  "0 0 -1 -1." << std::endl;
  std::cout << "To verify payload contents, use the command " << std::endl <<
            "   b2conditionsdb dump -f localdb/dbstore_ECLClusteringParameters_xxx.root" << std::endl;

}

