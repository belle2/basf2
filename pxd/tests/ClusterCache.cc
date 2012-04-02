/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Ritter                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <pxd/reconstruction/ClusterCache.h>
#include <gtest/gtest.h>

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
      ClusterCandidate cls;

      cache.setLast(2, 0, &cls);
      //Right pixel belongs to cluster
      EXPECT_EQ(&cls,                 cache.findCluster(3, 0));
      //Next to right is not clustered
      EXPECT_EQ((ClusterCandidate*)0, cache.findCluster(4, 0));
      //one down, two columns left is to far away
      EXPECT_EQ((ClusterCandidate*)0, cache.findCluster(0, 1));
      //We also want the bottom left pixel
      EXPECT_EQ(&cls,                 cache.findCluster(1, 1));
      //the bottom pixel
      EXPECT_EQ(&cls,                 cache.findCluster(2, 1));
      //and of course the bottom right.
      EXPECT_EQ(&cls,                 cache.findCluster(3, 1));
      //But everything farther away does not belong to the same
      //cluster
      EXPECT_EQ((ClusterCandidate*)0, cache.findCluster(4, 1));
      //Two rows down is also excluded
      EXPECT_EQ((ClusterCandidate*)0, cache.findCluster(0, 2));
      EXPECT_EQ((ClusterCandidate*)0, cache.findCluster(1, 2));
      EXPECT_EQ((ClusterCandidate*)0, cache.findCluster(2, 2));
      EXPECT_EQ((ClusterCandidate*)0, cache.findCluster(3, 2));
      EXPECT_EQ((ClusterCandidate*)0, cache.findCluster(4, 2));
    }

    /** Check that we do not cluster if a row has been skipped */
    TEST(ClusterCache, SkipRow)
    {
      ClusterCache cache;
      ClusterCandidate cls;

      cache.setLast(0, 0, &cls);
      EXPECT_EQ((ClusterCandidate*)0, cache.findCluster(0, 2));
      EXPECT_EQ((ClusterCandidate*)0, cache.findCluster(1, 2));
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
     * 0│ │1│ │2│ │ │2│ │ │ │ │
     *  ├─┼─┼─┼─┼─┼─┼─┼─┼─┼─┼─┤
     * 1│1│ │X│ │ │ │Y│ │ │ │ │
     *  ├─┼─┼─┼─┼─┼─┼─┼─┼─┼─┼─┤
     * 2│Z│ │ │ │ │ │ │ │ │ │ │
     *  ├─┼─┼─┼─┼─┼─┼─┼─┼─┼─┼─┤
     * 3│ │ │ │ │ │ │ │ │ │ │ │
     *  ├─┼─┼─┼─┼─┼─┼─┼─┼─┼─┼─┤
     * ↓│ │ │ │ │ │ │ │ │ │ │ │
     *  └─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┘
     * @endverbatim
     */
    TEST(ClusterCache, Merging)
    {
      ClusterCache cache;
      std::vector<ClusterCandidate> cls(2);
      cls[0].add(Pixel(0, 1));
      cls[1].add(Pixel(0, 2));
      //cls[2].add(Pixel(0,2));

      //Set the two clusters as shown above
      cache.setLast(1, 0, &cls[0]);
      cache.setLast(3, 0, &cls[1]);
      cache.setLast(6, 0, &cls[1]);
      cache.setLast(0, 1, &cls[0]);
      //And find a cluster for pixel X
      ClusterCandidate* found = cache.findCluster(2, 1);
      //We must have found the neighbours
      ASSERT_TRUE(found);
      //and the two cluster should have been merged
      ASSERT_EQ(2u, found->size());
      //The pointer should be one of the two
      EXPECT_TRUE(found == &cls[0] || found == &cls[1]);
      //The sum of indices should be ok
      EXPECT_EQ(3u, found->pixels()[0].getIndex() + found->pixels()[1].getIndex());
      //the other cluster should be empty now
      int other = (found == &cls[0]) ? 1 : 0;
      EXPECT_EQ(0u, cls[other].size());
      //Ok, now lets set the cluster
      cache.setLast(1, 1, found);

      //And add pixel Y
      ClusterCandidate* foundY = cache.findCluster(6, 1);
      EXPECT_EQ(found, foundY);
      cache.setLast(6, 1, foundY);

      //And finally pixel Z
      ClusterCandidate* foundZ = cache.findCluster(0, 2);
      EXPECT_EQ(found, foundZ);
    }

  } //PXD namespace
} //Belle namespace
