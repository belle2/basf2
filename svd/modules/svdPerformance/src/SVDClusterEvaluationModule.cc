/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <svd/modules/svdPerformance/SVDClusterEvaluationModule.h>
#include <tracking/dataobjects/RecoTrack.h>

using namespace std;
using namespace Belle2;

REG_MODULE(SVDClusterEvaluation)

SVDClusterEvaluationModule::SVDClusterEvaluationModule(): Module()
  , m_interCoor(nullptr)
  , m_interSigma(nullptr)
  , m_clsCoor(nullptr)
  , m_clsResid(nullptr)
  , m_clsMinResid(nullptr)
  , m_clsResid2D(nullptr)
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
  addParam("efficiency_halfWidth", m_halfWidth, " window half width for the determination of the efficiency", float(0.05));
  addParam("ClustersName", m_ClusterName, "Name of DUTs Cluster Store Array.", std::string(""));
  addParam("InterceptsName", m_InterceptName, "Name of Intercept Store Array.", std::string(""));
  addParam("TracksName", m_TrackName, "Name of Track Store Array.", std::string(""));
  addParam("UbinWidth", m_UbinWidth, "Histograms U-bin width (in um)", double(10));
  addParam("VbinWidth", m_VbinWidth, "Histograms V-bin width (in um)", double(10));
  addParam("groupNstrips", m_groupNstrips, "How many strips group together in the 2D residual VS position plot", int(128));
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

  // create new root file
  m_rootFilePtr = new TFile(m_rootFileName.c_str(), "RECREATE");

  //tree initialization
  m_tree = new TTree("tree", "RECREATE");
  b_experiment = m_tree->Branch("exp", &m_experiment, "exp/i");
  b_run = m_tree->Branch("run", &m_run, "run/i");
  b_layer = m_tree->Branch("layer", &m_layer, "layer/i");
  b_ladder = m_tree->Branch("ladder", &m_ladder, "ladder/i");
  b_sensor = m_tree->Branch("sensor", &m_sensor, "sensor/i");
  b_interU = m_tree->Branch("interU", &m_interU, "interU/F");
  b_interV = m_tree->Branch("interV", &m_interV, "interV/F");
  b_interErrU = m_tree->Branch("interErrU", &m_interErrU, "interErrU/F");
  b_interErrV = m_tree->Branch("interErrV", &m_interErrV, "interErrV/F");
  b_interUprime = m_tree->Branch("interUprime", &m_interUprime, "interUprime/F");
  b_interVprime = m_tree->Branch("interVprime", &m_interVprime, "interVprime/F");
  b_interErrUprime = m_tree->Branch("interErrUprime", &m_interErrUprime, "interErrUprime/F");
  b_interErrVprime = m_tree->Branch("interErrVprime", &m_interErrVprime, "interErrVprime/F");
  b_residU = m_tree->Branch("residU", &m_residU, "residU/F");
  b_residV = m_tree->Branch("residV", &m_residV, "residV/F");
  b_clUpos = m_tree->Branch("clUpos", &m_clUpos, "clUpos/F");
  b_clVpos = m_tree->Branch("clVpos", &m_clVpos, "clVpos/F");
  b_clUcharge = m_tree->Branch("clUcharge", &m_clUcharge, "clUcharge/F");
  b_clVcharge = m_tree->Branch("clVcharge", &m_clVcharge, "clVcharge/F");
  b_clUsnr = m_tree->Branch("clUsnr", &m_clUsnr, "clUsnr/F");
  b_clVsnr = m_tree->Branch("clVsnr", &m_clVsnr, "clVsnr/F");
  b_clUsize = m_tree->Branch("clUsize", &m_clUsize, "clUsize/i");
  b_clVsize = m_tree->Branch("clVsize", &m_clVsize, "clVsize/i");
  b_clUtime = m_tree->Branch("clUtime", &m_clUtime, "clUtime/F");
  b_clVtime = m_tree->Branch("clVtime", &m_clVtime, "clVtime/F");

  //tree initialization
  m_treeSummary = new TTree("summary", "RECREATE");
  bs_experiment = m_treeSummary->Branch("exp", &m_experiment, "exp/i");
  bs_run = m_treeSummary->Branch("run", &ms_run, "run/i");
  bs_layer = m_treeSummary->Branch("layer", &ms_layer, "layer/i");
  bs_ladder = m_treeSummary->Branch("ladder", &ms_ladder, "ladder/i");
  bs_sensor = m_treeSummary->Branch("sensor", &ms_sensor, "sensor/i");
  bs_effU = m_treeSummary->Branch("effU", &ms_effU, "effU/F");
  bs_effV = m_treeSummary->Branch("effV", &ms_effV, "effU/F");
  bs_effErrU = m_treeSummary->Branch("effErrU", &ms_effErrU, "effErrU/F");
  bs_effErrV = m_treeSummary->Branch("effErrV", &ms_effErrV, "effErrU/F");
  bs_nIntercepts = m_treeSummary->Branch("nIntercepts", &ms_nIntercepts, "nIntercepts/i");
  bs_residU = m_treeSummary->Branch("residU", &ms_residU, "residU/F");
  bs_residV = m_treeSummary->Branch("residV", &ms_residV, "residU/F");
  bs_misU = m_treeSummary->Branch("misU", &ms_misU, "misU/F");
  bs_misV = m_treeSummary->Branch("misV", &ms_misV, "misU/F");
  bs_statU = m_treeSummary->Branch("statU", &ms_statU, "statU/F");
  bs_statV = m_treeSummary->Branch("statV", &ms_statV, "statU/F");

}


