// author: marines marinesovitch alias Darek Slusarczyk 2012-2013, 2022
#include <stdexcept>
#include <cassert>
#include <vector>
#include <string>
#include <memory>
#include "BackEnd/h/beMapStream.h"
#include "BackEnd/h/beDocument.h"
#include "BackEnd/h/beController.h"
#include "BackEnd/h/beInstance.h"
#include "BackEnd/h/beBitmap.h"
#include "BackEnd/h/beTypes.h"
#include <jni.h>
#include <android/bitmap.h>
#include <android/log.h>

#define  LOG_TAG    "libamtestbackend"
#define  LOGI(...)  __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

namespace
{

using handle_t = jlong;

be::SInstance* raw2instance( handle_t beInstanceHandle )
{
	auto beInstance = reinterpret_cast< be::SInstance* >( beInstanceHandle );
	return beInstance;
}

be::IDocument* raw2document( handle_t beInstanceHandle )
{
	auto beInstance = reinterpret_cast< be::SInstance* >( beInstanceHandle );
	be::IDocument* beDocument = beInstance->d_document;
	return beDocument;
}

be::IController* raw2controller( handle_t beInstanceHandle )
{
	auto beInstance = reinterpret_cast< be::SInstance* >( beInstanceHandle );
	be::IController* beController = beInstance->d_controller;
	return beController;
}

std::string j2str( JNIEnv* env, jstring jstr )
{
	std::string result;
	const char* str = env->GetStringUTFChars( jstr, nullptr );
	if ( str != nullptr )
	{
		result = str;
		env->ReleaseStringUTFChars( jstr, str );
	}
	return result;
}

jstring str2j( JNIEnv* env, const std::string& str )
{
	jstring result = env->NewStringUTF( str.c_str() );
	return result;
}

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

class KBitmap :public be::IBitmap
{
	public:
		KBitmap( JNIEnv* env, jobject bitmap );
		~KBitmap() override = default;

	public:
		bool lock( be::color_t** buffer ) override;
		void unlock() override;

	private:
		JNIEnv* d_env;
		jobject d_bitmap;

};

// ----------------------------------------------------------------------------

KBitmap::KBitmap( JNIEnv* env, jobject bitmap )
	: d_env( env )
	, d_bitmap( bitmap )
{
}

bool KBitmap::lock( be::color_t** buffer )
{
	AndroidBitmapInfo info;
	if ( AndroidBitmap_getInfo( d_env, d_bitmap, &info ) == 0 )
	{
		if ( info.format == ANDROID_BITMAP_FORMAT_RGB_565 )
		{
			void* raw_pixels = nullptr;
			if ( AndroidBitmap_lockPixels( d_env, d_bitmap, &raw_pixels ) == 0 )
			{
				*buffer = reinterpret_cast< be::color_t* >( raw_pixels );
			}
		}
	}

	const bool result = (*buffer) != nullptr;
	return result;
}

void KBitmap::unlock()
{
	AndroidBitmap_unlockPixels( d_env, d_bitmap );
}

// ----------------------------------------------------------------------------

be::IMapStream* createMapStream(
	const jbyte* rawMapStream,
	jint rawMapStreamLength )
{
	//#define STUB_INPUT
	#ifdef STUB_INPUT
	return be::IMapStream::create();
	#else
	const int* mapStreamBegin = reinterpret_cast< const int* >( rawMapStream );
	const long mapStreamLength = static_cast<long>(rawMapStreamLength / sizeof( int ));
	const int* mapStreamEnd = mapStreamBegin + mapStreamLength;
	return be::IMapStream::create( mapStreamBegin, mapStreamEnd );
	#endif
}

} // anonymous namespace

// ----------------------------------------------------------------------------
// AMTest

