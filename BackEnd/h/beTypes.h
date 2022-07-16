// author: marines marinesovitch alias Darek Slusarczyk 2012-2013
#ifndef INC_BE_TYPES_H
#define INC_BE_TYPES_H

namespace be
{

#ifdef ANDROID
typedef short color_t;
#else
typedef int color_t;
#endif

typedef int color32_t;

typedef int coord_t;

// ----------------------------------------------------------------------------

struct SSize
{
	SSize(
		const coord_t width = 0,
		const coord_t height = 0 );

	bool operator!=( const SSize& rhs ) const;

	coord_t width;
	coord_t height;
};

// ----------------------------------------------------------------------------

struct SPoint
{
	SPoint(
		const coord_t x = 0,
		const coord_t y = 0 );

	bool operator==( const SPoint& rhs ) const;
	bool operator!=( const SPoint& rhs ) const;

	SSize operator-( const SPoint& rhs ) const;

	coord_t x;
	coord_t y;
};

typedef std::vector< SPoint > points_t;
typedef points_t::iterator points_it;
typedef points_t::const_iterator points_cit;

// ----------------------------------------------------------------------------

struct SRect
{
	SRect(
		const coord_t left = 0,
		const coord_t top = 0,
		const coord_t right = 0,
		const coord_t bottom = 0 );
	SRect(
		const SPoint& topLeft,
		const SSize& size );
	SRect(
		const SSize& size );

	bool operator!=( const SRect& rhs ) const;

	const SPoint getCenter() const;
	const SPoint getTopLeft() const;
	const SPoint getBottomRight() const;

	const SSize getSize() const;

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
	SMoveData( const EDirection direction );
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
		const EKind kind, 
		const int delta,
		const bool zoomInPlace,
		const be::SPoint& focusScreenPoint );

	const EKind d_kind;
	const int d_delta;
	const bool d_zoomInPlace;
	const be::SPoint d_focusScreenPoint;

};

} // namespace be

#endif
