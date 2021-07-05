/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
//-----------------------------------------------------------------------------
// $Id$
//-----------------------------------------------------------------------------
// Filename : TRGGRLMatch.h
// Section  : TRG CDC
// Owner    : Yun-Tsung Lai
// Email    : ytlai@hep1.phys.ntu.edu.tw
//-----------------------------------------------------------------------------
// $Log$
//-----------------------------------------------------------------------------

#include "framework/datastore/StoreArray.h"
#include "trg/cdc/Track.h"
#include "trg/ecl/dataobjects/TRGECLCluster.h"

namespace Belle2 {

  /** A class to represent a matching candidate in TRGGRL
   * A matching candidate consists of a TRGCDCTrack and TRGECLCluster.
   * The outputs are dr and dz between track and cluster.
   */

  class TRGGRLMatch {

    /** public member functions of TRGGRLMatch class
     * Private variables can be called by the corresponding get function
     */

  public:

    /// Constructor
    TRGGRLMatch(TRGCDCTrack* track, TRGECLCluster* cluster, int flag);

    /// Destructor
    virtual ~TRGGRLMatch();

  public:// Selectors

    /** Unpacker of the TRGGRLMatch */
    void dump(void);

    /** a function returns center (x) of track helix */
    double getCenter_x(void) const {return _center_x;};
    /** a function returns center (y) of track helix */
    double getCenter_y(void) const {return _center_y;};
    /** a function returns center (z) of track helix */
    double getCenter_z(void) const {return _center_z;};
    /** a function returns radius of the track to IP */
    double getCenter_r(void) const {return _r;};
    /** a function returns phi of the track center */
    double getCenter_phi(void) const {return _phi;};
    /** a function returns pt of the track */
    double getCenter_pt(void) const {return _pt;};
    /** a function returns z0 of track helix */
    double getCenter_z0(void) const {return _z0;};
    /** a function returns pz of track helix */
    double getCenter_pz(void) const {return _pz;};
    /** a function returns z slope (polar angle) of track helix */
    double getCenter_slope(void) const {return _slope;};

    /** a function returns x position of the cluster */
    double getCluster_x(void) const {return _cluster_x;};
    /** a function returns y position of the cluster */
    double getCluster_y(void) const {return _cluster_y;};
    /** a function returns z position of the cluster */
    double getCluster_z(void) const {return _cluster_z;};
    /** a function returns deposit energy of the cluster */
    double getCluster_e(void) const {return _cluster_e;};
    /** a function returns transverse distance of the cluster to IP */
    double getCluster_R(void) const {return _R;};
    /** a function returns distance of the cluster to IP */
    double getCluster_D(void) const {return _D;};
    /** a function returns 3-momentum of a track scaled by cluster 3D position */
    double getP_rescaled(void) const {return _re_scaled_p;};
    /** a function returns 3-momentum of a track */
    double getP(void) const {return _p;};

    /** a function returns calculated dr of the matching candidate */
    double getDr(void) const {return _dr;};
    /** a function returns calculated dz of the matching candidate */
    double getDz(void) const {return _dz;};
    /** a function returns calculated p over E of the matching candidate */
    double getPoe(void) const {return _poe;};

    /** a function retuens flag tells 3D information is valid or not */
    int getMatch3D(void) const {return _match3D;};


  private:

    /** a pointer to the TRGCDCTrack of the matching candidate */
    TRGCDCTrack* _track;

    /** a pointer to the TRGECLCluster of the matching candidate */
    TRGECLCluster* _cluster;

    /** flag tells 3D information is valid or not */
    int _match3D;

    /** center (x) of track helix */
    double _center_x;
    /** center (y) of track helix */
    double _center_y;
    /** center (z) of track helix */
    double _center_z;
    /** radius of track  */
    double _r;
    /** phi of the track center */
    double _phi;
    /** pt of track  */
    double _pt;
    /** p of track  */
    double _p;
    /** pz of track  */
    double _pz;
    /** z0 of track  */
    double _z0;
    /** z slope (polar angle) of track  */
    double _slope;

    /** x position of cluster */
    double _cluster_x;
    /** y position of cluster */
    double _cluster_y;
    /** z position of cluster */
    double _cluster_z;
    /** deposit energy of cluster */
    double _cluster_e;
    /** transverse distance of cluster to IP */
    double _R;
    /** distance of clusterto IP */
    double _D;
    /** track momentum scaled with 3D information from cluster */
    double _re_scaled_p;

    /** dr between track and cluster */
    double _dr;
    /** dz between track and cluster */
    double _dz;
    /** x position of the cluster calculated by matching */
    double _ex_x;
    /** y position of the cluster calculated by matching */
    double _ex_y;
    /** z position of the cluster calculated by matching */
    double _ex_z;
    /** calculated p over e  */
    double _poe;

    /** a function to execute calculation of matching */
    void calculate(void);
  };



} // namespace Belle2

