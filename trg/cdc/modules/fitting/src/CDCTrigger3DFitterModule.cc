#include "trg/cdc/modules/fitting/CDCTrigger3DFitterModule.h"

#include <framework/datastore/RelationVector.h>

#include <cdc/geometry/CDCGeometryPar.h>
#include <trg/cdc/Fitter3DUtility.h>
#include <trg/cdc/Fitter3D.h>
#include <trg/cdc/JSignal.h>
#include <trg/cdc/JSignalData.h>

using namespace std;
using namespace Belle2;

//this line registers the module with the framework and actually makes it available
//in steering files or the the module list (basf2 -m).
REG_MODULE(CDCTrigger3DFitter);

CDCTrigger3DFitterModule::CDCTrigger3DFitterModule() : Module::Module()
{
  setDescription(
    "The 3D fitter module of the CDC trigger.\n"
    "Selects stereo hits around a given 2D track and performs a linear fit "
    "in the s-z plane (s: 2D arclength).\n"
  );
  setPropertyFlags(c_ParallelProcessingCertified);
  addParam("hitCollectionName", m_hitCollectionName,
           "Name of the input StoreArray of CDCTriggerSegmentHits.",
           string(""));
  addParam("EventTimeName", m_EventTimeName,
           "Name of the event time object.",
           string(""));
  addParam("inputCollectionName", m_inputCollectionName,
           "Name of the StoreArray holding the input tracks from the 2D fitter.",
           string("TRGCDC2DFitterTracks"));
  addParam("outputCollectionName", m_outputCollectionName,
           "Name of the StoreArray holding the 3D output tracks.",
           string("TRGCDC3DFitterTracks"));
  addParam("fitterMode", m_fitterMode,
           "Fitter mode: 1: fast, 2: firmware",
           unsigned(1));
  addParam("minHits", m_minHits,
           "Minimal number of hits required for the fitting.",
           unsigned(2));
  addParam("xtSimple", m_xtSimple,
           "If true, use nominal drift velocity, otherwise use table "
           "for non-linear xt.",
           false);
  addParam("isVerbose", m_isVerbose,
           "If true, prints detail information.",
           false);
}

void
CDCTrigger3DFitterModule::initialize()
{
  // register DataStore elements
  m_tracks3D.registerInDataStore(m_outputCollectionName);
  m_tracks2D.isRequired(m_inputCollectionName);
  m_hits.isRequired(m_hitCollectionName);
  m_eventTime.isRequired(m_EventTimeName);
  // register relations
  m_tracks2D.registerRelationTo(m_tracks3D);
  m_tracks2D.requireRelationTo(m_hits);
  m_tracks3D.registerRelationTo(m_hits);


  TRGCDCFitter3D::getStereoGeometry(m_stGeometry);
  TRGCDCFitter3D::getStereoXt(m_stGeometry["priorityLayer"], m_stXts);
  //Fitter3DUtility::saveStereoXt(m_stXts, "stereoXt");
  //Fitter3DUtility::loadStereoXt("TODOdata/stereoXt",4,m_stXts);

  m_commonData = new Belle2::TRGCDCJSignalData();
  TRGCDCFitter3D::getConstants(m_mConstD, m_mConstV);

  // get geometry constants for first priority layers
  const CDC::CDCGeometryPar& cdc = CDC::CDCGeometryPar::Instance();
  // TODO: avoid hard coding the priority layers here
  vector<unsigned> iL = {10, 22, 34, 46};
  for (int iSt = 0; iSt < 4; ++iSt) {
    nWires.push_back(cdc.nWiresInLayer(iL[iSt]));
    rr.push_back(cdc.senseWireR(iL[iSt]));
    zToStraw.push_back(cdc.senseWireBZ(iL[iSt]));
    nShift.push_back(cdc.nShifts(iL[iSt]));
    angleSt.push_back(2 * rr.back() * sin(M_PI * nShift.back() / (2 * nWires.back())) /
                      (cdc.senseWireFZ(iL[iSt]) - zToStraw.back()));
    vector<double> xt(512);
    for (unsigned iTick = 0; iTick < xt.size(); ++iTick) {
      double t = iTick * 2 * cdc.getTdcBinWidth();
      if (m_xtSimple) {
        xt[iTick] = cdc.getNominalDriftV() * t;
      } else {
        double driftLength_0 = cdc.getDriftLength(t, iL[iSt], 0);
        double driftLength_1 = cdc.getDriftLength(t, iL[iSt], 1);
        xt[iTick] = (driftLength_0 + driftLength_1) / 2;
      }
    }
    xtTables.push_back(xt);
  }

}

