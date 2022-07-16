// author: marines marinesovitch alias Darek Slusarczyk 2012-2013
#ifndef INC_BE_TYPES_H
#define INC_BE_TYPES_H

namespace be
{

#ifdef ANDROID
using color_t = short;
#else
using color_t = int;
#endif

using color32_t = int;

using coord_t = int;

// ----------------------------------------------------------------------------

struct SSize
{
	SSize(
		coord_t width = 0,
		coord_t height = 0 );

	bool operator!=( const SSize& rhs ) const;

	coord_t width;
	coord_t height;
};

// ----------------------------------------------------------------------------

struct SPoint
{
	SPoint(
		coord_t x = 0,
		coord_t y = 0 );

	bool operator==( const SPoint& rhs ) const;
	bool operator!=( const SPoint& rhs ) const;

	SSize operator-( const SPoint& rhs ) const;

	coord_t x;
	coord_t y;
};

using points_t = std::vector< SPoint >;
using points_it = points_t::iterator;
using points_cit = points_t::const_iterator;

// ----------------------------------------------------------------------------

struct SRect
{
	SRect(
		coord_t left = 0,
		coord_t top = 0,
		coord_t right = 0,
		coord_t bottom = 0 );
	SRect(
		const SPoint& topLeft,
		const SSize& size );
	SRect(
		const SSize& size );

	bool operator!=( const SRect& rhs ) const;

	SPoint getCenter() const;
	SPoint getTopLeft() const;
	SPoint getBottomRight() const;

	SSize getSize() const;

	bool contains( const SPoint& pt ) const;
	bool isOnEdge( const SPoint& pt ) const;

	coord_t left;
	coord_t top;
	coord_t right;
	coord_t bottom;
};

// ----------------------------------------------------------------------------

enum EDirection
{
	UnknownDirection,
	North,
	East,
	South,
	West
};

struct SMoveData
{
	SMoveData( const SPoint& focusScreenPoint );
	SMoveData( EDirection direction );
	SMoveData( const SSize& delta );

	enum EKind
	{
		MoveToPoint,
		MoveInDirection,
		MoveDelta
	};

	const EKind d_kind;

	const EDirection d_direction;
	const SPoint d_focusScreenPoint;
	const SSize d_delta;
};

struct SZoomData
{
	enum EKind
	{
		ZoomNone,
		ZoomIn,
		ZoomOut
	};

	SZoomData(
		EKind kind,
		int delta,
		bool zoomInPlace,
		const be::SPoint& focusScreenPoint );

	const EKind d_kind;
	const int d_delta;
	const bool d_zoomInPlace;
	const be::SPoint d_focusScreenPoint;

};

} // namespace be

#endif
