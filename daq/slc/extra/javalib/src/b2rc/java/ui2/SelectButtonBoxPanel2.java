package b2rc.java.ui2;

import java.awt.Dimension;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;

import javax.swing.Box;
import javax.swing.BoxLayout;
import javax.swing.JButton;
import javax.swing.JLabel;
import javax.swing.JPanel;

import b2daq.database.DBObjectList;

public class SelectButtonBoxPanel2 extends JPanel {

	private static final long serialVersionUID = 1L;
	private JLabel _label;
	private JButton _button;
	private JLabel _label_value;
	private DBObjectList _obj;
	
	public SelectButtonBoxPanel2(String title, DBObjectList obj) {
		super();
		_obj = obj;
		setMaximumSize(new Dimension(300, 30));
		setLayout(new BoxLayout(this, BoxLayout.X_AXIS));
		_label = new JLabel(title);
		_label.setMaximumSize(new Dimension(90, 30));
		add(Box.createRigidArea(new Dimension(10, 30)));
		add(_label);
		_button = new JButton(obj.getConfigName());
		_button.setMaximumSize(new Dimension(90, 30));
		_button.setPreferredSize(new Dimension(90, 30));
		_button.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent arg0) {
				ParamViewPanel.showSelectPanel(_obj.getTable(), _obj.get(0));
			}
		});
		add(Box.createRigidArea(new Dimension(10, 30)));
		add(_button);
		add(Box.createRigidArea(new Dimension(10, 30)));
		_label_value = new JLabel(obj.getConfigName());
		_label_value.setMaximumSize(new Dimension(90, 30));
		add(_label_value);
		add(Box.createRigidArea(new Dimension(10, 30)));
	}

	public SelectButtonBoxPanel2(String title, DBObjectList obj, 
			int label_width, int label_height, int field_width, int field_height) {
		this(title, obj);
		setLabelSize(label_width, label_height);
		setFieldSize(field_width, field_height);
	}

	public void setText(String text) {
		_label_value.setText(text);
	}

	public String getText() {
		return _label_value.getText();
	}

	public String getLabel() {
		return _label.getText();
	}

	public JButton getButton() {
		return _button;
	}
	
	public void setFieldSize(int width, int height) {
		_button.setMaximumSize(new Dimension(width, height));
		_button.setPreferredSize(new Dimension(width, height));
	}
	
	public void setLabelSize(int width, int height) {
		_label.setMaximumSize(new Dimension(width, height));
		_label.setPreferredSize(new Dimension(width, height));
	}
	
	public void setEnabled(boolean enabled) {
		_button.setEnabled(enabled);
	}

}