void
CDCTrigger3DFitterModule::event()
{
  //cout<<"n2D tracks:"<<m_tracks2D.getEntries()<<endl;

  if (m_isVerbose) {
    cout << "Event" << endl;
    // stereo TS information
    cout << "----Stereo TS----" << endl;
    for (int iHit = 0; iHit < m_hits.getEntries(); ++iHit) {
      if (m_hits[iHit]->getISuperLayer() % 2 != 0)
        cout << "sl-iWire:" << m_hits[iHit]->getISuperLayer() << "-" << m_hits[iHit]->getIWire() << " pr:" <<
             m_hits[iHit]->getPriorityPosition() << " lr:" << m_hits[iHit]->getLeftRight() << " rt:" << m_hits[iHit]->getTDCCount() << " dt:" <<
             m_hits[iHit]->priorityTime() << endl;
    }
    cout << "----Stereo TS----" << endl;
    cout << "----EventTime----" << endl;
    // event time information
    cout << "eventTimeValid: " << m_eventTime->hasBinnedEventT0(Const::CDC);
    if (m_eventTime->hasBinnedEventT0(Const::CDC))  cout << " eventTime: " << m_eventTime->getBinnedEventT0(Const::CDC);
    cout << endl;
    cout << "----EventTime----" << endl;
    cout << "----2D----" << endl;
    // 2D information
    for (int iTrack = 0; iTrack < m_tracks2D.getEntries(); ++iTrack) {
      cout << "2D charge: " << m_tracks2D[iTrack]->getChargeSign() << " pt: " << 1. / abs(m_tracks2D[iTrack]->getOmega()) * 1.5 * 0.3 *
           0.01 << " phi0_i: " << m_tracks2D[iTrack]->getPhi0() << endl;
    }
    cout << "----2D----" << endl;
  }

  for (int itrack = 0; itrack < m_tracks2D.getEntries(); ++itrack) {
    int charge = m_tracks2D[itrack]->getChargeSign();
    double rho = 1. / abs(m_tracks2D[itrack]->getOmega());
    double phi = m_tracks2D[itrack]->getPhi0() - charge * M_PI_2;

    // select stereo hits
    vector<int> bestTSIndex(4, -1);
    vector<double> bestTSPhi(4, 9999);
    finder(charge, rho, phi, bestTSIndex, bestTSPhi);

    if (m_isVerbose) {
      cout << "----Found Stereo TS----" << endl;
      for (unsigned iSt = 0; iSt < 4; ++iSt) {
        if (bestTSIndex[iSt] == -1) continue;
        cout << "sl-iWire:" << iSt * 2 + 1 << "-" << m_hits[bestTSIndex[iSt]]->getIWire() << " pr:" <<
             m_hits[bestTSIndex[iSt]]->getPriorityPosition() << " lr:" << m_hits[bestTSIndex[iSt]]->getLeftRight() << " rt:" <<
             m_hits[bestTSIndex[iSt]]->getTDCCount() << " dt:" << m_hits[bestTSIndex[iSt]]->priorityTime() << endl;
      }
      cout << "----Found Stereo TS----" << endl;
      cout << "----2D----" << endl;
      cout << "charge: " << charge << " radius: " << rho << " phi0_c: " << phi << endl;
      cout << "----2D----" << endl;
    }

    // count the number of selected hits
    unsigned nHits = 0;
    for (unsigned iSt = 0; iSt < 4; ++iSt) {
      if (bestTSIndex[iSt] != -1) {
        nHits += 1;
      }
    }
    if (nHits < m_minHits) {
      B2DEBUG(100, "Not enough hits to do 3D fit (" << m_minHits << " needed, got " << nHits << ")");
      continue;
    }

    // do the fit and create a new track
    double z0 = 0;
    double cot = 0;
    double chi2 = 0;
    fitter(bestTSIndex, bestTSPhi, charge, rho, phi, z0, cot, chi2);

    //cout<<"nHits: "<<nHits<<" charge:"<<charge<<" rho:"<<rho<<" phi:"<<phi<<" z0:"<<z0<<" cot:"<<cot<<endl;

    // check if fit results are valid
    if (isnan(z0) || isnan(cot) || isnan(chi2)) {
      B2DEBUG(100, "3D fit failed");
      continue;
    }

    // save track
    CDCTriggerTrack* fittedTrack =
      m_tracks3D.appendNew(m_tracks2D[itrack]->getPhi0(), m_tracks2D[itrack]->getOmega(),
                           m_tracks2D[itrack]->getChi2D(),
                           z0, cot, chi2);
    // make relation to 2D track
    m_tracks2D[itrack]->addRelationTo(fittedTrack);
    // make relation to hits
    for (unsigned iSt = 0; iSt < 4; ++iSt) {
      if (bestTSIndex[iSt] != -1)
        fittedTrack->addRelationTo(m_hits[bestTSIndex[iSt]]);
    }
    // add axial relations from 2D track
    RelationVector<CDCTriggerSegmentHit> axialHits =
      m_tracks2D[itrack]->getRelationsTo<CDCTriggerSegmentHit>(m_hitCollectionName);
    for (unsigned ihit = 0; ihit < axialHits.size(); ++ihit) {
      fittedTrack->addRelationTo(axialHits[ihit]);
    }
  }
}

