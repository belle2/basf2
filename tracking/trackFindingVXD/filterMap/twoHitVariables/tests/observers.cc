/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Eugenio Paoloni (eugenio.paoloni@pi.infn.it)             *
 *               Jakob Lettenbichler (jakob.lettenbichler@oeaw.ac.at)     *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <gtest/gtest.h>

#include <tracking/spacePointCreation/SpacePoint.h>
#include <vxd/geometry/SensorInfoBase.h>
#include <tracking/trackFindingVXD/filterMap/twoHitVariables/Distance3DSquared.h>
#include <tracking/trackFindingVXD/filterMap/twoHitVariables/Distance2DXYSquared.h>
#include <tracking/trackFindingVXD/filterMap/twoHitVariables/Distance1DZ.h>
#include <tracking/trackFindingVXD/filterMap/twoHitVariables/SlopeRZ.h>
#include <tracking/trackFindingVXD/filterMap/twoHitVariables/Distance3DNormed.h>

#include <tracking/trackFindingVXD/filterMap/filterFramework/Shortcuts.h>

#include <tracking/vxdCaTracking/TwoHitFilters.h>

#include <svd/dataobjects/SVDCluster.h>
#include <pxd/dataobjects/PXDCluster.h>
#include <pxd/dataobjects/PXDTrueHit.h>
#include <svd/dataobjects/SVDTrueHit.h>
#include <mdst/dataobjects/MCParticle.h>

#include <framework/datastore/StoreArray.h>
#include <framework/dataobjects/EventMetaData.h>
#include <framework/dataobjects/ProfileInfo.h>
#include <framework/datastore/RelationsObject.h>

#include <tuple>
#include <utility>
#include <map>
#include <string>
#include <iostream>
#include <math.h>

#include <functional>

using namespace std;

using namespace Belle2;
/** TODO next steps:
 *
 * - write an observer reading relevant info (like whether two clusters of a spacePoint are related to the same mcParticle)
 * - implement easy container to be filled by observers
 * - write an observer which fills this container
 *
 * - write all the other observers planned
 *
 * */

namespace VXDTFObserversTest {


  /** this is a small helper function to create a sensorInfo to be used */
  VXD::SensorInfoBase provideSensorInfo(VxdID aVxdID, double globalX = 0., double globalY = 0., double globalZ = -0.)
  {
    // (SensorType type, VxdID id, double width, double length, double thickness, int uCells, int vCells, double width2=-1, double splitLength=-1, int vCells2=0)
    VXD::SensorInfoBase sensorInfoBase(VXD::SensorInfoBase::PXD, aVxdID, 2.3, 4.2, 0.3, 2, 4, -1);

    TGeoRotation r1;
    r1.SetAngles(45, 20, 30);      // rotation defined by Euler angles
    TGeoTranslation t1(globalX, globalY, globalZ);
    TGeoCombiTrans c1(t1, r1);
    TGeoHMatrix transform = c1;
    sensorInfoBase.setTransformation(transform);

    return sensorInfoBase;
  }



  /** returns a pxdCluster with given sensorID and local coordinates */
  PXDCluster providePXDCluster(double u, double v, VxdID aVxdID, double uError = 0.1, double vError = 0.1)
  {
    return PXDCluster(aVxdID, u, v, uError, vError, 0, 0, 1, 1, 1, 1 , 1, 1);
  }


  /** returns a svdCluster with given sensorID, uType and local position */
  SVDCluster provideSVDCluster(VxdID aVxdID, bool isU, double position, double error = 0.1)
  {
    return SVDCluster(aVxdID, isU, position, error, 0.1, 0.1, 1, 1, 1, 1);
  }


  /** when given index number for vxdID and global coordinates, a SpacePoint lying there will be returned */
  SpacePoint provideSpacePointDummy(unsigned short i, double X, double Y, double Z)
  {
    VxdID aVxdID = VxdID(i, i, i);
    VXD::SensorInfoBase sensorInfoBase = provideSensorInfo(aVxdID, X, Y, Z);

    PXDCluster aCluster = PXDCluster(aVxdID, 0., 0., 0.1, 0.1, 0, 0, 1, 1, 1, 1, 1, 1);

    return SpacePoint(&aCluster, &sensorInfoBase);
  }



  /** Test class for testing and developing new Observers. */
  class ObserversTest : public ::testing::Test {
  protected:

