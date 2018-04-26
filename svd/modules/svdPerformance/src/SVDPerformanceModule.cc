#include <svd/modules/svdPerformance/SVDPerformanceModule.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/RelationArray.h>
#include <framework/datastore/RelationVector.h>
#include <geometry/GeometryManager.h>
//#include <framework/dataobjects/EventMetaData.h>
#include <time.h>
#include <list>
#include <mdst/dataobjects/MCParticle.h>
#include <mdst/dataobjects/HitPatternVXD.h>
#include <svd/dataobjects/SVDTrueHit.h>
#include <svd/geometry/SensorInfo.h>
#include <vxd/geometry/GeoCache.h>

#include <boost/foreach.hpp>

using namespace std;
using namespace Belle2;

REG_MODULE(SVDPerformance)

SVDPerformanceModule::SVDPerformanceModule() : Module()
  , m_nTracks(0), m_Pvalue(), m_mom(0), m_nSVDhits(0)
{

  setDescription("This module check performances of SVD reconstruction of VXD TB data");

  addParam("outputFileName", m_rootFileName, "Name of output root file.", std::string("SVDPerformance_output.root"));

  addParam("is2017TBanalysis", m_is2017TBanalysis, "True if analyzing 2017 TB data.", bool(false));
  addParam("isSimulation", m_isSimulation, "True if analyzing simulated data.", bool(false));

  addParam("debugLowTime", m_debugLowTime, "Cluster Time below this number will produce a printout.", float(0.));

  addParam("ShaperDigitsName", m_ShaperDigitName, "Name of ShaperDigit Store Array.", std::string(""));
  addParam("RecoDigitsName", m_RecoDigitName, "Name of RecoDigit Store Array.", std::string(""));
  addParam("ClustersName", m_ClusterName, "Name of Cluster Store Array.", std::string(""));
  addParam("TrackListName", m_TrackName, "Name of Track Store Array.", std::string(""));
  addParam("TrackFitResultListName", m_TrackFitResultName, "Name of TracksFitResult Store Array.", std::string(""));
}

SVDPerformanceModule::~SVDPerformanceModule()
{

}

