# Collection of JNI knowledge

## Type information
1.  JNI reference types are organized in the hierarchy shown below
   ![alt text](http://docs.oracle.com/javase/7/docs/technotes/guides/jni/spec/images/types4.gif)
    
    In C, all other JNI reference types are defined to be the same as jobject. 
    For example:

    ```
    typedef jobject jclass;
    ```
    In C++, JNI introduces a set of dummy classes to enforce the subtyping relationship. For example:

    ```
    class _jobject {}; 
    class _jclass : public _jobject {}; 
    ... 
    typedef _jobject *jobject; 
    typedef _jclass *jclass;
    ```
2.  Method and field IDs are regular C pointer types:

    ```
    struct _jfieldID;                       // opaque structure
    typedef struct _jfieldID *jfieldID;     // field IDs
     
    struct _jmethodID;                      // opaque structure
    typedef struct _jmethodID *jmethodID;   // method IDs
    ```





## Compiling, linking and name lookup

1.  Since the Java VM is multithreaded, native libraries should also be compiled 
     and linked with multithread aware native compilers. 
    For example, the `-mt` flag should be used for C++ code compiled with the 
     Sun Studio compiler. 
    For code complied with the GNU `gcc` compiler, the flags `-D_REENTRANT` or 
     `-D_POSIX_C_SOURCE` should be used. 
    For more information please refer to the native compiler documentation.
2.  The system follows a standard, but platform-specific, approach to convert 
     the library name to a native library name. 
    For example, a Solaris system converts the name `pkg_Cls` to `libpkg_Cls.so`, 
     while a Win32 system converts the same `pkg_Cls` name to `pkg_Cls.dll`.
3.  The programmer can also call the JNI function `RegisterNatives()` to register 
     the native methods associated with a class. 
    The `RegisterNatives()` function is particularly useful with statically 
    linked functions.
4.  Dynamic linkers resolve entries based on their names. 
    A native method name is concatenated from the following components:
     * the prefix `Java_`
     * a mangled fully-qualified class name
     * an underscore (`_`) separator
     * a mangled method name
     * for overloaded native methods, two underscores (`__`) followed by the 
       mangled argument signature





## Local and global references

1.  Primitive types, such as `int`, `char`, and so on, are copied between 
     Java and native code. 
    Arbitrary Java objects, on the other hand, are passed by reference. 
    The VM must keep track of all objects that have been passed to the native code, 
     so that these objects are not freed by the garbage collector. 
    The native code, in turn, must have a way to inform the VM that it no longer 
     needs the objects. 
    In addition, the garbage collector must be able to move an object referred 
     to by the native code.
2.  The JNI divides object references used by the native code into two categories: 
     *local and global references*. 
    Local references are valid for the duration of a native method call, and are 
     automatically freed after the native method returns. 
    Global references remain valid until they are explicitly freed.
    Local references are only valid in the thread in which they are created. 
    The native code must not pass local references from one thread to another.
3.  Objects are passed to native methods as local references. 
    All Java objects returned by JNI functions are local references. 
    The JNI allows the programmer to create global references from local references. 
    JNI functions that expect Java objects accept both global and local references. 
    A native method may return a local or global reference to the VM as its result.
4.  To implement local references, the Java VM creates a registry for each 
     transition of control from Java to a native method. 
    A registry maps nonmovable local references to Java objects, and keeps the 
     objects from being garbage collected. 
    All Java objects passed to the native method (including those that are 
     returned as the results of JNI function calls) are automatically added to 
     the registry. 
    The registry is deleted after the native method returns, allowing all of its 
     entries to be garbage collected.
5.  There are times when the programmer should explicitly free a local reference. 
    Consider, for example, the following situations:
     * A native method accesses a large Java object, thereby creating a local 
       reference to the Java object. 
       The native method then performs additional computation before returning to 
       the caller. 
       The local reference to the large Java object will prevent the object from 
       being garbage collected, even if the object is no longer used in the 
       remainder of the computation.
     * A native method creates a large number of local references, although not 
       all of them are used at the same time. 
       Since the VM needs a certain amount of space to keep track of a local 
       reference, creating too many local references may cause the system to run 
       out of memory. 
       For example, a native method loops through a large array of objects, 
       retrieves the elements as local references, and operates on one element at 
       each iteration. 
       After each iteration, the programmer no longer needs the local reference 
       to the array element.

    The JNI allows the programmer to manually delete local references at any 
     point within a native method. 
    To ensure that programmers can manually free local references, JNI functions 
     are not allowed to create extra local references, except for references they 
     return as the result.
6.  The overhead of using accessor functions through opaque references is higher 
     than that of direct access to C data structures. 
    This overhead is not acceptable for large Java objects containing many primitive 
     data types, such as integer arrays and strings. 
    (Consider native methods that are used to perform vector and matrix calculations.) 
    It would be grossly inefficient to iterate through a Java array and retrieve 
     every element with a function call.
7.  One solution introduces a notion of “pinning” so that the native method can 
     ask the VM to pin down the contents of an array. 
    The native method then receives a direct pointer to the elements. 
    This approach, however, has two implications:
     * The garbage collector must support pinning.
     * The VM must lay out primitive arrays contiguously in memory. 
       Although this is the most natural implementation for most primitive arrays, 
       boolean arrays can be implemented as packed or unpacked. 
       Therefore, native code that relies on the exact layout of boolean arrays 
       will not be portable.

    We adopt a compromise that overcomes both of the above problems.
    
    First, we provide a set of functions to copy primitive array elements between 
    a segment of a Java array and a native memory buffer. 
    Use these functions if a native method needs access to only a small number 
    of elements in a large array.
    
    Second, programmers can use another set of functions to retrieve a pinned-down 
    version of array elements. 
    Keep in mind that these functions may require the Java VM to perform storage 
    allocation and copying. 
    Whether these functions in fact copy the array depends on the VM implementation, 
     as follows:
     * If the garbage collector supports pinning, and the layout of the array is 
       the same as expected by the native method, then no copying is needed.
     * Otherwise, the array is copied to a nonmovable memory block (for example, 
       in the C heap) and the necessary format conversion is performed. 
       A pointer to the copy is returned.
       
    Lastly, the interface provides functions to inform the VM that the native code 
     no longer needs to access the array elements. 
    When you call these functions, the system either unpins the array, or it 
     reconciles the original array with its non-movable copy and frees the copy.
    
    A JNI implementation must ensure that native methods running in multiple 
     threads can simultaneously access the same array. 
    For example, the JNI may keep an internal counter for each pinned array so 
     that one thread does not unpin an array that is also pinned by another thread. 
    Note that the JNI does not need to lock primitive arrays for exclusive access 
     by a native method. 
    Simultaneously updating a Java array from different threads leads to nondeterministic results.
8.  The JNI allows native code to access the fields and to call the methods of 
    Java objects. 
    The JNI identifies methods and fields by their symbolic names and type signatures. 
    A two-step process factors out the cost of locating the field or method from 
     its name and signature. 
    For example, to call the method f in class cls, the native code first obtains 
     a method ID, as follows:

     ```
     jmethodID mid = env->GetMethodID(cls, “f”, “(ILjava/lang/String;)D”);
     ```
    The native code can then use the method ID repeatedly without the cost of 
     method lookup, as follows:

     ```
     jdouble result = env->CallDoubleMethod(obj, mid, 10, str); 
     ```
9.  A field or method ID does not prevent the VM from unloading the class from 
     which the ID has been derived. 
    After the class is unloaded, the method or field ID becomes invalid. 
    The native code, therefore, must make sure to:
     * keep a live reference to the underlying class, or
     * recompute the method or field ID

    if it intends to use a method or field ID for an extended period of time.





## Exceptions

1.  Certain JNI functions use the Java exception mechanism to report error conditions. 
    In most cases, JNI functions report error conditions by returning an error 
    code **_and_** throwing a Java exception. 
    The error code is usually a special return value (such as `NULL`) that is 
    outside of the range of normal return values. 
    Therefore, the programmer can:
     * quickly check the return value of the last JNI call to determine if an 
       error has occurred, and
     * call a function, `ExceptionOccurred()`, to obtain the exception object 
       that contains a more detailed description of the error condition.
    There are two cases where the programmer needs to check for exceptions without being able to first check an error code:
     * The JNI functions that invoke a Java method return the result of the Java 
       method. 
       The programmer must call `ExceptionOccurred()` to check for possible 
       exceptions that occurred during the execution of the Java method.
     * Some of the JNI array access functions do not return an error code, but 
       may throw an `ArrayIndexOutOfBoundsException` or `ArrayStoreException`.

    In all other cases, a non-error return value guarantees that no exceptions have been thrown.
2.  There are two ways to handle an exception in native code:
     * The native method can choose to return immediately, causing the exception 
       to be thrown in the Java code that initiated the native method call.       
     * The native code can clear the exception by calling `ExceptionClear()`, 
       and then execute its own exception-handling code.

    After an exception has been raised, the native code must first clear the 
    exception before making other JNI calls. 
    When there is a pending exception, the JNI functions that are safe to call are

      ```
      ExceptionOccurred()
      ExceptionDescribe()
      ExceptionClear()
      ExceptionCheck()
      ReleaseStringChars()
      ReleaseStringUTFChars()
      ReleaseStringCritical()
      Release<Type>ArrayElements()
      ReleasePrimitiveArrayCritical()
      DeleteLocalRef()
      DeleteGlobalRef()
      DeleteWeakGlobalRef()
      MonitorExit()
      PushLocalFrame()
      PopLocalFrame()
      ```





## Misc.

1.  The following definition is provided for convenience.

    ```
    #define JNI_FALSE  0
    #define JNI_TRUE   1
    ```
2.  The Java method:
   
    ```
    long f (int n, String s, int[] arr); 
    ```
    has the following type signature:
    
    ```
    (ILjava/lang/String;[I)J 
    ```
3.  





## Tables

Table 2-1 Unicode Character Translation

| Escape sequence   | Denotes                                   |
| ----------------- | ----------------------------------------- |
| `_1`              | the character "`_`""                      |
| `_2`              | the character "`;`"                       |
| `_3`              | the character "["                         |
| `_0XXXX`          | a Unicode character XXXX.                 |
|                   | Note that lower case is used to represent |
|                   | non-ASCII Unicode characters, e.g.,       |
|                   | `_0abcd` as opposed to `_0ABCD`.          |

-----------------------------------------------------------------

Table 3-1 Primitive Types and Native Equivalents

| Java Type         | Native Type           | Description       |
| ----------------- | --------------------- | ----------------- |
| `boolean`         | `jboolean`            | unsigned 8 bits   |
| `byte`            | `jbyte`               | signed 8 bits     |
| `char`            | `jchar`               | unsigned 16 bits  |
| `short`           | `jshort`              | signed 16 bits    |
| `int`             | `jint`                | signed 32 bits    |
| `long`            | `jlong`               | signed 64 bits    |
| `float`           | `jfloat`              | 32 bits           |
| `double`          | `jdouble`             | 64 bits           |
| `void`            | `void`                | N/A               |

-----------------------------------------------------------------

Table 3-2 Java VM Type Signatures

| Type Signature                 | Java Type             |
| -----------------              | --------------------- |
| `Z`                            | `boolean`             |
| `B`                            | `byte`                |
| `C`                            | `char`                |
| `S`                            | `short`               |
| `I`                            | `int`                 |
| `J`                            | `long`                |
| `F`                            | `float`               |
| `D`                            | `double`              |
| `L` fully-qualified-class `;`  | fully-qualified-class |
| `[` type                       | type`[]`              |
| ( arg-types ) ret-type         | method type           |

-----------------------------------------------------------------






## Tips






## References
1.  [Java Native Interface Specification](https://docs.oracle.com/javase/8/docs/technotes/guides/jni/spec/jniTOC.html)
2.  
3.  
4.  
5.  
6.  
7.  
8.  
9.  
10. 
11. 
