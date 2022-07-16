// author: marines marinesovitch alias Darek Slusarczyk 2012-2013
#include "ph.h"
#include "beSegmentsManager.h"
#include "beUtils.h"
#include "beConsts.h"
#include "beConfig.h"

namespace be
{

namespace
{

const std::size_t IsBeginOrEndOfSectionFlagMask = 0x1;


// ----------------------------------------------------------------------------

inline point_pos_id_t composePointPosId(
	std::size_t segmentIndex,
	std::size_t pointIndex )
{
	assert( segmentIndex <= consts::MaxSegmentId );
	assert( pointIndex <= consts::MaxPointPosId );
	const std::size_t rawPointPosId = ( segmentIndex << consts::BitsForPointPosId ) + pointIndex;
	const point_pos_id_t result( rawPointPosId );
	return result;
}

inline void decomposePointPosId(
	const point_pos_id_t pointId,
	std::size_t* segmentIndex,
	std::size_t* pointIndex )
{
	const std::size_t rawPointPosId = pointId.get();
	*segmentIndex = rawPointPosId >> consts::BitsForPointPosId;
	*pointIndex = rawPointPosId & consts::MaxPointPosId;
	assert( *segmentIndex <= consts::MaxSegmentId );
	assert( *pointIndex <= consts::MaxPointPosId );
}

// ----------------------------------------------------------------------------

inline sect_pos_id_t composeSectPosId(
	std::size_t intervalSectionIndex,
	bool isEndOfSection )
{
	assert( intervalSectionIndex <= consts::MaxIntervalSectionId );
	const std::size_t rawSectPosId
		= ( intervalSectionIndex << consts::BitsForIsBeginOrEndOfSectionFlag )
		+ ( isEndOfSection ? 1 : 0 );
	const sect_pos_id_t result( rawSectPosId );
	return result;
}

inline void decomposeSectPosId(
	const sect_pos_id_t sectPosId,
	std::size_t* intervalSectionIndex,
	bool* isEndOfSection )
{
	const std::size_t rawSectionId = sectPosId.get();
	*intervalSectionIndex = rawSectionId >> consts::BitsForIsBeginOrEndOfSectionFlag;
	*isEndOfSection = utils::check_flag( rawSectionId, IsBeginOrEndOfSectionFlagMask );
	assert( *intervalSectionIndex <= consts::MaxIntervalSectionId );
}

inline std::size_t sectPosId2intervalSectionIndex(
	const sect_pos_id_t sectPosId )
{
	const std::size_t rawSectionId = sectPosId.get();
	const std::size_t result = rawSectionId >> consts::BitsForIsBeginOrEndOfSectionFlag;
	assert( result <= consts::MaxIntervalSectionId );
	return result;
}

// ----------------------------------------------------------------------------

inline section_id_t composeSectionId(
	std::size_t segmentIndex,
	std::size_t sectionIndex )
{
	assert( segmentIndex <= consts::MaxSegmentId );
	assert( sectionIndex <= consts::MaxSectionId );
	const std::size_t rawSectionId = ( segmentIndex << consts::BitsForSegmentId ) + sectionIndex;
	//const section_id_t result = reinterpret_cast< section_id_t >( rawSectionId );
	const section_id_t result = static_cast< section_id_t >( rawSectionId );
	return result;
}

inline void decomposeSectionId(
	const section_id_t sectionId,
	std::size_t* segmentIndex,
	std::size_t* sectionIndex )
{
	const std::size_t rawSectionId = sectionId.get();
	*segmentIndex = rawSectionId >> consts::BitsForSegmentId;
	*sectionIndex = rawSectionId & consts::MaxSectionId;
	assert( *segmentIndex <= consts::MaxSegmentId );
	assert( *sectionIndex <= consts::MaxSectionId );
}

// ----------------------------------------------------------------------------

struct sort_by_road_class
{
	bool operator()( const SRawSegment& lhs, const SRawSegment& rhs ) const
	{
		const int lRoadClass = lhs.d_roadClass;
		const int rRoadClass = rhs.d_roadClass;
		const bool result = lRoadClass < rRoadClass;
		return result;
	}
};

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

class KSegmentsCreator
{
	public:
		KSegmentsCreator(
			const road_classes_t& roadClasses,
			segments_t* segments );

