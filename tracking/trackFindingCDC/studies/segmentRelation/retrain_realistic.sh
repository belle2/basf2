SAMPLE=../samples/generic_const_b_10000.root

# Train feasibility filter.
basf2 record.py -- -d TrackingDetectorConstB -i $SAMPLE  --task train_realistic
trackfindingcdc_teacher "train_realistic_generic_const_b_10000.root" --truth=truth_positive --identifier RealisticSegmentRelationFilter.weights.xml
mv RealisticSegmentRelationFilter.weights.xml $BELLE2_LOCAL_DIR/tracking/data/RealisticSegmentRelationFilter.weights.xml

# Control plots for realistic filter
basf2 record.py -- -d TrackingDetectorConstB -i $SAMPLE --task eval_realistic
trackfindingcdc-classification-overview --truth=truth_positive -s "eval_realistic_generic_const_b_10000.root"