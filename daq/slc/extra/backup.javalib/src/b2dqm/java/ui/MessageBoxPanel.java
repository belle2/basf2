package b2dqm.java.ui;

import java.util.ArrayList;

import b2daq.java.ui.MessageViewPanel;
import b2daq.ui.Updatable;
import b2dqm.io.UpdatableMessage;

public class MessageBoxPanel extends MessageViewPanel implements Updatable {

	private static final long serialVersionUID = 1L;
	private ArrayList<UpdatableMessage> _text_v = new ArrayList<UpdatableMessage>();
	private String _header = "";
	private String _footer = "";
	
	public MessageBoxPanel() {
		super(13);
	}
	
	public MessageBoxPanel(int font_size) {
		super(font_size);
	}
	
	public void add(UpdatableMessage text) {
		_text_v.add(text);
	}
	
	public void update() {
		reset();
		add(_header);
		for (UpdatableMessage text : _text_v) {
			add(text.getMessage());
		}
		add(_footer);
		super.update();
	}

	public String getHeader() {
		return _header;
	}

	public void setHeader(String header) {
		_header = header;
	}

	public String getFooter() {
		return _footer;
	}

	public void setFooter(String footer) {
		_footer = footer;
	}

}
