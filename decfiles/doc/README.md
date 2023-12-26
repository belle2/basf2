<!-- #region -->
# MONTE CARLO EVENT TYPE DEFINITIONS

## A. Introduction
This section defines the scheme for Monte Carlo event types. The principle is that event types should describe the way a sample was produced, and classify the specific decay channels according to their global features. A multiple digit number represents the event type, carrying this useful information. An event type number is more convenient for distributed MC production.

The event type is an 10-digit number “gsdcltnkeu” where each individual digit is assigned a meaning according to the following principles:

- **g:** General flag, indicating the general category of the event type and the event production mechanism.
- **s:** Selection flag, indicating the criteria used to define this event type, based on the presence of certain particles.
- **d:** Decay flag, indicating the general features of the decays considered for this event type.
- **c:** Charm content flag for the selected decays in this event type.
- **l:** Lepton content flag for the selected decays in this event type.
- **t:** Tracks flag indicating the number of charged tracks in the selected decays.
- **n:** Neutrals flag indicating the number of neutral particles in the selected decays.
- **k:** Neutral kaon flag indicating the number of neutral kaons in the selected decays.
- **e:** Extra flag for disambiguation in some cases.
- **u:** User flag, used for generating decays with different generators.

This scheme is slightly modified to describe particle gun and background MC production. These exceptions are described later in this note. In addition to a number, the event types are given short names for human readability.

Event Type definitions should be listed in the header of each EvtGen decay file.

## B. Flag descriptions

### 1. General Flag
The definitions of the general flag are given in the table below. The value zero is not used here. At the current time only 6 general flags are defined. The exotic category will typically cover new physics.

| General Flag | Definition                 |
|--------------|----------------------------|
| 0            | Undefined                  |
| 1            | Events containing a b quark |
| 2            | Events containing a c quark |
| 3            | Light flavour pairs and lepton pairs |
| 4            | Exotic                     |
| 5            | Particle gun               |
| 6            | Machine background         |
| 7-9          | Undefined                  |

### 2. Selection Flag
The selection flag indicates which particles are required to be present in the event, and possibly forced to decay to specific modes. The definitions of the selection flag are given in the table below, and depend on the value of the general flag. The exotic flags are yet to be defined, but may include modes such as dark sectors.


| Selection Flag | Definition if the General Flag = 1, or 2|
|----------------|------------|
| 0              | Inclusive b(c) hadron production |
| 1              | B0(D-)      |
| 2              | B+(D0)      |
| 3              | B0s (Ds-)   |
| 4              | (J/ψ)       |
| 5              | Λb (Λc)     |
| 6              | Other type of b(c) resonance, the extra flag specifies the type of resonance |
| 7              | Other type of bb(cc) resonance, the extra flag specifies the type of resonance |
| 8              | Double (cc) resonance, the extra flag specifies the type of resonance |

| Flag | Definition if the General Flag = 3           |
|------|----------------------------------------------|
| 0    | No requirement                               |
| 1    | uubar                                        |
| 2    | ddbar                                        |
| 3    | ssbar                                        |
| 4    | τ+ τ- (γ)                                    |
| 5    | μ+ μ- (γ)                                    |
| 6    | e+ e- (γ)                                    |
| 7    | γγ                                           |
| 8    | Λ Λbar                                       |
| 9    | Other low multiplicity                       |

| Flag | Definition if the General Flag = 4      |
|------|-----------------------------------------|
| 0    | No requirement                          |
| 1    | Bottomonium (e.g., Zb states)           |
| 2-9  | Reserved                                |

| Flag | Definition if the General Flag = 5|
|------|------------------------------|
| 0    | No requirement               |
| 1    | e-                           |
| 2    | μ-                           |
| 3    | π-                           |
| 4    | K-                           |
| 5    | p                            |
| 6    | π0                           |
| 7    | γ                            |
| 8    | Geantino                      |
| 9    | Other particle type           |

