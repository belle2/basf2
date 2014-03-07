package b2daq.java.io;

import java.io.BufferedReader;
import java.io.FileInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.net.URL;
import java.net.URLConnection;
import java.util.ArrayList;
import java.util.LinkedHashMap;
import java.util.Iterator;
import java.util.Map;
import java.util.Set;
import java.util.jar.JarEntry;
import java.util.jar.JarFile;

public class ConfigFile {

	private Map<String, String> _map = new LinkedHashMap<String, String>();
	private String _dir = "";
	private ArrayList<String> _dir_v = new ArrayList<String>();
	private JarFile _jarfile;
	
	public ConfigFile() {
	}

	public ConfigFile(String is) {
		this();
		read(is);
	}

	public void read(String path) {
		try {
			String[] param = path.split(";");
			if (param.length > 1) {
				_jarfile = new JarFile(param[0]);
				JarEntry entry = (JarEntry) _jarfile.getEntry(param[1]);
				read(_jarfile.getInputStream(entry));
			} else {
				if (path.startsWith("http://") || path.startsWith("https://")) {
					URL url = new URL(path);
					URLConnection conn = url.openConnection();
					read(conn.getInputStream());
				} else {
					read(new FileInputStream(path));
				}
			}
		} catch (IOException e) {
			e.printStackTrace();
		}
	}

	public void read(InputStream is) throws IOException {
		BufferedReader br = new BufferedReader(new InputStreamReader(is));
		String s = "";
		String s_tmp = "";
		String dir = "";
		while ((s = br.readLine()) != null) {
			if (s.length() == 0 || s.startsWith("#"))
				continue;
			if (s.replace(" ", "").endsWith("\\")) {
				s_tmp += s.substring(0, s.lastIndexOf('\\'));
				continue;
			} else {
				if (s_tmp.length()>0){
					StringBuffer ss = new StringBuffer();
					ss.append(s_tmp);
					ss.append(s);
					s = ss.toString();
					s_tmp = "";
				}
			}
			if (s.startsWith("!cd")) {
				dir = s.replace("!cd", "").replace(" ", "").replace("\t", "");
				if (dir.length() > 0) {
					_dir_v.add(dir);
				}
				continue;
			}
			String[] str_v = s.split(":");
			if (str_v.length >= 2) {
				String label = str_v[0].replace(" ", "").replace("\t", "");
				String value = "";
				String str = str_v[1];
				for (int i = 2; i < str_v.length; i++) {
					str += ":" + str_v[i];
				}
				int i = 0;
				boolean is_comment = false;
				for (; i < str.length(); i++) {
					if (str.charAt(i) == '#' || str.charAt(i) == '\n')
						break;
					if ((str.charAt(i) == ' ' || str.charAt(i) == '\t'))
						continue;
					if (str.charAt(i) == '"') {
						is_comment = is_comment || (value.length() == 0);
						if (!is_comment) value += '"';
						for (i++; i < str.length(); i++) {
							if (str.charAt(i) == '"') {
								if (!is_comment) value += '"';
								i++;
								break;
							}
							value += str.charAt(i);
						}
						if (i >= str.length()) break;
					}
					if (str.charAt(i) == '"') {
						if (is_comment) break;
					}
					if (str.charAt(i) == '$') {
						i++;
						if (str.charAt(i) == '{') {
							for (i++; i < str.length(); i++) {
								if (str.charAt(i) == '}')
									break;
								value += str.charAt(i);
							}
						}
						String env = System.getenv(value);
						if (env != null) {
							value = env;
						} else if (_map.containsKey(value)) {
							value = _map.get(value);
						}
						continue;
					}
					value += str.charAt(i);
				}
				if (dir.length() > 0) {
					label = dir + "." + label;
				}
				if (_map.containsKey(label)) {
					_map.put(label, value);
				} else {
					_map.put(label, value);
				}
			}
		}
	}

	public void reset() {
		_map.clear();
		cd();
	}
	
	public boolean hasKey(String key) {
		String path = (_dir.length() > 0)?_dir + "." + key:key;
		return _map.containsKey(path);
	}

	public int getInt(String key) {
		try {
			return Integer.parseInt(getString(key));
		} catch (NumberFormatException e) {
			e.printStackTrace();
			return -1;
		}
	}

	public double getFloat(String key) {
		try {
			return Double.parseDouble(getString(key));
		} catch (NumberFormatException e) {
			return -1;
		}
	}

	public String getString(String key) {
		if (!hasKey(key)) {
			return "";
		} else {
			String path = (_dir.length() > 0)?_dir + "." + key:key;
			return _map.get(path);
		}
	}

	public void cd() {
		_dir = "";
	}

	public void cd(String dir) {
		_dir = (_dir.length() > 0)?_dir+"."+dir: dir;
	}

	public Set<String> getKeyList() {
		return _map.keySet();
	}

	public String toString() {
		Iterator<String> iter = _map.keySet().iterator();
		StringBuffer ss = new StringBuffer();
		ss.append("{\n");
		while (iter.hasNext()) {
			String key = iter.next();
			ss.append("'" + key + "': '" + _map.get(key) + "'");
			if (iter.hasNext())
				ss.append(",\n");
		}
		ss.append("\n}");
		return ss.toString();
	}

	static public void main(String[] argv) throws IOException {
		ConfigFile config = new ConfigFile("/home/tkonno/runcontrol.conf");
		config.cd("physics.regular.full");
		System.out.println("'"+config.getString("PXD.runtype")+"'");
		config.cd();
	}

	public ArrayList<String> getDirectories() {
		return _dir_v;
	}

}
