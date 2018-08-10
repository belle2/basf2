#include <svd/modules/svdPerformance/SVDClusterEvaluationModule.h>
#include <tracking/dataobjects/RecoTrack.h>
#include <TF1.h>
#include <TMath.h>
#include <TCanvas.h>
#include <TGraphErrors.h>
#include <TGraph.h>
#include <TLegend.h>

using namespace std;
using namespace Belle2;

REG_MODULE(SVDClusterEvaluation)

SVDClusterEvaluationModule::SVDClusterEvaluationModule() : Module()
{

  setDescription("This module check performances of SVD reconstruction of VXD TB data");

  addParam("outputFileName", m_rootFileName, "Name of output root file.", std::string("SVDClusterEvaluation_output.root"));

  addParam("LayerUnderStudy", m_theLayer, "Number of the layer under study. If 0, then all layers are plotted", int(0));
  addParam("InterceptSigmaMax", m_interSigmaMax,
           "Max of the histogram that contains the intercept statistical error. Default is OK for Phase2.", double(0.35));
  addParam("uFiducialLength", m_uFiducial,
           "length to be subtracted from the U-edge to consider intercepts inside the sensor. Positive values reduce the area; negative values increase the area",
           double(0));
  addParam("vFiducialLength", m_vFiducial,
           "length to be subtracted from the V-edge to consider intercepts inside the sensor. Positive values reduce the area; negative values increase the area",
           double(0));
  addParam("efficiency_nSigma", m_nSigma, " number of residual sigmas for the determination of the efficiency", float(5));
  addParam("ClustersName", m_ClusterName, "Name of DUTs Cluster Store Array.", std::string(""));
  addParam("InterceptsName", m_InterceptName, "Name of Intercept Store Array.", std::string(""));
  addParam("TracksName", m_TrackName, "Name of Track Store Array.", std::string(""));
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
  m_tracks.isRequired(m_TrackName);

  m_rootFilePtr = new TFile(m_rootFileName.c_str(), "RECREATE");

  m_width_LargeS_U = 5.772;
  m_width_LargeS_V = 12.290;
  m_width_SmallS_U = 3.855;
  m_width_SmallS_V = m_width_LargeS_V;

  m_safety_margin = 0.2;

  m_UbinWidth /= m_cmTomicron;
  m_VbinWidth /= m_cmTomicron;

  m_nBins_LargeS_U = m_width_LargeS_U / m_UbinWidth;
  m_nBins_LargeS_V = m_width_LargeS_V / m_VbinWidth;
  m_nBins_SmallS_U = m_width_SmallS_U / m_UbinWidth;
  m_nBins_SmallS_V = m_width_SmallS_V / m_VbinWidth;

  m_abs_LargeS_U = m_width_LargeS_U / 2 + m_safety_margin;
  m_abs_LargeS_V = m_width_LargeS_V / 2 + m_safety_margin;
  m_abs_SmallS_U = m_width_SmallS_U / 2 + m_safety_margin;
  m_abs_SmallS_V = m_width_SmallS_V / 2 + m_safety_margin;

}


void SVDClusterEvaluationModule::beginRun()
{

  if (m_interCoor != NULL) {
    //INTERCEPTS

    create_SVDHistograms_interCoor();

    create_SVDHistograms_interSigma();

    //CLUSTERS
    create_SVDHistograms_clsCoor();

    create_SVDHistograms_clsResid();
  }


  B2DEBUG(10, "creating empty histograms");
  B2DEBUG(10, "Large sensors, U side: width = " << m_width_LargeS_U << " cm, bin width = " << m_UbinWidth << " cm -> Nbins = " <<
          m_nBins_LargeS_U);
  B2DEBUG(10, "Large sensors, V side: width = " << m_width_LargeS_V << " cm, bin width = " << m_UbinWidth << " cm -> Nbins = " <<
          m_nBins_LargeS_V);

  B2DEBUG(10, "Small sensors, U side: width = " << m_width_SmallS_U << " cm, bin width = " << m_UbinWidth << " cm -> Nbins = " <<
          m_nBins_SmallS_U);
  B2DEBUG(10, "Small sensors, V side: width = " << m_width_SmallS_V << " cm, bin width = " << m_UbinWidth << " cm -> Nbins = " <<
          m_nBins_SmallS_V);




}