| Flag | Definition if the General Flag = 6|
|------|------------------|
| 0    | No requirement   |
| 1-9  | Reserved         |


### 3. Decay Flag
The decay flag indicates general features of the scheme used to force the decay of the particle(s) indicated by the selection flag. Tables below list the definitions. Events with additional photons, which may occur via radiative corrections, are considered as a single decay mode. Intermediate resonances that are not explicitly defined in the decay file (for example, the use of resonant decay models such as DALITZ and VUB), are not counted either.

| Flag | Definition if the General Flag is in 1-4          |
|------|------------------------------------------------------------------------------|
| 0    | Fully inclusive                                                              |
| 1    | The selected particles are forced to decay to a unique exclusive final state, with a unique intermediate state (if any). |
| 2    | The selected particles are forced to decay to a unique intermediate state, with multiple or inclusive final states. |
| 3    | The selected particles are forced to decay to one of several exclusive final state, with identical topology. |
| 4    | The selected particles are forced to decay to one of several exclusive final state, with different topology. |
| 5-8  | Same as 1-4, but the final state contains at least one neutrino. |
| 9    | The selected particles are forced to decay to a decay chain containing some inclusive decays (semi-inclusive mode), resulting in a number of different final states which are not explicitly specified. |

| Flag | Definition if the General Flag = 5 and Selection Flag is in 1-7|
|------|-------------------------------------------------|
| 0    | Particle                                        |
| 1    | Anti-particle                                   |
| 2    | Particle and Anti-particle                      |

| Flag | Definition if the General Flag = 5 and Selection Flag = 9   |
|------|-------------------------------------------------|
| 1    | J/ψ → μ μ                                       |
| 2    | J/ψ → e e                                       |

| Flag | Definition if the General Flag = 6 and the Charm Flag is 0 or 1 (i.e. beam gas)|
|------|-------------------------------------------------|
| 0    | Some gas distribution functions                 |
| 1-9  | Reserved                                        |

| Flag | Definition if the General Flag = 6 and the Charm Flag is 5 (i.e. synchrotron radiation)|
|------|-------------------------------------------------|
| 0-9  | Reserved                                        |


### 4. Charm Flag
This flag gives information about the presence or absence of charm hadrons in the forced part of the decay chain of the selected particle(s), as defined by the decay and selection flags. This flag is useful to determine the presence of a tertiary vertex from the weak decay of a charm hadron. Given the signal particle is flagged elsewhere, the charm content of the signal particle does not trigger the charm and lepton flag (such as D∗ and J/ψ signal particles). Tables below give the lists of definitions for general flags of 4 and below, and for above 4 respectively.

| Charm Flags if the general flag is less than 5 | ccbar | open-charm |
|------------------------------------------------|-------|------------|
| 0                                              |  0    |   0        |
| 1                                              |  1    |   0        |
| 2                                              |  >=2  |   0        |
| 3                                              |  >=0  |   1        |
| 4                                              |  >=1  |   >=1      |
| 5                                              |  >=2  |   >=1      |
| 6                                              |  >=0  |   >=2      |
| 7-9                                            |  not  |   defined  |

| Flag | Definition if the general Flag = 5 |
|------|------------------------|
| 0    | Fixed momentum         |
| 1    | Flat theta             |
| 2    | Momentum range         |

| Flag | Definition if the general Flag = 6 |
|------|------------------------|
| 0    | beam gas               |
| 1-9  | to be defined          |


### 5. Lepton Flag
This flag gives information about the presence or absence of electrons, muons, and taus in the forced part of the decay chain of the selected particle(s), as defined by the decay and selection flags. This flag specifically applies to general flags of less than 5.


| Flag | muon | electron | tau |
|------|------|----------|-----|
| 0    | 0    | 0        | 0   |
| 1    | 1    | 0        | 0   |
| 2    | 0    | 1        | 0   |
| 3    | 0    | 0        | 1   |
| 4    | 2    | 0        | 0   |
| 5    | 0    | 2        | 0   |
| 6    | 0    | 0        | 2   |
| 7    | Flavour | inclusive | 1 lepton |
| 8    | Flavour | inclusive | 2 leptons|
| 9    | LFV | modes ||


