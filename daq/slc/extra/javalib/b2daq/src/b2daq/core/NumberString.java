package b2daq.core;

public class NumberString {

    public static String toString(double value, int ns) {
        double v = value;
        if (v == 0) {
            return "0";
        }
        if (v < 0) {
            v *= -1;
        }
        int nr = 0;
        while (v < 0.1) {
            v *= 10;
            nr++;
        }
        if (v >= 10000) {
            return String.format("%s%." + ns + "e", (value < 0) ? "-1" : "", v);
        } else if (v <= 0.01) {
            return String.format("%s%." + ns + "e", (value < 0) ? "-1" : "", v);
        } else {
            return String.format("%s%." + ns + "f", (value < 0) ? "-1" : "", v);
        }
    }

}
