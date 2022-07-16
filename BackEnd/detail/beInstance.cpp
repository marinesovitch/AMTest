// author: marines marinesovitch alias Darek Slusarczyk 2012-2013
#include "ph.h"
#include "beInstance.h"
#include "beDocumentImpl.h"
#include "beInternalDocument.h"
#include "beControllerImpl.h"
#include "beController.h"

namespace be
{

SInstance::SInstance()
	: d_document( 0 )
	, d_controller( 0 )
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
	if ( mapStream != 0 )
	{
		std::unique_ptr< IInternalDocument > internalDocument( createDocument( mapStream ) );
		if ( internalDocument.get() != 0 )
		{
			d_controller = createController( internalDocument.get() );
			if ( d_controller != 0 )
			{
				d_document = internalDocument.release();
				result = true;
			}
		}
	}
	return result;
}

} // namespace be
