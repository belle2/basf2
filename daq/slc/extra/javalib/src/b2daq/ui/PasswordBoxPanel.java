package b2daq.ui;

import java.awt.Dimension;

import javax.swing.Box;
import javax.swing.BoxLayout;
import javax.swing.JLabel;
import javax.swing.JPanel;
import javax.swing.JPasswordField;

public class PasswordBoxPanel extends JPanel {

	private static final long serialVersionUID = 1L;
	private JLabel _label;
	private JPasswordField _text_field;

	public PasswordBoxPanel(String title, String text) {
		super();
		setMaximumSize(new Dimension(300, 30));
		setLayout(new BoxLayout(this, BoxLayout.X_AXIS));
		_label = new JLabel(title);
		_label.setMaximumSize(new Dimension(90, 30));
		add(Box.createRigidArea(new Dimension(10, 30)));
		add(_label);
		_text_field = new JPasswordField(text);
		_text_field.setMaximumSize(new Dimension(150, 30));
		_text_field.setPreferredSize(new Dimension(150, 30));
		add(Box.createRigidArea(new Dimension(10, 30)));
		add(_text_field);
		add(Box.createRigidArea(new Dimension(10, 30)));
	}

	public String getText() {
		return new String(_text_field.getPassword());
	}

	public void setEnabled(boolean enabled) {
		_text_field.setEnabled(enabled);
	}

}
