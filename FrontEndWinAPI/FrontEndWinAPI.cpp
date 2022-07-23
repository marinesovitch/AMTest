// author: marines marinesovitch alias Darek Slusarczyk 2012-2013, 2022
#include "stdafx.h"
#include "resource.h"
#include "feView.h"
#include "BackEnd/h/beMapStream.h"
#include "BackEnd/h/beDocument.h"
#include "BackEnd/h/beController.h"
#include "BackEnd/h/beInstance.h"
#include "BackEnd/h/beTypes.h"

namespace
{

const std::string MapFilePath = "./mapa.dat";

be::IMapStream* createMapStream()
{
	//#define STUB_INPUT
	#ifdef STUB_INPUT
	be::IMapStream* mapStream = be::IMapStream::create();
	#else
	be::IMapStream* mapStream = be::IMapStream::create( MapFilePath );
	#endif
	return mapStream;
}

void initBackendInstance( be::SInstance* beInstance )
{
	std::unique_ptr< be::IMapStream > mapStream( createMapStream() );
	if ( !beInstance->init( mapStream.get() ) )
		throw std::runtime_error( "Cannot initialize backend instance. Check if map file '"
			+ MapFilePath + "' is available. " );
}

} // anonymous namespace

int APIENTRY _tWinMain(
	_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPTSTR    lpCmdLine,
	_In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER( hPrevInstance );
	UNREFERENCED_PARAMETER( lpCmdLine );

	int result = FALSE;
	try
	{
		be::SInstance beInstance;
		initBackendInstance( &beInstance );
		std::unique_ptr< fe::IView > view(
			fe::createView( hInstance, beInstance.d_document, beInstance.d_controller ) );
		if ( view->init( nCmdShow ) )
			result = view->run();
	}
	catch ( std::exception& e )
	{
		const std::string& msg = e.what();
		::MessageBox( 0, msg.c_str(), "Error", MB_OK );
	}
	catch ( ... )
	{
		::MessageBox( 0, "Unknown internal error!", "Error", MB_OK );
	}

	return result;
}