void SVDPerformanceModule::initialize()
{

  m_svdShapers.isRequired(m_ShaperDigitName);
  m_svdRecos.isOptional(m_RecoDigitName);
  m_svdClusters.isRequired(m_ClusterName);
  //  m_Tracks.isRequired(m_TrackName);
  //  m_recoTracks.isRequired();
  //  m_tfr.isRequired(m_TrackFitResultName);


  B2INFO("    ShaperDigits: " << m_ShaperDigitName);
  B2INFO("      RecoDigits: " << m_RecoDigitName);
  B2INFO("        Clusters: " << m_ClusterName);
  B2INFO("          Tracks: " << m_TrackName);
  B2INFO(" TrackFitResults: " << m_TrackFitResultName);


  //create list of histograms to be saved in the rootfile
  m_histoList_track = new TList;
  m_histoList_corr = new TList;

  for (int i = 0; i < m_nLayers; i++) {
    m_histoList_cluster[i] = new TList;
    m_histoList_clTRK[i] = new TList;
    m_histoList_reco[i] = new TList;
    m_histoList_shaper[i] = new TList;
  }

  m_rootFilePtr = new TFile(m_rootFileName.c_str(), "RECREATE");

  TString NameOfHisto = "";
  TString TitleOfHisto = "";

  for (int s = 0; s < m_nLayers; s++)
    if (m_is2017TBanalysis)
      sensorsOnLayer[s] = 2;
    else
      sensorsOnLayer[s] = s + 2;

  //create histograms
  for (int i = 0; i < m_nLayers; i ++) //loop on Layers
    for (int j = 0; j < (int)sensorsOnLayer[i]; j ++) { //loop on Sensors

      TString nameLayer = "";
      nameLayer += i + 3;

      TString nameSensor = "";
      if (m_is2017TBanalysis) {
        if (i == 0)
          nameSensor += j + 1;
        else if (i == 1 || i == 2)
          nameSensor += j + 2;
        else if (i == 3)
          nameSensor += j + 3;
      } else
        nameSensor += j + 1;

      for (int k = 0; k < m_nSides; k ++) { //loop on Sides


        TString nameSide = "";
        if (k == 1)
          nameSide = "U";
        else if (k == 0)
          nameSide = "V";

        //SHAPER DIGITS
        NameOfHisto = "shaper_N_L" + nameLayer + "S" + nameSensor + "" + nameSide;
        TitleOfHisto = "number of ShaperDigits (L" + nameLayer + ", sensor" + nameSensor + "," + nameSide + " side)";
        h_nShaper[i][j][k] = createHistogram1D(NameOfHisto, TitleOfHisto, 100, 0, 100, "n ShaperDigits", m_histoList_shaper[i]);


        //RECO DIGITS
        NameOfHisto = "reco_N_L" + nameLayer + "S" + nameSensor + "" + nameSide;
        TitleOfHisto = "number of RecoDigits (L" + nameLayer + ", sensor" + nameSensor + "," + nameSide + " side)";
        h_nReco[i][j][k] = createHistogram1D(NameOfHisto, TitleOfHisto, 100, 0, 100, "n RecoDigits", m_histoList_reco[i]);

        NameOfHisto = "reco_charge_L" + nameLayer + "S" + nameSensor + "" + nameSide;
        TitleOfHisto = "charge of RecoDigits (L" + nameLayer + ", sensor" + nameSensor + "," + nameSide + " side)";
        h_recoCharge[i][j][k] = createHistogram1D(NameOfHisto, TitleOfHisto, 100, 0, 100000, "charge (e-)", m_histoList_reco[i]);

        NameOfHisto = "reco_energy_L" + nameLayer + "S" + nameSensor + "" + nameSide;
        TitleOfHisto = "energy collected by RecoDigits (L" + nameLayer + ", sensor" + nameSensor + "," + nameSide + " side)";
        h_recoEnergy[i][j][k] = createHistogram1D(NameOfHisto, TitleOfHisto, 100, 0, 360, "energy (keV)", m_histoList_reco[i]);

        NameOfHisto = "reco_noise_L" + nameLayer + "S" + nameSensor + "" + nameSide;
        TitleOfHisto = "strip noise (L" + nameLayer + ", sensor" + nameSensor + "," + nameSide + " side)";
        h_stripNoise[i][j][k] = createHistogram1D(NameOfHisto, TitleOfHisto, 200, 300, 1800, "strip noise", m_histoList_reco[i]);

        NameOfHisto = "reco_time_L" + nameLayer + "S" + nameSensor + "" + nameSide;
        TitleOfHisto = "strip time (L" + nameLayer + ", sensor" + nameSensor + "," + nameSide + " side)";
        h_recoTime[i][j][k] = createHistogram1D(NameOfHisto, TitleOfHisto, 200, -100, 100, "strip time", m_histoList_reco[i]);


        //CLUSTERS RELATED TO TRACKS
        NameOfHisto = "clTRK_N_L" + nameLayer + "S" + nameSensor + "" + nameSide;
        TitleOfHisto = "number of clusters related to Tracks (L" + nameLayer + ", sensor" + nameSensor + "," + nameSide + " side)";
        h_nCltrk[i][j][k] = createHistogram1D(NameOfHisto, TitleOfHisto, 50, 0, 50, "n clusters", m_histoList_clTRK[i]);

        NameOfHisto = "clTRK_size_L" + nameLayer + "S" + nameSensor + "" + nameSide;
        TitleOfHisto = "cluster size (L" + nameLayer + ", sensor" + nameSensor + "," + nameSide + " side)";
        h_cltrkSize[i][j][k] = createHistogram1D(NameOfHisto, TitleOfHisto, 15, 0, 15, "cluster size", m_histoList_clTRK[i]);

        NameOfHisto = "clTRK_charge_L" + nameLayer + "S" + nameSensor + "" + nameSide;
        TitleOfHisto = "cluster Charge (L" + nameLayer + ", sensor" + nameSensor + "," + nameSide + " side)";
        h_cltrkCharge[i][j][k] = createHistogram1D(NameOfHisto, TitleOfHisto, 500, 0, 100000, "charge(e-)", m_histoList_clTRK[i]);

        NameOfHisto = "clTRK_energy_L" + nameLayer + "S" + nameSensor + "" + nameSide;
        TitleOfHisto = "cluster Energy (L" + nameLayer + ", sensor" + nameSensor + "," + nameSide + " side)";
        h_cltrkEnergy[i][j][k] = createHistogram1D(NameOfHisto, TitleOfHisto, 360, 0, 360, "energy (keV)", m_histoList_clTRK[i]);

        NameOfHisto = "clTRK_SN_L" + nameLayer + "S" + nameSensor + "" + nameSide;
        TitleOfHisto = "cluster S/N (L" + nameLayer + ", sensor" + nameSensor + "," + nameSide + " side)";
        h_cltrkSN[i][j][k] = createHistogram1D(NameOfHisto, TitleOfHisto, 150, 0, 150, "S/N", m_histoList_clTRK[i]);

        NameOfHisto = "clTRK_chrgVSsize_L" + nameLayer + "S" + nameSensor + "" + nameSide;
        TitleOfHisto = "cluster charge VS size (L" + nameLayer + ", sensor" + nameSensor + "," + nameSide + " side)";
        h_cltrkChargeVSSize[i][j][k] = createHistogram2D(NameOfHisto, TitleOfHisto, 500, 0, 100000, "charge (e-)", 15, 0, 15, "cl size",
                                                         m_histoList_clTRK[i]);

        NameOfHisto = "clTRK_SNVSsize_L" + nameLayer + "S" + nameSensor + "" + nameSide;
        TitleOfHisto = "cluster SN ratio VS size (L" + nameLayer + ", sensor" + nameSensor + "," + nameSide + " side)";
        h_cltrkSNVSSize[i][j][k] = createHistogram2D(NameOfHisto, TitleOfHisto, 150, 0, 150, "S/N", 15, 0, 15, "cl size",
                                                     m_histoList_clTRK[i]);

        NameOfHisto = "clTRK_TimeVSsize_L" + nameLayer + "S" + nameSensor + "" + nameSide;
        TitleOfHisto = "cluster time VS size (L" + nameLayer + ", sensor" + nameSensor + "," + nameSide + " side)";
        h_cltrkTimeVSSize[i][j][k] = createHistogram2D(NameOfHisto, TitleOfHisto, 200, -100, 100, "cluster time (ns)", 15, 0, 15, "cl size",
                                                       m_histoList_clTRK[i]);

        NameOfHisto = "clTRK_time_L" + nameLayer + "S" + nameSensor + "" + nameSide;
        TitleOfHisto = "cluster time (L" + nameLayer + ", sensor" + nameSensor + "," + nameSide + " side)";
        h_cltrkTime[i][j][k] = createHistogram1D(NameOfHisto, TitleOfHisto, 200, -100, 100, "cluster time (ns)", m_histoList_clTRK[i]);

        if (m_isSimulation) {
          NameOfHisto = "clTRK_timeVStrueTime_L" + nameLayer + "S" + nameSensor + "" + nameSide;
          TitleOfHisto = "cluster time VS true hit time (L" + nameLayer + ", sensor" + nameSensor + "," + nameSide + " side)";
          h_cltrkTimeVSTrueTime[i][j][k] = createHistogram2D(NameOfHisto, TitleOfHisto, 200, -100, 100, "cluster time", 60, -30, 30,
                                                             "true time", m_histoList_clTRK[i]);
        }

        //1 STRIP CLUSTERS RELATED TO TRACKS
        NameOfHisto = "1clTRK_charge_L" + nameLayer + "S" + nameSensor + "" + nameSide;
        TitleOfHisto = "1-strip cluster Charge (L" + nameLayer + ", sensor" + nameSensor + "," + nameSide + " side)";
        h_1cltrkCharge[i][j][k] = createHistogram1D(NameOfHisto, TitleOfHisto, 500, 0, 100000, "charge(e-)", m_histoList_clTRK[i]);


        NameOfHisto = "1clTRK_SN_L" + nameLayer + "S" + nameSensor + "" + nameSide;
        TitleOfHisto = "1-strip cluster S/N (L" + nameLayer + ", sensor" + nameSensor + "," + nameSide + " side)";
        h_1cltrkSN[i][j][k] = createHistogram1D(NameOfHisto, TitleOfHisto, 150, 0, 150, "S/N", m_histoList_clTRK[i]);

        //2-STRIP CLUSTERS RELATED TO TRACKS
        NameOfHisto = "2clTRK_charge_L" + nameLayer + "S" + nameSensor + "" + nameSide;
        TitleOfHisto = "2-strip cluster Charge (L" + nameLayer + ", sensor" + nameSensor + "," + nameSide + " side)";
        h_2cltrkCharge[i][j][k] = createHistogram1D(NameOfHisto, TitleOfHisto, 500, 0, 100000, "charge(e-)", m_histoList_clTRK[i]);


        NameOfHisto = "2clTRK_SN_L" + nameLayer + "S" + nameSensor + "" + nameSide;
        TitleOfHisto = "2-strip cluster S/N (L" + nameLayer + ", sensor" + nameSensor + "," + nameSide + " side)";
        h_2cltrkSN[i][j][k] = createHistogram1D(NameOfHisto, TitleOfHisto, 150, 0, 150, "S/N", m_histoList_clTRK[i]);

        //CLUSTERS NOT RELATED TO TRACKS
        NameOfHisto = "clNOtrk_N_L" + nameLayer + "S" + nameSensor + "" + nameSide;
        TitleOfHisto = "number of clusters NOT related to Tracks (L" + nameLayer + ", sensor" + nameSensor + "," + nameSide + " side)";
        h_nCl[i][j][k] = createHistogram1D(NameOfHisto, TitleOfHisto, 50, 0, 50, "n clusters", m_histoList_cluster[i]);

        NameOfHisto = "clNOtrk_size_L" + nameLayer + "S" + nameSensor + "" + nameSide;
        TitleOfHisto = "cluster size, NOT related to tracks (L" + nameLayer + ", sensor" + nameSensor + "," + nameSide + " side)";
        h_clSize[i][j][k] = createHistogram1D(NameOfHisto, TitleOfHisto, 15, 0, 15, "cluster size", m_histoList_cluster[i]);

        NameOfHisto = "clNOtrk_charge_L" + nameLayer + "S" + nameSensor + "" + nameSide;
        TitleOfHisto = "cluster Charge, NOT related to tracks (L" + nameLayer + ", sensor" + nameSensor + "," + nameSide + " side)";
        h_clCharge[i][j][k] = createHistogram1D(NameOfHisto, TitleOfHisto, 500, 0, 100000, "charge (e-)", m_histoList_cluster[i]);

        NameOfHisto = "clNOtrk_energy_L" + nameLayer + "S" + nameSensor + "" + nameSide;
        TitleOfHisto = "cluster Energy, NOT related to tracks (L" + nameLayer + ", sensor" + nameSensor + "," + nameSide + " side)";
        h_clEnergy[i][j][k] = createHistogram1D(NameOfHisto, TitleOfHisto, 360, 0, 360, "energy (keV)", m_histoList_cluster[i]);

        NameOfHisto = "clNOtrk_maxbin_L" + nameLayer + "S" + nameSensor + "" + nameSide;
        TitleOfHisto = "cluster Seed maxbin, NOT related to tracks (L" + nameLayer + ", sensor" + nameSensor + "," + nameSide + " side)";
        h_clSeedMaxbin[i][j][k] = createHistogram1D(NameOfHisto, TitleOfHisto, 6, 0, 6, "max bin", m_histoList_cluster[i]);

        NameOfHisto = "clNOtrk_energyVSmaxbin_L" + nameLayer + "S" + nameSensor + "" + nameSide;
        TitleOfHisto = "cluster Energy vs seed max bin U, NOT related to tracks (L" + nameLayer + ", sensor" + nameSensor + "," + nameSide +
                       " side)";
        h_clEnergyVSMaxbin[i][j][k] = createHistogram2D(NameOfHisto, TitleOfHisto, 360, 0, 360, "energy (keV)", 6, 0, 6, "seed max bin",
                                                        m_histoList_cluster[i]);

        if (k == 1) {
          NameOfHisto = "clNOtrk_energyVScoorU_L" + nameLayer + "S" + nameSensor + "" + nameSide;
          TitleOfHisto = "cluster Energy vs coor U, NOT related to tracks (L" + nameLayer + ", sensor" + nameSensor + "," + nameSide +
                         " side)";
          h_clEnergyVSCoorU[i][j][1] = createHistogram2D(NameOfHisto, TitleOfHisto, 360, 0, 360, "energy (keV)", 200, -3, 3, "coor U (cm)",
                                                         m_histoList_cluster[i]);

          NameOfHisto = "clNOtrk_coorU1VScoorU2_L" + nameLayer + "S" + nameSensor + "" + nameSide;
          TitleOfHisto = "cluster coor1 U vs coor2 U, TB=3,4,5 NOT related to tracks (L" + nameLayer + ", sensor" + nameSensor + "," +
                         nameSide +
                         " side)";
          h_clCoor1VSCoor2[i][j][1] = createHistogram1D(NameOfHisto, TitleOfHisto, 400, 0, 4, "delta U (cm)",
                                                        m_histoList_cluster[i]);

          NameOfHisto = "clNOtrk_cellIDU1VScellIDU2_L" + nameLayer + "S" + nameSensor + "" + nameSide;
          TitleOfHisto = "cluster cellID1 U vs cellID2 U, TB=3,4,5  NOT related to tracks (L" + nameLayer + ", sensor" + nameSensor + "," +
                         nameSide +
                         " side)";
          h_clCellID1VSCellID2[i][j][1] = createHistogram1D(NameOfHisto, TitleOfHisto, 768, 0, 768, "delta U (# of cells)",
                                                            m_histoList_cluster[i]);

          NameOfHisto = "clNOtrk_energy12VSdeltaU_L" + nameLayer + "S" + nameSensor + "" + nameSide;
          TitleOfHisto = "cluster energy1+2 U vs delta U, TB=3,4,5  NOT related to tracks (L" + nameLayer + ", sensor" + nameSensor + "," +
                         nameSide +
                         " side)";
          h_clEnergy12VSdelta[i][j][1] = createHistogram2D(NameOfHisto, TitleOfHisto, 360, 0, 360, "energy (keV)", 768, 0, 768,
                                                           "delta U (# of cells)",
                                                           m_histoList_cluster[i]);

        } else {
          NameOfHisto = "clNOtrk_energyVScoorV_L" + nameLayer + "S" + nameSensor + "" + nameSide;
          TitleOfHisto = "cluster Energy vs coor V, NOT related to tracks (L" + nameLayer + ", sensor" + nameSensor + "," + nameSide +
                         " side)";
          h_clEnergyVSCoorV[i][j][0] = createHistogram2D(NameOfHisto, TitleOfHisto, 360, 0, 360, "energy (keV)", 200, -6.5, 6.5,
                                                         "coor V (cm)", m_histoList_cluster[i]);

          NameOfHisto = "clNOtrk_coorV1VScoorV2_L" + nameLayer + "S" + nameSensor + "" + nameSide;
          TitleOfHisto = "cluster coor1 V vs coor2 V, TB=3,4,5 NOT related to tracks (L" + nameLayer + ", sensor" + nameSensor + "," +
                         nameSide +
                         " side)";
          h_clCoor1VSCoor2[i][j][0] = createHistogram1D(NameOfHisto, TitleOfHisto, 400, 0, 6, "delta V (cm)",
                                                        m_histoList_cluster[i]);
          NameOfHisto = "clNOtrk_cellIDV1VScellIDV2_L" + nameLayer + "S" + nameSensor + "" + nameSide;
          TitleOfHisto = "cluster cellID1 V vs cellID2 V, TB=3,4,5 NOT related to tracks (L" + nameLayer + ", sensor" + nameSensor + "," +
                         nameSide +
                         " side)";
          h_clCellID1VSCellID2[i][j][0] = createHistogram1D(NameOfHisto, TitleOfHisto, 768, 0, 768, "delta V (# of cells)",
                                                            m_histoList_cluster[i]);

          NameOfHisto = "clNOtrk_energy12VSdeltaV_L" + nameLayer + "S" + nameSensor + "" + nameSide;
          TitleOfHisto = "cluster energy1+2 V vs delta V, TB=3,4,5  NOT related to tracks (L" + nameLayer + ", sensor" + nameSensor + "," +
                         nameSide +
                         " side)";
          h_clEnergy12VSdelta[i][j][0] = createHistogram2D(NameOfHisto, TitleOfHisto, 360, 0, 360, "energy (keV)", 768, 0, 768,
                                                           "delta V (# of cells)",
                                                           m_histoList_cluster[i]);

        }

        NameOfHisto = "clNOtrk_SN_L" + nameLayer + "S" + nameSensor + "" + nameSide;
        TitleOfHisto = "cluster S/N, NOT related to tracks (L" + nameLayer + ", sensor" + nameSensor + "," + nameSide + " side)";
        h_clSN[i][j][k] = createHistogram1D(NameOfHisto, TitleOfHisto, 150, 0, 150, "S/N", m_histoList_cluster[i]);

        NameOfHisto = "clNOtrk_chrgVSsize_L" + nameLayer + "S" + nameSensor + "" + nameSide;
        TitleOfHisto = "cluster charge VS size, NOT related to tracks (L" + nameLayer + ", sensor" + nameSensor + "," + nameSide + " side)";
        h_clChargeVSSize[i][j][k] = createHistogram2D(NameOfHisto, TitleOfHisto, 500, 0, 100000, "charge (e-)", 15, 0, 15, "cl size",
                                                      m_histoList_cluster[i]);
        NameOfHisto = "clNOtrk_SNVSsize_L" + nameLayer + "S" + nameSensor + "" + nameSide;
        TitleOfHisto = "cluster S/N ratio VS size, NOT related to tracks (L" + nameLayer + ", sensor" + nameSensor + "," + nameSide +
                       " side)";
        h_clSNVSSize[i][j][k] = createHistogram2D(NameOfHisto, TitleOfHisto, 150, 0, 150, "S/N", 15, 0, 15, "cl size",
                                                  m_histoList_cluster[i]);

        NameOfHisto = "clNOtrk_TimeVSsize_L" + nameLayer + "S" + nameSensor + "" + nameSide;
        TitleOfHisto = "cluster time VS size, NOT related to tracks (L" + nameLayer + ", sensor" + nameSensor + "," + nameSide + " side)";
        h_clTimeVSSize[i][j][k] = createHistogram2D(NameOfHisto, TitleOfHisto, 200, -100, 100, "cluster time (ns)", 15, 0, 15, "cl size",
                                                    m_histoList_cluster[i]);

        NameOfHisto = "clNOtrk_time_L" + nameLayer + "S" + nameSensor + "" + nameSide;
        TitleOfHisto = "cluster time (L" + nameLayer + ", sensor" + nameSensor + "," + nameSide + " side)";
        h_clTime[i][j][k] = createHistogram1D(NameOfHisto, TitleOfHisto, 200, -100, 100, "cluster time", m_histoList_cluster[i]);


        if (m_isSimulation) {
          NameOfHisto = "clNOtrk_timeVStrueTime_L" + nameLayer + "S" + nameSensor + "" + nameSide;
          TitleOfHisto = "cluster time VS true hit time (L" + nameLayer + ", sensor" + nameSensor + "," + nameSide + " side)";
          h_clTimeVSTrueTime[i][j][k] = createHistogram2D(NameOfHisto, TitleOfHisto, 200, -100, 100, "cluster time", 60, -30, 30, "true time",
                                                          m_histoList_cluster[i]);
        }

      }
      NameOfHisto = "clNOtrk_coorUVScoorV_L" + nameLayer + "S" + nameSensor;
      TitleOfHisto = "cluster coor U VS cluster coor V, TB=3,4,5 (L" + nameLayer + ", sensor" + nameSensor + ")";
      h_clCoorUVSCoorV[i][j] = createHistogram2D(NameOfHisto, TitleOfHisto, 200, -6.5, 6.5, "coor V", 200, -3, 3, "coor U",
                                                 m_histoList_cluster[i]);

      NameOfHisto = "clNOtrk_energyUVSenergyV_L" + nameLayer + "S" + nameSensor;
      TitleOfHisto = "cluster energy U VS cluster energy V, TB=3,4,5 (L" + nameLayer + ", sensor" + nameSensor + ")";
      h_clEnergyUVSEnergyV[i][j] = createHistogram2D(NameOfHisto, TitleOfHisto, 360, 0, 360, "energy V(keV)", 360, 0, 360,
                                                     "energy U (keV)",
                                                     m_histoList_cluster[i]);
      //QUI
    }

  //correlations
  h_cltrk_UU = createHistogram1D("clTRK_dt_UU", "track-related U-U clusters time difference", 200, -100, 100, "time difference (ns)",
                                 m_histoList_corr);
  h_cltrk_VV = createHistogram1D("clTRK_dt_VV", "track-related V-V clusters time difference", 200, -100, 100, "time difference (ns)",
                                 m_histoList_corr);
  h_cltrk_UV = createHistogram1D("clTRK_dt_UV", "track-related U-V clusters time difference", 200, -100, 100, "time difference (ns)",
                                 m_histoList_corr);

  h_cltrkTime_L4uL5u = createHistogram2D("clTRK_time_L4uL5u", "track-related cluster times on L4 and L5 U sides", 200, -100, 100,
                                         "L4u cluster time", 200, -100, 100, "L5u cluster time",
                                         m_histoList_corr);

  h_cltrkTime_L4vL5v = createHistogram2D("clTRK_time_L4vL5v", "track-related cluster times on L4 and L5 V sides", 200, -100, 100,
                                         "L4v cluster time", 200, -100, 100, "L5v cluster time",
                                         m_histoList_corr);

  h_cltrkTime_L5uL5v = createHistogram2D("clTRK_time_L5uL5v", "track-related cluster times on L5 U vs V sides", 200, -100, 100,
                                         "L5u cluster time", 200, -100, 100, "L5v cluster time",
                                         m_histoList_corr);



  // //correlations
  // h_cl_UU = createHistogram1D("clNOTRK_dt_UU", "track-related U-U clusters time difference", 200, -100, 100, "time difference (ns)",
  //                             m_histoList_corr);
  // h_cl_VV = createHistogram1D("clNOTRK_dt_VV", "track-related V-V clusters time difference", 200, -100, 100, "time difference (ns)",
  //                             m_histoList_corr);
  // h_cl_UV = createHistogram1D("clNOTRK_dt_UV", "track-related U-V clusters time difference", 200, -100, 100, "time difference (ns)",
  //                             m_histoList_corr);

  //tracks
  m_nTracks = createHistogram1D("h1nTracks", "number of Tracks per event", 50, 0, 50, "n Tracks", m_histoList_track);
  m_Pvalue = createHistogram1D("h1pValue", "Tracks p value", 100, 0, 1, "p value", m_histoList_track);
  m_mom = createHistogram1D("h1momentum", " Tracks Momentum", 200, 0, 10, "p (GeV/c)", m_histoList_track);
  m_nSVDhits = createHistogram1D("h1nSVDhits", "# SVD hits per track", 20, 0, 20, "# SVD hits", m_histoList_track);

  m_ntracks = 0;
}