void SVDClusterEvaluationModule::event()
{
  //  int nEvent = m_eventMetaData->getEvent();
  //  B2INFO("nEvent = "<<nEvent);

  bool isU = true;

  //intercepts
  for (int inter = 0 ; inter < m_svdIntercepts.getEntries(); inter++) {

    if (!isRelatedToTrack(m_svdIntercepts[inter]))
      continue;

    VxdID::baseType theVxdID = (VxdID::baseType)m_svdIntercepts[inter]->getSensorID();
    double coorU = m_svdIntercepts[inter]->getCoorU();
    double coorV = m_svdIntercepts[inter]->getCoorV();
    double sigmaU = m_svdIntercepts[inter]->getSigmaU();
    double sigmaV = m_svdIntercepts[inter]->getSigmaV();

    const VXD::SensorInfoBase& theSensorInfo = m_geoCache.getSensorInfo(theVxdID);
    if (theSensorInfo.inside(coorU, coorV, -m_uFiducial, -m_vFiducial)) {
      m_interCoor->fill(theVxdID, isU, coorU, coorV);
      m_interSigma->fill(theVxdID, isU, sigmaU);
      m_interSigma->fill(theVxdID, !isU, sigmaV);

      //loop on clusters
      for (int cls = 0 ; cls < m_svdClusters.getEntries(); cls++) {

        VxdID::baseType clVxdID = (VxdID::baseType)m_svdClusters[cls]->getSensorID();
        if (clVxdID != theVxdID)
          continue;

        double interCoor = coorV;
        //      double interSigma = sigmaV;
        if (m_svdClusters[cls]->isUCluster()) {
          interCoor = coorU;
          //interSigma = sigmaU;
        }

        double resid = interCoor - m_svdClusters[cls]->getPosition();
        m_clsResid->fill(theVxdID, m_svdClusters[cls]->isUCluster(), resid);
      }
    }

  }

  //clusters
  for (int cls = 0 ; cls < m_svdClusters.getEntries(); cls++) {

    VxdID::baseType theVxdID = (VxdID::baseType)m_svdClusters[cls]->getSensorID();
    m_clsCoor->fill(theVxdID, m_svdClusters[cls]->isUCluster(), m_svdClusters[cls]->getPosition());

  }


}


void SVDClusterEvaluationModule::endRun()
{

}


