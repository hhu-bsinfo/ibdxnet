/*
 * Copyright (C) 2017 Heinrich-Heine-Universitaet Duesseldorf,
 * Institute of Computer Science, Department Operating Systems
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License,
 * or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>
 */

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
 * Signature: (SIIJIIIILde/hhu/bsinfo/net/ib/JNIIbdxnet/SendHandler;
 * Lde/hhu/bsinfo/net/ib/JNIIbdxnet/RecvHandler;
 * Lde/hhu/bsinfo/net/ib/JNIIbdxnet/DiscoveryHandler;
 * Lde/hhu/bsinfo/net/ib/JNIIbdxnet/ConnectionHandler;ZZ)Z
 */
JNIEXPORT jboolean JNICALL Java_de_hhu_bsinfo_net_ib_JNIIbdxnet_init
    (JNIEnv *, jclass, jshort, jint, jint, jlong, jint, jint, jint, jint,
     jobject, jobject, jobject, jobject, jboolean, jboolean);

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