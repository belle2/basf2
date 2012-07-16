#include "analysis/particle/Momentum.h"

using namespace Belle2;

Momentum::Momentum()
  : m_momentum(),
    m_position(),
    m_error(7, 0),
    m_vertex(),
    m_vertexError(3, 0),
    m_decayVertex(),
    m_decayVertexError(3, 0)
{
}

Momentum::Momentum(const Momentum& p)
  : m_momentum(p.m_momentum),
    m_position(p.m_position),
    m_error(p.m_error),
    m_vertex(p.m_vertex),
    m_vertexError(p.m_vertexError),
    m_decayVertex(p.m_decayVertex),
    m_decayVertexError(p.m_decayVertexError)
{
}

Momentum::Momentum(const HepLorentzVector& p,
                   const HepSymMatrix& error)
  : m_momentum(p),
    m_position(),
    m_error(7, 0),
    m_vertex(),
    m_vertexError(3, 0),
    m_decayVertex(),
    m_decayVertexError(3, 0)
{
  if (error.num_row() == 4) {
    m_error.sub(1, error);
    //sets 0.0 in momentum position correlation elements.
    for (unsigned i = 0; i < 4; i++) {
      for (unsigned j = 4; j < 7; j++) {
        m_error[i][j] = 0.0;
      }
    }
    for (unsigned i = 4; i < 7; i++) {
      for (unsigned j = i; j < 7; j++) {
        m_error[i][j] = 0.0;
      }
    }
  } else {
    // TODO: write out an error message
    // cout(Debugout::WARN,"Momentum") << "Error in momentum(const HepLorentzVector&," << std::endl;
    // cout(Debugout::WARN,"Momentum") << "                  const HepSymMatrix&) of Mometum Class" << std::endl;
    // cout(Debugout::WARN,"Momentum") << "                        ~~~~~~~~~~~~~--- 4 x 4 error matrix!!! " << std::endl;
    //sets 0.0 in all elements.
    for (unsigned i = 0; i < 7; i++) {
      for (unsigned j = i; j < 7; j++) {
        m_error[i][j] = 0.0;
      }
    }
  }
}

/*
   Warning
 */
