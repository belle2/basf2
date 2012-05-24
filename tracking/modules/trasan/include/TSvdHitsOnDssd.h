#if 0 // old
#ifndef _DEFINE_TSVD_HITS_ON_DSSD_H_
#define _DEFINE_TSVD_HITS_ON_DSSD_H_



#define HEP_SHORT_NAMES
#include "tracking/modules/trasan/AList.h"
#ifndef CLHEP_POINT3D_H
#include "CLHEP/Geometry/Point3D.h"
#endif
#if defined(BELLE_NAMESPACE)
namespace Belle {
#endif

  class TSvdHitsOnDssd {
  public:
    // constructors
    TSvdHitsOnDssd();
    TSvdHitsOnDssd(Geosvd_dssd_Manager& geoMgr, Alignsvd_dssd_Manager& alMgr, int id);

    // destructor
    virtual ~TSvdHitsOnDssd() {}

    // extracters
    int id(void) { return m_id; }
    int hybrid(int side) { return m_hybrid[side]; }
    double phi(void) { return m_phi; }
    double r(void) { return m_r; }
    double dx(void) { return m_dx; }

    int direction(int id) { return m_direction[id]; }
    int rla(int id) { return m_rla[id]; }
    int phiRla(void) { return m_rla[0]; }
    int zRla(void) { return m_rla[1]; }

    int phiPatterns(void) { return phiClusters.length(); }
    int zPatterns(void) { return zClusters.length(); }

    int patterns(void) { return phiClusters.length() * zClusters.length(); }

    Recsvd_cluster& phiCluster(int i) { return *phiClusters[i]; }
    Recsvd_cluster& zCluster(int i) { return *zClusters[i]; }

    HepGeom::Point3D<double>  x(int phiIndex, int zIndex);
    HepGeom::Point3D<double>  x(int index = 0);

    // calculate global position
    double phiGlobal(double lsa, int index = 0); // returns x,y
    double zGlobal(double lsa); // returns z

    // caluculate local position
    double phiLocal(double lsa, int index = 0);
    double zLocal(double lsa);

    // copy operator
    TSvdHitsOnDssd& operator = (const TSvdHitsOnDssd&);

    // modify
    void phiCluster(Recsvd_cluster* c) { phiClusters.append(c); }
    void zCluster(Recsvd_cluster* c) { zClusters.append(c); }

    void removePhiCluster(Recsvd_cluster* c) { phiClusters.remove(c); }
    void removeZCluster(Recsvd_cluster* c) { zClusters.remove(c); }

    // dump
    void dumpClusters(void);
    void dumpDssd(void);

  private:
    int m_id; // DSSD ID #
    double m_phi; // global PHI
    double m_dx;  // global x shift
    double m_r;   // global r
    double m_z;   // global z;
    double m_rot[3][3]; // local rotation matrix
    double m_shift[3];  // local shift parameter
    int m_direction[2]; // readout <-> RLA direction,       0:phi,1:z
    int m_rla[2];       // first RLA number of phi(z) side, 0:phi,1:z
    int m_hybrid[2];    // Hybrid #,                        0:phi,1:z

    AList<Recsvd_cluster> phiClusters;
    AList<Recsvd_cluster> zClusters;
  };


#if defined(BELLE_NAMESPACE)
} // namespace Belle
#endif
#endif /* _DEFINE_TSVD_HITS_ON_DSSD_H_ */
#endif // if 0
