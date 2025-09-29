##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

'''
Convenience function to add the SmartBackground module to the path
and handle event selection/rejection.
'''

import basf2 as b2


def add_smartbkg_filtering(
        skim_code,
        path,
        payload="SmartBKGWeights.onnx",
        empty_path=None,
        debug_mode=False,
        event_type=None,
        activation_params=None):
    """
        Adds event preselection based on the SmartBkg neural network.
        Should be used only for directly skimmed MC productions.
        Must be added to the path after the generator.add_abc_generator but before simulation.add_simulation modules.
        Given a specific skim the neural network predicts the probability of the event passing the skim.
        The event is then kept in the given path with this probability or otherwise discarded to the empty path.
        Kept events are weighted with their inverse probability to ensure unbiased distributions
        (weights are saved in the event meta data,  by multiplying them to the generated weight).
        Use case is the reduction of simulation time for directly skimmed MC productions.

        Parameters:
            skim_code (int or str): LFN code of the used skim (available for all skims via skim.code)
            path (basf2.Path): main path with generator modules, used for pass events
            payload (str): name of the payload storing neural network weights in ONNX format
            empty_path (basf2.Path or None): path rejected events are given to (new empty path if None)
            debug_mode (bool): enables debug mode (events are never rejected, instead the neural network prediction
                               is written to the event extra info as 'SmartBKG_Prediction')
            event_type (str or None): type of events thar are generated, allowed values are
                                      'charged', 'mixed', 'uubar', 'ddbar', 'ssbar', 'ccbar', 'taupair';
                                      if None, automatially determined from the event extra info
            activation_params (tuple(float, float) or None): custom parameters (a, b) for the activation function
                                                             (useful for testing/validation);
                                                             if None, prefitted values for the chosen skim are used
    """

    sbkg = b2.register_module("SmartBackground")
    skim_code = int(skim_code)
    sbkg.param("skimCode", skim_code)
    sbkg.param("debugMode", debug_mode)
    sbkg.param("payload", payload)
    if event_type is not None:
        sbkg.param("overrideEventType", True)
        sbkg.param("eventType", event_type)
    if activation_params is not None:
        sbkg.param("overrideActivation", True)
        if len(activation_params) != 2:
            b2.B2FATAL("add_smartbkg_filtering: activation_override_params must be a list or tuple of two floats (a, b)")
        sbkg.param("activationOverrideParams", [float(x) for x in activation_params])
    path.add_module(sbkg)
    if empty_path is None:
        empty_path = b2.create_path()
    sbkg.if_value("==0", empty_path)