void SVDClusterEvaluationModule::beginRun()
{

  if (m_interCoor == nullptr) {

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

  //tree variables - event
  StoreObjPtr<EventMetaData> meta;
  m_run = meta->getRun();
  m_experiment = meta->getExperiment();

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

    //tree variables - sensor
    m_layer = VxdID(theVxdID).getLayerNumber();
    m_ladder = VxdID(theVxdID).getLadderNumber();
    m_sensor = VxdID(theVxdID).getSensorNumber();
    //tree variables - intercept
    m_interU =  coorU;
    m_interV =  coorV;
    m_interErrU = sigmaU;
    m_interErrV = sigmaV;
    m_interUprime = m_svdIntercepts[inter]->getUprime();
    m_interVprime = m_svdIntercepts[inter]->getVprime();
    m_interErrUprime = m_svdIntercepts[inter]->getSigmaUprime();
    m_interErrVprime = m_svdIntercepts[inter]->getSigmaVprime();

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
      int idU = -99;
      int idV = -99;
      m_residU = -99;
      m_clUpos = -99;
      m_clUcharge = -99;
      m_clUsnr = -99;
      m_clUsize = -99;
      m_clUtime = -99;
      m_residV = -99;
      m_clVpos = -99;
      m_clVcharge = -99;
      m_clVsnr = -99;
      m_clVsize = -99;
      m_clVtime = -99;

      //loop on clusters
      for (int cls = 0 ; cls < m_svdClusters.getEntries(); cls++) {

        VxdID::baseType clVxdID = (VxdID::baseType)m_svdClusters[cls]->getSensorID();
        if (clVxdID != theVxdID)
          continue;

        double interCoor = coorV;
        double clPos = m_svdClusters[cls]->getPosition();
        //      double interSigma = sigmaV;
        if (m_svdClusters[cls]->isUCluster()) {
          interCoor = coorU;
          //interSigma = sigmaU;
          clPos = m_svdClusters[cls]->getPosition(coorV);
        }
        double resid = interCoor - clPos;
        m_clsResid->fill(theVxdID, m_svdClusters[cls]->isUCluster(), resid);
        m_clsResid2D->fill(theVxdID, m_svdClusters[cls]->isUCluster(), clPos, resid);

        //looking for the minimal residual
        if (m_svdClusters[cls]->isUCluster()) {
          if (fabs(resid) < fabs(minresidU)) {
            minfoundU = true;
            minresidU = resid;
            idU = cls;
          }
        } else {
          if (fabs(resid) < fabs(minresidV)) {
            minfoundV = true;
            minresidV = resid;
            idV = cls;
          }
        }
      }
      if (minfoundU) {
        m_clsMinResid->fill(theVxdID, true, minresidU);
        m_residU = minresidU;
        m_clUpos = m_svdClusters[idU]->getPosition(coorV);
        m_clUcharge = m_svdClusters[idU]->getCharge();
        m_clUsnr = m_svdClusters[idU]->getSNR();
        m_clUsize = (int)m_svdClusters[idU]->getSize();
        m_clUtime = m_svdClusters[idU]->getClsTime();
      }
      if (minfoundV) {
        m_clsMinResid->fill(theVxdID, false, minresidV);
        m_residV = minresidV;
        m_clVpos = m_svdClusters[idV]->getPosition();
        m_clVcharge = m_svdClusters[idV]->getCharge();
        m_clVsnr = m_svdClusters[idV]->getSNR();
        m_clVsize = (int)m_svdClusters[idV]->getSize();
        m_clVtime = m_svdClusters[idV]->getClsTime();
      }

      //fill only if inside fiducial area
      m_tree->Fill();

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

  StoreObjPtr<EventMetaData> meta;
  ms_run = meta->getRun();
  ms_experiment = meta->getExperiment();

  if (m_rootFilePtr != nullptr) {
    m_rootFilePtr->cd();

    m_tree->Write();

    const int Nsensors = 172;//L6
    //    float sensors[Nsensors]; //sensor identificator
    //    float sensorsErr[Nsensors]; //sensor identificator
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
      //      sensors[i] = i;
      //      sensorsErr[i] = 0;
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


    TH1F* h_effU = new TH1F("hEffU", Form("U-Side Summary, %.1f#sigma or #pm%.1f mm", m_nSigma, m_halfWidth * 10), 1, 0, 1);
    h_effU->SetCanExtend(TH1::kAllAxes);
    h_effU->SetStats(0);
    h_effU->GetXaxis()->SetTitle("sensor");
    h_effU->GetYaxis()->SetTitle("U efficiency");
    TH1F* h_effV = new TH1F("hEffV", Form("V-Side Summary, %.1f#sigma or #pm%.1f mm", m_nSigma, m_halfWidth * 10), 1, 0, 1);
    h_effV->SetCanExtend(TH1::kAllAxes);
    h_effV->SetStats(0);
    h_effV->GetXaxis()->SetTitle("sensor");
    h_effV->GetYaxis()->SetTitle("V efficiency");

    TDirectory* oldDir = gDirectory;

    int s = 0; //sensor counter;

    for (auto layer : m_geoCache.getLayers(VXD::SensorInfoBase::SVD)) {
      int currentLayer = layer.getLayerNumber();
      ms_layer = currentLayer;

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
          ms_ladder = (VxdID)sensor.getLadderNumber();
          ms_sensor = (VxdID)sensor.getSensorNumber();

          dir_inter->cd();
          (m_interCoor->getHistogram(sensor, 1))->Write();
          for (int view = SVDHistograms<TH1F>::VIndex ; view < SVDHistograms<TH1F>::UIndex + 1; view++) {
            dir_cls->cd();
            (m_clsCoor->getHistogram(sensor, view))->Write();

            dir_inter->cd();
            float stat = (m_interSigma->getHistogram(sensor, view))->GetMean();
            ms_nIntercepts = (m_interSigma->getHistogram(sensor, view))->GetEntries();
            (m_interSigma->getHistogram(sensor, view))->Write();

            dir_resid->cd();
            TH1F* res = m_clsMinResid->getHistogram(sensor, view);
            Double_t median, q;
            q = 0.5; // 0.5 for "median"
            {
              if (view == SVDHistograms<TH1F>::UIndex) {
                sensorU[s] = Form("%d.%d.%dU", currentLayer, ladder.getLadderNumber(), sensor.getSensorNumber());
                B2DEBUG(10, "U-side efficiency for " << currentLayer << "." << ladder.getLadderNumber() << "." << sensor.getSensorNumber());

                if (res->GetEntries() > 0) {

                  res->GetQuantiles(1, &median, &q);

                  residU[s] = getOneSigma(res);
                  misU[s] = median;

                  float halfWindow = m_nSigma * residU[s];
                  if (m_nSigma == 0)
                    halfWindow = m_halfWidth;

                  int binMin = res->FindBin(misU[s] - halfWindow);
                  int binMax = res->FindBin(misU[s] + halfWindow);
                  B2DEBUG(10, "from " << misU[s] - halfWindow << " -> binMin = " << binMin);
                  B2DEBUG(10, "to " << misU[s] + halfWindow << " -> binMax = " << binMax);

                  int num = 0;
                  for (int bin = binMin; bin < binMax + 1; bin++)
                    num = num + res->GetBinContent(bin);

                  float bkg = 0;
                  for (int bin = 1; bin < binMin; bin++)
                    bkg = bkg + res->GetBinContent(bin);
                  for (int bin = binMax; bin < res->GetNbinsX() + 1; bin++)
                    bkg = bkg + res->GetBinContent(bin);
                  //remove background clusters estimated from sidebands
                  num = num - bkg * (binMax - binMin + 1.) / (binMin + res->GetNbinsX() - binMax - 1);

                  if (ms_nIntercepts > 0) {
                    effU[s] = 1.*num / ms_nIntercepts;
                    //filling efficiency histogram
                    h_effU->Fill(sensorU[s], effU[s]);
                    if (effU[s] > 1)
                      B2WARNING("something is wrong! efficiency greater than 1: " << num << "/" << ms_nIntercepts);
                    effUErr[s] = sqrt(effU[s] * (1 - effU[s]) / ms_nIntercepts);
                  }
                  B2DEBUG(10, "num = " << num);
                  B2DEBUG(10, "den = " << ms_nIntercepts);
                  B2RESULT("U-side efficiency for " << currentLayer << "." << ladder.getLadderNumber() << "." << sensor.getSensorNumber() << " = " <<
                           effU[s] << " ± " << effUErr[s]);

                  //filling summary Histograms for the U side
                  h_statU->Fill(sensorU[s], stat * m_cmTomicron);
                  h_residU->Fill(sensorU[s], residU[s]*m_cmTomicron);
                  h_misU->Fill(sensorU[s], misU[s]*m_cmTomicron);

                  //finally set branch values
                  ms_residU = residU[s];
                  ms_misU = misU[s];
                  ms_effU = effU[s];
                  ms_effErrU = effUErr[s];
                  ms_statU = stat;
                } else {
                  //set to some values if residual histogram is empty
                  ms_residU = -99;
                  ms_misU = -99;
                  ms_effU = -99;
                  ms_effErrU = -99;
                  ms_statU = -99;
                }


              } else { // V-side
                sensorV[s] = Form("%d.%d.%dV", currentLayer, ladder.getLadderNumber(), sensor.getSensorNumber());
                B2DEBUG(10, "V-side efficiency for " << currentLayer << "." << ladder.getLadderNumber() << "." << sensor.getSensorNumber());

                if (res->GetEntries() > 0) {

                  res->GetQuantiles(1, &median, &q);

                  residV[s] = getOneSigma(res);
                  misV[s] = median;
                  ms_misV = misV[s];

                  float halfWindow = m_nSigma * residV[s];
                  if (m_nSigma == 0)
                    halfWindow = m_halfWidth;

                  int binMin = res->FindBin(misV[s] - halfWindow);
                  int binMax = res->FindBin(misV[s] + halfWindow);
                  B2DEBUG(10, "from " << misV[s] - halfWindow << " -> binMin = " << binMin);
                  B2DEBUG(10, "to " << misV[s] + halfWindow << " -> binMax = " << binMax);

                  //determine signal clusters
                  int num = 0;
                  for (int bin = binMin; bin < binMax + 1; bin++)
                    num = num + res->GetBinContent(bin);

                  float bkg = 0;
                  for (int bin = 1; bin < binMin; bin++)
                    bkg = bkg + res->GetBinContent(bin);
                  for (int bin = binMax; bin < res->GetNbinsX() + 1; bin++)
                    bkg = bkg + res->GetBinContent(bin);
                  //remove background clusters estimated from sidebands
                  num = num - bkg * (binMax - binMin + 1.) / (binMin + res->GetNbinsX() - binMax - 1);

                  if (ms_nIntercepts > 0) {
                    effV[s] = 1.*num / ms_nIntercepts;
                    //filling efficiency histogram
                    h_effV->Fill(sensorV[s], effV[s]);
                    if (effV[s] > 1)
                      B2WARNING("something is wrong! efficiency greater than 1: " << num << "/" << ms_nIntercepts);
                    effVErr[s] = sqrt(effV[s] * (1 - effV[s]) / ms_nIntercepts);
                  }
                  B2DEBUG(10, "num = " << num);
                  B2DEBUG(10, "den = " << ms_nIntercepts);
                  B2RESULT("V-side efficiency for " << currentLayer << "." << ladder.getLadderNumber() << "." << sensor.getSensorNumber() << " = " <<
                           effV[s] << " ± " << effVErr[s]);

                  //filling summary Histograms for the V side
                  h_statV->Fill(sensorV[s], stat * m_cmTomicron);
                  h_residV->Fill(sensorV[s], residV[s]*m_cmTomicron);
                  h_misV->Fill(sensorV[s], misV[s]*m_cmTomicron);

                  //finally set branch values
                  ms_residV = residV[s];
                  ms_misV = misV[s];
                  ms_effV = effV[s];
                  ms_effErrV = effVErr[s];
                  ms_statV = stat;
                } else {
                  //set to some values if residual histogram is empty
                  ms_residV = -99;
                  ms_misV = -99;
                  ms_effV = -99;
                  ms_effErrV = -99;
                  ms_statV = -99;
                }

              }

            }
            B2DEBUG(50, "writing out resid histograms for " << sensor.getLayerNumber() << "." << sensor.getLadderNumber() << "." <<
                    sensor.getSensorNumber() << "." << view);
            (m_clsResid->getHistogram(sensor, view))->Write();
            (res)->Write();
            (m_clsResid2D->getHistogram(sensor, view))->Write();


          }
          m_treeSummary->Fill();
          s++;
        }
    }



    oldDir->cd();
    m_treeSummary->Write();

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


double SVDClusterEvaluationModule::getOneSigma(TH1F* h1)
{

  TH1F* h1_res = (TH1F*)h1->Clone("h1_res");
  double probs[2] = {0.16, 1 - 0.16};
  double quant[2] = {0, 0};
  int nbinsHisto = h1_res->GetNbinsX();
  h1_res->SetBinContent(1, h1_res->GetBinContent(0) + h1_res->GetBinContent(1));
  h1_res->SetBinContent(nbinsHisto, h1_res->GetBinContent(nbinsHisto) + h1_res->GetBinContent(nbinsHisto + 1));
  h1_res->SetBinContent(0, 0);
  h1_res->SetBinContent(nbinsHisto + 1, 0);
  h1_res->GetQuantiles(2, quant, probs);

  return (-quant[0] + quant[1]) / 2;
}
