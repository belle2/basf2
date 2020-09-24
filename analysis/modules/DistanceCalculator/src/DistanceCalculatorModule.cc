/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2019 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Sourav Dey, Abi Soffer                                   *
 * Past Contributors: Omer Benami                                         *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <analysis/modules/DistanceCalculator/DistanceCalculatorModule.h>

#include <Eigen/Dense>
#include <iostream>

using namespace std;
using namespace Belle2;

const float eps = 1E-6;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(DistanceCalculator)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

DistanceCalculatorModule::DistanceCalculatorModule() : Module(),
  m_distanceCovMatrix(0)
{
  // Set module properties
  setDescription("Calculates distance between two vertices, distance of closest approach between a vertex and a track, distance of closest approach between two tracks, distance of closest approach between a vertex/track and Btube");

  // Parameter definitions
  addParam("listName", m_listName, "", std::string(""));
  addParam("decayString", m_decayString, "", std::string(""));
  addParam("mode", m_mode,
           "Specifies how the distance is calculated:\n"
           "vertextrack: calculate the distance of closest approach between a track and a vertex, taking the first candidate as vertex, default\n"
           "trackvertex: calculate the distance of closest approach between a track and a vertex, taking the first candidate as track,\n"
           "2tracks: calculates the distance of closest approach between two tracks,\n"
           "2vertices: calculates the distance between two vertices,\n"
           "vertexbtube: calculates the distance of closest approach between a vertex and a Btube,\n"
           "trackbtube: calculates the distance of closest approach between a track and a Btube",
           std::string("vertextrack"));

  m_distance = Eigen::Matrix<double, Eigen::Dynamic, 1>::Zero(3);
}

DistanceCalculatorModule::~DistanceCalculatorModule()
{
}

void DistanceCalculatorModule::initialize()
{
  if (m_decayString != "") {
    m_decayDescriptor.init(m_decayString);
  } else {
    B2FATAL("Please provide a decay string for DistanceCalculatorModule");
  }
  m_plist.isRequired(m_listName);
}

Eigen::Vector3d getDocaTrackVertex(const Particle* pTrack, const Particle* pVertex)
{
  Eigen::Vector3d v(pTrack->getPx(), pTrack->getPy(), pTrack->getPz());
  Eigen::Vector3d p_ray(pTrack->getX(), pTrack->getY(), pTrack->getZ());
  Eigen::Vector3d p(pVertex->getX(), pVertex->getY(), pVertex->getZ());
  Eigen::Vector3d v_dir = v.normalized();
  Eigen::Vector3d r = p - p_ray;
  return r.dot(v_dir) * v_dir - r;
}

TMatrixFSym getDocaTrackVertexError(const Particle* pTrack, const Particle* pVertex)
{
  //covariance matrix of r
  TMatrixFSym err_r = pTrack->getVertexErrorMatrix() + pVertex->getVertexErrorMatrix();
  Eigen::Vector3d V(pTrack->getPx(), pTrack->getPy(), pTrack->getPz());
  Eigen::Vector3d v = V.normalized();
  //d_j = r_j - v_j * v_k r_k
  //Jij = del_i d_j = delta_ij - v_i * v_j
  //Since the vector of closest approach is a linear function of r, its
  //propagation of errors is exact
  TMatrixFSym Jacobian(3);
  //Fill the jacobian matrix according to the equation:
  // J_ij = delta_ij -v(i)v(j)
  for (int i = 0; i < 3; i++)
    for (int j = 0; j < 3; j++)
      Jacobian(i, j) = -v(i) * v(j);
  for (int i = 0; i < 3; i++)
    Jacobian(i, i) += 1;

  return err_r.Similarity(Jacobian);  //calculates J * err_r * J^T, and returns it
}

/*
 * This method calculates the Doca vector between two tracks (i.e. lines)
 * This is done using the following equation:
 * d = (n*(x1-x2))n
 * where d is the doca vector, n is a unit vector that is orthogonal to both lines,
 * and x1 and x2 are points on the first and second line respectively
 */