void SVDClusterEvaluationModule::terminate()
{

  if (m_rootFilePtr != NULL) {
    m_rootFilePtr->cd();

    const int Nsensors = 172;//L6
    float sensors[Nsensors]; //sensor identificator
    float sensorsErr[Nsensors]; //sensor identificator
    float residU[Nsensors]; //U residuals
    float residV[Nsensors]; //V residuals
    float statU[Nsensors]; //U intercept stat error
    float statV[Nsensors]; //V intercept stat error
    float misU[Nsensors]; //U misalignment
    float misV[Nsensors]; //V misalignment
    //    float resolU[Nsensors]; //U residuals
    //    float resolV[Nsensors]; //V residuals
    float effU[Nsensors];
    float effV[Nsensors];
    float effUErr[Nsensors];
    float effVErr[Nsensors];

    for (int i = 0; i < Nsensors; i++) {
      sensors[i] = i;
      sensorsErr[i] = 0;
      residU[i] = 0;
      residV[i] = 0;
      statU[i] = 0;
      statV[i] = 0;
      misU[i] = 0;
      misV[i] = 0;
      //      resolU[i] = 0;
      //      resolV[i] = 0;
      effU[i] = -1;
      effV[i] = -1;
      effUErr[i] = 0;
      effVErr[i] = 0;
    }

    TDirectory* oldDir = gDirectory;

    int s = 0; //sensor counter;

    for (auto layer : m_geoCache.getLayers(VXD::SensorInfoBase::SVD)) {
      int currentLayer = layer.getLayerNumber();

      if (m_theLayer != 0 && currentLayer != m_theLayer)
        continue;

      TString interName = Form("interceptsL%d", layer.getLayerNumber());
      TString clsName = Form("clustersL%d", layer.getLayerNumber());
      TString residName = Form("residualsL%d", layer.getLayerNumber());
      TDirectory* dir_inter = oldDir->mkdir(interName.Data());
      TDirectory* dir_cls = oldDir->mkdir(clsName.Data());
      TDirectory* dir_resid = oldDir->mkdir(residName.Data());
      for (auto ladder : m_geoCache.getLadders(layer))
        for (Belle2::VxdID sensor :  m_geoCache.getSensors(ladder)) {
          dir_inter->cd();
          (m_interCoor->getHistogram(sensor, 1))->Write();
          for (int view = SVDHistograms<TH1F>::VIndex ; view < SVDHistograms<TH1F>::UIndex + 1; view++) {
            dir_cls->cd();
            (m_clsCoor->getHistogram(sensor, view))->Write();

            dir_inter->cd();
            float stat = (m_interSigma->getHistogram(sensor, view))->GetMean() * m_cmTomicron;;
            int den = (m_interSigma->getHistogram(sensor, view))->GetEntries();
            if (view == SVDHistograms<TH1F>::UIndex)
              statU[s] = stat;
            else
              statV[s] = stat;
            (m_interSigma->getHistogram(sensor, view))->Write();

            dir_resid->cd();
            TH1F* res = m_clsResid->getHistogram(sensor, view);
            if (! fitResiduals(res))
              continue;
            TF1* func = res->GetFunction("function");
            if (func != NULL) {
              if (view == SVDHistograms<TH1F>::UIndex) {
                B2DEBUG(10, "layer = " << currentLayer << ", ladder = " << ladder.getLadderNumber() << " sensor " << sensor.getSensorNumber());
                residU[s] = func->GetParameter("sigma1");
                misU[s] = func->GetParameter("mean1");
                int binMin = res->FindBin(misU[s] - m_nSigma * residU[s]);
                int binMax = res->FindBin(misU[s] + m_nSigma * residU[s]);
                B2DEBUG(10, "from " << misU[s] - m_nSigma * residU[s] << " -> binMin = " << binMin);
                B2DEBUG(10, "to " << misU[s] + m_nSigma * residU[s] << " -> binMax = " << binMax);
                int num = 0;
                for (int bin = binMin; bin < binMax + 1; bin++)
                  num = num + res->GetBinContent(bin);
                if (den > 0) {
                  effU[s] = 1.*num / den;
                  effUErr[s] = sqrt(effU[s] * (1 - effU[s]) / den);
                }
                B2DEBUG(10, "num = " << num);
                B2DEBUG(10, "den = " << den);
                B2DEBUG(10, "eff = " << effU[s] << " ± " << effUErr[s]);
                residU[s] *= m_cmTomicron;
                misU[s] *= m_cmTomicron;
              } else {
                B2DEBUG(10, "layer = " << currentLayer << ", ladder = " << ladder.getLadderNumber() << " sensor " << sensor.getSensorNumber());
                residV[s] = func->GetParameter("sigma1");
                misV[s] = func->GetParameter("mean1");
                int binMin = res->FindBin(misV[s] - m_nSigma * residV[s]);
                int binMax = res->FindBin(misV[s] + m_nSigma * residV[s]);
                B2DEBUG(10, "from " << misV[s] - m_nSigma * residV[s] << " -> binMin = " << binMin);
                B2DEBUG(10, "to " << misV[s] + m_nSigma * residV[s] << " -> binMax = " << binMax);
                int num = 0;
                for (int bin = binMin; bin < binMax + 1; bin++)
                  num = num + res->GetBinContent(bin);
                if (den > 0) {
                  effV[s] = 1.*num / den;
                  effVErr[s] = sqrt(effV[s] * (1 - effV[s]) / den);
                }

                B2DEBUG(10, "num = " << num);
                B2DEBUG(10, "den = " << den);
                B2DEBUG(10, "eff = " << effV[s] << " ± " << effVErr[s]);
                residV[s] *= m_cmTomicron;
                misV[s] *= m_cmTomicron;
              }
            }

            (res)->Write();
          }
          s++;
        }
    }


    TGraph* g_residU = new TGraph(Nsensors, sensors, residU);
    g_residU->SetName("gresidU");
    g_residU->SetTitle("U-Side Summary");
    TGraph* g_residV = new TGraph(Nsensors, sensors, residV);
    g_residV->SetName("gresidV");
    g_residV->SetTitle("V-Side Summary");

    TGraph* g_statU = new TGraph(Nsensors, sensors, statU);
    g_statU->SetName("gstatU");
    g_statU->SetTitle("U Intercept Statistical Error");
    TGraph* g_statV = new TGraph(Nsensors, sensors, statV);
    g_statV->SetName("gstatV");
    g_statV->SetTitle("V Intercept Statistical Error");

    TGraph* g_misU = new TGraph(Nsensors, sensors, misU);
    g_misU->SetName("gmisU");
    g_misU->SetTitle("U Residual Misalignment");
    TGraph* g_misV = new TGraph(Nsensors, sensors, misV);
    g_misV->SetName("gmisV");
    g_misV->SetTitle("V Residual Misalignment");


    TGraphErrors* g_effU = new TGraphErrors(Nsensors, sensors, effU, sensorsErr, effUErr);
    g_effU->SetName("geffU");
    g_effU->SetTitle(Form("U-Side Summary, %.1f#sigma", m_nSigma));
    TGraphErrors* g_effV = new TGraphErrors(Nsensors, sensors, effV, sensorsErr, effVErr);
    g_effV->SetName("geffV");
    g_effV->SetTitle(Form("V-Side Summary, %.1f#sigma", m_nSigma));

    oldDir->cd();
    g_residU->Write();
    g_residV->Write();
    g_statU->Write();
    g_statV->Write();
    g_misU->Write();
    g_misV->Write();
    g_effU->Write();
    g_effV->Write();

    TCanvas* c_summaryU = new TCanvas("summaryU", "U-side Summary");
    g_residU->Draw("AP");
    g_residU->SetLineColor(kRed);
    g_residU->SetMarkerColor(kRed);
    g_residU->SetMarkerStyle(20);
    g_statU->Draw("sameP");
    g_statU->SetLineColor(kBlue);
    g_statU->SetMarkerColor(kBlue);
    g_statU->SetMarkerStyle(22);
    g_misU->Draw("sameP");
    g_misU->SetLineColor(kBlack);
    g_misU->SetMarkerColor(kBlack);
    g_misU->SetMarkerStyle(21);
    TLegend* leg = new TLegend(0.7, 0.7, 0.9, 0.9);
    leg->AddEntry(g_residU, "residuals", "lP");
    leg->AddEntry(g_statU, "extrapolation", "lP");
    leg->AddEntry(g_misU, "misalignment", "lP");
    leg->Draw("same");
    c_summaryU->Write();

    TCanvas* c_summaryV = new TCanvas("summaryV", "V-side Summary");
    g_residV->Draw("AP");
    g_residV->SetLineColor(kRed);
    g_residV->SetMarkerColor(kRed);
    g_residV->SetMarkerStyle(20);
    g_statV->Draw("sameP");
    g_statV->SetLineColor(kBlue);
    g_statV->SetMarkerColor(kBlue);
    g_statV->SetMarkerStyle(22);
    g_misV->Draw("sameP");
    g_misV->SetLineColor(kBlack);
    g_misV->SetMarkerColor(kBlack);
    g_misV->SetMarkerStyle(21);
    leg->Draw("same");
    c_summaryV->Write();

    TCanvas* c_effU = new TCanvas("effU", "U-side Cluster Efficiency");
    g_effU->Draw("AP");
    g_effU->SetLineColor(kRed);
    g_effU->SetMarkerColor(kRed);
    g_effU->SetMarkerStyle(20);
    c_effU->Write();
    TCanvas* c_effV = new TCanvas("effV", "V-side Cluster Efficiency");
    g_effV->Draw("AP");
    g_effV->SetLineColor(kRed);
    g_effV->SetMarkerColor(kRed);
    g_effV->SetMarkerStyle(20);
    c_effV->Write();
  }

  m_rootFilePtr->Close();
}