void SVDPerformanceModule::beginRun()
{
  m_nEvents = 0;
}

void SVDPerformanceModule::event()
{
  m_nEvents++;
  float c_eTOkeV = 3.6 / 1000; //keV = e * c_eTOkeV

  //  StoreObjPtr<EventMetaData> eventMetaDataPtr;

  //ShaperDigits
  int nShaperDigi[m_nLayers][m_nSensors][m_nSides];

  //RecoDigits
  int nRecoDigi[m_nLayers][m_nSensors][m_nSides];

  // SVD clusters
  int nCl[m_nLayers][m_nSensors][m_nSides];
  int nCltrk[m_nLayers][m_nSensors][m_nSides];

  for (int i = 0; i < m_nLayers; i ++) //loop on Layers
    for (int j = 0; j < m_nSensors; j ++) //loop on Sensors
      for (int k = 0; k < m_nSides; k ++) { //loop on Sides
        nShaperDigi[i][j][k] = 0;
        nRecoDigi[i][j][k] = 0;
        nCl[i][j][k] = 0;
        nCltrk[i][j][k] = 0;
      }

  //tracks
  if (m_Tracks) {
    m_nTracks->Fill(m_Tracks.getEntries());
    m_ntracks += m_Tracks.getEntries();
  }

  BOOST_FOREACH(Track & track, m_Tracks) {

    const TrackFitResult* tfr = NULL;
    if (m_is2017TBanalysis)
      tfr = track.getTrackFitResult(Const::electron);
    else
      tfr = track.getTrackFitResult(Const::pion);
    if (tfr) {
      m_Pvalue->Fill(tfr->getPValue());
      m_mom->Fill(tfr->getMomentum().Mag());
      m_nSVDhits->Fill((tfr->getHitPatternVXD()).getNSVDHits());

      if (m_is2017TBanalysis) {
        if ((tfr->getPValue() < 0.001) || (tfr->getMomentum().Mag() < 1))
          continue;
      }
    }
    RelationVector<RecoTrack> theRC = DataStore::getRelationsWithObj<RecoTrack>(&track);
    RelationVector<SVDCluster> svdClustersTrack = DataStore::getRelationsWithObj<SVDCluster>(theRC[0]);


    for (int cl = 0 ; cl < (int)svdClustersTrack.size(); cl++) {

      float clCharge = svdClustersTrack[cl]->getCharge();
      float clEnergy = svdClustersTrack[cl]->getCharge() * c_eTOkeV;
      int clSize = svdClustersTrack[cl]->getSize();
      float clSN = svdClustersTrack[cl]->getSNR();
      float clTime = svdClustersTrack[cl]->getClsTime();
      VxdID::baseType theVxdID = (VxdID::baseType)svdClustersTrack[cl]->getSensorID();
      int layer = VxdID(theVxdID).getLayerNumber() - 3;
      int sensor = getSensor(layer, VxdID(theVxdID).getSensorNumber(), m_is2017TBanalysis);
      int side = svdClustersTrack[cl]->isUCluster();

      nCltrk[layer][sensor][side]++;
      h_cltrkCharge[layer][sensor][side]->Fill(clCharge);
      h_cltrkEnergy[layer][sensor][side]->Fill(clEnergy);
      h_cltrkSize[layer][sensor][side]->Fill(clSize);
      h_cltrkSN[layer][sensor][side]->Fill(clSN);
      h_cltrkTime[layer][sensor][side]->Fill(clTime);

      h_cltrkChargeVSSize[layer][sensor][side]->Fill(clCharge, clSize);
      h_cltrkSNVSSize[layer][sensor][side]->Fill(clSN, clSize);
      h_cltrkTimeVSSize[layer][sensor][side]->Fill(clTime, clSize);

      if (layer == 2) //layer5
        for (int cll = 0 ; cll < cl; cll++) {
          VxdID::baseType theVxdID2 = (VxdID::baseType)svdClustersTrack[cll]->getSensorID();
          int side2 = svdClustersTrack[cll]->isUCluster();

          if ((VxdID(theVxdID2).getLayerNumber() == 5) && (side2 != side)) { //L5uL5v
            if (side)
              h_cltrkTime_L5uL5v->Fill(svdClustersTrack[cl]->getClsTime(), svdClustersTrack[cll]->getClsTime());
            else
              h_cltrkTime_L5uL5v->Fill(svdClustersTrack[cll]->getClsTime(), svdClustersTrack[cl]->getClsTime());
          }

          if (VxdID(theVxdID2).getLayerNumber() == 4) { //L4

            if (side2 && side2 == side) //L4uL5u
              h_cltrkTime_L4uL5u->Fill(svdClustersTrack[cl]->getClsTime(), svdClustersTrack[cll]->getClsTime());

            if (!side2 && side2 == side) //L4vL5v
              h_cltrkTime_L4vL5v->Fill(svdClustersTrack[cl]->getClsTime(), svdClustersTrack[cll]->getClsTime());

          }
        }

      //fill time difference
      for (int cl2 = 0 ; cl2 < cl ; cl2++) {

        int layerDist = abs(VxdID(theVxdID).getLayerNumber() - svdClustersTrack[cl2]->getSensorID().getLayerNumber());

        int side2 = svdClustersTrack[cl2]->isUCluster();

        if (layerDist == 0) {
          if ((side == 0) && (side2 == 1))
            h_cltrk_UV -> Fill(svdClustersTrack[cl2]->getClsTime() - svdClustersTrack[cl]->getClsTime());

          if ((side == 1) && (side2 == 0))
            h_cltrk_UV -> Fill(svdClustersTrack[cl]->getClsTime() - svdClustersTrack[cl2]->getClsTime());
        } else if (layerDist == 1) {
          if ((side == 1) && (side2 == 1))
            h_cltrk_UU -> Fill(svdClustersTrack[cl]->getClsTime() - svdClustersTrack[cl2]->getClsTime());

          if ((side == 0) && (side2 == 0))
            h_cltrk_VV -> Fill(svdClustersTrack[cl]->getClsTime() - svdClustersTrack[cl2]->getClsTime());
        }



      }

      if (svdClustersTrack[cl]->getClsTime() < m_debugLowTime) {

        B2DEBUG(10, "CLUSTER WITH A TIME BELOW " << m_debugLowTime << "ns");
        B2DEBUG(10, "size = " << svdClustersTrack[cl]->getSize() << ", SNR = " << svdClustersTrack[cl]->getSNR() << " charge = " <<
                svdClustersTrack[cl]->getCharge() << ", SeedCharge = " << svdClustersTrack[cl]->getSeedCharge() << ", time = " <<
                svdClustersTrack[cl]->getClsTime());
      }


      if (m_isSimulation) {
        RelationVector<SVDTrueHit> svdTrueHitsTrack = DataStore::getRelationsWithObj<SVDTrueHit>(svdClustersTrack[cl]);

        for (int i = 0; i < (int)svdTrueHitsTrack.size(); i++) {
          //      if(svdTrueHitsTrack.size()>0){
          h_cltrkTimeVSTrueTime[layer][sensor][side]->Fill(svdClustersTrack[cl]->getClsTime(), svdTrueHitsTrack[i]->getGlobalTime());
          if (svdClustersTrack[cl]->getClsTime() < m_debugLowTime)
            B2DEBUG(10, "True Hit Time = " << svdTrueHitsTrack[i]->getGlobalTime() << ", EnergyDep = " << svdTrueHitsTrack[i]->getEnergyDep() <<
                    ", size = " << svdTrueHitsTrack.size());
        }


        RelationVector<MCParticle> mcParticleTrack = DataStore::getRelationsWithObj<MCParticle>(svdClustersTrack[cl]);

        if (svdClustersTrack[cl]->getClsTime() < m_debugLowTime)
          if ((int)mcParticleTrack.size() > 0)
            B2DEBUG(10, "MCParticle PDG = " << mcParticleTrack[0]->getPDG() << ", energy = " <<  mcParticleTrack[0]->getEnergy() << ", size = "
                    << mcParticleTrack.size());


      }

      if (clSize == 1) {
        h_1cltrkCharge[layer][sensor][side]->Fill(clCharge);
        h_1cltrkSN[layer][sensor][side]->Fill(svdClustersTrack[cl]->getSNR());
      }

      if (clSize == 2) {
        h_2cltrkCharge[layer][sensor][side]->Fill(clCharge);
        h_2cltrkSN[layer][sensor][side]->Fill(svdClustersTrack[cl]->getSNR());
      }
    }

  }

  if (m_Tracks)
    B2DEBUG(1, "%%%%%%%% NEW EVENT,  number of Tracks =  " << m_Tracks.getEntries());

  //shaper digits
  for (int digi = 0 ; digi < m_svdShapers.getEntries(); digi++) {


    VxdID::baseType theVxdID = (VxdID::baseType)m_svdShapers[digi]->getSensorID();
    int layer = VxdID(theVxdID).getLayerNumber() - 3;
    int sensor = getSensor(layer, VxdID(theVxdID).getSensorNumber(), m_is2017TBanalysis);
    int side = m_svdShapers[digi]->isUStrip();
    nShaperDigi[layer][sensor][side]++;


    if (!m_svdShapers[digi]->isUStrip()) {
      if (((layer == 0) && (m_svdShapers[digi]->getCellID() > 767)) ||
          ((layer != 0) && (m_svdShapers[digi]->getCellID() > 511)))
        B2WARNING(" SVDShaperDigits: unexpected cellID for Layer " << layer << " Ladder " <<  VxdID(theVxdID).getLadderNumber() <<
                  " Sensor " << VxdID(theVxdID).getSensorNumber() << " V side, strip = " << m_svdShapers[digi]->getCellID());
    } else {
      if (m_svdShapers[digi]->getCellID() > 767)
        B2WARNING(" SVDShaperDigits:  unexpected cellID for Layer " << layer << " Ladder " << VxdID(theVxdID).getLadderNumber() <<
                  " Sensor " << VxdID(theVxdID).getSensorNumber() << " U side, strip = " << m_svdShapers[digi]->getCellID());
    }

  }

  //reco digits
  if (m_svdRecos.isValid()) {
    for (int digi = 0 ; digi < m_svdRecos.getEntries(); digi++) {

      VxdID::baseType theVxdID = (VxdID::baseType)m_svdRecos[digi]->getSensorID();
      int layer = VxdID(theVxdID).getLayerNumber() - 3;
      int sensor = getSensor(layer, VxdID(theVxdID).getSensorNumber(), m_is2017TBanalysis);
      int side = m_svdRecos[digi]->isUStrip();
      int cellID = m_svdRecos[digi]->getCellID();

      float thisNoise = m_NoiseCal.getNoiseInElectrons(theVxdID, side, cellID);

      h_stripNoise[layer][sensor][side]->Fill(thisNoise);
      h_recoCharge[layer][sensor][side]->Fill(m_svdRecos[digi]->getCharge());
      h_recoEnergy[layer][sensor][side]->Fill(m_svdRecos[digi]->getCharge()*c_eTOkeV);
      h_recoTime[layer][sensor][side]->Fill(m_svdRecos[digi]->getTime());
      nRecoDigi[layer][sensor][side]++;
    }
  }

  //clusters  NOT related to tracks
  for (int cl = 0 ; cl < m_svdClusters.getEntries(); cl++) {

    float clCharge = m_svdClusters[cl]->getCharge();
    float clEnergy = m_svdClusters[cl]->getCharge() * c_eTOkeV;
    float clCoor = m_svdClusters[cl]->getPosition();
    int clSize = m_svdClusters[cl]->getSize();
    float clTime = m_svdClusters[cl]->getClsTime();
    float clSN = m_svdClusters[cl]->getSNR();
    float seed_maxbin = -1;

    RelationVector<RecoTrack> theRC = DataStore::getRelationsWithObj<RecoTrack>(m_svdClusters[cl]);

    if ((int)theRC.size() > 0)
      continue;

    //look for the max bin of the seed
    RelationVector<SVDRecoDigit> theRecoDigits = DataStore::getRelationsWithObj<SVDRecoDigit>(m_svdClusters[cl]);
    int index_seed = 0;
    float charge = 0;
    for (int r = 0; r < (int)theRecoDigits.size(); r++)
      if (theRecoDigits.weight(r) > charge) {
        index_seed = r;
        charge = theRecoDigits[r]->getCharge();
      }

    if (index_seed > -1) {
      RelationVector<SVDShaperDigit> theSeedShaperDigits = DataStore::getRelationsWithObj<SVDShaperDigit>(theRecoDigits[index_seed]);

      Belle2::SVDShaperDigit::APVFloatSamples samples;
      samples = theSeedShaperDigits[0]->getSamples();
      float amplitude = 0;
      const int nAPVSamples = 6;
      for (int k = 0; k < nAPVSamples; k ++) {
        if (samples[k] > amplitude) {
          amplitude = samples[k];
          seed_maxbin = k;
        }
      }
    }

    VxdID::baseType theVxdID = (VxdID::baseType)m_svdClusters[cl]->getSensorID();
    int side = m_svdClusters[cl]->isUCluster();
    int layer = VxdID(theVxdID).getLayerNumber() - 3;
    int sensor = getSensor(layer, VxdID(theVxdID).getSensorNumber(), m_is2017TBanalysis);

    for (int cl2 = 0 ; cl2 < cl; cl2++) {
      if (clSize > 1)
        break;

      VxdID::baseType theVxdID2 = (VxdID::baseType)m_svdClusters[cl2]->getSensorID();
      int side2 = m_svdClusters[cl2]->isUCluster();
      int layer2 = VxdID(theVxdID2).getLayerNumber() - 3;
      int sensor2 = getSensor(layer, VxdID(theVxdID2).getSensorNumber(), m_is2017TBanalysis);

      float clCoor1 = m_svdClusters[cl]->getPosition();
      float clCoor2 = m_svdClusters[cl2]->getPosition();
      float clEnergy1 = m_svdClusters[cl]->getCharge() * c_eTOkeV;
      float clEnergy2 = m_svdClusters[cl2]->getCharge() *  c_eTOkeV;
      if ((layer != layer2) || (sensor != sensor2))
        continue;

      if (seed_maxbin < 3 || seed_maxbin > 5)
        continue;

      int cellID1 = -1;
      for (int r = 0; r < (int)theRecoDigits.size(); r++)
        if (cellID1 < theRecoDigits[r]->getCellID())
          cellID1 = theRecoDigits[r]->getCellID();

      RelationVector<SVDRecoDigit> theRecoDigits2 = DataStore::getRelationsWithObj<SVDRecoDigit>(m_svdClusters[cl2]);
      int dist = 768;
      for (int r = 0; r < (int)theRecoDigits2.size(); r++)
        if (cellID1 - theRecoDigits2[r]->getCellID() < dist)
          dist = cellID1 - theRecoDigits2[r]->getCellID();

      if (side == 1 && side2 == 1) {
        h_clCoor1VSCoor2[layer][sensor][1]->Fill(clCoor1 - clCoor2); //
        h_clCellID1VSCellID2[layer][sensor][1]->Fill(dist);
        h_clEnergy12VSdelta[layer][sensor][1]->Fill(clEnergy1 + clEnergy2, dist); //
      }
      if (side == 0 && side2 == 0) {
        h_clCoor1VSCoor2[layer][sensor][0]->Fill(clCoor1 - clCoor2); //
        h_clCellID1VSCellID2[layer][sensor][0]->Fill(dist);
        h_clEnergy12VSdelta[layer][sensor][0]->Fill(clEnergy1 + clEnergy2, dist); //
      }

      if (side == 1 && side2 == 0) {
        h_clCoorUVSCoorV[layer][sensor]->Fill(clCoor2, clCoor1); //V, U
        h_clEnergyUVSEnergyV[layer][sensor]->Fill(clEnergy2, clEnergy1); //V, U
      }
      if (side == 0 && side2 == 1) {
        h_clCoorUVSCoorV[layer][sensor]->Fill(clCoor1, clCoor2);
        h_clEnergyUVSEnergyV[layer][sensor]->Fill(clEnergy1, clEnergy2);
      }
    }

    nCl[layer][sensor][side]++;

    if (m_svdClusters[cl]->isUCluster())
      h_clEnergyVSCoorU[layer][sensor][side]->Fill(clEnergy, clCoor);
    else
      h_clEnergyVSCoorV[layer][sensor][side]->Fill(clEnergy, clCoor);

    h_clCharge[layer][sensor][side]->Fill(clCharge);
    h_clEnergy[layer][sensor][side]->Fill(clEnergy);
    h_clSeedMaxbin[layer][sensor][side]->Fill(seed_maxbin);
    h_clEnergyVSMaxbin[layer][sensor][side]->Fill(clEnergy, seed_maxbin);
    h_clSize[layer][sensor][side]->Fill(clSize);
    h_clSN[layer][sensor][side]->Fill(clSN);
    h_clTime[layer][sensor][side]->Fill(clTime);

    h_clChargeVSSize[layer][sensor][side]->Fill(clCharge, clSize);
    h_clSNVSSize[layer][sensor][side]->Fill(clSN, clSize);
    h_clTimeVSSize[layer][sensor][side]->Fill(clTime, clSize);

    if (m_isSimulation) {
      RelationVector<SVDTrueHit> svdTrueHits = DataStore::getRelationsWithObj<SVDTrueHit>(m_svdClusters[cl]);
      if (svdTrueHits.size() > 0)
        h_clTimeVSTrueTime[layer][sensor][side]->Fill(m_svdClusters[cl]->getClsTime(), svdTrueHits[0]->getGlobalTime());
    }

  }

  for (int i = 0; i < m_nLayers; i ++) //loop on Layers
    for (int j = 0; j < (int)sensorsOnLayer[i]; j ++)  //loop on Sensors
      for (int k = 0; k < m_nSides; k ++) { //loop on Sides
        h_nShaper[i][j][k]->Fill(nShaperDigi[i][j][k]);
        h_nReco[i][j][k]->Fill(nRecoDigi[i][j][k]);
        h_nCl[i][j][k]->Fill(nCl[i][j][k]);
        h_nCltrk[i][j][k]->Fill(nCltrk[i][j][k]);
      }
}


