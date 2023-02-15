.. _convertedobjects:

Converted data objects and other information
============================================
This section briefly describes what is converted, in what way, and how the
converted objects should be used in the analysis.

-----------------------------
Charged Final State Particles
-----------------------------
BASF and BASF2 use different Helix parameterisations, however there is a
well defined transformation from one parameterisation to the other. The Belle MDST
format stores in addition to the five helix parameters also the reference point
(or pivot point), which is assumed to be always point ``(0,0,0)`` in the case of
Belle II MDST format. Therefore in the conversion process any charged track
found in Belle MDST with a pivot point different from ``(0,0,0)`` is first transformed
such that its pivot point becomes ``(0,0,0)``. This is especially important
in the case of conversion of ``V0``'s daughter tracks.

.. note::
   There is nothing special to note regarding the usage of converted charged
   tracks in BASF2. Use the usual ``fillParticleList(...)`` or
   ``fillParticleLists(...)`` analysis functions to create and fill charged
   kaon, pion, electron, muon and proton `ParticleList`_ s.

.. _ParticleList: https://software.belle2.org/|release|/classBelle2_1_1ParticleList.html

~~~~~~~~~~~~~~~~~~~~~~~
Particle Identification
~~~~~~~~~~~~~~~~~~~~~~~
Despite the different parameterisations, charged final state particles can still
be reconstructed using the `fillParticleList` function in basf2.
But due to the different definition, as well as detector, it is not
recommended to use Belle II style PID in b2bii.

basf provided three different packages for PID:
 * atc_pid (KID) to separate kaons and pions, but also used for proton id
 * eID (electron ID) to separate electrons from hadrons
 * muid (muon ID) to separate muons from hadrons

Each of them in its own way combined information collected from various sub detector
systems (CDC, ACC, TOF, ECL, KLM). The combination of individual likelihoods from
each sub detector system is in some cases (eID) combined with the usage of external
information, such as a priori probabilities of each particle type that is read from
the Belle DB.
Due to this fact the Belle-like PID probabilities can not be reproduced in BASF2 from
the raw likelihoods and special Belle-legacy variables that reproduce them are
introduced.

Alternatively, we can use the following predefined Belle-style PID variables to
reproduce them:

   +------------------------------+------------------------------+------------------+
   | Separation                   | basf                         | basf2            |
   +==============================+==============================+==================+
   | :math:`K` vs :math:`\pi`     | atc_pid(3,1,5,3,2).prob(...) | atcPIDBelle(3,2) |
   +------------------------------+------------------------------+------------------+
   | :math:`p` vs :math:`\pi`     | atc_pid(3,1,5,4,2).prob(...) | atcPIDBelle(4,2) |
   +------------------------------+------------------------------+------------------+
   | :math:`p` vs :math:`K`       | atc_pid(3,1,5,4,3).prob(...) | atcPIDBelle(4,3) |
   +------------------------------+------------------------------+------------------+
   | electron vs hadron           | eid.prob(3,-1,5)             | eIDBelle         |
   +------------------------------+------------------------------+------------------+
   | muon likelihood              | Muid_mdst.Muon_likelihood()  | muIDBelle        |
   +------------------------------+------------------------------+------------------+
   | muon likelihood quality flag | Muid_mdst.Prerejection()     | muIDBelleQuality |
   +------------------------------+------------------------------+------------------+

-----------------------------
Neutral Final State Particles
-----------------------------
When it comes to ECL related objects the BASF and basf2 MDST data formats differ
substantially which makes one-to-one conversion impossible. The reconstructed
``ECL clusters``, both charged (with matched charged track) and neutral
(without matched charged track), are in BASF stored as ``Mdst_ecl`` (and ``Mdst_ecl_aux``)
and in BASF2 as ECLCLusters. These two datatypes match quite good.
However, BASF MDST format has two additional data types: ``Mdst_Gamma`` and ``Mdst_Pi0``,
for which there exist no equivalent data type in the BASF2 MDST format.
Instead, the B2BII converter by default creates ``gamma:mdst`` and ``pi0:mdst``
ParticleLists, which are filled with particle objects created for each
``Mdst_Gamma`` and ``Mdst_Pi0`` entry.

.. warning::
   Use ``gamma:mdst`` and ``pi0:mdst`` ParticleLists.
   Don't use ``fillParticleList(...)`` to create photon candidates or don't
   reconstruct ``pi0`` candidate from pairs of two photons by yourself.

.. note::
   Mass-constraint fit has been applied to ``pi0`` candidates in ``Mdst_Pi0``.
   However, covariance matrix is not stored in panther tables. If you need
   this information in the analysis, you can redo the mass constraint fit in basf2.