	public:
		bool run( raw_segments_t* segments );

	private:
		void initSegments(
			const raw_segments_t& rawSegments );
		void addSegment(
			const SRawSegment& rawSegment );
		void addSegmentPoints(
			const points_t& rawPoints,
			segment_points_t* segmentPoints );

	private:
		const road_classes_t& d_roadClasses;
		std::size_t d_segmentIndex;
		segments_t* d_segments;

};

// ----------------------------------------------------------------------------

KSegmentsCreator::KSegmentsCreator(
	const road_classes_t& roadClasses,
	segments_t* segments )
	: d_roadClasses( roadClasses )
	, d_segmentIndex( 0 )
	, d_segments( segments )
{
}

bool KSegmentsCreator::run( raw_segments_t* rawSegments )
{
	d_segments->reserve( rawSegments->size() );

	/*
		sort by road class - at drawing the most important roads will be
		painted at the end (over less important)

		CAUTION! sort it before initSegments, else indexes will be
		corrupted !!!
	*/
	std::sort( rawSegments->begin(), rawSegments->end(), sort_by_road_class() );

	initSegments( *rawSegments );

	const bool result = !d_segments->empty();
	return result;
}

void KSegmentsCreator::initSegments( const raw_segments_t& rawSegments )
{
	for ( auto it = rawSegments.begin()
		; it != rawSegments.end()
		; ++it, ++d_segmentIndex )
	{
		const SRawSegment& rawSegment = *it;
		addSegment( rawSegment );
	}
}

void KSegmentsCreator::addSegment( const SRawSegment& rawSegment )
{
	const int roadClassIndex = rawSegment.d_roadClass;
	assert( ( 0 <= roadClassIndex ) && ( roadClassIndex <= consts::MaxRoadClassIndex ) );
	d_segments->push_back( SSegment( roadClassIndex ) );

	SSegment& segment = d_segments->back();
	const points_t& rawPoints = rawSegment.d_points;
	segment_points_t* segmentPoints = &segment.d_points;
	addSegmentPoints( rawPoints, segmentPoints );
}

void KSegmentsCreator::addSegmentPoints(
	const points_t& rawPoints,
	segment_points_t* segmentPoints )
{
	const std::size_t pointsCount = rawPoints.size();
	assert( 1 < pointsCount );
	segmentPoints->reserve( pointsCount );

	std::size_t pointIndex = 0;
	for ( auto it = rawPoints.begin()
		; it != rawPoints.end()
		; ++it, ++pointIndex )
	{
		const point_pos_id_t id = composePointPosId( d_segmentIndex, pointIndex );
		const SPoint& point = *it;
		const SPointPos pointPos( id, point );
		segmentPoints->push_back( pointPos );
	}
}

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

class KIntervalSectionsCreator
{
	public:
		explicit KIntervalSectionsCreator( interval_sections_t* intervalSections );

	public:
		void run( const segments_t& segments );

	private:
		void resetContext();
		void traverseSegment(
			const SSegment& segment );
		void addIntervalSections();
		void addHorizontalSection(
			coord_t raw_x0,
			coord_t raw_x1,
			coord_t y );
		void addVerticalSection(
			coord_t y0,
			coord_t y1,
			coord_t x );
		sect_pos_id_t generateSectPosId(
			bool isEndOfSection ) const;

	private:
		std::size_t d_segmentIndex;
		interval_sections_t* d_intervalSections;

