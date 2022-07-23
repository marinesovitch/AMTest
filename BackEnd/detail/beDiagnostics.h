// author: marines marinesovitch alias Darek Slusarczyk 2012-2013, 2022
#ifndef INC_BE_DIAGNOSTICS_H
#define INC_BE_DIAGNOSTICS_H

#include "beInternalTypes.h"

namespace be
{

struct IInternalDocument;

namespace diag
{

void dumpRawSegments( const raw_segments_t& segments );

void dumpPoints( const raw_segments_t& segments );

void dumpRect( const SRect& rect );

void dumpView( const SViewData& viewData );

void dumpSections(
	const IInternalDocument* document,
	const section_ids_t& sections );

} // namespace diag

} // namespace be

#endif
