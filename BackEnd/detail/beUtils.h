// author: marines marinesovitch alias Darek Slusarczyk 2012-2013, 2022
#ifndef INC_BE_UTILS_H
#define INC_BE_UTILS_H

#include "beInternalTypes.h"
#include "beBigCoordTypes.h"

namespace be
{

namespace utils
{

template< typename TKey, typename TMask >
bool check_flag( TKey key, TMask mask )
{
	const bool result = ( key & mask ) == mask;
	return result;
}

template< typename TValue >
bool is_odd( const TValue value )
{
	const bool result = ( value & 1 ) == 1;
	return result;
}

template< typename TIterator, typename TPred >
bool is_sorted( TIterator begin, TIterator end, TPred pred )
{
	bool result = true;
	if ( begin != end )
	{
		for ( TIterator it = begin + 1; it != end; ++it  )
		{
			if ( !pred( *( it - 1 ), *it ) )
			{
				result = false;
				break;
			}
		}
	}
	return result;
}

template< typename item_id_it >
item_id_it get_median(
	item_id_it begin,
	item_id_it end )
{
	assert( begin != end );

	const std::size_t itemCount = std::distance( begin, end );
	std::size_t middleOffset = itemCount / 2;
	if ( ( itemCount & 1 ) == 0 ) // if itemsCount is even, then decrement result
		--middleOffset;

	item_id_it median_it = begin + middleOffset;
	assert( median_it != end );

	return median_it;
}

template< typename TIterator >
void delete_container( TIterator begin, TIterator end )
{
	for ( TIterator it = begin
		; it != end
		; ++it )
	{
		delete (*it);
	}
}

// ----------------------------------------------------------------------------

template< typename TValue >
inline bool isValueInRange(
	const TValue minPos,
	const TValue coord,
	const TValue maxPos,
	const bool includingMax )
{
	if ( includingMax )
		return ( minPos <= coord ) && ( coord <= maxPos );
	else
		return ( minPos <= coord ) && ( coord < maxPos );
}

bool checkSectionLength( const SPoint& begin, const SPoint& end );
bool isShiftOverflow( coord_t coord, int shiftCounter );

} // namespace utils

} // namespace be

#endif
