#include <Math/Vector3D.h>
#include <framework/dataobjects/EventMetaData.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/logging/Logger.h>
#include <map>
#include <math.h>
#include <string>
#include <top/modules/TOPBackSplashTiming/TOPBackSplashTimingModule.h>

#include <RooAbsPdf.h>
#include <RooAddPdf.h>
#include <RooArgList.h>
#include <RooArgSet.h>
#include <RooDataSet.h>
#include <RooExponential.h>
#include <RooFitResult.h>
#include <RooFormulaVar.h>
#include <RooGenericPdf.h>
#include <RooJohnson.h>
#include <RooPlot.h>
#include <RooProdPdf.h>
#include <RooRealVar.h>
#include <TCanvas.h>
#include <TF1.h>
#include <TH1F.h>
#include <TLine.h>
#include <TPad.h>
#include <TPaveText.h>

using namespace Belle2;

// Register module so basf2 can find it
REG_MODULE(TOPBackSplashTiming);

TOPBackSplashTimingModule::TOPBackSplashTimingModule()
  : Module(),
    m_eclClusters("ECLClusters"),   // DataStore name
    m_digits("TOPDigits"),
    m_MCParticles("MCParticles"),
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
  setDescription("Print energy of all ECL clusters");
}

std::vector<RooWorkspace> TOPBackSplashTimingModule::prepareFitModels()
{
  std::vector<RooWorkspace> wss;
  for (auto row : m_fitparams) {
    std::string xname = "x_" + std::to_string(row[0]);
    RooRealVar x("x", xname.c_str(), row[1], row[2]);
    RooRealVar mu("mu", "mu", row[8], row[1], row[2]);
    RooRealVar gamma("gamma", "gamma", row[6], row[6], row[6]);
    RooRealVar lambda("lambda", "lambda", row[7], row[7], row[7]);
    RooRealVar delta("delta", "delta", row[4], row[4], row[4]);

    // Forward pulse timing model
    RooJohnson john("john", "john", x, mu, lambda, gamma, delta, 0);

    RooRealVar shift_minus_mu("shift_minus_mu", "shift_minus_mu", row[10], row[10], row[10]);
    RooRealVar coeff("coeff", "coeff", row[3], row[3], row[3]);
    RooRealVar w("w", "w", row[9], row[9], row[9]);
    RooRealVar frac("frac", "frac", row[5], row[5], row[5]);

    RooFormulaVar shift("shift", "@0+@1", RooArgList(mu, shift_minus_mu));
    RooFormulaVar xshifted("xshifted", "@0-@1", RooArgList(x, shift));
    RooExponential exp("exp", "exp", xshifted, coeff);
    RooGenericPdf smooth_step("smooth_step", "1.0/(1.0 + exp((-@0)/@1))", RooArgList(xshifted, w));

    // Secondary pulse timing model
    RooProdPdf smoothedexp("smoothedexp", "exp * smooth_step", RooArgList(exp, smooth_step));

    // Combined model
    std::string modelname = "model_" + std::to_string(row[0]);
    RooAddPdf model("model", modelname.c_str(), john, smoothedexp, frac);

    std::string wsname = "ws_" + std::to_string(row[0]);
    RooWorkspace ws("ws", wsname.c_str());

    ws.import(model);

    wss.push_back(ws);
  }

  return wss;
}

int TOPBackSplashTimingModule::getModuleFromPhi(double phi)
{
  // Derive slot no. from azimuthal angle
  if (phi < 0) {
    phi += 2 * M_PI;
  }
  return int(phi / (M_PI / 8) + 1);
}

