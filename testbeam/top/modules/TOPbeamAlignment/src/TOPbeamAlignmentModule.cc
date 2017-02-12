/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

// Own include
#include <testbeam/top/modules/TOPbeamAlignment/TOPbeamAlignmentModule.h>
#include <top/reconstruction/TOPreco.h>
#include <top/reconstruction/TOPtrack.h>
#include <top/reconstruction/TOPconfigure.h>
#include <top/geometry/TOPGeometryPar.h>



// framework - DataStore
#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/RelationArray.h>

// framework aux
#include <framework/gearbox/Unit.h>
#include <framework/gearbox/Const.h>
#include <framework/logging/Logger.h>

// DataStore classes
#include <framework/dataobjects/EventMetaData.h>
#include <top/dataobjects/TOPDigit.h>

#include <TVector3.h>
#include <boost/multi_array.hpp>

using namespace std;

namespace Belle2 {

  //-----------------------------------------------------------------
  //                 Register module
  //-----------------------------------------------------------------

  REG_MODULE(TOPbeamAlignment)

  //-----------------------------------------------------------------
  //                 Implementation
  //-----------------------------------------------------------------

  TOPbeamAlignmentModule::TOPbeamAlignmentModule() : Module()
  {
    // set module description (e.g. insert text)
    setDescription("Performs alignment of the beam using maximum likelihood scan");

    // Add parameters
    addParam("outputFileName", m_outputFileName, "Output file name",
             string("beamAlignment.root"));
    addParam("beamParticle", m_beamParticle, "beam particle: one of e, mu, pi, K, p",
             string("e"));
    addParam("beamMomentum", m_beamMomentum, "beam momentum [GeV/c]", 2.1);
    vector<double> defaultListDouble;
    addParam("scanY", m_scanY, "scan in y [cm]: central, step, np, a",
             defaultListDouble);
    addParam("scanZ", m_scanZ, "scan in z [cm]: central, step, np, a",
             defaultListDouble);
    addParam("scanTheta", m_scanTheta, "scan in theta [deg]: central, step, np, a",
             defaultListDouble);
    addParam("scanPhi", m_scanPhi, "scan in phi [deg]: central, step, np, a",
             defaultListDouble);
    addParam("scanT0", m_scanT0, "scan in t0: np (half number of points)", 20);
    addParam("tMin", m_tMin, "PDF histogram: minimal t [ns]", 0.0);
    addParam("tMax", m_tMax, "PDF histogram: maximal t [ns]", 50.0);
    addParam("numBins", m_numBins, "PDF histogram: number of bins in t", 1000);
    addParam("numIterations", m_numIterations, "number of iterations", 1);
    addParam("parabolicMaximum", m_parabolicMaximum, "calculate paraboloc maximum",
             false);
    addParam("minBkgPerBar", m_minBkgPerBar,
             "Minimal number of background photons per bar", 0.0);
    addParam("electronicJitter", m_electronicJitter,
             "r.m.s of electronic jitter [ns] - not used!", 50e-3);

    m_mass = 0;
    m_x0 = 0;  // by definition!
    m_y0 = 0;
    m_z0 = 0;
    m_theta = 0;
    m_phi = 0;
    m_t0 = 0;
    m_numChannels = 0;
    m_numPMTrows = 0;
    m_numPMTcols = 0;
    m_numPADrows = 0;
    m_numPADcols = 0;
    m_numEvents = 0;

    m_file = NULL;
    m_ringImage = NULL;
    m_pdfInitial = NULL;
    m_pdfAligned = NULL;

  }

  TOPbeamAlignmentModule::~TOPbeamAlignmentModule()
  {
  }

