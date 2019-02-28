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

SVDClusterEvaluationModule::SVDClusterEvaluationModule(): Module()
  , m_interCoor(NULL)
  , m_interSigma(NULL)
  , m_clsCoor(NULL)
  , m_clsResid(NULL)
  , m_clsMinResid(NULL)
  , m_clsResid2D(NULL)
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
  addParam("groupNstrips", m_groupNstrips, "How many strips group together in the 2D residual VS position plot", int(128));
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

  if (m_interCoor == NULL) {

    //INTERCEPTS
    create_SVDHistograms_interCoor();

    create_SVDHistograms_interSigma();

    //CLUSTERS
    create_SVDHistograms_clsCoor();

    create_SVDHistograms_clsResid();

    B2DEBUG(10, "Empty histograms have beein created");
    B2DEBUG(10, "Large sensors, U side: width = " << m_width_LargeS_U << " cm, bin width = " << m_UbinWidth << " cm -> Nbins = " <<
            m_nBins_LargeS_U);
    B2DEBUG(10, "Large sensors, V side: width = " << m_width_LargeS_V << " cm, bin width = " << m_UbinWidth << " cm -> Nbins = " <<
            m_nBins_LargeS_V);

    B2DEBUG(10, "Small sensors, U side: width = " << m_width_SmallS_U << " cm, bin width = " << m_UbinWidth << " cm -> Nbins = " <<
            m_nBins_SmallS_U);
    B2DEBUG(10, "Small sensors, V side: width = " << m_width_SmallS_V << " cm, bin width = " << m_UbinWidth << " cm -> Nbins = " <<
            m_nBins_SmallS_V);
  }






}

