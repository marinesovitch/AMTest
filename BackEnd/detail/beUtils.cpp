// author: marines marinesovitch alias Darek Slusarczyk 2012-2013, 2022
#include "ph.h"
#include "beUtils.h"
#include "beBigCoordTypes.h"
#include "beConsts.h"

namespace be
{

namespace utils
{

bool checkSectionLength( const SPoint& begin, const SPoint& end )
{
	const big_coord_t maxSectionLength = consts::MaxSectionLength;
	const coord_t x0 = begin.x;
	const coord_t x1 = end.x;
	const coord_t y0 = begin.y;
	const coord_t y1 = end.y;
	const bool result
		= std::abs( x1 - x0 ) < maxSectionLength
		&& std::abs( y1 - y0 ) < maxSectionLength;
	return result;
}

bool isShiftOverflow( coord_t coord, int shiftCounter )
{
	const coord_t shiftedCoord = coord << shiftCounter;
	const big_coord_t bigCoord = static_cast< big_coord_t >( coord ) << shiftCounter;
	const bool overflow = shiftedCoord != bigCoord;
	return overflow;
}

} // namespace utils

} // namespace be