extern "C" JNIEXPORT handle_t JNICALL Java_com_amtest_frontend_AMTest_createBackendInstance(
	JNIEnv* env,
	jobject /*obj*/,
	jbyteArray jmapBytesArray,
	jint jmapBytesArraySize )
{
	LOGI("createBackendInstance %d", jmapBytesArraySize);
	handle_t beInstanceHandle = 0;
	try
	{
		jboolean isCopy = false;
		jbyte* rawMapStream = env->GetByteArrayElements( jmapBytesArray, &isCopy );

		std::unique_ptr< be::SInstance > beInstance( new be::SInstance() );
		std::unique_ptr< be::IMapStream > mapStream( createMapStream( rawMapStream, jmapBytesArraySize ) );
		if ( beInstance->init( mapStream.get() ) )
			beInstanceHandle = reinterpret_cast< handle_t >( beInstance.release() );

		env->ReleaseByteArrayElements( jmapBytesArray, rawMapStream, JNI_ABORT );

		LOGI("new instance %p", reinterpret_cast<void*>( beInstanceHandle ));
	}
	catch ( std::exception& e )
	{
		const std::string& reason = e.what();
		LOGE("%s", reason.c_str());
	}
	return beInstanceHandle;
}

extern "C" JNIEXPORT void JNICALL Java_com_amtest_frontend_AMTest_destroyBackendInstance(
	JNIEnv* /*env*/,
	jobject /*obj*/,
	handle_t beInstanceHandle )
{
	LOGI("destroyBackendInstance %p", reinterpret_cast<void*>( beInstanceHandle ));
	be::SInstance* beInstance = raw2instance( beInstanceHandle );
	delete beInstance;
}

// ----------------------------------------------------------------------------
// KDocument

extern "C" JNIEXPORT jstring JNICALL Java_com_amtest_frontend_KDocument_getState(
	JNIEnv* env,
	jobject /*obj*/,
	handle_t beInstanceHandle )
{
	LOGI("getState");
	be::IDocument* beDocument = raw2document( beInstanceHandle );
	const std::string& stateStr = beDocument->getState();
	jstring result = str2j( env, stateStr );
	return result;
}

extern "C" JNIEXPORT void JNICALL Java_com_amtest_frontend_KDocument_setState(
	JNIEnv* env,
	jobject /*obj*/,
	handle_t beInstanceHandle,
	jstring jstateString )
{
	LOGI("setState");
	be::IDocument* beDocument = raw2document( beInstanceHandle );
	const std::string& stateStr = j2str( env, jstateString );
	beDocument->setState( stateStr );
}

extern "C" JNIEXPORT jint JNICALL Java_com_amtest_frontend_KDocument_getBkColor(
	JNIEnv* /*env*/,
	jobject /*obj*/,
	handle_t beInstanceHandle )
{
	LOGI("getBkColor");
	be::IDocument* beDocument = raw2document( beInstanceHandle );
	jint result = beDocument->getBkColor();
	return result;
}

// ----------------------------------------------------------------------------
// KController

extern "C" JNIEXPORT jboolean JNICALL Java_com_amtest_frontend_KController_setDeviceSize(
	JNIEnv* /*env*/,
	jobject /*obj*/,
	handle_t beInstanceHandle,
	jint width,
	jint height )
{
	LOGI("setDeviceSize %d %d", width, height);
	be::IController* beController = raw2controller( beInstanceHandle );
	const be::SSize deviceSize( width, height );
	jboolean result = beController->setDeviceSize( deviceSize );
	return result;
}

extern "C" JNIEXPORT jboolean JNICALL Java_com_amtest_frontend_KController_moveTo(
	JNIEnv* /*env*/,
	jobject /*obj*/,
	handle_t beInstanceHandle,
	jint focusPosX,
	jint focusPosY )
{
	LOGI("moveto %d %d", focusPosX, focusPosY);
	be::IController* beController = raw2controller( beInstanceHandle );
	const be::SPoint focusScreenPoint( focusPosX, focusPosY );
	const be::SMoveData moveData( focusScreenPoint );
	const bool result = beController->move( moveData );
	return result;
}

