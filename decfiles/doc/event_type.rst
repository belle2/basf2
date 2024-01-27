Event Type
==========

This section defines the scheme for MC **event types**. The principle is that event types should
describe the way a sample was produced, and classify the specific decay channels according to their
global features. A multiple digit number represents the event type, carrying this useful information.
An event type number is more convenient for distributed MC production.

The event type is an 10-digit number **gsdcltnkeu** where each individual digit is assigned a meaning
according to the following principles:

- **g:** General Flag, indicating the general category of the event type and the event production
  mechanism.
- **s:** Selection Flag, indicating the criteria used to define this event type, based on the presence
  of certain particles.
- **d:** Decay Elag, indicating the general features of the decays considered for this event type.
- **c:** Charm Flag, for the selected decays in this event type.
- **l:** Lepton Flag, for the selected decays in this event type.
- **t:** Tracks Flag, indicating the number of charged tracks in the selected decays.
- **n:** Neutrals Flag, indicating the number of neutral particles in the selected decays.
- **k:** Kaon Flag, indicating the number of neutral kaons, neutrons and lambdas in the selected
  decays.
- **e:** Extra Flag, for disambiguation in some cases.
- **u:** User Flag, used for generating decays with different generators.

This scheme is slightly modified to describe particle gun and background MC production. These
exceptions are described later in this page. In addition to a number, the event types are given short
names for human readability.

Event type definitions should be listed in the header of each decay file.

Flag Descriptions
-----------------

General Flag
++++++++++++

The definitions of the **General Flag** are given in the table below. The value zero is not used here.
At the current time only 6 General Flags are defined. The exotic category will typically cover new
physics.

+--------------+--------------------------------------+
| General Flag | Definition                           |
+==============+======================================+
| 0            | Undefined                            |
+--------------+--------------------------------------+
| 1            | Events containing a :math:`b` quark  |
+--------------+--------------------------------------+
| 2            | Events containing a :math:`c` quark  |
+--------------+--------------------------------------+
| 3            | Light flavour pairs and lepton pairs |
+--------------+--------------------------------------+
| 4            | Exotic                               |
+--------------+--------------------------------------+
| 5            | Particle gun                         |
+--------------+--------------------------------------+
| 6            | Machine background                   |
+--------------+--------------------------------------+
| 7-9          | Undefined                            |
+--------------+--------------------------------------+

Selection Flag
++++++++++++++

The **Selection Flag** indicates which particles are required to be present in the event, and possibly
forced to decay to specific modes. The definitions of the selection flag are given in the table below,
and depend on the value of the General Flag. The Exotic Flags are yet to be defined, but may include
modes such as dark sectors.

+----------------+-------------------------------------------------------------------------------------------------------------+
| Selection Flag | Definition if the General Flag is 1 or 2                                                                    |
+================+=============================================================================================================+
| 0              | Inclusive :math:`b` (:math:`c`) hadron production                                                           |
+----------------+-------------------------------------------------------------------------------------------------------------+
| 1              | :math:`B^0` (:math:`D^+`)                                                                                   |
+----------------+-------------------------------------------------------------------------------------------------------------+
| 2              | :math:`B^+` (:math:`D^0`)                                                                                   |
+----------------+-------------------------------------------------------------------------------------------------------------+
| 3              | :math:`B^0_s` (:math:`D^+_s`)                                                                               |
+----------------+-------------------------------------------------------------------------------------------------------------+
| 4              | :math:`J/\psi`                                                                                              |
+----------------+-------------------------------------------------------------------------------------------------------------+
| 5              | :math:`\Lambda_b` (:math:`\Lambda_c`)                                                                       |
+----------------+-------------------------------------------------------------------------------------------------------------+
| 6              | Other type of :math:`b` (:math:`c`) resonance, the extra flag specifies the type of resonance               |
+----------------+-------------------------------------------------------------------------------------------------------------+
| 7              | Other type of :math:`b\bar{b}` (:math:`c\bar{c}`) resonance, the extra flag specifies the type of resonance |
+----------------+-------------------------------------------------------------------------------------------------------------+
| 8              | Double (:math:`c\bar{c}`) resonance, the extra flag specifies the type of resonance                         |
+----------------+-------------------------------------------------------------------------------------------------------------+

