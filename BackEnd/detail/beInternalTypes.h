// author: marines marinesovitch alias Darek Slusarczyk 2012-2013, 2022
#ifndef INC_BE_INTERNAL_TYPES_H
#define INC_BE_INTERNAL_TYPES_H

#include "beTypes.h"

namespace be
{

enum EOrientation
{
	UnknownOrientation,
	Horizontal,
	Vertical,
	InclinedHorizontal,
	InclinedVertical
};

enum EChildSide
{
	NoParent,
	LeftChild,
	RightChild
};

// ----------------------------------------------------------------------------

using bools_t = std::vector< bool >;
using bools_it = bools_t::iterator;
using bools_cit = bools_t::const_iterator;

// ----------------------------------------------------------------------------

class id_handle_t
{
	public:
		using value_t = std::uint32_t;

	protected:
		id_handle_t() : handle(null_handle()) {}
		explicit id_handle_t(value_t handle) : handle(handle) {}
		explicit id_handle_t(std::uint64_t handle);

	public:
		bool operator==(const id_handle_t& rhs) const { return handle == rhs.handle; }
		bool operator!=(const id_handle_t& rhs) const { return !operator==(rhs); }
		bool operator<(const id_handle_t& rhs) const { return handle < rhs.handle; }

	public:
		void reset() { handle = null_handle(); }
		bool is_null() const { return handle == null_handle(); }
		value_t get() const { return handle; }

	public:
		static constexpr std::size_t size_of() { return sizeof(id_handle_t::value_t); }

	private:
		constexpr value_t null_handle() const { return static_cast<id_handle_t::value_t>(-1); }

	private:
		value_t handle;
};

std::ostream& operator<<(std::ostream& os, const id_handle_t& handle);

// ----------------------------------------------------------------------------

struct SRawSegment
{
	SRawSegment( int roadClass );

	int d_roadClass;
	points_t d_points;
};

using raw_segments_t = std::vector< SRawSegment >;
using raw_segments_it = raw_segments_t::iterator;
using raw_segments_cit = raw_segments_t::const_iterator;

// ----------------------------------------------------------------------------

struct SRoadClass
{
	SRoadClass(
		coord_t thickness,
		color_t color );
	SRoadClass(
		coord_t thickness,
		color_t color,
		coord_t outlineThickness,
		color_t outlineColor );

	bool hasOutline() const;

	const coord_t d_thickness;
	const color_t d_color;

	const coord_t d_outlineThickness;
	const color_t d_outlineColor;

	const coord_t d_fullThickness;
};

using road_classes_t = std::vector< SRoadClass* >;
using road_classes_it = road_classes_t::iterator;
using road_classes_cit = road_classes_t::const_iterator;

// ----------------------------------------------------------------------------

class point_pos_id_t : public id_handle_t
{
	public:
		explicit point_pos_id_t(id_handle_t::value_t handle) : id_handle_t(handle) {}
		explicit point_pos_id_t(std::uint64_t handle) : id_handle_t(handle) {}
};

using point_ids_t = std::vector< point_pos_id_t >;
using point_ids_it = point_ids_t::iterator;
using point_ids_cit = point_ids_t::const_iterator;

// ----------------------------------------------------------------------------

struct SPointPos
{
	SPointPos(
		const point_pos_id_t pointposid,
		const SPoint& point );

	point_pos_id_t d_id;
	SPoint d_point;

};

using point_positions_t = std::vector< const SPointPos* >;
using point_positions_it = point_positions_t::iterator;
using point_positions_cit = point_positions_t::const_iterator;

// ----------------------------------------------------------------------------

using segment_points_t = std::vector< SPointPos >;
using segment_points_it = segment_points_t::iterator;
using segment_points_cit = segment_points_t::const_iterator;

struct SSegment
{
	SSegment( const int roadClassIndex );

	int d_roadClassIndex;
	segment_points_t d_points;
};

using segments_t = std::vector< SSegment >;
using segments_it = segments_t::iterator;
using segments_cit = segments_t::const_iterator;

class segment_id_t : public id_handle_t
{
	public:
		explicit segment_id_t(id_handle_t::value_t handle) : id_handle_t(handle) {}
		explicit segment_id_t(std::uint64_t handle) : id_handle_t(handle) {}
};

// ----------------------------------------------------------------------------

class section_id_t : public id_handle_t
{
	public:
		section_id_t() = default;
		explicit section_id_t(id_handle_t::value_t handle) : id_handle_t(handle) {}
		explicit section_id_t(std::uint64_t handle) : id_handle_t(handle) {}
};

using section_ids_t = std::vector< section_id_t >;
using section_ids_it = section_ids_t::iterator;
using section_ids_cit = section_ids_t::const_iterator;

// ----------------------------------------------------------------------------

class sect_pos_id_t : public id_handle_t
{
	public:
		explicit sect_pos_id_t(id_handle_t::value_t handle) : id_handle_t(handle) {}
		explicit sect_pos_id_t(std::uint64_t handle) : id_handle_t(handle) {}
};

using sect_pos_ids_t = std::vector< sect_pos_id_t >;
using sect_pos_ids_it = sect_pos_ids_t::iterator;
using sect_pos_ids_cit = sect_pos_ids_t::const_iterator;

// ----------------------------------------------------------------------------

struct SSectionPos
{
	SSectionPos(
		const sect_pos_id_t sectposid,
		const SPoint& point );

	sect_pos_id_t d_id;
	SPoint d_point;

};

using section_positions_t = std::vector< const SSectionPos* >;
using section_positions_it = section_positions_t::iterator;
using section_positions_cit = section_positions_t::const_iterator;

// ----------------------------------------------------------------------------

struct SIntervalSection
{
	SIntervalSection(
		const section_id_t sectid,
		const SSectionPos& begin,
		const SSectionPos& end );

	bool hasOrientation( EOrientation orientation ) const;

	section_id_t d_sectid;
	SSectionPos d_begin;
	SSectionPos d_end;
};

using interval_sections_t = std::vector< SIntervalSection >;
using interval_sections_it = interval_sections_t::iterator;
using interval_sections_cit = interval_sections_t::const_iterator;

// ----------------------------------------------------------------------------

class interval_section_id_t : public id_handle_t
{
	public:
		explicit interval_section_id_t(id_handle_t::value_t handle) : id_handle_t(handle) {}
		explicit interval_section_id_t(std::uint64_t handle) : id_handle_t(handle) {}
};

using interval_section_ids_t = std::vector< interval_section_id_t >;
using interval_section_ids_it = interval_section_ids_t::iterator;
using interval_section_ids_cit = interval_section_ids_t::const_iterator;

// ----------------------------------------------------------------------------

struct SSection
{
	SSection();
	int d_roadClassIndex;
	const SPoint* d_begin;
	const SPoint* d_end;
};

// ----------------------------------------------------------------------------

struct SViewData
{
	SViewData();

	bool operator!=( const SViewData& rhs ) const;

	SRect d_mapRect;
	SPoint d_viewportCenter;
	SSize d_deviceSize;
	SSize d_screenSize;
	int d_zoomFactor;
};

} // namespace be

#endif
