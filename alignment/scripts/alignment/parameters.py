from ROOT import Belle2


def cdc_layers(layers=None):
    if layers is None:
        layers = [lyr for lyr in range(0, 56)]

    result = []

    wire = 511
    for layer in layers:
        for param in [1, 2, 6, 11, 12, 16]:
            wireid = Belle2.WireID(layer, wire).getEWire()
            label = Belle2.GlobalLabel()
            label.construct(Belle2.CDCAlignment.getGlobalUniqueID(), wireid, param)
            result.append(label.label())

    return result


def cdc_wires(layers=None):
    if layers is None:
        layers = range(0, 56)

    wires_in_layer = [
        160, 160, 160, 160, 160, 160, 160, 160,
        160, 160, 160, 160, 160, 160,
        192, 192, 192, 192, 192, 192,
        224, 224, 224, 224, 224, 224,
        256, 256, 256, 256, 256, 256,
        288, 288, 288, 288, 288, 288,
        320, 320, 320, 320, 320, 320,
        352, 352, 352, 352, 352, 352,
        384, 384, 384, 384, 384, 384]

    result = []

    for layer in layers:
        for wire in range(0, wires_in_layer[layer]):
            # Unique id of CDCAlignment db object
            cdcid = Belle2.CDCAlignment.getGlobalUniqueID()

            wireid = Belle2.WireID(layer, wire).getEWire()
            label = Belle2.GlobalLabel()

            label.construct(cdcid, wireid, Belle2.CDCAlignment.wireBwdX)
            result.append(label.label())

            label.construct(cdcid, wireid, Belle2.CDCAlignment.wireFwdX)
            result.append(label.label())

            label.construct(cdcid, wireid, Belle2.CDCAlignment.wireBwdY)
            result.append(label.label())

            label.construct(cdcid, wireid, Belle2.CDCAlignment.wireFwdY)
            result.append(label.label())

    return result


def cdc_t0s():
    wires_in_layer = [
        160, 160, 160, 160, 160, 160, 160, 160,
        160, 160, 160, 160, 160, 160,
        192, 192, 192, 192, 192, 192,
        224, 224, 224, 224, 224, 224,
        256, 256, 256, 256, 256, 256,
        288, 288, 288, 288, 288, 288,
        320, 320, 320, 320, 320, 320,
        352, 352, 352, 352, 352, 352,
        384, 384, 384, 384, 384, 384]

    result = []

    for layer in range(0, 56):
        for wire in range(0, wires_in_layer[layer]):
            label = Belle2.GlobalLabel()
            label.construct(Belle2.CDCTimeZeros.getGlobalUniqueID(), Belle2.WireID(layer, wire).getEWire(), 0)
            result.append(label.label())

    return result


def vxd_halfshells(pxd=True, svd=True, parameters=None, ying=True, yang=True, pat=True, mat=True):
    if parameters is None:
        parameters = [1, 2, 3, 4, 5, 6]

    result = []

    shells = []
    _ying = Belle2.VxdID(1, 0, 0, 1)
    _yang = Belle2.VxdID(1, 0, 0, 2)
    _pat = Belle2.VxdID(3, 0, 0, 1)
    _mat = Belle2.VxdID(3, 0, 0, 2)

    if pxd:
        if ying:
            shells.append(_ying)
        if yang:
            shells.append(_yang)
    if svd:
        if pat:
            shells.append(_pat)
        if mat:
            shells.append(_mat)

    for vxdid in shells:
        for param in parameters:
            vxdid = Belle2.VxdID(vxdid).getID()
            label = Belle2.GlobalLabel()
            label.construct(Belle2.VXDAlignment.getGlobalUniqueID(), vxdid, param)
            result.append(label.label())

    return result


def beamspot():

    result = []

    for param in [1, 2, 3]:
        label = Belle2.GlobalLabel()
        label.construct(Belle2.BeamSpot.getGlobalUniqueID(), 0, param)
        result.append(label.label())

    return result


def vxd_ladders(layers=None, parameters=None):
    if layers is None:
        layers = [1, 2, 3, 4, 5, 6]
    if parameters is None:
        parameters = [1, 2, 3, 4, 5, 6]

    result = []

    ladders = [8, 12, 7, 10, 12, 16]

    for layer in layers:
        for ladder in range(1, ladders[layer - 1] + 1):
            for ipar in parameters:
                label = Belle2.GlobalLabel()
                label.construct(Belle2.VXDAlignment.getGlobalUniqueID(), Belle2.VxdID(layer, ladder, 0).getID(), ipar)
                result.append(label.label())

    return result


def vxd_sensors(layers=None, rigid=True, surface=True, surface2=True, surface3=True, surface4=True, parameters=None):
    if layers is None:
        layers = [1, 2, 3, 4, 5, 6]

    params_rigid = [1, 2, 3, 4, 5, 6]
    params_surface2 = [31, 32, 33]
    params_surface3 = [41, 42, 43, 44]
    params_surface4 = [51, 52, 53, 54, 55]

    params = []
    if rigid:
        params += params_rigid

    if surface:
        if surface2:
            params += params_surface2
        if surface3:
            params += params_surface3
        if surface4:
            params += params_surface4

    # Allow full override by user
    if parameters:
        params = parameters

    result = []

    ladders = [8, 12, 7, 10, 12, 16]
    sensors = [2, 2, 2, 3, 4, 5]

    for layer in layers:
        for ladder in range(1, ladders[layer - 1] + 1):
            for sensor in range(1, sensors[layer - 1] + 1):
                for param in params:
                    label = Belle2.GlobalLabel()
                    label.construct(Belle2.VXDAlignment.getGlobalUniqueID(), Belle2.VxdID(layer, ladder, sensor).getID(), param)
                    result.append(label.label())
    return result


def vxd():
    return vxd_sensors() + vxd_ladders() + vxd_halfshells()


def pxd():
    return vxd_sensors(layers=[1, 2]) + vxd_ladders(layers=[1, 2]) + vxd_halfshells(pxd=True, svd=False)


def svd():
    return vxd_sensors(layers=[3, 4, 5, 6]) + vxd_ladders(layers=[3, 4, 5, 6]) + vxd_halfshells(pxd=False, svd=True)


def all():
    # TODO: klm
    return beamspot() + pxd() + svd() + cdc_layers() + cdc_wires() + cdc_t0s()


if __name__ == '__main__':
    print("Number of available parameters:")
    print("BeamSpot:", len(beamspot()))
    print("PXD:", len(pxd()))
    print("SVD:", len(svd()))
    print("( VXD:", len(vxd()), ")")
    assert(len(vxd()) == len(pxd()) + len(svd()))
    print("CDC layers:", len(cdc_layers()))
    print("CDC wires:", len(cdc_wires()))
    print("CDC T0s:", len(cdc_t0s()))
    print("TOTAL:", len(all()))
