// author: marines marinesovitch alias Darek Slusarczyk 2012-2013
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

typedef std::vector< bool > bools_t;
typedef bools_t::iterator bools_it;
typedef bools_t::const_iterator bools_cit;

// ----------------------------------------------------------------------------

class id_handle_t
{
	public:
		typedef std::uint32_t value_t;

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
	SRawSegment( const int roadClass );

	int d_roadClass;
	points_t d_points;
};

typedef std::vector< SRawSegment > raw_segments_t;
typedef raw_segments_t::iterator raw_segments_it;
typedef raw_segments_t::const_iterator raw_segments_cit;

// ----------------------------------------------------------------------------

struct SRoadClass
{
	SRoadClass(
		const coord_t thickness,
		const color_t color );
	SRoadClass(
		const coord_t thickness,
		const color_t color,
		const coord_t outlineThickness,
		const color_t outlineColor );

	bool hasOutline() const;

	const coord_t d_thickness;
	const color_t d_color;

	const coord_t d_outlineThickness;
	const color_t d_outlineColor;

	const coord_t d_fullThickness;
};

typedef std::vector< SRoadClass* > road_classes_t;
typedef road_classes_t::iterator road_classes_it;
typedef road_classes_t::const_iterator road_classes_cit;

// ----------------------------------------------------------------------------

class point_pos_id_t : public id_handle_t
{
	public:
		explicit point_pos_id_t(id_handle_t::value_t handle) : id_handle_t(handle) {}
		explicit point_pos_id_t(std::uint64_t handle) : id_handle_t(handle) {}
};

typedef std::vector< point_pos_id_t > point_ids_t;
typedef point_ids_t::iterator point_ids_it;
typedef point_ids_t::const_iterator point_ids_cit;

// ----------------------------------------------------------------------------

struct SPointPos
{
	SPointPos(
		const point_pos_id_t pointposid,
		const SPoint& point );

	point_pos_id_t d_id;
	SPoint d_point;

};

typedef std::vector< const SPointPos* > point_positions_t;
typedef point_positions_t::iterator point_positions_it;
typedef point_positions_t::const_iterator point_positions_cit;

// ----------------------------------------------------------------------------

typedef std::vector< SPointPos > segment_points_t;
typedef segment_points_t::iterator segment_points_it;
typedef segment_points_t::const_iterator segment_points_cit;

struct SSegment
{
	SSegment( const int roadClassIndex );

	int d_roadClassIndex;
	segment_points_t d_points;
};

typedef std::vector< SSegment > segments_t;
typedef segments_t::iterator segments_it;
typedef segments_t::const_iterator segments_cit;

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

typedef std::vector< section_id_t > section_ids_t;
typedef section_ids_t::iterator section_ids_it;
typedef section_ids_t::const_iterator section_ids_cit;

// ----------------------------------------------------------------------------

class sect_pos_id_t : public id_handle_t
{
	public:
		explicit sect_pos_id_t(id_handle_t::value_t handle) : id_handle_t(handle) {}
		explicit sect_pos_id_t(std::uint64_t handle) : id_handle_t(handle) {}
};

typedef std::vector< sect_pos_id_t > sect_pos_ids_t;
typedef sect_pos_ids_t::iterator sect_pos_ids_it;
typedef sect_pos_ids_t::const_iterator sect_pos_ids_cit;

// ----------------------------------------------------------------------------

struct SSectionPos
{
	SSectionPos(
		const sect_pos_id_t sectposid,
		const SPoint& point );

	sect_pos_id_t d_id;
	SPoint d_point;

};

typedef std::vector< const SSectionPos* > section_positions_t;
typedef section_positions_t::iterator section_positions_it;
typedef section_positions_t::const_iterator section_positions_cit;

// ----------------------------------------------------------------------------

struct SIntervalSection
{
	SIntervalSection(
		const section_id_t sectid,
		const SSectionPos& begin,
		const SSectionPos& end );

	bool hasOrientation( const EOrientation orientation ) const;

	section_id_t d_sectid;
	SSectionPos d_begin;
	SSectionPos d_end;
};

typedef std::vector< SIntervalSection > interval_sections_t;
typedef interval_sections_t::iterator interval_sections_it;
typedef interval_sections_t::const_iterator interval_sections_cit;

// ----------------------------------------------------------------------------

class interval_section_id_t : public id_handle_t
{
	public:
		explicit interval_section_id_t(id_handle_t::value_t handle) : id_handle_t(handle) {}
		explicit interval_section_id_t(std::uint64_t handle) : id_handle_t(handle) {}
};

typedef std::vector< interval_section_id_t > interval_section_ids_t;
typedef interval_section_ids_t::iterator interval_section_ids_it;
typedef interval_section_ids_t::const_iterator interval_section_ids_cit;

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
