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

#include <framework/logging/Logger.h>

#include <ecl/geometry/ECLGeometryPar.h>
#include <ecl/dataobjects/ECLSimHit.h>
#include <ecl/dataobjects/ECLHit.h>

#include "trg/ecl/TrgEclFAM.h"
#include "trg/ecl/TrgEclCluster.h"

#include "trg/ecl/dataobjects/TRGECLDigi.h"
#include "trg//ecl/dataobjects/TRGECLDigi0.h"
#include "trg/ecl/dataobjects/TRGECLHit.h"

#include <stdlib.h>
#include <iostream>
#include <math.h>
#include <TRandom.h>

using namespace std;
using namespace Belle2;
//using namespace TRG;
//
//
//
TrgEclFAM::TrgEclFAM(): TimeRange(0)//, bin(0)
{

  CoeffSigPDF0.clear();
  CoeffSigPDF1.clear();
  CoeffNoise31.clear();
  CoeffNoise32.clear();
  CoeffNoise33.clear();
  MatrixParallel.clear();
  MatrixSerial.clear();

  _TCMap = new TrgEclMapping();


  for (int iTCIdm = 0; iTCIdm < 576; iTCIdm++) {
    noutput[iTCIdm] = 0;
    ninput[iTCIdm] = 0;

    for (int  iTime = 0; iTime < 80; iTime++) {
      TCEnergy[iTCIdm][iTime] = 0;
      TCTiming[iTCIdm][iTime] = 0;
      TCBeambkgTag[iTCIdm][iTime] = 0;
      TCBkgContribution[iTCIdm][iTime] = 0;
      TCSigContribution[iTCIdm][iTime] = 0;

    }
    TCEnergy_tot[iTCIdm] = 0;
    TCTiming_tot[iTCIdm] = 0;
  }
  for (int iTCIdm = 0; iTCIdm < 576; iTCIdm++) {
    for (int iii = 0; iii < 60 ; iii++) {
      TCFitEnergy[iTCIdm][iii] = 0;
      TCFitTiming[iTCIdm][iii] = 0;
      TCRawEnergy[iTCIdm][iii] = 0;
      TCRawTiming[iTCIdm][iii] = 0;
      TCRawBkgTag[iTCIdm][iii] = 0;


    }
  }
  for (int iTCIdm = 0; iTCIdm < 576; iTCIdm++) {
    for (int  iTime = 0; iTime < 64; iTime++) {
      TCDigiE[iTCIdm][iTime] = 0.;
      TCDigiT[iTCIdm][iTime] = 0.;
    }
  }
  for (int iini = 0; iini < 100; iini++) {
    returnE[iini] = 0;
    returnT[iini] = 0;


  }


}
//
//
//
TrgEclFAM::~TrgEclFAM()
{

  delete _TCMap;

}
//
//
//

void
TrgEclFAM::setup(int m_nEvent,
                 int digi_method
                )
{

  //
  // prepare coefficient for fitting
  //
  readFAMDB();
  //
  // prepare Matrix for Noise generation
  //
  readNoiseLMatrix();
  //
  int TableFlag = 1;//1: ECLHit ,2: ECLSimHit
  // initialize parameters
  //

  //
  // digitization0
  //
  getTCHit(TableFlag);
  cout << m_nEvent << endl;
  if (digi_method == 2) { digitization02(); }   // no-fit method = backup method 1
  else if (digi_method == 3) { digitization03(); } // orignal method = backup method 2
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
TrgEclFAM::getTCHit(int TableFlag)
{
  //
  // float (*E_cell)[80]  = new float[8736][80];
  // float (*T_ave)[80]   = new float[8736][80]; // (1)Averaged time[ns] between hit point in Xtal and PD.
  // float (*Tof_ave)[80] = new float[8736][80]; // (2)Averaged time[ns] between hit point in Xtal and IP.
  // int (*beambkg_tag)[80] = new int[8736][80]; // (2)Averaged time[ns] between hit point in Xtal and IP.


  //  int nBinTime = 80; // "80" covers t=-4 ~ 4us even for 100ns binning.
  /*
  for (int iXCell = 0; iXCell < 8736; iXCell++) {
    for (int iBinTime = 0; iBinTime < 80; iBinTime++) {
      E_cell[iXCell][iBinTime] = 0;
      T_ave[iXCell][iBinTime] = 0;
      Tof_ave[iXCell][iBinTime] = 0;
      beambkg_tag[iXCell][iBinTime] = 0;


    }

  }
  */
  std::vector< std::vector<float> > E_cell(8736, std::vector<float>(80, 0.0));
  std::vector< std::vector<float> > T_ave(8736, std::vector<float>(80, 0.0));
  std::vector< std::vector<float> > Tof_ave(8736, std::vector<float>(80, 0.0));
  std::vector< std::vector<float> > beambkg_tag(8736, std::vector<float>(80, 0.0));

  int nBinTime = 80;
  TimeRange = 4000; // -4us ~ 4us
  ////-------------------------------------------------------------------
  //                          read Xtal data
  //---------------------------------------------------------------------
  if (TableFlag == 1) { // read  ECLHit table
    StoreArray<ECLHit> eclHitArray("ECLHits");
    int nHits_hit = eclHitArray.getEntries() - 1;
    //
    for (int iHits = 0; iHits < nHits_hit; iHits++) {
      // Get a hit
      ECLHit* aECLHit = eclHitArray[iHits];
      int beambkg = aECLHit->getBackgroundTag();

      // Hit geom. info
      int hitCellId  = aECLHit->getCellId() - 1;
      float hitE     = aECLHit->getEnergyDep() / Unit::GeV;
      float aveT     = aECLHit->getTimeAve(); // ns :time from  IP  to PD
      if (aveT < - TimeRange || aveT > TimeRange) {continue;} //Choose - TimeRange ~ TimeTange
      int  TimeIndex = (int)((aveT + TimeRange) / 100); //Binning : -4000 = 1st bin ~  4000 80th bin.
      for (int iECLCell = 0; iECLCell < 8736; iECLCell++) {
        if (hitCellId == iECLCell) {

          E_cell[iECLCell][TimeIndex]  = E_cell[iECLCell][TimeIndex] + hitE;
          T_ave[iECLCell][TimeIndex]   = T_ave[iECLCell][TimeIndex] + hitE * aveT;

          beambkg_tag[iECLCell][TimeIndex] = beambkg ;

        }
      }

    }
    for (int iECLCell = 0; iECLCell < 8736; iECLCell++) {
      for (int  iTime = 0; iTime < nBinTime; iTime++) {
        if (E_cell[iECLCell][iTime] > 0) {
          T_ave[iECLCell][iTime]   = T_ave[iECLCell][iTime] / E_cell[iECLCell][iTime];
        }
      }
    }//Get Energy weighted timing of each TC.
  }


  if (TableFlag == 2) { // read ECLSimHit
    ECL::ECLGeometryPar* eclp = ECL::ECLGeometryPar::Instance();
    //=====================
    // Loop over all hits of steps
    //=====================
    StoreArray<ECLSimHit> eclArray("ECLSimHits");
    int nHits = eclArray.getEntries();
    //
    for (int iHits = 0; iHits < nHits; iHits++) {
      // Get a hit
      ECLSimHit* aECLSimHit = eclArray[iHits];

      int hitCellId  = aECLSimHit->getCellId() - 1;
      float hitE     = aECLSimHit->getEnergyDep() / Unit::GeV;
      float hitTOF      = aECLSimHit->getFlightTime() / Unit::ns;

      G4ThreeVector t = aECLSimHit->getPosIn(); // [cm], Hit position in Xtal (based on from IP)
      TVector3 HitInPos(t.x(), t.y(), t.z()); // = aECLSimHit->getPosIn(); // [cm], Hit position in Xtal (based on from IP)
      TVector3 PosCell  = eclp->GetCrystalPos(hitCellId); // [cm], Xtal position (based on from IP)
      TVector3 VecCell  = eclp->GetCrystalVec(hitCellId);
      // "local_pos_r" = Distance between track hit in Xtal and
      // rear surface(max=30, min=0) of the Xtal.
      float local_pos_r = (15.0 - (HitInPos - PosCell) * VecCell);
      if (hitTOF < - TimeRange || hitTOF >  TimeRange) {continue;}
      int TimeIndex = (int)((hitTOF + TimeRange) / 100);
      for (int iECLCell = 0; iECLCell < 8736; iECLCell++) {
        if (hitCellId == iECLCell && hitTOF < 8000) {
          // time interval is (0.5, 0.1, 0.05us) => (16, 80, 160bins) between T=0-8us.
          //int TimeIndex = (int) (hitTOF / 100);
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
    int iTCIdm = _TCMap->getTCIdFromXtalId(iXtalIdm + 1) - 1;
    for (int  iTime = 0; iTime < nBinTime; iTime++) {
      if (E_cell[iXtalIdm][iTime] < 1e-9) {continue;}  // 0.01MeV cut
      TCEnergy[iTCIdm][iTime] += E_cell[iXtalIdm][iTime];
      TCTiming[iTCIdm][iTime] += E_cell[iXtalIdm][iTime] * (T_ave[iXtalIdm][iTime]);
      if (beambkg_tag[iXtalIdm][iTime] > 0) {TCBkgContribution[iTCIdm][iTime] += E_cell[iXtalIdm][iTime];}
      if (beambkg_tag[iXtalIdm][iTime] == 0) {TCSigContribution[iTCIdm][iTime] += E_cell[iXtalIdm][iTime];}

    }
  }

  for (int iTCIdm = 0; iTCIdm < 576; iTCIdm++) {
    for (int  iTime = 0; iTime < nBinTime; iTime++) {
      double maxbkgE = 0;
      int maxbkgtag = 0;
      if (TCEnergy[iTCIdm][iTime] < 1e-9) {continue;}  // 0.01MeV cut
      TCTiming[iTCIdm][iTime] /= TCEnergy[iTCIdm][iTime];
      if (TCBkgContribution[iTCIdm][iTime] < TCSigContribution[iTCIdm][iTime]) {
        TCBeambkgTag[iTCIdm][iTime] = 0; //signal Tag : 0
      } else {
        for (int iXtalIdm = 0; iXtalIdm < 8736; iXtalIdm++) {
          int iTCId = _TCMap->getTCIdFromXtalId(iXtalIdm + 1) - 1;
          if (iTCIdm != iTCId) {continue;}
          if (maxbkgE < E_cell[iXtalIdm][iTime]) {
            maxbkgE = E_cell[iXtalIdm][iTime];
            maxbkgtag =  beambkg_tag[iXtalIdm][iTime];
          }
        }
        TCBeambkgTag[iTCIdm][iTime] = maxbkgtag;
      }
    }
  }
  //-------------------------------------------------
  // Save Raw data
  //--------------------------------------------
  for (int iTCIdm = 0; iTCIdm < 576; iTCIdm++) {
    ninput[iTCIdm] = 0;
    for (int  iTime = 39; iTime < nBinTime; iTime++) {
      if (TCEnergy[iTCIdm][iTime] >= 0.05) {
        TCRawEnergy[iTCIdm][ninput[iTCIdm]] = TCEnergy[iTCIdm][iTime];
        TCRawTiming[iTCIdm][ninput[iTCIdm]] = TCTiming[iTCIdm][iTime] + GetTCLatency(iTCIdm);
        TCRawBkgTag[iTCIdm][ninput[iTCIdm]] = TCBeambkgTag[iTCIdm][iTime];
        ninput[iTCIdm]++;
      }
    }
  }


  //--------------------------
  // TC energy and timing in t=0-1us as true values.
  //--------------------------
  for (int iTCIdm = 0; iTCIdm < 576; iTCIdm++) {
    for (int  iTime = 40; iTime < 50; iTime++) {
      TCEnergy_tot[iTCIdm] += TCEnergy[iTCIdm][iTime];
      TCTiming_tot[iTCIdm] += TCTiming[iTCIdm][iTime] * TCEnergy[iTCIdm][iTime];
    }
    TCTiming_tot[iTCIdm] /= TCEnergy_tot[iTCIdm];

  }


  //
  // delete [] beambkg_tag;
  // delete [] E_cell;
  // delete [] T_ave;
  // delete [] Tof_ave;
  return;
}
//
//
//
void
TrgEclFAM::digitization01(void)
{
  //
  double cut_energy_tot = 0.03; // [GeV]
  int nbin_pedestal = 4; // = nbin_pedestal*fam_sampling_interval [ns] in total
  double fam_sampling_interval = 125; // [ns]
  int NSampling = 64; // # of sampling array


  /*
  float (*noise_pileup)[64] = new float [576][64];  // [GeV]
  float (*noise_parallel)[64] = new float [576][64];  // [GeV]
  float (*noise_serial)[64] =  new float [576][64];  // [GeV]
  float* X_pr = new float [64];
  float* X_sr = new float [64];
  */
  std::vector< std::vector<float> > noise_pileup(576, std::vector<float>(64, 0.0));   // [GeV]
  std::vector< std::vector<float> > noise_parallel(576, std::vector<float>(64, 0.0));   // [GeV]
  std::vector< std::vector<float> > noise_serial(576, std::vector<float>(64, 0.0));   // [GeV]
  std::vector<float> X_pr(64, 0.0);
  std::vector<float> X_sr(64, 0.0);


  /*
  for (int iTCIdm = 0; iTCIdm < 576; iTCIdm++) {
    for (int iSampling = 0; iSampling < NSampling; iSampling++) {
      noise_pileup[iTCIdm][iSampling] = 0;
      noise_parallel[iTCIdm][iSampling] = 0;
      noise_serial[iTCIdm][iSampling] = 0;
      X_pr[iSampling] = 0;
      X_sr[iSampling] = 0;
    }
  }
  */

  // (Make sampling time random between FAM sampling intervals)
  double random_sampling_correction = 0; // [ns]
  random_sampling_correction = gRandom->Rndm() * fam_sampling_interval;
  //==================
  // (01)Signal digitization
  //==================
  for (int iTCIdm = 0; iTCIdm < 576; iTCIdm++) {
    if (TCEnergy_tot[iTCIdm] < cut_energy_tot) {continue;} // TC energy_tot cut
    for (int iTimeBin = 0; iTimeBin < 80; iTimeBin++) {
      if (TCEnergy[iTCIdm][iTimeBin] < 0.0001) {continue;} // 0.1MeV cut on TC bin_energy
      for (int iSampling = 0; iSampling < NSampling; iSampling++) {
        // inputTiming is in [us] <-- Be careful, here is NOT [ns]
        double inputTiming
          = (-TCTiming[iTCIdm][iTimeBin] - TimeRange + (-nbin_pedestal + iSampling) * fam_sampling_interval) * 0.001;
        inputTiming += random_sampling_correction * 0.001;
        if (inputTiming > 0) {
          TCDigiE[iTCIdm][iSampling] += SimplifiedFADC(0, inputTiming) * TCEnergy[iTCIdm][iTimeBin];
        }
      }
      for (int iSampling = 0; iSampling < NSampling; iSampling++) {
        TCDigiT[iTCIdm][iSampling] = (-nbin_pedestal + iSampling - TimeRange / fam_sampling_interval) * fam_sampling_interval;
        TCDigiT[iTCIdm][iSampling] += random_sampling_correction;
      }
    }
  }
  //
  //
  //
  if (0) {
    FILE* f_out_dat = fopen("ztsim.no_noise.dat", "w");
    for (int iTCIdm = 0; iTCIdm < 576; iTCIdm++) {
      if (TCEnergy_tot[iTCIdm] < cut_energy_tot) { continue; }   // TC energy_tot cut
      fprintf(f_out_dat, "%5i %8.5f %8.5f %8.1f ",
              iTCIdm + 1, TCEnergy_tot[iTCIdm], TCEnergy[iTCIdm][0], TCDigiT[iTCIdm][0]);
      for (int iSampling = 0; iSampling < NSampling; iSampling++) {
        fprintf(f_out_dat, "%7.4f ", TCDigiE[iTCIdm][iSampling]);
      }
      fprintf(f_out_dat, "\n");
    }
    fclose(f_out_dat);
  }
  //==================
  // (01)noise embedding
  //==================
  //  double tmin_noise = -nbin_pedestal * fam_sampling_interval * 0.001; // [us]
  //double tgen = NSampling * fam_sampling_interval * 0.001 - tmin_noise; // [us]
  double  tmin_noise = -4; // orignal
  double tgen = 10.3;   // orignal
  int bkg_level = 1030;
  double ttt0 = 0; // [us]
  double ttt1 = 0; // [us]
  double ttt2 = 0; // [us]
  //
  double frac_pileup   = 0.035; // pileup noise fraction?
  double frac_parallel = 0.023; // parralel noise fraction?
  double frac_serial   = 0.055; // serial noise fraction?
  double times_pileup   =  1;   // noise scale based on Belle noise.
  double times_parallel =  3.15;   // noise scale
  double times_serial   =  3.15;   // noise scale
//   double times_parallel =  1.0;   // noise scale
//   double times_serial   =  1.0;   // noise scale


  double corr_pileup   = times_pileup   * frac_pileup   * sqrt(fam_sampling_interval * 0.001);
  double corr_parallel = times_parallel * frac_parallel * sqrt(fam_sampling_interval * 0.001);
  double corr_serial   = times_serial   * frac_serial   * sqrt(fam_sampling_interval * 0.001);
  // corr_pileup   = 0.011068;   // default in case 1xBelle noise in 100ns sampling
  // corr_parallel = 0.00727324; // default in case 1xBelle noise in 100ns sampling
  // corr_serial   = 0.0173925;  // default in case 1xBelle noise in 100ns sampling


  if (0) {
    for (int iTCIdm = 0; iTCIdm < 576; iTCIdm++) {
      if (TCEnergy_tot[iTCIdm] < cut_energy_tot) { continue; }   // TC energy_tot cut
      for (int jjj = 0; jjj < bkg_level; jjj++) {
        ttt0 = -(tmin_noise + tgen * gRandom->Rndm()); // [us]
        ttt1 = -(tmin_noise + tgen * gRandom->Rndm());  // [us]
        ttt2 = -(tmin_noise + tgen * gRandom->Rndm());  // [us]
        for (int iSampling = 0; iSampling < NSampling; iSampling++) {
          // (pile-up noise)
          if (ttt0 > 0) { TCDigiE[iTCIdm][iSampling] += SimplifiedFADC(0, ttt0) * corr_pileup * 0.001; }
          // (parallel noise)
          if (ttt1 > 0) { TCDigiE[iTCIdm][iSampling] += SimplifiedFADC(1, ttt1) * corr_parallel; }
          // (serial noise)
          if (ttt2 > 0) { TCDigiE[iTCIdm][iSampling] += SimplifiedFADC(2, ttt2) * corr_serial; }
          ttt0 += fam_sampling_interval * 0.001;
          ttt1 += fam_sampling_interval * 0.001;
          ttt2 += fam_sampling_interval * 0.001;
        }
      }
    }
  }
  if (1) { //use L Matrix
    for (int iTCIdm = 0; iTCIdm < 576; iTCIdm++) {

      if (TCEnergy_tot[iTCIdm] < cut_energy_tot) { continue; }   // TC energy_tot cut

      for (int iSampling = 0; iSampling < NSampling; iSampling++) {
        X_pr[iSampling] = gRandom ->Gaus(0, 1);
        X_sr[iSampling] = gRandom ->Gaus(0, 1);
      }

      for (int iSampling = 0; iSampling < NSampling; iSampling++) {
        for (int jSampling = 0; jSampling < NSampling; jSampling++) {
          noise_parallel[iTCIdm][iSampling] += 10 * corr_parallel * MatrixParallel[iSampling][jSampling] * X_pr[jSampling];
          noise_serial[iTCIdm][iSampling] += 10 * corr_serial * MatrixSerial[iSampling][jSampling] * X_sr[jSampling];
        }
      }

      for (int iSampling = 0; iSampling < NSampling; iSampling++) {

        TCDigiE[iTCIdm][iSampling] += noise_pileup[iTCIdm][iSampling] + noise_parallel[iTCIdm][iSampling] +
                                      noise_serial[iTCIdm][iSampling];
      }
    }
    if (0) {
      for (int iTCIdm = 0; iTCIdm < 576; iTCIdm++) { //Only Pile-up noise use old method.
        if (TCEnergy_tot[iTCIdm] < cut_energy_tot) { continue; }   // TC energy_tot cut
        for (int jjj = 0; jjj < bkg_level; jjj++) {
          ttt0 = -(tmin_noise + tgen * gRandom->Rndm()); // [us]
          for (int iSampling = 0; iSampling < NSampling; iSampling++) {
            // (pile-up noise)
            if (ttt0 > 0) { TCDigiE[iTCIdm][iSampling] += SimplifiedFADC(0, ttt0) * corr_pileup * 0.001; }
            ttt0 += fam_sampling_interval * 0.001;
          }
        }
      }
    }
  }



  //
  //
  //
  if (0) {
    FILE* f_out_dat = fopen("ztsim.noise.dat", "w");
    for (int iTCIdm = 0; iTCIdm < 576; iTCIdm++) {
      if (TCEnergy_tot[iTCIdm] < cut_energy_tot) { continue; }   // TC energy_tot cut
      fprintf(f_out_dat, "%5i %8.5f %8.5f %8.1f ",
              iTCIdm + 1, TCEnergy_tot[iTCIdm], TCEnergy[iTCIdm][0], TCDigiT[iTCIdm][0]);
      for (int iSampling = 0; iSampling < NSampling; iSampling++) {
        fprintf(f_out_dat, "%7.4f ", TCDigiE[iTCIdm][iSampling]);
      }
      fprintf(f_out_dat, "\n");
    }
    fclose(f_out_dat);
  }
  //============
  // (01)fit
  //============
  for (int jjj = 0; jjj < 20; jjj++) {
    returnE[jjj] =  0  ;
    returnT[jjj] =  0   ;
  }



  for (int iTCIdm = 0; iTCIdm < 576; iTCIdm++) {
    noutput[iTCIdm] = 0;
    if (TCEnergy_tot[iTCIdm] < cut_energy_tot) {continue;} // TC energy_tot cut

    FAMFit(nbin_pedestal,
           NSampling,
           iTCIdm,
           fam_sampling_interval,
           TCDigiE[iTCIdm]);

    for (int inoutput = 0; inoutput < noutput[iTCIdm]; inoutput++) {
      TCFitEnergy[iTCIdm][inoutput] = returnE[inoutput];
      TCFitTiming[iTCIdm][inoutput] = returnT[inoutput] + GetTCLatency(iTCIdm) + random_sampling_correction;
    }
  }
  //
  //
  //
  /*
    delete [] X_pr;
    delete [] X_sr;
    delete [] noise_pileup;
    delete [] noise_serial;
    delete [] noise_parallel;
  */


  return;

}
//
//
//
void
TrgEclFAM::digitization02(void)
{

  double cut_energy_tot = 0.03; // [GeV]
  int nbin_pedestal = 4; // = nbin_pedestal*fam_sampling_interval [ns] in total
  double fam_sampling_interval = 125; //@ [ns]
  int NSampling = 64;

  std::vector< std::vector<double> > noise_pileup(576, std::vector<double>(64, 0.0));    // [GeV]
  std::vector< std::vector<double> > noise_parallel(576, std::vector<double>(64, 0.0));    // [GeV]
  std::vector< std::vector<double> > noise_serial(576, std::vector<double>(64, 0.0));    // [GeV]
  std::vector<double> X_pr(64, 0.0);
  std::vector<double> X_sr(64, 0.0);
  /*
  double (*noise_pileup)[64] = new double [576][64];  // [GeV]
  double (*noise_parallel)[64] = new double [576][64];  // [GeV]
  double (*noise_serial)[64] =  new double [576][64];  // [GeV]
  double* X_pr = new double [64];
  double* X_sr = new double [64];

  for (int iTCIdm = 0; iTCIdm < 576; iTCIdm++) {
    for (int iSampling = 0; iSampling < NSampling; iSampling++) {
      noise_pileup[iTCIdm][iSampling] = 0;
      noise_parallel[iTCIdm][iSampling] = 0;
      noise_serial[iTCIdm][iSampling] = 0;
      X_pr[iSampling] = 0;
      X_sr[iSampling] = 0;

    }
  }
  */




  //@ Make sampling time random between FAM sampling intervals
  double random_sampling_correction = 0; //@ [ns]
  random_sampling_correction = gRandom->Rndm() * fam_sampling_interval;
  //==================
  // (02)Signal digitization
  //==================
  for (int iTCIdm = 0; iTCIdm < 576; iTCIdm++) {
    if (TCEnergy_tot[iTCIdm] < cut_energy_tot) {continue;} // TC energy_tot cut
    for (int iTimeBin = 0; iTimeBin < 80; iTimeBin++) {
      if (TCEnergy[iTCIdm][iTimeBin] < 0.0001) {continue;} // 0.1 MeV cut on TC bin_energy
      cout << TCTiming[iTCIdm][iTimeBin] << endl;
      for (int iSampling = 0; iSampling < NSampling; iSampling++) {
        //@ inputTiming is in [us] <-- Be careful, here is NOT [ns]
        double inputTiming
          = (-TCTiming[iTCIdm][iTimeBin] - TimeRange + (-nbin_pedestal + iSampling) * fam_sampling_interval) * 0.001;
        inputTiming += random_sampling_correction * 0.001;
        TCDigiE[iTCIdm][iSampling] += SimplifiedFADC(0, inputTiming) * TCEnergy[iTCIdm][iTimeBin];
      }
      for (int iSampling = 0; iSampling < NSampling; iSampling++) {
        TCDigiT[iTCIdm][iSampling] = (-nbin_pedestal + iSampling - TimeRange / fam_sampling_interval) * fam_sampling_interval;
        TCDigiT[iTCIdm][iSampling] += random_sampling_correction;
      }
    }
  }
  //==================
  // (02)noise embedding
  //==================
  //  double tmin_noise = -nbin_pedestal * fam_sampling_interval * 0.001; // [us]
  // double tgen =  NSampling * fam_sampling_interval * 0.001 - tmin_noise; // [us]
  double  tmin_noise = -4; // orignal
  double tgen = 10.3;   //
  int bkg_level = 1030;
  double ttt0 = 0; //@ [us]
  double ttt1 = 0; //@ [us]
  double ttt2 = 0; //@ [us]
  double frac_pileup   = 0.035; // pileup noise fraction?
  double frac_parallel = 0.023; // parralel noise fraction?
  double frac_serial   = 0.055; // serial noise fraction?
  double times_pileup   =  3.15;   // noise scale based on Belle noise.
  double times_parallel =  3.15;   // noise scale
  double times_serial   =  3.15;   // noise scale
  double corr_pileup   = times_pileup   * frac_pileup   * sqrt(fam_sampling_interval * 0.001);
  double corr_parallel = times_parallel * frac_parallel * sqrt(fam_sampling_interval * 0.001);
  double corr_serial   = times_serial   * frac_serial   * sqrt(fam_sampling_interval * 0.001);
  // corr_pileup   = 0.011068;
  // corr_parallel = 0.00727324;
  // corr_serial   = 0.0173925;
//   double * X_pr = new double [80];
//   double* X_sr= new double [80];



  if (0) {
    for (int iTCIdm = 0; iTCIdm < 576; iTCIdm++) {
      if (TCEnergy_tot[iTCIdm] < cut_energy_tot) { continue; }   // TC energy_tot cut
      for (int jjj = 0; jjj < bkg_level; jjj++) {
        ttt0 = -(tmin_noise + tgen * gRandom->Rndm());  //@ [us]
        ttt1 = -(tmin_noise + tgen * gRandom->Rndm());  //@ [us]
        ttt2 = -(tmin_noise + tgen * gRandom->Rndm());  //@ [us]
        for (int iSampling = 0; iSampling < NSampling; iSampling++) {
          //@ (pile-up noise)
          if (ttt0 > 0) { TCDigiE[iTCIdm][iSampling] += SimplifiedFADC(0, ttt0) * corr_pileup * 0.001; }
          //@ (parallel noise)
          if (ttt1 > 0) { TCDigiE[iTCIdm][iSampling] += SimplifiedFADC(1, ttt1) * corr_parallel; }
          //@ (serial noise)
          if (ttt2 > 0) { TCDigiE[iTCIdm][iSampling] += SimplifiedFADC(2, ttt2) * corr_serial; }
          ttt0 += fam_sampling_interval * 0.001;
          ttt1 += fam_sampling_interval * 0.001;
          ttt2 += fam_sampling_interval * 0.001;
        }
      }
    }
  }
  if (1) { //use L Matrix
    for (int iTCIdm = 0; iTCIdm < 576; iTCIdm++) {

      if (TCEnergy_tot[iTCIdm] < cut_energy_tot) { continue; }   // TC energy_tot cut

      for (int iSampling = 0; iSampling < NSampling; iSampling++) {
        X_pr[iSampling] = gRandom ->Gaus(0, 1);
        X_sr[iSampling] = gRandom ->Gaus(0, 1);
      }

      for (int iSampling = 0; iSampling < NSampling; iSampling++) {
        for (int jSampling = 0; jSampling < NSampling; jSampling++) {
          noise_parallel[iTCIdm][iSampling] += 10 * corr_parallel * MatrixParallel[iSampling][jSampling] * X_pr[jSampling];
          noise_serial[iTCIdm][iSampling] += 10 * corr_serial * MatrixSerial[iSampling][jSampling] * X_sr[jSampling];
        }
      }

      for (int iSampling = 0; iSampling < NSampling; iSampling++) {

        TCDigiE[iTCIdm][iSampling] += noise_pileup[iTCIdm][iSampling] + noise_parallel[iTCIdm][iSampling] +
                                      noise_serial[iTCIdm][iSampling];
      }
    }
    for (int iTCIdm = 0; iTCIdm < 576; iTCIdm++) { //Only Pile-up noise use old method.
      if (TCEnergy_tot[iTCIdm] < cut_energy_tot) { continue; }   // TC energy_tot cut
      for (int jjj = 0; jjj < bkg_level; jjj++) {
        ttt0 = -(tmin_noise + tgen * gRandom->Rndm()); // [us]
        for (int iSampling = 0; iSampling < NSampling; iSampling++) {
          // (pile-up noise)
          if (ttt0 > 0) { TCDigiE[iTCIdm][iSampling] += SimplifiedFADC(0, ttt0) * corr_pileup * 0.001; }
          ttt0 += fam_sampling_interval * 0.001;
        }
      }
    }
  }


  //==================
  // (02)Peak search
  //==================
  //@ T_a and T_b is time at sampling points in which 0.6*E exists.
  int ta_id[20] = {1000}; //@ id of T_a
  double ttt_a[20] = {0}; //@ time of T_a
  double ttt_b[20] = {0}; //@ time of T_b
  for (int iTCIdm = 0; iTCIdm < 576; iTCIdm++) {
    noutput[iTCIdm] = 0;
    if (TCEnergy_tot[iTCIdm] < cut_energy_tot) { continue; }   // TC energy_tot cut


    double threshold = 0.05; //GeV
    int maxId[500] = {0};
    int count_up = 0;
    int count_down = 0;
    int flag_up = 3;
    int flag_down = 3;

    double max = 0;
    for (int iSampling = 1; iSampling < NSampling; iSampling++) {
      //-------------------------------------------------------------------------
      //Peak finding Method 1
      //------------------------------------------------------------------------

      if (TCDigiE[iTCIdm][iSampling] >= max) {

        max = TCDigiE[iTCIdm][iSampling];
        maxId[noutput[iTCIdm]] = iSampling;
        count_up ++;
        count_down = 0;
      } else {
        count_down++;
        if (count_down >= flag_down) {
          if (count_up >= flag_up) {
            if (threshold < max) {
              max = 0;
              count_up = 0;
              count_down = 0;

              double NoiseLevel = 0;
              double NoiseCount = 0;
              for (int iNoise = 0; iNoise < 5; iNoise++) {
                int iNoiseReplace = (maxId[noutput[iTCIdm]] - 10) + iNoise;
                if (iNoiseReplace >= 0) {
                  NoiseLevel += TCDigiE[iTCIdm][iNoiseReplace];
                  NoiseCount++;
                }
              }
              if (NoiseCount != 0) { NoiseLevel /= NoiseCount; }
              TCFitEnergy[iTCIdm][noutput[iTCIdm]] = TCDigiE[iTCIdm][maxId[noutput[iTCIdm]]] - NoiseLevel;
              if (!(maxId[noutput[iTCIdm]] - 1)) {
                for (int iSampling = 1; iSampling < maxId[noutput[iTCIdm]] + 3; iSampling++) {
                  TCDigiE[iTCIdm][iSampling] -= NoiseLevel;
                }
              } else {
                for (int iSampling = maxId[noutput[iTCIdm] - 1]; iSampling < maxId[noutput[iTCIdm]] + 3; iSampling++) {
                  TCDigiE[iTCIdm][iSampling] -= NoiseLevel;
                }
              }
              //@ Search T_a ID
              for (int iSearch = 0; iSearch < 5; iSearch++) {

                if (TCDigiE[iTCIdm][maxId[noutput[iTCIdm]] - iSearch]  > 0.6 * TCFitEnergy[iTCIdm][noutput[iTCIdm]] &&
                    TCDigiE[iTCIdm][maxId[noutput[iTCIdm]] - iSearch - 1] < 0.6 * TCFitEnergy[iTCIdm][noutput[iTCIdm]]) {
                  ta_id[noutput[iTCIdm]] = maxId[noutput[iTCIdm]] - iSearch - 1;
                }
              }

              //@ Estimate timing of t0
              if (ta_id[noutput[iTCIdm]] == 1000) {
                printf("TrgEclFAM::digi02> Cannot find TC Timing (TCId=%5i, E=%8.5f)!!!\n", iTCIdm - 1, TCFitEnergy[iTCIdm][0]);
                B2ERROR("TrgEclFAM::digi02> Cannot find TC Timing");
              } else {
                ttt_a[noutput[iTCIdm]] = TCDigiT[iTCIdm][ta_id[noutput[iTCIdm]]];
                ttt_b[noutput[iTCIdm]] = TCDigiT[iTCIdm][ta_id[noutput[iTCIdm]] + 1];
                TCFitTiming[iTCIdm][noutput[iTCIdm]] =
                  ttt_a[noutput[iTCIdm]] +
                  (0.6 * TCFitEnergy[iTCIdm][noutput[iTCIdm]] - TCDigiE[iTCIdm][ta_id[noutput[iTCIdm]]]) * (ttt_b[noutput[iTCIdm]] -
                      ttt_a[noutput[iTCIdm]])
                  / (TCDigiE[iTCIdm][ta_id[noutput[iTCIdm]] + 1] - TCDigiE[iTCIdm][ta_id[noutput[iTCIdm]]]);
                //@ time between t0 and 0.6*peak_energy
                //@ Alex's number = 274.4 (how he got this value ?)
                //@ by my check = 278.7 [ns]
                TCFitTiming[iTCIdm][noutput[iTCIdm]] -= (278.7 + 2) + GetTCLatency(iTCIdm);
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
  /*
  delete [] X_pr;
  delete [] X_sr;
  delete [] noise_pileup;
  delete [] noise_serial;
  delete [] noise_parallel;
  */

}
//
//
//
void
TrgEclFAM::digitization03(void)
{
  //===============
  // (03)Signal digitization (w/ 12ns interval for method-0)
  //===============
  double cut_energy_tot = 0.03; // [GeV]
  int nbin_pedestal = 100;
  float fam_sampling_interval = 12; // [ns]

  std::vector< std::vector<float> > TCDigiEnergy(576, std::vector<float>(666, 0.0));    // [GeV]
  std::vector< std::vector<float> > TCDigiTiming(576, std::vector<float>(666, 0.0));    // [ns]
  /*
  float (*TCDigiEnergy)[666] = new  float [576][666];  // [GeV]
  float (*TCDigiTiming)[666] = new float [576][666];  // [ns]

  for (int iTCIdm = 0; iTCIdm < 576; iTCIdm++) {
    for (int iSampling = 0; iSampling < 666; iSampling++) {

    }
  }
  */




  int NSampling = 666;

  // Make sampling time random between FAM sampling intervals
  float random_sampling_correction = 0; // [ns]
  random_sampling_correction = gRandom->Rndm() * fam_sampling_interval;
  for (int iTCIdm = 0; iTCIdm < 576; iTCIdm++) {
    if (TCEnergy_tot[iTCIdm] < cut_energy_tot) {continue;} // TC energy_tot cut
    for (int iTimeBin = 0; iTimeBin < 80; iTimeBin++) {
      if (TCEnergy[iTCIdm][iTimeBin] < 0.0001) { continue; }   // 0.1 MeV cut on TC bin_energy
      for (int iSampling = 0; iSampling < NSampling; iSampling++) {
        // inputTiming is in [us] <-- Be careful, here is NOT [ns]
        float inputTiming
          = (-TCTiming[iTCIdm][iTimeBin] - TimeRange + (-nbin_pedestal + iSampling) * fam_sampling_interval) * 0.001;
        inputTiming += random_sampling_correction * 0.001;
        TCDigiEnergy[iTCIdm][iSampling] += FADC(0, inputTiming) * TCEnergy[iTCIdm][iTimeBin];
      }
    }
    for (int iSampling = 0; iSampling < NSampling; iSampling++) {
      TCDigiTiming[iTCIdm][iSampling] = (-nbin_pedestal + iSampling - TimeRange / fam_sampling_interval) * fam_sampling_interval;
      TCDigiTiming[iTCIdm][iSampling] += random_sampling_correction;
    }
  }
  //==================
  // (03)noise embedding
  //==================
  // double tmin_noise = -nbin_pedestal * fam_sampling_interval * 0.001; // [us]
  //  double tgen = NSampling * fam_sampling_interval * 0.001 - tmin_noise; // [us]
  double tmin_noise = -4; // orignal
  double   tgen = 10.3;   //
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
  double corr_pileup   = times_pileup   * frac_pileup   * sqrt(fam_sampling_interval * 0.001);
  double corr_parallel = times_parallel * frac_parallel * sqrt(fam_sampling_interval * 0.001);
  double corr_serial   = times_serial   * frac_serial   * sqrt(fam_sampling_interval * 0.001);
  corr_pileup   = 0.011068;
  corr_parallel = 0.00727324;
  corr_serial   = 0.0173925;


  for (int iTCIdm = 0; iTCIdm < 576; iTCIdm++) {
    if (TCEnergy_tot[iTCIdm] < cut_energy_tot) { continue; }   // 1 MeV TC energy cut
    for (int jjj = 0; jjj < bkg_level; jjj++) {
      ttt0 = -(tmin_noise + tgen * gRandom->Rndm());  // [us]
      ttt1 = -(tmin_noise + tgen * gRandom->Rndm());  // [us]
      ttt2 = -(tmin_noise + tgen * gRandom->Rndm());  // [us]
      for (int iSampling = 0; iSampling < NSampling; iSampling++) {
        // (pile-up noise)
        if (ttt0 > 0) { TCDigiEnergy[iTCIdm][iSampling] += FADC(0, ttt0) * corr_pileup * 0.001; }
        // (parallel noise)
        if (ttt1 > 0) { TCDigiEnergy[iTCIdm][iSampling] += FADC(1, ttt1) * corr_parallel; }
        // (serial noise)
        if (ttt2 > 0) { TCDigiEnergy[iTCIdm][iSampling] += FADC(2, ttt2) * corr_serial; }
        ttt0 += fam_sampling_interval * 0.001;
        ttt1 += fam_sampling_interval * 0.001;
        ttt2 += fam_sampling_interval * 0.001;
      }
    }
  }



  //
  //
  //
  if (0) {
    FILE* f_out_dat;
    f_out_dat = fopen("ztsim.dat", "w");
    for (int iTCIdm = 0; iTCIdm < 576; iTCIdm++) {
      if (TCEnergy[iTCIdm][0] < 0.001) { continue; }   // 1 MeV TC energy cut
      fprintf(f_out_dat, "%5i %8.5f %8.5f ",
              iTCIdm + 1, TCEnergy[iTCIdm][0], TCTiming[iTCIdm][0]);
      for (int iSampling = 0; iSampling < NSampling; iSampling++) {
        fprintf(f_out_dat, "%3i %8.5f\n", iSampling, TCDigiEnergy[iTCIdm][iSampling]);
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
    noutput[iTCIdm] = 0;
    if (TCEnergy_tot[iTCIdm] < cut_energy_tot) { continue; }   // TC energy_tot cut
    int maxId[500] = {0};
    int count_up = 0;
    int count_down = 0;
    int flag_up = 30;;
    int flag_down = 40;
    double max = 0;
    for (int iSampling = 1; iSampling < NSampling; iSampling++) {

      if (TCDigiEnergy[iTCIdm][iSampling] >= max) {

        max = TCDigiEnergy[iTCIdm][iSampling];
        maxId[noutput[iTCIdm]] = iSampling;
        count_up ++;
        count_down = 0;
      } else {
        count_down++;
        if (count_down >= flag_down) {
          if (count_up >= flag_up) {
            if (threshold < max) {
              max = 0;
              count_up = 0;
              count_down = 0;
              //@ Remove noise effect
              float NoiseLevel = 0;
              float NoiseCount = 0;
              for (int iNoise = 0; iNoise < 42; iNoise++) {
                int iNoiseReplace = (maxId[noutput[iTCIdm]] - 88) + iNoise;
                if (iNoiseReplace >= 0) {
                  NoiseLevel += TCDigiEnergy[iTCIdm][iNoiseReplace];
                  NoiseCount++;
                }
              }
              if (NoiseCount != 0) { NoiseLevel /= NoiseCount; }
              TCFitEnergy[iTCIdm][noutput[iTCIdm]] = TCDigiEnergy[iTCIdm][maxId[noutput[iTCIdm]]] - NoiseLevel;
              TCFitTiming[iTCIdm][noutput[iTCIdm]] = TCDigiTiming[iTCIdm][maxId[noutput[iTCIdm]]] - max_shape_time + GetTCLatency(iTCIdm);
              noutput[iTCIdm]++;
            }
          }
        }
      }
    }
  }

  /*
  delete []  TCDigiEnergy;
  delete []  TCDigiTiming;
  */


  return;
}
//
//
//
void
TrgEclFAM::save(int m_nEvent)
{
  //---------------
  // Root Output
  //---------------
  int m_hitNum = 0;
  for (int iTCIdm = 0; iTCIdm < 576; iTCIdm++) {
    for (int ininput = 0; ininput < ninput[iTCIdm] ; ininput++) {
      if (TCFitEnergy[iTCIdm][ininput] < 0.05) {
        continue;  // 50 MeV Fitted TC energy cut
      }
      if (TCFitEnergy[iTCIdm][ininput] == 0 && TCFitTiming[iTCIdm][ininput] == 0) {
        continue;
      }
      if (noutput[iTCIdm] != ninput[iTCIdm]) {continue;}
      StoreArray<TRGECLDigi> TCDigiArray;
      TCDigiArray.appendNew();
      m_hitNum = TCDigiArray.getEntries() - 1;
      TCDigiArray[m_hitNum]->setEventId(m_nEvent);
      TCDigiArray[m_hitNum]->setTCId(iTCIdm + 1);
      TCDigiArray[m_hitNum]->setTCnOutput(ininput + 1);
      TCDigiArray[m_hitNum]->setPhiId(_TCMap->getTCPhiIdFromTCId(iTCIdm + 1));
      TCDigiArray[m_hitNum]->setThetaId(_TCMap->getTCThetaIdFromTCId(iTCIdm + 1));
      TCDigiArray[m_hitNum]->setRawEnergy(TCRawEnergy[iTCIdm][ininput]);
      TCDigiArray[m_hitNum]->setRawTiming(TCRawTiming[iTCIdm][ininput]);
      TCDigiArray[m_hitNum]->setRawEnergyTot(TCEnergy_tot[iTCIdm]);
      TCDigiArray[m_hitNum]->setRawTimingTot(TCTiming_tot[iTCIdm]);
      TCDigiArray[m_hitNum]->setFitEnergy(TCFitEnergy[iTCIdm][ininput]);
      TCDigiArray[m_hitNum]->setFitTiming(TCFitTiming[iTCIdm][ininput]);
      TCDigiArray[m_hitNum]->setBeamBkgTag(TCRawBkgTag[iTCIdm][ininput]);

    }
  }
  m_hitNum = 0;
  for (int iTCIdm = 0; iTCIdm < 576; iTCIdm++) {
    for (int iBinTime = 0; iBinTime < 80; iBinTime++) {
      if (TCEnergy[iTCIdm][iBinTime] < 0.001) {continue;}
      StoreArray<TRGECLDigi0> TCDigiArray;
      TCDigiArray.appendNew();
      m_hitNum = TCDigiArray.getEntries() - 1;

      TCDigiArray[m_hitNum]->setEventId(m_nEvent);
      TCDigiArray[m_hitNum]->setTCId(iTCIdm + 1);
      TCDigiArray[m_hitNum]->setiBinTime(iBinTime);
      TCDigiArray[m_hitNum]->setRawEnergy(TCEnergy[iTCIdm][iBinTime]);
      TCDigiArray[m_hitNum]->setRawTiming(TCTiming[iTCIdm][iBinTime]);
      TCDigiArray[m_hitNum]->setBeamBkgTag(TCBeambkgTag[iTCIdm][iBinTime]);

    }
  }
  m_hitNum = 0;

  for (int iTCIdm = 0; iTCIdm < 576;  iTCIdm++) {
    for (int inoutput = 0; inoutput < noutput[iTCIdm] ; inoutput++) {
      if (TCFitEnergy[iTCIdm][inoutput] < 0.001) {
        continue;  // 50 MeV Fitted TC energy cut
      }
      if (TCFitEnergy[iTCIdm][inoutput] == 0 && TCFitTiming[iTCIdm][inoutput] == 0) {
        continue;
      }
      int bkgtag = 0;


      for (int iBinTime = 0; iBinTime < 80; iBinTime++) {
        if (TCEnergy[iTCIdm][iBinTime] < 0.1) {continue;}
        if (abs(TCEnergy[iTCIdm][iBinTime] - TCFitEnergy[iTCIdm][inoutput]) < 0.01
            && abs(TCTiming[iTCIdm][iBinTime] - TCFitTiming[iTCIdm][inoutput]) < 100.) {
          bkgtag = TCBeambkgTag[iTCIdm][iBinTime];
        }
      }

      if (bkgtag == 0) {
        if (TCFitTiming[iTCIdm][inoutput] < -50 || TCFitTiming[iTCIdm][inoutput] > 200) {
          bkgtag = 999;
        }

      }




      StoreArray<TRGECLHit> TrgEclHitArray;

      TrgEclHitArray.appendNew();
      m_hitNum = TrgEclHitArray.getEntries() - 1;
      TrgEclHitArray[m_hitNum]->setEventId(m_nEvent);
      TrgEclHitArray[m_hitNum]->setTCId(iTCIdm + 1);
      TrgEclHitArray[m_hitNum]->setTCOutput(inoutput + 1);
      TrgEclHitArray[m_hitNum]->setEnergyDep(TCFitEnergy[iTCIdm][inoutput]);
      TrgEclHitArray[m_hitNum] ->setTimeAve(TCFitTiming[iTCIdm][inoutput]);
      TrgEclHitArray[m_hitNum]->setBeamBkgTag(bkgtag);

    }
  }
  // for (int iTCIdm = 0; iTCIdm < 576; iTCIdm++) {

  //   _tcnoutput.push_back(noutput[iTCIdm]);

  //   _tcid.push_back(iTCIdm + 1);

  //   _tcthetaid.push_back(_TCMap->getTCThetaIdFromTCId(iTCIdm + 1));

  //   _tcphiid.push_back(_TCMap->getTCPhiIdFromTCId(iTCIdm + 1));

  // }
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
                double timing)
{

  //--------------------------------------
  //
  // o "timing" unit is [us]
  // o flag_gen = 0(=signal), 1(=parallel), 2(=serial)
  // o return value(PDF) is [GeV]
  //
  //--------------------------------------
  double tsh, dd;
  // static double tc, fm, fff, tt, dt, tc2, tm, tsc, tris;
//   static double amp, td, t1, b1, t2, b2, ts, dft, as;
  static double tc, tc2, tsc, tris, b1, b2;
  static double amp, td, t1, t2,  dft, as;


  static int ifir = 0;

  if (ifir == 0) {

    td =  0.10;  // diff time    (  0.10)
    t1 =  0.10;  // integ1 real  (  0.10)
    b1 = 30.90;  // integ1 imag  ( 30.90)
    t2 =  0.01;  // integ2 real  (  0.01)
    b2 = 30.01;  // integ2 imag  ( 30.01)
    double    ts =  1.00;  // scint decay  (  1.00)
    dft = 0.600; // diff delay   ( 0.600)
    as = 0.548;  // diff frac    ( 0.548)
    //
    amp = 1.0;
    tris = 0.01;
    tsc = ts;
    //
    int im = 0;
    int ij = 0;
    int fm = 0;
    tc = 0;
    double tt = u_max(u_max(td, t1), u_max(t2, ts)) * 2;
    int flag_once = 0;
    while (flag_once == 0) {
      double   dt = tt / 1000;
      double tm = 0;
      for (int j = 1; j <= 1000; j++) {
        tc2 = tc - dft;
        double  fff =
          (ShapeF(tc, t1, b1, t2, b2, td, tsc) -
           ShapeF(tc, t1, b1, t2, b2, td, tris) * 0.01) -
          (ShapeF(tc2, t1, b1, t2, b2, td, tsc) -
           ShapeF(tc2, t1, b1, t2, b2, td, tris) * 0.01) * as;
        if (fff > fm) {
          fm = fff;
          tm = tc;
          im = j;
        }
        tc = tc + dt;
      }
      if (im >= 1000) {
        tt = 2 * tt;
        flag_once = 0;
        continue;
      }
      if (ij == 0) {
        ij = 1;
        tc = 0.99 * tm;
        dt = tm * 0.02 / 1000;
        flag_once = 0;
        continue;
      }
      flag_once = 1;
    }
    amp = 1.0 / fm;
    ifir = 1;
  }
  //
  //
  double pdf = 0;
  if (flag_gen == 0) {
    //-----<signal>
    tc2 = timing - dft;
    pdf = amp * (
            (ShapeF(timing,  t1, b1, t2, b2, td, tsc) -
             ShapeF(timing,  t1, b1, t2, b2, td, tris) * 0.01) -
            (ShapeF(tc2,       t1, b1, t2, b2, td, tsc) -
             ShapeF(tc2,       t1, b1, t2, b2, td, tris) * 0.01) * as);
  } else if (flag_gen == 1) {
    //-----<parallel>
    tc2 = timing - dft;
    tsh = 0.001;
    pdf = amp * (
            ShapeF(timing,  t1, b1, t2, b2, td, tsh) -
            ShapeF(tc2,       t1, b1, t2, b2, td, tsh) * as);
    pdf = pdf * 0.001; // GeV
  } else {
    //-----<serial>
    tc2 = timing - dft;
    tsh = 0.001;
    pdf = amp * (
            ShapeF(timing,  t1, b1, t2, b2, td, tsh) -
            ShapeF(tc2,       t1, b1, t2, b2, td, tsh) * as);
    //
    tc = timing - 0.01;
    if (tc < 0) { tc = 0; }
    dd = timing - tc;
    tc2 = tc - dft;
    pdf = (amp * (
             ShapeF(tc, t1, b1, t2, b2, td, tsh) -
             ShapeF(tc2, t1, b1, t2, b2, td, tsh) * as) - pdf) / dd;
    pdf = pdf * 0.001; // GeV
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
                  double ts1)
{
  double dr;
  double dzna;
  double das1, das0, dac0;
  double dcs0s, dsn0s, dcs0d, dsn0d;
  double dcs1s, dsn1s, dcs1d, dsn1d;

  double sv123 = 0.0;
  if (t00 < 0) return 0;

  dr = (ts1 - td1) / td1;
  if (fabs(dr) <= 1.0e-5) {
    if (ts1 > td1) { ts1 = td1 * 1.00001; }
    else         { ts1 = td1 * 0.99999; }
  }
  //
  dr = (pow((t01 - t02), 2) + pow((tb1 - tb2), 2)) / (pow((t01), 2) + pow((tb1), 2));
  if (dr <= 1.e-10) {
    if (t01 < t02) { t01 = t02 / 1.00001; }
    else         { t01 = t02 / 0.99999; }
  }
  if (t00 <= 0.0) return 0;
  //
  //
  //
  double a1 = 1 / t01;
  double a2 = 1 / tb1;
  double b1 = 1 / t02;
  double b2 = 1 / tb2;
  double c1 = 1 / td1;
  double c2 = 1 / ts1;

  das0 = b2 * (pow((b1 - a1), 2) + (b2 + a2) * (b2 - a2));
  dac0 = -2 * (b1 - a1) * a2 * b2;
  das1 = a2 * (pow((b1 - a1), 2) - (b2 + a2) * (b2 - a2));

  dsn0s = ((c2 - a1) * das0  - (-a2) * dac0)   / (pow(a2, 2) + pow((c2 - a1), 2));
  dcs0s = ((-a2) * das0 + (c2 - a1) * dac0)   / (pow(a2, 2) + pow((c2 - a1), 2));

  dsn1s = ((c2 - b1) * das1  - (-b2) * (-dac0)) / (pow(b2, 2) + pow((c2 - b1), 2));
  dcs1s = ((-b2) * das1 + (c2 - b1) * (-dac0)) / (pow(b2, 2) + pow((c2 - b1), 2));

  dsn0d = ((c1 - a1) * das0  - (-a2) * dac0)   / (pow(a2, 2) + pow((c1 - a1), 2));
  dcs0d = ((-a2) * das0 + (c1 - a1) * dac0)   / (pow(a2, 2) + pow((c1 - a1), 2));

  dsn1d = ((c1 - b1) * das1  - (-b2) * (-dac0)) / (pow(b2, 2) + pow((c1 - b1), 2));
  dcs1d = ((-b2) * das1 + (c1 - b1) * (-dac0)) / (pow(b2, 2) + pow((c1 - b1), 2));
  //
  //
  //
  dzna = (pow((b1 - a1), 2) + pow((b2 - a2), 2)) * (pow((b1 - a1), 2) + pow((a2 + b2), 2));

  sv123 = (
            (dcs0s + dcs1s) * exp(-c2 * t00) * (-1) +
            (dcs0d + dcs1d) * exp(-c1 * t00) +
            ((dsn0s - dsn0d) * sin(a2 * t00) + (dcs0s - dcs0d) * cos(a2 * t00)) * exp(-a1 * t00) +
            ((dsn1s - dsn1d) * sin(b2 * t00) + (dcs1s - dcs1d) * cos(b2 * t00)) * exp(-b1 * t00)
          )
          / dzna / (1 / c2 - 1 / c1);

  return sv123;
}

//
//
//


double
TrgEclFAM::SimplifiedFADC(int flag_gen,
                          double timing)
{

  //--------------------------------------
  //
  // o "timing" unit is [us]
  // o flag_gen = 0(=signal), 1(=parallel), 2(=serial)
  // o return value(PDF) is [GeV]
  // o Generate signal shape using a simplified function.
  //
  //
  //--------------------------------------
  double tsh, dd;
//   static double tc, fm, fff, tt, dt, tc2, tm, tsc, tris;
//   static double amp, td, t1, t2, ts, dft, as;
  static double tc, tc2, tsc, tris;
  static double amp, td, t1, t2,  dft, as;


  //  int im, ij;

  static int ifir = 0;

  if (ifir == 0) {

    td =  0.10;  // diff time    (  0.10)
    t1 =  0.10;  // integ1 real  (  0.10)
    // b1 = 30.90;  // integ1 imag  ( 30.90)
    t2 =  0.01;  // integ2 real  (  0.01)
    // b2 = 30.01;  // integ2 imag  ( 30.01)
    double ts =  1.00;  // scint decay  (  1.00)
    dft = 0.600; // diff delay   ( 0.600)
    as = 0.548;  // diff frac    ( 0.548)
    //
    amp = 1.0;
    tris = 0.01;
    // ts0 = 0.0;
    tsc = ts;
    double    fm = 0;
    //
    int  im = 0;
    int  ij = 0;
    tc = 0;
    double tt = u_max(u_max(td, t1), u_max(t2, ts)) * 2;
    int flag_once = 0;
    while (flag_once == 0) {
      double dt = tt / 1000;

      double tm = 0;
      for (int j = 1; j <= 1000; j++) {
        tc2 = tc - dft;
        double fff =
          (ShapeF(tc, tsc) -
           ShapeF(tc, tris) * 0.01) -
          (ShapeF(tc2, tsc) -
           ShapeF(tc2, tris) * 0.01) * as;
        if (fff > fm) {
          fm = fff;
          tm = tc;
          im = j;
        }
        tc = tc + dt;
      }
      if (im >= 1000) {
        tt = 2 * tt;
        flag_once = 0;
        continue;
      }
      if (ij == 0) {
        ij = 1;
        tc = 0.99 * tm;
        dt = tm * 0.02 / 1000;
        flag_once = 0;
        continue;
      }
      flag_once = 1;
    }
    amp = 1.0 / fm;
    ifir = 1;
  }
  //
  //
  double pdf = 0;
  if (flag_gen == 0) {
    //-----<signal>
    tc2 = timing - dft;
    pdf = amp * (
            (ShapeF(timing, tsc) -
             ShapeF(timing, tris) * 0.01) -
            (ShapeF(tc2,    tsc) -
             ShapeF(tc2,      tris) * 0.01) * as);
  } else if (flag_gen == 1) {
    //-----<parallel>
    tc2 = timing - dft;
    tsh = 0.001;
    pdf = amp * (
            ShapeF(timing, tsh) -
            ShapeF(tc2,     tsh) * as);
    pdf = pdf * 0.001; // GeV
  } else {
    //-----<serial>
    tc2 = timing - dft;
    tsh = 0.001;
    pdf = amp * (
            ShapeF(timing, tsh) -
            ShapeF(tc2,    tsh) * as);
    //
    tc = timing - 0.01;
    if (tc < 0) { tc = 0; }
    dd = timing - tc;
    tc2 = tc - dft;
    pdf = (amp * (
             ShapeF(tc, tsh) -
             ShapeF(tc2, tsh) * as) - pdf) / dd;
    pdf = pdf * 0.001; // GeV
  }

  return pdf;
}


double TrgEclFAM::ShapeF(double t00, double ts1)
{

  double dzna;
  // double das1,das0,dac0;
  double dcs0s, dsn0s, dcs0d, dsn0d;
  double dcs1s, dsn1s, dcs1d, dsn1d;
  double a1, a2, b1, b2, c1, c2;
  double sv123 = 0.0;

  if (t00 <= 0.0) return 0;



  a1 = 10 ;
  a2 =  0.0323625 ;
  b1 = 100;
  b2 = 0.0333222;
  c1 = 10;

  // das0 =  269.91;
  //dac0 = -0.1941;
  // das1 =  262.136;
  if (ts1 == 1) {
    c2 = 1;
    dsn0s =  -29.9897;
    dcs0s =   -0.08627;

    dsn1s =  -2.64784;
    dcs1s =  -0.00285194;
  }
  if (ts1 == 0.01) {
    c2 = 100;
    dsn0s =  2.999 ;
    dcs0s =   -0.00323517;

    dsn1s =  5.82524;
    dcs1s = -7866.7;
  }
  if (ts1 == 0.001) {
    c2 = 1000;
    dsn0s = 0.272636;
    dcs0s = -0.000204983;

    dsn1s = 0.291262;
    dcs1s = 0.000204894;
  }

  dsn0d =  -5.998;
  dcs0d =   -8340.22;

  dsn1d =  -2.91262;
  dcs1d = -0.00323517;
  //
  //
  //
  dzna = 6.561e+07;








  sv123 = (
            (dcs0s + dcs1s) * exp(-c2 * t00) * (-1) +
            (dcs0d + dcs1d) * exp(-c1 * t00) +
            ((dsn0s - dsn0d) * sin(a2 * t00) + (dcs0s - dcs0d) * cos(a2 * t00)) * exp(-a1 * t00) +
            ((dsn1s - dsn1d) * sin(b2 * t00) + (dcs1s - dcs1d) * cos(b2 * t00)) * exp(-b1 * t00)
          )
          / dzna / (1 / c2 - 1 / c1);


  return sv123;
}



//
//
//
double
TrgEclFAM::u_max(double aaa, double bbb)
{

  if (aaa > bbb) { return aaa; }
  else        { return bbb; }
}
//
//

void
TrgEclFAM::FAMFit(int nbin_pedestal,
                  int NSampling,
                  int iTCIdm,
                  double fam_sampling_interval,
                  double digiEnergy[])
{
  //============================
  // In this function,
  // o Energy unit must be [MeV/c2]
  // o Time unit must be [us]
  // but
  // o return energy unit must be [GeV/c2]
  // o return time unit must be [ns]
  //============================
  double* TCFitSample = new double [12];  // MeV/c2
  double* preped = new double [4];

  int pedFlag = 0;
  double CoeffAAA = 0;
  double CoeffBBB = 0;
  double CoeffPPP = 0;
  int dTBin = 0;
  int ShiftdTBin = 0;
  int Nsmalldt = 10;
  int SmallOffset = 1;
  double IntervaldT  = fam_sampling_interval * 0.001 / Nsmalldt;
  double EThreshold = 100; //[GeV]
  int FitSleepCounter   = 100; // counter to suspend fit
  int FitSleepThreshold = 2;   // # of clk to suspend fit
  double FitE = 0;
  double FitT = 0;

  noutput[iTCIdm] = 0;

  for (int iShift = 23; iShift < (NSampling - 12); iShift++) { // In order to avoid BKG shaping effect, iShift start from 20.

    FitSleepCounter++;
    if (FitSleepCounter <= FitSleepThreshold) {continue;}
    for (int iFitSample = 0; iFitSample < 12; iFitSample++) {
      int iReplace = iFitSample + iShift;
      TCFitSample[iFitSample] = digiEnergy[iReplace] * 1000.0;
      if (0) {
        if (pedFlag == 1 && iFitSample < 4) {
          TCFitSample[iFitSample] = preped[iFitSample];
          pedFlag = 0;
        }
      }
    }
    //
    //
    //

    dTBin = (int)(ShiftdTBin + Nsmalldt);
    if (dTBin < 1)  {dTBin =   1;}
    if (dTBin > 20) {dTBin = 20;}

    CoeffAAA = 0;
    CoeffBBB = 0;
    CoeffPPP = 0;
    for (int iFitSample = 0; iFitSample < 12; iFitSample++) {
      CoeffAAA += CoeffNoise31[dTBin - 1][iFitSample] * TCFitSample[iFitSample];
      CoeffBBB += CoeffNoise32[dTBin - 1][iFitSample] * TCFitSample[iFitSample];
      CoeffPPP += CoeffNoise33[dTBin - 1][iFitSample] * TCFitSample[iFitSample];
    }
    double deltaT = CoeffBBB / CoeffAAA; // deltaT [us]

    ShiftdTBin = int(deltaT / IntervaldT + dTBin);

    FitE = CoeffAAA;

    //-------
    // Require "expected time" is around middle of table = Nsmalldt.
    //-------
    double condition_t = -(deltaT + dTBin * IntervaldT - fam_sampling_interval * 0.001);


    if (fabs(condition_t) < 0.8 * (fam_sampling_interval * 0.001) && FitE > EThreshold) {
      FitT = condition_t + (SmallOffset + iShift + nbin_pedestal - 5) * (fam_sampling_interval * 0.001);
      //  cout << FitT << endl;

      pedFlag = 1;
      if (0) {
        for (int jjj = 0; jjj < 30; jjj++) {
          double ttt = -condition_t + 0.001 * fam_sampling_interval * (jjj - 3.5);
          if (condition_t < 0.144) {ttt = -condition_t + 0.001 * fam_sampling_interval * (jjj - 4.5);}
          double sig = 0;

          sig = SimplifiedFADC(0, ttt);

          int itime = (int)(FitT / (fam_sampling_interval * 0.001));
          cout <<  digiEnergy[ itime + jjj] << "   "  << itime + jjj << "   " << FitE* sig * 0.001 << " " << "   " << condition_t << "    "  ;
          digiEnergy[itime + jjj] = digiEnergy[itime + jjj] - CoeffAAA * sig * 0.001;
          cout << digiEnergy[itime + jjj]  << endl;
// cout <<  endl;

        }
      }

      //--------
      //  double chi  = 0.0;
      //  for(int iii=0; iii<14; iii++){
      //    pp[iii] = noise_fl1[iii];
      //    pf[iii] = p1r + CoeffAAA * f0[iii][i51] + b1r * f1[iii][i51];
      //    if(iii<bin14) {
      //      chi = chi + fabs(pp[iii]-pf[iii]) * fabs(pp[iii]-pf[iii]);
      //    }
      //  }
      //--------
      //  break;



      returnE[noutput[iTCIdm]] = FitE / 1000.0; // [GeV/c2]
      returnT[noutput[iTCIdm]] = FitT * 1000 - TimeRange;
      //cout << FitT * 1000 - TimeRange <<endl;
      noutput[iTCIdm]++;
      FitSleepCounter = 0;
      ShiftdTBin = 0;

    }
  }
  //
  //
  //
  delete [] TCFitSample ;
  delete [] preped;
  return;
}


void TrgEclFAM:: readNoiseLMatrix()
{
  //double   L
  MatrixParallel = { { 0.00222966,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  },
    { 0.00175567,  0.0013744,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  },
    { 0.000978697,  0.00159799,  0.00120828,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  },
    { 0.000411268,  0.00106236,  0.00150164,  0.00119109,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  },
    { 0.000104886,  0.000533208,  0.00101586,  0.00149401,  0.00118828,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  },
    { 1.6782e-05,  0.000148717,  0.000548641,  0.00100194,  0.00149732,  0.00118411,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  },
    { 6.3743e-06,  1.90824e-05,  0.000163147,  0.000544966,  0.00100262,  0.00149888,  0.0011813,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  },
    { 2.35494e-06,  7.33264e-06,  1.93629e-05,  0.000164577,  0.000544718,  0.00100488,  0.00149767,  0.00118081,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  },
    { 8.45505e-07,  2.74031e-06,  7.45357e-06,  1.9282e-05,  0.000164886,  0.000545786,  0.00100482,  0.00149732,  0.00118077,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  },
    { 2.96716e-07,  9.92614e-07,  2.7925e-06,  7.42397e-06,  1.92962e-05,  0.000165394,  0.000546186,  0.00100452,  0.00149734,  0.00118075,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  },
    { 1.02205e-07,  3.50798e-07,  1.01349e-06,  2.78241e-06,  7.42938e-06,  1.93363e-05,  0.000165698,  0.000546122,  0.00100451,  0.00149735,  0.00118072,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  },
    { 3.46628e-08,  1.21525e-07,  3.58736e-07,  1.01011e-06,  2.78446e-06,  7.44503e-06,  1.93478e-05,  0.000165743,  0.000546108,  0.00100453,  0.00149734,  0.00118072,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  },
    { 1.16028e-08,  4.1411e-08,  1.24434e-07,  3.57618e-07,  1.01087e-06,  2.79044e-06,  7.44973e-06,  1.93465e-05,  0.000165746,  0.000546112,  0.00100453,  0.00149734,  0.00118072,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  },
    { 3.84053e-09,  1.39171e-08,  4.24471e-08,  1.24068e-07,  3.57887e-07,  1.01307e-06,  2.79233e-06,  7.44926e-06,  1.93464e-05,  0.00016575,  0.000546116,  0.00100453,  0.00149734,  0.00118072,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  },
    { 1.25893e-09,  4.62224e-09,  1.42781e-08,  4.23287e-08,  1.24163e-07,  3.58676e-07,  1.01379e-06,  2.79218e-06,  7.44922e-06,  1.93466e-05,  0.000165752,  0.000546116,  0.00100453,  0.00149734,  0.00118072,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  },
    { 4.09187e-10,  1.51963e-09,  4.74579e-09,  1.424e-08,  4.23611e-08,  1.24439e-07,  3.58941e-07,  1.01374e-06,  2.79217e-06,  7.44932e-06,  1.93467e-05,  0.000165753,  0.000546116,  0.00100453,  0.00149734,  0.00118072,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  },
    { 1.32005e-10,  4.95189e-10,  1.56129e-09,  4.73367e-09,  1.4251e-08,  4.2456e-08,  1.24533e-07,  3.58927e-07,  1.01374e-06,  2.79221e-06,  7.44937e-06,  1.93467e-05,  0.000165753,  0.000546116,  0.00100453,  0.00149734,  0.00118072,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  },
    { 4.23033e-11,  1.60111e-10,  5.09061e-10,  1.55744e-09,  4.73733e-09,  1.42831e-08,  4.24891e-08,  1.24529e-07,  3.58925e-07,  1.01375e-06,  2.79223e-06,  7.44937e-06,  1.93467e-05,  0.000165753,  0.000546116,  0.00100453,  0.00149734,  0.00118072,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  },
    { 1.34764e-11,  5.14127e-11,  1.6468e-10,  5.07849e-10,  1.55865e-09,  4.74806e-09,  1.42945e-08,  4.24878e-08,  1.24529e-07,  3.5893e-07,  1.01376e-06,  2.79223e-06,  7.44937e-06,  1.93467e-05,  0.000165753,  0.000546116,  0.00100453,  0.00149734,  0.00118072,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  },
    { 4.27025e-12,  1.64076e-11,  5.29043e-11,  1.643e-10,  5.08245e-10,  1.5622e-09,  4.7519e-09,  1.42941e-08,  4.24877e-08,  1.2453e-07,  3.58933e-07,  1.01376e-06,  2.79222e-06,  7.44937e-06,  1.93467e-05,  0.000165753,  0.000546116,  0.00100453,  0.00149734,  0.00118072,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  },
    { 1.34658e-12,  5.20739e-12,  1.68905e-11,  5.27855e-11,  1.64429e-10,  5.09406e-10,  1.56348e-09,  4.75179e-09,  1.42941e-08,  4.24882e-08,  1.24531e-07,  3.58933e-07,  1.01376e-06,  2.79222e-06,  7.44937e-06,  1.93467e-05,  0.000165753,  0.000546116,  0.00100453,  0.00149734,  0.00118072,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  },
    { 4.22769e-13,  1.64447e-12,  5.36263e-12,  1.68536e-11,  5.2827e-11,  1.64806e-10,  5.0983e-10,  1.56345e-09,  4.75177e-09,  1.42943e-08,  4.24886e-08,  1.24531e-07,  3.58933e-07,  1.01376e-06,  2.79222e-06,  7.44937e-06,  1.93467e-05,  0.000165753,  0.000546116,  0.00100453,  0.00149734,  0.00118072,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  },
    { 1.322e-13,  5.16973e-13,  1.69406e-12,  5.35117e-12,  1.68669e-11,  5.29484e-11,  1.64944e-10,  5.0982e-10,  1.56344e-09,  4.75184e-09,  1.42944e-08,  4.24886e-08,  1.24531e-07,  3.58933e-07,  1.01376e-06,  2.79222e-06,  7.44937e-06,  1.93467e-05,  0.000165753,  0.000546116,  0.00100453,  0.00149734,  0.00118072,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  },
    { 4.11877e-14,  1.61852e-13,  5.32724e-13,  1.69052e-12,  5.3554e-12,  1.69057e-11,  5.29933e-11,  1.64941e-10,  5.09818e-10,  1.56347e-09,  4.75188e-09,  1.42944e-08,  4.24886e-08,  1.24531e-07,  3.58933e-07,  1.01376e-06,  2.79222e-06,  7.44937e-06,  1.93467e-05,  0.000165753,  0.000546116,  0.00100453,  0.00149734,  0.00118072,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  },
    { 1.2789e-14,  5.04811e-14,  1.66829e-13,  5.31633e-13,  1.69186e-12,  5.36777e-12,  1.69202e-11,  5.29925e-11,  1.64941e-10,  5.09825e-10,  1.56348e-09,  4.75188e-09,  1.42944e-08,  4.24886e-08,  1.24531e-07,  3.58933e-07,  1.01376e-06,  2.79222e-06,  7.44937e-06,  1.93467e-05,  0.000165753,  0.000546116,  0.00100453,  0.00149734,  0.00118072,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  },
    { 3.95867e-15,  1.56904e-14,  5.20466e-14,  1.66494e-13,  5.32054e-13,  1.69578e-12,  5.37239e-12,  1.692e-11,  5.29923e-11,  1.64943e-10,  5.0983e-10,  1.56348e-09,  4.75188e-09,  1.42944e-08,  4.24886e-08,  1.24531e-07,  3.58933e-07,  1.01376e-06,  2.79222e-06,  7.44937e-06,  1.93467e-05,  0.000165753,  0.000546116,  0.00100453,  0.00149734,  0.00118072,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  },
    { 1.22183e-15,  4.86128e-15,  1.61807e-14,  5.19439e-14,  1.66626e-13,  5.33288e-13,  1.69725e-12,  5.37232e-12,  1.69199e-11,  5.2993e-11,  1.64944e-10,  5.09829e-10,  1.56348e-09,  4.75188e-09,  1.42944e-08,  4.24886e-08,  1.24531e-07,  3.58933e-07,  1.01376e-06,  2.79222e-06,  7.44937e-06,  1.93467e-05,  0.000165753,  0.000546116,  0.00100453,  0.00149734,  0.00118072,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  },
    { 3.76104e-16,  1.5017e-15,  5.01428e-15,  1.61493e-14,  5.19852e-14,  1.67013e-13,  5.33754e-13,  1.69723e-12,  5.37231e-12,  1.69201e-11,  5.29935e-11,  1.64944e-10,  5.09829e-10,  1.56348e-09,  4.75188e-09,  1.42944e-08,  4.24886e-08,  1.24531e-07,  3.58933e-07,  1.01376e-06,  2.79222e-06,  7.44937e-06,  1.93467e-05,  0.000165753,  0.000546116,  0.00100453,  0.00149734,  0.00118072,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  },
    { 1.15484e-16,  4.62624e-16,  1.54928e-15,  5.0047e-15,  1.61622e-14,  5.21062e-14,  1.6716e-13,  5.33748e-13,  1.69722e-12,  5.37238e-12,  1.69203e-11,  5.29935e-11,  1.64944e-10,  5.09829e-10,  1.56348e-09,  4.75188e-09,  1.42944e-08,  4.24886e-08,  1.24531e-07,  3.58933e-07,  1.01376e-06,  2.79222e-06,  7.44937e-06,  1.93467e-05,  0.000165753,  0.000546116,  0.00100453,  0.00149734,  0.00118072,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  },
    { 3.53776e-17,  1.42156e-16,  4.77368e-16,  1.54636e-15,  5.00869e-15,  1.61999e-14,  5.21521e-14,  1.67158e-13,  5.33747e-13,  1.69725e-12,  5.37243e-12,  1.69203e-11,  5.29935e-11,  1.64944e-10,  5.09829e-10,  1.56348e-09,  4.75188e-09,  1.42944e-08,  4.24886e-08,  1.24531e-07,  3.58933e-07,  1.01376e-06,  2.79222e-06,  7.44937e-06,  1.93467e-05,  0.000165753,  0.000546116,  0.00100453,  0.00149734,  0.00118072,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  },
    { 1.08141e-17,  4.35782e-17,  1.46712e-16,  4.7648e-16,  1.54759e-15,  5.02038e-15,  1.62142e-14,  5.21517e-14,  1.67158e-13,  5.33754e-13,  1.69726e-12,  5.37243e-12,  1.69203e-11,  5.29935e-11,  1.64944e-10,  5.09829e-10,  1.56348e-09,  4.75188e-09,  1.42944e-08,  4.24886e-08,  1.24531e-07,  3.58933e-07,  1.01376e-06,  2.79222e-06,  7.44937e-06,  1.93467e-05,  0.000165753,  0.000546116,  0.00100453,  0.00149734,  0.00118072,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  },
    { 3.29886e-18,  1.33294e-17,  4.49821e-17,  1.46442e-16,  4.76862e-16,  1.55121e-15,  5.02484e-15,  1.62141e-14,  5.21515e-14,  1.6716e-13,  5.33759e-13,  1.69726e-12,  5.37243e-12,  1.69203e-11,  5.29935e-11,  1.64944e-10,  5.09829e-10,  1.56348e-09,  4.75188e-09,  1.42944e-08,  4.24886e-08,  1.24531e-07,  3.58933e-07,  1.01376e-06,  2.79222e-06,  7.44937e-06,  1.93467e-05,  0.000165753,  0.000546116,  0.00100453,  0.00149734,  0.00118072,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  },
    { 1.0044e-18,  4.06863e-18,  1.37609e-17,  4.49005e-17,  1.4656e-16,  4.77977e-16,  1.55259e-15,  5.02481e-15,  1.6214e-14,  5.21523e-14,  1.67162e-13,  5.33759e-13,  1.69726e-12,  5.37243e-12,  1.69203e-11,  5.29935e-11,  1.64944e-10,  5.09829e-10,  1.56348e-09,  4.75188e-09,  1.42944e-08,  4.24886e-08,  1.24531e-07,  3.58933e-07,  1.01376e-06,  2.79222e-06,  7.44937e-06,  1.93467e-05,  0.000165753,  0.000546116,  0.00100453,  0.00149734,  0.00118072,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  },
    { 3.05258e-19,  1.23947e-18,  4.20092e-18,  1.37362e-17,  4.49366e-17,  1.46903e-16,  4.78405e-16,  1.55259e-15,  5.0248e-15,  1.62143e-14,  5.21527e-14,  1.67162e-13,  5.33759e-13,  1.69726e-12,  5.37243e-12,  1.69203e-11,  5.29935e-11,  1.64944e-10,  5.09829e-10,  1.56348e-09,  4.75188e-09,  1.42944e-08,  4.24886e-08,  1.24531e-07,  3.58933e-07,  1.01376e-06,  2.79222e-06,  7.44937e-06,  1.93467e-05,  0.000165753,  0.000546116,  0.00100453,  0.00149734,  0.00118072,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  },
    { 9.26167e-20,  3.76903e-19,  1.27994e-18,  4.19347e-18,  1.37472e-17,  4.50419e-17,  1.47035e-16,  4.78403e-16,  1.55258e-15,  5.02487e-15,  1.62144e-14,  5.21527e-14,  1.67162e-13,  5.33759e-13,  1.69726e-12,  5.37243e-12,  1.69203e-11,  5.29935e-11,  1.64944e-10,  5.09829e-10,  1.56348e-09,  4.75188e-09,  1.42944e-08,  4.24886e-08,  1.24531e-07,  3.58933e-07,  1.01376e-06,  2.79222e-06,  7.44937e-06,  1.93467e-05,  0.000165753,  0.000546116,  0.00100453,  0.00149734,  0.00118072,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  },
    { 2.80553e-20,  1.14412e-19,  3.89258e-19,  1.2777e-18,  4.19684e-18,  1.37795e-17,  4.50825e-17,  1.47034e-16,  4.78402e-16,  1.5526e-15,  5.02491e-15,  1.62144e-14,  5.21527e-14,  1.67162e-13,  5.33759e-13,  1.69726e-12,  5.37243e-12,  1.69203e-11,  5.29935e-11,  1.64944e-10,  5.09829e-10,  1.56348e-09,  4.75188e-09,  1.42944e-08,  4.24886e-08,  1.24531e-07,  3.58933e-07,  1.01376e-06,  2.79222e-06,  7.44937e-06,  1.93467e-05,  0.000165753,  0.000546116,  0.00100453,  0.00149734,  0.00118072,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  },
    { 8.48559e-21,  3.46739e-20,  1.18176e-19,  3.88582e-19,  1.27873e-18,  4.2067e-18,  1.37919e-17,  4.50823e-17,  1.47034e-16,  4.78409e-16,  1.55262e-15,  5.02491e-15,  1.62144e-14,  5.21527e-14,  1.67162e-13,  5.33759e-13,  1.69726e-12,  5.37243e-12,  1.69203e-11,  5.29935e-11,  1.64944e-10,  5.09829e-10,  1.56348e-09,  4.75188e-09,  1.42944e-08,  4.24886e-08,  1.24531e-07,  3.58933e-07,  1.01376e-06,  2.79222e-06,  7.44937e-06,  1.93467e-05,  0.000165753,  0.000546116,  0.00100453,  0.00149734,  0.00118072,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  },
    { 2.56286e-21,  1.04922e-20,  3.58187e-20,  1.17973e-19,  3.88895e-19,  1.28173e-18,  4.21051e-18,  1.37919e-17,  4.50822e-17,  1.47036e-16,  4.78413e-16,  1.55262e-15,  5.02491e-15,  1.62144e-14,  5.21527e-14,  1.67162e-13,  5.33759e-13,  1.69726e-12,  5.37243e-12,  1.69203e-11,  5.29935e-11,  1.64944e-10,  5.09829e-10,  1.56348e-09,  4.75188e-09,  1.42944e-08,  4.24886e-08,  1.24531e-07,  3.58933e-07,  1.01376e-06,  2.79222e-06,  7.44937e-06,  1.93467e-05,  0.000165753,  0.000546116,  0.00100453,  0.00149734,  0.00118072,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  },
    { 7.72993e-22,  3.17024e-21,  1.08397e-20,  3.57576e-20,  1.18068e-19,  3.89809e-19,  1.2829e-18,  4.2105e-18,  1.37919e-17,  4.50829e-17,  1.47037e-16,  4.78413e-16,  1.55262e-15,  5.02491e-15,  1.62144e-14,  5.21527e-14,  1.67162e-13,  5.33759e-13,  1.69726e-12,  5.37243e-12,  1.69203e-11,  5.29935e-11,  1.64944e-10,  5.09829e-10,  1.56348e-09,  4.75188e-09,  1.42944e-08,  4.24886e-08,  1.24531e-07,  3.58933e-07,  1.01376e-06,  2.79222e-06,  7.44937e-06,  1.93467e-05,  0.000165753,  0.000546116,  0.00100453,  0.00149734,  0.00118072,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  },
    { 2.32843e-22,  9.56572e-22,  3.27558e-21,  1.08214e-20,  3.57865e-20,  1.18346e-19,  3.90165e-19,  1.28289e-18,  4.21049e-18,  1.37921e-17,  4.50833e-17,  1.47037e-16,  4.78413e-16,  1.55262e-15,  5.02491e-15,  1.62144e-14,  5.21527e-14,  1.67162e-13,  5.33759e-13,  1.69726e-12,  5.37243e-12,  1.69203e-11,  5.29935e-11,  1.64944e-10,  5.09829e-10,  1.56348e-09,  4.75188e-09,  1.42944e-08,  4.24886e-08,  1.24531e-07,  3.58933e-07,  1.01376e-06,  2.79222e-06,  7.44937e-06,  1.93467e-05,  0.000165753,  0.000546116,  0.00100453,  0.00149734,  0.00118072,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  },
    { 7.00514e-23,  2.88252e-22,  9.8845e-22,  3.27008e-21,  1.08301e-20,  3.58707e-20,  1.18454e-19,  3.90164e-19,  1.28289e-18,  4.21055e-18,  1.37922e-17,  4.50833e-17,  1.47037e-16,  4.78413e-16,  1.55262e-15,  5.02491e-15,  1.62144e-14,  5.21527e-14,  1.67162e-13,  5.33759e-13,  1.69726e-12,  5.37243e-12,  1.69203e-11,  5.29935e-11,  1.64944e-10,  5.09829e-10,  1.56348e-09,  4.75188e-09,  1.42944e-08,  4.24886e-08,  1.24531e-07,  3.58933e-07,  1.01376e-06,  2.79222e-06,  7.44937e-06,  1.93467e-05,  0.000165753,  0.000546116,  0.00100453,  0.00149734,  0.00118072,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  },
    { 2.10504e-23,  8.67527e-23,  2.97884e-22,  9.86805e-22,  3.27273e-21,  1.08556e-20,  3.59036e-20,  1.18454e-19,  3.90163e-19,  1.28291e-18,  4.21059e-18,  1.37922e-17,  4.50833e-17,  1.47037e-16,  4.78413e-16,  1.55262e-15,  5.02491e-15,  1.62144e-14,  5.21527e-14,  1.67162e-13,  5.33759e-13,  1.69726e-12,  5.37243e-12,  1.69203e-11,  5.29935e-11,  1.64944e-10,  5.09829e-10,  1.56348e-09,  4.75188e-09,  1.42944e-08,  4.24886e-08,  1.24531e-07,  3.58933e-07,  1.01376e-06,  2.79222e-06,  7.44937e-06,  1.93467e-05,  0.000165753,  0.000546116,  0.00100453,  0.00149734,  0.00118072,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  },
    { 6.31856e-24,  2.60782e-23,  8.96594e-23,  2.97392e-22,  9.87602e-22,  3.28044e-21,  1.08656e-20,  3.59035e-20,  1.18453e-19,  3.90169e-19,  1.28292e-18,  4.21059e-18,  1.37922e-17,  4.50833e-17,  1.47037e-16,  4.78413e-16,  1.55262e-15,  5.02491e-15,  1.62144e-14,  5.21527e-14,  1.67162e-13,  5.33759e-13,  1.69726e-12,  5.37243e-12,  1.69203e-11,  5.29935e-11,  1.64944e-10,  5.09829e-10,  1.56348e-09,  4.75188e-09,  1.42944e-08,  4.24886e-08,  1.24531e-07,  3.58933e-07,  1.01376e-06,  2.79222e-06,  7.44937e-06,  1.93467e-05,  0.000165753,  0.000546116,  0.00100453,  0.00149734,  0.00118072,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  },
    { 1.89458e-24,  7.8303e-24,  2.69541e-23,  8.95124e-23,  2.97633e-22,  9.89932e-22,  3.28346e-21,  1.08656e-20,  3.59035e-20,  1.18455e-19,  3.90172e-19,  1.28292e-18,  4.21059e-18,  1.37922e-17,  4.50833e-17,  1.47037e-16,  4.78413e-16,  1.55262e-15,  5.02491e-15,  1.62144e-14,  5.21527e-14,  1.67162e-13,  5.33759e-13,  1.69726e-12,  5.37243e-12,  1.69203e-11,  5.29935e-11,  1.64944e-10,  5.09829e-10,  1.56348e-09,  4.75188e-09,  1.42944e-08,  4.24886e-08,  1.24531e-07,  3.58933e-07,  1.01376e-06,  2.79222e-06,  7.44937e-06,  1.93467e-05,  0.000165753,  0.000546116,  0.00100453,  0.00149734,  0.00118072,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  },
    { 5.67497e-25,  2.3486e-24,  8.09394e-24,  2.69102e-23,  8.95848e-23,  2.98335e-22,  9.90843e-22,  3.28346e-21,  1.08656e-20,  3.5904e-20,  1.18456e-19,  3.90172e-19,  1.28292e-18,  4.21059e-18,  1.37922e-17,  4.50833e-17,  1.47037e-16,  4.78413e-16,  1.55262e-15,  5.02491e-15,  1.62144e-14,  5.21527e-14,  1.67162e-13,  5.33759e-13,  1.69726e-12,  5.37243e-12,  1.69203e-11,  5.29935e-11,  1.64944e-10,  5.09829e-10,  1.56348e-09,  4.75188e-09,  1.42944e-08,  4.24886e-08,  1.24531e-07,  3.58933e-07,  1.01376e-06,  2.79222e-06,  7.44937e-06,  1.93467e-05,  0.000165753,  0.000546116,  0.00100453,  0.00149734,  0.00118072,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  },
    { 1.69821e-25,  7.03706e-25,  2.42786e-24,  8.08084e-24,  2.6932e-23,  8.97964e-23,  2.9861e-22,  9.90844e-22,  3.28345e-21,  1.08657e-20,  3.59043e-20,  1.18456e-19,  3.90172e-19,  1.28292e-18,  4.21059e-18,  1.37922e-17,  4.50833e-17,  1.47037e-16,  4.78413e-16,  1.55262e-15,  5.02491e-15,  1.62144e-14,  5.21527e-14,  1.67162e-13,  5.33759e-13,  1.69726e-12,  5.37243e-12,  1.69203e-11,  5.29935e-11,  1.64944e-10,  5.09829e-10,  1.56348e-09,  4.75188e-09,  1.42944e-08,  4.24886e-08,  1.24531e-07,  3.58933e-07,  1.01376e-06,  2.79222e-06,  7.44937e-06,  1.93467e-05,  0.000165753,  0.000546116,  0.00100453,  0.00149734,  0.00118072,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  },
    { 5.07706e-26,  2.10641e-25,  7.27505e-25,  2.42395e-24,  8.08739e-24,  2.69957e-23,  8.98793e-23,  2.98611e-22,  9.90841e-22,  3.28349e-21,  1.08658e-20,  3.59043e-20,  1.18456e-19,  3.90172e-19,  1.28292e-18,  4.21059e-18,  1.37922e-17,  4.50833e-17,  1.47037e-16,  4.78413e-16,  1.55262e-15,  5.02491e-15,  1.62144e-14,  5.21527e-14,  1.67162e-13,  5.33759e-13,  1.69726e-12,  5.37243e-12,  1.69203e-11,  5.29935e-11,  1.64944e-10,  5.09829e-10,  1.56348e-09,  4.75188e-09,  1.42944e-08,  4.24886e-08,  1.24531e-07,  3.58933e-07,  1.01376e-06,  2.79222e-06,  7.44937e-06,  1.93467e-05,  0.000165753,  0.000546116,  0.00100453,  0.00149734,  0.00118072,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  },
    { 1.51651e-26,  6.29919e-26,  2.1778e-25,  7.26342e-25,  2.42592e-24,  8.10651e-24,  2.70206e-23,  8.98794e-23,  2.9861e-22,  9.90855e-22,  3.28353e-21,  1.08658e-20,  3.59043e-20,  1.18456e-19,  3.90172e-19,  1.28292e-18,  4.21059e-18,  1.37922e-17,  4.50833e-17,  1.47037e-16,  4.78413e-16,  1.55262e-15,  5.02491e-15,  1.62144e-14,  5.21527e-14,  1.67162e-13,  5.33759e-13,  1.69726e-12,  5.37243e-12,  1.69203e-11,  5.29935e-11,  1.64944e-10,  5.09829e-10,  1.56348e-09,  4.75188e-09,  1.42944e-08,  4.24886e-08,  1.24531e-07,  3.58933e-07,  1.01376e-06,  2.79222e-06,  7.44937e-06,  1.93467e-05,  0.000165753,  0.000546116,  0.00100453,  0.00149734,  0.00118072,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  },
    { 4.5259e-27,  1.88205e-26,  6.51309e-26,  2.17434e-25,  7.26931e-25,  2.43166e-24,  8.11402e-24,  2.70206e-23,  8.98792e-23,  2.98614e-22,  9.90865e-22,  3.28352e-21,  1.08658e-20,  3.59043e-20,  1.18456e-19,  3.90172e-19,  1.28292e-18,  4.21059e-18,  1.37922e-17,  4.50833e-17,  1.47037e-16,  4.78413e-16,  1.55262e-15,  5.02491e-15,  1.62144e-14,  5.21527e-14,  1.67162e-13,  5.33759e-13,  1.69726e-12,  5.37243e-12,  1.69203e-11,  5.29935e-11,  1.64944e-10,  5.09829e-10,  1.56348e-09,  4.75188e-09,  1.42944e-08,  4.24886e-08,  1.24531e-07,  3.58933e-07,  1.01376e-06,  2.79222e-06,  7.44937e-06,  1.93467e-05,  0.000165753,  0.000546116,  0.00100453,  0.00149734,  0.00118072,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  },
    { 1.3496e-27,  5.61826e-27,  1.94608e-26,  6.50279e-26,  2.1761e-25,  7.28651e-25,  2.43391e-24,  8.11403e-24,  2.70206e-23,  8.98805e-23,  2.98617e-22,  9.90864e-22,  3.28352e-21,  1.08658e-20,  3.59043e-20,  1.18456e-19,  3.90172e-19,  1.28292e-18,  4.21059e-18,  1.37922e-17,  4.50833e-17,  1.47037e-16,  4.78413e-16,  1.55262e-15,  5.02491e-15,  1.62144e-14,  5.21527e-14,  1.67162e-13,  5.33759e-13,  1.69726e-12,  5.37243e-12,  1.69203e-11,  5.29935e-11,  1.64944e-10,  5.09829e-10,  1.56348e-09,  4.75188e-09,  1.42944e-08,  4.24886e-08,  1.24531e-07,  3.58933e-07,  1.01376e-06,  2.79222e-06,  7.44937e-06,  1.93467e-05,  0.000165753,  0.000546116,  0.00100453,  0.00149734,  0.00118072,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  },
    { 4.02128e-28,  1.67575e-27,  5.80974e-27,  1.94302e-26,  6.50807e-26,  2.18125e-25,  7.29327e-25,  2.43392e-24,  8.11401e-24,  2.7021e-23,  8.98813e-23,  2.98617e-22,  9.90864e-22,  3.28352e-21,  1.08658e-20,  3.59043e-20,  1.18456e-19,  3.90172e-19,  1.28292e-18,  4.21059e-18,  1.37922e-17,  4.50833e-17,  1.47037e-16,  4.78413e-16,  1.55262e-15,  5.02491e-15,  1.62144e-14,  5.21527e-14,  1.67162e-13,  5.33759e-13,  1.69726e-12,  5.37243e-12,  1.69203e-11,  5.29935e-11,  1.64944e-10,  5.09829e-10,  1.56348e-09,  4.75188e-09,  1.42944e-08,  4.24886e-08,  1.24531e-07,  3.58933e-07,  1.01376e-06,  2.79222e-06,  7.44937e-06,  1.93467e-05,  0.000165753,  0.000546116,  0.00100453,  0.00149734,  0.00118072,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  },
    { 1.19727e-28,  4.99423e-28,  1.73296e-27,  5.80066e-27,  1.9446e-26,  6.52349e-26,  2.18328e-25,  7.29329e-25,  2.43391e-24,  8.11413e-24,  2.70212e-23,  8.98813e-23,  2.98617e-22,  9.90864e-22,  3.28352e-21,  1.08658e-20,  3.59043e-20,  1.18456e-19,  3.90172e-19,  1.28292e-18,  4.21059e-18,  1.37922e-17,  4.50833e-17,  1.47037e-16,  4.78413e-16,  1.55262e-15,  5.02491e-15,  1.62144e-14,  5.21527e-14,  1.67162e-13,  5.33759e-13,  1.69726e-12,  5.37243e-12,  1.69203e-11,  5.29935e-11,  1.64944e-10,  5.09829e-10,  1.56348e-09,  4.75188e-09,  1.42944e-08,  4.24886e-08,  1.24531e-07,  3.58933e-07,  1.01376e-06,  2.79222e-06,  7.44937e-06,  1.93467e-05,  0.000165753,  0.000546116,  0.00100453,  0.00149734,  0.00118072,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  },
    { 3.56205e-29,  1.48728e-28,  5.16502e-28,  1.73027e-27,  5.80537e-27,  1.94921e-26,  6.52956e-26,  2.18329e-25,  7.29327e-25,  2.43394e-24,  8.1142e-24,  2.70212e-23,  8.98813e-23,  2.98617e-22,  9.90864e-22,  3.28352e-21,  1.08658e-20,  3.59043e-20,  1.18456e-19,  3.90172e-19,  1.28292e-18,  4.21059e-18,  1.37922e-17,  4.50833e-17,  1.47037e-16,  4.78413e-16,  1.55262e-15,  5.02491e-15,  1.62144e-14,  5.21527e-14,  1.67162e-13,  5.33759e-13,  1.69726e-12,  5.37243e-12,  1.69203e-11,  5.29935e-11,  1.64944e-10,  5.09829e-10,  1.56348e-09,  4.75188e-09,  1.42944e-08,  4.24886e-08,  1.24531e-07,  3.58933e-07,  1.01376e-06,  2.79222e-06,  7.44937e-06,  1.93467e-05,  0.000165753,  0.000546116,  0.00100453,  0.00149734,  0.00118072,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  },
    { 1.05902e-29,  4.42583e-29,  1.53822e-28,  5.15702e-28,  1.73167e-27,  5.81913e-27,  1.95102e-26,  6.52957e-26,  2.18328e-25,  7.29338e-25,  2.43397e-24,  8.1142e-24,  2.70212e-23,  8.98813e-23,  2.98617e-22,  9.90864e-22,  3.28352e-21,  1.08658e-20,  3.59043e-20,  1.18456e-19,  3.90172e-19,  1.28292e-18,  4.21059e-18,  1.37922e-17,  4.50833e-17,  1.47037e-16,  4.78413e-16,  1.55262e-15,  5.02491e-15,  1.62144e-14,  5.21527e-14,  1.67162e-13,  5.33759e-13,  1.69726e-12,  5.37243e-12,  1.69203e-11,  5.29935e-11,  1.64944e-10,  5.09829e-10,  1.56348e-09,  4.75188e-09,  1.42944e-08,  4.24886e-08,  1.24531e-07,  3.58933e-07,  1.01376e-06,  2.79222e-06,  7.44937e-06,  1.93467e-05,  0.000165753,  0.000546116,  0.00100453,  0.00149734,  0.00118072,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  },
    { 3.14637e-30,  1.3161e-29,  4.57766e-29,  1.53585e-28,  5.16122e-28,  1.73578e-27,  5.82456e-27,  1.95103e-26,  6.52956e-26,  2.18331e-25,  7.29345e-25,  2.43397e-24,  8.1142e-24,  2.70212e-23,  8.98813e-23,  2.98617e-22,  9.90864e-22,  3.28352e-21,  1.08658e-20,  3.59043e-20,  1.18456e-19,  3.90172e-19,  1.28292e-18,  4.21059e-18,  1.37922e-17,  4.50833e-17,  1.47037e-16,  4.78413e-16,  1.55262e-15,  5.02491e-15,  1.62144e-14,  5.21527e-14,  1.67162e-13,  5.33759e-13,  1.69726e-12,  5.37243e-12,  1.69203e-11,  5.29935e-11,  1.64944e-10,  5.09829e-10,  1.56348e-09,  4.75188e-09,  1.42944e-08,  4.24886e-08,  1.24531e-07,  3.58933e-07,  1.01376e-06,  2.79222e-06,  7.44937e-06,  1.93467e-05,  0.000165753,  0.000546116,  0.00100453,  0.00149734,  0.00118072,  0,  0,  0,  0,  0,  0,  0,  0,  0,  },
    { 9.34181e-31,  3.91094e-30,  1.36131e-29,  4.57064e-29,  1.5371e-28,  5.17346e-28,  1.7374e-27,  5.82457e-27,  1.95103e-26,  6.52965e-26,  2.18333e-25,  7.29344e-25,  2.43397e-24,  8.1142e-24,  2.70212e-23,  8.98813e-23,  2.98617e-22,  9.90864e-22,  3.28352e-21,  1.08658e-20,  3.59043e-20,  1.18456e-19,  3.90172e-19,  1.28292e-18,  4.21059e-18,  1.37922e-17,  4.50833e-17,  1.47037e-16,  4.78413e-16,  1.55262e-15,  5.02491e-15,  1.62144e-14,  5.21527e-14,  1.67162e-13,  5.33759e-13,  1.69726e-12,  5.37243e-12,  1.69203e-11,  5.29935e-11,  1.64944e-10,  5.09829e-10,  1.56348e-09,  4.75188e-09,  1.42944e-08,  4.24886e-08,  1.24531e-07,  3.58933e-07,  1.01376e-06,  2.79222e-06,  7.44937e-06,  1.93467e-05,  0.000165753,  0.000546116,  0.00100453,  0.00149734,  0.00118072,  0,  0,  0,  0,  0,  0,  0,  0,  },
    { 2.7719e-31,  1.16141e-30,  4.0455e-30,  1.35923e-29,  4.57436e-29,  1.54075e-28,  5.17829e-28,  1.7374e-27,  5.82456e-27,  1.95105e-26,  6.52971e-26,  2.18333e-25,  7.29344e-25,  2.43397e-24,  8.1142e-24,  2.70212e-23,  8.98813e-23,  2.98617e-22,  9.90864e-22,  3.28352e-21,  1.08658e-20,  3.59043e-20,  1.18456e-19,  3.90172e-19,  1.28292e-18,  4.21059e-18,  1.37922e-17,  4.50833e-17,  1.47037e-16,  4.78413e-16,  1.55262e-15,  5.02491e-15,  1.62144e-14,  5.21527e-14,  1.67162e-13,  5.33759e-13,  1.69726e-12,  5.37243e-12,  1.69203e-11,  5.29935e-11,  1.64944e-10,  5.09829e-10,  1.56348e-09,  4.75188e-09,  1.42944e-08,  4.24886e-08,  1.24531e-07,  3.58933e-07,  1.01376e-06,  2.79222e-06,  7.44937e-06,  1.93467e-05,  0.000165753,  0.000546116,  0.00100453,  0.00149734,  0.00118072,  0,  0,  0,  0,  0,  0,  0,  },
    { 8.21977e-32,  3.44679e-31,  1.20143e-30,  4.03935e-30,  1.36034e-29,  4.58522e-29,  1.54219e-28,  5.17831e-28,  1.7374e-27,  5.82465e-27,  1.95107e-26,  6.52971e-26,  2.18333e-25,  7.29344e-25,  2.43397e-24,  8.1142e-24,  2.70212e-23,  8.98813e-23,  2.98617e-22,  9.90864e-22,  3.28352e-21,  1.08658e-20,  3.59043e-20,  1.18456e-19,  3.90172e-19,  1.28292e-18,  4.21059e-18,  1.37922e-17,  4.50833e-17,  1.47037e-16,  4.78413e-16,  1.55262e-15,  5.02491e-15,  1.62144e-14,  5.21527e-14,  1.67162e-13,  5.33759e-13,  1.69726e-12,  5.37243e-12,  1.69203e-11,  5.29935e-11,  1.64944e-10,  5.09829e-10,  1.56348e-09,  4.75188e-09,  1.42944e-08,  4.24886e-08,  1.24531e-07,  3.58933e-07,  1.01376e-06,  2.79222e-06,  7.44937e-06,  1.93467e-05,  0.000165753,  0.000546116,  0.00100453,  0.00149734,  0.00118072,  0,  0,  0,  0,  0,  0,  },
    { 2.43604e-32,  1.02229e-31,  3.5657e-31,  1.19961e-30,  4.04264e-30,  1.36357e-29,  4.58951e-29,  1.5422e-28,  5.1783e-28,  1.73742e-27,  5.8247e-27,  1.95107e-26,  6.52971e-26,  2.18333e-25,  7.29344e-25,  2.43397e-24,  8.1142e-24,  2.70212e-23,  8.98813e-23,  2.98617e-22,  9.90864e-22,  3.28352e-21,  1.08658e-20,  3.59043e-20,  1.18456e-19,  3.90172e-19,  1.28292e-18,  4.21059e-18,  1.37922e-17,  4.50833e-17,  1.47037e-16,  4.78413e-16,  1.55262e-15,  5.02491e-15,  1.62144e-14,  5.21527e-14,  1.67162e-13,  5.33759e-13,  1.69726e-12,  5.37243e-12,  1.69203e-11,  5.29935e-11,  1.64944e-10,  5.09829e-10,  1.56348e-09,  4.75188e-09,  1.42944e-08,  4.24886e-08,  1.24531e-07,  3.58933e-07,  1.01376e-06,  2.79222e-06,  7.44937e-06,  1.93467e-05,  0.000165753,  0.000546116,  0.00100453,  0.00149734,  0.00118072,  0,  0,  0,  0,  0,  },
    { 7.21544e-33,  3.03023e-32,  1.0576e-31,  3.56032e-31,  1.20059e-30,  4.05223e-30,  1.36485e-29,  4.58953e-29,  1.54219e-28,  5.17837e-28,  1.73744e-27,  5.8247e-27,  1.95107e-26,  6.52971e-26,  2.18333e-25,  7.29344e-25,  2.43397e-24,  8.1142e-24,  2.70212e-23,  8.98813e-23,  2.98617e-22,  9.90864e-22,  3.28352e-21,  1.08658e-20,  3.59043e-20,  1.18456e-19,  3.90172e-19,  1.28292e-18,  4.21059e-18,  1.37922e-17,  4.50833e-17,  1.47037e-16,  4.78413e-16,  1.55262e-15,  5.02491e-15,  1.62144e-14,  5.21527e-14,  1.67162e-13,  5.33759e-13,  1.69726e-12,  5.37243e-12,  1.69203e-11,  5.29935e-11,  1.64944e-10,  5.09829e-10,  1.56348e-09,  4.75188e-09,  1.42944e-08,  4.24886e-08,  1.24531e-07,  3.58933e-07,  1.01376e-06,  2.79222e-06,  7.44937e-06,  1.93467e-05,  0.000165753,  0.000546116,  0.00100453,  0.00149734,  0.00118072,  0,  0,  0,  0,  },
    { 2.13599e-33,  8.97689e-33,  3.13503e-32,  1.05601e-31,  3.56322e-31,  1.20344e-30,  4.05603e-30,  1.36485e-29,  4.58952e-29,  1.54221e-28,  5.17842e-28,  1.73744e-27,  5.8247e-27,  1.95107e-26,  6.52971e-26,  2.18333e-25,  7.29344e-25,  2.43397e-24,  8.1142e-24,  2.70212e-23,  8.98813e-23,  2.98617e-22,  9.90864e-22,  3.28352e-21,  1.08658e-20,  3.59043e-20,  1.18456e-19,  3.90172e-19,  1.28292e-18,  4.21059e-18,  1.37922e-17,  4.50833e-17,  1.47037e-16,  4.78413e-16,  1.55262e-15,  5.02491e-15,  1.62144e-14,  5.21527e-14,  1.67162e-13,  5.33759e-13,  1.69726e-12,  5.37243e-12,  1.69203e-11,  5.29935e-11,  1.64944e-10,  5.09829e-10,  1.56348e-09,  4.75188e-09,  1.42944e-08,  4.24886e-08,  1.24531e-07,  3.58933e-07,  1.01376e-06,  2.79222e-06,  7.44937e-06,  1.93467e-05,  0.000165753,  0.000546116,  0.00100453,  0.00149734,  0.00118072,  0,  0,  0,  },
    { 6.31983e-34,  2.65787e-33,  9.28771e-33,  3.13033e-32,  1.05687e-31,  3.57168e-31,  1.20457e-30,  4.05605e-30,  1.36485e-29,  4.58958e-29,  1.54223e-28,  5.17842e-28,  1.73744e-27,  5.8247e-27,  1.95107e-26,  6.52971e-26,  2.18333e-25,  7.29344e-25,  2.43397e-24,  8.1142e-24,  2.70212e-23,  8.98813e-23,  2.98617e-22,  9.90864e-22,  3.28352e-21,  1.08658e-20,  3.59043e-20,  1.18456e-19,  3.90172e-19,  1.28292e-18,  4.21059e-18,  1.37922e-17,  4.50833e-17,  1.47037e-16,  4.78413e-16,  1.55262e-15,  5.02491e-15,  1.62144e-14,  5.21527e-14,  1.67162e-13,  5.33759e-13,  1.69726e-12,  5.37243e-12,  1.69203e-11,  5.29935e-11,  1.64944e-10,  5.09829e-10,  1.56348e-09,  4.75188e-09,  1.42944e-08,  4.24886e-08,  1.24531e-07,  3.58933e-07,  1.01376e-06,  2.79222e-06,  7.44937e-06,  1.93467e-05,  0.000165753,  0.000546116,  0.00100453,  0.00149734,  0.00118072,  0,  0,  },
    { 1.8689e-34,  7.86516e-34,  2.75e-33,  9.27386e-33,  3.13289e-32,  1.05939e-31,  3.57504e-31,  1.20457e-30,  4.05604e-30,  1.36487e-29,  4.58963e-29,  1.54223e-28,  5.17842e-28,  1.73744e-27,  5.8247e-27,  1.95107e-26,  6.52971e-26,  2.18333e-25,  7.29344e-25,  2.43397e-24,  8.1142e-24,  2.70212e-23,  8.98813e-23,  2.98617e-22,  9.90864e-22,  3.28352e-21,  1.08658e-20,  3.59043e-20,  1.18456e-19,  3.90172e-19,  1.28292e-18,  4.21059e-18,  1.37922e-17,  4.50833e-17,  1.47037e-16,  4.78413e-16,  1.55262e-15,  5.02491e-15,  1.62144e-14,  5.21527e-14,  1.67162e-13,  5.33759e-13,  1.69726e-12,  5.37243e-12,  1.69203e-11,  5.29935e-11,  1.64944e-10,  5.09829e-10,  1.56348e-09,  4.75188e-09,  1.42944e-08,  4.24886e-08,  1.24531e-07,  3.58933e-07,  1.01376e-06,  2.79222e-06,  7.44937e-06,  1.93467e-05,  0.000165753,  0.000546116,  0.00100453,  0.00149734,  0.00118072,  0,  },
    { 5.52395e-35,  2.32624e-34,  8.1381e-34,  2.74592e-33,  9.28142e-33,  3.14033e-32,  1.06038e-31,  3.57506e-31,  1.20457e-30,  4.0561e-30,  1.36488e-29,  4.58963e-29,  1.54223e-28,  5.17842e-28,  1.73744e-27,  5.8247e-27,  1.95107e-26,  6.52971e-26,  2.18333e-25,  7.29344e-25,  2.43397e-24,  8.1142e-24,  2.70212e-23,  8.98813e-23,  2.98617e-22,  9.90864e-22,  3.28352e-21,  1.08658e-20,  3.59043e-20,  1.18456e-19,  3.90172e-19,  1.28292e-18,  4.21059e-18,  1.37922e-17,  4.50833e-17,  1.47037e-16,  4.78413e-16,  1.55262e-15,  5.02491e-15,  1.62144e-14,  5.21527e-14,  1.67162e-13,  5.33759e-13,  1.69726e-12,  5.37243e-12,  1.69203e-11,  5.29935e-11,  1.64944e-10,  5.09829e-10,  1.56348e-09,  4.75188e-09,  1.42944e-08,  4.24886e-08,  1.24531e-07,  3.58933e-07,  1.01376e-06,  2.79222e-06,  7.44937e-06,  1.93467e-05,  0.000165753,  0.000546116,  0.00100453,  0.00149734,  0.00118072  }
  }
  ;
  //double L
  MatrixSerial = { { 0.013249,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  },
    { 0.00475532,  0.0123662,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  },
    { 0.00274508,  0.0040392,  0.012316,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  },
    { 0.00210822,  0.00213035,  0.00394697,  0.0122871,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  },
    { 0.000376401,  0.00211398,  0.00217582,  0.00399753,  0.012256,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  },
    { 1.98224e-05,  0.000395649,  0.00213374,  0.00220255,  0.00397452,  0.0122547,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  },
    { 0.00059039,  -0.000205793,  0.000340816,  0.00209816,  0.00223999,  0.0039839,  0.0122367,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  },
    { 0.00165779,  -4.95393e-06,  -0.000346551,  0.000233604,  0.00221429,  0.00226548,  0.00389532,  0.012141,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  },
    { 0.00126864,  0.00128829,  -7.01616e-05,  -0.000397125,  0.000287709,  0.00222591,  0.00222532,  0.00384044,  0.0121394,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  },
    { 0.000687338,  0.00109489,  0.00127108,  -7.94679e-05,  -0.00038827,  0.000289372,  0.00222296,  0.00224361,  0.00384555,  0.0121344,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  },
    { 0.000317359,  0.000614368,  0.00109251,  0.00127564,  -8.75235e-05,  -0.000390029,  0.000296407,  0.00225782,  0.00225064,  0.00383409,  0.0121295,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  },
    { 0.00013335,  0.000288736,  0.000614988,  0.00109745,  0.00127107,  -8.78134e-05,  -0.000389504,  0.000298635,  0.0022587,  0.00224948,  0.0038354,  0.0121291,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  },
    { 5.2629e-05,  0.000122631,  0.00028945,  0.000617872,  0.00109573,  0.00127143,  -8.97687e-05,  -0.000397417,  0.000297259,  0.00226201,  0.00225458,  0.00383378,  0.0121278,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  },
    { 1.98609e-05,  4.87488e-05,  0.000123036,  0.00029082,  0.000617307,  0.00109606,  0.00127165,  -8.56127e-05,  -0.000397023,  0.000296772,  0.00226256,  0.00225423,  0.00383371,  0.0121277,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  },
    { 7.24665e-06,  1.84921e-05,  4.89358e-05,  0.000123619,  0.00029065,  0.000617492,  0.00109675,  0.001287,  -8.23599e-05,  -0.000402957,  0.000290846,  0.00226433,  0.00225779,  0.00383321,  0.0121252,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  },
    { 2.57546e-06,  6.7736e-06,  1.85699e-05,  4.91674e-05,  0.000123572,  0.000290737,  0.000617964,  0.00110859,  0.00128981,  -8.76795e-05,  -0.000408429,  0.000292502,  0.00226756,  0.00225733,  0.0038293,  0.0121242,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  },
    { 8.96199e-07,  2.41469e-06,  6.8039e-06,  1.86576e-05,  4.91554e-05,  0.000123609,  0.00029098,  0.000624391,  0.00111019,  0.00128724,  -8.97364e-05,  -0.000407831,  0.000293576,  0.00226741,  0.00225637,  0.0038293,  0.0121242,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  },
    { 3.06502e-07,  8.42315e-07,  2.42598e-06,  6.83599e-06,  1.86549e-05,  4.917e-05,  0.000123717,  0.000293947,  0.000625148,  0.00110915,  0.00128718,  -8.98748e-05,  -0.000408129,  0.000293623,  0.00226838,  0.00225742,  0.00382911,  0.0121239,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  },
    { 1.03318e-07,  2.88652e-07,  8.46391e-07,  2.4374e-06,  6.83552e-06,  1.86605e-05,  4.92145e-05,  0.000124964,  0.000294271,  0.000624761,  0.00110947,  0.001287,  -9.01261e-05,  -0.000408095,  0.00029403,  0.00226884,  0.00225732,  0.00382887,  0.0121239,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  },
    { 3.4402e-08,  9.74643e-08,  2.90087e-07,  8.50369e-07,  2.43738e-06,  6.83755e-06,  1.86777e-05,  4.97063e-05,  0.000125093,  0.000294133,  0.000624996,  0.00110937,  0.00128703,  -9.01484e-05,  -0.000408414,  0.000293807,  0.00226888,  0.00225754,  0.00382893,  0.0121238,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  },
    { 1.13347e-08,  3.24992e-08,  9.79591e-08,  2.91448e-07,  8.50401e-07,  2.4381e-06,  6.84398e-06,  1.88632e-05,  4.97555e-05,  0.000125045,  0.000294255,  0.000624947,  0.00110944,  0.001287,  -9.03853e-05,  -0.000408601,  0.000293842,  0.00226908,  0.0022576,  0.00382881,  0.0121238,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  },
    { 3.70048e-09,  1.07209e-08,  3.26671e-08,  9.84181e-08,  2.9147e-07,  8.50652e-07,  2.44042e-06,  6.91165e-06,  1.88813e-05,  4.97394e-05,  0.0001251,  0.000294234,  0.000624997,  0.00110943,  0.00128716,  -9.01351e-05,  -0.000408641,  0.000293698,  0.00226905,  0.00225766,  0.00382887,  0.0121237,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  },
    { 1.19844e-09,  3.5038e-09,  1.07771e-08,  3.282e-08,  9.84289e-08,  2.91556e-07,  8.5147e-07,  2.46447e-06,  6.91811e-06,  1.8876e-05,  4.97619e-05,  0.000125092,  0.00029426,  0.00062499,  0.00110961,  0.00128751,  -9.02068e-05,  -0.000408924,  0.00029364,  0.00226921,  0.00225776,  0.00382875,  0.0121237,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  },
    { 3.85382e-10,  1.1358e-09,  3.52239e-09,  1.08275e-08,  3.28245e-08,  9.84579e-08,  2.91839e-07,  8.59835e-07,  2.46672e-06,  6.91636e-06,  1.88847e-05,  4.97588e-05,  0.000125103,  0.000294257,  0.0006251,  0.00110984,  0.00128746,  -9.03819e-05,  -0.00040896,  0.000293728,  0.00226926,  0.0022577,  0.00382869,  0.0121236,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  },
    { 1.23146e-10,  3.65539e-10,  1.14189e-09,  3.53884e-09,  1.08292e-08,  3.28341e-08,  9.8554e-08,  2.94699e-07,  8.6061e-07,  2.46616e-06,  6.9196e-06,  1.88836e-05,  4.97635e-05,  0.000125102,  0.00029431,  0.000625218,  0.00110982,  0.00128741,  -9.03843e-05,  -0.000408957,  0.00029373,  0.00226927,  0.00225773,  0.0038287,  0.0121236,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  },
    { 3.9128e-11,  1.16891e-10,  3.67517e-10,  1.14722e-09,  3.53948e-09,  1.08324e-08,  3.28664e-08,  9.9518e-08,  2.94961e-07,  8.60429e-07,  2.46733e-06,  6.9192e-06,  1.88854e-05,  4.9763e-05,  0.000125125,  0.000294363,  0.000625207,  0.00110981,  0.00128742,  -9.0404e-05,  -0.000408969,  0.000293747,  0.00226931,  0.00225774,  0.00382868,  0.0121236,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  },
    { 1.23691e-11,  3.71649e-11,  1.17529e-10,  3.6923e-10,  1.14745e-09,  3.54052e-09,  1.08431e-08,  3.31873e-08,  9.96054e-08,  2.94904e-07,  8.60839e-07,  2.46719e-06,  6.91987e-06,  1.88852e-05,  4.97723e-05,  0.000125147,  0.000294359,  0.00062521,  0.00110982,  0.00128741,  -9.04038e-05,  -0.00040897,  0.000293745,  0.00226931,  0.00225774,  0.00382869,  0.0121236,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  },
    { 3.89207e-12,  1.17555e-11,  3.73691e-11,  1.18076e-10,  3.6931e-10,  1.14778e-09,  3.54403e-09,  1.09488e-08,  3.32162e-08,  9.95873e-08,  2.95045e-07,  8.60793e-07,  2.46743e-06,  6.91982e-06,  1.88888e-05,  4.97808e-05,  0.000125145,  0.000294361,  0.000625213,  0.00110981,  0.00128742,  -9.04147e-05,  -0.000408988,  0.00029374,  0.00226932,  0.00225775,  0.00382868,  0.0121236,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  },
    { 1.21953e-12,  3.70095e-12,  1.18205e-11,  3.7543e-11,  1.18103e-10,  3.69418e-10,  1.14893e-09,  3.57854e-09,  1.09582e-08,  3.32105e-08,  9.96353e-08,  2.9503e-07,  8.60878e-07,  2.46741e-06,  6.92113e-06,  1.88919e-05,  4.97801e-05,  0.000125147,  0.000294363,  0.000625213,  0.00110982,  0.00128741,  -9.04157e-05,  -0.000408989,  0.000293741,  0.00226932,  0.00225775,  0.00382868,  0.0121236,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  },
    { 3.80658e-13,  1.16021e-12,  3.72154e-12,  1.18754e-11,  3.75522e-11,  1.18138e-10,  3.69787e-10,  1.1601e-09,  3.58161e-09,  1.09565e-08,  3.32266e-08,  9.96301e-08,  2.95059e-07,  8.60871e-07,  2.46788e-06,  6.92227e-06,  1.88917e-05,  4.97808e-05,  0.000125147,  0.000294363,  0.000625216,  0.00110982,  0.00128742,  -9.04109e-05,  -0.000408996,  0.000293736,  0.00226932,  0.00225775,  0.00382868,  0.0121236,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  },
    { 1.18398e-13,  3.62304e-13,  1.1667e-12,  3.73883e-12,  1.18785e-11,  3.75631e-11,  1.18256e-10,  3.7338e-10,  1.16109e-09,  3.58105e-09,  1.09618e-08,  3.32249e-08,  9.96401e-08,  2.95057e-07,  8.61036e-07,  2.46828e-06,  6.92219e-06,  1.8892e-05,  4.9781e-05,  0.000125148,  0.000294364,  0.000625216,  0.00110983,  0.00128743,  -9.04186e-05,  -0.000409002,  0.000293738,  0.00226933,  0.00225775,  0.00382868,  0.0121236,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  },
    { 3.67061e-14,  1.12735e-13,  3.64339e-13,  1.17212e-12,  3.73983e-12,  1.1882e-11,  3.76008e-11,  1.19404e-10,  3.73696e-10,  1.16092e-09,  3.5828e-09,  1.09612e-08,  3.32282e-08,  9.96394e-08,  2.95113e-07,  8.61175e-07,  2.46826e-06,  6.9223e-06,  1.88921e-05,  4.97811e-05,  0.000125148,  0.000294364,  0.000625221,  0.00110983,  0.00128742,  -9.04205e-05,  -0.000409002,  0.00029374,  0.00226933,  0.00225775,  0.00382868,  0.0121236,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  },
    { 1.13456e-14,  3.49636e-14,  1.13371e-13,  3.66029e-13,  1.17244e-12,  3.74092e-12,  1.18939e-11,  3.79656e-11,  1.19504e-10,  3.73643e-10,  1.16149e-09,  3.58262e-09,  1.09623e-08,  3.3228e-08,  9.96585e-08,  2.95161e-07,  8.61166e-07,  2.4683e-06,  6.92232e-06,  1.88921e-05,  4.97813e-05,  0.000125148,  0.000294367,  0.000625222,  0.00110983,  0.00128742,  -9.04209e-05,  -0.000409002,  0.00029374,  0.00226933,  0.00225775,  0.00382868,  0.0121236,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  },
    { 3.49711e-15,  1.08108e-14,  3.51616e-14,  1.13896e-13,  3.66134e-13,  1.17278e-12,  3.74469e-12,  1.20092e-11,  3.79974e-11,  1.19488e-10,  3.73827e-10,  1.16143e-09,  3.58299e-09,  1.09623e-08,  3.32344e-08,  9.96744e-08,  2.95157e-07,  8.6118e-07,  2.46831e-06,  6.92234e-06,  1.88922e-05,  4.97813e-05,  0.000125149,  0.000294367,  0.000625221,  0.00110983,  0.00128742,  -9.04214e-05,  -0.000409002,  0.00029374,  0.00226933,  0.00225775,  0.00382868,  0.0121236,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  },
    { 1.07514e-15,  3.33332e-15,  1.08722e-14,  3.53246e-14,  1.1393e-13,  3.66241e-13,  1.17397e-12,  3.78097e-12,  1.20192e-11,  3.79924e-11,  1.19547e-10,  3.73808e-10,  1.16155e-09,  3.58296e-09,  1.09644e-08,  3.32397e-08,  9.96734e-08,  2.95163e-07,  8.61184e-07,  2.46831e-06,  6.92236e-06,  1.88922e-05,  4.97817e-05,  0.000125149,  0.000294367,  0.000625222,  0.00110983,  0.00128742,  -9.04214e-05,  -0.000409003,  0.00029374,  0.00226933,  0.00225775,  0.00382868,  0.0121236,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  },
    { 3.29743e-16,  1.0251e-15,  3.35233e-15,  1.09226e-14,  3.53352e-14,  1.13963e-13,  3.66611e-13,  1.18534e-12,  3.78411e-12,  1.20177e-11,  3.80111e-11,  1.19541e-10,  3.73846e-10,  1.16154e-09,  3.58365e-09,  1.09661e-08,  3.32393e-08,  9.96751e-08,  2.95164e-07,  8.61185e-07,  2.46832e-06,  6.92237e-06,  1.88923e-05,  4.97818e-05,  0.000125149,  0.000294367,  0.000625222,  0.00110983,  0.00128742,  -9.04219e-05,  -0.000409003,  0.00029374,  0.00226933,  0.00225775,  0.00382868,  0.0121236,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  },
    { 1.00904e-16,  3.14481e-16,  1.03096e-15,  3.36785e-15,  1.0926e-14,  3.53455e-14,  1.14079e-13,  3.70159e-13,  1.18632e-12,  3.78364e-12,  1.20236e-11,  3.80093e-11,  1.19553e-10,  3.73844e-10,  1.16176e-09,  3.58422e-09,  1.0966e-08,  3.32399e-08,  9.96755e-08,  2.95164e-07,  8.61189e-07,  2.46832e-06,  6.92242e-06,  1.88923e-05,  4.97818e-05,  0.000125149,  0.000294367,  0.000625222,  0.00110983,  0.00128742,  -9.04217e-05,  -0.000409003,  0.00029374,  0.00226933,  0.00225775,  0.00382868,  0.0121236,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  },
    { 3.08122e-17,  9.62584e-17,  3.16284e-16,  1.03573e-15,  3.36891e-15,  1.09291e-14,  3.53814e-14,  1.15182e-13,  3.70464e-13,  1.18617e-12,  3.78551e-12,  1.2023e-11,  3.80132e-11,  1.19552e-10,  3.73916e-10,  1.16194e-09,  3.58418e-09,  1.09662e-08,  3.324e-08,  9.96758e-08,  2.95165e-07,  8.61189e-07,  2.46834e-06,  6.92243e-06,  1.88923e-05,  4.97818e-05,  0.000125149,  0.000294367,  0.000625222,  0.00110983,  0.00128742,  -9.04219e-05,  -0.000409004,  0.00029374,  0.00226933,  0.00225775,  0.00382868,  0.0121236,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  },
    { 9.39029e-18,  2.94009e-17,  9.68118e-17,  3.17748e-16,  1.03606e-15,  3.36989e-15,  1.09403e-14,  3.57234e-14,  1.15277e-13,  3.70421e-13,  1.18676e-12,  3.78533e-12,  1.20243e-11,  3.80129e-11,  1.19576e-10,  3.73975e-10,  1.16193e-09,  3.58425e-09,  1.09663e-08,  3.32401e-08,  9.96761e-08,  2.95166e-07,  8.61196e-07,  2.46835e-06,  6.92243e-06,  1.88924e-05,  4.97818e-05,  0.000125149,  0.000294367,  0.000625222,  0.00110983,  0.00128742,  -9.04221e-05,  -0.000409004,  0.00029374,  0.00226933,  0.00225775,  0.00382868,  0.0121236,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  },
    { 2.85646e-18,  8.96222e-18,  2.95703e-17,  9.72596e-17,  3.17852e-16,  1.03636e-15,  3.37332e-15,  1.1046e-14,  3.57528e-14,  1.15264e-13,  3.70605e-13,  1.18671e-12,  3.78572e-12,  1.20242e-11,  3.80203e-11,  1.19594e-10,  3.73971e-10,  1.16195e-09,  3.58426e-09,  1.09663e-08,  3.32402e-08,  9.96762e-08,  2.95168e-07,  8.61197e-07,  2.46835e-06,  6.92244e-06,  1.88924e-05,  4.97818e-05,  0.000125149,  0.000294367,  0.000625222,  0.00110983,  0.00128742,  -9.04221e-05,  -0.000409004,  0.00029374,  0.00226933,  0.00225775,  0.00382868,  0.0121236,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  },
    { 8.67394e-19,  2.72683e-18,  9.01398e-18,  2.9707e-17,  9.72919e-17,  3.17944e-16,  1.03742e-15,  3.40591e-15,  1.1055e-14,  3.57488e-14,  1.15321e-13,  3.70587e-13,  1.18683e-12,  3.7857e-12,  1.20265e-11,  3.80262e-11,  1.19593e-10,  3.73978e-10,  1.16196e-09,  3.58427e-09,  1.09663e-08,  3.32403e-08,  9.9677e-08,  2.95169e-07,  8.61197e-07,  2.46835e-06,  6.92244e-06,  1.88924e-05,  4.97818e-05,  0.000125149,  0.000294367,  0.000625222,  0.00110983,  0.00128742,  -9.04221e-05,  -0.000409004,  0.00029374,  0.00226933,  0.00225775,  0.00382868,  0.0121236,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  },
    { 2.62958e-19,  8.28197e-19,  2.74261e-18,  9.05565e-18,  2.9717e-17,  9.73202e-17,  3.18269e-16,  1.04744e-15,  3.40869e-15,  1.10538e-14,  3.57666e-14,  1.15315e-13,  3.70625e-13,  1.18682e-12,  3.78643e-12,  1.20284e-11,  3.80259e-11,  1.19595e-10,  3.73979e-10,  1.16196e-09,  3.58428e-09,  1.09663e-08,  3.32405e-08,  9.96771e-08,  2.95168e-07,  8.61198e-07,  2.46835e-06,  6.92244e-06,  1.88924e-05,  4.97818e-05,  0.000125149,  0.000294367,  0.000625222,  0.00110983,  0.00128742,  -9.04221e-05,  -0.000409004,  0.00029374,  0.00226933,  0.00225775,  0.00382868,  0.0121236,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  },
    { 7.95939e-20,  2.51123e-19,  8.32999e-19,  2.75528e-18,  9.05873e-18,  2.97257e-17,  9.74196e-17,  3.21341e-16,  1.04829e-15,  3.40833e-15,  1.10593e-14,  3.57649e-14,  1.15327e-13,  3.70623e-13,  1.18705e-12,  3.78702e-12,  1.20283e-11,  3.80266e-11,  1.19596e-10,  3.7398e-10,  1.16197e-09,  3.58429e-09,  1.09664e-08,  3.32406e-08,  9.96771e-08,  2.95169e-07,  8.61198e-07,  2.46835e-06,  6.92244e-06,  1.88924e-05,  4.97818e-05,  0.000125149,  0.000294367,  0.000625222,  0.00110983,  0.00128742,  -9.04221e-05,  -0.000409004,  0.00029374,  0.00226933,  0.00225775,  0.00382868,  0.0121236,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  },
    { 2.40564e-20,  7.60253e-20,  2.52582e-19,  8.36848e-19,  2.75623e-18,  9.06137e-18,  2.9756e-17,  9.83596e-17,  3.21602e-16,  1.04818e-15,  3.41002e-15,  1.10588e-14,  3.57686e-14,  1.15327e-13,  3.70695e-13,  1.18723e-12,  3.78698e-12,  1.20285e-11,  3.80267e-11,  1.19596e-10,  3.73982e-10,  1.16197e-09,  3.58431e-09,  1.09664e-08,  3.32406e-08,  9.96772e-08,  2.95169e-07,  8.61198e-07,  2.46835e-06,  6.92244e-06,  1.88924e-05,  4.97818e-05,  0.000125149,  0.000294367,  0.000625222,  0.00110983,  0.00128742,  -9.04221e-05,  -0.000409004,  0.00029374,  0.00226933,  0.00225775,  0.00382868,  0.0121236,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  },
    { 7.26062e-21,  2.29817e-20,  7.64677e-20,  2.53749e-19,  8.3714e-19,  2.75704e-18,  9.07064e-18,  3.00431e-17,  9.84395e-17,  3.21569e-16,  1.0487e-15,  3.40987e-15,  1.10599e-14,  3.57683e-14,  1.15349e-13,  3.70753e-13,  1.18722e-12,  3.78706e-12,  1.20286e-11,  3.80268e-11,  1.19597e-10,  3.73982e-10,  1.16198e-09,  3.58432e-09,  1.09664e-08,  3.32406e-08,  9.96772e-08,  2.95169e-07,  8.61198e-07,  2.46835e-06,  6.92244e-06,  1.88924e-05,  4.97818e-05,  0.000125149,  0.000294367,  0.000625222,  0.00110983,  0.00128742,  -9.04221e-05,  -0.000409004,  0.00029374,  0.00226933,  0.00225775,  0.00382868,  0.0121236,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  },
    { 2.18846e-21,  6.93739e-21,  2.31157e-20,  7.68208e-20,  2.53838e-19,  8.37384e-19,  2.75986e-18,  9.15812e-18,  3.00674e-17,  9.84296e-17,  3.21729e-16,  1.04865e-15,  3.41022e-15,  1.10599e-14,  3.57753e-14,  1.15367e-13,  3.70749e-13,  1.18725e-12,  3.78707e-12,  1.20286e-11,  3.8027e-11,  1.19597e-10,  3.73985e-10,  1.16198e-09,  3.58432e-09,  1.09664e-08,  3.32406e-08,  9.96772e-08,  2.95169e-07,  8.61198e-07,  2.46835e-06,  6.92244e-06,  1.88924e-05,  4.97818e-05,  0.000125149,  0.000294367,  0.000625222,  0.00110983,  0.00128742,  -9.04221e-05,  -0.000409004,  0.00029374,  0.00226933,  0.00225775,  0.00382868,  0.0121236,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  },
    { 6.58803e-22,  2.09136e-21,  6.97789e-21,  2.32224e-20,  7.68482e-20,  2.53912e-19,  8.38242e-19,  2.78647e-18,  9.16553e-18,  3.00645e-17,  9.84787e-17,  3.21715e-16,  1.04876e-15,  3.4102e-15,  1.1062e-14,  3.57808e-14,  1.15366e-13,  3.70756e-13,  1.18725e-12,  3.78708e-12,  1.20286e-11,  3.8027e-11,  1.19598e-10,  3.73986e-10,  1.16198e-09,  3.58432e-09,  1.09664e-08,  3.32406e-08,  9.96772e-08,  2.95169e-07,  8.61198e-07,  2.46835e-06,  6.92244e-06,  1.88924e-05,  4.97818e-05,  0.000125149,  0.000294367,  0.000625222,  0.00110983,  0.00128742,  -9.04221e-05,  -0.000409004,  0.00029374,  0.00226933,  0.00225775,  0.00382868,  0.0121236,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  },
    { 1.98084e-22,  6.29662e-22,  2.10358e-21,  7.01009e-21,  2.32308e-20,  7.68706e-20,  2.54173e-19,  8.46322e-19,  2.78872e-18,  9.16464e-18,  3.00795e-17,  9.84742e-17,  3.21748e-16,  1.04876e-15,  3.41086e-15,  1.10637e-14,  3.57805e-14,  1.15368e-13,  3.70758e-13,  1.18725e-12,  3.78709e-12,  1.20286e-11,  3.80273e-11,  1.19598e-10,  3.73985e-10,  1.16198e-09,  3.58432e-09,  1.09664e-08,  3.32406e-08,  9.96772e-08,  2.95169e-07,  8.61198e-07,  2.46835e-06,  6.92244e-06,  1.88924e-05,  4.97818e-05,  0.000125149,  0.000294367,  0.000625222,  0.00110983,  0.00128742,  -9.04221e-05,  -0.000409004,  0.00029374,  0.00226933,  0.00225775,  0.00382868,  0.0121236,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  },
    { 5.94904e-23,  1.89349e-22,  6.33349e-22,  2.11329e-21,  7.01264e-21,  2.32375e-20,  7.69494e-20,  2.56622e-19,  8.47004e-19,  2.78845e-18,  9.16922e-18,  3.00781e-17,  9.84844e-17,  3.21746e-16,  1.04896e-15,  3.41139e-15,  1.10636e-14,  3.57812e-14,  1.15368e-13,  3.70759e-13,  1.18726e-12,  3.7871e-12,  1.20287e-11,  3.80274e-11,  1.19598e-10,  3.73986e-10,  1.16198e-09,  3.58432e-09,  1.09664e-08,  3.32406e-08,  9.96773e-08,  2.95169e-07,  8.61198e-07,  2.46835e-06,  6.92244e-06,  1.88924e-05,  4.97818e-05,  0.000125149,  0.000294367,  0.000625222,  0.00110983,  0.00128742,  -9.04221e-05,  -0.000409004,  0.00029374,  0.00226933,  0.00225775,  0.00382868,  0.0121236,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  },
    { 1.78471e-23,  5.68743e-23,  1.90459e-22,  6.3627e-22,  2.11406e-21,  7.01468e-21,  2.32614e-20,  7.76908e-20,  2.56829e-19,  8.46925e-19,  2.78985e-18,  9.1688e-18,  3.00812e-17,  9.84837e-17,  3.21809e-16,  1.04912e-15,  3.41136e-15,  1.10638e-14,  3.57813e-14,  1.15369e-13,  3.7076e-13,  1.18726e-12,  3.78713e-12,  1.20288e-11,  3.80274e-11,  1.19598e-10,  3.73986e-10,  1.16198e-09,  3.58432e-09,  1.09664e-08,  3.32406e-08,  9.96773e-08,  2.95169e-07,  8.61198e-07,  2.46835e-06,  6.92244e-06,  1.88924e-05,  4.97818e-05,  0.000125149,  0.000294367,  0.000625222,  0.00110983,  0.00128742,  -9.04221e-05,  -0.000409004,  0.00029374,  0.00226933,  0.00225775,  0.00382868,  0.0121236,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  },
    { 5.34856e-24,  1.70645e-23,  5.72081e-23,  1.91337e-22,  6.36505e-22,  2.11468e-21,  7.02188e-21,  2.34855e-20,  7.77533e-20,  2.56805e-19,  8.47348e-19,  2.78972e-18,  9.16975e-18,  3.0081e-17,  9.8503e-17,  3.21858e-16,  1.04911e-15,  3.41142e-15,  1.10639e-14,  3.57814e-14,  1.15369e-13,  3.7076e-13,  1.18727e-12,  3.78713e-12,  1.20288e-11,  3.80274e-11,  1.19598e-10,  3.73986e-10,  1.16198e-09,  3.58432e-09,  1.09664e-08,  3.32406e-08,  9.96773e-08,  2.95169e-07,  8.61198e-07,  2.46835e-06,  6.92244e-06,  1.88924e-05,  4.97818e-05,  0.000125149,  0.000294367,  0.000625222,  0.00110983,  0.00128742,  -9.04221e-05,  -0.000409004,  0.00029374,  0.00226933,  0.00225775,  0.00382868,  0.0121236,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  },
    { 1.6013e-24,  5.11462e-24,  1.71647e-23,  5.74719e-23,  1.91408e-22,  6.3669e-22,  2.11685e-21,  7.08951e-21,  2.35043e-20,  7.77462e-20,  2.56933e-19,  8.47309e-19,  2.79001e-18,  9.16969e-18,  3.00869e-17,  9.85181e-17,  3.21855e-16,  1.04913e-15,  3.41144e-15,  1.10639e-14,  3.57815e-14,  1.15369e-13,  3.70763e-13,  1.18727e-12,  3.78713e-12,  1.20288e-11,  3.80274e-11,  1.19598e-10,  3.73986e-10,  1.16198e-09,  3.58432e-09,  1.09664e-08,  3.32406e-08,  9.96773e-08,  2.95169e-07,  8.61198e-07,  2.46835e-06,  6.92244e-06,  1.88924e-05,  4.97818e-05,  0.000125149,  0.000294367,  0.000625222,  0.00110983,  0.00128742,  -9.04221e-05,  -0.000409004,  0.00029374,  0.00226933,  0.00225775,  0.00382868,  0.0121236,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  },
    { 4.78953e-25,  1.53143e-24,  5.14471e-24,  1.72439e-23,  5.74935e-23,  1.91464e-22,  6.37345e-22,  2.13723e-21,  7.0952e-21,  2.35022e-20,  7.77851e-20,  2.56922e-19,  8.47397e-19,  2.78999e-18,  9.17148e-18,  3.00915e-17,  9.85172e-17,  3.21861e-16,  1.04914e-15,  3.41144e-15,  1.10639e-14,  3.57816e-14,  1.1537e-13,  3.70764e-13,  1.18727e-12,  3.78714e-12,  1.20288e-11,  3.80274e-11,  1.19598e-10,  3.73986e-10,  1.16198e-09,  3.58432e-09,  1.09664e-08,  3.32406e-08,  9.96773e-08,  2.95169e-07,  8.61198e-07,  2.46835e-06,  6.92244e-06,  1.88924e-05,  4.97818e-05,  0.000125149,  0.000294367,  0.000625222,  0.00110983,  0.00128742,  -9.04221e-05,  -0.000409004,  0.00029374,  0.00226933,  0.00225775,  0.00382868,  0.0121236,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  },
    { 1.43125e-25,  4.58107e-25,  1.54045e-24,  5.16843e-24,  1.72504e-23,  5.75102e-23,  1.91661e-22,  6.43481e-22,  2.13895e-21,  7.09457e-21,  2.3514e-20,  7.77816e-20,  2.56948e-19,  8.47392e-19,  2.79054e-18,  9.17289e-18,  3.00912e-17,  9.85191e-17,  3.21863e-16,  1.04914e-15,  3.41146e-15,  1.1064e-14,  3.57819e-14,  1.1537e-13,  3.70764e-13,  1.18727e-12,  3.78714e-12,  1.20288e-11,  3.80274e-11,  1.19598e-10,  3.73986e-10,  1.16198e-09,  3.58432e-09,  1.09664e-08,  3.32406e-08,  9.96773e-08,  2.95169e-07,  8.61198e-07,  2.46835e-06,  6.92244e-06,  1.88924e-05,  4.97818e-05,  0.000125149,  0.000294367,  0.000625222,  0.00110983,  0.00128742,  -9.04221e-05,  -0.000409004,  0.00029374,  0.00226933,  0.00225775,  0.00382868,  0.0121236,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  },
    { 4.27325e-26,  1.3691e-25,  4.60808e-25,  1.54755e-24,  5.17039e-24,  1.72554e-23,  5.75694e-23,  1.93506e-22,  6.43996e-22,  2.13876e-21,  7.09812e-21,  2.35129e-20,  7.77897e-20,  2.56947e-19,  8.47558e-19,  2.79096e-18,  9.1728e-18,  3.00918e-17,  9.85195e-17,  3.21864e-16,  1.04914e-15,  3.41146e-15,  1.1064e-14,  3.57819e-14,  1.1537e-13,  3.70764e-13,  1.18727e-12,  3.78714e-12,  1.20288e-11,  3.80274e-11,  1.19598e-10,  3.73986e-10,  1.16198e-09,  3.58432e-09,  1.09664e-08,  3.32406e-08,  9.96773e-08,  2.95169e-07,  8.61198e-07,  2.46835e-06,  6.92244e-06,  1.88924e-05,  4.97818e-05,  0.000125149,  0.000294367,  0.000625222,  0.00110983,  0.00128742,  -9.04221e-05,  -0.000409004,  0.00029374,  0.00226933,  0.00225775,  0.00382868,  0.0121236,  0,  0,  0,  0,  0,  0,  0,  0,  0,  },
    { 1.27478e-26,  4.0881e-26,  1.37718e-25,  4.62931e-25,  1.54814e-24,  5.17189e-24,  1.72732e-23,  5.81234e-23,  1.9366e-22,  6.43941e-22,  2.13983e-21,  7.0978e-21,  2.35154e-20,  7.77892e-20,  2.56997e-19,  8.47687e-19,  2.79094e-18,  9.17298e-18,  3.0092e-17,  9.85198e-17,  3.21865e-16,  1.04914e-15,  3.41149e-15,  1.10641e-14,  3.57819e-14,  1.1537e-13,  3.70764e-13,  1.18727e-12,  3.78714e-12,  1.20288e-11,  3.80274e-11,  1.19598e-10,  3.73986e-10,  1.16198e-09,  3.58432e-09,  1.09664e-08,  3.32406e-08,  9.96773e-08,  2.95169e-07,  8.61198e-07,  2.46835e-06,  6.92244e-06,  1.88924e-05,  4.97818e-05,  0.000125149,  0.000294367,  0.000625222,  0.00110983,  0.00128742,  -9.04221e-05,  -0.000409004,  0.00029374,  0.00226933,  0.00225775,  0.00382868,  0.0121236,  0,  0,  0,  0,  0,  0,  0,  0,  },
    { 3.7998e-27,  1.21967e-26,  4.11225e-26,  1.38353e-25,  4.63109e-25,  1.54859e-24,  5.17722e-24,  1.74394e-23,  5.81699e-23,  1.93644e-22,  6.44263e-22,  2.13973e-21,  7.09854e-21,  2.35152e-20,  7.78044e-20,  2.57036e-19,  8.4768e-19,  2.79099e-18,  9.17302e-18,  3.0092e-17,  9.85201e-17,  3.21865e-16,  1.04915e-15,  3.41149e-15,  1.10641e-14,  3.57819e-14,  1.1537e-13,  3.70764e-13,  1.18727e-12,  3.78714e-12,  1.20288e-11,  3.80274e-11,  1.19598e-10,  3.73986e-10,  1.16198e-09,  3.58432e-09,  1.09664e-08,  3.32406e-08,  9.96773e-08,  2.95169e-07,  8.61198e-07,  2.46835e-06,  6.92244e-06,  1.88924e-05,  4.97818e-05,  0.000125149,  0.000294367,  0.000625222,  0.00110983,  0.00128742,  -9.04221e-05,  -0.000409004,  0.00029374,  0.00226933,  0.00225775,  0.00382868,  0.0121236,  0,  0,  0,  0,  0,  0,  0,  },
    { 1.13175e-27,  3.63586e-27,  1.22688e-26,  4.1312e-26,  1.38406e-25,  4.63243e-25,  1.55019e-24,  5.22703e-24,  1.74533e-23,  5.8165e-23,  1.93741e-22,  6.44234e-22,  2.13996e-21,  7.09849e-21,  2.35198e-20,  7.78163e-20,  2.57034e-19,  8.47696e-19,  2.791e-18,  9.17304e-18,  3.00921e-17,  9.85202e-17,  3.21868e-16,  1.04915e-15,  3.41149e-15,  1.10641e-14,  3.57819e-14,  1.1537e-13,  3.70764e-13,  1.18727e-12,  3.78714e-12,  1.20288e-11,  3.80274e-11,  1.19598e-10,  3.73986e-10,  1.16198e-09,  3.58432e-09,  1.09664e-08,  3.32406e-08,  9.96773e-08,  2.95169e-07,  8.61198e-07,  2.46835e-06,  6.92244e-06,  1.88924e-05,  4.97818e-05,  0.000125149,  0.000294367,  0.000625222,  0.00110983,  0.00128742,  -9.04221e-05,  -0.000409004,  0.00029374,  0.00226933,  0.00225775,  0.00382868,  0.0121236,  0,  0,  0,  0,  0,  0,  },
    { 3.36833e-28,  1.08302e-27,  3.65738e-27,  1.23253e-26,  4.1328e-26,  1.38446e-25,  4.63721e-25,  1.5651e-24,  5.2312e-24,  1.74518e-23,  5.81941e-23,  1.93732e-22,  6.44301e-22,  2.13994e-21,  7.09989e-21,  2.35234e-20,  7.78156e-20,  2.57039e-19,  8.477e-19,  2.79101e-18,  9.17308e-18,  3.00922e-17,  9.8521e-17,  3.21868e-16,  1.04915e-15,  3.41149e-15,  1.10641e-14,  3.5782e-14,  1.1537e-13,  3.70764e-13,  1.18727e-12,  3.78714e-12,  1.20288e-11,  3.80274e-11,  1.19598e-10,  3.73986e-10,  1.16198e-09,  3.58432e-09,  1.09664e-08,  3.32406e-08,  9.96773e-08,  2.95169e-07,  8.61198e-07,  2.46835e-06,  6.92244e-06,  1.88924e-05,  4.97818e-05,  0.000125149,  0.000294367,  0.000625222,  0.00110983,  0.00128742,  -9.04221e-05,  -0.000409004,  0.00029374,  0.00226933,  0.00225775,  0.00382868,  0.0121236,  0,  0,  0,  0,  0,  },
    { 1.00177e-28,  3.22356e-28,  1.08943e-27,  3.67422e-27,  1.23301e-26,  4.134e-26,  1.38589e-25,  4.68182e-25,  1.56635e-24,  5.23077e-24,  1.74606e-23,  5.81915e-23,  1.93753e-22,  6.44297e-22,  2.14036e-21,  7.10097e-21,  2.35232e-20,  7.78171e-20,  2.5704e-19,  8.47702e-19,  2.79102e-18,  9.17309e-18,  3.00924e-17,  9.85211e-17,  3.21868e-16,  1.04916e-15,  3.41149e-15,  1.10641e-14,  3.57819e-14,  1.1537e-13,  3.70764e-13,  1.18727e-12,  3.78714e-12,  1.20288e-11,  3.80274e-11,  1.19598e-10,  3.73986e-10,  1.16198e-09,  3.58432e-09,  1.09664e-08,  3.32406e-08,  9.96773e-08,  2.95169e-07,  8.61198e-07,  2.46835e-06,  6.92244e-06,  1.88924e-05,  4.97818e-05,  0.000125149,  0.000294367,  0.000625222,  0.00110983,  0.00128742,  -9.04221e-05,  -0.000409004,  0.00029374,  0.00226933,  0.00225775,  0.00382868,  0.0121236,  0,  0,  0,  0,  },
    { 2.97726e-29,  9.58791e-29,  3.24267e-28,  1.09445e-27,  3.67566e-27,  1.23337e-26,  4.13827e-26,  1.39922e-25,  4.68554e-25,  1.56622e-24,  5.23339e-24,  1.74598e-23,  5.81976e-23,  1.93751e-22,  6.44424e-22,  2.14069e-21,  7.1009e-21,  2.35237e-20,  7.78174e-20,  2.57041e-19,  8.47705e-19,  2.79102e-18,  9.17316e-18,  3.00925e-17,  9.85212e-17,  3.21868e-16,  1.04916e-15,  3.4115e-15,  1.10641e-14,  3.5782e-14,  1.1537e-13,  3.70764e-13,  1.18727e-12,  3.78714e-12,  1.20288e-11,  3.80274e-11,  1.19598e-10,  3.73986e-10,  1.16198e-09,  3.58432e-09,  1.09664e-08,  3.32406e-08,  9.96773e-08,  2.95169e-07,  8.61198e-07,  2.46835e-06,  6.92244e-06,  1.88924e-05,  4.97818e-05,  0.000125149,  0.000294367,  0.000625222,  0.00110983,  0.00128742,  -9.04221e-05,  -0.000409004,  0.00029374,  0.00226933,  0.00225775,  0.00382868,  0.0121236,  0,  0,  0,  },
    { 8.84255e-30,  2.84977e-29,  9.64478e-29,  3.2576e-28,  1.09488e-27,  3.67673e-27,  1.23464e-26,  4.17806e-26,  1.40033e-25,  4.68516e-25,  1.56701e-24,  5.23316e-24,  1.74616e-23,  5.81972e-23,  1.93789e-22,  6.44522e-22,  2.14067e-21,  7.10105e-21,  2.35238e-20,  7.78176e-20,  2.57042e-19,  8.47706e-19,  2.79105e-18,  9.17317e-18,  3.00925e-17,  9.85212e-17,  3.21868e-16,  1.04916e-15,  3.4115e-15,  1.10641e-14,  3.5782e-14,  1.1537e-13,  3.70764e-13,  1.18727e-12,  3.78714e-12,  1.20288e-11,  3.80274e-11,  1.19598e-10,  3.73986e-10,  1.16198e-09,  3.58432e-09,  1.09664e-08,  3.32406e-08,  9.96773e-08,  2.95169e-07,  8.61198e-07,  2.46835e-06,  6.92244e-06,  1.88924e-05,  4.97818e-05,  0.000125149,  0.000294367,  0.000625222,  0.00110983,  0.00128742,  -9.04221e-05,  -0.000409004,  0.00029374,  0.00226933,  0.00225775,  0.00382868,  0.0121236,  0,  0,  },
    { 2.62457e-30,  8.46455e-30,  2.86668e-29,  9.68918e-29,  3.25889e-28,  1.09519e-27,  3.68053e-27,  1.24651e-26,  4.18138e-26,  1.40022e-25,  4.68751e-25,  1.56694e-24,  5.2337e-24,  1.74615e-23,  5.82086e-23,  1.93819e-22,  6.44516e-22,  2.14071e-21,  7.10107e-21,  2.35238e-20,  7.78179e-20,  2.57042e-19,  8.47713e-19,  2.79105e-18,  9.17317e-18,  3.00925e-17,  9.85212e-17,  3.21868e-16,  1.04916e-15,  3.4115e-15,  1.10641e-14,  3.5782e-14,  1.15371e-13,  3.70764e-13,  1.18727e-12,  3.78714e-12,  1.20288e-11,  3.80274e-11,  1.19598e-10,  3.73986e-10,  1.16198e-09,  3.58432e-09,  1.09664e-08,  3.32406e-08,  9.96773e-08,  2.95169e-07,  8.61198e-07,  2.46835e-06,  6.92244e-06,  1.88924e-05,  4.97818e-05,  0.000125149,  0.000294367,  0.000625222,  0.00110983,  0.00128742,  -9.04221e-05,  -0.000409004,  0.00029374,  0.00226933,  0.00225775,  0.00382868,  0.0121236,  0,  },
    { 7.7852e-31,  2.51256e-30,  8.51483e-30,  2.87988e-29,  9.69303e-29,  3.25983e-28,  1.09633e-27,  3.71591e-27,  1.2475e-26,  4.18105e-26,  1.40092e-25,  4.6873e-25,  1.5671e-24,  5.23367e-24,  1.7465e-23,  5.82175e-23,  1.93817e-22,  6.44529e-22,  2.14072e-21,  7.10109e-21,  2.35239e-20,  7.7818e-20,  2.57044e-19,  8.47714e-19,  2.79105e-18,  9.17318e-18,  3.00925e-17,  9.85212e-17,  3.21868e-16,  1.04916e-15,  3.4115e-15,  1.10641e-14,  3.5782e-14,  1.15371e-13,  3.70764e-13,  1.18727e-12,  3.78714e-12,  1.20288e-11,  3.80274e-11,  1.19598e-10,  3.73986e-10,  1.16198e-09,  3.58432e-09,  1.09664e-08,  3.32406e-08,  9.96773e-08,  2.95169e-07,  8.61198e-07,  2.46835e-06,  6.92244e-06,  1.88924e-05,  4.97818e-05,  0.000125149,  0.000294367,  0.000625222,  0.00110983,  0.00128742,  -9.04221e-05,  -0.000409004,  0.00029374,  0.00226933,  0.00225775,  0.00382868,  0.0121236,  },
  };


//    for (int iii = 0; iii < 64; iii++) {
//      for (int jjj = 0; jjj < 64; jjj++) {
//        MatrixParallel[iii][jjj] = LMatrixParallel[iii][jjj];
//        MatrixSerial[iii][jjj] = LMatrixSerial[iii][jjj];

//      }
//    }

}

double TrgEclFAM::GetTCLatency(int TCId)
{

  //----------------------------------------------------------
  // TC latency = Flight time of slowest TC - Other TCs
  // Flight time was calculated supposing particle is gamma.
  //----------------------------------------------------------


  double TCLatency[576] =   { 0.715606, 0.417428, 4.32053e-07, 0.0101334, 0.41738, 0.715606, 0.417427, 2.63428e-06, 0.0101316, 0.417379, 0.715606, 0.417428, 0, 0.0101295, 0.41738, 0.715606, 0.417427, 5.84426e-06, 0.0101284, 0.41738, 0.715606, 0.417428, 4.32053e-07, 0.0101334, 0.41738, 0.715606, 0.417427, 2.63428e-06, 0.0101316, 0.417379, 0.715606, 0.417428, 0, 0.0101295, 0.41738, 0.715606, 0.417427, 5.84426e-06, 0.0101284, 0.41738, 0.715606, 0.417428, 4.32053e-07, 0.0101334, 0.41738, 0.715606, 0.417427, 2.63428e-06, 0.0101316, 0.417379, 0.715606, 0.417428, 0, 0.0101295, 0.41738, 0.715606, 0.417427, 5.84426e-06, 0.0101284, 0.41738, 0.715606, 0.417428, 4.32053e-07, 0.0101334, 0.41738, 0.715606, 0.417427, 2.63428e-06, 0.0101316, 0.417379, 0.715606, 0.417428, 0, 0.0101295, 0.41738, 0.715606, 0.417427, 5.84426e-06, 0.0101284, 0.41738, 0.0919657, 1.0329, 1.78857, 2.38025, 2.82433, 3.13196, 3.30927, 3.34687, 3.23671, 2.99469, 2.61992, 2.24861, 0.0919505, 1.03291, 1.78857, 2.38026, 2.82432, 3.13194, 3.30926, 3.34688, 3.23672, 2.99469, 2.61991, 2.24862, 0.0919648, 1.03289, 1.78857, 2.38026, 2.82433, 3.13195, 3.30928, 3.34687, 3.23672, 2.9947, 2.61993, 2.2486, 0.0919576, 1.0329, 1.78859, 2.38026, 2.82433, 3.13197, 3.30926, 3.34688, 3.23671, 2.99469, 2.61991, 2.2486, 0.0919594, 1.0329, 1.78858, 2.38025, 2.82433, 3.13195, 3.30927, 3.34688, 3.23671, 2.9947, 2.61992, 2.24861, 0.0919549, 1.0329, 1.78859, 2.38024, 2.82433, 3.13196, 3.30927, 3.34687, 3.23671, 2.9947, 2.61992, 2.24861, 0.0919564, 1.03289, 1.78858, 2.38024, 2.82433, 3.13197, 3.30927, 3.34688, 3.2367, 2.99469, 2.61991, 2.24861, 0.0919696, 1.0329, 1.78859, 2.38026, 2.82434, 3.13195, 3.30926, 3.34687, 3.23671, 2.99469, 2.61991, 2.24862, 0.0919613, 1.0329, 1.7886, 2.38024, 2.82432, 3.13197, 3.30925, 3.34686, 3.23672, 2.99471, 2.61992, 2.24861, 0.0919657, 1.0329, 1.78857, 2.38025, 2.82433, 3.13196, 3.30927, 3.34687, 3.23671, 2.99469, 2.61992, 2.24861, 0.0919505, 1.03291, 1.78857, 2.38026, 2.82432, 3.13194, 3.30926, 3.34688, 3.23672, 2.99469, 2.61991, 2.24862, 0.0919648, 1.03289, 1.78857, 2.38026, 2.82433, 3.13195, 3.30928, 3.34687, 3.23672, 2.9947, 2.61993, 2.2486, 0.0919576, 1.0329, 1.78859, 2.38026, 2.82433, 3.13197, 3.30926, 3.34688, 3.23671, 2.99469, 2.61991, 2.2486, 0.0919594, 1.0329, 1.78858, 2.38025, 2.82433, 3.13195, 3.30927, 3.34688, 3.23671, 2.9947, 2.61992, 2.24861, 0.0919549, 1.0329, 1.78859, 2.38024, 2.82433, 3.13196, 3.30927, 3.34687, 3.23671, 2.9947, 2.61992, 2.24861, 0.0919564, 1.03289, 1.78858, 2.38024, 2.82433, 3.13197, 3.30927, 3.34688, 3.2367, 2.99469, 2.61991, 2.24861, 0.0919696, 1.0329, 1.78859, 2.38026, 2.82434, 3.13195, 3.30926, 3.34687, 3.23671, 2.99469, 2.61991, 2.24862, 0.0919613, 1.0329, 1.7886, 2.38024, 2.82432, 3.13197, 3.30925, 3.34686, 3.23672, 2.99471, 2.61992, 2.24861, 0.0919657, 1.0329, 1.78857, 2.38025, 2.82433, 3.13196, 3.30927, 3.34687, 3.23671, 2.99469, 2.61992, 2.24861, 0.0919505, 1.03291, 1.78857, 2.38026, 2.82432, 3.13194, 3.30926, 3.34688, 3.23672, 2.99469, 2.61991, 2.24862, 0.0919648, 1.03289, 1.78857, 2.38026, 2.82433, 3.13195, 3.30928, 3.34687, 3.23672, 2.9947, 2.61993, 2.2486, 0.0919576, 1.0329, 1.78859, 2.38026, 2.82433, 3.13197, 3.30926, 3.34688, 3.23671, 2.99469, 2.61991, 2.2486, 0.0919594, 1.0329, 1.78858, 2.38025, 2.82433, 3.13195, 3.30927, 3.34688, 3.23671, 2.9947, 2.61992, 2.24861, 0.0919549, 1.0329, 1.78859, 2.38024, 2.82433, 3.13196, 3.30927, 3.34687, 3.23671, 2.9947, 2.61992, 2.24861, 0.0919564, 1.03289, 1.78858, 2.38024, 2.82433, 3.13197, 3.30927, 3.34688, 3.2367, 2.99469, 2.61991, 2.24861, 0.0919696, 1.0329, 1.78859, 2.38026, 2.82434, 3.13195, 3.30926, 3.34687, 3.23671, 2.99469, 2.61991, 2.24862, 0.0919613, 1.0329, 1.7886, 2.38024, 2.82432, 3.13197, 3.30925, 3.34686, 3.23672, 2.99471, 2.61992, 2.24861, 0.0919657, 1.0329, 1.78857, 2.38025, 2.82433, 3.13196, 3.30927, 3.34687, 3.23671, 2.99469, 2.61992, 2.24861, 0.0919505, 1.03291, 1.78857, 2.38026, 2.82432, 3.13194, 3.30926, 3.34688, 3.23672, 2.99469, 2.61991, 2.24862, 0.0919648, 1.03289, 1.78857, 2.38026, 2.82433, 3.13195, 3.30928, 3.34687, 3.23672, 2.9947, 2.61993, 2.2486, 0.0919576, 1.0329, 1.78859, 2.38026, 2.82433, 3.13197, 3.30926, 3.34688, 3.23671, 2.99469, 2.61991, 2.2486, 0.0919594, 1.0329, 1.78858, 2.38025, 2.82433, 3.13195, 3.30927, 3.34688, 3.23671, 2.9947, 2.61992, 2.24861, 0.0919549, 1.0329, 1.78859, 2.38024, 2.82433, 3.13196, 3.30927, 3.34687, 3.23671, 2.9947, 2.61992, 2.24861, 0.0919564, 1.03289, 1.78858, 2.38024, 2.82433, 3.13197, 3.30927, 3.34688, 3.2367, 2.99469, 2.61991, 2.24861, 0.0919696, 1.0329, 1.78859, 2.38026, 2.82434, 3.13195, 3.30926, 3.34687, 3.23671, 2.99469, 2.61991, 2.24862, 0.0919613, 1.0329, 1.7886, 2.38024, 2.82432, 3.13197, 3.30925, 3.34686, 3.23672, 2.99471, 2.61992, 2.24861, 3.37164, 2.58506, 2.56517, 3.37178, 3.37164, 2.58506, 2.56517, 3.37178, 3.37164, 2.58505, 2.56516, 3.37178, 3.37164, 2.58505, 2.56517, 3.37178, 3.37164, 2.58506, 2.56517, 3.37178, 3.37164, 2.58506, 2.56517, 3.37178, 3.37164, 2.58505, 2.56516, 3.37178, 3.37164, 2.58505, 2.56517, 3.37178, 3.37164, 2.58506, 2.56517, 3.37178, 3.37164, 2.58506, 2.56517, 3.37178, 3.37164, 2.58505, 2.56516, 3.37178, 3.37164, 2.58505, 2.56517, 3.37178, 3.37164, 2.58506, 2.56517, 3.37178, 3.37164, 2.58506, 2.56517, 3.37178, 3.37164, 2.58505, 2.56516, 3.37178, 3.37164, 2.58505, 2.56517, 3.37178};


  return TCLatency[TCId - 1];
}


void
TrgEclFAM::readFAMDB(void)
{
  //  double par_f0[20][12] ={

  CoeffSigPDF0 =  {
    { 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.13653, 0.54824, 0.86612, 0.99103, 0.98471, 0.80303},
    { 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.00016, 0.17169, 0.58834, 0.88675, 0.99502, 0.97827, 0.77270},
    { 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.00113, 0.20964, 0.62692, 0.90539, 0.99778, 0.96963, 0.74131},
    { 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.00401, 0.24979, 0.66379, 0.92209, 0.99942, 0.95834, 0.70919},
    { 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.00994, 0.29156, 0.69881, 0.93692, 1.00000, 0.94415, 0.67666},
    { 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.01980, 0.33439, 0.73187, 0.94996, 0.99960, 0.92700, 0.64398},
    { 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.03415, 0.37779, 0.76290, 0.96127, 0.99829, 0.90698, 0.61140},
    { 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.05317, 0.42126, 0.79184, 0.97096, 0.99614, 0.88428, 0.57915},
    { 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.07679, 0.46440, 0.81870, 0.97909, 0.99323, 0.85917, 0.54742},
    { 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.10472, 0.50684, 0.84345, 0.98575, 0.98951, 0.83197, 0.51637},
    { 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.13653, 0.54824, 0.86612, 0.99103, 0.98471, 0.80303, 0.48613},
    { 0.00000, 0.00000, 0.00000, 0.00000, 0.00016, 0.17169, 0.58834, 0.88675, 0.99502, 0.97827, 0.77270, 0.45682},
    { 0.00000, 0.00000, 0.00000, 0.00000, 0.00113, 0.20964, 0.62692, 0.90539, 0.99778, 0.96963, 0.74131, 0.42853},
    { 0.00000, 0.00000, 0.00000, 0.00000, 0.00401, 0.24979, 0.66379, 0.92209, 0.99942, 0.95834, 0.70919, 0.40133},
    { 0.00000, 0.00000, 0.00000, 0.00000, 0.00994, 0.29156, 0.69881, 0.93692, 1.00000, 0.94415, 0.67666, 0.37526},
    { 0.00000, 0.00000, 0.00000, 0.00000, 0.01980, 0.33439, 0.73187, 0.94996, 0.99960, 0.92700, 0.64398, 0.35035},
    { 0.00000, 0.00000, 0.00000, 0.00000, 0.03415, 0.37779, 0.76290, 0.96127, 0.99829, 0.90698, 0.61140, 0.32664},
    { 0.00000, 0.00000, 0.00000, 0.00000, 0.05317, 0.42126, 0.79184, 0.97096, 0.99614, 0.88428, 0.57915, 0.30411},
    { 0.00000, 0.00000, 0.00000, 0.00000, 0.07679, 0.46440, 0.81870, 0.97909, 0.99323, 0.85917, 0.54742, 0.28278},
    { 0.00000, 0.00000, 0.00000, 0.00000, 0.10472, 0.50684, 0.84345, 0.98575, 0.98951, 0.83197, 0.51637, 0.26261}
  };

  //double par_f1[20][12]={
  CoeffSigPDF1 = {
    { 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.00190, 2.68625, 3.26360, 1.73168, 0.36951, -0.44162, -2.37567},
    { 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.03251, 2.93224, 3.14993, 1.56999, 0.26902, -0.59639, -2.47343},
    { 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.13798, 3.13162, 3.02000, 1.41261, 0.17507, -0.79291, -2.54433},
    { 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.33813, 3.28419, 2.87710, 1.26028, 0.08751, -1.01714, -2.58993},
    { 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.62203, 3.39140, 2.72428, 1.11361, 0.00612, -1.25359, -2.61222},
    { 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.96356, 3.45582, 2.56427, 0.97304, -0.06932, -1.48875, -2.61338},
    { 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 1.33403, 3.48076, 2.39950, 0.83889, -0.13904, -1.71217, -2.59575},
    { 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 1.70835, 3.46999, 2.23212, 0.71138, -0.20331, -1.91641, -2.56167},
    { 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 2.06701, 3.42752, 2.06399, 0.59064, -0.26342, -2.09664, -2.51342},
    { 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 2.39605, 3.35740, 1.89672, 0.47669, -0.33434, -2.25010, -2.45320},
    { 0.00000, 0.00000, 0.00000, 0.00000, 0.00190, 2.68625, 3.26360, 1.73168, 0.36951, -0.44162, -2.37567, -2.38308},
    { 0.00000, 0.00000, 0.00000, 0.00000, 0.03251, 2.93224, 3.14993, 1.56999, 0.26902, -0.59639, -2.47343, -2.30496},
    { 0.00000, 0.00000, 0.00000, 0.00000, 0.13798, 3.13162, 3.02000, 1.41261, 0.17507, -0.79291, -2.54433, -2.22059},
    { 0.00000, 0.00000, 0.00000, 0.00000, 0.33813, 3.28419, 2.87710, 1.26028, 0.08751, -1.01714, -2.58993, -2.13155},
    { 0.00000, 0.00000, 0.00000, 0.00000, 0.62203, 3.39140, 2.72428, 1.11361, 0.00612, -1.25359, -2.61222, -2.03923},
    { 0.00000, 0.00000, 0.00000, 0.00000, 0.96356, 3.45582, 2.56427, 0.97304, -0.06932, -1.48875, -2.61338, -1.94489},
    { 0.00000, 0.00000, 0.00000, 0.00000, 1.33403, 3.48076, 2.39950, 0.83889, -0.13904, -1.71217, -2.59575, -1.84959},
    { 0.00000, 0.00000, 0.00000, 0.00000, 1.70835, 3.46999, 2.23212, 0.71138, -0.20331, -1.91641, -2.56167, -1.75428},
    { 0.00000, 0.00000, 0.00000, 0.00000, 2.06701, 3.42752, 2.06399, 0.59064, -0.26342, -2.09664, -2.51342, -1.65976},
    { 0.00000, 0.00000, 0.00000, 0.00000, 2.39605, 3.35740, 1.89672, 0.47669, -0.33434, -2.25010, -2.45320, -1.56669}
  };

//  double par_fg31[20][12]={
  CoeffNoise31 = {
    { -0.34005, -0.16500, -0.12133, -0.14945, -0.02294, -0.27032, -0.04882, 0.07051, 0.23329, 0.31887, 0.28760, 0.20763},
    { -0.35547, -0.16021, -0.11596, -0.15064, -0.01830, -0.29183, -0.02377, 0.08174, 0.24060, 0.31865, 0.28428, 0.19091},
    { -0.37239, -0.15395, -0.11024, -0.15206, -0.01374, -0.31398, 0.00497, 0.09328, 0.24787, 0.31939, 0.27816, 0.17271},
    { -0.39133, -0.14602, -0.10408, -0.15367, -0.01018, -0.33528, 0.03779, 0.10590, 0.25532, 0.32123, 0.26914, 0.15118},
    { -0.41201, -0.13631, -0.09786, -0.15545, -0.00889, -0.35288, 0.07388, 0.11939, 0.26266, 0.32458, 0.25714, 0.12574},
    { -0.43337, -0.12515, -0.09225, -0.15738, -0.01133, -0.36298, 0.11091, 0.13305, 0.26934, 0.32987, 0.24251, 0.09676},
    { -0.45385, -0.11331, -0.08798, -0.15932, -0.01864, -0.36208, 0.14546, 0.14600, 0.27477, 0.33730, 0.22617, 0.06548},
    { -0.47182, -0.10183, -0.08556, -0.16112, -0.03113, -0.34844, 0.17409, 0.15743, 0.27853, 0.34663, 0.20950, 0.03374},
    { -0.48610, -0.09163, -0.08514, -0.16262, -0.04805, -0.32289, 0.19455, 0.16682, 0.28056, 0.35706, 0.19393, 0.00349},
    { -0.49663, -0.08296, -0.08634, -0.16328, -0.06861, -0.28817, 0.20715, 0.17407, 0.28207, 0.36616, 0.18112, -0.02459},
    { -0.50436, -0.07532, -0.08854, -0.16229, -0.09272, -0.24710, 0.21418, 0.17941, 0.28545, 0.36998, 0.17275, -0.05144},
    { -0.51043, -0.06845, -0.09099, -0.16057, -0.11723, -0.20292, 0.21831, 0.18315, 0.29200, 0.36702, 0.16858, -0.07848},
    { -0.51602, -0.06259, -0.09285, -0.16106, -0.13553, -0.15904, 0.22204, 0.18578, 0.30153, 0.35894, 0.16623, -0.10741},
    { -0.52170, -0.05813, -0.09361, -0.16614, -0.14256, -0.11761, 0.22658, 0.18749, 0.31337, 0.34769, 0.16369, -0.13907},
    { -0.52741, -0.05553, -0.09316, -0.17688, -0.13571, -0.08045, 0.23188, 0.18820, 0.32685, 0.33462, 0.16028, -0.17268},
    { -0.53284, -0.05524, -0.09163, -0.19315, -0.11464, -0.04969, 0.23726, 0.18771, 0.34119, 0.32090, 0.15621, -0.20608},
    { -0.53767, -0.05736, -0.08931, -0.21367, -0.08143, -0.02722, 0.24201, 0.18605, 0.35540, 0.30775, 0.15204, -0.23659},
    { -0.54169, -0.06148, -0.08656, -0.23634, -0.04034, -0.01368, 0.24573, 0.18348, 0.36848, 0.29623, 0.14827, -0.26211},
    { -0.54485, -0.06684, -0.08369, -0.25891, 0.00343, -0.00800, 0.24836, 0.18053, 0.37956, 0.28703, 0.14513, -0.28174},
    { -0.54718, -0.07254, -0.08040, -0.28056, 0.04593, -0.00725, 0.25020, 0.17864, 0.38661, 0.28083, 0.14211, -0.29638}
  };

// double par_fg32[20][12]={

  CoeffNoise32 = {
    { -0.07680, 0.00074, 0.04672, 0.00598, 0.02151, -0.09937, 0.07807, 0.11668, 0.05597, -0.01337, -0.00295, -0.13318},
    { -0.07820, 0.01135, 0.04339, 0.00338, 0.02373, -0.10586, 0.10140, 0.10029, 0.04966, -0.01285, -0.01141, -0.12488},
    { -0.07873, 0.02017, 0.04065, 0.00144, 0.02418, -0.10718, 0.11882, 0.08745, 0.04387, -0.01080, -0.02133, -0.11854},
    { -0.07876, 0.02748, 0.03832, 0.00014, 0.02204, -0.10229, 0.13109, 0.07806, 0.03874, -0.00789, -0.03124, -0.11567},
    { -0.07820, 0.03359, 0.03604, -0.00066, 0.01712, -0.09097, 0.13884, 0.07119, 0.03404, -0.00441, -0.04056, -0.11602},
    { -0.07662, 0.03857, 0.03348, -0.00103, 0.00954, -0.07328, 0.14202, 0.06579, 0.02942, -0.00050, -0.04891, -0.11848},
    { -0.07351, 0.04231, 0.03042, -0.00103, -0.00023, -0.04991, 0.14031, 0.06102, 0.02462, 0.00361, -0.05593, -0.12169},
    { -0.06856, 0.04467, 0.02687, -0.00067, -0.01136, -0.02238, 0.13371, 0.05633, 0.01954, 0.00755, -0.06126, -0.12443},
    { -0.06185, 0.04566, 0.02302, 0.00004, -0.02284, 0.00706, 0.12294, 0.05148, 0.01431, 0.01082, -0.06472, -0.12592},
    { -0.05384, 0.04559, 0.01917, 0.00140, -0.03417, 0.03616, 0.10946, 0.04638, 0.00984, 0.01183, -0.06573, -0.12607},
    { -0.04527, 0.04493, 0.01560, 0.00378, -0.04534, 0.06326, 0.09507, 0.04098, 0.00750, 0.00846, -0.06357, -0.12541},
    { -0.03692, 0.04387, 0.01270, 0.00626, -0.05416, 0.08670, 0.08150, 0.03544, 0.00754, 0.00091, -0.05908, -0.12476},
    { -0.02945, 0.04232, 0.01081, 0.00692, -0.05696, 0.10514, 0.07005, 0.03007, 0.00922, -0.00869, -0.05435, -0.12510},
    { -0.02302, 0.04019, 0.01001, 0.00474, -0.05209, 0.11850, 0.06100, 0.02498, 0.01176, -0.01864, -0.05067, -0.12677},
    { -0.01742, 0.03737, 0.01013, -0.00029, -0.03978, 0.12702, 0.05386, 0.02007, 0.01461, -0.02812, -0.04828, -0.12918},
    { -0.01229, 0.03380, 0.01092, -0.00757, -0.02131, 0.13074, 0.04792, 0.01522, 0.01731, -0.03666, -0.04685, -0.13123},
    { -0.00739, 0.02957, 0.01214, -0.01612, 0.00124, 0.12981, 0.04260, 0.01040, 0.01944, -0.04391, -0.04597, -0.13180},
    { -0.00254, 0.02496, 0.01355, -0.02481, 0.02538, 0.12480, 0.03757, 0.00570, 0.02068, -0.04970, -0.04532, -0.13026},
    { 0.00234, 0.02033, 0.01500, -0.03273, 0.04877, 0.11685, 0.03273, 0.00129, 0.02077, -0.05400, -0.04478, -0.12657},
    { 0.00727, 0.01598, 0.01669, -0.03977, 0.06995, 0.10736, 0.02806, -0.00210, 0.01862, -0.05649, -0.04449, -0.12108}
  };

  //double par_fg33[20][12] ={
  CoeffNoise33 = {
    { 0.23489, 0.15972, 0.15032, 0.10007, 0.11840, 0.19195, 0.06781, -0.03192, -0.02200, 0.00700, 0.00703, 0.01672},
    { 0.24011, 0.15531, 0.15130, 0.10212, 0.11674, 0.20005, 0.05379, -0.02769, -0.02199, 0.00520, 0.00965, 0.01542},
    { 0.24547, 0.15134, 0.15183, 0.10393, 0.11574, 0.20645, 0.04122, -0.02526, -0.02233, 0.00264, 0.01342, 0.01555},
    { 0.25151, 0.14749, 0.15188, 0.10552, 0.11582, 0.21085, 0.02913, -0.02492, -0.02321, -0.00042, 0.01791, 0.01844},
    { 0.25820, 0.14351, 0.15168, 0.10697, 0.11738, 0.21253, 0.01719, -0.02618, -0.02446, -0.00402, 0.02307, 0.02412},
    { 0.26507, 0.13941, 0.15159, 0.10832, 0.12081, 0.21034, 0.00604, -0.02832, -0.02572, -0.00830, 0.02876, 0.03201},
    { 0.27142, 0.13544, 0.15195, 0.10955, 0.12636, 0.20331, -0.00315, -0.03069, -0.02666, -0.01327, 0.03460, 0.04116},
    { 0.27658, 0.13197, 0.15298, 0.11063, 0.13388, 0.19126, -0.00922, -0.03277, -0.02709, -0.01877, 0.04005, 0.05051},
    { 0.28015, 0.12931, 0.15468, 0.11151, 0.14283, 0.17510, -0.01160, -0.03425, -0.02700, -0.02439, 0.04457, 0.05909},
    { 0.28215, 0.12752, 0.15688, 0.11195, 0.15277, 0.15643, -0.01063, -0.03498, -0.02702, -0.02887, 0.04736, 0.06644},
    { 0.28303, 0.12639, 0.15932, 0.11157, 0.16349, 0.13693, -0.00746, -0.03493, -0.02832, -0.03028, 0.04754, 0.07271},
    { 0.28355, 0.12567, 0.16158, 0.11099, 0.17333, 0.11819, -0.00377, -0.03439, -0.03135, -0.02837, 0.04567, 0.07889},
    { 0.28448, 0.12519, 0.16325, 0.11164, 0.17954, 0.10105, -0.00121, -0.03380, -0.03581, -0.02438, 0.04353, 0.08653},
    { 0.28608, 0.12496, 0.16415, 0.11453, 0.18039, 0.08548, -0.00029, -0.03331, -0.04133, -0.01932, 0.04223, 0.09645},
    { 0.28815, 0.12521, 0.16432, 0.12005, 0.17506, 0.07165, -0.00066, -0.03276, -0.04761, -0.01365, 0.04196, 0.10828},
    { 0.29040, 0.12624, 0.16389, 0.12817, 0.16336, 0.06037, -0.00169, -0.03193, -0.05434, -0.00778, 0.04245, 0.12086},
    { 0.29259, 0.12824, 0.16303, 0.13834, 0.14605, 0.05262, -0.00282, -0.03071, -0.06109, -0.00221, 0.04325, 0.13271},
    { 0.29455, 0.13110, 0.16193, 0.14962, 0.12497, 0.04890, -0.00372, -0.02916, -0.06738, 0.00257, 0.04403, 0.14258},
    { 0.29624, 0.13453, 0.16076, 0.16096, 0.10251, 0.04890, -0.00424, -0.02750, -0.07277, 0.00618, 0.04458, 0.14984},
    { 0.29765, 0.13817, 0.15938, 0.17191, 0.08070, 0.05153, -0.00443, -0.02647, -0.07611, 0.00809, 0.04503, 0.15455}
  };


//    for (int iii = 0; iii < 20; iii++) {
//      for (int jjj = 0; jjj < 12; jjj++) {
//        CoeffSigPDF0[iii][jjj] = par_f0[iii][jjj];
//        CoeffSigPDF1[iii][jjj] = par_f1[iii][jjj];
//        CoeffNoise31[iii][jjj] = par_fg31[iii][jjj];
//        CoeffNoise32[iii][jjj] = par_fg32[iii][jjj];
//        CoeffNoise33[iii][jjj] = par_fg33[iii][jjj];
//      }
//    }

  //
  //
  //
  return;
}


//
//





//
//
//
