//---------------------------------------------------------------
// $Id$
//---------------------------------------------------------------
// Filename : TrgEclFAM.cc
// Section  : TRG ECL
// Owner    : InSoo Lee/Yuuji Unno
// Email    : islee@hep.hanyang.ac.kr / yunno@post.kek.jp
//---------------------------------------------------------------
// Description : A class to represent TRG ECL
//---------------------------------------------------------------
// $Log$
//---------------------------------------------------------------
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/StoreArray.h>
#include <framework/gearbox/Unit.h>
#include "framework/core/ModuleManager.h"
#include <framework/logging/Logger.h>

#include <ecl/geometry/ECLGeometryPar.h>
#include <ecl/dataobjects/ECLSimHit.h>
#include <ecl/dataobjects/ECLHit.h>

#include "trg/ecl/TrgEclFAM.h"
#include "trg/ecl/dataobjects/TRGECLDigi.h"
#include "trg//ecl/dataobjects/TRGECLDigi0.h"
#include "trg/ecl/dataobjects/TRGECLHit.h"

#include <stdlib.h>
#include <iostream>
#include <math.h>
#include <TRandom3.h>

using namespace std;
using namespace Belle2;
//using namespace TRG;
//
//
//
TrgEclFAM::TrgEclFAM(){
  
  _TCMap = new TrgEclMapping();
  
}
//
//
//
TrgEclFAM::~TrgEclFAM(){
  
  delete _TCMap;
  
}
//
//
//
void
TrgEclFAM::setup(int m_nEvent, 
		 int digi_method
		 ){
  
  //
  // prepare coefficient for fitting
  //
  readFAMDB();
  //
  int TableFlag = 1;//1: ECLHit ,2: ECLSimHit 
  // initialize parameters
  //
  for (int iTCIdm=0; iTCIdm<576; iTCIdm++) {
    for (int  iTime=0; iTime<80; iTime++) {
      TCEnergy[iTCIdm][iTime] = 0;
      TCTiming[iTCIdm][iTime] = 0;
    }
    TCEnergy_tot[iTCIdm] = 0;
    TCTiming_tot[iTCIdm] = 0;
  }
  for (int iTCIdm = 0; iTCIdm < 576; iTCIdm++) {
    for (int iii = 0; iii<20 ;iii++){
      TCFitEnergy[iTCIdm][iii]=0;
      TCFitTiming[iTCIdm][iii]=0;
    }
  }
  //
  // digitization0
  //
  getTCHit(TableFlag);
  cout << m_nEvent << endl;
  if      (digi_method==2){ digitization02(); } // no-fit method = backup method 1
  else if (digi_method==3){ digitization03(); } // orignal method = backup method 2
  else                    { digitization01(); } // fit method
  save(m_nEvent);
  //
  //
  //
  return;
}
//
//
//
void
TrgEclFAM::getTCHit(int TableFlag )
{
  //
  float (*E_cell)[80]  = new float[8736][80];
  float (*T_ave)[80]   = new float[8736][80]; // (1)Averaged time[ns] between hit point in Xtal and PD.
  float (*Tof_ave)[80] = new float[8736][80]; // (2)Averaged time[ns] between hit point in Xtal and IP.
  //  int nBinTime = 80; // "80" covers t=0-8us even for 100ns binning.
  for (int iXCell=0; iXCell<8736; iXCell++){
    for (int iBinTime=0; iBinTime<80; iBinTime++){
      E_cell[iXCell][iBinTime] = 0;
      T_ave[iXCell][iBinTime] = 0;
      Tof_ave[iXCell][iBinTime] = 0;
    }
  }
  int nBinTime = 80;
  ////-------------------------------------------------------------------
  //                          read Xtal data 
  //---------------------------------------------------------------------
  if(TableFlag ==1 ){// read  ECLHit table
    StoreArray<ECLHit> eclHitArray("ECLHits");
    int nHits_hit = eclHitArray.getEntries()-1;
    int TimeIndex = 0 ;
    
    //
    for (int iHits = 0; iHits < nHits_hit; iHits++) {
      // Get a hit
      ECLHit* aECLHit = eclHitArray[iHits];
      // Hit geom. info
      int hitCellId  = aECLHit->getCellId()-1;
      float hitE     = aECLHit->getEnergyDep() / Unit::GeV;
      float aveT     = aECLHit->getTimeAve();
      TimeIndex = (int)aveT/100;
      // "hitTO" [ns] is flight time of track at ECL from IP
      for (int iECLCell = 0; iECLCell < 8736; iECLCell++) {
	if (hitCellId == iECLCell) {
	  
     	  E_cell[iECLCell][TimeIndex]  = E_cell[iECLCell][TimeIndex]+hitE;
     	  T_ave[iECLCell][TimeIndex]   = aveT;  
	  
	}
      } // End loop crsyal 8736
    }
  }
  
  
  if(TableFlag==2){ // read ECLSimHit
    ECL::ECLGeometryPar * eclp = ECL::ECLGeometryPar::Instance();
    //=====================
    // Loop over all hits of steps
    //=====================
    StoreArray<ECLSimHit> eclArray("ECLSimHits");
    int nHits = eclArray.getEntries();
    //
    for (int iHits = 0; iHits < nHits; iHits++) {
      // Get a hit
      ECLSimHit* aECLSimHit = eclArray[iHits];
      
      int hitCellId  = aECLSimHit->getCellId()-1;
      float hitE     = aECLSimHit->getEnergyDep() / Unit::GeV;
      float hitTOF      = aECLSimHit->getFlightTime() / Unit::ns; 
      
      TVector3 HitInPos = aECLSimHit->getPosIn(); // [cm], Hit position in Xtal (based on from IP)
      TVector3 PosCell  = eclp->GetCrystalPos(hitCellId); // [cm], Xtal position (based on from IP)
      TVector3 VecCell  = eclp->GetCrystalVec(hitCellId);
      // "local_pos_r" = Distance between track hit in Xtal and 
      // rear surface(max=30, min=0) of the Xtal.
      float local_pos_r = (15.0 - (HitInPos - PosCell) * VecCell);
      for (int iECLCell = 0; iECLCell < 8736; iECLCell++) {
	if (hitCellId == iECLCell && hitTOF < 8000) {
	  // time interval is (0.5, 0.1, 0.05us) => (16, 80, 160bins) between T=0-8us.
	  int TimeIndex = (int) (hitTOF / 100);
	  E_cell[iECLCell][TimeIndex]  = E_cell[iECLCell][TimeIndex]  + hitE;
	  T_ave[iECLCell][TimeIndex]   = T_ave[iECLCell][TimeIndex]   + hitE * local_pos_r;
	  Tof_ave[iECLCell][TimeIndex] = Tof_ave[iECLCell][TimeIndex] + hitE * hitTOF;
	}
      } // End loop crsyal 8736
    }
    //
    //
    //
    //===============
    // Xtal energy and timing (0-8us, 0.2us interval, 40 bins)
    //===============
    for (int iECLCell = 0; iECLCell < 8736; iECLCell++) {
      for (int  TimeIndex = 0; TimeIndex < nBinTime; TimeIndex++) {
	
	if (E_cell[iECLCell][TimeIndex] < 1e-9) {continue;} // 0.01MeV cut
	
	T_ave[iECLCell][TimeIndex]   = T_ave[iECLCell][TimeIndex] / E_cell[iECLCell][TimeIndex];
	T_ave[iECLCell][TimeIndex]   = 
	  6.05 + 
	  0.0749  * T_ave[iECLCell][TimeIndex] - 
	  0.00112 * T_ave[iECLCell][TimeIndex] * T_ave[iECLCell][TimeIndex];
	Tof_ave[iECLCell][TimeIndex] = Tof_ave[iECLCell][TimeIndex] / E_cell[iECLCell][TimeIndex];
      } // 40bins,  Time interval = 200ns
    }
    //
    //
    //
    
    
  }
  for (int iXtalIdm = 0; iXtalIdm < 8736; iXtalIdm++) {
    int iTCIdm = _TCMap->getTCIdFromXtalId(iXtalIdm+1)-1;
    for (int  iTime = 0; iTime < nBinTime; iTime++) {
      if ( E_cell[iXtalIdm][iTime] < 1e-9) {continue;} // 0.01MeV cut
      TCEnergy[iTCIdm][iTime] += E_cell[iXtalIdm][iTime];
      TCTiming[iTCIdm][iTime] += E_cell[iXtalIdm][iTime]*(T_ave[iXtalIdm][iTime]);
    }
  }
  for (int iTCIdm=0; iTCIdm<576; iTCIdm++) {
    for (int  iTime = 0; iTime < nBinTime; iTime++) {
      if ( TCEnergy[iTCIdm][iTime] < 1e-9) {continue;} // 0.01MeV cut
      TCTiming[iTCIdm][iTime] /= TCEnergy[iTCIdm][iTime];
    }
  }
  
  //-------------------------------------------------
  // Save Raw data
  //--------------------------------------------
  for (int iTCIdm=0; iTCIdm<576; iTCIdm++) {
    ninput[iTCIdm] = 0;
    for (int  iTime = 0; iTime < nBinTime; iTime++) {
      if ( TCEnergy[iTCIdm][iTime] >= 0.05) {
	TCRawEnergy[iTCIdm][ninput[iTCIdm]] = TCEnergy[iTCIdm][iTime];
	TCRawTiming[iTCIdm][ninput[iTCIdm]] = TCTiming[iTCIdm][iTime];
	ninput[iTCIdm]++;
      }
    }
  }
  
  
  //--------------------------
  // TC energy and timing in t=0-1us as true values.
  //--------------------------
  int nBin_1us = (int) (1000.0 / 100);
  for (int iTCIdm=0; iTCIdm<576; iTCIdm++) {
    for (int  iTime = 0; iTime < nBin_1us; iTime++) {
      TCEnergy_tot[iTCIdm] += TCEnergy[iTCIdm][iTime];
      TCTiming_tot[iTCIdm] += TCTiming[iTCIdm][iTime]*TCEnergy[iTCIdm][iTime];
    }
    TCTiming_tot[iTCIdm] /= TCEnergy_tot[iTCIdm];
    
  }
  
  
  //
  delete [] E_cell;
  delete [] T_ave;
  delete [] Tof_ave;
  return;
}
//
//
//
void
TrgEclFAM::digitization01( void ){
  //
  double cut_energy_tot = 0.03; // [GeV]
  int nbin_pedestal = 5; // = nbin_pedestal*fam_sampling_interval [ns] in total
  double fam_sampling_interval = 96; // [ns]
  double TCDigiE[576][160] = {{0}};  // [GeV]
  double TCDigiT[576][160] = {{0}};  // [ns]
  int NSampling = 80; // # of sampling array
  
  TRandom3* rand00 = new TRandom3(0);
  // (Make sampling time random between FAM sampling intervals)
  double random_sampling_correction = 0; // [ns]
  random_sampling_correction = rand00->Rndm()*fam_sampling_interval;
  //==================
  // (01)Signal digitization
  //==================
  for (int iTCIdm=0; iTCIdm<576; iTCIdm++) {
    if (TCEnergy_tot[iTCIdm]<cut_energy_tot){continue;} // TC energy_tot cut
    for (int iTimeBin=0; iTimeBin<80; iTimeBin++) {
      if (TCEnergy[iTCIdm][iTimeBin]<0.0001){continue;} // 0.1MeV cut on TC bin_energy
      for (int iSampling=0; iSampling<NSampling; iSampling++) {
	// inputTiming is in [us] <-- Be careful, here is NOT [ns]
	double inputTiming 
	  = (-TCTiming[iTCIdm][iTimeBin]+(-nbin_pedestal+iSampling)*fam_sampling_interval)*0.001; 
	inputTiming += random_sampling_correction*0.001;
	if (inputTiming>0){
	  TCDigiE[iTCIdm][iSampling] += FADC(0, inputTiming) * TCEnergy[iTCIdm][iTimeBin];
	}
      }
      for (int iSampling=0; iSampling<NSampling; iSampling++) {
	TCDigiT[iTCIdm][iSampling] = (-nbin_pedestal+iSampling)*fam_sampling_interval;
	TCDigiT[iTCIdm][iSampling] += random_sampling_correction;
      }
    }
  }
  //
  //
  //
  if (0){
    FILE *f_out_dat = fopen("ztsim.no_noise.dat", "w");
    for (int iTCIdm=0; iTCIdm<576; iTCIdm++) {
      if ( TCEnergy_tot[iTCIdm] < cut_energy_tot ) { continue; } // TC energy_tot cut
      fprintf(f_out_dat, "%5i %8.5f %8.5f %8.1f ", 
	      iTCIdm+1, TCEnergy_tot[iTCIdm], TCEnergy[iTCIdm][0], TCDigiT[iTCIdm][0]);
      for (int iSampling=0; iSampling<NSampling; iSampling++) {
	fprintf(f_out_dat, "%7.4f ", TCDigiE[iTCIdm][iSampling]);
      }
      fprintf(f_out_dat, "\n");
    }
    fclose(f_out_dat);
  }
  //==================
  // (01)noise embedding
  //==================
  double tmin_noise = -nbin_pedestal*fam_sampling_interval*0.001; // [us]
  double tgen = NSampling*fam_sampling_interval*0.001-tmin_noise; // [us] 
  tmin_noise = -4; // orignal
  tgen = 10.3;   // orignal   
  int bkg_level = 1030;
  double ttt0 = 0; // [us]
  double ttt1 = 0; // [us]
  double ttt2 = 0; // [us]
  //
  double frac_pileup   = 0.035; // pileup noise fraction? 
  double frac_parallel = 0.023; // parralel noise fraction?
  double frac_serial   = 0.055; // serial noise fraction?
  double times_pileup   =  1;   // noise scale based on Belle noise.
  double times_parallel =  1;   // noise scale
  double times_serial   =  1;   // noise scale
  double corr_pileup   = times_pileup   * frac_pileup   * sqrt(fam_sampling_interval*0.001);
  double corr_parallel = times_parallel * frac_parallel * sqrt(fam_sampling_interval*0.001);
  double corr_serial   = times_serial   * frac_serial   * sqrt(fam_sampling_interval*0.001);
  // corr_pileup   = 0.011068;   // default in case 1xBelle noise in 100ns sampling
  // corr_parallel = 0.00727324; // default in case 1xBelle noise in 100ns sampling
  // corr_serial   = 0.0173925;  // default in case 1xBelle noise in 100ns sampling
  
  for (int iTCIdm=0; iTCIdm<576; iTCIdm++) {
    if ( TCEnergy_tot[iTCIdm] < cut_energy_tot ) { continue; } // TC energy_tot cut
    for (int jjj=0; jjj<bkg_level; jjj++){
      ttt0 = -(tmin_noise + tgen * rand00->Rndm());  // [us]
      ttt1 = -(tmin_noise + tgen * rand00->Rndm());  // [us]
      ttt2 = -(tmin_noise + tgen * rand00->Rndm());  // [us]
      for (int iSampling=0; iSampling<NSampling; iSampling++) {
    	// (pile-up noise)
    	if (ttt0>0){ TCDigiE[iTCIdm][iSampling] += FADC(0,ttt0) * corr_pileup * 0.001; }
    	// (parallel noise)
    	if (ttt1>0){ TCDigiE[iTCIdm][iSampling] += FADC(1,ttt1) * corr_parallel; }
    	// (serial noise)
    	if (ttt2>0){ TCDigiE[iTCIdm][iSampling] += FADC(2,ttt2) * corr_serial; }
    	ttt0 += fam_sampling_interval*0.001;
    	ttt1 += fam_sampling_interval*0.001;
    	ttt2 += fam_sampling_interval*0.001;
      }
    }
  }
  delete rand00;
  
  
  //
  //
  //
  if (0){
    FILE *f_out_dat = fopen("ztsim.noise.dat", "w");
    for (int iTCIdm=0; iTCIdm<576; iTCIdm++) {
      if ( TCEnergy_tot[iTCIdm] < cut_energy_tot ) { continue; } // TC energy_tot cut
      fprintf(f_out_dat, "%5i %8.5f %8.5f %8.1f ", 
	      iTCIdm+1, TCEnergy_tot[iTCIdm], TCEnergy[iTCIdm][0], TCDigiT[iTCIdm][0]);
      for (int iSampling=0; iSampling<NSampling; iSampling++) {
	fprintf(f_out_dat, "%7.4f ", TCDigiE[iTCIdm][iSampling]);
      }
      fprintf(f_out_dat, "\n");
    }
    fclose(f_out_dat);
  }
  //============
  // (01)fit
  //============
  for(int jjj=0; jjj <20; jjj++){
    returnE[jjj]=  0  ;
    returnT[jjj]=  0   ;
  }
  
  
  
  for (int iTCIdm=0; iTCIdm<576; iTCIdm++) {
    noutput[iTCIdm] = 0;
    if (TCEnergy_tot[iTCIdm] < cut_energy_tot){continue;} // TC energy_tot cut
    
    FAMFit(nbin_pedestal, 
	   NSampling, 
	   iTCIdm,
	   fam_sampling_interval, 
	   TCDigiE[iTCIdm] );
    
    for(int inoutput=0; inoutput<noutput[iTCIdm];inoutput++){
      TCFitEnergy[iTCIdm][inoutput] = returnE[inoutput];
      TCFitTiming[iTCIdm][inoutput] = returnT[inoutput]+random_sampling_correction;
    }
  }
  //
  //
  //
  return;
  
}
//
//
//
void
TrgEclFAM::digitization02( void ){
  
  double cut_energy_tot = 0.03; // [GeV]
  int nbin_pedestal = 5; // = nbin_pedestal*fam_sampling_interval [ns] in total
  double fam_sampling_interval = 96; //@ [ns]
  double TCDigiE[576][160] = {{0}};  //@ [GeV]
  double TCDigiT[576][160] = {{0}};  //@ [ns]
  int NSampling = 80;
  TRandom3* rand00 = new TRandom3(0);
  //@ Make sampling time random between FAM sampling intervals
  double random_sampling_correction = 0; //@ [ns]
  random_sampling_correction = rand00->Rndm()*fam_sampling_interval;
  //==================
  // (02)Signal digitization
  //==================
  for (int iTCIdm=0; iTCIdm<576; iTCIdm++) {
    if (TCEnergy_tot[iTCIdm]<cut_energy_tot){continue;} // TC energy_tot cut
    for (int iTimeBin=0; iTimeBin<80; iTimeBin++) {
      if (TCEnergy[iTCIdm][iTimeBin]<0.0001){continue;} // 0.1 MeV cut on TC bin_energy
      cout << TCTiming[iTCIdm][iTimeBin] << endl;
      for (int iSampling=0; iSampling<NSampling; iSampling++) {
	//@ inputTiming is in [us] <-- Be careful, here is NOT [ns]
	double inputTiming 
	  = (-TCTiming[iTCIdm][iTimeBin]+(-nbin_pedestal+iSampling)*fam_sampling_interval)*0.001; 
	inputTiming += random_sampling_correction*0.001;
	TCDigiE[iTCIdm][iSampling] += FADC(0, inputTiming) * TCEnergy[iTCIdm][iTimeBin];
      }
      for (int iSampling=0; iSampling<NSampling; iSampling++) {
	TCDigiT[iTCIdm][iSampling] = (-nbin_pedestal+iSampling)*fam_sampling_interval;
	TCDigiT[iTCIdm][iSampling] += random_sampling_correction;
      }
    }
  }
  //==================
  // (02)noise embedding
  //==================
  double tmin_noise = -nbin_pedestal*fam_sampling_interval*0.001; // [us]
  double tgen =  NSampling*fam_sampling_interval*0.001-tmin_noise; // [us] 
  tmin_noise = -4; // orignal
  tgen = 10.3;   // 
  int bkg_level = 1030;
  double ttt0 = 0; //@ [us]
  double ttt1 = 0; //@ [us]
  double ttt2 = 0; //@ [us]
  double frac_pileup   = 0.035; // pileup noise fraction? 
  double frac_parallel = 0.023; // parralel noise fraction?
  double frac_serial   = 0.055; // serial noise fraction?
  double times_pileup   =  1;   // noise scale based on Belle noise.
  double times_parallel =  1;   // noise scale
  double times_serial   =  1;   // noise scale
  double corr_pileup   = times_pileup   * frac_pileup   * sqrt(fam_sampling_interval*0.001);
  double corr_parallel = times_parallel * frac_parallel * sqrt(fam_sampling_interval*0.001);
  double corr_serial   = times_serial   * frac_serial   * sqrt(fam_sampling_interval*0.001);  
  // corr_pileup   = 0.011068;
  // corr_parallel = 0.00727324;
  // corr_serial   = 0.0173925;
  
  
  for (int iTCIdm=0; iTCIdm<576; iTCIdm++) {
    if ( TCEnergy_tot[iTCIdm] < cut_energy_tot ) { continue; } // TC energy_tot cut
    for (int jjj=0; jjj<bkg_level; jjj++){
      ttt0 = -(tmin_noise + tgen * rand00->Rndm());  //@ [us]
      ttt1 = -(tmin_noise + tgen * rand00->Rndm());  //@ [us]
      ttt2 = -(tmin_noise + tgen * rand00->Rndm());  //@ [us]
      for (int iSampling=0; iSampling<NSampling; iSampling++) {
  	//@ (pile-up noise)
  	if (ttt0>0){ TCDigiE[iTCIdm][iSampling] += FADC(0,ttt0) * corr_pileup * 0.001; }
  	//@ (parallel noise)
  	if (ttt1>0){ TCDigiE[iTCIdm][iSampling] += FADC(1,ttt1) * corr_parallel; }
  	//@ (serial noise)
  	if (ttt2>0){ TCDigiE[iTCIdm][iSampling] += FADC(2,ttt2) * corr_serial; }
  	ttt0 += fam_sampling_interval*0.001;
  	ttt1 += fam_sampling_interval*0.001;
  	ttt2 += fam_sampling_interval*0.001;
      }
    }
  }
  
  
  
  delete rand00;
  //==================
  // (02)Peak search
  //==================
  //@ T_a and T_b is time at sampling points in which 0.6*E exists.
  int ta_id[20] = {1000}; //@ id of T_a
  double ttt_a[20]={0}; //@ time of T_a
  double ttt_b[20]={0}; //@ time of T_b
  for (int iTCIdm = 0; iTCIdm < 576; iTCIdm++) {
    noutput[iTCIdm]=0;
    if ( TCEnergy_tot[iTCIdm] < cut_energy_tot ) { continue; } // TC energy_tot cut
    
    
    double threshold = 0.05; //GeV
    int maxId[500] = {0};
    int count_up = 0;
    int count_down = 0;
    int flag_up = 3;
    int flag_down = 3;
    
    double max = 0;
    for (int iSampling=1; iSampling<NSampling; iSampling++) {
      //-------------------------------------------------------------------------
      //Peak finding Method 1 
      //------------------------------------------------------------------------
      
      if (TCDigiE[iTCIdm][iSampling]>=max){
	
	max = TCDigiE[iTCIdm][iSampling];
	maxId[noutput[iTCIdm]]= iSampling;
	count_up ++;
	count_down =0;
      }
      else{
	count_down++;
	if(count_down >= flag_down){
	  if(count_up >= flag_up){
	    if(threshold<max){
	      max =0;
	      count_up =0;
	      count_down = 0;
	      
	      double NoiseLevel = 0;  
	      double NoiseCount = 0;
	      for (int iNoise=0; iNoise<5; iNoise++){
		int iNoiseReplace = (maxId[noutput[iTCIdm]]-10) + iNoise;
		if (iNoiseReplace>=0){
		  NoiseLevel += TCDigiE[iTCIdm][iNoiseReplace];
		  NoiseCount++;
		}
	      }
	      if (NoiseCount!=0) { NoiseLevel /= NoiseCount; }
	      TCFitEnergy[iTCIdm][noutput[iTCIdm]] = TCDigiE[iTCIdm][maxId[noutput[iTCIdm]]]-NoiseLevel;
	      if(!(maxId[noutput[iTCIdm]]-1)){
		for (int iSampling=1; iSampling<maxId[noutput[iTCIdm]]+3; iSampling++) {
		  TCDigiE[iTCIdm][iSampling] -= NoiseLevel;
		}
	      }
	      else{
		for (int iSampling=maxId[noutput[iTCIdm]-1]; iSampling<maxId[noutput[iTCIdm]]+3; iSampling++) {
		  TCDigiE[iTCIdm][iSampling] -= NoiseLevel;
		}
	      }
	      //@ Search T_a ID
	      for (int iSearch=0; iSearch<5; iSearch++){
		
		if ( TCDigiE[iTCIdm][maxId[noutput[iTCIdm]]-iSearch]  >0.6*TCFitEnergy[iTCIdm][noutput[iTCIdm]] &&
		     TCDigiE[iTCIdm][maxId[noutput[iTCIdm]]-iSearch-1]<0.6*TCFitEnergy[iTCIdm][noutput[iTCIdm]] ){
		  ta_id[noutput[iTCIdm]] = maxId[noutput[iTCIdm]]-iSearch-1;
		}
	      }
	      
	      //@ Estimate timing of t0
	      if (ta_id[noutput[iTCIdm]]==1000){ 
		printf("TrgEclFAM::digi02> Cannot find TC Timing (TCId=%5i, E=%8.5f)!!!\n", iTCIdm-1, TCFitEnergy[iTCIdm][0]); 
		B2ERROR("TrgEclFAM::digi02> Cannot find TC Timing"); 
	      } else {
		ttt_a[noutput[iTCIdm]] = TCDigiT[iTCIdm][ta_id[noutput[iTCIdm]]];
		ttt_b[noutput[iTCIdm]] = TCDigiT[iTCIdm][ta_id[noutput[iTCIdm]]+1];
		TCFitTiming[iTCIdm][noutput[iTCIdm]] = 
		  ttt_a[noutput[iTCIdm]]+
		  (0.6*TCFitEnergy[iTCIdm][noutput[iTCIdm]]-TCDigiE[iTCIdm][ta_id[noutput[iTCIdm]]])*(ttt_b[noutput[iTCIdm]]-ttt_a[noutput[iTCIdm]])
		  /(TCDigiE[iTCIdm][ta_id[noutput[iTCIdm]]+1]-TCDigiE[iTCIdm][ta_id[noutput[iTCIdm]]]);
		//@ time between t0 and 0.6*peak_energy
		//@ Alex's number = 274.4 (how he got this value ?)
		//@ by my check = 278.7 [ns]
		TCFitTiming[iTCIdm][noutput[iTCIdm]] -= (278.7 + 2);
		//@ here "+2" is a shift due to imperfectness of no-fit method.
	      }
	      noutput[iTCIdm]++;
	    }
	  }
	}
      }
    }
  }
  //
  //
  //
  return;
  
}
//
//
//
void
TrgEclFAM::digitization03( void ){
  //===============
  // (03)Signal digitization (w/ 12ns interval for method-0)
  //===============
  double cut_energy_tot = 0.03; // [GeV]
  int nbin_pedestal = 100;
  float fam_sampling_interval = 12; // [ns]
  float TCDigiE[576][666] = {{0}};  // [GeV]
  float TCDigiT[576][666] = {{0}};  // [ns]
  int NSampling = 666;
  TRandom3* rand00 = new TRandom3(0);
  // Make sampling time random between FAM sampling intervals
  float random_sampling_correction = 0; // [ns]
  random_sampling_correction = rand00->Rndm()*fam_sampling_interval;
  for (int iTCIdm=0; iTCIdm<576; iTCIdm++) {
    if (TCEnergy_tot[iTCIdm]<cut_energy_tot){continue;} // TC energy_tot cut
    for (int iTimeBin=0; iTimeBin<80; iTimeBin++) {
      if ( TCEnergy[iTCIdm][iTimeBin] < 0.0001 ) { continue; } // 0.1 MeV cut on TC bin_energy
      for (int iSampling=0; iSampling<NSampling; iSampling++) {
	// inputTiming is in [us] <-- Be careful, here is NOT [ns]
	float inputTiming 
	  = (-TCTiming[iTCIdm][iTimeBin]+(-nbin_pedestal+iSampling)*fam_sampling_interval)*0.001; 
	inputTiming += random_sampling_correction*0.001;
	TCDigiE[iTCIdm][iSampling] += FADC(0, inputTiming) * TCEnergy[iTCIdm][iTimeBin];
      }
    }
    for (int iSampling=0; iSampling<NSampling; iSampling++) {
      TCDigiT[iTCIdm][iSampling] = (-nbin_pedestal+iSampling)*fam_sampling_interval;
      TCDigiT[iTCIdm][iSampling] += random_sampling_correction;
    }
  }
  //==================
  // (03)noise embedding
  //==================
  double tmin_noise = -nbin_pedestal*fam_sampling_interval*0.001; // [us]
  double tgen = NSampling*fam_sampling_interval*0.001-tmin_noise; // [us] 
  tmin_noise = -4; // orignal
  tgen = 10.3;   // 
  int bkg_level = 1030;
  double ttt0 = 0; // [us]
  double ttt1 = 0; // [us]
  double ttt2 = 0; // [us]
  double frac_pileup   = 0.035; // pileup noise fraction? 
  double frac_parallel = 0.023; // parralel noise fraction?
  double frac_serial   = 0.055; // serial noise fraction?
  double times_pileup   =  1;   // noise scale based on Belle noise.
  double times_parallel =  1;   // noise scale
  double times_serial   =  1;   // noise scale
  double corr_pileup   = times_pileup   * frac_pileup   * sqrt(fam_sampling_interval*0.001);
  double corr_parallel = times_parallel * frac_parallel * sqrt(fam_sampling_interval*0.001);
  double corr_serial   = times_serial   * frac_serial   * sqrt(fam_sampling_interval*0.001);  
  corr_pileup   = 0.011068;
  corr_parallel = 0.00727324;
  corr_serial   = 0.0173925;
  
  
  for (int iTCIdm=0; iTCIdm<576; iTCIdm++) {
    if ( TCEnergy_tot[iTCIdm] < cut_energy_tot ) { continue; } // 1 MeV TC energy cut
    for (int jjj=0; jjj<bkg_level; jjj++){
      ttt0 = -(tmin_noise + tgen * rand00->Rndm());  // [us]
      ttt1 = -(tmin_noise + tgen * rand00->Rndm());  // [us]
      ttt2 = -(tmin_noise + tgen * rand00->Rndm());  // [us]
      for (int iSampling=0; iSampling<NSampling; iSampling++) {
 	// (pile-up noise)
 	if (ttt0>0){ TCDigiE[iTCIdm][iSampling] += FADC(0,ttt0) * corr_pileup * 0.001; }
 	// (parallel noise)
 	if (ttt1>0){ TCDigiE[iTCIdm][iSampling] += FADC(1,ttt1) * corr_parallel; }
 	// (serial noise)
 	if (ttt2>0){ TCDigiE[iTCIdm][iSampling] += FADC(2,ttt2) * corr_serial; }
 	ttt0 += fam_sampling_interval*0.001;
 	ttt1 += fam_sampling_interval*0.001;
 	ttt2 += fam_sampling_interval*0.001;
      }
    }
  }
  delete rand00;
  
  
  //
  //
  //
  if (0){
    FILE *f_out_dat;
    f_out_dat = fopen("ztsim.dat", "w");
    for (int iTCIdm=0; iTCIdm<576; iTCIdm++) {
      if ( TCEnergy[iTCIdm][0] < 0.001 ) { continue; } // 1 MeV TC energy cut
      fprintf(f_out_dat, "%5i %8.5f %8.5f ", 
	      iTCIdm+1, TCEnergy[iTCIdm][0], TCTiming[iTCIdm][0]);
      for (int iSampling=0; iSampling<NSampling; iSampling++) {
	fprintf(f_out_dat, "%3i %8.5f\n", iSampling, TCDigiE[iTCIdm][iSampling]);
      }
      fprintf(f_out_dat, "\n");
    }
    fclose(f_out_dat);
  }
  //==================
  // (03)Peak search
  //==================
  float max_shape_time = 563.48; // [ns], time between peak of PDF and t0.
  double threshold = 0.1; //GeV
  for (int iTCIdm = 0; iTCIdm < 576; iTCIdm++) {
    noutput[iTCIdm]=0;
    if ( TCEnergy_tot[iTCIdm] < cut_energy_tot ) { continue; } // TC energy_tot cut
    int maxId[500] = {0};
    int count_up = 0;
    int count_down = 0;
    int flag_up = 30;;
    int flag_down = 40;
    double max = 0;
    for (int iSampling=1; iSampling<NSampling; iSampling++) {
      
      if (TCDigiE[iTCIdm][iSampling]>=max){
	
	max = TCDigiE[iTCIdm][iSampling];
	maxId[noutput[iTCIdm]]= iSampling;
	count_up ++;
	count_down =0;
      }
      else{
	count_down++;
	if(count_down >= flag_down){
	  if(count_up >= flag_up){
	    if(threshold<max){
	      max =0;
	      count_up =0;
	      count_down = 0;
	      //@ Remove noise effect
	      float NoiseLevel = 0;  
	      float NoiseCount = 0;
	      for (int iNoise=0; iNoise<42; iNoise++){
		int iNoiseReplace = (maxId[noutput[iTCIdm]]-88) + iNoise;
		if (iNoiseReplace>=0){
		  NoiseLevel += TCDigiE[iTCIdm][iNoiseReplace];
		  NoiseCount++;
		}
	      }
	      if (NoiseCount!=0) { NoiseLevel /= NoiseCount; }
	      TCFitEnergy[iTCIdm][noutput[iTCIdm]] = TCDigiE[iTCIdm][maxId[noutput[iTCIdm]]]-NoiseLevel;
	      TCFitTiming[iTCIdm][noutput[iTCIdm]] = TCDigiT[iTCIdm][maxId[noutput[iTCIdm]]]-max_shape_time;
	      noutput[iTCIdm]++;
	    }
	  }
	}
      }
    }
  }
  return;
}
//
//
//
void
TrgEclFAM::save(int m_nEvent){
  //---------------
  // Root Output
  //---------------
  int m_hitNum = 0;
  for (int iTCIdm = 0; iTCIdm < 576; iTCIdm++) {
    for(int ininput=0; ininput< ninput[iTCIdm] ; ininput++){
      if ( TCFitEnergy[iTCIdm][ininput] < 0.05 ) { 
	continue;  // 50 MeV Fitted TC energy cut
      }
      if ( TCFitEnergy[iTCIdm][ininput]==0&& TCFitTiming[iTCIdm][ininput]==0 ) { 
	continue;
      }
      if(noutput[iTCIdm] != ninput[iTCIdm]){continue;}
      StoreArray<TRGECLDigi> TCDigiArray;
      if (!TCDigiArray) TCDigiArray.create();
      new(TCDigiArray.nextFreeAddress()) TRGECLDigi();
      m_hitNum = TCDigiArray.getEntries() - 1;
      TCDigiArray[m_hitNum]->setEventId(m_nEvent);
      TCDigiArray[m_hitNum]->setTCId(iTCIdm+1);
      TCDigiArray[m_hitNum]->setTCnOutput(ininput+1);
      TCDigiArray[m_hitNum]->setPhiId(_TCMap->getTCPhiIdFromTCId(iTCIdm+1));
      TCDigiArray[m_hitNum]->setThetaId(_TCMap->getTCThetaIdFromTCId(iTCIdm+1));
      TCDigiArray[m_hitNum]->setRawEnergy(TCRawEnergy[iTCIdm][ininput]);
      TCDigiArray[m_hitNum]->setRawTiming(TCRawTiming[iTCIdm][ininput]);
      TCDigiArray[m_hitNum]->setRawEnergyTot(TCEnergy_tot[iTCIdm]);
      TCDigiArray[m_hitNum]->setRawTimingTot(TCTiming_tot[iTCIdm]);
      TCDigiArray[m_hitNum]->setFitEnergy(TCFitEnergy[iTCIdm][ininput]);
      TCDigiArray[m_hitNum]->setFitTiming(TCFitTiming[iTCIdm][ininput]);
    }
  }
  m_hitNum = 0;
  for (int iTCIdm = 0; iTCIdm < 576; iTCIdm++) {
    for (int iBinTime = 0; iBinTime < 80; iBinTime++) {
      if ( TCEnergy[iTCIdm][iBinTime] < 0.001 ) {continue;}
      StoreArray<TRGECLDigi0> TCDigiArray;
      if (!TCDigiArray) TCDigiArray.create();
      new(TCDigiArray.nextFreeAddress()) TRGECLDigi0();
      
      m_hitNum = TCDigiArray.getEntries() - 1;
      
      TCDigiArray[m_hitNum]->setEventId(m_nEvent);
      TCDigiArray[m_hitNum]->setTCId(iTCIdm+1);
      TCDigiArray[m_hitNum]->setiBinTime(iBinTime);
      TCDigiArray[m_hitNum]->setRawEnergy(TCEnergy[iTCIdm][iBinTime]);
      TCDigiArray[m_hitNum]->setRawTiming(TCTiming[iTCIdm][iBinTime]);
    }
  }
  m_hitNum = 0;
  
  for (int iTCIdm = 0; iTCIdm < 576;  iTCIdm++) {
    for(int inoutput=0; inoutput< noutput[iTCIdm] ; inoutput++){
      if ( TCFitEnergy[iTCIdm][inoutput] < 0.001 ) { 
	continue;  // 50 MeV Fitted TC energy cut
      }
      if ( TCFitEnergy[iTCIdm][inoutput]==0&& TCFitTiming[iTCIdm][inoutput]==0 ) { 
	continue;
      }
      StoreArray<TRGECLHit> TrgEclHitArray;
      if (!TrgEclHitArray) TrgEclHitArray.create();
      
      new(TrgEclHitArray.nextFreeAddress()) TRGECLHit();
      m_hitNum = TrgEclHitArray.getEntries() - 1;
      TrgEclHitArray[m_hitNum]->setEventId(m_nEvent);
      TrgEclHitArray[m_hitNum]->setTCId(iTCIdm+ 1);
      TrgEclHitArray[m_hitNum]->setTCOutput(inoutput+1);
      TrgEclHitArray[m_hitNum]->setEnergyDep(TCFitEnergy[iTCIdm][inoutput]);
      TrgEclHitArray[m_hitNum] ->setTimeAve(TCFitTiming[iTCIdm][inoutput]);
    }
  }
  for (int iTCIdm = 0; iTCIdm < 576; iTCIdm++) {
    
    _tcnoutput.push_back(noutput[iTCIdm]);
    
    _tcid.push_back(iTCIdm + 1);
    
    _tcthetaid.push_back(_TCMap->getTCThetaIdFromTCId(iTCIdm+1));
    
    _tcphiid.push_back(_TCMap->getTCPhiIdFromTCId(iTCIdm+1));
    
  }
  //
  //
  //
  return;
}
//
//
//
double
TrgEclFAM::FADC(int flag_gen, 
		double timing){
  
  //--------------------------------------
  //
  // o "timing" unit is [us]
  // o flag_gen = 0(=signal), 1(=parallel), 2(=serial)
  // o return value(PDF) is [GeV]
  //
  //--------------------------------------
  double tsh,dd;
  static double tc,fm,fff,tt,dt,tc2,tm,tsc,tris;
  static double amp,td,t1,b1,t2,b2,ts,dft,as;
  int im,ij;
  
  static int ifir=0;
  
  if(ifir==0){
    
    td =  0.10;  // diff time    (  0.10)
    t1 =  0.10;  // integ1 real  (  0.10)
    b1 = 30.90;  // integ1 imag  ( 30.90)
    t2 =  0.01;  // integ2 real  (  0.01)
    b2 = 30.01;  // integ2 imag  ( 30.01)
    ts =  1.00;  // scint decay  (  1.00)
    dft= 0.600;  // diff delay   ( 0.600)
    as = 0.548;  // diff frac    ( 0.548)
    //
    amp = 1.0;
    tris = 0.01;
    tsc = ts;
    //
    im = 0;
    ij = 0;
    fm = 0;
    tc = 0;
    tt = u_max(u_max(td,t1),u_max(t2,ts))*2;
    int flag_once = 0;
    while(flag_once == 0){
      dt = tt/1000;
      for(int j=1; j<=1000; j++){
	tc2 = tc-dft;
	fff = 
	  (ShapeF(tc, t1,b1,t2,b2,td,tsc)-
	   ShapeF(tc, t1,b1,t2,b2,td,tris)*0.01)-
	  (ShapeF(tc2,t1,b1,t2,b2,td,tsc)-
	   ShapeF(tc2,t1,b1,t2,b2,td,tris)*0.01)*as;
	if( fff>fm ){
	  fm = fff;
	  tm = tc;
	  im = j;
	}
	tc = tc+dt;
      }
      if( im>=1000 ){
	tt = 2*tt;
	flag_once = 0;
	continue;
      }
      if( ij==0 ){
	ij = 1;
	tc = 0.99*tm;
	dt = tm*0.02/1000;
	flag_once = 0;
	continue;
      }
      flag_once = 1;
    }
    amp = 1.0/fm;
    ifir = 1;
  }
  //
  //
  double pdf = 0;
  if ( flag_gen==0 ){
    //-----<signal>
    tc2 = timing-dft;
    pdf = amp*(
	       (ShapeF(timing,  t1,b1,t2,b2,td,tsc)-
		ShapeF(timing,  t1,b1,t2,b2,td,tris)*0.01)-
	       (ShapeF(tc2,       t1,b1,t2,b2,td,tsc)-
		ShapeF(tc2,       t1,b1,t2,b2,td,tris)*0.01)*as);
  }else if( flag_gen==1 ){
    //-----<parallel>
    tc2 = timing-dft;
    tsh = 0.001;
    pdf = amp*(
	       ShapeF(timing,  t1,b1,t2,b2,td,tsh)-
	       ShapeF(tc2,       t1,b1,t2,b2,td,tsh)*as);
    pdf = pdf*0.001; // GeV
  }else{
    //-----<serial>
    tc2 = timing-dft;
    tsh = 0.001;
    pdf = amp*(
	       ShapeF(timing,  t1,b1,t2,b2,td,tsh)-
	       ShapeF(tc2,       t1,b1,t2,b2,td,tsh)*as);
    //
    tc = timing-0.01;
    if ( tc<0 ){ tc = 0; }
    dd = timing-tc;
    tc2 = tc-dft;
    pdf = (amp*(
		ShapeF(tc, t1,b1,t2,b2,td,tsh)-
		ShapeF(tc2,t1,b1,t2,b2,td,tsh)*as)-pdf)/dd;
    pdf = pdf*0.001; // GeV
  }
  
  return pdf;
}
//
//
//
double
TrgEclFAM::ShapeF(double t00,
		  double t01,
		  double tb1,
		  double t02,
		  double tb2,
		  double td1,
		  double ts1){
  
  double dr;
  double dzna;
  double das1,das0,dac0;
  double dcs0s,dsn0s,dcs0d,dsn0d;
  double dcs1s,dsn1s,dcs1d,dsn1d;
  
  double sv123 = 0.0;
  if ( t00<0 ) return 0;
  
  dr = (ts1-td1)/td1;
  if(fabs(dr)<=1.0e-5){
    if( ts1>td1 ){ ts1 = td1*1.00001; }
    else         { ts1 = td1*0.99999; }
  }
  //
  dr = (pow((t01-t02),2)+pow((tb1-tb2),2)) / (pow((t01),2)+pow((tb1),2));
  if( dr<=1.e-10 ){
    if( t01<t02 ){ t01 = t02/1.00001; }
    else         { t01 = t02/0.99999; }
  }
  if ( t00<=0.0 ) return 0;
  //
  //
  //
  double a1 = 1/t01;
  double a2 = 1/tb1;
  double b1 = 1/t02;
  double b2 = 1/tb2;
  double c1 = 1/td1;
  double c2 = 1/ts1;
  
  das0 = b2*(pow((b1-a1),2)+(b2+a2)*(b2-a2));
  dac0 = -2*(b1-a1)*a2*b2;
  das1 = a2*(pow((b1-a1),2)-(b2+a2)*(b2-a2));
  
  dsn0s = ((c2-a1)*das0  -(-a2)*dac0)   /(pow(a2,2)+pow((c2-a1),2));
  dcs0s = (  (-a2)*das0+(c2-a1)*dac0)   /(pow(a2,2)+pow((c2-a1),2));
  
  dsn1s = ((c2-b1)*das1  -(-b2)*(-dac0))/(pow(b2,2)+pow((c2-b1),2));
  dcs1s = (  (-b2)*das1+(c2-b1)*(-dac0))/(pow(b2,2)+pow((c2-b1),2));
  
  dsn0d = ((c1-a1)*das0  -(-a2)*dac0)   /(pow(a2,2)+pow((c1-a1),2));
  dcs0d = (  (-a2)*das0+(c1-a1)*dac0)   /(pow(a2,2)+pow((c1-a1),2));
  
  dsn1d = ((c1-b1)*das1  -(-b2)*(-dac0))/(pow(b2,2)+pow((c1-b1),2));
  dcs1d = (  (-b2)*das1+(c1-b1)*(-dac0))/(pow(b2,2)+pow((c1-b1),2));
  //
  //
  //
  dzna = (pow((b1-a1),2)+pow((b2-a2),2))*(pow((b1-a1),2)+pow((a2+b2),2));
  
  sv123 = (
	   (dcs0s+dcs1s)*exp(-c2*t00)*(-1)+
	   (dcs0d+dcs1d)*exp(-c1*t00)+
	   ((dsn0s-dsn0d)*sin(a2*t00)+(dcs0s-dcs0d)*cos(a2*t00))*exp(-a1*t00)+
 	   ((dsn1s-dsn1d)*sin(b2*t00)+(dcs1s-dcs1d)*cos(b2*t00))*exp(-b1*t00)
	   )
    /dzna/(1/c2-1/c1);
  
  return sv123;
}
//
//
//
double
TrgEclFAM::u_max(double aaa, double bbb){
  
  if (aaa>bbb){ return aaa; }
  else        { return bbb; }
}
//
//