  void TOPbeamAlignmentModule::initialize()
  {

    // particle mass
    m_mass = 0;
    if (m_beamParticle == string("e")) m_mass = Const::electron.getMass();
    if (m_beamParticle == string("mu")) m_mass = Const::muon.getMass();
    if (m_beamParticle == string("pi")) m_mass = Const::pion.getMass();
    if (m_beamParticle == string("K")) m_mass = Const::kaon.getMass();
    if (m_beamParticle == string("p")) m_mass = Const::proton.getMass();
    if (m_mass == 0) B2ERROR("unknown charged stable particle: " << m_beamParticle);

    Scan scanY(m_scanY, "scanY");
    m_y0 = scanY.getCenter();

    Scan scanZ(m_scanZ, "scanZ");
    m_z0 = scanZ.getCenter();

    Scan scanTheta(m_scanTheta, "scanTheta");
    m_theta = scanTheta.getCenter();

    Scan scanPhi(m_scanPhi, "scanPhi");
    m_phi = scanPhi.getCenter();

    m_t0 = 0;

    const auto* geo = TOP::TOPGeometryPar::Instance()->getGeometry();
    const auto& pmtArray = geo->getModule(1).getPMTArray();
    m_numPMTrows = pmtArray.getNumRows();
    m_numPMTcols = pmtArray.getNumColumns();
    m_numPADrows = pmtArray.getPMT().getNumRows();
    m_numPADcols = pmtArray.getPMT().getNumColumns();
    m_numChannels = m_numPMTrows * m_numPMTcols * m_numPADrows * m_numPADcols;
    m_numEvents = 0;

    if (m_outputFileName.empty()) B2ERROR("TOPbeamAlignment: no output file name given");
    m_file = new TFile(m_outputFileName.c_str(), "RECREATE");

    m_ringImage = new TH2F("ringImage", "Ring image", m_numChannels, 0, m_numChannels,
                           m_numBins, m_tMin, m_tMax);
    m_pdfInitial = new TH2F("pdfInitial", "PDF before alignment",
                            m_numChannels, 0, m_numChannels,
                            m_numBins, m_tMin, m_tMax);
    m_pdfAligned = new TH2F("pdfAligned", "PDF after alignment",
                            m_numChannels, 0, m_numChannels,
                            m_numBins, m_tMin, m_tMax);

    TOP::TOPconfigure config;
    config.print();

    for (int ich = 0; ich < m_numChannels; ich++) {
      m_rowWiseChannelID.push_back(rowWiseChannelID(ich));
    }

  }

  void TOPbeamAlignmentModule::beginRun()
  {
  }

  void TOPbeamAlignmentModule::event()
  {
    // fill ring image histogram

    StoreArray<TOPDigit> topDigits;
    int nHits = topDigits.getEntries();
    const auto* geo = TOP::TOPGeometryPar::Instance()->getGeometry();
    for (int i = 0; i < nHits; ++i) {
      TOPDigit* data = topDigits[i];
      m_ringImage->Fill(m_rowWiseChannelID[data->getPixelID() - 1],
                        geo->getNominalTDC().getTime(data->getTDC()));
    }
    m_numEvents++;

  }


  void TOPbeamAlignmentModule::endRun()
  {
  }

  void TOPbeamAlignmentModule::terminate()
  {
    // do beam alignment
    beamAlignment();

    // write histograms and close file
    m_file->cd();
    m_ringImage->Write();
    m_pdfInitial->Write();
    m_pdfAligned->Write();
    for (unsigned i = 0; i < m_scanHistograms.size(); i++) m_scanHistograms[i]->Write();
    m_file->Close();
  }

  void TOPbeamAlignmentModule::printModuleParams() const
  {
  }


  //---------- private methods -----------------------------------

  void TOPbeamAlignmentModule::beamAlignment()
  {
    // create reconstruction object

    double mass[1] = {m_mass};
    TOP::TOPreco reco(1, mass, m_minBkgPerBar);
    reco.clearData();

    // fill initial PDF
    fillPDF(reco, m_pdfInitial);

    // find best alignment by likelihood scan
    likelihoodScan(reco);

    // fill aligned PDF
    fillPDF(reco, m_pdfAligned);

  }


