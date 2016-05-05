#include "HelloWorld.h"
#include "jni.h"
#include  "stdio.h"
 
JNIEXPORT jstring JNICALL Java_HelloWorld_print(JNIEnv *env, jobject obj, jstring name)
{
  const char *str = (*env)->GetStringUTFChars(env, name, 0);
  printf("Hello world, %s, from %p\n", str, env);
  
  (*env)->ReleaseStringUTFChars(env, name, str);
  return (*env)->NewStringUTF(env, "success");
}
