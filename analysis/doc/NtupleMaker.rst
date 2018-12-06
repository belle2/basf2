NTupleMaker
============

.. warning::
        These tools are deprecated and will be removed. Please see `VariableManagerOutput` as the recommended alternative.

The `NtupleMaker`  module builds ntuples from a `ParticleList`_  . There is a number of ready-to-use plugins called `NtupleTool` for saving standard variables to the ntuples. These plugins can be selected and configured directly in the basf2 script.

How to create ntuples using the :doc:`NtupleMaker` module
---------------------------------------------------------

At the end of an analysis chain you end up with a list of Particle objects in the DataStore and you want to write the properties (e.g. kinematics or truth information) of these particles to an ntuple (TTree). Everybody who has done this before knows that this is a lot of work because every variable has to be defined, introduced to the TTree and set to the correct value.

In Belle II this work is done for you by the :doc:`NtupleMaker` module! Although individual analyses differ in particle reconstruction or selection criteria, at the end you need the same variables in the ntuple, e.g. the kinematics of the particles.

Let's consider that you reconstructed the decay :math:`B^-\to D^0(K^+\pi^-)\pi^-` by the other analysis modules and that a list 'B-:d0pi' with :math:`B^-` candidates is on the DataStore.

To create an ntuple in a ROOT file you need to add the :doc:`NtupleMaker`  module to you basf2 path (here by default to: `analysis_main` ). Instead of registering the module and setting its parameter by yourself you can use ntupleFile and ntupleTree python wrapper functions. In the beginning of your python script import these two functions from :code:`modularAnalysis.py`:

.. code-block:: python

    from modularAnalysis import ntupleFile
    from modularAnalysis import ntupleTree


Now create a new ROOT file:

.. code-block:: python

    ntupleFile('B2D0Pi-output.root')


Then you need to define what kind of information would you like to save and for which particles in the decay chain:

.. code-block:: python

    toolsB = ['EventMetaData', '^B-']
    toolsB += ['Kinematics', '^B- -> [^D0 -> ^K- ^pi+] ^pi-']
    toolsB += ['DeltaEMbc', '^B-']
    toolsB += ['PID', 'B- -> [D0 -> ^K- ^pi+] ^pi-']
    toolsB += ['MassBeforeFit', 'B- -> ^D0 pi-']
    toolsB += ['MCTruth', '^B- -> ^D0 ^pi-']

The particles in the decay chain for which you wish to save the desired information provided by the NtupleTool is selected with the :code:`^` sign. From the example above the PID related information is saved only for charged final state particles (:math:`K^-` and :math:`\pi^+` used to reconstruct the :math:`D^0` candidates and :math:`\pi^-` used in reconstruction of the :math:`B^-` candidates). Selecting :math:`B^-` and :math:`D^0` for PID NtupleTool wouldn't make sense, since the PID information is not available for composite (decaying) particles.

.. code-block:: python

    toolsB += ['PID', 'B- -> [D0 -> ^K- ^pi+] ^pi-']

On the other hand, `Mbc` and `deltaE` are meaningful only for B candidates therefore only the B candidate is selected when adding the `DeltaEMbc` NtupleTool:

.. code-block:: python

    toolsB += ['DeltaEMbc', '^B-']

Finally, tell the :doc:`NtupleMaker` what should be the name of the output tree and make the connection between `ParticleList`_  and the Ntuple tools

 
.. code-block:: python

    ntupleTree('btree', 'B-:d0pi', toolsB)


Ok, the :doc:`NtupleMaker`  is now initialised and it will create the ntuple btree in the file :code:`B2D0Pi-output.root`. The tree will contain variables given by the selected NtupleTools. The full list of these tools can be found here.

The name of the NtupleTool is followed by a DecayString. The :ref:`DecayString` holds the information how the :math:`B^-` particles are reconstructed, e.g. that the first daughter of the :math:`B^-` is a :math:`D^0` decaying to (:math:`K^+,\pi^-`). Only the kinematics of the particles selected with a preceeding :code:`^` are saved. In the case of the `EventMetaData` and `RecoStats` `NtupleTools` the :ref:`DecayString` is ignored. `Particle`_  names are defined to be those in evt.pdl, the `EvtGen <https://confluence.desy.de/display/BI/Software+EvtGen>`_ particle data table.


What if I want to create more than one ntuple?
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

You can add several :doc:`NtupleMaker` modules to the same path for example to save different decay channels. Just add a second :doc:`NtupleMaker` to the path and initialise it as described above:

.. code-block:: python

    toolsD = ['Kinematics', '^D0 -> ^K- ^pi+']
    toolsD += ['PID', 'D0 -> ^K- ^pi+']
    toolsD += ['MCTruth', '^D0']
    ntupleTree('d0tree', 'D0:kpi', toolsD)

This will write :code:`d0tree` to the file B2D0Pi-output.root filled with :math:`D^0` candidates from :code:`D0:kpi` `ParticleList`_ .

Please note that the output file names have to be identical. Output to multiple root files by the same job are not possible.


How do I save information for every event?
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

If you provide an empty string for the `ParticleList`_  name, the :doc:`NtupleMaker`  runs the `NtupleTools` for each event.

(You need to provide a valid :ref:`DecayString` for the NtupleTools, just use :code:`B-` or similar, it has no influence on the output.)

 

What if there is no NtupleTool for my brand new fancy variable?
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

You can write your own variable and use the ``CustomFloats`` tool. Detailed instructions can be found in `here`_. 
Please consider making a pull request so that your colleagues from the collaboration can profit, too.

.. _here: https://confluence.desy.de/display/BI/How+to+add+a+variable+to+the+VariableManager

.. _ParticleList: https://b2-master.belle2.org/software/development/classBelle2_1_1ParticleList.html

.. _Particle: https://b2-master.belle2.org/software/development/classBelle2_1_1Particle.html
