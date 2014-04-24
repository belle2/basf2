package b2dqm.io;

public class UpdatableMessage {

	private String _title = "";
	private String _text = "";

	public UpdatableMessage() {
		
	}

	public UpdatableMessage(String title) {
		_title = title;
	}
	
	public String getText() {
		return _text;
	}

	public void setText(String text) {
		_text = text;
	}

	public String getTitle() {
		return _title;
	}

	public void setTitle(String title) {
		_title = title;
	}

	public String getMessage() {
		StringBuffer ss = new StringBuffer();
		//ss.append(_title);
		ss.append(_text);
		return ss.toString();
	}

}
