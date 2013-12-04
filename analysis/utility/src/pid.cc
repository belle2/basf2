#include <analysis/utility/pid.h>

#include <genfit/Track.h>

#include <top/dataobjects/TOPBarHit.h>
#include <arich/dataobjects/ARICHAeroHit.h>

#include <framework/datastore/StoreArray.h>
#include <framework/datastore/RelationIndex.h>

const TOPLikelihood* getTOPLikelihood(const Track& track)
{
  StoreArray<TOPLikelihood> toplogL;
  StoreArray<genfit::Track>        gfTracks;
  StoreArray<Track>          tracks;

  RelationIndex<genfit::Track, TOPLikelihood> gfTracksToTOPLogL(gfTracks, toplogL);

  if (!(tracks && gfTracks && toplogL && gfTracksToTOPLogL))
    return 0;

  int trackIndex = tracks.getPtr()->IndexOf(&track);

  if (trackIndex < 0)
    return 0;

  // It is assumed that Tracks and genfit::Tracks have the same indices
  if (!gfTracks[trackIndex])
    return 0;

  if (gfTracksToTOPLogL.getFirstElementFrom(gfTracks[trackIndex]))
    return gfTracksToTOPLogL.getFirstElementFrom(gfTracks[trackIndex])->to;

  return 0;
}

const DedxLikelihood* getDEDXLikelihood(const Track& track)
{
  StoreArray<DedxLikelihood> dedxlogL;
  StoreArray<genfit::Track>        gfTracks;
  StoreArray<Track>          tracks;

  RelationIndex<genfit::Track, DedxLikelihood> gfTracksToDEDXLogL(gfTracks, dedxlogL);

  if (!(tracks && gfTracks && dedxlogL && gfTracksToDEDXLogL))
    return 0;

  int trackIndex = tracks.getPtr()->IndexOf(&track);

  if (trackIndex < 0)
    return 0;

  // It is assumed that Tracks and genfit::Tracks have the same indices
  if (!gfTracks[trackIndex])
    return 0;

  if (gfTracksToDEDXLogL.getFirstElementFrom(gfTracks[trackIndex]))
    return gfTracksToDEDXLogL.getFirstElementFrom(gfTracks[trackIndex])->to;

  return 0;
}

const TOPLikelihood* getTOPLikelihood(const MCParticle* particle)
{

  StoreArray<MCParticle> mcParticles;
  StoreArray<TOPBarHit>  topTracks;
  StoreArray<TOPLikelihood> toplogL;

  RelationIndex< TOPBarHit, TOPLikelihood > relTrackLikelihoods(topTracks, toplogL);
  RelationIndex<MCParticle, TOPBarHit> relMCParticleToTOPBarHit(mcParticles, topTracks);

  if (!(relTrackLikelihoods && relMCParticleToTOPBarHit)) {
    return 0;
  }

  if (relMCParticleToTOPBarHit.getFirstElementFrom(particle)) {
    const TOPBarHit* track = relMCParticleToTOPBarHit.getFirstElementFrom(particle)->to;

    if (relTrackLikelihoods.getFirstElementFrom(track)) {

      return relTrackLikelihoods.getFirstElementFrom(track)->to;

    }
  }

  return 0;
}


const ARICHLikelihood* getARICHLikelihood(const MCParticle* particle)
{
  StoreArray<ARICHLikelihood> arichLikelihoods;
  StoreArray<ARICHAeroHit> arichAeroHits;
  StoreArray<MCParticle> mcParticles;

  RelationIndex<MCParticle, ARICHAeroHit> arichAeroHitRel(mcParticles, arichAeroHits);
  RelationIndex<ARICHAeroHit, ARICHLikelihood> relAeroToLikelihood(arichAeroHits, arichLikelihoods);

  if (!(arichAeroHitRel && relAeroToLikelihood))
    return 0;

  if (arichAeroHitRel.getFirstElementFrom(particle)) {
    const ARICHAeroHit* track = arichAeroHitRel.getFirstElementFrom(particle)->to;
    if (relAeroToLikelihood.getFirstElementFrom(track))
      return relAeroToLikelihood.getFirstElementFrom(track)->to;
  }

  return 0;
}

double getTOPPID(int hyp1, int hyp2, const TOPLikelihood* logL)
{
  double logl1 = 0.;
  double logl2 = 0.;

  if (hyp1 == hyp2 || hyp1 > 5 || hyp1 < 1 || hyp2 > 5 || hyp2 < 1) {
    return 0;
  }

  if (logL) {
    if (logL->getFlag()) {

      if (hyp1 == 1)
        logl1 = logL->getLogL_e();
      if (hyp1 == 2)
        logl1 = logL->getLogL_mu();
      if (hyp1 == 3)
        logl1 = logL->getLogL_pi();
      if (hyp1 == 4)
        logl1 = logL->getLogL_K();
      if (hyp1 == 5)
        logl1 = logL->getLogL_p();

      if (hyp2 == 1)
        logl2 = logL->getLogL_e();
      if (hyp2 == 2)
        logl2 = logL->getLogL_mu();
      if (hyp2 == 3)
        logl2 = logL->getLogL_pi();
      if (hyp2 == 4)
        logl2 = logL->getLogL_K();
      if (hyp2 == 5)
        logl2 = logL->getLogL_p();

      return logl1 - logl2;
    }
  }
  return 0;
}

double getARICHPID(int hyp1, int hyp2, const ARICHLikelihood* logL)
{
  double logl1 = 0.;
  double logl2 = 0.;

  if (hyp1 == hyp2 || hyp1 > 5 || hyp1 < 1 || hyp2 > 5 || hyp2 < 1) {
    return 0;
  }

  if (logL) {
    if (logL->getFlag()) {

      if (hyp1 == 1)
        logl1 = logL->getLogL_e();
      if (hyp1 == 2)
        logl1 = logL->getLogL_mu();
      if (hyp1 == 3)
        logl1 = logL->getLogL_pi();
      if (hyp1 == 4)
        logl1 = logL->getLogL_K();
      if (hyp1 == 5)
        logl1 = logL->getLogL_p();


      if (hyp2 == 1)
        logl2 = logL->getLogL_e();
      if (hyp2 == 2)
        logl2 = logL->getLogL_mu();
      if (hyp2 == 3)
        logl2 = logL->getLogL_pi();
      if (hyp2 == 4)
        logl2 = logL->getLogL_K();
      if (hyp2 == 5)
        logl2 = logL->getLogL_p();

      return logl1 - logl2;
    }
  }
  return 0;
}

double getDEDXPID(const Const::ChargedStable& hyp1, const Const::ChargedStable& hyp2, const DedxLikelihood* logL)
{
  if (logL)
    return logL->getLogLikelihood(hyp1) - logL->getLogLikelihood(hyp2);

  return 0;
}