    /** prepare related storearrays of SpacePoints, SVD- and PXDClusters and MCParticles */
    virtual void SetUp()
    {
      spacePointData.registerInDataStore();
      pxdClusterData.registerInDataStore();
      svdClusterData.registerInDataStore();
      mcParticleData.registerInDataStore();

      spacePointData.registerRelationTo(pxdClusterData);
      pxdClusterData.registerRelationTo(mcParticleData);

      spacePointData.registerRelationTo(svdClusterData);
      svdClusterData.registerRelationTo(mcParticleData);

      DataStore::Instance().setInitializeActive(false);


      for (unsigned int i = 1; i < 3; ++i) { // prepare mcParticles, pxdClusters and spacePoints related to them
        MCParticle* aParticle = mcParticleData.appendNew();

        aParticle->setMomentum(float(i), float(i), float(i));

        VxdID aVxdID = VxdID(i, i, i);

        VXD::SensorInfoBase aSensorInfo = provideSensorInfo(aVxdID, (unsigned short)i, (unsigned short)i + 1., (unsigned short)i + 2.);

        const PXDCluster* pxdCluster = pxdClusterData.appendNew(providePXDCluster(0., 0., aVxdID));
        pxdCluster->addRelationTo(aParticle);

        SpacePoint* newSP = spacePointData.appendNew(pxdCluster, &aSensorInfo);
        B2DEBUG(10, " setup: new spacePoint got arrayIndex: " << newSP->getArrayIndex() << " and VxdID " << newSP->getVxdID());
        newSP->addRelationTo(pxdCluster);
      }

      mcParticleData[0]->setPDG(11);
      mcParticleData[1]->setPDG(13);
      mcParticleData[1]->addStatus(MCParticle::c_PrimaryParticle);


      for (unsigned int i = 3; i < 7; ++i) { // now SVDClusters and their related spacePoints

        VxdID aVxdID = VxdID(i, i, i);

        VXD::SensorInfoBase aSensorInfo = provideSensorInfo(aVxdID, (unsigned short)i, (unsigned short)i + 1., (unsigned short)i + 2.);

        unsigned int pID = (i - 3) / 2;

        const SVDCluster* clusterU = svdClusterData.appendNew(provideSVDCluster(aVxdID, true, 0.));
        clusterU->addRelationTo(mcParticleData[pID]);
        const SVDCluster* clusterV = svdClusterData.appendNew(provideSVDCluster(aVxdID, false, 0.));
        clusterV->addRelationTo(mcParticleData[pID]);

        std::vector<const SVDCluster*> clusterVector = {clusterU, clusterV};

        SpacePoint* newSP = spacePointData.appendNew(clusterVector, &aSensorInfo);
        newSP->addRelationTo(clusterU);
        newSP->addRelationTo(clusterV);
      }

      B2INFO("ObserversTest:SetUP: created " << mcParticleData.getEntries() << "/" << pxdClusterData.getEntries() << "/" <<
             svdClusterData.getEntries() << "/" << spacePointData.getEntries() << " mcParticles/pxdClusters/svdClusters/SpacePoints");
    }



    /** clear datastore */
    virtual void TearDown()
    {
      DataStore::Instance().reset();
    }

    StoreArray<SpacePoint> spacePointData; /**< some spacePoints to test RelationsInterface for observers. */
    StoreArray<PXDCluster> pxdClusterData; /**< some pxd clusters to test RelationsInterface for observers. */
    StoreArray<SVDCluster> svdClusterData; /**< some svd clusters to test RelationsInterface for observers. */
    StoreArray<MCParticle> mcParticleData; /**< some mcParticles to test RelationsInterface for observers. */
  };





  /** a tiny counter class for counting stuff */
  template < class T>
  class counter {
  public:
    static unsigned int used; /**< count number of times used */
    static unsigned int accepted; /**< count number of times result was accepted */
    static unsigned int rejected; /**< count number of times result was rejected */
    static unsigned int wasInf; /**< count number of times result was inf */
    static unsigned int wasNan; /**< count number of times result was nan */
    counter() {}; /**< constructor. */
    ~counter() {}; /**< destructor. */
    static void resetCounter()
    {
      counter<T>::used = 0;
      counter<T>::accepted = 0;
      counter<T>::rejected = 0;
      counter<T>::wasInf = 0;
      counter<T>::wasNan = 0;
    }  /**< resets all counters. */
  };



  /** a container for counting accepted and rejected stuff, just delivers some interfaces to make the code more readable */
  class CountContainer {
  public:
    CountContainer() { m_container.clear(); }  /**< constructor. */

    /** a vector containing IDs */
    typedef std::vector<int> Particles;


    /** key for the internal container of this map
     *
     * .first is a bool which is true, if combination was from the same particle which was primary
     * .second vector of IDs (e.g. PDG or mcParticleIDs)
     */
    typedef std::pair< bool, Particles > Key;


    /** simple struct for counting accepted and rejected cases. */
    struct AcceptRejectPair {
      AcceptRejectPair() : accept(0), reject(0) {}

      /** Increase respective counter if accepted or not
       * @param accepted : bool indicating if accepted or not
       */
      void Increase(bool accepted)
      {
        if (accepted) {
          accept += 1;
        } else {
          reject += 1;
        }
      }

      unsigned accept; /**< counts nTimes when it was accepted */
      unsigned reject; /**< counts nTimes when it was rejected */
    };


    /** clear container */
    void clear() { m_container.clear(); }


    /** size of container */
    unsigned int size() { return m_container.size(); }


    /** cleans a container of double entries */
    template<class ContainerType> static void uniqueIdentifier(ContainerType& particles)
    {
      std::sort(particles.begin(), particles.end());
      auto newEndIterator = std::unique(particles.begin(), particles.end());
      particles.resize(std::distance(particles.begin(), newEndIterator));
    }


    /** accepts a key (first parameter) and if given key was accepted by the filter (second parameter) */
    bool IncreaseCounter(Key& aKey, bool accepted)
    {
      bool keyAlreadyExisted = true;
      auto foundPos = m_container.find(aKey);
      if (foundPos == m_container.end())  {
        B2DEBUG(100, " the IDs " << key2str(aKey) << " collected haven't been found yet");
        foundPos = m_container.insert({aKey, AcceptRejectPair() }).first;
        keyAlreadyExisted = false;
      } else { B2DEBUG(100, " the IDs " << key2str(aKey) << " collected were already there"); }

      foundPos->second.Increase(accepted);

      return keyAlreadyExisted;
    }



