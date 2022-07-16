// author: marines marinesovitch alias Darek Slusarczyk 2012-2013
#include "ph.h"
#include "beContentsGenerator.h"
#include "beInternalDocument.h"
#include "beBitmap.h"
#include "beUtils.h"
#include "beConsts.h"

namespace be
{

namespace
{

class KPainter;

typedef void (KPainter::*TDrawJunctionFunc)( const SPoint& junction );

typedef std::pair< EOrientation, EOrientation > orientation_pair_t;
typedef std::map< orientation_pair_t, TDrawJunctionFunc > orientations2junction_t;
typedef orientations2junction_t::const_iterator orientations2junction_cit;

// ----------------------------------------------------------------------------

const coord_t ClipCatchTolerance = 16;
const coord_t SplitPointTolerance = 2;

// ----------------------------------------------------------------------------

int calcRowShift( const int columns )
{
	assert( 0 < columns );
	int result = 0;

	int value = columns;
	while ( value != 0 )
	{
		++result;
		value >>= 1;
	}

	assert( result != 0 );
	return result;
}

// ----------------------------------------------------------------------------

bool isSectionNormalized( const SPoint& begin, const SPoint& end )
{
	const bool result = ( begin.x < end.x ) 
		|| ( ( begin.x == end.x ) && ( begin.y <= end.y ) );
	return result;
}

void normalizeSection( SPoint* begin, SPoint* end )
{
	if ( !isSectionNormalized( *begin, *end ) )
		std::swap( *begin, *end );
	assert( isSectionNormalized( *begin, *end ) );
}

// ----------------------------------------------------------------------------

SRect prepareClipScreenRect( const SContentsGeneratorData* generatorData )
{
	const SSize& screenSize = generatorData->d_viewData.d_screenSize;
	const coord_t screenWidth = screenSize.width;
	const coord_t screenHeight = screenSize.height;

	assert( ( consts::MinScreenDim <= screenSize.width )
		&& ( consts::MinScreenDim <= screenSize.height ) );

	const coord_t left = 0;
	const coord_t top = 0;
	const coord_t right = screenWidth - 1;
	const coord_t bottom = screenHeight - 1;
	assert( ( left <= right ) && ( top <= bottom ) );
	const SRect result( left, top, right, bottom );
	return result;
}

// ----------------------------------------------------------------------------

int calcRoadClassFilter( const SContentsGeneratorData* generatorData )
{
	int roadClassFilter = 0;
	const int zoomFactor = generatorData->d_viewData.d_zoomFactor;
	const int MinFilterZoomLevel = consts::MaxZoomFactor - ( 2 * consts::MaxRoadClassIndex );
	assert( 0 < MinFilterZoomLevel );
	assert( consts::MinZoomFactor <= MinFilterZoomLevel );
	if ( MinFilterZoomLevel < zoomFactor )
	{
		const int num = consts::MaxRoadClassIndex * ( zoomFactor - MinFilterZoomLevel );
		const int denom = consts::MaxZoomFactor - MinFilterZoomLevel;
		roadClassFilter = num / denom;
		const int rem = num % denom;
		if ( 0 < rem )
			++roadClassFilter;
	}
	assert( ( 0 <= roadClassFilter ) && ( roadClassFilter <= consts::MaxRoadClassIndex ) );
	return roadClassFilter;
}

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

class KPixelArray
{
	public:
		KPixelArray( const SSize& deviceSize, const color_t bkColor );
		~KPixelArray();

	public:
		void putPixel( const coord_t x, const coord_t y );
		void putOutlinePixel( const coord_t x, const coord_t y );

		void setColor( const color_t color );
		void setOutlineColor( const color_t outlineColor );

	public:
		void fillBackground();
		void dump( color_t* dest ) const;

	private:
		bool checkPixelPos( const coord_t x, const coord_t y ) const;
		int calcIndex( const coord_t x, const coord_t y ) const;

		const int d_deviceRows;
		const int d_deviceColumns;

		//needed for quick indexing with use of shift instead of multiply
		const int d_deviceRowShift;
		const int d_alignedColumns;

		color_t* d_pixels;

		const color_t d_bkColor;

		color_t d_color;
		color_t d_outlineColor;

};

// ----------------------------------------------------------------------------

KPixelArray::KPixelArray( 
	const SSize& deviceSize,
	const color_t bkColor )
	: d_deviceRows( deviceSize.height )
	, d_deviceColumns( deviceSize.width )
	, d_deviceRowShift( calcRowShift( d_deviceColumns ) )
	, d_alignedColumns( 1 << d_deviceRowShift ) 
	, d_pixels( new color_t[ d_deviceRows * d_alignedColumns ] )
	, d_bkColor( bkColor )
	, d_color( 0 )
	, d_outlineColor( 0 )
{
	assert( d_deviceRowShift < ( sizeof( d_deviceRows ) * 8 ) );
	assert( d_deviceColumns <= d_alignedColumns );
	memset( d_pixels, 0, d_deviceRows * d_alignedColumns * sizeof( color_t ) );
}

KPixelArray::~KPixelArray()
{
	delete[] d_pixels;
}

// ----------------------------------------------------------------------------

inline void KPixelArray::putPixel( 
	const coord_t x, 
	const coord_t y )
{
	if ( checkPixelPos( x, y ) )
	{
		const int index = calcIndex( x, y );
		d_pixels[ index ] = d_color;
	}
}

inline void KPixelArray::putOutlinePixel(
	const coord_t x, 
	const coord_t y )
{
	if ( checkPixelPos( x, y ) )
	{
		const int index = calcIndex( x, y );
		const color_t currentColor = d_pixels[ index ];
		if ( currentColor != d_color )
			d_pixels[ index ] = d_outlineColor;
	}
}

inline void KPixelArray::setColor( const color_t color )
{
	d_color = color;
}

inline void KPixelArray::setOutlineColor( const color_t outlineColor )
{
	d_outlineColor = outlineColor;
}

// ----------------------------------------------------------------------------

void KPixelArray::fillBackground()
{
	color_t* currentRow = d_pixels;
	for ( int i = 0; i < d_deviceRows; ++i )
	{
		color_t* currentPixel = currentRow;
		for ( int i = 0; i < d_deviceColumns; ++i, ++currentPixel )
		{
			if ( *currentPixel == 0 )
				*currentPixel = d_bkColor;
		}
		currentRow += d_alignedColumns;
	}
}

void KPixelArray::dump( color_t* dest ) const
{
	if ( d_alignedColumns != d_deviceColumns )
	{
		color_t* srcRow = d_pixels;
		color_t* destRow = dest;
		const int rowLength = d_deviceColumns * sizeof( color_t );
		for ( int i = 0; i < d_deviceRows; ++i )
		{
			memcpy( destRow, srcRow, rowLength );
			srcRow += d_alignedColumns;
			destRow += d_deviceColumns;
		}
	}
	else
	{
		memcpy( dest, d_pixels, d_deviceRows * d_deviceColumns );
	}
}

// ----------------------------------------------------------------------------

inline bool KPixelArray::checkPixelPos( const coord_t x, const coord_t y ) const
{
	const bool result 
		= utils::isValueInRange( 0, x, d_deviceColumns, false )
		&& utils::isValueInRange( 0, y, d_deviceRows, false );
	return result;
}

inline int KPixelArray::calcIndex( const coord_t x, const coord_t y ) const
{
	const int index = ( y << d_deviceRowShift ) + x;
	return index;
}

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

class KPainter
{
	public:
		KPainter( 
			const SSize& deviceSize, 
			const SSize& screenSize, 
			const color_t bkColor );

