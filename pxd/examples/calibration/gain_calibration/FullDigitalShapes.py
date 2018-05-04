from shape_utils import *
from HitEstimator import ShapeClassifierTrainer
from HitEstimator import ShapeClassifier
import copy


class FullDigitalShapeClassifierTrainer(ShapeClassifierTrainer):
    """Class for training of FullDigitalShapeClassifier instances on track data"""

    def __init__(self, mintracks=2000, coveragecut=0.99, maxdivisions=5):
        super().__init__()
        # Minimum number of tracks for computing hits
        self.mintracks = mintracks
        # Stop adding hits to shape classifier once cut reached
        self.coveragecut = coveragecut
        # Maximum number hits per digital label
        self.maxdivisions = maxdivisions

    def mirrorShapeClassifier(self, shape_classifier, pixelkind):
        """Returns a mirrored version of shape_classifier of type FullDigitalShapeClassifier"""
        # Create a new shape classifier
        mirrored_classifier = FullDigitalShapeClassifier()

        for dlabel in shape_classifier.getDigitalLabels():
            # Compute the mirrored digital label
            mdlabel = self.mirror_dlabel(dlabel)
            # Compute mapping from signal index in dlabel to index in mdlabel
            index_map = list(self.mirror_signals(mdlabel, list(range(get_size(dlabel)))))
            # Compute the mirrored indexset
            mirrored_headtail = []
            for index in shape_classifier.getHeadTailIndices(dlabel):
                mirrored_headtail.append(index_map[index])
            # Compute the mirrored percentiles
            mirrored_percentiles = copy.deepcopy(shape_classifier.getPercentiles(dlabel))
            # Compute the mirrored hitmap
            mirrored_hitmap = {}
            hitmap = shape_classifier.getHitMap(dlabel)
            for index in hitmap:
                # Make copy of hit
                offset, cov, prob = copy.deepcopy(hitmap[index])
                # Mirroring the shape implies that shape origin shifts
                shift = (get_vsize(dlabel) - 1) * getPitchV(pixelkind)
                # Compute the new offsets
                offset[1] = shift - offset[1]
                # Compute the new covariance matrix
                cov[0, 1] *= -1.0
                cov[1, 0] *= -1.0
                # Store the result
                mirrored_hitmap[index] = (offset, cov, prob)

            # Store the results
            mirrored_classifier.setPercentiles(mdlabel, mirrored_percentiles)
            mirrored_classifier.setHitMap(mdlabel, mirrored_hitmap)
            mirrored_classifier.setHeadTailIndices(mdlabel, mirrored_headtail[0], mirrored_headtail[1])
        return mirrored_classifier

    def mirror_dlabel(self, dlabel):
        # Mirror the vcells
        vcells = get_vcells(dlabel)
        for i in range(len(vcells)):
            vcells[i] *= -1
        vmin = min(vcells)
        ndigits = get_size(dlabel)

        # Sort the mirrored digit list
        mdigits = zip(vcells, get_ucells(dlabel))
        mdigits_new = sorted(mdigits, key=lambda mdigit: (mdigit[0], mdigit[1]))

        # Construct mirrored shape string
        mdlabel = get_label_type(dlabel)
        for i, mdigit in enumerate(mdigits_new):
            mdlabel += 'D' + str(mdigit[0] - vmin) + '.' + str(mdigit[1])

        return mdlabel

    def mirror_signals(self, dlabel, signals):
        # Mirror the vcells
        vcells = get_vcells(dlabel)
        for i in range(len(vcells)):
            vcells[i] *= -1
        vmin = min(vcells)
        ndigits = get_size(dlabel)

        # Sort the mirrored digit list
        mdigits = zip(vcells, get_ucells(dlabel), signals)
        mdigits_new = sorted(mdigits, key=lambda mdigit: (mdigit[0], mdigit[1]))

        # Construct mirrored shape string
        mshape = 'E'
        for i, mdigit in enumerate(mdigits_new):
            mshape += 'D' + str(mdigit[0] - vmin) + '.' + str(mdigit[1]) + '.' + str(mdigit[2])

        return get_signals(mshape)

    def createShapeClassifier(self, Data, thetaU, thetaV):
        """Returns a new FullDigitalShapeClassifier trained on data from tracks in angle bin at theatU and thetaV"""

        # Instantiate a new classifier
        shape_classifier = FullDigitalShapeClassifier()

        # Total number of tracks for training
        alltracks = Data.shape[0]

        # Missing training data, return empty shape classifier
        if alltracks == 0:
            print('Added fulldigital shape classifier with coverage {:.2f}% on training data sample.'.format(0.0))
            return shape_classifier

        # Compute telescope covariance matrix
        telvaru = np.mean(Data['f9'])
        telvarv = np.mean(Data['f10'])
        telcovuv = np.mean(Data['f11'])
        telcov = np.array([[telvaru, telcovuv], [telcovuv, telvarv]])

        print("Start training shape classifier using {:d} tracks ...".format(alltracks))

        # Reduce clusters to shapes
        shapefunc = np.vectorize(get_shape)
        shapes = shapefunc(Data['f2'])

        # Find unique digital labels in shapes
        dlabelfunc = np.vectorize(get_digital_label)
        dlabels, counts = np.unique(dlabelfunc(shapes), return_counts=True)

        # Sort digital labels to decreasing frequency
        order = counts.argsort()[::-1]
        dlabels = dlabels[order]
        counts = counts[order]
        dprobs = counts.astype(np.float) / alltracks

        # Coverage of classifer on training data
        coverage = 0.0

        # Main loop over digital labels
        for i, dlabel in enumerate(dlabels):

            # Turn dlabel into proper string
            dlabel = str(dlabel)

            # Stop once number tracks to small
            if counts[i] < self.mintracks:
                break

            # Stop once target coverage is reached
            if coverage > self.coveragecut:
                break

            # Update the coverage
            coverage += dprobs[i]

            # Find subset of data for given digital label
            it = np.where(dlabelfunc(shapes) == dlabel)
            DData = Data[it]

            # Now, we try to find a best subdivision of the digital label
            # into smaller labels with improved spatial resolution. This
            # is done by computing a feature from cluster signals and
            # feeding it into a classifier. The classifier is just a
            # binning based on K percentiles computed from the sample of
            # features.

            # Compute percentiles and hitmap
            percentiles, hitmap = self.subdivide_data(DData, dlabel, thetaU, thetaV, telcov, alltracks)

            # Compute score for initial subdivision
            score = self.rankHits(hitmap)
            print('Subdivision for digital label {:} yields score {:.6f}'.format(dlabel, score))

            # Store results in shape classifier
            shape_classifier.setPercentiles(dlabel, percentiles)
            shape_classifier.setHitMap(dlabel, hitmap)
            # Remember entry/exit pixel cells (only needed for plotting)
            entry, exit = get_entry_exit_index(dlabel, thetaU, thetaV)
            shape_classifier.setHeadTailIndices(dlabel, entry, exit)

        print('Added fulldigital shape classifier with coverage {:.2f}% on training data sample.'.format(100 * coverage))

        return shape_classifier

    def subdivide_data(self, DData, dlabel, thetaU, thetaV, telcov, alltracks):
        """Returns hitmap and percentile array from sorted training data """

        # Number of tracks for training
        ntracks = DData.shape[0]

        # F is a feature array.
        F = np.zeros(ntracks, dtype=np.float)

        # This is only temporarily used helper
        tmp_classifier = FullDigitalShapeClassifier()

        for j in range(ntracks):
            # Get shape string
            shape = get_shape(DData[j]['f2'])
            # Compute feature
            F[j] = tmp_classifier.computeFeature(shape, thetaU, thetaV)

        # Train feature classifier
        percentiles, unique_indices, indices = self.train_features(F)

        # Compute array with true track intersections
        intersects = np.vstack((DData['f3'], DData['f4'])).transpose()

        # Dictionary with hits
        hitmap = {}

        for index in unique_indices:
            # Find all intersections of tracks for this label
            label_subset = np.where(indices == index)
            label_intersects = intersects[label_subset, :][0].transpose()

            label_offsets, label_cov, label_prob = self.computeHit(label_intersects, alltracks, telcov)

            # Make sure covariance matrix is positive definite
            if not np.all(np.linalg.eigvals(label_cov) > 0):
                print("WARNING: Estimated covariance matrix not positive definite. Skipping label ", label)
                continue

            hitmap[index] = (label_offsets, label_cov, label_prob)

        return percentiles, hitmap

    def train_features(self, F):
        """Train feature classifier to subdivide features array F
           Returns tuple of percentile array, unique feature indices labels and feature indices
        """

        # Number of features for training
        ntracks = F.shape[0]

        # Maximum number of labels is limited by statistics
        maxlabel = max(int(ntracks / self.mintracks), 1)

        # and limited by the user
        maxlabel = min(maxlabel, self.maxdivisions)

        # Try subdivision of digital label into K smaller labels
        for K in range(maxlabel, 0, -1):

            # Compute array of q values
            qValues = np.arange(0., 100.0, 100.0 / K)

            # Compute percentiles for q values
            percentiles = np.percentile(F, qValues)
            inds = np.digitize(F, percentiles)

            # Count tracks per label
            unique_inds, tracksPerLabel = np.unique(inds, return_counts=True)

            if np.min(tracksPerLabel) >= self.mintracks:
                return percentiles, unique_inds, inds