		section_id_t d_sectid;
		const SPoint* d_beginSectionPoint;
		const SPoint* d_endSectionPoint;

};

// ----------------------------------------------------------------------------

KIntervalSectionsCreator::KIntervalSectionsCreator(
	interval_sections_t* intervalSections )
	: d_segmentIndex( 0 )
	, d_intervalSections( intervalSections )
	, d_beginSectionPoint( nullptr )
	, d_endSectionPoint( nullptr )
{
}

void KIntervalSectionsCreator::run( const segments_t& segments )
{
	for ( auto it = segments.begin()
		; it != segments.end()
		; ++it, ++d_segmentIndex )
	{
		const SSegment& segment = *it;
		traverseSegment( segment );
		resetContext();
	}
}

void KIntervalSectionsCreator::resetContext()
{
	d_sectid.reset();
	d_beginSectionPoint = d_endSectionPoint = nullptr;
}

void KIntervalSectionsCreator::traverseSegment( const SSegment& segment )
{
	const segment_points_t& points = segment.d_points;
	assert( !points.empty() );
	for ( auto it = points.begin()
		; it != points.end()
		; ++it )
	{
		const SPointPos& segmentPos = *it;
		if ( d_beginSectionPoint == nullptr )
		{
			d_beginSectionPoint = &segmentPos.d_point;
		}
		else
		{
			const std::size_t sectionIndex = std::distance( points.begin(), it ) - 1;
			d_sectid = composeSectionId( d_segmentIndex, sectionIndex );

			d_endSectionPoint = &segmentPos.d_point;
			addIntervalSections();
			d_beginSectionPoint = d_endSectionPoint;
		}
	}
}

void KIntervalSectionsCreator::addIntervalSections()
{
	const coord_t x0 = d_beginSectionPoint->x;
	const coord_t y0 = d_beginSectionPoint->y;
	const coord_t x1 = d_endSectionPoint->x;
	const coord_t y1 = d_endSectionPoint->y;

	if ( x0 != x1 )
	{
		if ( y0 != y1 )
		{
			assert( ( x0 != x1 ) && ( y0 != y1 ) );
			addHorizontalSection( x0, x1, y0 );
			addHorizontalSection( x0, x1, y1 );
			addVerticalSection( y0, y1, x0 );
			addVerticalSection( y0, y1, x1 );
		}
		else
		{
			assert( ( x0 != x1 ) && ( y0 == y1 ) );
			addHorizontalSection( x0, x1, y0 );
		}
	}
	else
	{
		assert( ( x0 == x1 ) && ( y0 != y1 ) );
		addVerticalSection( y0, y1, x0 );
	}
}

void KIntervalSectionsCreator::addHorizontalSection(
	coord_t raw_x0,
	coord_t raw_x1,
	coord_t y )
{
	coord_t x0 = raw_x0;
	coord_t x1 = raw_x1;

	// begin coord should always be less than end one
	if ( x1 < x0 )
		std::swap( x0, x1 );

	assert( x0 < x1 );

	const sect_pos_id_t beginSectPosId = generateSectPosId( false );
	const SPoint beginPoint( x0, y );
	const SSectionPos beginSectionPos( beginSectPosId, beginPoint );

	const sect_pos_id_t endSectPosId = generateSectPosId( true );
	const SPoint endPoint( x1, y );
	const SSectionPos endSectionPos( endSectPosId, endPoint );

	const SIntervalSection intervalSection(
		d_sectid, beginSectionPos, endSectionPos );
	d_intervalSections->push_back( intervalSection );
}

void KIntervalSectionsCreator::addVerticalSection(
	coord_t raw_y0,
	coord_t raw_y1,
	coord_t x )
{
	coord_t y0 = raw_y0;
	coord_t y1 = raw_y1;

	// begin coord should always be less than end one
	if ( y1 < y0 )
		std::swap( y0, y1 );

	assert( y0 < y1 );

	const sect_pos_id_t beginSectPosId = generateSectPosId( false );
	const SPoint beginPoint( x, y0 );
	const SSectionPos beginSectionPos( beginSectPosId, beginPoint );

	const sect_pos_id_t endSectPosId = generateSectPosId( true );
	const SPoint endPoint( x, y1 );
	const SSectionPos endSectionPos( endSectPosId, endPoint );

	const SIntervalSection intervalSection(
		d_sectid, beginSectionPos, endSectionPos );
	d_intervalSections->push_back( intervalSection );
}

sect_pos_id_t KIntervalSectionsCreator::generateSectPosId(
	const bool isEndOfSection ) const
{
	const std::size_t intervalSectionIndex = d_intervalSections->size();
	const sect_pos_id_t result = composeSectPosId( intervalSectionIndex, isEndOfSection );
	return result;
}

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

class KSectionCollector
{
	protected:
		KSectionCollector(
			const KSegmentsManager& segmentsManager,
			const SRect& viewportRect,
			const segments_t& segments,
			const interval_sections_t& intervalSections,
			section_ids_t* sections );

