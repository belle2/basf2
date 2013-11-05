package b2daq.core;

public interface SerializableObject {

	public void readObject(Reader reader) throws Exception ;
	public void writeObject(Writer writer) throws Exception ;

}
