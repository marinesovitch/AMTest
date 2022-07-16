// author: marines marinesovitch alias Darek Slusarczyk 2012-2013
#ifndef INC_BE_DOCUMENT_H
#define INC_BE_DOCUMENT_H

#include "beTypes.h"

namespace be
{

struct IDocument
{
	public:
		virtual ~IDocument() = default;

	public:
		virtual std::string getState() const = 0;
		virtual void setState( const std::string& stateString ) = 0;

		virtual color32_t getBkColor() const = 0;
};

} // namespace be

#endif
