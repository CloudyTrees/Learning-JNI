public class HelloWorld {
	public native String print(final String name);  //native method
	static   //static initializer code
	{
		System.loadLibrary("cprint");
	} 
 
	public static void main(String[] args)
	{
		HelloWorld hw = new HelloWorld();
        	final String result = hw.print("whoever");
		System.out.println(result);
	}
}
