#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import basf2 as b2
from ROOT import Belle2

b2.logging.log_level = b2.LogLevel.WARNING


class CheckTrueHits(b2.Module):

    """
    Lists TrueHits with MCParticles and SimHits that generated them.
    """

    def __init__(self):
        """Initialize the module"""

        super(CheckTrueHits, self).__init__()
        #: TrueHit statistics for the PXD
        self.truehit_stats_pxd = {
            'through': {'total': 0, 'secondary': 0, 'remapped': 0},
            'into': {'total': 0, 'secondary': 0, 'remapped': 0},
            'out': {'total': 0, 'secondary': 0, 'remapped': 0},
            'inside': {'total': 0, 'secondary': 0, 'remapped': 0}
        }
        #: Truehit statistics for the SVD
        self.truehit_stats_svd = {
            'through': {'total': 0, 'secondary': 0, 'remapped': 0},
            'into': {'total': 0, 'secondary': 0, 'remapped': 0},
            'out': {'total': 0, 'secondary': 0, 'remapped': 0},
            'inside': {'total': 0, 'secondary': 0, 'remapped': 0}
        }

    def initialize(self):
        """ Does nothing """

    def beginRun(self):
        """ Does nothing """

    def event(self):
        """
        List VXD TrueHits, the MCParticles that generated them, related
        SimHits, and check the reconstruction of the mid-point parameters.
        """

        pxd_truehits = Belle2.PyStoreArray('PXDTrueHits')
        svd_truehits = Belle2.PyStoreArray('SVDTrueHits')
        geocache = Belle2.VXD.GeoCache.getInstance()

        for truehit in pxd_truehits:
            # Get the VXD and sensor thickness
            id = truehit.getSensorID()
            layer = id.getLayerNumber()
            ladder = id.getLadderNumber()
            sensor = id.getSensorNumber()
            sensor_info = geocache.get(id)
            thickness = sensor_info.getThickness()
            base_info = \
                '\nPXDTrueHit {index}: layer:{ly} ladder:{ld} sensor:{sr}'\
                .format(index=truehit.getArrayIndex(),
                        ly=layer, ld=ladder, sr=sensor)
            # Classify the TrueHit
            into_type = \
                abs(abs(truehit.getEntryW()) - thickness / 2.0) < 1.0e-6
            out_type = \
                abs(abs(truehit.getExitW()) - thickness / 2.0) < 1.0e-6
            through_type = into_type and out_type
            truehit_type_text = 'inside'  # should not create TrueHit
            if through_type:
                truehit_type_text = 'through'
            elif into_type:
                truehit_type_text = 'into'
            elif out_type:
                truehit_type_text = 'out'
            base_info = '\n' + 'TrueHit type: ' + truehit_type_text
            # Get the generating MCParticle and check sign of relation weight
            mcparticle_relations = truehit.getRelationsFrom('MCParticles')
            if mcparticle_relations.size() == 0:
                b2.B2INFO('Found PXDTrueHit w/o relation to MCParticles')
                continue
            weight = mcparticle_relations.weight(0)
            particle = mcparticle_relations[0]
            particle_type_text = 'secondary'
            if particle.hasStatus(Belle2.MCParticle.c_PrimaryParticle):
                particle_type_text = 'primary'
            mcparticle_message = 'MCParticle {index}: {text}'.format(
                index=particle.getArrayIndex(),
                text=particle_type_text + ' ')
            self.truehit_stats_pxd[truehit_type_text]['total'] += 1
            if particle_type_text == 'secondary':
                self.truehit_stats_pxd[truehit_type_text]['secondary'] += 1
            if weight < 0:
                self.truehit_stats_pxd[truehit_type_text]['remapped'] += 1
                mcparticle_message += 'remapped ' + str(weight)
            else:
                continue
            b2.B2INFO(base_info + '\n' + mcparticle_message)
            # Now get and print out the SimHits
            simhits = truehit.getRelationsTo('PXDSimHits')
            b2.B2INFO('SimHits:')
            for simhit in simhits:
                particle_type_text = 'secondary'
                simhit_weight = 0.0
                particle_index = -1
                #: get the sign of relation to MCParticle
                simhit_relations = simhit.getRelationsFrom('MCParticles')
                if simhit_relations.size() == 0:
                    particle_type_text = 'none'
                else:
                    simhit_weight = simhit_relations.weight(0)
                    particle = simhit_relations[0]
                    particle_index = particle.getArrayIndex()
                    if particle.hasStatus(Belle2.MCParticle.c_PrimaryParticle):
                        particle_type_text = 'primary'
                simhit_text = \
                    '{index} {ptext} {pindex} {wt:9.6f}'.format(
                        index=simhit.getArrayIndex(),
                        ptext=particle_type_text,
                        pindex=particle_index,
                        wt=simhit_weight)
                simhit_text += \
                    ' {u_in:8.4f} {v_in:8.4f} {w_in:8.4f}'.format(
                        u_in=simhit.getPosIn().X(),
                        v_in=simhit.getPosIn().Y(),
                        w_in=simhit.getPosIn().Z())
                simhit_text += \
                    '{time:8.5f}'.format(time=simhit.getGlobalTime())
                b2.B2INFO(simhit_text)
            # Print the truhit position
            midpoint_text = 'TrueHit position:\n'
            midpoint_text += \
                'IN: ({u_in:8.4f},{v_in:8.4f},{w_in:8.4f})\n'.format(
                    u_in=truehit.getEntryU(),
                    v_in=truehit.getEntryV(),
                    w_in=truehit.getEntryW())
            midpoint_text += \
                'MID: ({u_mid:8.4f},{v_mid:8.4f},{w_mid:8.4f})\n'.format(
                    u_mid=truehit.getU(),
                    v_mid=truehit.getV(),
                    w_mid=truehit.getW())
            midpoint_text += \
                'OUT: ({u_out:8.4f},{v_out:8.4f},{w_out:8.4f})\n'.format(
                    u_out=truehit.getExitU(),
                    v_out=truehit.getExitV(),
                    w_out=truehit.getExitW())
            # Print the truhit momenta
            midpoint_text += '\nTrueHit momentum:\n'
            midpoint_text += \
                'IN: ({pu_in:8.4f},{pv_in:8.4f},{pw_in:8.4f})\n'.format(
                    pu_in=truehit.getEntryMomentum().X(),
                    pv_in=truehit.getEntryMomentum().Y(),
                    pw_in=truehit.getEntryMomentum().Z())
            midpoint_text += \
                'MID: ({pu_mid:8.4f},{pv_mid:8.4f},{pw_mid:8.4f})\n'.format(
                    pu_mid=truehit.getMomentum().X(),
                    pv_mid=truehit.getMomentum().Y(),
                    pw_mid=truehit.getMomentum().Z())
            midpoint_text += \
                'OUT: ({pu_out:8.4f},{pv_out:8.4f},{pw_out:8.4f})\n'.format(
                    pu_out=truehit.getExitMomentum().X(),
                    pv_out=truehit.getExitMomentum().Y(),
                    pw_out=truehit.getExitMomentum().Z())
            # Print the truhit time
            midpoint_text += 'Time: {t:8.5f}\n'.format(
                t=truehit.getGlobalTime())
            b2.B2INFO(midpoint_text)

        for truehit in svd_truehits:
            # Get the VXD and sensor thickness
            id = truehit.getSensorID()
            layer = id.getLayerNumber()
            ladder = id.getLadderNumber()
            sensor = id.getSensorNumber()
            sensor_info = geocache.get(id)
            thickness = sensor_info.getThickness()
            base_info = \
                '\nSVDTrueHit {index}: layer:{ly} ladder:{ld} sensor:{sr}'\
                .format(index=truehit.getArrayIndex(),
                        ly=layer, ld=ladder, sr=sensor)
            # Classify the TrueHit
            into_type = \
                abs(abs(truehit.getEntryW()) - thickness / 2.0) < 1.0e-6
            out_type = \
                abs(abs(truehit.getExitW()) - thickness / 2.0) < 1.0e-6
            through_type = into_type and out_type
            truehit_type_text = 'inside'  # should not create TrueHit
            if through_type:
                truehit_type_text = 'through'
            elif into_type:
                truehit_type_text = 'into'
            elif out_type:
                truehit_type_text = 'out'
            base_info += '\n' + 'TrueHit type: ' + truehit_type_text
            # Get the generating MCParticle and check sign of relation weight
            mcparticle_relations = truehit.getRelationsFrom('MCParticles')
            if mcparticle_relations.size() == 0:
                b2.B2INFO('Found SVDTrueHit w/o relation to MCParticles')
                continue
            weight = mcparticle_relations.weight(0)
            particle = mcparticle_relations[0]
            particle_type_text = 'secondary'
            if particle.hasStatus(Belle2.MCParticle.c_PrimaryParticle):
                particle_type_text = 'primary'
            mcparticle_message = 'MCParticle {index}: {text}'.format(
                index=particle.getArrayIndex(),
                text=particle_type_text + ' ')
            self.truehit_stats_svd[truehit_type_text]['total'] += 1
            if particle_type_text == 'secondary':
                self.truehit_stats_svd[truehit_type_text]['secondary'] += 1
            if weight < 0:
                self.truehit_stats_svd[truehit_type_text]['remapped'] += 1
                mcparticle_message += 'remapped ' + str(weight)
            else:
                continue
            b2.B2INFO(base_info + '\n' + mcparticle_message)
            # Now get and print out the SimHits
            simhits = truehit.getRelationsTo('SVDSimHits')
            b2.B2INFO('SimHits:')
            for simhit in simhits:
                particle_type_text = 'secondary'
                simhit_weight = 0.0
                particle_index = -1
                #: get the sign of relation to MCParticle
                simhit_relations = simhit.getRelationsFrom('MCParticles')
                if simhit_relations.size() == 0:
                    particle_type_text = 'none'
                else:
                    simhit_weight = simhit_relations.weight(0)
                    particle = simhit_relations[0]
                    particle_index = particle.getArrayIndex()
                    if particle.hasStatus(Belle2.MCParticle.c_PrimaryParticle):
                        particle_type_text = 'primary'
                simhit_text = \
                    '{index} {ptext} {pindex} {wt:9.6f}'.format(
                        index=simhit.getArrayIndex(),
                        ptext=particle_type_text,
                        pindex=particle_index,
                        wt=simhit_weight)
                simhit_text += \
                    ' {u_in:8.4f} {v_in:8.4f} {w_in:8.4f}'.format(
                        u_in=simhit.getPosIn().X(),
                        v_in=simhit.getPosIn().Y(),
                        w_in=simhit.getPosIn().Z())
                simhit_text += \
                    '{time:8.5f} '.format(time=simhit.getGlobalTime())
                b2.B2INFO(simhit_text)
            # Print the truhit position
            midpoint_text = 'TrueHit position:\n'
            midpoint_text += \
                'IN: ({u_in:8.4f},{v_in:8.4f},{w_in:8.4f})\n'.format(
                    u_in=truehit.getEntryU(),
                    v_in=truehit.getEntryV(),
                    w_in=truehit.getEntryW())
            midpoint_text += \
                'MID: ({u_mid:8.4f},{v_mid:8.4f},{w_mid:8.4f})\n'.format(
                    u_mid=truehit.getU(),
                    v_mid=truehit.getV(),
                    w_mid=truehit.getW())
            midpoint_text += \
                'OUT: ({u_out:8.4f},{v_out:8.4f},{w_out:8.4f})\n'.format(
                    u_out=truehit.getExitU(),
                    v_out=truehit.getExitV(),
                    w_out=truehit.getExitW())
            # Print the truhit momenta
            midpoint_text += '\nTrueHit momentum:\n'
            midpoint_text += \
                'IN: ({pu_in:8.4f},{pv_in:8.4f},{pw_in:8.4f})\n'.format(
                    pu_in=truehit.getEntryMomentum().X(),
                    pv_in=truehit.getEntryMomentum().Y(),
                    pw_in=truehit.getEntryMomentum().Z())
            midpoint_text += \
                'MID: ({pu_mid:8.4f},{pv_mid:8.4f},{pw_mid:8.4f})\n'.format(
                    pu_mid=truehit.getMomentum().X(),
                    pv_mid=truehit.getMomentum().Y(),
                    pw_mid=truehit.getMomentum().Z())
            midpoint_text += \
                'OUT: ({pu_out:8.4f},{pv_out:8.4f},{pw_out:8.4f})\n'.format(
                    pu_out=truehit.getExitMomentum().X(),
                    pv_out=truehit.getExitMomentum().Y(),
                    pw_out=truehit.getExitMomentum().Z())
            # Print the truhit time
            midpoint_text += 'Time: {t:8.5f}\n'.format(
                t=truehit.getGlobalTime())
            b2.B2INFO(midpoint_text)

    def terminate(self):
        """ Write results """
        b2.B2INFO(
            '\nStatistics for PXD: {pxd};\nStatistics for SVD: {svd}\n'.format(
                pxd=str(self.truehit_stats_pxd),
                svd=str(self.truehit_stats_svd)
            )
        )