Momentum::Momentum(const Track& p, const double thisMass,
                   const Hep3Vector& newPivot)
  : m_momentum(p.getMomentum().Px(), p.getMomentum().Py(), p.getMomentum().Pz(),
               sqrt(p.getMomentum().Px()*p.getMomentum().Px() +
                    p.getMomentum().Py()*p.getMomentum().Py() +
                    p.getMomentum().Pz()*p.getMomentum().Pz() +
                    thisMass* thisMass)),
  m_error(7, 0),
  m_vertex(),
  m_vertexError(3, 0),
  m_decayVertex(),
  m_decayVertexError(3, 0)
{
  // fill the position ( = POCA)
  // What is the difference between position and vertex?
  m_position.setX(p.getPosition().X());
  m_position.setY(p.getPosition().Y());
  m_position.setZ(p.getPosition().Z());

  // fill the error matrix
  // m_error row          = px, py, pz, E, x, y, z
  // p.getErrorMatrix row = x, y, z, px, py, pz
  m_error[0][0] = p.getErrorMatrix()[3][3]; // px-px
  m_error[0][1] = p.getErrorMatrix()[3][4]; // px-py
  m_error[0][2] = p.getErrorMatrix()[3][5]; // px-pz
  m_error[1][1] = p.getErrorMatrix()[4][4]; // py-py
  m_error[1][2] = p.getErrorMatrix()[4][5]; // py-pz
  m_error[2][2] = p.getErrorMatrix()[5][5]; // pz-pz

  m_error[4][4] = p.getErrorMatrix()[0][0]; // x-x
  m_error[4][5] = p.getErrorMatrix()[0][1]; // x-y
  m_error[4][6] = p.getErrorMatrix()[0][2]; // x-z
  m_error[5][5] = p.getErrorMatrix()[1][1]; // y-y
  m_error[5][6] = p.getErrorMatrix()[1][2]; // y-z
  m_error[6][6] = p.getErrorMatrix()[2][2]; // z-z

  m_error[0][4] = p.getErrorMatrix()[3][0]; // px-x
  m_error[1][4] = p.getErrorMatrix()[4][0]; // py-x
  m_error[2][4] = p.getErrorMatrix()[5][0]; // pz-x
  m_error[0][5] = p.getErrorMatrix()[3][1]; // px-y
  m_error[1][5] = p.getErrorMatrix()[4][1]; // py-y
  m_error[2][5] = p.getErrorMatrix()[5][1]; // pz-y
  m_error[0][6] = p.getErrorMatrix()[3][2]; // px-z
  m_error[1][6] = p.getErrorMatrix()[4][2]; // py-z
  m_error[2][6] = p.getErrorMatrix()[5][2]; // pz-z

  // covariance E-(x,y,z) is set to 0
  m_error[3][4] = 0.0; // E-x
  m_error[3][5] = 0.0; // E-y
  m_error[3][6] = 0.0; // E-z

  // covariance E-(px,py,pz)
  // E = sqrt(px*px+py*py+pz*pz+m*m)
  // therfore covariance E:px
  // is given by
  // cov(px,E) = cov(px,px) * dE/dpx + cov(px,py) * dE/dpy + cov(px,pz) * dE/dpz
  // (similarly for covariance of E and py or pz)
  // and variance of E os given by
  // cov(E,E)  = cov(px,px) * |dE/dpx|^2 + cov(py,py) * |dE/dpy|^2 + cov(pz,pz) * |dE/dpz|^2
  //           + 2 * cov(px,py) * dE/dpx * dE/dpy
  //           + 2 * cov(px,pz) * dE/dpx * dE/dpz
  //           + 2 * cov(py,pz) * dE/dpy * dE/dpz

  double invE = 1.0 / m_momentum.t();
  double px   = m_momentum.x();
  double py   = m_momentum.y();
  double pz   = m_momentum.z();

  // cov(px,E)
  m_error[0][3] = invE * (m_error[0][0] * px + m_error[0][1] * py + m_error[0][2] * pz);
  // cov(py,E)
  m_error[1][3] = invE * (m_error[0][1] * px + m_error[1][1] * py + m_error[1][2] * pz);
  // cov(pz,E)
  m_error[2][3] = invE * (m_error[0][2] * px + m_error[1][2] * py + m_error[2][2] * pz);
  // cov(E,E)
  m_error[3][3] = invE * invE * (m_error[0][0] * px * px + m_error[1][1] * py * py + m_error[2][2] * pz * pz
                                 + 2 * m_error[0][1] * px * py
                                 + 2 * m_error[0][2] * px * pz
                                 + 2 * m_error[1][2] * py * pz);

  // Belle I below
#if 0
  int hyp = 4;
  if (thisMass < 0.005) { // e = 0.000511
    hyp = 0;
  } else if (thisMass < 0.110) { // mu = 0.1056
    hyp = 1;
  } else if (thisMass < 0.200) { // pi = 0.13956
    hyp = 2;
  } else if (thisMass < 0.5) { // K = 0.4936
    hyp = 3;
  }
  const Hep3Vector pivot(p.trk().mhyp(hyp).pivot_x(),
                         p.trk().mhyp(hyp).pivot_y(),
                         p.trk().mhyp(hyp).pivot_z());
  //HepMatrix  tmp_a(5,1);
  //tmp_a[0][0] = p.trk().mhyp(hyp).helix(0);
  //tmp_a[1][0] = p.trk().mhyp(hyp).helix(1);
  //tmp_a[2][0] = p.trk().mhyp(hyp).helix(2);
  //tmp_a[3][0] = p.trk().mhyp(hyp).helix(3);
  //tmp_a[4][0] = p.trk().mhyp(hyp).helix(4);
  //HepVector  a(tmp_a);
  HepVector  a(5);
  a[0] = p.trk().mhyp(hyp).helix(0);
  a[1] = p.trk().mhyp(hyp).helix(1);
  a[2] = p.trk().mhyp(hyp).helix(2);
  a[3] = p.trk().mhyp(hyp).helix(3);
  a[4] = p.trk().mhyp(hyp).helix(4);
  HepSymMatrix Ea(5, 0);
  Ea[0][0] = p.trk().mhyp(hyp).error(0);
  Ea[1][0] = p.trk().mhyp(hyp).error(1);
  Ea[1][1] = p.trk().mhyp(hyp).error(2);
  Ea[2][0] = p.trk().mhyp(hyp).error(3);
  Ea[2][1] = p.trk().mhyp(hyp).error(4);
  Ea[2][2] = p.trk().mhyp(hyp).error(5);
  Ea[3][0] = p.trk().mhyp(hyp).error(6);
  Ea[3][1] = p.trk().mhyp(hyp).error(7);
  Ea[3][2] = p.trk().mhyp(hyp).error(8);
  Ea[3][3] = p.trk().mhyp(hyp).error(9);
  Ea[4][0] = p.trk().mhyp(hyp).error(10);
  Ea[4][1] = p.trk().mhyp(hyp).error(11);
  Ea[4][2] = p.trk().mhyp(hyp).error(12);
  Ea[4][3] = p.trk().mhyp(hyp).error(13);
  Ea[4][4] = p.trk().mhyp(hyp).error(14);
  Helix helix(pivot, a, Ea);

  if (newPivot.x() != 0. ||
      newPivot.y() != 0. ||
      newPivot.z() != 0.) {
    helix.pivot(newPivot);
    m_momentum = helix.momentum(0., thisMass, m_position, m_error);
    return;
  } else {
    //...finds ref. point.
    if (pivot.x() != 0. ||
        pivot.y() != 0. ||
        pivot.z() != 0.) {
      //Point3D tmp(0.,0.,0.);
      //helix.pivot(tmp);
      helix.pivot(Hep3Vector(0., 0., 0.));
      m_momentum = helix.momentum(0., thisMass, m_position, m_error);
      return;
    } else {
      m_momentum = helix.momentum(0., thisMass, m_position, m_error);
      return;
    }
  }
#endif
}

