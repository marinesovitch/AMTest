// author: marines marinesovitch alias Darek Slusarczyk 2012-2013, 2022
#ifndef INC_BE_MAP_READER_H
#define INC_BE_MAP_READER_H

#include "beInternalTypes.h"

namespace be
{

struct IMapStream;

struct SReaderData
{
	SReaderData(
		IMapStream* mapStream,
		SRect* mapRect,
		bools_t* d_roadClasses,
		raw_segments_t* segments );

	IMapStream* d_mapStream;
	SRect* d_mapRect;
	bools_t* d_roadClasses;
	raw_segments_t* d_segments;
};

bool readMap( const SReaderData& data );

} // namespace be

#endif
