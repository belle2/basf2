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
	private ArrayList<String> _dir = new ArrayList<String>();
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
		while ((s = br.readLine()) != null) {
			if (s.length() == 0 || s.startsWith("#"))
				continue;
			String[] str_v = s.split(":");
			if (str_v.length >= 2) {
				String label = str_v[0].replace(" ", "").replace("\t", "");
				String value = "";
				int i = 0;
				for (; i < str_v[1].length(); i++) {
					if (str_v[1].charAt(i) == '#' || str_v[1].charAt(i) == '\n')
						break;
					if (str_v[1].charAt(i) == ' ' || str_v[1].charAt(i) == '\t')
						continue;
					if (str_v[1].charAt(i) == '"') {
						for (i++; i < str_v[1].length(); i++) {
							if (str_v[1].charAt(i) == '"')
								break;
							value += str_v[1].charAt(i);
						}
						break;
					}
					if (str_v[1].charAt(i) == '$') {
						i++;
						if (str_v[1].charAt(i) == '{') {
							for (i++; i < str_v[1].length(); i++) {
								if (str_v[1].charAt(i) == '}')
									break;
								value += str_v[1].charAt(i);
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
					value += str_v[1].charAt(i);
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
		String path = "";
		for (String d : _dir) {
			path += d + ".";
		}
		return _map.containsKey(path + key);
	}

	public int getInt(String key) {
		try {
			return Integer.parseInt(getString(key));
		} catch (NumberFormatException e) {
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
			return "null";
		} else {
			String path = "";
			for (String d : _dir) {
				path += d + ".";
			}
			System.out.println(path + key + " " + _map.get(path + key));
			return _map.get(path + key);
		}
	}

	public void cd() {
		_dir.clear();
	}

	public void cd(String base) {
		if (base.matches("\\.\\.")) {
			if (_dir.size() > 0) {
				_dir.remove(_dir.size());
			}
		} else {
			_dir.add(base);
		}
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
		ConfigFile config = new ConfigFile("/home/tkonno/test.conf");
		config.cd("h1");
		System.out.println(config.getString("fill.color"));
		System.out.println(config.getString("line.color"));
		config.cd();
	}

}
