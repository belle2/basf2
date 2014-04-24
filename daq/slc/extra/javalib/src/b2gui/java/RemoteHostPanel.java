package b2gui.java;

import java.awt.Dimension;

import javax.swing.Box;
import javax.swing.BoxLayout;
import javax.swing.JPanel;

import b2daq.java.ui.PasswordBoxPanel;
import b2daq.java.ui.TextBoxPanel;

public class RemoteHostPanel extends JPanel {
	private static final long serialVersionUID = 1L;
	private TextBoxPanel _proxy_name_panel = null;
	private TextBoxPanel _host_port_panel = null;
	private TextBoxPanel _account_panel = null;
	private PasswordBoxPanel _password_panel = null;
	
	public RemoteHostPanel(String proxy, int port, 
			String account, String password) {
		_proxy_name_panel = new TextBoxPanel("Proxy name : ", proxy);
		_host_port_panel = new TextBoxPanel("SSH Port : ", "" + port);
		_account_panel = new TextBoxPanel("Account : ", account);
		_password_panel = new PasswordBoxPanel("Password : ", password);
		setLayout(new BoxLayout(this, BoxLayout.Y_AXIS));
		add(Box.createRigidArea(new Dimension(0, 5)));
		add(_proxy_name_panel);
		add(Box.createRigidArea(new Dimension(0, 5)));
		add(_host_port_panel);
		add(Box.createRigidArea(new Dimension(0, 5)));
		add(_account_panel);
		add(Box.createRigidArea(new Dimension(0, 5)));
		add(_password_panel);
	}
	public String getProxyName() { return _proxy_name_panel.getText(); }
	public String getSSHPort() { return _host_port_panel.getText(); }
	public String getAccount() { return _account_panel.getText(); }
	public String getPassword() { return _password_panel.getText(); }
	public void setEnabled(boolean enabled) {
		_proxy_name_panel.setEnabled(enabled);
		_host_port_panel.setEnabled(enabled);
		_account_panel.setEnabled(enabled);
		_password_panel.setEnabled(enabled);
	}
	
	
}
