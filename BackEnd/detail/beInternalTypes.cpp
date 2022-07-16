// author: marines marinesovitch alias Darek Slusarczyk 2012-2013
#include "ph.h"
#include "beInternalTypes.h"

namespace be
{

id_handle_t::id_handle_t(std::uint64_t h) : handle(static_cast<value_t>(h))
{
	assert(h <= std::numeric_limits<value_t>::max());
}

// ----------------------------------------------------------------------------

std::ostream& operator<<(std::ostream& os, const id_handle_t& handle)
{
	return os << handle.get();
}

// ----------------------------------------------------------------------------

SRawSegment::SRawSegment( int roadClass )
	: d_roadClass( roadClass )
{
}

// ----------------------------------------------------------------------------

SRoadClass::SRoadClass(
	coord_t thickness,
	color_t color )
	: d_thickness( thickness )
	, d_color( color )
	, d_outlineThickness( 0 )
	, d_outlineColor( 0 )
	, d_fullThickness( d_thickness )
{
	assert( 0 < d_thickness );
}

SRoadClass::SRoadClass(
	coord_t thickness,
	color_t color,
	coord_t outlineThickness,
	color_t outlineColor )
	: d_thickness( thickness )
	, d_color( color )
	, d_outlineThickness( outlineThickness )
	, d_outlineColor( outlineColor )
	, d_fullThickness( d_thickness + 2 * d_outlineThickness )
{
	assert( ( 0 < d_thickness ) && ( 0 < d_outlineThickness ) );
}

bool SRoadClass::hasOutline() const
{
	const bool result = d_outlineThickness != 0;
	return result;
}

// ----------------------------------------------------------------------------

SPointPos::SPointPos(
	const point_pos_id_t pointposid,
	const SPoint& point )
	: d_id( pointposid )
	, d_point( point )
{
}

// ----------------------------------------------------------------------------

SSegment::SSegment( const int roadClassIndex )
	: d_roadClassIndex( roadClassIndex )
{
}

// ----------------------------------------------------------------------------

SSectionPos::SSectionPos(
	const sect_pos_id_t sectposid,
	const SPoint& point )
	: d_id( sectposid )
	, d_point( point )
{
}

// ----------------------------------------------------------------------------

SIntervalSection::SIntervalSection(
	const section_id_t sectid,
	const SSectionPos& begin,
	const SSectionPos& end )
	: d_sectid( sectid )
	, d_begin( begin )
	, d_end( end )
{
}

bool SIntervalSection::hasOrientation( EOrientation orientation ) const
{
	bool result = false;
	if ( orientation == Horizontal )
	{
		const coord_t y0 = d_begin.d_point.y;
		const coord_t y1 = d_end.d_point.y;
		if ( y0 == y1 )
			result = true;
	}
	else
	{
		assert( orientation == Vertical );
		const coord_t x0 = d_begin.d_point.x;
		const coord_t x1 = d_end.d_point.x;
		if ( x0 == x1 )
			result = true;
	}
	return result;
}

// ----------------------------------------------------------------------------

SSection::SSection()
	: d_roadClassIndex( 0 )
	, d_begin( nullptr )
	, d_end( nullptr )
{
}

// ----------------------------------------------------------------------------

SViewData::SViewData() : d_zoomFactor( 0 )
{
}

bool SViewData::operator!=( const SViewData& rhs ) const
{
	const bool result
		= ( d_mapRect != rhs.d_mapRect )
		|| ( d_viewportCenter != rhs.d_viewportCenter )
		|| ( d_deviceSize != rhs.d_deviceSize )
		|| ( d_screenSize != rhs.d_screenSize )
		|| ( d_zoomFactor != rhs.d_zoomFactor )
		;
	return result;
}

} // namespace be
