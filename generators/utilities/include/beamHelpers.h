/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <cmath>
#include <tuple>
#include <utility>
#include <vector>
#include <Eigen/Dense>


namespace Belle2 {

  inline double sqrt(double a) { return std::sqrt(a); }
  inline double tan(double a)  { return std::tan(a); }
  inline double atan(double a) { return std::atan(a); }


  struct DualNumber {
    double x, dx;
    DualNumber(double X, double dX) : x(X), dx(dX) {}
    DualNumber() : x(0), dx(0) {}
  };


  inline DualNumber operator+(DualNumber a, DualNumber b)
  {
    return DualNumber(a.x + b.x, a.dx + b.dx);
  }

  inline DualNumber operator+(DualNumber a, double b)
  {
    return DualNumber(a.x + b, a.dx);
  }

  inline DualNumber operator-(DualNumber a, double b)
  {
    return DualNumber(a.x - b, a.dx);
  }

  inline DualNumber operator-(double a, DualNumber b)
  {
    return DualNumber(a - b.x, -b.dx);
  }


  inline DualNumber operator/(double a, DualNumber b)
  {
    return DualNumber(a / b.x, - a / (b.x * b.x) * b.dx);
  }

  inline DualNumber operator/(DualNumber a, DualNumber b)
  {
    return DualNumber(a.x / b.x, (a.dx * b.x - a.x * b.dx) / (b.x * b.x));
  }


  inline DualNumber operator-(DualNumber a, DualNumber b)
  {
    return DualNumber(a.x - b.x, a.dx - b.dx);
  }

  inline DualNumber operator*(DualNumber a, DualNumber b)
  {
    return DualNumber(a.x * b.x, a.x * b.dx + b.x * a.dx);
  }

  inline DualNumber operator*(double a, DualNumber b)
  {
    return DualNumber(a * b.x, a * b.dx);
  }

  inline DualNumber sqrt(DualNumber a)
  {
    return  DualNumber(std::sqrt(a.x), 1. / (2 * std::sqrt(a.x)) * a.dx);
  }

  inline DualNumber atan(DualNumber a)
  {
    return  DualNumber(std::atan(a.x),  1. / (1 + a.x * a.x) * a.dx);
  }

  inline DualNumber tan(DualNumber a)
  {
    return  DualNumber(std::tan(a.x), (1.0 + std::tan(a.x) * std::tan(a.x)) * a.dx);
  }


  template<typename T>
  struct GeneralVector {
    T el[3];
    GeneralVector(T x, T y, T z) { el[0] = x; el[1] = y; el[2] = z; }
    T norm2() const { return (el[0] * el[0] + el[1] * el[1] + el[2] * el[2]); }

    T angleX() const { return atan(el[0] / el[2]); }
    T angleY() const { return atan(el[1] / el[2]); }
  };

  template<typename T>
  GeneralVector<T> operator+(GeneralVector<T> a, GeneralVector<T> b)
  {
    return GeneralVector<T>(a.el[0] + b.el[0], a.el[1] + b.el[1], a.el[2] + b.el[2]);
  }


  template<typename T>
  T dot(GeneralVector<T> a, GeneralVector<T> b)
  {
    return (a.el[0] * b.el[0] + a.el[1] * b.el[1] + a.el[2] * b.el[2]);
  }


  template<typename T>
  GeneralVector<T> operator*(T a, GeneralVector<T> b)
  {
    return GeneralVector<T>(a * b.el[0], a * b.el[1], a * b.el[2]);
  }

  // electron mass
  static const double me = 0.510998950e-3;

  template<typename T>
  T getEcms(GeneralVector<T> p1, GeneralVector<T> p2)
  {
    T E1 = sqrt(p1.norm2() + me * me);
    T E2 = sqrt(p2.norm2() + me * me);

    return  sqrt((E1 + E2) * (E1 + E2)  - (p1 + p2).norm2());
  }


