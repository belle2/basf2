#if 0 // old
// Functions for position calculation
// in SVD system from Yokoyama-san(U-tokyo)
// In future, when svd utilities functions will be supported,
// this fuctions will be replaced. (May 28, 1999)

#ifndef _DEFINE_TSVD_POSITION_H_
#define _DEFINE_TSVD_POSITION_H_

#ifndef CLHEP_POINT3D_H
#include "CLHEP/Geometry/Point3D.h"
#endif
#include "belleCLHEP/Geometry/Transform3D.h"

/// returns global position (includes alignment)
HepGeom::Point3D<double>
tsvd_position(int DSSD, double philsa, double zlsa);

/*
/// returns global position (not include alignment)
HepGeom::Point3D<double>
svd_nominalposition(int DSSD, double philsa, double zlsa);
*/

/// returns transformation flom DSSD local frame to global frame
HepTransform3D
tsvd_toglobal(int DSSD);

/// returns local position
HepGeom::Point3D<double>
tsvd_localposition(int DSSD, double philsa, double zlsa);

/// returns local x
double
tsvd_localx(int DSSD, double lsa);

/// returns local z
double
tsvd_localz(int DSSD, double lsa);

/// returns side of a DSSD. (0:phi-side is inner side, 1:z-side is inner side)
/// This should be replaced with table "svd_map".
int
tsvd_side(int DSSD);

/// returns half-ladder # of a DSSD.
/// this shoul be replaced with Tomura-kun's function.
int
tsvd_dssd2hladder(int DSSD);

/// From tomura-san(U-tokyo)
///

#define N_channel_c 128
#define N_chip_h 5
#define N_channel_h (N_channel_c*N_chip_h)
#define N_hybrid 128
#define N_channel (N_channel_h*N_hybrid)

int
TRLA2PhiDSSD(int rla);
int
TRLA2ZDSSD(int rla);
#endif /* _DEFINE_TSVD_POSITION_H_ */
#if defined(BELLE_NAMESPACE)
namespace Belle {
#endif
#if defined(BELLE_NAMESPACE)
} // namespace Belle
#endif
#endif
