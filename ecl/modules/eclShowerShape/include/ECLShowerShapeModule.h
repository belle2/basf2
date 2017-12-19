/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * This module calculates shower shape variables.                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Torben Ferber (ferber@physics.ubc.ca)                    *
 *               Guglielmo De Nardo (denardo@na.infn.it)                  *
 *               Alon Hershenhorn   (hershen@phas.ubc.ca)                 *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef ECLSHOWERSHAPEMODULE_H_
#define ECLSHOWERSHAPEMODULE_H_

// FRAMEWORK
#include <framework/core/Module.h>
#include <framework/database/DBObjPtr.h>
#include <framework/gearbox/Unit.h>

// ECL
#include <ecl/dataobjects/ECLShower.h>
#include <ecl/geometry/ECLNeighbours.h>
#include <framework/database/DBArray.h>
#include <ecl/dbobjects/ECLShowerShapeSecondMomentCorrection.h>

//MVA package
#include <mva/dataobjects/DatabaseRepresentationOfWeightfile.h>
#include <mva/interface/Weightfile.h>
#include <mva/interface/Expert.h>

namespace Belle2 {

  /** Class to perform the shower correction */
  class ECLShowerShapeModule : public Module {

  public:

    /** Enumeration of type for second moment corrections */
    enum {
      c_thetaType = 0, /**< type of theta identifier */
      c_phiType = 1, /**< type of phi identifier */
    };


    /** Constructor. */
    ECLShowerShapeModule();

    /** Destructor. */
    ~ECLShowerShapeModule();

    /** Initialize. */
    virtual void initialize();

    /** Begin run. */
    virtual void beginRun();

    /** Event. */
    virtual void event();

    /** End run. */
    virtual void endRun();

    /** Terminate. */
    virtual void terminate();

  private:

    /**Struct used to hold information of the digits projected to a plane perpendicular to the shower direction */
    struct ProjectedECLDigit {

      /** weighted energy */
      double energy;

      /** radial distance */
      double rho;

      /** polar angel */
      double alpha;
    };

    // Module Parameters
    double m_zernike_n1_rho0; /**< Scaling factor for radial distances in perpendicular plane, used in Zernike moment calculation for N1 showers */
    double m_zernike_n2_rho0; /**< Scaling factor for radial distances in perpendicular plane, used in Zernike moment calculation for N2 showers */
    bool m_zernike_useFarCrystals; /**< Determines if to include or ignore crystals with rho > rho0 in perpendicular plane, used in Zernike moment calculation*/
    double m_avgCrystalDimension; /**< Average crystal dimension [cm] */

    const double m_BRLthetaMin = 32.2 * Unit::deg; /**< Minimum theta of barrel used for choosing which Zernike MVA to apply */
    const double m_BRLthetaMax = 128.7 * Unit::deg; /**< Maximum theta of barrel used for choosing which Zernike MVA to apply */

    const unsigned int m_numZernikeMVAvariables = 22; /**< number of variables expected in the Zernike MVA weightfile */

    std::string m_zernike_MVAidentifier_FWD; /**< Zernike moment MVA - FWD endcap weight-file */
    std::string m_zernike_MVAidentifier_BRL; /**< Zernike moment MVA - Barrel weight-file */
    std::string m_zernike_MVAidentifier_BWD; /**< Zernike moment MVA - BWD endcap weight-file */
    std::unique_ptr<DBObjPtr<DatabaseRepresentationOfWeightfile>>
                                                               m_weightfile_representation_FWD; /**< Database pointer to the Database representation of the Zernike moment MVA weightfile for FWD*/
    std::unique_ptr<DBObjPtr<DatabaseRepresentationOfWeightfile>>
                                                               m_weightfile_representation_BRL; /**< Database pointer to the Database representation of the Zernike moment MVA weightfile for BRL*/
    std::unique_ptr<DBObjPtr<DatabaseRepresentationOfWeightfile>>
                                                               m_weightfile_representation_BWD; /**< Database pointer to the Database representation of the Zernike moment MVA weightfile for BWD*/
    std::unique_ptr<MVA::Expert> m_expert_FWD; /**< Pointer to the current MVA Expert for FWD*/
    std::unique_ptr<MVA::Expert> m_expert_BRL; /**< Pointer to the current MVA Expert for BRL*/
    std::unique_ptr<MVA::Expert> m_expert_BWD; /**< Pointer to the current MVA Expert for BWD*/
    std::unique_ptr<MVA::SingleDataset>
    m_dataset; /**< Pointer to the current dataset. It is assumed it holds 22 entries, 11 Zernike moments of N2 shower, followed by 11 Zernike moments of N1 shower. */

    /** Neighbour map 9 neighbours, for E9oE21 and E1oE9. */
    ECL::ECLNeighbours* m_neighbourMap9;

    /** Neighbour map 21 neighbours, for E9oE21. */
    ECL::ECLNeighbours* m_neighbourMap21;

    /** initialize MVA weight files from DB
     */
    void initializeMVAweightFiles(const std::string& identifier,
                                  std::unique_ptr<DBObjPtr<DatabaseRepresentationOfWeightfile>>& weightFileRepresentation);

