package b2daq.core;

import java.io.IOException;
import java.util.Date;

public class Time implements Serializable {

	private long _s;
	private long _us;

	public Time() {
		set();
	}

	public Time(int time) {
		set((long)time*1000, 0);
	}

	public Time(long s, long us) {
		_s = s;
		_us = us;
		adjust();
	}

	public void clear() {
		_s = _us = 0;
	}

	public void set() {
		long ns = System.nanoTime();
		_s = new Date().getTime();// (long) ns / 1000000000;
		_us = (long) (ns % 1000000000) / 1000;
	}

	public void set(double time) {
		_s = (long) time;
		_us = (long) ((time % 1.) * 1000000);
		adjust();
	}

	public void set(long s, long us) {
		_s = s;
		_us = us;
		adjust();
	}

	public void set(String msg) {
		String[] str_v = msg.split(".");
		if (str_v.length != 2) return;
		_s = Long.parseLong(str_v[0]);
		_us = Long.parseLong(str_v[1]);
	}

	public double get() {
		return (double) _s + ((double) _us) / 1000000.;
	}

	public long getSecond() {
		return _s;
	}

	public long getMicroSecond() {
		return _us;
	}

	public void readObject(Reader reader) throws IOException {
		_s = reader.readLong() * 1000;
		_us = reader.readLong();
	}

	public void writeObject(Writer writer) throws IOException {
		writer.writeLong(_s / 1000);
		writer.writeLong(_us);
	}

	public String toString() {
		return "" + _s + "." + convert(6, _us);
	}

	private void adjust() {
		if (1000000 <= Math.abs(_us)) {
			_s += _us / 1000000;
			_us = _us - (_us / 1000000) * 1000000;
		}
		if (0 < _s && _us < 0) {
			_s--;
			_us += 1000000;
		}
		if (_s < 0 && 0 < _us) {
			_s++;
			_us -= 1000000;
		}
	}

	private String convert(int order, long s) {
		String nstr = "" + s;
		String str = "";
		for (int n = 0; n < order - nstr.length(); n++) {
			str += "0";
		}
		return str + "" + nstr;
	}

	@SuppressWarnings("deprecation")
	public String toDateString() {
		Date date = new Date(getSecond());
		String text = "" + (1900 + date.getYear()) + "/";
		if (date.getMonth() + 1 < 10) text += "0";
		text += (date.getMonth() + 1) + "/";
		if (date.getDate() < 10)	text += "0";
		text += date.getDate() + " ";
		if (date.getHours() < 10) text += "0";
		text += date.getHours() + ":";
		if (date.getMinutes() < 10)	text += "0";
		text += date.getMinutes();
		return text;
	}

	@SuppressWarnings("deprecation")
	public String toTimeString() {
		Date date = new Date(getSecond());
		String text = "";
		if (date.getHours() < 10) text += "0";
		text += date.getHours() + ":";
		if (date.getMinutes() < 10)	text += "0";
		text += date.getMinutes();
		return text;
	}
}
