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

    static public void main(String[] argv) {
        System.out.println(NumberString.toString(1.5280956416000001E9, 10));
        System.out.println(NumberString.toString(1510.0e-1, 0));
        System.out.println(NumberString.toString(0.5100, 1));
        System.out.println(NumberString.toString(0.000005100, 5));
        System.out.println(NumberString.toString(-510, 0));
        System.out.println(NumberString.toString(0, 5));
    }
}
