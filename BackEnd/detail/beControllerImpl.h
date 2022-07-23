// author: marines marinesovitch alias Darek Slusarczyk 2012-2013, 2022
#ifndef INC_BE_CONTROLLER_IMPL_H
#define INC_BE_CONTROLLER_IMPL_H

namespace be
{

struct IInternalDocument;
struct IController;

IController* createController( IInternalDocument* document );

} // namespace be

#endif
