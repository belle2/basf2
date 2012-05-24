
#if 0 // old
#include <math.h>
#include <iostream>
#include "tracking/modules/trasan/TSvdHitsOnDssd.h"

#include "tracking/modules/trasan/TSvdPosition.h"


#if defined(BELLE_NAMESPACE)
namespace Belle {
#endif


  TSvdHitsOnDssd::TSvdHitsOnDssd()
  {
    m_id = -1;
    m_phi = m_r = m_dx = m_z = 0.;
    for (int i = 0; i < 3; ++i) {
      m_shift[i] = 0.;
      for (int j = 0; j < 3; ++j) {
        m_rot[i][j] = 0.;
      }
    }
    for (int i = 0; i < 2; ++i) {
      m_direction[i] = 0;
      m_rla[i] = 0;
      m_hybrid[i] = -1;
    }
  }

  TSvdHitsOnDssd::TSvdHitsOnDssd(Geosvd_dssd_Manager& geoMgr,
                                 Alignsvd_dssd_Manager& alMgr,
                                 int id)
  {
    m_id = id;

    m_phi = geoMgr[id].phi() * M_PI / 180.;
    m_r   = geoMgr[id].r();
    m_dx  = geoMgr[id].dx();
    m_z   = geoMgr[id].z();

    double xrot = alMgr[id].xrot();
    double yrot = alMgr[id].yrot();
    double zrot = alMgr[id].zrot();

    double sin_x = sin(xrot);
    double cos_x = cos(xrot);
    double sin_y = sin(yrot);
    double cos_y = cos(yrot);
    double sin_z = sin(zrot);
    double cos_z = cos(zrot);

    // m_rot = Hy*Hx*Hz
    m_rot[0][0] =  cos_y * cos_z + sin_x * sin_y * sin_z;
    m_rot[0][1] = -cos_y * sin_z + sin_x * sin_y * cos_z;
    m_rot[0][2] =  cos_x * sin_y;

    m_rot[1][0] =  cos_x * sin_z;
    m_rot[1][1] =  cos_x * cos_z;
    m_rot[1][2] = -sin_x;

    m_rot[2][0] = -sin_y * cos_z + sin_x * cos_y * sin_z;
    m_rot[2][1] =  sin_y * sin_z + sin_x * cos_y * cos_z;
    m_rot[2][2] =  cos_x * cos_y;

    m_shift[0] = alMgr[id].xshf();
    m_shift[1] = alMgr[id].yshf();
    m_shift[2] = alMgr[id].zshf();

    if ((geoMgr[id].half()) % 2 == geoMgr[id].side()) {
      m_direction[0] = -1;
    } else {
      m_direction[0] = 1;
    }
    m_direction[1] = 1;

    if (0 <= id && id < 16) {
      m_hybrid[0] = id * 2 + ((id + 1) % 2);
      m_hybrid[1] = id * 2 + (id % 2);

      m_rla[0] = m_hybrid[0] * 640;
      m_rla[1] = m_hybrid[1] * 640;
    } else if (16 <= id && id < 46) {
      m_hybrid[0] = ((id + 8) / 3) * 4 + ((id - 1) % 3 + 1);
      m_hybrid[1] = ((id + 10) / 3) * 4 - ((id - 1) % 3);

      m_rla[0] = m_hybrid[0] * 640;
      m_rla[1] = m_hybrid[1] * 640;
    } else if (46 <= id && id < 102) {
      m_hybrid[0] = (id + 27) - (id % 2) * 2;
      m_hybrid[1] = id + 26;

      m_rla[0] = m_hybrid[0] * 640;
      m_rla[1] = m_hybrid[1] * 640;
    } else {
      m_rla[0] = m_rla[1] = 0;
    }
  }

  double
  TSvdHitsOnDssd::phiGlobal(double lsa, int index)
  {
    if (index != 0 && index != 1)return 0.;
    double pglobal[2];
    // 320 is average value.
    HepGeom::Point3D<double>  x = tsvd_position(m_id, lsa, 320.);
    pglobal[0] = x.x();
    pglobal[1] = x.y();
    return pglobal[index];
  }

  double
  TSvdHitsOnDssd::zGlobal(double lsa)
  {
    // 320 is average value.
    HepGeom::Point3D<double>  x = tsvd_position(m_id, 320., lsa);
    return x.z();
  }

  double
  TSvdHitsOnDssd::phiLocal(double lsa, int index)
  {
    std::cout << "not supports....." << std::endl;
    if (index == 0)
      return -(lsa - 321.) * 0.0050; // x
    else
      return 0.; // y
  }