void SVDPerformanceModule::endRun()
{

}


void SVDPerformanceModule::terminate()
{

  /*
  B2RESULT(" number tracks = " << m_ntracks);
  B2RESULT(" average number of cluster per layer:");
  B2RESULT(" Layer 3, u = " << h_nCl_L3u->GetMean() << ", v = " << h_nCl_L3v->GetMean());
  B2RESULT(" Layer 4, u = " << h_nCl_L4u->GetMean() << ", v = " << h_nCl_L4v->GetMean());
  B2RESULT(" Layer 5, u = " << h_nCl_L5u->GetMean() << ", v = " << h_nCl_L5v->GetMean());
  B2RESULT(" Layer 6, u = " << h_nCl_L6u->GetMean() << ", v = " << h_nCl_L6v->GetMean());
  B2RESULT(" average cluster size per layer:");
  B2RESULT(" Layer 3, u = " << h_clSize_L3u->GetMean() << ", v = " << h_clSize_L3v->GetMean());
  B2RESULT(" Layer 4, u = " << h_clSize_L4u->GetMean() << ", v = " << h_clSize_L4v->GetMean());
  */
  if (m_rootFilePtr != NULL) {
    m_rootFilePtr->cd();

    TDirectory* oldDir = gDirectory;
    TObject* obj;

    TDirectory* dir_track = oldDir->mkdir("tracks");
    dir_track->cd();
    TIter nextH_track(m_histoList_track);
    while ((obj = nextH_track())) {
      if (obj->InheritsFrom("TH1F"))((TH1F*)obj)->Scale(1. / m_nEvents);
      obj->Write();
    }


    TDirectory* dir_shaper = oldDir->mkdir("shaper");
    dir_shaper->cd();
    for (int i = 0; i < m_nLayers; i++) {
      TString layerName = "shaperL";
      layerName += i + 3;
      TDirectory* dir_layer = dir_shaper->mkdir(layerName.Data());
      dir_layer->cd();
      TIter nextH_shaper(m_histoList_shaper[i]);
      while ((obj = nextH_shaper())) {
        if (obj->InheritsFrom("TH1F"))((TH1F*)obj)->Scale(1. / m_nEvents);
        obj->Write();
      }
    }


    TDirectory* dir_reco = oldDir->mkdir("reco");
    dir_reco->cd();
    for (int i = 0; i < m_nLayers; i++) {
      TString layerName = "recoL";
      layerName += i + 3;
      TDirectory* dir_layer = dir_reco->mkdir(layerName.Data());
      dir_layer->cd();
      TIter nextH_reco(m_histoList_reco[i]);
      while ((obj = nextH_reco())) {
        if (obj->InheritsFrom("TH1F"))((TH1F*)obj)->Scale(1. / m_nEvents);
        obj->Write();
      }
    }

    TDirectory* dir_cluster = oldDir->mkdir("clusters");
    dir_cluster->cd();
    for (int i = 0; i < m_nLayers; i++) {
      TString layerName = "clusterL";
      layerName += i + 3;
      TDirectory* dir_layer = dir_cluster->mkdir(layerName.Data());
      dir_layer->cd();
      TIter nextH_cluster(m_histoList_cluster[i]);
      while ((obj = nextH_cluster())) {
        if (obj->InheritsFrom("TH1F"))((TH1F*)obj)->Scale(1. / m_nEvents);
        obj->Write();
      }
    }

    TDirectory* dir_clTRK = oldDir->mkdir("clustersTrk");
    dir_clTRK->cd();
    for (int i = 0; i < m_nLayers; i++) {
      TString layerName = "clstrkL";
      layerName += i + 3;
      TDirectory* dir_layer = dir_clTRK->mkdir(layerName.Data());
      dir_layer->cd();
      TIter nextH_clTRK(m_histoList_clTRK[i]);
      while ((obj = nextH_clTRK())) {
        if (obj->InheritsFrom("TH1F"))((TH1F*)obj)->Scale(1. / m_nEvents);
        obj->Write();
      }
    }

    oldDir->cd();
    TIter nextH_corr(m_histoList_corr);
    while ((obj = nextH_corr())) {
      if (obj->InheritsFrom("TH1F"))
        ((TH1F*)obj)->Scale(1. / m_nEvents);
      obj->Write();
    }

    m_rootFilePtr->Close();


  }



}


