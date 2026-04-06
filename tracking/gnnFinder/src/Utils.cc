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
  std::generate(pool.begin(), pool.end(), [] { return new KDTNode(); });
}

KDTNodePool::~KDTNodePool()
{
  for (KDTNode* node : pool) {
    delete node;
  }
}

KDTNode* KDTNodePool::allocate()
{
  if (index >= pool.size())
    B2ERROR("KDTNodePool:allocate() exceeded pool capacity.");
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
  const int dim = depth % 2;
  const auto cmp = [dim](const KDTHit & a, const KDTHit & b) {
    return (dim == 0) ? a.x < b.x : a.y < b.y;
  };

  const size_t n = std::distance(begin, end);
  const auto mid  = begin + n / 2;  // Median element becomes the node's pivot

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
    const double d = query.squaredDistanceTo(node->hit);
    if (d < bestDist) {
      bestDist = d;
      best = node->hit;
    }
  }

  // Determine which child subtree lies on the same side as the query point
  const int dim = node->dim;
  const double qVal = (dim == 0) ? query.x : query.y;
  const double nVal = (dim == 0) ? node->hit.x : node->hit.y;

  KDTNode* near = (qVal < nVal) ? node->left : node->right;
  KDTNode* far  = (qVal < nVal) ? node->right : node->left;

  // Always descend into the near subtree first (more likely to improve bestDist)
  nearestNeighbor(near, query, best, bestDist);

  // Pruning: search only the "far" side if the distance to the
  // splitting plane is less than the best distance found so far
  const double diff = qVal - nVal;
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
  // Return an empty vector if kdtHits is empty
  if (kdtHits.empty())
    return std::vector<int> {};

  // Build a KD-tree over the hits; pool size equals number of hits (one node per hit)
  KDTNodePool pool(kdtHits.size());
  KDTNode* root = buildKDTree(kdtHits.begin(), kdtHits.end(), 0, pool);

  // Define the starting hit (-1 here means it's not a real hit)
  const KDTHit startingHit{startingX, startingY, -1};

  // Linear scan to find the real hit closest to the starting position.
  // A KD-tree query is not used here because the starting position is not in the tree,
  // and the list is typically short enough that a scan is negligible.
  const KDTHit& firstHit = *std::min_element(kdtHits.begin(), kdtHits.end(),
  [&startingHit](const KDTHit & a, const KDTHit & b) {
    return a.squaredDistanceTo(startingHit) < b.squaredDistanceTo(startingHit);
  });

  // Mark the first hit as consumed so it won't be returned by nearestNeighbor
  markUsed(root, firstHit);

  // Prepare the output
  std::vector<int> sortedIndices;
  sortedIndices.reserve(kdtHits.size());
  sortedIndices.push_back(firstHit.hitIndex);

  // Greedy chain: at each step find the nearest unused hit to the current one
  KDTHit currentHit = firstHit;
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

std::pair<double, double> Belle2::GNNFinder::Utils::intersectCylinderXY(const ROOT::Math::XYZVector& pos,
    const ROOT::Math::XYZVector& mom,
    const double targetR)
{
  // Check if we are already outside or at the boundary
  double rSq = pos.X() * pos.X() + pos.Y() * pos.Y();
  if (rSq >= targetR * targetR)
    return {pos.X(), pos.Y()};
  // Coefficients for a*t^2 + b*t + c = 0
  // Solving for |(pos + t*mom).xy| = targetR
  double a = mom.X() * mom.X() + mom.Y() * mom.Y();
  double b = 2.0 * (pos.X() * mom.X() + pos.Y() * mom.Y());
  double c = rSq - (targetR * targetR);
  double discriminant = b * b - 4.0 * a * c;
  if (discriminant < 0 or a == 0)
    return {pos.X(), pos.Y()};
  double sqrtD = std::sqrt(discriminant);
  double invA = 1.0 / a;
  double t1 = 0.5 * (-b + sqrtD) * invA;
  double t2 = 0.5 * (-b - sqrtD) * invA;
  // Get the first positive intersection point
  double t = -1.0;
  if (t1 > 0 && t2 > 0) t = std::min(t1, t2);
  else if (t1 > 0)      t = t1;
  else if (t2 > 0)      t = t2;
  if (t > 0)
    return {pos.X() + t * mom.X(), pos.Y() + t * mom.Y()};
  return {pos.X(), pos.Y()};
}