  void TOPbeamAlignmentModule::fillPDF(TOP::TOPreco& reco, TH2F* histogram)
  {
    TVector3 momentum;
    momentum.SetMagThetaPhi(m_beamMomentum, m_theta * Unit::deg, m_phi * Unit::deg);
    TOP::TOPtrack track(m_x0, m_y0, m_z0, momentum.Px(), momentum.Py(), momentum.Pz(),
                        0.0, 1);
    reco.reconstruct(track);
    if (reco.getFlag() != 1) {
      B2WARNING("beamAlignment::fillPDF: reco flag = false");
      return;
    }
    double LogL[1], ExpPhot[1];
    int Nphot;
    reco.getLogL(1, LogL, ExpPhot, Nphot);

    Scan time(m_numBins, m_tMin, m_tMax);
    double norm = m_numEvents * ExpPhot[0] * time.step;

    for (int ich = 0; ich < m_numChannels; ich++) {
      for (int i = 0; i < time.nPoints; i++) {
        double t = time.getPoint(i);
        double pdf = norm * reco.getPDF(ich, t, m_mass);
        histogram->Fill(m_rowWiseChannelID[ich], t - m_t0, pdf);
      }
    }
  }


  void TOPbeamAlignmentModule::likelihoodScan(TOP::TOPreco& reco)
  {
    cout << "beam alignment: initial" << endl;
    cout << "  --> y = " << m_y0 << " z = " << m_z0 << " theta = " << m_theta
         << " phi = " << m_phi << " t0 = " << m_t0 << endl;

    for (int iter = 0; iter < m_numIterations; iter++) {
      // theta - z
      Scan scanZ(m_scanZ, string("z"));
      Scan scanTheta(m_scanTheta, string("theta"));
      findMaximumLikelihood(reco, scanTheta, scanZ, m_theta, m_z0, iter);
      cout << "  --> y = " << m_y0 << " z = " << m_z0 << " theta = " << m_theta
           << " phi = " << m_phi << " t0 = " << m_t0 << endl;

      // phi - y
      Scan scanY(m_scanY, string("y"));
      Scan scanPhi(m_scanPhi, string("phi"));
      if (scanY.nPoints == 1 && scanPhi.nPoints == 1) break;
      findMaximumLikelihood(reco, scanPhi, scanY, m_phi, m_y0, iter);
      cout << "  --> y = " << m_y0 << " z = " << m_z0 << " theta = " << m_theta
           << " phi = " << m_phi << " t0 = " << m_t0 << endl;
    }
  }


