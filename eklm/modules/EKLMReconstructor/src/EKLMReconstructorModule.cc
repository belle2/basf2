/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Timofey Uglov, Kirill Chilikin                           *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

/* Belle2 headers. */
#include <eklm/modules/EKLMReconstructor/EKLMReconstructorModule.h>
#include <eklm/reconstruction/Reconstructor.h>
#include <framework/core/ModuleManager.h>

using namespace Belle2;

REG_MODULE(EKLMReconstructor)

EKLMReconstructorModule::EKLMReconstructorModule() : Module()
{
  setDescription("EKLM reconstruction module.");
  setPropertyFlags(c_ParallelProcessingCertified);
  m_TransformData = NULL;
}

EKLMReconstructorModule::~EKLMReconstructorModule()
{
}

void EKLMReconstructorModule::initialize()
{
  StoreArray<EKLMHit2d> hit2ds;
  StoreArray<EKLMDigit> digits;
  hit2ds.registerInDataStore();
  digits.isRequired();
  hit2ds.registerRelationTo(digits);
  m_TransformData = new EKLM::TransformData(true, NULL);
}

void EKLMReconstructorModule::beginRun()
{
  m_rec = new EKLM::Reconstructor(m_TransformData);
}

void EKLMReconstructorModule::event()
{
  m_rec->create2dHits();
}

void EKLMReconstructorModule::endRun()
{
  delete m_rec;
}

void EKLMReconstructorModule::terminate()
{
  delete m_TransformData;
}
