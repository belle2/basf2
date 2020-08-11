#include "trg/ecl/modules/rec_ecl/rec_ecl.h"

using namespace std;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(rec_ecl)
//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

rec_eclModule::rec_eclModule() : Module()
{
  //Set module properties
  setDescription("reconstruct TC with ECL data module");
}


rec_eclModule::~rec_eclModule()
{
}

void rec_eclModule::terminate()
{
  delete trgeclmap;
}

void rec_eclModule::initialize()
{

  m_ECLCalDigitData.registerInDataStore();
  m_ECLDigitData.registerInDataStore();
  m_TRGECLCluster.registerInDataStore("TRGECLClusters_rec_ecl");

  trgeclmap = new TrgEclMapping();
}

void rec_eclModule::beginRun()
{

  // for(int iii = 0; iii < 8736; iii++){
  //   double tcid        = (double) trgeclmap->getTCIdFromXtalId(iii+1);
  //     printf("%5i %5i\n", iii+1, (int)tcid);
  // }

}


void rec_eclModule::event()
{

  double ecl_cid     = 0;      // ecl xtal id
  double ecl_tcid    = 0;      // tc id for ecl_cid
  double ecl_xtal_energy  = 0; // (MeV) ecl xtal  energy
  int    ecl_quality = -1;     // ecl fit quality
  int    ecl_timing  = -1000;  // (ns) ecl xtal timing
  double ecl_timing_threshold_low  = -200; // (ns)  xtal timing selection
  double ecl_timing_threshold_high =  200; // (ns)  xtal timing selection
  double ecl_xtcid_energy_sum[576] = {};
  double ecl_energy_sum = 0;
  int ecl_count = 0;
  int ecl_thetaid = 0;
  double ecltimingsum[576];
  for (const auto& eclcalhit : m_ECLCalDigitData) {

    // (ecl) calibation status check and cut
    if (!eclcalhit.isCalibrated()) {continue;}
    if (eclcalhit.isFailedFit()) {continue;}
    if (eclcalhit.isTimeResolutionFailed()) {continue;}

    // (ecl) xtal-id
    ecl_cid   = (double) eclcalhit.getCellId();
    //std::cout << ecl_cid << std::endl;
    // theta id
    //ecl_thetaid   = (int) eclcalhit.getTCThetaIdFromTCId();

    // (ecl) fitter quality check and cut
    ecl_quality = -1;
    for (const auto& eclhit : m_ECLDigitData) {
      if (ecl_cid == eclhit.getCellId()) {
        ecl_quality = (int) eclhit.getQuality();
      }
    }
    if (ecl_quality != 0) {continue;}

    // (ecl) xtal energy
    ecl_xtal_energy  = eclcalhit.getEnergy(); // ECLCalDigit


    // (ecl) timing cut
    ecl_timing = eclcalhit.getTime();
    if (ecl_timing < ecl_timing_threshold_low ||
        ecl_timing > ecl_timing_threshold_high) {continue;}

    // (ecl) tc-id for xtal-id
    ecl_tcid  = (double) trgeclmap->getTCIdFromXtalId(ecl_cid);
    ecl_thetaid = (double) trgeclmap->getTCThetaIdFromTCId(ecl_tcid);


    for (int i = 0; i < 576; i++) {
      //if(ecl_tcid == i+1  && ecl_thetaid != (1 || 2 || 3 || 16 || 17 )){
      if (ecl_tcid == i + 1  && ecl_thetaid >= 2 && ecl_thetaid <= 15) {
        //std::cout << ecl_thetaid << std::endl;
        //std::cout << ecl_xtal_energy<< std::endl;
        ecl_xtcid_energy_sum[i] = ecl_xtcid_energy_sum[i] + ecl_xtal_energy;
        //std::cout << ecl_xtcid_energy_sum[i]<< std::endl;
        ecltimingsum[i] = ecl_timing;
      }
      //else{
      //ecl_xtcid_energy_sum[i] =ecl_xtcid_energy_sum[i] + 0;
      //ecltimingsum[i] =0;}
    }
  }

  for (int i = 0; i < 576; i++) {
    //std::cout << i<< std::endl;
    //std::cout << ecl_xtcid_energy_sum[i] << std::endl;


    m_TRGECLCluster.appendNew();
    int ntups = m_TRGECLCluster.getEntries() - 1;
    //std::cout << ntups << std::endl;
    m_TRGECLCluster[ntups]->m_ClusterId = i;
    m_TRGECLCluster[ntups]->m_edep = ecl_xtcid_energy_sum[i];
    m_TRGECLCluster[ntups]->m_TimeAve = ecltimingsum[i];

  }

}





void rec_eclModule::endRun()
{
}
