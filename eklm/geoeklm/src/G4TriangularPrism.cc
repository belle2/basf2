/*************************************************************************
*  BASF2 (Belle Analysis Framework 2)                                    *
*  Copyright(C) 2010 - Belle II Collaboration                            *
*                                                                        *
*  Author: The Belle II Collaboration                                    *
*  Contributors: Kirill Chilikin                                         *
*                                                                        *
*  This software is provided "as is" without any warranty.               *
* ***********************************************************************/

#include <eklm/geoeklm/G4TriangularPrism.h>
#include <cmath>

using namespace std;
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
  m_is = NULL;
  m_tube = new(nothrow) G4Tubs("Tube_" + name, 0., max(r1, r2), halfZlen,
                               phi1, fabs(phi2 - phi1));
  if (m_tube == NULL)
    return;
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
  m_box = new(nothrow) G4Box("Box_" + name, rl * sin_alpha,
                             rl * cos_alpha, halfZlen);
  if (r1 >= r2)
    t = G4RotateZ3D((phi1 + alpha) * rad - 90.0 * deg);
  else
    t = G4RotateZ3D((phi2 - alpha) * rad + 90.0 * deg);
  if (m_box == NULL)
    return;
  m_is = new(nothrow) G4IntersectionSolid(name, m_tube, m_box, t);
}

G4TriangularPrism::~G4TriangularPrism()
{
}

G4VSolid* G4TriangularPrism::getSolid()
{
  return m_is;
}

