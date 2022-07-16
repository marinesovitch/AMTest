// author: marines marinesovitch alias Darek Slusarczyk 2012-2013
#include "ph.h"
#include "beTypes.h"

namespace be
{

SSize::SSize(
	const coord_t w,
	const coord_t h )
	: width ( w )
	, height( h )
{
}

bool SSize::operator!=( const SSize& rhs ) const
{
	const bool result = ( width != rhs.width ) || ( height != rhs.height );
	return result;
}

// ----------------------------------------------------------------------------

SPoint::SPoint(
	const coord_t ix,
	const coord_t iy )
	: x( ix )
	, y( iy )
{
}

bool SPoint::operator==( const SPoint& rhs ) const
{
	const bool result = ( x == rhs.x ) && ( y == rhs.y );
	return result;
}

bool SPoint::operator!=( const SPoint& rhs ) const
{
	const bool result = ( x != rhs.x ) || ( y != rhs.y );
	return result;
}

SSize SPoint::operator-( const SPoint& rhs ) const
{
	const coord_t cx = x - rhs.x;
	const coord_t cy = y - rhs.y;
	const SSize result( cx, cy );
	return result;
}

// ----------------------------------------------------------------------------

SRect::SRect(
	const coord_t l,
	const coord_t t,
	const coord_t r,
	const coord_t b )
	: left( l )
	, top( t ) 
	, right( r )
	, bottom( b )
{
}

SRect::SRect(
	const SPoint& topLeft,
	const SSize& size )
	: left( topLeft.x )
	, top( topLeft.y ) 
	, right( topLeft.x + size.width )
	, bottom( topLeft.y + size.height )
{
}

SRect::SRect(const SSize& size )
	: left( 0 )
	, top( 0 ) 
	, right( size.width )
	, bottom( size.height )
{
}

bool SRect::operator!=( const SRect& rhs ) const
{
	const bool result 
		= ( left != rhs.left )
		|| ( top != rhs.top ) 
		|| ( right != rhs.right )
		|| ( bottom != rhs.bottom )
		;
	return result;
}

const SPoint SRect::getCenter() const
{
	const coord_t x = left + ( right - left ) / 2;
	const coord_t y = top + ( bottom - top ) / 2;
	const SPoint result( x, y );
	return result;
}

const SPoint SRect::getTopLeft() const
{
	const SPoint result( left, top );
	return result;
}

const SPoint SRect::getBottomRight() const
{
	const SPoint result( right, bottom );
	return result;
}

const SSize SRect::getSize() const
{
	const coord_t width = right - left;
	const coord_t height = bottom - top;
	const SSize result( width, height );
	return result;
}

bool SRect::contains( const SPoint& pt ) const
{
	const bool result = ( left <= pt.x ) && ( pt.x <= right )
		&& ( top <= pt.y ) && ( pt.y <= bottom );
	return result;
}

bool SRect::isOnEdge( const SPoint& pt ) const
{
	const bool result = ( ( left == pt.x ) || ( pt.x == right ) )
		&& ( ( top == pt.y ) || ( pt.y == bottom ) );
	return result;
}

// ----------------------------------------------------------------------------

SMoveData::SMoveData( const SPoint& focusScreenPoint )
	: d_kind( MoveToPoint )
	, d_direction( UnknownDirection )
	, d_focusScreenPoint( focusScreenPoint )
{
}

SMoveData::SMoveData( const EDirection direction )
	: d_kind( MoveInDirection )
	, d_direction( direction )
{
}

SMoveData::SMoveData( const SSize& delta )
	: d_kind( MoveDelta )
	, d_direction( UnknownDirection )
	, d_delta( delta )
{
}

// ----------------------------------------------------------------------------

SZoomData::SZoomData( 
	const EKind kind, 
	const int delta,
	const bool zoomInPlace,
	const be::SPoint& focusScreenPoint )
	: d_kind( kind )
	, d_delta( delta )
	, d_zoomInPlace( zoomInPlace )
	, d_focusScreenPoint( focusScreenPoint )
{
}

} // namespace be