bool SVDClusterEvaluationModule::isRelatedToTrack(SVDIntercept* inter)
{

  RelationVector<RecoTrack> theRC = DataStore::getRelationsWithObj<RecoTrack>(inter);
  if (theRC.size() == 0)
    return false;

  RelationVector<Track> theTrack = theRC[0]->getRelationsWith<Track>(m_TrackName);

  if (theTrack.size() == 0)
    return false;

  return true;

}

bool SVDClusterEvaluationModule::fitResiduals(TH1F* res)
{

  float range = 0.4;

  B2DEBUG(10, "fitting " << res->GetName());
  TF1* function = new TF1("function", "gaus(0)+gaus(3)", -range, range);
  function->SetParNames("N1", "mean1", "sigma1", "N2", "mean2", "sigma2");
  function->SetParameter(0, 10);
  function->SetParLimits(0, 0, 1000000);
  function->SetParameter(1, 0);
  function->SetParameter(2, 0.01);
  function->SetParLimits(2, 0, 0.1);
  function->SetParameter(3, 1);
  function->SetParLimits(3, 0, 1000000);
  function->SetParameter(4, 0);
  function->SetParameter(5, 1);
  function->SetParLimits(5, 0, 10);

  int fitStatus =  res->Fit(function, "R");
  if (fitStatus == 0)
    return true;
  else
    return false;

}

