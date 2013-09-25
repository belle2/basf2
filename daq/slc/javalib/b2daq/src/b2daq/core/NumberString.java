package b2daq.core;

public class NumberString {

	public static String toString(double value, int ns) {
		double v = value;
		if (v == 0)
			return "0";
		if ( v < 0 )
			v *= -1;
		int nr = 0;
		while ( v < 0.1 ) {
			v *= 10;
			nr++;
		}
		String str = "" + v;

		String[] num = str.split("\\.");
		if ( num.length > 0 ) {
			str += ".0";
			num = str.split("\\.");
		}
		num[1] += "0";
		if ( num[0].charAt(0) == '0' ) {
			int nfloor = 0;
			while ( nfloor < num[1].length() ) {
				if ( num[1].charAt(nfloor) != '0' )
					break;
				nfloor++;
			}
			if ( ns >= 0 ) {
				while ( num[1].length() <= ns + 1 ) {
					num[1] += "0";
				}
				if ( nfloor + nr > 0 ) {
					str = num[1].charAt(nfloor) + "."
							+ num[1].substring(nfloor + 1, nfloor + 1 + ns)
							+ "e-" + (nfloor + nr + 1);
				} else {
					str = "0." + num[1].substring(nfloor, nfloor + 1 + ns);
				}
			} else {
				str = num[1].charAt(nfloor) + "e-" + (nfloor + nr + 1);
			}
		} else {
			if ( num[1].charAt(0) == 'u' ) num[1] = "0";
			while ( num[1].length() <= ns + 1 ) {
				num[1] += "0";
			}
			if ( num[0].length() > 4 && v > 10000.d ) {
				str = "" + num[0].charAt(0);
				if ( ns > 0 ) str += "." + num[0].substring(1, ns + 1);
				str += "e+" + (int)Math.floor(Math.log10(v));
			} else {
				str = num[0];
				if ( ns > 0 ) str += "." + num[1].substring(0, ns);
			}
		}
		if ( value > 0 )
			return str;
		else
			return "-" + str;
	}

	static public void main(String[] argv) {
		System.out.println(NumberString.toString(0.5, 0));
		/*
		System.out.println(NumberString.toString(1510.0, 0));
		System.out.println(NumberString.toString(0.5100, 1));
		System.out.println(NumberString.toString(0.000005100, 5));
		System.out.println(NumberString.toString(-510, 0));
		System.out.println(NumberString.toString(0, 5));
		*/
	}
}
