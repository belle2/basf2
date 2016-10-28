/// WARNING JKL: temporarily commented out until I have copied the relevant stuff into VXDTFModule again.
// // /**************************************************************************
// //
// //  * BASF2 (Belle Analysis Framework 2)                                     *
// //  * Copyright(C) 2011 - Belle II Collaboration                             *
// //  *                                                                        *
// //  * Author: The Belle II Collaboration                                     *
// //  * Contributors: Jakob Lettenbichler                                      *
// //  *                                                                        *
// //  * This software is provided "as is" without any warranty.                *
// //  **************************************************************************/
// // #include <iostream>
// //
// // #include "tracking/modules/vxdtfRedesign/TFRedesignModule.h"
// // /// JKL Feb2 2016: commented out for being able to continue working:
// // // // // // // #include <tracking/trackFindingVXD/sectorMap/map/SectorMap.h>
// // // // // // // #include "tracking/trackFindingVXD/environment/VXDTFFilters.h"
// // // // // // // #include "tracking/trackFindingVXD/environment/FilterSetTypes.h"
// //
// // // framework
// // #include <framework/gearbox/Const.h>
// // #include <framework/gearbox/GearDir.h> // needed for reading xml-files
// // #include <framework/core/Environment.h> // getNumberProcesses
// // #include <geometry/GeometryManager.h>
// // #include <geometry/bfieldmap/BFieldMap.h>
// // #include <tracking/dataobjects/VXDTFSecMap.h>
// // #include <vxd/geometry/GeoCache.h>
// // #include <vxd/geometry/SensorInfoBase.h>
// // #include <framework/datastore/StoreObjPtr.h>
// //
// //
// // #include <svd/reconstruction/SVDRecoHit2D.h>
// // #include <svd/reconstruction/SVDRecoHit.h>
// // #include <pxd/reconstruction/PXDRecoHit.h>
// // #include <tracking/gfbfield/GFGeant4Field.h>
// //
// // #include "tracking/dataobjects/FilterID.h"
// // #include <tracking/spacePointCreation/SpacePoint.h>
// // #include <tracking/trackFindingVXD/sectorMapTools/SectorTools.h>
// //
// // //genfit
// //
// // #include <genfit/Track.h>
// // #include <genfit/AbsTrackRep.h>
// // #include <genfit/RKTrackRep.h>
// // #include <genfit/FieldManager.h>
// // #include <genfit/Tools.h>
// // #include <genfit/KalmanFitterRefTrack.h>
// // #include <genfit/KalmanFitter.h>
// // #include <genfit/MaterialEffects.h>
// //
// //
// // //root packages:
// // #include <TMatrixT.h>
// // #include <TMatrixD.h>
// // #include <TGeoManager.h>
// // #include <TMath.h>
// //
// // //STL or C/C++-packages;
// // #include <algorithm>
// // #include <functional>
// // #include <utility>
// // #include <math.h>
// // #include <time.h>
// // #include <fstream>
// // #include <iomanip>      // std::setprecision
// // #include <array>
// //
// //
// // //Boost-packages:
// // #include "boost/tuple/tuple_comparison.hpp" // needed for sorting tuples
// // #include <boost/foreach.hpp>
// // #include <boost/bind/bind.hpp>
// // #include <boost/format.hpp>
// // #include <boost/algorithm/string/split.hpp>
// // #include <boost/algorithm/string/classification.hpp> // needed for is_any_of
// // #include <boost/lexical_cast.hpp>
// // #include <boost/math/special_functions/sign.hpp>
// // #include <boost/lambda/bind.hpp>
// // #include <boost/lambda/lambda.hpp>
// // #include <boost/lambda/casts.hpp>
// // // #include <boost/chrono/chrono_io.hpp>
// //
// // // #include <valgrind/callgrind.h>
// //
// // #ifdef HAS_CALLGRIND
// // #include <valgrind/callgrind.h>
// // #endif
// //
// //
// // using namespace std;
// // using namespace Belle2;
// // using boost::format;
// // using boost::lexical_cast;
// // using boost::math::sign;
// // using boost::bind;
// // // using boost::chrono::high_resolution_clock;
// //
// //
// // // random generator function:
// // ptrdiff_t rngWrapper(ptrdiff_t i) { return gRandom->Integer(i); }
// // // pointer object to it:
// // ptrdiff_t (*p_rngWrapper)(ptrdiff_t) = rngWrapper;
// //
// //
// //
// // //-----------------------------------------------------------------
// // //                 Register the Module
// // //-----------------------------------------------------------------
// // REG_MODULE(TFRedesign)
// //
// //
// // TFRedesignModule::TFRedesignModule() : Module()
// // {
// //   // initialization of many variables:
// //   InitializeInConstructor();
// //   resetCountersAtBeginRun();
// //
// //   /// setting standard values for steering parameters
// //   std::vector<std::string> sectorSetup = {"std"};
// //
// //   vector<int>  highestAllowedLayer = {6};
// //   vector<int>  minLayer = {4};
// //   vector<int>  minState = {2};
// //
// //   vector<double> reserveHitsThreshold = {0.6};
// //
// //   vector<bool> activateTRUE = { true };
// //   vector<bool> activateFALSE = { false };
// //   vector<bool> activateZigZagXY = { false, true, true }; // 1st, 2nd, 3rd-pass
// //   vector<double> tuneZERO = { 0 };
// //   vector<double> tuneCircleFit = { 00000001 }; // 1st, 2nd, 3rd-pass chi2-threshold
// //
// //   vector<string> rootFileNameVals = {"VXDTFoutput", "UPDATE"};
// //
// //   //Set module properties
// //   setDescription("Trackfinder for the SVD using cellular automaton techniques, kalman filter (genfit) and a hopfield network as well.");
// //   setPropertyFlags(c_ParallelProcessingCertified);
// //
// //   ///Steering parameter import
// //
// //   addParam("debugMode",
// //            m_PARAMDebugMode,
// //            "some code will only be executed if this mode is enabled (leading to more verbose output)",
// //            bool(false));
// //   addParam("sectorSetup",
// //            m_PARAMsectorSetup,
// //            "lets you chose the sectorSetup (compatibility of sensors, individual cutoffs,...) accepts 'std', 'low', 'high' and 'personal', please note that the chosen setup has to exist as a xml-file in ../tracking/data/friendList_XXX.xml. If you can not create your own xml files using e.g. the filterCalculatorModule, use params for  'tuneCutoffXXX' (for tuning single values) or 'tuneCutoffs' (for tuning all at once) instead. multipass supported by setting setups in a row",
// //            sectorSetup);
// //
// //   addParam("tuneCutoffs",
// //            m_PARAMtuneCutoffs,
// //            "for rapid changes of all cutoffs (no personal xml files needed), reduces/enlarges the range of the cutoffs in percent (lower and upper values are changed by this value). Only valid in range -99% < x < +1000%",
// //            double(0.22));
// //   addParam("GFTrackCandidatesColName",
// //            m_PARAMgfTrackCandsColName,
// //            "Name of collection holding the genfit::TrackCandidates (output)",
// //            string(""));
// //   addParam("InfoBoardName",
// //            m_PARAMinfoBoardName,
// //            "Name of container used for data transfer to TFAnalyzer, only used when TESTERexpandedTestingRoutines == true",
// //            string(""));
// //
// //   addParam("pxdClustersName",
// //            m_PARAMpxdClustersName,
// //            "Name of storeArray containing pxd clusters (only valid when using secMap supporting pxd clusters)",
// //            string(""));
// //   addParam("svdClustersName",
// //            m_PARAMsvdClustersName,
// //            "Name of storeArray containing svd clusters (only valid when using secMap supporting svd clusters)",
// //            string(""));
// //
// //   addParam("nameOfInstance",
// //            m_PARAMnameOfInstance,
// //            "Name of trackFinder, usefull, if there is more than one VXDTF running at the same time. Note: please choose short names",
// //            string("VXDTF"));
// //
// //
// //   addParam("activateDistance3D",
// //            m_PARAMactivateDistance3D,
// //            " set True/False for each setup individually",
// //            activateTRUE);
// //   addParam("activateDistanceXY",
// //            m_PARAMactivateDistanceXY,
// //            " set True/False for each setup individually",
// //            activateTRUE);
// //   addParam("activateDistanceZ",
// //            m_PARAMactivateDistanceZ,
// //            " set True/False for each setup individually",
// //            activateFALSE);
// //   addParam("activateSlopeRZ",
// //            m_PARAMactivateSlopeRZ,
// //            " set True/False for each setup individually",
// //            activateTRUE);
// //   addParam("activateNormedDistance3D",
// //            m_PARAMactivateNormedDistance3D,
// //            " set True/False for each setup individually",
// //            activateFALSE);
// //
// //   addParam("activateAngles3DHioC",
// //            m_PARAMactivateAngles3DHioC,
// //            " set True/False for each setup individually",
// //            activateTRUE);
// //   addParam("activateAnglesXYHioC",
// //            m_PARAMactivateAnglesXYHioC,
// //            " set True/False for each setup individually",
// //            activateTRUE);
// //   addParam("activateAnglesRZHioC",
// //            m_PARAMactivateAnglesRZHioC,
// //            " set True/False for each setup individually",
// //            activateFALSE);
// //   addParam("activateDeltaSlopeRZHioC",
// //            m_PARAMactivateDeltaSlopeRZHioC,
// //            " set True/False for each setup individually",
// //            activateFALSE);
// //   addParam("activateDistance2IPHioC",
// //            m_PARAMactivateDistance2IPHioC,
// //            " set True/False for each setup individually",
// //            activateFALSE);
// //   addParam("activatePTHioC",
// //            m_PARAMactivatePTHioC,
// //            " set True/False for each setup individually",
// //            activateFALSE);
// //   addParam("activateHelixParameterFitHioC",
// //            m_PARAMactivateHelixParameterFitHioC,
// //            " set True/False for each setup individually",
// //            activateFALSE);
// //
// //   addParam("tuneAngles3DHioC",
// //            m_PARAMtuneAngles3DHioC,
// //            " tune for each setup individually, in %",
// //            tuneZERO);
// //   addParam("tuneAnglesXYHioC",
// //            m_PARAMtuneAnglesXYHioC,
// //            " tune for each setup individually, in %",
// //            tuneZERO);
// //   addParam("tuneAnglesRZHioC",
// //            m_PARAMtuneAnglesRZHioC,
// //            " tune for each setup individually, in %",
// //            tuneZERO);
// //   addParam("tuneDeltaSlopeRZHioC",
// //            m_PARAMtuneDeltaSlopeRZHioC,
// //            " tune for each setup individually, in %",
// //            tuneZERO);
// //   addParam("tuneDistance2IPHioC",
// //            m_PARAMtuneDistance2IPHioC,
// //            " tune for each setup individually, in %",
// //            tuneZERO);
// //   addParam("tunePTHioC",
// //            m_PARAMtunePTHioC,
// //            " tune for each setup individually, in %",
// //            tuneZERO);
// //   addParam("tuneHelixParameterFitHioC",
// //            m_PARAMtuneHelixParameterFitHioC,
// //            " tune for each setup individually, in %",
// //            tuneZERO);
// //
// //   addParam("activateDeltaPtHioC",
// //            m_PARAMactivateDeltaPtHioC,
// //            " set True/False for each setup individually",
// //            activateTRUE);
// //   addParam("activateDeltaDistance2IPHioC",
// //            m_PARAMactivateDeltaDistance2IPHioC,
// //            " set True/False for each setup individually",
// //            activateTRUE);
// //   addParam("tuneDeltaPtHioC",
// //            m_PARAMtuneDeltaPtHioC,
// //            " tune for each setup individually, in %",
// //            tuneZERO);
// //   addParam("tuneDeltaDistance2IPHioC",
// //            m_PARAMtuneDeltaDistance2IPHioC,
// //            " tune for each setup individually, in %",
// //            tuneZERO);
// //
// //
// //   addParam("activateAngles3D",
// //            m_PARAMactivateAngles3D,
// //            " set True/False for each setup individually",
// //            activateTRUE);
// //   addParam("activateAnglesXY",
// //            m_PARAMactivateAnglesXY,
// //            " set True/False for each setup individually",
// //            activateFALSE);
// //   addParam("activateAnglesRZ",
// //            m_PARAMactivateAnglesRZ,
// //            " set True/False for each setup individually",
// //            activateFALSE);
// //   addParam("activateDeltaSlopeRZ",
// //            m_PARAMactivateDeltaSlopeRZ,
// //            " set True/False for each setup individually",
// //            activateTRUE);
// //   addParam("activateDistance2IP",
// //            m_PARAMactivateDistance2IP,
// //            " set True/False for each setup individually",
// //            activateFALSE);
// //   addParam("activatePT",
// //            m_PARAMactivatePT,
// //            " set True/False for each setup individually",
// //            activateFALSE);
// //   addParam("activateHelixParameterFit",
// //            m_PARAMactivateHelixParameterFit,
// //            " set True/False for each setup individually",
// //            activateFALSE);
// //   addParam("activateDeltaSOverZ",
// //            m_PARAMactivateDeltaSOverZ,
// //            " set True/False for each setup individually",
// //            activateFALSE);
// //   addParam("activateDeltaSlopeZOverS",
// //            m_PARAMactivateDeltaSlopeZOverS,
// //            " set True/False for each setup individually",
// //            activateFALSE);
// //
// //   addParam("activateZigZagXY",
// //            m_PARAMactivateZigZagXY,
// //            " set True/False for each setup individually",
// //            activateZigZagXY);
// //   addParam("activateZigZagXYWithSigma",
// //            m_PARAMactivateZigZagXYWithSigma,
// //            " set True/False for each setup individually",
// //            activateFALSE);
// //   addParam("activateZigZagRZ",
// //            m_PARAMactivateZigZagRZ,
// //            " set True/False for each setup individually",
// //            activateFALSE);
// //   addParam("activateDeltaPt",
// //            m_PARAMactivateDeltaPt,
// //            " set True/False for each setup individually",
// //            activateFALSE);
// //   addParam("activateDeltaDistance2IP",
// //            m_PARAMactivateDeltaDistance2IP,
// //            " set True/False for each setup individually",
// //            activateFALSE);
// //   addParam("activateCircleFit",
// //            m_PARAMactivateCircleFit,
// //            " set True/False for each setup individually",
// //            activateFALSE);
// //
// //
// //   addParam("tuneDistance3D",
// //            m_PARAMtuneDistance3D,
// //            " tune for each setup individually in %",
// //            tuneZERO);
// //   addParam("tuneDistanceXY",
// //            m_PARAMtuneDistanceXY,
// //            " tune for each setup individually, in %",
// //            tuneZERO);
// //   addParam("tuneDistanceZ",
// //            m_PARAMtuneDistanceZ,
// //            " tune for each setup individually, in %",
// //            tuneZERO);
// //   addParam("tuneSlopeRZ",
// //            m_PARAMtuneSlopeRZ,
// //            " tune for each setup individually, in %",
// //            tuneZERO);
// //   addParam("tuneNormedDistance3D",
// //            m_PARAMtuneNormedDistance3D,
// //            " tune for each setup individually, in %",
// //            tuneZERO);
// //
// //   addParam("tuneAngles3D",
// //            m_PARAMtuneAngles3D,
// //            " tune for each setup individually, in %",
// //            tuneZERO);
// //   addParam("tuneAnglesXY",
// //            m_PARAMtuneAnglesXY,
// //            " tune for each setup individually, in %",
// //            tuneZERO);
// //   addParam("tuneAnglesRZ",
// //            m_PARAMtuneAnglesRZ,
// //            " tune for each setup individually, in %",
// //            tuneZERO);
// //   addParam("tuneDeltaSlopeRZ",
// //            m_PARAMtuneDeltaSlopeRZ,
// //            " tune for each setup individually, in %",
// //            tuneZERO);
// //   addParam("tuneDistance2IP",
// //            m_PARAMtuneDistance2IP,
// //            " tune for each setup individually, in %",
// //            tuneZERO);
// //   addParam("tunePT",
// //            m_PARAMtunePT,
// //            " tune for each setup individually, in %",
// //            tuneZERO);
// //   addParam("tuneHelixParameterFit",
// //            m_PARAMtuneHelixParameterFit,
// //            " tune for each setup individually, in %",
// //            tuneZERO);
// //   addParam("tuneDeltaSOverZ",
// //            m_PARAMtuneDeltaSOverZ,
// //            " tune for each setup individually, in %",
// //            tuneZERO);
// //   addParam("tuneDeltaSlopeZOverS",
// //            m_PARAMtuneDeltaSlopeZOverS,
// //            " tune for each setup individually, in %",
// //            tuneZERO);
// //
// //   addParam("tuneZigZagXY",
// //            m_PARAMtuneZigZagXY,
// //            " currently not in use, only here for symmetrical reasons",
// //            tuneZERO);
// //   addParam("tuneZigZagXYWithSigma",
// //            m_PARAMtuneZigZagXYWithSigma,
// //            " currently not in use, only here for symmetrical reasons",
// //            tuneZERO);
// //
// //   addParam("tuneZigZagRZ",
// //            m_PARAMtuneZigZagRZ,
// //            " currently not in use, only here for symmetrical reasons",
// //            tuneZERO);
// //   addParam("tuneDeltaPt",
// //            m_PARAMtuneDeltaPt,
// //            " tune for each setup individually, in %",
// //            tuneZERO);
// //   addParam("tuneDeltaDistance2IP",
// //            m_PARAMtuneDeltaDistance2IP,
// //            " tune for each setup individually, in %",
// //            tuneZERO);
// //   addParam("tuneCircleFit",
// //            m_PARAMtuneCircleFit,
// //            " threshold for pValues calculated by the circleFiter for each tc. If pValue is lower than threshold, tc gets discarded",
// //            tuneCircleFit);
// //
// //
// //   addParam("highOccupancyThreshold",
// //            m_PARAMhighOccupancyThreshold,
// //            "if there are more hit-combinations at a sensor than chosen threshhold, a special high-occupancy-mode will be used to filter more hits",
// //            int(400));
// //   addParam("killBecauseOfOverlappsThreshold",
// //            m_PARAMkillBecauseOfOverlappsThreshold,
// //            "if there are more TCs overlapping than chosen threshold value, event kalman gets replaced by circleFit. If there are 10 times more than threshold value of TCs, the complete event gets aborted",
// //            int(500));
// //   addParam("killEventForHighOccupancyThreshold",
// //            m_PARAMkillEventForHighOccupancyThreshold,
// //            "if there are more segments than threshold value, the complete event gets aborted",
// //            (unsigned int)(5500));
// //
// //   addParam("tccMinLayer",
// //            m_PARAMminLayer,
// //            "determines lowest layer considered by track candidate collector",
// //            minLayer);
// //   addParam("tccMinState",
// //            m_PARAMminState,
// //            "determines lowest state of cells considered by track candidate collector",
// //            minState);
// //   addParam("omega",
// //            m_PARAMomega,
// //            "tuning parameter for the hopfield network",
// //            double(0.5));
// //   addParam("reserveHitsThreshold",
// //            m_PARAMreserveHitsThreshold,
// //            "tuning parameter for multi-pass-setup, valid values are 0-1 ( = 0-100%). It defines how many percent of the TCs (sorted by QI) are allowed to reserve their hits (which disallows further passes to use these hits). This does not mean that TCs which were not allowed to reserve their hits will be deleted, this only means that they have to compete with TCs of other passes for their hits again. Setting the values to 100% = 1 means, no hits used by tcs surviving that pass are reused, 0% = 0 means every tc has to compete with all tcs of other passes (quite similar to former behavior), standard is 50% = 0.5",
// //            reserveHitsThreshold);
// //
// //   //for testing purposes:
// //   addParam("activateAlwaysTrue2Hit",
// //            m_PARAMactivateAlwaysTrue2Hit,
// //            " set True/False for each setup individually",
// //            activateFALSE);
// //   addParam("activateAlwaysFalse2Hit",
// //            m_PARAMactivateAlwaysFalse2Hit,
// //            " set True/False for each setup individually",
// //            activateFALSE);
// //   addParam("activateRandom2Hit",
// //            m_PARAMactivateRandom2Hit,
// //            " set True/False for each setup individually",
// //            activateFALSE);
// //   addParam("activateAlwaysTrue3Hit",
// //            m_PARAMactivateAlwaysTrue3Hit,
// //            " set True/False for each setup individually",
// //            activateFALSE);
// //   addParam("activateAlwaysFalse3Hit",
// //            m_PARAMactivateAlwaysFalse3Hit,
// //            " set True/False for each setup individually",
// //            activateFALSE);
// //   addParam("activateRandom3Hit",
// //            m_PARAMactivateRandom3Hit,
// //            " set True/False for each setup individually",
// //            activateFALSE);
// //   addParam("activateAlwaysTrue4Hit",
// //            m_PARAMactivateAlwaysTrue4Hit,
// //            " set True/False for each setup individually",
// //            activateFALSE);
// //   addParam("activateAlwaysFalse4Hit",
// //            m_PARAMactivateAlwaysFalse4Hit,
// //            " set True/False for each setup individually",
// //            activateFALSE);
// //   addParam("activateRandom4Hit",
// //            m_PARAMactivateRandom4Hit,
// //            " set True/False for each setup individually",
// //            activateFALSE);
// //   addParam("tuneAlwaysTrue2Hit",
// //            m_PARAMtuneAlwaysTrue2Hit,
// //            " set True/False for each setup individually",
// //            tuneZERO);
// //   addParam("tuneAlwaysFalse2Hit",
// //            m_PARAMtuneAlwaysFalse2Hit,
// //            " set True/False for each setup individually",
// //            tuneZERO);
// //   addParam("tuneRandom2Hit",
// //            m_PARAMtuneRandom2Hit,
// //            " set True/False for each setup individually",
// //            tuneZERO);
// //   addParam("tuneAlwaysTrue3Hit",
// //            m_PARAMtuneAlwaysTrue3Hit,
// //            " set True/False for each setup individually",
// //            tuneZERO);
// //   addParam("tuneAlwaysFalse3Hit",
// //            m_PARAMtuneAlwaysFalse3Hit,
// //            " set True/False for each setup individually",
// //            tuneZERO);
// //   addParam("tuneRandom3Hit",
// //            m_PARAMtuneRandom3Hit,
// //            " set True/False for each setup individually",
// //            tuneZERO);
// //   addParam("tuneAlwaysTrue4Hit",
// //            m_PARAMtuneAlwaysTrue4Hit,
// //            " set True/False for each setup individually",
// //            tuneZERO);
// //   addParam("tuneAlwaysFalse4Hit",
// //            m_PARAMtuneAlwaysFalse4Hit,
// //            " set True/False for each setup individually",
// //            tuneZERO);
// //   addParam("tuneRandom4Hit",
// //            m_PARAMtuneRandom4Hit,
// //            " set True/False for each setup individually",
// //            tuneZERO);
// //
// //   addParam("highestAllowedLayer",
// //            m_PARAMhighestAllowedLayer,
// //            "set value below 6 if you want to exclude outer layers (standard is 6)",
// //            highestAllowedLayer);
// //   addParam("standardPdgCode",
// //            m_PARAMpdGCode,
// //            "standard value is 211 (pi+), ATTENTION, instead of using inconsistent sign of PdGList, in this module positively charged particles are always positive and negatively charged ones are negative (relevant for leptons)",
// //            int(211));
// //   addParam("artificialMomentum",
// //            m_PARAMartificialMomentum,
// //            "standard value is 0. if StandardValue is changed to a nonZero value, the magnitude of the momentum seed is set artificially using this value, if value < 0, not only the magnitude is set using the norm of the value, but direction of momentum is reversed too, if you want to change charge, use parameter 'standardPdgCode'",
// //            double(0));
// //
// //
// //   addParam("cleanOverlappingSet",
// //            m_PARAMcleanOverlappingSet,
// //            "when true, TCs which are found more than once (possible because of multipass) will get filtered",
// //            bool(false));
// //
// //   addParam("qiSmear",
// //            m_PARAMqiSmear,
// //            " set True if you want to smear QI's of TCs (needed when no trackLength is chosen for QI calculation) ",
// //            bool(false));
// //   addParam("smearMean",
// //            m_PARAMsmearMean,
// //            " when qiSmear = True, bias of perturbation can be set here",
// //            double(0.0));
// //   addParam("smearSigma",
// //            m_PARAMsmearSigma,
// //            " when qiSmear = True, degree of perturbation can be set here",
// //            double(0.000001));
// //
// //   addParam("calcQIType",
// //            m_PARAMcalcQIType,
// //            "allows you to chose the way, the QI's of the TC's shall be calculated. currently supported: 'kalman','trackLength', 'circleFit', 'straightLine'",
// //            string("circleFit"));
// //   addParam("calcSeedType",
// //            m_PARAMcalcSeedType,
// //            "allows you to chose the way, the seed-mometa of the TC's shall be calculated. currently supported: 'helixFit', 'straightLine'",
// //            string("helixFit"));
// //   addParam("filterOverlappingTCs",
// //            m_PARAMfilterOverlappingTCs,
// //            "allows you to chose the which technique shall be used for filtering overlapping TCs, currently supported: 'hopfield', 'greedy', 'none'",
// //            string("hopfield"));
// //   addParam("storeBrokenQI",
// //            m_PARAMstoreBrokenQI,
// //            "if true, TC survives QI-calculation-process even if fit was not possible",
// //            bool(true));
// //   addParam("KFBackwardFilter",
// //            m_KFBackwardFilter,
// //            "determines whether the kalman filter moves inwards or backwards, 'True' means inwards",
// //            bool(false));
// //   addParam("TESTERexpandedTestingRoutines",
// //            m_TESTERexpandedTestingRoutines,
// //            "set true if you want to export expanded infos of TCs for further analysis (setting to false means that the DataObject called 'VXDTFInfoBoard' will not be stored)",
// //            bool(true));
// //   addParam("writeToRoot",
// //            m_PARAMwriteToRoot,
// //            "set true if you want to export the p-values of the fitters in a root file named by parameter 'rootFileName'",
// //            bool(false));
// //   addParam("rootFileName",
// //            m_PARAMrootFileName,
// //            "fileName used for p-value export. Will be ignored if parameter 'writeToRoot' is false (standard)",
// //            rootFileNameVals);
// //   addParam("displayCollector",
// //            m_PARAMdisplayCollector,
// //            "Collector operating flag: 0 = no collector, 1 = collect for analysis, 2 = collect for display",
// //            int(0));
// //
// //   addParam("useTimeSeedAsQI", m_PARAMuseTimeSeedAsQI,
// //            "JKL - WARNING evil hack: uses an ugly workaround to be able to use realistically determined quality indicators (TFRedesignModule fills the TimeSeed with its determined QI) ",
// //            false);
// //
// //
// //   if (m_PARAMwriteToRoot == true and Environment::Instance().getNumberProcesses() > 0) {
// //     B2WARNING("VXDTF::initialize: writeToRoot enabled and basf2 is running in multi-threaded mode - this can cause nondeterministic behavior if VXDTF was not manually set to vxdtf.set_property_flags(0) in the steering file!")
// //   }
// // }
// //
// //
// // TFRedesignModule::~TFRedesignModule()
// // {
// //
// // }
// //
// // void TFRedesignModule::initialize()
// // {
// //   B2DEBUG(1, "-----------------------------------------------\n       entering VXD CA track finder (" << m_PARAMnameOfInstance <<
// //           ") - initialize:");
// //
// //   checkAndSetupModuleParameters();
// //
// //   prepareExternalTools();
// //
// //   B2DEBUG(1, "       leaving VXD CA track finder (" << m_PARAMnameOfInstance <<
// //           ") - initialize\n-----------------------------------------------");
// // }
// //
// // /** *************************************+************************************* **/
// // /** ***********************************+ + +*********************************** **/
// // /** *******************************+  beginRun +******************************* **/
// // /** ***********************************+ + +*********************************** **/
// // /** *************************************+************************************* **/
// //
// // void TFRedesignModule::beginRun()
// // {
// //   B2INFO("-----------------------------------------------\n       entering VXD CA track finder (" << m_PARAMnameOfInstance <<
// //          ") - beginRun.\n       if you want to have some basic infos during begin- and endrun about it, set debug level 1 or 2. Debug level 3 or more gives you event wise output (the higher the level, the more verbose it gets, highest level: 175)");
// //   B2DEBUG(50,
// //           "##### be careful, current TF status does not support more than one run per initialization! #####"); /// WARNING TODO: check whether this is still valid
// //
// //   setupPasses();
// //
// //   if (m_PARAMdisplayCollector > 0) { importSectorMapsToDisplayCollector(); }
// //
// //   resetCountersAtBeginRun();
// //
// //   B2INFO(m_PARAMnameOfInstance <<
// //          "leaving VXD CA track finder (TFRedesignModule) - beginRun...\n       -----------------------------------------------");
// // }
// //
// // /** *************************************+************************************* **/
// // /** ***********************************+ + +*********************************** **/
// // /** *********************************+ event +********************************* **/
// // /** ***********************************+ + +*********************************** **/
// // /** *************************************+************************************* **/
// //
// // void TFRedesignModule::the_real_event()
// // {
// //
// //   bool abortEvent = initializeEvent();
// //   if (abortEvent == true) return;
// //
// //   /** Section 3 - importing hits and find their papaSectors.**/
// //   addVirtualInteractionPoint();
// //
// //   createClusterInfo();
// //
// //   /// means: is true when at least one pass wants PXD hits:
// //   if (m_usePXDHits == true) assignPXDHitsToSectors();
// //
// //   /// means: is true when at least one pass wants SVD hits:
// //   if (m_useSVDHits == true) assignSVDHitsToSectors();
// //
// //   if (LogSystem::Instance().isLevelEnabled(LogConfig::c_Debug, 3, PACKAGENAME()) == true) {
// //     B2DEBUG(3, "VXDTF- import hits: of " << m_evInfoPack.nSVDClusters << " svdClusters, " << m_evInfoPack.numSVDHits <<
// //             " svd2Dclusters (all passes), " << m_evInfoPack.nPXDClusters  << " pxdClusters, " << m_badSectorRangeCtr <<
// //             " hits had to be discarded because out of sector range")
// //   }
// //   /** Section 3 - end **/
// //
// //
// //   int passNumber = 0;
// //   /// /// /// WARNING WARNING WARNING --- pass loop begin --- WARNING WARNING WARNING
// //   for (PassData* currentPass : m_passSetupVector) {
// //
// //     preparePass(currentPass, passNumber);
// //
// //     /** Section 4 - SEGFINDER **/
// //     abortEvent = doTheSegFinder(currentPass, passNumber);
// //     if (abortEvent == true) return;
// //     /** Section 4 - end **/
// //
// //
// //     /** Section 5 - NEIGHBOURFINDER **/
// //     abortEvent = doTheNeighbourFinder(currentPass, passNumber);
// //     if (abortEvent == true) return;
// //     /** Section 5 - end **/
// //
// //
// //     /** Section 6 - Cellular Automaton**/
// //     abortEvent = doTheCellularAutomaton(currentPass, passNumber);
// //     if (abortEvent == true) return;
// //     /** Section 6 - end **/
// //
// //
// //     /** Section 7 - Track Candidate Collector (TCC) **/
// //     abortEvent = doTheTrackCandidateCollector(currentPass, passNumber);
// //     if (abortEvent == true) return;
// //     /** Section 7 - end **/
// //
// //
// //     /** Section 8 - tcFilter **/
// //     doTheTcFilter(currentPass, passNumber);
// //     /** Section 8 - end **/
// //
// //
// //     /** Section 9 - check overlap */
// //     unsigned int totalOverlaps = doTheOverlapCheck(currentPass);
// //     /** Section 9 - end */
// //
// //
// //     /** Section 10 - calc QI for each TC */
// //     abortEvent = doTheQICalculation(totalOverlaps);
// //     if (abortEvent == true) return;
// //     /** Section 10 - end */
// //
// //
// //     /** Section 11 - cleanOverlappingSet */
// //     if (totalOverlaps > 2 && m_PARAMcleanOverlappingSet == true) { totalOverlaps = doTheOverlapCleaning(totalOverlaps); }
// //     /** Section 11 - end */
// //
// //
// //     /** redo overlapping check - necessary since condition changed through cleanOverlappingSet! */
// //     /** Section 9b - check overlap 2 */
// //     totalOverlaps = redoTheOverlapCheck(totalOverlaps);
// //     /** Section 9b - end */
// //
// //
// //     /** Section 12 - Hopfield */
// //     doTheFilterOverlappingTCs(currentPass, passNumber, totalOverlaps);
// //     /** Section 12 - end */
// //
// //     passNumber++;
// //   } /// /// /// WARNING WARNING WARNING --- pass loop end --- WARNING WARNING WARNING
// //
// //
// //   int countOverbookedClusters = 0;
// //   /// each clusterInfo knows which TCs are using it, following loop therefore checks each for overlapping ones
// //   for (ClusterInfo& aCluster : m_clustersOfEvent) {
// //     bool isOB = aCluster.isOverbooked();
// //     if (isOB == true) { countOverbookedClusters++; }
// //   } // now each TC knows whether it is overbooked or not (aCluster.isOverbooked() implicitly checked this)
// //   if (countOverbookedClusters != 0 and m_filterOverlappingTCs != 0) {
// //     B2ERROR("after pass loop (and Hopfield/greedy there): there are " << countOverbookedClusters << " clusters of " <<
// //             m_clustersOfEvent.size() << " marked as 'overbooked'...")
// //   }
// //
// //
// //   /** Section 13 - generateGFTrackCand */
// //   doTheGFTrackCandGeneration();
// //   /** Section 13 - end */
// //
// //
// //   /** Section 14 - final stuff of event including cleanup */
// //   finalEventCleanup();
// //   /** Section 14 - end */
// // }
// //
// //
// //
// // void TFRedesignModule::event()
// // {
// // #ifdef HAS_CALLGRIND
// //   CALLGRIND_START_INSTRUMENTATION;
// // #endif
// //   this->the_real_event();
// // #ifdef HAS_CALLGRIND
// //   CALLGRIND_STOP_INSTRUMENTATION;
// // #endif
// // }
// //
// //
// // /** *************************************+************************************* **/
// // /** ***********************************+ + +*********************************** **/
// // /** ********************************+  endRun +******************************** **/
// // /** ***********************************+ + +*********************************** **/
// // /** *************************************+************************************* **/
// //
// //
// // void TFRedesignModule::endRun()
// // {
// //   /** REDESIGNCOMMENT ENDRUN 1:
// //    * * short:
// //    * endRun
// //    *
// //    ** long (+personal comments):
// //    * debugging and DQM related stuff is happening here
// //    *
// //    ** dependency of module parameters (global):
// //    * m_PARAMnameOfInstance, m_PARAMcalcSeedType, m_PARAMtuneCutoffs,
// //    * m_PARAMcalcQIType, m_PARAMfilterOverlappingTCs, m_PARAMnameOfInstance,
// //    * m_PARAMactivateBaselineTF,
// //    *
// //    ** dependency of global in-module variables:
// //    * m_TESTERbadSensors, m_TESTERlogEvents, m_eventCounter,
// //    * m_passSetupVector, m_totalPXDClusters, m_totalSVDClusters,
// //    * m_totalTELClusters, m_totalSVDClusterCombis, m_badSectorRangeCounter,
// //    * m_badFriendCounter, m_TESTERdistortedHitCtr, m_TESTERbadSectorRangeCounterForClusters,
// //    * m_TESTERclustersPersSectorNotMatching, m_TESTERovercrowdedStrangeSensors, m_TESTERSVDOccupancy,
// //    * m_TESTERtimeConsumption, m_TESTERlogEvents, m_baselinePass,
// //    * m_TESTERsucceededBaselineTF, m_TESTERacceptedBrokenHitsTrack, m_TESTERrejectedBrokenHitsTrack,
// //    * m_TESTERnoHitsAtEvent, m_TESTERtotalsegmentsSFCtr, m_TESTERdiscardedSegmentsSFCtr,
// //    * m_TESTERtotalsegmentsNFCtr, m_TESTERdiscardedSegmentsNFCtr, m_TESTERtriggeredZigZagXY,
// //    * m_TESTERtriggeredZigZagXYWithSigma, m_TESTERtriggeredZigZagRZ, m_TESTERtriggeredCircleFit,
// //    * m_TESTERtriggeredDpT, m_TESTERapprovedByTCC, m_TESTERcountTotalTCsAfterTCC,
// //    * m_TESTERcountTotalTCsAfterTCCFilter, m_TESTERcountTotalTCsFinal, m_TESTERbadHopfieldCtr,
// //    * m_TESTERHopfieldLetsOverbookedTCsAliveCtr, m_TESTERfilteredBadSeedTCs, m_TESTERcleanOverlappingSetStartedCtr,
// //    * m_TESTERfilteredOverlapsQI, m_TESTERfilteredOverlapsQICtr, m_TESTERNotFilteredOverlapsQI,
// //    * m_TESTERgoodFitsCtr, m_TESTERbadFitsCtr, m_TESTERbrokenEventsCtr,
// //    * m_TESTERkalmanSkipped, m_TESTERbrokenCaRound, m_TESTERhighOccupancyCtr,
// //    * m_TESTERcountTotalUsedIndicesFinal, m_TESTERcountTotalUsedHitsFinal
// //    *
// //    ** dependency of global stuff just because of B2XX-output or debugging only:
// //    * m_TESTERbadSensors, m_TESTERlogEvents, m_PARAMnameOfInstance,
// //    * m_eventCounter, m_passSetupVector, m_PARAMcalcSeedType,
// //    * m_PARAMtuneCutoffs, m_PARAMcalcQIType, m_PARAMfilterOverlappingTCs,
// //    * m_totalPXDClusters, m_totalSVDClusters, m_totalTELClusters,
// //    * m_totalSVDClusterCombis, m_badSectorRangeCounter, m_badFriendCounter,
// //    * m_TESTERdistortedHitCtr, m_TESTERbadSectorRangeCounterForClusters, m_TESTERclustersPersSectorNotMatching,
// //    * m_TESTERovercrowdedStrangeSensors, m_TESTERSVDOccupancy, m_TESTERtimeConsumption,
// //    * m_TESTERlogEvents, m_PARAMnameOfInstance, m_PARAMactivateBaselineTF,
// //    * m_baselinePass, m_TESTERsucceededBaselineTF, m_TESTERacceptedBrokenHitsTrack,
// //    * m_TESTERrejectedBrokenHitsTrack, m_TESTERnoHitsAtEvent, m_TESTERtotalsegmentsSFCtr,
// //    * m_TESTERdiscardedSegmentsSFCtr, m_TESTERtotalsegmentsNFCtr, m_TESTERdiscardedSegmentsNFCtr,
// //    * m_TESTERtriggeredZigZagXY, m_TESTERtriggeredZigZagXYWithSigma, m_TESTERtriggeredZigZagRZ,
// //    * m_TESTERtriggeredCircleFit, m_TESTERtriggeredDpT, m_TESTERapprovedByTCC,
// //    * m_TESTERcountTotalTCsAfterTCC, m_TESTERcountTotalTCsAfterTCCFilter, m_TESTERcountTotalTCsFinal,
// //    * m_TESTERbadHopfieldCtr, m_TESTERHopfieldLetsOverbookedTCsAliveCtr, m_TESTERfilteredBadSeedTCs,
// //    * m_TESTERcleanOverlappingSetStartedCtr, m_TESTERfilteredOverlapsQI, m_TESTERfilteredOverlapsQICtr,
// //    * m_TESTERNotFilteredOverlapsQI, m_TESTERgoodFitsCtr, m_TESTERbadFitsCtr,
// //    * m_TESTERbrokenEventsCtr, m_TESTERkalmanSkipped, m_TESTERbrokenCaRound,
// //    * m_TESTERhighOccupancyCtr, m_TESTERcountTotalUsedIndicesFinal, m_TESTERcountTotalUsedHitsFinal
// //    *
// //    ** in-module-function-calls:
// //    */
// //
// //
// //   vector<string>::iterator newEndOfVector;
// //   std::vector<EventInfoPackage>::iterator newEndOfAnothterVector;
// //
// //   /// WARNING: std:unique does delete double entries but does NOT resize the vector! This means that for every removed element, at the end of the vector remains one random value stored
// //   std::sort(m_TESTERbadSensors.begin(), m_TESTERbadSensors.end());
// //   newEndOfVector = std::unique(m_TESTERbadSensors.begin(), m_TESTERbadSensors.end());
// //   m_TESTERbadSensors.resize(std::distance(m_TESTERbadSensors.begin(), newEndOfVector));
// //   std::sort(m_TESTERlogEvents.begin(), m_TESTERlogEvents.end());
// //   newEndOfAnothterVector = std::unique(m_TESTERlogEvents.begin(), m_TESTERlogEvents.end());
// //   m_TESTERlogEvents.resize(std::distance(m_TESTERlogEvents.begin(), newEndOfAnothterVector));
// //   std::reverse(m_TESTERlogEvents.begin(), m_TESTERlogEvents.end());
// //
// //   B2DEBUG(1, " ############### " << m_PARAMnameOfInstance << " endRun of " <<  m_eventCounter + 1 <<
// //           " events ###############\nfor explanations of the values, activate DEBUG-mode (level 2 or greater)!")
// //
// //   if (m_eventCounter == 0) { m_eventCounter = 1; }
// //   float invNEvents = 1. / float(m_eventCounter);
// //   string lineHigh = "------------------------------------------------------------------------------------------";
// //   string lineApnd = "--------------------------";
// //   B2DEBUG(1, lineHigh << lineApnd << lineApnd)
// //   B2DEBUG(1, m_PARAMnameOfInstance << " settings: number of passes: " << m_passSetupVector.size() << ", tuneCutoffs: " <<
// //           m_PARAMtuneCutoffs << ", QIfilterMode: " << m_PARAMcalcQIType << ", way to calc Seed: " << m_PARAMcalcSeedType <<
// //           ", filterOverlappingTCs: " << m_PARAMfilterOverlappingTCs << ", chosen settings: ")
// //   stringstream infoStuff2, secInfo;
// //
// //   B2DEBUG(1, lineHigh << lineApnd << lineApnd)
// //   B2DEBUG(1, "detector\t| setup\t\t\t| maxLayer\t| minLayer\t| minState\t| sfTests\t| nfTests\t| TcFilterTests\t|")
// //   for (PassData* currentPass : m_passSetupVector) {
// //     for (double limit : currentPass->secConfigU) { secInfo << limit << " "; }
// //     secInfo << "\t\t|";
// //     for (double limit : currentPass->secConfigV) { secInfo << limit << " "; }
// //     secInfo << "\t\t|\n";
// //     B2DEBUG(1, currentPass->chosenDetectorType << "\t| " << currentPass->sectorSetup << "\t| " << currentPass->highestAllowedLayer <<
// //             "\t\t| " << currentPass->minLayer << "\t\t| " << currentPass->minState << "\t\t| " << currentPass->activatedSegFinderTests <<
// //             "\t\t| " << currentPass->activatedNbFinderTests << "\t\t| " << currentPass->activatedTccFilterTests << "\t|")
// //   }
// //   B2DEBUG(2,
// //           "Explanation: setup defines region (pT) to be searched(details in additionalInfo of XML-file)\n maxLayer is highest allowed layer, minLayer = lowest...\nminState lowest state of CA-cells used for seeding a TC\nXTests are number of activated tests, X=sf->segfinder, nf->neighbourFinder, TcFilter = TcFilter")
// //   B2DEBUG(1, lineHigh << lineApnd << lineApnd)
// //   B2DEBUG(1, "\nsecUConfig \t\t| secVConfig \t\t|\n" << secInfo.str())
// //   B2DEBUG(2,
// //           "Explanation: secConfigs are cuts for sector-calculation (cut*sensorWidth for U, cut*sensorLength for V), same value for each sensor")
// //
// //   for (string& badSensor : m_TESTERbadSensors) { infoStuff2 << badSensor << " "; }
// //   B2DEBUG(1, "In following sensors there were hits out of the sensitive plane:\n" << infoStuff2.str());
// //
// //   B2DEBUG(1, lineHigh  << "\n pxdc\t| svdc\t| svdH\t| catC\t| ghR\t\t| noSc\t| noFd\t| ooR \t| no2D\t| 1Dsn\t| 1DHO \t|\n " <<
// //           m_totalPXDClusters << "\t| " << m_totalSVDClusters << "\t| " << int(m_totalSVDClusters * 0.5) << "\t| " << m_totalSVDClusterCombis
// //           << "\t| " << double(m_totalSVDClusterCombis) / (double(m_totalSVDClusters) * 0.5) << "\t| " << m_badSectorRangeCounter << "\t| " <<
// //           m_badFriendCounter << "\t| " << m_TESTERdistortedHitCtr << "\t| " << m_TESTERbadSectorRangeCounterForClusters << "\t| " <<
// //           m_TESTERclustersPersSectorNotMatching << "\t| " << m_TESTERovercrowdedStrangeSensors << "\t|")
// //
// //   B2DEBUG(2,
// //           "Explanation: pxdc: number of PXDClusters, 2D), svdc: SVDClusters(1D)\n svdH: # of SVDHits(guess of actual number of 2D-Track- and -BG-hits)\n catC: SVDClusterCombinations(combining u/v, including ghosthits)\n ghR: ghostHitRate(is only a guess, TODO: should be correctly determined!!)\n noSc: are hits discarded, where no fitting sector could be found\n noFd: hits having no hits in compatible sectors (friends), ooR: sensors which were not in sensitive plane of sensor, no2D: SVDclusters, were no 2nd cluster could be found\n 1Dsn: times where a SVDsensor hat not the same number of u and v clusters, 1DHO: like 1Dsn, but where a high occupancy case prevented reconstructing them")
// //
// //   B2DEBUG(1, lineHigh << lineApnd << lineApnd <<
// //           "\n NoH\t| sf+\t| sf-\t| nf+\t| nf-\t| zzXY\t| zXYS\t| zzRZ\t| cf\t| dpT\t| tcc+\t| Ttcc\t| Ttcf\t| Tfin\t| NNns\t| NNov\t|\n " <<
// //           m_TESTERnoHitsAtEvent << "\t| " << m_TESTERtotalsegmentsSFCtr << "\t| " << m_TESTERdiscardedSegmentsSFCtr << "\t| " <<
// //           m_TESTERtotalsegmentsNFCtr << "\t| " << m_TESTERdiscardedSegmentsNFCtr << "\t|" << m_TESTERtriggeredZigZagXY << "\t| " <<
// //           m_TESTERtriggeredZigZagXYWithSigma << "\t| " << m_TESTERtriggeredZigZagRZ << "\t| " << m_TESTERtriggeredCircleFit << "\t| " <<
// //           m_TESTERtriggeredDpT << "\t| " << m_TESTERapprovedByTCC << "\t| " << m_TESTERcountTotalTCsAfterTCC << "\t| " <<
// //           m_TESTERcountTotalTCsAfterTCCFilter << "\t| " << m_TESTERcountTotalTCsFinal << "\t| " << m_TESTERbadHopfieldCtr << "\t| " <<
// //           m_TESTERHopfieldLetsOverbookedTCsAliveCtr << "\t|")
// //
// //   B2DEBUG(2,
// //           "Explanation: NoH: Events without hits\n sf+: segfinder segments activated, sf-: -discarded, nf+: nbFinder segments activated, nf-: discarded, zzXY: zigzagXY got triggered, zzRZ: same for zigzagRZ, cf: circleFit(tuneCircleFitValue), dpT: deltaPt\n tcc+: approved by tcc, Ttcc: total number of TCs after TCC, Ttcf: after TCC-filter, Tfin: total number of final TCs, NNns: the Hopfield network had no survivors, NNov: the Hopfield network accepted overlapping TCs (which should never occur! -> if != 0: Bug!)")
// //
// //   B2DEBUG(1, lineHigh << lineApnd << lineApnd <<
// //           "\n civ4\t| clo+\t| cloK\t| cloF\t| cloS\t| kf+\t| kf-\t| TFHo\t| kfHo\t| CA-\t| Ho+\t|\n " << m_TESTERfilteredBadSeedTCs << "\t| "
// //           << m_TESTERcleanOverlappingSetStartedCtr << "\t| " << m_TESTERfilteredOverlapsQI << "\t| " << m_TESTERfilteredOverlapsQICtr <<
// //           "\t| " << m_TESTERNotFilteredOverlapsQI << "\t| " << m_TESTERgoodFitsCtr << "\t| " << m_TESTERbadFitsCtr << "\t| " <<
// //           m_TESTERbrokenEventsCtr << "\t| " << m_TESTERkalmanSkipped << "\t| " << m_TESTERbrokenCaRound << "\t| " << m_TESTERhighOccupancyCtr
// //           << "\t|")
// //
// //   B2DEBUG(2,
// //           "Explanation: civ4: nTimes calcInitialValues4TCs filtered TCs, clo+: cleanOverlappingSet got activated, cloK: cleanOverlappingSet killed at least 1 TC, cloF: # of TCs filtered/killed by cleanOverlappingSet, cloS: # of TCs survived cleanOverlappingSet, kf+: nTimes, where a kalman fit was possible, kf-: where it failed, TFHo: where the TF had to be terminated (event lost!), kfHo: kalmanSkipped because of high occupancy (alternative QICalc like CF activated), CA-: # of times, where CA had to be aborted, Ho+: when highOccupancyMode was activated")
// //
// //   B2DEBUG(1, lineHigh << lineApnd << lineApnd)
// //
// //   string printOccupancy;
// //   int h1 = 0, h2t4 = 0, h5t9 = 0, h10t16 = 0, h17t25 = 0, h26t50 = 0, h51t100 = 0, h101t200 = 0, h201t300 = 0, h301t400 = 0,
// //       h401t500 = 0, h501t600 = 0, h601t700 = 0, h701t800 = 0, h800plus = 0;
// //   int nTotalHits = 0;
// //   for (int nHitsMinus1 = 0; nHitsMinus1 < int(m_TESTERSVDOccupancy.size()); ++ nHitsMinus1) {
// //     int thisValue = m_TESTERSVDOccupancy.at(nHitsMinus1);
// //     if (thisValue == 0) { continue; }
// //     nTotalHits += thisValue;
// //     printOccupancy = printOccupancy + " got " + boost::lexical_cast<string>(thisValue) + " times a sensor with " +
// //                      boost::lexical_cast<string>(nHitsMinus1 + 1) + " hits\n";
// //
// //     if (h1 < 1) { h1 += thisValue; }
// //     else if (nHitsMinus1 < 4) { h2t4 += thisValue; }
// //     else if (nHitsMinus1 < 9) { h5t9 += thisValue; }
// //     else if (nHitsMinus1 < 16) { h10t16 += thisValue; }
// //     else if (nHitsMinus1 < 25) { h17t25 += thisValue; }
// //     else if (nHitsMinus1 < 50) { h26t50 += thisValue; }
// //     else if (nHitsMinus1 < 100) { h51t100 += thisValue; }
// //     else if (nHitsMinus1 < 200) { h101t200 += thisValue; }
// //     else if (nHitsMinus1 < 300) { h201t300 += thisValue; }
// //     else if (nHitsMinus1 < 400) { h301t400 += thisValue; }
// //     else if (nHitsMinus1 < 500) { h401t500 += thisValue; }
// //     else if (nHitsMinus1 < 600) { h501t600 += thisValue; }
// //     else if (nHitsMinus1 < 700) { h601t700 += thisValue; }
// //     else if (nHitsMinus1 < 800) { h701t800 += thisValue; }
// //     else { h800plus += thisValue; }
// //   }
// //   double pFac = 100. / double(nTotalHits); // percentageFactor
// //   B2DEBUG(2, printOccupancy);
// //   B2DEBUG(1, std::fixed << std::setprecision(2) << " of " << nTotalHits <<
// //           " svd-hit-combinations: Lists hits per sensor in percent\n 1\t|2-4\t|5-9\t|-16\t|-25\t|-50\t|-100\t|-200\t|-300\t|-400\t|-500\t|-600\t|-700\t|-800\t|>800\t|highest value occured \t|\n "
// //           << double(h1 * pFac) << "\t| " << double(h2t4 * pFac) << "\t| " << double(h5t9 * pFac) << "\t| " << double(
// //             h10t16 * pFac) << "\t| " << double(h17t25 * pFac) << "\t| " << double(h26t50 * pFac) << "\t| " << double(
// //             h51t100 * pFac) << "\t| " << double(h101t200 * pFac) << "\t| " << double(h201t300 * pFac) << "\t| " << double(
// //             h301t400 * pFac) << "\t| " << double(h401t500 * pFac) << "\t| " << double(h501t600 * pFac) << "\t| " << double(
// //             h601t700 * pFac) << "\t| " << double(h701t800 * pFac) << "\t| " << double(h800plus * pFac) << "\t| " << m_TESTERSVDOccupancy.size()
// //           + 1 << "\t\t\t| ")
// //   B2DEBUG(1, lineHigh << lineApnd << lineApnd)
// //
// //   B2DEBUG(1, std::fixed << std::setprecision(2) << " total time consumption in milliseconds: \n " <<
// //           "HSort\t|baseTF\t|sgFind\t|nbFind\t|CA \t|tcCol\t|tcFlt\t|kalmn\t|chkOvr\t|clnOvr\t|neuNet\t|others\t|\n" <<
// //           (m_TESTERtimeConsumption.hitSorting.count() * 0.001) << "\t|" << (m_TESTERtimeConsumption.baselineTF.count() * 0.001) << "\t|" <<
// //           (m_TESTERtimeConsumption.segFinder.count() * 0.001) << "\t|" << (m_TESTERtimeConsumption.nbFinder.count() * 0.001) << "\t|" <<
// //           (m_TESTERtimeConsumption.cellularAutomaton.count() * 0.001) << "\t|" << (m_TESTERtimeConsumption.tcc.count() * 0.001) << "\t|" <<
// //           (m_TESTERtimeConsumption.postCAFilter.count() * 0.001) << "\t|" << (m_TESTERtimeConsumption.kalmanStuff.count() * 0.001) << "\t|"
// //           << (m_TESTERtimeConsumption.checkOverlap.count() * 0.001) << "\t|" << (m_TESTERtimeConsumption.cleanOverlap.count() * 0.001) <<
// //           "\t|" << (m_TESTERtimeConsumption.neuronalStuff.count() * 0.001) << "\t|" << (m_TESTERtimeConsumption.intermediateStuff.count() *
// //               0.001) << "\t|")
// //
// // //   m_eventCounter
// //   B2DEBUG(1, std::fixed << std::setprecision(2) << " mean time consumption in microseconds: \n " <<
// //           "HSort\t|sgFind\t|nbFind\t|CA \t|tcCol\t|tcFlt\t|kalmn\t|chkOvr\t|clnOvr\t|neuNet\t|others\t|\n" <<
// //           (m_TESTERtimeConsumption.hitSorting.count() * invNEvents) << "\t|" << (m_TESTERtimeConsumption.segFinder.count() * invNEvents) <<
// //           "\t|" << (m_TESTERtimeConsumption.nbFinder.count() * invNEvents) << "\t|" << (m_TESTERtimeConsumption.cellularAutomaton.count() *
// //               invNEvents) << "\t|" << (m_TESTERtimeConsumption.tcc.count() * invNEvents) << "\t|" << (m_TESTERtimeConsumption.postCAFilter.count()
// //                   * invNEvents) << "\t|" << (m_TESTERtimeConsumption.kalmanStuff.count() * invNEvents) << "\t|" <<
// //           (m_TESTERtimeConsumption.checkOverlap.count() * invNEvents) << "\t|" << (m_TESTERtimeConsumption.cleanOverlap.count() * invNEvents)
// //           << "\t|" << (m_TESTERtimeConsumption.neuronalStuff.count() * invNEvents) << "\t|" << (m_TESTERtimeConsumption.
// //               intermediateStuff.count() * invNEvents) << "\t|")
// //
// //   B2DEBUG(2,
// //           "Explanation: HSort: hit sorting, baseTF: baseline TF, sgFind: segment finder , nbFind: neighbouring segments finder, CA: cellular automaton, tcCol: track candidate collector, tcFlt: track candidate filter (e.g. circleFit), kalmn: kalman filter, chkOvr: checking track candidates for overlapping clusters, clnOvr: cleaning track candidates for overlapping clusters, neuNet: neuronal network of Hopfield type, others: everything which was not listed above")
// //
// //   B2DEBUG(1, lineHigh << lineApnd << lineApnd)
// //
// //   int numLoggedEvents = m_TESTERlogEvents.size();
// //   int median = numLoggedEvents / 2;
// //   int q1 = numLoggedEvents / 100;
// //   int q25 = numLoggedEvents / 4;
// //   int q75 = median + q25;
// //   int q10 = numLoggedEvents / 10;
// //   int q90 = 9 * q10;
// //   int q99 = 99 * q1;
// //   B2DEBUG(1, m_PARAMnameOfInstance << " there were " << numLoggedEvents <<
// //           " events recorded by the eventLogger, listing slowest, fastest, median q0.1 and q0.9 event:" << endl)
// //   if (numLoggedEvents != 0) {
// //     int meanTimeConsumption = 0;
// //     for (EventInfoPackage& infoPackage : m_TESTERlogEvents) {
// //       meanTimeConsumption += infoPackage.duration.totalTime.count();
// //     }
// //     B2DEBUG(2, "slowest event: " << m_TESTERlogEvents.at(0).Print());
// //     B2DEBUG(2, "q1 event: " << m_TESTERlogEvents.at(q1).Print());
// //     B2DEBUG(2, "q10 event: " << m_TESTERlogEvents.at(q10).Print());
// //     B2DEBUG(2, "q25 event: " << m_TESTERlogEvents.at(q25).Print());
// //     B2DEBUG(2, "median event: " << m_TESTERlogEvents.at(median).Print());
// //     B2DEBUG(2, "q75 event: " << m_TESTERlogEvents.at(q75).Print());
// //     B2DEBUG(2, "q90 event: " << m_TESTERlogEvents.at(q90).Print());
// //     B2DEBUG(2, "q99 event: " << m_TESTERlogEvents.at(q99).Print());
// //     B2DEBUG(2, "fastest event: " << m_TESTERlogEvents.at(numLoggedEvents - 1).Print());
// //     B2DEBUG(1, "manually calculated mean: " << meanTimeConsumption / numLoggedEvents << ", and median: " << m_TESTERlogEvents.at(
// //               median).duration.totalTime.count() << " of time consumption per event");
// //
// //     vector<EventInfoPackage> logEventsCopy =
// //       m_TESTERlogEvents; // copying original since we want to change the internal order now for several times and do not want to break the original
// //     vector<EventInfoPackage> infoQ(
// //       5); // for each value we want to find the key figures, we store one entry. first is min, third is median, last is max
// //     stringstream pxdClusterStream, svdClusterStream, svdHitStream, twoHitCombiStream, twoHitActivatedStream, twoHitDiscardedStream;
// //
// //     // sort by nPxdClusters:
// //     std::sort(
// //       logEventsCopy.begin(),
// //       logEventsCopy.end(),
// //       [](const EventInfoPackage & a, const EventInfoPackage & b) -> bool { return a.nPXDClusters < b.nPXDClusters; }
// //     );
// //     infoQ.at(0).nPXDClusters = logEventsCopy.at(0).nPXDClusters;
// //     infoQ.at(1).nPXDClusters = logEventsCopy.at(q25).nPXDClusters;
// //     infoQ.at(2).nPXDClusters = logEventsCopy.at(median).nPXDClusters;
// //     infoQ.at(3).nPXDClusters = logEventsCopy.at(q75).nPXDClusters;
// //     infoQ.at(4).nPXDClusters = logEventsCopy.at(numLoggedEvents - 1).nPXDClusters;
// //     pxdClusterStream << infoQ[0].nPXDClusters << " / " << infoQ[1].nPXDClusters << " / " << infoQ[2].nPXDClusters << " / " <<
// //                      infoQ[3].nPXDClusters << " / " << infoQ[4].nPXDClusters << "\n";
// //
// //     // sort by m_evInfoPack.nSVDClusters:
// //     std::sort(
// //       logEventsCopy.begin(),
// //       logEventsCopy.end(),
// //       [](const EventInfoPackage & a, const EventInfoPackage & b) -> bool { return a.nSVDClusters < b.nSVDClusters; }
// //     );
// //     infoQ.at(0).nSVDClusters = logEventsCopy.at(0).nSVDClusters;
// //     infoQ.at(1).nSVDClusters = logEventsCopy.at(q25).nSVDClusters;
// //     infoQ.at(2).nSVDClusters = logEventsCopy.at(median).nSVDClusters;
// //     infoQ.at(3).nSVDClusters = logEventsCopy.at(q75).nSVDClusters;
// //     infoQ.at(4).nSVDClusters = logEventsCopy.at(numLoggedEvents - 1).nSVDClusters;
// //     svdClusterStream << infoQ[0].nSVDClusters << " / " << infoQ[1].nSVDClusters << " / " << infoQ[2].nSVDClusters << " / " <<
// //                      infoQ[3].nSVDClusters << " / " << infoQ[4].nSVDClusters << "\n";
// //
// //     // sort by nSVDClusterCombis:
// //     std::sort(
// //       logEventsCopy.begin(),
// //       logEventsCopy.end(),
// //       [](const EventInfoPackage & a, const EventInfoPackage & b) -> bool { return a.numSVDHits < b.numSVDHits; }
// //     );
// //     infoQ.at(0).numSVDHits = logEventsCopy.at(0).numSVDHits;
// //     infoQ.at(1).numSVDHits = logEventsCopy.at(q25).numSVDHits;
// //     infoQ.at(2).numSVDHits = logEventsCopy.at(median).numSVDHits;
// //     infoQ.at(3).numSVDHits = logEventsCopy.at(q75).numSVDHits;
// //     infoQ.at(4).numSVDHits = logEventsCopy.at(numLoggedEvents - 1).numSVDHits;
// //     svdHitStream << infoQ[0].numSVDHits << " / " << infoQ[1].numSVDHits << " / " << infoQ[2].numSVDHits << " / " << infoQ[3].numSVDHits
// //                  << " / " << infoQ[4].numSVDHits << "\n";
// //
// //     // sort by 2HitCombis:
// //     std::sort(
// //       logEventsCopy.begin(),
// //       logEventsCopy.end(),
// //       [](const EventInfoPackage & a, const EventInfoPackage & b) -> bool { return a.numHitCombisTotal < b.numHitCombisTotal; }
// //     );
// //     infoQ.at(0).numHitCombisTotal = logEventsCopy.at(0).numHitCombisTotal;
// //     infoQ.at(1).numHitCombisTotal = logEventsCopy.at(q25).numHitCombisTotal;
// //     infoQ.at(2).numHitCombisTotal = logEventsCopy.at(median).numHitCombisTotal;
// //     infoQ.at(3).numHitCombisTotal = logEventsCopy.at(q75).numHitCombisTotal;
// //     infoQ.at(4).numHitCombisTotal = logEventsCopy.at(numLoggedEvents - 1).numHitCombisTotal;
// //     twoHitCombiStream << infoQ[0].numHitCombisTotal << " / " << infoQ[1].numHitCombisTotal << " / " << infoQ[2].numHitCombisTotal <<
// //                       " / " << infoQ[3].numHitCombisTotal << " / " << infoQ[4].numHitCombisTotal << "\n";
// //
// //     // sort by 2HitCombisActivated:
// //     std::sort(
// //       logEventsCopy.begin(),
// //       logEventsCopy.end(),
// //       [](const EventInfoPackage & a, const EventInfoPackage & b) -> bool { return a.segFinderActivated < b.segFinderActivated; }
// //     );
// //     infoQ.at(0).segFinderActivated = logEventsCopy.at(0).segFinderActivated;
// //     infoQ.at(1).segFinderActivated = logEventsCopy.at(q25).segFinderActivated;
// //     infoQ.at(2).segFinderActivated = logEventsCopy.at(median).segFinderActivated;
// //     infoQ.at(3).segFinderActivated = logEventsCopy.at(q75).segFinderActivated;
// //     infoQ.at(4).segFinderActivated = logEventsCopy.at(numLoggedEvents - 1).segFinderActivated;
// //     twoHitActivatedStream << infoQ[0].segFinderActivated << " / " << infoQ[1].segFinderActivated << " / " << infoQ[2].segFinderActivated
// //                           << " / " << infoQ[3].segFinderActivated << " / " << infoQ[4].segFinderActivated << "\n";
// //
// //     // sort by 2HitCombisDiscarded:
// //     std::sort(
// //       logEventsCopy.begin(),
// //       logEventsCopy.end(),
// //       [](const EventInfoPackage & a, const EventInfoPackage & b) -> bool { return a.segFinderDiscarded < b.segFinderDiscarded; }
// //     );
// //     infoQ.at(0).segFinderDiscarded = logEventsCopy.at(0).segFinderDiscarded;
// //     infoQ.at(1).segFinderDiscarded = logEventsCopy.at(q25).segFinderDiscarded;
// //     infoQ.at(2).segFinderDiscarded = logEventsCopy.at(median).segFinderDiscarded;
// //     infoQ.at(3).segFinderDiscarded = logEventsCopy.at(q75).segFinderDiscarded;
// //     infoQ.at(4).segFinderDiscarded = logEventsCopy.at(numLoggedEvents - 1).segFinderDiscarded;
// //     twoHitDiscardedStream << infoQ[0].segFinderDiscarded << " / " << infoQ[1].segFinderDiscarded << " / " << infoQ[2].segFinderDiscarded
// //                           << " / " << infoQ[3].segFinderDiscarded << " / " << infoQ[4].segFinderDiscarded << "\n";
// //
// //     B2INFO(" VXDTF - endRun: ###############\n" <<
// //            "within " << m_eventCounter << " events, there were a total number of " << m_TESTERcountTotalTCsFinal << " TCs " <<
// //            "and " << float(m_TESTERcountTotalTCsFinal) * invNEvents << " TCs per event" <<
// //            "(" << m_TESTERbrokenEventsCtr << " events killed for high occupancy).\n" <<
// //            "Mean track length (indices/hits): " << float(m_TESTERcountTotalUsedIndicesFinal) / float(m_TESTERcountTotalTCsFinal) << "/" <<
// //            float(m_TESTERcountTotalUsedHitsFinal)
// //      / float(m_TESTERcountTotalTCsFinal) <<
// //          "\n min / q0.25 / median / q0.75 / max\n" <<
// //            "nPxdClusters           " << pxdClusterStream.str() <<
// //            "nSVDClusters           " << svdClusterStream.str() <<
// //            "nSVDClusterCombis      " << svdHitStream.str() <<
// //            "2HitCombis             " << twoHitCombiStream.str() <<
// //            "2HitCombisActivated    " << twoHitActivatedStream.str() <<
// //            "2HitCombisDiscarded    " << twoHitDiscardedStream.str() <<
// //            "VXDTF -endRun - end ###############")
// //   }
// //
// //
// //   // runWise cleanup:
// //   for (PassData* currentPass : m_passSetupVector) {
// //     for (secMapEntry aSector : currentPass->sectorMap) {
// //       delete aSector.second;
// //     }
// //     currentPass->sectorMap.clear();
// //     delete currentPass; // deleting struct itself
// //   }
// //
// //   m_passSetupVector.clear();
// // }
// //
// //
// //
// // void TFRedesignModule::terminate()
// // {
// //   /** REDESIGNCOMMENT TERMINATE 1:
// //    * * short:
// //    * root output for track- and event-wise info...
// //    *
// //    ** long (+personal comments):
// //    * only relevant for DQM-stuff
// //    *
// //    ** dependency of module parameters (global):
// //    *
// //    ** dependency of global in-module variables:
// //    * m_treeTrackWisePtr, m_treeEventWisePtr, m_rootFilePtr,
// //    *
// //    ** dependency of global stuff just because of B2XX-output or debugging only:
// //    * m_treeTrackWisePtr, m_treeEventWisePtr, m_rootFilePtr,
// //    *
// //    ** in-module-function-calls:
// //    */
// //   if (m_treeTrackWisePtr != NULL and m_treeEventWisePtr != NULL) {
// //     m_rootFilePtr->cd(); //important! without this the famework root I/O (SimpleOutput etc) could mix with the root I/O of this module
// //     m_treeTrackWisePtr->Write();
// //     m_treeEventWisePtr->Write();
// //     m_rootFilePtr->Close();
// //   }
// //   B2INFO("VXDTF::terminate: random number check: " << gRandom->Uniform(1.0))
// // }
// //
// //
// //
// // /** ***** findTCs ***** **/
// // ///Recursive CoreFunction of the Track Candidate Collector, stores every possible way starting at a Seed (VXDSegmentCell)
// // void TFRedesignModule::findTCs(TCsOfEvent& tcList,  VXDTFTrackCandidate* currentTC, short int maxLayer)
// // {
// //   /** REDESIGNCOMMENT FINDTCS 1:
// //    * * short:
// //    *
// //    ** long (+personal comments):
// //    * recursive
// //    *
// //    ** dependency of module parameters (global):
// //    *
// //    ** dependency of global in-module variables:
// //    *
// //    ** dependency of global stuff just because of B2XX-output or debugging only:
// //    *
// //    ** in-module-function-calls:
// //    * findTCs(tcList, currentTC, maxLayer)
// //    * findTCs(tcList, pTCCopy, maxLayer)
// //    */
// //   VXDSegmentCell* pLastSeg = currentTC->getSegments().back(); // get last entry in segList of TC
// //   if (FullSecID(pLastSeg->getOuterHit()->getSectorName()).getLayerID() != maxLayer) { pLastSeg->setSeed(false); }
// //
// //   const std::list<VXDSegmentCell*>* neighbours = pLastSeg->getAllInnerNeighbours();
// //   std::list<VXDSegmentCell*>::const_iterator nbIter = neighbours->begin();
// //   int nbSize = pLastSeg->sizeOfAllInnerNeighbours();
// //
// //   if (nbSize == 0)  { // currentTC is complete
// //     currentTC->setTrackNumber(tcList.size());
// //     tcList.push_back(currentTC);
// //   } else if (nbSize == 1) {
// //     VXDSegmentCell* pNextSeg =  *nbIter;
// //     currentTC->addSegments(pNextSeg);
// //     currentTC->addHits(pNextSeg->getInnerHit()); // automatically informs current Hit that it got owned by a TC
// //     findTCs(tcList, currentTC, maxLayer);
// //
// //   } else { // nbSize > 1
// //     ++nbIter; // iterator points to the second entry of the list of neighbours!
// //     for (; nbIter != neighbours->end(); ++nbIter) {
// //       VXDSegmentCell* pNextSeg =  *nbIter;
// //       VXDTFTrackCandidate* pTCCopy = new VXDTFTrackCandidate(currentTC);   // make a clone of old TC
// //       pTCCopy->addSegments(pNextSeg);
// //       pTCCopy->addHits(pNextSeg->getInnerHit());
// //
// //       findTCs(tcList, pTCCopy, maxLayer);
// //     } // makes clones of current TC for each neighbour (excluding the first one) and adds it to the TCs if neighbour fits in the scheme...
// //
// //     //separate step for the first neighbour in line (has to be done after the cloning parts)...
// //     VXDSegmentCell* pNextSeg =  *(neighbours->begin());
// //     currentTC->addSegments(pNextSeg);
// //     currentTC->addHits(pNextSeg->getInnerHit());
// //
// //     findTCs(tcList, currentTC, maxLayer);
// //   }
// // }
// //
// //
// //
// // /** ***** hopfield ***** **/
// // /// Neuronal network filtering overlapping Track Candidates by searching best subset of TC's
// // void TFRedesignModule::hopfield(TCsOfEvent& tcVector, double omega)
// // {
// //   /** REDESIGNCOMMENT HOPFIELD 1:
// //    * * short:
// //    *
// //    ** long (+personal comments):
// //    * that function has a lot of debug only output
// //    *
// //    ** dependency of module parameters (global):
// //    * m_PARAMdisplayCollector, m_PARAMDebugMode, m_PARAMnameOfInstance,
// //    * m_PARAMqiSmear
// //    *
// //    ** dependency of global in-module variables:
// //    * m_eventCounter, m_collector, m_TESTERbadHopfieldCtr,
// //    *
// //    *
// //    ** dependency of global stuff just because of B2XX-output or debugging only:
// //    * m_eventCounter, m_PARAMdisplayCollector, m_collector,
// //    * m_PARAMDebugMode, m_TESTERbadHopfieldCtr, m_PARAMnameOfInstance,
// //    * m_PARAMqiSmear
// //    *
// //    ** in-module-function-calls:
// //    * greedy(tcVector)
// //    */
// //   int nTCs = tcVector.size();
// //   if (nTCs < 2) { B2FATAL("Hopfield got only " << nTCs << " overlapping TCs! This should not be possible!"); return; }
// //
// //   // TMatrixD = MatrixT <double>
// //   TMatrixD W(nTCs, nTCs);  /// weight matrix, knows compatibility between each possible pair of TCs
// //   TMatrixD xMatrix(1, nTCs); /// Neuron values
// //   TMatrixD xMatrixCopy(1, nTCs); // copy of initial values
// //   TMatrixD xMatrixOld(1, nTCs);
// //   TMatrixD actMatrix(1, nTCs);
// //   TMatrixD tempMatrix(1, nTCs);
// //   TMatrixD tempXMatrix(1, nTCs);
// //
// //   vector<double> QIsOfTCs;
// //   QIsOfTCs.reserve(nTCs);
// //   for (VXDTFTrackCandidate* tc : tcVector) {
// //     B2DEBUG(10, "hopfield, QI: " << tc->getTrackQuality())
// //     QIsOfTCs.push_back(tc->getTrackQuality());
// //   }
// //
// //   int countCasesWhen2NeuronsAreCompatible = 0;
// //   double compatibleValue = (1.0 - omega) / double(nTCs - 1);
// //   list<int> hitsBoth, hitsItrk, hitsJtrk;
// //   for (int itrk = 0; itrk < nTCs; itrk++) {
// //
// //     hitsItrk = tcVector[itrk]->getHopfieldHitIndices();
// //     hitsItrk.sort();
// //     int nHitsItrk = hitsItrk.size();
// //
// //     for (int jtrk = itrk + 1; jtrk < nTCs; jtrk++) {
// //
// //       hitsBoth = hitsItrk;
// //       hitsJtrk = tcVector[jtrk]->getHopfieldHitIndices();
// //       hitsJtrk.sort();
// //       int nHitsJtrk = hitsJtrk.size();
// //
// //       hitsBoth.merge(hitsJtrk);
// //
// //       hitsBoth.unique();
// //
// //       int nHitsBoth = hitsBoth.size();
// //
// //       if (nHitsBoth < (nHitsItrk + nHitsJtrk)) { // means they share hits if true
// //         W(itrk, jtrk) = -1.0;
// //         W(jtrk, itrk) = -1.0;
// //       } else {
// //         W(itrk, jtrk) = compatibleValue;
// //         W(jtrk, itrk) = compatibleValue;
// //         countCasesWhen2NeuronsAreCompatible++;
// //       }
// //     }
// //   } // checking compatibility of TCs (compatible if they dont share hits, not compatible if they share ones)
// //
// //   if (countCasesWhen2NeuronsAreCompatible == 0) {
// //     B2DEBUG(2, "VXDTF event " << m_eventCounter << ": hopfield: no compatible neurons found, chosing TC by best QI...");
// //     int tempIndex = 0;
// //     double tempQI = tcVector[0]->getTrackQuality();
// //     for (int i = 1; i < nTCs; i++) {
// //       double tempQI2 = tcVector[i]->getTrackQuality();
// //       if (tempQI < tempQI2) { tempIndex = i; }
// //     }
// //     for (int i = 0; i < nTCs; i++) {
// //       if (i != tempIndex) {
// //         tcVector[i]->setCondition(false);
// //
// //         // Update Collector TC - hopfield
// //         if (m_PARAMdisplayCollector > 0) {
// //           m_collector.updateTC(tcVector[i]->getCollectorID(), CollectorTFInfo::m_nameHopfield, CollectorTFInfo::m_idHopfield, vector<int>(), {FilterID::hopfield});
// //         }
// //
// //
// //       }
// //     }
// //     return; // leaving hopfield after chosing the last man standing
// //   }
// //
// //   if ((m_PARAMDebugMode == true) && (LogSystem::Instance().isLevelEnabled(LogConfig::c_Debug, 100, PACKAGENAME()) == true)) {
// //     stringstream printOut;
// //     printMyMatrix(W, printOut);
// //     B2DEBUG(100, " weight matrix W: " << endl << printOut.str() << endl);
// //   }
// //
// //   vector<int> sequenceVector(nTCs, 0);
// //
// //   for (int i = 0; i < nTCs; i++) {
// //     double rNum = gRandom->Uniform(1.0);
// //     xMatrix(0, i) = rNum;
// //     sequenceVector[i] = i;
// //   }
// //   xMatrixCopy = xMatrix;
// //
// //   double T = 3.1; // original: 3.1
// //   double Tmin = 0.1;
// //   double cmax = 0.01;
// //   double c = 1.0;
// //   double act = 0.0;
// //
// //   int nNcounter = 0;
// //   std::array<double, 100> cValues; // protocolling all values of c
// //   cValues.fill(0);
// //
// //   while (c > cmax) {
// //
// //     random_shuffle(sequenceVector.begin(), sequenceVector.end(), rngWrapper);
// //
// //     xMatrixOld = xMatrix;
// //
// //     for (int i : sequenceVector) {
// //       double aTempVal = 0.0;
// //       for (int a = 0; a < nTCs; a++) { aTempVal = aTempVal + W(i, a) * xMatrix(0, a); } // doing it by hand...
// //
// //       act = aTempVal + omega * QIsOfTCs[i];
// //
// //       xMatrix(0, i) = 0.5 * (1. + tanh(act / T));
// //
// //       B2DEBUG(100, "tc, random number " << i << " -  old value: " << xMatrix(0, i))
// //     }
// //
// //     T = 0.5 * (T + Tmin);
// //
// //     tempMatrix = (xMatrix - xMatrixOld);
// //     tempMatrix.Abs();
// //     c = tempMatrix.Max();
// //     B2DEBUG(10, " c value is " << c << " at iteration " << nNcounter)
// //     cValues.at(nNcounter) = c;
// //
// //     xMatrixOld = xMatrix;
// //
// //     if (nNcounter == 99 || std::isnan(c) == true) {
// //       stringstream cOutPut;
// //       for (auto entry : cValues) { if (entry != 0) { cOutPut << entry << " ";} }
// //       B2ERROR("Hopfield took " << nNcounter << " iterations or is nan, current c/cmax: " << c << "/" << cmax << " c-history: " <<
// //               cOutPut.str()); break;
// //     }
// //     nNcounter++;
// //   }
// //
// //   B2DEBUG(3, "Hopfield network - found subset of TCs within " << nNcounter << " iterations... with c=" << c);
// //   list<VXDTFHit*> allHits;
// //   int survivorCtr = 0;
// //
// //   for (int i = 0; i < nTCs; i++) { if (xMatrix(0, i) > 0.7) { survivorCtr++; } }
// //
// //   if (survivorCtr == 0) {
// //     m_TESTERbadHopfieldCtr++;
// //     B2DEBUG(3, "VXDTF event " << m_eventCounter << ": hopfield had no survivors! now using greedy... ")
// //     greedy(tcVector); /// greedy
// //     if (LogSystem::Instance().isLevelEnabled(LogConfig::c_Debug, 4, PACKAGENAME()) == true) {
// //       for (int i = 0; i < nTCs; i++) {
// //         B2DEBUG(4, "tc " << i << " - got final neuron value: " << xMatrix(0,
// //                 i) << " while having " << int((tcVector.at(i)->getHits()).size()) << " hits and quality indicator " << tcVector.at(
// //                   i)->getTrackQuality())
// //       }
// //     }
// //
// //     survivorCtr = 0;
// //     for (VXDTFTrackCandidate* tc : tcVector) {
// //       if (tc->getCondition() == true)  { survivorCtr++; }
// //     } // should now have got some survivors
// //   } else {
// //     for (int i = 0; i < nTCs; i++) {
// //       B2DEBUG(50, "tc " << i << " - got final neuron value: " << xMatrix(0,
// //               i) << " while having " << int((tcVector.at(i)->getHits()).size()) << " hits and quality indicator " << tcVector.at(
// //                 i)->getTrackQuality())
// //       if (xMatrix(0, i) > 0.7) { /// do we want to let this threshold hardcoded?
// //         tcVector[i]->setCondition(true);
// //
// //         // Update Collector TC - hopfield
// //         if (m_PARAMdisplayCollector > 0) {
// //           m_collector.updateTC(tcVector[i]->getCollectorID(), "", CollectorTFInfo::m_idAlive, {FilterID::hopfield}, vector<int>());
// //         }
// //
// //
// //       } else {
// //         tcVector[i]->setCondition(false);
// //
// //         // Update Collector TC - hopfield
// //         if (m_PARAMdisplayCollector > 0) {
// //           m_collector.updateTC(tcVector[i]->getCollectorID(), CollectorTFInfo::m_nameHopfield, CollectorTFInfo::m_idHopfield, vector<int>(), {FilterID::hopfield});
// //         }
// //
// //       }
// //       tcVector[i]->setNeuronValue(xMatrix(0, i));
// //
// //       bool condi = tcVector[i]->getCondition();
// //       if (condi == true) {
// //         const vector<VXDTFHit*>& currentHits = tcVector[i]->getHits();
// //         for (int j = 0; j < int(currentHits.size()); ++j) { allHits.push_back(currentHits[j]); }
// //       }
// //     }
// //   }
// //
// //   if (survivorCtr == 0) { // in this case the NN AND greedy could not find any compatible entries
// //     B2WARNING(m_PARAMnameOfInstance << " smear:" << m_PARAMqiSmear << " event " << m_eventCounter <<
// //               ": hopfield and greedy had no survivors! ")
// //
// //     /** file output: */
// //     if (m_PARAMDebugMode == true) {
// //       stringstream noSurvivors, qiVec, xMatrixBegin, xMatrixEnd, weightMatrix, fileName;
// //       noSurvivors << "event " << m_eventCounter << endl;
// //       qiVec << "qiVector " << endl;
// //       xMatrixBegin << "neurons at start: " << endl;
// //       xMatrixEnd << "neurons at end: " << endl;
// //
// //       printMyMatrix(W, weightMatrix);
// //
// //       for (int i = 0; i < nTCs; i++) {
// //         qiVec << tcVector[i]->getTrackQuality() << " ";
// //         xMatrixBegin << xMatrixCopy(0, i) << " ";
// //         xMatrixEnd << xMatrix(0, i) << " ";
// //
// //         B2WARNING(m_PARAMnameOfInstance << " tc " << i << " - got final neuron value: " << xMatrix(0,
// //                   i) << " while having " << int((tcVector.at(i)->getHits()).size()) << " hits and quality indicator " << tcVector.at(
// //                     i)->getTrackQuality())
// //       }
// //
// //       noSurvivors << xMatrixBegin.str() << endl << xMatrixEnd.str() << endl << qiVec.str() << endl << weightMatrix.str() << endl;
// //       ofstream myfile;
// //       fileName << "noSurvivors" << m_PARAMnameOfInstance << ".txt";
// //       myfile.open((fileName.str()).c_str(), ios::out | ios::app);
// //       myfile << noSurvivors.str();
// //       myfile.close();
// //     }
// //     /** file output-end */
// //   }
// //   int sizeOld = allHits.size();
// //   list<VXDTFHit*> tempHits = allHits;
// //   allHits.sort(); allHits.unique();
// //   int sizeNew = allHits.size();
// //
// //   if (sizeOld != sizeNew) { /// checks for strange behavior of NN
// //     B2ERROR(m_PARAMnameOfInstance << ", NN smear" << m_PARAMqiSmear << " event " << m_eventCounter <<
// //             ": illegal result! Overlapping TCs (with " << sizeOld - sizeNew << " overlapping hits) accepted!")
// //     tempHits.sort();
// //     int hctr = 0;
// //     for (VXDTFHit* hit : tempHits) {
// //       if (hit->getDetectorType() == Const::IR) {
// //         B2ERROR("Even more illegal result: hit " << hctr << " is attached to interaction point! Perp/secID " << hit->Perp() << "/" <<
// //                 hit->getSectorString())
// //       } else if (hit->getDetectorType() == Const::PXD) {
// //         B2WARNING("hit " << hctr << " is a PXD-hit with clusterIndexUV: " << hit->getClusterIndexUV())
// //       } else if (hit->getDetectorType() == Const::TEST) {
// //         B2WARNING("hit " << hctr << " is a TEL-hit with clusterIndexUV: " << hit->getClusterIndexUV())
// //       } else if (hit->getDetectorType() == Const::SVD) {
// //         B2WARNING("hit " << hctr << " is a SVD-hit with clusterIndexU/V: " << hit->getClusterIndexU() << "/" << hit->getClusterIndexV())
// //       } else {
// //         B2ERROR("Most illegal result: hit " << hctr << " could not be attached to any detector!")
// //       }
// //       ++hctr;
// //     }
// //
// //     stringstream overlappingTCsAccepted, qiVec, xMatrixBegin, xMatrixEnd, weightMatrix, fileName;
// //     overlappingTCsAccepted << "event " << m_eventCounter << endl;
// //     qiVec << "qiVector " << endl;
// //     xMatrixBegin << "neurons at start: " << endl;
// //     xMatrixEnd << "neurons at end: " << endl;
// //
// //     printMyMatrix(W, weightMatrix);
// //
// //     for (int i = 0; i < nTCs; i++) {
// //       qiVec << tcVector[i]->getTrackQuality() << " ";
// //       xMatrixBegin << xMatrixCopy(0, i) << " ";
// //       xMatrixEnd << xMatrix(0, i) << " ";
// //     }
// //
// //     overlappingTCsAccepted << xMatrixBegin.str() << endl << xMatrixEnd.str() << endl << qiVec.str() << endl << weightMatrix.str() <<
// //                            endl;
// //     ofstream myfile;
// //     fileName << "overlappingTCsAccepted" << m_PARAMnameOfInstance << ".txt";
// //     myfile.open((fileName.str()).c_str(), ios::out | ios::app);
// //     myfile << overlappingTCsAccepted.str();
// //     myfile.close();
// //   }
// // }
// //
// //
// //
// // void TFRedesignModule::reserveHits(TCsOfEvent& tcVector, PassData* currentPass)
// // {
// //   /** REDESIGNCOMMENT RESERVEHITS 1:
// //    * * short:
// //    *
// //    ** long (+personal comments):
// //    *
// //    ** dependency of module parameters (global):
// //    *
// //    ** dependency of global in-module variables:
// //    *
// //    ** dependency of global stuff just because of B2XX-output or debugging only:
// //    *
// //    ** in-module-function-calls:
// //    */
// //   list< pair< double, VXDTFTrackCandidate*> > allTCs;
// //   double threshold = currentPass->reserveHitsThreshold;
// //   int countAlive = 0;
// //
// //   B2DEBUG(10, "reserveHits for pass " << currentPass->sectorSetup << " with threshold " << threshold << " and total number of TCs " <<
// //           tcVector.size())
// //
// //   for (VXDTFTrackCandidate* tc : tcVector) {  // store tcs in list of current TCs
// //     if (tc->getCondition() == false) continue;
// //     ++countAlive;
// //     if (tc->isReserved() == true) continue;
// //     allTCs.push_back({tc->getTrackQuality(), tc});
// //   }
// //
// //   allTCs.sort();
// //   allTCs.reverse();
// //   int tcsUnreserved = allTCs.size(), // nTCs which haven't reserved their hits
// //       limit2Go = int(double(tcsUnreserved) * threshold + 0.5), // nTCs allowed to reserve their hits
// //       count2Limit = 0,
// //       countSucceeded = 0;
// //   B2DEBUG(3, "reserveHits - pass " << currentPass->sectorSetup << ": threshold " << threshold << ", total number of TCs " <<
// //           tcVector.size() << ", alive " << countAlive << ", & unreserved " << tcsUnreserved << ", results in " << limit2Go <<
// //           " TCs to be allowed to reserve their hits")
// //
// //   list< pair< double, VXDTFTrackCandidate*> >::iterator currentTC = allTCs.begin();
// //   while (count2Limit < limit2Go) {
// //     bool succeeded = currentTC->second->setReserved();
// //     if (succeeded == true) ++countSucceeded;
// //     ++currentTC;
// //     ++count2Limit;
// //     if (std::distance(allTCs.begin(), currentTC) > tcsUnreserved) {
// //       B2FATAL(" reserveHits: iterator goes rampage!")
// //     }
// //   }
// //   B2DEBUG(3, "reserveHits - pass " << currentPass->sectorSetup << ": threshold " << threshold << ", totalTcs: " << tcVector.size() <<
// //           ", tcs alive&unreserved: " << allTCs.size() << ", allows " << limit2Go << " TCs to reserve their hits, managed " << countSucceeded
// //           << " to succeed")
// // }
// //
// //
// //
// // /** ***** greedy ***** **/
// // /// search for nonOverlapping trackCandidates using Greedy algorithm (start with TC of highest QI, remove all TCs incompatible with current TC, if there are still TCs there, repeat step until no incompatible TCs are there any more)
// // void TFRedesignModule::greedy(TCsOfEvent& tcVector)
// // {
// //   /** REDESIGNCOMMENT GREEDY 1:
// //    * * short:
// //    *
// //    ** long (+personal comments):
// //    *
// //    ** dependency of module parameters (global):
// //    *
// //    ** dependency of global in-module variables:
// //    *
// //    ** dependency of global stuff just because of B2XX-output or debugging only:
// //    *
// //    ** in-module-function-calls:
// //    * greedyRecursive(overlappingTCs, totalSurvivingQI, countSurvivors, countKills)
// //    */
// //   list< pair< double, VXDTFTrackCandidate*> > overlappingTCs;
// //
// //   int countTCsAliveAtStart = 0, countSurvivors = 0, countKills = 0;
// //   double totalSurvivingQI = 0, totalQI = 0;
// //   for (VXDTFTrackCandidate* tc : tcVector) {  // store tcs in list of current overlapping TCs
// //     ++countTCsAliveAtStart;
// //     if (tc->getCondition() == false) continue;
// //
// //     double qi = tc->getTrackQuality();
// //     totalQI += qi;
// //
// //     if (int(tc->getBookingRivals().size()) == 0) { // tc is clean and therefore automatically accepted
// //       totalSurvivingQI += qi;
// //       countSurvivors++;
// //       continue;
// //     }
// //
// //     overlappingTCs.push_back({qi, tc});
// //   }
// //
// //   overlappingTCs.sort();
// //   overlappingTCs.reverse();
// //
// //   greedyRecursive(overlappingTCs, totalSurvivingQI, countSurvivors, countKills);
// //
// //   B2DEBUG(3, "VXDTFModule::greedy: total number of TCs: " << tcVector.size() << " with totalQi " << totalQI <<
// //           ", TCs alive at begin of greedy algoritm: " << countTCsAliveAtStart << ", TCs survived: " << countSurvivors << ", TCs killed: " <<
// //           countKills)
// // }
// //
// //
// // /** ***** greedyRecursive ***** **/
// // /// used by VXDTFModule::greedy, recursive function which takes tc with highest QI and kills all its rivals. After that, TC gets removed and process is repeated with shrinking list of TCs until no TCs alive has got rivals alive
// // void TFRedesignModule::greedyRecursive(std::list< std::pair<double, VXDTFTrackCandidate*> >& overlappingTCs,
// //                                        double& totalSurvivingQI,
// //                                        int& countSurvivors,
// //                                        int& countKills)
// // {
// //   /** REDESIGNCOMMENT GREEDYRECURSIVE 1:
// //    * * short:
// //    *
// //    ** long (+personal comments):
// //    *
// //    ** dependency of module parameters (global):
// //    * m_PARAMdisplayCollector,
// //    *
// //    ** dependency of global in-module variables:
// //    * m_collector,
// //    *
// //    ** dependency of global stuff just because of B2XX-output or debugging only:
// //    * m_PARAMdisplayCollector, m_collector,
// //    *
// //    ** in-module-function-calls:
// //    * greedyRecursive(overlappingTCs, totalSurvivingQI, countSurvivors, countKills)
// //    */
// //   if (overlappingTCs.empty() == true) return;
// //
// //   list< pair<double, VXDTFTrackCandidate*> >::iterator tcEntry = overlappingTCs.begin();
// //
// //   while (tcEntry->second->getCondition() == false) {
// //     tcEntry = overlappingTCs.erase(tcEntry);
// //     if (tcEntry == overlappingTCs.end() or overlappingTCs.empty() == true) return;
// //   }
// //
// //   double qi = tcEntry->first;
// //
// //   for (VXDTFTrackCandidate* rival : tcEntry->second->getBookingRivals()) {
// //     if (rival->getCondition() == false) continue;
// //
// //     countKills++;
// //
// //     if (qi >= rival->getTrackQuality()) {
// //       rival->setCondition(false);
// //
// //       // Update Collector TC - hopfield
// //       if (m_PARAMdisplayCollector > 0) {
// //         m_collector.updateTC(rival->getCollectorID(), CollectorTFInfo::m_nameHopfield, CollectorTFInfo::m_idHopfield, vector<int>(), {FilterID::greedy});
// //       }
// //
// //     } else {
// //       tcEntry->second->setCondition(false);
// //
// //       // Update Collector TC - hopfield
// //       if (m_PARAMdisplayCollector > 0) {
// //         m_collector.updateTC(tcEntry->second->getCollectorID(),  CollectorTFInfo::m_nameHopfield, CollectorTFInfo::m_idHopfield,
// //                              vector<int>(), {FilterID::greedy});
// //       }
// //
// //
// //       break;
// //     }
// //   }
// //
// //   if (tcEntry->second->getCondition() == true) {
// //     countSurvivors++;
// //     totalSurvivingQI += qi;
// //   }
// //
// //   if (overlappingTCs.empty() != true) { overlappingTCs.pop_front(); }
// //
// //
// //   greedyRecursive(overlappingTCs, totalSurvivingQI, countSurvivors, countKills);
// //
// //   return;
// // }
// //
// //
// // /** ***** tcDuel ***** **/
// // /// for that easy situation we dont need the neuronal network or other algorithms for finding the best subset...
// // void TFRedesignModule::tcDuel(TCsOfEvent& tcVector)
// // {
// //   /** REDESIGNCOMMENT TCDUEL 1:
// //    * * short:
// //    *
// //    ** long (+personal comments):
// //    *
// //    ** dependency of module parameters (global):
// //    * m_PARAMdisplayCollector,
// //    *
// //    ** dependency of global in-module variables:
// //    * m_collector,
// //    *
// //    ** dependency of global stuff just because of B2XX-output or debugging only:
// //    * m_PARAMdisplayCollector, m_collector,
// //    *
// //    ** in-module-function-calls:
// //    */
// //   if (tcVector.at(0)->getTrackQuality() > tcVector.at(1)->getTrackQuality()) {
// //     tcVector.at(1)->setCondition(false);
// //
// //     // Update Collector TC - hopfield
// //     if (m_PARAMdisplayCollector > 0) {
// //       m_collector.updateTC(tcVector.at(1)->getCollectorID(), CollectorTFInfo::m_nameHopfield, CollectorTFInfo::m_idHopfield,
// //                            vector<int>(), {FilterID::tcDuel});
// //     }
// //
// //   } else {
// //     tcVector.at(0)->setCondition(false);
// //
// //     // Update Collector TC - hopfield
// //     if (m_PARAMdisplayCollector > 0) {
// //       m_collector.updateTC(tcVector.at(0)->getCollectorID(), CollectorTFInfo::m_nameHopfield, CollectorTFInfo::m_idHopfield,
// //                            vector<int>(), {FilterID::tcDuel});
// //     }
// //
// //
// //   }
// //   B2DEBUG(10, "2 overlapping Track Candidates found, tcDuel choses the last TC standing on its own")
// // }
// //
// //
// //
// // /** ***** searchSector4Hit ***** **/
// // /// searches for sectors fitting current hit coordinates, returns blank string if nothing could be found
// // Belle2::SectorNameAndPointerPair TFRedesignModule::searchSector4Hit(VxdID aVxdID,
// //     TVector3 localHit,
// // //     TVector3 sensorSize, /// WARNING TODO: remove that entry, its not needed anymore
// //     Belle2::MapOfSectors& m_sectorMap,
// //     vector<double>& uConfig,
// //     vector<double>& vConfig)
// // {
// //   /** REDESIGNCOMMENT SEARCHSECTOR4HIT 1:
// //    * * short:
// //    *
// //    ** long (+personal comments):
// //    * creates an instance of a VXD::SensorInfoBase every time it is called, could be optimized...
// //    *
// //    ** dependency of module parameters (global):
// //    *
// //    ** dependency of global in-module variables:
// //    *
// //    ** dependency of global stuff just because of B2XX-output or debugging only:
// //    *
// //    ** in-module-function-calls:
// //    */
// //   Belle2::MapOfSectors::iterator secMapIter = m_sectorMap.begin();
// //
// //   uint aSecID = 0;
// //   unsigned int aFullSecID = numeric_limits<unsigned int>::max();
// //
// //   SectorTools aTool = SectorTools();
// //
// //   std::pair<double, double> aCoorLocal  = {localHit.X(), localHit.Y()},
// //                             aCoorNormalized/*,
// //           aRelCoor = {localHit.X(), localHit.Y()}*/;
// //
// // //   VXD::SensorInfoBase sensorInfoBase = VXD::GeoCache::get(aVxdID);
// //
// //
// //   // Normalization of the Coordinates
// //   aCoorNormalized = SpacePoint::convertLocalToNormalizedCoordinates(aCoorLocal, aVxdID);
// //
// //   B2DEBUG(100, "searchSector4Hit: aCoorLocal: " << aCoorLocal.first << "/" << aCoorLocal.second << ", vxdID: " << aVxdID <<
// //           ", aCoorNormalized: " << aCoorNormalized.first << "/ " << aCoorNormalized.second);
// //
// //   // Calculate the SectorID (SectorTool-Object)
// //   aSecID = aTool.calcSecID(uConfig, vConfig, aCoorNormalized);
// //
// //   // if Sec ID not found aSecID = max. uShort
// //   if (aSecID != std::numeric_limits<unsigned short>::max()) {
// //     B2DEBUG(100, "aSecID: " << aSecID)
// //
// //     aFullSecID = FullSecID(aVxdID, false, aSecID).getFullSecID();
// //     B2DEBUG(100, "searchSector4Hit: calculated secID: " << aFullSecID << "/" << FullSecID(aFullSecID))
// //     secMapIter = m_sectorMap.find(aFullSecID);
// //
// //     if (secMapIter == m_sectorMap.end()) {
// //       aFullSecID = FullSecID(aVxdID, true, aSecID).getFullSecID();
// //       B2DEBUG(150, "searchSector4Hit: secID not found, trying : " << aFullSecID << "/" << FullSecID(aFullSecID))
// //       secMapIter = m_sectorMap.find(aFullSecID);
// //     }
// //
// //     if (secMapIter == m_sectorMap.end()) {
// //       B2DEBUG(5, "searchSector4Hit: secID " << aFullSecID << "/" << FullSecID(aFullSecID) << " does not exist in secMap. Setting to: " <<
// //               numeric_limits<unsigned int>::max() << "/" << FullSecID(numeric_limits<unsigned int>::max()))
// //       // if you wonder why the sublayerID is always 1 in that output, at this point searching for the secID with sublayerID 0 and 1 has been done, last setting was check for sublayerID of 1...
// //       aFullSecID = numeric_limits<unsigned int>::max();
// //     }
// //   } else {
// //     B2DEBUG(100, "Sec ID not found")
// //   }
// //
// //   return {aFullSecID, secMapIter}; // SectorNameAndPointerPair
// // }
// //
// //
// //
// // /** ***** segFinder ***** **/
// // /// searches for segments in given pass and returns number of discarded segments
// // unsigned int TFRedesignModule::segFinder(PassData* currentPass)
// // {
// //   /** REDESIGNCOMMENT SEGFINDER 1:
// //    * * short:
// //    *
// //    ** long (+personal comments):
// //    *
// //    ** dependency of module parameters (global):
// //    * m_PARAMDebugMode, m_PARAMdisplayCollector, m_PARAMkillEventForHighOccupancyThreshold,
// //    *
// //    ** dependency of global in-module variables:
// //    * m_eventCounter, m_collector, m_highOccupancyCase,
// //    *
// //    ** dependency of global stuff just because of B2XX-output or debugging only:
// //    * m_eventCounter, m_PARAMDebugMode, m_PARAMdisplayCollector,
// //    * m_collector,
// //    *
// //    ** in-module-function-calls:
// //    * importCollectorCell(m_aktpassNumber, "", CollectorTFInfo::m_idAlive, acceptedRejectedFilters, mainHit->getCollectorID(), friendHit->getCollectorID())
// //    * SegFinderHighOccupancy(currentPass, currentPass->threeHitFilterBox)
// //    */
// //
// //   B2DEBUG(10, "segFinder: for this pass: activatedSegFinderTests: " << currentPass->activatedSegFinderTests << ", d3D: " <<
// //           currentPass->distance3D.first << ", dXY: " << currentPass->distanceXY.first << ", dZ: " << currentPass->distanceZ.first <<
// //           ", normD3D: " << currentPass->normedDistance3D.first << ", slRZ: " << currentPass->slopeRZ.first << endl <<
// //           ", activatedHighOccupancySegFinderTests: " << currentPass->activatedHighOccupancySegFinderTests << ", a3Dhioc: " <<
// //           currentPass->anglesHighOccupancy3D.first << ", aXYhioc: " << currentPass->anglesHighOccupancyXY.first << ", aRZhioc: " <<
// //           currentPass->anglesHighOccupancyRZ.first << ", d2IPhioc: " << currentPass->distanceHighOccupancy2IP.first << ", dSlRZhioc: " <<
// //           currentPass->deltaSlopeHighOccupancyRZ.first << ", pThioc: " << currentPass->pTHighOccupancy.first << ", hFithioc: " <<
// //           currentPass->helixParameterHighOccupancyFit.first)
// //
// //   unsigned int currentFriendID = 0;
// //   TVector3 currentVector, tempVector;
// //   TVector3* currentCoords, *friendCoords;
// //   vector< pair < int, bool > >
// //   acceptedRejectedFilters; // collects for each filter whether it accepted or rejected the current segment. .first is filterID as int, .second is true if accepted, else false
// //   vector<pair <VXDTFHit*, VXDSector*> > allFriendHits; // collecting all hits of friend sectors and their sector for fast access
// //   bool accepted = false; // recycled return value of the filters
// //   int simpleSegmentQI = 0; // considers only min and max cutoff values, but could be weighed by order of relevance
// //   unsigned int discardedSegmentsCounter = 0;
// //   unsigned int activatedSegmentsCounter = 0;
// //
// //   /// JKL Feb2 2016: commented out for being able to continue working:
// // // // //   StoreObjPtr< SectorMap<VXDTFHit, TwoHitFilterShowCase> > sectorMap("", DataStore::c_Persistent);;
// // // // //   auto filters = sectorMap->getFilters(currentPass->getName());
// //
// //   for (VXDSector* mainSector : currentPass->sectorVector) {
// //
// //     const vector<VXDSector*>& hisFriends = mainSector->getFriendPointers(); // loading friends of sector
// //     FullSecID mainSecID = mainSector->getSecID();
// //     B2DEBUG(125, "SegFinder: current sector is called: " << mainSecID << " which has " << hisFriends.size() << " friends");
// //
// //     allFriendHits.clear();
// //     for (VXDSector* friendSector : hisFriends) {
// //       B2DEBUG(175, " > friendSector of " << mainSecID << " is called: " << FullSecID(friendSector->getSecID()));
// //
// //       const vector<VXDTFHit*>& friendHits = friendSector->getHits();
// //       for (VXDTFHit* friendHit : friendHits) {
// //         allFriendHits.push_back({friendHit, friendSector});
// //       }
// //     } // iterating through friendsectors and importing their containing hits. Gene: why are we paying this performance penalty?
// //
// //     const vector<VXDTFHit*>& ownHits = mainSector->getHits(); // loading own hits of sector
// //     int nCurrentHits = ownHits.size();
// //     int nFriendHits = allFriendHits.size();
// //     B2DEBUG(125, "SegFinder: sector " << mainSecID << " has " << nCurrentHits << " hits and additional " << nFriendHits <<
// //             " hits among its friends");
// //
// //     for (int currentMainHit = 0; currentMainHit < nCurrentHits; currentMainHit++) {
// //       int currentFriendHit = 0;
// //
// //       VXDTFHit* mainHit = ownHits.at(currentMainHit);//[currentMainHit]; // Gene: hit 1 on mainSecID
// //
// //       if (nFriendHits == 0 && mainHit->getNumberOfSegments() == 0) {
// //         B2DEBUG(10, "event " << m_eventCounter << ": current Hit has no friendHits although layer is " << mainSecID.getLayerID() <<
// //                 " and secID: " << mainSecID)
// //         continue;
// //       }
// //
// //       if (mainHit->isReserved() == true) { continue; }
// //       currentCoords = mainHit->getHitCoordinates();
// //
// //       B2DEBUG(20, "Sector " << mainSecID << " has got " << nCurrentHits << " hits in his area and " << nFriendHits <<
// //               " hits lying among his friends! ");
// //       for (auto& friendEntry : allFriendHits) {  /*(int friendHit = 0; friendHit < nFriendHits; ++friendHit)*/
// //         ++currentFriendHit;
// //         B2DEBUG(50, "At sector " << mainSecID << " hit " << currentMainHit << " and friend " << currentFriendHit << "/" << nFriendHits <<
// //                 " will now be executed:")
// //         acceptedRejectedFilters.clear();
// //         simpleSegmentQI = 0;
// //
// //         VXDTFHit* friendHit = friendEntry.first;  // Gene: hit 2 on friendSecID (see later)
// //         VXDSector* friendSector = friendEntry.second;
// //         if (friendHit->isReserved() == true) { continue; }
// //
// //         currentFriendID = friendSector->getSecID();
// //         FullSecID friendSecID = FullSecID(currentFriendID);
// //
// //     /// JKL Feb2 2016: commented out for being able to continue working:
// // // // //         // Gene all the work is concentrated here
// // // // //         bool GeneAccepted = filters->getTwoHitFilters(mainSecID, friendSecID).accept(*mainHit , *friendHit);
// //
// //         friendCoords = friendHit->getHitCoordinates();
// //
// //         currentPass->twoHitFilterBox.resetValues(*currentCoords, *friendCoords, mainSector, currentFriendID);
// //
// //
// //         if (currentPass->distance3D.first == true) { // min & max!
// //           accepted = currentPass->twoHitFilterBox.checkDist3D(FilterID::distance3D);
// //           if (accepted == true) {
// //             simpleSegmentQI++;
// //             B2DEBUG(10, " dist3d: segment approved! SectorCombi: " << mainSecID << "/" << friendSecID << ", minCutoff: " <<
// //                     currentPass->twoHitFilterBox.getCutoffs(FilterID::distance3D).first << ", calcValue: " << currentPass->twoHitFilterBox.calcDist3D()
// //                     << ", maxCutoff: " << currentPass->twoHitFilterBox.getCutoffs(FilterID::distance3D).second)
// //             acceptedRejectedFilters.push_back({FilterID::distance3D, true});
// //           } else {
// //             B2DEBUG(10, " dist3d: segment discarded! SectorCombi: " << mainSecID << "/" << friendSecID << ", minCutoff: " <<
// //                     currentPass->twoHitFilterBox.getCutoffs(FilterID::distance3D).first << ", calcValue: " << currentPass->twoHitFilterBox.calcDist3D()
// //                     << ", maxCutoff: " << currentPass->twoHitFilterBox.getCutoffs(FilterID::distance3D).second)
// //             acceptedRejectedFilters.push_back({FilterID::distance3D, false});
// //             if (m_PARAMDebugMode == true) {
// //               pair <double, double> cutoffs = currentPass->twoHitFilterBox.getCutoffs(FilterID::distance3D);
// //               B2WARNING("dist3D - SectorCombi: " << mainSecID << "/" << friendSecID << ", minCutoff: " << cutoffs.first << ", calcValue: " <<
// //                         currentPass->twoHitFilterBox.calcDist3D() << ", maxCutoff: " << cutoffs.second)
// //             }
// //           } // else segment not approved
// //         } else { B2DEBUG(175, " dist3d is not activated for pass: " << currentPass->sectorSetup << "!") }
// //
// //
// //         if (currentPass->distanceXY.first == true) { // min & max!
// //           accepted = currentPass->twoHitFilterBox.checkDistXY(FilterID::distanceXY);
// //           if (accepted == true) {
// //             simpleSegmentQI++;
// //             B2DEBUG(10, " distxy: segment approved! SectorCombi: " << mainSecID << "/" << friendSecID << ", minCutoff: " <<
// //                     currentPass->twoHitFilterBox.getCutoffs(FilterID::distanceXY).first << ", calcValue: " << currentPass->twoHitFilterBox.calcDistXY()
// //                     << ", maxCutoff: " << currentPass->twoHitFilterBox.getCutoffs(FilterID::distanceXY).second)
// //             acceptedRejectedFilters.push_back({FilterID::distanceXY, true});
// //           } else {
// //             B2DEBUG(10, " distxy: segment discarded! SectorCombi: " << mainSecID << "/" << friendSecID << ", minCutoff: " <<
// //                     currentPass->twoHitFilterBox.getCutoffs(FilterID::distanceXY).first << ", calcValue: " << currentPass->twoHitFilterBox.calcDistXY()
// //                     << ", maxCutoff: " << currentPass->twoHitFilterBox.getCutoffs(FilterID::distanceXY).second)
// //             acceptedRejectedFilters.push_back({FilterID::distanceXY, false});
// //             if (m_PARAMDebugMode == true) {
// //               pair <double, double> cutoffs = currentPass->twoHitFilterBox.getCutoffs(FilterID::distanceXY);
// //               B2WARNING("distxy - SectorCombi: " << mainSecID << "/" << friendSecID << ", minCutoff: " << cutoffs.first << ", calcValue: " <<
// //                         currentPass->twoHitFilterBox.calcDistXY() << ", maxCutoff: " << cutoffs.second)
// //             }
// //           } // else segment not approved
// //         } else { B2DEBUG(175, " distxy is not activated for pass: " << currentPass->sectorSetup << "!") }
// //
// //         if (currentPass->distanceZ.first == true) { // min & max!
// //           accepted = currentPass->twoHitFilterBox.checkDistZ(FilterID::distanceZ);
// //           if (accepted == true) {
// //             simpleSegmentQI++;
// //             B2DEBUG(150, " distz: segment approved! SectorCombi: " << mainSecID << "/" << friendSecID << ", minCutoff: " <<
// //                     currentPass->twoHitFilterBox.getCutoffs(FilterID::distanceZ).first << ", calcValue: " << currentPass->twoHitFilterBox.calcDistZ() <<
// //                     ", maxCutoff: " << currentPass->twoHitFilterBox.getCutoffs(FilterID::distanceZ).second)
// //             acceptedRejectedFilters.push_back({FilterID::distanceZ, true});
// //           } else {
// //             B2DEBUG(150, " distz: segment discarded! SectorCombi: " << mainSecID << "/" << friendSecID << ", minCutoff: " <<
// //                     currentPass->twoHitFilterBox.getCutoffs(FilterID::distanceZ).first << ", calcValue: " << currentPass->twoHitFilterBox.calcDistZ() <<
// //                     ", maxCutoff: " << currentPass->twoHitFilterBox.getCutoffs(FilterID::distanceZ).second)
// //             acceptedRejectedFilters.push_back({FilterID::distanceZ, false});
// //             if (m_PARAMDebugMode == true) {
// //               pair <double, double> cutoffs = currentPass->twoHitFilterBox.getCutoffs(FilterID::distanceZ);
// //               B2WARNING("distz - SectorCombi: " << mainSecID << "/" << friendSecID << ", minCutoff: " << cutoffs.first << ", calcValue: " <<
// //                         currentPass->twoHitFilterBox.calcDistZ() << ", maxCutoff: " << cutoffs.second)
// //             }
// //           } // else segment not approved
// //         } else { B2DEBUG(175, " distz is not activated for pass: " << currentPass->sectorSetup << "!") }
// //
// //         if (currentPass->normedDistance3D.first == true) { // max only
// //           accepted = currentPass->twoHitFilterBox.checkNormedDist3D(FilterID::normedDistance3D);
// //           if (accepted == true) {
// //             simpleSegmentQI++;
// //             B2DEBUG(150, " normeddist3d: segment approved! SectorCombi: " << mainSecID << "/" << friendSecID << ", minCutoff: " <<
// //                     currentPass->twoHitFilterBox.getCutoffs(FilterID::normedDistance3D).first << ", calcValue: " <<
// //                     currentPass->twoHitFilterBox.calcNormedDist3D() << ", maxCutoff: " << currentPass->twoHitFilterBox.getCutoffs(
// //                       FilterID::normedDistance3D).second)
// //             acceptedRejectedFilters.push_back({FilterID::normedDistance3D, true});
// //           } else {
// //             B2DEBUG(150, " normeddist3d: segment discarded! SectorCombi: " << mainSecID << "/" << friendSecID << ", minCutoff: " <<
// //                     currentPass->twoHitFilterBox.getCutoffs(FilterID::normedDistance3D).first << ", calcValue: " <<
// //                     currentPass->twoHitFilterBox.calcNormedDist3D() << ", maxCutoff: " << currentPass->twoHitFilterBox.getCutoffs(
// //                       FilterID::normedDistance3D).second)
// //             acceptedRejectedFilters.push_back({FilterID::normedDistance3D, false});
// //             if (m_PARAMDebugMode == true) {
// //               pair <double, double> cutoffs = currentPass->twoHitFilterBox.getCutoffs(FilterID::normedDistance3D);
// //               B2WARNING("normeddist3d - SectorCombi: " << mainSecID << "/" << friendSecID << ", minCutoff: " << cutoffs.first << ", calcValue: "
// //                         << currentPass->twoHitFilterBox.calcNormedDist3D() << ", maxCutoff: " << cutoffs.second)
// //             }
// //           } // else segment not approved
// //         } else { B2DEBUG(175, " normeddist3d is not activated for pass: " << currentPass->sectorSetup << "!") }
// //
// //         if (currentPass->slopeRZ.first == true) { // min & max!
// //           accepted = currentPass->twoHitFilterBox.checkSlopeRZ(FilterID::slopeRZ);
// //           if (accepted == true) {
// //             simpleSegmentQI++;
// //             B2DEBUG(150, " slopeRZ: segment approved! SectorCombi: " << mainSecID << "/" << friendSecID << ", minCutoff: " <<
// //                     currentPass->twoHitFilterBox.getCutoffs(FilterID::slopeRZ).first << ", calcValue: " << currentPass->twoHitFilterBox.calcSlopeRZ() <<
// //                     ", maxCutoff: " << currentPass->twoHitFilterBox.getCutoffs(FilterID::slopeRZ).second)
// //             acceptedRejectedFilters.push_back({FilterID::slopeRZ, true});
// //           } else {
// //             B2DEBUG(150, " slopeRZ: segment discarded! SectorCombi: " << mainSecID << "/" << friendSecID << ", minCutoff: " <<
// //                     currentPass->twoHitFilterBox.getCutoffs(FilterID::slopeRZ).first << ", calcValue: " << currentPass->twoHitFilterBox.calcSlopeRZ() <<
// //                     ", maxCutoff: " << currentPass->twoHitFilterBox.getCutoffs(FilterID::slopeRZ).second)
// //             acceptedRejectedFilters.push_back({FilterID::slopeRZ, false});
// //             if (m_PARAMDebugMode == true) {
// //               pair <double, double> cutoffs = currentPass->twoHitFilterBox.getCutoffs(FilterID::slopeRZ);
// //               B2WARNING("slopeRZ - SectorCombi: " << mainSecID << "/" << friendSecID << ", minCutoff: " << cutoffs.first << ", calcValue: " <<
// //                         currentPass->twoHitFilterBox.calcSlopeRZ() << ", maxCutoff: " << cutoffs.second)
// //             }
// //           } // else segment not approved
// //         } else { B2DEBUG(175, " slopeRZ is not activated for pass: " << currentPass->sectorSetup << "!") }
// //
// //         /// The following tests are debug-tests WARNING uncomment only if needed!:
// // //         if (currentPass->alwaysTrue2Hit.first == true) { // min & max!
// // //           accepted = currentPass->twoHitFilterBox.checkAlwaysTrue2Hit(FilterID::alwaysTrue2Hit);
// // //           if (accepted == true) {
// // //             simpleSegmentQI++;
// // //             B2DEBUG(150, " alwaysTrue2Hit: segment approved! SectorCombi: " << mainSecID << "/" << friendSecID)
// // //             acceptedRejectedFilters.push_back( {FilterID::alwaysTrue2Hit, true} );
// // //           } else {
// // //             B2DEBUG(150, " alwaysTrue2Hit: segment discarded! SectorCombi: " << mainSecID << "/" << friendSecID)
// // //             acceptedRejectedFilters.push_back( {FilterID::alwaysTrue2Hit, false} );
// // //           } // else segment not approved
// // //         } else { B2DEBUG(175, " alwaysTrue2Hit is not activated for pass: " << currentPass->sectorSetup << "!") }
// // //         if (currentPass->alwaysFalse2Hit.first == true) { // min & max!
// // //           accepted = currentPass->twoHitFilterBox.checkAlwaysFalse2Hit(FilterID::alwaysFalse2Hit);
// // //           if (accepted == true) {
// // //             simpleSegmentQI++;
// // //             B2DEBUG(150, " alwaysFalse2Hit: segment approved! SectorCombi: " << mainSecID << "/" << friendSecID)
// // //             acceptedRejectedFilters.push_back( {FilterID::alwaysFalse2Hit, true} );
// // //           } else {
// // //             B2DEBUG(150, " alwaysFalse2Hit: segment discarded! SectorCombi: " << mainSecID << "/" << friendSecID)
// // //             acceptedRejectedFilters.push_back( {FilterID::alwaysFalse2Hit, false} );
// // //           } // else segment not approved
// // //         } else { B2DEBUG(175, " alwaysFalse2Hit is not activated for pass: " << currentPass->sectorSetup << "!") }
// // //         if (currentPass->random2Hit.first == true) { // min & max!
// // //           accepted = currentPass->twoHitFilterBox.checkRandom2Hit(FilterID::random2Hit);
// // //           if (accepted == true) {
// // //             simpleSegmentQI++;
// // //             B2DEBUG(150, " random2Hit: segment approved! SectorCombi: " << mainSecID << "/" << friendSecID)
// // //             acceptedRejectedFilters.push_back( {FilterID::random2Hit, true} );
// // //           } else {
// // //             B2DEBUG(150, " random2Hit: segment discarded! SectorCombi: " << mainSecID << "/" << friendSecID)
// // //             acceptedRejectedFilters.push_back( {FilterID::random2Hit, false} );
// // //           } // else segment not approved
// // //         } else { B2DEBUG(175, " random2Hit is not activated for pass: " << currentPass->sectorSetup << "!") }
// //
// // //         cout << endl
// //
// // //              << currentPass->twoHitFilterBox.getCutoffs(FilterID::distance3D).first << " < "
// // //              << currentPass->twoHitFilterBox.calcDist3D() << " < "
// // //              << currentPass->twoHitFilterBox.getCutoffs(FilterID::distance3D).second << " | "
// //
// // //              << currentPass->twoHitFilterBox.getCutoffs(FilterID::distanceXY).first << " < "
// // //              << currentPass->twoHitFilterBox.calcDistXY() << " < "
// // //              << currentPass->twoHitFilterBox.getCutoffs(FilterID::distanceXY).second << " | "
// //
// // //              << currentPass->twoHitFilterBox.getCutoffs(FilterID::slopeRZ).first << " < "
// // //              << currentPass->twoHitFilterBox.calcSlopeRZ() << " < "
// // //              << currentPass->twoHitFilterBox.getCutoffs(FilterID::slopeRZ).second << " | "
// //
// // //              << mainSecID.getFullSecID() << " " << friendSecID.getFullSecID() << " GenRes: "
// //
// // //              << GeneAccepted << " " << (currentPass->activatedSegFinderTests == simpleSegmentQI)
// // //       << endl;
// //
// //         if (simpleSegmentQI < currentPass->activatedSegFinderTests) {
// //           if (LogSystem::Instance().isLevelEnabled(LogConfig::c_Debug, 50, PACKAGENAME()) == true) {
// //             stringstream outputStream;
// //             for (auto& entry : acceptedRejectedFilters) {
// //               outputStream << FilterID().getFilterString(entry.first) << " " << entry.second << ", ";
// //             }
// //             B2DEBUG(50, "SegFINDER: segment discarded! simpleSegmentQI = " << simpleSegmentQI << ", threshold: " <<
// //                     currentPass->activatedSegFinderTests << " Outer/inner Segment: " << mainSecID << "/" << friendSecID << endl << "FilterResults: " <<
// //                     outputStream.str() << ", needed threshold: " << currentPass->activatedSegFinderTests  << "\n")
// //           }
// //
// //           discardedSegmentsCounter++;
// //
// //           // Collector Cells Import for not saved VXDSegmentCell
// //           if (m_PARAMdisplayCollector > 0) {
// //             int cell_id =
// //               importCollectorCell(m_aktpassNumber, "", CollectorTFInfo::m_idAlive, acceptedRejectedFilters, mainHit->getCollectorID(),
// //                                   friendHit->getCollectorID());
// //
// //             // Seperate Update is necessary to mark it died (because of died_at of other Objects)
// //             m_collector.updateCell(cell_id, CollectorTFInfo::m_nameCellFinder, CollectorTFInfo::m_idCellFinder, vector<int>(), vector<int>(),
// //                                    -1, -2, -1, vector<int>());
// //
// //             // Cell ID not saved because no VXDSegmentCell created
// //           }
// //
// //           continue;
// //         }
// //
// //         if ((m_highOccupancyCase == true) && (currentMainHit < (nCurrentHits - 1))
// //             && (currentPass->activatedHighOccupancySegFinderTests != 0)) {
// //           currentPass->threeHitFilterBox.resetValues(*currentCoords, *friendCoords, currentPass->origin, mainSector, currentFriendID);
// //           bool testPassed = SegFinderHighOccupancy(currentPass, currentPass->threeHitFilterBox);
// //           if (testPassed == false) {
// //             if (LogSystem::Instance().isLevelEnabled(LogConfig::c_Debug, 50, PACKAGENAME()) == true) {
// //               stringstream outputStream;
// //               for (auto& entry : acceptedRejectedFilters) {
// //                 outputStream << FilterID().getFilterString(entry.first) << " " << entry.second << ", ";
// //               }
// //               B2DEBUG(1, "SegFINDER: segment discarded! Outer/inner Segment: " << mainSecID << "/" << friendSecID << endl << "FilterResults: " <<
// //                       outputStream.str() << ", needed threshold: " << currentPass->activatedSegFinderTests << "\n")
// //             }
// //             discardedSegmentsCounter++;
// //
// //             // Collector Cells Import for not saved VXDSegmentCell
// //             if (m_PARAMdisplayCollector > 0) {
// //               int cell_id =
// //                 importCollectorCell(m_aktpassNumber, "", CollectorTFInfo::m_idAlive, acceptedRejectedFilters, mainHit->getCollectorID(),
// //                                     friendHit->getCollectorID());
// //
// //               // Seperate Update is necessary to mark it died (because of died_at of other Objects)
// //               m_collector.updateCell(cell_id, CollectorTFInfo::m_nameCellFinder, CollectorTFInfo::m_idCellFinder, vector<int>(), vector<int>(),
// //                                      -1, -2, -1, vector<int>());
// //               // Cell ID not saved because no VXDSegmentCell created
// //             }
// //
// //             continue;
// //           }
// //         }
// //         if (LogSystem::Instance().isLevelEnabled(LogConfig::c_Debug, 50, PACKAGENAME()) == true) {
// //           stringstream outputStream;
// //           for (auto& entry : acceptedRejectedFilters) {
// //             outputStream << FilterID().getFilterString(entry.first) << " " << entry.second << ", ";
// //           }
// //           B2DEBUG(50, "SegFINDER: segment approved!  Outer/inner Segment: " << mainSecID << "/" << FullSecID(
// //                     currentFriendID) << endl << "FilterResults: " << outputStream.str() << "\n")
// //         }
// //         VXDSegmentCell* pCell = new VXDSegmentCell(mainHit, friendHit, mainSector, friendSector);
// //
// //         currentPass->activeCellList.push_back(pCell);
// //         currentPass->totalCellVector.push_back(pCell);
// //         int segPos = currentPass->totalCellVector.size() - 1;
// //         friendHit->addOuterCell(segPos);
// //         mainHit->addInnerCell(segPos);
// //
// //         mainSector->addInnerSegmentCell(pCell);
// //         friendSector->addOuterSegmentCell(pCell);
// //         ++activatedSegmentsCounter;
// //
// //         // Collector Cells Import (Main)
// //         if (m_PARAMdisplayCollector > 0) {
// //           int cell_id = importCollectorCell(m_aktpassNumber, "", CollectorTFInfo::m_idAlive, acceptedRejectedFilters,
// //                                             mainHit->getCollectorID(), friendHit->getCollectorID());
// //
// //           // Collector Cell <=> Cell
// //           pCell->setCollectorID(cell_id);
// //         }
// //
// //         if (activatedSegmentsCounter > m_PARAMkillEventForHighOccupancyThreshold) {
// //           B2DEBUG(1, "number of activated segments reached threshold " << activatedSegmentsCounter << ", stopping segFinder now")
// //           return discardedSegmentsCounter;
// //         } // security check
// //       } //iterating through all my friendHits
// //     }
// //   } // iterating through all active sectors - segFinder
// //
// //   B2DEBUG(10, "SegFINDER: manually counted activated/discarded segments: " << activatedSegmentsCounter << "/" <<
// //           discardedSegmentsCounter << ", activeCellList.size: " << currentPass->activeCellList.size() << ", totalCellVector.size: " <<
// //           currentPass->totalCellVector.size())
// //   return discardedSegmentsCounter;
// // }
// //
// //
// //
// //
// // // Imports Collector Cell (only for Collector)
// // // changes acceptedRejectedFilters => acceptedFilters, rejectedFilters
// // int TFRedesignModule::importCollectorCell(int pass_index, std::string died_at, int died_id,
// //                                           std::vector<std::pair<int, bool>> acceptedRejectedFilters, int hit1, int hit2)
// // {
// //   /** REDESIGNCOMMENT IMPORTCOLLECTORCELL 1:
// //    * * short:
// //    *
// //    ** long (+personal comments):
// //    * the whole function is just for debugging/testing/DQM
// //    *
// //    ** dependency of module parameters (global):
// //    *
// //    ** dependency of global in-module variables:
// //    * m_collector
// //    *
// //    ** dependency of global stuff just because of B2XX-output or debugging only:
// //    * m_collector
// //    *
// //    ** in-module-function-calls:
// //    */
// //
// //   std::vector<int> assigned_Hit_IDs = { hit1, hit2};
// //
// //   std::vector<int> acceptedFilters;
// //   std::vector<int> rejectedFilters;
// //
// //   for (auto entry : acceptedRejectedFilters) {
// //     B2DEBUG(10, "acceptedRejected: " << entry.first << "; (True/False): " << entry.second);
// //     if (entry.second == true) {
// //       acceptedFilters.push_back(entry.first);
// //
// //     } else {
// //       rejectedFilters.push_back(entry.first);
// //     }
// //
// //   }
// //
// //   int cell_id = m_collector.importCell(pass_index, died_at, died_id, acceptedFilters, rejectedFilters, assigned_Hit_IDs);
// //
// //   return cell_id;
// // }
// //
// //
// //
// //
// //
// // bool TFRedesignModule::SegFinderHighOccupancy(PassData* currentPass, NbFinderFilters& threeHitFilterBox)
// // {
// //   /** REDESIGNCOMMENT SEGFINDERHIGHOCCUPANCY 1:
// //    * * short:
// //    *
// //    ** long (+personal comments):
// //    * does partially the same as neighborfinder.
// //    * Difference is e.g. that instead of combining two neighboring cells,
// //    * a cell is combined with a virtual cell which has the shared hit as outer hit and the origin as inner hit.
// //    * Maybe an intelligent rewrite allows to replace this one using a more flexible neighborfinder-implementation
// //    *
// //    ** dependency of module parameters (global):
// //    * m_PARAMDebugMode,
// //    *
// //    ** dependency of global in-module variables:
// //    *
// //    ** dependency of global stuff just because of B2XX-output or debugging only:
// //    * m_PARAMDebugMode,
// //    *
// //    ** in-module-function-calls:
// //    */
// //   int simpleSegmentQI = 0;
// //   vector< pair < int, bool > >
// //   acceptedRejectedFilters; // collects for each filter whether it accepted or rejected the current segment. .first is filterID as int, .second is true if accepted, else false
// //   bool accepted;
// //
// //   if (currentPass->anglesHighOccupancy3D.first == true) { // min & max!
// //     accepted = threeHitFilterBox.checkAngle3D(FilterID::anglesHighOccupancy3D);
// //     if (accepted == true) {
// //       simpleSegmentQI++;
// //       B2DEBUG(150, " anglesHighOccupancy3D: segment approved!")
// //       acceptedRejectedFilters.push_back({FilterID::anglesHighOccupancy3D, true});
// //     } else {
// //       B2DEBUG(150, " anglesHighOccupancy3D: segment discarded!")
// //       acceptedRejectedFilters.push_back({FilterID::anglesHighOccupancy3D, false});
// //       if (m_PARAMDebugMode == true) {
// //         pair <double, double> cutoffs = threeHitFilterBox.getCutoffs(FilterID::anglesHighOccupancy3D);
// //         B2WARNING("anglesHighOccupancy3D - minCutoff: " << cutoffs.first << ", calcValue: " << threeHitFilterBox.calcAngle3D() <<
// //                   ", maxCutoff: " << cutoffs.second)
// //       }
// //     } // else segment not approved
// //   } else { B2DEBUG(175, " anglesHighOccupancy3D is not activated for pass: " << currentPass->sectorSetup << "!") }
// //
// //   if (currentPass->anglesHighOccupancyXY.first == true) { // min & max!
// //     accepted = threeHitFilterBox.checkAngleXY(FilterID::anglesHighOccupancyXY);
// //     if (accepted == true) {
// //       simpleSegmentQI++;
// //       B2DEBUG(150, " anglesHighOccupancyxy: segment approved!")
// //       acceptedRejectedFilters.push_back({FilterID::anglesHighOccupancyXY, true});
// //     } else {
// //       B2DEBUG(150, " anglesHighOccupancyxy: segment discarded!")
// //       acceptedRejectedFilters.push_back({FilterID::anglesHighOccupancyXY, false});
// //       if (m_PARAMDebugMode == true) {
// //         pair <double, double> cutoffs = threeHitFilterBox.getCutoffs(FilterID::anglesHighOccupancyXY);
// //         B2WARNING("anglesHighOccupancyxy - minCutoff: " << cutoffs.first << ", calcValue: " << threeHitFilterBox.calcAngleXY() <<
// //                   ", maxCutoff: " << cutoffs.second)
// //       }
// //     } // else segment not approved
// //   } else { B2DEBUG(175, " anglesHighOccupancyxy is not activated for pass: " << currentPass->sectorSetup << "!") }
// //
// //   if (currentPass->anglesHighOccupancyRZ.first == true) { // min & max!
// //     accepted = threeHitFilterBox.checkAngleRZ(FilterID::anglesHighOccupancyRZ);
// //     if (accepted == true) {
// //       simpleSegmentQI++;
// //       B2DEBUG(150, " anglesHighOccupancyRZ: segment approved!")
// //       acceptedRejectedFilters.push_back({FilterID::anglesHighOccupancyRZ, true});
// //     } else {
// //       B2DEBUG(150, " anglesHighOccupancyRZ: segment discarded!")
// //       acceptedRejectedFilters.push_back({FilterID::anglesHighOccupancyRZ, false});
// //       if (m_PARAMDebugMode == true) {
// //         pair <double, double> cutoffs = threeHitFilterBox.getCutoffs(FilterID::anglesHighOccupancyRZ);
// //         B2WARNING("anglesHighOccupancyRZ - minCutoff: " << cutoffs.first << ", calcValue: " << threeHitFilterBox.calcAngleRZ() <<
// //                   ", maxCutoff: " << cutoffs.second)
// //       }
// //     } // else segment not approved
// //   } else { B2DEBUG(175, " anglesHighOccupancyRZ is not activated for pass: " << currentPass->sectorSetup << "!") }
// //
// //   if (currentPass->distanceHighOccupancy2IP.first == true) { // max only
// //     try {
// //       accepted = threeHitFilterBox.checkCircleDist2IP(FilterID::distanceHighOccupancy2IP);
// //     } catch (FilterExceptions::Straight_Line& anException) {
// //       B2WARNING("Exception caught: " << FilterID::distanceHighOccupancy2IP << " failed with exception: " << anException.what() <<
// //                 " test-result is set negative...")
// //       accepted = false;
// //     } catch (FilterExceptions::Circle_too_small& anException) {
// //       B2WARNING("Exception caught: " << FilterID::distanceHighOccupancy2IP << " failed with exception: " << anException.what() <<
// //                 " test-result is set negative...")
// //       accepted = false;
// //     }
// //     if (accepted == true) {
// //       simpleSegmentQI++;
// //       B2DEBUG(150, " distanceHighOccupancy2IP: segment approved!")
// //       acceptedRejectedFilters.push_back({FilterID::distanceHighOccupancy2IP, true});
// //     } else {
// //       B2DEBUG(150, " distanceHighOccupancy2IP: segment discarded!")
// //       acceptedRejectedFilters.push_back({FilterID::distanceHighOccupancy2IP, false});
// //       if (m_PARAMDebugMode == true) {
// //         pair <double, double> cutoffs = threeHitFilterBox.getCutoffs(FilterID::distanceHighOccupancy2IP);
// //         B2WARNING("distanceHighOccupancy2IP - minCutoff: " << cutoffs.first << ", calcValue: " << threeHitFilterBox.calcCircleDist2IP() <<
// //                   ", maxCutoff: " << cutoffs.second)
// //       }
// //     } // else segment not approved
// //   } else { B2DEBUG(175, " distanceHighOccupancy2IP is not activated for pass: " << currentPass->sectorSetup << "!") }
// //
// //   if (currentPass->deltaSlopeHighOccupancyRZ.first == true) { // min & max!
// //     accepted = threeHitFilterBox.checkDeltaSlopeRZ(FilterID::deltaSlopeHighOccupancyRZ);
// //     if (accepted == true) {
// //       simpleSegmentQI++;
// //       B2DEBUG(150, " deltaSlopeHighOccupancyRZ: segment approved!")
// //       acceptedRejectedFilters.push_back({FilterID::deltaSlopeHighOccupancyRZ, true});
// //     } else {
// //       B2DEBUG(150, " deltaSlopeHighOccupancyRZ: segment discarded!")
// //       acceptedRejectedFilters.push_back({FilterID::deltaSlopeHighOccupancyRZ, false});
// //       if (m_PARAMDebugMode == true) {
// //         pair <double, double> cutoffs = threeHitFilterBox.getCutoffs(FilterID::deltaSlopeHighOccupancyRZ);
// //         B2WARNING("deltaSlopeHighOccupancyRZ - minCutoff: " << cutoffs.first << ", calcValue: " << threeHitFilterBox.calcDeltaSlopeRZ() <<
// //                   ", maxCutoff: " << cutoffs.second)
// //       }
// //     } // else segment not approved
// //   } else { B2DEBUG(175, " deltaSlopeHighOccupancyRZ is not activated for pass: " << currentPass->sectorSetup << "!") }
// //
// //   if (currentPass->pTHighOccupancy.first == true) { // min & max!
// //     try {
// //       accepted = threeHitFilterBox.checkPt(FilterID::pTHighOccupancy);
// //     } catch (FilterExceptions::Straight_Line& anException) {
// //       B2WARNING("Exception caught: " << FilterID::pTHighOccupancy << " failed with exception: " << anException.what() <<
// //                 " test-result is set negative...")
// //       accepted = false;
// //     } catch (FilterExceptions::Circle_too_small& anException) {
// //       B2WARNING("Exception caught: " << FilterID::pTHighOccupancy << " failed with exception: " << anException.what() <<
// //                 " test-result is set negative...")
// //       accepted = false;
// //     }
// //     if (accepted == true) {
// //       simpleSegmentQI++;
// //       B2DEBUG(150, " pTHighOccupancy: segment approved!")
// //       acceptedRejectedFilters.push_back({FilterID::pTHighOccupancy, true});
// //     } else {
// //       B2DEBUG(150, " pTHighOccupancy: segment discarded!")
// //       acceptedRejectedFilters.push_back({FilterID::pTHighOccupancy, false});
// //       if (m_PARAMDebugMode == true) {
// //         pair <double, double> cutoffs = threeHitFilterBox.getCutoffs(FilterID::pTHighOccupancy);
// //         B2WARNING("pTHighOccupancy - minCutoff: " << cutoffs.first << ", calcValue: " << threeHitFilterBox.calcPt() << ", maxCutoff: " <<
// //                   cutoffs.second)
// //       }
// //     } // else segment not approved
// //   } else { B2DEBUG(175, " pTHighOccupancy is not activated for pass: " << currentPass->sectorSetup << "!") }
// //
// //   if (currentPass->helixParameterHighOccupancyFit.first == true) { // min & max!
// //     try {
// //       accepted = threeHitFilterBox.checkHelixParameterFit(FilterID::helixParameterHighOccupancyFit);
// //     } catch (FilterExceptions::Straight_Line& anException) {
// //       B2WARNING("Exception caught: " << FilterID::helixParameterHighOccupancyFit << " failed with exception: " << anException.what() <<
// //                 " test-result is set negative...")
// //       accepted = false;
// //     } catch (FilterExceptions::Circle_too_small& anException) {
// //       B2WARNING("Exception caught: helixParameterHighOccupancyFit failed with exception: " << anException.what() <<
// //                 " test-result is set negative...")
// //       accepted = false;
// //     }
// //     if (accepted == true) {
// //       simpleSegmentQI++;
// //       B2DEBUG(150, " helixParameterHighOccupancyFit: segment approved!")
// //       acceptedRejectedFilters.push_back({FilterID::helixParameterHighOccupancyFit, true});
// //     } else {
// //       B2DEBUG(150, " helixParameterHighOccupancyFit: segment discarded!")
// //       acceptedRejectedFilters.push_back({FilterID::helixParameterHighOccupancyFit, false});
// //       if (m_PARAMDebugMode == true) {
// //         pair <double, double> cutoffs = threeHitFilterBox.getCutoffs(FilterID::helixParameterHighOccupancyFit);
// //         B2WARNING("helixParameterHighOccupancyFit - minCutoff: " << cutoffs.first << ", calcValue: " <<
// //                   threeHitFilterBox.calcHelixParameterFit() << ", maxCutoff: " << cutoffs.second)
// //       }
// //     } // else segment not approved
// //   } else { B2DEBUG(175, " slopeRZ is not activated for pass: " << currentPass->sectorSetup << "!") }
// //
// //   if (simpleSegmentQI < currentPass->activatedHighOccupancySegFinderTests) {
// //     if (LogSystem::Instance().isLevelEnabled(LogConfig::c_Debug, 75, PACKAGENAME()) == true) {
// //       stringstream outputStream;
// //       for (auto& entry : acceptedRejectedFilters) {
// //         outputStream << FilterID().getFilterString(entry.first) << " " << entry.second << ", ";
// //       }
// //       B2DEBUG(50, "SegFINDER HIOC: segment discarded! simpleSegmentQI = " << simpleSegmentQI << ", threshold: " <<
// //               currentPass->activatedHighOccupancySegFinderTests << "FilterResults: " << outputStream.str() << ", needed threshold: " <<
// //               currentPass->activatedHighOccupancySegFinderTests << "\n")
// //     }
// //     return false;
// //   } else {
// //     if (LogSystem::Instance().isLevelEnabled(LogConfig::c_Debug, 75, PACKAGENAME()) == true) {
// //       stringstream outputStream;
// //       for (auto& entry : acceptedRejectedFilters) {
// //         outputStream << FilterID().getFilterString(entry.first) << " " << entry.second << ", ";
// //       }
// //       B2DEBUG(50, "SegFINDER HIOC: segment approved! simpleSegmentQI = " << simpleSegmentQI << ", threshold: " <<
// //               currentPass->activatedHighOccupancySegFinderTests << "FilterResults: " << outputStream.str() << ", needed threshold: " <<
// //               currentPass->activatedHighOccupancySegFinderTests << "\n")
// //     }
// //     return true;
// //   }
// // }
// //
// //
// //
// // /** ***** neighbourFinder ***** **/
// // /// filters neighbouring segments in given pass and returns number of discarded segments
// // int TFRedesignModule::neighbourFinder(PassData* currentPass)
// // {
// //   /** REDESIGNCOMMENT NEIGHBOURFINDER 1:
// //    * * short:
// //    *
// //    ** long (+personal comments):
// //    *
// //    ** dependency of module parameters (global):
// //    * m_PARAMDebugMode, m_PARAMdisplayCollector
// //    *
// //    ** dependency of global in-module variables:
// //    * m_highOccupancyCase, m_collector
// //    *
// //    ** dependency of global stuff just because of B2XX-output or debugging only:
// //    * m_PARAMDebugMode, m_PARAMdisplayCollector, m_collector,
// //    *
// //    ** in-module-function-calls:
// //    * NbFinderHighOccupancy(currentPass, currentPass->fourHitFilterBox)
// //    */
// //   B2DEBUG(10, "nbFinder: for this pass: activatedNbFinderTests: " << currentPass->activatedNbFinderTests << ", a3D: " <<
// //           currentPass->angles3D.first << ", aXY: " << currentPass->anglesXY.first << ", aRZ: " << currentPass->anglesRZ.first << ", d2IP: " <<
// //           currentPass->distance2IP.first << ", dSlRZ: " << currentPass->deltaSlopeRZ.first << ", pT: " << currentPass->pT.first << ", hFit: "
// //           << currentPass->helixParameterFit.first << ", SOverZ: " << currentPass->deltaSOverZ.first << ", hFit: " <<
// //           currentPass->deltaSlopeZOverS.first << endl << "activatedHighOccupancyNbFinderTests: " <<
// //           currentPass->activatedHighOccupancyNbFinderTests << ", dD2IPhioc: " << currentPass->deltaDistanceHighOccupancy2IP.first <<
// //           ", dPthioc: " << currentPass->deltaPtHighOccupancy.first)
// //
// //   vector< pair < int, bool > >
// //   acceptedRejectedFilters; // collects for each filter whether it accepted or rejected the current segment. .first is filterID as int, .second is true if accepted, else false
// //   bool accepted = false; // recycled return value of the filters
// //   int NFdiscardedSegmentsCounter = 0, activatedSeedsCounter = 0;
// //   unsigned int currentFriendID; // not needed: outerLayerID, innerLayerID
// //   TVector3 outerVector, centerVector, innerVector, outerTempVector;
// //   TVector3* outerCoords, *centerCoords, *innerCoords;
// //   TVector3 innerTempVector,
// //            cpA/*central point of innerSegment*/,
// //            cpB/*central point of mediumSegment*/,
// //            nA/*normal vector of segment a*/,
// //            nB/*normal vector of segment b*/,
// //            intersectionAB;
// //   int simpleSegmentQI =
// //     0; // better than segmentApproved, but still digital (only min and max cutoff values), but could be weighed by order of relevance
// //   unsigned int centerLayerIDNumber = 0;
// //
// //
// //   for (VXDSector* mainSector : currentPass->sectorVector) {
// //     FullSecID mainSecID = mainSector->getSecID();
// //     B2DEBUG(75, "neighbourFinder: SectorSequence is named " << mainSecID);
// //
// //     vector<VXDSegmentCell*> outerSegments = mainSector->getInnerSegmentCells(); // loading segments of sector
// //     int nOuterSegments = outerSegments.size();
// //     for (int thisOuterSegment = 0; thisOuterSegment < nOuterSegments; thisOuterSegment++) {
// //       outerCoords = outerSegments[thisOuterSegment]->getOuterHit()->getHitCoordinates();
// //       centerCoords = outerSegments[thisOuterSegment]->getInnerHit()->getHitCoordinates();
// //       currentFriendID = outerSegments[thisOuterSegment]->getInnerHit()->getSectorName();
// //       centerLayerIDNumber = FullSecID(currentFriendID).getLayerID();// for seed-check
// //
// //       const vector<int>& innerSegments = outerSegments[thisOuterSegment]->getInnerHit()->getAttachedInnerCell();
// //       int nInnerSegments = innerSegments.size();
// //
// //       for (int thisInnerSegment = 0; thisInnerSegment < nInnerSegments; thisInnerSegment++) {
// //         acceptedRejectedFilters.clear();
// //         VXDSegmentCell* currentInnerSeg = currentPass->totalCellVector[innerSegments[thisInnerSegment]];
// //         innerCoords = currentInnerSeg->getInnerHit()->getHitCoordinates();
// //
// //         simpleSegmentQI = 0;
// //         currentPass->threeHitFilterBox.resetValues(*outerCoords, *centerCoords, *innerCoords, mainSector, currentFriendID);
// //
// //         if (currentPass->angles3D.first == true) { // min & max!
// //           accepted = currentPass->threeHitFilterBox.checkAngle3D(FilterID::angles3D);
// //           if (accepted == true) {
// //             simpleSegmentQI++;
// //             B2DEBUG(150, " angles3D: segment approved!")
// //             acceptedRejectedFilters.push_back({FilterID::angles3D, true});
// //           } else {
// //             B2DEBUG(150, " angles3D: segment discarded!")
// //             acceptedRejectedFilters.push_back({FilterID::angles3D, false});
// //             if (m_PARAMDebugMode == true) {
// //               pair <double, double> cutoffs = currentPass->threeHitFilterBox.getCutoffs(FilterID::angles3D);
// //               B2WARNING("angles3D - SectorCombi: " << mainSecID << "/" << FullSecID(currentFriendID) << ", minCutoff: " << cutoffs.first <<
// //                         ", calcValue: " << currentPass->threeHitFilterBox.calcAngle3D() << ", maxCutoff: " << cutoffs.second)
// //             }
// //           } // else segment not approved
// //         } else { B2DEBUG(175, " angles3D is not activated for pass: " << currentPass->sectorSetup << "!") }
// //
// //         if (currentPass->anglesXY.first == true) { // min & max!
// //           accepted = currentPass->threeHitFilterBox.checkAngleXY(FilterID::anglesXY);
// //           if (accepted == true) {
// //             simpleSegmentQI++;
// //             B2DEBUG(150, " anglesXY: segment approved!")
// //             acceptedRejectedFilters.push_back({FilterID::anglesXY, true});
// //           } else {
// //             B2DEBUG(150, " anglesXY: segment discarded!")
// //             acceptedRejectedFilters.push_back({FilterID::anglesXY, false});
// //             if (m_PARAMDebugMode == true) {
// //               pair <double, double> cutoffs = currentPass->threeHitFilterBox.getCutoffs(FilterID::anglesXY);
// //               B2WARNING("anglesXY - SectorCombi: " << mainSecID << "/" << FullSecID(currentFriendID) << ", minCutoff: " << cutoffs.first <<
// //                         ", calcValue: " << currentPass->threeHitFilterBox.calcAngleXY() << ", maxCutoff: " << cutoffs.second)
// //             }
// //           } // else segment not approved
// //         } else { B2DEBUG(175, " anglesXY is not activated for pass: " << currentPass->sectorSetup << "!") }
// //
// //         if (currentPass->anglesRZ.first == true) { // min & max!
// //           accepted = currentPass->threeHitFilterBox.checkAngleRZ(FilterID::anglesRZ);
// //           if (accepted == true) {
// //             simpleSegmentQI++;
// //             B2DEBUG(150, " anglesRZ: segment approved!")
// //             acceptedRejectedFilters.push_back({FilterID::anglesRZ, true});
// //           } else {
// //             B2DEBUG(150, " anglesRZ: segment discarded!")
// //             acceptedRejectedFilters.push_back({FilterID::anglesRZ, false});
// //             if (m_PARAMDebugMode == true) {
// //               pair <double, double> cutoffs = currentPass->threeHitFilterBox.getCutoffs(FilterID::anglesRZ);
// //               B2WARNING("anglesRZ - SectorCombi: " << mainSecID << "/" << FullSecID(currentFriendID) << ", minCutoff: " << cutoffs.first <<
// //                         ", calcValue: " << currentPass->threeHitFilterBox.calcAngleRZ() << ", maxCutoff: " << cutoffs.second)
// //             }
// //           } // else segment not approved
// //         } else { B2DEBUG(175, " anglesRZ is not activated for pass: " << currentPass->sectorSetup << "!") }
// //
// //         if (currentPass->distance2IP.first == true) { // max only
// //           try {
// //             accepted = currentPass->threeHitFilterBox.checkCircleDist2IP(FilterID::distance2IP);
// //           } catch (FilterExceptions::Straight_Line& anException) {
// //             B2WARNING("Exception caught: " << FilterID::distance2IP << " failed with exception: " << anException.what() <<
// //                       " test-result is set negative...")
// //             accepted = false;
// //           } catch (FilterExceptions::Circle_too_small& anException) {
// //             B2WARNING("Exception caught: " << FilterID::distance2IP << " failed with exception: " << anException.what() <<
// //                       " test-result is set negative...")
// //             accepted = false;
// //           }
// //           if (accepted == true) {
// //             simpleSegmentQI++;
// //             B2DEBUG(150, " distance2IP: segment approved!")
// //             acceptedRejectedFilters.push_back({FilterID::distance2IP, true});
// //           } else {
// //             B2DEBUG(150, " distance2IP: segment discarded!")
// //             acceptedRejectedFilters.push_back({FilterID::distance2IP, false});
// //             if (m_PARAMDebugMode == true) {
// //               pair <double, double> cutoffs = currentPass->threeHitFilterBox.getCutoffs(FilterID::distance2IP);
// //               B2WARNING("distance2IP - SectorCombi: " << mainSecID << "/" << FullSecID(currentFriendID) << ", minCutoff: " << cutoffs.first <<
// //                         ", calcValue: " << currentPass->threeHitFilterBox.calcCircleDist2IP() << ", maxCutoff: " << cutoffs.second)
// //             }
// //           } // else segment not approved
// //         } else { B2DEBUG(175, " distance2IP is not activated for pass: " << currentPass->sectorSetup << "!") }
// //
// //         if (currentPass->deltaSlopeRZ.first == true) { // min & max!
// //           accepted = currentPass->threeHitFilterBox.checkDeltaSlopeRZ(FilterID::deltaSlopeRZ);
// //           if (accepted == true) {
// //             simpleSegmentQI++;
// //             B2DEBUG(150, " deltaSlopeRZ: segment approved!")
// //             acceptedRejectedFilters.push_back({FilterID::deltaSlopeRZ, true});
// //           } else {
// //             B2DEBUG(150, " deltaSlopeRZ: segment discarded!")
// //             acceptedRejectedFilters.push_back({FilterID::deltaSlopeRZ, false});
// //             if (m_PARAMDebugMode == true) {
// //               pair <double, double> cutoffs = currentPass->threeHitFilterBox.getCutoffs(FilterID::deltaSlopeRZ);
// //               B2WARNING("deltaSlopeRZ - SectorCombi: " << mainSecID << "/" << FullSecID(currentFriendID) << ", minCutoff: " << cutoffs.first <<
// //                         ", calcValue: " << currentPass->threeHitFilterBox.calcDeltaSlopeRZ() << ", maxCutoff: " << cutoffs.second)
// //             }
// //           } // else segment not approved
// //         } else { B2DEBUG(175, " deltaSlopeRZ is not activated for pass: " << currentPass->sectorSetup << "!") }
// //
// //         if (currentPass->pT.first == true) { // min & max!
// //           try {
// //             accepted = currentPass->threeHitFilterBox.checkPt(FilterID::pT);
// //           } catch (FilterExceptions::Straight_Line& anException) {
// //             B2WARNING("Exception caught: " << FilterID::pT << " failed with exception: " << anException.what() <<
// //                       " test-result is set negative...")
// //             accepted = false;
// //           } catch (FilterExceptions::Circle_too_small& anException) {
// //             B2WARNING("Exception caught: " << FilterID::pT << " failed with exception: " << anException.what() <<
// //                       " test-result is set negative...")
// //             accepted = false;
// //           }
// //           if (accepted == true) {
// //             simpleSegmentQI++;
// //             B2DEBUG(150, " pT: segment approved!")
// //             acceptedRejectedFilters.push_back({FilterID::pT, true});
// //           } else {
// //             B2DEBUG(150, " pT: segment discarded!")
// //             acceptedRejectedFilters.push_back({FilterID::pT, false});
// //             if (m_PARAMDebugMode == true) {
// //               pair <double, double> cutoffs = currentPass->threeHitFilterBox.getCutoffs(FilterID::pT);
// //               B2WARNING("pT - SectorCombi: " << mainSecID << "/" << FullSecID(currentFriendID) << ", minCutoff: " << cutoffs.first <<
// //                         ", calcValue: " << currentPass->threeHitFilterBox.calcPt() << ", maxCutoff: " << cutoffs.second)
// //             }
// //           } // else segment not approved
// //         } else { B2DEBUG(175, " pT is not activated for pass: " << currentPass->sectorSetup << "!") }
// //
// //         if (currentPass->helixParameterFit.first == true) { // min & max!
// //           try {
// //             accepted = currentPass->threeHitFilterBox.checkHelixParameterFit(FilterID::helixParameterFit);
// //           } catch (FilterExceptions::Straight_Line& anException) {
// //             B2WARNING("Exception caught: " << FilterID::helixParameterFit << " failed with exception: " << anException.what() <<
// //                       " test-result is set negative...")
// //             accepted = false;
// //           } catch (FilterExceptions::Circle_too_small& anException) {
// //             B2WARNING("Exception caught: " << FilterID::helixParameterFit << " failed with exception: " << anException.what() <<
// //                       " test-result is set negative...")
// //             accepted = false;
// //           }
// //
// //           if (accepted == true) {
// //             simpleSegmentQI++;
// //             B2DEBUG(150, " helixParameterFit: segment approved!")
// //             acceptedRejectedFilters.push_back({FilterID::helixParameterFit, true});
// //           } else {
// //             B2DEBUG(150, " helixParameterFit: segment discarded!")
// //             acceptedRejectedFilters.push_back({FilterID::helixParameterFit, false});
// //             if (m_PARAMDebugMode == true) {
// //               pair <double, double> cutoffs = currentPass->threeHitFilterBox.getCutoffs(FilterID::helixParameterFit);
// //               B2WARNING("helixParameterFit - SectorCombi: " << mainSecID << "/" << FullSecID(currentFriendID) << ", minCutoff: " << cutoffs.first
// //                         << ", calcValue: " << currentPass->threeHitFilterBox.calcHelixParameterFit() << ", maxCutoff: " << cutoffs.second)
// //             }
// //           } // else segment not approved
// //         } else { B2DEBUG(175, " helixParameterFit is not activated for pass: " << currentPass->sectorSetup << "!") }
// //
// //         if (currentPass->deltaSOverZ.first == true) { // min & max!
// //           accepted = currentPass->threeHitFilterBox.checkDeltaSOverZ(FilterID::deltaSOverZ);
// //           if (accepted == true) {
// //             simpleSegmentQI++;
// //             B2DEBUG(150, " deltaSOverZ: segment approved!")
// //             acceptedRejectedFilters.push_back({FilterID::deltaSOverZ, true});
// //           } else {
// //             B2DEBUG(150, " deltaSOverZ: segment discarded!")
// //             acceptedRejectedFilters.push_back({FilterID::deltaSOverZ, false});
// //             if (m_PARAMDebugMode == true) {
// //               pair <double, double> cutoffs = currentPass->threeHitFilterBox.getCutoffs(FilterID::deltaSOverZ);
// //               B2WARNING("deltaSOverZ - SectorCombi: " << mainSecID << "/" << FullSecID(currentFriendID) << ", minCutoff: " << cutoffs.first <<
// //                         ", calcValue: " << currentPass->threeHitFilterBox.calcDeltaSOverZ() << ", maxCutoff: " << cutoffs.second)
// //             }
// //           } // else segment not approved
// //         } else { B2DEBUG(175, " deltaSOverZ is not activated for pass: " << currentPass->sectorSetup << "!") }
// //
// //         if (currentPass->deltaSlopeZOverS.first == true) { // min & max!
// //           accepted = currentPass->threeHitFilterBox.checkDeltaSlopeZOverS(FilterID::deltaSlopeZOverS);
// //           if (accepted == true) {
// //             simpleSegmentQI++;
// //             B2DEBUG(150, " deltaSlopeZOverS: segment approved!")
// //             acceptedRejectedFilters.push_back({FilterID::deltaSlopeZOverS, true});
// //           } else {
// //             B2DEBUG(150, " deltaSlopeZOverS: segment discarded!")
// //             acceptedRejectedFilters.push_back({FilterID::deltaSlopeZOverS, false});
// //             if (m_PARAMDebugMode == true) {
// //               pair <double, double> cutoffs = currentPass->threeHitFilterBox.getCutoffs(FilterID::deltaSlopeZOverS);
// //               B2WARNING("deltaSlopeZOverS - SectorCombi: " << mainSecID << "/" << FullSecID(currentFriendID) << ", minCutoff: " << cutoffs.first
// //                         << ", calcValue: " << currentPass->threeHitFilterBox.calcDeltaSlopeZOverS() << ", maxCutoff: " << cutoffs.second)
// //             }
// //           } // else segment not approved
// //         } else { B2DEBUG(175, " deltaSlopeZOverS is not activated for pass: " << currentPass->sectorSetup << "!") }
// //
// //         // The following tests are debug-tests WARNING uncomment only if needed!:
// // //         if (currentPass->alwaysTrue3Hit.first == true) { // min & max!
// // //           accepted = currentPass->threeHitFilterBox.checkAlwaysTrue3Hit(FilterID::alwaysTrue3Hit);
// // //           if (accepted == true) {
// // //             simpleSegmentQI++;
// // //             B2DEBUG(150, " alwaysTrue3Hit: segment approved! SectorCombi: " << mainSecID << "/" << FullSecID(currentFriendID))
// // //             acceptedRejectedFilters.push_back( {FilterID::alwaysTrue3Hit, true} );
// // //           } else {
// // //             B2DEBUG(150, " alwaysTrue3Hit: segment discarded! SectorCombi: " << mainSecID << "/" << FullSecID(currentFriendID))
// // //             acceptedRejectedFilters.push_back( {FilterID::alwaysTrue3Hit, false} );
// // //           } // else segment not approved
// // //         } else { B2DEBUG(175, " alwaysTrue3Hit is not activated for pass: " << currentPass->sectorSetup << "!") }
// // //         if (currentPass->alwaysFalse3Hit.first == true) { // min & max!
// // //           accepted = currentPass->threeHitFilterBox.checkAlwaysFalse3Hit(FilterID::alwaysFalse3Hit);
// // //           if (accepted == true) {
// // //             simpleSegmentQI++;
// // //             B2DEBUG(150, " alwaysFalse3Hit: segment approved! SectorCombi: " << mainSecID << "/" << FullSecID(currentFriendID))
// // //             acceptedRejectedFilters.push_back( {FilterID::alwaysFalse3Hit, true} );
// // //           } else {
// // //             B2DEBUG(150, " alwaysFalse3Hit: segment discarded! SectorCombi: " << mainSecID << "/" << FullSecID(currentFriendID))
// // //             acceptedRejectedFilters.push_back( {FilterID::alwaysFalse3Hit, false} );
// // //           } // else segment not approved
// // //         } else { B2DEBUG(175, " alwaysFalse3Hit is not activated for pass: " << currentPass->sectorSetup << "!") }
// // //         if (currentPass->random3Hit.first == true) { // min & max!
// // //           accepted = currentPass->threeHitFilterBox.checkRandom3Hit(FilterID::random3Hit);
// // //           if (accepted == true) {
// // //             simpleSegmentQI++;
// // //             B2DEBUG(150, " random3Hit: segment approved! SectorCombi: " << mainSecID << "/" << FullSecID(currentFriendID))
// // //             acceptedRejectedFilters.push_back( {FilterID::random3Hit, true} );
// // //           } else {
// // //             B2DEBUG(150, " random3Hit: segment discarded! SectorCombi: " << mainSecID << "/" << FullSecID(currentFriendID))
// // //             acceptedRejectedFilters.push_back( {FilterID::random3Hit, false} );
// // //           } // else segment not approved
// // //         } else { B2DEBUG(175, " random3Hit is not activated for pass: " << currentPass->sectorSetup << "!") }
// //
// //
// //         if (simpleSegmentQI < currentPass->activatedNbFinderTests) {
// //           if (LogSystem::Instance().isLevelEnabled(LogConfig::c_Debug, 50, PACKAGENAME()) == true) {
// //             stringstream outputStream;
// //             for (auto& entry : acceptedRejectedFilters) {
// //               outputStream << FilterID().getFilterString(entry.first) << " " << entry.second << ", ";
// //             }
// //             B2DEBUG(50, "neighbourFINDER: segment discarded! simpleSegmentQI = " << simpleSegmentQI << ", threshold: " <<
// //                     currentPass->activatedNbFinderTests << " Outer/inner Segment: " << mainSecID << "/" << FullSecID(
// //                       currentFriendID) << "/" << FullSecID(currentInnerSeg->getInnerHit()->getSectorName()) << endl << "FilterResults: " <<
// //                     outputStream.str()  << endl)
// //           }
// //
// //
// //           // Collector-TRACKLET: here 3-hit-cell-discarded update (nbFinder)
// //           if (m_PARAMdisplayCollector > 0) {
// //             B2DEBUG(100, "VXDTF: Display: Module Collector importTracklet");
// //
// //             // Filters vectors for update
// //             std::vector<int> acceptedFilters;
// //             std::vector<int> rejectedFilters;
// //
// //             for (auto entry : acceptedRejectedFilters) {
// //               B2DEBUG(100, "Collector-TRACKLET discarded: acceptedRejected: " << entry.first << "; (T/F): " << entry.second);
// //               if (entry.second == true) {
// //                 acceptedFilters.push_back(entry.first);
// //
// //               } else {
// //                 rejectedFilters.push_back(entry.first);
// //               }
// //
// //             }
// //
// //             std::vector<std::pair<int, unsigned int>> allSegments;
// //
// //             // Connected Cells => vector for import
// //             allSegments.push_back(make_pair(outerSegments[thisOuterSegment]->getCollectorID(), outerSegments[thisOuterSegment]->getState()));
// //             allSegments.push_back(make_pair(currentInnerSeg->getCollectorID(), currentInnerSeg->getState()));
// //
// //             //importTracklet(int passIndex, std::string diedAt, int diedId, std::vector<int> accepted, std::vector<int> rejected, const std::vector<std::pair<int, unsigned int>> assignedCellIDs)
// //
// //             // delete possible ???
// //             m_collector.importTracklet(m_aktpassNumber, "", CollectorTFInfo::m_idAlive, acceptedFilters, rejectedFilters, allSegments);
// //
// //           }
// //
// //
// //
// //
// //
// //           continue;
// //         }
// //         if ((m_highOccupancyCase == true) && (currentPass->activatedHighOccupancyNbFinderTests != 0)) {
// //           if (currentPass->origin != *innerCoords) {
// //             currentPass->fourHitFilterBox.resetValues(*outerCoords, *centerCoords, *innerCoords, currentPass->origin, mainSector,
// //                                                       currentFriendID);
// //             bool testPassed = NbFinderHighOccupancy(currentPass, currentPass->fourHitFilterBox);
// //             if (testPassed == false) {
// //               if (LogSystem::Instance().isLevelEnabled(LogConfig::c_Debug, 50, PACKAGENAME()) == true) {
// //                 stringstream outputStream;
// //                 for (auto& entry : acceptedRejectedFilters) {
// //                   outputStream << FilterID().getFilterString(entry.first) << " " << entry.second << ", ";
// //                 }
// //                 B2DEBUG(50, "NbFINDERHighOccupancy: segment discarded! Outer/inner Segment: " <<  mainSecID << "/" << FullSecID(
// //                           currentFriendID) << "/" << FullSecID(currentInnerSeg->getInnerHit()->getSectorName()) << endl << "FilterResults: " <<
// //                         outputStream.str() << ", needed threshold: " << currentPass->activatedHighOccupancyNbFinderTests << "\n")
// //               }
// //
// //
// //               // Collector-TRACKLET: here 3-hit-cell-discarded update (nbFinder high occupancy
// //               if (m_PARAMdisplayCollector > 0) {
// //                 B2DEBUG(100, "VXDTF: Display: Module Collector importTracklet");
// //
// //                 // Filters vectors for update
// //                 std::vector<int> acceptedFilters;
// //                 std::vector<int> rejectedFilters;
// //
// //                 for (auto entry : acceptedRejectedFilters) {
// //                   B2DEBUG(100, "Collector-TRACKLET discarded High: acceptedRejected: " << entry.first << "; (T/F): " << entry.second);
// //                   if (entry.second == true) {
// //                     acceptedFilters.push_back(entry.first);
// //
// //                   } else {
// //                     rejectedFilters.push_back(entry.first);
// //                   }
// //
// //                 }
// //
// //                 std::vector<std::pair<int, unsigned int>> allSegments;
// //
// //                 // Connected Cells => vector for import
// //                 allSegments.push_back(make_pair(outerSegments[thisOuterSegment]->getCollectorID(), outerSegments[thisOuterSegment]->getState()));
// //                 allSegments.push_back(make_pair(currentInnerSeg->getCollectorID(), currentInnerSeg->getState()));
// //
// //                 //importTracklet(int passIndex, std::string diedAt, int diedId, std::vector<int> accepted, std::vector<int> rejected, const std::vector<std::pair<int, unsigned int>> assignedCellIDs)
// //                 m_collector.importTracklet(m_aktpassNumber, "", CollectorTFInfo::m_idAlive, acceptedFilters, rejectedFilters, allSegments);
// //
// //               }
// //
// //
// //
// //               continue;
// //             }
// //           }
// //         }
// //
// //         B2DEBUG(50, "neighbourFINDER: segment in " << mainSecID << " got friend in " << FullSecID(currentFriendID) <<
// //                 " with innermost hit in " << FullSecID(currentInnerSeg->getInnerHit()->getSectorName()) << " now (-> passed tests)" << "\n");
// //         outerSegments[thisOuterSegment]->addInnerNeighbour(currentInnerSeg);
// //         currentInnerSeg->addOuterNeighbour(outerSegments[thisOuterSegment]);
// //
// //         if (centerLayerIDNumber < currentPass->highestAllowedLayer) {
// //           B2DEBUG(50, "neighbourFINDER: current segment is not allowed being seed for CA")
// //           currentInnerSeg->setSeed(false);
// //         }
// //
// //
// //
// //         // Collector-TRACKLET: here 3-hit-cell accepted (nbFinder)
// //         if (m_PARAMdisplayCollector > 0) {
// //           B2DEBUG(10, "VXDTF: Display: Module Collector importTracklet");
// //
// //           // Filters vectors for update
// //           std::vector<int> acceptedFilters;
// //           std::vector<int> rejectedFilters;
// //
// //           for (auto entry : acceptedRejectedFilters) {
// //             B2DEBUG(10, "Collector-TRACKLET accepted acceptedRejected: " << entry.first << "; (T/F): " << entry.second);
// //             if (entry.second == true) {
// //               acceptedFilters.push_back(entry.first);
// //
// //             } else {
// //               rejectedFilters.push_back(entry.first);
// //             }
// //
// //           }
// //
// //           std::vector<std::pair<int, unsigned int>> allSegments;
// //
// //           // Connected Cells => vector for import
// //           allSegments.push_back(make_pair(outerSegments[thisOuterSegment]->getCollectorID(), outerSegments[thisOuterSegment]->getState()));
// //           allSegments.push_back(make_pair(currentInnerSeg->getCollectorID(), currentInnerSeg->getState()));
// //
// //           //importTracklet(int passIndex, std::string diedAt, int diedId, std::vector<int> accepted, std::vector<int> rejected, const std::vector<std::pair<int, unsigned int>> assignedCellIDs)
// //           m_collector.importTracklet(m_aktpassNumber, "", CollectorTFInfo::m_idAlive, acceptedFilters, rejectedFilters, allSegments);
// //
// //         }
// //
// //
// //
// //       } // iterating through inner segments
// //     } // iterating through outer segments
// //   } // iterating through all active sectors - friendFinder
// //
// //   //filtering lost segments (those without neighbours left):
// //   ActiveSegmentsOfEvent newActiveList;
// //   for (VXDSegmentCell* currentSeg : currentPass->activeCellList) {
// //     bool isStillAlive = currentSeg->dieIfNoNeighbours();
// //     if (isStillAlive == false) {
// //
// //       // Collector Cell died at NBFinder-lost
// //       if (m_PARAMdisplayCollector > 0) {
// //         B2DEBUG(10, "displayCollector: nbFinderLost !!! " << currentSeg->getCollectorID());
// //
// //         m_collector.updateCell(currentSeg->getCollectorID(), CollectorTFInfo::m_nameNbFinder, CollectorTFInfo::m_idNbFinder, vector<int>(), {FilterID::nbFinderLost},
// //                                -1, -2, currentSeg->getState(), vector<int>());
// //       }
// //
// //       NFdiscardedSegmentsCounter++;
// //     } else {
// //       currentSeg->copyNeighbourList(); /// IMPORTANT, without this step, no TCs can be found since all neighbours of each cell are erased from current list
// //       newActiveList.push_back(currentSeg);
// //       if (currentSeg->sizeOfInnerNeighbours() != 0) {
// //         currentSeg->increaseState();
// //       }
// //       if (currentSeg->isSeed() == true) { activatedSeedsCounter++; }
// //
// //       // Collector Cell ok at NBFinder-lost
// //       if (m_PARAMdisplayCollector > 0) {
// //
// //         m_collector.updateCell(currentSeg->getCollectorID(), "", CollectorTFInfo::m_idAlive, {FilterID::nbFinderLost}, vector<int>(), -1,
// //                                -1, currentSeg->getState(), vector<int>());
// //       }
// //
// //
// //     }
// //
// //     // Collector Cell update with Filters
// //
// //     // Collector-TRACKLET: TO DELETE
// //     /*
// //     if (m_PARAMdisplayCollector > 0) {
// //       B2DEBUG(10, "VXDTF: Display: Module Collector updateCell");
// //
// //       // Filters vectors for update
// //       std::vector<int> acceptedFilters;
// //       std::vector<int> rejectedFilters;
// //
// //       for (auto entry : acceptedRejectedFilters) {
// //         B2DEBUG(10, "acceptedRejected: " << entry.first << "; (T/F): " << entry.second);
// //         if (entry.second == true) {
// //           acceptedFilters.push_back(entry.first);
// //
// //         } else {
// //           rejectedFilters.push_back(entry.first);
// //         }
// //
// //       }
// //
// //       m_collector.updateCell(currentSeg->getCollectorID(), "", CollectorTFInfo::m_idAlive, acceptedFilters, rejectedFilters, -1, -1, currentSeg->getState(), vector<int>());
// //
// //     }
// //     */
// //
// //
// //
// //   }
// //   currentPass->activeCellList = newActiveList;
// //
// //   B2DEBUG(10, "neighbourFINDER-end: " << currentPass->activeCellList.size() << "/" << currentPass->totalCellVector.size() <<
// //           " cells in activeList/totalVector, " << NFdiscardedSegmentsCounter << " cells discarded, " << activatedSeedsCounter <<
// //           " cells set as seeds")
// //   return NFdiscardedSegmentsCounter;
// // }
// //
// //
// //
// // bool TFRedesignModule::NbFinderHighOccupancy(PassData* currentPass, TcFourHitFilters& fourHitFilterBox)
// // {
// //   /** REDESIGNCOMMENT NBFINDERHIGHOCCUPANCY 1:
// //    * * short:
// //    *
// //    ** long (+personal comments):
// //    *
// //    ** dependency of module parameters (global):
// //    * m_PARAMDebugMode,
// //    *
// //    ** dependency of global in-module variables:
// //    *
// //    ** dependency of global stuff just because of B2XX-output or debugging only:
// //    * m_PARAMDebugMode,
// //    *
// //    ** in-module-function-calls:
// //    */
// //   int simpleSegmentQI = 0;
// //   vector< pair < int, bool > >
// //   acceptedRejectedFilters; // collects for each filter whether it accepted or rejected the current segment. .first is filterID as int, .second is true if accepted, else false
// //   bool accepted;
// //
// //   if (currentPass->deltaDistanceHighOccupancy2IP.first == true) { // max only
// //     try {
// //       accepted = fourHitFilterBox.checkDeltaDistCircleCenter(FilterID::deltaDistanceHighOccupancy2IP);
// //     } catch (FilterExceptions::Straight_Line& anException) {
// //       B2DEBUG(1, "Exception caught: " << FilterID::deltaDistanceHighOccupancy2IP << " failed with exception: " << anException.what() <<
// //               " test-result is set negative...")
// //       accepted = false;
// //     } catch (FilterExceptions::Circle_too_small& anException) {
// //       B2DEBUG(1, "Exception caught: " << FilterID::deltaDistanceHighOccupancy2IP << " failed with exception: " << anException.what() <<
// //               " test-result is set negative...")
// //       accepted = false;
// //     }
// //
// //     if (accepted == true) {
// //       simpleSegmentQI++;
// //       B2DEBUG(150, " deltaDistanceHighOccupancy2IP: segment approved!")
// //       acceptedRejectedFilters.push_back({FilterID::deltaDistanceHighOccupancy2IP, true});
// //     } else {
// //       B2DEBUG(150, " deltaDistanceHighOccupancy2IP: segment discarded!")
// //       acceptedRejectedFilters.push_back({FilterID::deltaDistanceHighOccupancy2IP, false});
// //       if (m_PARAMDebugMode == true) {
// //         pair <double, double> cutoffs = fourHitFilterBox.getCutoffs(FilterID::deltaDistanceHighOccupancy2IP);
// //         B2WARNING("deltaDistanceHighOccupancy2IP - minCutoff: " << cutoffs.first << ", calcValue: " <<
// //                   fourHitFilterBox.deltaDistCircleCenter() << ", maxCutoff: " << cutoffs.second)
// //       }
// //     } // else segment not approved
// //   } else { B2DEBUG(175, " deltaDistanceHighOccupancy2IP is not activated for pass: " << currentPass->sectorSetup << "!") }
// //
// //   if (currentPass->deltaPtHighOccupancy.first == true) { // min & max!
// //     try {
// //       accepted = fourHitFilterBox.checkDeltapT(FilterID::deltapTHighOccupancy);
// //     } catch (FilterExceptions::Straight_Line& anException) {
// //       B2DEBUG(1, "Exception caught: " << FilterID::deltapTHighOccupancy << " failed with exception: " << anException.what() <<
// //               " test-result is set negative...")
// //       accepted = false;
// //     } catch (FilterExceptions::Circle_too_small& anException) {
// //       B2DEBUG(1, "Exception caught: " << FilterID::deltapTHighOccupancy << " failed with exception: " << anException.what() <<
// //               " test-result is set negative...")
// //       accepted = false;
// //     }
// //
// //     if (accepted == true) {
// //       simpleSegmentQI++;
// //       B2DEBUG(150, " deltaPtHighOccupancy: segment approved!")
// //       acceptedRejectedFilters.push_back({FilterID::deltapTHighOccupancy, true});
// //     } else {
// //       B2DEBUG(150, " deltaPtHighOccupancy: segment discarded!")
// //       acceptedRejectedFilters.push_back({FilterID::deltapTHighOccupancy, false});
// //       if (m_PARAMDebugMode == true) {
// //         pair <double, double> cutoffs = fourHitFilterBox.getCutoffs(FilterID::deltapTHighOccupancy);
// //         B2WARNING("deltaPtHighOccupancy - minCutoff: " << cutoffs.first << ", calcValue: " << fourHitFilterBox.deltapT() << ", maxCutoff: "
// //                   << cutoffs.second)
// //       }
// //     } // else segment not approved
// //   } else { B2DEBUG(175, " deltaPTHighOccupancy is not activated for pass: " << currentPass->sectorSetup << "!") }
// //
// //   if (simpleSegmentQI < currentPass->activatedHighOccupancyNbFinderTests) {
// //     if (LogSystem::Instance().isLevelEnabled(LogConfig::c_Debug, 50, PACKAGENAME()) == true) {
// //       stringstream outputStream;
// //       for (auto& entry : acceptedRejectedFilters) {
// //         outputStream << FilterID().getFilterString(entry.first) << " " << entry.second << ", ";
// //       }
// //       B2DEBUG(50, "nbFinder- HIOC: segment discarded! simpleSegmentQI = " << simpleSegmentQI << ", threshold: " <<
// //               currentPass->activatedHighOccupancyNbFinderTests << endl << "FilterResults: " << outputStream.str()  << endl)
// //     }
// //     return false;
// //   } else {
// //     B2DEBUG(100, "nbFinder- HIOC: segment passed tests)");
// //     return true;
// //   }
// // }
// //
// //
// //
// // /** ***** cellular automaton ***** **/
// // /// uses attribute "state" to find rows of compatible neighbours. State indicates length of row. This allows fast Track candidate collection
// // int TFRedesignModule::cellularAutomaton(PassData* currentPass)
// // {
// //   /** REDESIGNCOMMENT CELLULARAUTOMATON 1:
// //    * * short:
// //    *
// //    ** long (+personal comments):
// //    *
// //    ** dependency of module parameters (global):
// //    * m_PARAMdisplayCollector,
// //    *
// //    ** dependency of global in-module variables:
// //    * m_collector, m_TESTERbrokenCaRound
// //    *
// //    ** dependency of global stuff just because of B2XX-output or debugging only:
// //    * m_PARAMdisplayCollector, m_collector, m_TESTERbrokenCaRound,
// //    *
// //    ** in-module-function-calls:
// //    */
// //   int activeCells = 1; // is set 1 because of following while loop.
// //   int deadCells = 0;
// //   int caRound = 0;
// //   int highestCellState = 0;
// //   int goodNeighbours, countedSegments;
// //
// //   while (activeCells != 0) {
// //     activeCells = 0;
// //     caRound++;
// //
// //     /// CAstep:
// //     for (VXDSegmentCell* currentSeg : currentPass->activeCellList) {
// //       if (currentSeg->isActivated() == false) { continue; }
// //
// //       std::vector<int> idsNeighbours;
// //
// //       goodNeighbours = 0;
// //       list<VXDSegmentCell*>* currentNeighbourList = currentSeg->getInnerNeighbours();
// //       B2DEBUG(50, "CAstep: cell with outer/inner hit at sectors: " << currentSeg->getOuterHit()->getSectorString() << "/" <<
// //               currentSeg->getInnerHit()->getSectorString() << " has inner/outer friend at " << currentSeg->sizeOfInnerNeighbours() << "/" <<
// //               currentSeg->sizeOfOuterNeighbours() << ", only innerNeighbours count!")
// //
// //       list<VXDSegmentCell*>::iterator currentNeighbour = currentNeighbourList->begin();
// //       while (currentNeighbour != currentNeighbourList->end()) {
// //         //TODO probably outdated: can't deal with cells of special shape (e.g. two-layer cells) in some uncommon scenario yet -> search for test scenario
// //         if (currentSeg->getState() == (*currentNeighbour)->getState()) {
// //
// //           // Neighbour to Collector
// //           if (m_PARAMdisplayCollector > 0) {
// //             idsNeighbours.push_back((*currentNeighbour)->getCollectorID());
// //             B2DEBUG(100, "neighboring cell found with CollectorID: " << (*currentNeighbour)->getCollectorID());
// //           }
// //
// //
// //           goodNeighbours++;
// //           ++currentNeighbour;
// //           B2DEBUG(100, "neighboring cell found!")
// //
// //         } else {
// //           currentNeighbour = currentSeg->eraseInnerNeighbour(currentNeighbour); // includes currentNeighbour++;
// //         }
// //       }
// //       if (goodNeighbours != 0) {
// //         currentSeg->allowStateUpgrade(true);
// //         activeCells++;
// //         B2DEBUG(50, "CAstep: accepted cell found!")
// //
// //         // Collector Cell OK CA
// //         if (m_PARAMdisplayCollector > 0) {
// //           m_collector.updateCell(currentSeg->getCollectorID(), "", CollectorTFInfo::m_idAlive, {FilterID::cellularAutomaton}, vector<int>(),
// //                                  -1, -1, currentSeg->getState(), idsNeighbours);
// //         }
// //
// //       } else {
// //         currentSeg->setActivationState(false);
// //         deadCells++;
// //
// //         // Collector Cell died at CA
// //         if (m_PARAMdisplayCollector > 0) {
// //           m_collector.updateCell(currentSeg->getCollectorID(), "", CollectorTFInfo::m_idAlive, vector<int>(), {FilterID::cellularAutomaton},
// //                                  -1, -1, currentSeg->getState(), vector<int>());
// //         }
// //
// //
// //
// //       }
// //     }//CAStep
// //     B2DEBUG(10, "CA: before update-step: at round " << caRound << ", there are " << activeCells << " cells still alive, counted " <<
// //             deadCells << " dead cells so far")
// //
// //     /// Updatestep:
// //     for (VXDSegmentCell* currentSeg : currentPass->activeCellList) {
// //       B2DEBUG(50, "Updatestep: cell with outer/inner hit at sectors: " << currentSeg->getOuterHit()->getSectorString() << "/" <<
// //               currentSeg->getInnerHit()->getSectorString() << " has inner/outer friend at " << currentSeg->sizeOfInnerNeighbours() << "/" <<
// //               currentSeg->sizeOfOuterNeighbours() << "!")
// //       if (currentSeg->isUpgradeAllowed() == false) { continue; }
// //
// //       currentSeg->allowStateUpgrade(false);
// //       currentSeg->increaseState();
// //       B2DEBUG(50, "good cell  with outer/inner hit at sectors: " << currentSeg->getOuterHit()->getSectorString() << "/" <<
// //               currentSeg->getInnerHit()->getSectorString() << " upgraded!")
// //       if (currentSeg->getState() > highestCellState) { highestCellState = currentSeg->getState(); }
// //     }
// //
// //     B2DEBUG(10, "CA: " << caRound << ". round - " << activeCells << " living cells remaining. Highest state: " << highestCellState);
// //
// //     if (caRound > 15) { /// WARNING: hardcoded value
// //       B2ERROR("event " << m_eventCounter << ": VXDTF-CA: more than 15 ca rounds! " << activeCells << " living cells remaining");
// //       stringstream currentSectors;
// //       for (VXDSector* aSector : currentPass->sectorVector) {
// //         currentSectors << FullSecID(aSector->getSecID()) << " ";
// //       }
// //       B2ERROR("event " << m_eventCounter << ": VXDTF-CA: activated sectors: " << currentSectors.str())
// //       m_TESTERbrokenCaRound++;
// //       caRound = -1;
// //       break;
// //     }
// //   }
// //
// //
// //
// //   if (LogSystem::Instance().isLevelEnabled(LogConfig::c_Debug, 10, PACKAGENAME()) == true) {
// //     countedSegments = 0;
// //     for (VXDSegmentCell* currentSeg : currentPass->activeCellList) {
// //       B2DEBUG(100, "Post CA - Current state of cell: " << currentSeg->getState() << " with outer/inner hit at sectors: " <<
// //               currentSeg->getOuterHit()->getSectorString() << "/" << currentSeg->getInnerHit()->getSectorString());
// //       if (currentSeg->sizeOfInnerNeighbours() == 0 and currentSeg->sizeOfOuterNeighbours() == 0) { continue; }
// //       countedSegments++;
// //     }
// //     B2DEBUG(10, "CA - " << countedSegments << " segments have at least one friend");
// //   }
// //
// //   return caRound;
// // }
// //
// //
// //
// // /** ***** Track Candidate Collector (TCC) ***** **/
// // /// uses attribute "state" to find rows of compatible neighbours. State indicates length of row. This allows fast Track candidate collection
// // void TFRedesignModule::tcCollector(PassData* currentPass)
// // {
// //   /** REDESIGNCOMMENT TCCOLLECTOR 1:
// //    * * short:
// //    *
// //    ** long (+personal comments):
// //    *
// //    ** dependency of module parameters (global):
// //    * m_PARAMdisplayCollector,
// //    *
// //    ** dependency of global in-module variables:
// //    * m_collector, m_aktpassNumber, m_allTCsOfEvent,
// //    * m_TESTERcountTotalTCsAfterTCC
// //    *
// //    ** dependency of global stuff just because of B2XX-output or debugging only:
// //    * m_PARAMdisplayCollector, m_collector, m_aktpassNumber,
// //    * m_TESTERcountTotalTCsAfterTCC
// //    *
// //    ** in-module-function-calls:
// //    * findTCs(currentPass->tcVector, pTC, currentPass->highestAllowedLayer)
// //    */
// //   short int mayorLayerID/*, nSegmentsInSector*/;
// //   int findTCsCounter = 0;
// //   int tccMinState = currentPass->minState;
// //   for (VXDSector* aSector : currentPass->sectorVector)  {
// //     B2DEBUG(100, "TCC - entering sector " << FullSecID(aSector->getSecID()));
// //
// //     mayorLayerID = FullSecID(aSector->getSecID()).getLayerID();
// //     if (mayorLayerID < currentPass->minLayer) { continue; }
// //
// //     vector<VXDSegmentCell*> segmentsOfSector = aSector->getInnerSegmentCells(); // loading segments of sector
// //
// //     B2DEBUG(50, "TCC - sector " << FullSecID(aSector->getSecID()) << " has got " << segmentsOfSector.size() << " segments in its area");
// //
// //     for (VXDSegmentCell* currentSegment : segmentsOfSector) {
// //       if (currentSegment->isSeed() == false) { B2DEBUG(100, "current segment is no seed!"); continue; }
// //       if (currentSegment->getState() < tccMinState) { B2DEBUG(100, "current segment has no sufficent cellstate (is/threshold:" << currentSegment->getState() << "/" << tccMinState << ")..."); continue; }
// //
// //       VXDTFTrackCandidate* pTC = new VXDTFTrackCandidate();
// //       pTC->addSegments(currentSegment);
// //       pTC->addHits(currentSegment->getOuterHit());
// //       pTC->addHits(currentSegment->getInnerHit());
// //
// //       findTCs(currentPass->tcVector, pTC, currentPass->highestAllowedLayer);
// //       findTCsCounter++;
// //
// //       // Collector TC Import
// //       if (m_PARAMdisplayCollector > 0) {
// //
// //         std::vector<std::pair<int, unsigned int>> allSegments;
// //
// //         // Connected Cells => vector for import
// //         for (VXDSegmentCell* tf_currentSegment :  pTC->getSegments()) {
// //           allSegments.push_back(make_pair(tf_currentSegment->getCollectorID(), tf_currentSegment->getState()));
// //         }
// //
// //         int tcId = m_collector.importTC(m_aktpassNumber, "", CollectorTFInfo::m_idAlive, vector<int>(), vector<int>(), allSegments);
// //
// //         // Connect Collector TC <=> TC
// //         pTC->setCollectorID(tcId);
// //
// //       }
// //
// //
// //     }
// //   }
// //   int numTCsafterTCC = currentPass->tcVector.size(); // total number of tc's
// //
// //   m_allTCsOfEvent.insert(m_allTCsOfEvent.end(), currentPass->tcVector.begin(), currentPass->tcVector.end());
// //
// //   B2DEBUG(5, "findTCs activated " << findTCsCounter << " times, resulting in " << numTCsafterTCC << " track candidates")
// //
// //   if (LogSystem::Instance().isLevelEnabled(LogConfig::c_Debug, 10, PACKAGENAME()) == true) {
// //
// //     for (VXDTFTrackCandidate* aTC : currentPass->tcVector) {
// //       if (aTC->getCondition() == false) continue;
// //       stringstream hitInfoPacks;
// //       hitInfoPacks << "living TC with ID " << aTC->getTrackNumber() << " got hits (secID/clusterIndexU/V/UV): " << endl;
// //       for (const VXDTFHit* aHit : aTC->getHits()) {
// //         hitInfoPacks << aHit->getSectorString() << "/" << aHit->getClusterIndexU() << "/" << aHit->getClusterIndexV() << "/" <<
// //                      aHit->getClusterIndexUV() << " ";
// //       }
// //       B2DEBUG(10, hitInfoPacks.str())
// //     }
// //   }
// //
// //   m_TESTERcountTotalTCsAfterTCC += numTCsafterTCC;
// // }
// //
// //
// //
// // /** ***** Track Candidate Filter (tcFilter) ***** **/
// // int TFRedesignModule::tcFilter(PassData* currentPass, int passNumber)
// // {
// //   /** REDESIGNCOMMENT TCFILTER 1:
// //    * * short:
// //    *
// //    ** long (+personal comments):
// //    *
// //    ** dependency of module parameters (global):
// //    * m_PARAMdisplayCollector, m_PARAMDebugMode,
// //    *
// //    ** dependency of global in-module variables:
// //    * m_collector, m_TESTERtriggeredZigZagXY, m_TESTERtriggeredCircleFit,
// //    * m_TESTERtriggeredDpT, m_TESTERtriggeredDD2IP, m_TESTERtriggeredZigZagRZ,
// //    * m_TESTERapprovedByTCC, m_TESTERcountTotalTCsAfterTCCFilter
// //    *
// //    ** dependency of global stuff just because of B2XX-output or debugging only:
// //    * m_PARAMdisplayCollector, m_collector, m_TESTERtriggeredZigZagXY,
// //    * m_TESTERtriggeredCircleFit, m_TESTERtriggeredDpT, m_PARAMDebugMode,
// //    * m_TESTERtriggeredDD2IP, m_TESTERtriggeredZigZagRZ, m_TESTERapprovedByTCC,
// //    * m_TESTERcountTotalTCsAfterTCCFilter
// //    *
// //    ** in-module-function-calls:
// //    * doTheCircleFit(currentPass, (*currentTC), nCurrentHits, tcCtr)
// //    */
// //
// //   TCsOfEvent::iterator currentTC;
// //   TVector3* hitA, *hitB, *hitC, *hitD;
// //   TCsOfEvent tempTCList = currentPass->tcVector;
// //   int numTCsafterTCC = tempTCList.size();
// //   vector<TCsOfEvent::iterator> goodTCIndices;
// //   goodTCIndices.clear();
// //   int tcCtr = 0;
// //   B2DEBUG(10, "TC-filter: pass " << passNumber << " has got " << currentPass->tcVector.size() << " tcs")
// //   vector< pair<int, string> >
// //   killedList; // will contain the number of the TC (.first) and the type of filter (.second) which killed the TC
// //   for (currentTC = currentPass->tcVector.begin(); currentTC != currentPass->tcVector.end();
// //        ++currentTC) { // need iterators for later use
// //     const vector<VXDTFHit*>& currentHits = (*currentTC)->getHits(); /// IMPORTANT: currentHits[0] is outermost hit!
// //     int nCurrentHits = currentHits.size();
// //
// //     if (nCurrentHits == 3) {  /// in this case, dPt and zigzag filtering does not make sense
// //       (*currentTC)->removeVirtualHit();
// //       nCurrentHits = (*currentTC)->size();
// //       if (nCurrentHits < 3) {
// //         (*currentTC)->setCondition(false);
// //
// //         // Collector TC Update (tcFinderCurr)
// //         if (m_PARAMdisplayCollector > 0) {
// //           m_collector.updateTC((*currentTC)->getCollectorID(), CollectorTFInfo::m_nameTCC, CollectorTFInfo::m_idTCC, vector<int>(), {FilterID::tcFinderCurr});
// //         }
// //
// //
// //         B2DEBUG(20, " tc " << tcCtr << " got " << nCurrentHits << " hits and therefore will be deleted");
// //         if (LogSystem::Instance().isLevelEnabled(LogConfig::c_Debug, 1, PACKAGENAME()) == true) { killedList.push_back(make_pair(tcCtr, "not enough hits after removing virtual hit")); }
// //         continue;
// //       } else {
// //         goodTCIndices.push_back(currentTC); // we can not filter it, therefore we have to accept it
// //         B2DEBUG(20, " tc " << tcCtr << " got " << nCurrentHits << " hits and therefore won't be checked by TCC");
// //
// //         // Collector TC Update (tcFinderCurr)
// //         if (m_PARAMdisplayCollector > 0) {
// //           m_collector.updateTC((*currentTC)->getCollectorID(), "", CollectorTFInfo::m_idAlive, {FilterID::tcFinderCurr}, vector<int>());
// //         }
// //
// //       }
// //       tcCtr++;
// //       continue; // no further testing possible
// //     } else if (nCurrentHits < 3) {
// //       B2DEBUG(20, " tc " << tcCtr << " got " << nCurrentHits << " hits and therefore will be deleted");
// //       if (LogSystem::Instance().isLevelEnabled(LogConfig::c_Debug, 1, PACKAGENAME()) == true) { killedList.push_back(make_pair(tcCtr, "not enough hits")); }
// //       tcCtr++;
// //       (*currentTC)->setCondition(false);
// //
// //       // Collector TC Update (tcFinderCurr)
// //       if (m_PARAMdisplayCollector > 0) {
// //         m_collector.updateTC((*currentTC)->getCollectorID(), CollectorTFInfo::m_nameTCC, CollectorTFInfo::m_idTCC, vector<int>(), {FilterID::tcFinderCurr});
// //       }
// //
// //
// //       continue;
// //     }
// //
// //     vector<PositionInfo*> currentHitPositions;
// //     currentHitPositions.reserve(nCurrentHits);
// //     if (LogSystem::Instance().isLevelEnabled(LogConfig::c_Debug, 20, PACKAGENAME()) == true) {
// //       stringstream secNameOutput;
// //       secNameOutput << endl << " tc " << tcCtr << " got " << nCurrentHits << " hits and the following secIDs: ";
// //       for (VXDTFHit* currentHit : currentHits) {
// //         currentHitPositions.push_back(currentHit->getPositionInfo());
// //         string aSecName = FullSecID(currentHit->getSectorName()).getFullSecString();
// //         secNameOutput << aSecName << " ";
// //       } // used for output
// //       B2DEBUG(20, " " << secNameOutput.str() << " and " <<  nCurrentHits << " hits");
// //     } else {
// //       for (VXDTFHit* currentHit : currentHits) {
// //         currentHitPositions.push_back(currentHit->getPositionInfo());
// //       }
// //     }
// //
// //     // feeding trackletFilterbox with hits:
// //     currentPass->trackletFilterBox.resetValues(&currentHitPositions);
// //     bool isZiggZagging;
// //
// //     if (currentPass->zigzagXY.first == true) {
// //       isZiggZagging = currentPass->trackletFilterBox.ziggZaggXY();
// //       if (isZiggZagging == true) {
// //         B2DEBUG(20, "TCC filter: tc " << tcCtr << " rejected by ziggZaggXY! ");
// //         if (LogSystem::Instance().isLevelEnabled(LogConfig::c_Debug, 1, PACKAGENAME()) == true) { killedList.push_back(make_pair(tcCtr, "ziggZaggXY")); }
// //         m_TESTERtriggeredZigZagXY++; tcCtr++;
// //         (*currentTC)->setCondition(false);
// //
// //         // Collector TC Update (updateTC)
// //         if (m_PARAMdisplayCollector > 0) {
// //           m_collector.updateTC((*currentTC)->getCollectorID(), CollectorTFInfo::m_nameTCC, CollectorTFInfo::m_idTCC, vector<int>(), {FilterID::ziggZaggXY});
// //         }
// //
// //         continue;
// //       }
// //       B2DEBUG(20, " TCC filter ziggZaggXY approved TC " << tcCtr);
// //
// //       // Collector TC Update (ziggZaggXY)
// //       if (m_PARAMdisplayCollector > 0) {
// //         m_collector.updateTC((*currentTC)->getCollectorID(), "", CollectorTFInfo::m_idAlive,  {FilterID::ziggZaggXY}, vector<int>());
// //       }
// //
// //     }
// //
// //     if (m_calcQiType == 2 or currentPass->circleFit.first == true) {
// //       bool succeeded = false;
// //       try {
// //         succeeded = doTheCircleFit(currentPass, (*currentTC), nCurrentHits, tcCtr);
// //       } catch (FilterExceptions::Calculating_Curvature_Failed& anException) {
// //         succeeded = false;
// //         B2WARNING("tcFilter:doTheCircleFit failed, reason: " << anException.what())
// //       } catch (FilterExceptions::Center_Is_Origin& anException) {
// //         succeeded = false;
// //         B2WARNING("tcFilter:doTheCircleFit failed, reason: " << anException.what())
// //       }  catch (FilterExceptions::Circle_too_small& anException) {
// //         succeeded = false;
// //         B2WARNING("tcFilter:doTheCircleFit failed, reason: " << anException.what())
// //       }
// //       B2DEBUG(150, " TCFilter, circleFit succeeded: " << succeeded)
// //       if (succeeded == false) {
// //         if (LogSystem::Instance().isLevelEnabled(LogConfig::c_Debug, 1, PACKAGENAME()) == true) { killedList.push_back(make_pair(tcCtr, "circleFit")); }
// //         m_TESTERtriggeredCircleFit++; tcCtr++;
// //         (*currentTC)->setCondition(false);
// //
// //         // Collector TC Update (circlefit)
// //         if (m_PARAMdisplayCollector > 0) {
// //           m_collector.updateTC((*currentTC)->getCollectorID(), CollectorTFInfo::m_nameTCC, CollectorTFInfo::m_idTCC, vector<int>(), {FilterID::circlefit});
// //         }
// //
// //         continue;
// //       }
// //
// //       // Collector TC Update (circlefit)
// //       if (m_PARAMdisplayCollector > 0) {
// //         m_collector.updateTC((*currentTC)->getCollectorID(), "", CollectorTFInfo::m_idAlive,  {FilterID::circlefit}, vector<int>());
// //       }
// //
// //     }
// //
// //     int a = 0, b = 1, c = 2, d = 3;
// //     bool abortTC = false, accepted;
// //     for (; d <  nCurrentHits;) {
// //       hitA = currentHits[a]->getHitCoordinates();
// //       VXDSector* thisSector = currentPass->sectorMap.find(currentHits[a]->getSectorName())->second;
// //       hitB = currentHits[b]->getHitCoordinates();
// //       unsigned int friendID = currentHits[b]->getSectorName();
// //       hitC = currentHits[c]->getHitCoordinates();
// //       hitD = currentHits[d]->getHitCoordinates();
// //
// //       currentPass->fourHitFilterBox.resetValues(*hitA, *hitB, *hitC, *hitD, thisSector, friendID);
// //
// //       if (currentPass->deltaPt.first == true) {
// //         accepted = currentPass->fourHitFilterBox.checkDeltapT(FilterID::deltapT);
// //         if (accepted == false) {
// //           B2DEBUG(20, "TCC filter: tc " << tcCtr << " rejected by deltaPt! ");
// //           m_TESTERtriggeredDpT++;
// //           (*currentTC)->setCondition(false);
// //
// //           // Collector TC Update (deltapT)
// //           if (m_PARAMdisplayCollector > 0) {
// //             m_collector.updateTC((*currentTC)->getCollectorID(), CollectorTFInfo::m_nameTCC, CollectorTFInfo::m_idTCC, vector<int>(), {FilterID::deltapT});
// //           }
// //
// //
// //           if (m_PARAMDebugMode == true) {
// //             pair <double, double> cutoffs = currentPass->fourHitFilterBox.getCutoffs(FilterID::deltapT);
// //             B2WARNING("deltaPt - minCutoff: " << cutoffs.first << ", calcValue: " << currentPass->fourHitFilterBox.deltapT() << ", maxCutoff: "
// //                       << cutoffs.second)
// //           }
// //           abortTC = true;
// //           if (LogSystem::Instance().isLevelEnabled(LogConfig::c_Debug, 1, PACKAGENAME()) == true) {
// //             stringstream reason;
// //             reason << " bad deltaPt: " << currentPass->fourHitFilterBox.deltapT();
// //             killedList.push_back(make_pair(tcCtr, reason.str()));
// //           }
// //           break;
// //         }
// //         B2DEBUG(20, " TCC filter deltaPt approved TC " << tcCtr);
// //
// //         // Collector TC Update (deltapT)
// //         if (m_PARAMdisplayCollector > 0) {
// //           m_collector.updateTC((*currentTC)->getCollectorID(), "", CollectorTFInfo::m_idAlive, {FilterID::deltapT}, vector<int>());
// //         }
// //
// //
// //       }
// //       if (currentPass->deltaDistance2IP.first == true) {
// //         accepted = currentPass->fourHitFilterBox.checkDeltaDistCircleCenter(FilterID::deltaDistance2IP);
// //         if (accepted == false) {
// //           B2DEBUG(20, "TCC filter: tc " << tcCtr << " rejected by deltaDistance2IP! ");
// //           m_TESTERtriggeredDD2IP++;
// //           (*currentTC)->setCondition(false);
// //
// //           // Collector TC Update (deltaDistance2IP)
// //           if (m_PARAMdisplayCollector > 0) {
// //             m_collector.updateTC((*currentTC)->getCollectorID(), CollectorTFInfo::m_nameTCC, CollectorTFInfo::m_idTCC, vector<int>(), {FilterID::deltaDistance2IP});
// //           }
// //
// //
// //           if (m_PARAMDebugMode == true) {
// //             pair <double, double> cutoffs = currentPass->fourHitFilterBox.getCutoffs(FilterID::deltaDistance2IP);
// //             B2WARNING("deltaDistance2IP - minCutoff: " << cutoffs.first << ", calcValue: " <<
// //                       currentPass->fourHitFilterBox.deltaDistCircleCenter() << ", maxCutoff: " << cutoffs.second)
// //           }
// //           if (LogSystem::Instance().isLevelEnabled(LogConfig::c_Debug, 1, PACKAGENAME()) == true) {
// //             stringstream reason;
// //             reason << " bad deltaDistance2IP: " << currentPass->fourHitFilterBox.deltaDistCircleCenter();
// //             killedList.push_back(make_pair(tcCtr, reason.str()));
// //           }
// //           abortTC = true;
// //           break;
// //         }
// //         B2DEBUG(20, " TCC filter deltaDistance2IP approved TC " << tcCtr);
// //
// //         // Collector TC Update (deltaDistance2IP)
// //         if (m_PARAMdisplayCollector > 0) {
// //           m_collector.updateTC((*currentTC)->getCollectorID(), "", CollectorTFInfo::m_idAlive, {FilterID::deltaDistance2IP}, vector<int>());
// //         }
// //
// //       }
// //
// //       /// The following tests are debug-tests WARNING uncomment only if needed!:
// // //       if (currentPass->alwaysTrue4Hit.first == true) { // min & max!
// // //         accepted = currentPass->fourHitFilterBox.checkAlwaysTrue4Hit(FilterID::alwaysTrue4Hit);
// // //         if (accepted == true) {
// // //           B2DEBUG(150, " TCC filter alwaysTrue4Hit: segment approved! TC: " << tcCtr)
// // //
// // //           // Collector TC Update (alwaysTrue4Hit)
// // //           if (m_PARAMdisplayCollector > 0) {
// // //             m_collector.updateTC((*currentTC)->getCollectorID(), "", CollectorTFInfo::m_idAlive, {FilterID::alwaysTrue4Hit}, vector<int>());
// // //           }
// // //         } else {
// // //           B2DEBUG(150, " TCC filter alwaysTrue4Hit: segment discarded! TC: " << tcCtr)
// // //           break;
// // //         } // else segment not approved
// // //       } else { B2DEBUG(175, " TCC filter alwaysTrue4Hit is not activated for pass: " << currentPass->sectorSetup << "!") }
// // //       if (currentPass->alwaysFalse4Hit.first == true) { // min & max!
// // //         accepted = currentPass->fourHitFilterBox.checkAlwaysFalse4Hit(FilterID::alwaysFalse4Hit);
// // //         if (accepted == true) {
// // //           B2DEBUG(150, " TCC filter alwaysFalse4Hit: segment approved! TC: " << tcCtr)
// // //
// // //           // Collector TC Update (alwaysFalse4Hit)
// // //           if (m_PARAMdisplayCollector > 0) {
// // //             m_collector.updateTC((*currentTC)->getCollectorID(), "", CollectorTFInfo::m_idAlive, {FilterID::alwaysFalse4Hit}, vector<int>());
// // //           }
// // //         } else {
// // //           B2DEBUG(150, " TCC filter alwaysFalse4Hit: segment discarded! TC: " << tcCtr)
// // //           break;
// // //         } // else segment not approved
// // //       } else { B2DEBUG(175, " TCC filter alwaysFalse4Hit is not activated for pass: " << currentPass->sectorSetup << "!") }
// // //       if (currentPass->random4Hit.first == true) { // min & max!
// // //         accepted = currentPass->fourHitFilterBox.checkRandom4Hit(FilterID::random4Hit);
// // //         if (accepted == true) {
// // //           B2DEBUG(150, " TCC filter random4Hit: segment approved! TC: " << tcCtr)
// // //
// // //           // Collector TC Update (random4Hit)
// // //           if (m_PARAMdisplayCollector > 0) {
// // //             m_collector.updateTC((*currentTC)->getCollectorID(), "", CollectorTFInfo::m_idAlive, {FilterID::random4Hit}, vector<int>());
// // //           }
// // //         } else {
// // //           B2DEBUG(150, " TCC filter random4Hit: segment discarded! TC: " << tcCtr)
// // //           break;
// // //         } // else segment not approved
// // //       } else { B2DEBUG(175, " TCC filter random4Hit is not activated for pass: " << currentPass->sectorSetup << "!") }
// //       a++; b++; c++; d++;
// //     }
// //     if (abortTC == true) { tcCtr++; continue; }
// //
// //     // now we can delete the virtual hit for tests done without it (e.g. tests dealing with the r-z-plane, where the IP is not well defined beforehand)
// //     (*currentTC)->removeVirtualHit();
// //
// //     if (currentPass->zigzagRZ.first == true and nCurrentHits > 4) {
// //       // in this case we have still got enough hits for this test after removing virtual hit
// //       const vector<VXDTFHit*>& currentHits = (*currentTC)->getHits();
// //
// //       vector<PositionInfo*> currentHitPositions;
// //       for (VXDTFHit* currentHit : currentHits) {
// //         currentHitPositions.push_back(currentHit->getPositionInfo());
// //       }
// //       currentPass->trackletFilterBox.resetValues(&currentHitPositions);
// //
// //       isZiggZagging = currentPass->trackletFilterBox.ziggZaggRZ();
// //       if (isZiggZagging == true) {
// //         B2DEBUG(20, "TCC filter: tc " << tcCtr << " rejected by ziggZaggRZ! ");
// //         if (LogSystem::Instance().isLevelEnabled(LogConfig::c_Debug, 3, PACKAGENAME()) == true) { killedList.push_back(make_pair(tcCtr, "ziggZaggRZ")); }
// //         m_TESTERtriggeredZigZagRZ++; tcCtr++;
// //         (*currentTC)->setCondition(false);
// //
// //         // Collector TC Update (ziggZaggRZ)
// //         if (m_PARAMdisplayCollector > 0) {
// //           m_collector.updateTC((*currentTC)->getCollectorID(), CollectorTFInfo::m_nameTCC, CollectorTFInfo::m_idTCC, vector<int>(), {FilterID::ziggZaggRZ});
// //         }
// //
// //
// //         continue;
// //       }
// //       B2DEBUG(20, " TCC filter ziggZaggRZ approved TC " << tcCtr);
// //
// //       // Collector TC Update (ziggZaggRZ)
// //       if (m_PARAMdisplayCollector > 0) {
// //         m_collector.updateTC((*currentTC)->getCollectorID(), "", CollectorTFInfo::m_idAlive, {FilterID::ziggZaggRZ}, vector<int>());
// //       }
// //
// //
// //     }
// //     (*currentTC)->setPassIndex(passNumber);
// //     (*currentTC)->setCondition(true);
// //     goodTCIndices.push_back(currentTC);
// //     B2DEBUG(20, " TCC filter approved TC " << tcCtr);
// //     m_TESTERapprovedByTCC++;
// //
// //     tcCtr++;
// //   }
// //
// //   ///deleting TCs which did not survive the tcFilter-step
// //   int goodOnes = goodTCIndices.size(); // number of tc's after TCC-Filter
// //   if (goodOnes not_eq numTCsafterTCC) {
// //     if (LogSystem::Instance().isLevelEnabled(LogConfig::c_Debug, 4, PACKAGENAME()) == true) {
// //       stringstream deadTCs;
// //       deadTCs << " dead TCs are:";
// //       for (auto entry : killedList) { deadTCs << " " << entry.first << ", reason: " << entry.second; }
// //       B2DEBUG(4, std::endl << deadTCs.str())
// //     }
// //     currentPass->tcVector.clear();
// //     for (TCsOfEvent::iterator goodTCIndex : goodTCIndices) {
// //       currentPass->tcVector.push_back((*goodTCIndex));
// //
// //       // int currentIndex;
// //
// //
// //     }
// //   }
// //
// //   int numTC = 0;
// //   for (VXDTFTrackCandidate* currentTC : currentPass->tcVector) {
// //     const vector<VXDTFHit*>& currentHits = currentTC->getHits();
// //     int nHits = currentHits.size();
// //     stringstream secNameOutput;
// //     secNameOutput << endl << "after filtering virtual entries: tc " << numTC << " got " << nHits << " hits and the following secIDs: ";
// //     for (VXDTFHit* currentHit : currentHits) {  // now we are informing each cluster which TC is using it
// //       unsigned int aSecName = currentHit->getSectorName();
// //       secNameOutput << aSecName << "/" << FullSecID(aSecName).getFullSecString() << " ";
// //       if (currentHit->getDetectorType() == Const::PXD) {   // PXD
// //         currentHit->getClusterInfoUV()->addTrackCandidate(currentTC);
// //       } else if (currentHit->getDetectorType() == Const::TEST) {   // TEL
// //         currentHit->getClusterInfoUV()->addTrackCandidate(currentTC);
// //       } else {
// //         if (currentHit->getClusterInfoU() != NULL) { currentHit->getClusterInfoU()->addTrackCandidate(currentTC); } else {
// //           B2WARNING(m_PARAMnameOfInstance << " event " << m_eventCounter << ": currentSVDHit got no UCluster! " <<  aSecName << "/" <<
// //                     FullSecID(aSecName))
// //         }
// //         if (currentHit->getClusterInfoV() != NULL) { currentHit->getClusterInfoV()->addTrackCandidate(currentTC); } else {
// //           B2WARNING(m_PARAMnameOfInstance << " event " << m_eventCounter << ": currentSVDHit got no VCluster! " <<  aSecName << "/" <<
// //                     FullSecID(aSecName))
// //         }
// //       }
// //     } // used for output and informing each cluster which TC is using it
// //     B2DEBUG(20, " " << secNameOutput.str() << " and " << nHits << " hits");
// //     numTC++;
// //   }
// //
// //   m_TESTERcountTotalTCsAfterTCCFilter += goodOnes;
// //   B2DEBUG(10, "TCC-filter: tcList had " << numTCsafterTCC << " TCs. Of these, " << goodOnes <<
// //           " TC's were accepted as good ones by the TCC-Filter")
// //
// //   return goodOnes;
// // }
// //
// //
// //
// // void TFRedesignModule::calcInitialValues4TCs(PassData* currentPass)
// // {
// //   /** REDESIGNCOMMENT CALCINITIALVALUES4TCS 1:
// //    * * short:
// //    *
// //    ** long (+personal comments):
// //    *
// //    ** dependency of module parameters (global):
// //    * m_PARAMpdGCode, m_PARAMartificialMomentum,
// //    *
// //    ** dependency of global in-module variables:
// //    * m_calcSeedType, m_KFBackwardFilter, m_chargeSignFactor,
// //    *
// //    ** dependency of global stuff just because of B2XX-output or debugging only:
// //    *
// //    ** in-module-function-calls:
// //    */
// //   int pdgCode = 0;
// //   pair<TVector3, int> seedValue; // first is momentum vector, second is signCurvature
// //   pair<double, TVector3> returnValues; // first is chi2, second is momentum vector
// //
// //   for (VXDTFTrackCandidate* aTC : currentPass->tcVector) {
// //
// //     if (aTC->getCondition() == false) { continue; }
// //
// //     const vector<PositionInfo*>* currentHits = aTC->getPositionInfos();
// //
// //     pdgCode = m_PARAMpdGCode; // general setting
// //     if (m_calcSeedType == 0) { // helixFit
// //       currentPass->trackletFilterBox.resetValues(currentHits);
// //       seedValue = currentPass->trackletFilterBox.calcMomentumSeed(m_KFBackwardFilter, m_PARAMartificialMomentum);
// //       pdgCode = seedValue.second * m_PARAMpdGCode * m_chargeSignFactor; // improved one for curved tracks
// //     } else if (m_calcSeedType == 1) {
// //       returnValues = currentPass->trackletFilterBox.simpleLineFit3D(currentHits, m_KFBackwardFilter, m_PARAMartificialMomentum);
// //       seedValue.first = returnValues.second; // storing the momentum vector at the right place
// //     } else {
// //       B2WARNING("calcInitialValues4TCs: unknown seedCalculating type set! Using helixFit instead...")
// //     }
// //
// //     if (m_KFBackwardFilter == true) {
// //       aTC->setInitialValue((*currentHits)[0]->hitPosition, seedValue.first, pdgCode); // position, momentum, pdgCode
// //       B2DEBUG(5, " backward: TC has got seedRadius/momentum/pT of " << (*currentHits)[0]->hitPosition.Perp() << "/" <<
// //               seedValue.first.Mag() << "/" << seedValue.first.Perp() << "GeV and estimated pdgCode " << pdgCode);
// //     } else {
// //       aTC->setInitialValue((*currentHits).at(currentHits->size() - 1)->hitPosition, seedValue.first, pdgCode);
// //       B2DEBUG(5, "forward: TC has got seedRadius/momentum/pT of " << (*currentHits).at(currentHits->size() - 1)->hitPosition.Perp() << "/"
// //               << seedValue.first.Mag() << "/" << seedValue.first.Perp() << "GeV and estimated pdgCode " << pdgCode);
// //     }
// //
// //   }
// // }
// //
// //
// //
// // void TFRedesignModule::calcQIbyLength(TCsOfEvent& tcVector, PassSetupVector& passSetups)
// // {
// //   /** REDESIGNCOMMENT CALCQIBYLENGTH 1:
// //    * * short:
// //    *
// //    ** long (+personal comments):
// //    *
// //    ** dependency of module parameters (global):
// //    * m_PARAMsmearMean, m_PARAMsmearSigma,
// //    *
// //    ** dependency of global in-module variables:
// //    * m_nSectorSetups,
// //    *
// //    ** dependency of global stuff just because of B2XX-output or debugging only:
// //    *
// //    ** in-module-function-calls:
// //    */
// //   /// setting quality indices and smear result if chosen
// //   double firstValue = 0.0, rngValue = 0.0, maxLength = 0.0, numTotalLayers;
// //
// //   for (VXDTFTrackCandidate* currentTC : tcVector) {
// //     if (currentTC->getCondition() == false) { continue; }
// //     rngValue = gRandom->Gaus(m_PARAMsmearMean, m_PARAMsmearSigma);
// //     int currentPassIndex = currentTC->getPassIndex();
// //     B2DEBUG(100, " passIndex of currentTC: " << currentTC->getPassIndex())
// //     if (currentPassIndex < 0 or currentPassIndex > m_nSectorSetups - 1) { currentPassIndex = 0; }
// //     if (passSetups.size() == 0) {
// //       numTotalLayers = 6;
// //     } else {
// //       numTotalLayers = passSetups.at(currentPassIndex)->numTotalLayers;
// //     }
// //     B2DEBUG(100, " numTotalLayers of currentTC: " << numTotalLayers)
// //     maxLength =  0.5 / numTotalLayers;
// //
// //     if (rngValue < -0.4) { rngValue = -0.4; } else if (rngValue > 0.4) { rngValue = 0.4; }
// //
// //     if (m_PARAMqiSmear == true) {
// //       firstValue = rngValue + 1.0;
// //
// //     } else {
// //       firstValue = 1.0;
// //     }
// //     B2DEBUG(50, "setQQQ gets the following values: first value: " << firstValue << ", QQQScore: " << sqrt(firstValue * 0.5))
// //     currentTC->setQQQ(firstValue, 2.0);  // resulting QI = 0.3-0.7
// //     currentTC->setTrackQuality((currentTC->getQQQ() * double(currentTC->getHits().size()*maxLength)));
// //     currentTC->setFitSucceeded(true); // no real fit means, allways succeeded
// //   }
// // }
// //
// //
// //
// // void TFRedesignModule::calcQIbyStraightLine(TCsOfEvent& tcVector)
// // {
// //   /** REDESIGNCOMMENT CALCQIBYSTRAIGHTLINE 1:
// //    * * short:
// //    *
// //    ** long (+personal comments):
// //    *
// //    ** dependency of module parameters (global):
// //    * m_PARAMartificialMomentum, m_PARAMpdGCode,
// //    *
// //    ** dependency of global in-module variables:
// //    * m_KFBackwardFilter,
// //    *
// //    ** dependency of global stuff just because of B2XX-output or debugging only:
// //    *
// //    ** in-module-function-calls:
// //    */
// //   TrackletFilters fitterBox;
// //   std::pair<double, TVector3> lineFitResult = { 0., TVector3()};
// //
// //   for (VXDTFTrackCandidate* currentTC : tcVector) {
// //     if (currentTC->getCondition() == false) { continue; }
// //
// //     const vector<PositionInfo*>* currentHits = currentTC->getPositionInfos();
// //     lineFitResult.first = 0;
// //     lineFitResult.second.SetXYZ(0, 0, 0);
// //     bool blaBackward = false;
// //     if (m_KFBackwardFilter == false) { blaBackward = true; }
// //     try {
// //       lineFitResult = fitterBox.simpleLineFit3D(currentHits, blaBackward, m_PARAMartificialMomentum);
// //     } catch (FilterExceptions::Straight_Up& anException) {
// //       B2ERROR("VXDTFModule::calcQIbyStraightLine:lineFit failed , reason: " << anException.what() << ", killing TC...")
// //       currentTC->setFitSucceeded(false);
// //       continue;
// //     }
// //
// //     currentTC->setTrackQuality(TMath::Prob(lineFitResult.first,
// //                                            currentTC->size() * 2 - 4)); // 4 parameters are estimated, each hit is created by two measurements
// //
// //     if (m_KFBackwardFilter == true) {
// //       currentTC->setInitialValue((*currentHits)[0]->hitPosition, lineFitResult.second, m_PARAMpdGCode); // position, momentum, pdgCode
// //       B2DEBUG(5, " backward: TC has got seedRadius/momentum/pT of " << (*currentHits)[0]->hitPosition.Perp() << "/" <<
// //               lineFitResult.second.Mag() << "/" << lineFitResult.second.Perp() << "GeV and estimated pdgCode " << m_PARAMpdGCode);
// //     } else {
// //       currentTC->setInitialValue((*currentHits).at(currentHits->size() - 1)->hitPosition, lineFitResult.second, m_PARAMpdGCode);
// //       B2DEBUG(5, "forward: TC has got seedRadius/momentum/pT of " << (*currentHits).at(currentHits->size() - 1)->hitPosition.Perp() << "/"
// //               << lineFitResult.second.Mag() << "/" << lineFitResult.second.Perp() << "GeV and estimated pdgCode " << m_PARAMpdGCode);
// //     }
// //     currentTC->setFitSucceeded(true);
// //   }
// // }
// //
// //
// //
// // void TFRedesignModule::calcQIbyKalman(TCsOfEvent& tcVector)
// // {
// //   /** REDESIGNCOMMENT CALCQIBYKALMAN 1:
// //    * * short:
// //    *
// //    ** long (+personal comments):
// //    *
// //    ** dependency of module parameters (global):
// //    * m_PARAMdisplayCollector, m_PARAMqiSmear, m_PARAMstoreBrokenQI,
// //    *
// //    ** dependency of global in-module variables:
// //    * m_eventCounter, m_KFBackwardFilter, m_collector,
// //    * m_littleHelperBox, m_TESTERgoodFitsCtr, m_TESTERbadFitsCtr,
// //    *
// //    ** dependency of global stuff just because of B2XX-output or debugging only:
// //    * m_eventCounter, m_PARAMdisplayCollector, m_collector,
// //    * m_TESTERgoodFitsCtr, m_TESTERbadFitsCtr,
// //    *
// //    ** in-module-function-calls:
// //    * writeToRootFile(pVal, chi2, currentTC->getEstRadius(), ndf)
// //    * writeToRootFile(0, 0, currentTC->getEstRadius(), 0)
// //    */
// //   /// produce GFTrackCands for each currently living TC and calculate real kalman-QI's
// //   genfit::KalmanFitter kalmanFilter;
// //
// //   for (VXDTFTrackCandidate* currentTC : tcVector) {
// //     if (currentTC->getCondition() == false) { continue; }
// //
// //     genfit::AbsTrackRep* trackRep = new genfit::RKTrackRep(currentTC->getPDGCode());
// //
// //     genfit::Track track(trackRep, currentTC->getInitialCoordinates(), currentTC->getInitialMomentum());
// //     // FIXME: the fit shouldn't be very sensitive to these values, but
// //     // they should ideally be "typical" expected errors for the fitted
// //     // track.
// //     TMatrixDSym covSeed(6);
// //     covSeed(0, 0) = 1e-3;
// //     covSeed(1, 1) = 1e-3;
// //     covSeed(2, 2) = 4e-3;
// //     covSeed(3, 3) = 0.01e-3;
// //     covSeed(4, 4) = 0.01e-3;
// //     covSeed(5, 5) = 0.04e-3;
// //     track.setCovSeed(covSeed);
// //
// //     //track.setSmoothing(false);
// //
// //     for (VXDTFHit* tfHit : currentTC->getHits()) {
// //       if (tfHit->getDetectorType() == Const::PXD) {
// //         PXDRecoHit* newRecoHit = new PXDRecoHit(tfHit->getClusterInfoUV()->getPXDCluster());
// //         track.insertMeasurement(newRecoHit);
// //       } else if (tfHit->getDetectorType() == Const::SVD) {
// // //         TVector3 pos = *(tfHit->getHitCoordinates()); // jan192014, old way, global coordinates
// // //         SVDRecoHit2D* newRecoHit = new SVDRecoHit2D(tfHit->getVxdID(), pos[0], pos[1]);
// //         //   SVDRecoHit2D* newRecoHit = new SVDRecoHit2D(tfHit->getVxdID(),
// //         //                                              tfHit->getClusterInfoU()->getSVDCluster()->getPosition(),
// //         //                                              tfHit->getClusterInfoV()->getSVDCluster()->getPosition()); /// WARNING test jan192014: local instead of global coordinates
// //         SVDRecoHit2D* newRecoHit = new SVDRecoHit2D(*tfHit->getClusterInfoU()->getSVDCluster(),
// //                                                     *tfHit->getClusterInfoV()->getSVDCluster()); /// WARNING test feb072014: direct references to clusters
// //         track.insertMeasurement(newRecoHit);
// //       } else {
// //         B2ERROR("VXDTFModule::calcQIbyKalman: event " << m_eventCounter << " a hit has unknown detector type ( " << tfHit->getDetectorType()
// //                 << ") discarding hit")
// //       }
// //     }
// //
// //     B2DEBUG(50, "VXDTFModule::calcQIbyKalman, nPxdIndices : " << currentTC->getPXDHitIndices().size() << ", nSvdIndices : " <<
// //             currentTC->getSVDHitIndices().size() << ", nTelIndices : " << currentTC->getTELHitIndices().size() << ", nHitsInTrack: " <<
// //             track.getNumPoints())
// //
// //     try {
// //       if (m_KFBackwardFilter == true) {
// //         kalmanFilter.processTrackPartially(&track, track.getCardinalRep(), -1, 0); /// do the kalman fit
// //       } else {
// //         kalmanFilter.processTrackPartially(&track, track.getCardinalRep(),  0, -1); /// do the kalman fit
// //       }
// //
// //     } catch (exception& e) {
// //       //         std::cerr << e.what();
// //       B2WARNING("VXDTFModule::calcQIbyKalman event " << m_eventCounter << ":, processTrack failed with message: " << e.what() <<
// //                 "! skipping current TC")
// //       currentTC->setCondition(false); // do not store TCs with failed fits if param-flag is set to false
// //
// //       // Collector TC Update (calcQIbyKalman)
// //       if (m_PARAMdisplayCollector > 0) {
// //         std::vector<int> filter_calcQIbyKalman = {FilterID::calcQIbyKalman};
// //
// //         m_collector.updateTC(currentTC->getCollectorID(), CollectorTFInfo::m_nameQI, CollectorTFInfo::m_idQI, vector<int>(),
// //                              filter_calcQIbyKalman);
// //       }
// //
// //       continue;
// //     }
// //
// //     /// WARNING JKL: again, same TC could be updated several times (see down below), is that what you want?
// //     // Collector TC Update (calcQIbyKalman)
// //     if (m_PARAMdisplayCollector > 0) {
// //       std::vector<int> filter_calcQIbyKalman = {FilterID::calcQIbyKalman};
// //
// //       m_collector.updateTC(currentTC->getCollectorID(), "", CollectorTFInfo::m_idAlive, filter_calcQIbyKalman, vector<int>());
// //     }
// //
// //     if (kalmanFilter.isTrackFitted(&track, trackRep)) {
// //       int direction = 1;
// //       if (m_KFBackwardFilter)
// //         direction = -1;
// //       double pVal = kalmanFilter.getPVal(&track, trackRep, direction);
// //       double chi2 = kalmanFilter.getChiSqu(&track, trackRep, direction);
// //       int ndf = kalmanFilter.getNdf(&track, trackRep, 1) + kalmanFilter.getChiSqu(&track, trackRep, 1);
// //       B2DEBUG(10, "calcQI4TC succeeded: calculated kalmanQI: " << chi2 << ", forward-QI: " << kalmanFilter.getChiSqu(&track, trackRep,
// //               1) << " with NDF: " << ndf << ", p-value: " << pVal << ", nHits: " <<  track.getNumPoints())
// // //       RKTrackRep::getPosMomCov
// //       if (pVal < 0.000001 and m_PARAMqiSmear == true) {
// //         currentTC->setTrackQuality(m_littleHelperBox.smearNormalizedGauss(pVal));
// //       } else {
// //         currentTC->setTrackQuality(pVal);
// //       }
// //       writeToRootFile(pVal, chi2, currentTC->getEstRadius(), ndf); // possible TODO: could try to get a fitted radius value by the Kalman
// //
// //       currentTC->setFitSucceeded(true);
// //       m_TESTERgoodFitsCtr++;
// //     } else {
// //       B2DEBUG(10, "calcQI4TC failed...")
// //       m_TESTERbadFitsCtr++;
// //       writeToRootFile(0, 0, currentTC->getEstRadius(), 0); // possible TODO: could try to get a fitted radius value by the Kalman
// //       currentTC->setFitSucceeded(false);
// //       if (m_PARAMqiSmear == true) {
// //         currentTC->setTrackQuality(m_littleHelperBox.smearNormalizedGauss(0.));
// //       } else {
// //         currentTC->setTrackQuality(0);
// //       }
// //
// //       if (m_PARAMstoreBrokenQI == false) {
// //         currentTC->setCondition(false); // do not store TCs with failed fits if param-flag is set to false
// //
// //         // Collector TC Update (calcQIbyKalman)
// //         if (m_PARAMdisplayCollector > 0) {
// //           std::vector<int> filter_calcQIbyKalman = {FilterID::calcQIbyKalman};
// //
// //           m_collector.updateTC(currentTC->getCollectorID(), CollectorTFInfo::m_nameQI, CollectorTFInfo::m_idQI, vector<int>(),
// //                                filter_calcQIbyKalman);
// //         }
// //
// //       }
// //     }
// //   }
// // }
// //
// //
// //
// // genfit::TrackCand TFRedesignModule::generateGFTrackCand(VXDTFTrackCandidate* currentTC)
// // {
// //   /** REDESIGNCOMMENT GENERATEGFTRACKCAND 1:
// //    * * short:
// //    *
// //    ** long (+personal comments):
// //    *
// //    ** dependency of module parameters (global):
// //    *
// //    ** dependency of global in-module variables:
// //    * m_eventCounter,
// //    *
// //    ** dependency of global stuff just because of B2XX-output or debugging only:
// //    * m_eventCounter,
// //    *
// //    ** in-module-function-calls:
// //    */
// //   genfit::TrackCand newGFTrackCand;
// //
// //   B2DEBUG(50, "VXDTFModule::generateGFTrackCand, after newGFTrackCand")
// //
// //   TVector3 posIn = currentTC->getInitialCoordinates();
// //   TVector3 momIn = currentTC->getInitialMomentum();
// //   if (momIn.Mag2() == 0) { B2WARNING("event " << m_eventCounter << ", generateGFTrackCand: currentTC got momentum of 0! probably you forgot to activate magnetic field or you have set the magnetic field value to 0 (in your secMap) and you forgot to set 'artificialMomentum'")}
// //   TVectorD stateSeed(6); //(x,y,z,px,py,pz)
// //   TMatrixDSym covSeed(6);
// //   int pdgCode = currentTC->getPDGCode();
// //   const vector<int>& telHits = currentTC->getTELHitIndices();
// //   const vector<int>& pxdHits = currentTC->getPXDHitIndices();
// //   const vector<int>& svdHits = currentTC->getSVDHitIndices();
// //
// //   if (LogSystem::Instance().isLevelEnabled(LogConfig::c_Debug, 10, PACKAGENAME()) == true) {
// //     stringstream printIndices;
// //     printIndices << "TEL: ";
// //     for (int index : telHits) { printIndices << index << " "; }
// //     printIndices << "PXD: ";
// //     for (int index : pxdHits) { printIndices << index << " "; }
// //     printIndices << ", SVD: ";
// //     for (int index : svdHits) { printIndices << index << " "; }
// //     B2DEBUG(10, "generated GFTC with seed.Mag/p/pT/PDGcode of " << posIn.Mag() << "/" << momIn.Mag() << "/" << momIn.Perp() << "/" <<
// //             pdgCode << " and the following hits: " << printIndices.str());
// //   }
// //
// //   bool gotNan = false;
// //
// //   auto lambdaCheckVector4NAN = [](TVector3 & aVector) -> bool { /// testing c++11 lambda functions...
// //     return std::isnan(aVector.Mag2()); // if one of them is 'nan', Mag2 will be 'nan' too
// //   }; // should be converted to normal function, since feature could be used much more often...
// //   if (lambdaCheckVector4NAN(posIn) == true) { B2ERROR("event " << m_eventCounter << ":helixParameterFit: posIn got 'nan'-entries x/y/z: " << posIn.X() << "/" << posIn.Y() << "/" << posIn.Z()); gotNan = true; }
// //   if (lambdaCheckVector4NAN(momIn) == true) { B2ERROR("event " << m_eventCounter << ":helixParameterFit: momIn got 'nan'-entries x/y/z: " << momIn.X() << "/" << momIn.Y() << "/" << momIn.Z()); gotNan = true; }
// //
// //   stateSeed(0) = posIn[0]; stateSeed(1) = posIn[1]; stateSeed(2) = posIn[2];
// //   stateSeed(3) = momIn[0]; stateSeed(4) = momIn[1]; stateSeed(5) = momIn[2];
// //   covSeed(0, 0) = 0.01 ; covSeed(1, 1) = 0.01 ; covSeed(2, 2) = 0.04 ; // 0.01 = 0.1^2 = dx*dx =dy*dy. 0.04 = 0.2^2 = dz*dz
// //   covSeed(3, 3) = 0.01 ; covSeed(4, 4) = 0.01 ; covSeed(5, 5) = 0.04 ;
// // //   B2DEBUG(10, "generating GFTrackCandidate: posIn.Mag(): " << posIn.Mag() << ", momIn.Mag(): " << momIn.Mag() << ", pdgCode: " << pdgCode);
// //
// //   //newGFTrackCand.set6DSeedAndPdgCode(stateSeed, pdgCode, covSeed);
// //   if (gotNan == true) {
// //     stringstream hitIndices;
// //     hitIndices << "TEL: ";
// //     for (int index : telHits) { hitIndices << index << " "; }
// //     hitIndices << "PXD: ";
// //     for (int index : pxdHits) { hitIndices << index << " "; }
// //     hitIndices << ", SVD: ";
// //     for (int index : svdHits) { hitIndices << index << " "; }
// //
// //     hitIndices << "\n Magnitudes: ";
// //     for (TVector3* hitPos : currentTC->getHitCoordinates()) {
// //       hitIndices << hitPos->Mag() << " ";
// //     }
// //     B2WARNING("pdgCode: " << pdgCode << ", stateSeed0-5: " << stateSeed(0) << "/" << stateSeed(1) << "/" << stateSeed(
// //                 2) << "/" << stateSeed(3) << "/" << stateSeed(4) << "/" << stateSeed(5) << ", hitID/mag: " << hitIndices.str())
// //   }
// //
// //   newGFTrackCand.set6DSeedAndPdgCode(stateSeed, pdgCode);
// //
// // //   vector<int> hitIDs; // for checking hitIDs for double entries
// //   int hitIndex = -1; // temporary index for hit
// //   BOOST_REVERSE_FOREACH(auto * aHit,
// //                         currentTC->getHits()) { // order of hits within VXDTFTrackCandidate: outer->inner hits. GFTrackCand: inner->outer hits
// //     if (aHit->getDetectorType() == Const::SVD) {
// //       hitIndex = aHit->getClusterIndexU();
// //       if (hitIndex != -1) {
// //         newGFTrackCand.addHit(Const::SVD, hitIndex);
// // //        hitIDs.push_back(hitIndex);
// //       }
// //
// //       hitIndex = aHit->getClusterIndexV();
// //       if (hitIndex != -1) {
// //         newGFTrackCand.addHit(Const::SVD, hitIndex);
// // //        hitIDs.push_back(hitIndex);
// //       }
// //     } else { // TEL and PXD behave the same
// //       hitIndex = aHit->getClusterIndexUV();
// //       if (hitIndex != -1) {
// //         newGFTrackCand.addHit(aHit->getDetectorType(), hitIndex);
// // //        hitIDs.push_back(hitIndex);
// //       }
// //     }
// //   }
// //
// //   /// for debugging purposes only: use setTimeSeed for storing qualityIndicator (WARNING this is just a workaround, actually not a valid behavior!)
// //   if (m_PARAMuseTimeSeedAsQI) { newGFTrackCand.setTimeSeed(currentTC->getTrackQuality()); }
// //
// //   return newGFTrackCand;
// // }
// //
// //
// //
// // int TFRedesignModule::cleanOverlappingSet(TCsOfEvent& tcVector)
// // {
// //   /** REDESIGNCOMMENT CLEANOVERLAPPINGSET 1:
// //    * * short:
// //    *
// //    ** long (+personal comments):
// //    *
// //    ** dependency of module parameters (global):
// //    * m_PARAMdisplayCollector,
// //    *
// //    ** dependency of global in-module variables:
// //    * m_collector, m_TESTERfilteredOverlapsQI, m_TESTERfilteredOverlapsQICtr,
// //    * m_TESTERNotFilteredOverlapsQI, m_eventCounter
// //    *
// //    ** dependency of global stuff just because of B2XX-output or debugging only:
// //    * m_PARAMdisplayCollector, m_collector, m_TESTERfilteredOverlapsQI,
// //    * m_TESTERfilteredOverlapsQICtr, m_TESTERNotFilteredOverlapsQI, m_eventCounter,
// //    *
// //    ** in-module-function-calls:
// //    */
// //   int nIHits = 0, nJHits = 0, nMergedHits = 0, killedTCs = 0;
// //   list<int> ihitIDs, jhitIDs, mergedHitIDs;
// //
// //   for (VXDTFTrackCandidate* aTC : tcVector) {
// //     if (aTC->getCondition() == false) { continue; }
// //     ihitIDs = aTC->getHopfieldHitIndices();
// //     ihitIDs.sort();
// //     nIHits = ihitIDs.size();
// //
// //     const TCsOfEvent& rivals = aTC->getBookingRivals();
// //     for (VXDTFTrackCandidate* bTC : rivals) {
// //       if (bTC->getCondition() == false) { continue; }
// //       jhitIDs = bTC->getHopfieldHitIndices();
// //       jhitIDs.sort();
// //       nJHits = jhitIDs.size();
// //
// //       mergedHitIDs = ihitIDs;
// //       mergedHitIDs.merge(jhitIDs); // sorts entries of jhitIDs into mergedHitIDs
// //       mergedHitIDs.unique();
// //       nMergedHits = mergedHitIDs.size();
// //
// //       if (nMergedHits <= nIHits || nMergedHits <= nJHits) { // in this case one TC is part of the other one
// //         killedTCs++;
// //         if (aTC->getTrackQuality() > bTC->getTrackQuality()) {
// //           bTC->setCondition(false);
// //
// //           // Collector TC Update (overlapping)
// //           if (m_PARAMdisplayCollector > 0) {
// //             std::vector<int> filter_overlapping = {FilterID::overlapping};
// //
// //             m_collector.updateTC(bTC->getCollectorID(), CollectorTFInfo::m_nameOverlap, CollectorTFInfo::m_idOverlap, vector<int>(),
// //                                  filter_overlapping);
// //           }
// //
// //         } else {
// //           aTC->setCondition(false);
// //
// //           // Collector TC Update (overlapping)
// //           if (m_PARAMdisplayCollector > 0) {
// //             std::vector<int> filter_overlapping = {FilterID::overlapping};
// //
// //             m_collector.updateTC(aTC->getCollectorID(), CollectorTFInfo::m_nameOverlap, CollectorTFInfo::m_idOverlap, vector<int>(),
// //                                  filter_overlapping);
// //           }
// //
// //           break; // checkOverlappingSet only filters TCs where one is a complete subset of the other. breaking here means, that aTC is subset of bTC, which makes any further filtering using aTC useless. By definition, the bTC has not been the outer loop yet (both know each other through the clusters, therefore bTC would have killed aTC already)
// //         }
// //
// //         /// WARNING JKL: following part is executed twice for aTC ( but only once for bTC, since following part can not be reached by it)
// //         // Collector TC Update (overlapping)
// //         if (m_PARAMdisplayCollector > 0) {
// //           std::vector<int> filter_overlapping = {FilterID::overlapping};
// //
// //           m_collector.updateTC(bTC->getCollectorID(), "", CollectorTFInfo::m_idAlive, filter_overlapping, vector<int>());
// //         }
// //
// //       }
// //
// //     }
// //   }
// //   m_TESTERfilteredOverlapsQI += killedTCs;
// //   B2DEBUG(10, "VXDTFModule event " << m_eventCounter << ": so far, (this round/total)" << m_TESTERfilteredOverlapsQI << "/" <<
// //           killedTCs << " TCs have been killed by funcCleanOverlappingSet...")
// //   /// now all TCs which were completely redundant should be dead. Therefore some TCs could be clean by now -> collecting all overlapping TCs which are still alive:
// //   if (killedTCs not_eq 0) {
// //     /// since cleanOverlappingSet also influences other TCs, it is safer not to refill them here
// //     m_TESTERfilteredOverlapsQICtr++;
// //   } else { m_TESTERNotFilteredOverlapsQI++; B2DEBUG(10, "VXDTFModule event " << m_eventCounter << ": no TC is subset of other TC")}
// //   return killedTCs;
// // }
// //
// //
// //
// // string TFRedesignModule::EventInfoPackage::Print()
// // {
// //   /** REDESIGNCOMMENT EVENTINFOPACKAGE::PRINT 1:
// //    * * short:
// //    *
// //    ** long (+personal comments):
// //    * only relevant for DQM/debugging/testing
// //    *
// //    ** dependency of module parameters (global):
// //    *
// //    ** dependency of global in-module variables:
// //    *
// //    ** dependency of global stuff just because of B2XX-output or debugging only:
// //    *
// //    ** in-module-function-calls:
// //    */
// //   stringstream output;
// //   output << " timeConsumption of event " << evtNumber << " in microseconds: " << endl;
// //
// //   output << "total: " << duration.totalTime.count();
// //   output << ", hitsorting: " << duration.hitSorting.count();
// //   output << ", sf: "  << duration.segFinder.count();
// //   output << ", nf: " << duration.nbFinder.count();
// //   output << ", ca: " << duration.cellularAutomaton.count();
// //   output << ", tcc: " << duration.tcc.count();
// //   output << ", tcfilter: " << duration.postCAFilter.count() << endl;
// //   output << ", overlap: " << duration.checkOverlap.count();
// //   output << ", kf: " << duration.kalmanStuff.count();
// //   output << ", cleanOverlap: " << duration.cleanOverlap.count();
// //   output << ", nn: " << duration.neuronalStuff.count();
// //   output << ", other: " << duration.intermediateStuff.count() << endl;
// //
// //   output << " results: ";
// //   output << "nPXDCluster: " << nPXDClusters << ", nSVDCluster: " << nSVDClusters << ", nSVDHits(x-Passes): " << numSVDHits << endl;
// //   output << "sfActivated: " << segFinderActivated << ", discarded: " << segFinderDiscarded << ", nfActivated: " << nbFinderActivated
// //          << ", discarded: " << nbFinderDiscarded << endl;
// //   output << "tccApproved: " << tccApprovedTCs << ", nTCsAfterTCC: " << numTCsAfterTCC << ", nTCsPostTCfilter: " <<
// //          numTCsAfterTCCfilter << ", nTCsKilledOverlap: " << numTCsKilledByCleanOverlap << ", nTCsFinal: " << numTCsfinal << endl;
// //
// //   return output.str();
// // }
// //
// //
// //
// // // store each cluster (as a clusterPtr) in a map(uniID, sensorStruct), where sensorStruct contains 2 vectors (uClusters and vClusters).
// // // in the end a map containing illuminated sensors - and each cluster inhabiting them - exists.
// // void TFRedesignModule::findSensors4Clusters(ActiveSensorsOfEvent& activatedSensors, vector<ClusterInfo>& clusters)
// // {
// //   /** REDESIGNCOMMENT FINDSENSORS4CLUSTERS 1:
// //    * * short:
// //    *
// //    ** long (+personal comments):
// //    *
// //    ** dependency of module parameters (global):
// //    *
// //    ** dependency of global in-module variables:
// //    *
// //    ** dependency of global stuff just because of B2XX-output or debugging only:
// //    *
// //    ** in-module-function-calls:
// //    * findSensor4Cluster(activatedSensors, cluster)
// //    */
// //   for (ClusterInfo& cluster : clusters) {
// //     if (cluster.isSVD() == false) { continue; }
// //     findSensor4Cluster(activatedSensors, cluster);
// //   }
// //   B2DEBUG(20, activatedSensors.size() << " SVD sensors activated...")
// // }
// //
// //
// // // store a cluster (as a clusterPtr) in a map(uniID, sensorStruct), where sensorStruct contains 2 vectors (uClusters and vClusters).
// // void TFRedesignModule::findSensor4Cluster(ActiveSensorsOfEvent& activatedSensors, ClusterInfo& aClusterInfo)
// // {
// //   /** REDESIGNCOMMENT FINDSENSORS4CLUSTER 1:
// //    * * short:
// //    *
// //    ** long (+personal comments):
// //    *
// //    ** dependency of module parameters (global):
// //    *
// //    ** dependency of global in-module variables:
// //    *
// //    ** dependency of global stuff just because of B2XX-output or debugging only:
// //    *
// //    ** in-module-function-calls:
// //    */
// //   ActiveSensorsOfEvent::iterator sensorIter;
// //   typedef pair<int, SensorStruct > mapEntry;
// //
// //   VxdID aVxdID = aClusterInfo.getSVDCluster()->getSensorID();
// //   int aUniID = aVxdID.getID();
// //   sensorIter = activatedSensors.find(aUniID);
// //   if (sensorIter == activatedSensors.end()) {
// //     SensorStruct newSensor;
// //     newSensor.layerID = aVxdID.getLayerNumber();
// //     sensorIter = activatedSensors.insert(sensorIter, mapEntry(aUniID, newSensor)); //activatedSensors.find(aUniID);
// //   }
// //   if (aClusterInfo.getSVDCluster()->isUCluster() == true) {
// //     sensorIter->second.uClusters.push_back(&aClusterInfo);
// //   } else {
// //     sensorIter->second.vClusters.push_back(&aClusterInfo);
// //   } // else: it's no cluster
// // }
// //
// //
// //
// // // iterate through map of activated sensors & combine each possible combination of clusters. Store them in a vector of structs, where each struct carries an u & a v cluster
// // TFRedesignModule::BrokenSensorsOfEvent TFRedesignModule::find2DSVDHits(ActiveSensorsOfEvent& activatedSensors,
// //     std::vector<ClusterHit>& clusterHitList)
// // {
// //   /** REDESIGNCOMMENT FIND2DSVDHITS 1:
// //    * * short:
// //    *
// //    ** long (+personal comments):
// //    * is used by the baseLineTF and the normal one!
// //    *
// //    ** dependency of module parameters (global):
// //    * m_PARAMnameOfInstance,
// //    *
// //    ** dependency of global in-module variables:
// //    * m_TESTERbadSectorRangeCounterForClusters, m_eventCounter, m_TESTERclustersPersSectorNotMatching,
// //    * m_TESTERSVDOccupancy, m_PARAMhighOccupancyThreshold, m_highOccupancyCase,
// //    * m_TESTERhighOccupancyCtr,
// //    *
// //    ** dependency of global stuff just because of B2XX-output or debugging only:
// //    * m_TESTERbadSectorRangeCounterForClusters, m_eventCounter, m_TESTERclustersPersSectorNotMatching,
// //    * m_TESTERSVDOccupancy, m_PARAMhighOccupancyThreshold, m_highOccupancyCase,
// //    * m_TESTERhighOccupancyCtr, m_PARAMnameOfInstance,
// //    *
// //    ** in-module-function-calls:
// //    */
// //   typedef pair<unsigned int, SensorStruct > mapEntry;
// //   BrokenSensorsOfEvent strangeSensors;
// //   int occupancy = m_TESTERSVDOccupancy.size(), numHits = 0;
// //   bool isStrange = false;
// //   for (const mapEntry& aSensor : activatedSensors) {
// //     int numUclusters = aSensor.second.uClusters.size();
// //     int numVclusters = aSensor.second.vClusters.size();
// //     B2DEBUG(100, " sensor " << FullSecID(VxdID(aSensor.first), false,
// //                                          0) << " has got " << numUclusters << " uClusters and " << numVclusters << " vClusters")
// //     if (numUclusters == 0 || numVclusters == 0) {
// //       m_TESTERbadSectorRangeCounterForClusters++;
// //       B2DEBUG(3, "at event: " << m_eventCounter << " sensor " << FullSecID(VxdID(aSensor.first), false,
// //               0) << " at layer " << aSensor.second.layerID << " has got " << numUclusters << "/" << numVclusters << " u/vclusters!")
// //       isStrange = true;
// //     }
// //     if (numUclusters != numVclusters) {
// //       m_TESTERclustersPersSectorNotMatching++;
// //       B2DEBUG(4, "at event: " << m_eventCounter << " at sensor " << FullSecID(VxdID(aSensor.first), false,
// //               0) << " at layer " << aSensor.second.layerID << " number of clusters do not match: Has got " << numUclusters << "/" << numVclusters
// //               << " u/vclusters!")
// //       isStrange = true;
// //     }
// //     if (isStrange == true) { strangeSensors.push_back(aSensor.first); isStrange = false; }
// //     /// possible extension: adding here the case for strange sensors (instead of returning them), code down below would be used only for normal ones, see
// //
// //     for (int uClNum = 0; uClNum < numUclusters; ++uClNum) {
// //       for (int vClNum = 0; vClNum < numVclusters; ++vClNum) {
// //         ClusterHit aClusterCombi;
// //         aClusterCombi.uCluster = aSensor.second.uClusters[uClNum];
// //         aClusterCombi.vCluster = aSensor.second.vClusters[vClNum];
// //         clusterHitList.push_back(aClusterCombi);
// //         ++numHits;
// //       }
// //     }
// //
// //     // protocolling number of 2D-cluster-combinations per sensor
// //     if (numHits == 0) { continue; }
// //     if (occupancy < numHits) {
// //       m_TESTERSVDOccupancy.resize(numHits + 1, 0);
// //       occupancy = numHits;
// //     }
// //     m_TESTERSVDOccupancy[numHits - 1] += 1;
// //     if (m_PARAMhighOccupancyThreshold < occupancy) {
// //       m_highOccupancyCase = true;
// //       m_TESTERhighOccupancyCtr++;
// //     } else { m_highOccupancyCase = false; }
// //
// //     numHits = 0;
// //   }
// //
// //   list<int> bleh;
// //
// //   if (strangeSensors.empty() and LogSystem::Instance().isLevelEnabled(LogConfig::c_Debug, 4, PACKAGENAME()) == true) {
// //     stringstream output;
// //     output << m_PARAMnameOfInstance << " - event: " << m_eventCounter <<
// //            ": there were strange sensors (having missing clusters) during this event, activated Sensors (ATTENTION: these are sensors, not sectors, therefore no sublayer and sector-info) were:\n";
// //     for (const mapEntry& aSensor : activatedSensors) { output << " " << FullSecID(VxdID(aSensor.first), false, 0); }
// //     output << "\n strange sensors were:\n";
// //     for (unsigned int sensorID : strangeSensors) { output << " " << FullSecID(VxdID(sensorID), false, 0); }
// //     B2DEBUG(4, "\n\n" << output.str() << "\n\n")
// //   }
// //
// //   return strangeSensors;
// // }
// //
// //
// //
// // bool TFRedesignModule::doTheCircleFit(PassData* thisPass, VXDTFTrackCandidate* aTc, int nHits, int tcCtr, int addDegreesOfFreedom)
// // {
// //   /** REDESIGNCOMMENT TERMINATE 1:
// //    * * short:
// //    *
// //    ** long (+personal comments):
// //    *
// //    ** dependency of module parameters (global):
// //    * m_PARAMdisplayCollector , m_PARAMqiSmear
// //    *
// //    ** dependency of global in-module variables:
// //    * m_TESTERtriggeredCircleFit, m_collector, m_calcQiType,
// //    * m_littleHelperBox,
// //    *
// //    ** dependency of global stuff just because of B2XX-output or debugging only:
// //    * m_TESTERtriggeredCircleFit, m_PARAMdisplayCollector, m_collector,
// //    *
// //    ** in-module-function-calls:
// //    *  writeToRootFile(probability, chi2, estimatedRadius, nHits - 3 + addDegreesOfFreedom)
// //    */
// //   boostNsec duration;
// //   boostClock::time_point timer = boostClock::now();
// //   double closestApproachPhi, closestApproachR, estimatedRadius, estimatedCurvature;
// //   double chi2 = thisPass->trackletFilterBox.circleFit(closestApproachPhi, closestApproachR, estimatedCurvature);
// //   estimatedRadius = 1. / estimatedCurvature;
// //   if (estimatedRadius < 0) { estimatedRadius = -estimatedRadius; }
// //   aTc->setEstRadius(estimatedRadius);
// //   if (chi2 < 0) { chi2 = 0; }
// //   double probability = TMath::Prob(chi2, nHits - 3 + addDegreesOfFreedom);
// //   // why is there nHits - 3 + addDegreesOfFreedom? Answer: each hit is one additional degree of freedom (since only the measurement of the u-coordinate of the sensors can be used) but 3 parameters are measured, therefore 3 has to be substracted from the number of hits to get the ndf. The additional degree of freedom (+1) is there, since the origin is used as another hit for the circlefitter but has two degrees of freedom instead of one for the normal hits. therefore +1 has to be added again.
// //   B2DEBUG(10, "TCC Filter at tc " << tcCtr << ": estimated closestApproachPhi, closestApproachR, estimatedRadius: " <<
// //           closestApproachPhi << ", " << closestApproachR << ", " << estimatedRadius << " got fitted with chi2 of " << chi2 <<
// //           " and probability of " << probability << " with ndf: " << nHits - 3 + addDegreesOfFreedom)
// //   if (thisPass->circleFit.first == true and probability < thisPass->circleFit.second) {  // means tc is bad
// //     B2DEBUG(20, "TCC filter: tc " << tcCtr << " rejected by circleFit! ");
// //     m_TESTERtriggeredCircleFit++; tcCtr++;
// //     aTc->setCondition(false);
// //
// //     // Collector TC Update (circlefit)
// //     if (m_PARAMdisplayCollector > 0) {
// //       std::vector<int> filterCircleFit = {FilterID::circlefit};
// //
// //       m_collector.updateTC(aTc->getCollectorID(), CollectorTFInfo::m_nameTCC, CollectorTFInfo::m_idTCC, vector<int>(), filterCircleFit);
// //     }
// //
// //     writeToRootFile(probability, chi2, estimatedRadius, nHits - 3 + addDegreesOfFreedom);
// //     return false;
// //   }
// //   if (m_calcQiType == 2) {
// //     writeToRootFile(probability, chi2, estimatedRadius, nHits - 3 + addDegreesOfFreedom);
// //     if (m_PARAMqiSmear == true) { probability = m_littleHelperBox.smearNormalizedGauss(probability); }
// //     aTc->setTrackQuality(probability);
// //     aTc->setFitSucceeded(true);
// //   }
// //   boostClock::time_point timer2 = boostClock::now();
// //   duration = boost::chrono::duration_cast<boostNsec>(timer2 - timer);
// //
// //   // Collector TC Update (circlefit)
// //   if (m_PARAMdisplayCollector > 0) {
// //     std::vector<int> filterCircleFit = {FilterID::circlefit};
// //
// //     m_collector.updateTC(aTc->getCollectorID(), "", CollectorTFInfo::m_idAlive, filterCircleFit, vector<int>());
// //   }
// //
// //   B2DEBUG(20, " TCC filter circleFit approved TC " << tcCtr << " with nHits: " <<  nHits << ", time consumption: " << duration.count()
// //           << " ns");
// //   return true;
// // }
// //
// //
// //
// //
// // /////////////////////////////////////////////////////////////////////////////////////////////////////////
// // /// REDESIGN - new functions encapsulating smaller tasks
// // /////////////////////////////////////////////////////////////////////////////////////////////////////////
// //
// // ////////////////////////////////////////
// // /// REDESIGN - Functions for initialize:
// // ////////////////////////////////////////
// //
// // void TFRedesignModule::checkAndSetupModuleParameters()
// // {
// //   /** REDESIGNCOMMENT INITIALIZE 1:
// //    * * short:
// //    * safety checks for many parameters
// //    *
// //    ** long (+personal comments):
// //    * safety checks are not done for each parameter, but for those which are often set wrong (by me or others).
// //    * one can notice that some parameters appear as variables too.
// //    * Reason for this is to make the parameters human readable and work internally with faster data types
// //    *
// //    ** dependency of module parameters (global):
// //    * m_PARAMhighOccupancyThreshold, m_PARAMpdGCode, m_PARAMtuneCutoffs,
// //    * m_PARAMreserveHitsThreshold, m_PARAMnameOfInstance, m_PARAMactivateBaselineTF,
// //    * m_PARAMcalcQIType, m_PARAMcalcSeedType, m_PARAMfilterOverlappingTCs,
// //    * m_PARAMwriteToRoot, m_PARAMrootFileName, m_PARAMsmearMean,
// //    * m_PARAMsmearSigma
// //    *
// //    ** dependency of global in-module variables:
// //    * m_chargeSignFactor, m_calcQiType, m_calcSeedType,
// //    * m_filterOverlappingTCs, m_rootFilePtr, m_treeEventWisePtr,
// //    * m_treeTrackWisePtr, m_rootTimeConsumption, m_rootPvalues,
// //    * m_rootChi2, m_rootCircleRadius, m_rootNdf,
// //    * m_littleHelperBox
// //    *
// //    ** dependency of global stuff just because of B2XX-output:
// //    * m_PARAMnameOfInstance,
// //    *
// //    ** in-module-function-calls:
// //    */
// //
// //   m_littleHelperBox.resetValues(m_PARAMsmearMean, m_PARAMsmearSigma);
// //
// //   if (m_PARAMhighOccupancyThreshold < 0) { m_PARAMhighOccupancyThreshold = 0; }
// //
// //   if (m_PARAMpdGCode > 10
// //       and m_PARAMpdGCode <
// //       18) { // in this case, its a lepton. since leptons with positive sign have got negative codes, this must be taken into account
// //     m_chargeSignFactor = 1;
// //   } else { m_chargeSignFactor = -1; }
// //
// //   // check misusage of parameters:
// //   if (m_PARAMtuneCutoffs <= -99.0 or m_PARAMtuneCutoffs > 1000.0) {
// //     B2WARNING(m_PARAMnameOfInstance << ": chosen value for parameter 'tuneCutoffs' is invalid, reseting value to standard (=0.0)...")
// //     m_PARAMtuneCutoffs = 0.;
// //   } else {
// //     m_PARAMtuneCutoffs = m_PARAMtuneCutoffs * 0.01; // reformatting to faster calculation
// //   }
// //   for (double& value : m_PARAMreserveHitsThreshold) {
// //     if (value < 0. or value > 1.0) {
// //       B2WARNING(m_PARAMnameOfInstance <<
// //                 ": chosen value for parameter 'reserveHitsThreshold' is invalid, reseting value to standard (=0.5)...")
// //       value = 0.5;
// //     }
// //   }
// //
// //   B2DEBUG(1, m_PARAMnameOfInstance << "::initialize: chosen calcQIType is '" << m_PARAMcalcQIType << "'")
// //   if (m_PARAMcalcQIType == "trackLength") {
// //     m_calcQiType = 0;
// //   } else if (m_PARAMcalcQIType == "kalman") {
// //     m_calcQiType = 1;
// //   } else if (m_PARAMcalcQIType == "circleFit") {
// //     m_calcQiType = 2;
// //   } else if (m_PARAMcalcQIType == "straightLine") {
// //     m_calcQiType = 3;
// //   } else {
// //     B2WARNING(m_PARAMnameOfInstance << "::initialize: chosen qiType '" << m_PARAMcalcQIType <<
// //               "' is unknown, setting standard to circleFit...")
// //     m_calcQiType = 2;
// //   }
// //
// //   B2DEBUG(1, m_PARAMnameOfInstance << "::initialize: chosen calcSeedType is '" << m_PARAMcalcSeedType << "'")
// //   if (m_PARAMcalcSeedType == "helixFit") {
// //     m_calcSeedType = 0;
// //   } else if (m_PARAMcalcSeedType == "straightLine") {
// //     m_calcSeedType = 1;
// //   } else {
// //     B2WARNING(m_PARAMnameOfInstance << "::initialize: chosen seedType '" << m_PARAMcalcSeedType <<
// //               "' is unknown, setting standard to helixFit...")
// //     m_calcSeedType = 0;
// //   }
// //
// //   B2DEBUG(1, m_PARAMnameOfInstance << "::initialize: chosen technique to filter overlapping TCs is '" << m_PARAMfilterOverlappingTCs
// //           << "'")
// //   if (m_PARAMfilterOverlappingTCs == "hopfield") {
// //     m_filterOverlappingTCs = 2;
// //   } else if (m_PARAMfilterOverlappingTCs == "greedy") {
// //     m_filterOverlappingTCs = 1;
// //   } else if (m_PARAMfilterOverlappingTCs == "none") {
// //     m_filterOverlappingTCs = 0;
// //   } else {
// //     B2WARNING(m_PARAMnameOfInstance << "::initialize: chosen technique to filter overlapping TCs '" << m_PARAMfilterOverlappingTCs <<
// //               "' is unknown, setting standard to greedy...")
// //     m_filterOverlappingTCs = 1;
// //   }
// //
// //   if (m_PARAMwriteToRoot == true) {
// //     m_PARAMrootFileName.at(0) += ".root";
// //     m_rootFilePtr = new TFile(m_PARAMrootFileName.at(0).c_str(), m_PARAMrootFileName.at(1).c_str()); // alternative: UPDATE
// //
// //     m_treeEventWisePtr = new TTree("m_treeEventWisePtr", "anEventWiseTree");
// //     m_treeEventWisePtr->Branch("duration", &m_rootTimeConsumption);
// //
// //     string goodHits = "goodHitsFoundAt";
// //     string sf = "segFinder";
// //     string sm = "secMap";
// //     string nf = "nbFinder";
// //     string tcc = "tcCollector";
// //     string ftc = "finalTC";
// //     string x = "GlobalPos_X";
// //     string y = "GlobalPos_Y";
// //     string z = "GlobalPos_Z";
// //     string r = "GlobalPos_R";
// //
// //     for (unsigned index = 0; index < 6 ; ++index) {
// //       string layerID = std::to_string(index + 1);
// //       m_rootGoodHitPosSMX.at(index) = {};
// //       m_rootGoodHitPosSMY.at(index) = {};
// //       m_rootGoodHitPosSMZ.at(index) = {};
// //       m_rootGoodHitPosSMR.at(index) = {};
// //       m_rootGoodHitPosSFX.at(index) = {};
// //       m_rootGoodHitPosSFY.at(index) = {};
// //       m_rootGoodHitPosSFZ.at(index) = {};
// //       m_rootGoodHitPosSFR.at(index) = {};
// //       m_rootGoodHitPosNFX.at(index) = {};
// //       m_rootGoodHitPosNFY.at(index) = {};
// //       m_rootGoodHitPosNFZ.at(index) = {};
// //       m_rootGoodHitPosNFR.at(index) = {};
// //       m_rootGoodHitPosTCCX.at(index) = {};
// //       m_rootGoodHitPosTCCY.at(index) = {};
// //       m_rootGoodHitPosTCCZ.at(index) = {};
// //       m_rootGoodHitPosTCCR.at(index) = {};
// //       m_rootGoodHitPosFTCX.at(index) = {};
// //       m_rootGoodHitPosFTCY.at(index) = {};
// //       m_rootGoodHitPosFTCZ.at(index) = {};
// //       m_rootGoodHitPosFTCR.at(index) = {};
// //
// //       m_treeEventWisePtr->Branch((goodHits + sm + x + layerID).c_str(), &m_rootGoodHitPosSMX[index]);
// //       m_treeEventWisePtr->Branch((goodHits + sm + y + layerID).c_str(), &m_rootGoodHitPosSMY[index]);
// //       m_treeEventWisePtr->Branch((goodHits + sm + z + layerID).c_str(), &m_rootGoodHitPosSMZ[index]);
// //       m_treeEventWisePtr->Branch((goodHits + sm + r + layerID).c_str(), &m_rootGoodHitPosSMR[index]);
// //
// //       m_treeEventWisePtr->Branch((goodHits + sf + x + layerID).c_str(), &m_rootGoodHitPosSFX[index]);
// //       m_treeEventWisePtr->Branch((goodHits + sf + y + layerID).c_str(), &m_rootGoodHitPosSFY[index]);
// //       m_treeEventWisePtr->Branch((goodHits + sf + z + layerID).c_str(), &m_rootGoodHitPosSFZ[index]);
// //       m_treeEventWisePtr->Branch((goodHits + sf + r + layerID).c_str(), &m_rootGoodHitPosSFR[index]);
// //
// //       m_treeEventWisePtr->Branch((goodHits + nf + x + layerID).c_str(), &m_rootGoodHitPosNFX[index]);
// //       m_treeEventWisePtr->Branch((goodHits + nf + y + layerID).c_str(), &m_rootGoodHitPosNFY[index]);
// //       m_treeEventWisePtr->Branch((goodHits + nf + z + layerID).c_str(), &m_rootGoodHitPosNFZ[index]);
// //       m_treeEventWisePtr->Branch((goodHits + nf + r + layerID).c_str(), &m_rootGoodHitPosNFR[index]);
// //
// //       m_treeEventWisePtr->Branch((goodHits + tcc + x + layerID).c_str(), &m_rootGoodHitPosTCCX[index]);
// //       m_treeEventWisePtr->Branch((goodHits + tcc + y + layerID).c_str(), &m_rootGoodHitPosTCCY[index]);
// //       m_treeEventWisePtr->Branch((goodHits + tcc + z + layerID).c_str(), &m_rootGoodHitPosTCCZ[index]);
// //       m_treeEventWisePtr->Branch((goodHits + tcc + r + layerID).c_str(), &m_rootGoodHitPosTCCR[index]);
// //
// //       m_treeEventWisePtr->Branch((goodHits + ftc + x + layerID).c_str(), &m_rootGoodHitPosFTCX[index]);
// //       m_treeEventWisePtr->Branch((goodHits + ftc + y + layerID).c_str(), &m_rootGoodHitPosFTCY[index]);
// //       m_treeEventWisePtr->Branch((goodHits + ftc + z + layerID).c_str(), &m_rootGoodHitPosFTCZ[index]);
// //       m_treeEventWisePtr->Branch((goodHits + ftc + r + layerID).c_str(), &m_rootGoodHitPosFTCR[index]);
// //     }
// //
// //     m_treeEventWisePtr->Branch((goodHits + sm + x).c_str(), &m_rootAllGoodHitPosSMX);
// //     m_treeEventWisePtr->Branch((goodHits + sm + y).c_str(), &m_rootAllGoodHitPosSMY);
// //     m_treeEventWisePtr->Branch((goodHits + sm + z).c_str(), &m_rootAllGoodHitPosSMZ);
// //     m_treeEventWisePtr->Branch((goodHits + sm + r).c_str(), &m_rootAllGoodHitPosSMR);
// //
// //     m_treeEventWisePtr->Branch((goodHits + sf + x).c_str(), &m_rootAllGoodHitPosSFX);
// //     m_treeEventWisePtr->Branch((goodHits + sf + y).c_str(), &m_rootAllGoodHitPosSFY);
// //     m_treeEventWisePtr->Branch((goodHits + sf + z).c_str(), &m_rootAllGoodHitPosSFZ);
// //     m_treeEventWisePtr->Branch((goodHits + sf + r).c_str(), &m_rootAllGoodHitPosSFR);
// //
// //     m_treeEventWisePtr->Branch((goodHits + nf + x).c_str(), &m_rootAllGoodHitPosNFX);
// //     m_treeEventWisePtr->Branch((goodHits + nf + y).c_str(), &m_rootAllGoodHitPosNFY);
// //     m_treeEventWisePtr->Branch((goodHits + nf + z).c_str(), &m_rootAllGoodHitPosNFZ);
// //     m_treeEventWisePtr->Branch((goodHits + nf + r).c_str(), &m_rootAllGoodHitPosNFR);
// //
// //     m_treeEventWisePtr->Branch((goodHits + tcc + x).c_str(), &m_rootAllGoodHitPosTCCX);
// //     m_treeEventWisePtr->Branch((goodHits + tcc + y).c_str(), &m_rootAllGoodHitPosTCCY);
// //     m_treeEventWisePtr->Branch((goodHits + tcc + z).c_str(), &m_rootAllGoodHitPosTCCZ);
// //     m_treeEventWisePtr->Branch((goodHits + tcc + r).c_str(), &m_rootAllGoodHitPosTCCR);
// //
// //     m_treeEventWisePtr->Branch((goodHits + ftc + x).c_str(), &m_rootAllGoodHitPosFTCX);
// //     m_treeEventWisePtr->Branch((goodHits + ftc + y).c_str(), &m_rootAllGoodHitPosFTCY);
// //     m_treeEventWisePtr->Branch((goodHits + ftc + z).c_str(), &m_rootAllGoodHitPosFTCZ);
// //     m_treeEventWisePtr->Branch((goodHits + ftc + r).c_str(), &m_rootAllGoodHitPosFTCR);
// //
// //
// //     m_treeTrackWisePtr = new TTree("m_treeTrackWisePtr", "aTrackWiseTree");
// //     m_treeTrackWisePtr->Branch("pValues", &m_rootPvalues);
// //     m_treeTrackWisePtr->Branch("chi2Values", &m_rootChi2);
// //     m_treeTrackWisePtr->Branch("circleRadii", &m_rootCircleRadius);
// //     m_treeTrackWisePtr->Branch("ndfValues", &m_rootNdf);
// //   } else {
// //     m_rootFilePtr = NULL;
// //     m_treeTrackWisePtr = NULL;
// //   }
// // }
// //
// //
// //
// // void TFRedesignModule::prepareExternalTools()
// // {
// //   /** REDESIGNCOMMENT INITIALIZE 2:
// //    * * short:
// //    * register storearray-related stuff
// //    *
// //    ** long (+personal comments):
// //    * some are only created if their related parameters are set to the values needed.
// //    * additionally, one module parameter is not flagged with m_PARAM... but with m_TESTER... (which are parameters related only to testing purposes),
// //    * such an inconsistency shall be surpressed for the redesign.
// //    * DisplayCollector-related stuff is needed for display and for debugging, therefore always listed for debug-stuff too.
// //    * creates dependencies of the testbeam package (TelCluster)
// //    *
// //    ** dependency of module parameters (global):
// //    * m_PARAMgfTrackCandsColName, m_TESTERexpandedTestingRoutines, m_PARAMinfoBoardName,
// //    * m_PARAMdisplayCollector, m_PARAMpxdClustersName, m_PARAMsvdClustersName
// //    *
// //    ** dependency of global in-module variables:
// //    * m_collector, m_pxdClusters, m_svdClusters
// //    *
// //    ** dependency of global stuff just because of B2XX-output or debugging only:
// //    * m_PARAMnameOfInstance, m_PARAMinfoBoardName, m_PARAMdisplayCollector,
// //    * m_collector
// //    *
// //    ** in-module-function-calls:
// //    */
// //
// //
// //   /// genfit::TrackCandidate
// //   m_finalTrackCandidates.registerInDataStore(m_PARAMgfTrackCandsColName);
// //   m_PARAMgfTrackCandsColName = m_finalTrackCandidates.getName();
// //
// //   if (!genfit::MaterialEffects::getInstance()->isInitialized()) {
// //     B2FATAL("Material effects not set up.  Please use SetupGenfitExtrapolationModule.");
// //   }
// //
// //   /// temporary members for testing purposes (minimal testing routines)
// //   if (m_TESTERexpandedTestingRoutines == true) {
// //     m_extraInfo4GFTCs.registerInDataStore(m_PARAMinfoBoardName);
// //     m_PARAMinfoBoardName = m_extraInfo4GFTCs.getName();
// //   }
// //
// //   // registerPersistence (StoreArrays & RelationArray) for the Collector
// //   if (m_PARAMdisplayCollector > 0) {
// //     B2DEBUG(10, "VXDTF: Display: Module Collector initPersistent");
// //     m_collector.initPersistent();
// //   }
// //
// //   m_pxdClusters.isOptional(m_PARAMpxdClustersName);
// //   if (m_pxdClusters.isOptional(m_PARAMpxdClustersName) == true and m_usePXDHits == true) { m_PARAMpxdClustersName = m_pxdClusters.getName(); }
// //
// //   m_svdClusters.isOptional(m_PARAMsvdClustersName);
// //   if (m_svdClusters.isOptional(m_PARAMsvdClustersName) == true and m_useSVDHits == true) { m_PARAMsvdClustersName = m_svdClusters.getName(); }
// //
// // }
// //
// //
// //
// // //////////////////////////////////////
// // /// REDESIGN - Functions for beginRun:
// // //////////////////////////////////////
// //
// // void TFRedesignModule::setupPasses()
// // {
// //   /** REDESIGNCOMMENT BEGINRUN 1:
// //    * * short:
// //    * setup for sectorMaps used for passes
// //    *
// //    ** long (+personal comments):
// //    * all that stuff will move to the sectorMap-Module
// //    *
// //    ** dependency of module parameters (global):
// //    * m_PARAMsectorSetup, m_PARAMtuneCutoffs, m_PARAMhighestAllowedLayer,
// //    * m_PARAMreserveHitsThreshold, m_PARAMminLayer, m_PARAMminState,
// //    * m_PARAMactivateDistance3D, m_PARAMactivateDistanceXY, m_PARAMactivateDistanceZ,
// //    * m_PARAMactivateSlopeRZ, m_PARAMactivateNormedDistance3D, m_PARAMactivateAlwaysTrue2Hit,
// //    * m_PARAMactivateAlwaysFalse2Hit, m_PARAMactivateRandom2Hit, m_PARAMactivateAngles3DHioC,
// //    * m_PARAMactivateAnglesXYHioC, m_PARAMactivateAnglesRZHioC, m_PARAMactivateDeltaSlopeRZHioC,
// //    * m_PARAMactivateDistance2IPHioC, m_PARAMactivatePTHioC, m_PARAMactivateHelixParameterFitHioC,
// //    * m_PARAMactivateAnglesXY, m_PARAMactivateAnglesRZ, m_PARAMactivateDeltaSlopeRZ,
// //    * m_PARAMactivateDistance2IP, m_PARAMactivatePT, m_PARAMactivateHelixParameterFit,
// //    * m_PARAMactivateDeltaSOverZ, m_PARAMactivateDeltaSlopeZOverS, m_PARAMactivateAlwaysTrue3Hit,
// //    * m_PARAMactivateAlwaysFalse3Hit, m_PARAMactivateRandom3Hit, m_PARAMactivateDeltaPtHioC,
// //    * m_PARAMactivateDeltaDistance2IPHioC, m_PARAMactivateZigZagXY, m_PARAMactivateZigZagXYWithSigma,
// //    * m_PARAMactivateZigZagRZ, m_PARAMactivateDeltaPt, m_PARAMactivateCircleFit,
// //    * m_PARAMactivateDeltaDistance2IP, m_PARAMactivateAlwaysTrue4Hit, m_PARAMactivateAlwaysFalse4Hit,
// //    * m_PARAMactivateRandom4Hit
// //    * m_PARAMtuneDistance3D, m_PARAMtuneDistanceXY, m_PARAMtuneDistanceZ,
// //    * m_PARAMtuneSlopeRZ, m_PARAMtuneNormedDistance3D, m_PARAMtuneAlwaysTrue2Hit,
// //    * m_PARAMtuneAlwaysFalse2Hit, m_PARAMtuneRandom2Hit, m_PARAMtuneAngles3DHioC,
// //    * m_PARAMtuneAnglesXYHioC, m_PARAMtuneAnglesRZHioC, m_PARAMtuneDeltaSlopeRZHioC,
// //    * m_PARAMtuneDistance2IPHioC, m_PARAMtunePTHioC, m_PARAMtuneHelixParameterFitHioC,
// //    * m_PARAMtuneAnglesXY, m_PARAMtuneAnglesRZ, m_PARAMtuneDeltaSlopeRZ,
// //    * m_PARAMtuneDistance2IP, m_PARAMtunePT, m_PARAMtuneHelixParameterFit,
// //    * m_PARAMtuneDeltaSOverZ, m_PARAMtuneDeltaSlopeZOverS, m_PARAMtuneAlwaysTrue3Hit,
// //    * m_PARAMtuneAlwaysFalse3Hit, m_PARAMtuneRandom3Hit, m_PARAMtuneDeltaPtHioC,
// //    * m_PARAMtuneDeltaDistance2IPHioC, m_PARAMtuneZigZagXY, m_PARAMtuneZigZagXYWithSigma,
// //    * m_PARAMtuneZigZagRZ, m_PARAMtuneDeltaPt, m_PARAMtuneCircleFit,
// //    * m_PARAMtuneDeltaDistance2IP, m_PARAMtuneAlwaysTrue4Hit, m_PARAMtuneAlwaysFalse4Hit,
// //    * m_PARAMtuneRandom4Hit
// //    *
// //    ** dependency of global in-module variables:
// //    * m_usePXDHits, m_useSVDHits, m_useTELHits,
// //    * m_nSectorSetups, m_passSetupVector,
// //    *
// //    *
// //    ** dependency of global stuff just because of B2XX-output or debugging only:
// //    * m_PARAMnameOfInstance, m_PARAMactivateAlwaysTrue2Hit, m_PARAMactivateAlwaysFalse2Hit,
// //    * m_PARAMactivateRandom2Hit, m_PARAMactivateAlwaysTrue3Hit, m_PARAMactivateAlwaysFalse3Hit,
// //    * m_PARAMactivateRandom3Hit, m_PARAMactivateAlwaysTrue4Hit, m_PARAMactivateAlwaysFalse4Hit,
// //    * m_PARAMactivateRandom4Hit, m_PARAMhighOccupancyThreshold, m_PARAMtuneAlwaysTrue2Hit,
// //    * m_PARAMtuneAlwaysFalse2Hit, m_PARAMtuneRandom2Hit, m_PARAMtuneAlwaysTrue3Hit,
// //    * m_PARAMtuneAlwaysFalse3Hit, m_PARAMtuneRandom3Hit, m_PARAMtuneAlwaysTrue4Hit,
// //    * m_PARAMtuneAlwaysFalse4Hit, m_PARAMtuneRandom4Hit,
// //    *
// //    ** in-module-function-calls:
// //    */
// //
// //   // here some variables copied from the passes (will be rewritten for each pass, therefore only the settings of the last pass will survive)
// //   TVector3 origin;
// //   string detectorType;
// //   /// for each setup, fill parameters, calc numTotalLayers... TODO: failsafe implementation (currently no protection against bad user imput) lacks of style, longterm goal, export that procedure into a function
// //
// //   m_usePXDHits = false, m_useSVDHits = false;
// //
// //   m_nSectorSetups = m_PARAMsectorSetup.size();
// //
// //
// //   for (int i = 0; i < m_nSectorSetups; ++i) {
// //
// //     PassData* newPass = new PassData;
// //
// //     newPass->sectorSetup = m_PARAMsectorSetup.at(i);
// //
// //     newPass->generalTune = m_PARAMtuneCutoffs; // for all passes the same
// //
// //     VXDTFSecMap::Class(); // essential, needed for root, waiting for root 6 to be removed (hopefully)
// //     string chosenSetup = newPass->sectorSetup;
// //     string directory = "/Detector/Tracking/CATFParameters/" + chosenSetup;
// //     const VXDTFSecMap* newMap = NULL;
// //     try {
// //       newMap = dynamic_cast<const VXDTFSecMap*>(Gearbox::getInstance().getTObject(directory.c_str()));
// //     } catch (exception& e) {
// //       B2FATAL("VXDTFModule::initialize: could not load sectorMap. Reason: exception thrown: " << e.what() <<
// //               ", this means you have to check whether the sectorMaps stored in ../tracking/data/VXDTFindex.xml and/or ../testbeam/vxd/data/VXDTFindexTF.xml are uncommented and locally unpacked and available!")
// //     }
// //
// //     const double magneticField = newMap->getMagneticFieldStrength();
// //     newPass->magneticFieldStrength = magneticField;
// //     newPass->threeHitFilterBox.resetMagneticField(magneticField);
// //     newPass->fourHitFilterBox.resetMagneticField(magneticField);
// //     newPass->trackletFilterBox.resetMagneticField(magneticField);
// //
// //     newPass->additionalInfo = newMap->getAdditionalInfo();
// //
// //     newPass->secConfigU = newMap->getSectorConfigU();
// //     newPass->secConfigV = newMap->getSectorConfigV();
// //     origin = newMap->getOrigin();
// //     newPass->origin = origin;
// //     stringstream secConU, secConV;
// //     for (double entry : newPass->secConfigU) { secConU << " " << entry; }
// //     for (double entry : newPass->secConfigV) { secConV << " " << entry; }
// //     B2INFO(" pass " << newPass->sectorSetup << "-setting: got magneticFieldStrength: " << magneticField << ", origin at: (" << origin[0]
// //            << "," << origin[1] << "," << origin[2] << ") and sectorConfig \n U: " << secConU.str() << endl << " V: " << secConV.str() << endl
// //            << " and additional Info: " << newPass->additionalInfo)
// //
// //
// //     if (int (m_PARAMhighestAllowedLayer.size()) < i + 1) {
// //       B2DEBUG(1, m_PARAMnameOfInstance <<
// //               "highestAllowedLayer not set for each sectorMap, copying first choice (you can ignore this warning if you don't want to set parameters for each pass separately)")
// //       newPass->highestAllowedLayer = m_PARAMhighestAllowedLayer.at(m_PARAMhighestAllowedLayer.size() - 1);
// //     } else {
// //       newPass->highestAllowedLayer = m_PARAMhighestAllowedLayer.at(i);
// //     }
// //     detectorType = newMap->getDetectorType();
// //     newPass->chosenDetectorType = detectorType;
// //     newPass->numTotalLayers = 0;
// //
// //     if (detectorType.find("SVD") != std::string::npos) {
// //       m_useSVDHits = true;
// //       newPass->useSVDHits = true;
// //       newPass->numTotalLayers +=
// //         4; // WARNING hardcoded! can we get this info from the system itself? WARNING find where this is still used and find out its purpose (dangerous when some layers are missing?)
// //
// //     }
// //     if (detectorType.find("PXD") != std::string::npos) {
// //       m_usePXDHits = true;
// //       newPass->usePXDHits = true;
// //       newPass->numTotalLayers +=
// //         2; // WARNING hardcoded! can we get this info from the system itself? WARNING find where this is still used and find out its purpose (dangerous when some layers are missing?)
// //
// //     }
// //     if (m_usePXDHits == false and m_useSVDHits == false) {
// //       B2ERROR(m_PARAMnameOfInstance << "Pass " << i << " with setting '" << chosenSetup <<
// //               "': chosen detectorType via param 'detectorType' (" << detectorType << ") is invalid, resetting value to standard (=VXD)")
// //       m_useSVDHits = true;
// //       m_usePXDHits = true;
// //       newPass->chosenDetectorType = "VXD";
// //       newPass->usePXDHits = true;
// //       newPass->useSVDHits = true;
// //     }
// //
// //
// //     newPass->numTotalLayers = newPass->numTotalLayers - (6 - newPass->highestAllowedLayer);
// //     if (newPass->numTotalLayers < 2) { newPass->numTotalLayers = 2; }   // to avoid division by zero in some cases
// //     B2DEBUG(1, m_PARAMnameOfInstance << "Pass " << i << ": chosen detectorType: " << newPass->chosenDetectorType <<
// //             " and chosen sectorSetup: " << newPass->sectorSetup)
// //
// //
// //     if (int (m_PARAMreserveHitsThreshold.size()) < i + 1) {
// //       B2DEBUG(1,
// //               "reserveHitsThreshold not set for each sectorMap, copying first choice (you can ignore this warning if you don't want to set parameters for each pass separately)")
// //       newPass->reserveHitsThreshold = m_PARAMreserveHitsThreshold.at(m_PARAMreserveHitsThreshold.size() - 1) ;
// //     } else {
// //       newPass->reserveHitsThreshold = m_PARAMreserveHitsThreshold.at(i);
// //       if (newPass->reserveHitsThreshold < 0) { newPass->reserveHitsThreshold = 0; } else if (newPass->reserveHitsThreshold > 1.) { newPass->reserveHitsThreshold = 1.; }
// //     }
// //     if (int (m_PARAMminLayer.size()) < i + 1) {
// //       B2DEBUG(1, m_PARAMnameOfInstance <<
// //               "minLayer not set for each sectorMap, copying first choice (you can ignore this warning if you don't want to set parameters for each pass separately)")
// //       newPass->minLayer = m_PARAMminLayer.at(m_PARAMminLayer.size() - 1);
// //     } else {
// //       newPass->minLayer = m_PARAMminLayer.at(i);
// //     }
// //     if (int (m_PARAMminState.size()) < i + 1) {
// //       B2DEBUG(1, m_PARAMnameOfInstance <<
// //               "minState not set for each sectorMap, copying first choice (you can ignore this warning if you don't want to set parameters for each pass separately)")
// //       newPass->minState = m_PARAMminState.at(m_PARAMminState.size() - 1);
// //     } else {
// //       newPass->minState = m_PARAMminState.at(i);
// //     }
// //
// //     B2DEBUG(10, "starting import of segFinderFilters:")
// //
// //     int sfCtr = 0, sfhoCtr = 0, nfCtr = 0, nfhoCtr = 0, tccfCtr = 0; // counting number of activated tests for each filter step
// //     ///sFinder:
// //     if (int (m_PARAMactivateDistance3D.size()) < i + 1) {
// //       newPass->distance3D.first = m_PARAMactivateDistance3D.at(m_PARAMactivateDistance3D.size() - 1);
// //     } else {
// //       newPass->distance3D.first = m_PARAMactivateDistance3D.at(i);
// //     }
// //     if (int (m_PARAMactivateDistanceXY.size()) < i + 1) {
// //       newPass->distanceXY.first = m_PARAMactivateDistanceXY.at(m_PARAMactivateDistanceXY.size() - 1);
// //     } else {
// //       newPass->distanceXY.first = m_PARAMactivateDistanceXY.at(i);;
// //     }
// //     if (int (m_PARAMactivateDistanceZ.size()) < i + 1) {
// //       newPass->distanceZ.first = m_PARAMactivateDistanceZ.at(m_PARAMactivateDistanceZ.size() - 1);
// //     } else {
// //       newPass->distanceZ.first = m_PARAMactivateDistanceZ.at(i);
// //     }
// //     if (int (m_PARAMactivateSlopeRZ.size()) < i + 1) {
// //       newPass->slopeRZ.first = m_PARAMactivateSlopeRZ.at(m_PARAMactivateSlopeRZ.size() - 1);
// //     } else {
// //       newPass->slopeRZ.first = m_PARAMactivateSlopeRZ.at(i);
// //     }
// //     if (int (m_PARAMactivateNormedDistance3D.size()) < i + 1) {
// //       newPass->normedDistance3D.first = m_PARAMactivateNormedDistance3D.at(m_PARAMactivateNormedDistance3D.size() - 1);
// //     } else {
// //       newPass->normedDistance3D.first = m_PARAMactivateNormedDistance3D.at(i);
// //     }
// //     if (int (m_PARAMactivateAlwaysTrue2Hit.size()) < i + 1) {
// //       newPass->alwaysTrue2Hit.first = m_PARAMactivateAlwaysTrue2Hit.at(m_PARAMactivateAlwaysTrue2Hit.size() - 1);
// //     } else {
// //       newPass->alwaysTrue2Hit.first = m_PARAMactivateAlwaysTrue2Hit.at(i);
// //     }
// //     if (int (m_PARAMactivateAlwaysFalse2Hit.size()) < i + 1) {
// //       newPass->alwaysFalse2Hit.first = m_PARAMactivateAlwaysFalse2Hit.at(m_PARAMactivateAlwaysFalse2Hit.size() - 1);
// //     } else {
// //       newPass->alwaysFalse2Hit.first = m_PARAMactivateAlwaysFalse2Hit.at(i);
// //     }
// //     if (int (m_PARAMactivateRandom2Hit.size()) < i + 1) {
// //       newPass->random2Hit.first = m_PARAMactivateRandom2Hit.at(m_PARAMactivateRandom2Hit.size() - 1);
// //     } else {
// //       newPass->random2Hit.first = m_PARAMactivateRandom2Hit.at(i);
// //     }
// //     if (newPass->distance3D.first == true) { sfCtr++; }
// //     if (newPass->distanceXY.first == true) { sfCtr++; }
// //     if (newPass->distanceZ.first == true) { sfCtr++; }
// //     if (newPass->slopeRZ.first == true) { sfCtr++; }
// //     if (newPass->normedDistance3D.first == true) { sfCtr++; }
// //     if (newPass->alwaysTrue2Hit.first == true) { sfCtr++; }
// //     if (newPass->alwaysFalse2Hit.first == true) { sfCtr++; }
// //     if (newPass->random2Hit.first == true) { sfCtr++; }
// //     B2DEBUG(2, "finished importing segFinderFilters, " << sfCtr << " filters imported")
// //     ///sFinder ho finder (2+1 hit)
// //     B2DEBUG(10, "starting import of segFinderHioCFilters:")
// //     if (int (m_PARAMactivateAngles3DHioC.size()) < i + 1) {
// //       newPass->anglesHighOccupancy3D.first = m_PARAMactivateAngles3DHioC.at(m_PARAMactivateAngles3DHioC.size() - 1);
// //     } else {
// //       newPass->anglesHighOccupancy3D.first =  m_PARAMactivateAngles3DHioC.at(i);
// //     }
// //     if (int (m_PARAMactivateAnglesXYHioC.size()) < i + 1) {
// //       newPass->anglesHighOccupancyXY.first = m_PARAMactivateAnglesXYHioC.at(m_PARAMactivateAnglesXYHioC.size() - 1);
// //     } else {
// //       newPass->anglesHighOccupancyXY.first = m_PARAMactivateAnglesXYHioC.at(i);
// //     }
// //     if (int (m_PARAMactivateAnglesRZHioC.size()) < i + 1) {
// //       newPass->anglesHighOccupancyRZ.first = m_PARAMactivateAnglesRZHioC.at(m_PARAMactivateAnglesRZHioC.size() - 1);
// //     } else {
// //       newPass->anglesHighOccupancyRZ.first = m_PARAMactivateAnglesRZHioC.at(i);
// //     }
// //     if (int (m_PARAMactivateDeltaSlopeRZHioC.size()) < i + 1) {
// //       newPass->deltaSlopeHighOccupancyRZ.first = m_PARAMactivateDeltaSlopeRZHioC.at(m_PARAMactivateDeltaSlopeRZHioC.size() - 1);
// //     } else {
// //       newPass->deltaSlopeHighOccupancyRZ.first = m_PARAMactivateDeltaSlopeRZHioC.at(i);
// //     }
// //     if (int (m_PARAMactivateDistance2IPHioC.size()) < i + 1) {
// //       newPass->distanceHighOccupancy2IP.first = m_PARAMactivateDistance2IPHioC.at(m_PARAMactivateDistance2IPHioC.size() - 1);
// //     } else {
// //       newPass->distanceHighOccupancy2IP.first =  m_PARAMactivateDistance2IPHioC.at(i);
// //     }
// //     if (int (m_PARAMactivatePTHioC.size()) < i + 1) {
// //       newPass->pTHighOccupancy.first = m_PARAMactivatePTHioC.at(m_PARAMactivatePTHioC.size() - 1);
// //     } else {
// //       newPass->pTHighOccupancy.first =  m_PARAMactivatePTHioC.at(i);
// //     }
// //     if (int (m_PARAMactivateHelixParameterFitHioC.size()) < i + 1) {
// //       newPass->helixParameterHighOccupancyFit.first = m_PARAMactivateHelixParameterFitHioC.at(m_PARAMactivateHelixParameterFitHioC.size()
// //                                                       - 1);
// //     } else {
// //       newPass->helixParameterHighOccupancyFit.first =  m_PARAMactivateHelixParameterFitHioC.at(i);
// //     }
// //     if (newPass->anglesHighOccupancy3D.first == true) { sfhoCtr++; }
// //     if (newPass->anglesHighOccupancyXY.first == true) { sfhoCtr++; }
// //     if (newPass->anglesHighOccupancyRZ.first == true) { sfhoCtr++; }
// //     if (newPass->deltaSlopeHighOccupancyRZ.first == true) { sfhoCtr++; }
// //     if (newPass->distanceHighOccupancy2IP.first == true) { sfhoCtr++; }
// //     if (newPass->pTHighOccupancy.first == true) { sfhoCtr++; }
// //     if (newPass->helixParameterHighOccupancyFit.first == true) { sfhoCtr++; }
// //     B2DEBUG(2, "finished importing segFinderHioCFilters, " << sfhoCtr << " filters imported")
// //
// //     ///nbFinder:
// //     B2DEBUG(10, "starting import of nbFinderFilters:")
// //     if (int (m_PARAMactivateAngles3D.size()) < i + 1) {
// //       newPass->angles3D.first = m_PARAMactivateAngles3D.at(m_PARAMactivateAngles3D.size() - 1);
// //     } else {
// //       newPass->angles3D.first =  m_PARAMactivateAngles3D.at(i);
// //     }
// //     if (int (m_PARAMactivateAnglesXY.size()) < i + 1) {
// //       newPass->anglesXY.first = m_PARAMactivateAnglesXY.at(m_PARAMactivateAnglesXY.size() - 1);
// //     } else {
// //       newPass->anglesXY.first = m_PARAMactivateAnglesXY.at(i);
// //     }
// //     if (int (m_PARAMactivateAnglesRZ.size()) < i + 1) {
// //       newPass->anglesRZ.first = m_PARAMactivateAnglesRZ.at(m_PARAMactivateAnglesRZ.size() - 1);
// //     } else {
// //       newPass->anglesRZ.first = m_PARAMactivateAnglesRZ.at(i);
// //     }
// //     if (int (m_PARAMactivateDeltaSlopeRZ.size()) < i + 1) {
// //       newPass->deltaSlopeRZ.first = m_PARAMactivateDeltaSlopeRZ.at(m_PARAMactivateDeltaSlopeRZ.size() - 1);
// //     } else {
// //       newPass->deltaSlopeRZ.first = m_PARAMactivateDeltaSlopeRZ.at(i);
// //     }
// //     if (int (m_PARAMactivateDistance2IP.size()) < i + 1) {
// //       newPass->distance2IP.first = m_PARAMactivateDistance2IP.at(m_PARAMactivateDistance2IP.size() - 1);
// //     } else {
// //       newPass->distance2IP.first =  m_PARAMactivateDistance2IP.at(i);
// //     }
// //     if (int (m_PARAMactivatePT.size()) < i + 1) {
// //       newPass->pT.first = m_PARAMactivatePT.at(m_PARAMactivatePT.size() - 1);
// //     } else {
// //       newPass->pT.first =  m_PARAMactivatePT.at(i);
// //     }
// //     if (int (m_PARAMactivateHelixParameterFit.size()) < i + 1) {
// //       newPass->helixParameterFit.first = m_PARAMactivateHelixParameterFit.at(m_PARAMactivateHelixParameterFit.size() - 1);
// //     } else {
// //       newPass->helixParameterFit.first =  m_PARAMactivateHelixParameterFit.at(i);
// //     }
// //     if (int (m_PARAMactivateDeltaSOverZ.size()) < i + 1) {
// //       newPass->deltaSOverZ.first = m_PARAMactivateDeltaSOverZ.at(m_PARAMactivateDeltaSOverZ.size() - 1);
// //     } else {
// //       newPass->deltaSOverZ.first =  m_PARAMactivateDeltaSOverZ.at(i);
// //     }
// //     if (int (m_PARAMactivateDeltaSlopeZOverS.size()) < i + 1) {
// //       newPass->deltaSlopeZOverS.first = m_PARAMactivateDeltaSlopeZOverS.at(m_PARAMactivateDeltaSlopeZOverS.size() - 1);
// //     } else {
// //       newPass->deltaSlopeZOverS.first =  m_PARAMactivateDeltaSlopeZOverS.at(i);
// //     }
// //     if (int (m_PARAMactivateAlwaysTrue3Hit.size()) < i + 1) {
// //       newPass->alwaysTrue3Hit.first = m_PARAMactivateAlwaysTrue3Hit.at(m_PARAMactivateAlwaysTrue3Hit.size() - 1);
// //     } else {
// //       newPass->alwaysTrue3Hit.first = m_PARAMactivateAlwaysTrue3Hit.at(i);
// //     }
// //     if (int (m_PARAMactivateAlwaysFalse3Hit.size()) < i + 1) {
// //       newPass->alwaysFalse3Hit.first = m_PARAMactivateAlwaysFalse3Hit.at(m_PARAMactivateAlwaysFalse3Hit.size() - 1);
// //     } else {
// //       newPass->alwaysFalse3Hit.first = m_PARAMactivateAlwaysFalse3Hit.at(i);
// //     }
// //     if (int (m_PARAMactivateRandom3Hit.size()) < i + 1) {
// //       newPass->random3Hit.first = m_PARAMactivateRandom3Hit.at(m_PARAMactivateRandom3Hit.size() - 1);
// //     } else {
// //       newPass->random3Hit.first = m_PARAMactivateRandom3Hit.at(i);
// //     }
// //     if (newPass->angles3D.first == true) { nfCtr++; }
// //     if (newPass->anglesXY.first == true) { nfCtr++; }
// //     if (newPass->anglesRZ.first == true) { nfCtr++; }
// //     if (newPass->deltaSlopeRZ.first == true) { nfCtr++; }
// //     if (newPass->distance2IP.first == true) { nfCtr++; }
// //     if (newPass->pT.first == true) { nfCtr++; }
// //     if (newPass->helixParameterFit.first == true) { nfCtr++; }
// //     if (newPass->deltaSOverZ.first == true) { nfCtr++; }
// //     if (newPass->deltaSlopeZOverS.first == true) { nfCtr++; }
// //     if (newPass->alwaysTrue3Hit.first == true) { nfCtr++; }
// //     if (newPass->alwaysFalse3Hit.first == true) { nfCtr++; }
// //     if (newPass->random3Hit.first == true) { nfCtr++; }
// //     B2DEBUG(2, "finished importing nFinderFilters, " << nfCtr << " filters imported")
// //     ///nFinder ho finder (3+1 hit)
// //     B2DEBUG(10, "starting import of nbFinderHioCFilters:")
// //     if (int (m_PARAMactivateDeltaPtHioC.size()) < i + 1) {
// //       newPass->deltaPtHighOccupancy.first = m_PARAMactivateDeltaPtHioC.at(m_PARAMactivateDeltaPtHioC.size() - 1);
// //     } else {
// //       newPass->deltaPtHighOccupancy.first = m_PARAMactivateDeltaPtHioC.at(i);
// //     }
// //     if (int (m_PARAMactivateDeltaDistance2IPHioC.size()) < i + 1) {
// //       newPass->deltaDistanceHighOccupancy2IP.first = m_PARAMactivateDeltaDistance2IPHioC.at(m_PARAMactivateDeltaDistance2IPHioC.size() -
// //                                                      1);
// //     } else {
// //       newPass->deltaDistanceHighOccupancy2IP.first = m_PARAMactivateDeltaDistance2IPHioC.at(i);
// //     }
// //     if (newPass->deltaPtHighOccupancy.first == true) { nfhoCtr++; }
// //     if (newPass->deltaDistanceHighOccupancy2IP.first == true) { nfhoCtr++; }
// //
// //     /// post-TCC-filter:
// //     if (int (m_PARAMactivateZigZagXY.size()) < i + 1) {
// //       newPass->zigzagXY.first = m_PARAMactivateZigZagXY.at(m_PARAMactivateZigZagXY.size() - 1);
// //     } else {
// //       newPass->zigzagXY.first = m_PARAMactivateZigZagXY.at(i);
// //     }
// //     if (int (m_PARAMactivateZigZagXYWithSigma.size()) < i + 1) {
// //       newPass->zigzagXYWithSigma.first = m_PARAMactivateZigZagXYWithSigma.at(m_PARAMactivateZigZagXYWithSigma.size() - 1);
// //     } else {
// //       newPass->zigzagXYWithSigma.first = m_PARAMactivateZigZagXYWithSigma.at(i);
// //     }
// //     if (int (m_PARAMactivateZigZagRZ.size()) < i + 1) {
// //       newPass->zigzagRZ.first = m_PARAMactivateZigZagRZ.at(m_PARAMactivateZigZagRZ.size() - 1);
// //     } else {
// //       newPass->zigzagRZ.first = m_PARAMactivateZigZagRZ.at(i);
// //     }
// //     if (int (m_PARAMactivateDeltaPt.size()) < i + 1) {
// //       newPass->deltaPt.first = m_PARAMactivateDeltaPt.at(m_PARAMactivateDeltaPt.size() - 1);
// //     } else {
// //       newPass->deltaPt.first = m_PARAMactivateDeltaPt.at(i);
// //     }
// //     if (int (m_PARAMactivateCircleFit.size()) < i + 1) {
// //       newPass->circleFit.first = m_PARAMactivateCircleFit.at(m_PARAMactivateCircleFit.size() - 1);
// //     } else {
// //       newPass->circleFit.first = m_PARAMactivateCircleFit.at(i);
// //     }
// //     if (int (m_PARAMactivateDeltaDistance2IP.size()) < i + 1) {
// //       newPass->deltaDistance2IP.first = m_PARAMactivateDeltaDistance2IP.at(m_PARAMactivateDeltaDistance2IP.size() - 1);
// //     } else {
// //       newPass->deltaDistance2IP.first = m_PARAMactivateDeltaDistance2IP.at(i);
// //     }
// //     if (int (m_PARAMactivateAlwaysTrue4Hit.size()) < i + 1) {
// //       newPass->alwaysTrue4Hit.first = m_PARAMactivateAlwaysTrue4Hit.at(m_PARAMactivateAlwaysTrue4Hit.size() - 1);
// //     } else {
// //       newPass->alwaysTrue4Hit.first = m_PARAMactivateAlwaysTrue4Hit.at(i);
// //     }
// //     if (int (m_PARAMactivateAlwaysFalse4Hit.size()) < i + 1) {
// //       newPass->alwaysFalse4Hit.first = m_PARAMactivateAlwaysFalse4Hit.at(m_PARAMactivateAlwaysFalse4Hit.size() - 1);
// //     } else {
// //       newPass->alwaysFalse4Hit.first = m_PARAMactivateAlwaysFalse4Hit.at(i);
// //     }
// //     if (int (m_PARAMactivateRandom4Hit.size()) < i + 1) {
// //       newPass->random4Hit.first = m_PARAMactivateRandom4Hit.at(m_PARAMactivateRandom4Hit.size() - 1);
// //     } else {
// //       newPass->random4Hit.first = m_PARAMactivateRandom4Hit.at(i);
// //     }
// //     if (newPass->zigzagXY.first == true) { tccfCtr++; }
// //     if (newPass->zigzagXYWithSigma.first == true) { tccfCtr++; }
// //     if (newPass->zigzagRZ.first == true) { tccfCtr++; }
// //     if (newPass->deltaPt.first == true) { tccfCtr++; }
// //     if (newPass->circleFit.first == true) { tccfCtr++; }
// //     if (newPass->deltaDistance2IP.first == true) { tccfCtr++; }
// //     if (newPass->alwaysTrue4Hit.first == true) { tccfCtr++; }
// //     if (newPass->alwaysFalse4Hit.first == true) { tccfCtr++; }
// //     if (newPass->random4Hit.first == true) { tccfCtr++; }
// //
// //     newPass->activatedSegFinderTests = sfCtr;
// //     newPass->activatedHighOccupancySegFinderTests = sfhoCtr;
// //     newPass->activatedNbFinderTests = nfCtr;
// //     newPass->activatedHighOccupancyNbFinderTests = nfhoCtr;
// //     newPass->activatedTccFilterTests = tccfCtr;
// //     B2DEBUG(2, "finished importing nFinderHioCFilters, " << tccfCtr << " filters imported")
// //     B2DEBUG(1, m_PARAMnameOfInstance << "Pass " << i << " VXD Track finder: " << sfCtr << " segFinder tests, " << nfCtr <<
// //             " friendFinder tests and " << tccfCtr << " TCC filter tests are enabled. HighOccupancy-mode activated for > " <<
// //             m_PARAMhighOccupancyThreshold << " hits per sensor");
// //
// //     B2DEBUG(10, "starting import of tuning values for filters:")
// //     if (int (m_PARAMtuneDistance3D.size()) < i + 1) {
// //       newPass->distance3D.second = m_PARAMtuneDistance3D.at(m_PARAMtuneDistance3D.size() - 1);
// //     } else {
// //       newPass->distance3D.second = m_PARAMtuneDistance3D.at(i);
// //     }
// //     if (int (m_PARAMtuneDistanceXY.size()) < i + 1) {
// //       newPass->distanceXY.second = m_PARAMtuneDistanceXY.at(m_PARAMtuneDistanceXY.size() - 1);
// //     } else {
// //       newPass->distanceXY.second = m_PARAMtuneDistanceXY.at(i);;
// //     }
// //     if (int (m_PARAMtuneDistanceZ.size()) < i + 1) {
// //       newPass->distanceZ.second = m_PARAMtuneDistanceZ.at(m_PARAMtuneDistanceZ.size() - 1);
// //     } else {
// //       newPass->distanceZ.second = m_PARAMtuneDistanceZ.at(i);
// //     }
// //     if (int (m_PARAMtuneSlopeRZ.size()) < i + 1) {
// //       newPass->slopeRZ.second = m_PARAMtuneSlopeRZ.at(m_PARAMtuneSlopeRZ.size() - 1);
// //     } else {
// //       newPass->slopeRZ.second = m_PARAMtuneSlopeRZ.at(i);
// //     }
// //     if (int (m_PARAMtuneNormedDistance3D.size()) < i + 1) {
// //       newPass->normedDistance3D.second = m_PARAMtuneNormedDistance3D.at(m_PARAMtuneNormedDistance3D.size() - 1);
// //     } else {
// //       newPass->normedDistance3D.second = m_PARAMtuneNormedDistance3D.at(i);
// //     }
// //     if (int (m_PARAMtuneAlwaysTrue2Hit.size()) < i + 1) {
// //       newPass->alwaysTrue2Hit.second = m_PARAMtuneAlwaysTrue2Hit.at(m_PARAMtuneAlwaysTrue2Hit.size() - 1);
// //     } else {
// //       newPass->alwaysTrue2Hit.second = m_PARAMtuneAlwaysTrue2Hit.at(i);
// //     }
// //     if (int (m_PARAMtuneAlwaysFalse2Hit.size()) < i + 1) {
// //       newPass->alwaysFalse2Hit.second = m_PARAMtuneAlwaysFalse2Hit.at(m_PARAMtuneAlwaysFalse2Hit.size() - 1);
// //     } else {
// //       newPass->alwaysFalse2Hit.second = m_PARAMtuneAlwaysFalse2Hit.at(i);
// //     }
// //     if (int (m_PARAMtuneRandom2Hit.size()) < i + 1) {
// //       newPass->random2Hit.second = m_PARAMtuneRandom2Hit.at(m_PARAMtuneRandom2Hit.size() - 1);
// //     } else {
// //       newPass->random2Hit.second = m_PARAMtuneRandom2Hit.at(i);
// //     }
// //     if (int (m_PARAMtuneAngles3D.size()) < i + 1) {
// //       newPass->angles3D.second = m_PARAMtuneAngles3D.at(m_PARAMtuneAngles3D.size() - 1);
// //     } else {
// //       newPass->angles3D.second =  m_PARAMtuneAngles3D.at(i);
// //     }
// //     if (int (m_PARAMtuneAnglesXY.size()) < i + 1) {
// //       newPass->anglesXY.second = m_PARAMtuneAnglesXY.at(m_PARAMtuneAnglesXY.size() - 1);
// //     } else {
// //       newPass->anglesXY.second = m_PARAMtuneAnglesXY.at(i);
// //     }
// //     if (int (m_PARAMtuneAnglesRZ.size()) < i + 1) {
// //       newPass->anglesRZ.second = m_PARAMtuneAnglesRZ.at(m_PARAMtuneAnglesRZ.size() - 1);
// //     } else {
// //       newPass->anglesRZ.second = m_PARAMtuneAnglesRZ.at(i);
// //     }
// //     if (int (m_PARAMtuneDeltaSlopeRZ.size()) < i + 1) {
// //       newPass->deltaSlopeRZ.second = m_PARAMtuneDeltaSlopeRZ.at(m_PARAMtuneDeltaSlopeRZ.size() - 1);
// //     } else {
// //       newPass->deltaSlopeRZ.second = m_PARAMtuneDeltaSlopeRZ.at(i);
// //     }
// //     if (int (m_PARAMtuneDistance2IP.size()) < i + 1) {
// //       newPass->distance2IP.second = m_PARAMtuneDistance2IP.at(m_PARAMtuneDistance2IP.size() - 1);
// //     } else {
// //       newPass->distance2IP.second =  m_PARAMtuneDistance2IP.at(i);
// //     }
// //     if (int (m_PARAMtunePT.size()) < i + 1) {
// //       newPass->pT.second = m_PARAMtunePT.at(m_PARAMtunePT.size() - 1);
// //     } else {
// //       newPass->pT.second =  m_PARAMtunePT.at(i);
// //     }
// //     if (int (m_PARAMtuneHelixParameterFit.size()) < i + 1) {
// //       newPass->helixParameterFit.second = m_PARAMtuneHelixParameterFit.at(m_PARAMtuneHelixParameterFit.size() - 1);
// //     } else {
// //       newPass->helixParameterFit.second =  m_PARAMtuneHelixParameterFit.at(i);
// //     }
// //     if (int (m_PARAMtuneDeltaSOverZ.size()) < i + 1) {
// //       newPass->deltaSOverZ.second = m_PARAMtuneDeltaSOverZ.at(m_PARAMtuneDeltaSOverZ.size() - 1);
// //     } else {
// //       newPass->deltaSOverZ.second =  m_PARAMtuneDeltaSOverZ.at(i);
// //     }
// //     if (int (m_PARAMtuneDeltaSlopeZOverS.size()) < i + 1) {
// //       newPass->deltaSlopeZOverS.second = m_PARAMtuneDeltaSlopeZOverS.at(m_PARAMtuneDeltaSlopeZOverS.size() - 1);
// //     } else {
// //       newPass->deltaSlopeZOverS.second =  m_PARAMtuneDeltaSlopeZOverS.at(i);
// //     }
// //     if (int (m_PARAMtuneAlwaysTrue3Hit.size()) < i + 1) {
// //       newPass->alwaysTrue3Hit.second = m_PARAMtuneAlwaysTrue3Hit.at(m_PARAMtuneAlwaysTrue3Hit.size() - 1);
// //     } else {
// //       newPass->alwaysTrue3Hit.second = m_PARAMtuneAlwaysTrue3Hit.at(i);
// //     }
// //     if (int (m_PARAMtuneAlwaysFalse3Hit.size()) < i + 1) {
// //       newPass->alwaysFalse3Hit.second = m_PARAMtuneAlwaysFalse3Hit.at(m_PARAMtuneAlwaysFalse3Hit.size() - 1);
// //     } else {
// //       newPass->alwaysFalse3Hit.second = m_PARAMtuneAlwaysFalse3Hit.at(i);
// //     }
// //     if (int (m_PARAMtuneRandom3Hit.size()) < i + 1) {
// //       newPass->random3Hit.second = m_PARAMtuneRandom3Hit.at(m_PARAMtuneRandom3Hit.size() - 1);
// //     } else {
// //       newPass->random3Hit.second = m_PARAMtuneRandom3Hit.at(i);
// //     }
// //     if (int (m_PARAMtuneZigZagXY.size()) < i + 1) {
// //       newPass->zigzagXY.second = m_PARAMtuneZigZagXY.at(m_PARAMtuneZigZagXY.size() - 1);
// //     } else {
// //       newPass->zigzagXY.second = m_PARAMtuneZigZagXY.at(i);
// //     }
// //     if (int (m_PARAMtuneZigZagXYWithSigma.size()) < i + 1) {
// //       newPass->zigzagXYWithSigma.second = m_PARAMtuneZigZagXYWithSigma.at(m_PARAMtuneZigZagXYWithSigma.size() - 1);
// //     } else {
// //       newPass->zigzagXYWithSigma.second = m_PARAMtuneZigZagXYWithSigma.at(i);
// //     }
// //     if (int (m_PARAMtuneZigZagRZ.size()) < i + 1) {
// //       newPass->zigzagRZ.second = m_PARAMtuneZigZagRZ.at(m_PARAMtuneZigZagRZ.size() - 1);
// //     } else {
// //       newPass->zigzagRZ.second = m_PARAMtuneZigZagRZ.at(i);
// //     }
// //     if (int (m_PARAMtuneDeltaPt.size()) < i + 1) {
// //       newPass->deltaPt.second = m_PARAMtuneDeltaPt.at(m_PARAMtuneDeltaPt.size() - 1);
// //     } else {
// //       newPass->deltaPt.second = m_PARAMtuneDeltaPt.at(i);
// //     }
// //     if (int (m_PARAMtuneDeltaDistance2IP.size()) < i + 1) {
// //       newPass->deltaDistance2IP.second = m_PARAMtuneDeltaDistance2IP.at(m_PARAMtuneDeltaDistance2IP.size() - 1);
// //     } else {
// //       newPass->deltaDistance2IP.second = m_PARAMtuneDeltaDistance2IP.at(i);
// //     }
// //     if (int (m_PARAMtuneAlwaysTrue4Hit.size()) < i + 1) {
// //       newPass->alwaysTrue4Hit.second = m_PARAMtuneAlwaysTrue4Hit.at(m_PARAMtuneAlwaysTrue4Hit.size() - 1);
// //     } else {
// //       newPass->alwaysTrue4Hit.second = m_PARAMtuneAlwaysTrue4Hit.at(i);
// //     }
// //     if (int (m_PARAMtuneAlwaysFalse4Hit.size()) < i + 1) {
// //       newPass->alwaysFalse4Hit.second = m_PARAMtuneAlwaysFalse4Hit.at(m_PARAMtuneAlwaysFalse4Hit.size() - 1);
// //     } else {
// //       newPass->alwaysFalse4Hit.second = m_PARAMtuneAlwaysFalse4Hit.at(i);
// //     }
// //     if (int (m_PARAMtuneRandom4Hit.size()) < i + 1) {
// //       newPass->random4Hit.second = m_PARAMtuneRandom4Hit.at(m_PARAMtuneRandom4Hit.size() - 1);
// //     } else {
// //       newPass->random4Hit.second = m_PARAMtuneRandom4Hit.at(i);
// //     }
// //     if (int (m_PARAMtuneCircleFit.size()) < i + 1) {
// //       newPass->circleFit.second = m_PARAMtuneCircleFit.at(m_PARAMtuneCircleFit.size() - 1);
// //     } else {
// //       newPass->circleFit.second = m_PARAMtuneCircleFit.at(i);
// //     }
// //     B2DEBUG(10, "starting import of tuning values of HioC filters:")
// //     // high occupancy cases:
// //     if (int (m_PARAMtuneAngles3DHioC.size()) < i + 1) {
// //       newPass->anglesHighOccupancy3D.second = m_PARAMtuneAngles3DHioC.at(m_PARAMtuneAngles3DHioC.size() - 1);
// //     } else {
// //       newPass->anglesHighOccupancy3D.second =  m_PARAMtuneAngles3DHioC.at(i);
// //     }
// //     if (int (m_PARAMtuneAnglesXYHioC.size()) < i + 1) {
// //       newPass->anglesHighOccupancyXY.second = m_PARAMtuneAnglesXYHioC.at(m_PARAMtuneAnglesXYHioC.size() - 1);
// //     } else {
// //       newPass->anglesHighOccupancyXY.second = m_PARAMtuneAnglesXYHioC.at(i);
// //     }
// //     if (int (m_PARAMtuneAnglesRZHioC.size()) < i + 1) {
// //       newPass->anglesHighOccupancyRZ.second = m_PARAMtuneAnglesRZHioC.at(m_PARAMtuneAnglesRZHioC.size() - 1);
// //     } else {
// //       newPass->anglesHighOccupancyRZ.second = m_PARAMtuneAnglesRZHioC.at(i);
// //     }
// //     if (int (m_PARAMtuneDeltaSlopeRZHioC.size()) < i + 1) {
// //       newPass->deltaSlopeHighOccupancyRZ.second = m_PARAMtuneDeltaSlopeRZHioC.at(m_PARAMtuneDeltaSlopeRZHioC.size() - 1);
// //     } else {
// //       newPass->deltaSlopeHighOccupancyRZ.second = m_PARAMtuneDeltaSlopeRZHioC.at(i);
// //     }
// //     if (int (m_PARAMtuneDistance2IPHioC.size()) < i + 1) {
// //       newPass->distanceHighOccupancy2IP.second = m_PARAMtuneDistance2IPHioC.at(m_PARAMtuneDistance2IPHioC.size() - 1);
// //     } else {
// //       newPass->distanceHighOccupancy2IP.second =  m_PARAMtuneDistance2IPHioC.at(i);
// //     }
// //     if (int (m_PARAMtunePTHioC.size()) < i + 1) {
// //       newPass->pTHighOccupancy.second = m_PARAMtunePTHioC.at(m_PARAMtunePTHioC.size() - 1);
// //     } else {
// //       newPass->pTHighOccupancy.second =  m_PARAMtunePTHioC.at(i);
// //     }
// //     if (int (m_PARAMtuneHelixParameterFitHioC.size()) < i + 1) {
// //       newPass->helixParameterHighOccupancyFit.second = m_PARAMtuneHelixParameterFitHioC.at(m_PARAMtuneHelixParameterFitHioC.size() - 1);
// //     } else {
// //       newPass->helixParameterHighOccupancyFit.second =  m_PARAMtuneHelixParameterFitHioC.at(i);
// //     }
// //     if (int (m_PARAMtuneDeltaPtHioC.size()) < i + 1) {
// //       newPass->deltaPtHighOccupancy.second = m_PARAMtuneDeltaPtHioC.at(m_PARAMtuneDeltaPtHioC.size() - 1);
// //     } else {
// //       newPass->deltaPtHighOccupancy.second = m_PARAMtuneDeltaPtHioC.at(i);
// //     }
// //     if (int (m_PARAMtuneDeltaDistance2IPHioC.size()) < i + 1) {
// //       newPass->deltaDistanceHighOccupancy2IP.second = m_PARAMtuneDeltaDistance2IPHioC.at(m_PARAMtuneDeltaDistance2IPHioC.size() - 1);
// //     } else {
// //       newPass->deltaDistanceHighOccupancy2IP.second = m_PARAMtuneDeltaDistance2IPHioC.at(i);
// //     }
// //
// //
// //     /// importing sectorMap including friend Information and friend specific cutoffs
// //     std::pair<int, int> countedFriendsAndCutoffs = newPass->importSectorMap(newMap->getSectorMap(), newMap->getDistances(),
// //                                                    newMap->isFilterByDistance2OriginActivated());
// //
// //     B2DEBUG(1, m_PARAMnameOfInstance << " Pass " << i << ", setup " << chosenSetup << ": importing secMap with " <<
// //             newMap->getSectorMap().size() << " sectors -> imported: " << newPass->sectorMap.size() << "/" << countedFriendsAndCutoffs.first <<
// //             "/" << countedFriendsAndCutoffs.second << " sectors/friends/(friends w/o existing filters)");
// //
// //     if (LogSystem::Instance().isLevelEnabled(LogConfig::c_Debug, 1, PACKAGENAME()) == true) {  /// printing total Map:
// //       int sectorCtr = 0, friendCtr = 0, cutoffTypesCtr = 0; // counters
// //       vector<int> currentCutOffTypes;
// //       for (auto& mapEntry : newPass->sectorMap) {  // looping through sectors
// //         VXDSector* thisSector = mapEntry.second;
// //         const vector<unsigned int> currentFriends = thisSector->getFriends();
// //         uint nFriends = currentFriends.size();
// //         B2DEBUG(25, "Opening sector " << FullSecID(mapEntry.first) << "/" << FullSecID(thisSector->getSecID()) << " at dist2Origin " <<
// //                 thisSector->getDistance() << " which has got " << nFriends << " friends for internal supportedCuttoffsList and " <<
// //                 thisSector->getFriendPointers().size() << " friendSectors as pointers");
// //         if (nFriends != uint(thisSector->getFriendMapSize()) or nFriends != thisSector->getFriendPointers().size()) {
// //           B2WARNING(" number of friends do not match in sector " << FullSecID(mapEntry.first) <<
// //                     ": friends by friendVector vs nEntries vs nFriendPointers in FriendMap: " << nFriends << "/" << thisSector->getFriendMapSize() <<
// //                     "/" << thisSector->getFriendPointers().size())
// //         }
// //
// //         for (VXDSector* aFriend : thisSector->getFriendPointers()) {
// //           uint friendInt = aFriend->getSecID();
// //           currentCutOffTypes = thisSector->getSupportedCutoffs(friendInt);
// //           for (auto cutOffType : currentCutOffTypes) { // looping through cutoffs
// //             const Belle2::Cutoff* aCutoff = thisSector->getCutoff(cutOffType, friendInt);
// //             if (aCutoff == NULL) { continue; }
// //             B2DEBUG(175, " cutoff is of type: " << FilterID().getFilterString(cutOffType) << ", min: " << aCutoff->getMinValue() << ", max:" <<
// //                     aCutoff->getMaxValue());
// //             cutoffTypesCtr++;
// //           }
// //           B2DEBUG(175, "cutoffTypesCtr got " << cutoffTypesCtr << " cutoffs")
// //           ++friendCtr;
// //         }
// //         ++sectorCtr;
// //       }
// //       B2DEBUG(1, m_PARAMnameOfInstance << " Pass " << i << ": manually counted a total of " << sectorCtr << "/" << friendCtr << "/" <<
// //               cutoffTypesCtr << " setors/friends/cutoffs in sectorMap"); // oldLevel5
// //     }
// //
// //     m_passSetupVector.push_back(newPass); /// store pass for eternity (well until end of program)
// //   }
// // }
// //
// //
// // void TFRedesignModule::importSectorMapsToDisplayCollector()
// // {
// //   /** REDESIGNCOMMENT BEGINRUN 3:
// //    * * short:
// //    * stores imported secMaps into the display-collector
// //    *
// //    ** long (+personal comments):
// //    *
// //    ** dependency of module parameters (global):
// //    *
// //    ** dependency of global in-module variables:
// //    * m_passSetupVector, m_collector,
// //    *
// //    ** dependency of global stuff just because of B2XX-output or debugging only:
// //    * m_collector
// //    *
// //    ** in-module-function-calls:
// //    */
// //
// //   //KeySectors dosn't function => so pair Int int
// //   std::vector< std::pair<std::pair<unsigned int, unsigned int>, std::vector<unsigned int> > > sectorsDisplayAllPass;
// //   std::vector<unsigned int> sectorsDisplayFriends;
// //
// //   for (uint i = 0; i < m_passSetupVector.size(); i++) {
// //     B2DEBUG(10, "PassNr. " << i << "Size of Sector Map: " << m_passSetupVector.at(i)->sectorMap.size());
// //
// //     // Read Sector Map => into map
// //     for (auto& currentSector : m_passSetupVector.at(i)->sectorMap) {
// //       sectorsDisplayFriends.clear();
// //
// //       // Friends read and store in second vector
// //       for (auto& currentFriend : currentSector.second->getFriends()) {
// //         sectorsDisplayFriends.push_back(currentFriend);
// //       }
// //
// //       sectorsDisplayAllPass.push_back({ {i, currentSector.second->getSecID()}, sectorsDisplayFriends });
// //
// //     }
// //
// //     // Init all Sectors, secConfigU & secConfigV used for PositionInfo
// //     m_collector.initSectors(sectorsDisplayAllPass, m_passSetupVector.at(i)->secConfigU, m_passSetupVector.at(i)->secConfigV);
// //
// //     for (auto infosector : m_passSetupVector.at(i)->secConfigU) {
// //       B2DEBUG(100, "InitSector secConfigU: " << infosector);
// //     }
// //
// //     for (auto infosector : m_passSetupVector.at(i)->secConfigV) {
// //       B2DEBUG(100, "InitSector secConfigV: " << infosector);
// //     }
// //   }
// // }
// //
// //
// //
// // void TFRedesignModule::InitializeInConstructor()
// // {
// //   m_chargeSignFactor = 0;
// //   m_usePXDHits = false;
// //   m_useSVDHits = false;
// //   m_nSectorSetups = -1;
// //   m_highOccupancyCase = false;
// //   m_tcThreshold = -1;
// //   m_filterOverlappingTCs = -1;
// //   m_rootFilePtr = NULL;
// //   m_treeTrackWisePtr = NULL;
// //   m_treeEventWisePtr = NULL;
// //   m_treeHitWisePtr = NULL;
// //   m_rootTimeConsumption = -1;
// //   m_rootPvalues = -1;
// //   m_rootChi2 = -1;
// //   m_rootCircleRadius = -1;
// //   m_rootNdf = -1;
// //   m_calcQiType = -1;
// //   m_calcSeedType = -1;
// //   m_aktpassNumber  = -1;
// //   m_badSectorRangeCtr = 0;
// // }
// //
// //
// //
// // //////////////////////////////////////
// // /// REDESIGN - Functions for event:
// // //////////////////////////////////////
// //
// //
// //
// // bool TFRedesignModule::initializeEvent()
// // {
// //   /** REDESIGNCOMMENT EVENT 1:
// //    * * short:
// //    * just some preparations to store Infos for debugging and other basic stuff
// //    *
// //    ** long (+personal comments):
// //    * EventInfoPackage is currently storing the info needed for the TFAnalyzerModule to determine the efficiency of the TF
// //    *
// //    ** dependency of module parameters (global):
// //    * m_PARAMdisplayCollector, m_usePXDHits, m_useSVDHits,
// //    *
// //    *
// //    ** dependency of global in-module variables:
// //    * m_eventCounter, m_collector, m_usePXDHits,
// //    * m_evInfoPack, m_TESTERnoHitsAtEvent, m_totalPXDClusters,
// //    * m_totalSVDClusters, m_badSectorRangeCtr
// //    *
// //    ** dependency of global stuff just because of B2XX-output or debugging only:
// //    * m_PARAMdisplayCollector, m_collector, m_TESTERnoHitsAtEvent,
// //    * m_totalPXDClusters, m_totalSVDClusters, m_badSectorRangeCtr
// //    *
// //    ** in-module-function-calls:
// //    * stopTFevent();
// //    */
// //
// //
// //   B2DEBUG(3, "################## entering vxd CA track finder - event " << m_eventCounter << " ######################");
// //
// //   m_eventCounter++; // we count the event number manually to get usefull results for endRun-info when activating multicore
// //
// //   m_evInfoPack.startEvent(m_eventCounter); // stores meta info like counters and time consumption for current event
// //
// //   /** cleaning will be done at the end of the event **/
// //
// //   // init once per event => Clear store Arrays
// //   if (m_PARAMdisplayCollector > 0) { m_collector.intEvent(); }
// //
// //   // if one of the following is false, the value stays at 0
// //   if (m_usePXDHits == true) { m_evInfoPack.nPXDClusters = m_pxdClusters.getEntries(); }
// //   if (m_useSVDHits == true) { m_evInfoPack.nSVDClusters = m_svdClusters.getEntries(); }
// //
// //   if ((m_evInfoPack.nPXDClusters + m_evInfoPack.nSVDClusters) == 0) {
// //
// //     B2DEBUG(3, "event: " << m_evInfoPack.evtNumber << ": there are no hits, terminating event...");
// //     m_TESTERnoHitsAtEvent++;
// //
// //     m_evInfoPack.duration.intermediateStuff += m_evInfoPack.TicToc();
// //     stopTFevent();
// //     return true;
// //   }
// //
// //   B2DEBUG(3, "event: " << m_evInfoPack.evtNumber << ": nPxdClusters: " << m_evInfoPack.nPXDClusters << ", nSvdClusters: " <<
// //           m_evInfoPack.nSVDClusters);
// //   m_totalPXDClusters += m_evInfoPack.nPXDClusters;
// //   m_totalSVDClusters += m_evInfoPack.nSVDClusters;
// //   m_badSectorRangeCtr = 0;
// //
// //   if (m_PARAMwriteToRoot == true) {
// //     for (auto& vector : m_rootGoodHitPosSMX) { vector.clear(); }
// //     for (auto& vector : m_rootGoodHitPosSMY) { vector.clear(); }
// //     for (auto& vector : m_rootGoodHitPosSMZ) { vector.clear(); }
// //     for (auto& vector : m_rootGoodHitPosSMR) { vector.clear(); }
// //
// //     for (auto& vector : m_rootGoodHitPosSFX) { vector.clear(); }
// //     for (auto& vector : m_rootGoodHitPosSFY) { vector.clear(); }
// //     for (auto& vector : m_rootGoodHitPosSFZ) { vector.clear(); }
// //     for (auto& vector : m_rootGoodHitPosSFR) { vector.clear(); }
// //
// //     for (auto& vector : m_rootGoodHitPosNFX) { vector.clear(); }
// //     for (auto& vector : m_rootGoodHitPosNFY) { vector.clear(); }
// //     for (auto& vector : m_rootGoodHitPosNFZ) { vector.clear(); }
// //     for (auto& vector : m_rootGoodHitPosNFR) { vector.clear(); }
// //
// //     for (auto& vector : m_rootGoodHitPosTCCX) { vector.clear(); }
// //     for (auto& vector : m_rootGoodHitPosTCCY) { vector.clear(); }
// //     for (auto& vector : m_rootGoodHitPosTCCZ) { vector.clear(); }
// //     for (auto& vector : m_rootGoodHitPosTCCR) { vector.clear(); }
// //
// //     for (auto& vector : m_rootGoodHitPosFTCX) { vector.clear(); }
// //     for (auto& vector : m_rootGoodHitPosFTCY) { vector.clear(); }
// //     for (auto& vector : m_rootGoodHitPosFTCZ) { vector.clear(); }
// //     for (auto& vector : m_rootGoodHitPosFTCR) { vector.clear(); }
// //
// //
// //     m_rootAllGoodHitPosSMX.clear();
// //     m_rootAllGoodHitPosSMY.clear();
// //     m_rootAllGoodHitPosSMZ.clear();
// //     m_rootAllGoodHitPosSMR.clear();
// //
// //     m_rootAllGoodHitPosSFX.clear();
// //     m_rootAllGoodHitPosSFY.clear();
// //     m_rootAllGoodHitPosSFZ.clear();
// //     m_rootAllGoodHitPosSFR.clear();
// //
// //     m_rootAllGoodHitPosNFX.clear();
// //     m_rootAllGoodHitPosNFY.clear();
// //     m_rootAllGoodHitPosNFZ.clear();
// //     m_rootAllGoodHitPosNFR.clear();
// //
// //     m_rootAllGoodHitPosTCCX.clear();
// //     m_rootAllGoodHitPosTCCY.clear();
// //     m_rootAllGoodHitPosTCCZ.clear();
// //     m_rootAllGoodHitPosTCCR.clear();
// //
// //     m_rootAllGoodHitPosFTCX.clear();
// //     m_rootAllGoodHitPosFTCY.clear();
// //     m_rootAllGoodHitPosFTCZ.clear();
// //     m_rootAllGoodHitPosFTCR.clear();
// //   }
// //
// //   return false;
// // }
// //
// //
// //
// // void TFRedesignModule::stopTFevent()
// // {
// //   m_evInfoPack.duration.totalTime += m_evInfoPack.eventTicToc();
// //
// //   if (m_PARAMwriteToRoot == true) {
// //     m_rootTimeConsumption = m_evInfoPack.duration.totalTime.count();
// //
// //     m_treeEventWisePtr->Fill();
// //   }
// //
// //   m_TESTERtimeConsumption += m_evInfoPack.duration;
// //
// //   m_TESTERlogEvents.push_back(m_evInfoPack);
// //
// //   B2DEBUG(3, "event: " << m_evInfoPack.evtNumber << ", duration : " << m_evInfoPack.duration.totalTime.count() << "ns");
// // }
// //
// //
// //
// // void TFRedesignModule::addVirtualInteractionPoint()
// // {
// //   /// Section 3a
// //   /** REDESIGNCOMMENT EVENT 2:
// //    * * short:
// //    * Section 3a - prepare/load in- and output create VXDTFhits representing the virtual IP
// //    *
// //    ** long (+personal comments):
// //    * this is the inner part of the virtual segment, which connects a hit on the innermost layer to the IP
// //    *
// //    ** dependency of module parameters (global):
// //    *
// //    ** dependency of global in-module variables:
// //    * m_passSetupVector, m_evInfoPack
// //    *
// //    ** dependency of global stuff just because of B2XX-output or debugging only:
// //    *
// //    ** in-module-function-calls:
// //    */
// //
// //   //   generating virtual Hit at position (0, 0, 0) - needed for virtual segment.
// //   PositionInfo vertexInfo;
// //   unsigned int centerSector = FullSecID().getFullSecID(); // automatically produces secID of centerSector
// //   VxdID centerVxdID = VxdID(0, 0, 0); // dummy VxdID for virtual IP
// //   int passNumber = m_passSetupVector.size() - 1;
// //
// //   BOOST_REVERSE_FOREACH(PassData * currentPass, m_passSetupVector) {
// //     currentPass->centerSector = centerSector;
// //     vertexInfo.hitPosition = currentPass->origin;
// //     if (vertexInfo.hitPosition.Mag() > 0.5) { // probably no Belle2-VXD-Setup -> Vertex not known
// //       vertexInfo.sigmaU = 1.5; //0.1;
// //       vertexInfo.sigmaV = 1.5; //0.1;
// //       vertexInfo.hitSigma.SetXYZ(1.5, 1.5, 2.5);
// //     } else {
// //       vertexInfo.sigmaU = 0.15;
// //       vertexInfo.sigmaV = 0.15;
// //       vertexInfo.hitSigma.SetXYZ(0.15, 0.15, 2.5);
// //     }
// //     VXDTFHit* vertexHit = new VXDTFHit(vertexInfo, passNumber, NULL, NULL, NULL, Const::IR, centerSector, centerVxdID,
// //                                        0.0); // has no position in HitList, because it doesn't exist...
// //     MapOfSectors::iterator secIt = currentPass->sectorMap.find(centerSector);
// //     if (secIt != currentPass->sectorMap.end()) {
// //       secIt->second->addHit(vertexHit);
// //       B2DEBUG(25, "centerSector got VxdID(input/stored): " << centerVxdID << "/" << vertexHit->getVxdID() << " and secID(input/stored): "
// //               << FullSecID() << "/" << FullSecID(vertexHit->getSectorName()) << ", coded as uInt: " << centerSector)
// //     } else {
// //       B2FATAL("Pass " << currentPass->sectorSetup << ": could not add virtual center hit!")
// //     }
// //
// //     currentPass->hitVector.push_back(vertexHit); // used for event-cleaning
// //     passNumber--;
// //   }
// //
// //   m_evInfoPack.duration.intermediateStuff += m_evInfoPack.TicToc();
// // }
// //
// //
// //
// // void TFRedesignModule::createClusterInfo()
// // {
// //   /// Section 3b
// //   /** REDESIGNCOMMENT EVENT 3:
// //    * * short:
// //    * Section 3b - store ClusterInfo for all Clusters (PXD & SVD)
// //    *
// //    ** long (+personal comments):
// //    * contains dependency of testbeam package
// //    *
// //    ** dependency of module parameters (global):
// //    * m_PARAMdisplayCollector
// //    *
// //    ** dependency of global in-module variables:
// //    * m_passSetupVector, m_collector, m_TESTERtimeConsumption,
// //    * m_evInfoPack, m_clustersOfEvent
// //    *
// //    ** dependency of global stuff just because of B2XX-output or debugging only:
// //    * m_PARAMdisplayCollector, m_collector, m_TESTERtimeConsumption,
// //    *
// //    ** in-module-function-calls:
// //    */
// //
// //   m_evInfoPack.newTic();
// //
// //   m_clustersOfEvent.resize((m_evInfoPack.nPXDClusters + m_evInfoPack.nSVDClusters), ClusterInfo());
// //
// //   for (unsigned int i = 0; i < m_evInfoPack.nPXDClusters; ++i) {
// //     m_clustersOfEvent[i] = ClusterInfo(i, i, true, false, false, m_pxdClusters[i], NULL);
// //
// //     B2DEBUG(50, "Pxd clusterInfo:  PXDcluster " << i << " in position " << i << " stores real Cluster " << m_clustersOfEvent.at(
// //               i).getRealIndex() << " at indexPosition of own list (m_clustersOfEvent): " << m_clustersOfEvent.at(
// //               i).getOwnIndex() << " withClustersOfeventSize: " << m_clustersOfEvent.size())
// //   }
// //
// //   // 1. (PXD) Import of the Clusters (Collector)
// //   if (m_PARAMdisplayCollector > 0) {
// //
// //     for (uint m = 0; m < m_passSetupVector.size(); m++) {
// //       for (unsigned int i = 0; i < m_evInfoPack.nPXDClusters; ++i) {
// //
// //         // importCluster (int passIndex, std::string diedAt, int accepted, int rejected, int detectorType, int relativePosition)
// //         int clusterid = m_collector.importCluster(m, "", CollectorTFInfo::m_idAlive, vector<int>(), vector<int>(), Const::PXD, i);
// //
// //         // CollectorID safe to connect Cluster to Cluster in Collector
// //         m_clustersOfEvent.at(i).setCollectorID(clusterid);
// //       }
// //     }
// //   }
// //
// //
// //   for (unsigned int i = 0; i < m_evInfoPack.nSVDClusters; ++i) {
// //     m_clustersOfEvent[i + m_evInfoPack.nPXDClusters] = ClusterInfo(i, i + m_evInfoPack.nPXDClusters, false, true, false, NULL,
// //                                                        m_svdClusters[i]);
// //
// //     B2DEBUG(50, "Svd clusterInfo:  SVDcluster " << i << " in position " << i + m_evInfoPack.nPXDClusters << " stores real Cluster " <<
// //             m_clustersOfEvent.at(i + m_evInfoPack.nPXDClusters).getRealIndex() << " at indexPosition of own list (m_clustersOfEvent): " <<
// //             m_clustersOfEvent.at(i + m_evInfoPack.nPXDClusters).getOwnIndex() << " withClustersOfeventSize: " << m_clustersOfEvent.size())
// //   } // the position in the vector is NOT the index it has stored (except if there are no PXDClusters)
// //
// //   // 2. (SVD) Import of the Clusters (Collector)
// //   if (m_PARAMdisplayCollector > 0) {
// //     for (uint m = 0; m < m_passSetupVector.size(); m++) {
// //
// //       for (unsigned int i = 0 + m_evInfoPack.nPXDClusters; i < m_evInfoPack.nSVDClusters + m_evInfoPack.nPXDClusters; ++i) {
// //
// //         // importCluster (int passIndex, std::string diedAt, int accepted, int rejected, int detectorType, int relativePosition)
// //         int clusterid = m_collector.importCluster(m, "", CollectorTFInfo::m_idAlive, vector<int>(), vector<int>(), Const::SVD, i);
// //
// //         // CollectorID safe to connect Cluster to Cluster in Collector
// //         m_clustersOfEvent.at(i).setCollectorID(clusterid);
// //       }
// //     }
// //   }
// //
// //   m_evInfoPack.duration.hitSorting += m_evInfoPack.TicToc();
// // }
// //
// //
// //
// // void TFRedesignModule::assignPXDHitsToSectors()
// // {
// //   /// Section 3d
// //   /** REDESIGNCOMMENT EVENT 5:
// //   * * short:
// //   * Section 3d - assign sectors for PXD Clusters
// //   * Creates VXDTFHits
// //   *
// //   ** long (+personal comments):
// //   *
// //   ** dependency of module parameters (global):
// //   * m_PARAMdisplayCollector,
// //   *
// //   ** dependency of global in-module variables:
// //   * m_passSetupVector, m_TESTERbadSensors, m_collector,
// //   * m_evInfoPack
// //   *
// //   ** dependency of global stuff just because of B2XX-output or debugging only:
// //   * m_eventCounter, m_TESTERbadSensors, m_PARAMdisplayCollector,
// //   * m_collector,
// //   *
// //   ** in-module-function-calls:
// //   * searchSector4Hit(aVxdID, hitLocal, currentPass->sectorMap, currentPass->secConfigU, currentPass->secConfigV)
// //   */
// //
// //   B2DEBUG(3, "VXDTF event " << m_eventCounter << ": size of arrays, PXDCluster: " << m_evInfoPack.nPXDClusters << ", SVDCLuster: " <<
// //           m_evInfoPack.nSVDClusters << ", m_clustersOfEvent: " << m_clustersOfEvent.size());
// //
// //   TVector3 hitLocal, hitSigma;
// //   PositionInfo hitInfo;
// //   FullSecID aSecID;
// //   VxdID aVxdID;
// //   m_evInfoPack.newTic();
// //   VxdID::baseType aLayerID;
// //
// //
// //   /** searching sectors for hit:*/
// //
// //   for (unsigned int iPart = 0; iPart < m_evInfoPack.nPXDClusters;
// //        ++iPart) { /// means: m_evInfoPack.nPXDClusters > 0 if at least one pass wants PXD hits
// //     const PXDCluster* const aClusterPtr = m_pxdClusters[iPart];
// //
// //     B2DEBUG(100, " pxdCluster has clusterIndexUV: " << iPart
// //             << " with collected charge: " << aClusterPtr->getCharge()
// //             << " and their infoClass is at: " << iPart << " with collected charge: "
// //             << m_pxdClusters[iPart]->getCharge())
// //
// //     hitLocal.SetXYZ(aClusterPtr->getU(), aClusterPtr->getV(), 0);
// //     hitSigma.SetXYZ(aClusterPtr->getUSigma(), aClusterPtr->getVSigma(), 0);
// //
// //     aVxdID = aClusterPtr->getSensorID();
// //     aLayerID = aVxdID.getLayerNumber();
// //     VXD::SensorInfoBase aSensorInfo = VXD::GeoCache::get(aVxdID);
// //     hitInfo.hitPosition = aSensorInfo.pointToGlobal(hitLocal);
// //     hitInfo.sigmaU = aClusterPtr->getUSigma();
// //     hitInfo.sigmaV = aClusterPtr->getVSigma();
// //     hitInfo.hitSigma = aSensorInfo.vectorToGlobal(hitSigma);
// //     B2DEBUG(100, " pxdluster got global pos X/Y/Z: " << hitInfo.hitPosition.X() << "/" << hitInfo.hitPosition.Y() << "/" <<
// //             hitInfo.hitPosition.Z() << ", global var X/Y/Z: " << hitInfo.hitSigma.X() << "/" << hitInfo.hitSigma.Y() << "/" <<
// //             hitInfo.hitSigma.Z())
// //
// //     B2DEBUG(175, "local pxd hit coordinates (u,v): (" << hitLocal[0] << "," << hitLocal[1] << ")");
// //
// //     int passNumber = 0;
// //     for (PassData* currentPass : m_passSetupVector) {
// //
// //       if (currentPass->usePXDHits == false) { continue; }  // PXD is included in 0 & -1
// //       if (aLayerID > currentPass->highestAllowedLayer) { continue; }   // skip particle if True
// //       SectorNameAndPointerPair activatedSector = searchSector4Hit(aVxdID,
// //                                                                   hitLocal,
// //                                                                   currentPass->sectorMap,
// //                                                                   currentPass->secConfigU,
// //                                                                   currentPass->secConfigV);
// //
// //
// //       aSecID = activatedSector.first;
// //       MapOfSectors::iterator secMapIter = activatedSector.second;
// //
// //       if (aSecID.getFullSecID() == numeric_limits<unsigned int>::max()) {
// //         B2DEBUG(10, "VXDTF - event " << m_eventCounter << ": pxdhit with vxdID/layerID " << aVxdID << "/" << aLayerID <<
// //                 " out of sector range (setup " << currentPass->sectorSetup << ", type  " << currentPass->chosenDetectorType << "). (" << FullSecID(
// //                   activatedSector.first) << " does not exist) Discarding hit...");
// //         m_badSectorRangeCtr++;
// //         m_TESTERbadSensors.push_back(FullSecID(aSecID).getFullSecString());
// //
// //         // importHits 2.
// //         if (m_PARAMdisplayCollector > 0) {
// //
// //           std::vector<int> assignedIDs = { m_clustersOfEvent.at(iPart).getCollectorID() };
// //
// //           int hitID = m_collector.importHit(passNumber, CollectorTFInfo::m_nameHitFinder, CollectorTFInfo::m_idHitFinder, vector<int>(),
// //                                             vector<int>(), assignedIDs, aSecID, hitInfo.hitPosition, hitInfo.hitSigma);
// //
// //           // no hitid is saved because Hit is not used
// //           B2DEBUG(100, "Hit imported = died at hitfinder: " << hitID);
// //
// //         }
// //
// //         continue;
// //       }
// //
// //       B2DEBUG(50, " PXDCluster: with posOfHit in StoreArray: " << iPart << " is found again within FullsecID " << aSecID <<
// //               " using sectorSetup " << currentPass->sectorSetup);
// //
// //       VXDTFHit* pTFHit = new VXDTFHit(hitInfo, passNumber, NULL, NULL, &m_clustersOfEvent[iPart], Const::PXD, aSecID, aVxdID,
// //                                       0.0); // no timeInfo for PXDHits
// //
// //       // importHits 3.
// //       if (m_PARAMdisplayCollector > 0) {
// //
// //         std::vector<int> assignedIDs = { m_clustersOfEvent.at(iPart).getCollectorID() };
// //
// //         int hitId = m_collector.importHit(passNumber, "", CollectorTFInfo::m_idAlive, vector<int>(), vector<int>(), assignedIDs, aSecID,
// //                                           hitInfo.hitPosition, hitInfo.hitSigma);
// //
// //         // Connection Hit <=> Hit in Collector
// //         pTFHit->setCollectorID(hitId);
// //
// //         B2DEBUG(100, "Coor asecid: " << aSecID << ", position Hit: " << hitInfo.hitPosition.X() << "/" << hitInfo.hitPosition.Y() << "/" <<
// //                 hitInfo.hitPosition.Z() << ", uCoord: " << hitLocal.X() << ", vCoord: " << hitLocal.Y());
// //
// //       }
// //
// //       currentPass->hitVector.push_back(pTFHit);
// //       secMapIter->second->addHit(pTFHit);
// //       currentPass->sectorVector.push_back(secMapIter->second);// should be a pointer to a sector
// //
// //       B2DEBUG(150, "size of sectorVector/hitVector: " << currentPass->sectorVector.size() << "/" << currentPass->hitVector.size());
// //       passNumber++;
// //     }
// //   }
// //   m_evInfoPack.duration.hitSorting += m_evInfoPack.TicToc();
// // }
// //
// //
// //
// // void TFRedesignModule::assignSVDHitsToSectors()
// // {
// //   /// Section 3d part 2
// //   /** REDESIGNCOMMENT EVENT 5b:
// //    * * short:
// //    * Section 3d part 2 - find 2-SVDCluster-Combis and assign sectors to them.
// //    * Creates VXDTFHits
// //    *
// //    ** long (+personal comments):
// //    *
// //    ** dependency of module parameters (global):
// //    * m_PARAMdisplayCollector, m_PARAMnameOfInstance, m_PARAMkillEventForHighOccupancyThreshold,
// //    *
// //    ** dependency of global in-module variables:
// //    * m_passSetupVector, m_TESTERbadSensors, m_collector,
// //    * m_TESTERdistortedHitCtr, m_badSectorRangeCounter, m_totalSVDClusterCombis,
// //    * m_evInfoPack
// //    *
// //    ** dependency of global stuff just because of B2XX-output or debugging only:
// //    * m_eventCounter, m_TESTERbadSensors, m_PARAMdisplayCollector,
// //    * m_collector, m_PARAMnameOfInstance, m_TESTERdistortedHitCtr,
// //    * m_badSectorRangeCounter, m_totalSVDClusterCombis,
// //    *
// //    ** in-module-function-calls:
// //    * searchSector4Hit(aVxdID, hitLocal, currentPass->sectorMap, currentPass->secConfigU, currentPass->secConfigV)
// //    * findSensors4Clusters(activatedSensors, m_clustersOfEvent)
// //    * find2DSVDHits(activatedSensors, clusterHitList)
// //    * stopBrokenEvent()
// //    */
// //   m_evInfoPack.newTic();
// //
// //   TVector3 hitLocal, hitSigma;
// //   PositionInfo hitInfo;
// //   double vSize, uSizeAtHit, uCoord, vCoord;
// //   FullSecID aSecID;
// //   VxdID aVxdID;
// //   VxdID::baseType aLayerID;
// //
// //   ActiveSensorsOfEvent activatedSensors; // mapEntry.first: vxdID, mapEntry.second SensorStruct having some extra info
// //
// //   vector<ClusterHit> clusterHitList;
// //
// //   // store each cluster (as a clusterPtr) in a map(uniID, sensorStruct), where sensorStruct contains 2 vectors (uClusters and vClusters).
// //   // in the end a map containing illuminated sensors - and each cluster inhabiting them - exists:
// //   findSensors4Clusters(activatedSensors, m_clustersOfEvent);                                    ///findSensors4Clusters
// //
// //   // iterate through map of activated sensors & combine each possible combination of clusters. Store them in a vector of structs, where each struct carries an u & a v cluster:
// //   BrokenSensorsOfEvent brokenSensors = find2DSVDHits(activatedSensors, clusterHitList);       /// find2DSVDHits
// //
// //   if (brokenSensors.empty()) {
// //     /// TODO: in this case, we should build 1D-clusters
// //     if (LogSystem::Instance().isLevelEnabled(LogConfig::c_Debug, 3, PACKAGENAME()) == true) {
// //       stringstream badSensors;
// //       for (unsigned int sensorID : brokenSensors) { badSensors << " " << FullSecID(VxdID(sensorID)/*, false, 0*/); }
// //       B2DEBUG(3, m_PARAMnameOfInstance << " - event " << m_eventCounter << ": there are " << brokenSensors.size() <<
// //               " sensors with strange cluster-behavior (num of u and v clusters does not mach), sensors are: \n" << badSensors.str())
// //     }
// //   }
// //
// //   unsigned int nClusterCombis = clusterHitList.size();
// //
// //   if (clusterHitList.size() > m_PARAMkillEventForHighOccupancyThreshold) {
// //     B2DEBUG(1, m_PARAMnameOfInstance << " event " << m_eventCounter << ": total number of clusterCombis: " << nClusterCombis <<
// //             ", terminating event! There were " << m_evInfoPack.nPXDClusters << "/" << m_evInfoPack.nSVDClusters << "/" << nClusterCombis <<
// //             " TEL-/PXD-/SVD-clusters/SVD-cluster-combinations.")
// //     stopBrokenEvent();
// //     m_evInfoPack.duration.hitSorting += m_evInfoPack.TicToc();
// //     return;
// //   }
// //
// //   for (ClusterHit& aClusterCombi : clusterHitList) {
// //     ClusterInfo* uClusterInfo = aClusterCombi.uCluster;
// //     ClusterInfo* vClusterInfo = aClusterCombi.vCluster;
// //     const SVDCluster* uClusterPtr = uClusterInfo->getSVDCluster(); // uCluster
// //     const SVDCluster* vClusterPtr = vClusterInfo->getSVDCluster(); // vCluster
// //     int clusterIndexU = uClusterInfo->getRealIndex();
// //     int clusterIndexV = vClusterInfo->getRealIndex();
// //
// //     float timeStampU = uClusterPtr->getClsTime();
// //     float timeStampV = vClusterPtr->getClsTime();
// //
// //     B2DEBUG(100, " svdClusterCombi has clusterIndexU/clusterIndexV: " << clusterIndexU << "/" << clusterIndexV <<
// //             " with collected charge u/v: " << uClusterPtr->getCharge() << "/" << vClusterPtr->getCharge() <<
// //             " and their infoClasses are at u/v: " << uClusterInfo->getOwnIndex() << "/" << vClusterInfo->getOwnIndex() <<
// //             " with collected charge u/v: " << m_svdClusters[clusterIndexU]->getCharge() << "/" << m_svdClusters[clusterIndexV]->getCharge())
// //
// //     aVxdID = uClusterPtr->getSensorID();
// //     aLayerID = aVxdID.getLayerNumber();
// //     VXD::SensorInfoBase aSensorInfo = VXD::GeoCache::get(aVxdID);
// //     if ((aSensorInfo.getBackwardWidth() > aSensorInfo.getForwardWidth()) == true) {   // isWedgeSensor
// //       hitLocal.SetX(SpacePoint::getUWedged({uClusterPtr->getPosition(), vClusterPtr->getPosition()}, aVxdID, &aSensorInfo));
// // //       hitLocal.SetX((aSensorInfo.getWidth(vClusterPtr->getPosition()) / aSensorInfo.getWidth(0)) * uClusterPtr->getPosition());
// //     } else { // rectangular Sensor
// //       hitLocal.SetX(uClusterPtr->getPosition());
// //     }
// //     hitLocal.SetY(vClusterPtr->getPosition()); // always correct
// //     hitLocal.SetZ(0.);
// //     hitSigma.SetXYZ(uClusterPtr->getPositionSigma(), vClusterPtr->getPositionSigma(), 0);
// //
// //     hitInfo.hitPosition = aSensorInfo.pointToGlobal(hitLocal);
// //     hitInfo.hitSigma = aSensorInfo.vectorToGlobal(hitSigma);
// //     hitInfo.sigmaU = uClusterPtr->getPositionSigma();
// //     hitInfo.sigmaV = vClusterPtr->getPositionSigma();
// //
// //     B2DEBUG(100, "svdClusterCombi got global pos X/Y/Z: " << hitInfo.hitPosition.X() << "/" << hitInfo.hitPosition.Y() << "/" <<
// //             hitInfo.hitPosition.Z() << ", global var X/Y/Z: " << hitInfo.hitSigma.X() << "/" << hitInfo.hitSigma.Y() << "/" <<
// //             hitInfo.hitSigma.Z())
// //
// //     // local(0,0,0) is the center of the sensorplane
// //     vSize = 0.5 * aSensorInfo.getVSize();
// //     uSizeAtHit = 0.5 * aSensorInfo.getUSize(hitLocal[1]);
// //
// //     uCoord = hitLocal[0] + uSizeAtHit; // *0,5 putting (0,0) from the center to the edge of the plane (considers the trapeziodal shape)
// //     vCoord = hitLocal[1] + vSize;
// //
// //     if (uCoord < 0.) {   /// if Hit is not in sensorRange, move it back to the range
// //       uCoord = 0. + 0.0001;
// //       m_TESTERdistortedHitCtr++;
// //     } else if (uCoord > uSizeAtHit * 2.) {
// //       uCoord = uSizeAtHit * 2. - 0.0001;
// //       m_TESTERdistortedHitCtr++;
// //     }
// //     if (vCoord < 0.) {
// //       vCoord = 0. + 0.0001;
// //       m_TESTERdistortedHitCtr++;
// //     } else if (vCoord > vSize * 2.) {
// //       vCoord = vSize * 2. - 0.0001;
// //       m_TESTERdistortedHitCtr++;
// //     }
// //
// //     B2DEBUG(175, "local svd hit coordinates (u,v): (" << hitLocal[0] << "," << hitLocal[1] << ")");
// //
// //     int passNumber = 0;
// //     for (PassData* currentPass : m_passSetupVector) {
// //       if (aLayerID > currentPass->highestAllowedLayer) {
// //
// //         // importHits 4.
// //         if (m_PARAMdisplayCollector > 0) {
// //
// //           std::vector<int> assignedIDs = { clusterIndexU, clusterIndexV };
// //
// //           std::vector<int> rejectedFilters = {FilterID::overHighestAllowedLayer};
// //
// //           int hitID = m_collector.importHit(passNumber, CollectorTFInfo::m_nameHitFinder, CollectorTFInfo::m_idHitFinder, vector<int>(),
// //                                             rejectedFilters, assignedIDs, -1, hitInfo.hitPosition, hitInfo.hitSigma);
// //
// //           // no hitid is saved because Hit is not used
// //           B2DEBUG(100, "Hit imported = died at hitfinder (highestAllowedLayer): " << hitID);
// //         }
// //
// //         continue;
// //       }   // skip particle if True
// //       if (currentPass->useSVDHits == false) { continue; }   // SVD is included in 1 & -1, but not in 0
// //
// //
// //       SectorNameAndPointerPair activatedSector = searchSector4Hit(aVxdID,
// //                                                                   hitLocal,
// //                                                                   currentPass->sectorMap,
// //                                                                   currentPass->secConfigU,
// //                                                                   currentPass->secConfigV);
// //
// //
// //       aSecID = activatedSector.first;
// //
// //       MapOfSectors::iterator secMapIter =  activatedSector.second;
// //
// //       if (aSecID.getFullSecID() == numeric_limits<unsigned int>::max())  {
// //         B2DEBUG(10, "VXDTF - event " << m_eventCounter << ": svdhit with vxdID/layerID " << aVxdID << "/" << aLayerID <<
// //                 " out of sector range(setup " << currentPass->sectorSetup << ", type  " << currentPass->chosenDetectorType << "). (SecID " << aSecID
// //                 << " does not exist) Discarding hit...");
// //         m_badSectorRangeCtr++;
// //         m_TESTERbadSensors.push_back(FullSecID(aSecID).getFullSecString());
// //
// //         // importHits 5.
// //         if (m_PARAMdisplayCollector > 0) {
// //
// //           std::vector<int> assignedIDs = { clusterIndexU, clusterIndexV };
// //
// //           std::vector<int> rejectedFilters = {FilterID::outOfSectorRange};
// //
// //           int hitID = m_collector.importHit(passNumber, CollectorTFInfo::m_nameHitFinder, CollectorTFInfo::m_idHitFinder, vector<int>(),
// //                                             rejectedFilters, assignedIDs, aSecID, hitInfo.hitPosition, hitInfo.hitSigma);
// //
// //           // no hitid is saved because Hit is not used
// //           B2DEBUG(100, "Hit imported = died at hitfinder: " << hitID);
// //         }
// //
// //         continue;
// //       }
// //
// //       B2DEBUG(50, "A SVDCluster is found again within secID " << aSecID << " using sectorSetup " << currentPass->sectorSetup);
// //
// //       VXDTFHit* pTFHit = new VXDTFHit(hitInfo, passNumber, uClusterInfo, vClusterInfo, NULL, Const::SVD, aSecID, aVxdID,
// //                                       0.5 * (timeStampU + timeStampV));
// //
// //       // importHits 6.
// //       if (m_PARAMdisplayCollector > 0) {
// //
// //         std::vector<int> assignedIDs = { clusterIndexU, clusterIndexV };
// //
// //         int hit_id =  m_collector.importHit(passNumber, "", CollectorTFInfo::m_idAlive, vector<int>(), vector<int>(), assignedIDs, aSecID,
// //                                             hitInfo.hitPosition, hitInfo.hitSigma);
// //
// //         // Connect Hit <=> Collector Hit
// //         pTFHit->setCollectorID(hit_id);
// //
// //         B2DEBUG(100, "Coord aSecId: " << aSecID << ", position Hit: " << hitInfo.hitPosition.X() << "/" << hitInfo.hitPosition.Y() << "/" <<
// //                 hitInfo.hitPosition.Z() << ", uCoord: " << uCoord << ", vCoord: " << vCoord << ", aVxdID: " << aVxdID.getID());
// //
// //         B2DEBUG(100, "getUSize: " << aSensorInfo.getUSize() << ", getVSize: " << aSensorInfo.getVSize() << ", getThickness: " <<
// //                 aSensorInfo.getThickness());
// //       }
// //
// //
// //       currentPass->hitVector.push_back(pTFHit);
// //       secMapIter->second->addHit(pTFHit);
// //       currentPass->sectorVector.push_back(secMapIter->second);// should be a pointer to a sector
// //
// //       B2DEBUG(150, "size of sectorVector: " << currentPass->sectorVector.size() << "size of hitVector: " <<
// //               currentPass->hitVector.size());
// //       passNumber++;
// //     }
// //   }
// //
// //   m_badSectorRangeCounter += m_badSectorRangeCtr;
// //   m_totalSVDClusterCombis += nClusterCombis;
// //   m_evInfoPack.numSVDHits += nClusterCombis;
// //
// //   m_evInfoPack.duration.hitSorting += m_evInfoPack.TicToc();
// // }
// //
// //
// //
// // void TFRedesignModule::preparePass(PassData* currentPass, int passNumber)
// // {
// //   /** REDESIGNCOMMENT EVENT 5c:
// //    * * short:
// //    * prepares the pass dependent stuff
// //    *
// //    ** long (+personal comments):
// //    *
// //    ** dependency of module parameters (global):
// //    * m_PARAMdisplayCollector,
// //    *
// //    ** dependency of global in-module variables:
// //    * m_evInfoPack
// //    *
// //    ** dependency of global stuff just because of B2XX-output or debugging only:
// //    * m_PARAMdisplayCollector
// //    *
// //    ** in-module-function-calls:
// //    */
// //
// //   m_evInfoPack.newTic();
// //
// //   // Set current Pass Number for Collector
// //   if (m_PARAMdisplayCollector > 0) { m_aktpassNumber = passNumber; }
// //
// //   B2DEBUG(10, "Pass " << passNumber << ": sectorVector has got " << currentPass->sectorVector.size() <<
// //           " entries before applying unique & sort");
// //
// //   // inverse sorting and removing unique entries so we can get a list of activated sectors where the outermost sector is the first in the list:
// //   std::sort(currentPass->sectorVector.begin(), currentPass->sectorVector.end(),
// //   [&](const VXDSector * secA, const VXDSector * secB) { return *secA > *secB; });
// //   auto newEndOfVector = std::unique(currentPass->sectorVector.begin(), currentPass->sectorVector.end(),
// //   [&](const VXDSector * secA, const VXDSector * secB) { return *secA == *secB; });
// //   currentPass->sectorVector.resize(std::distance(currentPass->sectorVector.begin(), newEndOfVector));
// //
// //   if (LogSystem::Instance().isLevelEnabled(LogConfig::c_Debug, 10, PACKAGENAME()) == true) {
// //     stringstream infoStream;
// //     for (const VXDSector* aSector : currentPass->sectorVector) {
// //       infoStream << FullSecID(aSector->getSecID()) << ": " << aSector->getHits().size() << ",  ";
// //     }
// //     B2DEBUG(10, "Before sf: have got " << currentPass->hitVector.size() <<  " hits in sectors: " << endl << infoStream.str())
// //   }
// //
// //   if (m_PARAMwriteToRoot == true) {
// //     for (const VXDTFHit* hit : currentPass->hitVector) {
// //       if (hit->getVxdID().getLayerNumber() == 0) {continue; }
// //       m_rootGoodHitPosSMX.at(hit->getVxdID().getLayerNumber() - 1).push_back(hit->X());
// //       m_rootGoodHitPosSMY.at(hit->getVxdID().getLayerNumber() - 1).push_back(hit->Y());
// //       m_rootGoodHitPosSMZ.at(hit->getVxdID().getLayerNumber() - 1).push_back(hit->Z());
// //       m_rootGoodHitPosSMR.at(hit->getVxdID().getLayerNumber() - 1).push_back(hit->Perp());
// //
// //       m_rootAllGoodHitPosSMX.push_back(hit->X());
// //       m_rootAllGoodHitPosSMY.push_back(hit->Y());
// //       m_rootAllGoodHitPosSMZ.push_back(hit->Z());
// //       m_rootAllGoodHitPosSMR.push_back(hit->Perp());
// //     }
// //   }
// //
// //   m_evInfoPack.duration.hitSorting += m_evInfoPack.TicToc();
// // }
// //
// //
// //
// // bool TFRedesignModule::doTheSegFinder(PassData* currentPass, int passNumber)
// // {
// //   /** REDESIGNCOMMENT EVENT 6:
// //    * * short:
// //    * Section 4 - for each activated sector: combine compatible ones and create segments for each accepted combination
// //    *
// //    ** long (+personal comments):
// //    * several TFs will use that section with totally the same interface, therefore generalized approach important (function/class)
// //    *
// //    ** dependency of module parameters (global):
// //    * m_PARAMkillEventForHighOccupancyThreshold, m_PARAMnameOfInstance,
// //    *
// //    ** dependency of global in-module variables:
// //    * m_TESTERtotalsegmentsSFCtr, m_TESTERdiscardedSegmentsSFCtr, m_eventCounter,
// //    * m_evInfoPack
// //    *
// //    ** dependency of global stuff just because of B2XX-output or debugging only:
// //    * m_TESTERtotalsegmentsSFCtr, m_TESTERdiscardedSegmentsSFCtr, m_eventCounter,
// //    * m_PARAMnameOfInstance,
// //    *
// //    ** in-module-function-calls:
// //    * segFinder(currentPass)
// //    * stopBrokenEvent()
// //    */
// //   m_evInfoPack.newTic();
// //   bool killEvent = false;
// //
// //   B2DEBUG(3, "Pass #" << passNumber << " - " << currentPass->sectorSetup << ": " << currentPass->sectorVector.size() <<
// //           " sectors(sectorVector) activated with total of " << currentPass->hitVector.size() << " VXDTFhits, -> starting segFinder...");
// //
// //   unsigned int discardedSegments = segFinder(currentPass);                             /// calling funtion "segFinder"
// //   unsigned int activatedSegments = currentPass->activeCellList.size();
// //   m_TESTERtotalsegmentsSFCtr += activatedSegments;
// //   m_TESTERdiscardedSegmentsSFCtr += discardedSegments;
// //   B2DEBUG(3, "VXDTF-event " << m_eventCounter << ", pass" << passNumber << " @ segfinder - " << activatedSegments <<
// //           " segments activated, " << discardedSegments << " discarded");
// //   m_evInfoPack.segFinderActivated += activatedSegments;
// //   m_evInfoPack.segFinderDiscarded += discardedSegments;
// //   m_evInfoPack.numHitCombisTotal += (activatedSegments + discardedSegments);
// //
// //   m_evInfoPack.duration.segFinder += m_evInfoPack.TicToc();
// //
// //   if (m_PARAMwriteToRoot == true) {
// //     for (const VXDTFHit* hit : currentPass->hitVector) {
// //       if (hit->getNumberOfSegments() < 1 or hit->getVxdID().getLayerNumber() == 0) { continue; }
// //       m_rootGoodHitPosSFX.at(hit->getVxdID().getLayerNumber() - 1).push_back(hit->X());
// //       m_rootGoodHitPosSFY.at(hit->getVxdID().getLayerNumber() - 1).push_back(hit->Y());
// //       m_rootGoodHitPosSFZ.at(hit->getVxdID().getLayerNumber() - 1).push_back(hit->Z());
// //       m_rootGoodHitPosSFR.at(hit->getVxdID().getLayerNumber() - 1).push_back(hit->Perp());
// //
// //       m_rootAllGoodHitPosSFX.push_back(hit->X());
// //       m_rootAllGoodHitPosSFY.push_back(hit->Y());
// //       m_rootAllGoodHitPosSFZ.push_back(hit->Z());
// //       m_rootAllGoodHitPosSFR.push_back(hit->Perp());
// //     }
// //   }
// //
// //   if (activatedSegments > m_PARAMkillEventForHighOccupancyThreshold) {
// //     B2DEBUG(1, m_PARAMnameOfInstance << " event " << m_eventCounter << ": total number of activated segments: " << activatedSegments <<
// //             ", terminating event! There were " << m_evInfoPack.nPXDClusters << "/" << m_evInfoPack.nSVDClusters << "/" <<
// //             m_evInfoPack.numSVDHits << " PXD-/SVD-clusters/SVD-cluster-combinations.")
// //
// //     stopBrokenEvent();
// //
// //     killEvent = true;
// //   }
// //   return killEvent;
// // }
// //
// //
// //
// // void TFRedesignModule::stopBrokenEvent()
// // {
// //   m_TESTERbrokenEventsCtr++;
// //
// //   /** cleaning part **/
// //   for (PassData* currentPass : m_passSetupVector) {
// //     cleanEvent(currentPass);
// //   }
// //   stopTFevent();
// // }
// //
// //
// //
// // bool TFRedesignModule::doTheNeighbourFinder(PassData* currentPass, int passNumber)
// // {
// //   /** REDESIGNCOMMENT EVENT 7:
// //   * * short:
// //   * Section 5 - neighbourfinder, checks segments sharing a center hit for valid combinations.
// //   * Valid ones are flagged as neighbours and outer segments get a state upgrade (for CA)
// //   *
// //   ** long (+personal comments):
// //   * Like for the segFinder, several different TFs/Modules want to access that part with the same interface
// //   *
// //   ** dependency of module parameters (global):
// //   * m_PARAMkillEventForHighOccupancyThreshold, m_PARAMnameOfInstance
// //   *
// //   ** dependency of global in-module variables:
// //   * m_TESTERtotalsegmentsNFCtr, m_TESTERdiscardedSegmentsNFCtr, m_eventCounter,
// //   * m_evInfoPack
// //   *
// //   ** dependency of global stuff just because of B2XX-output or debugging only:
// //   * m_TESTERtotalsegmentsNFCtr, m_TESTERdiscardedSegmentsNFCtr, m_PARAMnameOfInstance,
// //   * m_eventCounter,
// //   *
// //   ** in-module-function-calls:
// //   * neighbourFinder(currentPass)
// //   * stopBrokenEvent()
// //   */
// //   m_evInfoPack.newTic();
// //   bool killEvent = false;
// //
// //   B2DEBUG(5, "pass " << passNumber << ": starting neighbourFinder...");
// //   int discardedSegments = neighbourFinder(currentPass);                       /// calling funtion "neighbourFinder"
// //   unsigned int activatedSegments = currentPass->activeCellList.size();
// //   m_TESTERtotalsegmentsNFCtr += activatedSegments;
// //   m_TESTERdiscardedSegmentsNFCtr += discardedSegments;
// //   B2DEBUG(3, "VXDTF-event " << m_eventCounter << ", pass" << passNumber << " @ nbfinder - " << activatedSegments <<
// //           " segments activated, " << discardedSegments << " discarded");
// //   m_evInfoPack.nbFinderActivated += activatedSegments;
// //   m_evInfoPack.nbFinderDiscarded += discardedSegments;
// //
// //   m_evInfoPack.duration.nbFinder += m_evInfoPack.TicToc();
// //
// //   if (m_PARAMwriteToRoot == true) {
// //     for (const VXDTFHit* hit : currentPass->hitVector) {
// //       if (hit->getVxdID().getLayerNumber() == 0) {continue; }
// //       int cellsAlive = 0;
// //       for (int index : hit->getAttachedInnerCell()) {
// //         cellsAlive += currentPass->totalCellVector.at(index)->sizeOfOuterNeighbours();
// //         cellsAlive += currentPass->totalCellVector.at(index)->sizeOfInnerNeighbours();
// //       }
// //       if (cellsAlive < 1) { continue; }
// //       m_rootGoodHitPosNFX.at(hit->getVxdID().getLayerNumber() - 1).push_back(hit->X());
// //       m_rootGoodHitPosNFY.at(hit->getVxdID().getLayerNumber() - 1).push_back(hit->Y());
// //       m_rootGoodHitPosNFZ.at(hit->getVxdID().getLayerNumber() - 1).push_back(hit->Z());
// //       m_rootGoodHitPosNFR.at(hit->getVxdID().getLayerNumber() - 1).push_back(hit->Perp());
// //
// //       m_rootAllGoodHitPosNFX.push_back(hit->X());
// //       m_rootAllGoodHitPosNFY.push_back(hit->Y());
// //       m_rootAllGoodHitPosNFZ.push_back(hit->Z());
// //       m_rootAllGoodHitPosNFR.push_back(hit->Perp());
// //     }
// //   }
// //
// //   if (activatedSegments > m_PARAMkillEventForHighOccupancyThreshold) {
// //     B2DEBUG(1, m_PARAMnameOfInstance << " event " << m_eventCounter << ": after nbFinder, total number of activated segments: " <<
// //             activatedSegments << ", terminating event! There were " << m_evInfoPack.nPXDClusters << "/" << m_evInfoPack.nSVDClusters << "/" <<
// //             m_evInfoPack.numSVDHits << " PXD-/SVD-clusters/SVD-cluster-combinations.")
// //     stopBrokenEvent();
// //     killEvent = true;
// //   }
// //   return killEvent;
// // }
// //
// //
// //
// // bool TFRedesignModule::doTheCellularAutomaton(PassData* currentPass, int passNumber)
// // {
// //   /** REDESIGNCOMMENT EVENT 8:
// //   * * short:
// //   * Section 6 - Cellular Automaton
// //   *
// //   ** long (+personal comments):
// //   *
// //   ** dependency of module parameters (global):
// //   * m_PARAMnameOfInstance,
// //   *
// //   ** dependency of global in-module variables:
// //   * m_eventCounter, m_evInfoPack
// //   *
// //   ** dependency of global stuff just because of B2XX-output or debugging only:
// //   * m_PARAMnameOfInstance, m_eventCounter,
// //   *
// //   ** in-module-function-calls:
// //   * cellularAutomaton(currentPass)
// //   * stopBrokenEvent()
// //   */
// //   m_evInfoPack.newTic();
// //   bool killEvent = false;
// //
// //   int numRounds = cellularAutomaton(currentPass);
// //   B2DEBUG(3, "pass " << passNumber << ": cellular automaton finished in " << numRounds << " rounds");
// //   m_evInfoPack.duration.cellularAutomaton += m_evInfoPack.TicToc();
// //   if (numRounds < 0) {
// //     B2ERROR(m_PARAMnameOfInstance << " event " << m_eventCounter <<
// //             ": cellular automaton entered an infinite loop, therefore aborted, terminating event! There were " << m_evInfoPack.nPXDClusters <<
// //             "/" << m_evInfoPack.nSVDClusters << "/" << m_evInfoPack.numSVDHits <<
// //             " PXD-/SVD-clusters/SVD-cluster-combinations, total number of activated segments: " << m_evInfoPack.nbFinderActivated)
// //     stopBrokenEvent();
// //     killEvent = true;
// //   }
// //   return killEvent;
// // }
// //
// //
// //
// // bool TFRedesignModule::doTheTrackCandidateCollector(PassData* currentPass, int passNumber)
// // {
// //   /** REDESIGNCOMMENT EVENT 9:
// //   * * short:
// //   * Section 7 - Track Candidate Collector (TCC)
// //   *
// //   ** long (+personal comments):
// //   *
// //   ** dependency of module parameters (global):
// //   * m_PARAMkillEventForHighOccupancyThreshold, m_PARAMnameOfInstance,
// //   *
// //   ** dependency of global in-module variables:
// //   * m_eventCounter, m_evInfoPack
// //   *
// //   ** dependency of global stuff just because of B2XX-output or debugging only:
// //   * m_PARAMnameOfInstance, m_eventCounter,
// //   *
// //   ** in-module-function-calls:
// //   * delFalseFriends(currentPass, currentPass->origin)
// //   * tcCollector(currentPass)
// //   * stopBrokenEvent()
// //   */
// //   m_evInfoPack.newTic();
// //   bool killEvent = false;
// //
// //   delFalseFriends(currentPass, currentPass->origin);        /// delFalseFriends
// //   tcCollector(currentPass);                   /// tcCollector
// //   int survivingTCs = currentPass->tcVector.size();
// //   B2DEBUG(3, "pass " << passNumber << ": track candidate collector generated " << survivingTCs << " TCs");
// //   m_evInfoPack.numTCsAfterTCC += survivingTCs;
// //
// //   m_evInfoPack.duration.tcc += m_evInfoPack.TicToc();
// //
// //   if (m_PARAMwriteToRoot == true) {
// //     for (VXDTFTrackCandidate* tc : currentPass->tcVector) {
// //
// //       for (const VXDTFHit* hit : tc->getHits()) {
// //         if (hit->getVxdID().getLayerNumber() == 0) {continue; }
// //         m_rootGoodHitPosTCCX.at(hit->getVxdID().getLayerNumber() - 1).push_back(hit->X());
// //         m_rootGoodHitPosTCCY.at(hit->getVxdID().getLayerNumber() - 1).push_back(hit->Y());
// //         m_rootGoodHitPosTCCZ.at(hit->getVxdID().getLayerNumber() - 1).push_back(hit->Z());
// //         m_rootGoodHitPosTCCR.at(hit->getVxdID().getLayerNumber() - 1).push_back(hit->Perp());
// //
// //         m_rootAllGoodHitPosTCCX.push_back(hit->X());
// //         m_rootAllGoodHitPosTCCY.push_back(hit->Y());
// //         m_rootAllGoodHitPosTCCZ.push_back(hit->Z());
// //         m_rootAllGoodHitPosTCCR.push_back(hit->Perp());
// //       }
// //     }
// //   }
// //
// //   if (m_evInfoPack.numTCsAfterTCC > m_PARAMkillEventForHighOccupancyThreshold / 3) {
// //     B2DEBUG(1, m_PARAMnameOfInstance << " event " << m_eventCounter << ": total number of tcs after tcc: " <<
// //             m_evInfoPack.numTCsAfterTCC << ", terminating event! There were " << m_evInfoPack.nPXDClusters << "/" << m_evInfoPack.nSVDClusters
// //             << "/" << m_evInfoPack.numSVDHits << " PXD-/SVD-clusters/SVD-cluster-combinations, total number of activated segments: " <<
// //             m_evInfoPack.nbFinderActivated)
// //     stopBrokenEvent();
// //     killEvent = true;
// //   }
// //   return killEvent;
// // }
// //
// //
// //
// // void TFRedesignModule::doTheTcFilter(PassData* currentPass, int passNumber)
// // {
// //   /** REDESIGNCOMMENT EVENT 10:
// //   * * short:
// //   * Section 8 - tcFilter
// //   *
// //   ** long (+personal comments):
// //   *
// //   ** dependency of module parameters (global):
// //   *
// //   ** dependency of global in-module variables:
// //   * m_evInfoPack
// //   *
// //   ** dependency of global stuff just because of B2XX-output or debugging only:
// //   * m_evInfoPack
// //   *
// //   ** in-module-function-calls:
// //   * tcFilter(currentPass, passNumber)
// //   */
// //   m_evInfoPack.newTic();
// //   if (int(currentPass->tcVector.size()) != 0) {
// //     int survivingTCs = tcFilter(currentPass, passNumber);           ///tcFilter
// //     m_evInfoPack.numTCsAfterTCCfilter += survivingTCs;
// //     B2DEBUG(3, "pass " << passNumber << ": track candidate filter, " << survivingTCs << " TCs survived.");
// //   } else {
// //     B2DEBUG(3, "pass " << passNumber << " has no TCs therefore not starting tcFilter()");
// //   }
// //
// //   m_evInfoPack.duration.postCAFilter += m_evInfoPack.TicToc();
// // }
// //
// //
// //
// // unsigned int TFRedesignModule::doTheOverlapCheck(PassData* currentPass)
// // {
// //   /** REDESIGNCOMMENT EVENT 11:
// //   * * short:
// //   * Section 9 - check overlap
// //   *
// //   ** long (+personal comments):
// //   * does the overlap check (ClusterInfo::isOverbooked) and additionally calculates initial values for the TCs
// //   * This uses the helixFit
// //   *
// //   ** dependency of module parameters (global):
// //   * m_PARAMdisplayCollector,
// //   *
// //   ** dependency of global in-module variables:
// //   * m_tcVectorOverlapped, m_calcQiType, m_tcVector,
// //   * m_collector, m_evInfoPack, m_eventCounter
// //   *
// //   ** dependency of global stuff just because of B2XX-output or debugging only:
// //   * m_PARAMdisplayCollector, m_collector, m_evInfoPack,
// //   * m_eventCounter
// //   *
// //   ** in-module-function-calls:
// //   * calcInitialValues4TCs(currentPass)
// //   */
// //   m_evInfoPack.newTic();
// //   unsigned int countOverbookedClusters = 0, clustersReserved = 0;
// //   m_tcVectorOverlapped.clear(); // July13: should be filled freshly for each pass
// //   /// each clusterInfo knows which TCs are using it, following loop therefore checks each for overlapping ones
// //   B2DEBUG(3, " checking overlaps now: there are " << m_clustersOfEvent.size() << " clusters ...")
// //   for (ClusterInfo& aCluster : m_clustersOfEvent) {
// //     bool isOB = aCluster.isOverbooked();
// //     if (isOB == true) { countOverbookedClusters++; }
// //     if (aCluster.isReserved() == true) { clustersReserved++; }
// //   } // now each TC knows whether it is overbooked or not (aCluster.isOverbooked() implicitly checked this)
// //   B2DEBUG(3, "after checking overlaps: there are " << countOverbookedClusters << " clusters of " << m_clustersOfEvent.size() <<
// //           " marked as 'overbooked'... (reserved: " << clustersReserved << ")")
// //
// //   unsigned int countCurrentTCs = 0;
// //   if (m_calcQiType != 3) {
// //     /// determine initial values for all track Candidates:
// //     calcInitialValues4TCs(currentPass);                                                /// calcInitialValues4TCs
// //   } // QiType 3 = straightLine which calculates the seed by itself
// //
// //   for (VXDTFTrackCandidate* currentTC : currentPass->tcVector) {
// //     if (currentTC->getCondition() == false) { continue; }   // in this case, the TC is already filtered
// //
// //     m_tcVector.push_back(currentTC);
// //
// //     bool overbookedCheck = currentTC->checkOverlappingState();
// //     if (overbookedCheck == true) { m_tcVectorOverlapped.push_back(currentTC); }
// //
// //     // Collector TC Update for Overlapping
// //     if (m_PARAMdisplayCollector > 0) {
// //
// //       if (overbookedCheck) {
// //         // int tcid, std::string diedAt, int accepted, int rejected
// //         m_collector.updateTC(currentTC->getCollectorID(), "",  CollectorTFInfo::m_idAlive, {FilterID::overlapping}, vector<int>());
// //       } else {
// //         m_collector.updateTC(currentTC->getCollectorID(), "",  CollectorTFInfo::m_idAlive, vector<int>(), {FilterID::overlapping});
// //       }
// //     }
// //
// //     countCurrentTCs++;
// //   }
// //
// //   B2DEBUG(3, "event " << m_eventCounter << ": " << m_tcVectorOverlapped.size() << " overlapping track candidates found within " <<
// //           countCurrentTCs << " new TCs of this pass alive")
// //   m_evInfoPack.duration.checkOverlap += m_evInfoPack.TicToc();
// //
// //   return m_tcVectorOverlapped.size();
// // }
// //
// //
// //
// // bool TFRedesignModule::doTheQICalculation(unsigned int totalOverlaps)
// // {
// //   /** REDESIGNCOMMENT EVENT 12:
// //   * * short:
// //   * Section 10 - calc QI for each TC
// //   *
// //   ** long (+personal comments):
// //   *
// //   ** dependency of module parameters (global):
// //   * m_PARAMkillEventForHighOccupancyThreshold, m_PARAMnameOfInstance,
// //   *
// //   ** dependency of global in-module variables:
// //   * m_calcQiType, m_evInfoPack, m_eventCounter,
// //   * m_passSetupVector, m_TESTERkalmanSkipped, m_tcVector,
// //   *
// //   ** dependency of global stuff just because of B2XX-output or debugging only:
// //   * m_PARAMnameOfInstance, m_eventCounter, m_evInfoPack
// //   * m_TESTERkalmanSkipped,
// //   *
// //   ** in-module-function-calls:
// //   * calcQIbyKalman(m_tcVector)
// //   * calcQIbyLength(m_tcVector, m_passSetupVector)
// //   * calcQIbyStraightLine(m_tcVector)
// //   * stopBrokenEvent()
// //   */
// //   m_evInfoPack.newTic();
// //   bool killEvent = false;
// //
// //   /// since KF is rather slow, Kf will not be used when there are many overlapping TCs. In this case, the simplified QI-calculator will be used.
// //   bool allowKalman = false;
// //   if (m_calcQiType == 1) { allowKalman = true; }
// //   if (totalOverlaps > m_PARAMkillEventForHighOccupancyThreshold / 3) {
// //     B2DEBUG(1, m_PARAMnameOfInstance << " event " << m_eventCounter << ": total number of overlapping track candidates: " <<
// //             totalOverlaps << ", termitating event!\nThere were " << m_evInfoPack.nPXDClusters << "/" << m_evInfoPack.nSVDClusters << "/" <<
// //             m_evInfoPack.numSVDHits << " PXD-/SVD-clusters/SVD-cluster-combinations.")
// //     stopBrokenEvent();
// //     killEvent = true;
// //
// //   } else if (totalOverlaps > m_PARAMkillEventForHighOccupancyThreshold / 10 && m_calcQiType == 1) {
// //     B2DEBUG(1, m_PARAMnameOfInstance << " event " << m_eventCounter << ": total number of overlapping TCs is " << totalOverlaps <<
// //             " and therefore KF is too slow, will use simple QI (trackLength) calculation which produces worse results")
// //     allowKalman = false;
// //     m_TESTERkalmanSkipped++;
// //   }
// //
// //   if (killEvent == false) {
// //     if (m_calcQiType == 1 and allowKalman == true) {
// //       calcQIbyKalman(m_tcVector); /// calcQIbyKalman
// //     } else if (m_calcQiType == 0) {
// //       calcQIbyLength(m_tcVector, m_passSetupVector);                              /// calcQIbyLength
// //     } else if (m_calcQiType == 3) { // and if totalOverlaps > 500
// //       calcQIbyStraightLine(m_tcVector);                                              ///calcQIbyStraightLine
// //     }
// //   }
// //
// //   m_evInfoPack.duration.kalmanStuff += m_evInfoPack.TicToc();
// //
// //   return killEvent;
// // }
// //
// //
// //
// // unsigned int TFRedesignModule::doTheOverlapCleaning(unsigned int totalOverlaps)
// // {
// //   /** REDESIGNCOMMENT EVENT 13:
// //   * * short:
// //   * Section 11 - cleanOverlappingSet
// //   *
// //   ** long (+personal comments):
// //   *
// //   ** dependency of module parameters (global):
// //   * m_PARAMcleanOverlappingSet,
// //   *
// //   ** dependency of global in-module variables:
// //   * m_tcVectorOverlapped, m_TESTERcleanOverlappingSetStartedCtr,
// //   * m_evInfoPack,
// //   *
// //   ** dependency of global stuff just because of B2XX-output or debugging only:
// //   * m_TESTERcleanOverlappingSetStartedCtr, m_evInfoPack,
// //   *
// //   ** in-module-function-calls:
// //   * cleanOverlappingSet(m_tcVectorOverlapped)
// //   */
// //   m_evInfoPack.newTic();
// //
// //   int killedTCs = -1;
// //   int totalKilledTCs = 0, cleaningRepeatedCtr = 0;
// //   while (killedTCs != 0) {
// //     cleaningRepeatedCtr ++;
// //     killedTCs = cleanOverlappingSet(m_tcVectorOverlapped); /// removes TCs which are found more than once completely
// //     totalOverlaps = m_tcVectorOverlapped.size();
// //     B2DEBUG(4, "out of funcCleanOverlappingSet-iteration " << cleaningRepeatedCtr << ": killed " << killedTCs << " TCs")
// //
// //     totalKilledTCs += killedTCs;
// //   }
// //   B2DEBUG(3, "out of funcCleanOverlappingSet: killed " << totalKilledTCs << " TCs within " << cleaningRepeatedCtr << " iterations")
// //   m_TESTERcleanOverlappingSetStartedCtr++;
// //   m_evInfoPack.numTCsKilledByCleanOverlap += totalKilledTCs;
// //
// //   m_evInfoPack.duration.cleanOverlap += m_evInfoPack.TicToc();
// //   return totalOverlaps;
// // }
// //
// //
// //
// // unsigned int TFRedesignModule::redoTheOverlapCheck(unsigned int totalOverlaps)
// // {
// //   /** REDESIGNCOMMENT EVENT 14:
// //   * * short:
// //   * Section 9b - check overlap 2
// //   *
// //   ** long (+personal comments):
// //   * redo overlapping check - necessary since condition changed through cleanOverlappingSet
// //   *
// //   ** dependency of module parameters (global):
// //   *
// //   ** dependency of global in-module variables:
// //   * m_tcVectorOverlapped, m_tcVector, m_evInfoPack,
// //   * m_clustersOfEvent
// //   *
// //   ** dependency of global stuff just because of B2XX-output or debugging only:
// //   * m_evInfoPack,
// //   *
// //   ** in-module-function-calls:
// //   */
// //   m_evInfoPack.newTic();
// //   unsigned int countOverbookedClusters = 0;
// //   // each clusterInfo knows which TCs are using it, following loop therefore checks each for overlapping ones
// //   for (ClusterInfo& aCluster : m_clustersOfEvent) {
// //     bool isOB = aCluster.isOverbooked();
// //     if (isOB == true) { countOverbookedClusters++; }
// //   } // now each TC knows whether it is overbooked or not (aCluster.isOverbooked() implicitly checked this)
// //
// //   m_tcVectorOverlapped.clear();
// //   for (VXDTFTrackCandidate* currentTC : m_tcVector) {
// //     if (currentTC->getCondition() == false) { continue; }
// //
// //     bool overbookedCheck = currentTC->checkOverlappingState();
// //     /** // WARNING IMPORTANT (July,24th, 2013): they do not deliver the same results, but checkOverlappingState produces less overlapping tcs while there are still no overlaps in final result, if something else occurs, new studies necessary!!
// //     * bool overbookedGet = currentTC->getOverlappingState();
// //     * if (overbookedCheck != overbookedGet) {
// //     *  B2WARNING(m_PARAMnameOfInstance << " event " << m_eventCounter << ": getOverlappingState ("<<overbookedGet<<") != checkOverlappingState ("<<overbookedCheck<<")! ")
// //     * }
// //     * if (overbookedGet == true || overbookedCheck == true )
// //     */
// //     if (overbookedCheck == true) { m_tcVectorOverlapped.push_back(currentTC); }
// //   }
// //   totalOverlaps = m_tcVectorOverlapped.size();
// //
// //   B2DEBUG(3, "after checking overlaps again: there are " << countOverbookedClusters << " clusters of " << m_clustersOfEvent.size() <<
// //           " still 'overbooked', tcVector: " << m_tcVector.size() << ", tcVectorOverlapped: " << totalOverlaps)
// //
// //   m_evInfoPack.duration.checkOverlap += m_evInfoPack.TicToc();
// //
// //   return totalOverlaps;
// // }
// //
// //
// //
// // void TFRedesignModule::doTheFilterOverlappingTCs(PassData* currentPass, int passNumber, unsigned int totalOverlaps)
// // {
// //   /** REDESIGNCOMMENT EVENT 15:
// //   * * short:
// //   * Section 12 - Hopfield
// //   *
// //   ** long (+personal comments):
// //   * does also include the reserve hits-step, where hits of the best TCs (highest QI) are blocked for further passes
// //   *
// //   ** dependency of module parameters (global):
// //   * m_PARAMomega,
// //   *
// //   ** dependency of global in-module variables:
// //   * m_filterOverlappingTCs, m_tcVectorOverlapped, m_tcVector,
// //   * m_eventCounter, m_evInfoPack, m_passSetupVector,
// //   *
// //   ** dependency of global stuff just because of B2XX-output or debugging only:
// //   * m_eventCounter, m_evInfoPack,
// //   *
// //   ** in-module-function-calls:
// //   * hopfield(m_tcVectorOverlapped, m_PARAMomega)
// //   * greedy(m_tcVectorOverlapped)
// //   * tcDuel(m_tcVectorOverlapped)
// //   * reserveHits(m_tcVector, currentPass)
// //   */
// //   m_evInfoPack.newTic();
// //   if (totalOverlaps > 2) { // checking overlapping TCs for best subset, if there are more than 2 different TC's
// //     if (m_filterOverlappingTCs == 2) {   /// use Hopfield neuronal network
// //
// //       hopfield(m_tcVectorOverlapped, m_PARAMomega);                             /// hopfield
// //
// //       int testTCs = 0;
// //       for (VXDTFTrackCandidate* currentTC : m_tcVector) {
// //         if (currentTC->getCondition() == false) continue;
// //         if (currentTC->checkOverlappingState() == true) { testTCs++; }
// //       }
// //       if (testTCs > 0) {
// //         B2ERROR(" event " << m_eventCounter << ": m_tcVector: " << m_tcVector.size() << ", m_tcVectorOverlapped: " <<
// //                 m_tcVectorOverlapped.size() << ", overlapping tcs surviving Hopfield: " << testTCs << " <- should not happen!")
// //       }
// //       B2DEBUG(3, " event " << m_eventCounter << ": m_tcVector: " << m_tcVector.size() << ", m_tcVectorOverlapped: " <<
// //               m_tcVectorOverlapped.size() << ", overlapping tcs surviving Hopfield: " << testTCs)
// //
// //     } else if (m_filterOverlappingTCs == 1) {   /// use Greedy algorithm
// //
// //       greedy(m_tcVectorOverlapped);                                               /// greedy
// //
// //     } else { /* do nothing -> accept overlapping TCs */ }
// //   } else if (m_filterOverlappingTCs != 0 and totalOverlaps == 2) {
// //
// //     tcDuel(m_tcVectorOverlapped);                                                 /// tcDuel
// //
// //   } else { B2DEBUG(10, " less than 2 overlapping Track Candidates found, no need for neuronal network") }
// //
// //
// //
// //   if ((LogSystem::Instance().isLevelEnabled(LogConfig::c_Debug, 3, PACKAGENAME()) == true)) {
// //     unsigned int nTCsAlive = 0, nTCsAliveInOverlapped = 0;
// //     for (VXDTFTrackCandidate* currentTC : m_tcVector) {
// //       if (currentTC->getCondition() == false) { continue; }
// //       nTCsAlive++;
// //     }
// //     for (VXDTFTrackCandidate* currentTC : m_tcVectorOverlapped) {
// //       if (currentTC->getCondition() == false) { continue; }
// //       nTCsAliveInOverlapped++;
// //     }
// //
// //     B2DEBUG(3, "before exporting TCs, length of m_tcVector: " << m_tcVector.size() << " with " << nTCsAlive << "/" << m_tcVector.size()
// //             - nTCsAlive << " alive/dead TCs, m_tcVectorOverlapped: " << m_tcVectorOverlapped.size() << " with " << nTCsAliveInOverlapped << "/"
// //             << m_tcVectorOverlapped.size() - nTCsAliveInOverlapped << " alive/dead TCs");
// //   }
// //
// //   if (passNumber < int(m_passSetupVector.size() - 1)) { reserveHits(m_tcVector, currentPass); }
// //
// //   m_evInfoPack.duration.neuronalStuff += m_evInfoPack.TicToc();
// // }
// //
// //
// //
// // void TFRedesignModule::doTheGFTrackCandGeneration()
// // {
// //   /** REDESIGNCOMMENT EVENT 16:
// //   * * short:
// //   * Section 13 - generateGFTrackCand
// //   *
// //   ** long (+personal comments):
// //   *
// //   ** dependency of module parameters (global):
// //   * m_TESTERexpandedTestingRoutines, m_evInfoPack, m_PARAMdisplayCollector,
// //   * m_PARAMnameOfInstance,
// //   *
// //   ** dependency of global in-module variables:
// //   * m_tcVector, m_collector, m_TESTERcountTotalUsedHitsFinal,
// //   * m_TESTERcountTotalUsedIndicesFinal, m_eventCounter, m_filterOverlappingTCs,
// //   * m_TESTERcountTotalTCsFinal
// //   *
// //   ** dependency of global stuff just because of B2XX-output or debugging only:
// //   * m_TESTERexpandedTestingRoutines, m_evInfoPack, m_PARAMdisplayCollector,
// //   * m_collector, m_TESTERcountTotalUsedHitsFinal, m_TESTERcountTotalUsedIndicesFinal,
// //   * m_TESTERcountTotalTCsFinal,
// //   *
// //   ** in-module-function-calls:
// //   * generateGFTrackCand(currentTC)
// //   */
// //   m_evInfoPack.newTic();
// //   list<int> tempIndices,
// //        totalIndices;// temp: per tc, total, all of them. collects all indices used by final TCs, used to check for final overlapping TCs
// //   int nTotalHitsInTCs = 0;
// //   for (VXDTFTrackCandidate* currentTC : m_tcVector) {
// //     if (currentTC->getCondition() == false) { continue; }
// //
// //     tempIndices = currentTC->getHopfieldHitIndices();
// //     tempIndices.sort();
// //     totalIndices.merge(tempIndices);
// //
// //     nTotalHitsInTCs += currentTC->size();
// //
// //     if ((LogSystem::Instance().isLevelEnabled(LogConfig::c_Debug, 10, PACKAGENAME()) == true)) {
// //       stringstream printClusterInfoIndices;
// //       printClusterInfoIndices << "Total: ";
// //       for (int index : totalIndices) { printClusterInfoIndices << index << " "; }
// //
// //       stringstream printIndices;
// //
// //       const vector<int>& pxdHits = currentTC->getPXDHitIndices();
// //       printIndices << "PXD: ";
// //       for (int index : pxdHits) { printIndices << index << " "; }
// //
// //       const vector<int>& svdHits = currentTC->getSVDHitIndices();
// //       printIndices << ", SVD: ";
// //       for (int index : svdHits) { printIndices << index << " "; }
// //
// //       B2DEBUG(10, "before starting generateGFTrackCand: generated GFTC with following real hits/clusters: " << printIndices.str() <<
// //               ", intermediate clusters: " << printClusterInfoIndices.str() << ", tempIndices: " << pxdHits.size() + svdHits.size() <<
// //               ", totalIndices: " << totalIndices.size());
// //     }
// //
// //
// //
// //     /// REACTIVATE the following code snippet if you want to test callgrind-stuff
// //     // #if 0
// //     genfit::TrackCand gfTC = generateGFTrackCand(currentTC);                          /// generateGFTrackCand
// //
// //     if (m_TESTERexpandedTestingRoutines == true) { // store data for the TFAnalyzer
// //
// //       VXDTFInfoBoard newBoard;
// //
// //       int indexNumber = m_finalTrackCandidates.getEntries(); // valid for both, GFTrackCand and InfoBoard
// //       gfTC.setMcTrackId(indexNumber); // so the GFTrackCand knows which index of infoBoard is assigned to it
// //
// //       newBoard.assignGFTC(indexNumber); // same number aDEBUGs for the genfit::TrackCand
// //       newBoard.fitIsPossible(currentTC->getFitSucceeded());
// //       newBoard.setProbValue(currentTC->getTrackQuality());
// //       m_extraInfo4GFTCs.appendNew(newBoard);
// //     }
// //
// //     // Collector TC Fit Update (replaces VXDTFInfoBoard)
// //     if (m_PARAMdisplayCollector > 0) {
// //       int indexNumber = m_finalTrackCandidates.getEntries();
// //       int tcId = currentTC->getCollectorID();
// //
// //       B2DEBUG(10, "NEW InfoBoard tcId: " << tcId << "ProbValue: " << currentTC->getTrackQuality() << ";  isFitPossible: " <<
// //               currentTC->getFitSucceeded());
// //
// //       m_collector.updateTCFitInformation(tcId, currentTC->getFitSucceeded(), currentTC->getTrackQuality(), indexNumber);
// //     }
// //
// //     m_finalTrackCandidates.appendNew(gfTC);
// //     // #endif
// //
// //   }
// //   m_TESTERcountTotalUsedHitsFinal += nTotalHitsInTCs;
// //
// //
// //   // do a recheck, wheter there were unexpected problems or not:
// //
// //   int nTotalIndices = totalIndices.size();
// //   m_TESTERcountTotalUsedIndicesFinal += nTotalIndices;
// //
// //   totalIndices.sort(); // should already be sorted, but just to be sure....
// //   tempIndices = totalIndices;
// //   totalIndices.unique();
// //
// //   int nReducedIndices = totalIndices.size();
// //   int nFinalTCs = m_finalTrackCandidates.getEntries();
// //
// //   if ((LogSystem::Instance().isLevelEnabled(LogConfig::c_Debug, 1, PACKAGENAME()) == true)) {
// //     int tcPos = 0;
// //     for (VXDTFTrackCandidate* currentTC : m_tcVector) {
// //       if (currentTC->getCondition() == false) { tcPos++; continue; }
// //       TVector3 momentum = currentTC->getInitialMomentum();
// //       B2DEBUG(20, "event " << m_eventCounter << ": TC " << tcPos <<
// //               " has got condition = true, means its a final TC (TC-output). Its total Momentum is" << momentum.Mag() <<
// //               ", its transverse momentum is: " << momentum.Perp())
// //       tcPos++;
// //     }
// //     B2DEBUG(3, m_PARAMnameOfInstance << " - event " << m_eventCounter << ": " << nFinalTCs <<
// //             " final track candidates determined! Having a total number of hit indices: "  << nTotalIndices <<
// //             ", and after doube entry removal: " << nReducedIndices)
// //   }
// //
// //   if (nReducedIndices != nTotalIndices) {
// //     stringstream aStream;
// //     for (int index : tempIndices) { aStream << " " << index; }
// //
// //     if (m_filterOverlappingTCs == 0) {
// //       B2DEBUG(1, m_PARAMnameOfInstance << " - event " << m_eventCounter << ": " << nFinalTCs <<
// //               " final TCs determined. Having a total number of hit indices: "  << nTotalIndices << ", and after doube entry removal: " <<
// //               nReducedIndices << "!\n before unique, totalIndices had the following entries: " << aStream.str() <<
// //               "\n, is allowed since filtering overlaps is deactivated...")
// //     } else {
// //       B2ERROR(m_PARAMnameOfInstance << " - event " << m_eventCounter << ": " << nFinalTCs <<
// //               " final TCs determined. Having a total number of hit indices: "  << nTotalIndices << ", and after doube entry removal: " <<
// //               nReducedIndices << "!\n before unique, totalIndices had the following entries: " << aStream.str() <<
// //               "\n, should not occur (check hopfield or greedy)!")
// //     }
// //   }
// //
// //   if (m_PARAMwriteToRoot == true) {
// //     for (VXDTFTrackCandidate* tc : m_tcVector) {
// //       if (tc->getCondition() == false) { continue; }
// //
// //       for (const VXDTFHit* hit : tc->getHits()) {
// //         if (hit->getVxdID().getLayerNumber() == 0) {continue; }
// //         m_rootGoodHitPosFTCX.at(hit->getVxdID().getLayerNumber() - 1).push_back(hit->X());
// //         m_rootGoodHitPosFTCY.at(hit->getVxdID().getLayerNumber() - 1).push_back(hit->Y());
// //         m_rootGoodHitPosFTCZ.at(hit->getVxdID().getLayerNumber() - 1).push_back(hit->Z());
// //         m_rootGoodHitPosFTCR.at(hit->getVxdID().getLayerNumber() - 1).push_back(hit->Perp());
// //
// //         m_rootAllGoodHitPosFTCX.push_back(hit->X());
// //         m_rootAllGoodHitPosFTCY.push_back(hit->Y());
// //         m_rootAllGoodHitPosFTCZ.push_back(hit->Z());
// //         m_rootAllGoodHitPosFTCR.push_back(hit->Perp());
// //       }
// //     }
// //   }
// //
// //   m_TESTERcountTotalTCsFinal += nFinalTCs;
// //   m_evInfoPack.numTCsfinal += nFinalTCs;
// //
// //   m_evInfoPack.duration.intermediateStuff += m_evInfoPack.TicToc();
// // }
// //
// //
// //
// // void TFRedesignModule::finalEventCleanup()
// // {
// //   /** REDESIGNCOMMENT EVENT 17:
// //   * * short:
// //   * Section 14 - final stuff of event including cleanup
// //   *
// //   ** long (+personal comments):
// //   *
// //   ** dependency of module parameters (global):
// //   * m_PARAMdisplayCollector, m_PARAMwriteToRoot,
// //   *
// //   ** dependency of global in-module variables:
// //   * m_collector, m_passSetupVector, m_evInfoPack,
// //   * m_treeEventWisePtr, m_TESTERlogEvents
// //   *
// //   ** dependency of global stuff just because of B2XX-output or debugging only:
// //   * m_PARAMdisplayCollector, m_collector, m_PARAMwriteToRoot,
// //   * m_evInfoPack, m_treeEventWisePtr, m_TESTERlogEvents
// //   *
// //   ** in-module-function-calls:
// //   * cleanEvent(currentPass)
// //   */
// //   m_evInfoPack.newTic();
// //   // Silent Kill = Mark not used objects as deleted
// //   // Safe Information = Information into StoreArrays
// //   if (m_PARAMdisplayCollector > 0) {
// //
// //     m_collector.silentKill();
// //     m_collector.safeInformation();
// //   }
// //
// //   /** cleaning part **/
// //   for (PassData* currentPass : m_passSetupVector) {
// //     cleanEvent(currentPass);
// //   }
// //
// //   m_evInfoPack.duration.intermediateStuff += m_evInfoPack.TicToc();
// //   stopTFevent();
// // }