### 6. Track Flag
The track flag is equal to the total number of “stable” charged particles in the forced part of the decay chain of the selected particles. The forced part of the decay chains is defined by the decay flag, and the selected particles are those involved in the definition of the selection flag. Only the following charged particles are counted: protons, charged pions, charged kaons, electrons, and muons (tracks from K0s and Λ are counted). If the count is larger than 9, the flag is set to 9. The value 0 is possible. If the decay flag is 2, 3, 4, 7, or 8 the assignment of the track flag may become ambiguous. In that case, tracks should be counted using the dominant or more representative forced decay chain. For machine background event types (g=6) the track flag is used to indicate for which beam the events are produced, as defined in the table below.

| Flag | Definition if the general  flag = 6 |
|------|------------------------|
| 0-9  | specific beam settings |

For particle gun event types (g=5), the last flags tnkxu are used differently and have special meanings to indicate the parameters used in the generation. See the table below.

| Charm flag c | tnxu flag | Definition if the general flag = 5 (particle guns) |
|--------------|-----------|----------------------------------------------------|
| 0            |  tnkxu    | value of the momentum of the particle (in MeV)     |
| 1            |  tnk      | minimum value of θ_lab                              |
|              |  kxu      | maximum value of θ_lab if minimum is below 100 degrees, otherwise this means delta theta |


### 7. Neutral Flag
The neutral flag indicates the presence of some particular ECL neutral particles in the forced part of the decay chains of the selected particles. The forced part of the decay chains is defined by the decay flag, and the selected particles are those involved in the definition of the selection flag. The following ECL particles are considered: γ (excluding photons from π0 → γγ and η → γγ decays and radiative photons such as photons produced by PHOTOS for example in J/ψ -> ℓ+ℓ−γ), π0 and η decaying to two photons. The π0 and η particles are flagged even if they are not explicitly decayed in the decay file but are left to decay according to DECAY.DEC. The neutral flag is defined according to the table below. (Note that as the flag cannot exceed 9, bits 1 and 2 should not be set if bit 3 is already set)

| Bit (flag) | Definition                                                         |
|------------|--------------------------------------------------------------------|
| bit 0 (+1) | At least one gamma not from π0 → γγ, η → γγ nor a radiative photon |
| bit 1 (+2) | One π0 → γγ                                                        |
| bit 2 (+4) | At least two π0 → γγ                                               |
| bit 3 (+8) | At least one η → γγ                                                |


### 8. Kaon Flag
The kaon flag indicates the presence of some particular neutral kaons, lambda, or neutrons in the forced part of the decay chains of the selected particles. The forced part of the decay chains is defined by the decay flag, and the selected particles are those involved in the definition of the selection flag. The following neutral particles are considered: KS0 or Λ decaying to two charged hadrons, neutrons, and KL0. The kaon flag is defined according to the table below. (Note that as the flag cannot exceed 9, bits 1 and 2 should not be set if bit 3 is already set)

| Bit (flag) | Definition                              |
|------------|-----------------------------------------|
| bit 0 (+1) | At least one KS → π+ π-                 |
| bit 1 (+2) | At least one KL                         |
| bit 2 (+4) | At least one Λ → p π, or Λbar → pbar π+ |
| bit 3 (+8) | At least one neutron                    |


### 9. Extra Flag
The extra and user flags have no pre-defined meaning. The extra flag should be used to distinguish different decay chains that happen to have the same general, selection, charm, track, and neutral flags. In case more than 10 different decay chains need to be distinguished, then the user flag should be used as well to make sure that each decay chain has a unique event type.

A special case is inclusive production, which is primarily for backgrounds. The user flag is used to distinguish between the different inclusive background processes. See the 2nd table below for a list of inclusive background processes. 

