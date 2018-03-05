"""Contains functions that operate on populated path adjust some module parameters"""

import basf2


def adjust_module(path, module_type, params={}, **kwds):
    """Set parameters of the module with the given type in the path to the given parameters

    Arguments
    ---------
    path : basf2.Path
        Module path to be adjusted.
    module_type : str
        Type of the module to adjusted as returned by Module.type()
    params : dict
       Dictionary of parameters served to Module.param(params)
    **kwds
       Keyword parameters served to Module.param(params)
    """
    for module in path.modules():
        if module.type() == module_type:
            module.param(params)
            module.param(kwds)
        for sub_path in module.get_all_condition_paths():
            adjust_module(sub_path, module_type, params=params, **kwds)


def skip_modules_after(path, module_type):
    """Inserts a conditional path at the module of the given type such that modules after are skipped.

    Arguments
    ---------
    path : basf2.Path
        Module path to be adjusted.
    module_type : str
        Type of the module to adjusted as returned by Module.type()
    """
    for module in path.modules():
        if module.type() == module_type:
            module.return_value(False)
            module.if_false(basf2.create_path())
            return


def disable_deltas(path):
    """Disables the generation of delta electrons in FullSim

    Arguments
    ---------
    path : basf2.Path
    """

    for module in path.modules():
        if module.type() == 'FullSim':
            module.param('ProductionCut', 1000000.)


def keep_secondaries(path):
    """Keep all secondaries from the FullSim

    Arguments
    ---------
    path : basf2.Path
    """

    for module in path.modules():
        if module.type() == 'FullSim':
            module.param({
                "StoreAllSecondaries": True,
                "SecondariesEnergyCut": 0,
                "trajectoryStore": 2,
            })


def enable_wire_by_wire_mode(path):
    """Enables the wire by wire material distribution in the CDC

    Arguments
    ---------
    path : basf2.Path
    """
    for module in path.modules():

        if module.type() == 'Gearbox':
            module.param('override',
                         [("/DetectorComponent[@name='CDC']//MaterialDefinitionMode",
                           "2", ""),
                          ]
                         )
        if module.type() == 'FullSim':
            # per Ozaki-san's recommendatation for wire-by-wire CDC mode
            module.param('deltaChordInMagneticField', 0.001)
