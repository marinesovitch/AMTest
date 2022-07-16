// author: marines marinesovitch alias Darek Slusarczyk 2012-2013
#ifndef INC_BE_CONSTS_H
#define INC_BE_CONSTS_H

#include "beInternalTypes.h"

#ifndef ANDROID
#pragma warning( disable : 4308 )
#endif

namespace be
{

namespace consts
{

// ----------------------------------------------------------------------------
// graphics

//#ifdef NDEBUG
const SPoint InitViewportCenter( 29000, 22000 );
const int InitZoomFactor = 8;
//#else
//const SPoint InitViewportCenter( 0, 0 );
//const int InitZoomFactor = 2;
//#endif

const int MinZoomFactor = 0; //-2;
const int MaxZoomFactor = 22;

const coord_t MinCoord = std::numeric_limits< coord_t >::min();
const coord_t MaxCoord = std::numeric_limits< coord_t >::max();

const coord_t MinScreenDim = 16;

const coord_t OutlineDefaultThickness = 1;

// ----------------------------------------------------------------------------
// map

const int MaxRoadClassIndex = 7;

const std::size_t BitsForSegmentId = 16;
const std::size_t MaxSegmentId = ( 1 << BitsForSegmentId ) - 1;

const std::size_t BitsForSectionId = section_id_t::size_of() * 8 - BitsForSegmentId;
const std::size_t MaxSectionId = ( std::size_t(1) << BitsForSectionId ) - 1;

/*
	point_pos_id_t
	31-16 - segment id
	15-00 - point id
*/
const std::size_t BitsForPointPosId = point_pos_id_t::size_of() * 8 - BitsForSegmentId;
const std::size_t MaxPointPosId = ( std::size_t(1) << BitsForPointPosId ) - 1;

/*
	sect_pos_id_t
	bits
	31-01 - interval section id
	   00 - 0 == beginning of section, 1 == end of section
*/
const std::size_t BitsForIsBeginOrEndOfSectionFlag = 1;

const std::size_t BitsForIntervalSectionId = section_id_t::size_of() * 8 - BitsForIsBeginOrEndOfSectionFlag;
const std::size_t MaxIntervalSectionId = ( std::size_t(1) << BitsForIntervalSectionId ) - 1U;

const coord_t MaxSectionLength = MaxCoord >> 2;

// ----------------------------------------------------------------------------
// colors

#ifdef ANDROID
#define MAKERGB(Red,Green,Blue) (color_t( (Blue/8) | ((Green/4)<<5) | ((Red/8)<<11) ))
const color32_t OPAQUE_ALPHA = 0xFF;
#define MAKERGB32(Red,Green,Blue) (color32_t( Blue | (Green<<8) | (Red<<16) | (OPAQUE_ALPHA<<24) ))
#else
#define MAKERGB(Red,Green,Blue) (color_t( Red | (Green<<8) | (Blue<<16) ))
#define MAKERGB32(Red,Green,Blue) (color32_t( Red | (Green<<8) | (Blue<<16) ))
#endif // ANDROID

const color_t BackgroundColor = MAKERGB(100,100,100);
const color32_t BackgroundColor32 = MAKERGB32(100,100,100);
const color_t OutlineColor = MAKERGB(255,255,255);

extern color_t RoadClassColors[ MaxRoadClassIndex + 1 ];

} // namespace consts

} // namespace be

#ifndef ANDROID
#pragma warning( default : 4308 )
#endif

#endif