    /** for given key, the function returns the result found. If key was not found in container, a AcceptRejectPair with 0, 0 is returned */
    AcceptRejectPair ReturnResult(const Key& givenKey)
    {

      auto foundPos =  m_container.find(givenKey);

      if (foundPos == m_container.end())  {
        return AcceptRejectPair();
      }

      return foundPos->second;
    }



    /** for given key, the function returns the result found. If key was not found in container, a AcceptRejectPair with 0, 0 is returned */
    AcceptRejectPair ReturnResult(const Particles& givenKey)
    {
      for (const auto& aKey : m_container) {
        B2DEBUG(100, "comparing given particles: " << vec2str(givenKey) << " with entry: " << vec2str(
                  aKey.first.second) << " with result " << (aKey.first.second == givenKey));
        if (aKey.first.second == givenKey) { return aKey.second; }
      }

      return AcceptRejectPair();
    }



    /** for easy printing of results collected so far */
    void PrintResults(string identifier = "unknown")
    {
      for (auto& entry : m_container) {
        B2WARNING(" for " << identifier << "-combination: " <<
                  key2str(entry.first) <<
                  ", combi was accepted/rejected: " <<
                  entry.second.accept <<
                  "/" <<
                  entry.second.reject);
      }
    }



    /** small helper for easily printing vectors */
    static std::string key2str(const Key& aKey)
    {
      return key2str(&aKey);
    }


    /** small helper for easily printing vectors */
    static std::string key2str(const Key* aKey)
    {
      string output;

      if (aKey->first == true) { output += "GoodCombi: "; }
      else { output += "BadCombi: "; }

      output += vec2str(aKey->second);

      return output;
    }


    /** small helper for easily printing vectors */
    template<class Type> static std::string vec2str(const vector<Type>& vec)
    {
      string output;

      for (const auto& entry : vec) {
        output += " " + std::to_string(entry);
      }

      return output;
    }
  protected:
    std::map< Key, AcceptRejectPair> m_container; /**< collects the data */
  };



  /** a tiny counter class for counting stuff retrieved from MC-bla */
  template < class T>
  class counterMC {
  public:
    static CountContainer
    pdGacceptedRejected; /** map for pdgCodes (key: vector of pdgCodes found for given hits, sorted) storing how often it was accepted/rejected  value.first/value.second */
    static CountContainer
    mcIDacceptedRejected; /** map for mcParticleIDs (key, vector of mcParticleIDs (pair: first: true, if combination was from the same particle which was primary, found for given hits, sorted), storing how often it was accepted/rejected value.first/value.second */
    counterMC() {};  /**< constructor. */
    ~counterMC() {};  /**< destructor. */
    static void resetCounter()
    {
      counterMC<T>::pdGacceptedRejected.clear();
      counterMC<T>::mcIDacceptedRejected.clear();
    }  /**< resets counters. */
  };



  /** Initialise all static variables */
  template<class T> unsigned int counter<T>::used(0);
  template<class T> unsigned int counter<T>::accepted(0);
  template<class T> unsigned int counter<T>::rejected(0);
  template<class T> unsigned int counter<T>::wasInf(0);
  template<class T> unsigned int counter<T>::wasNan(0);

  template<class T> CountContainer counterMC< T >::pdGacceptedRejected =
    CountContainer(); /**< counts nCases accepted/rejected for each pdgCode-combination occured */
  template<class T> CountContainer counterMC< T >::mcIDacceptedRejected =
    CountContainer(); /**< counts nCases accepted/rejected for each particleID-combination occured */





  /** this observer does simply count the number of times, the attached SelectionVariable was used */
  class CountUsedObserver : public VoidObserver {
  public:
    /** notifier counting how often a SelectionVariable was used */
    template<class Var, typename ... otherTypes>
    static void notify(const Var&,
                       otherTypes ...)
    {
      counter<Var>::used ++ ;
    }
  };





  /** this observer does simply count the number of times, the attached SelectionVariable was accepted or rejected */
  class CountAcceptRejectObserver : public VoidObserver {
  public:
    /** notifier counting how often a SelectionVariable was accepted/rejected */
    template<class Var, class RangeType>
    static void notify(const Var&,
                       typename Var::variableType fResult,
                       const RangeType& range,
                       const typename Var::argumentType&,
                       const typename Var::argumentType&)
    {
      if (range.contains(fResult)) {
        counter<Var>::accepted ++;
      } else {
        counter<Var>::rejected ++;
      }
    }
  };





  /** this observer does simply count the number of times, the attached Filter resulted in isinf or isnan */
  class CountBadCaseObserver : public VoidObserver {
  public:
    /** notifier counting how often a SelectionVariable was resulting in nan or inf */
    template<class Var, typename ... otherTypes>
    static void notify(const Var&,
                       typename Var::variableType fResult,
                       otherTypes ...)
    {
      if (std::isinf(fResult)) {
        counter<Var>::wasInf ++;
      } else if (std::isnan(fResult)) {
        counter<Var>::wasNan ++;
      }
    }
  };





