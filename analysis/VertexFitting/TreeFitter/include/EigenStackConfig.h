/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributor: Jo-Frederik Krohn                                         *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once
/**
 * This allows us to tell Eigen to use stack allocated fixed sized matrices.
 * Dirty and fast.
 *
 * there's 2 limits here:
 *  - EIGEN_STACK_ALLOCATION_LIMIT (128kb) a macro in EIGEN
 *    that has to be extended if you want larger matrices than 110by110
 *    it has to be set before EIGEN is included (as a parameter for scons)
 *
 *  - what we accept as reasonable stack usage (MAX_MATRIX_SIZE)
 *
 * To give you a feeling: 100x100 results in the cov upddate function
 * in the Kalman class beeing 300kb big.
 * 120x120 already exceeds the Eigen limit (for a single matrix).
*/


#if defined(__GNUC__) && !defined(__clang__)
#define TREEFITTER_NO_STACK_WARNING \
  _Pragma("GCC diagnostic push") \
  _Pragma("GCC diagnostic ignored \"-Wstack-usage=\"")
#define TREEFITTER_RESTORE_WARNINGS \
  _Pragma("GCC diagnostic pop")
#else
#define TREEFITTER_NO_STACK_WARNING
#define TREEFITTER_RESTORE_WARNINGS
#endif

#define MAX_MATRIX_SIZE 100
