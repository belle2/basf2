/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <Math/Vector3D.h>
#include <framework/dataobjects/EventMetaData.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/logging/Logger.h>
#include <math.h>
#include <string>
#include <top/modules/TOPBackSplashTiming/TOPBackSplashTimingModule.h>
#include <top/dataobjects/TOPLikelihood.h>
#include <tracking/dataobjects/ExtHit.h>

#include <RooAddPdf.h>
#include <RooArgList.h>
#include <RooArgSet.h>
#include <RooExponential.h>
#include <RooFormulaVar.h>
#include <RooGenericPdf.h>
#include <RooJohnson.h>
#include <RooPolynomial.h>
#include <RooPlot.h>
#include <RooProdPdf.h>
#include <RooMsgService.h>
#include <TCanvas.h>
#include <TF1.h>
#include <TLine.h>
#include <TPaveText.h>

using namespace Belle2;

REG_MODULE(TOPBackSplashTiming);

TOPBackSplashTimingModule::TOPBackSplashTimingModule()
  : Module(),
    m_eclClusters("ECLClusters"),   // DataStore name
    m_digits("TOPDigits"),
    m_tracks("Tracks"),
    m_fitparams{{{  -0.6,   4,  52,  -0.145075,  0.885072,  0.386489,  -1.361947,   0.662144,   7.134312,  0.449013,  27.397423},
    {  -0.5,   4,  51,  -0.082732,  0.795969,  0.674888,  -1.281492,   0.232786,   6.784821,  0.452656,  26.333126},
    {  -0.4,   5,  50,  -0.071727,  0.828312,  0.657696,  -1.228757,   0.297860,   7.371104,  0.514466,  24.651866},
    {  -0.3,   5,  49,  -0.071360,  0.833236,  0.628764,  -1.167383,   0.418553,   7.939829,  0.421581,  23.109511},
    {  -0.2,   6,  49,  -0.075286,  0.850655,  0.631255,  -1.187544,   0.526128,   8.469763,  0.345813,  21.632224},
    {  -0.1,   6,  48,  -0.084704,  0.888931,  0.598693,  -1.350362,   0.595812,   9.081677,  0.433728,  20.394818},
    {   0.0,   7,  48,  -0.091469,  0.859441,  0.587823,  -1.244311,   0.641963,   9.767411,  0.364095,  18.956789},
    {   0.1,   7,  48,  -0.100383,  0.941895,  0.548498,  -1.434151,   0.806756,  10.400005,  0.382099,  17.762602},
    {   0.2,   9,  47,  -0.108262,  0.917920,  0.531122,  -1.417409,   0.875909,  11.205481,  0.355874,  16.347529},
    {   0.3,   9,  48,  -0.125444,  1.024868,  0.493756,  -1.684685,   1.010871,  11.895900,  0.330729,  15.227423},
    {   0.4,  10,  48,  -0.140058,  1.022069,  0.458708,  -1.517339,   1.246522,  13.112702,  0.366439,  13.597788},
    {   0.5,  12,  47,  -0.144628,  1.201504,  0.450720,  -2.059850,   1.280475,  13.949971,  0.278829,  12.030733},
    {   0.6,  13,  48,  -0.156869,  1.095534,  0.451224,  -1.872655,   1.412490,  15.791843,  0.292740,   9.729090},
    {   0.7,  15,  50,  -0.167605,  2.117748,  0.352186,  -1.999952,   3.009412,  17.997337,  0.332440,   6.500688},
    {   0.8,  19,  51,  -0.232060,  0.726019,  0.581841,  -0.142827,   1.205148,  24.876122,  0.457538,   1.328250}}}
