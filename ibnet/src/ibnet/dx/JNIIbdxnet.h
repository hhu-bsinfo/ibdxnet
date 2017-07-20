/* DO NOT EDIT THIS FILE - it is machine generated */
#include <jni.h>
/* Header for class de_hhu_bsinfo_net_ib_JNIIdxbnet */

#ifndef _Included_de_hhu_bsinfo_net_ib_JNIIdxbnet
#define _Included_de_hhu_bsinfo_net_ib_JNIIdxbnet
#ifdef __cplusplus
extern "C" {
#endif
/*
 * Class:     de_hhu_bsinfo_net_ib_JNIIdxbnet
 * Method:    init
 * Signature: (SIIIIIILde/hhu/bsinfo/net/ib/JNIIbdxnet/SendHandler;Lde/hhu/bsinfo/net/ib/JNIIbdxnet/RecvHandler;Lde/hhu/bsinfo/net/ib/JNIIbdxnet/DiscoveryHandler;Lde/hhu/bsinfo/net/ib/JNIIbdxnet/ConnectionHandler;ZZ)Z
 */
JNIEXPORT jboolean JNICALL Java_de_hhu_bsinfo_net_ib_JNIIbdxnet_init
    (JNIEnv *, jclass, jshort, jint, jint, jint, jint, jint, jint, jobject, jobject, jobject, jobject, jboolean, jboolean);

/*
 * Class:     de_hhu_bsinfo_net_ib_JNIIbdxnet
 * Method:    shutdown
 * Signature: ()Z
 */
JNIEXPORT jboolean JNICALL Java_de_hhu_bsinfo_net_ib_JNIIbdxnet_shutdown
    (JNIEnv *, jclass);

/*
 * Class:     de_hhu_bsinfo_net_ib_JNIIbdxnet
 * Method:    addNode
 * Signature: (I)V
 */
JNIEXPORT void JNICALL Java_de_hhu_bsinfo_net_ib_JNIIbdxnet_addNode
    (JNIEnv *, jclass, jint);

/*
 * Class:     de_hhu_bsinfo_net_ib_JNIIbdxnet
 * Method:    getSendBufferAddress
 * Signature: (S)J
 */
JNIEXPORT jlong JNICALL Java_de_hhu_bsinfo_net_ib_JNIIbdxnet_getSendBufferAddress
    (JNIEnv *, jclass, jshort);

/*
 * Class:     de_hhu_bsinfo_net_ib_JNIIbdxnet
 * Method:    returnRecvBuffer
 * Signature: (J)V
 */
JNIEXPORT void JNICALL Java_de_hhu_bsinfo_net_ib_JNIIbdxnet_returnRecvBuffer
    (JNIEnv *, jclass, jlong);

#ifdef __cplusplus
}
#endif
#endif