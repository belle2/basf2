
#if 0 // old

//#include "panther/panther.h"


#include "tracking/modules/trasan/TSvdPosition.h"

#include "belleCLHEP/Vector/Rotation.h"
#include "CLHEP/Vector/ThreeVector.h"


#if defined(BELLE_NAMESPACE)
namespace Belle {
#endif


/// returns local x
  double
  tsvd_localx(int DSSD, double lsa)
  {
    // inner side ? p(rphi)side->0, n(z)side->1
    int side = tsvd_side(DSSD);
    if (side < 0) {
      std::cout << " Warning :: Wrong parameter assignment in  tsvd_localx " << std::endl;
      return DBL_MAX;
    }

    //forward or backward? forward->0, backward->1
    int pos = tsvd_dssd2hladder(DSSD) % 2;

    if (pos == side) {
      // (forward && rphi is inner) or ( backward && rphi is outer)
      return (0.0050 * (320.0 - lsa));
    } else {
      // (forward && rphi is outer) or ( backward && rphi is inner)
      //return ( 0.0050 * (319.0 - lsa) );
      return (0.0050 * (320.0 - lsa));
    }

    std::cout << " Warning :: Wrong parameter assignment in  tsvd_localx " << std::endl;
    return DBL_MAX;
  }

/// returns local z
  double
  tsvd_localz(int DSSD, double lsa)
  {
    int side = tsvd_side(DSSD);

    switch (side) {
      case 0:
        return (0.0084 * (319.5 - lsa));
        break;
      case 1:
        return (-0.0084 * (319.5 - lsa));
        break;
      default:
        std::cout << " Warning :: Wrong parameter assignment in  tsvd_localz " << std::endl;
        return DBL_MAX;
        break;
    }

    std::cout << " Warning :: Wrong parameter assignment in  tsvd_localz " << std::endl;
    return DBL_MAX;
  }

/// returns local position
  HepGeom::Point3D<double>
  tsvd_localposition(int DSSD, double philsa, double zlsa)
  {
    return HepGeom::Point3D<double> (tsvd_localx(DSSD, philsa), 0.0, tsvd_localz(DSSD, zlsa));
  }

/// returns transformation flom DSSD local frame to global frame
  HepTransform3D
  tsvd_toglobal(int DSSD)
  {
    Alignsvd_dssd_Manager& alignMgr = Alignsvd_dssd_Manager::get_manager();
    Geosvd_dssd_Manager& geoMgr = Geosvd_dssd_Manager::get_manager();

    Alignsvd_dssd& align = alignMgr[DSSD];
    Geosvd_dssd& geo = geoMgr[DSSD];

    // local rotation and shift
    HepRotation lrotation;
    lrotation.rotateZ(align.zrot());
    lrotation.rotateX(align.xrot());
    lrotation.rotateY(align.yrot());

    CLHEP::Hep3Vector lshift(align.xshf(), align.yshf(), align.zshf());

    // local transformation
    HepTransform3D ltrans(lrotation, lshift);

    // global rotation and shift
    HepRotation grotation;
    CLHEP::Hep3Vector gshift(geo.dx(), geo.r(), geo.z());
    // The unit of phi in GeoSVD_DSSD is degree!!
    double phi = geo.phi() * M_PI / 180.0;
    gshift.rotateZ(phi);
    grotation.rotateZ(phi);

    // global transformation
    HepTransform3D gtrans(grotation, gshift);

    // first local transformation -> global transformation
    return gtrans * ltrans;
  }

/// returns global position (includes alignment)
  HepGeom::Point3D<double>
  tsvd_position(int DSSD, double philsa, double zlsa)
  {
    return tsvd_toglobal(DSSD) * tsvd_localposition(DSSD, philsa, zlsa);
  }

/// returns side of a DSSD. (0:phi-side is inner side, 1:z-side is inner side)
/// This should be replaced with table "svd_map".
  int
  tsvd_side(int DSSD)
  {
    if (DSSD < 0 || DSSD >= 102) {
      return -1;
    } else if (DSSD < 16) {
      return ((DSSD + 1) % 2);
    } else if (DSSD < 46) {
      return ((DSSD - 16) % 3 == 1) ? 0 : 1;
    } else {
      return ((DSSD - 46 + 1) % 2);
    }

    return -1;
  }

/// returns half-ladder # of a DSSD.
/// this shoul be replaced with Tomura-kun's function.
  int
  tsvd_dssd2hladder(int DSSD)
  {
    if (DSSD < 0 || DSSD >= 102) {
      return -1;
    } else if (DSSD < 16) {
      return DSSD;
    } else if (DSSD < 46) {
      return ((DSSD - 16) % 3) == 0 ? int((DSSD - 16) / 3) * 2 + 16 : int((DSSD - 16) / 3) * 2 + 17 ;
    } else {
      return int(DSSD / 2) + 13;
    }

    return -1;

  }

  int
  TRLA2PhiDSSD(int rla)
  {
    if (0 <= rla && rla < 20480) {
      int tmp = rla / N_channel_h;
      if (tmp % 4 == 1 || tmp % 4 == 2) return tmp / 2;
      else return -1;
    } else if (20480 <= rla && rla < 46080) {
      int tmp = (rla - 20480) / N_channel_h;
      if (tmp % 4) return (tmp / 4) * 3 + tmp % 4 + 15;
      else return -1;
    } else if (46080 <= rla && rla < N_channel) {
      int tmp = (rla - 46080) / N_channel_h;
      return tmp - (tmp % 2) * 2 + 47;
    } else return -1;
  }

  int
  TRLA2ZDSSD(int rla)
  {
    if (0 <= rla && rla < 20480) {
      int tmp = rla / N_channel_h;
      if (tmp % 4 == 0 || tmp % 4 == 3) return tmp / 2;
      else return -1;
    } else if (20480 <= rla && rla < 46080) {
      int tmp = (rla - 20480) / N_channel_h;
      if (tmp % 4 == 0) return (tmp / 4) * 3 + 16;
      else if (tmp % 4 == 2 || tmp % 4 == 3) return (tmp / 4) * 3 - tmp % 4 + 20;
      else return -1;
    } else if (46080 <= rla && rla < N_channel) {
      int tmp = (rla - 46080) / N_channel_h;
      return tmp + 46;
    } else return -1;
  }

#if defined(BELLE_NAMESPACE)
} // namespace Belle
#endif
#endif
