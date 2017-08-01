# calibration/cg\_compare\_straight\_depths

Compare two sets of [straight depths](../../data/straight_depths.html).

    calibration/cg_compare_straight_depths depths1.json depths2.json [depths.txt]

Compares only the feature straight depths that exist in both `depths1.json` and `depth2.json`. Calculates their root mean square difference. Optionally outputs them together to `depths.txt`.
