#include <analysis/utility/makeMother.h>
#include <analysis/KFit/MakeMotherKFit.h>

unsigned makeMother(VertexFitKFit& kv, Belle1::Particle& mother)
{

  unsigned n = kv.getTrackCount();
  MakeMotherKFit kmm;
  kmm.setMagneticField(1.5);

  for (unsigned i = 0; i < n; ++i) {
    kmm.addTrack(kv.getTrackMomentum(i),
                 kv.getTrackPosition(i),
                 kv.getTrackError(i),
                 kv.getTrack(i).getCharge());

    for (unsigned j = i + 1; j < n; ++j) {
      kmm.setCorrelation(kv.getCorrelation(i, j));
    }
  }

  kmm.setVertex(kv.getVertex());
  kmm.setVertexError(kv.getVertexError());

  unsigned err = kmm.doMake();
  if (err != 0)return 0;
  mother.momentum().momentumPosition(kmm.getMotherMomentum(),
                                     kmm.getMotherPosition(),
                                     kmm.getMotherError());
  return 1;
}

unsigned makeMother(MassFitKFit& km, Belle1::Particle& mother)
{

  unsigned n = km.getTrackCount();
  MakeMotherKFit kmm;
  kmm.setMagneticField(1.5);

  for (unsigned i = 0; i < n; ++i) {
    kmm.addTrack(km.getTrackMomentum(i),
                 km.getTrackPosition(i),
                 km.getTrackError(i),
                 km.getTrack(i).getCharge());

    if (km.getFlagFitWithVertex())
      kmm.setTrackVertexError(km.getTrackVertexError(i));

    for (unsigned j = i + 1; j < n; ++j) {
      kmm.setCorrelation(km.getCorrelation(i, j));
    }
  }

  kmm.setVertex(km.getVertex());

  if (km.getFlagFitWithVertex()) {
    kmm.setVertexError(km.getVertexError());
  }

  unsigned err = kmm.doMake();
  if (err != 0)return 0;
  mother.momentum().momentumPosition(kmm.getMotherMomentum(),
                                     kmm.getMotherPosition(),
                                     kmm.getMotherError());
  return 1;
}

/* Updates the momentum of mother particle after mass constrained vertex fit. */
unsigned makeMother(MassVertexFitKFit& km, Belle1::Particle& mother)
{

  unsigned n = km.getTrackCount();
  MakeMotherKFit kmm;
  kmm.setMagneticField(1.5);

  for (unsigned i = 0; i < n; ++i) {
    kmm.addTrack(km.getTrackMomentum(i),
                 km.getTrackPosition(i),
                 km.getTrackError(i),
                 km.getTrack(i).getCharge());

    for (unsigned j = i + 1; j < n; ++j) {
      kmm.setCorrelation(km.getCorrelation(i, j));
    }
  }

  kmm.setVertex(km.getVertex());
  kmm.setVertexError(km.getVertexError());

  unsigned err = kmm.doMake();
  if (err != 0)return 0;
  mother.momentum().momentumPosition(kmm.getMotherMomentum(),
                                     kmm.getMotherPosition(),
                                     kmm.getMotherError());
  return 1;
}
