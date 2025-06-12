#pragma once

#include "KDTHit.h"
#include <cstddef>

typedef struct KDTNode {
  KDTHit hit;
  KDTNode* left;
  KDTNode* right;
  bool used;
  int dim;
} KDTNode;