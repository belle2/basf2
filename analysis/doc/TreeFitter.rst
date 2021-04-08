.. _treeFitter:

Tree Fitter
===========


The TreeFitter is a global fitting tool to simultaneously fit an entire decay chain. It tries to find the best decay vertex positions and momenta for the given decay hypothesis. Final state particles will be constrained to their masses, meaning no energy is extracted from the fit but pdg-mass and momentum measurement are used to determine it (same as track fitting procedure). The fit extracts the 4-momenta (->including energy) and decay vertex positions of intermediate particles. If an intermediate particle has a flight length less than :math:`1~\mu m` - for example a strongly decaying hadronic resonance - its decay and production vertex will be merged. The production vertex is the decay vertex of the mother particle. Note that the other fitters do not do this.

The fit tries to find the solutions to a system of equations or at least the solution closest to the measurement given the measurement uncertainty and noise. The distance between (linearised) hypothesis and measurement is calculated using :math:`\chi^2 = \sum \frac{h-m}{\sigma}` the final goodness of the fit then can be evaluated using the p-value defined as the incomplete (lower = 1 - upper) gamma function of the :math:`\chi^2` and degrees of freedom :math:`r` divided by two:
:math:`1 -\int^{ \frac{\chi^2}{2} }_{0} t^{r/2-1} e^{-t} dt~/~\Gamma(\frac{r}{2}).` 

This quantity denotes the probability the observed :math:`\chi^2` exceeds the expectations for a correct model by chance (see ROOT documentation ``TMath::Prob(Double_t chi2,Int_t ndf)``).

The number of degrees of freedom is the number of equations (constraints) minus the number of parameters extracted (vertices, momenta, ...) and has to be larger than zero, otherwise the fit will display a warning and do nothing.
The constraints can be counted as following:

+--------------+------------------------------+------------------------------------------+
| Particle     | parameters                   | constraints                              |
+==============+==============================+==========================================+
| track        | px, py, pz                   | 5 (helix)                                |
+--------------+------------------------------+------------------------------------------+
| photon       | px, py, pz                   | 3 (see paper)                            |
+--------------+------------------------------+------------------------------------------+
| Intermediate | px, py, pz, E, x, y, z, tau  | 4 (kinematic) + 3 (geometric) + 1 (mass) |
+--------------+------------------------------+--------------+---------------------------+

:math:`\tau` is the decay length. For intermediates it depends, generally there are 4 equations from the kinematic constraint, if they have a flight length longer than :math:`1~\mu m` AND a well defined production vertex (a mother particle or an origin constraint) we extract the flight length and can use a geometric constraint, which the fit will automatically do unless configured otherwise. If the particle shares the decay vertex with the mother we will only extract this vertex parameters once in the fit. A mass constraint will only be used if the user configures the fit to do so.
So for example for :math:`J/\psi \rightarrow l^+ l^-` the calculation is the following: 
:math:`r = 5 (track) + 5 (track) + 4 (kinematic) \\- 3 (track) - 3 (track) \\- 7 (J/\psi:~px, py, pz, E, x, y, z) = 1.` 

While for :math:`B \rightarrow D(K \pi \pi^0)\pi`:  
:math:`r = 3\dot 5 (track) + 2 \cdot 3 (gamma) \\+ 3 \cdot 4 (kinematic) + 3 (D:geometric) \\- 6 (B:~x,y,z,px,py,pz) - 7 (D:~x,y,z,tau,px,py,pz) \\- 3 (\pi^0:~px,py,pz) - 3 \cdot 3 (track) \\- 2 \cdot 3 (gamma) = 5.`

Citation
########

Krohn, J.-F. *et al*. *Nucl.Instrum.Meth.A* **976** (2020) 164269
https://doi.org/10.1016/j.nima.2020.164269

Usage
#####

The user reconstructs the desired decay chain and then passes the head of the tree to the vertex fitter. So when you are reconstructing :math:`B->D(K \pi)\pi` the list name of the B-meson has to be passed and the fit fits everything down the stream. 
The vertex fitter has a convenience function: `vertex.treeFit`.


.. hint:: The TreeFit will most likely change the kinematic properties of the decay head (mother) and,
          if the option ``updateAllDaughters`` is turned on, also of the daughter particles.
          To store the quantities prior to the TreeFit and be able to write them out to a ntuple for further analysis
          it is recommended to run `variablesToExtraInfo` before the TreeFit.

Troubleshooting and FAQ
-----------------------

- The TreeFitter is extremely sensitive to how the initial covariance matrix is initialised, we haven't found a good general way to do it. If you see that the signal efficiency of you fit is below 95% this might be the problem. Feel free to report this. But keep in mind garbage in -> garbage out.  
- Currently we are constraint by stack sizes to not extract more than 100 parameters per fit. If this is a problem report this and this can be extended (at the cost of speed of the fits).
- Vertex fitting MC particles (ie. particle lists created with `modularAnalysis.fillParticleListsFromMC`) can lead to crashes. **MC particles do not need to be and should not be vertexed.** If you wish to study the vertex resolution please fit the reconstructed particles and compare the vertex variables with those of the truth matched MC values.   

Parameters of the convenience function
######################################

Import and use the convenience function:

.. autofunction:: vertex.treeFit
   :noindex:

All other parameters are the same as for the module, see below.
The user has to set a parameter ``confidenceLevel`` that corresponds to the p-value as calculated above. Fits with values below the set value will be discarded as junk.

(Advanced) module configuration
###############################

The following are parameters as they would be passed to THE MODULE. The convenience function has slightly different names to make it more pythonic, see the doc of the convenience function. The meaning is the same.
To use the module add it to the path::

    list_name = 'B0:list_name'
    path.add_module('TreeFitter',
        particleList=list_name
        ...
        )
    # additionally set a name for the module in the path, not necessary
    for m in path.modules():
      if "TreeFitter" ==  m.name():
        m.set_name(f'TreeFitter_{list_name}')

.. b2-modules::
   :modules: TreeFitter
   :noindex:

There are more parameters which are based on optimisations we made. It is strongly recommended to not touch them.