TH1F*  SVDPerformanceModule::createHistogram1D(const char* name, const char* title,
                                               Int_t nbins, Double_t min, Double_t max,
                                               const char* xtitle, TList* histoList)
{

  TH1F* h = new TH1F(name, title, nbins, min, max);

  h->GetXaxis()->SetTitle(xtitle);

  if (histoList)
    histoList->Add(h);

  return h;
}


TH2F*  SVDPerformanceModule::createHistogram2D(const char* name, const char* title,
                                               Int_t nbinsX, Double_t minX, Double_t maxX,
                                               const char* titleX,
                                               Int_t nbinsY, Double_t minY, Double_t maxY,
                                               const char* titleY, TList* histoList)
{

  TH2F* h = new TH2F(name, title, nbinsX, minX, maxX, nbinsY, minY, maxY);

  h->GetXaxis()->SetTitle(titleX);
  h->GetYaxis()->SetTitle(titleY);

  if (histoList)
    histoList->Add(h);

  return h;
}

TH3F*  SVDPerformanceModule::createHistogram3D(const char* name, const char* title,
                                               Int_t nbinsX, Double_t minX, Double_t maxX,
                                               const char* titleX,
                                               Int_t nbinsY, Double_t minY, Double_t maxY,
                                               const char* titleY,
                                               Int_t nbinsZ, Double_t minZ, Double_t maxZ,
                                               const char* titleZ,
                                               TList* histoList)
{

  TH3F* h = new TH3F(name, title, nbinsX, minX, maxX, nbinsY, minY, maxY, nbinsZ, minZ, maxZ);

  h->GetXaxis()->SetTitle(titleX);
  h->GetYaxis()->SetTitle(titleY);
  h->GetZaxis()->SetTitle(titleZ);

  if (histoList)
    histoList->Add(h);

  return h;
}