class FullDigitalShapeClassifier(ShapeClassifier):
    """
    The FullDigitalShapeClassifier classifies shapes taking into account
    the full sequence of digits, called the digital shape.
    All shapes mapped to the same label are guaranteed to have the same
    digital shape.
    """

    def __init__(self):
        super().__init__()
        self.percentile_dict = {}
        self.headtail_dict = {}

    def getHeadTailIndices(self, dlabel):
        return self.headtail_dict[dlabel]

    def setHeadTailIndices(self, dlabel, entry, exit):
        self.headtail_dict[dlabel] = [entry, exit]

    def computeFeature(self, shape, thetaU, thetaV):
        """Returns feature scalar """
        feature = 0.0
        entry, exit = get_entry_exit_index(shape, thetaU, thetaV)

        if not entry == exit:
            # Feature is the eta value from entry/exit signals
            picked_signals = pick_signals(shape, indexset=[entry, exit])
            feature = picked_signals[0] / (picked_signals[0] + picked_signals[1])
        else:
            # Single digit shape. Feature is the digit charge
            picked_signals = pick_signals(shape, indexset=[entry])
            feature = picked_signals[0]

        return feature

    def getLabel(self, shape, thetaU, thetaV):
        """Returns label tuple for shape at given angles """
        dlabel = get_digital_label(shape)
        if dlabel in self.getDigitalLabels():
            # Compute feature
            feature = self.computeFeature(shape, thetaU, thetaV)
            # Compute the index
            index = int(np.digitize(feature, self.getPercentiles(dlabel)))
            # Return label
            return dlabel, index
        else:
            return None

    def getLabelString(self, shape, thetaU, thetaV):
        return str(self.getLabel(shape, thetaU, thetaV))

    def getHit(self, label):
        """Returns hit for label """
        return self.hits_dict[label[0]][label[1]]

    def hasHit(self, label):
        """ Returns True if there are valid position corrections available """
        if label is None:
            return False
        if not label[0] in self.hits_dict:
            return False
        if not label[1] in self.getHitMap(label[0]):
            return False
        return True

    def getLabels(self):
        """Returns list of labels """
        labels = []
        for dlabel in self.getDigitalLabels():
            for index in self.getHitMap(dlabel):
                labels.append((dlabel, index))
        return labels

    def getCells(self, dlabel):
        """Returns ucells and vcells for label """
        return get_ucells(dlabel), get_vcells(dlabel)

    def highlightCell(self, dlabel, index):
        """Flag for highlighting cell in plots"""
        if index in self.getHeadTailIndices(dlabel):
            return True
        else:
            return False

    def getDigitalProb(self, dlabel):
        """ Returns probability of digital label in training sample"""
        prob = 0.0
        for index, hit in self.getHitMap(dlabel).items():
            prob += hit[2]
        return prob

    def setPercentiles(self, dlabel, percentiles):
        """ Set array with percentila bins for digital label"""
        self.percentile_dict[dlabel] = percentiles

    def getPercentiles(self, dlabel):
        """ Returns array with percentila bins for digital label"""
        return self.percentile_dict[dlabel]

    def setHitMap(self, dlabel, hitmap):
        """ Set hits for digital label """
        self.hits_dict[dlabel] = hitmap

    def getHitMap(self, dlabel):
        """Returns  hits for digital labels """
        return self.hits_dict[dlabel]

    def getDigitalLabels(self):
        """Returns list of digital labels in shape classifier"""
        return self.hits_dict.keys()

    def getNumberOfLabels(self, dlabel):
        """Returns number of (sub) labels for given digital label"""
        return self.percentile_dict[dlabel].shape[0]
