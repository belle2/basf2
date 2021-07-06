/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

/* Own header. */
#include <klm/eklm/geometry/G4TriangularPrism.h>

/* CLHEP headers. */
#include <CLHEP/Units/SystemOfUnits.h>

/* C++ headers. */
#include <cmath>
#include <new>

using namespace Belle2;

G4TriangularPrism::G4TriangularPrism(const G4String& name,
                                     double r1, double phi1,
                                     double r2, double phi2,
                                     G4double halfZlen)
{
  double rl;
  double rs;
  double delta_phi;
  double tg_alpha;
  double alpha;
  double sin_alpha;
  double cos_alpha;
  G4Transform3D t;
  m_is = nullptr;
  try {
    m_tube = new G4Tubs("Tube_" + name, 0., std::max(r1, r2), halfZlen,
                        phi1, fabs(phi2 - phi1));
  } catch (std::bad_alloc& ba) {
    goto err_mem1;
  }
  if (r1 >= r2) {
    rl = r1;
    rs = r2;
  } else {
    rl = r2;
    rs = r1;
  }
  delta_phi = phi2 - phi1;
  tg_alpha = (rl / rs - cos(delta_phi)) / sin(delta_phi);
  alpha = atan(tg_alpha);
  sin_alpha = tg_alpha / sqrt(1.0 + tg_alpha * tg_alpha);
  cos_alpha = 1.0 / sqrt(1.0 + tg_alpha * tg_alpha);
  try {
    m_box = new G4Box("Box_" + name, rl * sin_alpha,
                      rl * cos_alpha, halfZlen);
  } catch (std::bad_alloc& ba) {
    goto err_mem2;
  }
  if (r1 >= r2)
    t = G4RotateZ3D((phi1 + alpha) * CLHEP::rad - 90.0 * CLHEP::deg);
  else
    t = G4RotateZ3D((phi2 - alpha) * CLHEP::rad + 90.0 * CLHEP::deg);
  try {
    m_is = new G4IntersectionSolid(name, m_tube, m_box, t);
  } catch (std::bad_alloc& ba) {
    goto err_mem3;
  }
  return;
err_mem3:
  delete m_box;
err_mem2:
  delete m_tube;
err_mem1:
  throw (std::bad_alloc());
}

G4TriangularPrism::~G4TriangularPrism()
{
}