  /** this observer does simply print the name of the SelectionVariable and the result of its value-function as a Warning */
  class InfoObserver : public VoidObserver {
  public:
    /** notifier producing a info message if SelectionVariable was accepted and a Warning if otherwise */
    template<class Var, class RangeType>
    static void notify(const Var& filterType,
                       typename Var::variableType fResult,
                       const RangeType& range,
                       const typename Var::argumentType& outerHit,
                       const typename Var::argumentType& innerHit)
    {

      stringstream outputStream;
      outputStream << filterType.name()
                   << " with outer-/innerhit: "
                   << outerHit.getPosition().PrintStringXYZ()
                   << "/"
                   << innerHit.getPosition().PrintStringXYZ()
                   << " having indices "
                   << outerHit.getArrayIndex()
                   << "/"
                   << innerHit.getArrayIndex()
                   << " and VxdIDs "
                   << outerHit.getVxdID()
                   << "/"
                   << innerHit.getVxdID()
                   << " results in "
                   << fResult
                   << " & accepted: "
                   << (range.contains(fResult) ? string("true") : string("false"))
                   << " in range "
                   << range.getInf()
                   << "/"
                   << range.getSup();

      if (range.contains(fResult)) {
        B2INFO(outputStream.str());
      } else {
        B2WARNING(outputStream.str());
      }
    }
  };




  /** this observer identifies the McParticles responsible for creating underlying clusters for given spacePoints and counts how often a particle type was accepted and how often it was rejected */
  class CountAcceptedRejectedMCParticleObserver : public VoidObserver {
  public:

    /** notifier producing a info message if SelectionVariable was accepted and a Warning if otherwise */
    template<class Var, class RangeType>
    static void notify(const Var&,
                       typename Var::variableType fResult,
                       const RangeType& range,
                       const typename Var::argumentType& outerHit,
                       const typename Var::argumentType& innerHit)
    {
      B2INFO("CountAcceptedRejectedMCParticleObserver called"  << endl
             << "range: (  " << range.getInf() << " , " << range.getSup() << " )" << endl
             << "var =     " << fResult << endl
             << "outerHit: (" << outerHit.X() << " , "
             << outerHit.Y() << " , "
             << outerHit.Z() << " ) (x,y,z) " << endl
             << "innerHit: (" << innerHit.X() << " , "
             << innerHit.Y() << " , "
             << innerHit.Z() << " ) (x,y,z) " << endl
            );

      // we use the pdgCodes of the particles as identifier (not unique, since more than one primary particle can have the same PDG-code) and count their acceptance rate:
      CountContainer::Key myPDGs = createKey(outerHit, innerHit, true);

      counterMC< Var >::pdGacceptedRejected.IncreaseCounter(myPDGs, range.contains(fResult));


      // now we do the same but with mcParticleIDs:

      CountContainer::Key myPIDs = createKey(outerHit, innerHit, false);

      counterMC< Var >::mcIDacceptedRejected.IncreaseCounter(myPIDs, range.contains(fResult));
    }



    /** collects all PDGs connected to given hit.
    *
    * does only work, if hit has direct relations PXD/SVDCluster which have relations to MCParticle.
    *
    * first parameter is a hit (e.g. spacePoint) which is related to MCParticles.
    * second parameter is the vector for collecting the PDGcodes found.
    * */
    template <class hitType>
    static void collectPDGs(const hitType& aHit, vector< pair< bool, int> >& collectedPDGs)
    {

      std::vector<const MCParticle*> collectedParticles;

      collectMCParticles(aHit, collectedParticles);

      for (const MCParticle* aParticle : collectedParticles) {
        collectedPDGs.push_back({aParticle->hasStatus(MCParticle::c_PrimaryParticle), aParticle->getPDG()});
      }
    }



    /** collects all particleIDs connected to given hit.
    *
    * does only work, if hit has direct relations PXD/SVDCluster which have relations to MCParticle.
    *
    * first parameter is a hit (e.g. spacePoint) which is related to MCParticles.
    * second parameter is the vector for collecting the PDGcodes found.
    * */
    template <class hitType>
    static void collectParticleIDs(const hitType& aHit, vector< pair< bool, int> >& collectedIDS)
    {

      std::vector<const MCParticle*> collectedParticles;

      collectMCParticles(aHit, collectedParticles);

      for (const MCParticle* aParticle : collectedParticles) {
        collectedIDS.push_back({aParticle->hasStatus(MCParticle::c_PrimaryParticle), aParticle->getIndex()});
      }
    }

    /** for two hits given, a key for the CountContainer is returned. if usePDG == true, PDGcode will be used as identifyer, if false, the ParticleID will be used */
    template <class hitType>
    static CountContainer::Key createKey(const hitType& hitA, const hitType& hitB, bool usePDG)
    {
      CountContainer::Key newKey;

      vector< pair< bool, int> > collectedIDS;

      if (usePDG) {
        collectPDGs(hitA, collectedIDS);
        collectPDGs(hitB, collectedIDS);
      } else {
        collectParticleIDs(hitA, collectedIDS);
        collectParticleIDs(hitB, collectedIDS);
      }

      CountContainer::uniqueIdentifier(collectedIDS);

      if (collectedIDS.size() == 1) {
        newKey.first = collectedIDS[0].first;
        newKey.second = { collectedIDS[0].second };
        return newKey;
      }

      newKey.first = false;

      for (auto& entry : collectedIDS) {
        newKey.second.push_back(entry.second);
      }

      return newKey;
    }