  void TOPbeamAlignmentModule::findMaximumLikelihood(TOP::TOPreco& reco,
                                                     const Scan& scanX,
                                                     const Scan& scanY,
                                                     double& X, double& Y,
                                                     int iter)
  {
    cout << "beam alignment: find maximum in " << scanY.name << "-" << scanX.name
         << " plane, iteration " << iter << endl;

    // fill 2D array of log Likelihoods

    //    double logL[scanX.nPoints][scanY.nPoints];
    //    double T0[scanX.nPoints][scanY.nPoints];
    boost::multi_array<double, 2> logL(boost::extents[scanX.nPoints][scanY.nPoints]);
    boost::multi_array<double, 2> T0(boost::extents[scanX.nPoints][scanY.nPoints]);
    for (int i = 0; i < scanX.nPoints; i++) {
      for (int j = 0; j < scanY.nPoints; j++) {
        X = scanX.getPoint(i);
        Y = scanY.getPoint(j);
        logL[i][j] = getLogLikelihood(reco);
        T0[i][j] = m_t0;
      }
    }

    // find maximum

    int i0 = 0;
    int j0 = 0;
    double maxlogL = logL[i0][j0];
    for (int i = 0; i < scanX.nPoints; i++) {
      for (int j = 0; j < scanY.nPoints; j++) {
        if (logL[i][j] > maxlogL) {
          i0 = i; j0 = j; maxlogL = logL[i][j];
        }
      }
    }
    if (scanX.nPoints > 1) {
      if (i0 == 0) B2WARNING(scanX.name << ": maximum found at lower border");
      if (i0 == scanX.nPoints - 1) B2WARNING(scanX.name << ": maximum found at upper border");
    }
    if (scanY.nPoints > 1) {
      if (j0 == 0) B2WARNING(scanY.name << ": maximum found at lower border");
      if (j0 == scanY.nPoints - 1) B2WARNING(scanY.name << ": maximum found at upper border");
    }

    // calculate parabolic maximum and return results

    double x = 0;
    if (m_parabolicMaximum && scanX.nPoints > 2)
      x = getParabolicMaximum(logL[i0 - 1][j0], logL[i0][j0], logL[i0 + 1][j0]);
    X = scanX.getPoint(i0) + x * scanX.step;

    double y = 0;
    if (m_parabolicMaximum && scanY.nPoints > 2)
      y = getParabolicMaximum(logL[i0][j0 - 1], logL[i0][j0], logL[i0][j0 + 1]);
    Y = scanY.getPoint(j0) + y * scanY.step;

    m_t0 = T0[i0][j0];

    // save arrays into histograms

    string iteration = numberToString(iter);
    string name = string("logL_") + scanY.name + string("_") + scanX.name
                  + string("_") + iteration;
    string title = string("log likelihoods ") + scanY.name + string(" vs. ") +
                   scanX.name + string(", iteration ") + iteration + string(": ") +
                   scanX.name + string(" = ") + numberToString(X) + string(" ") +
                   scanY.name + string(" = ") + numberToString(Y);
    TH2F* hist1 = new TH2F(name.c_str(), title.c_str(),
                           scanX.nPoints, scanX.xmin, scanX.xmax,
                           scanY.nPoints, scanY.xmin, scanY.xmax);

    for (int i = 0; i < scanX.nPoints; i++) {
      for (int j = 0; j < scanY.nPoints; j++) {
        hist1->SetBinContent(i + 1, j + 1, logL[i][j] - maxlogL);
      }
    }
    m_scanHistograms.push_back(hist1);

    name = string("T0_") + scanY.name + string("_") + scanX.name
           + string("_") + iteration;
    title = string("common offset time ") + scanY.name + string(" vs. ") +
            scanX.name + string(", iteration ") + iteration + string(": t0 = ") +
            numberToString(m_t0);
    TH2F* hist2 = new TH2F(name.c_str(), title.c_str(),
                           scanX.nPoints, scanX.xmin, scanX.xmax,
                           scanY.nPoints, scanY.xmin, scanY.xmax);

    for (int i = 0; i < scanX.nPoints; i++) {
      for (int j = 0; j < scanY.nPoints; j++) {
        hist2->SetBinContent(i + 1, j + 1, T0[i][j]);
      }
    }
    m_scanHistograms.push_back(hist2);

  }