void
CDCTrigger3DFitterModule::finder(int charge, double rho, double phi,
                                 vector<int>& bestTSIndex, vector<double>& bestTSPhi)
{
  vector<double> stAxPhi(4);
  for (int iSt = 0; iSt < 4; ++iSt) {
    stAxPhi[iSt] = Fitter3DUtility::calStAxPhi(charge, angleSt[iSt], zToStraw[iSt],
                                               rr[iSt], rho, phi);
  }
  // get candidates
  vector<vector<int>> candidatesIndex(4, vector<int>());
  vector<vector<double>> candidatesPhi(4, vector<double>());
  vector<vector<double>> candidatesDiffStWires(4, vector<double>());
  for (int ihit = 0; ihit < m_hits.getEntries(); ++ihit) {
    //// Print things
    //cout<<"id:"<<m_hits[ihit]->getSegmentID()<<" pr:"<<m_hits[ihit]->getPriorityPosition()<<" lr:"<<m_hits[ihit]->getLeftRight()<<" rt:"<<m_hits[ihit]->priorityTime()<<" fastestTime:"<<m_hits[ihit]->fastestTime()<<" foundTime:"<<m_hits[ihit]->foundTime()<<endl;
    //cout<<"tdc:"<<m_hits[ihit]->getTDCCount()<<" adc:"<<m_hits[ihit]->getADCCount()<<" isl:"<<m_hits[ihit]->getISuperLayer()<<" ilayer:"<<m_hits[ihit]->getILayer()<<" iwire:"<<m_hits[ihit]->getIWire()<<endl;
    // Reject second priority TSs.
    if (m_hits[ihit]->getPriorityPosition() != 3) continue;
    // only stereo hits
    unsigned iSL = m_hits[ihit]->getISuperLayer();
    if (iSL % 2 == 0) continue;
    // skip hits with too large radius
    if (2 * rho < rr[iSL / 2]) continue;
    // Find number of wire difference
    double wirePhi = m_hits[ihit]->getIWire() * 2. * M_PI / nWires[iSL / 2];
    double tsDiffSt = stAxPhi[iSL / 2] - wirePhi;
    if (tsDiffSt > M_PI) tsDiffSt -= 2 * M_PI;
    if (tsDiffSt < -M_PI) tsDiffSt += 2 * M_PI;
    tsDiffSt = tsDiffSt / 2 / M_PI * nWires[iSL / 2];
    // Save index if condition is in 10 wires
    if ((iSL / 2) % 2 == 0) {
      if (tsDiffSt > 0 && tsDiffSt <= 10) {
        candidatesIndex[iSL / 2].push_back(ihit);
        candidatesPhi[iSL / 2].push_back(wirePhi);
        candidatesDiffStWires[iSL / 2].push_back(tsDiffSt);
      }
    } else {
      if (tsDiffSt < 0 && tsDiffSt >= -10) {
        candidatesIndex[iSL / 2].push_back(ihit);
        candidatesPhi[iSL / 2].push_back(wirePhi);
        candidatesDiffStWires[iSL / 2].push_back(tsDiffSt);
      }
    } // End of saving index
  } // Candidate loop

  // Pick middle candidate if multiple candidates
  // mean wire diff
  double meanWireDiff[4] = { 3.68186, 3.3542, 3.9099, 4.48263 };
  for (int iSt = 0; iSt < 4; ++iSt) {
    //cout<<"iSt: "<<iSt<<" nCandidates:"<<candidatesIndex[iSt].size()<<endl;
    //if (candidatesIndex[iSt].size() >1) continue;
    double bestDiff = 9999;
    for (int iTS = 0; iTS < int(candidatesIndex[iSt].size()); ++iTS) {
      double diff = abs(abs(candidatesDiffStWires[iSt][iTS]) - meanWireDiff[iSt]);
      // Pick the better TS
      if (diff < bestDiff) {
        bestDiff = diff;
        bestTSPhi[iSt] = candidatesPhi[iSt][iTS];
        bestTSIndex[iSt] = candidatesIndex[iSt][iTS];
      }
    } // TS loop
  } // Layer loop


  //StoreArray<Track> tracks;
  //for (int iTrack = 0; iTrack < tracks.getEntries(); iTrack++)
  //{
  //  cout<<tracks[iTrack].getTrackFitResults().size()<<endl;
  //}

}