Eigen::Vector3d getDocaTracks(const Particle* p1, const Particle* p2)
{
  Eigen::Vector3d v1(p1->getPx(), p1->getPy(), p1->getPz());
  Eigen::Vector3d v2(p2->getPx(), p2->getPy(), p2->getPz());
  Eigen::Vector3d n = v1.cross(v2); //The doca vector must be orthogonal to both lines.
  if (n.norm() < eps) {
    //if tracks are parallel then the distance is independent of the point on the line
    //and we can use the old way to calculate the DOCA
    return getDocaTrackVertex(p1, p2);
  }
  Eigen::Vector3d n_dir = n.normalized();
  Eigen::Vector3d p1v(p1->getX(), p1->getY(), p1->getZ());
  Eigen::Vector3d p2v(p2->getX(), p2->getY(), p2->getZ());
  Eigen::Vector3d r = p1v - p2v;
  return r.dot(n_dir) * n_dir;
}

TMatrixFSym getDocaTracksError(const Particle* p1, const Particle* p2)
{
  //covariance matrix of r
  TMatrixFSym err_r = p1->getVertexErrorMatrix() + p2->getVertexErrorMatrix();
  Eigen::Vector3d p1p(p1->getPx(), p1->getPy(), p1->getPz());
  Eigen::Vector3d p2p(p2->getPx(), p2->getPy(), p2->getPz());
  Eigen::Vector3d n = p1p.cross(p2p);
  if (n.norm() < eps) {
    return getDocaTrackVertexError(p1, p2);
  }
  Eigen::Vector3d n_dir = n.normalized();
  //d_j = n_dir_j * n_dir_k r_k
  //Jij = del_i d_j = n_dir_i * n_dir_j
  //Since the vector of closest approach is a linear function of r, it's
  //propagation of errors is exact
  TMatrixFSym Jacobian(3);
  //Fill the jacobian matrix according to the equation:
  // J_ij = n(i)n(j)
  for (int i = 0; i < 3; i++)
    for (int j = 0; j < 3; j++)
      Jacobian(i, j) = n_dir(i) * n_dir(j);
  return err_r.Similarity(Jacobian); //calculates J * err_r * J^T, and returns it
}

Eigen::Vector3d getDistanceVertices(const Particle* p1, const Particle* p2)
{
  Eigen::Vector3d p1v(p1->getX(), p1->getY(), p1->getZ());
  Eigen::Vector3d p2v(p2->getX(), p2->getY(), p2->getZ());
  Eigen::Vector3d r = p2v - p1v;
  return r;
}

TMatrixFSym getDistanceVerticesErrors(const Particle* p1, const Particle* p2)
{
  TMatrixFSym err_r = p1->getVertexErrorMatrix() + p2->getVertexErrorMatrix();
  return err_r;
}

void DistanceCalculatorModule::getDistance(const Particle* p1, const Particle* p2)
{
  if (m_mode == "2tracks") {
    m_distance = getDocaTracks(p1, p2);
  }
  if (m_mode == "2vertices") {
    m_distance = getDistanceVertices(p1, p2);
  }
  if (m_mode == "vertextrack") {
    m_distance = getDocaTrackVertex(p2, p1);
  }
  if (m_mode == "trackvertex") {
    m_distance = getDocaTrackVertex(p1, p2);
  }
}

void DistanceCalculatorModule::getDistanceErrors(const Particle* p1, const Particle* p2)
{
  if (m_mode == "2tracks") {
    m_distanceCovMatrix = getDocaTracksError(p1, p2);
  }
  if (m_mode == "2vertices") {
    m_distanceCovMatrix = getDistanceVerticesErrors(p1, p2);
  }
  if (m_mode == "vertextrack") {
    m_distanceCovMatrix = getDocaTrackVertexError(p2, p1);
  }
  if (m_mode == "trackvertex") {
    m_distanceCovMatrix = getDocaTrackVertexError(p1, p2);
  }
}

