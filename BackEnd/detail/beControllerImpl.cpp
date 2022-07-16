// author: marines marinesovitch alias Darek Slusarczyk 2012-2013
#include "ph.h"
#include "beControllerImpl.h"
#include "beController.h"
#include "beInternalDocument.h"
#include "beContentsGenerator.h"
#include "beUtils.h"
#include "beBigCoordTypes.h"
#include "beConsts.h"
#include "beDiagnostics.h"
#include "beConfig.h"

namespace be
{

namespace
{

const SBigRect MaxViewportRect( consts::MinCoord, consts::MinCoord, consts::MaxCoord, consts::MaxCoord );

// ----------------------------------------------------------------------------

big_coord_t zoomCoord( coord_t coord, int zoomFactor )
{
	big_coord_t result = coord;
	if ( 0 < zoomFactor )
		result <<= zoomFactor;
	else if ( zoomFactor < 0 )
		result >>= -zoomFactor;
	return result;
}

SBigSize zoomSize( const SSize& size, int zoomFactor )
{
	const big_coord_t width = zoomCoord( size.width, zoomFactor );
	const big_coord_t height = zoomCoord( size.height, zoomFactor );
	const SBigSize result( width, height );
	return result;
}

// ----------------------------------------------------------------------------

class KController : public IController
{
	public:
		explicit KController( IInternalDocument* document );
		~KController() override = default;

	public:
		// IController
		bool setDeviceSize( const SSize& deviceSize ) override;
		bool move( const SMoveData& moveData ) override;
		bool zoom( const SZoomData& zoomData ) override;
		bool resetView() override;

		bool generateContents( IBitmap* bitmap ) override;

		std::string getParamsDescription() const override;

	private:
		SSize calcScreenSize(
			const SSize& deviceSize,
			int zoomFactor ) const;
		int calcZoomFactor(
			const SZoomData& zoomData,
			int currentZoomFactor ) const;

	private:
		SBigRect calcViewportRect(
			const SBigPoint& viewportCenter,
			const SSize& screenSize,
			int zoomFactor ) const;
		SRect calcViewportRect(
			const SViewData& viewData ) const;

		void updateViewport(
			const SSize& deviceSize,
			const SSize& screenSize,
			const SPoint& currentViewportCenter,
			const SBigSize& delta,
			int zoomFactor,
			const SBigRect& clipRect,
			SPoint* viewportCenter ) const;

		SPoint calcViewportCenter(
			const SMoveData& moveData,
			const SRect& screenRect ) const;
		SPoint calcViewportCenterInDirection(
			const SMoveData& moveData,
			const SRect& screenRect ) const;
		SBigSize calcZoomDelta(
			const SSize& deviceSize,
			const SSize& screenSize,
			int currentZoomFactor,
			int newZoomFactor,
			const SZoomData& zoomData ) const;
		SBigSize calcDeltaForAnchoredScreenPoint(
			const SRect& screenRect,
			int currentZoomFactor,
			int newZoomFactor,
			const SPoint& screenPoint ) const;
		SBigSize calcDelta(
			const SSize& deviceSize,
			const SSize& screenSize,
			int zoomFactor,
			const SPoint& newViewportCenter ) const;
		coord_t calcCoordDelta(
			coord_t deviceDimSize,
			coord_t screenDimSize,
			coord_t clickScreenCoord ) const;
		void correctPoint(
			const SBigRect& viewportRect,
			const SBigRect& clipRect,
			SBigPoint* viewportCenter ) const;
		void correctCoord(
			big_coord_t viewportMinPos,
			big_coord_t viewportMaxPos,
			big_coord_t clipRectMinPos,
			big_coord_t clipRectMaxPos,
			big_coord_t* viewportCenterCoord ) const;

	private:
		bool canGenerateContents( const SViewData& viewData ) const;
		bool updateViewData( const SViewData& viewData );

	private:
		void dumpState() const;

