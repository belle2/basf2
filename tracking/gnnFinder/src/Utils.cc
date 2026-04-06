/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <tracking/gnnFinder/Utils.h>
#include <framework/logging/Logger.h>

#include <algorithm>
#include <limits>


using namespace Belle2::GNNFinder::Utils;


KDTNodePool::KDTNodePool(size_t capacity) : index(0)
{
  pool.resize(capacity);
  for (size_t i = 0; i < capacity; ++i) {
    pool[i] = new KDTNode();
  }
}

KDTNodePool::~KDTNodePool()
{
  for (KDTNode* node : pool) {
    delete node;
  }
}

KDTNode* KDTNodePool::allocate()
{
  if (index >= pool.size()) {
    B2ERROR("KDTNodePool:allocate() exceeded pool capacity.");
  }
  // Hand out the next pre-allocated slot and advance the cursor
  return pool[index++];
}

void KDTNodePool::reset()
{
  // Rewind the cursor so all slots can be reused in the next event
  // without freeing / reallocating memory
  index = 0;
}

template<typename Iterator, typename Compare>
inline void HitOrderer::insertionSort(Iterator begin, Iterator end, Compare cmp)
{
  for (Iterator i = begin; i != end; ++i) {
    // Bubble element i leftward until the sequence is locally sorted
    for (Iterator j = i; j != begin && cmp(*j, *(j - 1)); --j) {
      std::iter_swap(j, j - 1);
    }
  }
}

KDTNode* HitOrderer::buildKDTree(std::vector<KDTHit>::iterator begin, std::vector<KDTHit>::iterator end, int depth,
                                 KDTNodePool& pool, const size_t INSERTION_SORT_THRESHOLD)
{
  if (begin >= end)
    return nullptr;

  // Alternate splitting dimension with each level of recursion
  int dim = depth % 2;
  auto cmp = [dim](const KDTHit & a, const KDTHit & b) {
    return (dim == 0) ? a.x < b.x : a.y < b.y;
  };

  size_t n = std::distance(begin, end);
  auto mid  = begin + n / 2;  // Median element becomes the node's pivot

  if (n < INSERTION_SORT_THRESHOLD) {
    // Full sort for small ranges: cheaper than nth_element + recursion overhead
    HitOrderer::insertionSort(begin, end, cmp);
  } else {
    // Partial sort: guarantees *mid is the true median, rest unsorted
    std::nth_element(begin, mid, end, cmp);
  }

  // Claim a node from the pool and populate it with the median hit
  KDTNode* node = pool.allocate();
  node->hit = *mid;
  node->used = false;  // Will be set to true once this hit is added to the track
  node->dim = dim;

  // Recurse on the left (< median) and right (> median) sub-ranges
  node->left =  HitOrderer::buildKDTree(begin, mid, depth + 1, pool);
  node->right = HitOrderer::buildKDTree(mid + 1, end, depth + 1, pool);

  return node;
}

void HitOrderer::nearestNeighbor(KDTNode* node, const KDTHit& query, KDTHit& best, double& bestDist)
{
  if (!node)
    return;

  // Evaluate the current node if it hasn't been assigned to the track yet
  if (!node->used) {
    double d = query.squaredDistanceTo(node->hit);
    if (d < bestDist) {
      bestDist = d;
      best = node->hit;
    }
  }

  // Determine which child subtree lies on the same side as the query point
  int dim = node->dim;
  double qVal = (dim == 0) ? query.x : query.y;
  double nVal = (dim == 0) ? node->hit.x : node->hit.y;

  KDTNode* near = (qVal < nVal) ? node->left : node->right;
  KDTNode* far  = (qVal < nVal) ? node->right : node->left;

  // Always descend into the near subtree first (more likely to improve bestDist)
  nearestNeighbor(near, query, best, bestDist);

  // Pruning: search only the "far" side if the distance to the
  // splitting plane is less than the best distance found so far
  double diff = qVal - nVal;
  if ((diff * diff) < bestDist) {
    nearestNeighbor(far, query, best, bestDist);
  }
}

bool HitOrderer::markUsed(KDTNode* node, const KDTHit& hit)
{
  if (!node)
    return false;

  // Found the matching node: mark and return
  if (node->hit.hitIndex == hit.hitIndex) {
    node->used = true;
    return true;
  }

  // Descend into the child whose partition contains the hit's coordinate
  const int    dim  = node->dim;
  const double hVal = (dim == 0) ? hit.x      : hit.y;
  const double nVal = (dim == 0) ? node->hit.x : node->hit.y;

  KDTNode* first  = (hVal < nVal) ? node->left  : node->right;
  KDTNode* second = (hVal < nVal) ? node->right : node->left;

  // If the primary branch fails (e.g. due to equal coordinates on the
  // splitting axis), fall back to the other child rather than giving up
  return markUsed(first, hit) or markUsed(second, hit);
}

void HitOrderer::freeKDTree(KDTNode* node)
{
  if (!node)
    return;
  HitOrderer::freeKDTree(node->left);
  HitOrderer::freeKDTree(node->right);
  delete node;
}

std::vector<int> HitOrderer::orderHits(const double startingX, const double startingY,
                                       // cppcheck-suppress passedByValue
                                       std::vector<KDTHit> kdtHits)
{
  // Build a KD-tree over the hits; pool size equals number of hits (one node per hit)
  KDTNodePool pool(kdtHits.size());
  KDTNode* root = buildKDTree(kdtHits.begin(), kdtHits.end(), 0, pool);

  // Define the starting hit (-1 here means it's not a real hit)
  const KDTHit startingHit{startingX, startingY, -1};
  // Linear scan to find the real hit closest to the starting position.
  // A KD-tree query is not used here because the starting position is not in the tree,
  // and the list is typically short enough that a scan is negligible.
  KDTHit currentHit = kdtHits[0];
  double bestDist = std::numeric_limits<double>::max();
  for (const auto& kdtHit : kdtHits) {
    const double d = kdtHit.squaredDistanceTo(startingHit);
    if (d < bestDist) {
      bestDist = d;
      currentHit = kdtHit;
    }
  }

  // Mark the first hit as consumed so it won't be returned by nearestNeighbor
  markUsed(root, currentHit);

  std::vector<int> sortedIndices;
  sortedIndices.reserve(kdtHits.size());
  sortedIndices.push_back(currentHit.hitIndex);

  // Greedy chain: at each step find the nearest unused hit to the current one
  for (size_t i = 1; i < kdtHits.size(); ++i) {
    KDTHit bestNeighbor;
    double bestNeighborDist = std::numeric_limits<double>::max();
    nearestNeighbor(root, currentHit, bestNeighbor, bestNeighborDist);

    // If no unused hit was found (all remaining are marked used), stop early
    if (bestNeighborDist == std::numeric_limits<double>::max())
      break;

    currentHit = bestNeighbor;
    sortedIndices.push_back(currentHit.hitIndex);
    markUsed(root, currentHit);  // Prevent this hit from being selected again
  }

  return sortedIndices;
}
