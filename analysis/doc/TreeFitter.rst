.. _treeFitter:

Tree Fitter
===========



The TreeFitter is a global fitting tool to simultaneously fit an entire decay chain. It tries to find the best decay vertex positions and momenta for the given decay hypothesis. Final state particles will be constraint to their masses, meaning no energy is extracted from the fit but pdg-mass and momentum measurement are used to determine it (same as track fitting procedure). The fit extracts the 4-momenta (->including energy) and decay vertex positions of intermediate particles. If an intermediate particle has a flight length less than :math:`1~\mu m` - for example a strongly decaying hadronic resonance - it's decay and production vertex will be merged. The production vertex is the decay vertex of the mother particle. Note that the other fitters do not do this.

The fit tries to find the solutions to a system of equations or at least the solution closest to the measurement given the measurement uncertainty and noise. The distance between (linearised) hypothesis and measurement is calculated using :math:`\chi^2 = \sum \frac{h-m}{\sigma}` the final goodness of the fit then can be evaluated using the p-value defined as the incomplete (lower) gamma function of the :math:`\chi^2` and degrees of freedom :math:`r` devided by two: :math:`\frac{ 1 - \int^{ \frac{\chi^2}{2} }_{0} t^{\frac{r}{2}-1} \exp{-t} dt }{\Gamma(\frac{r}{2})}`. This quantity denotes the probability the observed :math:`\chi^2` exceeds the expectations for a correct model by chance (see `TMath::Prob(Double_t chi2,Int_t ndf)`).

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
:math:`tau` is the decay length. For intermediates it depends, generally there are 4 equations from the kinematic constraint, if they have a flight length longer than :math:`1~\mu m` AND a well defined production vertex (a mother particle or an origin constraint) we extract the flight length and can use a geometric constraint, which the fit will automatically do unless configured otherwise. If the particle shares the decay vertex with the mother we will only extract this vertex parameters once in the fit. A mass constraint will only be used if the user configures the fit to do so.
So for example for :math:`J\psi` the calculation is the following: parameters: :math: `r = 5 (track) + 5 (track) + 4 (kinematic) - 3 (track) - 3 (track) - 7 (J\psi:~px, py, pz, E, x, y, z) = 1`. For :math:`B->D(K \pi \pi^0)pi` :math:`r = 3\dot 5 (track) + 2 \cdot 3 (gamma) + 3 \cdot 4 (kinematic) + 3 (D:geometric) - 6 (B:~x,y,z,px,py,pz) - 7 (D:~x,y,z,tau,px,py,pz) - 3 (\pi^0:~px,py,pz) - 3 \cdot 3 (track) - 2 \cdot 3 (gamma) = 5`. The :math:`pi^0` does not have to be mass constraint in this example.

Usage: 
######

The user reconstructs the desired decay chain and then passes the head of the tree to the vertex fitter. So when you are reconstructing :math:`B->D(K \pi)pi` the list name of the B-meson has to be passed and the fit fits everything down the stream. The vertex fitter has a convenience function in `/analysis/scripts/vertex.py` the current name is `vertexTree()`. See the documentation there.


Parameters of the conveniece function
#########################
Import and use the convenience function::
    from vertex import vertexTree
    ...
    vertexTree(list_name="B0:some_list", # mandatory
               conf_level= path=path,    # mandatory
               ...                       # optional parameters
               path=path                 # mandatory  
            )

- **list_name** Name of the head particle of the tree. Example: :math:`list_name="B0:some_list_name"`.
- **conf_level** p-value as explained above. Candidates with values below this cut will be removed from the particle list. The value to use depends on your analysis and it's figure of merit. Example :math:`conf_level=0.001`. Example :math:`conf_level=-1` to keep all candidates. The framework variable is called `chiProb`. 
- **massConstraint** List of pdg codes to mass constrain. Example :math:`massConstraint=[111]` cosntrain all :math:`\pi^0`.
All other parameters are the same as for the module, see below.

The user has to set a parameter `confidenceLevel` that corresponds to the p-value as calculated above. Fits with values below the set value will be discarded as junk.

Parameters of the MODULE:
#########################

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


- **particleList** Name of the head particle of the tree. Example: :math:`particleList="B0:some_list_name"` 
- **confidenceLevel** p-value cut for the particles to remove from the particle list. Set to :math:`-1` to keep all particles. Example: :math:`confidenceLevel=0.001`
- **updateAllDaughters** Update all daughters with the values found by the fit. By default only the head of tree will be updated, so that for example in math:`B->D(K \pi)pi` only the Bmeson will have the parameters of the fit. The D-meson will have the parameters inferred purely be the measurement of its daughters. Example: :math:`updateAllDaughters=True`
- **massConstraintList** list of pdg codes of particles to mass constrain. Example: :math:`massConstraintList=[111]` to constrain ALL :math:`\pi^0` in the fit. 
- **ipConstraint** constraint the PRODUCTION vertex of the head of the tree to the interaction point as obtained from the conditions database. Example: :math:`massConstraintList=[111]`
- **customOriginConstraint** constrain the PRODUCTION vertex of the head of tree to a custom region. The default values are the vertex and covariance of B-meson decays around the default IP. One could use this for example for D*-mesons where the mother decays with a neutrino, so that one has the geometric constraint but does not have to fit the B-meson. 
- **customOriginVertex** position used by **customOriginConstraint**. Default is the median decay position of B-meson obtained from MC9. Example: :math:`customOriginVertex=[x, y, z]` where xyz are floats.
- **customOriginCovariance** covariance used by **customOriginConstraint**. Default is the width of B-decays obtained from MC9. Implemented as row major vector: Example: :math:`customOriginVertex=[x, xy, xz, yx, yy, yz, zx, zy, zz]` where xyz are floats.
- **originDimension** The dimension used by **ipConstraint**. :math: `3 \rightarrow x,y,z`, :math: `2 \rightarrow x,y` Example: :math:`originDimension=2`. Only set this if you want a 2d beamconstraint. Does not work with custom origin, yet.
- **inflationFactorCovZ** a scalar to multiply the z-components (all of them zz, zx, zy) of the 3d ip-constraint covariance matrix with. This weakens the constraints z-component. Example: :math:`inflationFactorCovZ=1e5`.
- **autoSetGeoConstraintAndMergeVertices** Automatically determine what particles can have a geometric constraint and which decay vertices can be joined with the mother. Default is True. Don't touch this unless you really know why. Example: :math:`autoSetGeoConstraintAndMergeVertices=False`
- **geoConstraintList** If **autoSetGeoConstraintAndMergeVertices** is set to false this can be used to set the pdg codes of the particle you want to mass constrain. Example: :math:`geoConstraintList=[310] # 310 is K_S0`.  
- **sharedVertexList** If **autoSetGeoConstraintAndMergeVertices** is set to false this can be used to set the pdg codes of the particles who's decay vertices should be joined with their mothers. Example: :math:`sharedVertexList=[443] # 310 is Jpsi`.

There are more parameters which are based on optimisations we made. I strongly recommend to not touch them.

Known problems:
###############
- The TreeFitter is extremely sensitive to how the initial covariance matrix is initialised I haven't found a good general way to do it. If you see that the signal efficiency of you fit is below 95% this might be the problem. Feel free to send me an email. But keep in mind garbage in -> garbage out.  
- Currently we are constraint by stack sizes to not extract more than 100 parameters per fit. If this is a problem let me know and I can extend this (at the cost of speed of the fits).

.. autofunction:: vertex.vertexTree

Cite:
#####
Soon, paper under review for NIM.
