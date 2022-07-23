// author: marines marinesovitch alias Darek Slusarczyk 2012-2013, 2022
#ifndef INC_BE_RANGE_TREE_H
#define INC_BE_RANGE_TREE_H

#include "beInternalTypes.h"

namespace be
{

class KSegmentsManager;
class KViewportArea;

class KRangeTree
{
	public:
		KRangeTree( const KSegmentsManager& segmentsManager );
		~KRangeTree();

	public:
		void selectPoints(
			const KViewportArea& viewportArea,
			point_ids_t* pointids ) const;

	private:
		struct Impl;
		Impl* impl;

};

} // namespace be

#endif
