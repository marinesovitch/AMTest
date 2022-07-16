// author: marines marinesovitch alias Darek Slusarczyk 2012-2013
#ifndef INC_BE_CONTENTS_GENERATOR_H
#define INC_BE_CONTENTS_GENERATOR_H

#include "beInternalTypes.h"

namespace be
{

struct IInternalDocument;
struct IBitmap;

struct SContentsGeneratorData
{
	SContentsGeneratorData(
		const IInternalDocument& document,
		const SViewData& viewData,
		const SRect& viewportRect,
		const section_ids_t& sectionids,
		IBitmap* bitmap );

	const IInternalDocument& d_document;
	const SViewData& d_viewData;
	const SRect& d_viewportRect;
	const section_ids_t& d_sections;
	IBitmap* d_bitmap;
};

bool generateViewContents( SContentsGeneratorData* generatorData );

} // namespace be

#endif
