// author: marines marinesovitch alias Darek Slusarczyk 2012-2013
#ifndef INC_BE_SEGMENTS_MANAGER_H
#define INC_BE_SEGMENTS_MANAGER_H

#include "beInternalTypes.h"

namespace be
{

class KSegmentsManager
{
	protected:
		KSegmentsManager();

	public:
		virtual ~KSegmentsManager();

	public:
		static std::size_t maxSegmentCount();
		static std::size_t maxSegmentPointsCount();

	public:
		bool getPointPositions( point_positions_t* point_positions ) const;

	public:
		bool getSectPositions( 
			const EOrientation orientation,
			section_positions_t* sect_positions ) const;

		const SSectionPos* getSectionPos( const sect_pos_id_t sectposid ) const;

		const SSectionPos* getSectionBeginPos( const SSectionPos* sectpos ) const;
		const SSectionPos* getSectionEndPos( const SSectionPos* sectpos ) const;

		SPoint getSectionCrossPoint( const sect_pos_id_t sectposid ) const;

		static bool isSection( const SSectionPos* beginSectPos, const SSectionPos* endSectPos );
 
	protected:
		void init( raw_segments_t* rawSegments );

		void getSection( 
			const section_id_t sectid,
			SSection* section ) const;

		bool prepareSections( 
			const SRect& viewportRect,
			const point_ids_t& pointids,
			const sect_pos_ids_t& sectposids,
			section_ids_t* sections ) const;

		bool compareBruteForceSelectSections(
			const SRect& viewportRect,
			const section_ids_t& sections ) const;

	protected:
		road_classes_t d_roadClasses;
		segments_t d_segments;
		interval_sections_t d_intervalSections;

};

} // namespace be

#endif
