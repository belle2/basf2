/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <vector>
#include <string>
#include <analysis/VariableManager/Manager.h>
#include "TVector3.h"

namespace Belle2 {
  class Particle;

  namespace Variable {


    /**
     * return X component of the tag vertex
     *
     * requires that Vertex <-> Particle relation exists (returns NaN if it doesn't)
     */
    double particleTagVx(const Particle* particle);

    /**
     * return Y component of the tag vertex
     *
     * requires that Vertex <-> Particle relation exists (returns NaN if it doesn't)
     */
    double particleTagVy(const Particle* particle);

    /**
     * return Z component of the tag vertex
     *
     * requires that Vertex <-> Particle relation exists (returns NaN if it doesn't)
     */
    double particleTagVz(const Particle* particle);


    /**
     * return MC X component of the tag vertex
     *
     * requires that Vertex <-> Particle relation exists (returns NaN if it doesn't)
     */
    double particleTruthTagVx(const Particle* particle);

    /**
     * return MC Y component of the tag vertex
     *
     * requires that Vertex <-> Particle relation exists (returns NaN if it doesn't)
     */
    double particleTruthTagVy(const Particle* particle);

    /**
     * return MC Z component of the tag vertex
     *
     * requires that Vertex <-> Particle relation exists (returns NaN if it doesn't)
     */
    double particleTruthTagVz(const Particle* particle);

    /**
     * return the p-Value of the tag vertex
     *
     * requires that Vertex <-> Particle relation exists (returns NaN if it doesn't)
     */
    double particleTagVpVal(const Particle* particle);

    /**
     * return the number of tracks in the tag vertex
     *
     * requires that Vertex <-> Particle relation exists (returns NaN if it doesn't)
     */
    double particleTagVNTracks(const Particle* particle);

    /**
     * return the fit type of the tag vertex
     *
     * requires that Vertex <-> Particle relation exists (returns NaN if it doesn't)
     */
    double particleTagVNTracks(const Particle* particle);

    /**
     * returns the number of degrees of freedom in the tag vertex fit. can be non-integer due to adaptive vertex fitting.
     *
     * requires that TagVertex <-> Particle relation exists (returns NaN if not)
     */
    double particleTagVNDF(const Particle* particle);

    /**
     * returns chi^2 value of the tag vertex fit result.
     *
     * requires that TagVertex <-> Particle relation exists (returns NaN if not)
     */
    double particleTagVChi2(const Particle* particle);

    /**
     * returns the IP component of chi^2 value of the tag vertex fit result.
     *
     * requires that TagVertex <-> Particle relation exists (returns NaN if not)
     */
    double particleTagVChi2IP(const Particle* particle);


    /**
     * return the error of the X component of the tag vertex
     *
     * requires that Vertex <-> Particle relation exists (returns NaN if it doesn't)
     */
    double particleTagVxErr(const Particle* particle);

    /**
     * return the error of the Y component of the tag vertex
     *
     * requires that Vertex <-> Particle relation exists (returns NaN if it doesn't)
     */
    double particleTagVyErr(const Particle* particle);

    /**
     * return the error of the Z component of the tag vertex
     *
     * requires that Vertex <-> Particle relation exists (returns NaN if it doesn't)
     */
    double particleTagVzErr(const Particle* particle);

    /**
     * return Delta T (Brec - Btag) in ps
     *
     * requires that Vertex <-> Particle relation exists (returns NaN if it doesn't)
     */
    double particleDeltaT(const Particle* particle);

    /**
     * return Delta T as it was used in Belle in ps
     *
     * requires that Vertex <-> Particle relation exists (returns NaN if it doesn't)
     */
    double particleDeltaTBelle(const Particle* particle);


    /**
     * return Delta T error in ps
     *
     * requires that Vertex <-> Particle relation exists (returns NaN if it doesn't)
     */
    double particleDeltaTErr(const Particle* particle);

    /**
     * return Delta T - MCDeltaT in ps
     *
     * requires that Vertex <-> Particle relation exists (returns NaN if it doesn't)
     */
    double particleDeltaTRes(const Particle* particle);

