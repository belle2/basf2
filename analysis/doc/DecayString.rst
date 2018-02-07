.. _DecayString:

DecayString
===========
The DecayString is an elegant way of telling the analysis modules about the structure and the particles of a decay tree. The syntax is borrowed mainly from the LHCb software. Here comes the first example of a DecayString (you probably can already guess the meaning):

:code:`'B+ -> [anti-D0 -> K- pi+] pi+'`

The structure of the decay tree is internally represented by the DecayDescriptor class, but as analysis user you do not have to care about this.

Particles and Decays
====================
The atomic components of the DecayString and also the simplest valid DecayStrings are particles:

:code:`'B+', 'anti-D0', 'K-', 'pi+'`

The framework automatically translates the particle names to the corresponding PDG codes (e.g. 'B+' = 521).

A particle decay is represented by an arrow:

:code:`'B+ -> anti-D0 pi+'`

:code:`'anti-D0 -> K- pi+'`

On the right side, particles can be replaced by decaying particles which have to be put in brackets :code:`'[...]'` to build more complex decay trees.

:code:`'B+ -> [anti-D0 -> K- pi+] pi+'`

Selection
=========
Particles can be selected with a preceeding upper carret symbol, :code:`'^'`. 
This line selectes B+, anti-D0 and pi+ from the anti-D0 decay:

:code:`'^B+ -> [^anti-D0 -> K- ^pi+] pi^+'`

Inclusive Decays
================
The decay descriptor for inclusive decays can be used only in MCDecayFinder module and nowhere else. Inclusive decays can be specified by with :code:`'...'` for unreconstructed particles:

:code:`'B+ -> [anti-D0 -> K- pi+] ...'`

Arrows
======
For truth matching purposes different types of arrows are defined:

* :code:`'->'` exact match of the decay including intermediate resonances
* :code:`'-->'` intermediate resonances are ignored
* :code:`'=>'` radiated photons are ignored
* :code:`'==>'` intermediate resonances and radiated photons are ignored
 

