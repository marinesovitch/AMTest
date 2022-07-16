// author: marines marinesovitch alias Darek Slusarczyk 2012-2013
#include "ph.h"
#include "beTreeUtils.h"

namespace be
{

namespace utils
{

bool cmp_less_by_x( 
	const SPoint& lhs, 
	const SPoint& rhs,
	const bool resultIfEqual )
{
	bool result = false;

	const int lx = lhs.x;
	const int rx = rhs.x;
	if ( lx < rx ) 
	{
		result = true;
	}
	else if ( lx == rx ) 
	{
		const int ly = lhs.y;
		const int ry = rhs.y;
		if ( ly < ry )
		{
			result = true;
		}
		else if ( ly == ry )
		{
			result = resultIfEqual;
		}
	}

	return result;
}

bool cmp_less_by_y( 
	const SPoint& lhs, 
	const SPoint& rhs,
	const bool resultIfEqual )
{
	bool result = false;

	const int ly = lhs.y;
	const int ry = rhs.y;
	if ( ly < ry ) 
	{
		result = true;
	}
	else if ( ly == ry ) 
	{
		const int lx = lhs.x;
		const int rx = rhs.x;
		if ( lx < rx )
		{
			result = true;
		}
		else if ( lx == rx )
		{
			result = resultIfEqual;
		}
	}

	return result;
}

} // namespace utils

} // namespace be