    /** collects all mcParticles connected to given hit.
    *
    * does only work, if hit has direct relations PXD/SVDCluster which have relations to MCParticle.
    *
    * first parameter is a hit (e.g. spacePoint) which is related to MCParticles.
    * second parameter is the vector for collecting the PDGcodes found.
    * */
    template <class hitType>
    static void collectMCParticles(const hitType& aHit, std::vector<const MCParticle*>& collectedParticles)
    {
      RelationVector<PXDCluster> relatedToPXDClusters = aHit.template getRelationsTo<PXDCluster>();
      RelationVector<SVDCluster> relatedToSVDClusters = aHit.template getRelationsTo<SVDCluster>();

      for (const PXDCluster& aCluster : relatedToPXDClusters) {
        for (const MCParticle& aParticle : aCluster.getRelationsTo<MCParticle>()) {
          collectedParticles.push_back(&aParticle);
        }
      }

      for (const SVDCluster& aCluster : relatedToSVDClusters) {
        for (const MCParticle& aParticle : aCluster.getRelationsTo<MCParticle>()) {
          collectedParticles.push_back(&aParticle);
        }
      }
    }



    /** small helper for easily printing vectors */
    template<class Type> static std::string vec2str(const vector<Type>& vec)
    {
      string output;

      for (const auto& entry : vec) {
        output += " " + std::to_string(entry);
      }

      return output;
    }
  };


  /** this observer combines all the easy-to-get info retrievable by observers and prints it to screen */
  class ProvideBasicInfoObserver : public VoidObserver {
  public:
    /** notify function is called by the filter, this one combines all the easy-to-get info retrievable by observers and prints it to screen. */
    template<class Var, class RangeType>
    static void notify(const Var& filterType,
                       typename Var::variableType fResult,
                       const RangeType& range,
                       const typename Var::argumentType& outerHit,
                       const typename Var::argumentType& innerHit)
    {
      CountUsedObserver::notify(filterType);
      CountAcceptRejectObserver::notify(filterType, fResult, range, outerHit, innerHit);
      CountBadCaseObserver::notify(filterType, fResult, range, outerHit, innerHit);
      InfoObserver::notify(filterType, fResult, range, outerHit, innerHit);
    }
  };





  /**
   * WARNING TODO:
   * - BasicRootObserver (same as BasicInfoObserver but with storing info to container instead of printing it to screen (not working here, since trackNodes are needed for this (secMapID, secIDs)
   * - MCInfoObserver and subparts (mcParticleID, pT, isPrimary, purity level, pdgCode)
   * - MCRootObserver like BasicRootObserver but for MCInfoObserver instead of BasicInfoObserver
   *
   * - How to get secID and secMapID? -> trackNode
   * - How to get mcParticleID, pT, isPrimary, pdgCode? -> trackNode->SpacePoint->relation< MCParticle >
   * - How to get purity? -> advanced version of question before
   * - How to get the stuff into root-files? -> Observers store into static containers which are frequently emptied into another container which finally will be written to a root file
   * */


  /** this observer does simply collect other observers which are to be executed during ::notify */
  template<class FilterType> class VectorOfObservers : public VoidObserver {
  public:

    /** a typedef to make the stuff more readable */
    typedef std::function< void (const FilterType&, typename FilterType::variableType, const typename FilterType::argumentType&, const typename FilterType::argumentType&)>
    observerFunction;

    /** a typedef to make the c-style pointer more readable (can not be done with classic typedef) */
    using CStyleFunctionPointer = void(*)(const typename FilterType::argumentType&, const typename FilterType::argumentType&,
                                          const FilterType&, typename FilterType::variableType) ;

    /** iterate over all stored Observers and execute their notify-function */
    template<typename range, typename ... otherTypes>
    static void notify(const FilterType& filterType,
                       typename FilterType::variableType filterResult,
                       const range&,
                       const typename FilterType::argumentType& outerHit,
                       const typename FilterType::argumentType& innerHit,
                       otherTypes ...)
    {
      B2INFO(" Filter " << filterType.name() << " with Mag of outer-/innerHit " << outerHit.getPosition().Mag() << "/" <<
             innerHit.getPosition().Mag() << " got result of " << filterResult << " and Observer-Vector sm_collectedObservers got " <<
             VectorOfObservers<FilterType>::sm_collectedObservers.size() << " observers collected");
      B2INFO(" Filter " << filterType.name() << " with Mag of outer-/innerHit " << outerHit.getPosition().Mag() << "/" <<
             innerHit.getPosition().Mag() << " got result of " << filterResult << " and Observer-Vector sm_collectedObserversCSTYLE got " <<
             VectorOfObservers<FilterType>::sm_collectedObserversCSTYLE.size() << " observers collected");

      /// the idea of the following three lines have to work in the end (I basically want to loop over all attached observers and execute their notify function):
      //    for(auto& anObserver : CollectedObservers<FilterType>::collectedObservers) {
      //    anObserver(outerHit, innerHit, filterResult);
      //    }
      /// or
    }



    /** collects observers to be executed during notify (can not be used so far, but is long-term goal) */
    template <typename ObserverType>
    static void addObserver(observerFunction newObserver)
    {
      VectorOfObservers<FilterType>::sm_collectedObservers.push_back(std::bind(&newObserver, std::placeholders::_1, std::placeholders::_2,
          FilterType(), std::placeholders::_3));
    }



    /** collects observers with std::function to be executed during notify (variant A)*/
    static std::vector< observerFunction > sm_collectedObservers;
    /** collects observers with c-style function pointers to be executed during notify (variant B)*/
    static std::vector< CStyleFunctionPointer > sm_collectedObserversCSTYLE;

    /** demangled typeID used as type for the Vector */
//  static std::vector< std::_Bind<void (*(std::_Placeholder<1>, std::_Placeholder<2>, Belle2::Distance3DSquared, std::_Placeholder<3>))(Belle2::SpacePoint const&, Belle2::SpacePoint const&, Belle2::Distance3DSquared const&, float)> > sm_collectedObserversTry2;
  };


