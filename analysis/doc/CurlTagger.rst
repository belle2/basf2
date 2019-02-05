.. _CurlTagger:

CurlTagger
==========

Curl Tagger is a tool designed to identify and tag extra tracks caused by low pt particles curling around the detector. 

The module takes a particle list and assigns the first particle to a new bundle, a grouping of particles that the module believes to actually be the same particle. The module then loops through the rest of the particle list. For each subsequent particle a **selector** compares the particle to all existing bundles. This is done by comparing with each particle in the bundle and averaging over the result. **IF** the selector response is above a **responseCut** value for any bundle the particle is added to the bundle with the largest response. **ELSE** the particle starts a new bundle. Once the entire particle list has been assigned to bundles the particles in each bundle are ranked by 25dr^2 + dz^2, the particle with the lowest score is assigned to be the 'true' particle and the others are tagged as curl particles with extraInfo(isCurl)=1.

There are currently two **selectors**, the FastBDT based 'mva' and 'cut' which uses a few basic cuts. 'cut' is only optimised for Belle data and is included mainly for comparison purposes as this selection has previously been used (see BN#1079). It is reccomended to use 'mva'. If you have an idea to improve these selectors or to create a new one please do so and make a pull request.

If you set the mcTruth option to True this procedure will be repeated but instead of a selector particles are added to bundles based on their genParticleIndex and tagged with extraInfo(isTruthCurl)=1. This will allow you to check the performance of the module.

The responseCut has been set to maximise the average accuracy across the standard charged pion particle lists. If do want to optimise it for your use case you will need to run your script with different values and check 'isCurl' against 'isTruthCurl' as the cut is embedded deep in the selection.

.. warning:: The 'cut' selector is not optimised for Belle II data. Please use the 'mva' selector. If you require the simpler cut based selection please consider optimising it and creating a pull request.
