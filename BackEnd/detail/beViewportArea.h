// author: marines marinesovitch alias Darek Slusarczyk 2012-2013, 2022
#ifndef INC_BE_VIEWPORT_AREA_H
#define INC_BE_VIEWPORT_AREA_H

#include "beInternalTypes.h"

namespace be
{

struct SViewportBorder
{
	SViewportBorder(
		const SPoint& border,
		const bool isMin );

	bool isMin() const
	{
		return d_isMin;
	}

	bool isMax() const
	{
		return !d_isMin;
	}

	const SPoint d_border;
	bool d_isMin;
};

class KViewportArea
{
	public:
		KViewportArea( const SRect& rc );
		~KViewportArea() = default;

	public:
		const SViewportBorder& getLeftEdge() const;
		const SViewportBorder& getRightEdge() const;
		const SViewportBorder& getTopEdge() const;
		const SViewportBorder& getBottomEdge() const;

		const SViewportBorder& getMapTopBorder() const;

		const SViewportBorder& getLeftAxis() const;
		const SViewportBorder& getRightAxis() const;
		const SViewportBorder& getTopAxis() const;
		const SViewportBorder& getBottomAxis() const;

	public:
		bool contains( const SPoint& pos ) const;

	private:
		const SRect d_rect;

		const SViewportBorder d_leftEdge;
		const SViewportBorder d_rightEdge;
		const SViewportBorder d_topEdge;
		const SViewportBorder d_bottomEdge;

		const SViewportBorder d_mapTopBorder;

		const SViewportBorder d_leftAxis;
		const SViewportBorder d_rightAxis;
		const SViewportBorder d_topAxis;
		const SViewportBorder d_bottomAxis;

};

} // namespace be

#endif