  /** initialize static member of variant A*/
  template<typename FilterType> std::vector< typename VectorOfObservers<FilterType>::observerFunction >
  VectorOfObservers<FilterType>::sm_collectedObservers  = {};
  /** initialize static member of variant B*/
  template<typename FilterType> std::vector< typename VectorOfObservers<FilterType>::CStyleFunctionPointer >
  VectorOfObservers<FilterType>::sm_collectedObserversCSTYLE  = {};
  /** initialize static member of variant Try2*/
//   template<typename FilterType> std::vector< typename VectorOfObservers<FilterType>::CStyleFunctionPointer > VectorOfObservers<FilterType>::sm_collectedObserversTry2  = {};





  /** tests whether the setup of relations is doing what it is supposed to do */
  TEST_F(ObserversTest, TestRelationSetup)
  {
    EXPECT_EQ(6, spacePointData.getEntries());

    for (SpacePoint& aSP : spacePointData) {
      unsigned nullptrTrap = 0;
      RelationVector<PXDCluster> pxDClusters = aSP.getRelationsTo<PXDCluster>();
      for (PXDCluster& aCluster : pxDClusters) {
        MCParticle* aParticle = aCluster.getRelatedTo<MCParticle>();
        if (aParticle == NULL) { nullptrTrap = 1; }
        EXPECT_EQ(0, nullptrTrap);

        EXPECT_EQ(aSP.getArrayIndex(), aCluster.getArrayIndex());
        EXPECT_EQ(aSP.getArrayIndex(), aParticle->getArrayIndex());
        EXPECT_EQ(int(aParticle->getMomentum().X()), aParticle->getArrayIndex() + 1);
        EXPECT_EQ(int(aParticle->getMomentum().Y()), aParticle->getArrayIndex() + 1);
        EXPECT_EQ(int(aParticle->getMomentum().Z()), aParticle->getArrayIndex() + 1);

        nullptrTrap = 0;
      }

      RelationVector<SVDCluster> svDClusters = aSP.getRelationsTo<SVDCluster>();
      for (SVDCluster& aCluster : svDClusters) {
        MCParticle* aParticle = aCluster.getRelatedTo<MCParticle>();
        if (aParticle == NULL) { nullptrTrap = 2; }
        EXPECT_EQ(0, nullptrTrap);

        EXPECT_EQ(aSP.getArrayIndex(), 2 + aCluster.getArrayIndex() / 2);
        EXPECT_EQ(aSP.getArrayIndex() / 4, aParticle->getArrayIndex());
        EXPECT_EQ(int(aParticle->getMomentum().X()), aParticle->getArrayIndex() + 1);
        EXPECT_EQ(int(aParticle->getMomentum().Y()), aParticle->getArrayIndex() + 1);
        EXPECT_EQ(int(aParticle->getMomentum().Z()), aParticle->getArrayIndex() + 1);

        nullptrTrap = 0;
      }

      // at least one cluster type has to be connected to given spacePoint!
      EXPECT_NE(0, pxDClusters.size() + svDClusters.size());
    }
  }