	public:
		void dump( IBitmap* bitmap );

		void drawSection(
			const SPoint& begin,
			const SPoint& end,
			const SRoadClass* roadClass,
			const EOrientation orientation );

	private:
		typedef void (KPainter::*TPutPixelFunc)( const coord_t x, const coord_t y );

		void drawLine( 
			const EOrientation orientation,
			TPutPixelFunc horzPutPixelFunc, 
			TPutPixelFunc vertPutPixelFunc );

		void drawHorzLine(
			const coord_t x0,
			const coord_t x1,
			const coord_t y,
			TPutPixelFunc putPixelFunc );
		void drawVertLine( 
			const coord_t y0,
			const coord_t y1,
			const coord_t x,
			TPutPixelFunc putPixelFunc );
		void drawInclinedLine( TPutPixelFunc putPixelFunc );
		void drawHorzInclinedLine( 
			const coord_t dx,
			const coord_t dy,
			const coord_t xi,
			const coord_t yi,
			TPutPixelFunc putPixelFunc );
		void drawVertInclinedLine( 
			const coord_t dx,
			const coord_t dy,
			const coord_t xi,
			const coord_t yi,
			TPutPixelFunc putPixelFunc );

	private:
		void initOrientations2junction();
		void addOrientations2junction(
			const EOrientation first,
			const EOrientation second,
			TDrawJunctionFunc drawJunctionFunc );

		void drawJunction(
			const SRoadClass* roadClass,
			const EOrientation orientation );

		bool isJunctionByDefault( const orientation_pair_t prev2current ) const;

		bool findJunctionPoint( SPoint* junctionPoint ) const;

		void drawJunctionRect( const SPoint& junction );
		void fillJunctionRect(
			const coord_t x0,
			const coord_t y0,
			const coord_t x1,
			const coord_t y1 );

		void drawJunctionDiamond( const SPoint& junction );
		void drawRightInclinedLine(
			const coord_t x0,
			const coord_t y0,
			const coord_t x1,
			TPutPixelFunc putPixelFunc );
		void drawLeftInclinedLine(
			const coord_t x0,
			const coord_t y0,
			const coord_t x1,
			TPutPixelFunc putPixelFunc );

	private:
		void putSinglePixel( const coord_t x, const coord_t y );
		void putSingleOutlinePixel( const coord_t x, const coord_t y );

		void putHorzThickPixel( const coord_t x, const coord_t y );
		void putVertThickPixel( const coord_t x, const coord_t y );

		void putHorzThickPixelWithOutline( const coord_t x, const coord_t y );
		void putVertThickPixelWithOutline( const coord_t x, const coord_t y );

	private:
		void prepareInclinedSection();
		coord_t calcInclinedSectionEndPos(
			const coord_t rawEndPos,
			const coord_t step,
			const coord_t screenDim ) const;

		bool isPixelInScreenArea( const coord_t x, const coord_t y ) const;

	private:
		const coord_t d_screenWidth;
		const coord_t d_screenHeight;

		KPixelArray d_pixelArray;

		// current context
		coord_t d_x0;
		coord_t d_y0;
		coord_t d_x1;
		coord_t d_y1;

		coord_t d_beginOffset;
		coord_t d_endOffset;

		coord_t d_outlineThickness;

		// prev section context
		SPoint d_prevBegin;
		SPoint d_prevEnd;
		EOrientation d_prevOrientation;