void
CDCTrigger3DFitterModule::fitter(const vector<int>& bestTSIndex, [[maybe_unused]] vector<double>& bestTSPhi,
                                 int charge, double rho, double phi,
                                 double& z0, double& cot, double& chi2)
{
  // Find min timing [0, 511]

  // Convert format
  // rawStTSs[iSt] = [TS ID, TS LR, TS driftTime]
  vector<vector<int> > rawStTSs(4, vector<int> (3));
  for (unsigned iSt = 0; iSt < 4; ++iSt) {
    if (bestTSIndex[iSt] == -1) continue;
    rawStTSs[iSt][0] = m_hits[bestTSIndex[iSt]]->getIWire();
    rawStTSs[iSt][1] = m_hits[bestTSIndex[iSt]]->getLeftRight();
    rawStTSs[iSt][2] = m_hits[bestTSIndex[iSt]]->priorityTime();
  }
  int eventTimeValid = 0;
  int eventTime = 0;
  if (m_eventTime->hasBinnedEventT0(Const::CDC)) {
    eventTimeValid = 1;
    eventTime = m_eventTime->getBinnedEventT0(Const::CDC);
    //cout<<"eventTime: "<<eventTime<<endl;
  }
  double radius = rho;
  double phi_c = phi;
  if (phi_c > M_PI) phi_c -= 2 * M_PI;
  if (phi_c < -M_PI) phi_c += 2 * M_PI;

  vector<double> arcS(4, 0);
  vector<double> zz(4, 0);
  vector<double> invZError2(4, 0);

  // Do fit
  if (m_fitterMode == 1) Fitter3DUtility::fitter3D(m_stGeometry, m_stXts, eventTimeValid, eventTime, rawStTSs, charge, radius, phi_c,
                                                     z0, cot, chi2, arcS, zz, invZError2);

  if (m_fitterMode == 2) {
    Fitter3DUtility::fitter3DFirm(m_mConstD, m_mConstV, eventTimeValid, eventTime, rawStTSs, charge, radius, phi_c, m_commonData,
                                  m_mSignalStorage, m_mLutStorage);
    z0 = m_mSignalStorage["z0_r"].getRealInt();
    cot = m_mSignalStorage["cot_r"].getRealInt();
    chi2 = m_mSignalStorage["zChi2_r"].getRealInt();
    for (int iSt = 0; iSt < 4; iSt++) {
      invZError2[iSt] = m_mSignalStorage["iZError2_" + to_string(iSt)].getRealInt();
      arcS[iSt] = m_mSignalStorage["arcS_" + to_string(iSt)].getRealInt();
      zz[iSt] = m_mSignalStorage["zz_" + to_string(iSt)].getRealInt();
    }
  }

  if (m_isVerbose) {
    cout << "----3D----" << endl;
    cout << "arcS [0]: " << arcS[0] << " [1]: " << arcS[1] << " [2]: " << arcS[2] << " [3]: " << arcS[3] << endl;
    cout << "zz [0]: " << zz[0] << " [1]: " << zz[1] << " [2]: " << zz[2] << " [3]: " << zz[3] << endl;
    cout << "invZError2 [0]: " << invZError2[0] << " [1]: " << invZError2[1] << " [2]: " << invZError2[2] << " [3]: " << invZError2[3]
         << endl;
    cout << "z0: " << z0 << " cot: " << cot << " chi2: " << chi2 << endl;
    cout << "----3D----" << endl;
  }

  //double newZ0 = 0;
  //double newCot = 0;
  //double newChi2 = 0;
  //{
  //  // Convert format
  //  // rawStTSs[iSt] = [TS ID, TS LR, TS driftTime]
  //  vector<vector<int> > rawStTSs (4, vector<int> (3));
  //  for(unsigned iSt = 0; iSt <4; ++iSt)
  //  {
  //    if (bestTSIndex[iSt] == -1) continue;
  //    rawStTSs[iSt][0] = m_hits[bestTSIndex[iSt]]->getIWire();
  //    rawStTSs[iSt][1] = m_hits[bestTSIndex[iSt]]->getLeftRight();
  //    rawStTSs[iSt][2] = m_hits[bestTSIndex[iSt]]->priorityTime();
  //  }
  //  int eventTimeValid = 0;
  //  int eventTime = 0;
  //  if (m_eventTime->hasBinnedEventT0(Const::CDC))
  //  {
  //    eventTimeValid = 1;
  //    eventTime = m_eventTime->getBinnedEventT0(Const::CDC);
  //  }
  //  double radius = rho;
  //  double phi_c = phi;
  //  Fitter3DUtility::fitter3D(m_stGeometry, m_stXts, eventTimeValid, eventTime, rawStTSs, charge, radius, phi_c, newZ0, newCot, newChi2);
  //  //// Fit3D
  //  //vector<double> stTSs(4);
  //  //Fitter3DUtility::calPhiFast(m_stGeometry, m_stXts, eventTimeValid, eventTime, rawStTSs, stTSs);
  //  //vector<double> invZError2;
  //  //Fitter3DUtility::setErrorFast(rawStTSs, eventTimeValid, invZError2);
  //  //vector<double> zz(4, 0);
  //  //vector<double> arcS(4, 0);
  //  //for (unsigned iSt = 0; iSt < 4; iSt++) {
  //  //  if (rawStTSs[iSt][1] != 0) {
  //  //    zz[iSt] = Fitter3DUtility::calZ(charge, m_stGeometry["angleSt"][iSt], m_stGeometry["zToStraw"][iSt],
  //  //                                    m_stGeometry["cdcRadius"][iSt], stTSs[iSt], radius, phi_c);
  //  //    arcS[iSt] = Fitter3DUtility::calS(radius, m_stGeometry["cdcRadius"][iSt]);
  //  //  }
  //  //}
  //  //double z0 = 0;
  //  //double cot = 0;
  //  //double chi2 = 0;
  //  //Fitter3DUtility::rSFit(&invZError2[0], &arcS[0], &zz[0], z0, cot, chi2);
  //  //newZ0 = z0;
  //  //newCot = cot;
  //  //newChi2 = chi2;
  //  ////cout<<"[Calculate fast]"<<endl;
  //  ////cout<<"  [calPhi] "<<stTSs[0]<<" "<<stTSs[1]<<" "<<stTSs[2]<<" "<<stTSs[3]<<endl;
  //  ////cout<<"  [invZError2] "<<invZError2[0]<<" "<<invZError2[1]<<" "<<invZError2[2]<<" "<<invZError2[3]<<endl;
  //  ////cout<<"  [zz] "<<zz[0]<<" "<<zz[1]<<" "<<zz[2]<<" "<<zz[3]<<endl;
  //  ////cout<<"  [arcS] "<<arcS[0]<<" "<<arcS[1]<<" "<<arcS[2]<<" "<<arcS[3]<<endl;
  //  ////cout<<"  [z0] "<<z0<<" [cot] "<<cot<<" [chi2] "<<chi2<<endl;
  //}


  //int T0 = (m_eventTime->hasBinnedEventT0(Const::CDC))
  //         ? m_eventTime->getBinnedEventT0(Const::CDC)
  //         : 9999;
  //
  ////cout<<"T0:"<<T0<<endl;
  ////T0 = 9999;

  //// Fill information for stereo layers
  //vector<double> wirePhi(4, 9999);
  //vector<unsigned> LR(4, 0);
  //vector<int> driftTime(4, 9999);
  //for (unsigned iSt = 0; iSt < 4; ++iSt) {
  //  if (bestTSIndex[iSt] != -1) {
  //    wirePhi[iSt] = bestTSPhi[iSt];
  //    LR[iSt] = m_hits[bestTSIndex[iSt]]->getLeftRight();
  //    driftTime[iSt] = m_hits[bestTSIndex[iSt]]->priorityTime();
  //  }
  //} // End superlayer loop
  ////for (unsigned iSt = 0; iSt < 4; ++iSt) {
  ////  cout<<"iSt: "<<iSt<<" wirePhi: "<<wirePhi[iSt]<<" index: "<<bestTSIndex[iSt]<<" LR:"<<LR[iSt]<<" driftTime: "<<driftTime[iSt]<<endl;
  ////}

  //// Calculate phi3D.
  //vector<double> phi3D(4, 9999);
  //if (T0 == 9999) {
  //  for (unsigned iSt = 0; iSt < 4; iSt++) {
  //    phi3D[iSt] = wirePhi[iSt];
  //  }
  //} else {
  //  for (unsigned iSt = 0; iSt < 4; iSt++) {
  //    if (bestTSIndex[iSt] != -1) {
  //      // Get drift length from table.
  //      int t = driftTime[iSt] - T0;
  //      if (t < 0) t = 0;
  //      if (t > 511) t = 511;
  //      double driftLength = xtTables[iSt][t];
  //      phi3D[iSt] = Fitter3DUtility::calPhi(wirePhi[iSt], driftLength, rr[iSt], LR[iSt]);
  //    }
  //  }
  //}
  //// Get zerror for 3D fit
  //vector<double> driftZError({0.7676, 0.9753, 1.029, 1.372});
  //vector<double> wireZError({0.7676, 0.9753, 1.029, 1.372});
  //vector<double> zError(4, 9999);
  //vector<double> invZError2(4, 0);
  //for (unsigned iSt = 0; iSt < 4; ++iSt) {
  //  if (bestTSIndex[iSt] != -1) {
  //    // Check LR and eventTime
  //    if (LR[iSt] != 3 && T0 != 9999) zError[iSt] = driftZError[iSt];
  //    else zError[iSt] = wireZError[iSt];
  //    // Get inverse zerror ^ 2
  //    invZError2[iSt] = 1 / pow(zError[iSt], 2);
  //  }
  //}

  //// Calculate zz and arcS
  //vector<double> zz(4, 0);
  //vector<double> arcS(4, 0);
  //for (unsigned iSt = 0; iSt < 4; iSt++) {
  //  if (bestTSIndex[iSt] != -1) {
  //    zz[iSt] = Fitter3DUtility::calZ(charge, angleSt[iSt], zToStraw[iSt],
  //                                    rr[iSt], phi3D[iSt], rho, phi);
  //    arcS[iSt] = Fitter3DUtility::calS(rho, rr[iSt]);
  //  }
  //}

  //// Fit3D
  //Fitter3DUtility::rSFit(&invZError2[0], &arcS[0], &zz[0], z0, cot, chi2);

  ////cout<<"[Calculate fastOrg]"<<endl;
  ////cout<<"  [calPhi] "<<phi3D[0]<<" "<<phi3D[1]<<" "<<phi3D[2]<<" "<<phi3D[3]<<endl;
  ////cout<<"  [invZError2] "<<invZError2[0]<<" "<<invZError2[1]<<" "<<invZError2[2]<<" "<<invZError2[3]<<endl;
  ////cout<<"  [zz] "<<zz[0]<<" "<<zz[1]<<" "<<zz[2]<<" "<<zz[3]<<endl;
  ////cout<<"  [arcS] "<<arcS[0]<<" "<<arcS[1]<<" "<<arcS[2]<<" "<<arcS[3]<<endl;
  ////cout<<"  [z0] "<<z0<<" [cot] "<<cot<<" [chi2] "<<chi2<<endl;

  ////if(fabs(z0 - newZ0) > 0.00001) cout<<"z0: "<<z0<<" newZ0: "<<newZ0<<endl;
  ////if(fabs(cot - newCot) > 0.00001) cout<<"cot: "<<cot<<" newCot: "<<newCot<<endl;
  ////if(fabs(chi2 - newChi2) > 0.00001) cout<<"chi2: "<<chi2<<" newChi2: "<<newChi2<<endl;
}