// Fit params for combined pdf (See initialise for definition)
// Params:  0-cosTheta, 1-fit start, 2-fit end, 3-coeff, 4-delta, 5-frac, 6-gamma, 7-lambda, 8-mu,  9-w, 10-shift-mu
{
  setDescription(R"DOC(A module to save timing of neutral cluster events derived from nearby TOP interactions)DOC");

  addParam("saveFits", m_saveFits,
           "Set to true to save RooPlot of fit to TOP signal from which the timing is extracted, for debug.",
           false);
  addParam("minClusterE", m_minClusterE,
           "Minimum (incl.) cluster energy to be considered in adding relation to TOP time extraction [GeV]",
           0.5);
  addParam("minNphotons", m_minNphotons,
           "Minimum (incl.) no. of Cherenkov photons measured in TOP slot for which time extraction will take place",
           50);
  addParam("minClusterNHits", m_minClusterNHits,
           "Minimum (incl.) no. of crystals in cluster required for adding relation to TOP time extraction",
           10.0);
  addParam("includeSlotsWithTracks", m_includeSlotsWithTracks,
           "Flag to allow fitting of neutral cluster TOP signal even if there are charged tracks matched to the same slot",
           false);
  addParam("saveMoreFitParams", m_saveMoreFitParams,
           "Flag to save additonal parameters from TOP timing fit (e.g. RooFit errors) to output, for debug.",
           false);
}

void TOPBackSplashTimingModule::prepareFitModels()
{
  RooMsgService::instance().setGlobalKillBelow(RooFit::WARNING);

  // Container for RooWorkspaces
  m_wss.clear();

  for (auto row : m_fitparams) {
    std::string xname = "x_" + std::to_string(row[0]);
    RooRealVar x("x", xname.c_str(), -10, 80); // row[1], row[2]);
    RooRealVar mu("mu", "mu", row[8], -9, 80);//row[1], row[2]);
    RooRealVar gamma("gamma", "gamma", row[6], row[6], row[6]);
    RooRealVar lambda("lambda", "lambda", row[7], row[7], row[7]);
    RooRealVar delta("delta", "delta", row[4], row[4], row[4]);

    // Forward pulse timing model
    RooJohnson john("john", "john", x, mu, lambda, gamma, delta, 0);

    RooRealVar shift_minus_mu("shift_minus_mu", "shift_minus_mu", row[10], 0., 30.);
    RooRealVar coeff("coeff", "coeff", row[3], row[3], row[3]);
    RooRealVar w("w", "w", row[9], row[9], row[9]);
    RooRealVar frac("frac", "frac", row[5], 0.1, 1.);

    // Constants of fit
    gamma.setConstant(true);
    lambda.setConstant(true);
    delta.setConstant(true);
    shift_minus_mu.setConstant(true);
    coeff.setConstant(true);
    w.setConstant(true);

    RooFormulaVar shift("shift", "@0+@1", RooArgList(mu, shift_minus_mu));
    RooFormulaVar xshifted("xshifted", "@0-@1", RooArgList(x, shift));
    RooExponential exp("exp", "exp", xshifted, coeff);
    RooGenericPdf smooth_step("smooth_step", "1.0/(1.0 + exp((-@0)/@1))", RooArgList(xshifted, w));

    // Secondary pulse timing model
    RooProdPdf smoothedexp("smoothedexp", "exp * smooth_step", RooArgList(exp, smooth_step));

    // Signal pulse model
    RooAddPdf sgnModel("sgnModel", "signal model", john, smoothedexp, frac);

    RooRealVar slope("slope", "slope", 0);
    slope.setConstant(true);
    RooPolynomial bkgModel("bkgModel", "bkgModel", x, RooArgList(slope));

    RooRealVar sigPhotons("sigPhotons", "sigPhotons", 50, 0, 500);
    RooRealVar bkgPhotons("bkgPhotons", "bkgPhotons", 50, 0, 500);

    // Combined signal (dual pulse) with flat background noise
    std::string modelname = "model_" + std::to_string(row[0]);
    RooAddPdf model("model", modelname.c_str(), RooArgList(sgnModel, bkgModel), RooArgList(sigPhotons, bkgPhotons));

    std::string wsname = "ws_" + std::to_string(row[0]);
    RooWorkspace ws("ws", wsname.c_str());

    ws.import(model);

    m_wss.push_back(ws);
  }
}

int TOPBackSplashTimingModule::getModuleFromPhi(double phi)
{
  // Derive slot no. from azimuthal angle
  if (phi < 0) {
    phi += 2 * M_PI;
  }
  return int(phi / (M_PI / 8) + 1);
}

