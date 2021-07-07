/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 * External Contributor: Wouter Hulsbergen                                *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/


#include <analysis/VertexFitting/TreeFitter/Projection.h>

namespace TreeFitter {

  Projection::Projection(int dimP, int dimC) :
    m_H(dimC, dimP),
    m_V(dimC, dimC),
    m_residual(dimC),
    m_dimCov(dimC),
    m_dimProj(dimP),
    m_offset(0) {}

  void Projection::resetProjection()
  {
    m_residual = Eigen::Matrix < double, -1, 1, 0, 7, 1 >::Zero(m_dimCov);
    m_H = Eigen::Matrix < double, -1, -1, 0, 7, MAX_MATRIX_SIZE >::Zero(m_dimCov, m_dimProj);
    m_V = Eigen::Matrix < double, -1, -1, 0, 7, 7 >::Zero(m_dimCov, m_dimCov);
    m_offset = 0;
  }

}
