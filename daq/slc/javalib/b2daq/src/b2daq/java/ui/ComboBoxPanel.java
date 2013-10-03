package b2daq.java.ui;

import java.awt.Dimension;

import javax.swing.Box;
import javax.swing.BoxLayout;
import javax.swing.JComboBox;
import javax.swing.JLabel;
import javax.swing.JPanel;
import javax.swing.JTextField;

public class ComboBoxPanel extends JPanel {

	private static final long serialVersionUID = 1L;
	private JLabel _label;
	private JComboBox _text_combo;

	public ComboBoxPanel(String title, String [] text_v) {
		super();
		setMaximumSize(new Dimension(300, 30));
		setLayout(new BoxLayout(this, BoxLayout.X_AXIS));
		_label = new JLabel(title);
		_label.setMaximumSize(new Dimension(90, 30));
		add(Box.createRigidArea(new Dimension(10, 30)));
		add(_label);
		_text_combo = new JComboBox(text_v);
		_text_combo.setMaximumSize(new Dimension(150, 30));
		_text_combo.setPreferredSize(new Dimension(150, 30));
		add(Box.createRigidArea(new Dimension(10, 30)));
		add(_text_combo);
		add(Box.createRigidArea(new Dimension(10, 30)));
	}

	public String getText() {
		return _text_combo.getSelectedItem().toString();
	}

	public int getSelected() {
		return _text_combo.getSelectedIndex();
	}
	
	public void setFieldSize(int width, int height) {
		_text_combo.setMaximumSize(new Dimension(width, height));
		_text_combo.setPreferredSize(new Dimension(width, height));
	}
	
	public void setLabelSize(int width, int height) {
		_label.setMaximumSize(new Dimension(width, height));
		_label.setPreferredSize(new Dimension(width, height));
	}
	
	public void setEnabled(boolean enabled) {
		_text_combo.setEnabled(enabled);
	}

	public JComboBox getCombo() {
		return _text_combo;
	}

}
