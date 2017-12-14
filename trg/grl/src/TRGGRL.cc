//-----------------------------------------------------------------------------
// $Id$
//-----------------------------------------------------------------------------
// Filename : TRGGRL.cc
// Section  : TRG GRL
// Owner    : Yun-Tsung Lai
// Email    : ytlai@hep1.phys.ntu.edu.tw
//-----------------------------------------------------------------------------
// Description : A class to represent GRL.
//-----------------------------------------------------------------------------
// 0.00 : 2013/12/13 : First version
//-----------------------------------------------------------------------------

#define TRG_SHORT_NAMES
#define TRGGRL_SHORT_NAMES

#include <fstream>
#include "framework/datastore/StoreArray.h"
#include "trg/trg/Debug.h"
#include "trg/trg/Time.h"
#include "trg/trg/State.h"
#include "trg/trg/Signal.h"
#include "trg/trg/Channel.h"
#include "trg/trg/Utilities.h"
#include "trg/grl/TRGGRL.h"
//#include "trg/grl/TRGGRLMatch.h"
#include "trg/cdc/TRGCDC.h"
#include "trg/cdc/Track.h"
#include "trg/cdc/Helix.h"
#include "trg/ecl/dataobjects/TRGECLCluster.h"
#include "trg/ecl/TrgEclCluster.h"
#include <math.h>
#include <TFile.h>
#include <TTree.h>
#include <framework/logging/Logger.h>

# define M_PI           3.14159265358979323846

using namespace std;

namespace Belle2 {

  TRGGRL*
  TRGGRL::_grl = 0;

  string
  TRGGRL::name(void) const
  {
    return "TRGGRL";
  }

  string
  TRGGRL::version(void) const
  {
    return string("TRGGRL 0.01");
  }

  TRGGRL*
  TRGGRL::getTRGGRL(const string& configFile,
                    unsigned simulationMode,
                    unsigned fastSimulationMode,
                    unsigned firmwareSimulationMode)
  {
    if (_grl) {
      //delete _grl;
      _grl = 0;
    }

    if (configFile != "good-bye") {
      _grl = new TRGGRL(configFile,
                        simulationMode,
                        fastSimulationMode,
                        firmwareSimulationMode);
    } else {
      B2DEBUG(100, "TRGGRL::getTRGGRL ... good-bye");
      _grl = 0;
    }

    return _grl;
  }

  TRGGRL*
  TRGGRL::getTRGGRL(void)
  {
    if (! _grl)
      B2WARNING("TRGGRL::getTRGGRL !!! TRGGRL is not created yet");
    return _grl;
  }

  TRGGRL::TRGGRL(const string& configFile,
                 unsigned simulationMode,
                 unsigned fastSimulationMode,
                 unsigned firmwareSimulationMode)
    : _debugLevel(0),
      _configFilename(configFile),
      _simulationMode(simulationMode),
      _fastSimulationMode(fastSimulationMode),
      _firmwareSimulationMode(firmwareSimulationMode),
      _clock(Belle2_GDL::GDLSystemClock)
  {

    B2DEBUG(100, "TRGGRL ... TRGGRL initializing with " << _configFilename
            << "           mode=0x" << hex << _simulationMode << dec);

    initialize();

    B2DEBUG(100, "TRGGRL ... TRGGRL created with " << _configFilename);
  }

  void
  TRGGRL::initialize(void)
  {

    m_file = new TFile("trggrl.root", "RECREATE");
    h1 = new TTree("h1", "h1");

    h1->Branch("3d", &x0, "3d");
    h1->Branch("dr", &x1, "dr");
    h1->Branch("dz", &x2, "dz");
    h1->Branch("poe", &x3, "poe");
    h1->Branch("z0", &x4, "z0");
    h1->Branch("pt", &x5, "pt");
    h1->Branch("pz", &x6, "pz");
    h1->Branch("e", &x7, "e");

    configure();
  }

  void
  TRGGRL::terminate(void)
  {
  }

  void
  TRGGRL::dump(const string& msg) const
  {

    if (msg != "") B2DEBUG(100, "dump nothing...");

  }

  void
  TRGGRL::clear(void)
  {
  }

  void
  TRGGRL::fastClear(void)
  {
  }

  void
  TRGGRL::update(bool)
  {

    matchList.clear();
    matchList3D.clear();

    B2DEBUG(100, "do nothing...");

  }

  TRGGRL::~TRGGRL()
  {
    h1->Write();
    m_file->Write();
    m_file->Close();
    clear();
  }