void SVDClusterEvaluationModule::create_SVDHistograms_interCoor()
{

  TH2F h_coorUV_LargeSensor("interCoor_Large_L@layerL@ladderS@sensor@view",
                            "Intercept 2D Coordinate (layer @layer, ladder @ladder, sensor @sensor)",
                            m_nBins_LargeS_U, -m_abs_LargeS_U, m_abs_LargeS_U,
                            m_nBins_LargeS_V, -m_abs_LargeS_V, m_abs_LargeS_V);
  h_coorUV_LargeSensor.GetXaxis()->SetTitle("Intercept U coordinate (cm)");
  h_coorUV_LargeSensor.GetYaxis()->SetTitle("Intercept V coordinate (cm)");

  TH2F h_coorUV_SmallSensor("interCoor_Small_L@layerL@ladderS@sensor@view",
                            "Intercept 2D Coordinate (layer @layer, ladder @ladder, sensor @sensor)",
                            m_nBins_SmallS_U, -m_abs_SmallS_U, m_abs_SmallS_U,
                            m_nBins_SmallS_V, -m_abs_SmallS_V, m_abs_SmallS_V);
  h_coorUV_SmallSensor.GetXaxis()->SetTitle("Intercept U coordinate (cm)");
  h_coorUV_SmallSensor.GetYaxis()->SetTitle("Intercept V coordinate (cm)");


  m_interCoor = new SVDHistograms<TH2F>(h_coorUV_SmallSensor, h_coorUV_SmallSensor, h_coorUV_LargeSensor, h_coorUV_LargeSensor);
}


void SVDClusterEvaluationModule::create_SVDHistograms_interSigma()
{

  TH1F h_sigmaU("interSigmaU_L@layerL@ladderS@sensor@view",
                "U Intercept Sigma (layer @layer, ladder @ladder, sensor @sensor)",
                100, 0, m_interSigmaMax);
  h_sigmaU.GetXaxis()->SetTitle("Intercept U Error (cm)");

  TH1F h_sigmaV("interSigmaV_L@layerL@ladderS@sensor@view",
                "V Intercept Sigma (layer @layer, ladder @ladder, sensor @sensor)",
                100, 0, m_interSigmaMax);
  h_sigmaV.GetXaxis()->SetTitle("Intercept V Error (cm)");


  m_interSigma = new SVDHistograms<TH1F>(h_sigmaU, h_sigmaV, h_sigmaU, h_sigmaV);
}


void SVDClusterEvaluationModule::create_SVDHistograms_clsCoor()
{


  TH1F h_clcoorU_LargeSensor("clsCoorU_LS_L@layerL@ladderS@sensor@view",
                             "Cluster U Coordinate (layer @layer, ladder @ladder, sensor @sensor, side@view/@side)",
                             m_nBins_LargeS_U, -m_abs_LargeS_U, m_abs_LargeS_U);
  h_clcoorU_LargeSensor.GetXaxis()->SetTitle("Cluster U coordinate (cm)");

  TH1F h_clcoorV_LargeSensor("clsCoorV_LS_L@layerL@ladderS@sensor@view",
                             "Cluster V Coordinate (layer @layer, ladder @ladder, sensor @sensor, side@view/@side)",
                             m_nBins_LargeS_V, -m_abs_LargeS_V, m_abs_LargeS_V);
  h_clcoorV_LargeSensor.GetXaxis()->SetTitle("Cluster V coordinate (cm)");

  TH1F h_clcoorU_SmallSensor("clsCoorU_SS_L@layerL@ladderS@sensor@view",
                             "Cluster U Coordinate (layer @layer, ladder @ladder, sensor @sensor, side@view/@side)",
                             m_nBins_SmallS_U, -m_abs_SmallS_U, m_abs_SmallS_U);
  h_clcoorU_SmallSensor.GetXaxis()->SetTitle("Cluster U coordinate (cm)");

  TH1F h_clcoorV_SmallSensor("clsCoorV_SS_L@layerL@ladderS@sensor@view",
                             "Cluster V Coordinate (layer @layer, ladder @ladder, sensor @sensor, side@view/@side)",
                             m_nBins_SmallS_V, -m_abs_SmallS_V, m_abs_SmallS_V);
  h_clcoorV_SmallSensor.GetXaxis()->SetTitle("Cluster V coordinate (cm)");


  // ("evt number/%1.0f", m_group);

  m_clsCoor = new SVDHistograms<TH1F>(h_clcoorU_SmallSensor, h_clcoorV_SmallSensor, h_clcoorU_LargeSensor, h_clcoorV_LargeSensor);

}

void SVDClusterEvaluationModule::create_SVDHistograms_clsResid()
{

  float range = 0.5;
  int NbinsU = 200;//range*0.0001*2/m_UbinWidth;
  int NbinsV = 200;//range*0.0001*2/m_VbinWidth;

  //CLUSTER RESIDUALS
  TH1F h_clresidU_LargeSensor("clsResidU_LS_L@layerL@ladderS@sensor@view",
                              "U Cluster Residuals (layer @layer, ladder @ladder, sensor @sensor, side@view/@side)",
                              //                             m_nBins_LargeS_U, -m_abs_LargeS_U, m_abs_LargeS_U);
                              NbinsU, -range, range);
  h_clresidU_LargeSensor.GetXaxis()->SetTitle("residual (cm)");

  TH1F h_clresidV_LargeSensor("clsResidV_LS_L@layerL@ladderS@sensor@view",
                              "V Cluster Residuals (layer @layer, ladder @ladder, sensor @sensor, side@view/@side)",
                              //            m_nBins_LargeS_V, -m_abs_LargeS_V, m_abs_LargeS_V);
                              NbinsV, -range, range);
  h_clresidV_LargeSensor.GetXaxis()->SetTitle("residual (cm)");

  TH1F h_clresidU_SmallSensor("clsResidU_SS_L@layerL@ladderS@sensor@view",
                              "U Cluster Residuals (layer @layer, ladder @ladder, sensor @sensor, side@view/@side)",
                              //                             m_nBins_SmallS_U, -m_abs_SmallS_U, m_abs_SmallS_U);
                              NbinsU, -range, range);
  h_clresidU_SmallSensor.GetXaxis()->SetTitle("residual (cm)");

  TH1F h_clresidV_SmallSensor("clsResidV_SS_L@layerL@ladderS@sensor@view",
                              "V Cluster Residuals (layer @layer, ladder @ladder, sensor @sensor, side@view/@side)",
                              //                             m_nBins_SmallS_V, -m_abs_SmallS_V, m_abs_SmallS_V);
                              NbinsU, -range, range);
  h_clresidV_SmallSensor.GetXaxis()->SetTitle("residual (cm)");


  // ("evt number/%1.0f", m_group);

  m_clsResid = new SVDHistograms<TH1F>(h_clresidU_SmallSensor, h_clresidV_SmallSensor, h_clresidU_LargeSensor,
                                       h_clresidV_LargeSensor);
}
