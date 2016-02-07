/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Peter Kodys                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <pxd/calibration/PXDClusterShapeCalibrationAlgorithm.h>

#include <TH1F.h>
#include <TH2F.h>
#include <TMath.h>
#include <TRandom.h>
#include <TFile.h>

using namespace std;
using namespace Belle2;

PXDClusterShapeCalibrationAlgorithm::PXDClusterShapeCalibrationAlgorithm() : CalibrationAlgorithm("pxdClusterShapeCalibration")
{
  setDescription("Calibration of position/error corrections based on cluster shape in PXD");
}


Belle2::CalibrationAlgorithm::EResult PXDClusterShapeCalibrationAlgorithm::calibrate()
{
  // cluster will be check if:
  //   is with non-corrected cluster shape, otheway remove from corrections (to prevent multiple-corrections)
  //   is not close borders, otheway remove from corrections
  //   for givig kind of pixel type, u and v angle direction:
  //     is in in-pixel hitmap for giving cluster shape and giving angle, otheway if hitmap is fully empty:
  //        do those steps for full range simulation in-pixel hitmap
  //        if still not in in-pixel hitmap - remove from corrections
  //     look for bias and estimated error to angle in 3 most closes points in calibration tables
  //     apply correction with weigh dependent of distances to table points
  // Tables for every pixel type (8 types for PXD):
  //   2 sets of tables: - for selection of angles and shapes for real data
  //                     - for full range of angles and shapes from simulations
  //   in every set of tables are:
  //       - in-pixel 2D hitmaps for angles and shapes
  //       - u+v corrections of bias for angles and shapes
  //       - u+v error estimations for angles and shapes
  // so we expect u+v directions, 8 kinds of pixels, 18 x 18 angles, 15 shapes = 77760 cases
  //   for Bias correction
  //   for Error estimation (EstimError/Residual)
  // so we expect 8 kinds of pixels, 18 x 18 angles, 15 shapes = 38880 cases
  //   for in-pixel positions 2D maps, each map has 9x9 bins with binary value (0/1)
  // For ~1000 points per histogram we expect 25 millions events (1 event = 2x sensors, u+v coordinates)
  // We expect set of histograms mostly empty so no need any correction for giving parameters
  // Finaly we store for 2 sets of tables:
  //   - for Bias correction:      77760 short values
  //   - for Error estimation:     77760 short values
  //   - for in-pixel positions: 3149280 binary values
  //
  // Storring in database will be in TVectorT format
  // Using will be on boost/multi_array format in unordered map and hash table

  Char_t name_Case[500];
  Char_t name_SourceTree[500];
  Char_t name_OutFileCalibrations[500];
  Char_t name_OutFileDQM[500];
  Char_t name_OutDoExpertHistograms[500];

  Char_t nameh_phi[500];  // TODO set it fo arrais
  Char_t nameh_theta[500];
  Char_t nameh_phitheta[500];
  Char_t nameh_CorMatrixU[500];
  Char_t nameh_CorMatrixV[500];
  TH1F* h1_phi;
  TH1F* h1_theta;
  TH2F* h2_phitheta;
  TH2F* h2_CorMatrixU;
  TH2F* h2_CorMatrixV;

  sprintf(name_Case, "_RealData%i_Track%i_Calib%i_Pixel%i",
          (int)m_UseRealData, (int)m_UseTracks, m_CalibrationKind, m_PixelKind);
  sprintf(name_OutFileDQM, "pxdClShapeCalibrationDQM%s.root", name_Case);
  sprintf(name_OutFileCalibrations, "pxdClShapeCalibration%s.root", name_Case);
  sprintf(name_OutDoExpertHistograms, "pxdClShapeCalibrationHistos%s.root", name_Case);
  sprintf(name_SourceTree, "pxdCal");

  B2INFO("-----> name of files: DQM " << name_OutFileDQM << ", callibration " << name_OutFileCalibrations);

  if (m_DoExpertHistograms) {
    sprintf(nameh_phi, "h1_phi");
    sprintf(nameh_theta, "h1_theta");
    sprintf(nameh_phitheta, "h2_phitheta");
    sprintf(nameh_CorMatrixU, "CorMatrixU");
    sprintf(nameh_CorMatrixV, "CorMatrixV");

    B2DEBUG(30, "--> name of histos: " << nameh_phi << ", " << nameh_theta << ", " << nameh_phitheta);

    h1_phi = new TH1F(nameh_phi, nameh_phi, 100, -90, 90);
    h1_theta = new TH1F(nameh_theta, nameh_theta, 100, -90, 90);
    h2_phitheta = new TH2F(nameh_phitheta, nameh_phitheta, 50, -90, 90, 50, -90, 90);
    h2_CorMatrixU = new TH2F(nameh_CorMatrixU, nameh_CorMatrixU, 18, -90, 90, 18, -90, 90);
    h2_CorMatrixV = new TH2F(nameh_CorMatrixV, nameh_CorMatrixV, 18, -90, 90, 18, -90, 90);
  }

  B2INFO("-----> calibrate --> Entries: " << getObject<TTree>("pxdCal").GetEntries());
  B2INFO("-----> SettingImport: UseTracks:" << m_UseTracks <<
         " UseRealData:" << m_UseRealData <<
         " CompareTruePointTracks:" << m_CompareTruePointTracks <<
         " CalibrationKind:" << m_CalibrationKind <<
         " PixelKind:" << m_PixelKind
        );

  getObject<TTree>(name_SourceTree).SetBranchAddress("event", &m_evt);
  getObject<TTree>(name_SourceTree).SetBranchAddress("run", &m_run);
  getObject<TTree>(name_SourceTree).SetBranchAddress("exp", &m_exp);
  getObject<TTree>(name_SourceTree).SetBranchAddress("pid", &m_procId);
  getObject<TTree>(name_SourceTree).SetBranchAddress("layer", &m_layer);
  getObject<TTree>(name_SourceTree).SetBranchAddress("sensor", &m_sensor);
  getObject<TTree>(name_SourceTree).SetBranchAddress("pixelKind", &m_pixelKind);
  getObject<TTree>(name_SourceTree).SetBranchAddress("closeEdge", &m_closeEdge);
  getObject<TTree>(name_SourceTree).SetBranchAddress("shape", &m_shape);
  getObject<TTree>(name_SourceTree).SetBranchAddress("phiTrack", &m_phiTrack);
  getObject<TTree>(name_SourceTree).SetBranchAddress("thetaTrack", &m_thetaTrack);
  getObject<TTree>(name_SourceTree).SetBranchAddress("phiTrue", &m_phiTrue);
  getObject<TTree>(name_SourceTree).SetBranchAddress("thetaTrue", &m_thetaTrue);
  getObject<TTree>(name_SourceTree).SetBranchAddress("signal", &m_signal);
  getObject<TTree>(name_SourceTree).SetBranchAddress("seed", &m_seed);
  getObject<TTree>(name_SourceTree).SetBranchAddress("InPixUTrue", &m_InPixUTrue);
  getObject<TTree>(name_SourceTree).SetBranchAddress("InPixVTrue", &m_InPixVTrue);
  getObject<TTree>(name_SourceTree).SetBranchAddress("InPixU", &m_InPixU);
  getObject<TTree>(name_SourceTree).SetBranchAddress("InPixV", &m_InPixV);
  getObject<TTree>(name_SourceTree).SetBranchAddress("ResidUTrue", &m_ResidUTrue);
  getObject<TTree>(name_SourceTree).SetBranchAddress("ResidVTrue", &m_ResidVTrue);
  getObject<TTree>(name_SourceTree).SetBranchAddress("SigmaU", &m_SigmaU);
  getObject<TTree>(name_SourceTree).SetBranchAddress("SigmaV", &m_SigmaV);
  getObject<TTree>(name_SourceTree).SetBranchAddress("ResidUTrack", &m_ResidUTrack);
  getObject<TTree>(name_SourceTree).SetBranchAddress("ResidVTrack", &m_ResidVTrack);
  getObject<TTree>(name_SourceTree).SetBranchAddress("SigmaUTrack", &m_SigmaUTrack);
  getObject<TTree>(name_SourceTree).SetBranchAddress("SigmaVTrack", &m_SigmaVTrack);


  // create vector for storing on database:

  std::vector<TVectorD*> Correction_Bias(1);
  std::vector<TVectorD*> Correction_ErrorEstimation(1);
  std::vector<TVectorD*> Correction_BiasErr(1);
  std::vector<TVectorD*> Correction_ErrorEstimationErr(1);
  std::vector<TVectorD*> InPixelPosition(1);

  Correction_Bias[0] = new TVectorD(m_shapes * m_pixelkinds * m_dimensions * m_anglesU * m_anglesV);
  Correction_ErrorEstimation[0] = new TVectorD(m_shapes * m_pixelkinds * m_dimensions * m_anglesU * m_anglesV);
  Correction_BiasErr[0] = new TVectorD(m_shapes * m_pixelkinds * m_dimensions * m_anglesU * m_anglesV);
  Correction_ErrorEstimationErr[0] = new TVectorD(m_shapes * m_pixelkinds * m_dimensions * m_anglesU * m_anglesV);
  InPixelPosition[0] = new TVectorD(m_shapes * m_pixelkinds * m_anglesU * m_anglesV * m_in_pixelU * m_in_pixelV);


  // create tables for filling and normal using:

  typedef boost::multi_array<double, 5> correction_table_type;
  typedef boost::multi_array<double, 6> inpixel_table_type;

  correction_table_type TCorrection_Bias(boost::extents[m_shapes][m_pixelkinds][m_dimensions][m_anglesU][m_anglesV]);
  correction_table_type TCorrection_ErrorEstimation(boost::extents[m_shapes][m_pixelkinds][m_dimensions][m_anglesU][m_anglesV]);
  correction_table_type TCorrection_BiasErr(boost::extents[m_shapes][m_pixelkinds][m_dimensions][m_anglesU][m_anglesV]);
  correction_table_type TCorrection_ErrorEstimationErr(boost::extents[m_shapes][m_pixelkinds][m_dimensions][m_anglesU][m_anglesV]);
  inpixel_table_type TInPixelPosition(boost::extents[m_shapes][m_pixelkinds][m_anglesU][m_anglesV][m_in_pixelU][m_in_pixelV]);


  // presets of vectors and tables:

  Double_t* ValueCors = new Double_t[m_shapes * m_pixelkinds * m_dimensions * m_anglesU * m_anglesV];
  Double_t* ValueInPix = new Double_t[m_shapes * m_pixelkinds * m_anglesU * m_anglesV * m_in_pixelU * m_in_pixelV];
  for (int i = 0; i < m_shapes * m_pixelkinds * m_dimensions * m_anglesU * m_anglesV; i++)
    ValueCors[i] = 0.0;
  Correction_Bias[0]->SetElements(ValueCors);
  Correction_BiasErr[0]->SetElements(ValueCors);
  Correction_ErrorEstimationErr[0]->SetElements(ValueCors);
  for (int i = 0; i < m_shapes * m_pixelkinds * m_dimensions * m_anglesU * m_anglesV; i++)
    ValueCors[i] = 1.0;
  Correction_ErrorEstimation[0]->SetElements(ValueCors);
  for (int i = 0; i < m_shapes * m_pixelkinds * m_anglesU * m_anglesV * m_in_pixelU * m_in_pixelV; i++)
    ValueInPix[i] = 0.0;
  InPixelPosition[0]->SetElements(ValueInPix);

  for (int i_shape = 0; i_shape < m_shapes; i_shape++)
    for (int i_pk = 0; i_pk < m_pixelkinds; i_pk++)
      for (int i_axis = 0; i_axis < m_dimensions; i_axis++)
        for (int i_angleU = 0; i_angleU < m_anglesU; i_angleU++)
          for (int i_angleV = 0; i_angleV < m_anglesV; i_angleV++) {
            TCorrection_Bias[i_shape][i_pk][i_axis][i_angleU][i_angleV] = 0.0;
            TCorrection_ErrorEstimation[i_shape][i_pk][i_axis][i_angleU][i_angleV] = 1.0;
            TCorrection_BiasErr[i_shape][i_pk][i_axis][i_angleU][i_angleV] = 0.0;
            TCorrection_ErrorEstimationErr[i_shape][i_pk][i_axis][i_angleU][i_angleV] = 0.0;
          }
  for (int i_shape = 0; i_shape < m_shapes; i_shape++)
    for (int i_pk = 0; i_pk < m_pixelkinds; i_pk++)
      for (int i_angleU = 0; i_angleU < m_anglesU; i_angleU++)
        for (int i_angleV = 0; i_angleV < m_anglesV; i_angleV++)
          for (int i_ipU = 0; i_ipU < m_in_pixelU; i_ipU++)
            for (int i_ipV = 0; i_ipV < m_in_pixelV; i_ipV++)
              TInPixelPosition[i_shape][i_pk][i_angleU][i_angleV][i_ipU][i_ipV] = 0.0;

  B2DEBUG(30, "--> Presets done. ");

  //std::unordered_map<key_type, double, key_hash, key_equal> TCorrection_BiasMap;
  map_typeCorrs TCorrection_BiasMap;
  map_typeCorrs TCorrection_ErrorEstimationMap;
  map_typeCorrs TCorrection_BiasMapErr;
  map_typeCorrs TCorrection_ErrorEstimationMapErr;
  std::unordered_map<key_type2, double, key_hash2, key_equal2> TInPixelPositionMap;
  for (int i_shape = 0; i_shape < m_shapes; i_shape++)
    for (int i_pk = 0; i_pk < m_pixelkinds; i_pk++)
      for (int i_axis = 0; i_axis < m_dimensions; i_axis++)
        for (int i_angleU = 0; i_angleU < m_anglesU; i_angleU++)
          for (int i_angleV = 0; i_angleV < m_anglesV; i_angleV++) {
            TCorrection_BiasMap[make_tuple(i_shape, i_pk, i_axis, i_angleU, i_angleV)] = 0;
            TCorrection_ErrorEstimationMap[make_tuple(i_shape, i_pk, i_axis, i_angleU, i_angleV)] = 1.0;
            TCorrection_BiasMapErr[make_tuple(i_shape, i_pk, i_axis, i_angleU, i_angleV)] = 0;
            TCorrection_ErrorEstimationMapErr[make_tuple(i_shape, i_pk, i_axis, i_angleU, i_angleV)] = 0.0;
          }
  for (int i_shape = 0; i_shape < m_shapes; i_shape++)
    for (int i_pk = 0; i_pk < m_pixelkinds; i_pk++)
      for (int i_angleU = 0; i_angleU < m_anglesU; i_angleU++)
        for (int i_angleV = 0; i_angleV < m_anglesV; i_angleV++)
          for (int i_ipU = 0; i_ipU < m_in_pixelU; i_ipU++)
            for (int i_ipV = 0; i_ipV < m_in_pixelV; i_ipV++)
              TInPixelPositionMap[make_tuple(i_shape, i_pk, i_angleU, i_angleV, i_ipU, i_ipV)] = 0.0;

  B2DEBUG(30, "--> Ordering done. ");

  // presets of vectors and tables:

  //int n_Events = getObject<TTree>(name_SourceTree).GetEntries();
  int nSelRowsTemp = 0;
  for (int i_shape = 0; i_shape < m_shapes; i_shape++) {
    char cCat[100];
    TCut c1;
    TCut c2;
    TCut c3a;
    TCut c3b;
    TCut c4a;
    TCut c4b;
    TCut c5;
    TCut cFin;
    sprintf(cCat, "shape == %i", i_shape);
    c1.SetTitle(cCat);
    getObject<TTree>(name_SourceTree).Draw("ResidUTrack:ResidVTrack", c1, "goff");
    nSelRowsTemp = (int)getObject<TTree>(name_SourceTree).GetSelectedRows();
    printf("--> sh %i rows %i\n", i_shape, nSelRowsTemp);
    if (nSelRowsTemp < 10000) continue;
    for (int i_pk = 0; i_pk < m_pixelkinds; i_pk++) {
      sprintf(cCat, "pixelKind == %i", i_pk);
      c2.SetTitle(cCat);
      getObject<TTree>(name_SourceTree).Draw("ResidUTrack:ResidVTrack", c1 + c2, "goff");
      nSelRowsTemp = (int)getObject<TTree>(name_SourceTree).GetSelectedRows();
      if (nSelRowsTemp < 1000) continue;
      B2DEBUG(130, "--> Calibration for: " << i_shape << ", " << i_pk);
      printf(".");
      for (int i_angleU = 0; i_angleU < m_anglesU; i_angleU++) {
        double PhiMi = ((TMath::Pi() * i_angleU) / m_anglesU) - (TMath::Pi() / 2.0);
        double PhiMa = ((TMath::Pi() * (i_angleU + 1)) / m_anglesU) - (TMath::Pi() / 2.0);
        if ((m_UseRealData == kTRUE) || (m_UseTracks == kTRUE)) {
          sprintf(cCat, "phiTrack > %f", PhiMi);
          c3a.SetTitle(cCat);
          sprintf(cCat, "phiTrack < %f", PhiMa);
          c3b.SetTitle(cCat);
        } else if ((m_CalibrationKind == 2) || (m_UseTracks == kFALSE)) {
          sprintf(cCat, "phiTrue > %f", PhiMi);
          c3a.SetTitle(cCat);
          sprintf(cCat, "phiTrue < %f", PhiMa);
          c3b.SetTitle(cCat);
        }
        getObject<TTree>(name_SourceTree).Draw("ResidUTrack:ResidVTrack", c1 + c2 + c3a + c3b, "goff");
        nSelRowsTemp = (int)getObject<TTree>(name_SourceTree).GetSelectedRows();
        if (nSelRowsTemp < 500) continue;
        for (int i_angleV = 0; i_angleV < m_anglesV; i_angleV++) {
          B2DEBUG(130, "  --> AngleCalibration for: " << i_angleU << ", " << i_angleV);

// there are following combinations:
//   1) Real data:              phiTrack, thetaTrack, InPixU, InPixV, ResidUTrack, ResidVTrack, SigmaU, SigmaV, // SigmaUTrack, SigmaVTrack
//   2) Simulation from tracks: phiTrack, thetaTrack, InPixU, InPixV, ResidUTrack, ResidVTrack, SigmaU, SigmaV, // SigmaUTrack, SigmaVTrack
//   3) Simulation from TrueP.: phiTrue, thetaTrue, InPixUTrue, InPixVTrue, ResidUTrue, ResidVTrue, SigmaU, SigmaV
//   4) SpecialSimulation:      phiTrue, thetaTrue, InPixUTrue, InPixVTrue, ResidUTrue, ResidVTrue, SigmaU, SigmaV
//
//  1) m_UseRealData == kTRUE
//  2) m_UseTracks == kTRUE
//  3) m_UseTracks == kFALSE
//  4) m_CalibrationKind == 2
//
// For calibration need to have: InPix, Residual, Sigma
//
// Histograms are optional only for request,
//                  only for middle region +-30 deg in u and v = 6x6=36 cases
//                  only for basic 7 shapes: shape_1, 2_u, 2_v, 3_L,3_L_mirr_u, 3_L_mirr_v, 3_L_mirr_uv,
//
// Histograms: angles * m_shapes * m_pixelkinds = 36x7x8 = 2016 (2D) or 4032 (1D)
// Monitoring histograms:  2D ResidU + ResidV
//                         2D SigmaU/ResidU + SigmaV/ResidV
//                         nEvents
//
// Special histograms case 2)+3):  2D SigmaUTrack + SigmaVTrack
//                                 2D phiTrack - phiTrue + thetaTrack - thetaTrue
//                                 2D InPixU - InPixUTrue + InPixV - InPixVTrue
//                                 2D ResidUTrue - ResidUTrue + ResidVTrue - ResidVTrue

          double ThetaMi = ((TMath::Pi() * i_angleV) / m_anglesV) - (TMath::Pi() / 2.0);
          double ThetaMa = ((TMath::Pi() * (i_angleV + 1)) / m_anglesV) - (TMath::Pi() / 2.0);
          if ((m_UseRealData == kTRUE) || (m_UseTracks == kTRUE)) {
            sprintf(cCat, "thetaTrack > %f", ThetaMi);
            c4a.SetTitle(cCat);
            sprintf(cCat, "thetaTrack < %f", ThetaMa);
            c4b.SetTitle(cCat);
          } else if ((m_CalibrationKind == 2) || (m_UseTracks == kFALSE)) {
            sprintf(cCat, "thetaTrue > %f", ThetaMi);
            c4a.SetTitle(cCat);
            sprintf(cCat, "thetaTrue < %f", ThetaMa);
            c4b.SetTitle(cCat);
          }
          getObject<TTree>(name_SourceTree).Draw("ResidUTrack:ResidVTrack", c1 + c2 + c3a + c3b + c4a + c4b, "goff");
          nSelRowsTemp = (int)getObject<TTree>(name_SourceTree).GetSelectedRows();
          if (nSelRowsTemp < 500) continue;

          sprintf(cCat, "closeEdge == 0");
          c5.SetTitle(cCat);

          cFin = c1 && c2 && c3a && c3b && c4a && c4b && c5;

          B2DEBUG(130, "--> Selection criteria: " << cFin.GetTitle());
          if ((m_UseRealData == kTRUE) || (m_UseTracks == kTRUE)) {
            getObject<TTree>(name_SourceTree).Draw("ResidUTrack:ResidVTrack:ResidUTrack/SigmaU:ResidVTrack/SigmaV", cFin, "goff");
          } else if ((m_CalibrationKind == 2) || (m_UseTracks == kFALSE)) {
            getObject<TTree>(name_SourceTree).Draw("ResidUTrue:ResidVTrue:ResidUTrue/SigmaU:ResidVTrue/SigmaV", cFin, "goff");
          }
          //getObject<TTree>(name_SourceTree).GetReadEntry();
          int nSelRows = (int)getObject<TTree>(name_SourceTree).GetSelectedRows();
          if (nSelRows) {
            B2DEBUG(30, "--> Selected raws " << nSelRows);
            double* Col1 = getObject<TTree>(name_SourceTree).GetV1();
            double* Col2 = getObject<TTree>(name_SourceTree).GetV2();
            double* Col3 = getObject<TTree>(name_SourceTree).GetV3();
            double* Col4 = getObject<TTree>(name_SourceTree).GetV4();
//            printf("---------------> %s <---\n", cFin.GetTitle());
//            for (int i = 0; i < nSelRows; i++) {
//              printf("---------------> %i %f %f %f %f <---\n",i,Col1[i],Col2[i],Col3[i],Col4[i]);
//            }

            double RetVal;
            double RetValError;
            RetVal = 0;
            RetValError = 0;
            if (CalculateCorrection(1, nSelRows, Col1, &RetVal, &RetValError)) {
              TCorrection_BiasMap[make_tuple(i_shape, i_pk, 0, i_angleU, i_angleV)] = RetVal;
              TCorrection_BiasMapErr[make_tuple(i_shape, i_pk, 0, i_angleU, i_angleV)] = RetValError;
            }
            if (CalculateCorrection(1, nSelRows, Col2, &RetVal, &RetValError)) {
              TCorrection_BiasMap[make_tuple(i_shape, i_pk, 1, i_angleU, i_angleV)] = RetVal;
              TCorrection_BiasMapErr[make_tuple(i_shape, i_pk, 1, i_angleU, i_angleV)] = RetValError;
            }
            if (CalculateCorrection(2, nSelRows, Col3, &RetVal, &RetValError)) {
              TCorrection_ErrorEstimationMap[make_tuple(i_shape, i_pk, 0, i_angleU, i_angleV)] = RetVal;
              TCorrection_ErrorEstimationMapErr[make_tuple(i_shape, i_pk, 0, i_angleU, i_angleV)] = RetValError;
            }
            if (CalculateCorrection(2, nSelRows, Col4, &RetVal, &RetValError)) {
              TCorrection_ErrorEstimationMap[make_tuple(i_shape, i_pk, 1, i_angleU, i_angleV)] = RetVal;
              TCorrection_ErrorEstimationMapErr[make_tuple(i_shape, i_pk, 1, i_angleU, i_angleV)] = RetValError;
            }

            if (nSelRows >= m_MinHitsInPixels) {
              double PixSizeU = 50.0; //Unit::um  (TODO use more general way to obtain clustersize?)
              double PixSizeV = 0;      //Unit::um
              if ((i_pk == 0) || (i_pk == 4)) {
                PixSizeV = 55.0;
              } else if ((i_pk == 1) || (i_pk == 5)) {
                PixSizeV = 60.0;
              } else if ((i_pk == 2) || (i_pk == 6)) {
                PixSizeV = 70.0;
              } else if ((i_pk == 3) || (i_pk == 7)) {
                PixSizeV = 85.0;
              }
              char sVarexp[100];
              if ((m_UseRealData == kTRUE) || (m_UseTracks == kTRUE)) {
                sprintf(sVarexp, "((InPixU / %f) + 0.5) : ((InPixV / %f) + 0.5)", PixSizeU, PixSizeV);
              } else if ((m_CalibrationKind == 2) || (m_UseTracks == kFALSE)) {
                sprintf(sVarexp, "((InPixUTrue / %f) + 0.5):((InPixVTrue / %f) + 0.5)", PixSizeU, PixSizeV);
              }
              //            printf( "--->%i  %s \n", i_pk, sVarexp);
              for (int i_ipU = 0; i_ipU < m_in_pixelU; i_ipU++) {
                for (int i_ipV = 0; i_ipV < m_in_pixelV; i_ipV++) {
                  B2DEBUG(130, "--> InPixCalibration for: " << i_ipU << ", " << i_ipV);

                  TCut c6a;
                  TCut c6b;
                  TCut c7a;
                  TCut c7b;

                  double MinValU = (double)i_ipU / m_in_pixelU;
                  double MaxValU = (double)(i_ipU + 1) / m_in_pixelU;
                  double MinValV = (double)i_ipV / m_in_pixelV;
                  double MaxValV = (double)(i_ipV + 1) / m_in_pixelV;
                  sprintf(cCat, "((InPixU / %f) + 0.5) > %f", PixSizeU, MinValU);
                  c6a.SetTitle(cCat);
                  sprintf(cCat, "((InPixU / %f) + 0.5) < %f", PixSizeU, MaxValU);
                  c6b.SetTitle(cCat);
                  sprintf(cCat, "((InPixV / %f) + 0.5) > %f", PixSizeV, MinValV);
                  c7a.SetTitle(cCat);
                  sprintf(cCat, "((InPixV / %f) + 0.5) < %f", PixSizeV, MaxValV);
                  c7b.SetTitle(cCat);

                  cFin = c6a && c6b && c7a && c7b && c1 && c2 && c3a && c3b && c4a && c4b && c5;
                  //                printf("------------------------------------------> %s <---\n", cFin.GetTitle());

                  getObject<TTree>(name_SourceTree).Draw(sVarexp, cFin, "goff");

                  int nSelRows2 = (int)getObject<TTree>(name_SourceTree).GetSelectedRows();
                  if (nSelRows2 >= m_MinHitsAcceptInPixels) {
                    TInPixelPositionMap[make_tuple(i_shape, i_pk, i_angleU, i_angleV, i_ipU, i_ipV)] = 1.0;
                  }
                  //                double* Col5 = getObject<TTree>(name_SourceTree).GetV1();
                  //                double* Col6 = getObject<TTree>(name_SourceTree).GetV2();
                  //                for (int i = 0; i < nSelRows2; i++) {
                  //                  printf("------------------------------------------> %i %f %f <---\n",i,Col5[i],Col6[i]);
                  //                }
                }
              }
            }
          }
        }
      }
    }
  }

  B2DEBUG(30, "--> Presets2b done. ");

  auto key = make_tuple(1, 1, 0, 1, 0);
  double& data = TCorrection_BiasMap[key];
  cout << "Test - Old: table " << data << ", vector" << Correction_Bias[0]->GetMatrixArray()[3] << endl;

  Table2Vector(TCorrection_BiasMap, Correction_Bias[0]);
  Table2Vector(TCorrection_ErrorEstimationMap, Correction_ErrorEstimation[0]);
  Table2Vector(TCorrection_BiasMapErr, Correction_BiasErr[0]);
  Table2Vector(TCorrection_ErrorEstimationMapErr, Correction_ErrorEstimationErr[0]);
  Table2Vector(TInPixelPositionMap, InPixelPosition[0]);
//  Vector2Table(Correction_Bias[0], TCorrection_BiasMap);
//  Vector2Table(Correction_ErrorEstimation[0], TCorrection_ErrorEstimationMap);
//  Vector2Table(InPixelPosition[0], TInPixelPositionMap);

  cout << "Test - New: table " << data << " or " << TCorrection_BiasMap[key] << ", vector " << Correction_Bias[0]->GetMatrixArray()[3]
       << endl;

  B2DEBUG(30, "--> Save calibration to vectors done. ");

  /*
    printf("--> %f <--\n", Correction_Bias[55]->GetMatrixArray()[3]);
      if (i==55) printf("%f \n", Correction_Bias[i][3]);
    TFile *ff = new TFile("Test.root", "recreate");
    for (int i = 0; i < m_shapes * m_pixelkinds * m_dimensions; i++) {
    }
    ff->Close();

    printf("--> %i %i %i - %i\n",(int)Correction_Bias[55]->GetLwb(), Correction_Bias[55]->GetNrows(), Correction_Bias[55]->GetUpb(), Correction_Bias[55]->GetNoElements());
  */

  if (m_DoExpertHistograms) {
    TFile* fDQM = new TFile(name_OutFileDQM, "recreate");
    //  getObject<TTree>(name_SourceTree).Write(name_SourceTree);
    //  getObject<shape>("shape");
    //  getObject<TTree>("shape").Branch();
    h1_phi->Write(nameh_phi);
    h1_theta->Write(nameh_theta);
    h2_phitheta->Write(nameh_phitheta);
    h2_CorMatrixU->Write(nameh_CorMatrixU);
    h2_CorMatrixV->Write(nameh_CorMatrixV);
    fDQM->Close();
  }

  // Here to save corrections in TVectorT format, could change later
  // Use TFile for temporary use,
  // Final use with saveCalibration and database.
  char nameB[100];
  char nameEE[100];
  char nameIP[100];
  TFile* f = new TFile(name_OutFileCalibrations, "recreate");
  sprintf(nameB, "Correction_Bias");
  Correction_Bias[0]->Write(nameB);
  sprintf(nameEE, "Correction_ErrorEstimation");
  Correction_ErrorEstimation[0]->Write(nameEE);
  sprintf(nameIP, "InPixelPosition");
  InPixelPosition[0]->Write(nameIP);
  f->Close();

  saveCalibration(Correction_Bias[0], nameB);
  saveCalibration(Correction_ErrorEstimation[0], nameEE);
  saveCalibration(InPixelPosition[0], nameIP);

  delete [] ValueCors;
  delete [] ValueInPix;
  delete ValueCors;
  delete ValueInPix;
  return c_OK;
}