TH3F*  SVDPerformanceModule::createHistogram3D(const char* name, const char* title,
                                               Int_t nbinsX, Double_t* binsX,
                                               const char* titleX,
                                               Int_t nbinsY, Double_t* binsY,
                                               const char* titleY,
                                               Int_t nbinsZ, Double_t* binsZ,
                                               const char* titleZ,
                                               TList* histoList)
{

  TH3F* h = new TH3F(name, title, nbinsX, binsX, nbinsY, binsY, nbinsZ, binsZ);

  h->GetXaxis()->SetTitle(titleX);
  h->GetYaxis()->SetTitle(titleY);
  h->GetZaxis()->SetTitle(titleZ);

  if (histoList)
    histoList->Add(h);

  return h;
}

TH1*  SVDPerformanceModule::duplicateHistogram(const char* newname, const char* newtitle,
                                               TH1* h, TList* histoList)
{

  TH1F* h1 =  dynamic_cast<TH1F*>(h);
  TH2F* h2 =  dynamic_cast<TH2F*>(h);
  TH3F* h3 =  dynamic_cast<TH3F*>(h);

  TH1* newh = 0;

  if (h1)
    newh = new TH1F(*h1);
  if (h2)
    newh = new TH2F(*h2);
  if (h3)
    newh = new TH3F(*h3);

  newh->SetName(newname);
  newh->SetTitle(newtitle);

  if (histoList)
    histoList->Add(newh);


  return newh;
}

