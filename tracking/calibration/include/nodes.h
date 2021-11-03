/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/


#pragma once

#include <vector>
#include <Eigen/Dense>
using Eigen::VectorXd;
using Eigen::MatrixXd;

/** Get weights to calculate the integral over the Cheb nodes */
VectorXd GetWeights(int Size);

/** Get the vector with positions of the Cheb nodes */
VectorXd GetNodes(int Size);


/** Evaluate cheb pols up to Size at point x */
VectorXd getPols(int Size, double x);

/** Evaluate sum of cheb pols up to Size at vector x, x range is between 0 and 1 */
VectorXd getPolsSum(int Size, VectorXd x);


/** Transformation matrix between Cheb nodes and Cheb coefficients */
MatrixXd GetCoefs(int oldSize, bool isInverse = false);


/** Transformation matrix between Cheb nodes and Cheb coefficients with better normalization of the borders */
MatrixXd GetCoefsCheb(int oldSize);


/** Get Interpolation vector at point x from the function values at points xi */
VectorXd interpol(const VectorXd& xi, double x);


/** Get interpolated function value at point x */
double interpol(VectorXd xi, VectorXd vals, double x);