  void
  TRGGRL::simulate(void)
  {

    TRGCDC* _cdc = Belle2::TRGCDC::getTRGCDC();
    vector<TRGCDCTrack*> trackList = _cdc->getTrackList2D();
    vector<TRGCDCTrack*> trackList3D = _cdc->getTrackList3D();
    StoreArray<TRGECLCluster> ClusterArray;

    unsigned n_track = trackList.size();
    unsigned n_track3D = trackList3D.size();
    unsigned n_cluster = ClusterArray.getEntries();

//  if (TRGDebug::level() > 2) cout <<"yt_grl "<< n_cluster << " " << n_track << endl;

    for (unsigned i = 0; i < n_track; i++) {
      // vector<TRGGRLMatch *> match_i;
      if (n_cluster == 0) break;
      else if (n_cluster == 1) {
        TRGGRLMatch* match = new TRGGRLMatch(trackList[i], ClusterArray[0], 0);
        matchList.push_back(match);
      } else if (n_cluster > 1) {
        int best_j = 0; double old_dr = 99999;
        for (unsigned j = 0; j < n_cluster; j++) {
          TRGGRLMatch* match = new TRGGRLMatch(trackList[i], ClusterArray[j], 0);
          if (match->getDr() < old_dr) {best_j = j; old_dr = match->getDr();}
        }
        TRGGRLMatch* match = new TRGGRLMatch(trackList[i], ClusterArray[best_j], 0);
        matchList.push_back(match);
      }
    }

    for (unsigned i = 0; i < n_track3D; i++) {
      // vector<TRGGRLMatch *> match_i;
      if (n_cluster == 0) break;
      else if (n_cluster == 1) {
        TRGGRLMatch* match = new TRGGRLMatch(trackList3D[i], ClusterArray[0], 1);
        matchList.push_back(match);
      } else if (n_cluster > 1) {
        int best_j = 0; double old_dr = 99999;
        for (unsigned j = 0; j < n_cluster; j++) {
          TRGGRLMatch* match = new TRGGRLMatch(trackList3D[i], ClusterArray[j], 0);
          if (match->getDr() < old_dr) {best_j = j; old_dr = match->getDr();}
        }
        TRGGRLMatch* match = new TRGGRLMatch(trackList3D[i], ClusterArray[best_j], 0);
        matchList3D.push_back(match);
      }
    }

    //-----Fill tree
    for (unsigned i = 0; i < matchList.size(); i++) {
      TRGGRLMatch& match = * matchList[i];

      x0 = match.getMatch3D();
      x1 = match.getDr();
      x2 = match.getDz();
      x3 = match.getPoe();
      x4 = match.getCenter_z0();
      x5 = match.getCenter_pt();
      x6 = match.getCenter_pz();
      x7 = match.getCluster_e();
      h1->Fill();
    }

    for (unsigned i = 0; i < matchList3D.size(); i++) {
      TRGGRLMatch& match = * matchList3D[i];

      x0 = match.getMatch3D();
      x1 = match.getDr();
      x2 = match.getDz();
      x3 = match.getPoe();
      x4 = match.getCenter_z0();
      x5 = match.getCenter_pt();
      x6 = match.getCenter_pz();
      x7 = match.getCluster_e();
      h1->Fill();
    }

    //--------------------------------------


    const bool fast = (_simulationMode & 1);
    const bool firm = (_simulationMode & 2);
    if (fast)
      fastSimulation();
    if (firm)
      firmwareSimulation();
  }

  void
  TRGGRL::fastSimulation(void)
  {
  }

  void
  TRGGRL::firmwareSimulation(void)
  {
  }

  void
  TRGGRL::configure(void)
  {
  }
  /*
  bool
  TRGGRL::barrel_matching_2D(TRGCDCTrack * track, TRGECLCluster * cluster){

    //-- track/TRGCDC information
    const TRGCDCHelix & helix = track->helix();
    double pt = track->pt();
    double center_x = helix.center().x(), center_y = helix.center().y(), center_z = helix.center().z();
    double r = sqrt(center_x*center_x + center_y*center_y);//helix.radius();
    double phi = atan2(center_y, center_x) ;

    //-- cluster/TRGECL information
    double cluster_x = cluster->getPositionX(), cluster_y = cluster->getPositionY(), cluster_z = cluster->getPositionZ();
    double cluster_e = cluster->getEnergyDep();
    double R = sqrt(cluster_x*cluster_x + cluster_y*cluster_y);
    double D = sqrt(cluster_x*cluster_x + cluster_y*cluster_y + cluster_z*cluster_z);
    double re_scaled_p = pt*D/R;

    //-- calculation

    double theta0 = acos(R/(2*r)) + phi;
    double theta1 = 2*phi - theta0;

    double ex_x0 = R*cos(theta0), ex_y0 = R*sin(theta0), ex_x1 = R*cos(theta1), ex_y1 = R*sin(theta1);

    double dr0 = sqrt( (ex_x0-cluster_x)*(ex_x0-cluster_x) + (ex_y0-cluster_y)*(ex_y0-cluster_y) );
    double dr1 = sqrt( (ex_x1-cluster_x)*(ex_x1-cluster_x) + (ex_y1-cluster_y)*(ex_y1-cluster_y) );
    double dr = (dr0 < dr1) ? dr0 : dr1;

    if (TRGDebug::level() > 1) printf("%s %f %f %f %f %f %f %f \n","dump! ",dr,dr0,dr1,pt,re_scaled_p,cluster_e,re_scaled_p/cluster_e);

    if (TRGDebug::level() > 1 && dr > 30) {
    cout << " " << endl;
          cout << "double center_x = " << center_x << ";" <<endl;
          cout << "double center_y = " << center_y << ";" <<endl;
          cout << "double center_z = " << center_z << ";" <<endl;
          cout << "double radius = " << r << ";" <<endl;
    cout << "double pt = " << pt << ";" <<endl;
          cout << "double cluster_x = " << cluster_x << ";" <<endl;
          cout << "double cluster_y = " << cluster_y << ";" <<endl;
          cout << "double ex_x0 = " << ex_x0 << ";" <<endl;
          cout << "double ex_y0 = " << ex_y0 << ";" <<endl;
          cout << "double ex_x1 = " << ex_x1 << ";" <<endl;
          cout << "double ex_y1 = " << ex_y1 << ";" <<endl;
    }

    return true;
  }
  */



} // namespace Belle2