	private:
		static orientations2junction_t s_orientations2junction;

};

orientations2junction_t KPainter::s_orientations2junction;

// ----------------------------------------------------------------------------

KPainter::KPainter( 
	const SSize& deviceSize, 
	const SSize& screenSize, 
	const color_t bkColor )
	: d_screenWidth( screenSize.width )
	, d_screenHeight( screenSize.height )
	, d_pixelArray( deviceSize, bkColor )
	, d_x0( 0 )
	, d_y0( 0 )
	, d_x1( 0 )
	, d_y1( 0 )
	, d_beginOffset( 0 )
	, d_endOffset( 0 )
	, d_outlineThickness( 0 )
	, d_prevOrientation( UnknownOrientation )
{
	initOrientations2junction();
}

void KPainter::dump( IBitmap* bitmap )
{
	color_t* buffer = 0;
	if ( bitmap->lock( &buffer ) )
	{
		d_pixelArray.fillBackground();
		d_pixelArray.dump( buffer );
		bitmap->unlock();
	}
}

void KPainter::drawSection(
	const SPoint& begin,
	const SPoint& end,
	const SRoadClass* roadClass,
	const EOrientation orientation )
{
	assert( isSectionNormalized( begin, end ) );

	const color_t color = roadClass->d_color;
	d_pixelArray.setColor( color );

	d_x0 = begin.x;
	d_y0 = begin.y;
	d_x1 = end.x;
	d_y1 = end.y;

	const coord_t thickness = roadClass->d_fullThickness;
	if ( thickness == 1 )
	{
		d_beginOffset = 0;
		d_endOffset = 0;
		drawLine( 
			orientation, 
			&KPainter::putSinglePixel, 
			&KPainter::putSinglePixel );
	}
	else
	{
		assert( 1 < thickness );
		d_beginOffset = - ( thickness >> 1 );
		d_endOffset = d_beginOffset + thickness;
		d_outlineThickness = roadClass->d_outlineThickness;
		if ( roadClass->hasOutline() )
		{
			const color_t outlineColor = roadClass->d_outlineColor;
			d_pixelArray.setOutlineColor( outlineColor );

			drawLine(
				orientation,
				&KPainter::putHorzThickPixelWithOutline, 
				&KPainter::putVertThickPixelWithOutline );
			drawJunction( roadClass, orientation );
		}
		else
		{
			drawLine(
				orientation,
				&KPainter::putHorzThickPixel, 
				&KPainter::putVertThickPixel );
			drawJunction( roadClass, orientation );
		}
	}

	// store context
	d_prevBegin = begin;
	d_prevEnd = end;
	d_prevOrientation = orientation;
}

void KPainter::drawLine( 
	const EOrientation orientation,
	TPutPixelFunc horzPutPixelFunc, 
	TPutPixelFunc vertPutPixelFunc )
{
	switch ( orientation )
	{
		case Horizontal:
			drawHorzLine( d_x0, d_x1, d_y0, horzPutPixelFunc );
			break;

		case Vertical:
			drawVertLine( d_y0, d_y1, d_x0, vertPutPixelFunc );
			break;

		case InclinedHorizontal:
			drawInclinedLine( horzPutPixelFunc );
			break;

		case InclinedVertical:
			drawInclinedLine( vertPutPixelFunc );
			break;

		default:
			assert( !"unexpected orientation" );
	}
}

void KPainter::drawHorzLine( 
	const coord_t x0,
	const coord_t x1,
	const coord_t y,
	TPutPixelFunc putPixelFunc )
{
	assert( x0 <= x1 );
	for ( coord_t x = x0; x <= x1; ++x )
	{
		( this->*putPixelFunc )( x, y );
	}
}

void KPainter::drawVertLine( 
	const coord_t y0,
	const coord_t y1,
	const coord_t x,
	TPutPixelFunc putPixelFunc )
{
	assert( y0 <= y1 );
	for ( coord_t y = y0; y <= y1; ++y )
	{
		( this->*putPixelFunc )( x, y );
	}
}

void KPainter::drawInclinedLine( TPutPixelFunc putPixelFunc )
{
	prepareInclinedSection();

	coord_t dx = 0;
	coord_t dy = 0;

	coord_t xi = 0;
	coord_t yi = 0;

	if ( d_x0 < d_x1 )
	{ 
		dx = d_x1 - d_x0;
		xi = 1;
	} 
	else
	{ 
		dx = d_x0 - d_x1;
		xi = -1;
	}
     
	if ( d_y0 < d_y1 )
	{ 
		dy = d_y1 - d_y0;
		yi = 1;
	} 
	else
	{ 
		dy = d_y0 - d_y1;
		yi = -1;
	}

	assert( ( 0 < dx ) && ( 0 < dy ) );

	if ( dy < dx )
		drawHorzInclinedLine( dx, dy, xi, yi, putPixelFunc );
	else
		drawVertInclinedLine( dx, dy, xi, yi, putPixelFunc );
}

void KPainter::drawHorzInclinedLine( 
	const coord_t dx,
	const coord_t dy,
	const coord_t xi,
	const coord_t yi,
	TPutPixelFunc putPixelFunc )
{
	const coord_t dd = ( dy - dx ) << 1;
	const coord_t dp = dy << 1;

	coord_t di = dp - dx;

	coord_t x = d_x0;
	coord_t y = d_y0;

	const coord_t x_end = calcInclinedSectionEndPos( d_x1, xi, d_screenWidth );

	if ( ( ( 0 < xi ) && ( x < x_end ) ) 
		|| ( ( xi < 0 ) && ( x_end < x ) ) )
	{
		do
		{
			( this->*putPixelFunc )( x, y );

			x += xi;
			if ( 0 <= di )
			{ 
				y += yi;
				di += dd;
			} 
			else
			{
				di += dp;
			}
		}
		while ( x != x_end );
	}
}

void KPainter::drawVertInclinedLine( 
	const coord_t dx,
	const coord_t dy,
	const coord_t xi,
	const coord_t yi,
	TPutPixelFunc putPixelFunc )
{
	const coord_t dd = ( dx - dy ) << 1;
	const coord_t dp = dx << 1;

	coord_t di = dp - dy;

	coord_t x = d_x0;
	coord_t y = d_y0;

	const coord_t y_end = calcInclinedSectionEndPos( d_y1, yi, d_screenHeight );

	if ( ( ( 0 < yi ) && ( y < y_end ) ) 
		|| ( ( yi < 0 ) && ( y_end < y ) ) )
	{
		do
		{
			( this->*putPixelFunc )( x, y );

			y += yi;
			if ( 0 <= di )
			{ 
				x += xi;
				di += dd;
			} 
			else
			{
				di += dp;
			}
		}
		while ( y != y_end );
	}
}

// ----------------------------------------------------------------------------

void KPainter::initOrientations2junction()
{
	if ( s_orientations2junction.empty() )
	{
//		addOrientations2junction( Horizontal, Vertical, &KPainter::drawJunctionRect );
		addOrientations2junction( Horizontal, Vertical, &KPainter::drawJunctionDiamond );
		addOrientations2junction( Horizontal, InclinedVertical, &KPainter::drawJunctionDiamond );
		addOrientations2junction( Vertical, InclinedHorizontal, &KPainter::drawJunctionDiamond );
		addOrientations2junction( InclinedHorizontal, InclinedVertical, &KPainter::drawJunctionDiamond );
		addOrientations2junction( InclinedHorizontal, InclinedHorizontal, &KPainter::drawJunctionDiamond );
		addOrientations2junction( InclinedVertical, InclinedVertical, &KPainter::drawJunctionDiamond );
	}
}

void KPainter::addOrientations2junction(
	const EOrientation first,
	const EOrientation second,
	TDrawJunctionFunc drawJunctionFunc )
{
	s_orientations2junction[ std::make_pair( first, second ) ] = drawJunctionFunc;
	s_orientations2junction[ std::make_pair( second, first ) ] = drawJunctionFunc;
}

// ----------------------------------------------------------------------------

void KPainter::drawJunction(
	const SRoadClass* roadClass,
	const EOrientation orientation )
{
	const orientation_pair_t prev2current = std::make_pair( d_prevOrientation, orientation );
	orientations2junction_cit it = s_orientations2junction.find( prev2current );
	if ( it != s_orientations2junction.end() )
	{
		TDrawJunctionFunc drawJunctionFunc = it->second;
		SPoint junctionPoint;
		if ( findJunctionPoint( &junctionPoint ) )
		{
			( this->*drawJunctionFunc )( junctionPoint );
		}
	}
	else
	{
		assert( isJunctionByDefault( prev2current ) );
	}
}

bool KPainter::isJunctionByDefault( const orientation_pair_t prev2current ) const
{
	typedef std::set< orientation_pair_t > orientationset_t;
	typedef orientationset_t::const_iterator orientationset_cit;

	static orientationset_t s_orientationset;
	if ( s_orientationset.empty() )
	{
		s_orientationset.insert( std::make_pair( Horizontal, Horizontal ) );
		s_orientationset.insert( std::make_pair( Horizontal, InclinedHorizontal ) );
		s_orientationset.insert( std::make_pair( InclinedHorizontal, Horizontal ) );
		s_orientationset.insert( std::make_pair( Vertical, Vertical ) );
		s_orientationset.insert( std::make_pair( Vertical, InclinedVertical ) );
		s_orientationset.insert( std::make_pair( InclinedVertical, Vertical ) );
	}

	orientationset_cit it = s_orientationset.find( prev2current );
	const bool result = ( it != s_orientationset.end() ) || ( prev2current.first == UnknownOrientation );
	return result;
}

bool KPainter::findJunctionPoint( SPoint* junctionPoint ) const
{
	bool result = true;

	const SPoint begin( d_x0, d_y0 );
	const SPoint end( d_x1, d_y1 );

	if ( d_prevBegin == begin )
		*junctionPoint = begin;
	else if ( d_prevBegin == end )
		*junctionPoint = end;
	else if ( d_prevEnd == begin )
		*junctionPoint = begin;
	else if ( d_prevEnd == end )
		*junctionPoint = end;
	else
		result = false;

	return result;
}

void KPainter::drawJunctionRect( const SPoint& junction )
{
	coord_t x0 = junction.x + d_beginOffset;
	coord_t y0 = junction.y + d_beginOffset;
	coord_t x1 = junction.x + d_endOffset - 1;
	coord_t y1 = junction.y + d_endOffset - 1;
	for ( coord_t i = 0
		; i < d_outlineThickness
		; ++i, ++x0, ++y0, --x1, --y1 )
	{
		drawHorzLine( x0, x1, y0, &KPainter::putSingleOutlinePixel );
		drawHorzLine( x0, x1, y1, &KPainter::putSingleOutlinePixel );
		drawVertLine( y0, y1, x0, &KPainter::putSingleOutlinePixel );
		drawVertLine( y0, y1, x1, &KPainter::putSingleOutlinePixel );
	}
	fillJunctionRect( x0, y0, x1, y1 );
}

void KPainter::fillJunctionRect(
	const coord_t x0,
	const coord_t y0,
	const coord_t x1,
	const coord_t y1 )
{
	for ( coord_t y = y0; y <= y1; ++y )
	{
		drawHorzLine( x0, x1, y, &KPainter::putSinglePixel );
	}
}

void KPainter::drawJunctionDiamond( const SPoint& junction )
{
	const coord_t junction_x = junction.x;
	const coord_t junction_y = junction.y;
	coord_t x0 = junction_x + d_beginOffset;
	coord_t y0 = junction_y + d_beginOffset;
	coord_t x1 = junction_x + d_endOffset - 1;
	coord_t y1 = junction_y + d_endOffset - 1;
	for ( coord_t i = 0
		; x0 != junction_x
		; ++i, ++x0, ++y0, --x1, --y1 )
	{
		TPutPixelFunc putPixelFunc 
			= ( i < d_outlineThickness )
				? &KPainter::putSingleOutlinePixel : &KPainter::putSinglePixel;
		drawRightInclinedLine( x0, junction_y, junction_x, putPixelFunc );
		drawLeftInclinedLine( junction_x, y0, x1, putPixelFunc );
		drawRightInclinedLine( junction_x, y1, x1, putPixelFunc );
		drawLeftInclinedLine( x0, junction_y, junction_x, putPixelFunc );
	}
}

void KPainter::drawRightInclinedLine(
	const coord_t x0,
	const coord_t y0,
	const coord_t x1,
	TPutPixelFunc putPixelFunc )
{
	for ( coord_t x = x0, y = y0
		; x <= x1
		; ++x, --y )
	{
		( this->*putPixelFunc )( x, y );
	}
}

void KPainter::drawLeftInclinedLine(
	const coord_t x0,
	const coord_t y0,
	const coord_t x1,
	TPutPixelFunc putPixelFunc )
{
	for ( coord_t x = x0, y = y0
		; x <= x1
		; ++x, ++y )
	{
		( this->*putPixelFunc )( x, y );
	}
}

// ----------------------------------------------------------------------------

inline void KPainter::putSinglePixel( 
	const coord_t x, 
	const coord_t y )
{
	d_pixelArray.putPixel( x, y );
}

inline void KPainter::putSingleOutlinePixel( 
	const coord_t x, 
	const coord_t y )
{
	d_pixelArray.putOutlinePixel( x, y );
}

void KPainter::putHorzThickPixel(
	const coord_t x, 
	const coord_t init_y )
{
	const coord_t begin_y = std::max( 0, init_y + d_beginOffset );
	const coord_t end_y = std::min( init_y + d_endOffset, d_screenHeight );

	for ( coord_t y = begin_y; y < end_y; ++y )
	{
		putSinglePixel( x, y );
	}
}

void KPainter::putVertThickPixel(
	const coord_t init_x, 
	const coord_t y )
{
	const coord_t begin_x = std::max( 0, init_x + d_beginOffset );
	const coord_t end_x = std::min( init_x + d_endOffset, d_screenWidth );

	for ( coord_t x = begin_x; x < end_x; ++x )
	{
		putSinglePixel( x, y );
	}
}

void KPainter::putHorzThickPixelWithOutline(
	const coord_t x, 
	const coord_t init_y )
{
	const coord_t raw_begin_y = init_y + d_beginOffset;
	const coord_t raw_end_y = init_y + d_endOffset;
	const coord_t end_1st_outline_y = raw_begin_y + d_outlineThickness;
	const coord_t begin_2nd_outline_y = raw_end_y - d_outlineThickness;
	const coord_t begin_y = std::max( 0, raw_begin_y );
	const coord_t end_y = std::min( raw_end_y, d_screenHeight );

	for ( coord_t y = begin_y; y < end_y; ++y )
	{
		if ( ( end_1st_outline_y <= y ) && ( y < begin_2nd_outline_y ) )
			putSinglePixel( x, y );
		else
			putSingleOutlinePixel( x, y );
	}
}

void KPainter::putVertThickPixelWithOutline(
	const coord_t init_x, 
	const coord_t y )
{
	const coord_t raw_begin_x = init_x + d_beginOffset;
	const coord_t raw_end_x = init_x + d_endOffset;
	const coord_t end_1st_outline_x = raw_begin_x + d_outlineThickness;
	const coord_t begin_2nd_outline_x = raw_end_x - d_outlineThickness;
	const coord_t begin_x = std::max( 0, raw_begin_x );
	const coord_t end_x = std::min( raw_end_x , d_screenWidth );

	for ( coord_t x = begin_x; x < end_x; ++x )
	{
		if ( ( end_1st_outline_x <= x ) && ( x < begin_2nd_outline_x ) )
			putSinglePixel( x, y );
		else
			putSingleOutlinePixel( x, y );
	}
}

// ----------------------------------------------------------------------------

void KPainter::prepareInclinedSection()
{
	if ( !isPixelInScreenArea( d_x0, d_y0 ) && isPixelInScreenArea( d_x1, d_y1 ) )
	{
		// if begin is outside of screen and end is inside
		// then swap points
		std::swap( d_x0, d_x1 );
		std::swap( d_y0, d_y1 );
	}
}

coord_t KPainter::calcInclinedSectionEndPos( 
	const coord_t rawEndPos,
	const coord_t step,
	const coord_t screenDim ) const
{
	/*
		d_beginOffset/d_endOffset are used, so center of section may
		pass through edge of screen, but it is needed to correctly
		draw inclined section with thickness greater than 1, else at
		the end of section it would look like cut on one side
	*/
	const coord_t result
		= ( 0 < step )
			? std::min( rawEndPos, screenDim + d_endOffset )
			: std::max( d_beginOffset, rawEndPos );
	return result;
}

inline bool KPainter::isPixelInScreenArea( const coord_t x, const coord_t y ) const
{
	const bool result 
		= utils::isValueInRange( 0, x, d_screenWidth, false )
		&& utils::isValueInRange( 0, y, d_screenHeight, false );
	return result;
}

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

coord_t prepareMinCatchCoord( 
	const coord_t clipPos,
	const coord_t clipTolerance )
{
	coord_t result = 0;
	if ( consts::MinCoord + clipTolerance < clipPos)
		result = clipPos - clipTolerance;
	else 
		result = consts::MinCoord;
	return result;
}

coord_t prepareMaxCatchCoord( 
	const coord_t clipPos,
	const coord_t clipTolerance )
{
	coord_t result = 0;
	if ( clipPos < consts::MaxCoord - clipTolerance )
		result = clipPos + clipTolerance;
	else 
		result = consts::MaxCoord;
	return result;
}

SRect prepareClipRect( 
	const SRect& clipRect,
	const coord_t clipTolerance )
{
	const coord_t left = prepareMinCatchCoord( clipRect.left, clipTolerance );
	const coord_t top = prepareMinCatchCoord( clipRect.top, clipTolerance );
	const coord_t right = prepareMaxCatchCoord( clipRect.right, clipTolerance );
	const coord_t bottom = prepareMaxCatchCoord( clipRect.bottom, clipTolerance );

	const SRect result( left, top, right, bottom );
	return result;
}

// ----------------------------------------------------------------------------

class KClipSection
{
	public:
		KClipSection(
			const SRect& clipRect,
			const int clipExtraTolerance,
			SPoint* begin,
			SPoint* end );

