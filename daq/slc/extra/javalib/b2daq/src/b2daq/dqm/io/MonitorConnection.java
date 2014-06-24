package b2daq.dqm.io;

public class MonitorConnection extends MonitorEnum {

	public static final MonitorConnection UNKNOWN = new MonitorConnection(-1, "UNKNOWN");
	public static final MonitorConnection ON = new MonitorConnection(1, "ON");
	public static final MonitorConnection OFF = new MonitorConnection(0, "OFF");;

	private MonitorConnection(int value, String text) {
		_value = value;
		_text = text;
	}

	public MonitorConnection() {

	}

	public void setValue(int value) {
		if (value == OFF.getValue()) {
			_value = value;
			_text = OFF.getText();
		} else if (value == ON.getValue()) {
			_value = value;
			_text = ON.getText();
		} else {
			_value = UNKNOWN.getValue();
			_text = UNKNOWN.getText();
		}
	}

}
