package b2dqm.java.ui;

import java.awt.Dimension;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;

import javax.swing.Box;
import javax.swing.BoxLayout;
import javax.swing.JButton;
import javax.swing.JFrame;
import javax.swing.JLabel;
import javax.swing.JPanel;
import javax.swing.JTextField;

import b2daq.core.NumberString;
import b2dqm.core.Axis;
import b2dqm.java.ui.CanvasPanel;


public class AxisPopup extends JFrame {

	private static final long serialVersionUID = 1L;
	
	private CanvasPanel _canvas;
	private Axis _axis;
	private JTextField _max_field;
	private JTextField _min_field;
	
	public AxisPopup(CanvasPanel canvas, Axis axis, int x, int y) {
		_canvas = canvas;
		_axis = axis;
		setDefaultCloseOperation(JFrame.DISPOSE_ON_CLOSE);
		setSize(180, 160);
		setLocation(x, y);
		
		_max_field = new JTextField();
		_min_field = new JTextField();
		_max_field.setMaximumSize(new Dimension(120, 30));
		_max_field.setText(NumberString.toString(_axis.getMax(), 3));
		_min_field.setMaximumSize(new Dimension(120, 30));
		_min_field.setText(NumberString.toString(_axis.getMin(), 3));

		JButton okButton = new JButton("OK");
		okButton.addActionListener(new ActionListener(){
			public void actionPerformed(ActionEvent arg0) {
				double max = Double.parseDouble(_max_field.getText());
				double min = Double.parseDouble(_min_field.getText());
				if ( max > min ) {
					_axis.fixMax(true);
					_axis.setMax(max);
					_axis.fixMin(true);
					_axis.setMin(min);
				}
				_canvas.repaint();
				dispose();
			}
		});
		JButton canselButton = new JButton("Cansel");
		canselButton.addActionListener(new ActionListener(){
			public void actionPerformed(ActionEvent arg0) {
				dispose();
			}
		});

		JPanel maxPanel = new JPanel();
		maxPanel.setLayout(new BoxLayout(maxPanel, BoxLayout.X_AXIS));
		JLabel maxLabel = new JLabel("Max : ");
		maxLabel.setMaximumSize(new Dimension(40,30));
		maxPanel.add(maxLabel);
		maxPanel.add(Box.createRigidArea(new Dimension(0, 5)));
		maxPanel.add(_max_field);

		JPanel minPanel = new JPanel();
		minPanel.setLayout(new BoxLayout(minPanel, BoxLayout.X_AXIS));
		JLabel minLabel = new JLabel("Min : ");
		minLabel.setMaximumSize(new Dimension(40,30));
		minPanel.add(minLabel);
		minPanel.add(Box.createRigidArea(new Dimension(0, 5)));
		minPanel.add(_min_field);

		JPanel textPanel = new JPanel();
		textPanel.setLayout(new BoxLayout(textPanel, BoxLayout.Y_AXIS));
		textPanel.add(new JLabel("Set range"));
		textPanel.add(Box.createRigidArea(new Dimension(0, 8)));
		textPanel.add(maxPanel);
		textPanel.add(Box.createRigidArea(new Dimension(0, 5)));
		textPanel.add(minPanel);
		textPanel.add(Box.createRigidArea(new Dimension(0, 2)));

		JPanel hPanel = new JPanel();
		hPanel.setLayout(new BoxLayout(hPanel, BoxLayout.X_AXIS));
		hPanel.add(Box.createRigidArea(new Dimension(5, 0)));
		hPanel.add(okButton);
		hPanel.add(Box.createRigidArea(new Dimension(10, 0)));
		hPanel.add(canselButton);
		hPanel.add(Box.createRigidArea(new Dimension(5, 0)));
		
		JPanel middlePanel = new JPanel();
		middlePanel.setLayout(new BoxLayout(middlePanel, BoxLayout.X_AXIS));
		middlePanel.add(Box.createRigidArea(new Dimension(10, 10)));
		middlePanel.add(textPanel);
		middlePanel.add(Box.createRigidArea(new Dimension(10, 10)));
		
		JPanel mainPanel = new JPanel();
		mainPanel.setLayout(new BoxLayout(mainPanel, BoxLayout.Y_AXIS));
		mainPanel.add(Box.createRigidArea(new Dimension(10, 10)));
		mainPanel.add(middlePanel);
		mainPanel.add(Box.createRigidArea(new Dimension(10, 10)));
		mainPanel.add(hPanel);
		mainPanel.add(Box.createRigidArea(new Dimension(10, 10)));
		add(mainPanel);
	}	

}

