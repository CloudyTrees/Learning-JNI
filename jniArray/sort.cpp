#include "SortArray.h"
#include "jni.h"
 
/*
 * Class:     SortArray
 * Method:    sort
 * Signature: ([I)[I
 */
JNIEXPORT jintArray JNICALL Java_SortArray_sort
    (JNIEnv *env, jobject obj, jintArray arr){
 
    jsize arrLength = env->GetArrayLength(arr);
    jintArray arrSorted = env->NewIntArray(arrLength); 
 
    jint *arrOut = NULL;
    arrOut = env->GetIntArrayElements(arr, 0);
 
    for(jsize x = 0; x < arrLength; x++){
        for(jsize y = 0; y < arrLength - 1; y++){
            if(arrOut[y] > arrOut[y+1]){
                jsize temp = arrOut[y+1];
		arrOut[y+1] = arrOut[y];
		arrOut[y] = temp;
	    }
        }
    }
 
    env->SetIntArrayRegion(arrSorted, 0, arrLength, arrOut);
    env->ReleaseIntArrayElements(arr, arrOut, 0);
 
    return arrSorted;
}