	public:
		bool run( const EOrientation orientation );

	private:
		bool clipHorizontalSection();
		bool clipVerticalSection();
		bool clipInclinedSection();

		bool findSplitPoint( SPoint* splitPoint );
		SPoint clip( const SPoint& begin, const SPoint& end );

		void clearRestFlags();

		void calcCenter( 
			const SPoint& begin, 
			const SPoint& end, 
			SPoint* middle );
		coord_t calcCenterCoord(
			const coord_t begin, 
			const coord_t end, 
			bool* restFlag );

		void clipCoord(
			const coord_t minPos, 
			const coord_t maxPos, 
			coord_t* coord ) const;

	private:
		const SRect d_clipRect;
		const SRect d_catchRect;

		const coord_t d_left;
		const coord_t d_top; 
		const coord_t d_right;
		const coord_t d_bottom;

		bool d_restFlagX;
		bool d_restFlagY;

		SPoint& d_begin;
		SPoint& d_end;

};

// ----------------------------------------------------------------------------

KClipSection::KClipSection(
	const SRect& clipRect,
	const int clipExtraTolerance,
	SPoint* begin,
	SPoint* end )
	: d_clipRect( prepareClipRect( clipRect, clipExtraTolerance ) )
	, d_catchRect( prepareClipRect( d_clipRect, ClipCatchTolerance ) )
	, d_left( d_clipRect.left )
	, d_top( d_clipRect.top ) 
	, d_right( d_clipRect.right )
	, d_bottom( d_clipRect.bottom )
	, d_restFlagX( false )
	, d_restFlagY( false )
	, d_begin( *begin )
	, d_end( *end )
{
	assert( isSectionNormalized( d_begin, d_end ) );
}

bool KClipSection::run( const EOrientation orientation )
{
	bool result = false;
	switch ( orientation)
	{
		case Horizontal:
			result = clipHorizontalSection();
			break;

		case Vertical:
			result = clipVerticalSection();
			break;

		case InclinedHorizontal:
		case InclinedVertical:
			result = clipInclinedSection();
			break;

		default:
			assert( !"unknown orientation!" );
	}
	return result;
}

// ----------------------------------------------------------------------------

bool KClipSection::clipHorizontalSection()
{
	const bool result = utils::isValueInRange( d_top, d_begin.y, d_bottom, true );
	if ( result )
	{
		clipCoord( d_left, d_right, &d_begin.x );
		clipCoord( d_left, d_right, &d_end.x );
		assert( d_clipRect.contains( d_begin )
			&& d_clipRect.contains( d_end ) );
	}
	return result;
}

bool KClipSection::clipVerticalSection()
{
	const bool result = utils::isValueInRange( d_left, d_begin.x, d_right, true );
	if ( result )
	{
		clipCoord( d_top, d_bottom, &d_begin.y );
		clipCoord( d_top, d_bottom, &d_end.y );
		assert( d_clipRect.contains( d_begin )
			&& d_clipRect.contains( d_end ) );
	}
	return result;
}

bool KClipSection::clipInclinedSection()
{
	bool result = true;
	bool normalize = false;
	if ( d_clipRect.contains( d_begin ) )
	{
		if ( d_clipRect.contains( d_end ) )
		{
			// do nothing - clipping is not needed
		}
		else
		{
			d_end = clip( d_begin, d_end );
			normalize = true;
		}
	}
	else 
	{
		if ( d_clipRect.contains( d_end ) )
		{
			d_begin = clip( d_end, d_begin );
			normalize = true;
		}
		else
		{
			SPoint splitPoint;
			if ( findSplitPoint( &splitPoint ) )
			{
				d_begin = clip( splitPoint, d_begin );
				d_end = clip( splitPoint, d_end );
				normalize = true;
			}
			else
			{
				result = false;
			}
		}
	}

	if ( normalize )
		normalizeSection( &d_begin, &d_end );

	return result;
}

bool KClipSection::findSplitPoint( SPoint* splitPoint )
{
	bool found = false;

	assert( !d_restFlagX && !d_restFlagY );

	assert( d_begin.x < d_end.x );
	const bool vertGrowing = d_begin.y < d_end.y;

	SPoint prevSplitPoint( consts::MaxCoord, consts::MaxCoord );

	bool search = true;
	do
	{
		calcCenter( d_begin, d_end, splitPoint );

		if ( splitPoint->x < d_left )
			d_begin = *splitPoint;
		else if ( d_right < splitPoint->x )
			d_end = *splitPoint;
		else if ( splitPoint->y < d_top )
			( vertGrowing ? d_begin : d_end ) = *splitPoint;
		else if ( d_bottom < splitPoint->y )
			( vertGrowing ? d_end : d_begin ) = *splitPoint;
		else 
		{
			assert( d_clipRect.contains( *splitPoint ) );
			found = true;
			search = false;
		}

		if ( ( std::abs( splitPoint->x - prevSplitPoint.x ) <= SplitPointTolerance ) 
			&& ( std::abs( splitPoint->y - prevSplitPoint.y ) <= SplitPointTolerance ) ) 
		{
			search = false;
		}
		else
		{
			prevSplitPoint = *splitPoint;
		}
	}
	while( search );

	return found;
}

SPoint KClipSection::clip( const SPoint& begin, const SPoint& end )
{
	clearRestFlags();

	SPoint tempBegin = begin;
	SPoint middle;
	SPoint newEnd = end;
	while ( !d_catchRect.contains( newEnd ) )
	{
		calcCenter( tempBegin, newEnd, &middle );
		if ( d_clipRect.contains( middle ) && !d_clipRect.isOnEdge( middle ) )
			tempBegin = middle;
		else
			newEnd = middle;
	}
	return newEnd;
}

void KClipSection::clearRestFlags()
{
	d_restFlagX = false;
	d_restFlagY = false;
}

void KClipSection::calcCenter( 
	const SPoint& begin, 
	const SPoint& end, 
	SPoint* middle )
{
	middle->x = calcCenterCoord( begin.x, end.x, &d_restFlagX );
	middle->y = calcCenterCoord( begin.y, end.y, &d_restFlagY );
}

coord_t KClipSection::calcCenterCoord(
	const coord_t begin, 
	const coord_t end, 
	bool* restFlag )
{
	const coord_t diff = end - begin;
	coord_t complementation = 0;
	if ( utils::is_odd( diff ) )
	{
		if ( *restFlag )
		{
			*restFlag = false;
			complementation = 1;
		}
		else
		{
			*restFlag = true;
		}
	}

	const coord_t result = begin + ( diff >> 1 ) + complementation;
	return result;
}

void KClipSection::clipCoord( 
	const coord_t minPos, 
	const coord_t maxPos, 
	coord_t* coord ) const
{
	if ( *coord < minPos )
		*coord = minPos;
	else if ( maxPos < *coord )
		*coord = maxPos;
	assert( utils::isValueInRange( minPos, *coord, maxPos, true ) );
}

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

struct zoom_out 
{
	zoom_out( const int zoomFactor ) : d_zoomFactor( zoomFactor )
	{
	}

