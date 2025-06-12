#include "tracking/modules/CATFinder/HitOrderer.h"

#include <algorithm>
#include <limits>

template<typename Iterator, typename Compare>
inline void HitOrderer::insertionSort(Iterator begin, Iterator end, Compare cmp)
{
  for (Iterator i = begin; i != end; ++i) {
    for (Iterator j = i; j != begin && cmp(*j, *(j - 1)); --j) {
      std::iter_swap(j, j - 1);
    }
  }
}

KDTNode* HitOrderer::buildKDTree(std::vector<KDTHit>::iterator begin, std::vector<KDTHit>::iterator end, int depth,
                                 KDTNodePool& pool, const int INSERTION_SORT_THRESHOLD)
{

  if (begin >= end) return nullptr;

  int dim = depth % 2;
  auto cmp = [dim](const KDTHit & a, const KDTHit & b) {
    return (dim == 0) ? a.x < b.x : a.y < b.y;
  };

  size_t n = std::distance(begin, end);
  auto mid  = begin + n / 2;

  if (n < INSERTION_SORT_THRESHOLD) {
    HitOrderer::insertionSort(begin, end, cmp);
  } else {
    std::nth_element(begin, mid, end, cmp);
  }

  KDTNode* node = pool.allocate();
  node -> hit = *mid;
  node -> used = false;
  node -> dim = dim;

  node -> left =  HitOrderer::buildKDTree(begin, mid, depth + 1, pool);
  node -> right = HitOrderer::buildKDTree(mid + 1, end, depth + 1, pool);

  return node;
}

void HitOrderer::nearestNeighbor(KDTNode* node, const KDTHit& query, KDTHit& best, double& bestDist)
{
  if (!node) return;
  if (!node -> used) {
    double d = squaredDistance(query, node -> hit);
    if (d < bestDist) {
      bestDist = d;
      best = node -> hit;
    }
  }
}

bool HitOrderer::markUsed(KDTNode* node, const KDTHit& hit)
{
  if (!node) return false;
  if (node -> hit.hitIndex == hit.hitIndex) {
    node -> used = true;
    return true;
  }
  return markUsed(node -> left, hit) || markUsed(node -> right, hit);

}

void HitOrderer::freeKDTree(KDTNode* node)
{
  if (!node) return;
  HitOrderer::freeKDTree(node -> left);
  HitOrderer::freeKDTree(node -> right);
  delete node;
}

double HitOrderer::squaredDistance(const KDTHit& a, const KDTHit& b)
{
  double dx = a.x - b.x;
  double dy = a.y - b.y;
  return dx * dx + dy * dy;
}

std::vector<int> HitOrderer::orderHits(std::vector<double> startPosition, std::vector<std::vector<double>> nodes,
                                       std::vector<int>& CDCHitIndices)
{

  std::vector<KDTHit> kdtHits;
  kdtHits.reserve(CDCHitIndices.size());
  for (size_t i = 0; i < CDCHitIndices.size(); ++i) {
    kdtHits.push_back({nodes[i][0], nodes[i][1], CDCHitIndices[i]});
  }

  KDTNodePool pool(kdtHits.size());

  KDTNode* root = buildKDTree(kdtHits.begin(), kdtHits.end(), 0, pool);

  KDTHit currentHit = kdtHits[0];
  double bestDist = std::numeric_limits<double>::max();
  for (const auto& kdtHit : kdtHits) {
    double d = squaredDistance({startPosition[0], startPosition[1], -1}, kdtHit);
    if (d < bestDist) {
      bestDist = d;
      currentHit = kdtHit;
    }
  }

  markUsed(root, currentHit);

  std::vector<KDTHit> sortedHits;
  sortedHits.push_back(currentHit);

  for (size_t i = 1; i < kdtHits.size(); ++i) {
    KDTHit bestNeighbor;
    double bestNeighborDist = std::numeric_limits<double>::max();
    nearestNeighbor(root, currentHit, bestNeighbor, bestNeighborDist);

    if (bestNeighborDist == std::numeric_limits<double>::max()) break;

    currentHit = bestNeighbor;
    sortedHits.push_back(currentHit);
    markUsed(root, currentHit);
  }

  std::vector<int> sortedIndices;
  sortedIndices.reserve(sortedHits.size());
  for (KDTHit hit : sortedHits) {
    sortedIndices.push_back(hit.hitIndex);
  }

  return sortedIndices;

}

HitOrderer::HitOrderer()
{

}

HitOrderer::~HitOrderer()
{

}