  double TOPbeamAlignmentModule::getLogLikelihood(TOP::TOPreco& reco)
  {
    // set track

    TVector3 momentum;
    momentum.SetMagThetaPhi(m_beamMomentum, m_theta * Unit::deg, m_phi * Unit::deg);
    TOP::TOPtrack track(m_x0, m_y0, m_z0, momentum.Px(), momentum.Py(), momentum.Pz(),
                        0.0, 1);

    // call reconstruction

    reco.reconstruct(track);
    if (reco.getFlag() != 1) {
      B2WARNING("beamAlignment::fillPDF: reco flag = false");
      return -1e30;
    }

    // make PDF

    Scan time(m_numBins, m_tMin, m_tMax, m_scanT0);
    //    double pdf[m_numChannels][time.nPoints];
    boost::multi_array<double, 2> pdf(boost::extents[m_numChannels][time.nPoints]);
    for (int ich = 0; ich < m_numChannels; ich++) {
      for (int i = 0; i < time.nPoints; i++) {
        double t = time.getPoint(i);
        pdf[ich][i] = reco.getPDF(ich, t, m_mass);
      }
    }

    // calculate normalizations

    Scan scanT0(m_scanT0, time.step);
    std::vector<double> norm(scanT0.nPoints);

    for (int k = 0; k < scanT0.nPoints; k++) {
      norm[k] = 0;
      for (int ich = 0; ich < m_numChannels; ich++) {
        for (int i = 0; i < m_numBins; i++) {
          norm[k] += pdf[ich][i + k];
        }
      }
      if (norm[k] == 0) B2FATAL("getLogLikelihood: norm = 0 (at k=" << k << ")");
      norm[k] = log(norm[k]);
    }

    // switch to log

    for (int ich = 0; ich < m_numChannels; ich++) {
      for (int i = 0; i < time.nPoints; i++) {
        pdf[ich][i] = log(pdf[ich][i]);
      }
    }

    // calculate log likelihood

    std::vector<double> logL(scanT0.nPoints);

    for (int k = 0; k < scanT0.nPoints; k++) {
      logL[k] = 0;
      for (int ich = 0; ich < m_numChannels; ich++) {
        int chan = m_rowWiseChannelID[ich];
        for (int i = 0; i < m_numBins; i++) {
          logL[k] += m_ringImage->GetBinContent(chan + 1, i + 1) *
                     (pdf[ich][i + k] - norm[k]);
        }
      }
    }

    // find maximum

    int k0 = 0;
    double maxL = logL[k0];
    for (int k = 1; k < scanT0.nPoints; k++) {
      if (logL[k] > maxL) {maxL = logL[k]; k0 = k;}
    }
    m_t0 = scanT0.getPoint(k0);

    if (scanT0.nPoints < 3) return maxL;

    // find parabolic maximum by interpolation

    if (k0 == 0) {
      B2WARNING("scan T0: maximum found at lower border");
      return maxL;
    }
    if (k0 == scanT0.nPoints - 1) {
      B2WARNING("scan T0: maximum found at upper border");
      return maxL;
    }

    double x = getParabolicMaximum(logL[k0 - 1], logL[k0], logL[k0 + 1]);
    m_t0 = scanT0.getPoint(k0) + x * scanT0.step;
    maxL = getLogLikelihood(reco, m_t0);
    return maxL;
  }


  double TOPbeamAlignmentModule::getLogLikelihood(TOP::TOPreco& reco, double t0)
  {
    Scan time(m_numBins, m_tMin, m_tMax);
    //    double pdf[m_numChannels][time.nPoints];
    boost::multi_array<double, 2> pdf(boost::extents[m_numChannels][time.nPoints]);
    double norm = 0;
    for (int ich = 0; ich < m_numChannels; ich++) {
      for (int i = 0; i < time.nPoints; i++) {
        double t = time.getPoint(i);
        pdf[ich][i] = reco.getPDF(ich, t + t0, m_mass); // shifts back in time
        norm += pdf[ich][i];
      }
    }

    if (norm == 0) B2FATAL("getLogLikelihood: norm = 0");

    // calculate log likelihood

    double logL = 0;
    for (int ich = 0; ich < m_numChannels; ich++) {
      int chan = m_rowWiseChannelID[ich];
      for (int i = 0; i < time.nPoints; i++) {
        logL += m_ringImage->GetBinContent(chan + 1, i + 1) * log(pdf[ich][i] / norm);
      }
    }

    return logL;
  }


  double TOPbeamAlignmentModule::getParabolicMaximum(double yLeft,
                                                     double yCenter,
                                                     double yRight)
  {
    double D21 = yCenter - yLeft;
    double D32 = yRight - yCenter;
    double A = (D32 - D21) / 2;
    if (A == 0) return 0;
    double B = (D32 + D21) / 2;
    return - B / 2 / A;
  }


} // end Belle2 namespace


