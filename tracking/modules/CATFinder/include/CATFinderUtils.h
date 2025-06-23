/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <cstddef>
#include <vector>


namespace Belle2::CATFinderUtils {

  /**
  * @struct KDTHit
  * @brief Lightweight 2D representation of a CDC hit for KD-tree sorting.
  *
  * Represents a CDC hit by its 2D coordinates and a corresponding index.
  */
  typedef struct KDTHit {
    /** X coordinate of the hit. */
    double x;
    /** Y coordinate of the hit. */
    double y;
    /** Index of the hit in the original input collection. */
    int hitIndex;
    /**
     * @brief Compute squared distance to another KDTHit.
     * @param other Another KDTHit.
     * @return Squared Euclidean distance.
     */
    double squaredDistanceTo(const KDTHit& other) const
    {
      double dx = x - other.x;
      double dy = y - other.y;
      return dx * dx + dy * dy;
    }
  } KDTHit;

  /**
  * @struct KDTNode
  * @brief Node structure for a 2D KD-tree used in spatial hit sorting.
  *
  * Represents a node in a binary KD-tree with left and right child pointers,
  * and metadata for axis splitting and usage tracking.
  */
  typedef struct KDTNode {
    /** The hit associated with this node (contains x, y, and index). */
    KDTHit hit;
    /** Pointer to the left subtree (for values less than split). */
    KDTNode* left;
    /** Pointer to the right subtree (for values greater than split). */
    KDTNode* right;
    /** Flag indicating whether the node has already been used in traversal. */
    bool used;
    /** Splitting dimension: 0 for x-axis, 1 for y-axis. */
    int dim;
  } KDTNode;

  /**
  * @class KDTNodePool
  * @brief Memory pool for pre-allocating and reusing KDTNode objects.
  *
  * Improves performance by avoiding dynamic memory allocation during KD-tree construction.
  * Nodes are allocated once at initialization and reset between KD-tree builds.
  */
  class KDTNodePool {
  private:
    /** Internal storage for preallocated KD-tree nodes. */
    std::vector<KDTNode*> pool;
    /** Current index for the next available node in the pool. */
    size_t index;

  public:
    /**
     * @brief Constructs a pool with the specified capacity.
     *
     * Preallocates memory for a given number of KD-tree nodes.
     *
     * @param capacity Maximum number of KDTNodes to allocate.
     */
    KDTNodePool(size_t capacity);

    /**
     * @brief Destructor. Frees all allocated KDTNode objects.
     */
    ~KDTNodePool();

    /** Copy constructor. */
    KDTNodePool(const KDTNodePool&) = delete;

    /** Assignment operator. */
    KDTNodePool& operator=(const KDTNodePool&) = delete;

    /**
     * @brief Allocates and returns the next available KDTNode from the pool.
     *
     * @return Pointer to a KDTNode.
     * @note Logs an error if the pool capacity is exceeded.
     */
    KDTNode* allocate();

    /**
     * @brief Resets the pool index, making all nodes available for reuse.
     *
     * Does not deallocate any memory.
     */
    void reset();

  };

  /**
  * @class HitOrderer
  * @brief Sorts CDC hits spatially using KD-tree nearest neighbor traversal.
  *
  * Constructs a 2D KD-tree from hit coordinates and performs iterative
  * nearest-neighbor selection starting from a reference point to obtain a
  * spatially coherent hit ordering.
  */
  class HitOrderer {

  private:
    /**
     * @brief Recursively builds a balanced KD-tree from a range of KDTHits.
     *
     * The KD-tree is built by recursively selecting median splits along alternating axes (x, y),
     * using either `std::nth_element` or insertion sort for small partitions.
     *
     * @param begin Iterator to the beginning of the KDTHit range.
     * @param end Iterator to the end of the KDTHit range.
     * @param depth Current tree depth, used to alternate axis.
     * @param pool Object pool used to allocate KD-tree nodes.
     * @param INSERTION_SORT_THRESHOLD Switch to insertion sort for small ranges.
     * @return Pointer to the root node of the constructed KD-tree.
     */
    KDTNode* buildKDTree(std::vector<KDTHit>::iterator begin, std::vector<KDTHit>::iterator end, int depth, KDTNodePool& pool,
                         const int INSERTION_SORT_THRESHOLD = 10);

    /**
     * @brief Sorts a small range using insertion sort for performance.
     *
     * @tparam Iterator Type of iterator.
     * @tparam Compare Comparison function type.
     * @param begin Iterator to the beginning of the range.
     * @param end Iterator to the end of the range.
     * @param cmp Comparator function for sorting.
     */
    template<typename Iterator, typename Compare>
    inline void insertionSort(Iterator begin, Iterator end, Compare cmp);


    /**
     * @brief Recursively frees all nodes in a KD-tree.
     *
     * @param node Root node of the KD-tree to deallocate.
     */
    void freeKDTree(KDTNode* node);

    /**
     * @brief Finds the closest unused neighbor to a query hit in the KD-tree.
     *
     * The function recursively traverses the KD-tree and updates the best match
     * and distance if a closer unused hit is found.
     *
     * @param node Root of the KD-tree.
     * @param query Query hit to compare against.
     * @param best Best match found so far (output).
     * @param bestDist Squared distance to the best match (output).
     */
    void nearestNeighbor(KDTNode* node, const KDTHit& query, KDTHit& best, double& bestDist);

    /**
     * @brief Marks a KD-tree node as used if its hit index matches the given hit.
     *
     * Used to ensure that hits are not reused in subsequent nearest-neighbor queries.
     *
     * @param node Root of the KD-tree.
     * @param hit Hit to match by index.
     * @return True if a matching node was found and marked as used.
     */
    bool markUsed(KDTNode* node, const KDTHit& hit);


  public:
    /**
     * @brief Sorts CDC hits spatially based on proximity to a starting position.
     *
     * Builds a KD-tree from the input 2D hit positions and performs an iterative
     * nearest-neighbor traversal to generate a spatially ordered list of hit indices.
     *
     * @param position Starting 2D position (e.g., from condensation point) as {x, y}.
     * @param nodes List of 2D hit positions as {x, y} pairs.
     * @param CDCHitIndices Indices of hits corresponding to the `nodes` vector.
     * @return Vector of hit indices sorted by spatial proximity to the start position.
     */
    std::vector<int> orderHits(std::vector<double> position, std::vector<std::vector<double>> nodes, std::vector<int>& CDCHitIndices);

    /**
     * @brief Constructor.
     */
    HitOrderer() = default;

    /**
     * @brief Destructor.
     */
    ~HitOrderer() = default;
  };

}