Momentum::Momentum(const Track& p, const Ptype& ptype,
                   const Hep3Vector& newPivot)
  : m_error(7, 0),
    m_vertex(),
    m_vertexError(3, 0),
    m_decayVertex(),
    m_decayVertexError(3, 0)
{
  double thisMass = 0.;
  switch (abs(ptype.lund())) {
    case 11:
      thisMass = 0.000510999;
      break;
    case 13:
      thisMass = 0.1056584;
      break;
    case 321:
      thisMass = 0.493677;
      break;
    case 2212:
      thisMass = 0.9382720;
      break;
    default:
      thisMass = 0.139570;
  }

  m_momentum = HepLorentzVector(p.getMomentum().Px(), p.getMomentum().Py(), p.getMomentum().Pz(),
                                sqrt(p.getMomentum().Px() * p.getMomentum().Px() +
                                     p.getMomentum().Py() * p.getMomentum().Py() +
                                     p.getMomentum().Pz() * p.getMomentum().Pz() +
                                     thisMass * thisMass));


  // fill the position ( = POCA)
  // What is the difference between position and vertex?
  m_position.setX(p.getPosition().X());
  m_position.setY(p.getPosition().Y());
  m_position.setZ(p.getPosition().Z());

  // fill the error matrix
  // m_error row          = px, py, pz, E, x, y, z
  // p.getErrorMatrix row = x, y, z, px, py, pz
  m_error[0][0] = p.getErrorMatrix()[3][3]; // px-px
  m_error[0][1] = p.getErrorMatrix()[3][4]; // px-py
  m_error[0][2] = p.getErrorMatrix()[3][5]; // px-pz
  m_error[1][1] = p.getErrorMatrix()[4][4]; // py-py
  m_error[1][2] = p.getErrorMatrix()[4][5]; // py-pz
  m_error[2][2] = p.getErrorMatrix()[5][5]; // pz-pz
  m_error[4][4] = p.getErrorMatrix()[0][0]; // x-x
  m_error[4][5] = p.getErrorMatrix()[0][1]; // x-y
  m_error[4][6] = p.getErrorMatrix()[0][2]; // x-z
  m_error[5][5] = p.getErrorMatrix()[1][1]; // y-y
  m_error[5][6] = p.getErrorMatrix()[1][2]; // y-z
  m_error[6][6] = p.getErrorMatrix()[2][2]; // z-z
  m_error[0][4] = p.getErrorMatrix()[3][0]; // px-x
  m_error[1][4] = p.getErrorMatrix()[4][0]; // py-x
  m_error[2][4] = p.getErrorMatrix()[5][0]; // pz-x
  m_error[0][5] = p.getErrorMatrix()[3][1]; // px-y
  m_error[1][5] = p.getErrorMatrix()[4][1]; // py-y
  m_error[2][5] = p.getErrorMatrix()[5][1]; // pz-y
  m_error[0][6] = p.getErrorMatrix()[3][2]; // px-z
  m_error[1][6] = p.getErrorMatrix()[4][2]; // py-z
  m_error[2][6] = p.getErrorMatrix()[5][2]; // pz-z

  // covariance E-(x,y,z) is set to 0
  m_error[3][4] = 0.0; // E-x
  m_error[3][5] = 0.0; // E-y
  m_error[3][6] = 0.0; // E-z

  // covariance E-(px,py,pz)
  // E = sqrt(px*px+py*py+pz*pz+m*m)
  // therfore covariance E:px
  // is given by
  // cov(px,E) = cov(px,px) * dE/dpx + cov(px,py) * dE/dpy + cov(px,pz) * dE/dpz
  // (similarly for covariance of E and py or pz)
  // and variance of E os given by
  // cov(E,E)  = cov(px,px) * |dE/dpx|^2 + cov(py,py) * |dE/dpy|^2 + cov(pz,pz) * |dE/dpz|^2
  //           + 2 * cov(px,py) * dE/dpx * dE/dpy
  //           + 2 * cov(px,pz) * dE/dpx * dE/dpz
  //           + 2 * cov(py,pz) * dE/dpy * dE/dpz
  double invE = 1.0 / m_momentum.t();
  double px   = m_momentum.x();
  double py   = m_momentum.y();
  double pz   = m_momentum.z();

  // cov(px,E)
  m_error[0][3] = invE * (m_error[0][0] * px + m_error[0][1] * py + m_error[0][2] * pz);
  // cov(py,E)
  m_error[1][3] = invE * (m_error[0][1] * px + m_error[1][1] * py + m_error[1][2] * pz);
  // cov(pz,E)
  m_error[2][3] = invE * (m_error[0][2] * px + m_error[1][2] * py + m_error[2][2] * pz);
  // cov(E,E)
  m_error[3][3] = invE * invE * (m_error[0][0] * px * px + m_error[1][1] * py * py + m_error[2][2] * pz * pz
                                 + 2 * m_error[0][1] * px * py
                                 + 2 * m_error[0][2] * px * pz
                                 + 2 * m_error[1][2] * py * pz);


#if 0
  int hyp = 2;
  if (ptype.name() == "K+" || ptype.name() == "K-") {
    hyp = 3;
  } else if (ptype.name() == "E+" || ptype.name() == "E-") {
    hyp = 0;
  } else if (ptype.name() == "MU+" || ptype.name() == "MU-") {
    hyp = 1;
  } else if (ptype.name() == "P+" || ptype.name() == "AP+") {
    hyp = 4;
  }
  const Hep3Vector pivot(p.trk().mhyp(hyp).pivot_x(),
                         p.trk().mhyp(hyp).pivot_y(),
                         p.trk().mhyp(hyp).pivot_z());
  //HepMatrix  tmp_a(5,1);
  //tmp_a[0][0] = p.trk().mhyp(hyp).helix(0);
  //tmp_a[1][0] = p.trk().mhyp(hyp).helix(1);
  //tmp_a[2][0] = p.trk().mhyp(hyp).helix(2);
  //tmp_a[3][0] = p.trk().mhyp(hyp).helix(3);
  //tmp_a[4][0] = p.trk().mhyp(hyp).helix(4);
  //HepVector  a(tmp_a);
  HepVector  a(5);
  a[0] = p.trk().mhyp(hyp).helix(0);
  a[1] = p.trk().mhyp(hyp).helix(1);
  a[2] = p.trk().mhyp(hyp).helix(2);
  a[3] = p.trk().mhyp(hyp).helix(3);
  a[4] = p.trk().mhyp(hyp).helix(4);
  HepSymMatrix Ea(5, 0);
  Ea[0][0] = p.trk().mhyp(hyp).error(0);
  Ea[1][0] = p.trk().mhyp(hyp).error(1);
  Ea[1][1] = p.trk().mhyp(hyp).error(2);
  Ea[2][0] = p.trk().mhyp(hyp).error(3);
  Ea[2][1] = p.trk().mhyp(hyp).error(4);
  Ea[2][2] = p.trk().mhyp(hyp).error(5);
  Ea[3][0] = p.trk().mhyp(hyp).error(6);
  Ea[3][1] = p.trk().mhyp(hyp).error(7);
  Ea[3][2] = p.trk().mhyp(hyp).error(8);
  Ea[3][3] = p.trk().mhyp(hyp).error(9);
  Ea[4][0] = p.trk().mhyp(hyp).error(10);
  Ea[4][1] = p.trk().mhyp(hyp).error(11);
  Ea[4][2] = p.trk().mhyp(hyp).error(12);
  Ea[4][3] = p.trk().mhyp(hyp).error(13);
  Ea[4][4] = p.trk().mhyp(hyp).error(14);
  Helix helix(pivot, a, Ea);

  if (newPivot.x() != 0. ||
      newPivot.y() != 0. ||
      newPivot.z() != 0.) {
    helix.pivot(newPivot);
    m_momentum = helix.momentum(0., ptype.mass(), m_position, m_error);
    return;
  } else {
    if (pivot.x() != 0. ||
        pivot.y() != 0. ||
        pivot.z() != 0.) {
      //Point3D tmp(0.,0.,0.);
      //helix.pivot(tmp);
      helix.pivot(Hep3Vector(0., 0., 0.));
      m_momentum = helix.momentum(0., ptype.mass(), m_position, m_error);
      return;

    } else {
      m_momentum = helix.momentum(0., ptype.mass(), m_position, m_error);
      return;
    }
  }
#endif
}