TH1F*  SVDPerformanceModule::createHistogramsRatio(const char* name, const char* title,
                                                   TH1* hNum, TH1* hDen, bool isEffPlot,
                                                   int axisRef)
{

  TH1F* h1den =  dynamic_cast<TH1F*>(hDen);
  TH1F* h1num =  dynamic_cast<TH1F*>(hNum);
  TH2F* h2den =  dynamic_cast<TH2F*>(hDen);
  TH2F* h2num =  dynamic_cast<TH2F*>(hNum);
  TH3F* h3den =  dynamic_cast<TH3F*>(hDen);
  TH3F* h3num =  dynamic_cast<TH3F*>(hNum);

  TH1* hden = 0;
  TH1* hnum = 0;

  if (h1den) {
    hden = new TH1F(*h1den);
    hnum = new TH1F(*h1num);
  }
  if (h2den) {
    hden = new TH2F(*h2den);
    hnum = new TH2F(*h2num);
  }
  if (h3den) {
    hden = new TH3F(*h3den);
    hnum = new TH3F(*h3num);
  }

  TAxis* the_axis;
  TAxis* the_other1;
  TAxis* the_other2;

  if (axisRef == 0) {
    the_axis = hden->GetXaxis();
    the_other1 = hden->GetYaxis();
    the_other2 = hden->GetZaxis();
  } else if (axisRef == 1) {
    the_axis = hden->GetYaxis();
    the_other1 = hden->GetXaxis();
    the_other2 = hden->GetZaxis();
  } else if (axisRef == 2) {
    the_axis = hden->GetZaxis();
    the_other1 = hden->GetXaxis();
    the_other2 = hden->GetYaxis();
  } else
    return NULL;


  TH1F* h;
  if (the_axis->GetXbins()->GetSize())
    h = new TH1F(name, title, the_axis->GetNbins(), (the_axis->GetXbins())->GetArray());
  else
    h = new TH1F(name, title, the_axis->GetNbins(), the_axis->GetXmin(), the_axis->GetXmax());
  h->GetXaxis()->SetTitle(the_axis->GetTitle());

  h->GetYaxis()->SetRangeUser(0.00001, 1);

  double num = 0;
  double den = 0;
  Int_t bin = 0;
  Int_t nBins = 0;


  for (int the_bin = 1; the_bin < the_axis->GetNbins() + 1; the_bin++) {

    num = 0;
    den = 0 ;

    for (int other1_bin = 1; other1_bin < the_other1->GetNbins() + 1; other1_bin++)
      for (int other2_bin = 1; other2_bin < the_other2->GetNbins() + 1; other2_bin++) {

        if (axisRef == 0) bin = hden->GetBin(the_bin, other1_bin, other2_bin);
        else if (axisRef == 1) bin = hden->GetBin(other1_bin, the_bin, other2_bin);
        else if (axisRef == 2) bin = hden->GetBin(other1_bin, other2_bin, the_bin);

        if (hden->IsBinUnderflow(bin))
          B2INFO("  bin = " << bin << "(" << the_bin << "," << other1_bin << "," << other2_bin << "), UNDERFLOW");
        if (hden->IsBinOverflow(bin))
          B2INFO("  bin = " << bin << "(" << the_bin << "," << other1_bin << "," << other2_bin << "), OVERFLOW");

        num += hnum->GetBinContent(bin);
        den += hden->GetBinContent(bin);

        nBins++;

      }

    double eff = 0;
    double err = 0;

    if (den > 0) {
      eff = (double)num / den;
      err = sqrt(eff * (1 - eff)) / sqrt(den);
    }

    if (isEffPlot) {
      h->SetBinContent(the_bin, eff);
      h->SetBinError(the_bin, err);
    } else {
      h->SetBinContent(the_bin, 1 - eff);
      h->SetBinError(the_bin, err);
    }

  }

  return h;

}


