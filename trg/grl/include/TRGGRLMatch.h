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

    double getCenter_x(void) const{return _center_x;};
    double getCenter_y(void) const{return _center_y;};
    double getCenter_z(void) const{return _center_z;};
    double getCenter_r(void) const{return _r;};
    double getCenter_phi(void) const{return _phi;};
    double getCenter_pt(void) const{return _pt;};
    double getCenter_z0(void) const{return _z0;};
    double getCenter_pz(void) const{return _pz;};
    double getCenter_slope(void) const{return _slope;};

    double getCluster_x(void) const{return _cluster_x;};
    double getCluster_y(void) const{return _cluster_y;};
    double getCluster_z(void) const{return _cluster_z;};
    double getCluster_e(void) const{return _cluster_e;};
    double getCluster_R(void) const{return _R;};
    double getCluster_D(void) const{return _D;};
    double getP_rescaled(void) const{return _re_scaled_p;};
    double getP(void) const{return _p;};

    double getDr(void) const{return _dr;};
    double getDz(void) const{return _dz;};
    double getPoe(void) const{return _poe;};

    int getMatch3D(void) const{return _match3D;};


private:

    TRGCDCTrack * _track;
    
    TRGECLCluster * _cluster; 
    
    int _match3D;

    /// center of track helix
    double _center_x, _center_y, _center_z, _r, _phi, _pt, _p, _pz, _z0, _slope;

    /// cluster information
    double _cluster_x, _cluster_y, _cluster_z, _cluster_e, _R, _D, _re_scaled_p;

    /// result of matching
    double _dr, _dz, _ex_x, _ex_y, _ex_z, _poe;

    void calculate(void);
};



} // namespace Belle2

