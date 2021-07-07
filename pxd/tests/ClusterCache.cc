/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <pxd/reconstruction/ClusterCache.h>
#include <gtest/gtest.h>
#include <bitset>

using namespace std;

namespace Belle2 {
  namespace PXD {
    /** Check that we cluster to hits next to each other but not if one is in between.
     *
     * So we set a cluster at position (2,0) marked c and the findCluster
     * should return this cluster for all positions marked 1. All other pixels
     * should return NULL. We do not check (0,0) and (1,0) or a row above C
     * since the cluster cache is meant to be used with ordered data so these
     * should already be done once we set (2,0).
     *
     * @verbatim
     * u 0 1 2 3 4 5 6 7 8 9 →
     * v┌─┬─┬─┬─┬─┬─┬─┬─┬─┬─┬─┐
     * 0│ │ │C│1│0│ │ │ │ │ │ │
     *  ├─┼─┼─┼─┼─┼─┼─┼─┼─┼─┼─┤
     * 1│0│1│1│1│0│ │ │ │ │ │ │
     *  ├─┼─┼─┼─┼─┼─┼─┼─┼─┼─┼─┤
     * 3│0│0│0│0│0│ │ │ │ │ │ │
     *  ├─┼─┼─┼─┼─┼─┼─┼─┼─┼─┼─┤
     * 4│ │ │ │ │ │ │ │ │ │ │ │
     *  ├─┼─┼─┼─┼─┼─┼─┼─┼─┼─┼─┤
     * ↓│ │ │ │ │ │ │ │ │ │ │ │
     *  └─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┘
     * @endverbatim
     */

    TEST(ClusterCache, FindNeighbours)
    {
      ClusterCache cache;
      for (int v = 0; v < 4; ++v) {
        for (int u = 0; u < 5; ++u) {
          if (v == 0 && u <= 2) continue;
          cache.clear();
          ClusterCandidate& cls1 = cache.findCluster(2, 0);
          if ((v == 0 && u == 3) || (v == 1 && u >= 1 && u <= 3)) {
            //Check that neighboring pixels return the same cluster
            EXPECT_EQ(&cls1, &cache.findCluster(u, v)) << "u: " << u << " v: " << v;
          } else {
            //And all other pixels return another cluster
            EXPECT_NE(&cls1, &cache.findCluster(u, v)) << "u: " << u << " v: " << v;
          }
        }
      }
    }

    /** Test that clusters get merged if they are found to have a common pixel.
     *
     * If we have to clusters called 1 and 2 like shown below and we add a
     * pixel at X than all occurences of 1 and 2 should be merged to one of
     * those two. It does not matter who is merged to whom but after adding
     * X the result must be
     * - one of the two clusters is empty
     * - the other cluster contains all pixels
     * - all pointers for 1 and 2 should point to the same pixel so that when
     *   we add pixels Y and Z we get the same pointer as for X
     *
     * @verbatim
     * u 0 1 2 3 4 5 6 7 8 9 →
     * v┌─┬─┬─┬─┬─┬─┬─┬─┬─┬─┬─┐
     * 0│ │ │1│ │2│ │3│ │ │ │ │
     *  ├─┼─┼─┼─┼─┼─┼─┼─┼─┼─┼─┤
     * 1│4│ │ │X│ │ │5│ │ │ │ │
     *  ├─┼─┼─┼─┼─┼─┼─┼─┼─┼─┼─┤
     * 2│ │6│Y│ │7│Z│ │ │ │ │ │
     *  ├─┼─┼─┼─┼─┼─┼─┼─┼─┼─┼─┤
     * 3│ │ │ │ │ │ │ │ │ │ │ │
     *  ├─┼─┼─┼─┼─┼─┼─┼─┼─┼─┼─┤
     * ↓│ │ │ │ │ │ │ │ │ │ │ │
     *  └─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┘
     * @endverbatim
     */
    TEST(ClusterCache, Merging)
    {
      //Create clusters 1, 2, 3 and 4
      ClusterCache cache;
      ClusterCandidate& cls1 = cache.findCluster(2, 0);
      ClusterCandidate& cls2 = cache.findCluster(4, 0);
      cls1.add(Pixel(1));
      cls2.add(Pixel(2));
      cache.findCluster(6, 0).add(Pixel(3));
      cache.findCluster(0, 1).add(Pixel(4));

      //Add Pixel X
      ClusterCandidate& foundX = cache.findCluster(3, 1);
      //and the clusters 1 and 2 should have been merged
      ASSERT_EQ(2u, foundX.size());
      //The pointer should be one of the two
      EXPECT_TRUE(&foundX == &cls1 || &foundX == &cls2);
      //The sum of indices should be ok
      EXPECT_EQ(3u, foundX.pixels()[0].getIndex() + foundX.pixels()[1].getIndex());
      //the other cluster should be empty now
      EXPECT_TRUE((cls1.size() == 0 && cls2.size() == 2) || (cls1.size() == 2 && cls2.size() == 0));

      //And add pixel 5, 6 and Y
      cache.findCluster(6, 1).add(Pixel(5));
      cache.findCluster(1, 2).add(Pixel(6));
      ClusterCandidate& foundY = cache.findCluster(2, 2);
      EXPECT_EQ(4u, foundY.size());
      {
        //Check for equality by merging empty clusters. merge returns a pointer
        //to the topmost cluster in a group of merged clusters
        ClusterCandidate t1, t2;
        EXPECT_EQ(foundX.merge(t1), foundY.merge(t2));
      }

      //And finally pixel 7 and Z
      cache.findCluster(4, 2).add(Pixel(7));
      ClusterCandidate& foundZ = cache.findCluster(5, 2);
      ASSERT_EQ(7u, foundY.size());
      {
        //Check for equality by merging empty clusters. merge returns a pointer
        //to the topmost cluster in a group of merged clusters
        ClusterCandidate t1, t2;
        EXPECT_EQ(foundX.merge(t1), foundZ.merge(t2));
      }

      //Check if all 7 pixels are present using a bitmask to see if we get every index from 1 to 7 once
      std::bitset<7> check_index(-1);
      for (const Pixel& px : foundZ.pixels()) {
        ASSERT_LT(0u, px.getIndex());
        ASSERT_GE(7u, px.getIndex());
        EXPECT_TRUE(check_index[px.getIndex() - 1]) << "index: " << px.getIndex();
        check_index[px.getIndex() - 1] = false;
      }
      EXPECT_TRUE(check_index.none());
    }

    /**
     * Check if the cluster cache is empty
     */
    TEST(ClusterCache, Empty)
    {
      ClusterCache cache;
      ASSERT_TRUE(cache.empty());
      cache.findCluster(0, 0);
      ASSERT_FALSE(cache.empty());
    }

    /** Check that out_of_range exceptions are raised if the pixel is out of range */
    TEST(ClusterCache, OutOfRange)
    {
      ClusterCache cache(250);
      for (int i = -10; i < 260; ++i) {
        if (i >= 0 && i < 250) {
          ASSERT_NO_THROW(cache.findCluster(i, 0));
        } else {
          ASSERT_THROW(cache.findCluster(i, 0), std::out_of_range);
        }
      }
    }
  } //PXD namespace
} //Belle namespace
