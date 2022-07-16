// author: marines marinesovitch alias Darek Slusarczyk 2012-2013
#include "ph.h"
#include "beMapStream.h"
#include "beConsts.h"

#ifndef ANDROID
#pragma warning( disable : 4996 )
#endif

namespace be
{

namespace
{

const int StreamTerminator = 0x89ABCDEF;

// see description of format at KMapReader
static const int s_rawDataStream[] = 
{
	1, // number of segments

	7, // road class
	2, // number of points
	// points { x, y, x, y, ... }
	-300, -300,
	-100, -100,

	6, // road class
	8, // number of points
	// points { x, y, x, y, ... }
	0, 0,
	100, 0,
	100, 100,
	200, 100,
	300, 200,
	300, 200,
	500, 300,
	600, 500,

	6, // road class
	9, // number of points
	// points { x, y, x, y, ... }
	1, 1,
	0, 500,
	100, 400,
	200, 400,
	300, 300,
	400, 500,
	500, 500,
	700, 300,
	800, 300,

	0, // road class
	2, // number of points
	// points { x, y, x, y, ... }
	100, 100,
	100, 400,

	1, // road class
	2, // number of points
	// points { x, y, x, y, ... }
	200, 100,
	200, 400,

	2, // road class
	2, // number of points
	// points { x, y, x, y, ... }
	300, 100,
	300, 400,

	3, // road class
	2, // number of points
	// points { x, y, x, y, ... }
	400, 100,
	400, 400,

	4, // road class
	2, // number of points
	// points { x, y, x, y, ... }
	500, 100,
	500, 400,

	5, // road class
	2, // number of points
	// points { x, y, x, y, ... }
	600, 100,
	600, 400,

	6, // road class
	2, // number of points
	// points { x, y, x, y, ... }
	700, 100,
	700, 400,

	7, // road class
	2, // number of points
	// points { x, y, x, y, ... }
	800, 100,
	800, 400,

	//1, // road class
	//3, // number of points
	//// points { x, y, x, y, ... }
	//0, 0,
	//3, 0,
	//3, 3,

	1, // road class
	3, // number of points
	// points { x, y, x, y, ... }
	-300, consts::MinCoord,
	1200, consts::MinCoord,
	3200, consts::MinCoord,

	0, // road class
	3, // number of points
	// points { x, y, x, y, ... }
	-500, consts::MinCoord + 300,
	1400, consts::MinCoord + 300,
	3800, consts::MinCoord + 300,

	1, // road class
	2, // number of points
	// points { x, y, x, y, ... }
	-100, 1300,
	3200, -100,

	2, // road class
	2, // number of points
	// points { x, y, x, y, ... }
	-100, 300,
	3200, 300,

	//1, // road class
	//3, // number of points
	//// points { x, y, x, y, ... }
	//500, 500,
	//505, 500,
	//505, 505

	3, // road class
	2, // number of points
	// points { x, y, x, y, ... }
	800, 100,
	900, 200,

	4, // road class
	2, // number of points
	// points { x, y, x, y, ... }
	100, 100,
	400, 100,

	5, // road class
	2, // number of points
	// points { x, y, x, y, ... }
	500, 500,
	500, 600,

	6, // road class
	2, // number of points
	// points { x, y, x, y, ... }
	200, 200,
	300, 200,

	7, // road class
	2, // number of points
	// points { x, y, x, y, ... }
	600, 100,
	600, 200,

	7, // road class
	2, // number of points
	// points { x, y, x, y, ... }
	500, 500,
	700, 500,

	6, // road class
	2, // number of points
	// points { x, y, x, y, ... }
	400, 100,
	400, 200,

	//1, // road class
	//3, // number of points
	//// points { x, y, x, y, ... }
	//-100, 100,
	//3000, 100,
	//3000, 150,

	//1, // road class
	//3, // number of points
	//// points { x, y, x, y, ... }
	//200, 200,
	//300, 200,
	//300, 300,

	//1, // road class
	//3, // number of points
	//// points { x, y, x, y, ... }
	//500, 500,
	//700, 500,
	//700, 700, 

	StreamTerminator
};

// ----------------------------------------------------------------------------

class KArrayMapStream : public IMapStream
{
	public:
		KArrayMapStream(
			const int* begin,
			const int* end );

	public:
		virtual bool getInt( int* value );

	private:
		const int* d_it;
		const int* d_end;

};

// ----------------------------------------------------------------------------

KArrayMapStream::KArrayMapStream(
	const int* begin,
	const int* end )
	: d_it( begin )
	, d_end( end )
{
}

bool KArrayMapStream::getInt( int* value )
{
	const bool canRead = ( d_it != d_end );
	if ( canRead )
	{
		*value = *d_it;
		assert( *value != StreamTerminator );
		++d_it;
	}
	return canRead;
}

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

class KFileMapStream : public IMapStream
{
	public:
		KFileMapStream();
		virtual ~KFileMapStream();

	public:
		 bool open( const std::string& fname );
		 bool open( const int fileDescriptor, const long offset );

	public:
		virtual bool getInt( int* value );

	private:
		FILE* d_file;

};

// ----------------------------------------------------------------------------

KFileMapStream::KFileMapStream()
	: d_file( 0 )
{
}

KFileMapStream::~KFileMapStream()
{
	if ( d_file != 0 )
		fclose( d_file );
}

bool KFileMapStream::open( const std::string& fname )
{
	assert( d_file == 0 );
	d_file = fopen( fname.c_str(), "rb" );
	const bool result = ( d_file != 0 );
	return result;
}

bool KFileMapStream::open( const int fileDescriptor, const long offset )
{
	assert( d_file == 0 );
	d_file = fdopen( fileDescriptor, "rb" );
	const bool result = ( d_file != 0 ) && ( fseek( d_file, offset, SEEK_SET ) == 0 );
	return result;
}

bool KFileMapStream::getInt( int* value )
{
	assert( d_file != 0 );
	const bool result = ( fread( value, sizeof( *value ), 1, d_file ) == 1 );
	return result;
}

} // anonymous namespace

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

IMapStream* IMapStream::create( const std::string& fname )
{
	KFileMapStream* mapStream = new KFileMapStream();
	if ( !mapStream->open( fname ) )
	{
		delete mapStream;
		mapStream = 0;
	}
	return mapStream;
}

IMapStream* IMapStream::create( const int fileDescriptor, const long offset )
{
	KFileMapStream* mapStream = new KFileMapStream();
	if ( !mapStream->open( fileDescriptor, offset ) )
	{
		delete mapStream;
		mapStream = 0;
	}
	return mapStream;
}

IMapStream* IMapStream::create( const int* begin, const int* end )
{
	IMapStream* mapStream = new KArrayMapStream( begin, end );
	return mapStream;
}

IMapStream* IMapStream::create()
{
	IMapStream* mapStream
		= create(
			s_rawDataStream,
			s_rawDataStream + ( sizeof( s_rawDataStream ) / sizeof( s_rawDataStream[ 0 ] ) ) );
	return mapStream;
}

// ----------------------------------------------------------------------------

IMapStream::~IMapStream()
{
}

} // namespace be

#ifndef ANDROID
#pragma warning( default : 4308 )
#endif