	public:
		virtual ~KSectionCollector() = default;

	protected:
		void preparePoint( point_pos_id_t pointid );

		void storeSection(
			std::size_t segmentIndex,
			std::size_t sectionIndex );
		void storeSection(
			const SIntervalSection& intervalSection );
		void storeSection(
			section_id_t sectid );

		bool doesSectionCrossViewport(
			const SIntervalSection& intervalSection ) const;

		bool uniqueSections();

	protected:
		const KSegmentsManager& d_segmentsManager;
		const SRect& d_viewportRect;
		const segments_t& d_segments;
		const interval_sections_t& d_intervalSections;
		section_ids_t* d_sections;

};

// ----------------------------------------------------------------------------

KSectionCollector::KSectionCollector(
	const KSegmentsManager& segmentsManager,
	const SRect& viewportRect,
	const segments_t& segments,
	const interval_sections_t& intervalSections,
	section_ids_t* sections )
	: d_segmentsManager( segmentsManager )
	, d_viewportRect( viewportRect )
	, d_segments( segments )
	, d_intervalSections( intervalSections )
	, d_sections( sections )
{
}

void KSectionCollector::preparePoint( point_pos_id_t pointid )
{
	std::size_t segmentIndex;
	std::size_t pointIndex;
	decomposePointPosId( pointid, &segmentIndex, &pointIndex );
	const SSegment& segment = d_segments[ segmentIndex ];
	const segment_points_t& points = segment.d_points;
	const std::size_t pointsCount = points.size();
	assert( 1 < pointsCount );
	assert( pointIndex < pointsCount );
	assert( d_viewportRect.contains( points[ pointIndex ].d_point ) );

	const std::size_t lastPointIndex = pointsCount - 1;

	if ( pointIndex < lastPointIndex )
	{
		const std::size_t sectionIndex = pointIndex;
		storeSection( segmentIndex, sectionIndex );
	}

	if ( 0 < pointIndex )
	{
		const std::size_t sectionIndex = pointIndex - 1;
		storeSection( segmentIndex, sectionIndex );
	}
}

void KSectionCollector::storeSection(
	std::size_t segmentIndex,
	std::size_t sectionIndex )
{
	const section_id_t sectid = composeSectionId( segmentIndex, sectionIndex );
	storeSection( sectid );
}

void KSectionCollector::storeSection( const SIntervalSection& intervalSection )
{
	const section_id_t sectid = intervalSection.d_sectid;
	storeSection( sectid );
}

void KSectionCollector::storeSection( section_id_t sectid )
{
	d_sections->push_back( sectid );
}

bool KSectionCollector::doesSectionCrossViewport(
	const SIntervalSection& intervalSection ) const
{
	bool result = false;
	const SPoint& beginPos = intervalSection.d_begin.d_point;
	const SPoint& endPos = intervalSection.d_end.d_point;
	if ( d_viewportRect.contains( beginPos ) || d_viewportRect.contains( endPos ) )
	{
		result = true;
	}
	else
	{
		const coord_t left = d_viewportRect.left;
		const coord_t top = d_viewportRect.top;
		const coord_t right = d_viewportRect.right;
		const coord_t bottom = d_viewportRect.bottom;

		const coord_t x0 = beginPos.x;
		const coord_t y0 = beginPos.y;
		const coord_t x1 = endPos.x;
		const coord_t y1 = endPos.y;

		if ( intervalSection.hasOrientation( Horizontal ) )
		{
			assert( y0 == y1 );
			if ( ( x0 <= left ) && ( left <= x1 )
				&& ( top <= y0 ) && ( y0 <= bottom ) )
			{
				result = true;
			}
		}
		else
		{
			assert( intervalSection.hasOrientation( Vertical ) );
			assert( x0 == x1 );
			if ( ( y0 <= top ) && ( top <= y1 )
				&& ( left <= x0 ) && ( x0 <= right ) )
			{
				result = true;
			}
		}

		if ( !result )
		{
			// cross section
			const coord_t x = std::min( x0, x1 );
			const coord_t y = std::min( y0, y1 );

			if ( ( x < left ) && ( y < top ) )
			{
				const sect_pos_id_t sectposid = intervalSection.d_begin.d_id;
				const SPoint& sectionRectRightBottomCorner
					= d_segmentsManager.getSectionCrossPoint( sectposid );
				if ( ( right < sectionRectRightBottomCorner.x ) && ( bottom < sectionRectRightBottomCorner.y ) )
					result = true;
			}
		}

	}
	return result;
}

bool KSectionCollector::uniqueSections()
{
	std::sort( d_sections->begin(), d_sections->end());
	auto it_new_end = std::unique( d_sections->begin(), d_sections->end() );
	d_sections->erase( it_new_end, d_sections->end() );
	const bool result = !d_sections->empty();
	return result;
}

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

class KPrepareSections : public KSectionCollector
{
	public:
		KPrepareSections(
			const KSegmentsManager& segmentsManager,
			const SRect& viewportRect,
			const segments_t& segments,
			const interval_sections_t& intervalSections,
			section_ids_t* sections );

