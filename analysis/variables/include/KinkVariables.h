/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <analysis/VariableManager/Manager.h>
#include <framework/gearbox/Const.h>
#include <Math/Vector3D.h>
#include <Math/Vector4D.h>

namespace Belle2 {
  class Particle;
  class Kink;
  class MCParticle;
  class Helix;

  namespace Variable {

    //-----------------------------------------------
    // Helper Functions

    /**
     * returns the kink daughter track Helix at the kink vertex
     */
    Helix kinkDaughterHelixAtKinkVertex(const Kink* kink);

    /**
     * returns the kink daughter momentum vector at the kink vertex
     */
    ROOT::Math::XYZVector kinkDaughterMomentumAtKinkVertex(const Kink* kink);

    /**
     * returns the kink mother track Helix at the kink vertex
     */
    Helix kinkMotherHelixAtKinkVertex(const Kink* kink);

    /**
     * returns the kink mother momentum vector at the kink vertex
     */
    ROOT::Math::XYZVector kinkMotherMomentumAtKinkVertex(const Kink* kink);

    /**
     * Calculates the kinematics of the daughter particle in the mother particle rest frame with respect to
     * the selected mass hypotheses
     * @param part mother particle created from the kink mdst object
     * @param motherType mother particle mass hypothesis
     * @param daughterType daughter particle mass hypothesis
     * @param returnCosTheta a flag to return the cosine of the angle between the daughter particle momentum
     * in the mother rest frame with respect to the mother particle momentum in the laboratory frame
     * @return magnitude of the momentum of the daughter particle in the mother particle rest frame or
     * cosine of the angle between the daughter particle momentum
     * in the mother rest frame with respect to the mother particle momentum in the laboratory frame
     */
    double kinkDaughterMomentumAndCosThetaInMotherRF(const Particle* part, Const::ChargedStable motherType,
                                                     Const::ChargedStable daughterType, bool returnCosTheta);

    /**
     * Calculates the four-momentum of the MCParticle at its decay vertex
     * by summation of four-momenta of its daughters
     * (checks that distance between the daughter particle production vertex and the decay vertex is
     * sufficiently small to avoid adding irrelevant particles)
     * @param p MCParticle of desired mother particle
     * @return four-momentum of the mother track at its decay vertex
     */
    ROOT::Math::PxPyPzEVector kinkMotherMCP4AtDecayVertex(const MCParticle* p);

    /**
     * Calculates the MC kinematics of the daughter particle in the mother particle rest frame
     * Makes sense only for real decays-in-flight; however, it is not checked here
     * @param part mother particle created from the kink mdst object
     * @param returnCosTheta a flag to return the cosine of the angle between the daughter particle momentum
     * in the mother rest frame with respect to the mother particle momentum in the laboratory frame
     * @return magnitude of the momentum of the daughter particle in the mother particle rest frame or
     * cosine of the angle between the daughter particle momentum
     * in the mother rest frame with respect to the mother particle momentum in the laboratory frame
     */
    double kinkDaughterMCMomentumAndCosThetaInMotherRF(const Particle* part, bool returnCosTheta);

    //-----------------------------------------------
    // MEASURED VARIABLES
    // Kink General Variables

    /**
     * returns the kink vertex X position
     */
    double kinkVertexX(const Particle* part);

    /**
     * returns the kink vertex Y position
     */
    double kinkVertexY(const Particle* part);

    /**
     * returns the kink vertex Z position
     */
    double kinkVertexZ(const Particle* part);

    /**
     * returns the filter ID with which kink was preselected
     */
    double kinkFilterID(const Particle* part);

    /**
     * returns the flag of the combined kink fit result
     */
    double kinkCombinedFitResultFlag(const Particle* part);

    /**
     * returns the first bit of the flag of the combined kink fit result
     */
    double kinkCombinedFitResultFlagBit1(const Particle* part);

    /**
     * returns the second bit of the flag of the combined kink fit result
     */
    double kinkCombinedFitResultFlagBit2(const Particle* part);

    /**
     * returns the third bit of the flag of the combined kink fit result
     */
    double kinkCombinedFitResultFlagBit3(const Particle* part);

    /**
     * returns the fourth bit of the flag of the combined kink fit result
     */
    double kinkCombinedFitResultFlagBit4(const Particle* part);

