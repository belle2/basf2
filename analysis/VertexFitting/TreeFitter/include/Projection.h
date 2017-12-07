/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributor: Francesco Tenchini, Jo-Frederik Krohn                     *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

//Define a class to store projection matrix, covariance matrix and residuals

#ifndef PROJECTION_H
#define PROJECTION_H

#include <CLHEP/Matrix/Vector.h>
#include <CLHEP/Matrix/SymMatrix.h>
#include <CLHEP/Matrix/Matrix.h>
#include <analysis/VertexFitting/TreeFitter/EigenTypes.h>

#include <string.h> //fix for memset (FT)

namespace TreeFitter {

  class Projection {
  public:
    /** constructor */
    Projection(int dimP, int dimC);
    /** destructor */
    virtual ~Projection();
    /** get projection matrix */
    EigenTypes::MatrixXd& getH() {return m_H;}
    /** get projection matrix */
    const EigenTypes::MatrixXd getH() const {return m_H;}
    /** get residuals */
    EigenTypes::ColVector& getResiduals() {return m_residual;}
    //Eigen::Matrix<double, 5, 1>& getResiduals() {return m_residual;}
    /** get residuals */
    const EigenTypes::ColVector getResiduals() const {return m_residual;}
    //const Eigen::Matrix<double, 5, 1> getResiduals() const {return m_residual;}
    /** ge element in residuals */
    double getElementInResiduals(int row) { return m_residual(m_offset + row, 1); }
    /** get residuals cov matrix */
    EigenTypes::MatrixXd& getV() {return m_V;}
    //Eigen::Matrix<double, 5, 5>& getV() {return m_V;}
    /** get residuals cov matrix */
    const EigenTypes::MatrixXd& getV() const {return m_V;}
    //const Eigen::Matrix<double, 5, 5>& getV() const {return m_V;}
    /** reset projection */
    void resetProjection();
    /** get chi2 */
    double getChiSquare();
    /**  get offset (for mergedcosntraints)*/
    unsigned int getOffset() { return m_offset; }



    // accessors to the projection matrix
    const CLHEP::HepMatrix& H() const { return m_matrixH; }
    double& H(int row, int col) { return m_matrixH(m_offset + row, col); }

    // accessors to the residual (or the 'value' of the constraint)
    const CLHEP::HepVector& r() const { return m_r; }
    CLHEP::HepVector& r() { return m_r; }
    double& r(int row) { return m_r(m_offset + row); }

    // accessors to the covariance matrix
    const CLHEP::HepSymMatrix& V() const { return m_matrixV; }
    //CLHEP::HepSymMatrix& V() { return m_matrixV ; }
    double& V(int row, int col) { return m_matrixV(m_offset + row, m_offset + col); }

    double& Vfast(int row, int col) { return m_matrixV.fast(m_offset + row, m_offset + col); }

    // reset
    /** reset projection */
    void reset();
    double chiSquare() const;

    /** increment offset */
    void incrementOffset(unsigned int i) { m_offset += i; }

    unsigned int offset() const { return m_offset; }

  private:
    CLHEP::HepMatrix m_matrixH;    // projection matrix
    CLHEP::HepVector m_r;    // constraint residual
    CLHEP::HepSymMatrix m_matrixV; // constraint variance (zero for lagrange constraints)
    //JFK: FIXME find max size 10 is larger than we need I think
    /** projection matrix */
    EigenTypes::MatrixXd m_H;
    //Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic, 0, 10, 10> m_H;
    /** constraint covarariance */
    //JFK: dangerrousway to make it faster 2017-10-02
    //Eigen::Matrix<double, 5, 5> m_V;
    EigenTypes::MatrixXd m_V;
    //Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic, 0, 10, 10> m_V;
    /** residual */
    //JFK: this is dangerrous but faster  2017-10-02
    //Eigen::Matrix<double, 5, 1> m_residual;
    EigenTypes::ColVector m_residual;
    //Eigen::Matrix<double, Eigen::Dynamic, 1, 0, 10, 1> m_residual;
    /** dimension of the covariance */
    int m_dimCov;
    /** dimension of H */
    int m_dimProj;

    /** offset for constraint index. used when there's more then one constraint projected */
    unsigned int m_offset;
  } ;
}
#endif
