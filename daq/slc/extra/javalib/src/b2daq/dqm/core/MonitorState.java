package b2daq.dqm.core;

public class MonitorState extends MonitorEnum {

	public static final MonitorState UNKNOWN = new MonitorState(-1, "UNKNOWN");
	public static final MonitorState NOTUSED = new MonitorState(0, "NOTUSED");
	public static final MonitorState BOOTED = new MonitorState(1, "BOOTED");
	public static final MonitorState READY = new MonitorState(2, "READY");
	public static final MonitorState RUNNING = new MonitorState(3, "RUNNING");
	public static final MonitorState EXCEPTION = new MonitorState(101, "EXCEPTION");

	private MonitorState(int value, String text) {
		_value = value;
		_text = text;
	}

	public MonitorState() {

	}

	public void setValue(int value) {
		if (value == NOTUSED.getValue()) {
			_value = value;
			_text = NOTUSED.getText();
		} else if (value == BOOTED.getValue()) {
			_value = value;
			_text = BOOTED.getText();
		} else if (value == READY.getValue()) {
			_value = value;
			_text = READY.getText();
		} else if (value == RUNNING.getValue()) {
			_value = value;
			_text = RUNNING.getText();
		} else if (value == EXCEPTION.getValue()) {
			_value = value;
			_text = EXCEPTION.getText();
		} else {
			_value = UNKNOWN.getValue();
			_text = UNKNOWN.getText();
		}
	}

}
