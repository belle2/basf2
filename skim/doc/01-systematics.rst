Systematics skims
~~~~~~~~~~~~~~~~~

.. automodule:: skim.systematics
    :members:
    :undoc-members:

.. _using-syst-combined-skims:

Using ``SystematicsCombined`` skims
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The `SystematicsCombinedHadronic` and `SystematicsCombinedLowMulti` skims are a bit special, as they contain events passing a logical ``OR`` of individual skim classes, and no pre-defined `particle lists <ParticleList>`.
In order to select events of a given skim class in the combination, one has to filter events based on the desired flag first (eg. ``SystematicsKshort``) via :py:func:`modularAnalysis.applyEventCuts`, like in the following example:

.. code-block:: python

    import basf2 as b2
    import modularAnalysis as ma
    import variables.utils as vu
    import variables.collections as vc
    import stdCharged as stdc

    path = b2.create_path()

    input_file = "/PATH/TO/INPUT/SKIM/FILE.root"
    ma.inputMdst("default", filename=input_file, path=path)

    ma.applyEventCuts("eventExtraInfo(passes_SystematicsKshort)", path=path)

    # From now on, it's just a standard analysis!
    # Note that you need to build all the particle lists yourself,
    # as the SystematicsCombined skims do not contain pre-defined lists.

    stdc.stdPi(listtype="all", path=path)
    ma.cutAndCopyList("pi+:good", "pi+:all", f"abs(dr) < 2.0 and abs(dz) < 5.0 and p > 0.1", path=path)

    ma.reconstructDecay("K_S0:sig -> pi+:good pi-:good", "0.45 < M < 0.55", path=path)

    ks_aliases = vu.create_aliases_for_selected(vc.kinematics, "^K_S0 -> pi+ pi-", prefix="K_S0")
    pi_aliases = vu.create_aliases_for_selected(vc.kinematics+vc.pid, "K_S0 -> ^pi+ ^pi-", prefix=["pi_p", "pi_m"])

    ma.variablesToNtuple(decayString="K_S0:sig",
                         variables=ks_aliases+pi_aliases,
                         treename="kspipi",
                         filename="output.root",
                         path=path)

    b2.process(path)

.. note::

    Occasionally, the `SystematicsCombinedHadronic` and `SystematicsCombinedLowMulti` skims are produced in cDST ("rawFormat") format to enable low-level performance studies.
    In such an occurrence, one has to modify the above code snippet slightly to account for the different input data level and the post-tracking reconstruction:

    .. code-block:: python

        from reconstruction import prepare_cdst_analysis

	path.add_module("RootInput", inputFileName=input_file)

	ma.applyEventCuts("eventExtraInfo(passes_SystematicsKshort)", path=path)

	# Run the post-tracking reco.
	prepare_cdst_analysis(path, mc=False) # mc=True if input is MC.