    /**
     * return generated Delta Tau (Brec - Btag), i.e. difference of proper times (in ps)
     *
     * requires that Vertex <-> Particle relation exists (returns NaN if it doesn't)
     */
    double particleMCDeltaTau(const Particle* particle);

    /**
     * return generated Delta T (Brec - Btag) in boost-direction kinematic approximation (in ps)
     *
     * requires that Vertex <-> Particle relation exists (returns NaN if it doesn't)
     */
    double particleMCDeltaT(const Particle* particle);

    /**
     * return generated Delta l (Brec - Btag) in boost-direction (in cm)
     *
     * requires that Vertex <-> Particle relation exists (returns NaN if it doesn't)
     */
    double particleMCDeltaL(const Particle* particle);


    /**
     * return Delta Z (Brec - Btag) in cm
     *
     * requires that Vertex <-> Particle relation exists (returns NaN if it doesn't)
     */
    double particleDeltaZ(const Particle* particle);

    /**
     * returns the error of Delta Z (Brec - Btag) in cm
     *
     * requires that Vertex <-> Particle relation exists (returns NaN if it doesn't)
     */
    double particleDeltaZErr(const Particle* particle);

    /**
     * return Delta Boost direction (Brec - Btag) in cm
     *
     * requires that Vertex <-> Particle relation exists (returns NaN if it doesn't)
     */
    double particleDeltaB(const Particle* particle);

    /**
     * returns the error of the difference Delta Boost direction (Brec - Btag) in cm
     *
     * requires that Vertex <-> Particle relation exists (returns NaN if it doesn't)
     */
    double particleDeltaBErr(const Particle* particle);

    /**
     * returns the vertex component in the boost direction
     *
     */
    double vertexBoostDirection(const Particle* particle);

    /**
     * returns the vertex component in the direction orthogonal to the boost
     *
     */
    double vertexOrthogonalBoostDirection(const Particle* particle);

    /**
     * returns the MC vertex component in the boost direction
     *
     */
    double vertexTruthBoostDirection(const Particle* particle);

    /**
     * returns the MC vertex component in the direction orthogonal to the boost
     *
     */
    double vertexTruthOrthogonalBoostDirection(const Particle* particle);

    /**
     * returns the vertex error component in the boost direction
     *
     */
    double vertexErrBoostDirection(const Particle* particle);

    /**
     * returns the vertex error in the direction orthogonal boost direction
     *
     */
    double vertexErrOrthBoostDirection(const Particle* particle);



    /**
     *  Returns the TagV component in the boost direction
     *
     */
    double tagVBoostDirection(const Particle* particle);

    /**
     * Returns the TagV component in the direction orthogonal to the boost
     *
     */
    double tagVOrthogonalBoostDirection(const Particle* particle);

    /**
     * Returns the MC TagV component in the boost direction
     *
     */
    double tagVTruthBoostDirection(const Particle* particle);

    /**
     * Returns the MC TagV component in the direction orthogonal to the boost
     *
     */
    double tagVTruthOrthogonalBoostDirection(const Particle* particle);

    /**
     * Returns the error of TagV in the boost direction
     *
     */
    double tagVErrBoostDirection(const Particle* particle);

    /**
     * Returns the error of TagV in the direction orthogonal to the boost
     *
     */
    double tagVErrOrthogonalBoostDirection(const Particle* particle);

    /**
     * Returns internal mc flavor used in the TagV module. Only to be used for internal checks by developers.
     *
     */
    double particleInternalTagVMCFlavor(const Particle* particle);

    /**
     * Return the norm of the momentum of the tag track indexed by trackIndex
     *
     */
    double tagTrackMomentum(const Particle* part, const std::vector<double>& trackIndex);

    /**
     * Return the X component of the momentum of the tag track indexed by trackIndex
     *
     */
    double tagTrackMomentumX(const Particle* part, const std::vector<double>& trackIndex);

    /**
     * Return the Y component of the momentum of the tag track indexed by trackIndex
     *
     */
    double tagTrackMomentumY(const Particle* part, const std::vector<double>& trackIndex);

    /**
     * Return the Z component of the momentum of the tag track indexed by trackIndex
     *
     */
    double tagTrackMomentumZ(const Particle* part, const std::vector<double>& trackIndex);

    /**
     * Returns the d0 parameter of the tag track indexed by trackIndex
     *
     */
    double tagTrackD0(const Particle* part, const std::vector<double>& trackIndex);

    /**
     * Returns the z0 parameter of the tag track indexed by trackIndex
     *
     */
    double tagTrackZ0(const Particle* part, const std::vector<double>& trackIndex);

    /**
     * returns the number of tracks used by rave to fit the vertex (not counting the ones coming from Kshorts)
     *
     */
    double particleTagVNFitTracks(const Particle* particle);

    /**
     * returns the weight assigned by Rave to the tag track indexed by trackIndex
     *
     */
    double tagTrackRaveWeight(const Particle* part, const std::vector<double>& trackIndex);

    /**
     * returns the distance between the centre of the constraint and the tag track indexed by track index
     *
     */
    double tagTrackDistanceToConstraint(const Particle* part, const std::vector<double>& trackIndex);


    /**
     * returns the estimated uncertainty on the distance between the centre of the constraint and
     * the tag track indexed by track index
     *
     */
    double tagTrackDistanceToConstraintErr(const Particle* part, const std::vector<double>& trackIndex);

    /**
     * returns the significance of the distance between the centre of the constraint and
     * the tag track indexed by track index (computed as distance / uncertainty)
     *
     */
    double tagTrackDistanceToConstraintSignificance(const Particle* part, const std::vector<double>& trackIndex);


    /**
     * returns the distance between the centre of the constraint and the tag vtx
     *
     */
    double tagVDistanceToConstraint(const Particle* part);

    /**
     * returns the estimated uncertainty on the distance between the centre of the constraint and
     * the tag vtx
     *
     */
    double tagVDistanceToConstraintErr(const Particle* part);

    /**
     * returns the significance of the distance between the tag vtx and the centre of the constraint
     * (computed as distance / uncertainty)
     */
    double tagVDistanceToConstraintSignificance(const Particle* part);


    /**
     * returns the measured distance between the tag vtx and the tag track indexed by trackIndex
     *
     */
    double tagTrackDistanceToTagV(const Particle* part, const std::vector<double>& trackIndex);

    /**
     * returns the estimated uncertainty on the distance between the tag vtx and
     * the tag track indexed by trackIndex
     *
     */
    double tagTrackDistanceToTagVErr(const Particle* part, const std::vector<double>& trackIndex);

    /**
     * returns the significance of the distance between the tag vtx and the tag track indexed by trackIndex
     * (computed as distance / uncertainty)
     *
     */
    double tagTrackDistanceToTagVSignificance(const Particle* part, const std::vector<double>& trackIndex);

    /**
     * returns the true distance between the true B Tag decay vertex and the particle
     * corresponding to the tag vtx track indexed by trackIndex.
     *
     */
    double tagTrackTrueDistanceToTagV(const Particle* part, const std::vector<double>& trackIndex);

    /**
     * Returns the vector between the mc particle corresponding to the ith tag vtx track
     * and the true tag B decay vertex.
     *
     */
    TVector3 tagTrackTrueVecToTagV(const Particle* part, const std::vector<double>& trackIndex);

    /**
     * Returns the X coordinate of the vector between the mc particle corresponding to the ith tag vtx track
     * and the true tag B decay vertex.
     *
     */
    double tagTrackTrueVecToTagVX(const Particle* part, const std::vector<double>& trackIndex);

    /**
     * Returns the Y coordinate of the vector between the mc particle corresponding to the ith tag vtx track
     * and the true tag B decay vertex.
     *
     */
    double tagTrackTrueVecToTagVY(const Particle* part, const std::vector<double>& trackIndex);

    /**
     * Returns the Z coordinate of the vector between the mc particle corresponding to the ith tag vtx track
     * and the true tag B decay vertex.
     *
     */
    double tagTrackTrueVecToTagVZ(const Particle* part, const std::vector<double>& trackIndex);

    /**
     * return  the true momentum of the MC particle corresponding to the ith tag vtx track.
     *
     */
    TVector3 tagTrackTrueMomentum(const Particle* part, const std::vector<double>& trackIndex);