    /** Load MVA weight file and set pointer of expert.
     * If weightFileRepresentation is the BRL MVA, also set m_dataset size according to weightFileRepresentation MVA::GeneralOptions
     */
    void initializeMVA(const std::string& identifier,
                       std::unique_ptr<DBObjPtr<DatabaseRepresentationOfWeightfile>>& weightFileRepresentation, std::unique_ptr<MVA::Expert>& expert);

    /**
    * Set showr shape variables.
    * For correct zernike MVA calculation, must be run first on the N2 shower in each connected region, then on the N1 showers of the connected region.
    * calculateZernikeMVA - determines if MVA calculation should be performed. Should be false, for example, if there was no N2 shower, so MVA can't be calculated.
    * The function does not set the zernikeMVA variable for N2 showers, because it is calculated from zernikeMVAs of the N1 showers, so it has to be set elsewhere (unless calculateZernikeMVA is false, in which case it is set to 0.0).
    */
    void setShowerShapeVariables(ECLShower* eclShower, const bool calculateZernikeMVA) const;

    /** Shower shape variable: Lateral energy. */
    double computeLateralEnergy(const std::vector<ProjectedECLDigit>& projectedDigits, const double avgCrystalDimension) const;

    /** Compute the absolute value of the complex Zernike moment Znm.
        The moments are computed in a plane perpendicular to the direction of the shower.
        The plane's origin is at the intersection of the shower direction with the plane.
        The origin is at a distance from the interaction point equal to the shower distance from the interaction point.

        n, m - are the Zernike polynomial rank
        R0 - is a scaling factor used to normalize the distances in the described plane.
        It also sets the maximum distance from the origin (the Zernike polynomials are defined only on the unit circle).
        All points in the plane with a distance larger than R0 from the origin are ignored.

        Valid values of n,m are n,m >= 0, m <= n.
        If n or m are invalid the function returns 0.0.
        */

    double computeAbsZernikeMoment(const std::vector<ProjectedECLDigit>& projectedDigits, const double totalEnergy, const int n,
                                   const int m,
                                   const double rho) const;

    /** Compute the second moment in the plane perpendicular to the direction of the shower.
        The plane's origin is at the intersection of the shower direction with the plane.
        The origin is at a distance from the interaction point equal to the shower distance from the interaction point. */
    double computeSecondMoment(const std::vector<ProjectedECLDigit>& shower, const double totalEnergy) const;

    /** Compute projections of the ECLCalDigits to the perpendicular plane */
    std::vector<ProjectedECLDigit> projectECLDigits(const ECLShower& shower) const;

    /** The radial part of the Zernike polynomial
     * n,m - Zernike polynomial rank
     * rho - radial distance             */
    double Rnm(const int n, const int m, const double rho) const;

    /** Return the complex value of the Zernike polynomial of rank n,m.
    Znm(rho,alpha) = Rnm(rho) * exp(i*m*alpha)
    rho - radial distance
    alpha - polar angle */
    std::complex<double> zernikeValue(const double rho, const double alpha, const int n, const int m) const;

    /** Shower shape variable: E9oE21
     The energy ratio is calculated taking the weighted 3x3 (=9) and the weighted 5x5-corners (=21) crystals around the central crystal.
     If the shower is smaller than this, the reduced number is used for this. */
    double computeE9oE21(const ECLShower&) const;

    /** Shower shape variable: E1oE9
     The energy ratio is calculated taking the weighted central (=1) and the weighted 3x3 (=9) crystals around the central crystal.
     If the shower is smaller than this, the reduced number is used for this. */
    double computeE1oE9(const ECLShower&) const;

    DBArray<ECLShowerShapeSecondMomentCorrection> m_secondMomentCorrectionArray;  /**< Shower shape corrections from DB */

    /** TGraphs that hold the corrections
     */
    TGraph m_secondMomentCorrections[2][10];

    /** Prepare corrections for second moment
     * Will be called whenever the m_secondMomentCorrectionArray get updated
     * Clears m_secondMomentCorrections and fills it from the updated m_secondMomentCorrectionArray
     */
    void prepareSecondMomentCorrectionsCallback();

    /** Get corrections for second moment
     */
    double getSecondMomentCorrection(const double theta, const double phi, const int hypothesis) const;

  public:
    /** We need names for the data objects to differentiate between PureCsI and default*/

    /** Default name ECLShowers */
    virtual const char* eclShowerArrayName() const
    { return "ECLShowers" ; }

    /** Default name ECLCalDigits */
    virtual const char* eclCalDigitArrayName() const
    { return "ECLCalDigits" ; }

    /** Default name ECLConnectedRegions */
    virtual const char* eclConnectedRegionArrayName() const
    { return "ECLConnectedRegions" ; }

  }; // end of ECLShowerShapeModule


  /** The very same module but for PureCsI */
  class ECLShowerShapePureCsIModule : public ECLShowerShapeModule {
  public:

    /** PureCsI name ECLShowersPureCsI */
    virtual const char* eclShowerArrayName() const override
    { return "ECLShowersPureCsI" ; }

    /** PureCsI name ECLCalDigitsPureCsI */
    virtual const char* eclCalDigitArrayName() const override
    { return "ECLCalDigitsPureCsI" ; }

    /** PureCsI name ECLConnectedRegionsPureCsI */
    virtual const char* eclConnectedRegionArrayName() const override
    { return "ECLConnectedRegionsPureCsI" ; }

  }; // end of ECLShowerShapePureCsIModule

} // end of Belle2 namespace

#endif