  /** tests possibility of accessing MCData */
  TEST_F(ObserversTest, TestMCDataAccess)
  {
    Filter< Distance3DSquared<SpacePoint>, Range<double, double>, VoidObserver > unobservedFilter(Range<double, double>(2.5,
        3.5));
    Filter< Distance3DSquared<SpacePoint>, Range<double, double>, CountAcceptedRejectedMCParticleObserver > observedFilter(
      unobservedFilter);
    auto mcCounter = counterMC< Distance3DSquared<SpacePoint> >();

    for (int i = 1 ; i < spacePointData.getEntries(); i++) {
      SpacePoint& spA = *spacePointData[i];
      SpacePoint& spB = *spacePointData[i - 1];
      B2DEBUG(10, "spData-Sps got arraIndices: " << spacePointData[i]->getArrayIndex() << "/" << spacePointData[i - 1]->getArrayIndex() <<
              " and VxdIDs " << spacePointData[i]->getVxdID() << "/" << spacePointData[i - 1]->getVxdID());
      observedFilter.accept(spA, spB);
    }

    EXPECT_EQ(3, mcCounter.pdGacceptedRejected.size());
    EXPECT_EQ(3, mcCounter.mcIDacceptedRejected.size());

    mcCounter.pdGacceptedRejected.PrintResults("pdgCode");

    /// TODO further tests!
    // the filter was too loose, everything is accepted
    EXPECT_EQ(1, mcCounter.pdGacceptedRejected.ReturnResult(CountContainer::Particles{11}).accept);
    EXPECT_EQ(3, mcCounter.pdGacceptedRejected.ReturnResult(CountContainer::Particles{11, 13}).accept);
    EXPECT_EQ(1, mcCounter.pdGacceptedRejected.ReturnResult(CountContainer::Particles{13}).accept);
    // and nothing was rejected...
    EXPECT_EQ(0, mcCounter.pdGacceptedRejected.ReturnResult(CountContainer::Particles{11}).reject);
    EXPECT_EQ(0, mcCounter.pdGacceptedRejected.ReturnResult(CountContainer::Particles{11, 13}).reject);
    EXPECT_EQ(0, mcCounter.pdGacceptedRejected.ReturnResult(CountContainer::Particles{13}).reject);


    mcCounter.mcIDacceptedRejected.PrintResults("pID");

    EXPECT_EQ(1, mcCounter.mcIDacceptedRejected.ReturnResult(CountContainer::Particles{1}).accept);
    EXPECT_EQ(3, mcCounter.mcIDacceptedRejected.ReturnResult(CountContainer::Particles{1, 2}).accept);
    EXPECT_EQ(1, mcCounter.mcIDacceptedRejected.ReturnResult(CountContainer::Particles{2}).accept);

    EXPECT_EQ(0, mcCounter.mcIDacceptedRejected.ReturnResult(CountContainer::Particles{1}).reject);
    EXPECT_EQ(0, mcCounter.mcIDacceptedRejected.ReturnResult(CountContainer::Particles{1, 2}).reject);
    EXPECT_EQ(0, mcCounter.mcIDacceptedRejected.ReturnResult(CountContainer::Particles{2}).reject);


    // now we set the filter using values which are too strict
    Filter< Distance3DSquared<SpacePoint>, Range<double, double>, VoidObserver > unobservedFilterStrict(Range<double, double>
        (3.1, 3.5));
    Filter< Distance3DSquared<SpacePoint>, Range<double, double>, CountAcceptedRejectedMCParticleObserver >
    observedFilterStrict(
      unobservedFilterStrict);

    for (int i = 1 ; i < spacePointData.getEntries(); i++) {
      SpacePoint& spA = *spacePointData[i];
      SpacePoint& spB = *spacePointData[i - 1];
      observedFilterStrict.accept(spA, spB);
    }

    EXPECT_EQ(3, mcCounter.pdGacceptedRejected.size());
    EXPECT_EQ(3, mcCounter.mcIDacceptedRejected.size());

    // first filter does write into the same container, therefore values didn't change for accepted
    EXPECT_EQ(1, mcCounter.pdGacceptedRejected.ReturnResult(CountContainer::Particles{11}).accept);
    EXPECT_EQ(3, mcCounter.pdGacceptedRejected.ReturnResult(CountContainer::Particles{11, 13}).accept);
    EXPECT_EQ(1, mcCounter.pdGacceptedRejected.ReturnResult(CountContainer::Particles{13}).accept);

    // second filter does reject everything:
    EXPECT_EQ(1, mcCounter.pdGacceptedRejected.ReturnResult(CountContainer::Particles{11}).reject);
    EXPECT_EQ(3, mcCounter.pdGacceptedRejected.ReturnResult(CountContainer::Particles{11, 13}).reject);
    EXPECT_EQ(1, mcCounter.pdGacceptedRejected.ReturnResult(CountContainer::Particles{13}).reject);

    EXPECT_EQ(1, mcCounter.mcIDacceptedRejected.ReturnResult(CountContainer::Particles{1}).accept);
    EXPECT_EQ(3, mcCounter.mcIDacceptedRejected.ReturnResult(CountContainer::Particles{1, 2}).accept);
    EXPECT_EQ(1, mcCounter.mcIDacceptedRejected.ReturnResult(CountContainer::Particles{2}).accept);

    EXPECT_EQ(1, mcCounter.mcIDacceptedRejected.ReturnResult(CountContainer::Particles{1}).reject);
    EXPECT_EQ(3, mcCounter.mcIDacceptedRejected.ReturnResult(CountContainer::Particles{1, 2}).reject);
    EXPECT_EQ(1, mcCounter.mcIDacceptedRejected.ReturnResult(CountContainer::Particles{2}).reject);

  }