    /**
     * return the X component of the true momentum of the MC particle corresponding to the ith tag vtx track.
     *
     */
    double tagTrackTrueMomentumX(const Particle* part, const std::vector<double>& trackIndex);

    /**
     * return the Y component of the true momentum of the MC particle corresponding to the ith tag vtx track.
     *
     */
    double tagTrackTrueMomentumY(const Particle* part, const std::vector<double>& trackIndex);

    /**
     * return the Z component of the true momentum of the MC particle corresponding to the ith tag vtx track.
     *
     */
    double tagTrackTrueMomentumZ(const Particle* part, const std::vector<double>& trackIndex);

    /**
     * return the true origin of the MC particle corresponding to the ith tag vtx track.
     *
     */
    TVector3 tagTrackTrueOrigin(const Particle* part, const std::vector<double>& trackIndex);

    /**
     * return the X component of the true origin of the MC particle corresponding to the ith tag vtx track.
     *
     */
    double tagTrackTrueOriginX(const Particle* part, const std::vector<double>& trackIndex);

    /**
     * return the Y component of the true origin of the MC particle corresponding to the ith tag vtx track.
     *
     */
    double tagTrackTrueOriginY(const Particle* part, const std::vector<double>& trackIndex);

    /**
     * return the Z component of the true origin of the MC particle corresponding to the ith tag vtx track.
     *
     */
    double tagTrackTrueOriginZ(const Particle* part, const std::vector<double>& trackIndex);

    /**
     * return the status of the fit performed with the true track parameters.
     * 0 fit performed with measured parameters
     * 1 fit performed with true parameters
     * 2 unable to recover truth parameters
     *
     */
    int fitTruthStatus(const Particle* part);

    /**
     * return the status of the fit performed with the rolled back position of tracks.
     * 0 fit performed with measured parameters
     * 1 fit performed with rolled back tracks
     * 2 unable to recover truth parameters
     *
     */
    int rollbackStatus(const Particle* part);

    //**********************************
    //Meta variables
    //**********************************

    /**
     * This is a pointer to the various functions that compute information related to the tag tracks
     *
     */
    typedef double (*TagTrFPtr)(const Particle*, const std::vector<double>&);

    /**
     * returns the average over the tag tracks of the variable given in argument.
     * The variable is one of the tagTrack... variables. Tag tracks which are assigned a 0
     * weight are ignored
     *
     */
    Manager::FunctionPtr tagTrackAverage(const std::vector<std::string>& variable);

    /**
     * returns the maximum over the tag tracks of the variable given in argument.
     * The variable is one of the tagTrack... variables. Tag tracks which are assigned a 0
     * weight are ignored
     *
     */
    Manager::FunctionPtr tagTrackMax(const std::vector<std::string>& variable);

    /**
     * returns the minimum over the tag tracks of the variable given in argument.
     * The variable is one of the tagTrack... variables. Tag tracks which are assigned a 0
     * weight are ignored
     *
     */
    Manager::FunctionPtr tagTrackMin(const std::vector<std::string>& variable);

    /**
     * returns the sum over the tag tracks of the variable given in argument.
     * The variable is one of the tagTrack... variables. Tag tracks which are assigned a 0
     * weight are ignored
     *
     */
    Manager::FunctionPtr tagTrackSum(const std::vector<std::string>& variable);

    /**
     * returns the average over the tag tracks of the square of the variable given in argument.
     * The variable is one of the tagTrack... variables. Tag tracks which are assigned a 0
     * weight are ignored
     *
     */
    Manager::FunctionPtr tagTrackAverageSquares(const std::vector<std::string>& variable);

    /**
     * returns the average over the tag tracks of the variable given in argument, weighted
     * by the weights of the tag vertex fitter
     * The variable is one of the tagTrack... variables.
     *
     */
    Manager::FunctionPtr tagTrackWeightedAverage(const std::vector<std::string>& variable);

    /**
     * returns the average over the tag tracks of the square of the variable given in argument, weighted
     * by the weights of the tag vertex fitter
     * The variable is one of the tagTrack... variables.
     *
     */
    Manager::FunctionPtr tagTrackWeightedAverageSquares(const std::vector<std::string>& variable);

  }
}