void PXDClusterShapeCalibrationAlgorithm::Table2Vector(map_typeCorrs& TCorrection, TVectorD* CorVector)
{
  Double_t* ValueCors = new Double_t[m_shapes * m_pixelkinds * m_dimensions * m_anglesU * m_anglesV];
  for (int i_shape = 0; i_shape < m_shapes; i_shape++)
    for (int i_pk = 0; i_pk < m_pixelkinds; i_pk++)
      for (int i_axis = 0; i_axis < m_dimensions; i_axis++)
        for (int i_angleU = 0; i_angleU < m_anglesU; i_angleU++)
          for (int i_angleV = 0; i_angleV < m_anglesV; i_angleV++) {
            auto key = make_tuple(i_shape, i_pk, i_axis, i_angleU, i_angleV);
            auto& data = TCorrection[key];
            int i_vector = i_shape * m_pixelkinds * m_dimensions * m_anglesU * m_anglesV;
            i_vector += i_pk * m_dimensions * m_anglesU * m_anglesV;
            i_vector += i_axis * m_anglesU * m_anglesV;
            i_vector += i_angleU * m_anglesV;
            i_vector += i_angleV;
            ValueCors[i_vector] = data;
          }
  CorVector->SetElements(ValueCors);
//  B2DEBUG(30, "--> Inside Table2Vector " << TCorrection[1][0][0][0][8]);
//  B2DEBUG(30, "--> Inside Table2Vector " << CorVector->GetMatrixArray()[3]);

  delete [] ValueCors;
  delete ValueCors;


}

