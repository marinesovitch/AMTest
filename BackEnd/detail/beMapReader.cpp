// author: marines marinesovitch alias Darek Slusarczyk 2012-2013
#include "ph.h"
#include "beMapReader.h"
#include "beSegmentsManager.h"
#include "beUtils.h"
#include "beMapStream.h"
#include "beConsts.h"

namespace be
{

namespace
{

class KDeserializator
{
	public:
		KDeserializator( IMapStream* mapStream );

	public:
		int getInt();
		void getPoint( SPoint* point );

	private:
		IMapStream& d_mapStream;

};

// ----------------------------------------------------------------------------

KDeserializator::KDeserializator( IMapStream* mapStream )
	: d_mapStream( *mapStream )
{
}

int KDeserializator::getInt()
{
	int result = 0;
	if ( !d_mapStream.getInt( &result ) )
	{
		throw std::out_of_range( "unexpected end of map file" );
	}
	return result;
}

void KDeserializator::getPoint( SPoint* point )
{
	point->x = getInt();
	point->y = getInt();
}

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

/*
	read binary file 
	format (assumption int == 4 bytes):

	int raw_segments_count
	segment {
		int road_class
		int points_count
		point {
			int x1, y1
		} * points_count
	} * raw_segments_count
*/
class KMapReader
{
	public:
		KMapReader( const SReaderData& data );

	public:
		bool run();

	private:
		void readSegment();
		bool readPoints( points_t* points );

		void updateRoadClasses( const int roadClass );

		void updateAreaDims( const points_t& points );
		void updateAreaDimsByPoint( const SPoint& point );
		void updateAreaDimByPos( const int pos, int* dimMin, int* dimMax );

	private:
		KDeserializator d_input;
		SRect& d_mapRect;
		bools_t& d_roadClasses;
		raw_segments_t& d_segments;

};

// ----------------------------------------------------------------------------

KMapReader::KMapReader( const SReaderData& data )
	: d_input( data.d_mapStream )
	, d_mapRect( *data.d_mapRect )
	, d_roadClasses( *data.d_roadClasses )
	, d_segments( *data.d_segments )
{	
	d_mapRect = SRect( 
		std::numeric_limits< coord_t >::max()
		, std::numeric_limits< coord_t >::max()
		, std::numeric_limits< coord_t >::min() 
		, std::numeric_limits< coord_t >::min() );
}

bool KMapReader::run()
{
	const std::size_t segmentsCount = d_input.getInt();
	assert( segmentsCount <= KSegmentsManager::maxSegmentCount() );
	d_segments.reserve( segmentsCount );
	for ( std::size_t i = 0
		; i < segmentsCount 
		; ++i )
	{
		readSegment();
	}

	const bool result = !d_segments.empty();
	return result;
}

// ----------------------------------------------------------------------------

void KMapReader::readSegment()
{
	const int roadClass = d_input.getInt();
	assert( roadClass <= consts::MaxRoadClassIndex );
	d_segments.push_back( SRawSegment( roadClass ) );
	updateRoadClasses( roadClass );

	SRawSegment& segment = d_segments.back();
	points_t& segmentPoints = segment.d_points;
	if ( readPoints( &segmentPoints ) )
	{
		assert( ( 1 < segmentPoints.size() ) 
			&& ( segmentPoints.size() <= KSegmentsManager::maxSegmentPointsCount() ) );
		updateAreaDims( segmentPoints );
	}
}

bool KMapReader::readPoints( points_t* points )
{
	const int pointsCount = d_input.getInt();
	points->reserve( pointsCount );
	SPoint point;
	for ( int i = 0; i < pointsCount; ++i )
	{
		d_input.getPoint( &point );
		if ( points->empty()
			|| ( point.x != points->back().x ) 
			|| ( point.y != points->back().y ) )
		{
			assert( points->empty() 
				|| utils::checkSectionLength( points->back(), point ) );
			points->push_back( point );
		}
	}

	const bool result = ( 1 < points->size() );
	return result;
}

// ----------------------------------------------------------------------------

void KMapReader::updateRoadClasses( const int roadClass )
{
	const std::size_t newSize = roadClass + 1;
	if ( d_roadClasses.size() < newSize )
		d_roadClasses.resize( newSize );
	d_roadClasses[ roadClass ] = true;
}

void KMapReader::updateAreaDims( const points_t& points )
{
	for ( points_cit it = points.begin()
		; it != points.end()
		; ++it )
	{
		const SPoint& point = *it;
		updateAreaDimsByPoint( point );
	}
}

void KMapReader::updateAreaDimsByPoint( const SPoint& point )
{
	const coord_t x = point.x;
	updateAreaDimByPos( x, &d_mapRect.left, &d_mapRect.right );

	const coord_t y = point.y;
	updateAreaDimByPos( y, &d_mapRect.top, &d_mapRect.bottom );
}

void KMapReader::updateAreaDimByPos( 
	const coord_t pos, 
	coord_t* dimMin, 
	coord_t* dimMax )
{
	if ( pos < ( *dimMin ) )
		*dimMin = pos;
	else if ( ( *dimMax ) < pos )
		*dimMax = pos;
}

} // anonymous namespace

// ----------------------------------------------------------------------------

SReaderData::SReaderData(
	IMapStream* mapStream,
	SRect* mapRect,
	bools_t* roadClasses,
	raw_segments_t* segments )
	: d_mapStream( mapStream )
	, d_mapRect( mapRect )
	, d_roadClasses( roadClasses )
	, d_segments( segments )
{
}

bool readMap( const SReaderData& data )
{
	KMapReader reader( data );
	const bool result = reader.run();
	return result;
}

} // namespace be
