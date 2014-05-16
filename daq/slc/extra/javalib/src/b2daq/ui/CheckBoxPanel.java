package b2daq.ui;

import java.awt.Dimension;

import javax.swing.Box;
import javax.swing.BoxLayout;
import javax.swing.JCheckBox;
import javax.swing.JLabel;
import javax.swing.JPanel;

public class CheckBoxPanel extends JPanel {

	private static final long serialVersionUID = 1L;
	private JLabel _label;
	private JCheckBox _checkbox;

	public CheckBoxPanel(String title, boolean enabled) {
		super();
		setMaximumSize(new Dimension(300, 30));
		setLayout(new BoxLayout(this, BoxLayout.X_AXIS));
		_label = new JLabel(title);
		_label.setMaximumSize(new Dimension(90, 30));
		add(Box.createRigidArea(new Dimension(10, 30)));
		add(_label);
		_checkbox = new JCheckBox("", enabled);
		_checkbox.setMaximumSize(new Dimension(150, 30));
		_checkbox.setPreferredSize(new Dimension(150, 30));
		add(Box.createRigidArea(new Dimension(10, 30)));
		add(_checkbox);
		add(Box.createRigidArea(new Dimension(10, 30)));
	}

	public CheckBoxPanel(String title, boolean enabled, 
			int label_width, int label_height, int field_width, int field_height) {
		this(title, enabled);
		setLabelSize(label_width, label_height);
		setFieldSize(field_width, field_height);
	}

	public void setText(String text) {
		_checkbox.setText(text);
	}

	public String getText() {
		return _checkbox.getText();
	}

	public boolean isSelected() {
		return _checkbox.isSelected();
	}

	public String getLabel() {
		return _label.getText();
	}

	public void setFieldSize(int width, int height) {
		_checkbox.setMaximumSize(new Dimension(width, height));
		_checkbox.setPreferredSize(new Dimension(width, height));
	}
	
	public void setLabelSize(int width, int height) {
		_label.setMaximumSize(new Dimension(width, height));
		_label.setPreferredSize(new Dimension(width, height));
	}
	
	public void setEnabled(boolean enabled) {
		_checkbox.setEnabled(enabled);
	}

}
