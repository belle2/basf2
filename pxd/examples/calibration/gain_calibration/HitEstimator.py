from shape_utils import *
from ROOT import TH2F


class AngularGrid(object):
    """2D histogram for binning incidence angles thetau,thetaV of tracks into a planar sensor."""

    def __init__(self, uBins=18, uMin=-90.0, uMax=+90.0, vBins=18, vMin=-90.0, vMax=+90.0, name='h2angles'):
        self.h2grid = TH2F(name, '', uBins, uMin, uMax, vBins, vMin, vMax)
        self.uBins = uBins
        self.vBins = vBins

        for uBin in range(1, uBins + 1):
            for vBin in range(1, vBins + 1):
                self.h2grid.SetBinContent(uBin, vBin, -1)

    def getUBins(self):
        return self.uBins

    def getVBins(self):
        return self.vBins

    def findBin(self, uAngle, vAngle):
        uBin = self.h2grid.GetXaxis().FindBin(uAngle)
        vBin = self.h2grid.GetYaxis().FindBin(vAngle)
        return uBin, vBin

    def isOutOfRange(self, uBin, vBin):
        return (uBin < 1) or (uBin > self.uBins) or (vBin < 1) or (vBin > self.vBins)

    def getCenter(self, uBin, vBin):
        uCenter = self.h2grid.GetXaxis().GetBinCenter(uBin)
        vCenter = self.h2grid.GetYaxis().GetBinCenter(vBin)
        return uCenter, vCenter

    def setGridContent(self, uBin, vBin, index):
        self.h2grid.SetBinContent(uBin, vBin, int(index))

    def getGridContent(self, uBin, vBin):
        return int(self.h2grid.GetBinContent(uBin, vBin))


class HitEstimator(object):
    """
    The HitEstimator class provides the user interface for track dependent
    and cluster-shape aware hit estimation.
    """

    def __init__(self, grids):
        self.grids = grids
        self.shape_classifiers = {}

        # Initialize shape_classifiers
        for pixelkind in self.getPixelkinds():
            self.shape_classifiers[pixelkind] = []
            for uBin in range(1, self.grids[pixelkind].getUBins() + 1):
                for vBin in range(1, self.grids[pixelkind].getVBins() + 1):
                    self.grids[pixelkind].setGridContent(uBin, vBin, len(self.shape_classifiers[pixelkind]))
                    self.shape_classifiers[pixelkind].append(ShapeClassifier())

    def getPixelkinds(self):
        """ Returns list of pixelkinds"""
        return self.grids.keys()

    def getGrid(self, pixelkind):
        """Returns AngularGrid"""
        return self.grids[pixelkind]

    def setShapeClassifier(self, shape_classifier, uBin, vBin, pixelkind):
        """Sets shape classifier for angular bin and pixelkind"""
        if pixelkind in self.grids.keys():
            if not self.grids[pixelkind].isOutOfRange(uBin, vBin):
                key = self.grids[pixelkind].getGridContent(uBin, vBin)
                self.shape_classifiers[pixelkind][key] = shape_classifier

    def getShapeClassifier(self, uBin, vBin, pixelkind):
        """Returns shape classifier for angular bin and pixelkind"""
        key = self.grids[pixelkind].getGridContent(uBin, vBin)
        return self.shape_classifiers[pixelkind][key]

    def getShapeClassifierFromAngles(self, thetaU, thetaV, pixelkind):
        """Returns shape classifier for incidence angles and pixelkind"""
        uBin, vBin = self.grids[pixelkind].findBin(thetaU, thetaV)
        key = self.grids[pixelkind].getGridContent(uBin, vBin)
        return self.shape_classifiers[pixelkind][key]

    def hasHit(self, shape, thetaU, thetaV, pixelkind):
        """ Returns True if there are valid position corrections available """

        # Check pixelkind is valid
        if pixelkind not in self.grids.keys():
            return False

        # Check thetaU, thetaV are inside grid
        uBin, vBin = self.grids[pixelkind].findBin(thetaU, thetaV)
        if self.grids[pixelkind].isOutOfRange(uBin, vBin):
            return False

        # Check there is a calibrated label
        shape_classifier = self.getShapeClassifier(uBin, vBin, pixelkind)
        label = shape_classifier.getLabel(shape, thetaU, thetaV)
        if not shape_classifier.hasHit(label):
            return False

        return True

    def getHit(self, shape, thetaU, thetaV, pixelkind):
        """
        Returns hit data for label. Hit is implemented as tuple (offset, cov, prob)
        offset: position corrrection for shape, relative to center of pixel (uStart,vStart) in cluster
        cov   : covariance matrix for offsets
        prob  : probability for observing shape, relative to training data for shape classifier
        """
        shape_classifier = self.getShapeClassifierFromAngles(thetaU, thetaV, pixelkind)
        label = shape_classifier.getLabel(shape, thetaU, thetaV)
        return shape_classifier.getHit(label)


class ShapeClassifier(object):
    """
    Mother class for all shape classifiers. A shape
    classifier assigns a label to each shape and
    provides hits for all labels.

    The mother class implements an user interface for
    an empty shape classifer.
    """

    def __init__(self):
        self.hits_dict = {}

    def getLabel(self, shape, thetaU, thetaV):
        """Returns label for shape given incidence angles """
        return None

    def getHit(self, label):
        """Returns hit for label """
        return None

    def getLabels(self):
        """Returns list of labels """
        return []

    def isEmpty(self):
        """ Returns True for empty hit dictionary"""
        return (len(self.hits_dict) == 0)

    def getCells(self, label):
        """Returns ucells and vcells for label """
        return [], []


class ShapeClassifierTrainer(object):
    """
    Mother class for all algorithms for training
    shape classifiers on training data.

    The mother class implements an user interface for
    training an empty shape classifer.
    """

    def __init__(self):
        pass

    def createShapeClassifier(self, SortedData, thetaU, thetaV):
        return ShapeClassifier()

    def mirrorShapeClassifier(self, shape_classifier, pixelkind):
        return ShapeClassifier()

    def rankHits(self, hits):
        flat_covs = []
        weights = []

        for label in hits.keys():
            offsets, cov, prob = hits[label]
            flat_covs.append([cov[0, 0], cov[1, 1], cov[0, 1]])
            weights.append(prob)

        weights = np.array(weights)
        flat_covs = np.array(flat_covs)
        flat_average = np.average(flat_covs, axis=0, weights=weights)
        return math.sqrt(flat_average[0] + flat_average[1])

    def getCoverage(self, hits):
        """ Returns coverage of hits on training data """
        coverage = 0.0
        for label in hits.keys():
            coverage += hits[label][2]
        return coverage

    def computeHit(self, intersects, alltracks):
        """ Returns hit computed from sorted intesections """

        # Compute offsets
        offsets = np.mean(intersects, axis=1)
        # Compute covariance matrix
        cov = np.cov(intersects)
        # Compute probability
        prob = float(intersects.shape[1]) / alltracks
        return (offsets, cov, prob)