void PXDClusterShapeCalibrationAlgorithm::Table2Vector(map_typeInPics& TInPix, TVectorD* InPixVector)
{
  Double_t* ValueInPix = new Double_t[m_shapes * m_pixelkinds * m_anglesU * m_anglesV * m_in_pixelU * m_in_pixelV];
  for (int i_shape = 0; i_shape < m_shapes; i_shape++)
    for (int i_pk = 0; i_pk < m_pixelkinds; i_pk++)
      for (int i_angleU = 0; i_angleU < m_anglesU; i_angleU++)
        for (int i_angleV = 0; i_angleV < m_anglesV; i_angleV++)
          for (int i_ipU = 0; i_ipU < m_in_pixelU; i_ipU++)
            for (int i_ipV = 0; i_ipV < m_in_pixelV; i_ipV++) {
              auto key = make_tuple(i_shape, i_pk, i_angleU, i_angleV, m_in_pixelU, m_in_pixelV);
              auto& data = TInPix[key];
              int i_vector = i_shape * m_pixelkinds * m_anglesU * m_anglesV * m_in_pixelU * m_in_pixelV;
              i_vector += i_pk * m_anglesU * m_anglesV * m_in_pixelU * m_in_pixelV;
              i_vector += i_angleU * m_anglesV * m_in_pixelU * m_in_pixelV;
              i_vector += i_angleV * m_in_pixelU * m_in_pixelV;
              i_vector += i_ipU * m_in_pixelV;
              i_vector += i_ipV;
              ValueInPix[i_vector] = data;
            }
  InPixVector->SetElements(ValueInPix);
  delete [] ValueInPix;
  delete ValueInPix;
}