# Particle gun module
particlegun = b2.register_module('ParticleGun')
# Create Event information
eventinfosetter = b2.register_module('EventInfoSetter')
# Show progress of processing
progress = b2.register_module('Progress')
# Load parameters
gearbox = b2.register_module('Gearbox')
# Create geometry
geometry = b2.register_module('Geometry')
# Run simulation
simulation = b2.register_module('FullSim')
# simulation.param('StoreAllSecondaries', True)
# PXD digitization module
printParticles = CheckTrueHits()
printParticles.set_log_level(b2.LogLevel.INFO)

# Specify number of events to generate
eventinfosetter.param({'evtNumList': [200], 'runList': [1]})

# Set parameters for particlegun
particlegun.param({
    'nTracks': 1,
    'varyNTracks': True,
    'pdgCodes': [211, -211, 11, -11],
    'momentumGeneration': 'normalPt',
    'momentumParams': [2, 1],
    'phiGeneration': 'normal',
    'phiParams': [0, 360],
    'thetaGeneration': 'uniformCos',
    'thetaParams': [17, 150],
    'vertexGeneration': 'normal',
    'xVertexParams': [0, 1],
    'yVertexParams': [0, 1],
    'zVertexParams': [0, 1],
    'independentVertices': False,
})


# create processing path
main = b2.create_path()
main.add_module(eventinfosetter)
main.add_module(progress)
main.add_module(particlegun)
main.add_module(gearbox)
main.add_module(geometry)
main.add_module(simulation)
main.add_module(printParticles)

# generate events
b2.process(main)

# show call statistics
print(b2.statistics)