Momentum::Momentum(const MdstGamma& p)
  : m_momentum(p.getpx(), p.getpy(), p.getpz(),
               sqrt(p.getpx()*p.getpx() + p.getpy()*p.getpy() + p.getpz()*p.getpz())),
  m_position(),
  m_error(7, 0),
  m_vertex(),
  m_vertexError(3, 0),
  m_decayVertex(),
  m_decayVertexError(3, 0)
{
}

Momentum::Momentum(const MdstPi0& p)
  : m_momentum(p.getpx(), p.getpy(), p.getpz(), p.getenergy()),
    m_position(),
    m_error(7, 0),
    m_vertex(),
    m_vertexError(3, 0),
    m_decayVertex(),
    m_decayVertexError(3, 0)
{
}

Momentum::Momentum(RecCRECL& p)
  : m_position(),
    m_error(7, 0),
    m_vertex(),
    m_vertexError(3, 0),
    m_decayVertex(),
    m_decayVertexError(3, 0)
{
  double en = p.GetEnergy();
  double th = p.GetTheta();
  double ph = p.GetPhi();
  double px = en * sin(th) * cos(ph);
  double py = en * sin(th) * sin(ph);
  double pz = en * cos(th);
  m_momentum = HepLorentzVector(px, py, pz, en);
}