void TOPBackSplashTimingModule::makePlot(double cosTheta, double clusterE, int moduleID, RooAbsPdf* model,
                                         RooRealVar* x, RooDataSet data, RooFitResult* res, int nTracksPerSlot)
{
  // For debugging
  int cosThetaClusterIndex = int(std::round(cosTheta * 10));  // integer bin in [-6, 8]
  double nearestClusterCosTheta = cosThetaClusterIndex / 10.0;
  cosThetaClusterIndex += 6;  // shift to [0, 14]
  int binning = (m_fitparams[cosThetaClusterIndex][2] - m_fitparams[cosThetaClusterIndex][1]);

  // Create RooPlot
  TCanvas* c = new TCanvas("c", "Johnson SU fit", 900, 900);
  StoreObjPtr<EventMetaData> eventMetaData;
  std::string rooPlotTitle = "evtNo: " + std::to_string(eventMetaData->getEvent()) + " runNo.: " + std::to_string(
                               eventMetaData->getRun()) + " clusterE: " + std::to_string(
                               clusterE) + " slot: " + std::to_string(moduleID) + " fitted cosTheta: " + std::to_string(nearestClusterCosTheta) +
                             " no. of tracks in slot: " + std::to_string(nTracksPerSlot);

  RooPlot* frame = x->frame(RooFit::Title(rooPlotTitle.c_str()));
  data.plotOn(frame, RooFit::Binning(binning));
  model->plotOn(frame);

  model->plotOn(frame,
                RooFit::MoveToBack(),
                RooFit::Components("john"),
                RooFit::LineColor(kCyan));

  model->plotOn(frame,
                RooFit::MoveToBack(),
                RooFit::Components("smoothedexp"),
                RooFit::LineColor(kSpring));

  // reduced chi-square, 1 fit param
  double redchisq = frame->chiSquare(res->floatParsFinal().getSize());

  // Canvas
  frame->Draw();

  // Stat box
  TPaveText* box = new TPaveText(0.7, 0.65, 0.98, 0.92, "NDC");
  box->SetFillColor(0);
  box->SetBorderSize(1);

  // Header
  int nPhotons = data.numEntries();
  box->AddText(Form("nPhotons=%d", nPhotons));

  // Parameters
  double xpeak1 = 0;
  for (int i = 0; i < res->floatParsFinal().getSize(); ++i) {
    RooRealVar* p = (RooRealVar*)res->floatParsFinal().at(i);
    box->AddText(Form("%s = %.3f ± %.3f",
                      p->GetName(),
                      p->getVal(),
                      p->getError()));
    if ((std::string)p->GetName() == "mu") {
      xpeak1 = p->getVal();
    }
  }
  // Johnson mu (used as extracted time)
  TLine* xpeak1line = new TLine(xpeak1, 0, xpeak1, 1e6);
  xpeak1line->SetLineStyle(2);
  xpeak1line->SetLineColor(kRed);
  xpeak1line->SetLineWidth(5);
  xpeak1line->Draw();

  // Extra info
  box->AddText("Results");
  box->AddText(Form("forward peak: %.2f ns", xpeak1));
  box->AddText(Form("redchisq = %.4f", redchisq));

  box->SetTextSizePixels(50);
  box->Draw();

  // Save canvas
  std::string roofitname = "TOPBackSplashFit_evtNo_" + std::to_string(eventMetaData->getEvent()) + "_runNo_" + std::to_string(
                             eventMetaData->getRun()) + "_clusterE_" + std::to_string(
                             clusterE) + "_slot_" + std::to_string(moduleID) + "_cosTheta_" + std::to_string(
                             nearestClusterCosTheta) + ".png";
  c->SaveAs(roofitname.c_str());
}

