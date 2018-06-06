

NTupleMaker
============

The `NtupleMaker`  module builds ntuples from a `ParticleList`_  . There is a number of ready-to-use plugins called `NtupleTool` for saving standard variables to the ntuples. These plugins can be selected and configured directly in the basf2 script.

Click here to learn how to develop your own NtupleTool

How to create ntuples using the `NtupleMaker` module
----------------------------------------------------

At the end of an analysis chain you end up with a list of Particle objects in the DataStore and you want to write the properties (e.g. kinematics or truth information) of these particles to an ntuple (TTree). Everybody who has done this before knows that this is a lot of work because every variable has to be defined, introduced to the TTree and set to the correct value.

In Belle II this work is done for you by the `NtupleMaker` module! Although individual analyses differ in particle reconstruction or selection criteria, at the end you need the same variables in the ntuple, e.g. the kinematics of the particles.

Let's consider that you reconstructed the decay :math:`B^-\to D^0(K^+\pi^-)\pi^-` by the other analysis modules and that a list 'B-:d0pi' with :math:`B^-` candidates is on the DataStore.

To create an ntuple in a ROOT file you need to add the `NtupleMaker`  module to you basf2 path (here by default to: `analysis_main` ). Instead of registering the module and setting its parameter by yourself you can use ntupleFile and ntupleTree python wrapper functions. In the beginning of your python script import these two functions from :code:`modularAnalysis.py`:

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

Finally, tell the `NtupleMaker` what should be the name of the output tree and make the connection between `ParticleList`_  and the Ntuple tools

 
.. code-block:: python

    ntupleTree('btree', 'B-:d0pi', toolsB)


Ok, the `NtupleMaker`  is now initialised and it will create the ntuple btree in the file :code:`B2D0Pi-output.root`. The tree will contain variables given by the selected NtupleTools. The full list of these tools can be found here.

The name of the NtupleTool is followed by a DecayString. The :ref:`DecayString` holds the information how the :math:`B^-` particles are reconstructed, e.g. that the first daughter of the :math:`B^-` is a :math:`D^0` decaying to (:math:`K^+,\pi^-`). Only the kinematics of the particles selected with a preceeding :code:`^` are saved. In the case of the `EventMetaData` and `RecoStats` `NtupleTools` the :ref:`DecayString` is ignored. `Particle`_  names are defined to be those in evt.pdl, the `EvtGen <https://confluence.desy.de/display/BI/Software+EvtGen>`_ particle data table.


What if I want to create more than one ntuple?
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

You can add several `NtupleMaker` modules to the same path for example to save different decay channels. Just add a second `NtupleMaker` to the path and initialise it as described above:

.. code-block:: python

    toolsD = ['Kinematics', '^D0 -> ^K- ^pi+']
    toolsD += ['PID', 'D0 -> ^K- ^pi+']
    toolsD += ['MCTruth', '^D0']
    ntupleTree('d0tree', 'D0:kpi', toolsD)

This will write :code:`d0tree` to the file B2D0Pi-output.root filled with :math:`D^0` candidates from :code:`D0:kpi` `ParticleList`_ .

Please note that the output file names have to be identical. Output to multiple root files by the same job are not possible.


How do I save information for every event?
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

If you provide an empty string for the `ParticleList`_  name, the `NtupleMaker`  runs the `NtupleTools` for each event.

(You need to provide a valid :ref:`DecayString` for the NtupleTools, just use :code:`B-` or similar, it has no influence on the output.)

 

What if there is no NtupleTool for my brand new fancy variable?
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

You can write your own NtupleTool. Detailed instructions can be found in this tutorial. Please consider uploading your NtupleTool to svn that your colleagues from the collaboration can profit, too.

 

Aren't the predefined tools limiting the creativity of my analysis?
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

No, not at all. These tools do the boring part for you and you have more time for the real physics.

How to develop your own NtupleTool
----------------------------------

Before starting developing a new `NtupleTool`, check here if not somebody else has already done the work for you. The `NtupleTool`s are located in the directory :code:`analysis/NtupleTools/`. A good start is to look there and see how the existing tools are implemented.