	void operator()( SPoint* point ) const
	{
		point->x >>= d_zoomFactor;
		point->y >>= d_zoomFactor;
	}

	const int d_zoomFactor;

	static const SZoomData::EKind Kind = SZoomData::ZoomOut;

};

struct no_zoom
{
	no_zoom( const int /*zoomFactor*/ )
	{
	}

	void operator()( SPoint* /*point */) const
	{
		// do nothing
	}

	static const SZoomData::EKind Kind = SZoomData::ZoomNone;

};

struct zoom_in
{
	zoom_in( const int zoomFactor ) : d_zoomFactor( zoomFactor )
	{
	}

	void operator()( SPoint* point ) const
	{
		assert( !utils::isShiftOverflow( point->x, -d_zoomFactor ) );
		point->x <<= -d_zoomFactor;
		assert( !utils::isShiftOverflow( point->y, -d_zoomFactor ) );
		point->y <<= -d_zoomFactor;
	}

	const int d_zoomFactor;

	static const SZoomData::EKind Kind = SZoomData::ZoomIn;

};

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

class KContentsGenerator
{
	public:
		KContentsGenerator( SContentsGeneratorData* generatorData );
		~KContentsGenerator();

	private:
		void prepareRoadClasses( const road_classes_t& baseRoadClasses );
		SRoadClass* prepareRoadClass( const SRoadClass* baseRoadClass ) const;
		coord_t prepareRoadClassThickness( const coord_t defaultThickness ) const;
		coord_t prepareRoadClassOutlineThickness( const coord_t defaultThickness ) const;

