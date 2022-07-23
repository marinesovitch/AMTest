// author: marines marinesovitch alias Darek Slusarczyk 2012-2013, 2022
#include "ph.h"
#include "beBigCoordTypes.h"
#include "beConsts.h"

namespace be
{

namespace
{

bool isCoordInRange( const big_coord_t coord )
{
	const bool result = ( consts::MinCoord <= coord ) && ( coord <= consts::MaxCoord );
	return result;
}

} // anonymous namespace

namespace utils
{

coord_t big2coord( const big_coord_t coord )
{
	assert( isCoordInRange( coord ) );
	const auto result = static_cast< coord_t >( coord );
	return result;
}

} // namespace utils

// ----------------------------------------------------------------------------

SBigSize::SBigSize(
	const big_coord_t w,
	const big_coord_t h )
	: width ( w )
	, height( h )
{
}

SBigSize SBigSize::operator-( const SBigSize& rhs ) const
{
	const big_coord_t diffWidth = width - rhs.width;
	const big_coord_t diffHeight = height - rhs.height;
	const SBigSize result( diffWidth, diffHeight );
	return result;
}

// ----------------------------------------------------------------------------

SBigPoint::SBigPoint(
	const big_coord_t ix,
	const big_coord_t iy )
	: x( ix )
	, y( iy )
{
}

SBigPoint::SBigPoint(
	const SPoint& point )
	: x( point.x )
	, y( point.y )
{
}

SPoint SBigPoint::getPoint() const
{
	const SPoint result( utils::big2coord( x ), utils::big2coord( y ) );
	return result;
}

bool SBigPoint::operator!=( const SBigPoint& rhs ) const
{
	const bool result = ( x != rhs.x ) || ( y != rhs.y );
	return result;
}

// ----------------------------------------------------------------------------

SBigRect::SBigRect(
	big_coord_t l,
	big_coord_t t,
	big_coord_t r,
	big_coord_t b ) noexcept
	: left( l )
	, top( t )
	, right( r )
	, bottom( b )
{
}

SBigRect::SBigRect(
	const SRect& rect )
	: left( rect.left )
	, top( rect.top )
	, right( rect.right )
	, bottom( rect.bottom )
{
}

SBigRect::SBigRect(
	const SBigPoint& topLeft,
	const SBigSize& size )
	: left( topLeft.x )
	, top( topLeft.y )
	, right( topLeft.x + size.width )
	, bottom( topLeft.y + size.height )
{
}

SRect SBigRect::getRect() const
{
	const SRect result(
		utils::big2coord( left ),
		utils::big2coord( top ),
		utils::big2coord( right ),
		utils::big2coord( bottom ) );
	return result;
}

} // namespace be
