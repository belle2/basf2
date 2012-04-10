#include <analysis/utility/pid.h>

#include <top/dataobjects/TOPTrack.h>
#include <arich/dataobjects/ARICHAeroHit.h>

#include <framework/datastore/StoreArray.h>
#include <framework/datastore/RelationIndex.h>
#include <framework/datastore/RelationArray.h>

const TOPLikelihoods* getTOPLikelihoods(const MCParticle* particle)
{

  StoreArray<MCParticle> mcParticles;
  StoreArray<TOPTrack>  topTracks;
  StoreArray<TOPLikelihoods> toplogL;

  RelationArray  testrelTrackLikelihoods(topTracks, toplogL);
  RelationArray  testrelMCParticleToTOPTrack(mcParticles, topTracks);

  if (!(testrelTrackLikelihoods && testrelMCParticleToTOPTrack)) {
    return 0;
  }

  RelationIndex< TOPTrack, TOPLikelihoods > relTrackLikelihoods(topTracks, toplogL);
  RelationIndex<MCParticle, TOPTrack> relMCParticleToTOPTrack(mcParticles, topTracks);

  if (!(relTrackLikelihoods && relMCParticleToTOPTrack)) {
    return 0;
  }

  if (relMCParticleToTOPTrack.getFirstTo(particle)) {
    const TOPTrack* track = relMCParticleToTOPTrack.getFirstTo(particle)->to;

    if (relTrackLikelihoods.getFirstTo(track)) {

      return relTrackLikelihoods.getFirstTo(track)->to;

    }
  }

  return 0;
}


const ARICHLikelihoods* getARICHLikelihoods(const MCParticle* particle)
{

  StoreArray<ARICHLikelihoods> arichLikelihoods;
  StoreArray<ARICHAeroHit> arichAeroHits;
  StoreArray<MCParticle> mcParticles;

  RelationArray testarichAeroHitRel(mcParticles, arichAeroHits);
  RelationArray testrelAeroToLikelihood(arichAeroHits, arichLikelihoods);

  if (!(testarichAeroHitRel && testrelAeroToLikelihood)) {
    return 0;
  }


  RelationIndex<MCParticle, ARICHAeroHit> arichAeroHitRel(mcParticles, arichAeroHits);
  RelationIndex<ARICHAeroHit, ARICHLikelihoods> relAeroToLikelihood(arichAeroHits, arichLikelihoods);

  if (!(arichAeroHitRel && relAeroToLikelihood)) {
    return 0;
  }

  if (arichAeroHitRel.getFirstTo(particle)) {
    const ARICHAeroHit* track = arichAeroHitRel.getFirstTo(particle)->to;
    if (relAeroToLikelihood.getFirstTo(track)) {

      return relAeroToLikelihood.getFirstTo(track)->to;

    }
  }

  return 0;
}


double getTOPPID(int hyp1, int hyp2, const MCParticle* mctrack)
{

  double logl1 = 0.;
  double logl2 = 0.;

  if (hyp1 == hyp2 || hyp1 > 5 || hyp1 < 1 || hyp2 > 5 || hyp2 < 1) {
    return 0;
  }


  if (getTOPLikelihoods(mctrack)) {
    if (getTOPLikelihoods(mctrack)->getFlag()) {

      if (hyp1 == 1) {
        logl1 = getTOPLikelihoods(mctrack)->getLogL_e();
      }
      if (hyp1 == 2) {
        logl1 = getTOPLikelihoods(mctrack)->getLogL_mu();
      }
      if (hyp1 == 3) {
        logl1 = getTOPLikelihoods(mctrack)->getLogL_pi();
      }
      if (hyp1 == 4) {
        logl1 = getTOPLikelihoods(mctrack)->getLogL_K();
      }
      if (hyp1 == 5) {
        logl1 = getTOPLikelihoods(mctrack)->getLogL_p();
      }

      if (hyp2 == 1) {
        logl2 = getTOPLikelihoods(mctrack)->getLogL_e();
      }
      if (hyp2 == 2) {
        logl2 = getTOPLikelihoods(mctrack)->getLogL_mu();
      }
      if (hyp2 == 3) {
        logl2 = getTOPLikelihoods(mctrack)->getLogL_pi();
      }
      if (hyp2 == 4) {
        logl2 = getTOPLikelihoods(mctrack)->getLogL_K();
      }
      if (hyp2 == 5) {
        logl2 = getTOPLikelihoods(mctrack)->getLogL_p();
      }
      return logl1 - logl2;
    }
  }
  return 0;
}

double getARICHPID(int hyp1, int hyp2, const MCParticle* mctrack)
{

  double logl1 = 0.;
  double logl2 = 0.;

  if (hyp1 == hyp2 || hyp1 > 5 || hyp1 < 1 || hyp2 > 5 || hyp2 < 1) {
    return 0;
  }


  if (getARICHLikelihoods(mctrack)) {
    if (getARICHLikelihoods(mctrack)->getFlag()) {

      if (hyp1 == 1) {
        logl1 = getARICHLikelihoods(mctrack)->getLogL_e();
      }
      if (hyp1 == 2) {
        logl1 = getARICHLikelihoods(mctrack)->getLogL_mu();
      }
      if (hyp1 == 3) {
        logl1 = getARICHLikelihoods(mctrack)->getLogL_pi();
      }
      if (hyp1 == 4) {
        logl1 = getARICHLikelihoods(mctrack)->getLogL_K();
      }
      if (hyp1 == 5) {
        logl1 = getARICHLikelihoods(mctrack)->getLogL_p();
      }

      if (hyp2 == 1) {
        logl2 = getARICHLikelihoods(mctrack)->getLogL_e();
      }
      if (hyp2 == 2) {
        logl2 = getARICHLikelihoods(mctrack)->getLogL_mu();
      }
      if (hyp2 == 3) {
        logl2 = getARICHLikelihoods(mctrack)->getLogL_pi();
      }
      if (hyp2 == 4) {
        logl2 = getARICHLikelihoods(mctrack)->getLogL_K();
      }
      if (hyp2 == 5) {
        logl2 = getARICHLikelihoods(mctrack)->getLogL_p();
      }
      return logl1 - logl2;
    }
  }
  return 0;
}