	public:
		bool run( SContentsGeneratorData* generatorData );

	private:
		template< typename zoom_t >
		bool drawSections( SContentsGeneratorData* generatorData );

		bool filterSection( const SSection& section ) const;

		template< typename zoom_t >
		void drawSection();

	private:
		void initSection();

		template< typename zoom_t >
		bool preZoomStep();

		template< typename zoom_t >
		void zoomSection();

		template< typename zoom_t >
		bool postZoomStep();

	private:
		void initScreenPoints();
		void setRoadClass();
		void setSectionOrientation();
		bool updateSectionOrientation();
		bool isInclinedOrientation() const;
		EOrientation calcSectionOrientation() const;
		coord_t calcClipTolerance() const;

		void prepareScreenPoint( 
			const SPoint& viewportTopLeftCorner, 
			SPoint* point ) const;

		bool clipSection( const SRect& clipRect, const int clipExtraTolerance );

	private:
		const SRect d_viewportRect;
		const SPoint d_viewportTopLeft;
		SPoint d_zoomedViewportTopLeft;
		const SRect d_clipScreenRect;
		const int d_zoomFactor;
		const bool d_isZoomIn;
		road_classes_t d_roadClasses;
		const int d_roadClassFilter;
		KPainter d_painter;

		// context - current/prev section
		SSection d_section;
		const SRoadClass* d_roadClass;
		EOrientation d_orientation;

