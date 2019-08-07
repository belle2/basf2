Deep Flavor Tagger
==================

**Authors: J. Gemmler**

The Deep Flavor Tagger is a multivariate tool to estimate the
flavor of :math:`B_{tag}` mesons, without having to reconstruct the decays
explicitly.



Principle
---------
Many B mesons decay in flavor specific decay modes, which offer a unique
signature for their final state particles. Certain attributes, like fast
leptons or slow pions can be used to infer directly to the flavor of the
tag B meson.

As opposed to the category based flavor tagger, see :ref:`analysis/doc/FlavorTagger:Flavor Tagging Principle` ,
those specific attributes are not incorporated by hand, but the representation data is directly 'learned' by a deep
neural network, which is described in the next section.

The advantage of this method is that the algorithm is potentially susceptible to exploit a wider case of attributes of
the dataset, than putting in only explicit knowledge by pre crafted features.

Furthermore this method allows a variety of domain adaption methods, which could be object of
future studies.


Algorithm
---------

The core of the algorithm is a multilayer perceptron with 8 hidden layers. It is trained with
stochastic gradient descent.

.. figure:: figs/network_scheme.png

It is implemented with the `tensorflow <https://www.tensorflow.org/about>`_ framework, which is interfaced via the
`mva`.

Currently, the input of the algorithm is based on tracks only, but can be extended to other objects, e.g. cluster
without too many changes.
The charged tracks are grouped by charged, and sorted by momentum. If an event has less then 5 positively or 5
negatively charged tracks, a specific kind of zero padding is applied.
The scheme of the input parameters are shown below:

.. figure:: figs/tracks_scheme.png
  :width: 20em
  :align: center


The output of the algorithm :math:`O(tag)`can be interpreted as a probability to have a :math:`B^\_{tag}^0`. The range of
the output is :math:`[0, 1]`. If it is likely that the tagside of an event is related to a :math:`\bar{B}_{tag}^0` the
output is in the regime of 0. The value 0.5 corresponds to a random decision.

It can easily transformed to  ``qr`` with

::

   qr = 2 * O(tag) - 1.


How to use
----------

The usage of the Deep Flavor Tagger is pretty simple and straight forward.

At first, you have to be sure, that you have downloaded the correct weight file from the database.

::

   # for now
   b2conditionsdb download analysis_tools_release-03-01-00 -f official_rel010204_roeB0Flavor_mine380_maxe500_seed1241  # for b2bii
   b2conditionsdb download analysis_tools_release-03-01-00 -f mc12b_roeB0Flavor_mine380_maxe500_seed1237  # for belle2 (mc12b)

::

   # in the near future
   b2conditionsdb download analysis_tools_release-<current release> -f FlavorTagger_Belle_B2nunubarBGx1DNN_1 # for b2bii
   b2conditionsdb download analysis_tools_release-<current release> -f FlavorTagger_Belle2_B2nunubarBGx1DNN_1  # for belle2

There are also currently efforts to integrate the database functions directly in the classifier to make the usage even
more user-friendly.

The files will be available in the working directory at

::

   centraldb/dbstore_FlavorTagger_Belle_B2nunubarBGx1DNN_1.root # for b2bii
   centraldb/dbstore_FlavorTagger_Belle2_B2nunubarBGx1DNN_1.root # for belle2


In your *steering file* you have to import the the interface to the Flavor Tagger

::

    from dft.DeepFlavorTagger import DeepFlavorTagger

After reconstructing your signal B meson , make sure that you build the rest of event:

::

    ma.buildRestOfEvent('B0:sig', path=path)

To apply with basic functionality, considering the weight file name is ``FlavorTagger_Belle2_B2nunubarBGx1DNN_1``

.. note::
    if available, the algorithm can also be used with .xml weight files e.g.
    ``FlavorTagger_Belle2_B2nunubarBGx1DNN_1.xml``


::

    DeepFlavorTagger('B0:sig',
                     mode='expert',
                     uniqueIdentifier='FlavorTagger_Belle2_B2nunubarBGx1DNN_1',
                     output_variable='network_output',
                     path=path)

The output variable will be available as extra info to the signal particle list 'B0:sig' and can for example be
written out with the following command. It is probably also helpful for MC studies, to write out the MC truth of the
tag side.

::

    ma.variablesToNtuple(decayString='B0:sig',
                         variables=['qrCombined', 'extraInfo(network_output)'],
                         path=path)

To use the flavor tagger with b2bii you have to select a different set of variables:

::

    BELLE_FLAVOR_TAG_VARIABLES = [
        'useCMSFrame(p)',
        'useCMSFrame(cosTheta)',
        'useCMSFrame(phi)',
        'atcPIDBelle(3,2)',
        'eIDBelle',
        'muIDBelle',
        'atcPIDBelle(4,2)',
        'nCDCHits',
        'nSVDHits',
        'dz',
        'dr',
        'chiProb',
    ]

    DeepFlavorTagger(..., variable_list=BELLE_FLAVOR_TAG_VARIABLES)

Studies have shown, that we can reduce a certain kind of bias, with applying additional rest of event cuts, you can do
this with the following argument. You can basically treat this like cuts in an RoE mask.

::

    DeepFlavorTagger(..., additional_roe_filter='dr < 2 and abs(dz) < 4')




Functions
---------

.. automodule:: dft.DeepFlavorTagger
   :members: DeepFlavorTagger
   :undoc-members:
