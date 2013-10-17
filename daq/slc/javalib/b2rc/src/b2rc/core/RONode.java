package b2rc.core;

public class RONode extends RCNode {

	public static String TAG = "ro_node";

	public String[] _sender_v = new String[20];
	public int _sender_i = 0;
	public String _script;

	public String getScript() {
		return _script;
	}

	public void setScript(String script) {
		_script = script;
	}

	public void addSender(String sender) {
		if (_sender_i < _sender_v.length) {
			_sender_v[_sender_i] = sender;
			_sender_i++;
		}
	}

	public void clearSenders() {
		_sender_v = new String[20];
		_sender_i = 0;
	}

	public String getSender(int i) {
		return _sender_v[i];
	}

	public String[] getSenders() {
		return _sender_v;
	}

	public void setSender(int i, String sender) {
		_sender_v[i] = sender;
	}

	@Override
	public String getSQLFields() {
		StringBuffer buf = new StringBuffer();
		buf.append(super.getSQLFields());
		buf.append(", script text");
		for (int i = 0; i < _sender_v.length; i++) {
			buf.append(", sender_id_" + i + " smallint");
		}
		return buf.toString();
	}

	@Override
	public String getSQLLabels() {
		StringBuffer buf = new StringBuffer();
		buf.append(super.getSQLLabels());
		buf.append(", script");
		for (int i = 0; i < _sender_v.length; i++) {
			buf.append(", sender_id_" + i);
		}
		return buf.toString();
	}

	@Override
	public String getSQLValues() {
		StringBuffer buf = new StringBuffer();
		buf.append(super.getSQLValues());
		buf.append(", '"+_script+"'");
		for (int i = 0; i < _sender_v.length; i++) {
			if (_sender_v[i] != null) {
				buf.append(", '" + _sender_v[i] + "'");
			} else {
				buf.append(", ''");
			}
		}
		return buf.toString();
	}

	@Override
	public String getTag() {
		return TAG;
	}

}
