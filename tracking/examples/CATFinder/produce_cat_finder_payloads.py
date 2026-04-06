##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

import torch
import yaml
from gnn_tracking import CDCNet


def produce_onnx_model():
    model_path = '/path/to/model.pt'
    config_path = '/path/to/config.yaml'

    with open(config_path) as f:
        config = yaml.safe_load(f)

    _net = CDCNet(
            input_dim=len(config['dataset']['input_features']),
            k=config['model']['k'],
            nblocks=config['model']['blocks'],
            coord_dim=config['model']['coord_dim'],
            dim1=config['model']['dim1'],
            dim2=config['model']['dim2'],
            space_dimensions=config['model'].get('space_dimensions', 4),
            momentum=config['model'].get('momentum', 0.6),
        ).to('cpu')

    loaded_model = torch.load(model_path, map_location='cpu')

    state_dict = loaded_model['model_state_dict']
    keys = list(state_dict.keys())
    new_state_dict = {}
    for i in range(len(keys)):
        key = keys[i]
        if key.startswith('module.'):
            key = key.replace('module.', '', 1)
        # Remap old checkpoint name to new attribute name
        if key.startswith('p_ccoords_layer'):
            key = key.replace('p_ccoords_layer', 'p_coords_layer', 1)
        new_state_dict[key] = state_dict[keys[i]]

    _net.load_state_dict(new_state_dict)
    _net.eval()

    torch.save(_net.state_dict(), "cdcnet.pt")

    with torch.no_grad():
        x = torch.rand(1000, 7)
        torch.onnx.export(
            _net,
            (x,),
            "cdcnet.onnx",
            input_names=["input"],
            output_names=["beta", "coordinates", "momentum", "vertex", "charge"],
            dynamic_shapes=[{0: "hits"}],
            dynamo=True,
            external_data=False,
        )


def produce_payloads():
    import ROOT  # noqa

    iov = ROOT.Belle2.IntervalOfValidity(0, 0, -1, -1)
    database = ROOT.Belle2.Database.Instance()

    onnx_path = '/path/to/cdcnet.onnx'

    database.addPayload('CATFinderWeightFile', onnx_path, iov)

    parameters = ROOT.Belle2.CATFinderParameters()
    parameters.setTDCOffset(4100)
    parameters.setTDCScale(1100)
    parameters.setADCClip(600)
    parameters.setSLayerScale(10)
    parameters.setCLayerScale(56)
    parameters.setLayerScale(10)
    parameters.setSpatialCoordinatesScale(100)
    parameters.setNInputFeatures(7)
    parameters.setLatentSpaceNDim(3)
    parameters.setTBeta(0.7)
    parameters.setTDistance(0.7)
    parameters.setMaxRadius(0.3)
    parameters.setMinNumberHits(7)
    parameters.setInputTFeaturesName('input')
    parameters.setOutputTBetaName('beta')
    parameters.setOutputTCoordinatesName('coordinates')
    parameters.setOutputTMomentumName('momentum')
    parameters.setOutputTVertexName('vertex')
    parameters.setOutputTChargeName('charge')

    database.storeData('CATFinderParameters', parameters, iov)


if __name__ == '__main__':
    produce_onnx_model()
    produce_payloads()
