// author: marines marinesovitch alias Darek Slusarczyk 2012-2013, 2022
#ifndef INC_BE_DOCUMENT_IMPL_H
#define INC_BE_DOCUMENT_IMPL_H

namespace be
{

struct IMapStream;
struct IInternalDocument;

IInternalDocument* createDocument( IMapStream* mapStream );

} // namespace be

#endif