void  SVDPerformanceModule::addInefficiencyPlots(TList* histoList, TH3F* h3_xPerMCParticle, TH3F* h3_MCParticle)
{

  if ((h3_xPerMCParticle == NULL) || (h3_MCParticle == NULL))
    return;

  //normalized to MCParticles
  TH1F* h_ineff_pt = createHistogramsRatio("hineffpt", "inefficiency VS pt, normalized to MCParticles", h3_xPerMCParticle,
                                           h3_MCParticle, false, 0);
  histoList->Add(h_ineff_pt);

  TH1F* h_ineff_theta = createHistogramsRatio("hinefftheta", "inefficiency VS #theta, normalized to MCParticles",
                                              h3_xPerMCParticle, h3_MCParticle, false, 1);
  histoList->Add(h_ineff_theta);

  TH1F* h_ineff_phi = createHistogramsRatio("hineffphi", "inefficiency VS #phi, normalized to MCParticles", h3_xPerMCParticle,
                                            h3_MCParticle, false, 2);
  histoList->Add(h_ineff_phi);

}

void  SVDPerformanceModule::addEfficiencyPlots(TList* histoList, TH3F* h3_xPerMCParticle, TH3F* h3_MCParticle)
{
  if ((h3_xPerMCParticle == NULL) || (h3_MCParticle == NULL))
    return;

  //normalized to MCParticles
  TH1F* h_eff_pt = createHistogramsRatio("heffpt", "efficiency VS pt, normalized to MCParticles", h3_xPerMCParticle,
                                         h3_MCParticle, true, 0);
  histoList->Add(h_eff_pt);

  TH1F* h_eff_theta = createHistogramsRatio("hefftheta", "efficiency VS #theta, normalized to MCParticles", h3_xPerMCParticle,
                                            h3_MCParticle, true, 1);
  histoList->Add(h_eff_theta);

  TH1F* h_eff_phi = createHistogramsRatio("heffphi", "efficiency VS #phi, normalized to MCParticles", h3_xPerMCParticle,
                                          h3_MCParticle, true, 2);
  histoList->Add(h_eff_phi);

}



void  SVDPerformanceModule::addPurityPlots(TList* histoList, TH3F* h3_MCParticlesPerX, TH3F* h3_X)
{
  if ((h3_X == NULL) || (h3_MCParticlesPerX == NULL))
    return;

//purity histograms
  TH1F* h_pur_pt = createHistogramsRatio("hpurpt", "purity VS pt", h3_MCParticlesPerX, h3_X, true, 0);
  histoList->Add(h_pur_pt);

  TH1F* h_pur_theta = createHistogramsRatio("hpurtheta", "purity VS #theta", h3_MCParticlesPerX, h3_X, true, 1);
  histoList->Add(h_pur_theta);

  TH1F* h_pur_phi = createHistogramsRatio("hpurphi", "purity VS #phi", h3_MCParticlesPerX, h3_X, true, 2);
  histoList->Add(h_pur_phi);

}