	public:
		bool run(
			const point_ids_t& pointids,
			const sect_pos_ids_t& sectposids );

	private:
		void preparePoints( const point_ids_t& pointids );

		void prepareSectPositions( const sect_pos_ids_t& sectposids );
		void prepareSectPos( sect_pos_id_t sectposid );

};

// ----------------------------------------------------------------------------

KPrepareSections::KPrepareSections(
	const KSegmentsManager& segmentsManager,
	const SRect& viewportRect,
	const segments_t& segments,
	const interval_sections_t& intervalSections,
	section_ids_t* sections )
	: KSectionCollector( segmentsManager, viewportRect, segments, intervalSections, sections )
{
}

bool KPrepareSections::run(
	const point_ids_t& pointids,
	const sect_pos_ids_t& sectposids )
{
	preparePoints( pointids );
	prepareSectPositions( sectposids );
	const bool result = uniqueSections();
	return result;
}

void KPrepareSections::preparePoints( const point_ids_t& pointids )
{
	for ( const point_pos_id_t& pointid : pointids )
	{
		preparePoint( pointid );
	}
}

void KPrepareSections::prepareSectPositions( const sect_pos_ids_t& sectposids )
{
	for ( const sect_pos_id_t& sectposid : sectposids )
	{
		prepareSectPos( sectposid );
	}
}

void KPrepareSections::prepareSectPos( sect_pos_id_t sectposid )
{
	const std::size_t intervalSectionIndex
		= sectPosId2intervalSectionIndex( sectposid );
	const SIntervalSection& intervalSection = d_intervalSections[ intervalSectionIndex ];
	assert( doesSectionCrossViewport( intervalSection ) );
	storeSection( intervalSection );
}

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

#ifdef BRUTE_FORCE_SELECT_SECTIONS_CHECKER
class KBruteForceSelectSections : public KSectionCollector
{
	public:
		KBruteForceSelectSections(
			const KSegmentsManager& segmentsManager,
			const SRect& viewportRect,
			const segments_t& segments,
			const interval_sections_t& intervalSections,
			section_ids_t* sections );

	public:
		void run();

	private:
		void gatherPointIds();
		void traverseSegment( const SSegment& segment );
		void preparePointPos( const SPointPos& pointPos );