    /**
     * returns the flag of the passing the distance criteria for the split kink at the kink vertex
     */
    double kinkSplitTrackDistanceAtVertexFlag(const Particle* part);

    /**
    * returns the number of reassigned hits between kink mother and daughter tracks
    */
    double kinkNumberOfReassignedHits(const Particle* part);

    /**
    * returns true if the charges of mother and daughter tracks are the same and false otherwise
    */
    double kinkIsSameCharge(const Particle* part);

    // Kink Decay Kinematics

    /**
    * returns kink daughter momentum in mother rest frame
    * with default pair of mass hypotheses, set by a user in the decay string
    */
    double kinkDaughterMomentumInMotherRF(const Particle* part);

    /**
    * returns kink daughter direction in mother rest frame with respect to mother momentum direction in the lab frame
    * with default pair of mass hypotheses, set by a user in the decay string
    */
    double kinkDaughterCosThetaInMotherRF(const Particle* part);

    /**
    * returns kink daughter momentum in mother rest frame with pion and kaon mass hypotheses, respectively
    */
    double kinkDaughterMomentumInMotherRFKPi(const Particle* part);

    /**
    * returns kink daughter direction in mother rest frame with respect to mother momentum direction in the lab frame
    * with pion and kaon mass hypotheses, respectively
    */
    double kinkDaughterCosThetaInMotherRFKPi(const Particle* part);

    /**
    * returns kink daughter momentum in mother rest frame with muon and kaon mass hypotheses, respectively
    */
    double kinkDaughterMomentumInMotherRFKMu(const Particle* part);

    /**
    * returns kink daughter direction in mother rest frame with respect to mother momentum direction in the lab frame
    * with muon and kaon mass hypotheses, respectively
    */
    double kinkDaughterCosThetaInMotherRFKMu(const Particle* part);

    /**
    * returns kink daughter momentum in mother rest frame with muon and pion mass hypotheses, respectively
    */
    double kinkDaughterMomentumInMotherRFPiMu(const Particle* part);

    /**
    * returns kink daughter direction in mother rest frame with respect to mother momentum direction in the lab frame
    * with muon and pion mass hypotheses, respectively
    */
    double kinkDaughterCosThetaInMotherRFPiMu(const Particle* part);

    /**
    * returns kink daughter momentum in mother rest frame with electron and muon mass hypotheses, respectively
    */
    double kinkDaughterMomentumInMotherRFMuE(const Particle* part);

    /**
    * returns kink daughter direction in mother rest frame with respect to mother momentum direction in the lab frame
    * with electron and muon mass hypotheses, respectively
    */
    double kinkDaughterCosThetaInMotherRFMuE(const Particle* part);

    // Kink Daughter Measured Track Parameters

    /**
     * returns variables calculated for the kink daughter track
     */
    Manager::FunctionPtr kinkDaughterTrack(const std::vector<std::string>& arguments);

    /**
     * returns variables calculated for the initial kink daughter track
     */
    Manager::FunctionPtr kinkDaughterInitTrack(const std::vector<std::string>& arguments);

    /**
     * return the D0 impact parameter of kink daughter track associated to this particle at kink vertex
     */
    double kinkDaughterTrackD0AtKinkVertex(const Particle* part);

    /**
     * return the Z0 impact parameter of kink daughter track associated to this particle at kink vertex
     */
    double kinkDaughterTrackZ0AtKinkVertex(const Particle* part);

    /**
     * return the Pt of kink daughter track associated to this particle at kink vertex
     */
    double kinkDaughterPtAtKinkVertex(const Particle* part);

    /**
     * return the Pz of kink daughter track associated to this particle at kink vertex
     */
    double kinkDaughterPzAtKinkVertex(const Particle* part);

    /**
     * return the P of kink daughter track associated to this particle at kink vertex
     */
    double kinkDaughterPAtKinkVertex(const Particle* part);

    // Kink Mother Measured Track Parameters

    /**
     * returns variables calculated for the initial kink mother track
     */
    Manager::FunctionPtr kinkMotherInitTrack(const std::vector<std::string>& arguments);

    /**
     * return the D0 impact parameter of kink mother track associated to this particle at kink vertex
     */
    double kinkMotherTrackD0AtKinkVertex(const Particle* part);

    /**
     * return the Z0 impact parameter of kink mother track associated to this particle at kink vertex
     */
    double kinkMotherTrackZ0AtKinkVertex(const Particle* part);

