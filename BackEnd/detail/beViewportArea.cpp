// author: marines marinesovitch alias Darek Slusarczyk 2012-2013
#include "ph.h"
#include "beViewportArea.h"
#include "beConsts.h"

namespace be
{

SViewportBorder::SViewportBorder( 
	const SPoint& border,
	const bool isMin )
	: d_border( border )
	, d_isMin( isMin )
{
}

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

KViewportArea::KViewportArea( const SRect& rect )
	: d_rect( rect )
	, d_leftEdge( SViewportBorder( SPoint( rect.left, consts::MinCoord ), true ) )
	, d_rightEdge( SViewportBorder( SPoint( rect.right, consts::MaxCoord ), false ) )
	, d_topEdge( SViewportBorder( SPoint( consts::MinCoord, rect.top ), true ) )
	, d_bottomEdge( SViewportBorder( SPoint( consts::MaxCoord, rect.bottom ), false ) )
	, d_mapTopBorder( SViewportBorder( SPoint( consts::MinCoord, consts::MinCoord ), false ) )
	, d_leftAxis( SViewportBorder( SPoint( rect.left, consts::MinCoord ), false ) )
	, d_rightAxis( SViewportBorder( SPoint( rect.right, consts::MaxCoord ), true ) )
	, d_topAxis( SViewportBorder( SPoint( consts::MinCoord, rect.top ), false ) )
	, d_bottomAxis( SViewportBorder( SPoint( consts::MaxCoord, rect.bottom ), true ) )
{
}

// ----------------------------------------------------------------------------

const SViewportBorder& KViewportArea::getLeftEdge() const
{
	return d_leftEdge;
}

const SViewportBorder& KViewportArea::getRightEdge() const
{
	return d_rightEdge;
}

const SViewportBorder& KViewportArea::getTopEdge() const
{
	return d_topEdge;
}

const SViewportBorder& KViewportArea::getBottomEdge() const
{
	return d_bottomEdge;
}

// ----------------------------------------------------------------------------

const SViewportBorder& KViewportArea::getMapTopBorder() const
{
	return d_mapTopBorder;
}

// ----------------------------------------------------------------------------

const SViewportBorder& KViewportArea::getLeftAxis() const
{
	return d_leftAxis;
}

const SViewportBorder& KViewportArea::getRightAxis() const
{
	return d_rightAxis;
}

const SViewportBorder& KViewportArea::getTopAxis() const
{
	return d_topAxis;
}

const SViewportBorder& KViewportArea::getBottomAxis() const
{
	return d_bottomAxis;
}

// ----------------------------------------------------------------------------

bool KViewportArea::contains( const SPoint& pos ) const
{
	const SRect& areaRect = d_rect;
	const bool result = areaRect.contains( pos );
	return result;
}

} // namespace be
