//-----------------------------------------------------------------------------
// $Id$
//-----------------------------------------------------------------------------
// Filename : Helix.h
// Section  : TRG CDC
// Owner    : Yoshihito Iwasaki
// Email    : yoshihito.iwasaki@kek.jp
//-----------------------------------------------------------------------------
// Description : A class to represent a track helix parameter in Belle style
//-----------------------------------------------------------------------------
// $Log$
//-----------------------------------------------------------------------------

#include <fstream>
#include "framework/datastore/StoreArray.h"
#include "trg/trg/Debug.h"
#include "trg/trg/Time.h"
#include "trg/trg/State.h"
#include "trg/trg/Signal.h"
#include "trg/trg/Channel.h"
#include "trg/trg/Utilities.h"
#include "trg/cdc/TRGCDC.h"
#include "trg/cdc/Track.h"
#include "trg/cdc/Helix.h"
#include "trg/ecl/dataobjects/TRGECLCluster.h"
#include "trg/ecl/TrgEclCluster.h"
#include <math.h>



namespace Belle2 {

/// TRGCDCHelix parameter class
class TRGGRLMatch {

  public:

    TRGGRLMatch(TRGCDCTrack * track, TRGECLCluster * cluster, int flag);

    /// Destructor
    virtual ~TRGGRLMatch();

  public:// Selectors

    void dump(void);

    double getCenter_x(void) const{return center_x;};
    double getCenter_y(void) const{return center_y;};
    double getCenter_z(void) const{return center_z;};
    double getCenter_r(void) const{return r;};
    double getCenter_phi(void) const{return phi;};
    double getCenter_pt(void) const{return pt;};
    double getCenter_z0(void) const{return z0;};
    double getCenter_pz(void) const{return pz;};
    double getCenter_slope(void) const{return slope;};

    double getCluster_x(void) const{return cluster_x;};
    double getCluster_y(void) const{return cluster_y;};
    double getCluster_z(void) const{return cluster_z;};
    double getCluster_e(void) const{return cluster_e;};
    double getCluster_R(void) const{return R;};
    double getCluster_D(void) const{return D;};
    double getP_rescaled(void) const{return re_scaled_p;};
    double getP(void) const{return p;};

    double getDr(void) const{return dr;};
    double getDz(void) const{return dz;};
    double getPoe(void) const{return poe;};

    int getMatch3D(void) const{return match3D;};

private:

    void calculate(void);

    int match3D;

    /// center of track helix
    double center_x, center_y, center_z, r, phi, pt, p, pz, z0, slope;

    /// cluster information
    double cluster_x, cluster_y, cluster_z, cluster_e, R, D, re_scaled_p;

    /// result of matching
    double dr, dz, ex_x, ex_y, ex_z, poe;

    TRGCDCTrack * _track;

    TRGECLCluster * _cluster;


};



} // namespace Belle2

