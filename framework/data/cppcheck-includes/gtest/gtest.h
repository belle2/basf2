/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
/** This is a dummy include file which is needed to help cppcheck understand
 * google test macros. Otherwise it returns syntax errors. This should be
 * solved upstream by providing a gtest.cfg which does something similar to
 * this.
 *
 * WARNING: DO NOT EVER USE THIS for anything else
 */
#pragma once
#define TEST(a, b) void gtest_##a##b()
#define TEST_F(a, b) void gtest_f_##a##b()
#define TEST_P(a, b) void gtest_p_##a##b()