double
Momentum::dMass(void) const
{
  // mass  = sqrt(E^2 - p^2)
  // dmass = (EdE - pxdpx - pydpy -pzdpz)/mass
  // dMass = dmass^2
  double E  = m_momentum.t();
  double px = m_momentum.x();
  double py = m_momentum.y();
  double pz = m_momentum.z();
  double main = E * E * m_error[3][3] + px * px * m_error[0][0] +
                py * py * m_error[1][1] + pz * pz * m_error[2][2];
  double sub  = E  * (px * m_error[0][3] + py * m_error[1][3] + pz * m_error[2][3]) -
                px * (py * m_error[0][1] + pz * m_error[0][2]) -
                py * pz * m_error[1][2];
  double mass = this->mass();
  if (mass != 0.)
    return (main - 2.*sub) / mass / mass;
  else
    return (main - 2.*sub) * 1.0e10; //temporary
}

void
Momentum::momentum(const HepLorentzVector& p, const HepSymMatrix& dp)
{
  m_momentum = p;
  if (dp.num_row() == 4) {
    m_error.sub(1, dp);
    //sets 0.0 in momentum position correlation elements.
    for (unsigned i = 0; i < 4; i++) {
      for (unsigned j = 4; j < 7; j++) {
        m_error[i][j] = 0.0;
      }
    }
  } else {
    // cout(Debugout::WARN,"Momentum") << "Error in momentum(const HepLorentzVector&," << std::endl;
    // cout(Debugout::WARN,"Momentum") << "                  const HepSymMatrix&) of Mometum Class" << std::endl;
    // cout(Debugout::WARN,"Momentum") << "                        ~~~~~~~~~~~~~--- 4 x 4 error matrix!!! " << std::endl;
    //sets 0.0 in all elements.
    for (unsigned i = 0; i < 7; i++) {
      for (unsigned j = i; j < 7; j++) {
        m_error[i][j] = 0.0;
      }
    }
  }
}

