// author: marines marinesovitch alias Darek Slusarczyk 2012-2013
#include "ph.h"
#include "beDiagnostics.h"
#include "beInternalDocument.h"
#include <iostream>

namespace be
{

namespace diag
{

namespace
{

struct compare_by_x
{
	bool operator()( const SPoint& lhs, const SPoint& rhs ) const
	{
		bool result = false;

		const int lx = lhs.x;
		const int rx = rhs.x;
		if ( lx < rx ) 
		{
			result = true;
		}
		else if ( lx == rx ) 
		{
			const int ly = lhs.y;
			const int ry = rhs.y;
			if ( ly < ry )
			{
				result = true;
			}
		}

		return result;
	}
};

struct compare_by_y
{
	bool operator()( const SPoint& lhs, const SPoint& rhs ) const
	{
		bool result = false;

		const int ly = lhs.y;
		const int ry = rhs.y;
		if ( ly < ry ) 
		{
			result = true;
		}
		else if ( ly == ry ) 
		{
			const int lx = lhs.x;
			const int rx = rhs.x;
			if ( lx < rx )
			{
				result = true;
			}
		}

		return result;
	}
};

// ----------------------------------------------------------------------------

void dumpPoint( const SPoint& point )
{
	std::cout << '(' << point.x << ',' << point.y << ')';
}

void dumpPoints( const points_t& points )
{
	for ( points_cit it = points.begin()
		; it != points.end()
		; ++it )
	{
		const SPoint& point = *it;
		dumpPoint( point );
		std::cout << std::endl;
	}
}

void dumpSegment( const SRawSegment& segment )
{
	std::cout << "class: " << segment.d_roadClass << std::endl;
	const points_t& points = segment.d_points;
	dumpPoints( points );
}

} // anonymous namespace

void dumpRawSegments( const raw_segments_t& segments )
{
	int index = 0;
	for ( raw_segments_cit it = segments.begin()
		; it != segments.end()
		; ++it, ++index )
	{
		std::cout << index << std::endl;
		const SRawSegment& segment = *it;
		dumpSegment( segment );
	}
}

// ----------------------------------------------------------------------------

void dumpPoints( const raw_segments_t& segments )
{
	points_t points;

	for ( raw_segments_cit it = segments.begin()
		; it != segments.end()
		; ++it )
	{
		const SRawSegment& segment = *it;
		const points_t& segmentPoints = segment.d_points;
		points.insert( 
			points.end(), 
			segmentPoints.begin(),
			segmentPoints.end() );
	}

	std::sort( points.begin(), points.end(), compare_by_x() );
	std::cout << "------------------- Points by X: " << points.size() << std::endl;
	dumpPoints( points );

	std::sort( points.begin(), points.end(), compare_by_y() );
	std::cout << "------------------- Points by Y: " << points.size() << std::endl;
	dumpPoints( points );
}

// ----------------------------------------------------------------------------

void dumpRect( const SRect& rect )
{
	const SSize& size = rect.getSize();
	std::cout << "(" << rect.left << ", " << rect.top 
		<< ", " << rect.right << ", " << rect.bottom << ")" 
		<< std::hex << " (" << rect.left << ", " << rect.top 
		<< ", " << rect.right << ", " << rect.bottom << ")" 
		<< std::dec << "  dims: " << size.width << ',' << size.height << std::endl;
}

// ----------------------------------------------------------------------------

void dumpView( const SViewData& viewData )
{
	static unsigned int s_counter = 0;
	std::cout << "------------------- ViewData: " << s_counter++ << std::endl
		<< "viewport center: " << viewData.d_viewportCenter.x << ' ' 
		<< viewData.d_viewportCenter.y << std::endl
		<< "device size: " << viewData.d_deviceSize.width << ' ' 
		<< viewData.d_deviceSize.height << std::endl
		<< "screen size: " << viewData.d_screenSize.width << ' ' 
		<< viewData.d_screenSize.height << std::endl
		<< "zoom factor: " << viewData.d_zoomFactor << std::endl
		;
}

void dumpSections( 
	const IInternalDocument* document,
	const section_ids_t& sections )
{
	const std::size_t SectionsLimit = 50;
	if ( sections.size() < SectionsLimit )
	{
		SSection section;
		for ( section_ids_cit it = sections.begin()
			; it != sections.end()
			; ++it )
		{
			const section_id_t sectid = *it;
			std::cout << sectid << ' ';
			document->getSection( sectid, &section );
			dumpPoint( *section.d_begin );
			std::cout << ' ' ;
			dumpPoint( *section.d_end );
			std::cout << std::endl;
		}
	}
}

} // namespace diag

} // namespace be