void
TrgEclFAM::FAMFit(int nbin_pedestal,
		  int NSampling,
		  int iTCIdm,
		  double fam_sampling_interval,
		  double digiEnergy[]){
  //============================
  // In this function,
  // o Energy unit must be [MeV/c2]
  // o Time unit must be [us]
  // but 
  // o return energy unit must be [GeV/c2]
  // o return time unit must be [ns]
  //============================
  double TCFitSample[14] = {0}; // MeV/c2
  double CoeffAAA=0;
  double CoeffBBB=0;
  double CoeffPPP=0;
  int dTBin = 0;
  int ShiftdTBin = 0;
  int Nsmalldt = 96;
  int SmallOffset = 1;
  double IntervaldT  = fam_sampling_interval*0.001/ Nsmalldt;
  double EThreshold = 50; //[GeV]
  int FitSleepCounter   = 100; // counter to suspend fit
  int FitSleepThreshold = 2;   // # of clk to suspend fit
  double FitE = 0;
  double FitT = 0;
  
  noutput[iTCIdm]= 0;
  
  for(int iShift=0; iShift<(NSampling-14); iShift++ ){
    
    FitSleepCounter++;
    if (FitSleepCounter<=FitSleepThreshold){continue;}
    for(int iFitSample=0; iFitSample<14; iFitSample++){ 
      int iReplace = iFitSample+iShift;
      TCFitSample[iFitSample] = digiEnergy[iReplace]*1000.0;
    }
    //
    //
    //
    
    dTBin = (int)(ShiftdTBin+Nsmalldt);
    if(dTBin<1)  {dTBin =   1;}
    if(dTBin>192){dTBin = 192;}
    
    CoeffAAA=0;
    CoeffBBB=0;
    CoeffPPP=0;
    for(int iFitSample=0; iFitSample<14; iFitSample++){
      CoeffAAA += CoeffNoise31[dTBin-1][iFitSample] * TCFitSample[iFitSample];
      CoeffBBB += CoeffNoise32[dTBin-1][iFitSample] * TCFitSample[iFitSample];
      CoeffPPP += CoeffNoise33[dTBin-1][iFitSample] * TCFitSample[iFitSample];
    }
    double deltaT = CoeffBBB/CoeffAAA;  // deltaT [us]
    
    ShiftdTBin = int(deltaT/IntervaldT+dTBin); 
    
    FitE = CoeffAAA;
    
    //-------
    // Require "expected time" is around middle of table = Nsmalldt.
    //-------
    double condition_t = -(deltaT + dTBin*IntervaldT - fam_sampling_interval*0.001);
    
    
    if( fabs(deltaT)<0.8*(fam_sampling_interval*0.001) && FitE>EThreshold ){
      FitT = condition_t + (SmallOffset+iShift+nbin_pedestal-1-5.5)*(fam_sampling_interval*0.001);
      //--------
      //	double chi  = 0.0;
      //	for(int iii=0; iii<14; iii++){
      //	  pp[iii] = noise_fl1[iii];
      //	  pf[iii] = p1r + CoeffAAA * f0[iii][i51] + b1r * f1[iii][i51];
      //	  if(iii<bin14) {
      //	    chi = chi + fabs(pp[iii]-pf[iii]) * fabs(pp[iii]-pf[iii]);
      //	  }
      //	}
      //--------
      //	break;
      returnE[noutput[iTCIdm]] = FitE/1000.0;  // [GeV/c2]
      returnT[noutput[iTCIdm]] = FitT*1000;
      noutput[iTCIdm]++;
      FitSleepCounter = 0;
      ShiftdTBin = 0;
      
    }
  }
  //
  //
  //
  return;
}

