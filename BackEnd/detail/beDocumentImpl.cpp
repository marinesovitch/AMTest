// author: marines marinesovitch alias Darek Slusarczyk 2012-2013, 2022
#include "ph.h"
#include "beDocumentImpl.h"
#include "beInternalDocument.h"
#include "beSegmentsManager.h"
#include "beMapReader.h"
#include "beRangeTree.h"
#include "beIntervalTree.h"
#include "beViewportArea.h"
#include "beUtils.h"
#include "beConsts.h"
#include "beDiagnostics.h"
#include "beConfig.h"

namespace be
{

namespace
{

class KDocument :
	public IInternalDocument,
	public KSegmentsManager
{
	public:
		explicit KDocument( IMapStream* mapStream );
		~KDocument() override = default;

	public:
		// IDocument
		std::string getState() const override;
		void setState( const std::string& stateString ) override;

		color32_t getBkColor() const override;

	public:
		// IInternalDocument
		const SViewData& getViewData() const override;
		bool setViewData( const SViewData& viewData ) override;

		const road_classes_t& getBaseRoadClasses() const override;

		bool selectSections(
			const SRect& viewportRect,
			section_ids_t* sections ) const override;

		void getSection(
			section_id_t sectid,
			SSection* section ) const override;

	private:
		void initRoadClasses( const bools_t& roadClassFlags );
		void initViewport();
		void createRangeTree();
		void createIntervalTree();

	private:
		SViewData d_viewData;
		std::unique_ptr< KRangeTree > d_rangeTree;
		std::unique_ptr< KIntervalTree > d_intervalTree;

};

// ----------------------------------------------------------------------------

KDocument::KDocument( IMapStream* mapStream )
{
	bools_t roadClassFlags;
	raw_segments_t rawSegments;
	SReaderData readerData(
		mapStream,
		&d_viewData.d_mapRect,
		&roadClassFlags,
		&rawSegments );
	if ( be::readMap( readerData ) )
	{
		#ifdef ENABLE_LOGGING
		//diag::dumpPoints( rawSegments );
		#endif
		initRoadClasses( roadClassFlags );
		init( &rawSegments );
		createRangeTree();
		createIntervalTree();
		initViewport();
	}
}

// ----------------------------------------------------------------------------

std::string KDocument::getState() const
{
	std::ostringstream os;
	os << d_viewData.d_viewportCenter.x << ' '
		<< d_viewData.d_viewportCenter.y << ' '
		<< d_viewData.d_zoomFactor;
	const std::string& result = os.str();
	return result;
}

void KDocument::setState( const std::string& stateString )
{
	std::istringstream is( stateString );
	is >> d_viewData.d_viewportCenter.x
		>> d_viewData.d_viewportCenter.y
		>> d_viewData.d_zoomFactor;
}

color32_t KDocument::getBkColor() const
{
	return consts::BackgroundColor32;
}

// ----------------------------------------------------------------------------

const SViewData& KDocument::getViewData() const
{
	return d_viewData;
}

bool KDocument::setViewData( const SViewData& viewData )
{
	const bool changed = ( d_viewData != viewData );
	if ( changed )
	{
		const SSize& deviceSize = viewData.d_deviceSize;
		const SSize& screenSize = viewData.d_screenSize;
		const int zoomFactor = viewData.d_zoomFactor;

		d_viewData.d_viewportCenter = viewData.d_viewportCenter;
		d_viewData.d_deviceSize = deviceSize;
		d_viewData.d_screenSize = screenSize;
		d_viewData.d_zoomFactor = zoomFactor;

		assert( ( 0 <= deviceSize.width ) && ( 0 <= deviceSize.height ) );
		assert( utils::isValueInRange( 0, screenSize.width, deviceSize.width, true ) );
		assert( utils::isValueInRange( 0, screenSize.height, deviceSize.height, true ) );
		assert( utils::isValueInRange( consts::MinZoomFactor, zoomFactor, consts::MaxZoomFactor, true ) );
	}
	return changed;
}

const road_classes_t& KDocument::getBaseRoadClasses() const
{
	return d_roadClasses;
}

bool KDocument::selectSections(
	const SRect& viewportRect,
	section_ids_t* sections ) const
{
	const KViewportArea viewportArea( viewportRect );
	point_ids_t pointids;
	if ( d_rangeTree )
		d_rangeTree->selectPoints( viewportArea, &pointids );
	sect_pos_ids_t sectposids;
	if ( d_intervalTree )
		d_intervalTree->selectSectPositions( viewportArea, &sectposids );
	const bool result = prepareSections( viewportRect, pointids, sectposids, sections );
	assert( compareBruteForceSelectSections( viewportRect, *sections ) );
	return result;
}

void KDocument::getSection(
	const section_id_t sectid,
	SSection* section ) const
{
	KSegmentsManager::getSection( sectid, section );
}

// ----------------------------------------------------------------------------

void KDocument::initRoadClasses( const bools_t& roadClassFlags )
{
	d_roadClasses.reserve( roadClassFlags.size() );
	int roadClassIndex = 0;
	const int lastRoadClassIndex = static_cast<int>(roadClassFlags.size() - 1);
	assert( lastRoadClassIndex <= consts::MaxRoadClassIndex );
	for ( auto it = roadClassFlags.begin()
		; it != roadClassFlags.end()
		; ++it, ++roadClassIndex )
	{
		SRoadClass* roadClass = nullptr;

		const bool isRoadClass = *it;
		if ( isRoadClass )
		{
			const color_t roadClassColor = consts::RoadClassColors[ roadClassIndex ];
			if ( roadClassIndex != lastRoadClassIndex )
			{
				const coord_t thickness = std::max( 1, roadClassIndex );
				roadClass = new SRoadClass( thickness, roadClassColor );
			}
			else
			{
				roadClass
					= new SRoadClass(
						roadClassIndex,
						roadClassColor,
						consts::OutlineDefaultThickness,
						consts::OutlineColor );
			}

		}

		d_roadClasses.push_back( roadClass );
	}
	assert( roadClassFlags.size() == d_roadClasses.size() );
}

void KDocument::initViewport()
{
	d_viewData.d_viewportCenter = consts::InitViewportCenter;
	d_viewData.d_zoomFactor = consts::InitZoomFactor;
}

void KDocument::createRangeTree()
{
	d_rangeTree = std::make_unique<KRangeTree>( *this );
}

void KDocument::createIntervalTree()
{
	d_intervalTree = std::make_unique<KIntervalTree>( *this );
}

} // anonymous namespace

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

IInternalDocument* createDocument( IMapStream* mapStream )
{
	return new KDocument( mapStream );
}

} // namespace be