//void TOPBackSplashTimingModule::makePlot()
//{
//  // For debugging
//  int binning = (nfin - nin) / 2;
//
//  // Create RooPlot
//  std::string rooPlotTitle = "evtNo: " + std::to_string(eventMetaData->getEvent()) + " clusterNo: " + std::to_string(
//                               iClusterToFit) + " slot: " + std::to_string(moduleIDindex + 1) + " fitted cosTheta: " + std::to_string(
//                               nearestClusterCosTheta) + " isTrack: " + std::to_string(isTrack);
//
//  RooPlot* frame = x.frame(RooFit::Title(rooPlotTitle.c_str()));
//  data.plotOn(frame, RooFit::Binning(binning));
//
//  model.plotOn(frame,
//               RooFit::Components(john),
//               RooFit::LineColor(kCyan),
//               RooFit::LineStyle(kDashed));
//
//  model.plotOn(frame,
//               RooFit::Components(newexp),
//               RooFit::LineColor(kSpring),
//               RooFit::LineStyle(kDashed));
//
//  model.plotOn(frame, RooFit::Range("fit"));
//
//  // Reduced chi-square
//  double redchisq = frame->chiSquare(res->floatParsFinal().getSize());
//
//  // Convert PDFs to TF1
//  TF1* tf1 = john.asTF(RooArgList(x));
//  TF1* tf2 = newexp.asTF(RooArgList(x));
//
//  // Peak finding
//  double peak1x = tf1->GetMaximumX(5, 15);
//  double peak1y = tf1->GetMaximum(5, 15);
//  double halfpeak1x = tf1->GetX(peak1y / 2.0, 5, peak1x);
//
//  double peak2x = tf2->GetMaximumX(25, 40);
//  double peak2y = tf2->GetMaximum(25, 40);
//  double halfpeak2x = tf2->GetX(peak2y / 2.0, 25, peak2x);
//
//  //// Pulls
//  //RooHist* pulls = frame->pullHist();
//  //RooPlot* pull_frame = x.frame(RooFit::Title(" "));
//  //pull_frame->addPlotable((RooPlotable*)pulls, "P");
//
//  // Guide lines
//  TLine* l0 = new TLine(x.getMin(), 0, x.getMax(), 0);
//  l0->SetLineStyle(2);
//
//  TLine* l3u = new TLine(x.getMin(), 3, x.getMax(), 3);
//  l3u->SetLineStyle(2);
//  l3u->SetLineColor(kRed);
//
//  TLine* l3d = new TLine(x.getMin(), -3, x.getMax(), -3);
//  l3d->SetLineStyle(2);
//  l3d->SetLineColor(kRed);
//
//  // Canvas
//  TCanvas* c = new TCanvas("c", "Johnson SU fit", 900, 900);
//
//  // Upper pad (main plot)
//  TPad* pad1 = new TPad("pad1", "pad1", 0, 0.30, 0.7, 1);
//  pad1->SetBottomMargin(0.02);
//  pad1->SetRightMargin(0.02);
//  pad1->Draw();
//  pad1->cd();
//
//  frame->Draw();
//
//  // Half-maximum lines
//  TLine* halfpeak1xline = new TLine(halfpeak1x, 0, halfpeak1x, 1e6);
//  halfpeak1xline->SetLineStyle(2);
//  halfpeak1xline->SetLineColor(kRed);
//  halfpeak1xline->SetLineWidth(5);
//  halfpeak1xline->Draw();
//
//  TLine* halfpeak2xline = new TLine(halfpeak2x, 0, halfpeak2x, 1e6);
//  halfpeak2xline->SetLineStyle(2);
//  halfpeak2xline->SetLineColor(kRed);
//  halfpeak2xline->SetLineWidth(5);
//  halfpeak2xline->Draw();
//
//  c->cd();
//
//  //// Lower pad (pulls)
//  //TPad* pad2 = new TPad("pad2", "pad2", 0, 0.00, 0.7, 0.30);
//  //pad2->SetTopMargin(0);
//  //pad2->SetBottomMargin(0.30);
//  //pad2->SetRightMargin(0.02);
//  //pad2->Draw();
//  //pad2->cd();
//  //
//  //pull_frame->SetXTitle("Calibrated calibrated time of Cherenkov photon detection (ns)");
//  //pull_frame->SetYTitle("Pulls");
//  //
//  //pull_frame->GetXaxis()->SetTitleSize(0.08);
//  //pull_frame->GetYaxis()->SetTitleSize(0.08);
//  //pull_frame->GetYaxis()->SetLabelSize(0.08);
//  //pull_frame->GetXaxis()->SetLabelSize(0.08);
//  //pull_frame->GetYaxis()->SetTitleOffset(0.5);
//  //
//  //pull_frame->Draw();
//  l0->Draw("same");
//  l3u->Draw("same");
//  l3d->Draw("same");
//
//  // Stat box
//  c->cd();
//
//  TPaveText* box = new TPaveText(0.7, 0.65, 0.98, 0.92, "NDC");
//  box->SetFillColor(0);
//  box->SetBorderSize(1);
//
//  // Header
//  int nPhotons = digitIndiciesInSlot.size();
//  box->AddText(Form("nPhotons=%d", nPhotons));
//
//  // Parameters
//  for (int i = 0; i < res->floatParsFinal().getSize(); ++i) {
//    RooRealVar* p = (RooRealVar*)res->floatParsFinal().at(i);
//    box->AddText(Form("%s = %.3f ± %.3f",
//                      p->GetName(),
//                      p->getVal(),
//                      p->getError()));
//  }
//
//  // Extra info
//  box->AddText("Rising edges");
//  box->AddText(Form("forward peak: %.2f ns", halfpeak1x));
//
//  if (peak1x < peak2x) {
//    box->AddText(Form("reflected peak: %.2f ns", halfpeak2x));
//  }
//
//  box->AddText(Form("redchisq = %.4f", redchisq));
//
//  box->SetTextSizePixels(50);
//  box->Draw();
//
//  // Save canvas
//  std::string roofitname = "fit_evtNo_" + std::to_string(eventMetaData->getEvent()) + "_clusterNo_" + std::to_string(
//                             iClusterToFit) + "_slot_" + std::to_string(moduleIDindex + 1) + "_cosTheta_" + std::to_string(
//                             nearestClusterCosTheta) + "_isTrack_" + std::to_string(isTrack) + ".png";
//  c->SaveAs(roofitname.c_str());
//  return;
//}

