.. _DecayString:

DecayString
===========

The DecayString is an elegant way of telling the analysis modules about the structure and the particles of a decay tree. The syntax is borrowed mainly from the LHCb software. Here comes the first example of a DecayString (you probably can already guess the meaning):

:code:`'B+ -> [anti-D0 -> K- pi+] pi+'`

The structure of the decay tree is internally represented by the DecayDescriptor class, but as analysis user you do not have to care about this.

Particles and Decays
--------------------
The atomic components of the DecayString and also the simplest valid DecayStrings are particles:

:code:`'B+', 'anti-D0', 'K-', 'pi+'`

The framework automatically translates the particle names to the corresponding PDG codes (e.g. 'B+' = 521).

A particle decay is represented by an arrow:

:code:`'B+ -> anti-D0 pi+'`

:code:`'anti-D0 -> K- pi+'`

On the right side, particles can be replaced by decaying particles which have to be put in brackets :code:`'[...]'` to build more complex decay trees.

:code:`'B+ -> [anti-D0 -> K- pi+] pi+'`

Selection
---------
Particles can be selected with a preceeding upper carret symbol, :code:`'^'`. 
This line selectes B+, anti-D0 and pi+ from the anti-D0 decay:

:code:`'^B+ -> [^anti-D0 -> K- ^pi+] pi^+'`

Marker of unspecified particle
------------------------------

Particle can be marked as unspecified particle with an at-sign, :code:`'@'`, in the decayString.
If the particle is marked as unspecified it will not checked for its identity when doing :ref:`MCMatching`. Any particle which decays into the correct daughters will be flagged as correct. For example the DecayString @Xsd -> K+ pi- would match all particles which decay into a Kaon and a pion, for example K*, B0, D0, ...
Still the daughters need to be stated correctly so this can be used for "sum of exclusive" decays.
Here is an example of use:

.. code-block:: python
 
        from modularAnalysis import reconstructDecay, copyLists
	reconstructDecay('@Xsd:0 -> K_S0:all',                   '', path=mypath)
	reconstructDecay('@Xsd:1 -> K+:loose pi-:loose',         '', path=mypath)
	reconstructDecay('@Xsd:2 -> K+:loose pi-:loose pi0:all', '', path=mypath)

	copyLists('Xsd:comb', ['Xsd:0', 'Xsd:1', 'Xsd:2'], path=mypath)
	# copyLists('@Xsd:comb', ['Xsd:0', 'Xsd:1', 'Xsd:2'], path=mypath) # this can't work

	reconstructDecay('B0:Xsdee -> Xsd:comb e+:loose e-:loose', '', path=mypath)

Grammar for custom MCMatching
-----------------------------
One can use the keywords for decay string to configure how :code:`'isCustomSignal'` is behaving. If these keywords are used, the missing particles are ignored or taken into account. For example, if :code:`'...'` is used, missing final state particles are ignored, hence :code:`'isCustomSignal'` works as :code:`'isSignalAcceptMissingMassive'`. 
Keywords must be placed at the end of the decay string. It is not allowed to put keywords in front of particles.

* :code:`'...'` Missing massive final state particles are ignored
* :code:`'?nu'` Missing neutrinos are ignored
* :code:`'!nu'` Missing neutrinos are taken into account (default)
* :code:`'?gamma'` Missing gammas are ignored 
* :code:`'!gamma'` Missing gammas are taken into account (default)

Here is an exapmle of use:

.. code-block:: python
 
        from modularAnalysis import reconstructDecay
	# Keywords must be placed behind all particles
	reconstructDecay('K_S0:missNu     -> pi+:loose e-:loose ?nu',      '', path=mypath)
	# isCustomSignal of K_S0:missNu accepts missing neutrino
	reconstructDecay('Xsu:missMassive -> K+:loose pi0:all ... ?gamma', '', path=mypath)
	# isCustomSignal of Xsu:missMassive accepts missing massive FSP and gamma (such as pi0 -> gamma gamma)
	reconstructDecay('B+:inclusive    -> mu-:loose ... ?nu ?gamma',    '', path=mypath)
	# isCustomSignal of B+:inclusive accepts missing massive FSP, neutrino, and gamma. 

Arrows
------
For truth matching purposes different types of arrows are defined:

* :code:`'->'` intermediate resonances and radiated photons are ignored
* :code:`'-->'`, :code:`'=>'` and :code:`'==>'` work same as :code:`'->'`. These three will be deprecated in release-05, please consider to use :code:`'->'`.
* :code:`'=direct=>'` intermediate resonances are considered but radiated photons are ignored
* :code:`'=norad=>'` radiated photons are considered but intermediate resonances are ignored
* :code:`'=exact=>'` exact match of the decay including intermediate resonances and radiated photons

Here is an exapmle of use:

.. code-block:: python
 
        from modularAnalysis import reconstructDecay
	reconstructDecay('B+:default ->        K+:loose e+:loose e-:loose', '', path=mypath)
	reconstructDecay('B+:direct  =direct=> K+:loose e+:loose e-:loose', '', path=mypath)
	reconstructDecay('B+:norad   =norad=>  K+:loose e+:loose e-:loose', '', path=mypath)
	reconstructDecay('B+:exact   =exact=>  K+:loose e+:loose e-:loose', '', path=mypath)

	# If one reconstructs B+ -> K+ e+ e- with above codes, one gets following results
	# isCustomSignal(B+:default) == 1
	# isCustomSignal(B+:direct)  == 1
	# isCustomSignal(B+:norad)   == 1
	# isCustomSignal(B+:exact)   == 1

	# If one reconstructs B+ -> K+ e+ e- and FSR photon with above codes, one gets following results
	# isCustomSignal(B+:default) == 1
	# isCustomSignal(B+:direct)  == 1
	# isCustomSignal(B+:norad)   == 0 Because the radiated photon is missed.
	# isCustomSignal(B+:exact)   == 0 Because the radiated photon is missed.

	# If one reconstructs B+ -> K+ Jpsi( -> e+ e- and FSR photon) with above codes, one gets following results
	# isCustomSignal(B+:default) == 1
	# isCustomSignal(B+:direct)  == 0 Because the intermediate resonance (Jpsi) is missed.
	# isCustomSignal(B+:norad)   == 0 Because the radiated photon is missed.
	# isCustomSignal(B+:exact)   == 0 Because the intermediate resonance (Jpsi) and the radiated photon are missed.

