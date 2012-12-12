/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Timofey Uglov, Kirill Chilikin                           *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef EKLMDigit_H
#define EKLMDigit_H

#include  <eklm/dataobjects/EKLMHitBase.h>
#include  <eklm/dataobjects/EKLMSimHit.h>
#include  "globals.hh"
#include  "CLHEP/Vector/ThreeVector.h"
#include  "TFitResultPtr.h"
#include  "TFitResult.h"

#include "G4VPhysicalVolume.hh"

#include <string>
namespace Belle2 {

  /**
   * Main reconstruction hit class. Contains information about the
   * hitted strips.
   */
  class EKLMDigit : public EKLMHitBase {

  public:

    /**
     * Constructor.
     */
    EKLMDigit() {};

    /**
     * Constructor from the EKLMSimHit.
     * @param[in] Hit EKLMSimHit.
     */
    EKLMDigit(const EKLMSimHit* Hit);

    /**
     * Destructor.
     */
    ~EKLMDigit() {};

    /**
     * Print stip name and some other useful information.
     */
    void Print() const;

    /**
     * Get number of photoelectrons.
     * @return Number of photoelectrons.
     */
    double getNumberPhotoElectrons() const;

    /**
     * Set the number of photo electorns.
     * @param[in] npe Number of photoelectrons.
     */
    void setNumberPhotoElectrons(double npe);

    /**
     * Get volume identifier.
     * @return Identifier.
     */
    int getVolumeID() const;

    /**
     * Set volume identifier.
     * @param[in] id Identifier.
     */
    void setVolumeID(int id);

    /**
     * Whether hit could be used late (if it passed discriminator threshold)
     * (getter).
     * @return True if could be used.
     */
    bool isGood() const;

    /**
     * Whether hit could be used late (if it passed discriminator threshold)
     * (setter).
     * @param[in] status True if could be used.
     */
    void isGood(bool status) ;

    /**
     * Get plane number.
     * @return Plane number.
     */
    int getPlane() const;

    /**
     * Set plane number.
     * @param[in] Plane Plane number.
     */
    void setPlane(int Plane);

    /**
     * Get strip number.
     * @return Strip number.
     */
    int getStrip() const;

    /**
     * Set strip number.
     * @param[in] Strip Strip number.
     */
    void setStrip(int Strip);

    /**
     * Get fit results.
     * @return Fit results.
     */
    const TFitResult* getFitResults() const;

    /**
     * Get fit status.
     * @return Fit status.
     */
    int getFitStatus();

    /**
     * Set fit status.
     * @param[in] s Fit status.
     */
    void setFitStatus(int s);

    /**
     * Set fit results.
     * @param[in] res Fit results.
     */
    void setFitResults(TFitResult& res);

    /**
     * Set fit results.
     * @param[in] resPtr Fit results.
     */
    void setFitResults(TFitResultPtr resPtr);

    /**
     * Set MC time shift.
     * @param[in] ts Time shift.
     */
    void setMCTS(double ts);

    /**
     * Get MC time shift.
     * @return Time shift.
     */
    double getMCTS() const;

  private:

    /** Number of plane. */
    int m_Plane;

    /** Number of strip. */
    int m_Strip;

    /** If hit passes threshold. */
    bool m_good;

    /** Number of photo electrons. */
    double m_NumberPhotoElectrons;

    /** Fit status. */
    int m_fitStatus;

    /** MC time. */
    /** FIXME: This variable is unused. Delete it? */
    double m_MCtime;

    /** MC time shift. */
    double m_timeshift;

    /** Volume identifier. */
    int m_volid;

    /** Fit results object. */
    TFitResult m_fitResults;

    /** Makes objects storable. */
    ClassDef(Belle2::EKLMDigit, 1);

  };

} // end of namespace Belle2

#endif //EKLMDigit_H