void
Momentum::position(const Hep3Vector& x, const HepSymMatrix& dx)
{
  m_position = x;
  if (dx.num_row() == 3) {
    m_error.sub(5, dx);
    //sets 0.0 in momentum position correlation elements.
    for (unsigned i = 0; i < 4; i++) {
      for (unsigned j = 4; j < 7; j++) {
        m_error[i][j] = 0.0;
      }
    }
  } else {
    // cout(Debugout::WARN,"Momentum") << "Error in position(const Hep3Vector&," << std::endl;
    // cout(Debugout::WARN,"Momentum") << "                  const HepSymMatrix&) of Mometum Class" << std::endl;
    // cout(Debugout::WARN,"Momentum") << "                        ~~~~~~~~~~~~~--- 3 x 3 error matrix!!! " << std::endl;
    //sets 0.0 in all elements.
    for (unsigned i = 0; i < 7; i++) {
      for (unsigned j = i; j < 7; j++) {
        m_error[i][j] = 0.0;
      }
    }
  }
}

void
Momentum::momentumPosition(const HepLorentzVector& p, const Hep3Vector& x,
                           const HepSymMatrix& dpx)
{
  m_momentum = p;
  m_position = x;
  if (dpx.num_row() == 7) {
    m_error = dpx;
  } else {
    // cout(Debugout::WARN,"Momentum") << "Error in momentumPosition(const HepLorentzVector&," << std::endl;
    // cout(Debugout::WARN,"Momentum") << "                          const Hep3Vector&,      " << std::endl;
    // cout(Debugout::WARN,"Momentum") << "                          const HepSymMatrix&) of Mometum Class" << std::endl;
    // cout(Debugout::WARN,"Momentum") << "                                ~~~~~~~~~~~~~--- 7 x 7 error matrix!!! " << std::endl;
    //sets 0.0 in all elements.
    for (unsigned i = 0; i < 7; i++) {
      for (unsigned j = i; j < 7; j++) {
        m_error[i][j] = 0.0;
      }
    }
  }
}