With the above exceptions, the user flag should be used to distinguish different cases based on the same decay chain, with no well-defined meaning. It could be used to distinguish between different decay modes, different physics settings, etc.. For example, the user flag is used to distinguish events produced with and without radiative corrections or to distinguish between polarised and unpolarised generators. 

For the production of samples of excited b or c states, the extra flag gives the information of the resonance to be produced. See the first table below for examples of excited state production, and note that: Extra and user flag are both user-defined, e.g. sequential count 00, 01, ... if the general flag = 4 and the selection flag = 1

| Extra Flag for excited state production | if the general flag = 1 | if the general flag = 2 and the selection flag = 6 | if the general flag = 2 and the selection flag =8 |
|---|-------|-------------|---------|
| 0 | Υ(4S) | D*+         | ψ(2S)   |
| 1 | Υ(1S) | D* 0        | X(3872) |
| 2 | Υ(2S) | Ds*+        | hc      |
| 3 | Υ(3S) | Ds2*+(2536) | χc0     |
| 4 | Υ(5S) | D2* 0(2460) | χc1     |
| 5 | Υ(6S) | D2*+(2460)  | χc2     |
| 6 |   -   | Ds1+(2460)  | ηc(1S)  |

| User Flag for inclusive backgrounds | Definition |
|---|-----------------|
| 0 | fully inclusive |
| 1 | rare hadronic   |
| 2 | u l ν           |
| 3 | d γ             |
| 4 | s γ             |
| 5 | c l ν           |


## C. Short names
The short name associated with each event type provides an easy way to identify the meaning of the event type. In most cases, it represents the decay chain. The decay file used by EVTGEN to generate the corresponding events is named after the short name, adding the suffix `.dec` to it. The short name is composed of three parts:

- **Particle:** Name of the particle forced to decay. Charge conjugation is always implied.
- **Decay:** Formed with the particle names constituting the forced part of the decay chain of the event type. Each level of the decay chain is separated by a comma and is indicated when it is not a trivial decay mode (for example, π0 -> γγ is not mentioned in this part of the short name), nor the usual decay mode (for example, Ds+ → K−π+π−). The full name of the particle is indicated and should respect capital or lowercase letters. However, to obtain shorter names, particle names can be shortened when they describe the second or lower levels of the decay chain, after the comma (for example, mu as m or gamma as g). Charges of particles are mentioned only if necessary to distinguish between two possible different final states like Bd_pi+pi- and Bd_pi0pi0.
- **Other:** A separate list that gives additional information about the decay chain. It should start with the main decay model used (if this is important, otherwise this can be omitted), followed by additional options used. The word cocktail is used to indicate a combination of different decay modes or different particles. The word incl is used when the particle is not forced to decay to any specific decay mode and will decay to any channel specified in the default EVTGEN decay file (DECAY.DEC), and the word X is used for semi-inclusive productions where accompanying particles are not specified.

The particle names to be used in nicknames (the second form is the short one):

- **B mesons:** Bd (B0d), Bu (B+u), Bs (B0s).
- **D mesons:** D0 (D0), D+ (D+), Ds (D+s), Dst+ (D*+), Dst0 (D*0), Dsst+ (D*+s), Jpsi (J/ψ) and , etac (ηc).
- **Strange hadrons:** K (K+), KS (K0S), KL (K0L) and Lambda (Λ0).
- **Unflavoured hadrons:** pi (π+/0), rho (ρ+/0), eta(η), etap(η'), a1(a1), phi(φ), p(p), n(n).
- **Other particles:** gamma/g (γ), e (e-), mu/m (μ-), tau (τ-).

For other event types, the rules are:

- **Particle gun production:** The short name is of the form gun, followed by the particle name and the particle gun conditions, separated by the equality sign. The particle gun conditions are used to describe the momentum or momentum range and the polar angle range.
- **Machine background generation:** The short name is of the form bkg, followed by the type of background. Then the conditions are indicated after the equality sign.

<!-- #endregion -->