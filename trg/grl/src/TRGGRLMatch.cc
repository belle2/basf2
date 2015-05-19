//-----------------------------------------------------------------------------
// $Id$
//-----------------------------------------------------------------------------
// Filename : TRGGRLMatch.cc
// Section  : TRG GRL
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
#include "trg/grl/TRGGRLMatch.h"
#include <math.h>

using namespace std;

namespace Belle2 {


TRGGRLMatch::TRGGRLMatch(TRGCDCTrack * track, TRGECLCluster * cluster, int flag)
: _track(track),
  _cluster(cluster),
  match3D(flag) {
calculate();
}


TRGGRLMatch::~TRGGRLMatch() {
}

void
TRGGRLMatch::calculate(void) {

        //-- track/TRGCDC information
        const TRGCDCHelix & helix = _track->helix();
        pt = _track->pt();
        center_x = helix.center().x();
	center_y = helix.center().y();
	center_z = helix.center().z();
        r = sqrt(center_x*center_x + center_y*center_y);//helix.radius();
        phi = atan2(center_y, center_x) ;

        //-- cluster/TRGECL information
        cluster_x = _cluster->getPositionX();
	cluster_y = _cluster->getPositionY();
	cluster_z = _cluster->getPositionZ();
        cluster_e = _cluster->getEnergyDep();
        R = sqrt(cluster_x*cluster_x + cluster_y*cluster_y);
        D = sqrt(cluster_x*cluster_x + cluster_y*cluster_y + cluster_z*cluster_z);
        re_scaled_p = pt*D/R;

        //-- calculation
        double theta0 = acos(R/(2*r)) + phi;
        double theta1 = 2*phi - theta0;

        double ex_x0 = R*cos(theta0), ex_y0 = R*sin(theta0), ex_x1 = R*cos(theta1), ex_y1 = R*sin(theta1);
        double dr0 = sqrt( (ex_x0-cluster_x)*(ex_x0-cluster_x) + (ex_y0-cluster_y)*(ex_y0-cluster_y) );
        double dr1 = sqrt( (ex_x1-cluster_x)*(ex_x1-cluster_x) + (ex_y1-cluster_y)*(ex_y1-cluster_y) );

        dr = (dr0 < dr1) ? dr0 : dr1;
	ex_x = (dr0 < dr1) ?  ex_x0: ex_x1;
	ex_y = (dr0 < dr1) ?  ex_y0: ex_y1;

	//z information
	if (match3D == 1) {
	z0 = helix.dz();
	slope = helix.tanl();
	ex_z = z0 + slope*R;
	pz = pt*slope;
	p = sqrt(pz*pz + pt*pt);
	dz = cluster_z - ex_z;
	poe = p/cluster_e;

	}

}


void
TRGGRLMatch::dump(void) {

        cout << "double center_x = " << center_x << ";" <<endl;
        cout << "double center_y = " << center_y << ";" <<endl;
        cout << "double center_z = " << center_z << ";" <<endl;
        cout << "double radius = " << r << ";" <<endl;
        cout << "double pt = " << pt << ";" <<endl;
        cout << "double p = " << p << ";" <<endl;
        cout << "double cluster_x = " << cluster_x << ";" <<endl;
        cout << "double cluster_y = " << cluster_y << ";" <<endl;
        cout << "double cluster_z = " << cluster_z << ";" <<endl;
        cout << "double ex_x = " << ex_x << ";" <<endl;
        cout << "double ex_y = " << ex_y << ";" <<endl;
        cout << "double ex_z = " << ex_z << ";" <<endl;

}


} // namespace Belle2