//void TOPBackSplashTimingModule::getMCNbar()
//{
//  for (const auto& part : m_MCParticles) {
//    if (part.getPDG() == int(-2112)) {
//      ROOT::Math::XYZVector nbar_mom = part.getMomentum();
//      float nbar_phi = nbar_mom.Phi();
//      float nbar_theta = nbar_mom.Theta();
//      ROOT::Math::XYZVector decayVertex = part.getDecayVertex();
//      float decayRho = decayVertex.Rho();
//
//      int nbar_slot;
//      if (nbar_phi > 0) { // nbar_phi will be between -pi and pi
//        nbar_slot = int(std::floor(nbar_phi / (2 * M_PI / 16)) + 1);
//      } else {
//        nbar_slot = int(std::floor((nbar_phi + 2 * M_PI) / (2 * M_PI / 16)) + 1);
//      }
//      float nbarCosTheta = std::cos(nbar_theta);
//      float nearestNbarCosTheta = std::round(nbarCosTheta * 10) / 10; //i.e. round to 0.1
//      B2INFO("Anti-neutron has theta / phi  / slot / nearest cosTheta / transverse dist:" << nbar_theta << " / " << nbar_phi << " / " <<
//             nbar_slot << " / " << nearestNbarCosTheta << " / " << decayRho);
//    }
//  }
//  return;
//}

int TOPBackSplashTimingModule::convertCosThetaToIndex(float nearestClusterCosTheta)
{
  int cosThetaIndex = int(nearestClusterCosTheta * 10) + 6;
  return cosThetaIndex;
}

