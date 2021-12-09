/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include "trg/trg/Signal.h"
#include "trg/cdc/TRGCDCTrack.h"
#include "trg/cdc/Helix.h"
#include "trg/ecl/dataobjects/TRGECLCluster.h"
#include "trg/grl/TRGGRLMatch.h"
#include <math.h>

using namespace std;

namespace Belle2 {


  TRGGRLMatch::TRGGRLMatch(TRGCDCTrack* track, TRGECLCluster* cluster, int flag)
    : _track(track),
      _cluster(cluster),
      _match3D(flag)
  {
    calculate();
  }

  TRGGRLMatch::~TRGGRLMatch()
  {
  }

  void
  TRGGRLMatch::calculate(void)
  {

    //-- track/TRGCDC information
    const TRGCDCHelix& helix = _track->helix();
    _pt = _track->pt();
    _center_x = helix.center().x();
    _center_y = helix.center().y();
    _center_z = helix.center().z();
    _r = sqrt(_center_x * _center_x + _center_y * _center_y); //helix.radius();
    _phi = atan2(_center_y, _center_x) ;

    //-- cluster/TRGECL information
    _cluster_x = _cluster->getPositionX();
    _cluster_y = _cluster->getPositionY();
    _cluster_z = _cluster->getPositionZ();
    _cluster_e = _cluster->getEnergyDep();
    _R = sqrt(_cluster_x * _cluster_x + _cluster_y * _cluster_y);
    _D = sqrt(_cluster_x * _cluster_x + _cluster_y * _cluster_y + _cluster_z * _cluster_z);
    _re_scaled_p = _pt * _D / _R;

    //-- calculation
    double theta0 = acos(_R / (2 * _r)) + _phi;
    double theta1 = 2 * _phi - theta0;

    double ex_x0 = _R * cos(theta0), ex_y0 = _R * sin(theta0), ex_x1 = _R * cos(theta1), ex_y1 = _R * sin(theta1);
    double dr0 = sqrt((ex_x0 - _cluster_x) * (ex_x0 - _cluster_x) + (ex_y0 - _cluster_y) * (ex_y0 - _cluster_y));
    double dr1 = sqrt((ex_x1 - _cluster_x) * (ex_x1 - _cluster_x) + (ex_y1 - _cluster_y) * (ex_y1 - _cluster_y));

    _dr = (dr0 < dr1) ? dr0 : dr1;
    _ex_x = (dr0 < dr1) ?  ex_x0 : ex_x1;
    _ex_y = (dr0 < dr1) ?  ex_y0 : ex_y1;

    //z information
    if (_match3D == 1) {
      _z0 = helix.dz();
      _slope = helix.tanl();
      _ex_z = _z0 + _slope * 2 * _r * asin(_R / (2 * _r));
      _pz = _pt * _slope;
      _p = sqrt(_pz * _pz + _pt * _pt);
      _dz = _cluster_z - _ex_z;
      _poe = _p / _cluster_e;

    }

  }


  void
  TRGGRLMatch::dump(void)
  {

    cout << "double center_x = " << _center_x << ";" << endl;
    cout << "double center_y = " << _center_y << ";" << endl;
    cout << "double center_z = " << _center_z << ";" << endl;
    cout << "double radius = " << _r << ";" << endl;
    cout << "double pt = " << _pt << ";" << endl;
    cout << "double p = " << _p << ";" << endl;
    cout << "double cluster_x = " << _cluster_x << ";" << endl;
    cout << "double cluster_y = " << _cluster_y << ";" << endl;
    cout << "double cluster_z = " << _cluster_z << ";" << endl;
    cout << "double ex_x = " << _ex_x << ";" << endl;
    cout << "double ex_y = " << _ex_y << ";" << endl;
    cout << "double ex_z = " << _ex_z << ";" << endl;

  }


} // namespace Belle2