//
//
//
void 
TrgEclFAM::readFAMDB(void){
  
  double par_f0[192][14] = {
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.00368, 0.15726, 0.47637, 0.76169, 0.93179, 0.99603, 0.98951, 0.89182},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.00401, 0.16011, 0.47978, 0.76409, 0.93295, 0.99626, 0.98918, 0.88996},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.00436, 0.16298, 0.48318, 0.76648, 0.93410, 0.99649, 0.98883, 0.88808},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.00473, 0.16586, 0.48658, 0.76885, 0.93524, 0.99671, 0.98848, 0.88619},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.00512, 0.16877, 0.48997, 0.77121, 0.93636, 0.99692, 0.98812, 0.88428},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.00553, 0.17169, 0.49336, 0.77356, 0.93748, 0.99713, 0.98775, 0.88235},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.00596, 0.17463, 0.49674, 0.77589, 0.93858, 0.99732, 0.98738, 0.88041},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.00641, 0.17759, 0.50011, 0.77821, 0.93967, 0.99751, 0.98699, 0.87846},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.00689, 0.18057, 0.50348, 0.78052, 0.94075, 0.99770, 0.98660, 0.87649},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.00739, 0.18356, 0.50684, 0.78281, 0.94182, 0.99787, 0.98620, 0.87451},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.00791, 0.18657, 0.51019, 0.78509, 0.94287, 0.99804, 0.98579, 0.87251},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.00846, 0.18959, 0.51354, 0.78735, 0.94392, 0.99820, 0.98536, 0.87049},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.00903, 0.19264, 0.51688, 0.78961, 0.94495, 0.99835, 0.98493, 0.86847},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.00963, 0.19569, 0.52021, 0.79184, 0.94598, 0.99850, 0.98449, 0.86642},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.01025, 0.19877, 0.52354, 0.79407, 0.94699, 0.99864, 0.98404, 0.86437},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.01090, 0.20186, 0.52686, 0.79628, 0.94799, 0.99877, 0.98358, 0.86230},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.01157, 0.20496, 0.53017, 0.79848, 0.94898, 0.99890, 0.98310, 0.86022},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.01227, 0.20808, 0.53347, 0.80067, 0.94996, 0.99901, 0.98262, 0.85812},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.01300, 0.21121, 0.53677, 0.80284, 0.95092, 0.99913, 0.98212, 0.85601},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.01375, 0.21435, 0.54006, 0.80500, 0.95188, 0.99923, 0.98161, 0.85389},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.01453, 0.21751, 0.54334, 0.80714, 0.95283, 0.99933, 0.98108, 0.85175},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.01534, 0.22068, 0.54661, 0.80927, 0.95376, 0.99942, 0.98055, 0.84960},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.01618, 0.22387, 0.54987, 0.81139, 0.95468, 0.99950, 0.98000, 0.84744},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.01704, 0.22707, 0.55313, 0.81349, 0.95560, 0.99958, 0.97944, 0.84527},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.01793, 0.23028, 0.55637, 0.81558, 0.95650, 0.99965, 0.97886, 0.84308},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.01885, 0.23350, 0.55961, 0.81766, 0.95739, 0.99972, 0.97827, 0.84088},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.01980, 0.23674, 0.56284, 0.81973, 0.95827, 0.99977, 0.97767, 0.83867},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.02078, 0.23998, 0.56606, 0.82178, 0.95914, 0.99982, 0.97705, 0.83645},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.02179, 0.24324, 0.56927, 0.82381, 0.96000, 0.99987, 0.97642, 0.83422},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.02282, 0.24651, 0.57248, 0.82584, 0.96085, 0.99991, 0.97577, 0.83197},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.02389, 0.24979, 0.57567, 0.82785, 0.96169, 0.99994, 0.97511, 0.82972},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.02498, 0.25308, 0.57885, 0.82984, 0.96252, 0.99996, 0.97443, 0.82745},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.02611, 0.25638, 0.58203, 0.83183, 0.96334, 0.99998, 0.97374, 0.82517},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.02726, 0.25968, 0.58519, 0.83380, 0.96415, 0.99999, 0.97303, 0.82289},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.02845, 0.26300, 0.58834, 0.83575, 0.96494, 1.00000, 0.97230, 0.82059},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.02966, 0.26633, 0.59149, 0.83770, 0.96573, 1.00000, 0.97156, 0.81828},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.03090, 0.26967, 0.59463, 0.83963, 0.96651, 0.99999, 0.97080, 0.81596},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.03218, 0.27301, 0.59775, 0.84154, 0.96727, 0.99998, 0.97002, 0.81363},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.03348, 0.27637, 0.60087, 0.84345, 0.96803, 0.99996, 0.96923, 0.81129},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.03482, 0.27973, 0.60397, 0.84534, 0.96878, 0.99994, 0.96842, 0.80894},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.03618, 0.28310, 0.60706, 0.84722, 0.96951, 0.99991, 0.96759, 0.80659},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.03757, 0.28648, 0.61015, 0.84908, 0.97024, 0.99987, 0.96675, 0.80422},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.03900, 0.28986, 0.61322, 0.85093, 0.97096, 0.99983, 0.96589, 0.80184},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.04045, 0.29325, 0.61629, 0.85277, 0.97166, 0.99978, 0.96501, 0.79946},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.04194, 0.29665, 0.61934, 0.85459, 0.97236, 0.99972, 0.96411, 0.79707},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.04345, 0.30005, 0.62238, 0.85640, 0.97305, 0.99966, 0.96319, 0.79467},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.04500, 0.30347, 0.62541, 0.85820, 0.97372, 0.99960, 0.96226, 0.79226},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.04657, 0.30688, 0.62843, 0.85998, 0.97439, 0.99952, 0.96130, 0.78984},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00000,-0.00000, 0.04818, 0.31030, 0.63144, 0.86175, 0.97505, 0.99945, 0.96033, 0.78741},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.04981, 0.31373, 0.63444, 0.86351, 0.97569, 0.99936, 0.95934, 0.78498},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.05147, 0.31716, 0.63742, 0.86526, 0.97633, 0.99927, 0.95834, 0.78254},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.05317, 0.32060, 0.64040, 0.86699, 0.97696, 0.99918, 0.95731, 0.78009},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.05489, 0.32404, 0.64336, 0.86871, 0.97758, 0.99908, 0.95626, 0.77763},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.05664, 0.32749, 0.64632, 0.87041, 0.97819, 0.99897, 0.95520, 0.77517},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.05842, 0.33094, 0.64926, 0.87210, 0.97879, 0.99886, 0.95412, 0.77270},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.06024, 0.33439, 0.65219, 0.87378, 0.97938, 0.99874, 0.95302, 0.77022},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.06208, 0.33785, 0.65511, 0.87545, 0.97996, 0.99862, 0.95190, 0.76774},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.06395, 0.34131, 0.65801, 0.87710, 0.98053, 0.99849, 0.95076, 0.76525},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.06584, 0.34478, 0.66091, 0.87874, 0.98110, 0.99836, 0.94960, 0.76275},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.06777, 0.34824, 0.66379, 0.88037, 0.98165, 0.99822, 0.94842, 0.76025},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.00001, 0.06972, 0.35171, 0.66666, 0.88199, 0.98219, 0.99807, 0.94722, 0.75774},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.00001, 0.07171, 0.35518, 0.66952, 0.88359, 0.98273, 0.99792, 0.94601, 0.75523},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.00001, 0.07372, 0.35866, 0.67237, 0.88518, 0.98326, 0.99777, 0.94477, 0.75271},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.00002, 0.07576, 0.36213, 0.67520, 0.88675, 0.98377, 0.99761, 0.94352, 0.75019},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.00002, 0.07783, 0.36561, 0.67803, 0.88832, 0.98428, 0.99744, 0.94225, 0.74766},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.00003, 0.07992, 0.36909, 0.68084, 0.88987, 0.98478, 0.99727, 0.94096, 0.74512},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.00004, 0.08204, 0.37257, 0.68364, 0.89141, 0.98527, 0.99710, 0.93965, 0.74258},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.00005, 0.08419, 0.37605, 0.68642, 0.89293, 0.98575, 0.99692, 0.93832, 0.74004},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.00007, 0.08637, 0.37953, 0.68920, 0.89445, 0.98622, 0.99673, 0.93697, 0.73749},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.00009, 0.08857, 0.38301, 0.69196, 0.89595, 0.98669, 0.99654, 0.93560, 0.73493},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.00011, 0.09080, 0.38649, 0.69471, 0.89743, 0.98714, 0.99634, 0.93421, 0.73238},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.00013, 0.09305, 0.38997, 0.69744, 0.89891, 0.98759, 0.99614, 0.93281, 0.72982},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.00016, 0.09534, 0.39345, 0.70017, 0.90037, 0.98802, 0.99594, 0.93139, 0.72725},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.00020, 0.09764, 0.39693, 0.70288, 0.90182, 0.98845, 0.99573, 0.92994, 0.72468},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.00024, 0.09998, 0.40041, 0.70558, 0.90326, 0.98887, 0.99551, 0.92848, 0.72211},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.00029, 0.10234, 0.40389, 0.70827, 0.90468, 0.98928, 0.99529, 0.92700, 0.71953},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.00034, 0.10472, 0.40737, 0.71094, 0.90610, 0.98969, 0.99507, 0.92550, 0.71695},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.00040, 0.10713, 0.41085, 0.71360, 0.90750, 0.99008, 0.99484, 0.92399, 0.71437},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.00047, 0.10956, 0.41432, 0.71625, 0.90888, 0.99047, 0.99460, 0.92245, 0.71178},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.00054, 0.11202, 0.41779, 0.71888, 0.91026, 0.99085, 0.99436, 0.92090, 0.70919},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.00063, 0.11450, 0.42126, 0.72151, 0.91162, 0.99122, 0.99412, 0.91933, 0.70660},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.00072, 0.11700, 0.42473, 0.72412, 0.91297, 0.99158, 0.99387, 0.91774, 0.70401},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.00082, 0.11953, 0.42820, 0.72671, 0.91431, 0.99193, 0.99361, 0.91613, 0.70141},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.00094, 0.12209, 0.43166, 0.72930, 0.91564, 0.99227, 0.99336, 0.91451, 0.69882},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.00107, 0.12466, 0.43512, 0.73187, 0.91695, 0.99261, 0.99309, 0.91287, 0.69621},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.00120, 0.12726, 0.43858, 0.73443, 0.91826, 0.99294, 0.99282, 0.91121, 0.69361},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.00135, 0.12988, 0.44204, 0.73697, 0.91955, 0.99326, 0.99255, 0.90953, 0.69101},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.00152, 0.13252, 0.44549, 0.73950, 0.92082, 0.99357, 0.99227, 0.90783, 0.68840},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.00170, 0.13519, 0.44894, 0.74202, 0.92209, 0.99388, 0.99199, 0.90612, 0.68579},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.00189, 0.13787, 0.45238, 0.74453, 0.92334, 0.99417, 0.99170, 0.90439, 0.68319},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.00209, 0.14058, 0.45582, 0.74702, 0.92459, 0.99446, 0.99141, 0.90265, 0.68058},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.00232, 0.14331, 0.45926, 0.74950, 0.92582, 0.99474, 0.99111, 0.90088, 0.67796},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.00255, 0.14606, 0.46269, 0.75196, 0.92704, 0.99502, 0.99080, 0.89910, 0.67535},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.00281, 0.14883, 0.46612, 0.75442, 0.92824, 0.99528, 0.99049, 0.89731, 0.67274},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.00308, 0.15162, 0.46954, 0.75686, 0.92944, 0.99554, 0.99017, 0.89549, 0.67013},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.00337, 0.15443, 0.47296, 0.75928, 0.93062, 0.99579, 0.98985, 0.89366, 0.66751},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.00368, 0.15726, 0.47637, 0.76169, 0.93179, 0.99603, 0.98951, 0.89182, 0.66490},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.00401, 0.16011, 0.47978, 0.76409, 0.93295, 0.99626, 0.98918, 0.88996, 0.66228},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.00436, 0.16298, 0.48318, 0.76648, 0.93410, 0.99649, 0.98883, 0.88808, 0.65967},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.00473, 0.16586, 0.48658, 0.76885, 0.93524, 0.99671, 0.98848, 0.88619, 0.65705},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.00512, 0.16877, 0.48997, 0.77121, 0.93636, 0.99692, 0.98812, 0.88428, 0.65443},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.00553, 0.17169, 0.49336, 0.77356, 0.93748, 0.99713, 0.98775, 0.88235, 0.65182},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.00596, 0.17463, 0.49674, 0.77589, 0.93858, 0.99732, 0.98738, 0.88041, 0.64921},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.00641, 0.17759, 0.50011, 0.77821, 0.93967, 0.99751, 0.98699, 0.87846, 0.64659},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.00689, 0.18057, 0.50348, 0.78052, 0.94075, 0.99770, 0.98660, 0.87649, 0.64398},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.00739, 0.18356, 0.50684, 0.78281, 0.94182, 0.99787, 0.98620, 0.87451, 0.64136},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.00791, 0.18657, 0.51019, 0.78509, 0.94287, 0.99804, 0.98579, 0.87251, 0.63875},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.00846, 0.18959, 0.51354, 0.78735, 0.94392, 0.99820, 0.98536, 0.87049, 0.63614},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.00903, 0.19264, 0.51688, 0.78961, 0.94495, 0.99835, 0.98493, 0.86847, 0.63353},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.00963, 0.19569, 0.52021, 0.79184, 0.94598, 0.99850, 0.98449, 0.86642, 0.63092},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.01025, 0.19877, 0.52354, 0.79407, 0.94699, 0.99864, 0.98404, 0.86437, 0.62831},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.01090, 0.20186, 0.52686, 0.79628, 0.94799, 0.99877, 0.98358, 0.86230, 0.62571},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.01157, 0.20496, 0.53017, 0.79848, 0.94898, 0.99890, 0.98310, 0.86022, 0.62310},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.01227, 0.20808, 0.53347, 0.80067, 0.94996, 0.99901, 0.98262, 0.85812, 0.62050},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.01300, 0.21121, 0.53677, 0.80284, 0.95092, 0.99913, 0.98212, 0.85601, 0.61790},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.01375, 0.21435, 0.54006, 0.80500, 0.95188, 0.99923, 0.98161, 0.85389, 0.61530},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.01453, 0.21751, 0.54334, 0.80714, 0.95283, 0.99933, 0.98108, 0.85175, 0.61270},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.01534, 0.22068, 0.54661, 0.80927, 0.95376, 0.99942, 0.98055, 0.84960, 0.61010},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.01618, 0.22387, 0.54987, 0.81139, 0.95468, 0.99950, 0.98000, 0.84744, 0.60751},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.01704, 0.22707, 0.55313, 0.81349, 0.95560, 0.99958, 0.97944, 0.84527, 0.60492},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.01793, 0.23028, 0.55637, 0.81558, 0.95650, 0.99965, 0.97886, 0.84308, 0.60233},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.01885, 0.23350, 0.55961, 0.81766, 0.95739, 0.99972, 0.97827, 0.84088, 0.59974},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.01980, 0.23674, 0.56284, 0.81973, 0.95827, 0.99977, 0.97767, 0.83867, 0.59716},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.02078, 0.23998, 0.56606, 0.82178, 0.95914, 0.99982, 0.97705, 0.83645, 0.59458},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.02179, 0.24324, 0.56927, 0.82381, 0.96000, 0.99987, 0.97642, 0.83422, 0.59200},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.02282, 0.24651, 0.57248, 0.82584, 0.96085, 0.99991, 0.97577, 0.83197, 0.58942},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.02389, 0.24979, 0.57567, 0.82785, 0.96169, 0.99994, 0.97511, 0.82972, 0.58685},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.02498, 0.25308, 0.57885, 0.82984, 0.96252, 0.99996, 0.97443, 0.82745, 0.58428},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.02611, 0.25638, 0.58203, 0.83183, 0.96334, 0.99998, 0.97374, 0.82517, 0.58172},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.02726, 0.25968, 0.58519, 0.83380, 0.96415, 0.99999, 0.97303, 0.82289, 0.57915},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.02845, 0.26300, 0.58834, 0.83575, 0.96494, 1.00000, 0.97230, 0.82059, 0.57659},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.02966, 0.26633, 0.59149, 0.83770, 0.96573, 1.00000, 0.97156, 0.81828, 0.57404},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.03090, 0.26967, 0.59463, 0.83963, 0.96651, 0.99999, 0.97080, 0.81596, 0.57148},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.03218, 0.27301, 0.59775, 0.84154, 0.96727, 0.99998, 0.97002, 0.81363, 0.56893},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.03348, 0.27637, 0.60087, 0.84345, 0.96803, 0.99996, 0.96923, 0.81129, 0.56639},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.03482, 0.27973, 0.60397, 0.84534, 0.96878, 0.99994, 0.96842, 0.80894, 0.56384},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.03618, 0.28310, 0.60706, 0.84722, 0.96951, 0.99991, 0.96759, 0.80659, 0.56131},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.03757, 0.28648, 0.61015, 0.84908, 0.97024, 0.99987, 0.96675, 0.80422, 0.55877},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.03900, 0.28986, 0.61322, 0.85093, 0.97096, 0.99983, 0.96589, 0.80184, 0.55624},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.04045, 0.29325, 0.61629, 0.85277, 0.97166, 0.99978, 0.96501, 0.79946, 0.55372},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.04194, 0.29665, 0.61934, 0.85459, 0.97236, 0.99972, 0.96411, 0.79707, 0.55119},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.04345, 0.30005, 0.62238, 0.85640, 0.97305, 0.99966, 0.96319, 0.79467, 0.54868},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.04500, 0.30347, 0.62541, 0.85820, 0.97372, 0.99960, 0.96226, 0.79226, 0.54616},
{ 0.00000, 0.00000, 0.00000, 0.00000,-0.00000, 0.04657, 0.30688, 0.62843, 0.85998, 0.97439, 0.99952, 0.96130, 0.78984, 0.54365},
{ 0.00000, 0.00000, 0.00000, 0.00000,-0.00000, 0.04818, 0.31030, 0.63144, 0.86175, 0.97505, 0.99945, 0.96033, 0.78741, 0.54115},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.04981, 0.31373, 0.63444, 0.86351, 0.97569, 0.99936, 0.95934, 0.78498, 0.53865},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.05147, 0.31716, 0.63742, 0.86526, 0.97633, 0.99927, 0.95834, 0.78254, 0.53615},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.05317, 0.32060, 0.64040, 0.86699, 0.97696, 0.99918, 0.95731, 0.78009, 0.53366},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.05489, 0.32404, 0.64336, 0.86871, 0.97758, 0.99908, 0.95626, 0.77763, 0.53118},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.05664, 0.32749, 0.64632, 0.87041, 0.97819, 0.99897, 0.95520, 0.77517, 0.52870},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.05842, 0.33094, 0.64926, 0.87210, 0.97879, 0.99886, 0.95412, 0.77270, 0.52622},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.06024, 0.33439, 0.65219, 0.87378, 0.97938, 0.99874, 0.95302, 0.77022, 0.52375},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.06208, 0.33785, 0.65511, 0.87545, 0.97996, 0.99862, 0.95190, 0.76774, 0.52128},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.06395, 0.34131, 0.65801, 0.87710, 0.98053, 0.99849, 0.95076, 0.76525, 0.51882},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.06584, 0.34478, 0.66091, 0.87874, 0.98110, 0.99836, 0.94960, 0.76275, 0.51637},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.06777, 0.34824, 0.66379, 0.88037, 0.98165, 0.99822, 0.94842, 0.76025, 0.51392},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00001, 0.06972, 0.35171, 0.66666, 0.88199, 0.98219, 0.99807, 0.94722, 0.75774, 0.51147},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00001, 0.07171, 0.35518, 0.66952, 0.88359, 0.98273, 0.99792, 0.94601, 0.75523, 0.50903},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00001, 0.07372, 0.35866, 0.67237, 0.88518, 0.98326, 0.99777, 0.94477, 0.75271, 0.50660},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00002, 0.07576, 0.36213, 0.67520, 0.88675, 0.98377, 0.99761, 0.94352, 0.75019, 0.50417},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00002, 0.07783, 0.36561, 0.67803, 0.88832, 0.98428, 0.99744, 0.94225, 0.74766, 0.50174},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00003, 0.07992, 0.36909, 0.68084, 0.88987, 0.98478, 0.99727, 0.94096, 0.74512, 0.49933},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00004, 0.08204, 0.37257, 0.68364, 0.89141, 0.98527, 0.99710, 0.93965, 0.74258, 0.49691},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00005, 0.08419, 0.37605, 0.68642, 0.89293, 0.98575, 0.99692, 0.93832, 0.74004, 0.49451},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00007, 0.08637, 0.37953, 0.68920, 0.89445, 0.98622, 0.99673, 0.93697, 0.73749, 0.49211},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00009, 0.08857, 0.38301, 0.69196, 0.89595, 0.98669, 0.99654, 0.93560, 0.73493, 0.48971},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00011, 0.09080, 0.38649, 0.69471, 0.89743, 0.98714, 0.99634, 0.93421, 0.73238, 0.48732},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00013, 0.09305, 0.38997, 0.69744, 0.89891, 0.98759, 0.99614, 0.93281, 0.72982, 0.48494},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00016, 0.09534, 0.39345, 0.70017, 0.90037, 0.98802, 0.99594, 0.93139, 0.72725, 0.48256},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00020, 0.09764, 0.39693, 0.70288, 0.90182, 0.98845, 0.99573, 0.92994, 0.72468, 0.48019},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00024, 0.09998, 0.40041, 0.70558, 0.90326, 0.98887, 0.99551, 0.92848, 0.72211, 0.47783},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00029, 0.10234, 0.40389, 0.70827, 0.90468, 0.98928, 0.99529, 0.92700, 0.71953, 0.47547},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00034, 0.10472, 0.40737, 0.71094, 0.90610, 0.98969, 0.99507, 0.92550, 0.71695, 0.47311},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00040, 0.10713, 0.41085, 0.71360, 0.90750, 0.99008, 0.99484, 0.92399, 0.71437, 0.47077},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00047, 0.10956, 0.41432, 0.71625, 0.90888, 0.99047, 0.99460, 0.92245, 0.71178, 0.46843},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00054, 0.11202, 0.41779, 0.71888, 0.91026, 0.99085, 0.99436, 0.92090, 0.70919, 0.46609},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00063, 0.11450, 0.42126, 0.72151, 0.91162, 0.99122, 0.99412, 0.91933, 0.70660, 0.46377},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00072, 0.11700, 0.42473, 0.72412, 0.91297, 0.99158, 0.99387, 0.91774, 0.70401, 0.46145},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00082, 0.11953, 0.42820, 0.72671, 0.91431, 0.99193, 0.99361, 0.91613, 0.70141, 0.45913},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00094, 0.12209, 0.43166, 0.72930, 0.91564, 0.99227, 0.99336, 0.91451, 0.69882, 0.45682},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00107, 0.12466, 0.43512, 0.73187, 0.91695, 0.99261, 0.99309, 0.91287, 0.69621, 0.45452},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00120, 0.12726, 0.43858, 0.73443, 0.91826, 0.99294, 0.99282, 0.91121, 0.69361, 0.45223},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00135, 0.12988, 0.44204, 0.73697, 0.91955, 0.99326, 0.99255, 0.90953, 0.69101, 0.44994},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00152, 0.13252, 0.44549, 0.73950, 0.92082, 0.99357, 0.99227, 0.90783, 0.68840, 0.44765},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00170, 0.13519, 0.44894, 0.74202, 0.92209, 0.99388, 0.99199, 0.90612, 0.68579, 0.44538},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00189, 0.13787, 0.45238, 0.74453, 0.92334, 0.99417, 0.99170, 0.90439, 0.68319, 0.44311},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00209, 0.14058, 0.45582, 0.74702, 0.92459, 0.99446, 0.99141, 0.90265, 0.68058, 0.44085},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00232, 0.14331, 0.45926, 0.74950, 0.92582, 0.99474, 0.99111, 0.90088, 0.67796, 0.43859},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00255, 0.14606, 0.46269, 0.75196, 0.92704, 0.99502, 0.99080, 0.89910, 0.67535, 0.43635},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00281, 0.14883, 0.46612, 0.75442, 0.92824, 0.99528, 0.99049, 0.89731, 0.67274, 0.43410},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00308, 0.15162, 0.46954, 0.75686, 0.92944, 0.99554, 0.99017, 0.89549, 0.67013, 0.43187},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00337, 0.15443, 0.47296, 0.75928, 0.93062, 0.99579, 0.98985, 0.89366, 0.66751, 0.42964}
  };
  //
  double par_f1[192][14] = {
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.31873, 2.83937, 3.41051, 2.40616, 1.16573, 0.23825,-0.33434,-1.85349},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.33811, 2.85854, 3.40526, 2.39284, 1.15408, 0.23066,-0.34130,-1.86945},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.35803, 2.87742, 3.39984, 2.37952, 1.14247, 0.22311,-0.34853,-1.88526},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.37849, 2.89599, 3.39426, 2.36617, 1.13089, 0.21561,-0.35601,-1.90091},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.39946, 2.91427, 3.38851, 2.35281, 1.11936, 0.20815,-0.36377,-1.91642},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.42094, 2.93225, 3.38261, 2.33944, 1.10787, 0.20072,-0.37181,-1.93177},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.44292, 2.94993, 3.37654, 2.32606, 1.09641, 0.19334,-0.38013,-1.94696},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.46537, 2.96731, 3.37032, 2.31267, 1.08499, 0.18600,-0.38875,-1.96200},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.48829, 2.98439, 3.36394, 2.29926, 1.07362, 0.17870,-0.39767,-1.97687},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.51166, 3.00117, 3.35740, 2.28585, 1.06228, 0.17145,-0.40690,-1.99158},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.53547, 3.01765, 3.35072, 2.27243, 1.05098, 0.16423,-0.41643,-2.00613},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.55970, 3.03382, 3.34388, 2.25900, 1.03973, 0.15705,-0.42627,-2.02052},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.58433, 3.04970, 3.33690, 2.24556, 1.02851, 0.14992,-0.43642,-2.03474},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.60936, 3.06527, 3.32978, 2.23212, 1.01733, 0.14282,-0.44688,-2.04879},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.63477, 3.08055, 3.32251, 2.21868, 1.00620, 0.13577,-0.45766,-2.06268},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.66054, 3.09552, 3.31509, 2.20523, 0.99510, 0.12875,-0.46875,-2.07639},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.68666, 3.11019, 3.30754, 2.19177, 0.98405, 0.12178,-0.48016,-2.08993},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.71310, 3.12455, 3.29985, 2.17832, 0.97303, 0.11484,-0.49187,-2.10331},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.73987, 3.13862, 3.29203, 2.16486, 0.96206, 0.10795,-0.50390,-2.11651},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.76693, 3.15239, 3.28407, 2.15141, 0.95113, 0.10110,-0.51623,-2.12953},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.79428, 3.16586, 3.27598, 2.13795, 0.94024, 0.09428,-0.52886,-2.14238},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.82191, 3.17903, 3.26776, 2.12450, 0.92940, 0.08751,-0.54178,-2.15506},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.84978, 3.19190, 3.25941, 2.11105, 0.91859, 0.08077,-0.55501,-2.16756},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.87790, 3.20447, 3.25094, 2.09760, 0.90783, 0.07407,-0.56852,-2.17989},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.90625, 3.21675, 3.24234, 2.08415, 0.89710, 0.06742,-0.58231,-2.19203},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.93481, 3.22873, 3.23362, 2.07071, 0.88642, 0.06080,-0.59638,-2.20400},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.96356, 3.24041, 3.22478, 2.05728, 0.87579, 0.05422,-0.61072,-2.21580},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.99250, 3.25180, 3.21582, 2.04385, 0.86519, 0.04768,-0.62533,-2.22741},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 1.02160, 3.26289, 3.20674, 2.03042, 0.85464, 0.04118,-0.64019,-2.23885},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 1.05086, 3.27369, 3.19755, 2.01701, 0.84413, 0.03472,-0.65531,-2.25010},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 1.08027, 3.28420, 3.18825, 2.00360, 0.83366, 0.02830,-0.67067,-2.26118},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 1.10980, 3.29442, 3.17883, 1.99020, 0.82323, 0.02191,-0.68627,-2.27208},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 1.13944, 3.30435, 3.16931, 1.97681, 0.81285, 0.01557,-0.70210,-2.28280},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 1.16919, 3.31399, 3.15968, 1.96344, 0.80251, 0.00926,-0.71815,-2.29334},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 1.19902, 3.32334, 3.14994, 1.95007, 0.79221, 0.00299,-0.73441,-2.30370},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 1.22893, 3.33241, 3.14010, 1.93671, 0.78196,-0.00324,-0.75088,-2.31388},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 1.25890, 3.34119, 3.13015, 1.92337, 0.77175,-0.00944,-0.76755,-2.32388},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 1.28893, 3.34968, 3.12011, 1.91004, 0.76158,-0.01559,-0.78441,-2.33370},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 1.31899, 3.35790, 3.10996, 1.89672, 0.75145,-0.02171,-0.80145,-2.34334},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 1.34908, 3.36583, 3.09972, 1.88342, 0.74137,-0.02779,-0.81867,-2.35280},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 1.37918, 3.37349, 3.08939, 1.87014, 0.73133,-0.03384,-0.83605,-2.36208},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 1.40929, 3.38086, 3.07896, 1.85686, 0.72133,-0.03984,-0.85358,-2.37119},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 1.43940, 3.38796, 3.06844, 1.84361, 0.71138,-0.04581,-0.87127,-2.38011},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 1.46949, 3.39479, 3.05782, 1.83037, 0.70147,-0.05174,-0.88909,-2.38886},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 1.49955, 3.40134, 3.04712, 1.81715, 0.69161,-0.05764,-0.90705,-2.39743},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 1.52957, 3.40762, 3.03634, 1.80395, 0.68178,-0.06349,-0.92514,-2.40582},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 1.55954, 3.41363, 3.02546, 1.79076, 0.67200,-0.06932,-0.94334,-2.41404},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 1.58946, 3.41937, 3.01451, 1.77760, 0.66227,-0.07510,-0.96164,-2.42207},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 1.61930, 3.42484, 3.00347, 1.76445, 0.65257,-0.08085,-0.98005,-2.42994},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 1.64908, 3.43005, 2.99235, 1.75133, 0.64293,-0.08656,-0.99855,-2.43762},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 1.67876, 3.43500, 2.98116, 1.73822, 0.63332,-0.09224,-1.01714,-2.44513},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.00001, 1.70835, 3.43968, 2.96988, 1.72514, 0.62376,-0.09788,-1.03580,-2.45247},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.00003, 1.73784, 3.44410, 2.95853, 1.71208, 0.61424,-0.10348,-1.05453,-2.45963},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.00007, 1.76722, 3.44827, 2.94711, 1.69904, 0.60476,-0.10905,-1.07333,-2.46662},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.00014, 1.79647, 3.45217, 2.93561, 1.68602, 0.59533,-0.11458,-1.09218,-2.47343},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.00026, 1.82560, 3.45583, 2.92405, 1.67303, 0.58595,-0.12008,-1.11107,-2.48007},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.00044, 1.85460, 3.45923, 2.91241, 1.66006, 0.57660,-0.12554,-1.13001,-2.48655},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.00071, 1.88345, 3.46237, 2.90071, 1.64712, 0.56730,-0.13097,-1.14898,-2.49285},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.00108, 1.91215, 3.46527, 2.88894, 1.63420, 0.55804,-0.13636,-1.16797,-2.49898},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.00159, 1.94069, 3.46793, 2.87710, 1.62130, 0.54883,-0.14171,-1.18699,-2.50494},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.00225, 1.96907, 3.47033, 2.86520, 1.60844, 0.53966,-0.14704,-1.20601,-2.51073},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.00310, 1.99728, 3.47250, 2.85324, 1.59560, 0.53053,-0.15232,-1.22504,-2.51635},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.00415, 2.02532, 3.47442, 2.84122, 1.58278, 0.52145,-0.15757,-1.24408,-2.52181},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.00545, 2.05316, 3.47610, 2.82914, 1.56999, 0.51241,-0.16279,-1.26310,-2.52710},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.00701, 2.08082, 3.47755, 2.81700, 1.55723, 0.50341,-0.16797,-1.28211,-2.53223},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.00887, 2.10829, 3.47876, 2.80481, 1.54450, 0.49446,-0.17312,-1.30111,-2.53719},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.01106, 2.13555, 3.47973, 2.79256, 1.53180, 0.48555,-0.17824,-1.32007,-2.54199},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.01360, 2.16261, 3.48048, 2.78026, 1.51912, 0.47669,-0.18332,-1.33901,-2.54663},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.01652, 2.18946, 3.48099, 2.76791, 1.50648, 0.46786,-0.18837,-1.35791,-2.55110},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.01985, 2.21609, 3.48128, 2.75550, 1.49386, 0.45908,-0.19338,-1.37677,-2.55542},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.02360, 2.24250, 3.48134, 2.74305, 1.48128, 0.45035,-0.19836,-1.39558,-2.55957},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.02781, 2.26868, 3.48118, 2.73055, 1.46872, 0.44166,-0.20331,-1.41434,-2.56357},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.03250, 2.29463, 3.48080, 2.71800, 1.45620, 0.43301,-0.20822,-1.43305,-2.56740},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.03769, 2.32035, 3.48019, 2.70541, 1.44370, 0.42440,-0.21310,-1.45168,-2.57109},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.04339, 2.34583, 3.47937, 2.69277, 1.43124, 0.41584,-0.21795,-1.47026,-2.57461},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.04962, 2.37107, 3.47834, 2.68009, 1.41881, 0.40732,-0.22277,-1.48876,-2.57798},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.05641, 2.39606, 3.47709, 2.66737, 1.40641, 0.39884,-0.22756,-1.50718,-2.58120},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.06377, 2.42079, 3.47563, 2.65461, 1.39405, 0.39041,-0.23233,-1.52552,-2.58426},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.07170, 2.44528, 3.47396, 2.64181, 1.38172, 0.38201,-0.23709,-1.54378,-2.58718},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.08022, 2.46951, 3.47208, 2.62898, 1.36942, 0.37367,-0.24184,-1.56194,-2.58994},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.08934, 2.49347, 3.47000, 2.61610, 1.35715, 0.36536,-0.24659,-1.58001,-2.59255},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.09907, 2.51717, 3.46771, 2.60319, 1.34492, 0.35710,-0.25136,-1.59799,-2.59501},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.10940, 2.54061, 3.46523, 2.59025, 1.33273, 0.34888,-0.25615,-1.61586,-2.59733},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.12036, 2.56378, 3.46254, 2.57728, 1.32057, 0.34070,-0.26099,-1.63363,-2.59950},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.13194, 2.58667, 3.45966, 2.56427, 1.30844, 0.33257,-0.26587,-1.65129,-2.60153},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.14414, 2.60929, 3.45659, 2.55123, 1.29635, 0.32447,-0.27083,-1.66883,-2.60341},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.15697, 2.63163, 3.45332, 2.53817, 1.28429, 0.31642,-0.27587,-1.68626,-2.60515},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.17041, 2.65369, 3.44986, 2.52507, 1.27227, 0.30842,-0.28101,-1.70357,-2.60675},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.18448, 2.67547, 3.44621, 2.51195, 1.26028, 0.30045,-0.28627,-1.72076,-2.60821},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.19917, 2.69697, 3.44238, 2.49881, 1.24833, 0.29253,-0.29166,-1.73782,-2.60953},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.21446, 2.71818, 3.43836, 2.48564, 1.23642, 0.28465,-0.29720,-1.75475,-2.61071},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.23037, 2.73911, 3.43416, 2.47244, 1.22455, 0.27681,-0.30290,-1.77155,-2.61175},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.24687, 2.75974, 3.42978, 2.45923, 1.21271, 0.26901,-0.30877,-1.78822,-2.61266},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.26397, 2.78009, 3.42523, 2.44599, 1.20091, 0.26126,-0.31484,-1.80475,-2.61343},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.28165, 2.80014, 3.42049, 2.43273, 1.18914, 0.25355,-0.32112,-1.82114,-2.61407},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.29991, 2.81990, 3.41559, 2.41945, 1.17742, 0.24588,-0.32761,-1.83739,-2.61458},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.31873, 2.83937, 3.41051, 2.40616, 1.16573, 0.23825,-0.33434,-1.85349,-2.61495},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.33811, 2.85854, 3.40526, 2.39284, 1.15408, 0.23066,-0.34130,-1.86945,-2.61520},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.35803, 2.87742, 3.39984, 2.37952, 1.14247, 0.22311,-0.34853,-1.88526,-2.61532},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.37849, 2.89599, 3.39426, 2.36617, 1.13089, 0.21561,-0.35601,-1.90091,-2.61531},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.39946, 2.91427, 3.38851, 2.35281, 1.11936, 0.20815,-0.36377,-1.91642,-2.61517},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.42094, 2.93225, 3.38261, 2.33944, 1.10787, 0.20072,-0.37181,-1.93177,-2.61491},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.44292, 2.94993, 3.37654, 2.32606, 1.09641, 0.19334,-0.38013,-1.94696,-2.61452},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.46537, 2.96731, 3.37032, 2.31267, 1.08499, 0.18600,-0.38875,-1.96200,-2.61402},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.48829, 2.98439, 3.36394, 2.29926, 1.07362, 0.17870,-0.39767,-1.97687,-2.61339},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.51166, 3.00117, 3.35740, 2.28585, 1.06228, 0.17145,-0.40690,-1.99158,-2.61264},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.53547, 3.01765, 3.35072, 2.27243, 1.05098, 0.16423,-0.41643,-2.00613,-2.61177},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.55970, 3.03382, 3.34388, 2.25900, 1.03973, 0.15705,-0.42627,-2.02052,-2.61078},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.58433, 3.04970, 3.33690, 2.24556, 1.02851, 0.14992,-0.43642,-2.03474,-2.60968},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.60936, 3.06527, 3.32978, 2.23212, 1.01733, 0.14282,-0.44688,-2.04879,-2.60846},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.63477, 3.08055, 3.32251, 2.21868, 1.00620, 0.13577,-0.45766,-2.06268,-2.60713},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.66054, 3.09552, 3.31509, 2.20523, 0.99510, 0.12875,-0.46875,-2.07639,-2.60568},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.68666, 3.11019, 3.30754, 2.19177, 0.98405, 0.12178,-0.48016,-2.08993,-2.60412},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.71310, 3.12455, 3.29985, 2.17832, 0.97303, 0.11484,-0.49187,-2.10331,-2.60245},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.73987, 3.13862, 3.29203, 2.16486, 0.96206, 0.10795,-0.50390,-2.11651,-2.60067},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.76693, 3.15239, 3.28407, 2.15141, 0.95113, 0.10110,-0.51623,-2.12953,-2.59879},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.79428, 3.16586, 3.27598, 2.13795, 0.94024, 0.09428,-0.52886,-2.14238,-2.59679},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.82191, 3.17903, 3.26776, 2.12450, 0.92940, 0.08751,-0.54178,-2.15506,-2.59469},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.84978, 3.19190, 3.25941, 2.11105, 0.91859, 0.08077,-0.55501,-2.16756,-2.59249},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.87790, 3.20447, 3.25094, 2.09760, 0.90783, 0.07407,-0.56852,-2.17989,-2.59018},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.90625, 3.21675, 3.24234, 2.08415, 0.89710, 0.06742,-0.58231,-2.19203,-2.58777},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.93481, 3.22873, 3.23362, 2.07071, 0.88642, 0.06080,-0.59638,-2.20400,-2.58526},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.96356, 3.24041, 3.22478, 2.05728, 0.87579, 0.05422,-0.61072,-2.21580,-2.58265},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.99250, 3.25180, 3.21582, 2.04385, 0.86519, 0.04768,-0.62533,-2.22741,-2.57994},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 1.02160, 3.26289, 3.20674, 2.03042, 0.85464, 0.04118,-0.64019,-2.23885,-2.57713},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 1.05086, 3.27369, 3.19755, 2.01701, 0.84413, 0.03472,-0.65531,-2.25010,-2.57423},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 1.08027, 3.28420, 3.18825, 2.00360, 0.83366, 0.02830,-0.67067,-2.26118,-2.57123},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 1.10980, 3.29442, 3.17883, 1.99020, 0.82323, 0.02191,-0.68627,-2.27208,-2.56813},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 1.13944, 3.30435, 3.16931, 1.97681, 0.81285, 0.01557,-0.70210,-2.28280,-2.56495},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 1.16919, 3.31399, 3.15968, 1.96344, 0.80251, 0.00926,-0.71815,-2.29334,-2.56167},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 1.19902, 3.32334, 3.14994, 1.95007, 0.79221, 0.00299,-0.73441,-2.30370,-2.55830},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 1.22893, 3.33241, 3.14010, 1.93671, 0.78196,-0.00324,-0.75088,-2.31388,-2.55484},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 1.25890, 3.34119, 3.13015, 1.92337, 0.77175,-0.00944,-0.76755,-2.32388,-2.55130},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 1.28893, 3.34968, 3.12011, 1.91004, 0.76158,-0.01559,-0.78441,-2.33370,-2.54766},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 1.31899, 3.35790, 3.10996, 1.89672, 0.75145,-0.02171,-0.80145,-2.34334,-2.54394},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 1.34908, 3.36583, 3.09972, 1.88342, 0.74137,-0.02779,-0.81867,-2.35280,-2.54014},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 1.37918, 3.37349, 3.08939, 1.87014, 0.73133,-0.03384,-0.83605,-2.36208,-2.53625},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 1.40929, 3.38086, 3.07896, 1.85686, 0.72133,-0.03984,-0.85358,-2.37119,-2.53228},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 1.43940, 3.38796, 3.06844, 1.84361, 0.71138,-0.04581,-0.87127,-2.38011,-2.52823},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 1.46949, 3.39479, 3.05782, 1.83037, 0.70147,-0.05174,-0.88909,-2.38886,-2.52410},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 1.49955, 3.40134, 3.04712, 1.81715, 0.69161,-0.05764,-0.90705,-2.39743,-2.51989},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 1.52957, 3.40762, 3.03634, 1.80395, 0.68178,-0.06349,-0.92514,-2.40582,-2.51560},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 1.55954, 3.41363, 3.02546, 1.79076, 0.67200,-0.06932,-0.94334,-2.41404,-2.51123},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 1.58946, 3.41937, 3.01451, 1.77760, 0.66227,-0.07510,-0.96164,-2.42207,-2.50679},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 1.61930, 3.42484, 3.00347, 1.76445, 0.65257,-0.08085,-0.98005,-2.42994,-2.50227},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 1.64908, 3.43005, 2.99235, 1.75133, 0.64293,-0.08656,-0.99855,-2.43762,-2.49768},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 1.67876, 3.43500, 2.98116, 1.73822, 0.63332,-0.09224,-1.01714,-2.44513,-2.49301},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00001, 1.70835, 3.43968, 2.96988, 1.72514, 0.62376,-0.09788,-1.03580,-2.45247,-2.48828},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00003, 1.73784, 3.44410, 2.95853, 1.71208, 0.61424,-0.10348,-1.05453,-2.45963,-2.48347},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00007, 1.76722, 3.44827, 2.94711, 1.69904, 0.60476,-0.10905,-1.07333,-2.46662,-2.47859},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00014, 1.79647, 3.45217, 2.93561, 1.68602, 0.59533,-0.11458,-1.09218,-2.47343,-2.47365},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00026, 1.82560, 3.45583, 2.92405, 1.67303, 0.58595,-0.12008,-1.11107,-2.48007,-2.46864},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00044, 1.85460, 3.45923, 2.91241, 1.66006, 0.57660,-0.12554,-1.13001,-2.48655,-2.46356},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00071, 1.88345, 3.46237, 2.90071, 1.64712, 0.56730,-0.13097,-1.14898,-2.49285,-2.45841},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00108, 1.91215, 3.46527, 2.88894, 1.63420, 0.55804,-0.13636,-1.16797,-2.49898,-2.45320},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00159, 1.94069, 3.46793, 2.87710, 1.62130, 0.54883,-0.14171,-1.18699,-2.50494,-2.44793},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00225, 1.96907, 3.47033, 2.86520, 1.60844, 0.53966,-0.14704,-1.20601,-2.51073,-2.44260},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00310, 1.99728, 3.47250, 2.85324, 1.59560, 0.53053,-0.15232,-1.22504,-2.51635,-2.43720},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00415, 2.02532, 3.47442, 2.84122, 1.58278, 0.52145,-0.15757,-1.24408,-2.52181,-2.43175},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00545, 2.05316, 3.47610, 2.82914, 1.56999, 0.51241,-0.16279,-1.26310,-2.52710,-2.42623},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00701, 2.08082, 3.47755, 2.81700, 1.55723, 0.50341,-0.16797,-1.28211,-2.53223,-2.42066},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.00887, 2.10829, 3.47876, 2.80481, 1.54450, 0.49446,-0.17312,-1.30111,-2.53719,-2.41503},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.01106, 2.13555, 3.47973, 2.79256, 1.53180, 0.48555,-0.17824,-1.32007,-2.54199,-2.40934},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.01360, 2.16261, 3.48048, 2.78026, 1.51912, 0.47669,-0.18332,-1.33901,-2.54663,-2.40360},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.01652, 2.18946, 3.48099, 2.76791, 1.50648, 0.46786,-0.18837,-1.35791,-2.55110,-2.39781},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.01985, 2.21609, 3.48128, 2.75550, 1.49386, 0.45908,-0.19338,-1.37677,-2.55542,-2.39196},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.02360, 2.24250, 3.48134, 2.74305, 1.48128, 0.45035,-0.19836,-1.39558,-2.55957,-2.38605},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.02781, 2.26868, 3.48118, 2.73055, 1.46872, 0.44166,-0.20331,-1.41434,-2.56357,-2.38010},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.03250, 2.29463, 3.48080, 2.71800, 1.45620, 0.43301,-0.20822,-1.43305,-2.56740,-2.37409},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.03769, 2.32035, 3.48019, 2.70541, 1.44370, 0.42440,-0.21310,-1.45168,-2.57109,-2.36804},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.04339, 2.34583, 3.47937, 2.69277, 1.43124, 0.41584,-0.21795,-1.47026,-2.57461,-2.36194},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.04962, 2.37107, 3.47834, 2.68009, 1.41881, 0.40732,-0.22277,-1.48876,-2.57798,-2.35579},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.05641, 2.39606, 3.47709, 2.66737, 1.40641, 0.39884,-0.22756,-1.50718,-2.58120,-2.34959},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.06377, 2.42079, 3.47563, 2.65461, 1.39405, 0.39041,-0.23233,-1.52552,-2.58426,-2.34334},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.07170, 2.44528, 3.47396, 2.64181, 1.38172, 0.38201,-0.23709,-1.54378,-2.58718,-2.33706},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.08022, 2.46951, 3.47208, 2.62898, 1.36942, 0.37367,-0.24184,-1.56194,-2.58994,-2.33072},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.08934, 2.49347, 3.47000, 2.61610, 1.35715, 0.36536,-0.24659,-1.58001,-2.59255,-2.32435},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.09907, 2.51717, 3.46771, 2.60319, 1.34492, 0.35710,-0.25136,-1.59799,-2.59501,-2.31793},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.10940, 2.54061, 3.46523, 2.59025, 1.33273, 0.34888,-0.25615,-1.61586,-2.59733,-2.31147},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.12036, 2.56378, 3.46254, 2.57728, 1.32057, 0.34070,-0.26099,-1.63363,-2.59950,-2.30496},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.13194, 2.58667, 3.45966, 2.56427, 1.30844, 0.33257,-0.26587,-1.65129,-2.60153,-2.29842},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.14414, 2.60929, 3.45659, 2.55123, 1.29635, 0.32447,-0.27083,-1.66883,-2.60341,-2.29184},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.15697, 2.63163, 3.45332, 2.53817, 1.28429, 0.31642,-0.27587,-1.68626,-2.60515,-2.28522},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.17041, 2.65369, 3.44986, 2.52507, 1.27227, 0.30842,-0.28101,-1.70357,-2.60675,-2.27857},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.18448, 2.67547, 3.44621, 2.51195, 1.26028, 0.30045,-0.28627,-1.72076,-2.60821,-2.27188},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.19917, 2.69697, 3.44238, 2.49881, 1.24833, 0.29253,-0.29166,-1.73782,-2.60953,-2.26515},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.21446, 2.71818, 3.43836, 2.48564, 1.23642, 0.28465,-0.29720,-1.75475,-2.61071,-2.25838},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.23037, 2.73911, 3.43416, 2.47244, 1.22455, 0.27681,-0.30290,-1.77155,-2.61175,-2.25159},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.24687, 2.75974, 3.42978, 2.45923, 1.21271, 0.26901,-0.30877,-1.78822,-2.61266,-2.24475},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.26397, 2.78009, 3.42523, 2.44599, 1.20091, 0.26126,-0.31484,-1.80475,-2.61343,-2.23789},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.28165, 2.80014, 3.42049, 2.43273, 1.18914, 0.25355,-0.32112,-1.82114,-2.61407,-2.23100},
{ 0.00000, 0.00000, 0.00000, 0.00000, 0.29991, 2.81990, 3.41559, 2.41945, 1.17742, 0.24588,-0.32761,-1.83739,-2.61458,-2.22407}
  };
  //
  double par_fg31[192][14] = {
{-0.36755,-0.11501,-0.17663,-0.12161,-0.09237,-0.00718,-0.20659,-0.00731, 0.01691, 0.14667, 0.19461, 0.26220, 0.17445, 0.29942},
{-0.36897,-0.11433,-0.17651,-0.12153,-0.09260,-0.00688,-0.20864,-0.00537, 0.01854, 0.14757, 0.19488, 0.26183, 0.17455, 0.29746},
{-0.37038,-0.11364,-0.17640,-0.12145,-0.09282,-0.00660,-0.21066,-0.00341, 0.02017, 0.14847, 0.19514, 0.26145, 0.17466, 0.29547},
{-0.37180,-0.11295,-0.17630,-0.12137,-0.09304,-0.00635,-0.21265,-0.00143, 0.02180, 0.14937, 0.19541, 0.26105, 0.17479, 0.29346},
{-0.37322,-0.11225,-0.17620,-0.12130,-0.09326,-0.00612,-0.21461, 0.00058, 0.02341, 0.15027, 0.19568, 0.26065, 0.17494, 0.29143},
{-0.37463,-0.11155,-0.17612,-0.12123,-0.09347,-0.00592,-0.21654, 0.00262, 0.02502, 0.15117, 0.19594, 0.26023, 0.17510, 0.28938},
{-0.37604,-0.11084,-0.17604,-0.12116,-0.09368,-0.00576,-0.21844, 0.00467, 0.02663, 0.15206, 0.19621, 0.25981, 0.17528, 0.28730},
{-0.37746,-0.11013,-0.17597,-0.12110,-0.09387,-0.00562,-0.22030, 0.00675, 0.02823, 0.15295, 0.19647, 0.25937, 0.17547, 0.28520},
{-0.37887,-0.10941,-0.17591,-0.12104,-0.09407,-0.00551,-0.22212, 0.00884, 0.02982, 0.15384, 0.19674, 0.25892, 0.17567, 0.28308},
{-0.38027,-0.10869,-0.17585,-0.12098,-0.09425,-0.00544,-0.22391, 0.01096, 0.03141, 0.15473, 0.19700, 0.25847, 0.17589, 0.28093},
{-0.38168,-0.10797,-0.17581,-0.12093,-0.09443,-0.00540,-0.22565, 0.01309, 0.03300, 0.15562, 0.19727, 0.25800, 0.17612, 0.27876},
{-0.38308,-0.10724,-0.17577,-0.12088,-0.09460,-0.00540,-0.22735, 0.01524, 0.03458, 0.15651, 0.19753, 0.25753, 0.17635, 0.27658},
{-0.38448,-0.10651,-0.17574,-0.12083,-0.09476,-0.00544,-0.22900, 0.01741, 0.03616, 0.15739, 0.19780, 0.25704, 0.17660, 0.27437},
{-0.38588,-0.10578,-0.17572,-0.12079,-0.09492,-0.00551,-0.23060, 0.01958, 0.03773, 0.15828, 0.19806, 0.25655, 0.17686, 0.27214},
{-0.38728,-0.10505,-0.17570,-0.12075,-0.09506,-0.00563,-0.23215, 0.02178, 0.03931, 0.15916, 0.19832, 0.25606, 0.17712, 0.26989},
{-0.38866,-0.10432,-0.17570,-0.12072,-0.09520,-0.00579,-0.23365, 0.02398, 0.04087, 0.16004, 0.19858, 0.25555, 0.17739, 0.26762},
{-0.39005,-0.10358,-0.17570,-0.12070,-0.09532,-0.00599,-0.23509, 0.02619, 0.04244, 0.16092, 0.19884, 0.25504, 0.17767, 0.26533},
{-0.39143,-0.10285,-0.17570,-0.12067,-0.09544,-0.00623,-0.23648, 0.02842, 0.04400, 0.16179, 0.19910, 0.25452, 0.17795, 0.26303},
{-0.39280,-0.10212,-0.17572,-0.12066,-0.09554,-0.00652,-0.23781, 0.03065, 0.04556, 0.16267, 0.19936, 0.25400, 0.17823, 0.26070},
{-0.39417,-0.10139,-0.17574,-0.12065,-0.09564,-0.00686,-0.23907, 0.03288, 0.04711, 0.16354, 0.19962, 0.25348, 0.17852, 0.25837},
{-0.39553,-0.10066,-0.17577,-0.12064,-0.09572,-0.00725,-0.24027, 0.03512, 0.04866, 0.16441, 0.19987, 0.25295, 0.17881, 0.25602},
{-0.39689,-0.09993,-0.17581,-0.12064,-0.09580,-0.00768,-0.24140, 0.03737, 0.05021, 0.16527, 0.20013, 0.25241, 0.17910, 0.25365},
{-0.39823,-0.09921,-0.17586,-0.12065,-0.09586,-0.00816,-0.24246, 0.03961, 0.05176, 0.16613, 0.20038, 0.25188, 0.17939, 0.25128},
{-0.39957,-0.09849,-0.17591,-0.12066,-0.09591,-0.00870,-0.24346, 0.04186, 0.05330, 0.16699, 0.20063, 0.25134, 0.17967, 0.24889},
{-0.40090,-0.09777,-0.17596,-0.12067,-0.09594,-0.00928,-0.24438, 0.04410, 0.05483, 0.16785, 0.20088, 0.25081, 0.17996, 0.24649},
{-0.40222,-0.09706,-0.17603,-0.12070,-0.09596,-0.00992,-0.24523, 0.04634, 0.05636, 0.16870, 0.20112, 0.25027, 0.18023, 0.24409},
{-0.40353,-0.09635,-0.17610,-0.12073,-0.09597,-0.01062,-0.24600, 0.04857, 0.05789, 0.16955, 0.20136, 0.24974, 0.18051, 0.24168},
{-0.40483,-0.09565,-0.17618,-0.12076,-0.09597,-0.01136,-0.24669, 0.05080, 0.05941, 0.17039, 0.20160, 0.24920, 0.18077, 0.23926},
{-0.40612,-0.09495,-0.17627,-0.12081,-0.09595,-0.01216,-0.24731, 0.05302, 0.06093, 0.17123, 0.20184, 0.24867, 0.18103, 0.23684},
{-0.40739,-0.09426,-0.17636,-0.12086,-0.09592,-0.01302,-0.24784, 0.05523, 0.06244, 0.17206, 0.20207, 0.24814, 0.18128, 0.23442},
{-0.40865,-0.09358,-0.17646,-0.12091,-0.09588,-0.01393,-0.24829, 0.05743, 0.06395, 0.17289, 0.20230, 0.24762, 0.18151, 0.23200},
{-0.40990,-0.09291,-0.17656,-0.12098,-0.09582,-0.01490,-0.24866, 0.05961, 0.06544, 0.17372, 0.20253, 0.24710, 0.18174, 0.22957},
{-0.41113,-0.09225,-0.17667,-0.12104,-0.09574,-0.01592,-0.24894, 0.06178, 0.06694, 0.17453, 0.20275, 0.24659, 0.18195, 0.22715},
{-0.41235,-0.09159,-0.17679,-0.12112,-0.09565,-0.01701,-0.24913, 0.06393, 0.06842, 0.17534, 0.20297, 0.24608, 0.18215, 0.22474},
{-0.41356,-0.09095,-0.17691,-0.12120,-0.09555,-0.01814,-0.24924, 0.06606, 0.06990, 0.17615, 0.20319, 0.24558, 0.18234, 0.22233},
{-0.41475,-0.09031,-0.17704,-0.12130,-0.09542,-0.01934,-0.24926, 0.06818, 0.07137, 0.17694, 0.20340, 0.24509, 0.18251, 0.21993},
{-0.41592,-0.08968,-0.17717,-0.12139,-0.09529,-0.02059,-0.24919, 0.07027, 0.07283, 0.17773, 0.20361, 0.24461, 0.18266, 0.21753},
{-0.41707,-0.08907,-0.17731,-0.12150,-0.09514,-0.02190,-0.24904, 0.07234, 0.07428, 0.17852, 0.20381, 0.24413, 0.18279, 0.21515},
{-0.41821,-0.08847,-0.17745,-0.12161,-0.09497,-0.02327,-0.24879, 0.07438, 0.07573, 0.17929, 0.20401, 0.24367, 0.18290, 0.21278},
{-0.41933,-0.08788,-0.17760,-0.12173,-0.09479,-0.02469,-0.24845, 0.07640, 0.07716, 0.18006, 0.20420, 0.24322, 0.18300, 0.21042},
{-0.42043,-0.08730,-0.17775,-0.12185,-0.09459,-0.02616,-0.24803, 0.07839, 0.07859, 0.18082, 0.20438, 0.24278, 0.18307, 0.20808},
{-0.42150,-0.08674,-0.17791,-0.12199,-0.09438,-0.02769,-0.24751, 0.08035, 0.08000, 0.18157, 0.20457, 0.24235, 0.18312, 0.20576},
{-0.42256,-0.08618,-0.17807,-0.12213,-0.09415,-0.02928,-0.24690, 0.08228, 0.08140, 0.18231, 0.20474, 0.24194, 0.18315, 0.20345},
{-0.42360,-0.08565,-0.17823,-0.12227,-0.09391,-0.03092,-0.24620, 0.08418, 0.08279, 0.18304, 0.20492, 0.24153, 0.18316, 0.20117},
{-0.42462,-0.08512,-0.17840,-0.12243,-0.09365,-0.03261,-0.24542, 0.08605, 0.08417, 0.18376, 0.20508, 0.24115, 0.18314, 0.19891},
{-0.42562,-0.08461,-0.17857,-0.12259,-0.09337,-0.03435,-0.24454, 0.08788, 0.08554, 0.18447, 0.20524, 0.24077, 0.18309, 0.19667},
{-0.42659,-0.08412,-0.17875,-0.12275,-0.09308,-0.03615,-0.24358, 0.08967, 0.08689, 0.18517, 0.20540, 0.24042, 0.18303, 0.19445},
{-0.42755,-0.08364,-0.17893,-0.12293,-0.09278,-0.03799,-0.24253, 0.09143, 0.08823, 0.18586, 0.20555, 0.24007, 0.18293, 0.19227},
{-0.42848,-0.08318,-0.17911,-0.12311,-0.09246,-0.03988,-0.24139, 0.09315, 0.08956, 0.18654, 0.20569, 0.23975, 0.18282, 0.19010},
{-0.42939,-0.08273,-0.17930,-0.12329,-0.09212,-0.04182,-0.24017, 0.09483, 0.09087, 0.18721, 0.20582, 0.23944, 0.18267, 0.18797},
{-0.43027,-0.08229,-0.17949,-0.12349,-0.09177,-0.04380,-0.23886, 0.09648, 0.09217, 0.18787, 0.20596, 0.23914, 0.18250, 0.18587},
{-0.43113,-0.08188,-0.17968,-0.12368,-0.09141,-0.04583,-0.23747, 0.09808, 0.09345, 0.18851, 0.20608, 0.23887, 0.18230, 0.18380},
{-0.43197,-0.08148,-0.17987,-0.12389,-0.09104,-0.04790,-0.23600, 0.09964, 0.09472, 0.18915, 0.20620, 0.23861, 0.18207, 0.18176},
{-0.43279,-0.08109,-0.18006,-0.12410,-0.09065,-0.05001,-0.23445, 0.10116, 0.09597, 0.18977, 0.20631, 0.23837, 0.18182, 0.17975},
{-0.43358,-0.08072,-0.18026,-0.12431,-0.09025,-0.05216,-0.23283, 0.10264, 0.09721, 0.19039, 0.20641, 0.23814, 0.18154, 0.17778},
{-0.43434,-0.08037,-0.18046,-0.12453,-0.08984,-0.05434,-0.23113, 0.10408, 0.09843, 0.19099, 0.20651, 0.23794, 0.18123, 0.17584},
{-0.43509,-0.08003,-0.18066,-0.12476,-0.08942,-0.05655,-0.22936, 0.10547, 0.09963, 0.19158, 0.20660, 0.23775, 0.18090, 0.17394},
{-0.43581,-0.07971,-0.18086,-0.12498,-0.08899,-0.05879,-0.22753, 0.10682, 0.10082, 0.19215, 0.20668, 0.23759, 0.18053, 0.17207},
{-0.43651,-0.07941,-0.18106,-0.12522,-0.08856,-0.06106,-0.22563, 0.10814, 0.10199, 0.19272, 0.20676, 0.23744, 0.18014, 0.17024},
{-0.43718,-0.07912,-0.18126,-0.12545,-0.08812,-0.06334,-0.22366, 0.10941, 0.10315, 0.19328, 0.20682, 0.23732, 0.17972, 0.16844},
{-0.43784,-0.07885,-0.18146,-0.12569,-0.08767,-0.06564,-0.22164, 0.11064, 0.10429, 0.19382, 0.20688, 0.23722, 0.17928, 0.16667},
{-0.43847,-0.07859,-0.18167,-0.12593,-0.08723,-0.06796,-0.21957, 0.11183, 0.10541, 0.19436, 0.20693, 0.23714, 0.17880, 0.16495},
{-0.43908,-0.07835,-0.18187,-0.12617,-0.08678,-0.07028,-0.21744, 0.11298, 0.10652, 0.19488, 0.20697, 0.23708, 0.17830, 0.16325},
{-0.43967,-0.07812,-0.18208,-0.12641,-0.08634,-0.07261,-0.21527, 0.11409, 0.10760, 0.19540, 0.20699, 0.23704, 0.17777, 0.16159},
{-0.44024,-0.07791,-0.18228,-0.12665,-0.08591,-0.07493,-0.21305, 0.11517, 0.10868, 0.19590, 0.20701, 0.23703, 0.17722, 0.15996},
{-0.44079,-0.07771,-0.18249,-0.12689,-0.08548,-0.07725,-0.21080, 0.11621, 0.10974, 0.19640, 0.20701, 0.23705, 0.17664, 0.15836},
{-0.44132,-0.07753,-0.18270,-0.12713,-0.08505,-0.07956,-0.20851, 0.11722, 0.11078, 0.19689, 0.20701, 0.23709, 0.17603, 0.15680},
{-0.44184,-0.07736,-0.18290,-0.12737,-0.08464,-0.08186,-0.20619, 0.11820, 0.11181, 0.19738, 0.20699, 0.23715, 0.17539, 0.15526},
{-0.44234,-0.07721,-0.18311,-0.12761,-0.08425,-0.08414,-0.20384, 0.11914, 0.11282, 0.19785, 0.20695, 0.23724, 0.17473, 0.15375},
{-0.44282,-0.07707,-0.18332,-0.12785,-0.08387,-0.08639,-0.20146, 0.12005, 0.11381, 0.19832, 0.20690, 0.23736, 0.17404, 0.15227},
{-0.44328,-0.07694,-0.18352,-0.12808,-0.08350,-0.08862,-0.19906, 0.12094, 0.11480, 0.19879, 0.20684, 0.23751, 0.17332, 0.15082},
{-0.44373,-0.07683,-0.18373,-0.12830,-0.08316,-0.09081,-0.19665, 0.12180, 0.11577, 0.19924, 0.20676, 0.23768, 0.17258, 0.14939},
{-0.44417,-0.07673,-0.18394,-0.12853,-0.08283,-0.09298,-0.19422, 0.12263, 0.11672, 0.19970, 0.20667, 0.23788, 0.17181, 0.14799},
{-0.44459,-0.07664,-0.18415,-0.12875,-0.08253,-0.09510,-0.19178, 0.12344, 0.11767, 0.20015, 0.20656, 0.23811, 0.17102, 0.14661},
{-0.44501,-0.07656,-0.18436,-0.12896,-0.08226,-0.09718,-0.18934, 0.12422, 0.11860, 0.20060, 0.20643, 0.23838, 0.17019, 0.14525},
{-0.44540,-0.07650,-0.18457,-0.12917,-0.08201,-0.09921,-0.18688, 0.12499, 0.11951, 0.20104, 0.20628, 0.23867, 0.16935, 0.14391},
{-0.44579,-0.07644,-0.18478,-0.12937,-0.08180,-0.10120,-0.18442, 0.12573, 0.12042, 0.20148, 0.20612, 0.23899, 0.16847, 0.14259},
{-0.44617,-0.07640,-0.18499,-0.12956,-0.08161,-0.10313,-0.18197, 0.12646, 0.12131, 0.20192, 0.20594, 0.23934, 0.16758, 0.14129},
{-0.44654,-0.07637,-0.18520,-0.12975,-0.08146,-0.10501,-0.17951, 0.12717, 0.12220, 0.20236, 0.20574, 0.23972, 0.16665, 0.14000},
{-0.44690,-0.07635,-0.18541,-0.12993,-0.08134,-0.10683,-0.17706, 0.12786, 0.12307, 0.20280, 0.20552, 0.24013, 0.16571, 0.13872},
{-0.44725,-0.07634,-0.18563,-0.13010,-0.08125,-0.10859,-0.17461, 0.12854, 0.12393, 0.20323, 0.20529, 0.24057, 0.16474, 0.13746},
{-0.44759,-0.07633,-0.18584,-0.13026,-0.08120,-0.11029,-0.17217, 0.12921, 0.12479, 0.20367, 0.20503, 0.24104, 0.16374, 0.13622},
{-0.44793,-0.07634,-0.18605,-0.13042,-0.08119,-0.11193,-0.16974, 0.12986, 0.12564, 0.20410, 0.20476, 0.24154, 0.16273, 0.13498},
{-0.44826,-0.07636,-0.18627,-0.13056,-0.08122,-0.11350,-0.16732, 0.13051, 0.12647, 0.20454, 0.20447, 0.24206, 0.16169, 0.13375},
{-0.44858,-0.07638,-0.18649,-0.13070,-0.08129,-0.11501,-0.16491, 0.13114, 0.12730, 0.20498, 0.20417, 0.24260, 0.16063, 0.13253},
{-0.44890,-0.07642,-0.18671,-0.13082,-0.08140,-0.11644,-0.16251, 0.13177, 0.12813, 0.20542, 0.20385, 0.24317, 0.15955, 0.13131},
{-0.44921,-0.07646,-0.18693,-0.13094,-0.08155,-0.11781,-0.16012, 0.13239, 0.12895, 0.20586, 0.20352, 0.24377, 0.15845, 0.13010},
{-0.44952,-0.07651,-0.18715,-0.13104,-0.08175,-0.11911,-0.15775, 0.13300, 0.12976, 0.20630, 0.20317, 0.24438, 0.15734, 0.12889},
{-0.44983,-0.07657,-0.18737,-0.13113,-0.08198,-0.12034,-0.15539, 0.13361, 0.13056, 0.20674, 0.20280, 0.24501, 0.15621, 0.12769},
{-0.45013,-0.07664,-0.18760,-0.13121,-0.08227,-0.12150,-0.15305, 0.13422, 0.13136, 0.20718, 0.20243, 0.24566, 0.15506, 0.12648},
{-0.45043,-0.07671,-0.18782,-0.13128,-0.08259,-0.12258,-0.15071, 0.13482, 0.13216, 0.20763, 0.20204, 0.24632, 0.15389, 0.12528},
{-0.45072,-0.07680,-0.18805,-0.13134,-0.08297,-0.12359,-0.14840, 0.13541, 0.13295, 0.20808, 0.20164, 0.24700, 0.15271, 0.12407},
{-0.45101,-0.07689,-0.18828,-0.13139,-0.08339,-0.12453,-0.14610, 0.13601, 0.13374, 0.20852, 0.20123, 0.24769, 0.15152, 0.12287},
{-0.45130,-0.07698,-0.18852,-0.13142,-0.08386,-0.12539,-0.14381, 0.13660, 0.13453, 0.20897, 0.20082, 0.24838, 0.15032, 0.12166},
{-0.45158,-0.07708,-0.18875,-0.13144,-0.08437,-0.12618,-0.14154, 0.13719, 0.13531, 0.20943, 0.20039, 0.24909, 0.14911, 0.12045},
{-0.45187,-0.07719,-0.18899,-0.13145,-0.08493,-0.12690,-0.13929, 0.13778, 0.13609, 0.20988, 0.19996, 0.24980, 0.14788, 0.11923},
{-0.45215,-0.07731,-0.18923,-0.13144,-0.08555,-0.12753,-0.13705, 0.13837, 0.13687, 0.21033, 0.19952, 0.25052, 0.14665, 0.11801},
{-0.45243,-0.07743,-0.18947,-0.13142,-0.08621,-0.12810,-0.13483, 0.13895, 0.13764, 0.21079, 0.19907, 0.25124, 0.14541, 0.11678},
{-0.45271,-0.07756,-0.18972,-0.13139,-0.08692,-0.12858,-0.13262, 0.13954, 0.13841, 0.21125, 0.19862, 0.25196, 0.14417, 0.11555},
{-0.45298,-0.07770,-0.18997,-0.13134,-0.08768,-0.12899,-0.13043, 0.14013, 0.13918, 0.21171, 0.19817, 0.25268, 0.14292, 0.11431},
{-0.45326,-0.07784,-0.19022,-0.13128,-0.08849,-0.12933,-0.12826, 0.14072, 0.13995, 0.21217, 0.19772, 0.25340, 0.14166, 0.11306},
{-0.45353,-0.07799,-0.19048,-0.13121,-0.08935,-0.12958,-0.12610, 0.14131, 0.14072, 0.21263, 0.19726, 0.25412, 0.14040, 0.11180},
{-0.45380,-0.07814,-0.19074,-0.13112,-0.09027,-0.12976,-0.12396, 0.14190, 0.14149, 0.21309, 0.19681, 0.25482, 0.13914, 0.11054},
{-0.45407,-0.07831,-0.19100,-0.13101,-0.09123,-0.12986,-0.12184, 0.14249, 0.14225, 0.21356, 0.19635, 0.25552, 0.13787, 0.10927},
{-0.45434,-0.07847,-0.19126,-0.13089,-0.09225,-0.12988,-0.11974, 0.14308, 0.14301, 0.21402, 0.19590, 0.25622, 0.13661, 0.10799},
{-0.45461,-0.07865,-0.19153,-0.13076,-0.09332,-0.12982,-0.11765, 0.14368, 0.14378, 0.21448, 0.19544, 0.25690, 0.13535, 0.10671},
{-0.45487,-0.07883,-0.19181,-0.13061,-0.09444,-0.12969,-0.11558, 0.14427, 0.14454, 0.21495, 0.19499, 0.25757, 0.13409, 0.10541},
{-0.45514,-0.07901,-0.19208,-0.13044,-0.09561,-0.12947,-0.11353, 0.14487, 0.14529, 0.21541, 0.19455, 0.25822, 0.13283, 0.10411},
{-0.45540,-0.07920,-0.19236,-0.13026,-0.09684,-0.12917,-0.11150, 0.14546, 0.14605, 0.21588, 0.19411, 0.25886, 0.13157, 0.10280},
{-0.45566,-0.07940,-0.19264,-0.13006,-0.09812,-0.12880,-0.10949, 0.14606, 0.14680, 0.21634, 0.19367, 0.25949, 0.13032, 0.10148},
{-0.45592,-0.07961,-0.19293,-0.12985,-0.09945,-0.12834,-0.10750, 0.14665, 0.14756, 0.21680, 0.19324, 0.26010, 0.12908, 0.10015},
{-0.45618,-0.07982,-0.19322,-0.12962,-0.10083,-0.12780,-0.10553, 0.14725, 0.14830, 0.21726, 0.19282, 0.26068, 0.12784, 0.09882},
{-0.45643,-0.08003,-0.19351,-0.12937,-0.10227,-0.12718,-0.10358, 0.14784, 0.14905, 0.21772, 0.19241, 0.26125, 0.12661, 0.09748},
{-0.45669,-0.08026,-0.19381,-0.12911,-0.10376,-0.12648,-0.10165, 0.14844, 0.14980, 0.21818, 0.19200, 0.26180, 0.12539, 0.09613},
{-0.45694,-0.08048,-0.19411,-0.12883,-0.10530,-0.12570,-0.09974, 0.14904, 0.15054, 0.21864, 0.19161, 0.26233, 0.12418, 0.09477},
{-0.45719,-0.08072,-0.19441,-0.12854,-0.10689,-0.12484,-0.09786, 0.14963, 0.15127, 0.21909, 0.19122, 0.26283, 0.12298, 0.09341},
{-0.45744,-0.08096,-0.19472,-0.12822,-0.10854,-0.12389,-0.09600, 0.15022, 0.15201, 0.21955, 0.19085, 0.26330, 0.12180, 0.09205},
{-0.45768,-0.08120,-0.19503,-0.12790,-0.11024,-0.12287,-0.09417, 0.15082, 0.15274, 0.22000, 0.19048, 0.26375, 0.12062, 0.09068},
{-0.45793,-0.08146,-0.19534,-0.12755,-0.11199,-0.12176,-0.09236, 0.15141, 0.15347, 0.22044, 0.19013, 0.26418, 0.11946, 0.08930},
{-0.45817,-0.08171,-0.19566,-0.12719,-0.11379,-0.12058,-0.09057, 0.15200, 0.15419, 0.22089, 0.18979, 0.26458, 0.11831, 0.08792},
{-0.45841,-0.08198,-0.19598,-0.12682,-0.11564,-0.11931,-0.08882, 0.15258, 0.15490, 0.22133, 0.18947, 0.26495, 0.11718, 0.08653},
{-0.45865,-0.08224,-0.19630,-0.12643,-0.11753,-0.11796,-0.08709, 0.15317, 0.15562, 0.22177, 0.18915, 0.26529, 0.11607, 0.08515},
{-0.45888,-0.08252,-0.19663,-0.12602,-0.11948,-0.11653,-0.08539, 0.15375, 0.15632, 0.22220, 0.18886, 0.26560, 0.11497, 0.08376},
{-0.45911,-0.08280,-0.19696,-0.12560,-0.12148,-0.11502,-0.08372, 0.15433, 0.15702, 0.22263, 0.18857, 0.26588, 0.11389, 0.08237},
{-0.45934,-0.08308,-0.19729,-0.12516,-0.12352,-0.11344,-0.08208, 0.15490, 0.15772, 0.22306, 0.18830, 0.26612, 0.11283, 0.08097},
{-0.45957,-0.08337,-0.19763,-0.12470,-0.12562,-0.11177,-0.08047, 0.15547, 0.15841, 0.22348, 0.18805, 0.26634, 0.11179, 0.07958},
{-0.45979,-0.08367,-0.19796,-0.12423,-0.12775,-0.11003,-0.07889, 0.15604, 0.15909, 0.22389, 0.18782, 0.26653, 0.11077, 0.07819},
{-0.46001,-0.08397,-0.19830,-0.12374,-0.12993,-0.10821,-0.07734, 0.15660, 0.15976, 0.22430, 0.18760, 0.26668, 0.10977, 0.07680},
{-0.46023,-0.08428,-0.19865,-0.12324,-0.13216,-0.10631,-0.07583, 0.15716, 0.16043, 0.22471, 0.18739, 0.26680, 0.10880, 0.07541},
{-0.46045,-0.08459,-0.19899,-0.12273,-0.13442,-0.10435,-0.07435, 0.15772, 0.16109, 0.22511, 0.18721, 0.26688, 0.10785, 0.07402},
{-0.46066,-0.08490,-0.19934,-0.12220,-0.13673,-0.10230,-0.07290, 0.15826, 0.16174, 0.22550, 0.18704, 0.26693, 0.10692, 0.07264},
{-0.46087,-0.08522,-0.19969,-0.12166,-0.13908,-0.10019,-0.07149, 0.15881, 0.16238, 0.22589, 0.18689, 0.26695, 0.10602, 0.07126},
{-0.46107,-0.08554,-0.20004,-0.12110,-0.14147,-0.09801,-0.07012, 0.15934, 0.16301, 0.22627, 0.18676, 0.26693, 0.10514, 0.06988},
{-0.46127,-0.08587,-0.20039,-0.12053,-0.14389,-0.09575,-0.06878, 0.15988, 0.16364, 0.22665, 0.18665, 0.26688, 0.10429, 0.06851},
{-0.46147,-0.08620,-0.20075,-0.11994,-0.14635,-0.09343,-0.06748, 0.16040, 0.16425, 0.22702, 0.18656, 0.26679, 0.10346, 0.06715},
{-0.46167,-0.08654,-0.20110,-0.11935,-0.14884,-0.09104,-0.06622, 0.16092, 0.16486, 0.22738, 0.18648, 0.26667, 0.10266, 0.06580},
{-0.46186,-0.08688,-0.20146,-0.11874,-0.15137,-0.08859,-0.06499, 0.16143, 0.16546, 0.22773, 0.18642, 0.26651, 0.10189, 0.06445},
{-0.46205,-0.08722,-0.20182,-0.11812,-0.15392,-0.08608,-0.06381, 0.16193, 0.16604, 0.22808, 0.18639, 0.26632, 0.10115, 0.06311},
{-0.46224,-0.08756,-0.20218,-0.11749,-0.15651,-0.08351,-0.06266, 0.16243, 0.16662, 0.22842, 0.18637, 0.26610, 0.10043, 0.06178},
{-0.46242,-0.08791,-0.20254,-0.11685,-0.15912,-0.08088,-0.06155, 0.16292, 0.16718, 0.22876, 0.18637, 0.26584, 0.09974, 0.06045},
{-0.46260,-0.08826,-0.20290,-0.11619,-0.16176,-0.07819,-0.06048, 0.16340, 0.16774, 0.22908, 0.18639, 0.26554, 0.09908, 0.05914},
{-0.46277,-0.08861,-0.20326,-0.11553,-0.16442,-0.07545,-0.05945, 0.16388, 0.16828, 0.22940, 0.18642, 0.26522, 0.09845, 0.05784},
{-0.46295,-0.08897,-0.20362,-0.11486,-0.16710,-0.07266,-0.05846, 0.16434, 0.16882, 0.22971, 0.18648, 0.26486, 0.09785, 0.05655},
{-0.46311,-0.08932,-0.20398,-0.11418,-0.16981,-0.06982,-0.05751, 0.16480, 0.16934, 0.23001, 0.18656, 0.26446, 0.09728, 0.05528},
{-0.46328,-0.08968,-0.20433,-0.11349,-0.17253,-0.06694,-0.05660, 0.16525, 0.16985, 0.23031, 0.18665, 0.26404, 0.09673, 0.05401},
{-0.46344,-0.09004,-0.20469,-0.11279,-0.17526,-0.06401,-0.05573, 0.16569, 0.17035, 0.23059, 0.18676, 0.26358, 0.09622, 0.05276},
{-0.46360,-0.09040,-0.20505,-0.11209,-0.17802,-0.06104,-0.05489, 0.16612, 0.17083, 0.23087, 0.18689, 0.26310, 0.09574, 0.05153},
{-0.46375,-0.09076,-0.20541,-0.11138,-0.18078,-0.05802,-0.05410, 0.16655, 0.17131, 0.23114, 0.18704, 0.26258, 0.09528, 0.05030},
{-0.46390,-0.09112,-0.20577,-0.11066,-0.18355,-0.05498,-0.05335, 0.16696, 0.17178, 0.23141, 0.18721, 0.26203, 0.09485, 0.04910},
{-0.46405,-0.09149,-0.20612,-0.10994,-0.18633,-0.05190,-0.05263, 0.16737, 0.17223, 0.23166, 0.18739, 0.26145, 0.09446, 0.04790},
{-0.46420,-0.09185,-0.20647,-0.10922,-0.18912,-0.04879,-0.05195, 0.16777, 0.17267, 0.23191, 0.18759, 0.26085, 0.09409, 0.04672},
{-0.46434,-0.09221,-0.20682,-0.10849,-0.19190,-0.04566,-0.05131, 0.16816, 0.17310, 0.23214, 0.18781, 0.26022, 0.09374, 0.04556},
{-0.46448,-0.09257,-0.20717,-0.10777,-0.19468,-0.04250,-0.05070, 0.16854, 0.17352, 0.23237, 0.18805, 0.25956, 0.09343, 0.04441},
{-0.46461,-0.09294,-0.20752,-0.10704,-0.19745,-0.03933,-0.05013, 0.16892, 0.17393, 0.23259, 0.18830, 0.25887, 0.09314, 0.04327},
{-0.46475,-0.09330,-0.20786,-0.10633,-0.20021,-0.03613,-0.04959, 0.16928, 0.17434, 0.23279, 0.18858, 0.25815, 0.09287, 0.04215},
{-0.46488,-0.09366,-0.20819,-0.10561,-0.20295,-0.03293,-0.04908, 0.16964, 0.17473, 0.23299, 0.18887, 0.25742, 0.09263, 0.04104},
{-0.46501,-0.09402,-0.20853,-0.10491,-0.20567,-0.02973,-0.04860, 0.16999, 0.17511, 0.23317, 0.18918, 0.25665, 0.09241, 0.03994},
{-0.46514,-0.09438,-0.20885,-0.10421,-0.20837,-0.02651,-0.04814, 0.17033, 0.17549, 0.23335, 0.18951, 0.25586, 0.09221, 0.03885},
{-0.46526,-0.09474,-0.20918,-0.10353,-0.21104,-0.02330,-0.04771, 0.17067, 0.17586, 0.23351, 0.18986, 0.25505, 0.09204, 0.03777},
{-0.46539,-0.09510,-0.20949,-0.10286,-0.21367,-0.02009,-0.04731, 0.17099, 0.17623, 0.23366, 0.19024, 0.25421, 0.09188, 0.03671},
{-0.46552,-0.09545,-0.20980,-0.10221,-0.21627,-0.01689,-0.04692, 0.17132, 0.17659, 0.23379, 0.19063, 0.25335, 0.09174, 0.03565},
{-0.46564,-0.09581,-0.21010,-0.10158,-0.21882,-0.01369,-0.04656, 0.17163, 0.17694, 0.23391, 0.19104, 0.25247, 0.09162, 0.03460},
{-0.46577,-0.09616,-0.21040,-0.10097,-0.22134,-0.01051,-0.04621, 0.17194, 0.17729, 0.23402, 0.19148, 0.25156, 0.09151, 0.03355},
{-0.46590,-0.09652,-0.21068,-0.10038,-0.22380,-0.00735,-0.04588, 0.17225, 0.17764, 0.23411, 0.19194, 0.25064, 0.09141, 0.03251},
{-0.46602,-0.09687,-0.21096,-0.09982,-0.22621,-0.00420,-0.04556, 0.17254, 0.17799, 0.23419, 0.19242, 0.24969, 0.09133, 0.03148},
{-0.46615,-0.09722,-0.21123,-0.09928,-0.22856,-0.00108,-0.04525, 0.17284, 0.17833, 0.23425, 0.19293, 0.24872, 0.09126, 0.03044},
{-0.46628,-0.09757,-0.21149,-0.09878,-0.23086, 0.00203,-0.04495, 0.17313, 0.17868, 0.23430, 0.19345, 0.24772, 0.09121, 0.02941},
{-0.46641,-0.09792,-0.21174,-0.09830,-0.23309, 0.00511,-0.04465, 0.17341, 0.17902, 0.23433, 0.19401, 0.24671, 0.09116, 0.02838},
{-0.46654,-0.09827,-0.21198,-0.09785,-0.23527, 0.00816,-0.04436, 0.17369, 0.17937, 0.23434, 0.19458, 0.24567, 0.09111, 0.02735},
{-0.46668,-0.09862,-0.21221,-0.09744,-0.23737, 0.01119,-0.04408, 0.17397, 0.17972, 0.23434, 0.19519, 0.24461, 0.09108, 0.02631},
{-0.46681,-0.09897,-0.21243,-0.09706,-0.23941, 0.01418,-0.04380, 0.17425, 0.18007, 0.23431, 0.19582, 0.24353, 0.09105, 0.02528},
{-0.46695,-0.09932,-0.21264,-0.09672,-0.24138, 0.01715,-0.04352, 0.17452, 0.18042, 0.23427, 0.19647, 0.24243, 0.09103, 0.02424},
{-0.46709,-0.09967,-0.21284,-0.09642,-0.24327, 0.02009,-0.04323, 0.17479, 0.18077, 0.23422, 0.19715, 0.24131, 0.09101, 0.02319},
{-0.46724,-0.10001,-0.21303,-0.09615,-0.24509, 0.02299,-0.04295, 0.17505, 0.18113, 0.23414, 0.19785, 0.24017, 0.09099, 0.02214},
{-0.46738,-0.10036,-0.21320,-0.09592,-0.24684, 0.02587,-0.04266, 0.17532, 0.18150, 0.23405, 0.19858, 0.23901, 0.09097, 0.02108},
{-0.46753,-0.10071,-0.21337,-0.09573,-0.24852, 0.02871,-0.04237, 0.17558, 0.18186, 0.23394, 0.19933, 0.23783, 0.09096, 0.02002},
{-0.46768,-0.10106,-0.21352,-0.09559,-0.25012, 0.03152,-0.04207, 0.17584, 0.18224, 0.23381, 0.20011, 0.23662, 0.09095, 0.01894},
{-0.46783,-0.10141,-0.21366,-0.09548,-0.25164, 0.03430,-0.04176, 0.17610, 0.18262, 0.23367, 0.20090, 0.23540, 0.09093, 0.01786},
{-0.46799,-0.10175,-0.21378,-0.09541,-0.25308, 0.03704,-0.04145, 0.17635, 0.18300, 0.23351, 0.20172, 0.23417, 0.09092, 0.01677},
{-0.46815,-0.10210,-0.21390,-0.09539,-0.25446, 0.03976,-0.04113, 0.17661, 0.18339, 0.23333, 0.20256, 0.23291, 0.09090, 0.01566},
{-0.46831,-0.10245,-0.21400,-0.09540,-0.25575, 0.04244,-0.04080, 0.17687, 0.18378, 0.23314, 0.20341, 0.23164, 0.09088, 0.01454},
{-0.46847,-0.10281,-0.21409,-0.09546,-0.25697, 0.04510,-0.04045, 0.17712, 0.18418, 0.23294, 0.20429, 0.23036, 0.09085, 0.01342},
{-0.46864,-0.10316,-0.21416,-0.09557,-0.25811, 0.04772,-0.04010, 0.17738, 0.18459, 0.23273, 0.20517, 0.22905, 0.09082, 0.01228},
{-0.46881,-0.10351,-0.21423,-0.09571,-0.25918, 0.05032,-0.03974, 0.17763, 0.18500, 0.23250, 0.20607, 0.22774, 0.09079, 0.01112},
{-0.46898,-0.10387,-0.21427,-0.09590,-0.26017, 0.05289,-0.03936, 0.17789, 0.18542, 0.23227, 0.20698, 0.22641, 0.09075, 0.00996},
{-0.46915,-0.10423,-0.21431,-0.09613,-0.26109, 0.05543,-0.03897, 0.17814, 0.18584, 0.23202, 0.20790, 0.22507, 0.09070, 0.00878},
{-0.46933,-0.10459,-0.21433,-0.09641,-0.26194, 0.05795,-0.03857, 0.17840, 0.18627, 0.23177, 0.20882, 0.22372, 0.09065, 0.00758},
{-0.46951,-0.10495,-0.21433,-0.09672,-0.26271, 0.06044,-0.03816, 0.17866, 0.18671, 0.23151, 0.20975, 0.22236, 0.09059, 0.00637},
{-0.46969,-0.10532,-0.21433,-0.09709,-0.26341, 0.06291,-0.03773, 0.17891, 0.18715, 0.23124, 0.21069, 0.22100, 0.09052, 0.00515},
{-0.46987,-0.10568,-0.21430,-0.09749,-0.26404, 0.06535,-0.03730, 0.17917, 0.18759, 0.23097, 0.21162, 0.21962, 0.09045, 0.00391},
{-0.47005,-0.10606,-0.21427,-0.09794,-0.26459, 0.06777,-0.03684, 0.17943, 0.18804, 0.23069, 0.21256, 0.21824, 0.09037, 0.00265},
{-0.47024,-0.10643,-0.21421,-0.09843,-0.26508, 0.07017,-0.03638, 0.17969, 0.18850, 0.23041, 0.21349, 0.21685, 0.09027, 0.00138}
  };
  //
  double par_fg32[192][14] = {
{-0.08686, 0.04052, 0.01094, 0.00630,-0.01494, 0.02455,-0.13184, 0.11156, 0.10512, 0.05652, 0.01590,-0.02080, 0.00024,-0.11721},
{-0.08642, 0.04075, 0.01054, 0.00621,-0.01471, 0.02362,-0.13050, 0.11249, 0.10426, 0.05614, 0.01577,-0.02131, 0.00096,-0.11781},
{-0.08598, 0.04097, 0.01016, 0.00612,-0.01447, 0.02265,-0.12910, 0.11337, 0.10342, 0.05577, 0.01564,-0.02181, 0.00167,-0.11842},
{-0.08553, 0.04117, 0.00978, 0.00602,-0.01422, 0.02164,-0.12763, 0.11421, 0.10262, 0.05541, 0.01551,-0.02231, 0.00237,-0.11903},
{-0.08508, 0.04135, 0.00940, 0.00593,-0.01397, 0.02060,-0.12611, 0.11501, 0.10183, 0.05505, 0.01538,-0.02280, 0.00304,-0.11963},
{-0.08462, 0.04152, 0.00904, 0.00583,-0.01370, 0.01951,-0.12453, 0.11576, 0.10108, 0.05470, 0.01526,-0.02329, 0.00370,-0.12024},
{-0.08416, 0.04168, 0.00868, 0.00572,-0.01343, 0.01840,-0.12289, 0.11646, 0.10034, 0.05435, 0.01513,-0.02377, 0.00433,-0.12085},
{-0.08370, 0.04182, 0.00833, 0.00562,-0.01314, 0.01724,-0.12119, 0.11713, 0.09963, 0.05401, 0.01501,-0.02423, 0.00495,-0.12145},
{-0.08324, 0.04195, 0.00798, 0.00551,-0.01285, 0.01605,-0.11944, 0.11775, 0.09894, 0.05368, 0.01488,-0.02469, 0.00554,-0.12205},
{-0.08277, 0.04206, 0.00764, 0.00540,-0.01255, 0.01482,-0.11764, 0.11834, 0.09828, 0.05335, 0.01476,-0.02514, 0.00610,-0.12265},
{-0.08230, 0.04216, 0.00731, 0.00528,-0.01224, 0.01356,-0.11577, 0.11888, 0.09763, 0.05303, 0.01463,-0.02557, 0.00664,-0.12323},
{-0.08182, 0.04224, 0.00698, 0.00516,-0.01192, 0.01227,-0.11386, 0.11938, 0.09700, 0.05271, 0.01450,-0.02600, 0.00716,-0.12381},
{-0.08134, 0.04231, 0.00666, 0.00503,-0.01159, 0.01094,-0.11189, 0.11985, 0.09639, 0.05239, 0.01438,-0.02641, 0.00764,-0.12438},
{-0.08085, 0.04237, 0.00635, 0.00491,-0.01125, 0.00957,-0.10987, 0.12027, 0.09580, 0.05208, 0.01425,-0.02680, 0.00810,-0.12494},
{-0.08036, 0.04242, 0.00604, 0.00478,-0.01090, 0.00818,-0.10780, 0.12066, 0.09523, 0.05177, 0.01412,-0.02719, 0.00853,-0.12549},
{-0.07987, 0.04245, 0.00574, 0.00464,-0.01054, 0.00675,-0.10567, 0.12101, 0.09467, 0.05147, 0.01399,-0.02755, 0.00893,-0.12602},
{-0.07937, 0.04247, 0.00544, 0.00451,-0.01017, 0.00529,-0.10350, 0.12132, 0.09413, 0.05116, 0.01386,-0.02791, 0.00930,-0.12654},
{-0.07886, 0.04247, 0.00515, 0.00436,-0.00980, 0.00380,-0.10128, 0.12160, 0.09361, 0.05087, 0.01373,-0.02824, 0.00964,-0.12705},
{-0.07835, 0.04246, 0.00486, 0.00422,-0.00941, 0.00229,-0.09900, 0.12183, 0.09309, 0.05057, 0.01360,-0.02857, 0.00995,-0.12754},
{-0.07783, 0.04244, 0.00458, 0.00407,-0.00902, 0.00074,-0.09669, 0.12203, 0.09259, 0.05027, 0.01346,-0.02887, 0.01023,-0.12801},
{-0.07731, 0.04241, 0.00430, 0.00392,-0.00862,-0.00084,-0.09432, 0.12220, 0.09211, 0.04998, 0.01332,-0.02916, 0.01048,-0.12847},
{-0.07678, 0.04237, 0.00403, 0.00377,-0.00821,-0.00244,-0.09191, 0.12232, 0.09163, 0.04969, 0.01318,-0.02943, 0.01069,-0.12891},
{-0.07624, 0.04231, 0.00376, 0.00361,-0.00779,-0.00407,-0.08945, 0.12241, 0.09116, 0.04939, 0.01304,-0.02968, 0.01087,-0.12932},
{-0.07570, 0.04224, 0.00350, 0.00345,-0.00736,-0.00573,-0.08695, 0.12247, 0.09071, 0.04910, 0.01289,-0.02992, 0.01102,-0.12972},
{-0.07515, 0.04215, 0.00324, 0.00329,-0.00692,-0.00741,-0.08441, 0.12249, 0.09026, 0.04881, 0.01274,-0.03014, 0.01113,-0.13010},
{-0.07459, 0.04206, 0.00299, 0.00312,-0.00648,-0.00911,-0.08182, 0.12247, 0.08982, 0.04852, 0.01259,-0.03033, 0.01121,-0.13045},
{-0.07402, 0.04195, 0.00274, 0.00295,-0.00603,-0.01084,-0.07919, 0.12241, 0.08939, 0.04823, 0.01244,-0.03052, 0.01126,-0.13078},
{-0.07345, 0.04183, 0.00250, 0.00278,-0.00557,-0.01259,-0.07653, 0.12232, 0.08897, 0.04794, 0.01228,-0.03068, 0.01128,-0.13109},
{-0.07287, 0.04170, 0.00226, 0.00260,-0.00510,-0.01436,-0.07382, 0.12220, 0.08855, 0.04765, 0.01212,-0.03082, 0.01126,-0.13137},
{-0.07228, 0.04155, 0.00203, 0.00243,-0.00463,-0.01615,-0.07108, 0.12204, 0.08814, 0.04735, 0.01196,-0.03095, 0.01121,-0.13162},
{-0.07168, 0.04140, 0.00180, 0.00224,-0.00415,-0.01795,-0.06830, 0.12184, 0.08773, 0.04706, 0.01179,-0.03105, 0.01113,-0.13185},
{-0.07107, 0.04123, 0.00158, 0.00206,-0.00366,-0.01978,-0.06549, 0.12160, 0.08733, 0.04676, 0.01162,-0.03114, 0.01101,-0.13206},
{-0.07046, 0.04105, 0.00136, 0.00188,-0.00317,-0.02162,-0.06264, 0.12134, 0.08693, 0.04647, 0.01145,-0.03121, 0.01086,-0.13224},
{-0.06983, 0.04086, 0.00114, 0.00169,-0.00267,-0.02347,-0.05976, 0.12103, 0.08654, 0.04617, 0.01127,-0.03126, 0.01068,-0.13238},
{-0.06920, 0.04065, 0.00093, 0.00150,-0.00216,-0.02534,-0.05686, 0.12070, 0.08615, 0.04586, 0.01109,-0.03129, 0.01047,-0.13251},
{-0.06856, 0.04044, 0.00073, 0.00131,-0.00165,-0.02722,-0.05392, 0.12032, 0.08576, 0.04556, 0.01091,-0.03130, 0.01022,-0.13260},
{-0.06790, 0.04021, 0.00053, 0.00112,-0.00113,-0.02912,-0.05096, 0.11992, 0.08537, 0.04525, 0.01072,-0.03129, 0.00995,-0.13266},
{-0.06724, 0.03998, 0.00033, 0.00092,-0.00061,-0.03102,-0.04797, 0.11947, 0.08499, 0.04494, 0.01053,-0.03127, 0.00964,-0.13270},
{-0.06658, 0.03973, 0.00014, 0.00073,-0.00008,-0.03293,-0.04496, 0.11900, 0.08460, 0.04463, 0.01034,-0.03123, 0.00930,-0.13270},
{-0.06590, 0.03947,-0.00005, 0.00053, 0.00045,-0.03484,-0.04192, 0.11849, 0.08421, 0.04431, 0.01014,-0.03116, 0.00894,-0.13268},
{-0.06521, 0.03920,-0.00023, 0.00033, 0.00099,-0.03676,-0.03887, 0.11795, 0.08383, 0.04400, 0.00994,-0.03108, 0.00854,-0.13263},
{-0.06451, 0.03892,-0.00040, 0.00013, 0.00153,-0.03869,-0.03580, 0.11737, 0.08344, 0.04367, 0.00974,-0.03099, 0.00812,-0.13254},
{-0.06381, 0.03863,-0.00057,-0.00007, 0.00207,-0.04062,-0.03271, 0.11677, 0.08306, 0.04335, 0.00953,-0.03087, 0.00767,-0.13243},
{-0.06309, 0.03833,-0.00074,-0.00027, 0.00261,-0.04255,-0.02961, 0.11613, 0.08267, 0.04302, 0.00932,-0.03074, 0.00719,-0.13228},
{-0.06237, 0.03803,-0.00090,-0.00047, 0.00316,-0.04448,-0.02650, 0.11546, 0.08228, 0.04269, 0.00911,-0.03060, 0.00669,-0.13211},
{-0.06164, 0.03771,-0.00106,-0.00067, 0.00371,-0.04640,-0.02337, 0.11476, 0.08189, 0.04235, 0.00889,-0.03043, 0.00616,-0.13191},
{-0.06090, 0.03738,-0.00121,-0.00087, 0.00426,-0.04833,-0.02024, 0.11403, 0.08150, 0.04202, 0.00868,-0.03025, 0.00560,-0.13167},
{-0.06015, 0.03705,-0.00135,-0.00107, 0.00482,-0.05025,-0.01710, 0.11327, 0.08110, 0.04167, 0.00845,-0.03006, 0.00503,-0.13141},
{-0.05939, 0.03670,-0.00149,-0.00127, 0.00537,-0.05216,-0.01396, 0.11248, 0.08070, 0.04133, 0.00823,-0.02985, 0.00442,-0.13112},
{-0.05863, 0.03635,-0.00163,-0.00147, 0.00593,-0.05407,-0.01081, 0.11167, 0.08030, 0.04098, 0.00800,-0.02962, 0.00380,-0.13080},
{-0.05785, 0.03599,-0.00176,-0.00167, 0.00649,-0.05596,-0.00767, 0.11082, 0.07990, 0.04063, 0.00777,-0.02938, 0.00315,-0.13045},
{-0.05707, 0.03562,-0.00189,-0.00187, 0.00704,-0.05785,-0.00452, 0.10995, 0.07949, 0.04027, 0.00753,-0.02913, 0.00249,-0.13007},
{-0.05629, 0.03525,-0.00201,-0.00207, 0.00760,-0.05972,-0.00138, 0.10906, 0.07908, 0.03991, 0.00730,-0.02886, 0.00180,-0.12966},
{-0.05549, 0.03487,-0.00212,-0.00226, 0.00815,-0.06158, 0.00175, 0.10814, 0.07867, 0.03955, 0.00706,-0.02858, 0.00109,-0.12923},
{-0.05469, 0.03448,-0.00223,-0.00246, 0.00870,-0.06342, 0.00487, 0.10719, 0.07826, 0.03918, 0.00681,-0.02829, 0.00037,-0.12877},
{-0.05389, 0.03409,-0.00233,-0.00265, 0.00924,-0.06524, 0.00798, 0.10623, 0.07784, 0.03881, 0.00657,-0.02798,-0.00037,-0.12829},
{-0.05308, 0.03369,-0.00243,-0.00284, 0.00978,-0.06704, 0.01108, 0.10524, 0.07742, 0.03844, 0.00632,-0.02766,-0.00113,-0.12779},
{-0.05226, 0.03328,-0.00253,-0.00302, 0.01031,-0.06880, 0.01415, 0.10424, 0.07699, 0.03807, 0.00606,-0.02733,-0.00191,-0.12726},
{-0.05144, 0.03287,-0.00262,-0.00320, 0.01083,-0.07054, 0.01720, 0.10322, 0.07656, 0.03770, 0.00580,-0.02698,-0.00269,-0.12671},
{-0.05062, 0.03246,-0.00271,-0.00338, 0.01134,-0.07224, 0.02023, 0.10218, 0.07613, 0.03733, 0.00554,-0.02662,-0.00350,-0.12614},
{-0.04980, 0.03204,-0.00279,-0.00356, 0.01183,-0.07390, 0.02323, 0.10113, 0.07570, 0.03695, 0.00527,-0.02625,-0.00432,-0.12555},
{-0.04897, 0.03162,-0.00286,-0.00372, 0.01231,-0.07552, 0.02619, 0.10007, 0.07527, 0.03658, 0.00499,-0.02586,-0.00515,-0.12494},
{-0.04815, 0.03119,-0.00293,-0.00389, 0.01277,-0.07709, 0.02912, 0.09900, 0.07483, 0.03620, 0.00471,-0.02545,-0.00599,-0.12432},
{-0.04732, 0.03077,-0.00300,-0.00404, 0.01321,-0.07861, 0.03202, 0.09792, 0.07439, 0.03583, 0.00441,-0.02503,-0.00685,-0.12369},
{-0.04650, 0.03034,-0.00307,-0.00419, 0.01362,-0.08007, 0.03487, 0.09684, 0.07395, 0.03546, 0.00411,-0.02460,-0.00772,-0.12305},
{-0.04567, 0.02990,-0.00313,-0.00433, 0.01402,-0.08148, 0.03767, 0.09575, 0.07351, 0.03510, 0.00381,-0.02415,-0.00860,-0.12240},
{-0.04485, 0.02947,-0.00318,-0.00446, 0.01438,-0.08283, 0.04043, 0.09466, 0.07307, 0.03473, 0.00349,-0.02368,-0.00949,-0.12174},
{-0.04404, 0.02903,-0.00324,-0.00459, 0.01472,-0.08411, 0.04315, 0.09357, 0.07263, 0.03437, 0.00316,-0.02320,-0.01039,-0.12107},
{-0.04323, 0.02860,-0.00329,-0.00471, 0.01503,-0.08532, 0.04581, 0.09249, 0.07219, 0.03402, 0.00282,-0.02270,-0.01131,-0.12040},
{-0.04242, 0.02816,-0.00334,-0.00481, 0.01530,-0.08646, 0.04841, 0.09140, 0.07175, 0.03367, 0.00248,-0.02218,-0.01223,-0.11973},
{-0.04162, 0.02773,-0.00338,-0.00491, 0.01555,-0.08753, 0.05097, 0.09032, 0.07131, 0.03332, 0.00212,-0.02165,-0.01316,-0.11906},
{-0.04082, 0.02729,-0.00342,-0.00500, 0.01575,-0.08853, 0.05346, 0.08925, 0.07088, 0.03298, 0.00174,-0.02110,-0.01410,-0.11839},
{-0.04003, 0.02685,-0.00346,-0.00508, 0.01593,-0.08945, 0.05590, 0.08819, 0.07044, 0.03265, 0.00136,-0.02053,-0.01505,-0.11772},
{-0.03925, 0.02642,-0.00350,-0.00514, 0.01606,-0.09029, 0.05828, 0.08713, 0.07001, 0.03233, 0.00096,-0.01994,-0.01601,-0.11706},
{-0.03848, 0.02598,-0.00353,-0.00520, 0.01616,-0.09105, 0.06060, 0.08609, 0.06958, 0.03201, 0.00055,-0.01933,-0.01698,-0.11640},
{-0.03772, 0.02555,-0.00357,-0.00525, 0.01621,-0.09173, 0.06286, 0.08506, 0.06915, 0.03170, 0.00013,-0.01871,-0.01795,-0.11575},
{-0.03696, 0.02512,-0.00360,-0.00528, 0.01623,-0.09232, 0.06506, 0.08404, 0.06873, 0.03140,-0.00031,-0.01807,-0.01893,-0.11510},
{-0.03621, 0.02469,-0.00363,-0.00530, 0.01620,-0.09284, 0.06720, 0.08303, 0.06831, 0.03110,-0.00076,-0.01741,-0.01992,-0.11447},
{-0.03548, 0.02426,-0.00366,-0.00532, 0.01614,-0.09327, 0.06928, 0.08204, 0.06789, 0.03082,-0.00122,-0.01673,-0.02092,-0.11384},
{-0.03475, 0.02383,-0.00368,-0.00532, 0.01603,-0.09361, 0.07130, 0.08107, 0.06748, 0.03054,-0.00169,-0.01604,-0.02192,-0.11323},
{-0.03403, 0.02341,-0.00371,-0.00531, 0.01588,-0.09387, 0.07325, 0.08011, 0.06707, 0.03027,-0.00218,-0.01533,-0.02293,-0.11263},
{-0.03333, 0.02298,-0.00373,-0.00528, 0.01568,-0.09405, 0.07515, 0.07917, 0.06666, 0.03001,-0.00268,-0.01461,-0.02394,-0.11204},
{-0.03263, 0.02256,-0.00376,-0.00525, 0.01545,-0.09414, 0.07698, 0.07825, 0.06626, 0.02976,-0.00318,-0.01387,-0.02496,-0.11146},
{-0.03194, 0.02214,-0.00378,-0.00520, 0.01517,-0.09415, 0.07876, 0.07734, 0.06587, 0.02951,-0.00370,-0.01313,-0.02598,-0.11090},
{-0.03126, 0.02173,-0.00381,-0.00514, 0.01484,-0.09408, 0.08048, 0.07645, 0.06548, 0.02928,-0.00423,-0.01238,-0.02700,-0.11035},
{-0.03060, 0.02131,-0.00383,-0.00507, 0.01447,-0.09393, 0.08214, 0.07558, 0.06509, 0.02905,-0.00476,-0.01162,-0.02802,-0.10982},
{-0.02994, 0.02090,-0.00385,-0.00499, 0.01406,-0.09369, 0.08374, 0.07472, 0.06471, 0.02883,-0.00531,-0.01085,-0.02904,-0.10930},
{-0.02929, 0.02049,-0.00388,-0.00489, 0.01361,-0.09337, 0.08529, 0.07388, 0.06433, 0.02862,-0.00585,-0.01009,-0.03005,-0.10880},
{-0.02866, 0.02008,-0.00390,-0.00479, 0.01312,-0.09298, 0.08678, 0.07307, 0.06396, 0.02841,-0.00640,-0.00932,-0.03107,-0.10831},
{-0.02803, 0.01968,-0.00393,-0.00467, 0.01258,-0.09250, 0.08822, 0.07226, 0.06359, 0.02822,-0.00696,-0.00855,-0.03208,-0.10784},
{-0.02742, 0.01928,-0.00395,-0.00453, 0.01200,-0.09195, 0.08960, 0.07148, 0.06323, 0.02802,-0.00751,-0.00778,-0.03308,-0.10738},
{-0.02681, 0.01888,-0.00398,-0.00439, 0.01138,-0.09132, 0.09093, 0.07071, 0.06287, 0.02784,-0.00807,-0.00702,-0.03408,-0.10695},
{-0.02621, 0.01848,-0.00401,-0.00423, 0.01071,-0.09062, 0.09221, 0.06996, 0.06252, 0.02766,-0.00863,-0.00627,-0.03506,-0.10652},
{-0.02562, 0.01808,-0.00403,-0.00406, 0.01001,-0.08984, 0.09344, 0.06923, 0.06217, 0.02749,-0.00918,-0.00553,-0.03604,-0.10612},
{-0.02505, 0.01769,-0.00406,-0.00388, 0.00927,-0.08899, 0.09462, 0.06851, 0.06183, 0.02732,-0.00973,-0.00479,-0.03701,-0.10573},
{-0.02448, 0.01730,-0.00409,-0.00369, 0.00848,-0.08807, 0.09576, 0.06781, 0.06149, 0.02715,-0.01028,-0.00407,-0.03796,-0.10535},
{-0.02391, 0.01691,-0.00412,-0.00348, 0.00766,-0.08707, 0.09684, 0.06713, 0.06116, 0.02699,-0.01082,-0.00337,-0.03891,-0.10500},
{-0.02336, 0.01652,-0.00415,-0.00326, 0.00680,-0.08601, 0.09788, 0.06646, 0.06083, 0.02684,-0.01136,-0.00268,-0.03984,-0.10465},
{-0.02282, 0.01613,-0.00419,-0.00303, 0.00590,-0.08488, 0.09887, 0.06580, 0.06050, 0.02669,-0.01189,-0.00200,-0.04075,-0.10433},
{-0.02228, 0.01575,-0.00422,-0.00279, 0.00496,-0.08369, 0.09982, 0.06516, 0.06018, 0.02654,-0.01241,-0.00135,-0.04165,-0.10402},
{-0.02175, 0.01537,-0.00426,-0.00253, 0.00399,-0.08243, 0.10072, 0.06454, 0.05986, 0.02640,-0.01292,-0.00072,-0.04253,-0.10372},
{-0.02123, 0.01499,-0.00430,-0.00227, 0.00298,-0.08111, 0.10158, 0.06393, 0.05955, 0.02625,-0.01343,-0.00011,-0.04340,-0.10344},
{-0.02072, 0.01461,-0.00434,-0.00199, 0.00193,-0.07972, 0.10240, 0.06333, 0.05923, 0.02612,-0.01392, 0.00048,-0.04424,-0.10317},
{-0.02022, 0.01424,-0.00438,-0.00170, 0.00085,-0.07827, 0.10317, 0.06274, 0.05893, 0.02598,-0.01440, 0.00104,-0.04507,-0.10291},
{-0.01972, 0.01386,-0.00442,-0.00140,-0.00026,-0.07677, 0.10390, 0.06217, 0.05862, 0.02584,-0.01487, 0.00158,-0.04588,-0.10267},
{-0.01923, 0.01349,-0.00446,-0.00109,-0.00141,-0.07520, 0.10460, 0.06161, 0.05832, 0.02571,-0.01532, 0.00209,-0.04666,-0.10244},
{-0.01874, 0.01312,-0.00451,-0.00076,-0.00259,-0.07358, 0.10525, 0.06106, 0.05802, 0.02558,-0.01576, 0.00257,-0.04743,-0.10223},
{-0.01826, 0.01276,-0.00456,-0.00043,-0.00380,-0.07190, 0.10586, 0.06052, 0.05772, 0.02545,-0.01619, 0.00303,-0.04817,-0.10202},
{-0.01779, 0.01239,-0.00461,-0.00008,-0.00504,-0.07016, 0.10643, 0.05999, 0.05743, 0.02532,-0.01660, 0.00345,-0.04889,-0.10183},
{-0.01732, 0.01203,-0.00466, 0.00027,-0.00632,-0.06838, 0.10697, 0.05947, 0.05713, 0.02519,-0.01700, 0.00385,-0.04958,-0.10165},
{-0.01686, 0.01166,-0.00471, 0.00064,-0.00762,-0.06654, 0.10747, 0.05897, 0.05684, 0.02506,-0.01738, 0.00421,-0.05026,-0.10148},
{-0.01641, 0.01130,-0.00476, 0.00101,-0.00894,-0.06465, 0.10792, 0.05847, 0.05655, 0.02492,-0.01774, 0.00454,-0.05090,-0.10132},
{-0.01596, 0.01094,-0.00482, 0.00140,-0.01030,-0.06271, 0.10835, 0.05798, 0.05626, 0.02479,-0.01809, 0.00484,-0.05153,-0.10116},
{-0.01552, 0.01059,-0.00487, 0.00180,-0.01168,-0.06072, 0.10873, 0.05750, 0.05598, 0.02466,-0.01842, 0.00511,-0.05213,-0.10102},
{-0.01508, 0.01023,-0.00493, 0.00220,-0.01309,-0.05869, 0.10908, 0.05702, 0.05569, 0.02453,-0.01874, 0.00534,-0.05270,-0.10088},
{-0.01464, 0.00988,-0.00499, 0.00262,-0.01452,-0.05661, 0.10939, 0.05656, 0.05540, 0.02439,-0.01903, 0.00555,-0.05324,-0.10075},
{-0.01421, 0.00953,-0.00505, 0.00304,-0.01597,-0.05448, 0.10967, 0.05610, 0.05511, 0.02426,-0.01931, 0.00572,-0.05377,-0.10063},
{-0.01379, 0.00918,-0.00511, 0.00347,-0.01745,-0.05232, 0.10991, 0.05564, 0.05483, 0.02412,-0.01957, 0.00585,-0.05426,-0.10052},
{-0.01336, 0.00883,-0.00517, 0.00392,-0.01895,-0.05011, 0.11011, 0.05520, 0.05454, 0.02398,-0.01981, 0.00595,-0.05473,-0.10041},
{-0.01295, 0.00848,-0.00523, 0.00436,-0.02046,-0.04787, 0.11029, 0.05476, 0.05425, 0.02384,-0.02003, 0.00602,-0.05516,-0.10030},
{-0.01253, 0.00814,-0.00529, 0.00482,-0.02200,-0.04558, 0.11042, 0.05432, 0.05397, 0.02370,-0.02024, 0.00605,-0.05558,-0.10020},
{-0.01212, 0.00780,-0.00536, 0.00529,-0.02355,-0.04326, 0.11052, 0.05389, 0.05368, 0.02355,-0.02042, 0.00605,-0.05596,-0.10011},
{-0.01172, 0.00746,-0.00542, 0.00576,-0.02512,-0.04091, 0.11059, 0.05347, 0.05339, 0.02341,-0.02059, 0.00602,-0.05632,-0.10002},
{-0.01132, 0.00712,-0.00549, 0.00624,-0.02670,-0.03852, 0.11062, 0.05305, 0.05309, 0.02326,-0.02074, 0.00595,-0.05665,-0.09993},
{-0.01092, 0.00678,-0.00555, 0.00672,-0.02830,-0.03610, 0.11062, 0.05263, 0.05280, 0.02310,-0.02087, 0.00586,-0.05695,-0.09984},
{-0.01052, 0.00645,-0.00562, 0.00722,-0.02991,-0.03365, 0.11059, 0.05222, 0.05251, 0.02295,-0.02098, 0.00572,-0.05723,-0.09975},
{-0.01013, 0.00612,-0.00568, 0.00771,-0.03153,-0.03117, 0.11053, 0.05181, 0.05221, 0.02279,-0.02107, 0.00556,-0.05747,-0.09967},
{-0.00974, 0.00579,-0.00575, 0.00822,-0.03317,-0.02867, 0.11043, 0.05141, 0.05191, 0.02262,-0.02115, 0.00536,-0.05769,-0.09958},
{-0.00935, 0.00547,-0.00581, 0.00873,-0.03481,-0.02614, 0.11030, 0.05100, 0.05161, 0.02246,-0.02120, 0.00513,-0.05788,-0.09950},
{-0.00897, 0.00514,-0.00588, 0.00924,-0.03645,-0.02359, 0.11014, 0.05060, 0.05130, 0.02229,-0.02124, 0.00487,-0.05805,-0.09941},
{-0.00858, 0.00482,-0.00594, 0.00976,-0.03811,-0.02102, 0.10994, 0.05021, 0.05100, 0.02211,-0.02126, 0.00458,-0.05818,-0.09933},
{-0.00821, 0.00451,-0.00601, 0.01028,-0.03977,-0.01843, 0.10972, 0.04981, 0.05069, 0.02194,-0.02126, 0.00426,-0.05829,-0.09924},
{-0.00783, 0.00419,-0.00607, 0.01081,-0.04143,-0.01582, 0.10946, 0.04942, 0.05037, 0.02176,-0.02125, 0.00391,-0.05838,-0.09915},
{-0.00745, 0.00388,-0.00614, 0.01133,-0.04309,-0.01320, 0.10918, 0.04903, 0.05006, 0.02157,-0.02121, 0.00353,-0.05843,-0.09906},
{-0.00708, 0.00357,-0.00620, 0.01187,-0.04476,-0.01056, 0.10887, 0.04864, 0.04974, 0.02139,-0.02117, 0.00313,-0.05846,-0.09896},
{-0.00671, 0.00326,-0.00626, 0.01240,-0.04643,-0.00791, 0.10852, 0.04825, 0.04942, 0.02119,-0.02110, 0.00269,-0.05846,-0.09886},
{-0.00634, 0.00296,-0.00632, 0.01294,-0.04809,-0.00526, 0.10815, 0.04786, 0.04909, 0.02100,-0.02102, 0.00223,-0.05844,-0.09876},
{-0.00597, 0.00266,-0.00638, 0.01347,-0.04975,-0.00259, 0.10775, 0.04747, 0.04877, 0.02080,-0.02092, 0.00174,-0.05839,-0.09866},
{-0.00561, 0.00237,-0.00644, 0.01401,-0.05141, 0.00008, 0.10732, 0.04709, 0.04843, 0.02060,-0.02081, 0.00123,-0.05832,-0.09855},
{-0.00525, 0.00208,-0.00650, 0.01455,-0.05306, 0.00276, 0.10687, 0.04670, 0.04810, 0.02039,-0.02068, 0.00069,-0.05822,-0.09843},
{-0.00489, 0.00179,-0.00656, 0.01509,-0.05470, 0.00544, 0.10638, 0.04632, 0.04776, 0.02018,-0.02054, 0.00013,-0.05810,-0.09831},
{-0.00453, 0.00151,-0.00661, 0.01563,-0.05634, 0.00812, 0.10588, 0.04593, 0.04742, 0.01996,-0.02038,-0.00046,-0.05796,-0.09819},
{-0.00417, 0.00123,-0.00666, 0.01618,-0.05796, 0.01080, 0.10535, 0.04555, 0.04708, 0.01975,-0.02021,-0.00107,-0.05779,-0.09806},
{-0.00381, 0.00095,-0.00671, 0.01671,-0.05958, 0.01348, 0.10479, 0.04517, 0.04673, 0.01952,-0.02003,-0.00169,-0.05760,-0.09792},
{-0.00346, 0.00068,-0.00676, 0.01725,-0.06118, 0.01615, 0.10421, 0.04478, 0.04638, 0.01930,-0.01983,-0.00234,-0.05739,-0.09778},
{-0.00310, 0.00041,-0.00681, 0.01779,-0.06277, 0.01881, 0.10361, 0.04440, 0.04602, 0.01907,-0.01962,-0.00301,-0.05715,-0.09764},
{-0.00275, 0.00015,-0.00685, 0.01833,-0.06435, 0.02147, 0.10298, 0.04402, 0.04566, 0.01883,-0.01940,-0.00370,-0.05690,-0.09748},
{-0.00240,-0.00011,-0.00689, 0.01886,-0.06591, 0.02411, 0.10233, 0.04364, 0.04530, 0.01860,-0.01917,-0.00441,-0.05662,-0.09732},
{-0.00205,-0.00037,-0.00693, 0.01939,-0.06746, 0.02674, 0.10167, 0.04325, 0.04494, 0.01836,-0.01893,-0.00513,-0.05633,-0.09716},
{-0.00170,-0.00062,-0.00697, 0.01992,-0.06898, 0.02936, 0.10098, 0.04287, 0.04457, 0.01811,-0.01867,-0.00587,-0.05601,-0.09699},
{-0.00135,-0.00086,-0.00700, 0.02044,-0.07049, 0.03196, 0.10028, 0.04249, 0.04420, 0.01786,-0.01841,-0.00662,-0.05568,-0.09681},
{-0.00101,-0.00110,-0.00704, 0.02095,-0.07197, 0.03455, 0.09955, 0.04211, 0.04383, 0.01761,-0.01813,-0.00739,-0.05533,-0.09663},
{-0.00066,-0.00134,-0.00706, 0.02146,-0.07342, 0.03711, 0.09882, 0.04172, 0.04346, 0.01735,-0.01785,-0.00817,-0.05497,-0.09644},
{-0.00032,-0.00157,-0.00709, 0.02196,-0.07485, 0.03964, 0.09806, 0.04134, 0.04308, 0.01709,-0.01755,-0.00897,-0.05458,-0.09624},
{ 0.00002,-0.00180,-0.00711, 0.02244,-0.07624, 0.04215, 0.09730, 0.04096, 0.04271, 0.01683,-0.01724,-0.00978,-0.05419,-0.09604},
{ 0.00036,-0.00202,-0.00712, 0.02292,-0.07760, 0.04463, 0.09652, 0.04058, 0.04233, 0.01655,-0.01692,-0.01060,-0.05378,-0.09584},
{ 0.00069,-0.00224,-0.00713, 0.02338,-0.07892, 0.04708, 0.09574, 0.04020, 0.04195, 0.01627,-0.01659,-0.01143,-0.05336,-0.09564},
{ 0.00102,-0.00245,-0.00714, 0.02382,-0.08019, 0.04949, 0.09494, 0.03982, 0.04158, 0.01599,-0.01625,-0.01227,-0.05293,-0.09543},
{ 0.00135,-0.00266,-0.00714, 0.02425,-0.08142, 0.05187, 0.09414, 0.03944, 0.04120, 0.01570,-0.01590,-0.01312,-0.05249,-0.09522},
{ 0.00168,-0.00287,-0.00713, 0.02466,-0.08261, 0.05421, 0.09334, 0.03906, 0.04083, 0.01540,-0.01553,-0.01399,-0.05205,-0.09500},
{ 0.00200,-0.00307,-0.00712, 0.02505,-0.08374, 0.05651, 0.09253, 0.03868, 0.04046, 0.01509,-0.01515,-0.01486,-0.05159,-0.09479},
{ 0.00232,-0.00327,-0.00710, 0.02541,-0.08482, 0.05877, 0.09172, 0.03830, 0.04009, 0.01478,-0.01476,-0.01574,-0.05113,-0.09458},
{ 0.00264,-0.00346,-0.00707, 0.02575,-0.08584, 0.06098, 0.09090, 0.03792, 0.03972, 0.01446,-0.01435,-0.01663,-0.05066,-0.09436},
{ 0.00295,-0.00365,-0.00704, 0.02607,-0.08681, 0.06314, 0.09009, 0.03755, 0.03936, 0.01412,-0.01393,-0.01753,-0.05019,-0.09415},
{ 0.00325,-0.00383,-0.00699, 0.02636,-0.08771, 0.06526, 0.08929, 0.03718, 0.03900, 0.01378,-0.01350,-0.01843,-0.04971,-0.09394},
{ 0.00356,-0.00401,-0.00695, 0.02662,-0.08855, 0.06733, 0.08848, 0.03681, 0.03865, 0.01343,-0.01305,-0.01935,-0.04924,-0.09374},
{ 0.00386,-0.00419,-0.00689, 0.02685,-0.08933, 0.06935, 0.08769, 0.03644, 0.03830, 0.01307,-0.01259,-0.02027,-0.04876,-0.09353},
{ 0.00415,-0.00436,-0.00682, 0.02705,-0.09004, 0.07132, 0.08689, 0.03607, 0.03796, 0.01270,-0.01211,-0.02120,-0.04828,-0.09334},
{ 0.00444,-0.00454,-0.00675, 0.02722,-0.09068, 0.07324, 0.08611, 0.03570, 0.03762, 0.01232,-0.01162,-0.02213,-0.04780,-0.09314},
{ 0.00472,-0.00470,-0.00667, 0.02735,-0.09125, 0.07511, 0.08534, 0.03534, 0.03729, 0.01193,-0.01111,-0.02308,-0.04732,-0.09296},
{ 0.00500,-0.00487,-0.00657, 0.02746,-0.09175, 0.07692, 0.08457, 0.03498, 0.03697, 0.01153,-0.01058,-0.02403,-0.04685,-0.09278},
{ 0.00527,-0.00503,-0.00647, 0.02753,-0.09218, 0.07869, 0.08382, 0.03462, 0.03666, 0.01111,-0.01004,-0.02498,-0.04638,-0.09260},
{ 0.00554,-0.00519,-0.00637, 0.02756,-0.09254, 0.08040, 0.08308, 0.03426, 0.03635, 0.01069,-0.00949,-0.02595,-0.04591,-0.09244},
{ 0.00581,-0.00535,-0.00625, 0.02756,-0.09283, 0.08206, 0.08235, 0.03390, 0.03605, 0.01025,-0.00892,-0.02692,-0.04544,-0.09228},
{ 0.00606,-0.00550,-0.00612, 0.02752,-0.09304, 0.08366, 0.08164, 0.03355, 0.03575, 0.00981,-0.00833,-0.02789,-0.04498,-0.09213},
{ 0.00632,-0.00565,-0.00598, 0.02745,-0.09319, 0.08521, 0.08093, 0.03320, 0.03547, 0.00935,-0.00773,-0.02887,-0.04452,-0.09199},
{ 0.00656,-0.00580,-0.00584, 0.02734,-0.09325, 0.08672, 0.08025, 0.03285, 0.03519, 0.00888,-0.00712,-0.02986,-0.04407,-0.09185},
{ 0.00681,-0.00595,-0.00568, 0.02719,-0.09325, 0.08817, 0.07957, 0.03251, 0.03492, 0.00841,-0.00649,-0.03085,-0.04363,-0.09173},
{ 0.00704,-0.00610,-0.00551, 0.02701,-0.09318, 0.08957, 0.07891, 0.03217, 0.03466, 0.00792,-0.00586,-0.03184,-0.04319,-0.09162},
{ 0.00727,-0.00624,-0.00534, 0.02679,-0.09303, 0.09092, 0.07827, 0.03183, 0.03441, 0.00743,-0.00521,-0.03283,-0.04276,-0.09151},
{ 0.00750,-0.00639,-0.00515, 0.02654,-0.09281, 0.09222, 0.07764, 0.03149, 0.03416, 0.00693,-0.00456,-0.03382,-0.04234,-0.09142},
{ 0.00772,-0.00653,-0.00496, 0.02625,-0.09252, 0.09348, 0.07703, 0.03116, 0.03392, 0.00643,-0.00390,-0.03482,-0.04192,-0.09133},
{ 0.00794,-0.00667,-0.00475, 0.02592,-0.09216, 0.09468, 0.07643, 0.03083, 0.03369, 0.00592,-0.00324,-0.03581,-0.04152,-0.09125},
{ 0.00815,-0.00681,-0.00454, 0.02555,-0.09174, 0.09585, 0.07585, 0.03050, 0.03346, 0.00541,-0.00258,-0.03680,-0.04112,-0.09118},
{ 0.00835,-0.00695,-0.00431, 0.02515,-0.09124, 0.09696, 0.07528, 0.03018, 0.03324, 0.00489,-0.00191,-0.03779,-0.04073,-0.09112},
{ 0.00855,-0.00709,-0.00408, 0.02472,-0.09068, 0.09803, 0.07473, 0.02986, 0.03303, 0.00437,-0.00124,-0.03877,-0.04035,-0.09107},
{ 0.00875,-0.00723,-0.00383, 0.02425,-0.09005, 0.09906, 0.07419, 0.02954, 0.03282, 0.00385,-0.00058,-0.03975,-0.03997,-0.09103},
{ 0.00894,-0.00737,-0.00357, 0.02374,-0.08936, 0.10004, 0.07366, 0.02922, 0.03262, 0.00333, 0.00008,-0.04073,-0.03961,-0.09100},
{ 0.00913,-0.00751,-0.00331, 0.02320,-0.08860, 0.10099, 0.07315, 0.02891, 0.03242, 0.00281, 0.00073,-0.04169,-0.03926,-0.09098},
{ 0.00931,-0.00765,-0.00303, 0.02262,-0.08778, 0.10189, 0.07266, 0.02860, 0.03223, 0.00230, 0.00138,-0.04265,-0.03891,-0.09096},
{ 0.00948,-0.00779,-0.00274, 0.02201,-0.08690, 0.10275, 0.07218, 0.02829, 0.03205, 0.00178, 0.00202,-0.04360,-0.03858,-0.09095},
{ 0.00965,-0.00793,-0.00245, 0.02137,-0.08596, 0.10357, 0.07171, 0.02799, 0.03187, 0.00127, 0.00264,-0.04453,-0.03825,-0.09095}
  };
  //
  double par_fg33[192][14] = {
{ 0.27453, 0.06457, 0.11335, 0.10418, 0.10740, 0.05084, 0.25847, 0.03389,-0.00485,-0.01540,-0.01259,-0.02301,-0.00973, 0.05833},
{ 0.27495, 0.06428, 0.11366, 0.10428, 0.10739, 0.05132, 0.25845, 0.03269,-0.00488,-0.01555,-0.01271,-0.02273,-0.01031, 0.05917},
{ 0.27538, 0.06399, 0.11397, 0.10438, 0.10737, 0.05182, 0.25839, 0.03150,-0.00492,-0.01571,-0.01284,-0.02246,-0.01089, 0.06002},
{ 0.27580, 0.06370, 0.11428, 0.10449, 0.10735, 0.05236, 0.25829, 0.03032,-0.00498,-0.01588,-0.01297,-0.02218,-0.01146, 0.06088},
{ 0.27623, 0.06342, 0.11458, 0.10460, 0.10733, 0.05292, 0.25815, 0.02916,-0.00506,-0.01606,-0.01310,-0.02190,-0.01203, 0.06176},
{ 0.27666, 0.06315, 0.11488, 0.10471, 0.10729, 0.05352, 0.25797, 0.02802,-0.00515,-0.01624,-0.01323,-0.02162,-0.01260, 0.06264},
{ 0.27709, 0.06288, 0.11518, 0.10482, 0.10725, 0.05415, 0.25774, 0.02689,-0.00526,-0.01642,-0.01337,-0.02135,-0.01315, 0.06354},
{ 0.27751, 0.06262, 0.11548, 0.10494, 0.10720, 0.05481, 0.25747, 0.02577,-0.00539,-0.01661,-0.01350,-0.02107,-0.01370, 0.06445},
{ 0.27794, 0.06237, 0.11578, 0.10506, 0.10715, 0.05551, 0.25716, 0.02467,-0.00554,-0.01681,-0.01364,-0.02080,-0.01423, 0.06537},
{ 0.27837, 0.06212, 0.11608, 0.10518, 0.10708, 0.05624, 0.25681, 0.02358,-0.00569,-0.01700,-0.01377,-0.02052,-0.01476, 0.06630},
{ 0.27880, 0.06187, 0.11637, 0.10531, 0.10701, 0.05700, 0.25641, 0.02251,-0.00586,-0.01721,-0.01391,-0.02026,-0.01528, 0.06723},
{ 0.27923, 0.06164, 0.11666, 0.10544, 0.10693, 0.05779, 0.25597, 0.02145,-0.00605,-0.01742,-0.01404,-0.01999,-0.01578, 0.06817},
{ 0.27966, 0.06140, 0.11695, 0.10557, 0.10684, 0.05862, 0.25548, 0.02040,-0.00625,-0.01763,-0.01418,-0.01973,-0.01627, 0.06912},
{ 0.28008, 0.06118, 0.11725, 0.10571, 0.10675, 0.05948, 0.25495, 0.01937,-0.00646,-0.01784,-0.01431,-0.01947,-0.01675, 0.07007},
{ 0.28050, 0.06096, 0.11754, 0.10585, 0.10665, 0.06037, 0.25436, 0.01836,-0.00668,-0.01806,-0.01444,-0.01922,-0.01721, 0.07102},
{ 0.28092, 0.06075, 0.11783, 0.10600, 0.10653, 0.06130, 0.25374, 0.01736,-0.00691,-0.01828,-0.01458,-0.01898,-0.01766, 0.07198},
{ 0.28134, 0.06055, 0.11811, 0.10614, 0.10641, 0.06226, 0.25306, 0.01638,-0.00716,-0.01850,-0.01471,-0.01874,-0.01809, 0.07293},
{ 0.28176, 0.06035, 0.11840, 0.10629, 0.10628, 0.06326, 0.25233, 0.01542,-0.00741,-0.01872,-0.01484,-0.01851,-0.01850, 0.07389},
{ 0.28217, 0.06016, 0.11869, 0.10645, 0.10614, 0.06429, 0.25156, 0.01447,-0.00767,-0.01894,-0.01497,-0.01828,-0.01890, 0.07484},
{ 0.28257, 0.05997, 0.11898, 0.10661, 0.10599, 0.06535, 0.25074, 0.01354,-0.00795,-0.01917,-0.01510,-0.01807,-0.01928, 0.07580},
{ 0.28297, 0.05980, 0.11926, 0.10677, 0.10584, 0.06645, 0.24987, 0.01263,-0.00822,-0.01940,-0.01523,-0.01786,-0.01964, 0.07675},
{ 0.28337, 0.05963, 0.11955, 0.10694, 0.10567, 0.06759, 0.24894, 0.01174,-0.00851,-0.01963,-0.01535,-0.01766,-0.01998, 0.07769},
{ 0.28376, 0.05947, 0.11983, 0.10711, 0.10550, 0.06876, 0.24797, 0.01087,-0.00880,-0.01985,-0.01548,-0.01747,-0.02030, 0.07863},
{ 0.28415, 0.05932, 0.12012, 0.10729, 0.10531, 0.06996, 0.24695, 0.01002,-0.00910,-0.02008,-0.01560,-0.01729,-0.02060, 0.07957},
{ 0.28453, 0.05918, 0.12040, 0.10747, 0.10512, 0.07120, 0.24587, 0.00919,-0.00941,-0.02031,-0.01572,-0.01712,-0.02088, 0.08049},
{ 0.28490, 0.05904, 0.12069, 0.10765, 0.10491, 0.07247, 0.24475, 0.00838,-0.00972,-0.02054,-0.01584,-0.01696,-0.02114, 0.08141},
{ 0.28526, 0.05892, 0.12097, 0.10784, 0.10470, 0.07377, 0.24357, 0.00759,-0.01003,-0.02076,-0.01595,-0.01682,-0.02137, 0.08232},
{ 0.28562, 0.05880, 0.12125, 0.10803, 0.10448, 0.07511, 0.24234, 0.00683,-0.01035,-0.02099,-0.01606,-0.01668,-0.02159, 0.08321},
{ 0.28597, 0.05869, 0.12154, 0.10822, 0.10425, 0.07648, 0.24106, 0.00608,-0.01067,-0.02121,-0.01617,-0.01656,-0.02178, 0.08410},
{ 0.28631, 0.05859, 0.12182, 0.10842, 0.10400, 0.07788, 0.23973, 0.00537,-0.01099,-0.02143,-0.01628,-0.01644,-0.02194, 0.08497},
{ 0.28664, 0.05850, 0.12210, 0.10863, 0.10375, 0.07931, 0.23835, 0.00467,-0.01131,-0.02165,-0.01638,-0.01635,-0.02209, 0.08583},
{ 0.28696, 0.05843, 0.12238, 0.10883, 0.10349, 0.08077, 0.23691, 0.00401,-0.01164,-0.02187,-0.01648,-0.01626,-0.02221, 0.08667},
{ 0.28727, 0.05836, 0.12266, 0.10904, 0.10322, 0.08227, 0.23543, 0.00337,-0.01197,-0.02208,-0.01658,-0.01619,-0.02230, 0.08749},
{ 0.28757, 0.05830, 0.12294, 0.10926, 0.10294, 0.08379, 0.23390, 0.00275,-0.01229,-0.02229,-0.01667,-0.01613,-0.02237, 0.08830},
{ 0.28787, 0.05825, 0.12322, 0.10948, 0.10266, 0.08535, 0.23231, 0.00216,-0.01262,-0.02250,-0.01676,-0.01608,-0.02242, 0.08909},
{ 0.28815, 0.05821, 0.12350, 0.10970, 0.10236, 0.08693, 0.23068, 0.00160,-0.01294,-0.02270,-0.01685,-0.01605,-0.02244, 0.08986},
{ 0.28842, 0.05818, 0.12378, 0.10993, 0.10206, 0.08854, 0.22900, 0.00107,-0.01327,-0.02290,-0.01693,-0.01603,-0.02244, 0.09061},
{ 0.28868, 0.05816, 0.12405, 0.11016, 0.10174, 0.09017, 0.22727, 0.00056,-0.01359,-0.02310,-0.01701,-0.01603,-0.02241, 0.09135},
{ 0.28892, 0.05816, 0.12433, 0.11039, 0.10142, 0.09184, 0.22550, 0.00009,-0.01391,-0.02329,-0.01709,-0.01604,-0.02236, 0.09205},
{ 0.28916, 0.05816, 0.12460, 0.11063, 0.10109, 0.09352, 0.22368,-0.00036,-0.01423,-0.02348,-0.01716,-0.01607,-0.02228, 0.09274},
{ 0.28938, 0.05818, 0.12487, 0.11087, 0.10075, 0.09523, 0.22181,-0.00077,-0.01454,-0.02366,-0.01723,-0.01611,-0.02217, 0.09340},
{ 0.28959, 0.05820, 0.12515, 0.11111, 0.10040, 0.09696, 0.21990,-0.00116,-0.01485,-0.02384,-0.01730,-0.01617,-0.02205, 0.09404},
{ 0.28979, 0.05824, 0.12542, 0.11136, 0.10005, 0.09872, 0.21795,-0.00152,-0.01515,-0.02401,-0.01736,-0.01624,-0.02189, 0.09466},
{ 0.28998, 0.05828, 0.12568, 0.11160, 0.09968, 0.10049, 0.21596,-0.00184,-0.01545,-0.02418,-0.01741,-0.01633,-0.02171, 0.09524},
{ 0.29015, 0.05834, 0.12595, 0.11186, 0.09932, 0.10228, 0.21393,-0.00214,-0.01575,-0.02434,-0.01747,-0.01643,-0.02151, 0.09581},
{ 0.29031, 0.05841, 0.12622, 0.11211, 0.09894, 0.10409, 0.21186,-0.00240,-0.01604,-0.02450,-0.01752,-0.01655,-0.02128, 0.09634},
{ 0.29046, 0.05849, 0.12648, 0.11237, 0.09856, 0.10592, 0.20975,-0.00263,-0.01632,-0.02465,-0.01756,-0.01668,-0.02103, 0.09685},
{ 0.29059, 0.05858, 0.12674, 0.11263, 0.09817, 0.10776, 0.20761,-0.00284,-0.01660,-0.02479,-0.01760,-0.01683,-0.02076, 0.09733},
{ 0.29071, 0.05868, 0.12700, 0.11289, 0.09777, 0.10962, 0.20543,-0.00301,-0.01687,-0.02493,-0.01764,-0.01699,-0.02046, 0.09779},
{ 0.29082, 0.05879, 0.12726, 0.11315, 0.09737, 0.11148, 0.20323,-0.00315,-0.01714,-0.02506,-0.01767,-0.01717,-0.02014, 0.09821},
{ 0.29092, 0.05891, 0.12751, 0.11342, 0.09697, 0.11336, 0.20099,-0.00326,-0.01739,-0.02519,-0.01770,-0.01736,-0.01979, 0.09861},
{ 0.29100, 0.05905, 0.12777, 0.11368, 0.09656, 0.11525, 0.19872,-0.00333,-0.01764,-0.02531,-0.01773,-0.01757,-0.01943, 0.09898},
{ 0.29107, 0.05919, 0.12802, 0.11395, 0.09614, 0.11714, 0.19643,-0.00338,-0.01789,-0.02542,-0.01775,-0.01779,-0.01904, 0.09932},
{ 0.29113, 0.05934, 0.12827, 0.11422, 0.09573, 0.11904, 0.19412,-0.00340,-0.01812,-0.02553,-0.01777,-0.01803,-0.01863, 0.09964},
{ 0.29117, 0.05950, 0.12851, 0.11449, 0.09531, 0.12094, 0.19178,-0.00339,-0.01835,-0.02563,-0.01778,-0.01828,-0.01820, 0.09992},
{ 0.29121, 0.05968, 0.12876, 0.11476, 0.09489, 0.12284, 0.18943,-0.00335,-0.01857,-0.02573,-0.01779,-0.01854,-0.01775, 0.10018},
{ 0.29123, 0.05986, 0.12900, 0.11503, 0.09447, 0.12474, 0.18706,-0.00328,-0.01878,-0.02582,-0.01779,-0.01882,-0.01728, 0.10041},
{ 0.29124, 0.06005, 0.12924, 0.11530, 0.09405, 0.12663, 0.18467,-0.00319,-0.01899,-0.02590,-0.01779,-0.01912,-0.01679, 0.10061},
{ 0.29124, 0.06024, 0.12947, 0.11557, 0.09363, 0.12851, 0.18228,-0.00308,-0.01919,-0.02599,-0.01779,-0.01943,-0.01628, 0.10079},
{ 0.29124, 0.06045, 0.12970, 0.11583, 0.09323, 0.13038, 0.17989,-0.00294,-0.01938,-0.02606,-0.01777,-0.01976,-0.01575, 0.10095},
{ 0.29122, 0.06066, 0.12994, 0.11610, 0.09282, 0.13222, 0.17749,-0.00277,-0.01956,-0.02614,-0.01775,-0.02010,-0.01520, 0.10108},
{ 0.29119, 0.06089, 0.13016, 0.11636, 0.09243, 0.13405, 0.17509,-0.00259,-0.01974,-0.02621,-0.01772,-0.02046,-0.01464, 0.10119},
{ 0.29116, 0.06112, 0.13039, 0.11662, 0.09205, 0.13585, 0.17269,-0.00239,-0.01991,-0.02628,-0.01769,-0.02084,-0.01406, 0.10128},
{ 0.29112, 0.06135, 0.13061, 0.11688, 0.09168, 0.13762, 0.17030,-0.00217,-0.02007,-0.02634,-0.01765,-0.02123,-0.01346, 0.10135},
{ 0.29108, 0.06159, 0.13084, 0.11713, 0.09133, 0.13936, 0.16792,-0.00194,-0.02023,-0.02641,-0.01759,-0.02164,-0.01284, 0.10141},
{ 0.29103, 0.06184, 0.13106, 0.11737, 0.09100, 0.14106, 0.16556,-0.00169,-0.02039,-0.02647,-0.01753,-0.02207,-0.01221, 0.10145},
{ 0.29098, 0.06210, 0.13127, 0.11762, 0.09068, 0.14272, 0.16321,-0.00143,-0.02054,-0.02653,-0.01746,-0.02252,-0.01157, 0.10147},
{ 0.29092, 0.06236, 0.13149, 0.11785, 0.09039, 0.14434, 0.16087,-0.00116,-0.02068,-0.02659,-0.01738,-0.02299,-0.01090, 0.10149},
{ 0.29086, 0.06262, 0.13170, 0.11808, 0.09011, 0.14591, 0.15856,-0.00088,-0.02082,-0.02666,-0.01729,-0.02347,-0.01022, 0.10149},
{ 0.29080, 0.06289, 0.13192, 0.11831, 0.08986, 0.14744, 0.15627,-0.00060,-0.02096,-0.02672,-0.01719,-0.02398,-0.00953, 0.10149},
{ 0.29074, 0.06316, 0.13213, 0.11853, 0.08964, 0.14891, 0.15401,-0.00031,-0.02109,-0.02679,-0.01707,-0.02450,-0.00882, 0.10147},
{ 0.29067, 0.06344, 0.13234, 0.11874, 0.08945, 0.15032, 0.15177,-0.00002,-0.02123,-0.02686,-0.01694,-0.02505,-0.00809, 0.10145},
{ 0.29061, 0.06372, 0.13255, 0.11894, 0.08928, 0.15168, 0.14956, 0.00028,-0.02136,-0.02694,-0.01680,-0.02561,-0.00735, 0.10143},
{ 0.29055, 0.06401, 0.13275, 0.11914, 0.08915, 0.15298, 0.14738, 0.00057,-0.02148,-0.02701,-0.01664,-0.02620,-0.00659, 0.10140},
{ 0.29049, 0.06429, 0.13296, 0.11932, 0.08905, 0.15422, 0.14524, 0.00087,-0.02161,-0.02709,-0.01648,-0.02681,-0.00582, 0.10137},
{ 0.29043, 0.06458, 0.13316, 0.11950, 0.08898, 0.15540, 0.14312, 0.00116,-0.02174,-0.02718,-0.01629,-0.02744,-0.00504, 0.10135},
{ 0.29037, 0.06488, 0.13337, 0.11967, 0.08894, 0.15651, 0.14104, 0.00145,-0.02186,-0.02727,-0.01610,-0.02809,-0.00424, 0.10132},
{ 0.29032, 0.06517, 0.13357, 0.11983, 0.08895, 0.15755, 0.13900, 0.00174,-0.02199,-0.02736,-0.01588,-0.02876,-0.00343, 0.10129},
{ 0.29027, 0.06547, 0.13378, 0.11998, 0.08898, 0.15853, 0.13699, 0.00202,-0.02211,-0.02747,-0.01566,-0.02945,-0.00260, 0.10127},
{ 0.29022, 0.06577, 0.13398, 0.12012, 0.08906, 0.15944, 0.13501, 0.00230,-0.02224,-0.02757,-0.01542,-0.03016,-0.00176, 0.10125},
{ 0.29018, 0.06607, 0.13418, 0.12026, 0.08917, 0.16027, 0.13308, 0.00257,-0.02237,-0.02768,-0.01517,-0.03090,-0.00091, 0.10124},
{ 0.29014, 0.06637, 0.13439, 0.12038, 0.08932, 0.16104, 0.13117, 0.00283,-0.02250,-0.02780,-0.01490,-0.03164,-0.00004, 0.10124},
{ 0.29011, 0.06668, 0.13459, 0.12049, 0.08951, 0.16174, 0.12931, 0.00308,-0.02262,-0.02793,-0.01462,-0.03241, 0.00083, 0.10124},
{ 0.29008, 0.06698, 0.13479, 0.12059, 0.08974, 0.16237, 0.12748, 0.00333,-0.02276,-0.02805,-0.01433,-0.03319, 0.00171, 0.10125},
{ 0.29006, 0.06729, 0.13500, 0.12068, 0.09001, 0.16292, 0.12569, 0.00356,-0.02289,-0.02819,-0.01403,-0.03399, 0.00261, 0.10126},
{ 0.29004, 0.06760, 0.13520, 0.12076, 0.09032, 0.16341, 0.12394, 0.00379,-0.02302,-0.02833,-0.01371,-0.03480, 0.00351, 0.10129},
{ 0.29002, 0.06791, 0.13541, 0.12083, 0.09067, 0.16382, 0.12222, 0.00401,-0.02316,-0.02848,-0.01339,-0.03562, 0.00442, 0.10133},
{ 0.29001, 0.06823, 0.13561, 0.12089, 0.09106, 0.16416, 0.12054, 0.00422,-0.02330,-0.02863,-0.01306,-0.03644, 0.00533, 0.10138},
{ 0.29001, 0.06854, 0.13582, 0.12094, 0.09150, 0.16443, 0.11889, 0.00441,-0.02344,-0.02879,-0.01272,-0.03728, 0.00625, 0.10143},
{ 0.29001, 0.06885, 0.13603, 0.12098, 0.09197, 0.16463, 0.11728, 0.00460,-0.02358,-0.02895,-0.01237,-0.03812, 0.00718, 0.10150},
{ 0.29002, 0.06917, 0.13624, 0.12100, 0.09249, 0.16476, 0.11571, 0.00478,-0.02373,-0.02912,-0.01202,-0.03897, 0.00811, 0.10159},
{ 0.29003, 0.06948, 0.13645, 0.12101, 0.09304, 0.16482, 0.11416, 0.00494,-0.02388,-0.02930,-0.01167,-0.03982, 0.00904, 0.10168},
{ 0.29004, 0.06980, 0.13666, 0.12101, 0.09364, 0.16481, 0.11266, 0.00510,-0.02403,-0.02947,-0.01131,-0.04066, 0.00997, 0.10178},
{ 0.29007, 0.07012, 0.13687, 0.12100, 0.09428, 0.16473, 0.11118, 0.00524,-0.02419,-0.02966,-0.01094,-0.04151, 0.01090, 0.10190},
{ 0.29009, 0.07044, 0.13709, 0.12098, 0.09497, 0.16458, 0.10974, 0.00538,-0.02434,-0.02985,-0.01058,-0.04236, 0.01182, 0.10203},
{ 0.29012, 0.07076, 0.13731, 0.12095, 0.09569, 0.16436, 0.10833, 0.00550,-0.02450,-0.03004,-0.01021,-0.04320, 0.01275, 0.10218},
{ 0.29016, 0.07108, 0.13753, 0.12090, 0.09646, 0.16407, 0.10696, 0.00562,-0.02466,-0.03023,-0.00985,-0.04403, 0.01367, 0.10233},
{ 0.29020, 0.07141, 0.13775, 0.12084, 0.09726, 0.16372, 0.10561, 0.00572,-0.02483,-0.03043,-0.00949,-0.04485, 0.01459, 0.10250},
{ 0.29025, 0.07173, 0.13797, 0.12077, 0.09811, 0.16329, 0.10430, 0.00582,-0.02500,-0.03063,-0.00913,-0.04567, 0.01550, 0.10268},
{ 0.29030, 0.07205, 0.13820, 0.12068, 0.09900, 0.16280, 0.10302, 0.00591,-0.02517,-0.03084,-0.00877,-0.04647, 0.01641, 0.10288},
{ 0.29035, 0.07238, 0.13842, 0.12059, 0.09993, 0.16225, 0.10177, 0.00598,-0.02534,-0.03104,-0.00842,-0.04726, 0.01730, 0.10308},
{ 0.29041, 0.07271, 0.13865, 0.12048, 0.10090, 0.16163, 0.10055, 0.00605,-0.02551,-0.03125,-0.00807,-0.04804, 0.01819, 0.10330},
{ 0.29047, 0.07304, 0.13888, 0.12036, 0.10191, 0.16094, 0.09936, 0.00611,-0.02569,-0.03147,-0.00773,-0.04880, 0.01907, 0.10353},
{ 0.29054, 0.07337, 0.13912, 0.12023, 0.10296, 0.16019, 0.09820, 0.00616,-0.02587,-0.03168,-0.00739,-0.04954, 0.01994, 0.10377},
{ 0.29061, 0.07370, 0.13935, 0.12008, 0.10405, 0.15937, 0.09707, 0.00620,-0.02604,-0.03189,-0.00706,-0.05027, 0.02080, 0.10403},
{ 0.29069, 0.07403, 0.13959, 0.11992, 0.10518, 0.15849, 0.09597, 0.00624,-0.02623,-0.03211,-0.00675,-0.05097, 0.02165, 0.10429},
{ 0.29077, 0.07436, 0.13983, 0.11975, 0.10635, 0.15755, 0.09490, 0.00627,-0.02641,-0.03233,-0.00644,-0.05166, 0.02248, 0.10457},
{ 0.29085, 0.07470, 0.14007, 0.11957, 0.10755, 0.15654, 0.09386, 0.00629,-0.02659,-0.03255,-0.00614,-0.05232, 0.02330, 0.10485},
{ 0.29093, 0.07503, 0.14032, 0.11938, 0.10880, 0.15547, 0.09285, 0.00630,-0.02677,-0.03276,-0.00585,-0.05296, 0.02411, 0.10515},
{ 0.29102, 0.07537, 0.14057, 0.11917, 0.11008, 0.15434, 0.09187, 0.00631,-0.02696,-0.03298,-0.00557,-0.05358, 0.02490, 0.10546},
{ 0.29111, 0.07571, 0.14082, 0.11895, 0.11140, 0.15315, 0.09092, 0.00631,-0.02714,-0.03320,-0.00530,-0.05417, 0.02567, 0.10577},
{ 0.29120, 0.07605, 0.14107, 0.11872, 0.11276, 0.15190, 0.09000, 0.00631,-0.02733,-0.03342,-0.00504,-0.05474, 0.02643, 0.10610},
{ 0.29130, 0.07639, 0.14132, 0.11848, 0.11415, 0.15059, 0.08911, 0.00630,-0.02751,-0.03364,-0.00480,-0.05528, 0.02717, 0.10643},
{ 0.29140, 0.07673, 0.14158, 0.11822, 0.11557, 0.14922, 0.08824, 0.00628,-0.02770,-0.03385,-0.00457,-0.05579, 0.02789, 0.10677},
{ 0.29150, 0.07708, 0.14184, 0.11796, 0.11704, 0.14779, 0.08741, 0.00627,-0.02788,-0.03407,-0.00436,-0.05628, 0.02859, 0.10712},
{ 0.29160, 0.07742, 0.14210, 0.11768, 0.11853, 0.14631, 0.08661, 0.00624,-0.02806,-0.03428,-0.00416,-0.05674, 0.02928, 0.10747},
{ 0.29171, 0.07777, 0.14236, 0.11739, 0.12006, 0.14477, 0.08584, 0.00622,-0.02824,-0.03450,-0.00397,-0.05717, 0.02994, 0.10783},
{ 0.29181, 0.07811, 0.14262, 0.11709, 0.12162, 0.14317, 0.08510, 0.00619,-0.02842,-0.03471,-0.00380,-0.05757, 0.03058, 0.10819},
{ 0.29192, 0.07846, 0.14289, 0.11678, 0.12321, 0.14152, 0.08438, 0.00616,-0.02860,-0.03491,-0.00364,-0.05794, 0.03121, 0.10856},
{ 0.29203, 0.07881, 0.14316, 0.11646, 0.12483, 0.13982, 0.08370, 0.00612,-0.02878,-0.03512,-0.00350,-0.05828, 0.03181, 0.10894},
{ 0.29215, 0.07916, 0.14343, 0.11613, 0.12649, 0.13806, 0.08305, 0.00608,-0.02895,-0.03532,-0.00338,-0.05860, 0.03239, 0.10931},
{ 0.29226, 0.07951, 0.14370, 0.11578, 0.12817, 0.13626, 0.08243, 0.00604,-0.02912,-0.03552,-0.00327,-0.05888, 0.03294, 0.10969},
{ 0.29237, 0.07987, 0.14397, 0.11543, 0.12988, 0.13440, 0.08184, 0.00600,-0.02929,-0.03572,-0.00318,-0.05913, 0.03348, 0.11008},
{ 0.29249, 0.08022, 0.14425, 0.11507, 0.13161, 0.13249, 0.08129, 0.00596,-0.02945,-0.03592,-0.00310,-0.05935, 0.03399, 0.11046},
{ 0.29261, 0.08057, 0.14452, 0.11469, 0.13337, 0.13054, 0.08076, 0.00592,-0.02961,-0.03611,-0.00305,-0.05953, 0.03447, 0.11085},
{ 0.29272, 0.08093, 0.14480, 0.11431, 0.13516, 0.12854, 0.08027, 0.00587,-0.02977,-0.03630,-0.00301,-0.05969, 0.03493, 0.11123},
{ 0.29284, 0.08128, 0.14508, 0.11392, 0.13696, 0.12649, 0.07980, 0.00583,-0.02993,-0.03648,-0.00298,-0.05982, 0.03537, 0.11162},
{ 0.29296, 0.08164, 0.14536, 0.11352, 0.13879, 0.12440, 0.07937, 0.00578,-0.03007,-0.03666,-0.00297,-0.05991, 0.03579, 0.11201},
{ 0.29308, 0.08199, 0.14564, 0.11311, 0.14064, 0.12227, 0.07897, 0.00574,-0.03022,-0.03684,-0.00298,-0.05998, 0.03617, 0.11239},
{ 0.29320, 0.08235, 0.14592, 0.11270, 0.14251, 0.12010, 0.07861, 0.00570,-0.03036,-0.03701,-0.00301,-0.06001, 0.03654, 0.11277},
{ 0.29332, 0.08271, 0.14620, 0.11227, 0.14440, 0.11789, 0.07827, 0.00565,-0.03050,-0.03718,-0.00306,-0.06001, 0.03687, 0.11315},
{ 0.29345, 0.08306, 0.14648, 0.11184, 0.14631, 0.11564, 0.07797, 0.00561,-0.03063,-0.03734,-0.00312,-0.05999, 0.03719, 0.11353},
{ 0.29357, 0.08342, 0.14676, 0.11140, 0.14823, 0.11336, 0.07770, 0.00557,-0.03075,-0.03750,-0.00320,-0.05993, 0.03747, 0.11390},
{ 0.29369, 0.08377, 0.14704, 0.11096, 0.15016, 0.11104, 0.07746, 0.00553,-0.03087,-0.03765,-0.00329,-0.05984, 0.03774, 0.11427},
{ 0.29381, 0.08413, 0.14733, 0.11051, 0.15211, 0.10869, 0.07725, 0.00549,-0.03099,-0.03780,-0.00340,-0.05973, 0.03797, 0.11463},
{ 0.29393, 0.08448, 0.14761, 0.11005, 0.15407, 0.10631, 0.07707, 0.00546,-0.03110,-0.03795,-0.00353,-0.05958, 0.03818, 0.11499},
{ 0.29406, 0.08483, 0.14789, 0.10959, 0.15604, 0.10390, 0.07693, 0.00543,-0.03120,-0.03809,-0.00368,-0.05941, 0.03836, 0.11535},
{ 0.29418, 0.08519, 0.14817, 0.10912, 0.15801, 0.10147, 0.07681, 0.00539,-0.03130,-0.03822,-0.00384,-0.05921, 0.03852, 0.11569},
{ 0.29430, 0.08554, 0.14845, 0.10865, 0.16000, 0.09901, 0.07673, 0.00537,-0.03139,-0.03835,-0.00402,-0.05898, 0.03866, 0.11603},
{ 0.29442, 0.08589, 0.14873, 0.10817, 0.16199, 0.09653, 0.07668, 0.00534,-0.03147,-0.03848,-0.00421,-0.05873, 0.03876, 0.11637},
{ 0.29455, 0.08624, 0.14901, 0.10769, 0.16398, 0.09403, 0.07666, 0.00532,-0.03155,-0.03859,-0.00442,-0.05845, 0.03885, 0.11669},
{ 0.29467, 0.08658, 0.14929, 0.10721, 0.16597, 0.09151, 0.07667, 0.00529,-0.03163,-0.03871,-0.00465,-0.05814, 0.03891, 0.11701},
{ 0.29479, 0.08693, 0.14957, 0.10673, 0.16797, 0.08898, 0.07671, 0.00528,-0.03169,-0.03882,-0.00489,-0.05781, 0.03894, 0.11732},
{ 0.29491, 0.08727, 0.14984, 0.10624, 0.16996, 0.08643, 0.07678, 0.00526,-0.03175,-0.03892,-0.00514,-0.05745, 0.03895, 0.11762},
{ 0.29503, 0.08761, 0.15012, 0.10575, 0.17196, 0.08387, 0.07688, 0.00525,-0.03181,-0.03902,-0.00541,-0.05708, 0.03893, 0.11791},
{ 0.29515, 0.08795, 0.15039, 0.10526, 0.17395, 0.08130, 0.07700, 0.00524,-0.03185,-0.03912,-0.00569,-0.05667, 0.03889, 0.11819},
{ 0.29527, 0.08829, 0.15066, 0.10477, 0.17593, 0.07872, 0.07716, 0.00524,-0.03190,-0.03921,-0.00599,-0.05625, 0.03883, 0.11847},
{ 0.29539, 0.08863, 0.15093, 0.10428, 0.17791, 0.07614, 0.07734, 0.00524,-0.03193,-0.03929,-0.00630,-0.05581, 0.03875, 0.11873},
{ 0.29551, 0.08896, 0.15120, 0.10378, 0.17988, 0.07355, 0.07755, 0.00524,-0.03196,-0.03937,-0.00662,-0.05535, 0.03864, 0.11898},
{ 0.29563, 0.08929, 0.15147, 0.10329, 0.18184, 0.07096, 0.07778, 0.00524,-0.03198,-0.03944,-0.00695,-0.05486, 0.03851, 0.11923},
{ 0.29575, 0.08961, 0.15173, 0.10281, 0.18378, 0.06838, 0.07804, 0.00525,-0.03200,-0.03951,-0.00730,-0.05436, 0.03836, 0.11946},
{ 0.29587, 0.08994, 0.15199, 0.10232, 0.18571, 0.06579, 0.07832, 0.00526,-0.03201,-0.03957,-0.00766,-0.05384, 0.03820, 0.11969},
{ 0.29599, 0.09026, 0.15225, 0.10184, 0.18762, 0.06322, 0.07862, 0.00527,-0.03202,-0.03963,-0.00803,-0.05331, 0.03801, 0.11990},
{ 0.29611, 0.09058, 0.15250, 0.10137, 0.18951, 0.06066, 0.07894, 0.00529,-0.03202,-0.03968,-0.00842,-0.05275, 0.03780, 0.12011},
{ 0.29623, 0.09089, 0.15275, 0.10091, 0.19137, 0.05811, 0.07929, 0.00531,-0.03202,-0.03972,-0.00881,-0.05219, 0.03758, 0.12031},
{ 0.29635, 0.09120, 0.15299, 0.10045, 0.19320, 0.05557, 0.07965, 0.00533,-0.03202,-0.03975,-0.00922,-0.05160, 0.03734, 0.12051},
{ 0.29647, 0.09151, 0.15323, 0.10001, 0.19501, 0.05306, 0.08002, 0.00535,-0.03201,-0.03978,-0.00965,-0.05100, 0.03709, 0.12069},
{ 0.29660, 0.09181, 0.15347, 0.09958, 0.19678, 0.05056, 0.08041, 0.00538,-0.03200,-0.03981,-0.01008,-0.05039, 0.03682, 0.12087},
{ 0.29672, 0.09212, 0.15370, 0.09917, 0.19851, 0.04809, 0.08081, 0.00541,-0.03199,-0.03982,-0.01053,-0.04976, 0.03654, 0.12105},
{ 0.29685, 0.09241, 0.15392, 0.09877, 0.20020, 0.04564, 0.08122, 0.00544,-0.03198,-0.03982,-0.01100,-0.04912, 0.03625, 0.12123},
{ 0.29697, 0.09271, 0.15414, 0.09839, 0.20184, 0.04323, 0.08163, 0.00547,-0.03197,-0.03982,-0.01148,-0.04847, 0.03595, 0.12140},
{ 0.29711, 0.09300, 0.15436, 0.09803, 0.20344, 0.04084, 0.08206, 0.00550,-0.03196,-0.03981,-0.01197,-0.04781, 0.03564, 0.12157},
{ 0.29724, 0.09329, 0.15456, 0.09769, 0.20499, 0.03848, 0.08249, 0.00553,-0.03195,-0.03978,-0.01248,-0.04713, 0.03533, 0.12173},
{ 0.29737, 0.09358, 0.15476, 0.09738, 0.20650, 0.03616, 0.08292, 0.00556,-0.03194,-0.03975,-0.01300,-0.04644, 0.03501, 0.12190},
{ 0.29751, 0.09386, 0.15495, 0.09708, 0.20794, 0.03388, 0.08335, 0.00559,-0.03194,-0.03971,-0.01354,-0.04574, 0.03468, 0.12207},
{ 0.29765, 0.09414, 0.15513, 0.09682, 0.20933, 0.03163, 0.08378, 0.00563,-0.03194,-0.03966,-0.01409,-0.04503, 0.03435, 0.12225},
{ 0.29780, 0.09442, 0.15531, 0.09658, 0.21067, 0.02941, 0.08420, 0.00566,-0.03194,-0.03959,-0.01466,-0.04430, 0.03402, 0.12243},
{ 0.29794, 0.09470, 0.15548, 0.09637, 0.21195, 0.02724, 0.08462, 0.00569,-0.03195,-0.03952,-0.01524,-0.04357, 0.03368, 0.12261},
{ 0.29809, 0.09497, 0.15564, 0.09619, 0.21316, 0.02510, 0.08504, 0.00572,-0.03196,-0.03943,-0.01585,-0.04282, 0.03334, 0.12280},
{ 0.29825, 0.09525, 0.15579, 0.09603, 0.21432, 0.02301, 0.08545, 0.00575,-0.03198,-0.03934,-0.01647,-0.04206, 0.03301, 0.12299},
{ 0.29840, 0.09552, 0.15593, 0.09591, 0.21541, 0.02095, 0.08585, 0.00578,-0.03200,-0.03923,-0.01710,-0.04129, 0.03267, 0.12319},
{ 0.29856, 0.09578, 0.15607, 0.09582, 0.21644, 0.01894, 0.08625, 0.00580,-0.03203,-0.03910,-0.01775,-0.04051, 0.03234, 0.12340},
{ 0.29873, 0.09605, 0.15619, 0.09576, 0.21740, 0.01696, 0.08663, 0.00583,-0.03207,-0.03897,-0.01842,-0.03972, 0.03201, 0.12362},
{ 0.29890, 0.09632, 0.15631, 0.09573, 0.21830, 0.01502, 0.08700, 0.00585,-0.03211,-0.03883,-0.01911,-0.03892, 0.03168, 0.12385},
{ 0.29907, 0.09658, 0.15642, 0.09574, 0.21914, 0.01313, 0.08736, 0.00587,-0.03216,-0.03867,-0.01981,-0.03810, 0.03136, 0.12408},
{ 0.29924, 0.09684, 0.15651, 0.09578, 0.21991, 0.01127, 0.08771, 0.00589,-0.03222,-0.03850,-0.02053,-0.03728, 0.03104, 0.12433},
{ 0.29942, 0.09710, 0.15660, 0.09585, 0.22062, 0.00945, 0.08804, 0.00590,-0.03228,-0.03832,-0.02126,-0.03645, 0.03072, 0.12459},
{ 0.29961, 0.09736, 0.15668, 0.09596, 0.22127, 0.00767, 0.08836, 0.00592,-0.03236,-0.03814,-0.02200,-0.03561, 0.03041, 0.12486},
{ 0.29979, 0.09762, 0.15675, 0.09610, 0.22184, 0.00593, 0.08867, 0.00593,-0.03244,-0.03794,-0.02276,-0.03476, 0.03011, 0.12515},
{ 0.29998, 0.09788, 0.15681, 0.09627, 0.22236, 0.00422, 0.08896, 0.00594,-0.03252,-0.03773,-0.02353,-0.03390, 0.02981, 0.12544},
{ 0.30018, 0.09814, 0.15686, 0.09648, 0.22281, 0.00255, 0.08923, 0.00595,-0.03262,-0.03751,-0.02431,-0.03304, 0.02952, 0.12575},
{ 0.30037, 0.09840, 0.15690, 0.09672, 0.22320, 0.00092, 0.08949, 0.00595,-0.03272,-0.03729,-0.02510,-0.03217, 0.02924, 0.12607},
{ 0.30057, 0.09866, 0.15693, 0.09700, 0.22353,-0.00068, 0.08974, 0.00595,-0.03283,-0.03706,-0.02589,-0.03129, 0.02897, 0.12640},
{ 0.30078, 0.09892, 0.15695, 0.09731, 0.22380,-0.00225, 0.08997, 0.00595,-0.03295,-0.03682,-0.02669,-0.03042, 0.02870, 0.12675},
{ 0.30098, 0.09918, 0.15696, 0.09765, 0.22400,-0.00379, 0.09019, 0.00595,-0.03307,-0.03658,-0.02749,-0.02953, 0.02844, 0.12711},
{ 0.30119, 0.09944, 0.15696, 0.09803, 0.22415,-0.00529, 0.09039, 0.00594,-0.03320,-0.03634,-0.02830,-0.02865, 0.02819, 0.12748},
{ 0.30141, 0.09970, 0.15695, 0.09844, 0.22424,-0.00676, 0.09057, 0.00593,-0.03334,-0.03609,-0.02910,-0.02776, 0.02795, 0.12786},
{ 0.30162, 0.09996, 0.15693, 0.09889, 0.22427,-0.00820, 0.09074, 0.00592,-0.03348,-0.03584,-0.02991,-0.02687, 0.02772, 0.12826},
{ 0.30184, 0.10022, 0.15690, 0.09937, 0.22424,-0.00962, 0.09089, 0.00590,-0.03363,-0.03559,-0.03071,-0.02599, 0.02750, 0.12867},
{ 0.30207, 0.10048, 0.15686, 0.09988, 0.22415,-0.01100, 0.09103, 0.00588,-0.03378,-0.03534,-0.03151,-0.02510, 0.02728, 0.12909},
{ 0.30229, 0.10075, 0.15681, 0.10042, 0.22401,-0.01236, 0.09116, 0.00586,-0.03394,-0.03508,-0.03230,-0.02422, 0.02708, 0.12953},
{ 0.30252, 0.10101, 0.15674, 0.10100, 0.22381,-0.01370, 0.09127, 0.00584,-0.03410,-0.03483,-0.03309,-0.02333, 0.02689, 0.12997}
  };
  //
for (int iii=0; iii<192; iii++){
  for (int jjj=0; jjj<14; jjj++){
      CoeffSigPDF0[iii][jjj] = par_f0[iii][jjj];
      CoeffSigPDF1[iii][jjj] = par_f1[iii][jjj];
      CoeffNoise31[iii][jjj] = par_fg31[iii][jjj];
      CoeffNoise32[iii][jjj] = par_fg32[iii][jjj];
      CoeffNoise33[iii][jjj] = par_fg33[iii][jjj];
    }
  }
  //
  //
  //
  return;
}
//
//
//