Since release-07, you can set ``convertNbar`` to ``true`` to copy Particles with energy
above 500 MeV in ``gamma:mdst`` to create a ParticleList ``anti-n0:mdst``. In the steering file,
you should use the module ``BelleNbarMVA`` to evaluate an MVA dedicated to the separation of
anti-neutrons from photons. As for the kinematic variable, you are advised to use
``reconstructDecayWithNeutralHadron`` to reconstruct the neutral hadron's 4-momentum
with mother mass constraint.

.. note::
   See also Anti-neutron identification (Belle Note 1592) for more information about
   ``BelleNbarMVA``.

.. warning::
   ``reconstructDecayWithNeutralHadron`` is only available in light-releases.

------------
V0 Particles
------------
As mentioned above (section on charged final state particles) all charged
tracks are parametrised with helix with the reference point set to (0,0,0)
in BASF2. This is not optimal in the case of ``V0s`` whose decay vertices can
be far away from the origin. Therefore all ``V0`` candidates from the ``Mdst_Vee2``
table in BASF are converted to Particles and collected in the ``K_S0:mdst``,
``Lambda0:mdst``, and ``gamma:v0mdst`` ParticleLists.
The created particles have momentum and decay vertex position set to values
given in Belle's ``Mdst_Vee2`` table and their daughters particles with
momentum and position at the pivot equal to ``V0`` decay vertex. In addition,
The Belle's quality indicators for :math:`K_S^0` and :math:`\Lambda` are
converted as well and attached as ``extraInfo`` variables.

~~~~~~~~~~~~~~~~~
Quality indicator
~~~~~~~~~~~~~~~~~

The quality indicators for :math:`K_S^0` and :math:`\Lambda` as estimated by the
`findKs`_ and `nisKsFinder`_ (for :math:`K_S^0`), and `FindLambda`_ (for
:math:`\Lambda^0`) are available as

   +-------------------------+-------------------------+
   | basf                    | basf2                   |
   +-------------------------+-------------------------+
   | findKs.goodKs()         | extraInfo(goodKs)       |
   +-------------------------+-------------------------+
   | nisKsFinder.nb_vlike()  | extraInfo(ksnbVLike)    |
   +-------------------------+-------------------------+
   | nisKsFinder.nb_nolam()  | extraInfo(ksnbNoLam)    |
   +-------------------------+-------------------------+
   | nisKsFinder.standard()  | extraInfo(ksnbStandard) |
   +-------------------------+-------------------------+
   | findLambda.goodLambda() | extraInfo(goodLambda)   |
   +-------------------------+-------------------------+

.. _findKs: http://belle.kek.jp/secured/belle_note/gn323/note323.ps.gz

.. _nisKsFinder: http://belle.kek.jp/secured/belle_note/gn1253/bn_1253v1.pdf

.. _FindLambda: https://belle.kek.jp/secured/belle_note/gn684/bn684.ps.gz

The vertex fit information of ``V0`` particles is also attached as ``extraInfo`` variables.

---------------------------
:math:`K_{L}^{0}` Particles
---------------------------
:math:`K_{L}^{0}` candidates are stored in the default ``K_L0:mdst`` ParticleList.

.. note::
   Use K_L0:mdst ParticleList. Don't use ``fillParticleList(...)``.

In Belle there was no explicit MC Matching for :math:`K_L^0`. Instead, people
used a hack. If a (MC) :math:`K_L^0` in ``Gen_HEPEVT`` panther table is found,
we set a relation to the (best) reconstructed :math:`K_L^0` with no
associated ``ECLCluster`` and within 15 degrees in :math:`\phi` and :math:`\theta`.
The cluster position for ``KLMClusters`` is only available if a :math:`K_L^0` was
associated to it, since this information is extracted from the :math:`K_L^0`.

--------------------------
Event Classification flags
--------------------------
Event classification is a sort of Data-mining process, which separates the Belle
data sample into several skims based on the underlying physics process.
As an event-based flag, event classification flags are converted and attached
as ``eventExtraInfo``.

Use the following Belle II variables to get the corresponding event classification
flags:


   +-------------------------+---------------------------------------+
   | basf                    | basf2                                 |
   +-------------------------+---------------------------------------+
   | evtcls_flag(N)          | eventExtraInfo(evtcls_flagN)          |
   +-------------------------+---------------------------------------+
   | evtcls_flag2(N)         | eventExtraInfo(evtcls_flag1N)         |
   +-------------------------+---------------------------------------+
   | evtcls_hadronic_flag(N) | eventExtraInfo(evtcls_hadronic_flagN) |
   +-------------------------+---------------------------------------+

.. note::
   Explanation of the event type can be found at
   `here <https://belle.kek.jp/secured/wiki/doku.php?id=software:event_classification>`__.
   Please refer to `bn390`_ for the details of Hadronic Event Selection.

.. _bn390: http://belle.kek.jp/secured/belle_note/gn390/bn390_012901.ps.gz


