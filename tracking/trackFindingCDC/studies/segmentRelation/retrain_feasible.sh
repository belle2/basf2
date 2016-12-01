SAMPLE=../samples/generic_const_b_10000.root

# Train feasibility filter.
basf2 record.py -- -d TrackingDetectorConstB -i $SAMPLE  --task train_feasible
trackfindingcdc_teacher "train_feasible_generic_const_b_10000.root" --truth=truth_positive --identifier FeasibleSegmentRelationFilter.weights.xml
mv FeasibleSegmentRelationFilter.weights.xml $BELLE2_LOCAL_DIR/tracking/data/FeasibleSegmentRelationFilter.weights.xml

# Control plots for feasible filter
basf2 record.py -- -d TrackingDetectorConstB -i $SAMPLE --task eval_feasible
trackfindingcdc-classification-overview --truth=truth_positive -s "eval_feasible_generic_const_b_10000.root"