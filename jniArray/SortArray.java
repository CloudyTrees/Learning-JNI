class SortArray {
	private native int[] sort(int[] arr);  //native method 
 
	static   //static initializer code
	{
		System.loadLibrary("SortArray");
	} 
 
	public static void main(String[] args)
	{
		int iArr[] = {4,5,2,7,1,9}; // Input array
		int oArr[]; //Output array
 
		SortArray arr = new SortArray();
 
		System.out.println("Unsorted array: ");
		for(int i = 0; i < iArr.length; i++){
			System.out.println(iArr[i] + " ");
		}
 
		oArr = arr.sort(iArr);
 
		System.out.println("Sorted array: ");
		for(int i = 0; i < oArr.length; i++){
			System.out.println(oArr[i] + " ");
		}
	}
}