		void gatherIntervalSectionIds();

};

// ----------------------------------------------------------------------------

KBruteForceSelectSections::KBruteForceSelectSections(
	const KSegmentsManager& segmentsManager,
	const SRect& viewportRect,
	const segments_t& segments,
	const interval_sections_t& intervalSections,
	section_ids_t* sections )
	: KSectionCollector( segmentsManager, viewportRect, segments, intervalSections, sections )
{
}

void KBruteForceSelectSections::run()
{
	gatherPointIds();
	gatherIntervalSectionIds();
	uniqueSections();
}

// ----------------------------------------------------------------------------

void KBruteForceSelectSections::gatherPointIds()
{
	for ( const SSegment& segment : d_segments )
	{
		traverseSegment( segment );
	}
}

void KBruteForceSelectSections::traverseSegment( const SSegment& segment )
{
	const segment_points_t& points = segment.d_points;
	for ( const SPointPos& pointPos : points )
	{
		preparePointPos( pointPos );
	}
}

void KBruteForceSelectSections::preparePointPos( const SPointPos& pointPos )
{
	const SPoint& point = pointPos.d_point;
	if ( d_viewportRect.contains( point ) )
	{
		const point_pos_id_t pointid = pointPos.d_id;
		preparePoint( pointid );
	}
}

void KBruteForceSelectSections::gatherIntervalSectionIds()
{
	for ( const SIntervalSection& intervalSection : d_intervalSections )
	{
		if ( doesSectionCrossViewport( intervalSection ) )
			storeSection( intervalSection );
	}
}
#endif // BRUTE_FORCE_SELECT_SECTIONS_CHECKER

} // anonymous namespace

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

KSegmentsManager::~KSegmentsManager()
{
	utils::delete_container( d_roadClasses.begin(), d_roadClasses.end() );
}

// ----------------------------------------------------------------------------

std::size_t KSegmentsManager::maxSegmentCount()
{
	return consts::MaxSegmentId;
}

std::size_t KSegmentsManager::maxSegmentPointsCount()
{
	return consts::MaxPointPosId;
}

// ----------------------------------------------------------------------------

bool KSegmentsManager::getPointPositions( point_positions_t* point_positions ) const
{
	for ( const SSegment& segment : d_segments )
	{
		const segment_points_t& points = segment.d_points;
		for ( const SPointPos& pointPos : points )
		{
			point_positions->push_back( &pointPos );
		}
	}

	const bool result = !point_positions->empty();
	return result;
}

// ----------------------------------------------------------------------------

bool KSegmentsManager::getSectPositions(
	const EOrientation orientation,
	section_positions_t* sect_positions ) const
{
	for ( const SIntervalSection& section : d_intervalSections )
	{
		if ( section.hasOrientation( orientation ) )
		{
			const SSectionPos& beginSectPos = section.d_begin;
			sect_positions->push_back( &beginSectPos );

			const SSectionPos& endSectPos = section.d_end;
			sect_positions->push_back( &endSectPos );
		}
	}

	const bool result = !sect_positions->empty();
	return result;
}

const SSectionPos* KSegmentsManager::getSectionPos( sect_pos_id_t sectposid ) const
{
	std::size_t intervalSectionIndex;
	bool isEndOfsection;
	decomposeSectPosId( sectposid, &intervalSectionIndex, &isEndOfsection );
	const SIntervalSection& intervalSection = d_intervalSections[ intervalSectionIndex ];
	const SSectionPos* result = isEndOfsection ? &intervalSection.d_end : &intervalSection.d_begin;
	return result;
}

const SSectionPos* KSegmentsManager::getSectionBeginPos( const SSectionPos* sectpos ) const
{
	const sect_pos_id_t sectPosId = sectpos->d_id;
	std::size_t rawSectPosId = sectPosId.get();
	rawSectPosId &= ~IsBeginOrEndOfSectionFlagMask;
	const sect_pos_id_t beginSectPosId( rawSectPosId );
	const SSectionPos* result
		= ( beginSectPosId == sectPosId ) ? sectpos : getSectionPos( beginSectPosId );
	return result;
}

const SSectionPos* KSegmentsManager::getSectionEndPos( const SSectionPos* sectpos ) const
{
	const sect_pos_id_t sectPosId = sectpos->d_id;
	std::size_t rawSectPosId = sectPosId.get();
	rawSectPosId |= IsBeginOrEndOfSectionFlagMask;
	const sect_pos_id_t endSectPosId( rawSectPosId );
	const SSectionPos* result
		= ( endSectPosId == sectPosId ) ? sectpos : getSectionPos( endSectPosId );
	return result;
}

SPoint KSegmentsManager::getSectionCrossPoint( sect_pos_id_t sectposid ) const
{
	std::size_t intervalSectionIndex;
	bool isEndOfsection;
	decomposeSectPosId( sectposid, &intervalSectionIndex, &isEndOfsection );
	const SIntervalSection& intervalSection = d_intervalSections[ intervalSectionIndex ];

	const section_id_t sectid = intervalSection.d_sectid;
	SSection section;
	getSection( sectid, &section );

	const SPoint& begin = *section.d_begin;
	const SPoint& end = *section.d_end;

	const coord_t x = std::max( begin.x, end.x );
	const coord_t y = std::max( begin.y, end.y );
	const SPoint result( x, y );
	return result;
}

bool KSegmentsManager::isSection(
	const SSectionPos* beginSectPos,
	const SSectionPos* endSectPos )
{
	const sect_pos_id_t firstSectPosId = beginSectPos->d_id;
	std::size_t firstIntervalSectionIndex;
	bool firstIsEndOfsection;
	decomposeSectPosId( firstSectPosId, &firstIntervalSectionIndex, &firstIsEndOfsection );

	const sect_pos_id_t secondSectPosId = endSectPos->d_id;
	std::size_t secondIntervalSectionIndex;
	bool secondIsEndOfsection;
	decomposeSectPosId( secondSectPosId, &secondIntervalSectionIndex, &secondIsEndOfsection );

	const bool result = ( firstIntervalSectionIndex == secondIntervalSectionIndex )
		&& !firstIsEndOfsection && secondIsEndOfsection;
	return result;
}

// ----------------------------------------------------------------------------

void KSegmentsManager::init( raw_segments_t* rawSegments )
{
	KSegmentsCreator segmentsCreator( d_roadClasses, &d_segments );
	if ( segmentsCreator.run( rawSegments ) )
	{
		KIntervalSectionsCreator intervalSectionsCreator( &d_intervalSections );
		intervalSectionsCreator.run( d_segments );
	}
}

void KSegmentsManager::getSection(
	const section_id_t sectid,
	SSection* section ) const
{
	std::size_t segmentIndex;
	std::size_t sectionIndex;
	decomposeSectionId( sectid, &segmentIndex, &sectionIndex );

	const SSegment& segment = d_segments[ segmentIndex ];
	const int roadClassIndex = segment.d_roadClassIndex;
	section->d_roadClassIndex = roadClassIndex;

	const segment_points_t& points = segment.d_points;
	const std::size_t sectionBeginPointIndex = sectionIndex;
	const std::size_t sectionEndPointIndex = sectionIndex + 1;
	assert( sectionEndPointIndex < points.size() );

	const SPointPos& sectionBeginPointPos = points[ sectionBeginPointIndex ];
	const SPoint& sectionBeginPoint = sectionBeginPointPos.d_point;
	section->d_begin = &sectionBeginPoint;

	const SPointPos& sectionEndPointPos = points[ sectionEndPointIndex ];
	const SPoint& sectionEndPoint = sectionEndPointPos.d_point;
	section->d_end = &sectionEndPoint;
}

bool KSegmentsManager::prepareSections(
	const SRect& viewportRect,
	const point_ids_t& pointids,
	const sect_pos_ids_t& sectposids,
	section_ids_t* sections ) const
{
	KPrepareSections prepareSection( *this, viewportRect, d_segments, d_intervalSections, sections );
	const bool result = prepareSection.run( pointids, sectposids );
	return result;
}

bool KSegmentsManager::compareBruteForceSelectSections(
	const SRect& viewportRect,
	const section_ids_t& sections ) const
{
	bool result = true;
	#ifdef BRUTE_FORCE_SELECT_SECTIONS_CHECKER
	section_ids_t bfSections;
	KBruteForceSelectSections bruteForceSelectSections(
		*this, viewportRect, d_segments, d_intervalSections, &bfSections );
	bruteForceSelectSections.run();

	section_ids_t diff_sections;
	std::set_difference(
		bfSections.begin(),
		bfSections.end(),
		sections.begin(),
		sections.end(),
		std::back_inserter( diff_sections ) );

	section_ids_t false_positive_sections;
	std::set_difference(
		sections.begin(),
		sections.end(),
		bfSections.begin(),
		bfSections.end(),
		std::back_inserter( false_positive_sections ) );

	result = diff_sections.empty() && false_positive_sections.empty();
	assert( ( "diffs found!", result ) );
	#endif
	return result;
}

} // namespace be