+----------------+------------------------------------+
| Selection Flag | Definition if the General Flag = 3 |
+================+====================================+
| 0              | No requirement                     |
+----------------+------------------------------------+
| 1              | :math:`u\bar{u}`                   |
+----------------+------------------------------------+
| 2              | :math:`d\bar{d}`                   |
+----------------+------------------------------------+
| 3              | :math:`s\bar{s}`                   |
+----------------+------------------------------------+
| 4              | :math:`\tau^+ \tau^- (\gamma)`     |
+----------------+------------------------------------+
| 5              | :math:`\mu^ +\mu^- (\gamma)`       |
+----------------+------------------------------------+
| 6              | :math:`e^+ e^- (\gamma)`           |
+----------------+------------------------------------+
| 7              | :math:`\gamma \gamma (\gamma)`     |
+----------------+------------------------------------+
| 8              | :math:`\Lambda\bar{\Lambda}`       |
+----------------+------------------------------------+
| 9              | Other low multiplicity             |
+----------------+------------------------------------+

+----------------+----------------------------------------+
| Selection Flag | Definition if the General Flag = 4     |
+================+========================================+
| 0              | No requirement                         |
+----------------+----------------------------------------+
| 1              | Bottomonium (e.g., :math:`Z_b` states) |
+----------------+----------------------------------------+
| 2-9            | Reserved                               |
+----------------+----------------------------------------+

+----------------+------------------------------------+
| Selection Flag | Definition if the General Flag = 5 |
+================+====================================+
| 0              | No requirement                     |
+----------------+------------------------------------+
| 1              | :math:`e^-`                        |
+----------------+------------------------------------+
| 2              | :math:`\mu^-`                      |
+----------------+------------------------------------+
| 3              | :math:`\pi^-`                      |
+----------------+------------------------------------+
| 4              | :math:`K^-`                        |
+----------------+------------------------------------+
| 5              | :math:`p`                          |
+----------------+------------------------------------+
| 6              | :math:`\pi^0`                      |
+----------------+------------------------------------+
| 7              | :math:`\gamma`                     |
+----------------+------------------------------------+
| 8              | Geantino                           |
+----------------+------------------------------------+
| 9              | Other particle type                |
+----------------+------------------------------------+

+----------------+------------------------------------+
| Selection Flag | Definition if the General Flag = 6 |
+================+====================================+
| 0              | No requirement                     |
+----------------+------------------------------------+
| 1-9            | Reserved                           |
+----------------+------------------------------------+


Decay Flag
++++++++++

The **Decay Flag** indicates general features of the scheme used to force the decay of the particle(s)
indicated by the Selection fFag. Tables below list the definitions. Events with additional photons,
which may occur via radiative corrections, are considered as a single decay mode. Intermediate 
esonances that are not explicitly defined in the decay file (for example, the use of resonant decay
models such as ``DALITZ`` and ``VUB``), are not counted either.

+------------+---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| Decay Flag | Definition if the General Flag is in 1-4                                                                                                                                                                |
+============+=========================================================================================================================================================================================================+
| 0          | Fully inclusive                                                                                                                                                                                         |
+------------+---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| 1          | The selected particles are forced to decay to a unique exclusive final state, with a unique intermediate state (if any).                                                                                |
+------------+---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| 2          | The selected particles are forced to decay to a unique intermediate state, with multiple or inclusive final states.                                                                                     |
+------------+---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| 3          | The selected particles are forced to decay to one of several exclusive final state, with identical topology.                                                                                            |
+------------+---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| 4          | The selected particles are forced to decay to one of several exclusive final state, with different topology.                                                                                            |
+------------+---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| 5-8        | Same as 1-4, but the final state contains at least one neutrino.                                                                                                                                        |
+------------+---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| 9          | The selected particles are forced to decay to a decay chain containing some inclusive decays (semi-inclusive mode), resulting in a number of different final states which are not explicitly specified. |
+------------+---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+

+------------+-----------------------------------------------------------------+
| Decay Flag | Definition if the General Flag = 5 and Selection Flag is in 1-7 |
+============+=================================================================+
| 0          | Particle                                                        |
+------------+-----------------------------------------------------------------+
| 1          | Anti-particle                                                   |
+------------+-----------------------------------------------------------------+
| 2          | Particle and Anti-particle                                      |
+------------+-----------------------------------------------------------------+

