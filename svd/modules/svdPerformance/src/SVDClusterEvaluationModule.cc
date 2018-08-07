#include <svd/modules/svdPerformance/SVDClusterEvaluationModule.h>
#include <TMath.h>

using namespace std;
using namespace Belle2;

REG_MODULE(SVDClusterEvaluation)

SVDClusterEvaluationModule::SVDClusterEvaluationModule() : Module()
{

  setDescription("This module check performances of SVD reconstruction of VXD TB data");

  addParam("outputFileName", m_rootFileName, "Name of output root file.", std::string("SVDClusterEvaluation_output.root"));

  addParam("ClustersName", m_ClusterName, "Name of DUTs Cluster Store Array.", std::string(""));
  addParam("InterceptsName", m_InterceptName, "Name of Intercept Store Array.", std::string(""));
  addParam("UbinWidth", m_UbinWidth, "Histograms U-bin width (in um)", double(10));
  addParam("VbinWidth", m_VbinWidth, "Histograms V-bin width (in um)", double(10));

}

SVDClusterEvaluationModule::~SVDClusterEvaluationModule()
{

}

void SVDClusterEvaluationModule::initialize()
{

  m_eventMetaData.isRequired();
  m_svdClusters.isRequired(m_ClusterName);
  m_svdIntercepts.isRequired(m_InterceptName);

  m_rootFilePtr = new TFile(m_rootFileName.c_str(), "RECREATE");

}


void SVDClusterEvaluationModule::beginRun()
{

  float width_LargeS_U = 5.772;
  float width_LargeS_V = 12.290;
  float width_SmallS_U = 3.855;
  float width_SmallS_V = width_LargeS_V;

  float safety_margin = 0.2;

  m_UbinWidth *= 0.0001;
  m_VbinWidth *= 0.0001;

  int nBins_LargeS_U = width_LargeS_U / m_UbinWidth;
  int nBins_LargeS_V = width_LargeS_V / m_VbinWidth;
  int nBins_SmallS_U = width_SmallS_U / m_UbinWidth;
  int nBins_SmallS_V = width_SmallS_V / m_VbinWidth;

  float abs_LargeS_U = width_LargeS_U / 2 + safety_margin;
  float abs_LargeS_V = width_LargeS_V / 2 + safety_margin;
  float abs_SmallS_U = width_SmallS_U / 2 + safety_margin;
  float abs_SmallS_V = width_SmallS_V / 2 + safety_margin;

  B2DEBUG(10, "creating empty histograms");
  B2DEBUG(10, "Large sensors, U side: width = " << width_LargeS_U << " cm, bin width = " << m_UbinWidth << " cm -> Nbins = " <<
          nBins_LargeS_U);
  B2DEBUG(10, "Large sensors, V side: width = " << width_LargeS_V << " cm, bin width = " << m_UbinWidth << " cm -> Nbins = " <<
          nBins_LargeS_V);

  B2DEBUG(10, "Small sensors, U side: width = " << width_SmallS_U << " cm, bin width = " << m_UbinWidth << " cm -> Nbins = " <<
          nBins_SmallS_U);
  B2DEBUG(10, "Small sensors, V side: width = " << width_SmallS_V << " cm, bin width = " << m_UbinWidth << " cm -> Nbins = " <<
          nBins_SmallS_V);


  //INTERCEPTS 2D COORDINATES
  TH2F h_coorUV_LargeSensor("interCoor_Large_L@layerL@ladderS@sensor@view",
                            "Intercept 2D Coordinate (layer @layer, ladder @ladder, sensor @sensor)",
                            nBins_LargeS_U, -abs_LargeS_U, abs_LargeS_U,
                            nBins_LargeS_V, -abs_LargeS_V, abs_LargeS_V);
  h_coorUV_LargeSensor.GetXaxis()->SetTitle("Intercept U coordinate (cm)");
  h_coorUV_LargeSensor.GetYaxis()->SetTitle("Intercept V coordinate (cm)");

  TH2F h_coorUV_SmallSensor("interCoor_Small_L@layerL@ladderS@sensor@view",
                            "Intercept 2D Coordinate (layer @layer, ladder @ladder, sensor @sensor)",
                            nBins_SmallS_U, -abs_SmallS_U, abs_SmallS_U,
                            nBins_SmallS_V, -abs_SmallS_V, abs_SmallS_V);
  h_coorUV_SmallSensor.GetXaxis()->SetTitle("Intercept U coordinate (cm)");
  h_coorUV_SmallSensor.GetYaxis()->SetTitle("Intercept V coordinate (cm)");

  m_interCoor = new SVDHistograms<TH2F>(h_coorUV_SmallSensor, h_coorUV_SmallSensor, h_coorUV_LargeSensor, h_coorUV_LargeSensor);

  B2DEBUG(10, "created all intercept coordinates 2D histograms");

  //CLUSTER 1D COORDINATES
  TH1F h_clcoorU_LargeSensor("clsCoorU_LS_L@layerL@ladderS@sensor@view",
                             "Cluster U Coordinate (layer @layer, ladder @ladder, sensor @sensor, side@view/@side)",
                             nBins_LargeS_U, -abs_LargeS_U, abs_LargeS_U);
  h_clcoorU_LargeSensor.GetXaxis()->SetTitle("Cluster U coordinate (cm)");

  TH1F h_clcoorV_LargeSensor("clsCoorV_LS_L@layerL@ladderS@sensor@view",
                             "Cluster V Coordinate (layer @layer, ladder @ladder, sensor @sensor, side@view/@side)",
                             nBins_LargeS_V, -abs_LargeS_V, abs_LargeS_V);
  h_clcoorV_LargeSensor.GetXaxis()->SetTitle("Cluster V coordinate (cm)");

  TH1F h_clcoorU_SmallSensor("clsCoorU_SS_L@layerL@ladderS@sensor@view",
                             "Cluster U Coordinate (layer @layer, ladder @ladder, sensor @sensor, side@view/@side)",
                             nBins_SmallS_U, -abs_SmallS_U, abs_SmallS_U);
  h_clcoorU_SmallSensor.GetXaxis()->SetTitle("Cluster U coordinate (cm)");

  TH1F h_clcoorV_SmallSensor("clsCoorV_SS_L@layerL@ladderS@sensor@view",
                             "Cluster V Coordinate (layer @layer, ladder @ladder, sensor @sensor, side@view/@side)",
                             nBins_SmallS_V, -abs_SmallS_V, abs_SmallS_V);
  h_clcoorV_SmallSensor.GetXaxis()->SetTitle("Cluster V coordinate (cm)");


  // ("evt number/%1.0f", m_group);

  m_clsCoor = new SVDHistograms<TH1F>(h_clcoorU_SmallSensor, h_clcoorV_SmallSensor, h_clcoorU_LargeSensor, h_clcoorV_LargeSensor);

  B2DEBUG(10, "created all cluster coordinates 1D histograms");

}