  double
  TSvdHitsOnDssd::zLocal(double lsa)
  {
    std::cout << "not supports....." << std::endl;
    if ((0 <= m_id && m_id <= 18 && (m_id % 2) == 0) ||
        m_id == 19 || m_id == 22 || m_id == 25 || m_id == 28 || m_id == 31 ||
        m_id == 34 || m_id == 37 || m_id == 40 || m_id == 43 ||
        m_id == 21 || m_id == 24 || m_id == 27 || m_id == 30 || m_id == 33 ||
        m_id == 36 || m_id == 39 || m_id == 42 || m_id == 45 ||
        (46 <= m_id && m_id <= 100 && (m_id % 2) == 0)) {
      //...forward
      //  0,  2,  4,  6,  8, 10, 12, 14
      // 16, 19, 22, 25, 28, 31, 34, 37, 40, 43
      // 18, 21, 24, 27, 30, 33, 36, 39, 42, 45
      // 46, 50, 54, 58, 62, 66, 70, 74, 78, 82, 86, 90, 94, 98
      // 48, 52, 56, 60, 64, 68, 72, 76, 80, 84, 88, 92, 96,100
      return (lsa - 320.5) * 0.0084;
    } else {
      //...backward
      return - (lsa - 320.5) * 0.0084;
    }
  }

  TSvdHitsOnDssd&
  TSvdHitsOnDssd::operator=(const TSvdHitsOnDssd& dssd)
  {
    if (this == &dssd) return *this;

    m_id  = dssd.m_id;
    m_phi = dssd.m_phi;
    m_dx  = dssd.m_dx;
    m_r   = dssd.m_r;
    m_z   = dssd.m_z;

    for (int i = 0; i < 3; ++i) {
      m_shift[i] = dssd.m_shift[i];
      for (int j = 0; j < 3; ++j) {
        m_rot[i][j] = dssd.m_rot[i][j];
      }
      if (i < 2) {
        m_direction[i] = dssd.m_direction[i];
        m_rla[i]       = dssd.m_rla[i];
        m_hybrid[i]    = dssd.m_hybrid[i];
      }
    }

    return *this;
  }

  void
  TSvdHitsOnDssd::dumpDssd(void)
  {
    std::cout << std::endl;
    std::cout << "-----DSSD information-----" << std::endl;
    std::cout << " DSSD ID  = " << m_id;
    std::cout << " phi = " << m_phi;
    std::cout << " dx  = " << m_dx;
    std::cout << " r   = " << m_r;
    std::cout << " z   = " << m_z << std::endl;
    std::cout << " ReadOut<->RLA Direction phi = " << m_direction[0];
    std::cout << " z = " << m_direction[1] << std::endl;
    std::cout << " First RLA# phi = " << m_rla[0];
    std::cout << " z = " << m_rla[1] << std::endl;
    std::cout << " Hybrid phi# = " << m_hybrid[0];
    std::cout << " z#   = " << m_hybrid[1] << std::endl;
    std::cout << std::endl;
  }

  void
  TSvdHitsOnDssd::dumpClusters(void)
  {
    std::cout << std::endl;
    std::cout << "-----DSSD information-----" << std::endl;
    std::cout << " DSSD ID  = " << m_id << std::endl;
    std::cout << " Clusters : " << std::endl;
    std::cout << "      phi : ";
    for (int i = 0; i < phiClusters.length(); ++i) {
      if (i != 0)std::cout << "            ";
      std::cout << phiClusters[i]->get_ID() << ", ("
                << phiGlobal(phiClusters[i]->lsa(), 0) << ","
                << phiGlobal(phiClusters[i]->lsa(), 1) << ") : Electrons = " << phiClusters[i]->electrons() << std::endl;
    }
    std::cout << "      z   : ";
    for (int i = 0; i < zClusters.length(); ++i) {
      if (i != 0)std::cout << "            ";
      std::cout << zClusters[i]->get_ID() << ", " << zGlobal(zClusters[i]->lsa())
                << " : Electrons = " << zClusters[i]->electrons() << std::endl;
    }
    std::cout << std::endl;
  }

  HepGeom::Point3D<double>
  TSvdHitsOnDssd::x(int phiIndex, int zIndex)
  {
    HepGeom::Point3D<double>  t(9999., 9999., 9999.);

    if (phiIndex >= (int)phiClusters.length() ||
        zIndex >= (int)zClusters.length())return t;
    t = tsvd_position(m_id, phiClusters[phiIndex]->lsa(), zClusters[zIndex]->lsa());
    return t;
  }

  HepGeom::Point3D<double>
  TSvdHitsOnDssd::x(int index)
  {
    HepGeom::Point3D<double>  t(9999., 9999., 9999.);

    if (index >= (int)(phiClusters.length()*zClusters.length()))return t;

    int phiIndex = index / (int)zClusters.length();
    int zIndex   = index % (int)zClusters.length();

    return x(phiIndex, zIndex);
  }

#if defined(BELLE_NAMESPACE)
} // namespace Belle
#endif
#endif