+------------+-----------------------------------------------------------+
| Decay Flag | Definition if the General Flag = 5 and Selection Flag = 9 |
+============+===========================================================+
| 1          | :math:`J/\psi \to \mu^+ \mu^-`                            |
+------------+-----------------------------------------------------------+
| 2          | :math:`J/\psi \to e^+ e^-`                                |
+------------+-----------------------------------------------------------+

+------------+---------------------------------------------------------------------------------+
| Decay Flag | Definition if the General Flag = 6 and the Charm Flag is 0 or 1 (i.e. beam gas) |
+============+=================================================================================+
| 0          | Some gas distribution functions                                                 |
+------------+---------------------------------------------------------------------------------+
| 1-9        | Reserved                                                                        |
+------------+---------------------------------------------------------------------------------+

+------------+----------------------------------------------------------------------------------------+
| Decay Flag | Definition if the General Flag = 6 and the Charm Flag = 5 (i.e. synchrotron radiation) |
+============+========================================================================================+
| 0-9        | Reserved                                                                               |
+------------+----------------------------------------------------------------------------------------+


Charm Flag
++++++++++

The **Charm Flag** gives information about the presence or absence of charm hadrons in the forced
part of the decay chain of the selected particle(s), as defined by the Decay Flag and the Selection
Flag. This flag is useful to determine the presence of a tertiary vertex from the weak decay of a 
harm hadron. Given the signal particle is flagged elsewhere, the charm content of the signal particle
does not trigger the charm and lepton flag (such as :math:`D^∗` and :math:`J/\psi` signal particles).
Tables below give the lists of definitions for General Flags of 4 and below, and for above 4
respectively.

+------------+------------------------------------------+
| Charm Flag | Definition if the General Flag is in 1-4 |
+============+==========================================+
| 0          | 0 :math:`c\bar{c}` and 0 open-charm      |
+------------+------------------------------------------+
| 1          | 1 :math:`c\bar{c}` and 0 open-charm      |
+------------+------------------------------------------+
| 2          | >=2 :math:`c\bar{c}` and 0 open-charm    |
+------------+------------------------------------------+
| 3          | >=0 :math:`c\bar{c}` and 1 open-charm    |
+------------+------------------------------------------+
| 4          | >=1 :math:`c\bar{c}` and >=1 open-charm  |
+------------+------------------------------------------+
| 5          | >=2 :math:`c\bar{c}` and >=1 open-charm  |
+------------+------------------------------------------+
| 6          | >=0 :math:`c\bar{c}` and >=2 open-charm  |
+------------+------------------------------------------+
| 7-9        | Not used                                 |
+------------+------------------------------------------+

+------------+------------------------------------+
| Charm Flag | Definition if the General Flag = 5 |
+============+====================================+
| 0          | Fixed momentum                     |
+------------+------------------------------------+
| 1          | Flat theta                         |
+------------+------------------------------------+
| 2          | Momentum range                     |
+------------+------------------------------------+

+------------+------------------------------------+
| Charm Flag | Definition if the General Flag = 6 |
+============+====================================+
| 0          | Beam gas                           |
+------------+------------------------------------+
| 1-9        | To be defined                      |
+------------+------------------------------------+


Lepton Flag
+++++++++++

The **Lepton Flag** gives information about the presence or absence of electrons, muons, and taus in
the forced part of the decay chain of the selected particle(s), as defined by the decay and selection
flags. This flag specifically applies to General Flags of less than 5.

+-------------+-----------------------------+-----------+----------+
| Lepton Flag | muons                       | electrons | taus     |
+=============+=============================+===========+==========+
| 0           | 0                           | 0         | 0        |
+-------------+-----------------------------+-----------+----------+
| 1           | 1                           | 0         | 0        |
+-------------+-----------------------------+-----------+----------+
| 2           | 0                           | 1         | 0        |
+-------------+-----------------------------+-----------+----------+
| 3           | 0                           | 0         | 1        |
+-------------+-----------------------------+-----------+----------+
| 4           | 2                           | 0         | 0        |
+-------------+-----------------------------+-----------+----------+
| 5           | 0                           | 2         | 0        |
+-------------+-----------------------------+-----------+----------+
| 6           | 0                           | 0         | 2        |
+-------------+-----------------------------+-----------+----------+
| 7           | Flavour inclusive 1 lepton  |           |          |
+-------------+-----------------------------+-----------+----------+
| 8           | Flavour inclusive 2 leptons |           |          |
+-------------+-----------------------------+-----------+----------+
| 9           | LFV modes                   |           |          |
+-------------+-----------------------------+-----------+----------+


