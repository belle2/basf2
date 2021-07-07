/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <pxd/modules/pxdReconstruction/PXDClusterCheckModule.h>

#include <framework/datastore/StoreArray.h>
#include <framework/datastore/RelationArray.h>
#include <framework/datastore/RelationIndex.h>
#include <framework/logging/Logger.h>

#include <mdst/dataobjects/MCParticle.h>
#include <pxd/dataobjects/PXDDigit.h>
#include <pxd/dataobjects/PXDCluster.h>
#include <pxd/dataobjects/PXDTrueHit.h>

using namespace std;
using namespace Belle2;
using namespace Belle2::PXD;


#define assert_float(A,B) if(!isClose((A),(B)))\
    B2FATAL("Assertion failed: " << #A << " (" << (A) << ") != " << #B << " (" << (B) << ")");
#define assert_eq(A,B) if((A)!=(B))\
    B2FATAL("Assertion failed: " << #A << " (" << (A) << ") != " << #B << " (" << (B) << ")");

namespace {
  /** Small helper function to compare two doubles with a given precision */
  bool isClose(double a, double b, double epsilon = 1e-6)
  {
    return a == b || fabs(a - b) < epsilon || (fabs(a / b) - 1.0) < epsilon;
  }

  /** Check that the ranges returned from to relationindices are identical */
  template<class range> void checkRelation(range a, range b)
  {
    //They should have the same number of elements
    assert_eq(std::distance(begin(a), end(a)), std::distance(begin(b), end(b)));
    //Loop over both ranges and compare index and weights
    for (decltype(begin(a)) ita = begin(a), itb = begin(b); ita != end(a) && itb != end(b); ++ita, ++itb) {
      assert_eq(ita->indexTo, itb->indexTo);
      assert_float(ita->weight, itb->weight);
    }
  }
}

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(PXDClusterCheck);

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

PXDClusterCheckModule::PXDClusterCheckModule() : Module()
{
  //Set module properties
  setDescription("This Modules compares to sets of clusters and their relations "
                 "to make sure they are identical. Intended to cross check Clusterizer. "
                 "Default Collection names are assumed for MCParticles, PXDTrueHits and "
                 "PXDDigits");
  setPropertyFlags(c_ParallelProcessingCertified);
  addParam("clustersOld", m_clustersOld, "Digits collection name", string(""));
  addParam("clustersNew", m_clustersNew, "Digits collection name", string(""));
}

void PXDClusterCheckModule::initialize()
{
  //Mark all StoreArrays as required
  StoreArray<MCParticle> storeMCParticles;
  StoreArray<PXDDigit> storeDigits;
  StoreArray<PXDTrueHit> storeTrueHits;
  StoreArray<PXDCluster> storeClustersOld(m_clustersOld);
  StoreArray<PXDCluster> storeClustersNew(m_clustersNew);
  storeMCParticles.isRequired();
  storeDigits.isRequired();
  storeTrueHits.isRequired();
  storeClustersOld.isRequired();
  storeClustersNew.isRequired();

  //And also all relations
  RelationArray relClustersMCParticlesOld(storeClustersOld, storeMCParticles);
  RelationArray relClustersTrueHitsOld(storeClustersOld, storeTrueHits);
  RelationArray relClustersDigitsOld(storeClustersOld, storeDigits);
  relClustersMCParticlesOld.isRequired();
  relClustersTrueHitsOld.isRequired();
  relClustersDigitsOld.isRequired();

  RelationArray relClustersMCParticlesNew(storeClustersNew, storeMCParticles);
  RelationArray relClustersTrueHitsNew(storeClustersNew, storeTrueHits);
  RelationArray relClustersDigitsNew(storeClustersNew, storeDigits);
  relClustersMCParticlesNew.isRequired();
  relClustersTrueHitsNew.isRequired();
  relClustersDigitsNew.isRequired();
}

void PXDClusterCheckModule::event()
{
  //Obtain all StoreArrays
  const StoreArray<MCParticle> storeMCParticles;
  const StoreArray<PXDDigit> storeDigits;
  const StoreArray<PXDTrueHit> storeTrueHits;
  const StoreArray<PXDCluster> storeClustersOld(m_clustersOld);
  const StoreArray<PXDCluster> storeClustersNew(m_clustersNew);
  //Obtain all relations
  RelationArray relCDOld(storeClustersOld, storeDigits);
  RelationArray relCDNew(storeClustersNew, storeDigits);
  //There might be differences in the order of the pixels so we sort the cluster relation
  if (relCDOld) relCDOld.consolidate();
  if (relCDNew) relCDNew.consolidate();
  //The rest should be sorted anyway so let's just assume they are
  const RelationIndex<PXDCluster, MCParticle> relClustersMCParticlesOld(storeClustersOld, storeMCParticles);
  const RelationIndex<PXDCluster, PXDTrueHit> relClustersTrueHitsOld(storeClustersOld, storeTrueHits);
  const RelationIndex<PXDCluster, MCParticle> relClustersMCParticlesNew(storeClustersNew, storeMCParticles);
  const RelationIndex<PXDCluster, PXDTrueHit> relClustersTrueHitsNew(storeClustersNew, storeTrueHits);

  //We want the same number of Clusters
  assert_eq(storeClustersOld.getEntries(), storeClustersNew.getEntries());
  const unsigned int nCls = storeClustersOld.getEntries();
  //And for each cluster we want one relation to digits, so size should be equal
  assert_eq(relCDOld.getEntries(), (int)nCls);
  assert_eq(relCDNew.getEntries(), (int)nCls);
  for (unsigned int i = 0; i < nCls; ++i) {
    //Loop through clusters and compare all members. Since we want to allow for
    //small deviations we do not use == on the whole cluster but compare the
    //members individually
    const PXDCluster& clsOld = *storeClustersOld[i];
    const PXDCluster& clsNew = *storeClustersNew[i];
    assert_eq(clsOld.getSensorID(), clsNew.getSensorID());
    assert_float(clsOld.getU(), clsNew.getU());
    assert_float(clsOld.getV(), clsNew.getV());
    assert_float(clsOld.getUSigma(), clsNew.getUSigma());
    assert_float(clsOld.getVSigma(), clsNew.getVSigma());
    assert_float(clsOld.getCharge(), clsNew.getCharge());
    assert_float(clsOld.getSeedCharge(), clsNew.getSeedCharge());
    assert_eq(clsOld.getSize(), clsNew.getSize());
    assert_eq(clsOld.getUSize(), clsNew.getUSize());
    assert_eq(clsOld.getVSize(), clsNew.getVSize());
    assert_eq(clsOld.getUStart(), clsNew.getUStart());
    assert_eq(clsOld.getVStart(), clsNew.getVStart());
    assert_float(clsOld.getRho(), clsNew.getRho());

    //Get the corresponding relation element
    const RelationElement reOld = relCDOld[i];
    const RelationElement reNew = relCDNew[i];
    //The relation is sorted so the index should be correct
    assert_eq(reOld.getFromIndex(), i);
    assert_eq(reNew.getFromIndex(), i);
    //And the number of relations should be equal to the cluster size
    assert_eq(reOld.getSize(), clsOld.getSize());
    assert_eq(reNew.getSize(), clsNew.getSize());
    //And the entries should be identical
    for (unsigned int j = 0; j < reOld.getSize(); ++j) {
      assert_eq(reOld.getToIndex(j), reNew.getToIndex(j));
      assert_eq(reOld.getWeight(j), reNew.getWeight(j));
    }
    //Now let's compare the MCParticle and TrueHit relations
    checkRelation(relClustersMCParticlesOld.getElementsFrom(clsOld), relClustersMCParticlesNew.getElementsFrom(clsNew));
    checkRelation(relClustersTrueHitsOld.getElementsFrom(clsOld), relClustersTrueHitsNew.getElementsFrom(clsNew));
  }
}
