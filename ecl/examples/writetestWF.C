/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
/* test script to write a dummy ECL-WF.root file containing the
covariance matrix, waveform parameters and other parameters
used by the fitting algorithm.
To produce the root file in root
gSystem->Load("libdataobjects")
.L ecl/examples/writetestWF.C++
writetestWF()

The file CIdToEclData.txt defining the mapping of the crystal to
ECLWaveformData object has to be present in the same directory 
*/
#include <TFile.h>
#include <TTree.h>
#include <include/ecl/dataobjects/ECLWaveformData.h>
#include <iostream>
#include <fstream>
#include <vector>
using namespace std;
using namespace Belle2;
void writetestWF()
{

  ifstream ctoecldatafile("CIdToEclData.txt");
  int cid, group;
  bool readerr = false;
  vector<int> grmap[300];
  while ( ! ctoecldatafile.eof() ) {
    ctoecldatafile >> cid >> group;
    cout << cid << " " << group << endl;
    
    if ( ctoecldatafile.eof() ) break;
    if ( group <= 0 || group >=300 ) {
      cout << "Error" << endl;
      readerr = true;
      break;
    }
    grmap[ group ].push_back( cid+1 );
  }

  if ( ! readerr ) {

    for ( int n=0; n< 300; n++ ) {
      vector<int>& v = grmap[n];
      /*
      vector<int>::const_iterator i;
      for ( i= v.begin(); i != v.end(); ++i )
	cout << *i << " " ;
      */
      cout << v.size() << endl;
    }

    cout << "Done building map" << endl;

    TFile f("ECL-WF.root","recreate");
    TTree t("EclWF","Waveform and covariance matrix");
    TTree t2("EclAlgo","Fitting algorithm parameters");
    TTree t3("EclNoise","Electronic noise matrix");
  
    ECLWaveformData* data = new ECLWaveformData;
    ECLWFAlgoParams* algo = new ECLWFAlgoParams;
    ECLNoiseData* noise = new ECLNoiseData;

    Int_t ncellId1;
    Int_t cellId1[8736];
    Int_t ncellId2;
    Int_t cellId2[8736];
    Int_t ncellId3;
    Int_t cellId3[8736];
  
    

    t.Branch("CovarianceM",&data,256000);
    t.Branch("ncellId",&ncellId1,"ncellId/I");
    t.Branch("cellId",cellId1,"cellId[ncellId]/I");

    t2.Branch("Algopars",&algo,256000);
    t2.Branch("ncellId",&ncellId2,"ncellId/I");
    t2.Branch("cellId",cellId2,"cellId[ncellId]/I");

    t3.Branch("NoiseM",&noise,256000);
    t3.Branch("ncellId",&ncellId3,"ncellId/I");
    t3.Branch("cellId",cellId3,"cellId[ncellId]/I");

    for ( int n=1; n<300; n++) {
      vector<int>& v = grmap[n];
      if ( v.size() == 0 ) continue;
      ncellId1 = ncellId2 = v.size();
      for ( size_t i = 0; i< v.size(); ++i)
				cellId2[i] = cellId1[i] = v[i];
      for ( size_t idx=0; idx< data -> c_nElements; ++idx)
	data->setMatrixElement( idx ,  idx*n);
      for ( size_t idx=0; idx< data -> c_nParams; ++idx)
	data->setWaveformPar(idx, (float) idx) ; 
      t.Fill();
    
      algo->ka = 0;
      algo->kb = 1;                        
      algo->kc = 2;
      algo->y0Startr = 3;
      algo->chiThresh = 4;
      algo->k1Chi = 5;
      algo->k2Chi = 6;
      algo->hitThresh = 7;
      algo->lowAmpThresh = 8;   
      algo->skipThresh = 9;
      t2.Fill();    
    }
    
    ncellId3 = 0;
    for ( size_t idx=0; idx< noise -> c_nElements; ++idx)
      noise->m_matrixElement[ idx ] =  idx;
  
    t3.Fill();
    
    t.Write();t2.Write();t3.Write();
  }
}