Track Flag
++++++++++

The **Track Flag** is equal to the total number of “stable” charged particles in the forced part of 
he decay chain of the selected particles. The forced part of the decay chains is defined by the Decay
Flag, and the selected particles are those involved in the definition of the Selection Flag. Only the
following charged particles are counted: protons, charged pions, charged kaons, electrons, and muons
(tracks from :math:`K^0_S` and :math:`\Lambda` are counted as well). If the count is larger than 9, 
he Track Flag is set to 9. The value 0 is possible. If the Decay Flag is 2, 3, 4, 7, or 8 the
assignment of the Track Flag may become ambiguous. In that case, tracks should be counted using the
dominant or more representative forced decay chain. For machine background event types (General
Flag = 6) the Track Flag is used to indicate for which beam the events are produced, as defined in the
table below.

+------------+------------------------------------+
| Track Flag | Definition if the General Flag = 6 |
+============+====================================+
| 0-9        | Specific beam settings             |
+------------+------------------------------------+

For particle gun event types (General Flag = 5), the last flags **tnkxu** are used differently and
have special meanings to indicate the parameters used in the generation. See the table below.

+------------+------------+----------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| Charm flag | tnkxu flag | Definition if the General Flag = 5                                                                                                                                         |
+============+============+============================================================================================================================================================================+
| 0          |  tnkxu     | Value of the momentum in MeV                                                                                                                                               |
+------------+------------+----------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| 1          |  tnk       | Value of :math:`\theta^{\text{min}}_{\text{lab}}` in degrees                                                                                                               |
+------------+------------+----------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| 1          |  kxu       | Value of :math:`\theta^{\text{max}}_{\text{lab}}` if :math:`\theta^{\text{min}}_{\text{lab}}` is below 100 degrees, otherwise this means :math:`\Delta\theta_{\text{lab}}` |
+------------+------------+----------------------------------------------------------------------------------------------------------------------------------------------------------------------------+


Neutral Flag
++++++++++++

The **Neutral Flag** indicates the presence of some particular neutral particles in the forced part
of the decay chains of the selected particles. The forced part of the decay chains is defined by the
Decay Flag, and the selected particles are those involved in the definition of the Selection Flag.
The following particles are considered: :math:`\gamma` (**excluding** photons from
:math:`\pi^0 \to \gamma\gamma` and :math:`\eta \to \gamma\gamma` decays and radiative photons such as
photons produced by ``PHOTOS`` for example in :math:`J/\psi \to l^+ l^- \gamma`), :math:`\pi^0` and
:math:`\eta` decaying to two photons. The :math:`\pi^0` and :math:`\eta` particles are flagged even
if they are not explicitly decayed in the decay file but are left to decay according to
``decfiles/dec/DECAY_BELLE2.dec``. The Neutral Flag is defined according to the table below. Note
that as the flag cannot exceed 9, bits 1 and 2 should not be set if bit 3 is already set.

+--------------+--------------------------------------------------------------------------------------------------------------------+
| Neutral Flag | Definition                                                                                                         |
+==============+====================================================================================================================+
| +1 (bit 0)   | At least one gamma not from :math:`\pi^0 \to \gamma \gamma`, :math:`\eta \to \gamma \gamma` nor a radiative photon |
+--------------+--------------------------------------------------------------------------------------------------------------------+
| +2 (bit 1)   | One :math:`\pi^0 \to \gamma \gamma`                                                                                |
+--------------+--------------------------------------------------------------------------------------------------------------------+
| +4 (bit 2)   | At least two :math:`\pi^0 \to \gamma \gamma`                                                                       |
+--------------+--------------------------------------------------------------------------------------------------------------------+
| +8 (bit 3)   | At least one :math:`\eta \to \gamma \gamma`                                                                        |
+--------------+--------------------------------------------------------------------------------------------------------------------+


