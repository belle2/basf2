/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Poyuan Chen, Guglielmo De Nardo                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <ecl/modules/eclDigitizer/ECLDigitizerModule.h>
#include <ecl/digitization/algorithms.h>
#include <ecl/dataobjects/ECLHit.h>
#include <ecl/dataobjects/ECLDigit.h>
#include <ecl/dataobjects/ECLDsp.h>
#include <ecl/dataobjects/ECLTrig.h>

#include <ecl/geometry/ECLGeometryPar.h>

#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/gearbox/Unit.h>
#include <framework/logging/Logger.h>
#include <framework/utilities/FileSystem.h>


//C++ STL
#include <cstdlib>
#include <iomanip>
#include <math.h>
#include <time.h>
#include <iomanip>
#include <utility> //contains pair

#define PI 3.14159265358979323846


// ROOT
#include <TVector3.h>
#include <TRandom.h>
#include <TFile.h>
#include <TTree.h>

using namespace std;
using namespace Belle2;
using namespace ECL;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(ECLDigitizer)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

ECLDigitizerModule::ECLDigitizerModule() : Module()
{
  //Set module properties
  setDescription("Creates ECLDigiHits from ECLHits.");
  setPropertyFlags(c_ParallelProcessingCertified);
  addParam("Background", m_background, "Flag to use the Digitizer configuration with backgrounds; Default is no background", false);
  addParam("Calibration", m_calibration, "Flag to use the Digitizer for Waveform fit Covariance Matrix calibration; Default is false",
           false);
}


ECLDigitizerModule::~ECLDigitizerModule()
{

}

void ECLDigitizerModule::initialize()
{
  m_nEvent  = 0 ;
  StoreArray<ECLDsp> ecldsp;
  StoreArray<ECLDigit> ecldigi;
  StoreArray<ECLTrig> ecltrig;
  ecldsp.registerInDataStore();
  ecldigi.registerInDataStore();
  ecltrig.registerInDataStore();
  readDSPDB();
}

void ECLDigitizerModule::beginRun()
{
}

int** ECLDigitizerModule::allocateMatrix(unsigned int nrows, unsigned int ncols) const
{
  int** pointer = new int* [nrows];
  assert(pointer != nullptr);
  int* data = new int[ncols * nrows];
  assert(data != nullptr);
  pointer[0] = data;
  for (unsigned int i = 1; i < nrows; ++i)
    pointer[i] = pointer[i - 1] + ncols;
  return pointer;
}

void ECLDigitizerModule::deallocate(std::vector<int**> matrices) const
{
  for (int** element : matrices) deallocate(element);
}
void ECLDigitizerModule::deallocate(int** pointer) const
{
  delete[] pointer[0];
  delete[] pointer;
}

