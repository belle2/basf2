/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Christian Pulvermacher                                   *
 *                                                                        *
 **************************************************************************/

#include <tracking/modules/spacePointCreator/SpacePointCreatorModule.h>

#include <framework/datastore/StoreArray.h>


using namespace std;
using namespace Belle2;


REG_MODULE(SpacePointCreator)

SpacePointCreatorModule::SpacePointCreatorModule() : Module()
{
  setDescription("Imports Clusters of the silicon detectors and converts them to spacePoints.");
}

void SpacePointCreatorModule::initialize()
{
  StoreArray<PXDCluster>::optional();
  StoreArray<SpacePoint>::registerPersistent();
}

void SpacePointCreatorModule::event()
{
  StoreArray<PXDCluster> pxdClusters;
  StoreArray<SpacePoint> spacePoints;
  spacePoints.create();

  for (unsigned int i = 0; i < uint(pxdClusters.getEntries()); ++i) {
    spacePoints.appendNew((pxdClusters[i]), i);
  }
}

void SpacePointCreatorModule::terminate()
{
  cerr << "\n";
}
