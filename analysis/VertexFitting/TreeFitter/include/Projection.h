/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 * External Contributor: Wouter Hulsbergen                                *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <analysis/VertexFitting/TreeFitter/EigenStackConfig.h>

#include <Eigen/Dense>

namespace TreeFitter {

  /** class to store the projected residuals and the corresponding jacobian as well as the covariance matrix in the system of residuals */
  class Projection {

  public:

    /** constructor */
    Projection(int dimP, int dimC);

    /** destructor */
    virtual ~Projection() {};

    /** get projection matrix */
    Eigen::Matrix < double, -1, -1, 0, 7, MAX_MATRIX_SIZE > & getH() {return m_H;}

    /** get projection matrix */
    const Eigen::Matrix < double, -1, -1, 0, 7, MAX_MATRIX_SIZE > getH() const {return m_H;}

    /** get residuals */
    Eigen::Matrix < double, -1, 1, 0, 7, 1 > & getResiduals() {return m_residual;}

    /** get residuals */
    const Eigen::Matrix < double, -1, 1, 0, 7, 1 > getResiduals() const {return m_residual;}

    /** ge element in residuals */
    double getElementInResiduals(int row) { return m_residual(m_offset + row, 1); }

    /** get residuals cov matrix */
    Eigen::Matrix < double, -1, -1, 0, 7, 7 > & getV() {return m_V;}

    /** get residuals cov matrix */
    const Eigen::Matrix < double, -1, -1, 0, 7, 7 > & getV() const {return m_V;}

    /** reset projection */
    void resetProjection();

    /**  get offset (for mergedcosntraints)*/
    unsigned int getOffset() { return m_offset; }

    /** increment offset */
    void incrementOffset(unsigned int i) { m_offset += i; }

    /** get offset */
    unsigned int offset() const { return m_offset; }

    /**get size */
    int getDim() const { return m_dimProj; }

    /**get size */
    int getDimCov() const { return m_dimCov; }
  private:

    /** jacobian of the constraint, converts x,p,E<->system of residuals */
    Eigen::Matrix < double, -1, -1, 0, 7, MAX_MATRIX_SIZE > m_H;

    /** covariance matrix of this constraint (r') */
    Eigen::Matrix < double, -1, -1, 0, 7, 7 > m_V;

    /** residuals of constraint */
    Eigen::Matrix < double, -1, 1, 0, 7, 1 > m_residual;

    /** dimension of the covariance */
    int m_dimCov;

    /** dimension of H */
    int m_dimProj;

    /** offset for constraint index. used when there's more then one constraint projected */
    unsigned int m_offset;
  };
}
