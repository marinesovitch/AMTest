// author: marines marinesovitch alias Darek Slusarczyk 2012-2013, 2022
#ifndef INC_BE_INTERNAL_DOCUMENT_H
#define INC_BE_INTERNAL_DOCUMENT_H

#include "beDocument.h"
#include "beInternalTypes.h"

namespace be
{

struct SSection;

struct IInternalDocument : public IDocument
{
	public:
		virtual const SViewData& getViewData() const = 0;
		virtual bool setViewData( const SViewData& viewData ) = 0;

		virtual const road_classes_t& getBaseRoadClasses() const = 0;

		virtual bool selectSections(
			const SRect& viewportRect,
			section_ids_t* sections ) const = 0;

		virtual void getSection(
			const section_id_t sectid,
			SSection* section ) const = 0;

};

} // namespace be

#endif