void TOPBackSplashTimingModule::fitTimingDigits(TOPBackSplashFitResult* fitresult, int moduleIDindex,
                                                std::vector<int> digitIndiciesInSlot,
                                                float nearestClusterCosTheta, int iClusterToFit)
{
  // Get cosTheta pdf
  int cosThetaIndex = convertCosThetaToIndex(nearestClusterCosTheta);
  RooRealVar* x =  m_wss[cosThetaIndex].var("x");
  RooAbsPdf* model =  m_wss[cosThetaIndex].pdf("model");

  //if (!x) {
  //    B2FATAL("RooRealVar x not found in workspace!");
  //}
  //if (!model) {
  //    B2FATAL("Model PDF not found!");
  //}
  //if (cosThetaIndex < 0 || cosThetaIndex >= m_wss.size()) {
  //B2FATAL("Invalid cosThetaIndex!");
  //}
  // TODO Get RooRealVar from cdb
  RooDataSet data("data", "unbinned", RooArgSet(*x));
  for (auto digitIndexInSlot : digitIndiciesInSlot) {
    double v = m_digits[digitIndexInSlot]->getTime();
    if (v >= m_fitparams[cosThetaIndex][1] && v <= m_fitparams[cosThetaIndex][2]) {
      x->setVal(v);
      data.add(RooArgSet(*x));
    }
  }
  B2INFO("Dataset entries: " << data.numEntries());
  // TODO: contniue if dataset is empty?

  //StoreObjPtr<EventMetaData> eventMetaData;
  RooFitResult* res = model->fitTo(data, RooFit::Save());//, RooFit::Range("fit"));

  //B2INFO("evtno: " << eventMetaData->getEvent() << " cluster no.: " << iClusterToFit << " slot: " << moduleIDindex + 1 << " nDigits: "
  //       << digitIndiciesInSlot.size() << " fitted cosTheta: " << nearestClusterCosTheta << " ith cluster: " << iClusterToFit);
  //for (int i = 0; i < res->floatParsFinal().getSize(); i++) {
  //    auto & p= (RooRealVar &) res->floatParsFinal()[i];
  //    B2INFO("param: " << p.GetName()<<".");
  //    B2INFO(typeid(p.GetName()).name());
  //    if ( std::string(p.GetName()) == "mu"){
  //        double mu_val = p.getVal();
  //        fitresult->setTime(mu_val);
  //        B2INFO("mu found");
  //    }
  //}

  //if (!john_arg || !smoothedexp_arg) {
  //    B2ERROR("Could not find PDF components 'john' or 'smoothedexp'");
  //    return;
  //}


  RooAbsArg* john_arg = model->getComponents()->find("john");
  //RooAbsArg* smoothedexp_arg= model->getComponents()->find("smoothedexp");
  RooAbsPdf* peak1_fit = static_cast<RooAbsPdf*>(john_arg);
  //RooAbsPdf* peak2_fit = static_cast<RooAbsPdf*>(smoothedexp_arg);
  //RooRealVar* x_model = m_wss[cosThetaIndex].var("x");
  //TF1* peak1_tf1 = peak1_fit->asTF(RooArgList(*x_model));
  TF1* peak1_tf1 = peak1_fit->asTF(RooArgList(*x));
  //TF1* peak2_tf1= peak2_fit->asTF(RooArgList(*x_model));
  double peak1_max = peak1_tf1->GetMaximum(x->getMin(), x->getMax());
  double peak1_rising_edge = peak1_tf1->GetX(0.5 * peak1_max, x->getMin(), x->getMax());

  fitresult->setTime(peak1_rising_edge);
  B2INFO("cosTheta " << cosThetaIndex << "init: " << m_fitparams[cosThetaIndex][8]);
  fitresult->setChisqdof(0); //TODO add chiw
  fitresult->setNphotons(data.sumEntries());
  return;
}



void TOPBackSplashTimingModule::initialize()
{
  B2INFO("TOPBackSplashTimingModule initialized");
  m_eclClusters.isRequired();
  m_digits.isRequired();
  m_fitresult.registerInDataStore();
  m_fitresult.registerRelationTo(m_eclClusters);
  m_wss = prepareFitModels();
}

void TOPBackSplashTimingModule::event()
{
  // 1st: Sort digit indicies into slots (TODO pointer or indicies)
  std::array<std::vector<int>, 16> digitIndiciesPerSlots;
  int nCleanDigits = 0;
  for (int i = 0; i < m_digits.getEntries(); i++) {
    const TOPDigit* digi = m_digits[i];
    if (digi->getTime() > 0) { //digitQuality cut??
      digitIndiciesPerSlots[ int(digi->getModuleID()) - 1].push_back(i);
      nCleanDigits++;
    }
  }

  // 2nd: Iterate over clusters
  int nClustersToFit = 0;
  for (int iCluster = 0; iCluster < m_eclClusters.getEntries(); iCluster++) {
    const ECLCluster*  cluster = m_eclClusters[iCluster];

    //Barrel and neutral hadron treatment of clusters only, with gt 100 MeV, no tracks
    if (cluster->getDetectorRegion() == 2 && cluster->hasHypothesis(ECLCluster::EHypothesisBit::c_neutralHadron)
        && cluster->getEnergy(ECLCluster::EHypothesisBit::c_neutralHadron) > 0.1 && cluster-> isTrack() == false) {

      // Derive module in front of cluster
      double phi = cluster->getPhi();
      int moduleID = TOPBackSplashTimingModule::getModuleFromPhi(phi); // Put this func back

      if (digitIndiciesPerSlots[moduleID - 1].size() == 0) {
        continue;
      }

      nClustersToFit++;
      float nearestClusterCosTheta = (float)std::round(std::cos(cluster->getTheta()) * 10) / 10;

      // 3rd: Fit clusters
      TOPBackSplashFitResult* fitresult = m_fitresult.appendNew();
      fitTimingDigits(fitresult, moduleID - 1, digitIndiciesPerSlots[moduleID - 1], nearestClusterCosTheta, nClustersToFit);

      //// 4th: Setting up ECL relation to fit
      fitresult->addRelationTo(cluster);
    }
  }
}