void PXDClusterShapeCalibrationAlgorithm::Vector2Table(TVectorD* CorVector, map_typeCorrs& TCorrection)
{

  for (int i_shape = 0; i_shape < m_shapes; i_shape++)
    for (int i_pk = 0; i_pk < m_pixelkinds; i_pk++)
      for (int i_axis = 0; i_axis < m_dimensions; i_axis++)
        for (int i_angleU = 0; i_angleU < m_anglesU; i_angleU++)
          for (int i_angleV = 0; i_angleV < m_anglesV; i_angleV++) {
            auto key = make_tuple(i_shape, i_pk, i_axis, i_angleU, i_angleV);
            auto& data = TCorrection[key];
            int i_vector = i_shape * m_pixelkinds * m_dimensions * m_anglesU * m_anglesV;
            i_vector += i_pk * m_dimensions * m_anglesU * m_anglesV;
            i_vector += i_axis * m_anglesU * m_anglesV;
            i_vector += i_angleU * m_anglesV;
            i_vector += i_angleV;
            data = CorVector->GetMatrixArray()[i_vector];
          }

  B2DEBUG(30, "--> Inside Vector2Table " << CorVector->GetMatrixArray()[3]);
}

void PXDClusterShapeCalibrationAlgorithm::Vector2Table(TVectorD* InPixVector, map_typeInPics& TInPix)
{

  for (int i_shape = 0; i_shape < m_shapes; i_shape++)
    for (int i_pk = 0; i_pk < m_pixelkinds; i_pk++)
      for (int i_angleU = 0; i_angleU < m_anglesU; i_angleU++)
        for (int i_angleV = 0; i_angleV < m_anglesV; i_angleV++)
          for (int i_ipU = 0; i_ipU < m_in_pixelU; i_ipU++)
            for (int i_ipV = 0; i_ipV < m_in_pixelV; i_ipV++) {
              auto key = make_tuple(i_shape, i_pk, i_angleU, i_angleV, m_in_pixelU, m_in_pixelV);
              auto& data = TInPix[key];
              int i_vector = i_shape * m_pixelkinds * m_anglesU * m_anglesV * m_in_pixelU * m_in_pixelV;
              i_vector += i_pk * m_anglesU * m_anglesV * m_in_pixelU * m_in_pixelV;
              i_vector += i_angleU * m_anglesV * m_in_pixelU * m_in_pixelV;
              i_vector += i_angleV * m_in_pixelU * m_in_pixelV;
              i_vector += i_ipU * m_in_pixelV;
              i_vector += i_ipV;
              data = InPixVector->GetMatrixArray()[i_vector];
            }

  B2DEBUG(30, "--> Inside Vector2Table " << InPixVector->GetMatrixArray()[3]);
}

