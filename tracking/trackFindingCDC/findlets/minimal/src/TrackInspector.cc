/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Dmitrii Neverov                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/findlets/minimal/TrackInspector.h>

#include <tracking/trackFindingCDC/eventdata/tracks/CDCTrack.h>

#include <framework/core/ModuleParamList.templateDetails.h>
#include <tracking/trackFindingCDC/utilities/StringManipulation.h>

#include <TMultiGraph.h>
#include <TGraph.h>
#include <TCanvas.h>
#include <TAxis.h>
#include <vector>

using namespace Belle2;
using namespace TrackFindingCDC;

std::string TrackInspector::getDescription()
{
  return "Findlet for printing out CDCtracks";
}

void TrackInspector::exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix)
{
  moduleParamList->addParameter(prefixed(prefix, "debugDraw"),
                                m_param_debugDraw,
                                "Draw found hit positions of the track",
                                m_param_debugDraw);
}

void TrackInspector::apply(std::vector<CDCTrack>& tracks)
{
  removeIncompleteTracks(tracks);
  static int nevent(0);
  if (tracks.size() == 0) {
    nevent++;
    return; //Nothing to draw
  }
  if (not m_param_debugDraw) return;  //Nothing to draw
  TCanvas canvA("axialCanvas", "CDC axial hits in an event", 0, 0, 1440, 1080);
  TCanvas canvS("stereoCanvas", "CDC stereo hits in an event", 0, 0, 1440, 1080);
  TMultiGraph* mgA = new TMultiGraph("axialTracks", "CDC axial tracks in the event;X, cm;Y, cm");
  TMultiGraph* mgS = new TMultiGraph("stereoTracks", "CDC stereo tracks in the event;Z, cm;R, cm");
  for (CDCTrack& track : tracks) {
    TGraph* grA = new TGraph();
    TGraph* grS = new TGraph();
    grA->SetLineWidth(2);
    grA->SetLineColor(9);
    grS->SetLineWidth(2);
    grS->SetLineColor(9);
    for (CDCRecoHit3D& hit : track) {
      Vector3D pos = hit.getRecoPos3D();
      const double R = std::sqrt(pos.x() * pos.x() + pos.y() * pos.y());
      const double X = pos.x();
      const double Y = pos.y();
      const double Z = pos.z();
      if (Z == 0 and hit.isAxial()) { // axial hits have no z information
        grA->SetPoint(grA->GetN(), X, Y);
      } else {
        grS->SetPoint(grS->GetN(), Z, R);
      }
    }
    mgA->Add(grA);
    mgS->Add(grS);
  }
  canvA.cd();
  mgA->Draw("APL*");
  canvS.cd();
  mgS->Draw("APL*");
  canvA.Update();
  canvS.Update();
  if (mgA->GetXaxis()) {
    mgA->GetXaxis()->SetLimits(-120, 120);
    mgA->GetYaxis()->SetRangeUser(-120, 120);
    canvA.SaveAs(Form("CDCaxialTracks_%i.png", nevent));
  }
  if (mgS->GetXaxis()) {
    mgS->GetXaxis()->SetLimits(-180, 180);
    mgS->GetYaxis()->SetRangeUser(0, 120);
    canvS.SaveAs(Form("CDCstereoTracks_%i.png", nevent));
  }
  nevent++;
}

void TrackInspector::removeIncompleteTracks(std::vector<CDCTrack>& tracks)
{
  for (auto it = tracks.begin(); it != tracks.end();) {
    bool stereoHitsPresent =
      false; //If stereo hit matcher can't find more hits than its threshold, it doesn't add any, but keeps the track
    for (CDCRecoHit3D& hit : *it) {
      if (not hit.isAxial()) stereoHitsPresent = true;
    }
    if (not stereoHitsPresent) {
      it = tracks.erase(it); //TODO mask hits or something?
    } else {
      ++it;
    }
  }
}