TOPBackSplashFitResult* TOPBackSplashTimingModule::fitTimingDigits(int moduleID,
    std::vector<const TOPDigit*>& digitsPerSlot, double clusterE, double clusterCosTheta, int nTracksPerSlot)
{
  // Get pdf according to cosTheta
  int cosThetaIndex = int(std::round(clusterCosTheta * 10));  // integer bin in [-6, 8]
  cosThetaIndex += 6;  // shift to [0, 14]

  // Note edge case: 0.85 < cosTheta < 0.8572 (TOP acceptance) rounds to 0.9
  // covers ~31 to 31.79 degrees, omit these cases
  if (cosThetaIndex >= int(m_wss.size())) {
    return nullptr;
  }

  RooRealVar* x =  m_wss[cosThetaIndex].var("x");

  RooDataSet data("data", "unbinned", RooArgSet(*x));
  for (auto digit : digitsPerSlot) {
    double v = digit->getTime();
    x->setVal(v);
    data.add(RooArgSet(*x));
  }
  // Second check no. of photons (if enough digits in fit boundaries)
  if (data.numEntries() < m_minNphotons) {
    return nullptr;
  }

  double clusterSinTheta = TMath::Sqrt(1 - clusterCosTheta * clusterCosTheta);
  double zeta = 119 * (clusterCosTheta / clusterSinTheta) + 83.;

  //Range for the main peak position.
  // minTim calculated with beta = 1 and direct propagation in the bar (no reflections)
  // maxTime calculate for a p=200 MeV neutron with reflections in the bar
  double minTime =  120 / (30 * clusterSinTheta) + zeta / 20.;
  double maxTime =  120 / (3 * clusterSinTheta) + 1.4 * zeta / 20.;
  double shift = 2 * (270 - zeta) / 20;

  m_wss[cosThetaIndex].var("sigPhotons")->setVal(data.numEntries() * 0.8);
  m_wss[cosThetaIndex].var("sigPhotons")->setMin(data.numEntries() * 0.2);
  m_wss[cosThetaIndex].var("sigPhotons")->setMax(data.numEntries() * 1.1);

  m_wss[cosThetaIndex].var("bkgPhotons")->setVal(data.numEntries() * 0.2);
  m_wss[cosThetaIndex].var("bkgPhotons")->setMin(data.numEntries() * 0.);
  m_wss[cosThetaIndex].var("bkgPhotons")->setMax(data.numEntries() * 1.1);

  m_wss[cosThetaIndex].var("shift_minus_mu")->setVal(shift);
  m_wss[cosThetaIndex].var("shift_minus_mu")->setMin(0.);
  m_wss[cosThetaIndex].var("shift_minus_mu")->setMax(shift * 1.3);
  m_wss[cosThetaIndex].var("shift_minus_mu")->setConstant(false);
  m_wss[cosThetaIndex].var("mu")->setVal(0.5 * (minTime + maxTime));
  m_wss[cosThetaIndex].var("mu")->setMin(minTime);
  m_wss[cosThetaIndex].var("mu")->setMax(maxTime);

  RooAbsPdf* model =  m_wss[cosThetaIndex].pdf("model");
  RooFitResult* res = model->fitTo(data, RooFit::Save(), RooFit::PrintLevel(-1), RooFit::Strategy(0), RooFit::Extended());

  if (res->status() > 0) {
    return nullptr;
  }

  double res_time = m_wss[cosThetaIndex].var("mu")->getValV();
  double res_timeErr = m_wss[cosThetaIndex].var("mu")->getError();
  double res_deltaT = m_wss[cosThetaIndex].var("shift_minus_mu")->getValV();
  double res_deltaTErr = m_wss[cosThetaIndex].var("shift_minus_mu")->getError();
  double res_frac = m_wss[cosThetaIndex].var("frac")->getValV();
  double res_fracErr = m_wss[cosThetaIndex].var("frac")->getError();
  double res_signalPhotons = m_wss[cosThetaIndex].var("sigPhotons")->getValV();
  double res_signalPhotonsErr = m_wss[cosThetaIndex].var("sigPhotons")->getError();


  // Deriving chi2/dof: Need to bin but not Draw
  int binning = (m_fitparams[cosThetaIndex][2] - m_fitparams[cosThetaIndex][1]);
  RooPlot* frame = x->frame();
  data.plotOn(frame, RooFit::Binning(binning));
  model->plotOn(frame);
  int nfloatParsFinal = res->floatParsFinal().getSize();
  double redchisq = frame->chiSquare(nfloatParsFinal);

  if (m_saveFits == true) {
    makePlot(clusterCosTheta, clusterE, moduleID,  model, x, data, res, nTracksPerSlot);
  }

  // Saving results & plotting (time, chi2/dof, no. photons)
  if (m_saveMoreFitParams) {
    TOPBackSplashFitResult* fitresult = m_fitresult.appendNew(res_time, res_timeErr, res_deltaT, res_deltaTErr, res_frac, res_fracErr,
                                                              res_signalPhotons, res_signalPhotonsErr, redchisq, data.sumEntries(), nTracksPerSlot);
    return fitresult;
  } else {
    TOPBackSplashFitResult* fitresult = m_fitresult.appendNew(res_time, redchisq, data.sumEntries(), nTracksPerSlot);
    return fitresult;
  }

}