Kaon Flag
+++++++++

The **Kaon Flag** indicates the presence of some particular neutral kaons, lambda, or neutrons in the
forced part of the decay chains of the selected particles. The forced part of the decay chains is
defined by the Decay Flag, and the selected particles are those involved in the definition of the
Selection Flag. The following neutral particles are considered: :math:`K^0_S` or :math:`\Lambda`
decaying to two charged hadrons, neutrons, and :math:`K^0_L`. The Kaon Flag is defined according to
the table below. Note that as the flag cannot exceed 9, bits 1 and 2 should not be set if bit 3 is
already set.

+------------+-------------------------------------------------------------------------------------+
| Kaon Flag  | Definition                                                                          |
+============+=====================================================================================+
| +1 (bit 0) | At least one :math:`K^0_S \to \pi^+ \pi^-`                                          |
+------------+-------------------------------------------------------------------------------------+
| +2 (bit 1) | At least one :math:`K^0_L`                                                          |
+------------+-------------------------------------------------------------------------------------+
| +4 (bit 2) | At least one :math:`\Lambda \to p \pi^-` or :math:`\bar{\Lambda} \to \bar{p} \pi^+` |
+------------+-------------------------------------------------------------------------------------+
| +8 (bit 3) | At least one neutron                                                                |
+------------+-------------------------------------------------------------------------------------+


Extra Flag and User Flag
++++++++++++++++++++++++

The **Extra Flag** and **User Flag** have no pre-defined meaning. The Extra Flag should be used to
distinguish different decay chains that happen to have the same General, Selection, Charm, Track, and
Neutral flags. In case more than 10 different decay chains need to be distinguished, then the User
Flag should be used as well to make sure that each decay chain has a unique event type.

A special case is inclusive production, which is primarily for backgrounds. The User Flag is used to
distinguish between the different inclusive background processes. See the second table below for a
list of inclusive background processes.

With the above exceptions, the User Flag should be used to distinguish different cases based on the
same decay chain, with no well-defined meaning. It could be used to distinguish between different
decay modes, different physics settings, etc. For example, the User Flag is used to distinguish
events produced with and without radiative corrections or to distinguish between polarised and
unpolarised generators.

For the production of samples of excited :math:`b` or :math:`c` states, the Extra Flag gives the
information of the resonance to be produced. See the first table below for examples of excited state
production, and note that: Extra and User Flag are both user-defined, e.g. sequential count 00, 01,
etc. if the General Flag = 4 and the Selection Flag = 1.

+-----------------------------------------+-------------------------+----------------------------------------------------+----------------------------------------------------+
| Extra Flag for excited state production | If the General Flag = 1 | If the General Flag = 2 and the Selection Flag = 6 | If the General Flag = 2 and the Selection Flag = 8 |
+=========================================+=========================+====================================================+====================================================+
| 0                                       | :math:`\Upsilon(4S)`    | :math:`D^{*+}`                                     | :math:`\psi(2S)`                                   |
+-----------------------------------------+-------------------------+----------------------------------------------------+----------------------------------------------------+
| 1                                       | :math:`\Upsilon(1S)`    | :math:`D^{*0}`                                     | :math:`X(3872)`                                    |
+-----------------------------------------+-------------------------+----------------------------------------------------+----------------------------------------------------+
| 2                                       | :math:`\Upsilon(2S)`    | :math:`D_s^{*+}`                                   | :math:`h_c`                                        |
+-----------------------------------------+-------------------------+----------------------------------------------------+----------------------------------------------------+
| 3                                       | :math:`\Upsilon(3S)`    | :math:`D_{s2}^{*+}(2536)`                          | :math:`\chi_{c0}`                                  |
+-----------------------------------------+-------------------------+----------------------------------------------------+----------------------------------------------------+
| 4                                       | :math:`\Upsilon(5S)`    | :math:`D_2^{*0}(2460)`                             | :math:`\chi_{c1}`                                  |
+-----------------------------------------+-------------------------+----------------------------------------------------+----------------------------------------------------+
| 5                                       | :math:`\Upsilon(6S)`    | :math:`D_2^{*+}(2460)`                             | :math:`\chi_{c2}`                                  |
+-----------------------------------------+-------------------------+----------------------------------------------------+----------------------------------------------------+
| 6                                       |                         | :math:`D_{s1}^+(2460)`                             | :math:`\eta_c(1S)`                                 |
+-----------------------------------------+-------------------------+----------------------------------------------------+----------------------------------------------------+