Eigen::Vector3d getDocaBtubeVertex(const Particle* pVertex, const Btube* tube)
{
  Eigen::Vector3d v(tube->getTubeDirection()(0, 0), tube->getTubeDirection()(1, 0), tube->getTubeDirection()(2, 0));
  Eigen::Vector3d p_ray(tube->getTubeCenter()(0, 0), tube->getTubeCenter()(1, 0), tube->getTubeCenter()(2, 0));
  Eigen::Vector3d p(pVertex->getX(), pVertex->getY(), pVertex->getZ());
  Eigen::Vector3d v_dir = v.normalized();
  Eigen::Vector3d r = p - p_ray;
  return r.dot(v_dir) * v_dir - r;
}

TMatrixFSym getDocaBtubeVertexError(const Particle* pVertex, const Btube* tube)
{
  TMatrixFSym err_r = tube->getTubeCenterErrorMatrix() + pVertex->getVertexErrorMatrix();
  Eigen::Vector3d V(tube->getTubeDirection()(0, 0), tube->getTubeDirection()(1, 0), tube->getTubeDirection()(2, 0));
  Eigen::Vector3d v = V.normalized();
  TMatrixFSym Jacobian(3);
  for (int i = 0; i < 3; i++)
    for (int j = 0; j < 3; j++)
      Jacobian(i, j) = -v(i) * v(j);
  for (int i = 0; i < 3; i++)
    Jacobian(i, i) += 1;

  return err_r.Similarity(Jacobian);
}

Eigen::Vector3d getDocaBtubeTrack(const Particle* pTrack, const Btube* tube)
{
  Eigen::Vector3d v1(pTrack->getPx(), pTrack->getPy(), pTrack->getPz());
  Eigen::Vector3d v2 = tube->getTubeDirection();
  Eigen::Vector3d n = v1.cross(v2); //The doca vector must be orthogonal to both lines.
  if (n.norm() < eps) {
    //if tracks are parallel then the distance is independent of the point on the line
    //and we can use the old way to calculate the DOCA
    return getDocaBtubeVertex(pTrack, tube);
  }
  Eigen::Vector3d n_dir = n.normalized();
  Eigen::Vector3d pv(pTrack->getX(), pTrack->getY(), pTrack->getZ());
  Eigen::Vector3d p2v(tube->getTubeCenter()(0, 0), tube->getTubeCenter()(1, 0), tube->getTubeCenter()(2, 0));
  Eigen::Vector3d r = pv - p2v;
  return r.dot(n_dir) * n_dir;
}

TMatrixFSym getDocaBtubeTrackError(const Particle* pTrack, const Btube* tube)
{
  TMatrixFSym err_r = pTrack->getVertexErrorMatrix() + tube->getTubeCenterErrorMatrix();
  Eigen::Vector3d p1p(pTrack->getPx(), pTrack->getPy(), pTrack->getPz());
  Eigen::Vector3d p2p(tube->getTubeDirection()(0, 0), tube->getTubeDirection()(1, 0), tube->getTubeDirection()(2, 0));
  Eigen::Vector3d n = p1p.cross(p2p);
  if (n.norm() < eps) {
    return getDocaBtubeVertexError(pTrack, tube);
  }
  Eigen::Vector3d n_dir = n.normalized();
  TMatrixFSym Jacobian(3);
  for (int i = 0; i < 3; i++)
    for (int j = 0; j < 3; j++)
      Jacobian(i, j) = n_dir(i) * n_dir(j);
  return err_r.Similarity(Jacobian);
}

void DistanceCalculatorModule::getBtubeDistance(const Particle* p, const Btube* t)
{
  if (m_mode == "vertexbtube") {
    m_distance = getDocaBtubeVertex(p, t);
  }
  if (m_mode == "trackbtube") {
    m_distance = getDocaBtubeTrack(p, t);
  }
}