void SVDClusterEvaluationModule::event()
{
  //  int nEvent = m_eventMetaData->getEvent();
  //  B2DEBUG(10, "nEvent = " << nEvent << ": n intercepts = " << m_svdIntercepts.getEntries() << "n clusters DUT = " << m_svdClusters.getEntries());
  bool isU = true;

  //intercepts
  for (int inter = 0 ; inter < m_svdIntercepts.getEntries(); inter++) {

    if (!isRelatedToTrack(m_svdIntercepts[inter]))
      continue;

    B2DEBUG(10, "this intercept is related to a good track");

    VxdID::baseType theVxdID = (VxdID::baseType)m_svdIntercepts[inter]->getSensorID();
    double coorU = m_svdIntercepts[inter]->getCoorU();
    double coorV = m_svdIntercepts[inter]->getCoorV();
    double sigmaU = m_svdIntercepts[inter]->getSigmaU();
    double sigmaV = m_svdIntercepts[inter]->getSigmaV();

    const VXD::SensorInfoBase& theSensorInfo = m_geoCache.getSensorInfo(theVxdID);
    if (theSensorInfo.inside(coorU, coorV, -m_uFiducial, -m_vFiducial)) {
      B2DEBUG(10, "intercept is inside fiducial area");

      m_interCoor->fill(theVxdID, isU, coorU, coorV);
      m_interSigma->fill(theVxdID, isU, sigmaU);
      m_interSigma->fill(theVxdID, !isU, sigmaV);

      double minresidU = 999;
      bool minfoundU = false;
      double minresidV = 999;
      bool minfoundV = false;

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
        m_clsResid2D->fill(theVxdID, m_svdClusters[cls]->isUCluster(), m_svdClusters[cls]->getPosition(), resid);

        //looking for the minimal residual
        if (m_svdClusters[cls]->isUCluster()) {
          if (fabs(resid) < fabs(minresidU)) {
            minfoundU = true;
            minresidU = resid;
          }
        } else {
          if (fabs(resid) < fabs(minresidV)) {
            minfoundV = true;
            minresidV = resid;
          }
        }
      }
      if (minfoundU)
        m_clsMinResid->fill(theVxdID, true, minresidU);
      if (minfoundV)
        m_clsMinResid->fill(theVxdID, false, minresidV);
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
    float misU[Nsensors]; //U misalignment
    float misV[Nsensors]; //V misalignment
    //    float resolU[Nsensors]; //U residuals
    //    float resolV[Nsensors]; //V residuals
    float effU[Nsensors];
    float effV[Nsensors];
    float effUErr[Nsensors];
    float effVErr[Nsensors];
    TString sensorU[Nsensors];
    TString sensorV[Nsensors];

    for (int i = 0; i < Nsensors; i++) {
      sensors[i] = i;
      sensorsErr[i] = 0;
      residU[i] = 0;
      residV[i] = 0;
      misU[i] = 0;
      misV[i] = 0;
      //      resolU[i] = 0;
      //      resolV[i] = 0;
      effU[i] = -1;
      effV[i] = -1;
      effUErr[i] = 0;
      effVErr[i] = 0;
      sensorU[i] = "";
      sensorV[i] = "";
    }

    TH1F* h_residU = new TH1F("hResidU", "U Residuals", 1, 0, 1);
    h_residU->SetCanExtend(TH1::kAllAxes);
    h_residU->SetStats(0);
    h_residU->GetXaxis()->SetTitle("sensor");
    h_residU->GetYaxis()->SetTitle("U residuals (#mum)");
    TH1F* h_residV = new TH1F("hResidV", "V Residuals", 1, 0, 1);
    h_residV->SetCanExtend(TH1::kAllAxes);
    h_residV->SetStats(0);
    h_residV->GetXaxis()->SetTitle("sensor");
    h_residV->GetYaxis()->SetTitle("V residuals (#mum)");

    TH1F* h_statU = new TH1F("hStatU", "U Intercept Statistical Error", 1, 0, 1);
    h_statU->SetCanExtend(TH1::kAllAxes);
    h_statU->SetStats(0);
    h_statU->GetXaxis()->SetTitle("sensor");
    h_statU->GetYaxis()->SetTitle("U extrap. error (#mum)");
    TH1F* h_statV = new TH1F("hStatV", "V Intercept Statistical Error", 1, 0, 1);
    h_statV->SetCanExtend(TH1::kAllAxes);
    h_statV->SetStats(0);
    h_statV->GetXaxis()->SetTitle("sensor");
    h_statV->GetYaxis()->SetTitle("V extrap. error (#mum)");

    TH1F* h_misU = new TH1F("hMisU", "U Residual Misalignment", 1, 0, 1);
    h_misU->SetCanExtend(TH1::kAllAxes);
    h_misU->SetStats(0);
    h_misU->GetXaxis()->SetTitle("sensor");
    h_misU->GetYaxis()->SetTitle("U misalignment (#mum)");
    TH1F* h_misV = new TH1F("hMisV", "V Residual Misalignment", 1, 0, 1);
    h_misV->SetCanExtend(TH1::kAllAxes);
    h_misV->SetStats(0);
    h_misV->GetXaxis()->SetTitle("sensor");
    h_misV->GetYaxis()->SetTitle("V misalignment (#mum)");


    TH1F* h_effU = new TH1F("hEffU", Form("U-Side Summary, %.1f#sigma", m_nSigma), 1, 0, 1);
    h_effU->SetCanExtend(TH1::kAllAxes);
    h_effU->SetStats(0);
    h_effU->GetXaxis()->SetTitle("sensor");
    h_effU->GetYaxis()->SetTitle("U efficiency");
    TH1F* h_effV = new TH1F("hEffV", Form("V-Side Summary, %.1f#sigma", m_nSigma), 1, 0, 1);
    h_effV->SetCanExtend(TH1::kAllAxes);
    h_effV->SetStats(0);
    h_effV->GetXaxis()->SetTitle("sensor");
    h_effV->GetYaxis()->SetTitle("V efficiency");

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
            (m_interSigma->getHistogram(sensor, view))->Write();

            dir_resid->cd();
            TH1F* res = m_clsMinResid->getHistogram(sensor, view);
            if (! fitResiduals(res)) {
              if (view == SVDHistograms<TH1F>::UIndex)
                B2DEBUG(10, "Fit to the Residuals of U-side " << currentLayer << "." << ladder.getLadderNumber() << "." << sensor.getSensorNumber()
                        << " not succesfull, skipping this side");
              else
                B2DEBUG(10, "Fit to the Residuals of V-side " << currentLayer << "." << ladder.getLadderNumber() << "." << sensor.getSensorNumber()
                        << " not succesfull, skipping this side");
              continue;
            }
            TF1* func = res->GetFunction("function");
            if (func == NULL) func = res->GetFunction("functionG1");
            if (func != NULL) {
              if (view == SVDHistograms<TH1F>::UIndex) {
                sensorU[s] = Form("%d.%d.%dU", currentLayer, ladder.getLadderNumber(), sensor.getSensorNumber());
                B2DEBUG(10, "U-side efficiency for " << currentLayer << "." << ladder.getLadderNumber() << "." << sensor.getSensorNumber());
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
                  //filling efficiency histogram
                  h_effU->Fill(sensorU[s], effU[s]);
                  if (effU[s] > 1)
                    B2WARNING("something is wrong! efficiency greater than 1: " << num << "/" << den);
                  effUErr[s] = sqrt(effU[s] * (1 - effU[s]) / den);
                }
                B2DEBUG(10, "num = " << num);
                B2DEBUG(10, "den = " << den);
                B2RESULT("U-side efficiency for " << currentLayer << "." << ladder.getLadderNumber() << "." << sensor.getSensorNumber() << " = " <<
                         effU[s] << " ± " << effUErr[s]);

                //filling summary Histograms for the U side
                h_statU->Fill(sensorU[s], stat);

                residU[s] *= m_cmTomicron;
                h_residU->Fill(sensorU[s], residU[s]);

                misU[s] *= m_cmTomicron;
                h_misU->Fill(sensorU[s], misU[s]);
              } else {
                sensorV[s] = Form("%d.%d.%dV", currentLayer, ladder.getLadderNumber(), sensor.getSensorNumber());
                B2DEBUG(10, "V-side efficiency for " << currentLayer << "." << ladder.getLadderNumber() << "." << sensor.getSensorNumber());
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
                  //filling efficiency histogram
                  h_effV->Fill(sensorV[s], effV[s]);
                  if (effV[s] > 1)
                    B2WARNING("something is wrong! efficiency greater than 1: " << num << "/" << den);
                  effVErr[s] = sqrt(effV[s] * (1 - effV[s]) / den);
                }
                B2DEBUG(10, "num = " << num);
                B2DEBUG(10, "den = " << den);
                B2RESULT("V-side efficiency for " << currentLayer << "." << ladder.getLadderNumber() << "." << sensor.getSensorNumber() << " = " <<
                         effV[s] << " ± " << effVErr[s]);

                //filing summay histograms for the V side
                h_statV->Fill(sensorV[s], stat);

                residV[s] *= m_cmTomicron;
                h_residV->Fill(sensorV[s], residV[s]);
                misV[s] *= m_cmTomicron;
                h_misV->Fill(sensorV[s], misV[s]);
              }
            }
            B2INFO("writing out resid histograms for " << sensor.getLayerNumber() << "." << sensor.getLadderNumber() << "." <<
                   sensor.getSensorNumber() << "." << view);
            (m_clsResid->getHistogram(sensor, view))->Write();
            (res)->Write();
            (m_clsResid2D->getHistogram(sensor, view))->Write();


          }
          s++;
        }
    }



    TGraphErrors* g_effU = new TGraphErrors(Nsensors, sensors, effU, sensorsErr, effUErr);
    g_effU->SetName("geffU");
    g_effU->SetTitle(Form("U-Side Summary, %.1f#sigma", m_nSigma));
    TGraphErrors* g_effV = new TGraphErrors(Nsensors, sensors, effV, sensorsErr, effVErr);
    g_effV->SetName("geffV");
    g_effV->SetTitle(Form("V-Side Summary, %.1f#sigma", m_nSigma));

    oldDir->cd();
    for (int bin = 0; bin < h_residU->GetNbinsX(); bin++)
      h_residU->SetBinError(bin, 0.);
    h_residU->Write();
    for (int bin = 0; bin < h_residV->GetNbinsX(); bin++)
      h_residV->SetBinError(bin, 0.);
    h_residV->Write();
    for (int bin = 0; bin < h_statU->GetNbinsX(); bin++)
      h_statU->SetBinError(bin, 0.);
    h_statU->Write();
    for (int bin = 0; bin < h_statV->GetNbinsX(); bin++)
      h_statV->SetBinError(bin, 0.);
    h_statV->Write();
    for (int bin = 0; bin < h_misU->GetNbinsX(); bin++)
      h_misU->SetBinError(bin, 0.);
    h_misU->Write();
    for (int bin = 0; bin < h_misV->GetNbinsX(); bin++)
      h_misV->SetBinError(bin, 0.);
    h_misV->Write();
    for (int bin = 0; bin < h_effU->GetNbinsX(); bin++)
      h_effU->SetBinError(bin, 0.);
    h_effU->Write();
    for (int bin = 0; bin < h_effV->GetNbinsX(); bin++)
      h_effV->SetBinError(bin, 0.);
    h_effV->Write();

    TCanvas* c_summaryU = new TCanvas("summaryU", "U-side Summary");
    h_residU->Draw("P");
    h_residU->SetLineColor(kRed);
    h_residU->SetMarkerColor(kRed);
    h_residU->SetMarkerStyle(20);
    h_statU->Draw("sameP");
    h_statU->SetLineColor(kBlue);
    h_statU->SetMarkerColor(kBlue);
    h_statU->SetMarkerStyle(22);
    h_misU->Draw("sameP");
    h_misU->SetLineColor(kBlack);
    h_misU->SetMarkerColor(kBlack);
    h_misU->SetMarkerStyle(21);
    TLegend* leg = new TLegend(0.7, 0.7, 0.9, 0.9);
    leg->AddEntry(h_residU, "residuals", "lP");
    leg->AddEntry(h_statU, "extrapolation", "lP");
    leg->AddEntry(h_misU, "misalignment", "lP");
    leg->Draw("same");
    c_summaryU->Write();

    TCanvas* c_summaryV = new TCanvas("summaryV", "V-side Summary");
    h_residV->Draw("P");
    h_residV->SetLineColor(kRed);
    h_residV->SetMarkerColor(kRed);
    h_residV->SetMarkerStyle(20);
    h_statV->Draw("sameP");
    h_statV->SetLineColor(kBlue);
    h_statV->SetMarkerColor(kBlue);
    h_statV->SetMarkerStyle(22);
    h_misV->Draw("sameP");
    h_misV->SetLineColor(kBlack);
    h_misV->SetMarkerColor(kBlack);
    h_misV->SetMarkerStyle(21);
    leg->Draw("same");
    c_summaryV->Write();

    TCanvas* c_effU = new TCanvas("effU", "U-side Cluster Efficiency");
    h_effU->Draw("P");
    h_effU->SetLineColor(kRed);
    h_effU->SetMarkerColor(kRed);
    h_effU->SetMarkerStyle(20);
    c_effU->Write();
    TCanvas* c_effV = new TCanvas("effV", "V-side Cluster Efficiency");
    h_effV->Draw("P");
    h_effV->SetLineColor(kRed);
    h_effV->SetMarkerColor(kRed);
    h_effV->SetMarkerStyle(20);
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

  B2DEBUG(10, "fitting N1G1+N2G2 " << res->GetName());
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

  if (fitStatus != 0) {
    B2DEBUG(10, "previous fit failed, now trying with N1G1 " << res->GetName());
    TF1* function1 = new TF1("functionG1", "gaus(0)", -range, range);
    function1->SetParNames("N1", "mean1", "sigma1");
    function1->SetParameter(0, 10);
    function1->SetParLimits(0, 0, 1000000);
    function1->SetParameter(1, 0);
    function1->SetParameter(2, 0.01);
    function1->SetParLimits(2, 0, 0.1);

    fitStatus =  res->Fit(function1, "R");
  }
  if (fitStatus == 0)
    return true;
  else
    return false;

}