extern "C" JNIEXPORT jboolean JNICALL Java_com_amtest_frontend_KController_moveInDirection(
	JNIEnv* /*env*/,
	jobject /*obj*/,
	handle_t beInstanceHandle,
	jint jdirection )
{
	LOGI("moveInDirection %d", jdirection);
	be::IController* beController = raw2controller( beInstanceHandle );
	const auto direction = static_cast< be::EDirection >( jdirection );
	const be::SMoveData moveData( direction );
	const bool result = beController->move( moveData );
	return result;
}

extern "C" JNIEXPORT jboolean JNICALL Java_com_amtest_frontend_KController_moveDelta(
	JNIEnv* /*env*/,
	jobject /*obj*/,
	handle_t beInstanceHandle,
	jint deltaX,
	jint deltaY )
{
	LOGI("moveDelta %d %d", deltaX, deltaY);
	be::IController* beController = raw2controller( beInstanceHandle );
	const be::SSize delta( deltaX, deltaY );
	const be::SMoveData moveData( delta );
	const bool result = beController->move( moveData );
	return result;
}

extern "C" JNIEXPORT jboolean JNICALL Java_com_amtest_frontend_KController_zoomIn(
	JNIEnv* /*env*/,
	jobject /*obj*/,
	handle_t beInstanceHandle,
	jint delta,
	jboolean zoomInPlace,
	jint focusPosX,
	jint focusPosY )
{
	LOGI("zoomIn %d %d %d %d", delta, (int)zoomInPlace, focusPosX, focusPosY);
	be::IController* beController = raw2controller( beInstanceHandle );
	const be::SPoint focusScreenPoint( focusPosX, focusPosY );
	const be::SZoomData zoomData( be::SZoomData::ZoomIn, delta, zoomInPlace, focusScreenPoint );
	const bool result = beController->zoom( zoomData );
	return result;
}

extern "C" JNIEXPORT jboolean JNICALL Java_com_amtest_frontend_KController_zoomOut(
	JNIEnv* /*env*/,
	jobject /*obj*/,
	handle_t beInstanceHandle,
	jint delta,
	jboolean zoomInPlace,
	jint focusPosX,
	jint focusPosY )
{
	LOGI("zoomOut %d %d %d %d", delta, (int)zoomInPlace, focusPosX, focusPosY);
	be::IController* beController = raw2controller( beInstanceHandle );
	const be::SPoint focusScreenPoint( focusPosX, focusPosY );
	const be::SZoomData zoomData( be::SZoomData::ZoomOut, delta, zoomInPlace, focusScreenPoint );
	const bool result = beController->zoom( zoomData );
	return result;
}

extern "C" JNIEXPORT jboolean JNICALL Java_com_amtest_frontend_KController_resetView(
	JNIEnv* /*env*/,
	jobject /*obj*/,
	handle_t beInstanceHandle )
{
	LOGI("resetView");
	be::IController* beController = raw2controller( beInstanceHandle );
	jboolean result = beController->resetView();
	return result;
}

extern "C" JNIEXPORT jboolean JNICALL Java_com_amtest_frontend_KController_generateContents(
	JNIEnv* env,
	jobject /*obj*/,
	handle_t beInstanceHandle,
	jobject jbitmap )
{
	LOGI("generateContents");
	be::IController* beController = raw2controller( beInstanceHandle );
	KBitmap bitmap( env, jbitmap );
	jboolean result = beController->generateContents( &bitmap );
	return result;
}

extern "C" JNIEXPORT jstring JNICALL Java_com_amtest_frontend_KController_getParamsDescription(
	JNIEnv* env,
	jobject /*obj*/,
	handle_t beInstanceHandle )
{
	LOGI("getParamsDescription");
	be::IController* beController = raw2controller( beInstanceHandle );
	const std::string& description = beController->getParamsDescription();
	jstring result = str2j( env, description );
	return result;
}
