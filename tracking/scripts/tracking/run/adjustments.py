"""Contains functions that operate on populated path adjust some module parameters"""


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


def disable_deltas(path):
    """Disables the generation of delta electrons in FullSim

    Arguments
    ---------
    path : basf2.Path
    """

    for module in path.modules():
        if module.type() == 'FullSim':
            module.param('ProductionCut', 1000000.)


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