void SVDClusterEvaluationModule::create_SVDHistograms_interCoor()
{

  TH2F h_coorUV_LargeSensor("interCoor_Large_L@layerL@ladderS@sensor",
                            "Intercept 2D Coordinate (layer @layer, ladder @ladder, sensor @sensor)",
                            m_nBins_LargeS_U, -m_abs_LargeS_U, m_abs_LargeS_U,
                            m_nBins_LargeS_V, -m_abs_LargeS_V, m_abs_LargeS_V);
  h_coorUV_LargeSensor.GetXaxis()->SetTitle("Intercept U coordinate (cm)");
  h_coorUV_LargeSensor.GetYaxis()->SetTitle("Intercept V coordinate (cm)");

  TH2F h_coorUV_SmallSensor("interCoor_Small_L@layerL@ladderS@sensor",
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



  m_clsResid = new SVDHistograms<TH1F>(h_clresidU_SmallSensor, h_clresidV_SmallSensor, h_clresidU_LargeSensor,
                                       h_clresidV_LargeSensor);

  //CLUSTER RESIDUALS VS CL POSITION
  const int Nzones_768 = 768 / m_groupNstrips;
  const int Nzones_512 = 512 / m_groupNstrips;

  TH2F h2_clresidU_LargeSensor("clsResid2DU_LS_L@layerL@ladderS@sensor@view",
                               "U Cluster Residuals VS U Cluster Position(layer @layer, ladder @ladder, sensor @sensor, side@view/@side)",
                               Nzones_768, -m_width_LargeS_U / 2, m_width_LargeS_U / 2, NbinsU, -range, range);
  h2_clresidU_LargeSensor.GetYaxis()->SetTitle("residual (cm)");
  h2_clresidU_LargeSensor.GetXaxis()->SetTitle("cluster position (cm)");

  TH2F h2_clresidV_LargeSensor("clsResid2DV_LS_L@layerL@ladderS@sensor@view",
                               "V Cluster Residuals (layer @layer, ladder @ladder, sensor @sensor, side@view/@side)",
                               Nzones_512, -m_width_LargeS_V / 2, m_width_LargeS_V / 2, NbinsV, -range, range);
  h2_clresidV_LargeSensor.GetYaxis()->SetTitle("residual (cm)");
  h2_clresidV_LargeSensor.GetXaxis()->SetTitle("cluster position (cm)");

  TH2F h2_clresidU_SmallSensor("clsResid2DU_SS_L@layerL@ladderS@sensor@view",
                               "U Cluster Residuals (layer @layer, ladder @ladder, sensor @sensor, side@view/@side)",
                               Nzones_768, -m_width_SmallS_U / 2, m_width_SmallS_U / 2, NbinsU, -range, range);
  h2_clresidU_SmallSensor.GetYaxis()->SetTitle("residual (cm)");
  h2_clresidU_SmallSensor.GetXaxis()->SetTitle("cluster position (cm)");

  TH2F h2_clresidV_SmallSensor("clsResid2DV_SS_L@layerL@ladderS@sensor@view",
                               "V Cluster Residuals (layer @layer, ladder @ladder, sensor @sensor, side@view/@side)",
                               Nzones_512, -m_width_SmallS_V / 2, m_width_SmallS_V / 2, NbinsU, -range, range);
  h2_clresidV_SmallSensor.GetYaxis()->SetTitle("residual (cm)");
  h2_clresidV_SmallSensor.GetXaxis()->SetTitle("cluster position (cm)");

  m_clsResid2D = new SVDHistograms<TH2F>(h2_clresidU_SmallSensor, h2_clresidV_SmallSensor, h2_clresidU_LargeSensor,
                                         h2_clresidV_LargeSensor);

  //CLUSTER MINIMUM RESIDUAL
  //CLUSTER RESIDUALS
  TH1F h_clminresidU_LargeSensor("clsMinResidU_LS_L@layerL@ladderS@sensor@view",
                                 "U Cluster Residuals (layer @layer, ladder @ladder, sensor @sensor, side@view/@side)",
                                 //                             m_nBins_LargeS_U, -m_abs_LargeS_U, m_abs_LargeS_U);
                                 NbinsU, -range, range);
  h_clminresidU_LargeSensor.GetXaxis()->SetTitle("residual (cm)");

  TH1F h_clminresidV_LargeSensor("clsMinResidV_LS_L@layerL@ladderS@sensor@view",
                                 "V Cluster Residuals (layer @layer, ladder @ladder, sensor @sensor, side@view/@side)",
                                 //            m_nBins_LargeS_V, -m_abs_LargeS_V, m_abs_LargeS_V);
                                 NbinsV, -range, range);
  h_clminresidV_LargeSensor.GetXaxis()->SetTitle("residual (cm)");

  TH1F h_clminresidU_SmallSensor("clsMinResidU_SS_L@layerL@ladderS@sensor@view",
                                 "U Cluster Residuals (layer @layer, ladder @ladder, sensor @sensor, side@view/@side)",
                                 //                             m_nBins_SmallS_U, -m_abs_SmallS_U, m_abs_SmallS_U);
                                 NbinsU, -range, range);
  h_clminresidU_SmallSensor.GetXaxis()->SetTitle("residual (cm)");

  TH1F h_clminresidV_SmallSensor("clsMinResidV_SS_L@layerL@ladderS@sensor@view",
                                 "V Cluster Residuals (layer @layer, ladder @ladder, sensor @sensor, side@view/@side)",
                                 //                             m_nBins_SmallS_V, -m_abs_SmallS_V, m_abs_SmallS_V);
                                 NbinsU, -range, range);
  h_clminresidV_SmallSensor.GetXaxis()->SetTitle("residual (cm)");



  m_clsMinResid = new SVDHistograms<TH1F>(h_clminresidU_SmallSensor, h_clminresidV_SmallSensor, h_clminresidU_LargeSensor,
                                          h_clminresidV_LargeSensor);


}