void TOPBackSplashTimingModule::initialize()
{
  B2INFO("TOPBackSplashTimingModule initialized");
  m_eclClusters.isRequired();
  m_digits.isRequired();
  m_tracks.isRequired();
  m_fitresult.registerInDataStore();
  m_fitresult.registerRelationTo(m_eclClusters);
  prepareFitModels();
}

void TOPBackSplashTimingModule::event()
{
  // Step 1: See which tracks can be matched to slots, ignore these slots
  // Even if only neutral clusters are being considered, the same slot
  // might be adjacent to an additional charged cluster, which will make be bkg
  // to anti-neutron TOP signal
  int nTracksPerSlots[16] = {0};
  for (const auto& track : m_tracks) {
    const auto* tl = track.getRelated<TOPLikelihood>();
    if (not tl) continue;
    const auto* te = tl->getRelated<ExtHit>();
    if (not te) continue;
    int slotID = te->getCopyID();
    // i.e. count how many charged track in slot
    nTracksPerSlots[slotID - 1]++;
  }

  // Step 2: Clean digitis, assign to slot
  std::array<std::vector<const TOPDigit*>, 16> digitsPerSlots;
  for (const auto& digi : m_digits) {
    if (digi.getHitQuality() != TOPDigit::c_Good) {
      continue;
    }
    if (digi.getTime() > 0 && digi.getTime() < 80) {
      digitsPerSlots[ int(digi.getModuleID()) - 1].push_back(&digi);
    }
  }
  // Step 3: Iterate over clusters with cleaning
  for (const auto& cluster : m_eclClusters) {

    // Clusters in barrel and within TOP acceptance,
    // Must have neutral hadron treatment of clusters only
    // No tracks matched to clusters
    // Min cluster energy, min clusterNHits (values passed to module)
    if (cluster.getTheta() > 31.0 * M_PI / 180.0 && cluster.getTheta() < 128.0 * M_PI / 180.0 &&
        cluster.getDetectorRegion() == 2 &&
        cluster.hasHypothesis(ECLCluster::EHypothesisBit::c_neutralHadron) &&
        cluster.isTrack() == false &&
        cluster.getEnergy(ECLCluster::EHypothesisBit::c_neutralHadron) >= m_minClusterE &&
        cluster.getNumberOfCrystals() >= m_minClusterNHits) {

      // Derive module in front of cluster
      double phi = cluster.getPhi();
      int moduleID = TOPBackSplashTimingModule::getModuleFromPhi(phi);
      if (m_includeSlotsWithTracks == false && nTracksPerSlots[moduleID - 1] > 0) {
        continue;
      }
      // minNphotons check: are there enough photons to fit?
      if (int(digitsPerSlots[moduleID - 1].size()) < m_minNphotons) {
        continue;
      }

      // For labelling plot file
      double clusterE = cluster.getEnergy(ECLCluster::EHypothesisBit::c_neutralHadron);

      // Step 4: Perform fit on digits nearest to cluster
      auto* fitresult = fitTimingDigits(moduleID, digitsPerSlots[moduleID - 1],
                                        clusterE, std::cos(cluster.getTheta()), nTracksPerSlots[moduleID - 1]);
      // Step 5: Setting up ECL relation to fit
      if (fitresult) {
        fitresult->addRelationTo(&cluster);
      }
    }
  }
}
