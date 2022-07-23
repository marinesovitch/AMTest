// author: marines marinesovitch alias Darek Slusarczyk 2012-2013, 2022
#include "ph.h"
#include "beInstance.h"
#include "beDocumentImpl.h"
#include "beInternalDocument.h"
#include "beControllerImpl.h"
#include "beController.h"

namespace be
{

SInstance::SInstance()
	: d_document(nullptr )
	, d_controller( nullptr )
{
}

SInstance::~SInstance()
{
	delete d_document;
	delete d_controller;
}

bool SInstance::init( IMapStream* mapStream )
{
	bool result = false;
	if ( mapStream != nullptr )
	{
		std::unique_ptr< IInternalDocument > internalDocument( createDocument( mapStream ) );
		if ( internalDocument )
		{
			d_controller = createController( internalDocument.get() );
			if ( d_controller )
			{
				d_document = internalDocument.release();
				result = true;
			}
		}
	}
	return result;
}

} // namespace be