void SVDClusterEvaluationModule::event()
{
  //  int nEvent = m_eventMetaData->getEvent();

  bool isU = true;

  //intercepts
  for (int inter = 0 ; inter < m_svdIntercepts.getEntries(); inter++) {

    VxdID::baseType theVxdID = (VxdID::baseType)m_svdIntercepts[inter]->getSensorID();
    m_interCoor->fill(theVxdID, isU, m_svdIntercepts[inter]->getCoorU(), m_svdIntercepts[inter]->getCoorV());

  }


  //clusters
  for (int cls = 0 ; cls < m_svdClusters.getEntries(); cls++) {

    VxdID::baseType theVxdID = (VxdID::baseType)m_svdClusters[cls]->getSensorID();
    m_clsCoor->fill(theVxdID, m_svdClusters[cls]->isUCluster(), m_svdClusters[cls]->getPosition());

  }



}


void SVDClusterEvaluationModule::terminate()
{

}


void SVDClusterEvaluationModule::endRun()
{

  if (m_rootFilePtr != NULL) {
    m_rootFilePtr->cd();

    TDirectory* oldDir = gDirectory;
    VXD::GeoCache& geoCache = VXD::GeoCache::getInstance();

    for (auto layer : geoCache.getLayers(VXD::SensorInfoBase::SVD)) {
      TString interName = Form("interceptsL%d", layer.getLayerNumber());
      TString clsName = Form("clustersL%d", layer.getLayerNumber());
      TDirectory* dir_inter = oldDir->mkdir(interName.Data());
      TDirectory* dir_cls = oldDir->mkdir(clsName.Data());
      for (auto ladder : geoCache.getLadders(layer))
        for (Belle2::VxdID sensor :  geoCache.getSensors(ladder)) {
          dir_inter->cd();
          (m_interCoor->getHistogram(sensor, 1))->Write();
          dir_cls->cd();
          for (int view = SVDHistograms<TH1F>::VIndex ; view < SVDHistograms<TH1F>::UIndex + 1; view++)
            (m_clsCoor->getHistogram(sensor, view))->Write();
        }
    }
  }
  m_rootFilePtr->Close();
}