		SPoint d_begin;
		SPoint d_end;

};

// ----------------------------------------------------------------------------

KContentsGenerator::KContentsGenerator( SContentsGeneratorData* generatorData )
	: d_viewportRect( generatorData->d_viewportRect )
	, d_viewportTopLeft( d_viewportRect.getTopLeft() )
	, d_zoomedViewportTopLeft( d_viewportRect.getTopLeft() )
	, d_clipScreenRect( prepareClipScreenRect( generatorData ) )
	, d_zoomFactor( generatorData->d_viewData.d_zoomFactor )
	, d_isZoomIn( d_zoomFactor < 0 )
	, d_roadClassFilter( calcRoadClassFilter( generatorData ) )
	, d_painter( 
		generatorData->d_viewData.d_deviceSize, 
		generatorData->d_viewData.d_screenSize, 
		consts::BackgroundColor )
	, d_roadClass( 0 )
	, d_orientation( UnknownOrientation )
{
	const IInternalDocument& document = generatorData->d_document;
	const road_classes_t& baseRoadClasses = document.getBaseRoadClasses();
	prepareRoadClasses( baseRoadClasses );
}

KContentsGenerator::~KContentsGenerator()
{
	utils::delete_container( d_roadClasses.begin(), d_roadClasses.end() );
}

void KContentsGenerator::prepareRoadClasses( 
	const road_classes_t& baseRoadClasses )
{
	for ( road_classes_cit it = baseRoadClasses.begin()
		; it != baseRoadClasses.end()
		; ++it )
	{
		const SRoadClass* baseRoadClass = *it;
		SRoadClass* roadClass = prepareRoadClass( baseRoadClass );
		d_roadClasses.push_back( roadClass );
	}
}

SRoadClass* KContentsGenerator::prepareRoadClass( 
	const SRoadClass* baseRoadClass ) const
{
	SRoadClass* roadClass = 0;
	if ( baseRoadClass != 0 )
	{
		const coord_t thickness 
			= prepareRoadClassThickness( baseRoadClass->d_thickness );
		const coord_t outlineThickness 
			= prepareRoadClassOutlineThickness( baseRoadClass->d_outlineThickness );
		if ( 0 < outlineThickness )
		{
			roadClass = new SRoadClass( 
				thickness,
				baseRoadClass->d_color,
				outlineThickness, 
				baseRoadClass->d_outlineColor );
		}
		else
		{
			roadClass = new SRoadClass( 
				thickness,
				baseRoadClass->d_color );
		}
	}
	return roadClass;
}

coord_t KContentsGenerator::prepareRoadClassThickness( const coord_t defaultThickness ) const
{
	assert( 0 < defaultThickness );
	coord_t thickness = defaultThickness;
	if ( d_zoomFactor < 0 )
	{
		thickness = ( -d_zoomFactor + 1 ) * defaultThickness;
	}
	else if ( 0 < d_zoomFactor )
	{
		const int ReductionZoomFactor = 1;
		if ( ReductionZoomFactor < d_zoomFactor )
		{
			const coord_t reduction = ( d_zoomFactor - ReductionZoomFactor ) / 2;
			thickness = std::max( 1, defaultThickness - reduction );
		}
	}
	assert( 0 < thickness );
	return thickness;
}

coord_t KContentsGenerator::prepareRoadClassOutlineThickness( const coord_t defaultThickness ) const
{
	coord_t thickness = defaultThickness;
	if ( defaultThickness != 0 )
	{
		if ( d_zoomFactor < 0 )
		{
			thickness = -d_zoomFactor + 1;
		}
		else if ( d_zoomFactor < 0 )
		{
			const int MaxOutlineZoomFactor = 14;
			if ( MaxOutlineZoomFactor < d_zoomFactor )
			{
				const coord_t reduction = d_zoomFactor - MaxOutlineZoomFactor;
				thickness = std::max( 0, defaultThickness - reduction );
			}
		}
	}
	assert( 0 <= thickness );
	return thickness;
}

// ----------------------------------------------------------------------------

bool KContentsGenerator::run( SContentsGeneratorData* generatorData )
{
	bool result = false;

	if ( 0 < d_zoomFactor )
		result = drawSections< zoom_out >( generatorData );
	else if ( d_zoomFactor < 0 )
		result = drawSections< zoom_in >( generatorData );
	else 
		result = drawSections< no_zoom >( generatorData );
		
	if ( result )
	{
		IBitmap* bitmap = generatorData->d_bitmap;
		d_painter.dump( bitmap );
	}

	return result;
}

template< typename zoom_t >
bool KContentsGenerator::drawSections( SContentsGeneratorData* generatorData )
{
	if ( !d_isZoomIn )
	{
		zoom_t zoom( d_zoomFactor );
		zoom( &d_zoomedViewportTopLeft );
	}

	const IInternalDocument& document = generatorData->d_document;
	const section_ids_t& sectionids = generatorData->d_sections;
	for ( section_ids_cit it = sectionids.begin()
		; it != sectionids.end()
		; ++it )
	{
		const section_id_t sectid = *it;
		document.getSection( sectid, &d_section );
		if ( filterSection( d_section ) )
		{
			drawSection< zoom_t >();
		}
	}

	const bool result = !sectionids.empty();
	return result;
}

bool KContentsGenerator::filterSection( const SSection& section ) const
{
	const int roadClassIndex = section.d_roadClassIndex;
	const bool result = d_roadClassFilter <= roadClassIndex;
	return result;
}

template< typename zoom_t >
void KContentsGenerator::drawSection()
{
	initSection();
	if ( preZoomStep< zoom_t >() )
	{
		zoomSection< zoom_t >();
		if ( postZoomStep< zoom_t >() )
		{
			d_painter.drawSection( d_begin, d_end, d_roadClass, d_orientation );
		}
	}
}

// ----------------------------------------------------------------------------

void KContentsGenerator::initSection()
{
	setRoadClass();
	initScreenPoints();
	setSectionOrientation();
}

template< typename zoom_t >
bool KContentsGenerator::preZoomStep()
{
	bool result = true;
	if ( d_isZoomIn )
	{
		if ( clipSection( d_viewportRect, ClipCatchTolerance ) )
		{
			prepareScreenPoint( d_viewportTopLeft, &d_begin );
			prepareScreenPoint( d_viewportTopLeft, &d_end );
		}
		else
		{
			result = false;
		}
	}
	return result;
}

template< typename zoom_t >
void KContentsGenerator::zoomSection()
{
	zoom_t zoom( d_zoomFactor );
	zoom( &d_begin );
	zoom( &d_end );

	if ( updateSectionOrientation() )
		normalizeSection( &d_begin, &d_end );
}

template< typename zoom_t >
bool KContentsGenerator::postZoomStep()
{
	if ( !d_isZoomIn )
	{
		prepareScreenPoint( d_zoomedViewportTopLeft, &d_begin );
		prepareScreenPoint( d_zoomedViewportTopLeft, &d_end );
	}

	bool result = false;
	const coord_t clipTolerance = calcClipTolerance();
	if ( clipSection( d_clipScreenRect, clipTolerance ) )
	{
		result = true;
		if ( updateSectionOrientation() )
		{
			// inclined section may be clipped to straight horizontal 
			// or vertical line, in such case clip is needed once again
			if ( ( d_orientation == Horizontal ) || ( d_orientation == Vertical ) )
			{
				result = clipSection( d_clipScreenRect, 0 );
			}
		}
	}
	return result;
}

// ----------------------------------------------------------------------------

void KContentsGenerator::initScreenPoints()
{
	const SPoint* begin = d_section.d_begin;
	const SPoint* end = d_section.d_end;

	if ( isSectionNormalized( *begin, *end ) )
	{
		d_begin = *begin;
		d_end = *end;
	}
	else
	{
		d_begin = *end;
		d_end = *begin;
	}

	assert( isSectionNormalized( d_begin, d_end ) );
}

void KContentsGenerator::setRoadClass()
{
	const std::size_t roadClassIndex = d_section.d_roadClassIndex;
	assert( roadClassIndex < d_roadClasses.size() );
	d_roadClass = d_roadClasses[ roadClassIndex ];
	assert( d_roadClass != 0 );
}

void KContentsGenerator::setSectionOrientation()
{
	d_orientation = calcSectionOrientation();
}

bool KContentsGenerator::updateSectionOrientation()
{
	bool result = false;
	if ( d_orientation == InclinedHorizontal || ( d_orientation == InclinedVertical ) )
	{
		const EOrientation currentOrientation = calcSectionOrientation();
		if ( d_orientation != currentOrientation )
		{
			d_orientation = currentOrientation;
			result = true;
		}
	}
	else
	{
		assert( ( d_orientation == Horizontal ) || ( d_orientation == Vertical ) );
	}
	return result;
}

bool KContentsGenerator::isInclinedOrientation() const
{
	const bool result = ( d_orientation == InclinedHorizontal )
		|| ( d_orientation == InclinedVertical );
	return result;
}

EOrientation KContentsGenerator::calcSectionOrientation() const
{
	EOrientation orientation = UnknownOrientation;

	const coord_t x0 = d_begin.x;
	const coord_t y0 = d_begin.y;
	const coord_t x1 = d_end.x;
	const coord_t y1 = d_end.y;

	const coord_t dx = std::abs( x1 - x0 );
	const coord_t dy = std::abs( y1 - y0 );

	if ( y0 == y1 )
	{
		orientation = Horizontal;
	}
	else if ( x0 == x1 )
	{
		orientation = Vertical;
	}
	else if ( dy <= dx )
	{
		orientation = InclinedHorizontal;
	}
	else
	{
		orientation = InclinedVertical;
	}

	return orientation;
}

coord_t KContentsGenerator::calcClipTolerance() const
{
	coord_t result = 0;
	if ( isInclinedOrientation() )
	{
		const coord_t thickness = d_roadClass->d_fullThickness;
		if ( 1 < thickness )
		{
			result = ( thickness >> 1 );
		}
	}
	return result;
}

void KContentsGenerator::prepareScreenPoint( 
	const SPoint& viewportTopLeftCorner, 
	SPoint* point ) const
{
	point->x -= viewportTopLeftCorner.x;
	point->y -= viewportTopLeftCorner.y;
}

bool KContentsGenerator::clipSection( 
	const SRect& clipRect,
	const int clipExtraTolerance )
{
	KClipSection clipSection( clipRect, clipExtraTolerance, &d_begin, &d_end );
	const bool result = clipSection.run( d_orientation );
	return result;
}

} // anonymous namespace

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

SContentsGeneratorData::SContentsGeneratorData(
	const IInternalDocument& document,
	const SViewData& viewData,
	const SRect& viewportRect,
	const section_ids_t& sectionids,
	IBitmap* bitmap )
	: d_document( document )
	, d_viewData( viewData )
	, d_viewportRect( viewportRect )
	, d_sections( sectionids )
	, d_bitmap( bitmap )
{
}

bool generateViewContents( SContentsGeneratorData* generatorData )
{
	KContentsGenerator contentsMaker( generatorData );
	const bool result = contentsMaker.run( generatorData );
	return result;
}

} // namespace be