    /**
     * return the Pt of kink mother track associated to this particle at kink vertex
     */
    double kinkMotherPtAtKinkVertex(const Particle* part);

    /**
     * return the Pz of kink mother track associated to this particle at kink vertex
     */
    double kinkMotherPzAtKinkVertex(const Particle* part);

    /**
     * return the P of kink mother track associated to this particle at kink vertex
     */
    double kinkMotherPAtKinkVertex(const Particle* part);

    //-----------------------------------------------
    // Check if the particle is a part of any Kink

    /**
     * return 1 if the particle is used in a Kink object and 0 otherwise
     */
    double particleIsInKink(const Particle* part);

    /**
     * return 1 if the particle is a mother in a Kink object and 0 otherwise
     */
    double particleIsMotherInKink(const Particle* part);

    /**
     * return 1 if the particle is a daughter in a Kink object and 0 otherwise
     */
    double particleIsDaughterInKink(const Particle* part);

    /**
     * return 1 if the particle is used as a split track to create a Kink object and 0 otherwise
     */
    double particleIsSplitKink(const Particle* part);

    //-----------------------------------------------
    // MC VARIABLES

    // Kink from track pair MC variables

    /**
     * return 1 if the mother and daughter tracks have MC relations and two of them are related and 0 otherwise
     */
    double kinkPairIsMCRelated(const Particle* part);

    /**
     * return 1 if the mother and daughter tracks have the same MCParticle relation and 0 otherwise
     */
    double kinkPairIsClone(const Particle* part);

    /**
     * return 1 if the mother and daughter tracks are mother/daughter related and 0 otherwise
     */
    double kinkPairIsReal(const Particle* part);

    /**
     * return 1 if the kink is a decay-in-flight and 0 otherwise
     */
    double kinkPairIsDecayInFlight(const Particle* part);

    /**
     * return 1 if the kink is a hadron scattering and 0 otherwise
     */
    double kinkPairIsHadronScattering(const Particle* part);

    // Kink from track pair MC kinematics

    /**
     * returns kink daughter momentum in mother rest frame MC (works only for kink created from a track pair)
     * Makes sense only for real decays-in-flight; however, it is not checked here
     */
    double kinkDaughterMomentumInMotherRFMC(const Particle* part);

    /**
     * returns kink daughter direction in mother rest frame with respect to mother momentum direction in the lab frame
     * MC (works only for kink created from a track pair)
     * Makes sense only for real decays-in-flight; however, it is not checked here
     */
    double kinkDaughterCosThetaInMotherRFMC(const Particle* part);

    // Kink from track pair daughter MC variables

    /**
     * returns variables for the MCParticle of the kink daughter for kinks created from two separate tracks.
     */
    Manager::FunctionPtr kinkPairDaughterMC(const std::vector<std::string>& arguments);

    // Kink from track pair mother MC variables

    /**
     * Helper function that return MCParticle of the kink mother for kinks created from two separate tracks.
     * It returns nullptr if MCParticle does not exist or the particle is not one of the required types
     */
    const MCParticle* kinkPairMotherMCParticle(const Particle* part);

    /**
     * returns variables for the MCParticle of the kink mother for kinks created from two separate tracks.
     */
    Manager::FunctionPtr kinkPairMotherMC(const std::vector<std::string>& arguments);

    /**
     * return generated PX of the kink mother at the decay vertex for kinks created from two separate tracks
     */
    double kinkPairMotherMCPXAtDecayVertex(const Particle* part);

    /**
     * return generated PY of the kink mother at the decay vertex for kinks created from two separate tracks
     */
    double kinkPairMotherMCPYAtDecayVertex(const Particle* part);

    /**
     * return generated PZ of the kink mother at the decay vertex for kinks created from two separate tracks
     */
    double kinkPairMotherMCPZAtDecayVertex(const Particle* part);

    /**
     * return generated PT of the kink mother at the decay vertex for kinks created from two separate tracks
     */
    double kinkPairMotherMCPTAtDecayVertex(const Particle* part);

    /**
     * return generated P of the kink mother at the decay vertex for kinks created from two separate tracks
     */
    double kinkPairMotherMCPAtDecayVertex(const Particle* part);

    /**
     * return generated E of the kink mother at the decay vertex for kinks created from two separate tracks
     */
    double kinkPairMotherMCEAtDecayVertex(const Particle* part);

  }
} // Belle2 namespace