void ECLDigitizerModule::event()
{


  StoreObjPtr< ECLLookupTable> eclWFAlgoParamsTable("ECLWFAlgoParamsTable", DataStore::c_Persistent);

  //Input Array
  StoreArray<ECLHit> eclHits;
  if (!eclHits) {
    B2DEBUG(100, "ECLHit in empty in event " << m_nEvent);
  }
  StoreArray<ECLNoiseData> eclNoiseData("ECLNoiseData", DataStore::c_Persistent);
  const ECLNoiseData* eclNoise = eclNoiseData[0];

  // Output Arrays
  StoreArray<ECLDigit> eclDigits;
  StoreArray<ECLDsp> eclDsps;
  StoreArray<ECLTrig> eclTrigs;

  vector<int> energyFit(8736, 0); //fit output : Amplitude
  vector<int> tFit(8736, 0);   //fit output : T_ave
  vector<int> qualityFit(8736, 0);   //fit output : T_ave
  array2d HitEnergy(boost::extents[8736][31]);
  std::fill(HitEnergy.origin(), HitEnergy.origin() + HitEnergy.size(), 0.0);
  vector<double> totalEnergyDeposit(8736, 0);
  double test_A[31] = {0};

  double dt = .02; //delta t for interpolation
  int n = 1250;//provide a shape array for interpolation
  double DeltaT =  gRandom->Uniform(0, 144);

  for (const auto& eclHit : eclHits) {
    int hitCellId       =  eclHit.getCellId() - 1; //0~8735
    double hitE         =  eclHit.getEnergyDep()  / Unit::GeV;
    double hitTimeAve   =  eclHit.getTimeAve() / Unit::us;

    if (hitTimeAve > 8.5) { continue;}
    totalEnergyDeposit[hitCellId] += hitE; //for summation deposit energy; do fit if this summation > 0.1 MeV

    for (int T_clock = 0; T_clock < 31; T_clock++) {
      double timeInt =  DeltaT * 2. / 508.; //us
      double sampleTime = (24. * 12. / 508.)  * (T_clock - 15) - hitTimeAve - timeInt + 0.32
                          ;//There is some time shift~0.32 us that is found Alex 2013.06.19.
      //        DspSamplingArray(&n, &sampleTime, &dt, m_ft, &test_A[T_clock]);//interpolation from shape array n=1250; dt =20ns
      test_A[T_clock] = DspSamplingArray(n, sampleTime, dt, & (*m_ft)[0]);//interpolation from shape array n=1250; dt =20ns
      HitEnergy[hitCellId][T_clock] += test_A[T_clock]  * hitE ;
    }//for T_clock 31 clock

  } //end loop over eclHitArray ii

  if (m_calibration) {

    // This has been added by Alex Bobrov for calibration
    // of covariance matrix artificially generate 100 MeV in time for each crystal

    int JiP;
    for (JiP = 0; JiP < 8736; JiP++) {
      double hitE = 0.1;
      double hitTimeAve = 0.0;
      // not needed in calibration
      //      totalEnergyDeposit[JiP] = hitE + totalEnergyDeposit[JiP];//for summation deposit energy; do fit if this summation > 0.1 MeV

      for (int T_clock = 0; T_clock < 31; T_clock++) {
        double timeInt =  DeltaT * 2. / 508.; //us
        double sampleTime = (24. * 12. / 508.)  * (T_clock - 15) - hitTimeAve - timeInt + 0.32
                            ;//There is some time shift~0.32 us that is found Alex 2013.06.19.
        test_A[T_clock] = DspSamplingArray(n, sampleTime, dt, & (*m_ft)[0]);//interpolation from shape array n=1250; dt =20ns
        //        DspSamplingArray(&n, &sampleTime, &dt, m_ft, &test_A[T_clock]);//interpolation from shape array n=1250; dt =20ns
        HitEnergy[JiP][T_clock] += test_A[T_clock]  * hitE ;
      }//for T_clock 31 clock
    }
  }
  // end of Alex Bobrov ad-hoc calibration...

  for (int iECLCell = 0; iECLCell < 8736; iECLCell++) {
    if (m_calibration || totalEnergyDeposit[iECLCell] > 0.0001) { // Bobrov removes this cut in calibration

      //Noise generation
      float z[31];
      for (int i = 0; i < 31; i++) z[i] = gRandom->Gaus(0, 1);

      float AdcNoise[31];
      eclNoise->generateCorrelatedNoise(z, AdcNoise);

      int FitA[31];
      for (int  T_clock = 0; T_clock < 31; T_clock++) {
        FitA[T_clock] = (int)(HitEnergy[iECLCell][T_clock] * 20000 + 3000 + AdcNoise[T_clock] * 20) ;
      }

      m_n16 = 16;
      m_lar = 0;
      m_ltr = 0;
      m_lq = 0;

      m_ttrig = int(DeltaT) ;
      if (m_ttrig < 0)m_ttrig = 0;
      if (m_ttrig > 143)m_ttrig = 143;

      unsigned int idIdx = (*eclWFAlgoParamsTable)[ iECLCell + 1 ]; //lookup table uses cellID definition [1,8736]
      unsigned int funcIdx = m_funcTable[iECLCell + 1 ];
      shapeFitter(&(m_idn[idIdx][0]), &(m_f[funcIdx][0][0]), &(m_f1[funcIdx][0][0]), &(m_fg41[funcIdx][0][0]), &(m_fg43[funcIdx][0][0]),
                  &(m_fg31[funcIdx][0][0]), &(m_fg32[funcIdx][0][0]), &(m_fg33[funcIdx][0][0]), &(FitA[0]), &m_ttrig,  &m_n16,  &m_lar, &m_ltr,
                  &m_lq);

      energyFit[iECLCell] = m_lar; //fit output : Amplitude 18-bits
      tFit[iECLCell] = m_ltr;    //fit output : T_ave 12 bits
      qualityFit[iECLCell] = m_lq;    //fit output : quality 2 bits

      if (energyFit[iECLCell] > 0) {
        const auto eclDsp = eclDsps.appendNew();
        eclDsp->setCellId(iECLCell + 1);
        eclDsp->setDspA(FitA);

        const auto eclDigit = eclDigits.appendNew();
        eclDigit->setCellId(iECLCell + 1);//iECLCell + 1= 1~8736
        eclDigit->setAmp(energyFit[iECLCell]);//E (GeV) = energyFit/20000;
        eclDigit->setTimeFit(tFit[iECLCell]);//t0 (us)= (1520 - m_ltr)*24.*12/508/(3072/2) ;
        eclDigit->setQuality(qualityFit[iECLCell]);
      }

    }//if Energy > 0.1 MeV
  } //store  each crystal hit

  const auto eclTrig = eclTrigs.appendNew();
  //  eclTrigArray[m_hitNum2]->setTimeTrig(DeltaT * 12. / 508.); //t0 (us)= (1520 - m_ltr)*24.*12/508/(3072/2) ;
  eclTrig->setTimeTrig(DeltaT  / 508.); //t0 (us)= (1520 - m_ltr)*24.*

  m_nEvent++;
}