Hep3Vector&
Momentum::vertex(const Hep3Vector&   vertex,
                 const HepSymMatrix& error)
{
  m_vertex = vertex;
  if (error.num_row() == 3) {
    m_vertexError = error;
  } else {
    // cout(Debugout::WARN,"Momentum") << "Error in vertex(const Hep3Vector&," << std::endl;
    // cout(Debugout::WARN,"Momentum") << "                const HepSymMatrix&) of Mometum Class" << std::endl;
    // cout(Debugout::WARN,"Momentum") << "                      ~~~~~~~~~~~~~--- 3 x 3 error matrix!!! " << std::endl;
    //sets 0.0 in all elements.
    for (unsigned i = 0; i < 3; i++) {
      for (unsigned j = i; j < 3; j++) {
        m_vertexError[i][j] = 0.0;
      }
    }
  }
  return m_vertex;
}


Hep3Vector&
Momentum::decayVertex(const Hep3Vector&   vertex,
                      const HepSymMatrix& error)
{
  m_decayVertex = vertex;
  if (error.num_row() == 3) {
    m_decayVertexError = error;
  } else {
    // cout(Debugout::WARN,"Momentum") << "Error in decayVertex(const Hep3Vector&," << std::endl;
    // cout(Debugout::WARN,"Momentum") << "                     const HepSymMatrix&) of Mometum Class" << std::endl;
    // cout(Debugout::WARN,"Momentum") << "                           ~~~~~~~~~~~~~--- 3 x 3 error matrix!!! " << std::endl;
    //sets 0.0 in all elements.
    for (unsigned i = 0; i < 3; i++) {
      for (unsigned j = i; j < 3; j++) {
        m_decayVertexError[i][j] = 0.0;
      }
    }
  }
  return m_decayVertex;
}


Momentum&
Momentum::operator = (const Momentum& p)
{
  if (this == &p)return *this;

  m_momentum = p.m_momentum;
  m_position = p.m_position;
  m_error    = p.m_error;
  m_vertex   = p.m_vertex;
  m_vertexError      = p.m_vertexError;
  m_decayVertex      = p.m_decayVertex;
  m_decayVertexError = p.m_decayVertexError;
  return *this;
}

void
Momentum::dump(const std::string& keyword, const std::string& prefix) const
{
  bool full = false;
  if (keyword.find("full") != std::string::npos) full = true;



  // cout(Debugout::DUMP,"Momentum") << prefix;
  // cout(Debugout::DUMP,"Momentum") << "Momentum:";
  // if (full || keyword.find("mass")       != std::string::npos)cout(Debugout::DUMP,"Momentum") << " m=" << mass();
  // if (full || keyword.find("momentum")   != std::string::npos)cout(Debugout::DUMP,"Momentum") << " p=" << p();
  // if (full || keyword.find("position")   != std::string::npos)cout(Debugout::DUMP,"Momentum") << " x=" << x();
  // if (full || keyword.find("production") != std::string::npos)cout(Debugout::DUMP,"Momentum") << " ProductionV=" << vertex();
  // if (full || keyword.find("decay")      != std::string::npos)cout(Debugout::DUMP,"Momentum") << " decayV=" << decayVertex();
  // if (full || keyword.find("return")     != std::string::npos)cout(Debugout::DUMP,"Momentum") << std::endl;
}