void DistanceCalculatorModule::getBtubeDistanceErrors(const Particle* p, const Btube* t)
{
  if (m_mode == "vertexbtube") {
    m_distanceCovMatrix = getDocaBtubeVertexError(p, t);
  }
  if (m_mode == "trackbtube") {
    m_distanceCovMatrix = getDocaBtubeTrackError(p, t);
  }
}

void DistanceCalculatorModule::event()
{
  unsigned int n = m_plist->getListSize();
  for (unsigned int i = 0; i < n; i++) {
    Particle* particle = m_plist->getParticle(i);
    std::vector<const Particle*> selectedParticles = m_decayDescriptor.getSelectionParticles(particle);
    if ((m_mode == "vertexbtube") || (m_mode == "trackbtube")) {
      if (selectedParticles.size() == 1) {
        const Btube* t = particle->getRelatedTo<Btube>();
        if (!t) {
          B2FATAL("No associated Btube found");
        }
        const Particle* p0 = selectedParticles[0];
        getBtubeDistance(p0, t);
        getBtubeDistanceErrors(particle, t);
      }
    } else {
      if (selectedParticles.size() == 2) {
        const Particle* p1 = selectedParticles[0];
        const Particle* p2 = selectedParticles[1];
        getDistance(p1, p2);
        getDistanceErrors(p1, p2);
      }
    }
    Eigen::Matrix3d DistanceCovMatrix_eigen;
    DistanceCovMatrix_eigen.resize(3, 3);
    for (int row = 0; row < 3; row++) {
      for (int column = 0; column < 3; column++) {
        DistanceCovMatrix_eigen(row, column) = m_distanceCovMatrix[row][column]; //converting from TMatrixFSym to Eigen
      }
    }
    float DistanceMag = m_distance.norm();
    double dist_err = 0;
    if (DistanceMag != 0) {
      Eigen::Vector3d dist_dir = m_distance.normalized();
      dist_err = sqrt(((dist_dir.transpose()) * (DistanceCovMatrix_eigen * dist_dir)).norm());
    }
    particle->writeExtraInfo("CalculatedDistance", DistanceMag);
    particle->writeExtraInfo("CalculatedDistanceError", dist_err);
    //The Distance vector always points from the "vertex" to the "track/btube", if a single vertex is involved. In case of two vertices, it points from the "first" vertex to the "second" vertex. For two tracks or one track and btube, the direction is along "first track x second track" or "track x btube"
    particle->writeExtraInfo("CalculatedDistanceVector_X", m_distance(0));
    particle->writeExtraInfo("CalculatedDistanceVector_Y", m_distance(1));
    particle->writeExtraInfo("CalculatedDistanceVector_Z", m_distance(2));
    //Save the Covariance Matrix
    particle->writeExtraInfo("CalculatedDistanceCovMatrixXX", m_distanceCovMatrix[0][0]);
    particle->writeExtraInfo("CalculatedDistanceCovMatrixXY", m_distanceCovMatrix[0][1]);
    particle->writeExtraInfo("CalculatedDistanceCovMatrixXZ", m_distanceCovMatrix[0][2]);
    particle->writeExtraInfo("CalculatedDistanceCovMatrixYX", m_distanceCovMatrix[1][0]);
    particle->writeExtraInfo("CalculatedDistanceCovMatrixYY", m_distanceCovMatrix[1][1]);
    particle->writeExtraInfo("CalculatedDistanceCovMatrixYZ", m_distanceCovMatrix[1][2]);
    particle->writeExtraInfo("CalculatedDistanceCovMatrixZX", m_distanceCovMatrix[2][0]);
    particle->writeExtraInfo("CalculatedDistanceCovMatrixZY", m_distanceCovMatrix[2][1]);
    particle->writeExtraInfo("CalculatedDistanceCovMatrixZZ", m_distanceCovMatrix[2][2]);
  }
}