Let's assume that you want to save the masses of the particles. Start by copying a header file of an existing `NtupleTool`, e.g. `Kinematics`. Change the name to the name you want, e.g. `NtupleMassTool` (don't forget to make sure you have a :code:`#pragma once` in your header). Then add the declaration of the variables you want to save. We use a pointer to an array of floats. Its length will be defined at run time depending on the number of particles selected in the basf2 steering script. Of course, it is possible to save more than one variable with a single `NtupleTool`.

.. code-block:: C++

    /** Particle mass. */
    float* m_fMass;

.. seealso:: 4-vectors are best saved as :code:`float[4]`. In your analysis macro you can initialise a `TLorentzVector` directly from :code:`float[4]`. (A `TLorentzVector` could be also written to the ntuple, but internally it uses double precision four the components which unnecessarily increases the size by a factor of two).


Now, we move to the source file and define how the branches of the TTree are created. `NtupleTool` inherits from the class `NtupleFlatTool` a pointer to the TTree m_tree and the `DecayDescriptor` m_decaydescriptor. From m_decaydescriptor we obtain the number of selected particles and their names. You will need a function that sets up the tree. Note the golden rule: if you use the C++ keyword new you should have a matching delete.

.. code-block:: C++

    /** Constructor */
    void NtupleMyTool::NtupleMyTool() {
    vector<string> strNames = m_decaydescriptor.getSelectionNames();
    int nDecayProducts = strNames.size();
    m_fMass = new float[nDecayProducts];   
    for (int iProduct = 0; iProduct < nDecayProducts; iProduct++) {
        m_tree->Branch((strNames[iProduct] + "_M").c_str(), &m_fMass[iProduct], (strNames[iProduct] + "_M/F").c_str());
        }
    }
 
    /** Destructor */
    void NtupleMyTool::~NtupleMyTool() {
        delete[] m_fMass; // deallocates the memory
    }

Finally, we define how the variable is calculated. The pointer p refers to a Particle in the list defined by the NtupleMaker parameter :code:`strList`. If the particle itself is selected and/or some of its daughters is given by the decay string in the basf2 steering script. The list of selected particles selparticles is obtained with the :code:`getSelection(Particle* p)` method of the `DecayDescriptor`.

.. code-block:: C++

    void NtupleKinematicsTool::eval(const Particle* particle) {
        if (!particle) {
            printf("NtupleKinematicsTool::eval - ERROR, no Particle found!\n");
            return;
        }
        vector <const Particle*> selparticles = m_decaydescriptor.getSelectionParticles(particle);
        int nDecayProducts = selparticles.size();
        for (int iProduct = 0; iProduct < nDecayProducts; iProduct++) {
            m_fMass[iProduct] = selparticles[iProduct]->getMass();
        }
    }

To tell the `NtupleMaker`  of the existence of the new NtupleTool we add in :code:`analysis/NtupleTools/src/NtupleToolList.cc`: 

.. code-block:: C++

    else if (strName.compare("Mass") == 0) return new NtupleMassTool(tree, d);

Comments
~~~~~~~~

Please share your tools with your colleagues from the collaboration.

* Try to group related variables in one NtupleTool, but not too many. As a rule of thumb: 2-5 variables are appropriate.
* Consider using Char_t (-127...127) for flags.
* If you wrote your tuple tool but it doesn't work in your steering file, it might happen that you forget to add it to :code:`NtupleListTool.cc`



NTuple tools
------------

Documentation about ntuple tool is not yet migrated from our `confluence page <https://confluence.desy.de/display/BI/Physics+NtupleTool>`_. 
If you have time to do the migration, feel free to take `agira ticket <https://agira.desy.de/browse/BII-2976>`_. In this case you might need nice guid on Sphinx documentation: :ref:`doctools` .

.. _ParticleList: https://b2-master.belle2.org/software/development/classBelle2_1_1ParticleList.html

.. _Particle: https://b2-master.belle2.org/software/development/classBelle2_1_1Particle.html