	private:
		IInternalDocument* d_document;

};

// ----------------------------------------------------------------------------

KController::KController( IInternalDocument* document )
	: d_document( document )
{
}

// ----------------------------------------------------------------------------

bool KController::setDeviceSize( const SSize& deviceSize )
{
	SViewData viewData = d_document->getViewData();

	const SSize& currentScreenSize = viewData.d_screenSize;
	const int zoomFactor = viewData.d_zoomFactor;
	const SSize& newScreenSize = calcScreenSize( deviceSize, zoomFactor );

	if ( currentScreenSize != newScreenSize )
	{
		const SBigSize nullDelta;
		updateViewport(
			deviceSize,
			newScreenSize,
			viewData.d_viewportCenter,
			nullDelta,
			zoomFactor,
			MaxViewportRect,
			&viewData.d_viewportCenter );

		viewData.d_screenSize = newScreenSize;
	}

	viewData.d_deviceSize = deviceSize;

	const bool viewChanged = updateViewData( viewData );
	return viewChanged;
}

bool KController::move( const SMoveData& moveData )
{
	SViewData viewData = d_document->getViewData();

	const SSize& deviceSize = viewData.d_deviceSize;
	const SSize& screenSize = viewData.d_screenSize;
	const int zoomFactor = viewData.d_zoomFactor;
	const SPoint& newViewportCenter = calcViewportCenter( moveData, screenSize );
	const SBigSize& delta = calcDelta(
		deviceSize,
		screenSize,
		zoomFactor,
		newViewportCenter );
	updateViewport(
		deviceSize,
		screenSize,
		viewData.d_viewportCenter,
		delta,
		zoomFactor,
		MaxViewportRect,
		&viewData.d_viewportCenter );


	const bool viewChanged = updateViewData( viewData );
	return viewChanged;
}

bool KController::zoom( const SZoomData& zoomData )
{
	bool viewChanged = false;

	SViewData viewData = d_document->getViewData();
	const int currentZoomFactor = viewData.d_zoomFactor;
	const int newZoomFactor = calcZoomFactor( zoomData, currentZoomFactor );
	if ( currentZoomFactor != newZoomFactor )
	{
		const SSize& deviceSize = viewData.d_deviceSize;
		const SSize& newScreenSize = calcScreenSize( viewData.d_deviceSize, newZoomFactor );
		SPoint& viewportCenter = viewData.d_viewportCenter;

		const SBigSize& delta = calcZoomDelta(
			deviceSize,
			newScreenSize,
			currentZoomFactor,
			newZoomFactor,
			zoomData );

		const SZoomData::EKind zoomKind = zoomData.d_kind;
		if ( zoomKind == SZoomData::ZoomIn )
		{
			const SBigRect& clipRect
				= zoomData.d_zoomInPlace ? SBigRect(calcViewportRect( viewData )) : MaxViewportRect;
			updateViewport(
				deviceSize,
				newScreenSize,
				viewportCenter,
				delta,
				newZoomFactor,
				clipRect,
				&viewportCenter );
		}
		else
		{
			assert( zoomKind == SZoomData::ZoomOut );
			updateViewport(
				deviceSize,
				newScreenSize,
				viewportCenter,
				delta,
				newZoomFactor,
				MaxViewportRect,
				&viewportCenter );
		}

		viewData.d_screenSize = newScreenSize;
		viewData.d_zoomFactor = newZoomFactor;

		viewChanged = updateViewData( viewData );
	}
	return viewChanged;
}

bool KController::resetView()
{
	SViewData viewData = d_document->getViewData();

	viewData.d_screenSize = calcScreenSize( viewData.d_deviceSize, consts::InitZoomFactor );
	viewData.d_viewportCenter = consts::InitViewportCenter;
	viewData.d_zoomFactor = consts::InitZoomFactor;

	const SBigSize nullDelta;
	updateViewport(
		viewData.d_deviceSize,
		viewData.d_screenSize,
		viewData.d_viewportCenter,
		nullDelta,
		viewData.d_zoomFactor,
		MaxViewportRect,
		&viewData.d_viewportCenter );

	const bool viewChanged = updateViewData( viewData );
	return viewChanged;
}

bool KController::generateContents( IBitmap* bitmap )
{
	bool result = false;

	const SViewData& viewData = d_document->getViewData();
	if ( canGenerateContents( viewData ) )
	{
		const SRect& viewportRect = calcViewportRect( viewData );
		section_ids_t sectionids;
		if ( d_document->selectSections( viewportRect, &sectionids ) )
		{
			#ifdef ENABLE_LOGGING
			diag::dumpSections( d_document, sectionids );
			#endif
			SContentsGeneratorData generatorData( *d_document, viewData, viewportRect, sectionids, bitmap );
			result = be::generateViewContents( &generatorData );
		}
	}

	return result;
}

std::string KController::getParamsDescription() const
{
	const SViewData& viewData = d_document->getViewData();
	const SRect& viewportRect = calcViewportRect( viewData );

	std::ostringstream os;
	os << "l " << viewportRect.left << " t " << viewportRect.top
		<< " r " << viewportRect.right << " b " << viewportRect.bottom
		<< " zoom " << viewData.d_zoomFactor;

	const std::string& result = os.str();
	return result;
}

// ----------------------------------------------------------------------------

SSize KController::calcScreenSize(
	const SSize& deviceSize,
	int zoomFactor ) const
{
	SSize newScreenSize;
	const int CoordBitsCount = sizeof( coord_t ) * 8;
	const int MinScreenDimZoomFactor = CoordBitsCount - consts::MaxZoomFactor;
	if ( zoomFactor <= MinScreenDimZoomFactor )
	{
		newScreenSize = deviceSize;
	}
	else
	{
		assert( ( 0 < zoomFactor ) && ( zoomFactor <= consts::MaxZoomFactor ) );
		const int MaxScreenDimShift = CoordBitsCount - zoomFactor;
		const coord_t maxScreenDim = ( 1 << MaxScreenDimShift );
		newScreenSize.width = std::min( deviceSize.width, maxScreenDim );
		newScreenSize.height = std::min( deviceSize.height, maxScreenDim );
	}
	return newScreenSize;
}

int KController::calcZoomFactor(
	const SZoomData& zoomData,
	int currentZoomFactor ) const
{
	int newZoomFactor = currentZoomFactor;

	const int zoomDelta = zoomData.d_delta;

	const SZoomData::EKind zoomKind = zoomData.d_kind;
	if ( zoomKind == SZoomData::ZoomIn )
	{
		newZoomFactor -= zoomDelta;
		if ( newZoomFactor < consts::MinZoomFactor )
			newZoomFactor = consts::MinZoomFactor;
	}
	else
	{
		assert( zoomKind == SZoomData::ZoomOut );
		newZoomFactor += zoomDelta;
		if ( consts::MaxZoomFactor < newZoomFactor )
			newZoomFactor = consts::MaxZoomFactor;
	}

	return newZoomFactor;
}

// ----------------------------------------------------------------------------

SBigRect KController::calcViewportRect(
	const SBigPoint& viewportCenter,
	const SSize& rawScreenSize,
	int zoomFactor ) const
{
	const SBigSize& screenSize = zoomSize( rawScreenSize, zoomFactor );
	const big_coord_t left = viewportCenter.x - ( screenSize.width >> 1 );
	const big_coord_t top = viewportCenter.y - ( screenSize.height >> 1 );
	const SBigPoint leftTopCorner( left, top );
	const SBigRect result( leftTopCorner, screenSize );
	return result;
}

SRect KController::calcViewportRect(
	const SViewData& viewData ) const
{
	const SBigPoint viewportCenter(viewData.d_viewportCenter);
	const SSize& screenSize = viewData.d_screenSize;
	const int zoomFactor = viewData.d_zoomFactor;
	const SBigRect rawViewportRect(calcViewportRect( viewportCenter, screenSize, zoomFactor ));
	const big_coord_t left = std::max( rawViewportRect.left, static_cast< big_coord_t>( consts::MinCoord ) );
	const big_coord_t top = std::max( rawViewportRect.top, static_cast< big_coord_t>( consts::MinCoord ) );
	const big_coord_t right = std::min( rawViewportRect.right, static_cast< big_coord_t>( consts::MaxCoord ) );
	const big_coord_t bottom = std::min( rawViewportRect.bottom, static_cast< big_coord_t>( consts::MaxCoord ) );
	const SBigRect viewportRect( left, top, right, bottom );
	const SRect& result = viewportRect.getRect();
	return result;
}

void KController::updateViewport(
	const SSize& deviceSize,
	const SSize& screenSize,
	const SPoint& currentViewportCenter,
	const SBigSize& delta,
	int zoomFactor,
	const SBigRect& clipRect,
	SPoint* viewportCenter ) const
{
	const big_coord_t x = currentViewportCenter.x + delta.width;
	const big_coord_t y = currentViewportCenter.y + delta.height;
	SBigPoint newViewportCenter( x, y );

	const SBigRect& newViewportRect = calcViewportRect( newViewportCenter, screenSize, zoomFactor );
	correctPoint( newViewportRect, clipRect, &newViewportCenter );

	if ( newViewportCenter != currentViewportCenter )
	{
		*viewportCenter = newViewportCenter.getPoint();
	}
}

SPoint KController::calcViewportCenter(
	const SMoveData& moveData,
	const SRect& screenRect ) const
{
	SPoint result;

	const SMoveData::EKind moveKind = moveData.d_kind;
	if ( moveKind == SMoveData::MoveToPoint )
	{
		result = moveData.d_focusScreenPoint;
	}
	else if ( moveKind == SMoveData::MoveInDirection )
	{
		result = calcViewportCenterInDirection( moveData, screenRect );
	}
	else
	{
		assert( moveKind == SMoveData::MoveDelta );
		const SPoint& screenCenter = screenRect.getCenter();
		const SSize& moveDelta = moveData.d_delta;
		result.x = screenCenter.x + moveDelta.width;
		result.y = screenCenter.y + moveDelta.height;
	}

	return result;
}

SPoint KController::calcViewportCenterInDirection(
	const SMoveData& moveData,
	const SRect& screenRect ) const
{
	SPoint result;

	assert( moveData.d_kind == SMoveData::MoveInDirection );
	const EDirection direction = moveData.d_direction;
	const SPoint& screenCenter = screenRect.getCenter();
	switch ( direction )
	{
		case North:
			result.x = screenCenter.x;
			result.y = screenRect.top;
			break;

		case East:
			result.x = screenRect.right;
			result.y = screenCenter.y;
			break;

		case South:
			result.x = screenCenter.x;
			result.y = screenRect.bottom;
			break;

		case West:
			result.x = screenRect.left;
			result.y = screenCenter.y;
			break;

		default:
			assert( !"unexpected direction" );
	}

	return result;
}

SBigSize KController::calcZoomDelta(
	const SSize& deviceSize,
	const SSize& screenSize,
	int currentZoomFactor,
	int newZoomFactor,
	const SZoomData& zoomData ) const
{
	SBigSize result;
	const bool zoomInPlace = zoomData.d_zoomInPlace;
	const SZoomData::EKind zoomKind = zoomData.d_kind;
	const SPoint& focusScreenPoint = zoomData.d_focusScreenPoint;
	if ( zoomData.d_zoomInPlace )
	{
		result = calcDeltaForAnchoredScreenPoint(
			screenSize,
			currentZoomFactor,
			newZoomFactor,
			focusScreenPoint );
	}
	else if ( zoomKind == SZoomData::ZoomIn )
	{
		result = calcDelta(
			deviceSize,
			screenSize,
			currentZoomFactor,
			focusScreenPoint );
	}
	else
	{
		assert( !zoomInPlace && ( zoomKind == SZoomData::ZoomOut ) );
	}
	return result;
}

SBigSize KController::calcDeltaForAnchoredScreenPoint(
	const SRect& screenRect,
	int currentZoomFactor,
	int newZoomFactor,
	const SPoint& screenPoint ) const
{
	const SPoint& screenCenter = screenRect.getCenter();
	const SSize& screenDiff = screenCenter - screenPoint;
	const SBigSize& currentCenterOffset = zoomSize( screenDiff, currentZoomFactor );
	const SBigSize& newCenterOffset = zoomSize( screenDiff, newZoomFactor );
	const SBigSize& result = newCenterOffset - currentCenterOffset;
	return result;
}

SBigSize KController::calcDelta(
	const SSize& deviceSize,
	const SSize& screenSize,
	int zoomFactor,
	const SPoint& newViewportCenter ) const
{
	const coord_t raw_delta_x = calcCoordDelta( deviceSize.width, screenSize.width, newViewportCenter.x );
	const coord_t raw_delta_y = calcCoordDelta( deviceSize.height, screenSize.height, newViewportCenter.y );

	const big_coord_t delta_x = zoomCoord( raw_delta_x, zoomFactor );
	const big_coord_t delta_y = zoomCoord( raw_delta_y, zoomFactor );

	const SBigSize result( delta_x, delta_y );
	return result;
}

coord_t KController::calcCoordDelta(
	coord_t deviceDimSize,
	coord_t screenDimSize,
	coord_t clickScreenCoord ) const
{
	coord_t result = 0;
	if ( screenDimSize == deviceDimSize )
	{
		result = clickScreenCoord - ( screenDimSize >> 1 );
	}
	else
	{
		// the whole map is smaller than device view, so it makes no
		// sense to calc delta
		assert( screenDimSize < deviceDimSize );
	}

	return result;
}

void KController::correctPoint(
	const SBigRect& viewportRect,
	const SBigRect& clipRect,
	SBigPoint* viewportCenter ) const
{
	correctCoord(
		viewportRect.left,
		viewportRect.right,
		clipRect.left,
		clipRect.right,
		&viewportCenter->x );

	correctCoord(
		viewportRect.top,
		viewportRect.bottom,
		clipRect.top,
		clipRect.bottom,
		&viewportCenter->y );
}

void KController::correctCoord(
	const big_coord_t viewportMinPos,
	const big_coord_t viewportMaxPos,
	const big_coord_t clipRectMinPos,
	const big_coord_t clipRectMaxPos,
	big_coord_t* viewportCenterCoord ) const
{
	big_coord_t offset = 0;

	if ( viewportMinPos < clipRectMinPos )
		offset = clipRectMinPos - viewportMinPos;
	else if ( clipRectMaxPos < viewportMaxPos )
		offset = clipRectMaxPos - viewportMaxPos;

	if ( offset != 0 )
		*viewportCenterCoord += offset;
}

// ----------------------------------------------------------------------------

bool KController::canGenerateContents( const SViewData& viewData ) const
{
	const SSize& screenSize = viewData.d_screenSize;
	const bool result = ( consts::MinScreenDim <= screenSize.width )
		&& ( consts::MinScreenDim <= screenSize.height );
	return result;
}

bool KController::updateViewData( const SViewData& viewData )
{
	const bool viewChanged = d_document->setViewData( viewData );
	dumpState();
	return viewChanged;
}

// ----------------------------------------------------------------------------

inline void KController::dumpState() const
{
	#ifdef ENABLE_LOGGING
	const SViewData& viewData = d_document->getViewData();
	diag::dumpView( viewData );
	const SRect& viewportRect = calcViewportRect( viewData );
	diag::dumpRect( viewportRect );
	#endif
}

} // anonymous namespace

// ----------------------------------------------------------------------------

IController* createController( IInternalDocument* document )
{
	IController* controller = new KController( document );
	return controller;
}

} // namespace be