  /** presents the functionality of the SpacePoint-creating function written above */
  TEST_F(ObserversTest, TestObserverFlexibility)
  {
    // Very verbose declaration, see below for convenient shortcuts
    Filter< Distance3DSquared<SpacePoint>, Range<double, double>, VoidObserver > unobservedFilter(Range<double, double>(0.,
        1.));

    //     Filter< Distance3DSquared<SpacePoint>, Range<double, double>, VectorOfObservers<Distance3DSquared> > filter(unobservedFilter);
    Filter< Distance3DSquared<SpacePoint>, Range<double, double>, InfoObserver > observedFilter(unobservedFilter);
    SpacePoint x1 = provideSpacePointDummy(1, 0.0f , 0.0f, 0.0f);
    SpacePoint x2 = provideSpacePointDummy(1, 0.5f , 0.0f, 0.0f);
    SpacePoint x3 = provideSpacePointDummy(1, 2.0f , 0.0f, 0.0f);
    auto myCounter = counter<Distance3DSquared<SpacePoint>>();
    myCounter.resetCounter();

    /// variant A (doesn't work, because of static function(?)):
//  auto storeFuncVariantA = std::bind( ((VectorOfObservers<Distance3DSquared>::observerFunction) &CountingObserver::notify), std::placeholders::_1, std::placeholders::_2, Distance3DSquared(), std::placeholders::_3);
    //  VectorOfObservers<Distance3DSquared>::sm_collectedObservers.push_back(storeFuncVariantA);

    /// variant B:
    auto storeFuncVariantB = std::bind(((VectorOfObservers<Distance3DSquared<SpacePoint>>::CStyleFunctionPointer)
                                        &CountUsedObserver::notify), std::placeholders::_1, std::placeholders::_2, Distance3DSquared<SpacePoint>(),
                                       std::placeholders::_3);

    char* realname(NULL);
    int status(0);
    realname = abi::__cxa_demangle(typeid(storeFuncVariantB).name(), 0, 0, &status);
    std::string name(realname);
    free(realname);
    B2INFO("storeFuncVariantB is of type: " << name);

//  VectorOfObservers<Distance3DSquared>::sm_collectedObserversTry2.push_back(storeFuncVariantB);


    /// doesn't work, different type, additionally static_cast doesn't work too (reinterpret_casat too dangerous, didn't try):
//  VectorOfObservers<Distance3DSquared>::sm_collectedObserversCSTYLE.push_back(storeFuncVariantB);

    /// long-term goal, something comparable to this:
    // VectorOfObservers<Distance3DSquared>::addObserver(CountUsedObserver);
    // VectorOfObservers<Distance3DSquared>::addObserver(WarningObserver);

    observedFilter.accept(x2, x1);
    observedFilter.accept(x3, x1);
    EXPECT_EQ(0 , myCounter.used);

    Filter< Distance3DSquared<SpacePoint>, Range<double, double>, ProvideBasicInfoObserver > anotherObservedFilter(
      unobservedFilter);

    anotherObservedFilter.accept(x2, x1);
    anotherObservedFilter.accept(x3, x1);
    EXPECT_EQ(2 , myCounter.used);
    EXPECT_EQ(1 , myCounter.accepted);
    EXPECT_EQ(1 , myCounter.rejected);
    EXPECT_EQ(0 , myCounter.wasInf);
    EXPECT_EQ(0 , myCounter.wasNan);
  }





  /** ignore what is written in this test, is simply used as a temporary storage for currently not used code-snippets */
  TEST_F(ObserversTest, ignoreMe)
  {
    //garbage:
    //  VectorOfObservers<Distance3DSquared>::addObserver(CountUsedObserver::notify);
    //  auto storeFunc = std::bind((void(*)(const SpacePoint&, const SpacePoint&, const Belle2::Distance3DSquared&, float))&CountUsedObserver::notify, std::placeholders::_1, std::placeholders::_2, Distance3DSquared(), std::placeholders::_3);
    //  auto storeFunc1 = std::bind(void(*)(const typename Distance3DSquared::argumentType&, const typename Distance3DSquared::argumentType&, const Distance3DSquared&, typename Distance3DSquared::variableType) /*&CountUsedObserver::notify*/), std::placeholders::_1, std::placeholders::_2, Distance3DSquared(), std::placeholders::_3);

//  ObserverVector<Distance3DSquared>::sm_collectedObservers.push_back(storeFunc);
    //  ObserverVector<Distance3DSquared>::sm_collectedObservers.push_back(std::bind((void(*)(const SpacePoint&, const SpacePoint&, const Belle2::Distance3DSquared&, float))&CountUsedObserver::notify, std::placeholders::_1, std::placeholders::_2, Distance3DSquared(), std::placeholders::_3));
    // std::vector< typename ObserverVector<FilterType>::observerFunction >();
    //     ObserverVector<VectorOfObservers>::sm_collectedObservers = std::vector< typename VectorOfObservers<Distance3DSquared>::observerFunction >();
//  ErrorObserver anErrorBla();
    //  WarningObserver aWarningBla();
    //  ObserverVector<Distance3DSquared>::addObserver(aWarningBla/*, filterBla*/);
    //  ObserverVector<Distance3DSquared>::addObserver(aBla/*, filterBla*/);
    // // // // // //   Distance3DSquared filterBla();

    /** a tiny CollectedObservers class for CollectedObservers stuff */
//   template<class FilterType> class CollectedObservers {
//   public:
//  typedef std::function< void (const typename FilterType::argumentType&, const typename FilterType::argumentType&, const FilterType&, typename FilterType::variableType)> observerFunction;
//
//  /** collects observers to be executed during notify */
//  static std::vector< observerFunction > collectedObservers;
//  CollectedObservers() {};
//  ~CollectedObservers() {};
//   };

//  static void addObserver( CountUsedObserver& newObserver) {
//    //  static void addObserver(observerFunction newObserver) {
//    ObserverVector<FilterType>::sm_collectedObservers.push_back(std::bind(&newObserver::notify, std::placeholders::_1, std::placeholders::_2, FilterType(), std::placeholders::_3));
//  }
  }

}
