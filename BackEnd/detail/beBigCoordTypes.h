// author: marines marinesovitch alias Darek Slusarczyk 2012-2013
#ifndef INC_BE_BIG_COORD_TYPES_H
#define INC_BE_BIG_COORD_TYPES_H

#include "beInternalTypes.h"

namespace be
{

using big_coord_t = long long;

namespace utils
{

coord_t big2coord( const big_coord_t coord );

} // namespace utils

// ----------------------------------------------------------------------------

struct SBigSize
{
	SBigSize(
		const big_coord_t width = 0,
		const big_coord_t height = 0 );

	SBigSize operator-( const SBigSize& rhs ) const;

	big_coord_t width;
	big_coord_t height;
};

// ----------------------------------------------------------------------------

struct SBigPoint
{
	SBigPoint(
		const big_coord_t x = 0,
		const big_coord_t y = 0 );
	explicit SBigPoint(
		const SPoint& point );

	SPoint getPoint() const;

	bool operator!=( const SBigPoint& rhs ) const;
	bool operator!=( const SPoint& rhs ) const {
		return operator!=(SBigPoint(rhs));
	}

	big_coord_t x;
	big_coord_t y;
};

// ----------------------------------------------------------------------------

struct SBigRect
{
	SBigRect(
		big_coord_t left = 0,
		big_coord_t top = 0,
		big_coord_t right = 0,
		big_coord_t bottom = 0 ) noexcept;
	explicit SBigRect(
		const SRect& rect );
	SBigRect(
		const SBigPoint& topLeft,
		const SBigSize& size );

	SRect getRect() const;

	big_coord_t left;
	big_coord_t top;
	big_coord_t right;
	big_coord_t bottom;
};

} // namespace be

#endif
