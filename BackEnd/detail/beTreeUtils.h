// author: marines marinesovitch alias Darek Slusarczyk 2012-2013
#ifndef INC_BE_TREE_UTILS_H
#define INC_BE_TREE_UTILS_H

#include "beInternalTypes.h"
#include "beViewportArea.h"

namespace be
{

namespace utils
{

bool cmp_less_by_x( 
	const SPoint& lhs, 
	const SPoint& rhs,
	const bool resultIfEqual );

bool cmp_less_by_y( 
	const SPoint& lhs, 
	const SPoint& rhs,
	const bool resultIfEqual );

// ----------------------------------------------------------------------------

template< typename position_t >
bool less_by_x( 
	const position_t lhs, 
	const position_t rhs )
{
	const SPoint& lpt = lhs->d_point;
	const SPoint& rpt = rhs->d_point;
	const bool resultIfEqual = lhs < rhs;
	const bool result = cmp_less_by_x( lpt, rpt, resultIfEqual );
	return result;
}

template< typename position_t >
bool less_by_x(
	const SViewportBorder& edge,
	const position_t rhs )
{
	const SPoint& lpt = edge.d_border;
	const SPoint& rpt = rhs->d_point;
	const bool resultIfEqual = edge.isMin();
	const bool result = cmp_less_by_x( lpt, rpt, resultIfEqual );
	return result;
}

template< typename position_t >
bool less_by_x( 
	const position_t lhs, 
	const SViewportBorder& edge )
{
	const SPoint& lpt = lhs->d_point;
	const SPoint& rpt = edge.d_border;
	const bool resultIfEqual = edge.isMax();
	const bool result = cmp_less_by_x( lpt, rpt, resultIfEqual );
	return result;
}

// ----------------------------------------------------------------------------

template< typename position_t >
bool less_by_y( 
	const position_t lhs, 
	const position_t rhs )
{
	const SPoint& lpt = lhs->d_point;
	const SPoint& rpt = rhs->d_point;
	const bool resultIfEqual = lhs < rhs;
	const bool result = cmp_less_by_y( lpt, rpt, resultIfEqual );
	return result;
}

template< typename position_t >
bool less_by_y(
	const SViewportBorder& edge,
	const position_t rhs )
{
	const SPoint& lpt = edge.d_border;
	const SPoint& rpt = rhs->d_point;
	const bool resultIfEqual = edge.isMin();
	const bool result = cmp_less_by_y( lpt, rpt, resultIfEqual );
	return result;
}

template< typename position_t >
bool less_by_y( 
	const position_t lhs, 
	const SViewportBorder& edge )
{
	const SPoint& lpt = lhs->d_point;
	const SPoint& rpt = edge.d_border;
	const bool resultIfEqual = edge.isMax();
	const bool result = cmp_less_by_y( lpt, rpt, resultIfEqual );
	return result;
}

// ----------------------------------------------------------------------------

struct compare_by_x
{
	template< typename position_t >
	bool operator()( const position_t lhs, const position_t rhs ) const
	{
		const bool result = less_by_x( lhs, rhs );
		return result;
	}

	template< typename position_t >
	bool operator()( const SViewportBorder& edge, const position_t position ) const
	{
		const bool result = less_by_x( edge, position );
		return result;
	}

	template< typename position_t >
	bool operator()( const position_t position, const SViewportBorder& edge ) const
	{
		const bool result = less_by_x( position, edge );
		return result;
	}

};

// ----------------------------------------------------------------------------

struct compare_by_y
{
	template< typename position_t >
	bool operator()( const position_t lhs, const position_t rhs ) const
	{
		const bool result = less_by_y( lhs, rhs );
		return result;
	}

	template< typename position_t >
	bool operator()( const SViewportBorder& edge, const position_t position ) const
	{
		const bool result = less_by_y( edge, position );
		return result;
	}

	template< typename position_t >
	bool operator()( const position_t position, const SViewportBorder& edge ) const
	{
		const bool result = less_by_y( position, edge );
		return result;
	}

};

} // namespace utils

} // namespace be

#endif
