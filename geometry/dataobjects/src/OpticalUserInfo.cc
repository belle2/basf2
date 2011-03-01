/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2011  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <geometry/dataobjects/OpticalUserInfo.h>
#include <framework/core/FrameworkExceptions.h>

using namespace std;
using namespace Belle2;


//Define exceptions
namespace Belle2 {
  /** Exception is thrown if the requested surface type could not be found. */
  BELLE2_DEFINE_EXCEPTION(SurfaceTypeNotFoundError, "The surface type '%1%' was not defined in the optical user information class !");
  /** Exception is thrown if the requested surface finish could not be found. */
  BELLE2_DEFINE_EXCEPTION(SurfaceFinishNotFoundError, "The surface finish '%1%' was not defined in the optical user information class !");
  /** Exception is thrown if the requested surface model could not be found. */
  BELLE2_DEFINE_EXCEPTION(SurfaceModelNotFoundError, "The surface model '%1%' was not defined in the optical user information class !");
}


OpticalUserInfo& OpticalUserInfo::operator=(const OpticalUserInfo & other)
{
  this->m_surfaceType = other.m_surfaceType;
  this->m_surfaceFinish = other.m_surfaceFinish;
  this->m_surfaceModel = other.m_surfaceModel;
  return *this;
}


void OpticalUserInfo::setSurfaceType(const string& surfaceType)
{
  SurfaceTypeNameMap::iterator optSurfIter = m_surfaceTypeNameMap.find(surfaceType);
  if (optSurfIter == m_surfaceTypeNameMap.end()) throw(SurfaceTypeNotFoundError() << surfaceType);

  m_surfaceType = optSurfIter->second;
}


void OpticalUserInfo::setSurfaceFinish(const string& surfaceFinish)
{
  SurfaceFinishNameMap::iterator optSurfIter = m_surfaceFinishNameMap.find(surfaceFinish);
  if (optSurfIter == m_surfaceFinishNameMap.end()) throw(SurfaceFinishNotFoundError() << surfaceFinish);

  m_surfaceFinish = optSurfIter->second;
}


void OpticalUserInfo::setSurfaceModel(const string& surfaceModel)
{
  SurfaceModelNameMap::iterator optSurfIter = m_surfaceModelNameMap.find(surfaceModel);
  if (optSurfIter == m_surfaceModelNameMap.end()) throw(SurfaceModelNotFoundError() << surfaceModel);

  m_surfaceModel = optSurfIter->second;
}


//============================================================================
//                          Protected methods
//============================================================================

void OpticalUserInfo::fillEnumMaps()
{
  //Surface type map
  m_surfaceTypeNameMap["dielectric_metal"] = dielectric_metal;
  m_surfaceTypeNameMap["dielectric_LUT"]   = dielectric_LUT;
  m_surfaceTypeNameMap["firsov"]           = firsov;
  m_surfaceTypeNameMap["x_ray"]            = x_ray;

  //Surface finish map
  m_surfaceFinishNameMap["polished"]             = polished;
  m_surfaceFinishNameMap["polishedfrontpainted"] = polishedfrontpainted;
  m_surfaceFinishNameMap["polishedbackpainted"]  = polishedbackpainted;
  m_surfaceFinishNameMap["ground"]               = ground;
  m_surfaceFinishNameMap["groundfrontpainted"]   = groundfrontpainted;
  m_surfaceFinishNameMap["groundbackpainted"]    = groundbackpainted;
  m_surfaceFinishNameMap["polishedlumirrorair"]  = polishedlumirrorair;
  m_surfaceFinishNameMap["polishedlumirrorglue"] = polishedlumirrorglue;
  m_surfaceFinishNameMap["polishedair"]          = polishedair;
  m_surfaceFinishNameMap["polishedteflonair"]    = polishedteflonair;
  m_surfaceFinishNameMap["polishedtioair"]       = polishedtioair;
  m_surfaceFinishNameMap["polishedtyvekair"]     = polishedtyvekair;
  m_surfaceFinishNameMap["polishedvm2000air"]    = polishedvm2000air;
  m_surfaceFinishNameMap["polishedvm2000glue"]   = polishedvm2000glue;
  m_surfaceFinishNameMap["etchedlumirrorair"]    = etchedlumirrorair;
  m_surfaceFinishNameMap["etchedlumirrorglue"]   = etchedlumirrorglue;
  m_surfaceFinishNameMap["etchedair"]            = etchedair;
  m_surfaceFinishNameMap["etchedteflonair"]      = etchedteflonair;
  m_surfaceFinishNameMap["etchedtioair"]         = etchedtioair;
  m_surfaceFinishNameMap["etchedtyvekair"]       = etchedtyvekair;
  m_surfaceFinishNameMap["etchedvm2000air"]      = etchedvm2000air;
  m_surfaceFinishNameMap["etchedvm2000glue"]     = etchedvm2000glue;
  m_surfaceFinishNameMap["groundlumirrorair"]    = groundlumirrorair;
  m_surfaceFinishNameMap["groundlumirrorglue"]   = groundlumirrorglue;
  m_surfaceFinishNameMap["groundair"]            = groundair;
  m_surfaceFinishNameMap["groundteflonair"]      = groundteflonair;
  m_surfaceFinishNameMap["groundtioair"]         = groundtioair;
  m_surfaceFinishNameMap["groundtyvekair"]       = groundtyvekair;
  m_surfaceFinishNameMap["groundvm2000air"]      = groundvm2000air;
  m_surfaceFinishNameMap["groundvm2000glue"]     = groundvm2000glue;

  //Surface model map
  m_surfaceModelNameMap["glisur"]  = glisur;
  m_surfaceModelNameMap["unified"] = unified;
  m_surfaceModelNameMap["LUT"]     = LUT;
}

ClassImp(OpticalUserInfo)
