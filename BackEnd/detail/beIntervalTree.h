// author: marines marinesovitch alias Darek Slusarczyk 2012-2013
#ifndef INC_BE_INTERVAL_TREE_H
#define INC_BE_INTERVAL_TREE_H

#include "beInternalTypes.h"

namespace be
{

class KSegmentsManager;
class KViewportArea;

class KIntervalTree
{
	public:
		KIntervalTree( const KSegmentsManager& segmentsManager );
		~KIntervalTree();

	public:
		void selectSectPositions(
			const KViewportArea& viewportArea,
			sect_pos_ids_t* sectposids ) const;

	private:
		class Impl;
		Impl* impl;

};

} // namespace be

#endif