int PXDClusterShapeCalibrationAlgorithm::getPhiIndex(float phi)
{
  int ind = (int)(phi + TMath::Pi() / 2.0) / (TMath::Pi() / m_anglesU);
  return ind;
}

int PXDClusterShapeCalibrationAlgorithm::getThetaIndex(float theta)
{
  int ind = (int)(theta + TMath::Pi() / 2.0) / (TMath::Pi() / m_anglesV);
  return ind;
}

int PXDClusterShapeCalibrationAlgorithm::CalculateCorrection(int CorCase, int n, double* array, double* val,
    double* valError)
{
  // CorCase: correction case for:
  //   1: Correction_Bias:            MinSamples: 100,  preset = 0, minCorrection = 0.5 micron, MinDistanceInErrors = 3
  //   2: Correction_ErrorEstimation  MinSamples: 100,  preset = 1, minCorrection = 0.05,       MinDistanceInErrors = 3

  int ret = 1;

  int MinSamples;
  double preset;
  double minCorrection;
  double MinDistanceInErrors;
  if (CorCase == 1) {
    MinSamples = 100;
    preset = 0.0;
    minCorrection = 0.5 * Unit::um;
    MinDistanceInErrors = 3.0;
  } else if (CorCase == 2) {
    MinSamples = 100;
    preset = 1.0;
    minCorrection = 0.05;
    MinDistanceInErrors = 3.0;
  } else {
    return 0;
  }

  *val = preset;
  *valError = 0.0;

  if (n < MinSamples) return 0;

  float QuantCut = 0.9; // TODO this parameter can be change...
  double quantiles[2];
  double prob[2];
  int nprob = 2;
  Bool_t isSorted = kFALSE;
  prob[0] = (1. - QuantCut) / 2.;
  prob[1] = 1.0 - prob[0];
  TMath::Quantiles(n, nprob, array, quantiles, prob, isSorted, 0, 7);
  for (int iD = 0; iD < n; iD++) {
    if ((array[iD] < quantiles[0]) || (array[iD] > quantiles[1])) {
      for (int iD2 = iD; iD2 < n - 1; iD2++) {
        array[iD2] = array[iD2 + 1];
      }
      iD--;
      n--;
    }
  }
  // TODO method of bootstrap resampling is best one, for medium time we use simplification

  double DevNew = TMath::Mean(n, array);
  double ErrNew = TMath::RMS(n, array);
  ErrNew /= 0.79; // Correction becouse we cut out 10% of array
  double SEM = ErrNew / sqrt(DevNew);

  // condition for using correction:
  if (DevNew < minCorrection) ret = 0;
  if (DevNew < (MinDistanceInErrors * SEM)) ret = 0;

  *val = DevNew;
  *valError = SEM;

  return ret;
}