+-------------------------------------+------------------+
| User Flag for inclusive backgrounds | Definition       |
+=====================================+==================+
| 0                                   | fully inclusive  |
+-------------------------------------+------------------+
| 1                                   | rare hadronic    |
+-------------------------------------+------------------+
| 2                                   | :math:`u l \nu`  |
+-------------------------------------+------------------+
| 3                                   | :math:`d \gamma` |
+-------------------------------------+------------------+
| 4                                   | :math:`s \gamma` |
+-------------------------------------+------------------+
| 5                                   | :math:`c l \nu`  |
+-------------------------------------+------------------+


Short Names
-----------

The short name associated with each event type provides an easy way to identify the meaning of the
event type. In most cases, it represents the decay chain. The short name is composed of three parts:

- **Particle:** Name of the particle forced to decay. Charge conjugation is always implied.
- **Decay:** Formed with the particle names constituting the forced part of the decay chain of the
  event type. Each level of the decay chain is separated by a comma and is indicated when it is not a
  trivial decay mode (for example, :math:`\pi^0 \to \gamma \gamma` is not mentioned in this part of
  the short name), nor the usual decay mode (for example, :math:`D_s^+ \to K^- \pi^+ \pi^-`). The
  full name of the particle is indicated and should respect capital or lowercase letters. However,
  to obtain shorter names, particle names can be shortened when they describe the second or lower
  levels of the decay chain, after the comma (for example, ``mu`` as ``m`` or ``gamma`` as ``g``).
  Charges of particles are mentioned only if necessary to distinguish between two possible different
  final states like ``Bd_pi+pi-`` and ``Bd_pi0pi0``.
- **Other:** A separate list that gives additional information about the decay chain. It should
  start with the main decay model used (if this is important, otherwise this can be omitted), followed
  by additional options used. The word ``cocktail`` is used to indicate a combination of different
  decay modes or different particles. The word ``incl`` is used when the particle is not forced to
  decay to any specific decay mode and will decay to any channel specified in the default decay file
  (``decfiles/dec/DECAY_BELLE2.dec``), and the word ``X`` is used for semi-inclusive productions where
  accompanying particles are not specified.

The particle names to be used in nicknames (the second form is the short one):

- **B mesons:** ``Bd`` (:math:`B^0`), ``Bu`` (:math:`B^+`), ``Bs`` (:math:`B_s^0`).
- **D mesons:** ``D0`` (:math:`D^0`), ``D+`` (:math:`D^+`), ``Ds`` (:math:`D_s^+`), ``Dst+``
  (:math:`D^{*+}`), ``Dst0`` (:math:`D^{*0}`), ``Dsst+`` (:math:`D_s^{*+}`), ``Jpsi`` (:math:`J/\psi`)
  and ``etac`` (:math:`\eta_c`).
- **Strange hadrons:** ``K`` (:math:`K^+`), ``KS`` (:math:`K^0_S`), ``KL`` (:math:`K^0_S`) and
  ``Lambda`` (:math:`\Lambda^0`).
- **Unflavoured hadrons:** ``pi`` (:math:`\pi^+` / :math:`\pi^0`), ``rho``
  (:math:`\rho^+` / :math:`\rho^0`), ``eta`` (:math:`\eta`), ``etap`` (:math:`\eta^{\prime}`),
  ``a1`` (:math:`a1`), ``phi`` (:math:`\phi`), ``p`` (:math:`p`), ``n`` (:math:`n`).
- **Other particles:** ``gamma`` / ``g`` (:math:`\gamma`), ``e`` (:math:`e^-`), ``mu`` / ``m``
  (:math:`\mu^-`), ``tau`` (:math:`\tau^-`).

For other event types, the rules are:

- **Particle gun production:** The short name is of the form ``gun``, followed by the particle name and
  the particle gun conditions, separated by the equality sign. The particle gun conditions are used to
  describe the momentum or momentum range and the polar angle range.
- **Machine background generation:** The short name is of the form ``bkg``, followed by the type of
  background. Then the conditions are indicated after the equality sign.