void ECLDigitizerModule::endRun()
{
}

void ECLDigitizerModule::terminate()
{
  deallocate(m_f);
  deallocate(m_f1);
  deallocate(m_fg31);
  deallocate(m_fg32);
  deallocate(m_fg33);
  deallocate(m_fg41);
  deallocate(m_fg43);

  for (auto i : m_idn) delete[] i;
  delete m_ft;
}

int ECLDigitizerModule::myPow(int x, int p)
{
  if (p == 0) return 1;
  if (p == 1) return x;
  return x * myPow(x, p - 1);
}


void ECLDigitizerModule::readDSPDB()
{

  string dataFileName;
  if (m_background) {
    dataFileName = FileSystem::findFile("/data/ecl/ECL-WF-BG.root");
    B2INFO("ECLDigitizer: Reading configuration data with background from: " << dataFileName);
  } else {
    dataFileName = FileSystem::findFile("/data/ecl/ECL-WF.root");
    B2INFO("ECLDigitizer: Reading configuration data without background from: " << dataFileName);
  }
  assert(! dataFileName.empty());


  StoreArray<ECLWaveformData> eclWaveformData("ECLWaveformData", DataStore::c_Persistent);
  eclWaveformData.registerInDataStore();
  StoreObjPtr< ECLLookupTable > eclWaveformDataTable("ECLWaveformDataTable", DataStore::c_Persistent);;
  eclWaveformDataTable.registerInDataStore();
  eclWaveformDataTable.create();
  StoreArray<ECLWFAlgoParams> eclWFAlgoParams("ECLWFAlgoParams", DataStore::c_Persistent);
  eclWFAlgoParams.registerInDataStore();
  StoreObjPtr< ECLLookupTable> eclWFAlgoParamsTable("ECLWFAlgoParamsTable", DataStore::c_Persistent);
  eclWFAlgoParamsTable.registerInDataStore();
  eclWFAlgoParamsTable.create();
  StoreArray<ECLNoiseData> eclNoiseData("ECLNoiseData", DataStore::c_Persistent);
  eclNoiseData.registerInDataStore();
  StoreObjPtr< ECLLookupTable > eclNoiseDataTable("ECLNoiseDataTable", DataStore::c_Persistent);;
  eclNoiseDataTable.registerInDataStore();
  eclNoiseDataTable.create();

  TFile rootfile(dataFileName.c_str());
  TTree* tree = (TTree*) rootfile.Get("EclWF");
  TTree* tree2 = (TTree*) rootfile.Get("EclAlgo");
  TTree* tree3 = (TTree*) rootfile.Get("EclNoise");

  if (tree == 0 || tree2 == 0 || tree3 == 0)
    B2FATAL("Data not found");

  Int_t ncellId;
  vector<Int_t> cellId(8736, 0);
  Int_t ncellId2;
  vector<Int_t> cellId2(8736, 0);
  Int_t ncellId3;
  vector<Int_t> cellId3(8736, 0);



  tree->GetBranch("CovarianceM")->SetAutoDelete(kFALSE);
  tree->SetBranchAddress("ncellId", &ncellId);
  tree->SetBranchAddress("cellId", &(cellId[0]));

  tree2->GetBranch("Algopars")->SetAutoDelete(kFALSE);
  tree2->SetBranchAddress("ncellId", &ncellId2);
  tree2->SetBranchAddress("cellId", &(cellId2[0]));

  tree3->GetBranch("NoiseM")->SetAutoDelete(kFALSE);
  tree3->SetBranchAddress("ncellId", &ncellId3);
  tree3->SetBranchAddress("cellId", &(cellId3[0]));

  ECLWaveformData* info = new ECLWaveformData;
  tree->SetBranchAddress("CovarianceM", &info);
  Long64_t nentries = tree->GetEntries();
  for (Long64_t ev = 0; ev < nentries; ev++) {
    tree->GetEntry(ev);
    eclWaveformData.appendNew(*info);
    for (Int_t i = 0; i < ncellId; ++i)
      (*eclWaveformDataTable) [cellId[i]] = ev;
  }
  delete info;

  ECLWFAlgoParams* algo = new ECLWFAlgoParams;
  tree2->SetBranchAddress("Algopars", &algo);
  nentries = tree2->GetEntries();
  for (Long64_t ev = 0; ev < nentries; ev++) {
    tree2->GetEntry(ev);
    eclWFAlgoParams.appendNew(*algo);
    m_idn.push_back(new short int [16]);
    for (Int_t i = 0; i < ncellId2; ++i)
      (*eclWFAlgoParamsTable) [cellId2[i]] = ev;
  }
  delete algo;

  ECLNoiseData* noise = new ECLNoiseData;
  tree3->SetBranchAddress("NoiseM", &noise);
  nentries = tree3->GetEntries();
  for (Long64_t ev = 0; ev < nentries; ev++) {
    tree3->GetEntry(ev);
    eclNoiseData.appendNew(*noise);
    if (ncellId3 == 0) {
      for (int i = 1; i <= 8736; i++)(*eclNoiseDataTable)[i] = 0;
      break;
    } else {
      for (Int_t i = 0; i < ncellId3; ++i)
        (*eclNoiseDataTable)[cellId3[i]] = ev;
    }
  }
  delete noise;

  rootfile.Close();

  typedef vector< pair<unsigned int, unsigned int> > PairIdx;
  PairIdx pairIdx;

  for (int icell = 1; icell <= 8736; icell++) {
    unsigned int wfIdx = (*eclWaveformDataTable) [ icell ];
    unsigned int algoIdx = (*eclWFAlgoParamsTable) [ icell ];
    pair<unsigned int, unsigned int> wfAlgoIdx = make_pair(wfIdx, algoIdx) ;
    bool found(false);
    for (unsigned int ielem = 0; ielem < pairIdx.size(); ++ielem) {
      if (wfAlgoIdx == pairIdx[ielem]) {
        m_funcTable[ icell ] = ielem;
        found = true;
        break;
      }
    }

    if (!found) {
      m_funcTable[ icell ] = pairIdx.size();
      m_f.push_back(allocateMatrix(192, 16));
      m_f1.push_back(allocateMatrix(192, 16));
      m_fg31.push_back(allocateMatrix(192, 16));
      m_fg32.push_back(allocateMatrix(192, 16));
      m_fg33.push_back(allocateMatrix(192, 16));
      m_fg41.push_back(allocateMatrix(24, 16));
      m_fg43.push_back(allocateMatrix(24, 16));
      pairIdx.push_back(wfAlgoIdx);
    }

  }

  float MP[10];

  //Presently all crystals share the same  Waveform function parameters.
  const ECLWaveformData* eclWFData = eclWaveformData[0];
  eclWFData->getWaveformParArray(MP);

  // varible for one channel

  float ss1[16][16];

  unsigned int ChN;

  array2d sg(boost::extents[16][192]), sg1(boost::extents[16][192]), sg2(boost::extents[16][192]);
  fill(sg.origin(), sg.origin() + sg.size(), 0.0);
  fill(sg1.origin(), sg1.origin() + sg1.size(), 0.0);
  fill(sg2.origin(), sg2.origin() + sg2.size(), 0.0);

  array2d f(boost::extents[192][16]), f1(boost::extents[192][16]),
          fg31(boost::extents[192][16]), fg32(boost::extents[192][16]), fg33(boost::extents[192][16]);
  array2d fg41(boost::extents[24][16]), fg43(boost::extents[192][16]);


  double  g1g1[192], gg[192], gg1[192], dgg[192];
  double  gg2[192], g1g2[192], g2g2[192];
  double  dgg1[192], dgg2[192];

  memset(g1g1, 0, sizeof(g1g1));
  memset(gg, 0, sizeof(gg));
  memset(gg1, 0, sizeof(gg1));
  memset(dgg, 0, sizeof(dgg));
  memset(gg2, 0, sizeof(gg2));
  memset(g1g2, 0, sizeof(g1g2));
  memset(g2g2, 0, sizeof(g2g2));
  memset(dgg1, 0, sizeof(dgg1));
  memset(dgg2, 0, sizeof(dgg2));


  double dt;
  double ts0;
  double del;


  int j1, endc, j, i;
  double ndt;
  double adt, dt0, t0, ddt, tc1;

  // set but not used
  //double tin;

  double t, tmd, tpd, ssssj, ssssj1, ssssi, ssssi1;
  double svp, svm;


  int ibb, iaa, idd, icc;
  int ia, ib, ic;

  // set but not used!
  // int i16;


  int ipardsp13;
  int ipardsp14;

  int n16;
  int k;

  int c_a;
  int c_b;
  int c_c;
  int c_16;

  // = {tr0 , tr1 , Bhard , 17952 , 19529 , 69 , 0 , 0 , 257 , -1 , 0 , 0 , 256*c1_chi+c2_chi , c_a+256*c_b , c_c+256*c_16 ,chi_thres };


  for (unsigned int ichannel = 0; ichannel < pairIdx.size(); ichannel++) {
    // if (ichannel % 1000 == 0) {printf("!!CnN=%d\n", ichannel);}
    ChN = pairIdx[ichannel].first;
    const ECLWaveformData* eclWFData = eclWaveformData[ ChN ];
    eclWFData->getMatrix(ss1);
    ChN = pairIdx[ichannel].second;
    const ECLWFAlgoParams* eclWFAlgo = eclWFAlgoParams[ChN];
    // shape function parameters

    c_a = eclWFAlgo->getka();
    c_b = eclWFAlgo->getkb();
    c_c = eclWFAlgo->getkc();
    c_16 = eclWFAlgo->gety0s();








    // = {tr0 , tr1 , Bhard , 17952 , 19529 , 69 , 0 , 0 , 257 , -1 , 0 , 0 , 256*c1_chi+c2_chi , c_a+256*c_b , c_c+256*c_16 ,chi_thres };

    m_idn[ ChN][ 0] = (short int) eclWFAlgo->getlAT() + 128;
    m_idn[ ChN][ 1] = (short int) eclWFAlgo->getsT()  + 128;
    m_idn[ ChN][ 2] = (short int) eclWFAlgo->gethT();
    m_idn[ ChN][ 3] = (short int)17952;
    m_idn[ ChN][ 4] = (short int)19529;
    m_idn[ ChN][ 5] = (short int)69;
    m_idn[ChN][ 6] = (short int)0;
    m_idn[ChN][ 7] = (short int)0;
    m_idn[ChN][ 8] = (short int)257;
    m_idn[ChN][ 9] = (short int) - 1;
    m_idn[ChN][10] = (short int)0;
    m_idn[ChN][11] = (short int)0;
    m_idn[ChN][12] = (short int)(256 * eclWFAlgo->getk1() + eclWFAlgo->getk2());
    m_idn[ChN][13] = (short int)(c_a + 256 * c_b);
    m_idn[ChN][14] = (short int)(c_c + 256 * (c_16 - 16));
    m_idn[ChN][15] = (short int)(eclWFAlgo->getcT());




    ///////////////////rragtds

    del = 0.;
    ts0 = 0.5;
    dt = 0.5;


    ndt = 96.;
    adt = 1. / ndt;
    endc = 2 * ndt;


    dt0 = adt * dt;
    t0 = -dt0 * ndt;




    for (j1 = 0; j1 < endc; j1++) {
      t0 = t0 + dt0;
      t = t0 - dt - del;

      // set but not used!
      //tin = t + dt;

      for (j = 0; j < 16; j++) {
        t = t + dt;

        if (t > 0) {


          f[j1][j] = ShaperDSP_F(t / 0.881944444, MP);

          ddt = 0.005 * dt;
          tc1 = t - ts0;
          tpd = t + ddt;
          tmd = t - ddt;


          if (tc1 > ddt) {
            svp = ShaperDSP_F(tpd / 0.881944444, MP);
            svm = ShaperDSP_F(tmd / 0.881944444, MP);

            f1[j1][j] = (svp - svm) / 2. / ddt;

          } else {


            f1[j1][j] = ShaperDSP_F(tpd / 0.881944444, MP) / (ddt + tc1);


          }// else tc1>ddt


        } //if t>0
        else {

          f[j1][j] = 0.;
          f1[j1][j] = 0.;

        }

      } //for j


    }



    for (j1 = 0; j1 < endc; j1++) {

      gg[j1] = 0.;
      gg1[j1] = 0.;
      g1g1[j1] = 0.;
      gg2[j1] = 0.;
      g1g2[j1] = 0.;
      g2g2[j1] = 0.;
      for (j = 0; j < 16; j++) {
        sg[j][j1] = 0.;
        sg1[j][j1] = 0.;
        sg2[j][j1] = 0.;

        ssssj1 = f1[j1][j];
        ssssj = f[j1][j];

        for (i = 0; i < 16; i++) {



          sg[j][j1] = sg[j][j1] + ss1[j][i] * f[j1][i];
          sg1[j][j1] = sg1[j][j1] + ss1[j][i] * f1[j1][i];

          sg2[j][j1] = sg2[j][j1] + ss1[j][i];




          ssssi = f[j1][i];
          ssssi1 = f1[j1][i];

          gg[j1] = gg[j1] + ss1[j][i] * ssssj * ssssi;



          gg1[j1] = gg1[j1] + ss1[j][i] * ssssj * ssssi1;
          g1g1[j1] = g1g1[j1] + ss1[j][i] * ssssi1 * ssssj1;

          gg2[j1] = gg2[j1] + ss1[j][i] * ssssj;
          g1g2[j1] = g1g2[j1] + ss1[j][i] * ssssj1;
          g2g2[j1] = g2g2[j1] + ss1[j][i];


        }   // for i


      } //for j

      dgg[j1] = gg[j1] * g1g1[j1] - gg1[j1] * gg1[j1];
      dgg1[j1] = gg[j1] * g2g2[j1] - gg2[j1] * gg2[j1];
      dgg2[j1] = gg[j1] * g1g1[j1] * g2g2[j1] - gg1[j1] * gg1[j1] * g2g2[j1] + 2 * gg1[j1] * g1g2[j1] * gg2[j1] - gg2[j1] * gg2[j1] *
                 g1g1[j1] - g1g2[j1] * g1g2[j1] * gg[j1];


      for (i = 0; i < 16; i++) {
        if (dgg2[j1] != 0) {

          fg31[j1][i] = ((g1g1[j1] * g2g2[j1] - g1g2[j1] * g1g2[j1]) * sg[i][j1] + (g1g2[j1] * gg2[j1] - gg1[j1] * g2g2[j1]) * sg1[i][j1] +
                         (gg1[j1] * g1g2[j1] - g1g1[j1] * gg2[j1]) * sg2[i][j1]) / dgg2[j1];



          fg32[j1][i] = ((g1g2[j1] * gg2[j1] - gg1[j1] * g2g2[j1]) * sg[i][j1] + (gg[j1] * g2g2[j1] - gg2[j1] * gg2[j1]) * sg1[i][j1] +
                         (gg1[j1] * gg2[j1] - gg[j1] * g1g2[j1]) * sg2[i][j1]) / dgg2[j1];


          fg33[j1][i] = ((gg1[j1] * g1g2[j1] - g1g1[j1] * gg2[j1]) * sg[i][j1] + (gg1[j1] * gg2[j1] - gg[j1] * g1g2[j1]) * sg1[i][j1] +
                         (gg[j1] * g1g1[j1] - gg1[j1] * gg1[j1]) * sg2[i][j1]) / dgg2[j1];


        }


        if (dgg1[j1] != 0) {
          if (j1 < 24) {

            fg41[j1][i] = (g2g2[j1] * sg[i][j1] - gg2[j1] * sg2[i][j1]) / dgg1[j1];
            fg43[j1][i] = (gg[j1] * sg2[i][j1] - gg2[j1] * sg[i][j1]) / dgg1[j1];
          }

        }





      }  // for i


    } // for j1 <endc




    //%%%%%%%%%%%%%%%%%%%adduction to integer

    n16 = 16;

    ipardsp13 = 14 + 14 * 256;
    ipardsp14 = 0 * 256 + 17;

    ibb = ipardsp13 / 256;
    iaa = ipardsp13 - ibb * 256;
//  idd=ipardsp14/256;
    icc = ipardsp14 - idd * 256;

    iaa = c_a;
    ibb = c_b;
    icc = c_c;


    ia = myPow(2, iaa);
    ib = myPow(2, ibb);
    ic = myPow(2, icc);

    // set but not used!
    // i16 = myPow(2, 15);


    ChN = ichannel;

    for (i = 0; i < 16; i++) {
      if (i == 0) {idd = n16;}
      else {idd = 1;}
      for (k = 0; k < 192; k++) {

        //  cout << "Reading channel " << ChN << " " << k << " " << i << endl;
        (m_f[ChN])[k][i] = (int)(f[k][i] * ia / idd + ia + 0.5) - ia;

        (m_f1[ChN])[k][i] = (int)(4 * f1[k][i] * ia / idd / 3 + ia + 0.5) - ia;




        (m_fg31[ChN])[k][i] = (int)(fg31[k][i] * ia / idd + ia + 0.5) - ia;

        (m_fg32[ChN])[k][i] = (int)(3 * fg32[k][i] * ib / idd / 4 + ib + 0.5) - ib;



        (m_fg33[ChN])[k][i] = (int)(fg33[k][i] * ic / idd + ic + 0.5) - ic;


        if (k <= 23) {

          (m_fg41[ChN])[k][i] = (int)(fg41[k][i] * ia / idd + ia + 0.5) - ia;


          (m_fg43[ChN])[k][i] = (int)(fg43[k][i] * ic / idd + ic + 0.5) - ic;



        }  // if k<=23


      } // for k


    }  // for i


  }


  TTree* tree4 = (TTree*) rootfile.Get("EclSampledSignalWF");
  if (tree4 != 0) {
    tree4->SetBranchAddress("EclSampledSignalWF", &m_ft);
    tree4->GetEntry(0);
  } else  m_ft = createDefSampledSignalWF();

}