  template<typename T>
  GeneralVector<T> getBoost(GeneralVector<T> p1, GeneralVector<T> p2)
  {
    T E1 = sqrt(p1.norm2() + me * me);
    T E2 = sqrt(p2.norm2() + me * me);

    return 1. / (E1 + E2) * (p1 + p2);
  }


  template<typename T>
  std::pair<T, T> getAnglesCMS(GeneralVector<T> p1, GeneralVector<T> p2)
  {
    GeneralVector<T> bv = getBoost(p1, p2);

    T gamma = 1.0 / sqrt(1 - bv.norm2());

    T E1 = sqrt(p1.norm2() + me * me);

    GeneralVector<T> pCMS = p1 + ((gamma - 1) / bv.norm2() * dot(p1, bv)  - gamma * E1) * bv;

    return std::make_pair(pCMS.angleX(), pCMS.angleY());
  }


  template<typename T>
  GeneralVector<T> getFourVector(T energy, T angleX, T angleY, bool isHER)
  {
    T p   = sqrt(energy * energy - me * me);

    double dir = isHER ? 1 : -1;

    T pz = dir * p / sqrt(tan(angleX) * tan(angleX) + tan(angleY) * tan(angleY) + 1.0);

    return GeneralVector<T>(pz * tan(angleX), pz * tan(angleY), pz);
  }


  inline Eigen::MatrixXd getGradientMatrix(double eH, double thXH, double thYH,
                                           double eL, double thXL, double thYL)
  {
    Eigen::MatrixXd grad(6, 6);

    //calculate derivatives wrt all 6 input variables
    for (int j = 0; j < 6; ++j) {

      std::vector<double> eps(6, 0.0);
      eps[j] = 1.0;

      GeneralVector<DualNumber> pH = getFourVector(DualNumber(eH, eps[0]), DualNumber(thXH, eps[1]), DualNumber(thYH, eps[2]), true);
      GeneralVector<DualNumber> pL = getFourVector(DualNumber(eL, eps[3]), DualNumber(thXL, eps[4]), DualNumber(thYL, eps[5]), false);


      DualNumber Ecms = getEcms(pH, pL);
      GeneralVector<DualNumber> boost = getBoost(pH, pL);

      DualNumber angleX, angleY;
      std::tie(angleX, angleY) = getAnglesCMS(pH, pL);

      grad(0, j) = Ecms.dx;
      grad(1, j) = boost.el[0].dx;
      grad(2, j) = boost.el[1].dx;
      grad(3, j) = boost.el[2].dx;
      grad(4, j) = angleX.dx;
      grad(5, j) = angleY.dx;
    }

    return grad;
  }


  inline Eigen::VectorXd getCentralValues(double eH, double thXH, double thYH,
                                          double eL, double thXL, double thYL)
  {
    Eigen::VectorXd mu(6);

    GeneralVector<double> pH = getFourVector<double>(eH, thXH, thYH, true);
    GeneralVector<double> pL = getFourVector<double>(eL, thXL, thYL, false);

    double Ecms = getEcms<double>(pH, pL);
    GeneralVector<double> boost = getBoost<double>(pH, pL);

    double angleX, angleY;
    std::tie(angleX, angleY) = getAnglesCMS<double>(pH, pL);

    mu(0) = Ecms;
    mu(1) = boost.el[0];
    mu(2) = boost.el[1];
    mu(3) = boost.el[2];
    mu(4) = angleX;
    mu(5) = angleY;

    return mu;
  }


  inline Eigen::MatrixXd transformCov(Eigen::MatrixXd covHER, Eigen::MatrixXd covLER, Eigen::MatrixXd grad)
  {

    Eigen::MatrixXd cov = Eigen::MatrixXd::Zero(6, 6);
    cov.block<3, 3>(0, 0) = covHER;
    cov.block<3, 3>(3, 3) = covLER;

    Eigen::MatrixXd covNew = grad * cov * grad.transpose();

    return covNew;

  }


}
