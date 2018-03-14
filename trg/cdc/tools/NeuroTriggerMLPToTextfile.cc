/* This program loads a set of trained MLPs from a rootfile
 * and saves the weights to a textfile with fixed point precision.
 *
 * output format:
 *
 * isector
 * ID0min ID0max ID1min ID1max ... ID8min ID8max
 * sectorpattern patternmask
 * nodes1 nodes2 ... nodesN
 * weight1 weight2 ... weightN
 * ... (repeated for all sectors)
 *
 * ID<i>min/max: ID ranges for super layer <i>
 * sectorpattern/patternmask: for sector selection based on hit pattern
 * nodes<i>: number of nodes in layer <i>
 * weight<i>: numerical value of weight <i>, multiplied by 2^precision
 *
 * weights connect input nodes of one layer to output nodes of the next layer.
 * order:
 * - ordered by layer
 * - within one layer, ordered by index of output node
 * - for fixed output node, ordered by index of input node
 *   (last input node = bias node)
 */

#include <iostream>
#include <fstream>
#include <TFile.h>
#include <TObjArray.h>
#include <trg/cdc/dataobjects/CDCTriggerMLP.h>
#include <cmath>

using namespace std;
using namespace Belle2;

int
main(int argc, char* argv[])
{
  // get arguments
  if (argc < 3) {
    cout << "Program needs at least 3 arguments:" << endl
         << " 1: MLP rootfile" << endl
         << " 2: precision for MLP weights" << endl
         << " 3: output filename" << endl;
    return -1;
  }
  string MLPFilename = argv[1];
  unsigned precisionWeights = atoi(argv[2]);

  TFile MLPFile(MLPFilename.c_str(), "READ");
  if (!MLPFile.IsOpen()) {
    cout << "Could not open file " << MLPFilename << endl;
    return -1;
  }
  TObjArray* MLPs = (TObjArray*)MLPFile.Get("MLPs");
  if (!MLPs) {
    MLPFile.Close();
    cout << "File " << MLPFilename << " does not contain key MLPs" << endl;
    return -1;
  }

  // load MLPs and write them to file
  ofstream weightstream(argv[3]);
  for (int isector = 0; isector < MLPs->GetEntriesFast(); ++isector) {
    CDCTriggerMLP* expert = dynamic_cast<CDCTriggerMLP*>(MLPs->At(isector));
    if (!expert) {
      cout << "Wrong type " << MLPs->At(isector)->ClassName()
           << ", ignoring this entry." << endl;
      continue;
    }
    // write sector number
    weightstream << isector << endl;
    // write ID ranges
    for (unsigned isl = 0; isl < 9; ++isl) {
      weightstream << expert->getIDRange(isl)[0] << " "
                   << expert->getIDRange(isl)[1] << " ";
    }
    weightstream << endl;
    // write sector pattern
    unsigned pattern = expert->getSLpatternUnmasked();
    weightstream << pattern << " " << expert->getSLpatternMask() << endl;
    // write number of nodes per network layer
    vector<unsigned> nNodes = {};
    for (unsigned il = 0; il < expert->nLayers(); ++il) {
      nNodes.push_back(expert->nNodesLayer(il));
      weightstream << nNodes.back() << " ";
    }
    weightstream << endl;
    // write weights and check range
    vector<float> weights = expert->getWeights();
    float minWeight = 0;
    float maxWeight = 0;
    for (unsigned iw = 0; iw < weights.size(); ++iw) {
      double weight = weights[iw];
      // set weights for unused inputs to 0
      if (iw < ((nNodes[0] + 1) * nNodes[1])) {
        unsigned isl = (iw % (nNodes[0] + 1)) / 3;
        if (isl < 9 && !((pattern >> isl) & 1)) weight = 0;
      }
      if (weight < minWeight) minWeight = weight;
      if (weight > maxWeight) maxWeight = weight;
      weightstream << round(weight * (1 << precisionWeights)) << " ";
    }
    weightstream << endl;
    cout << weights.size() << " weights in [" << minWeight << "," << maxWeight << "]" << endl;
  }
  MLPs->Clear();
  delete MLPs;
  MLPFile.Close();
  weightstream.close();

  return 0;
}
