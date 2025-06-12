#pragma once

#include "KDTHit.h"
#include "KDTNode.h"
#include "KDTNodePool.h"
#include <vector>

class HitOrderer {

private:
  KDTNode* buildKDTree(std::vector<KDTHit>::iterator begin, std::vector<KDTHit>::iterator end, int depth, KDTNodePool& pool,
                       const int INSERTION_SORT_THRESHOLD = 10);

  template<typename Iterator, typename Compare>
  inline void insertionSort(Iterator begin, Iterator end, Compare cmp);

  void freeKDTree(KDTNode* node);
  double squaredDistance(const KDTHit& a, const KDTHit& b);
  void nearestNeighbor(KDTNode* node, const KDTHit& query, KDTHit& best, double& bestDist);
  bool markUsed(KDTNode* node, const KDTHit& hit);


public:

  std::vector<int> orderHits(std::vector<double> position, std::vector<std::vector<double>> nodes, std::vector<int>& CDCHitIndices);

  HitOrderer();
  ~HitOrderer();
};

