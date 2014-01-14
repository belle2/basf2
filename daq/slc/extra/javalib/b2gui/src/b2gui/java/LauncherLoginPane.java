package b2gui.java;

import java.awt.Dimension;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;

import javax.swing.Box;
import javax.swing.BoxLayout;
import javax.swing.JCheckBox;
import javax.swing.JLabel;
import javax.swing.JOptionPane;
import javax.swing.JPanel;
import javax.swing.UIManager;

import b2daq.core.Log;
import b2daq.core.LogLevel;
import b2daq.java.ui.TextBoxPanel;
import b2gui.java.LauncherPanel.KUserInfo;

import com.jcraft.jsch.JSch;
import com.jcraft.jsch.Session;
import com.jcraft.jsch.UserInfo;

public class LauncherLoginPane extends JPanel {

	private static final long serialVersionUID = 1L;

	private TextBoxPanel _host_name_panel = new TextBoxPanel("Host name : ", "belle-rpc1");
	private RemoteHostPanel _remote_host_panel;
	private JCheckBox _check_box = new JCheckBox("use ssh tunneling");

	public LauncherLoginPane(String host, String proxy, int port, String account) {
		super();
		setSize(300, 200);
		setLayout(new BoxLayout(this, BoxLayout.Y_AXIS));
		JPanel title_panel = new JPanel();
		JLabel title_label = new JLabel("Belle 2 GUI launcher version " + Belle2GUILauncherJavaGUI.VERSION);
		title_label.setHorizontalAlignment(JLabel.CENTER);
		title_panel.setLayout(new BoxLayout(title_panel, BoxLayout.X_AXIS));
		title_panel.add(title_label);
		
		_host_name_panel.setText(host);
		JPanel check_panel = new JPanel();
		_check_box.setMaximumSize(new Dimension(300, 30));
		_check_box.setHorizontalAlignment(JLabel.LEFT);
		check_panel.setLayout(new BoxLayout(check_panel, BoxLayout.X_AXIS));
		check_panel.add(Box.createRigidArea(new Dimension(10, 10)));
		check_panel.add(_check_box);
		_remote_host_panel = new RemoteHostPanel (proxy, port, account,"");
		_check_box.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent arg0) {
				JCheckBox box = (JCheckBox) arg0.getSource();
				_remote_host_panel.setEnabled(box.isSelected());
			}
		});
		add(title_panel);
		add(Box.createRigidArea(new Dimension(0, 10)));
		add(_host_name_panel);
		add(Box.createRigidArea(new Dimension(0, 10)));
		add(check_panel);
		add(Box.createRigidArea(new Dimension(0, 5)));
		add(_remote_host_panel);
		_remote_host_panel.setEnabled(false);
	}

	public String getHostName() {
		return _host_name_panel.getText();
	}

	public String getProxyName() { 
		return _remote_host_panel.getProxyName();
	}
	
	public int getSSHPort() {
		return Integer.parseInt(_remote_host_panel.getSSHPort());
	}

	public String getAccount() {
		return _remote_host_panel.getAccount();
	}

	public String getPassword() {
		return _remote_host_panel.getPassword();
	}

	public boolean useSSH() {
		return _check_box.isSelected();
	}
		
	public static void showPane() {
		try {
			UIManager.setLookAndFeel(UIManager.getSystemLookAndFeelClassName());
		} catch (Exception e) {}
		LauncherFrame frame = LauncherFrame.get();
		LauncherLoginPane login_panel = LauncherFrame.get().getLoginPanel();
		Object[] message = { login_panel };
		String[] options = { "Login", "Cancel" };

		int result = JOptionPane.showOptionDialog(null, message, "Belle 2 GUI Launcher",
				JOptionPane.DEFAULT_OPTION, JOptionPane.PLAIN_MESSAGE, null,
				options, options[0]);
		switch (result) {
		case 0:
			frame.setUseSSH(login_panel.useSSH());
			if ( login_panel.useSSH() ) {
				String pass = login_panel.getPassword();
				String hidden_pass = "";
				for ( @SuppressWarnings("unused") byte c : pass.getBytes() ) {
					hidden_pass += "*";
				}
				frame.setEnableButtons(false);
				frame.setVisible(true);
				frame.getLog().add(new Log("connecting with " + login_panel.getHostName()
						+ " via ssh"));
				try {
					JSch jsch = new JSch();
					Session session = jsch.getSession(
							login_panel.getAccount(),
							login_panel.getProxyName(),
							login_panel.getSSHPort());
					UserInfo ui = new KUserInfo(login_panel);
					session.setUserInfo(ui);
					session.connect();
					session.disconnect();
					frame.getLog().add(new Log("ready to login via ssh tunnels.<br/>" +
							"Host name = " + login_panel.getHostName() + "<br />" +
							"Proxy = " + login_panel.getProxyName() + "<br />" +
							"Port for ssh= " + login_panel.getSSHPort() + "<br />" +
							"Account = " + login_panel.getAccount() + "<br />" +
							"Password = " + hidden_pass));
					frame.setHostName("belle-rpc1");
					frame.setEnableButtons(true);
				} catch (Exception e) {
					e.printStackTrace();
					frame.getLog().add(new Log("failed to make ssh connection.",LogLevel.ERROR));
					showPane();
				}
			} else {
				frame.setVisible(true);
				frame.getLog().add(new Log("host name = " + login_panel.getHostName()));
				frame.setHostName(login_panel.getHostName());
			}
			break;
		case 1:
			break;
		default:
			break;
		}
	}
}
