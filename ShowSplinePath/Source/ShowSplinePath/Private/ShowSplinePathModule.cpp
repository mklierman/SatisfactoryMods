#include "ShowSplinePathModule.h"
#include "SSP_RemoteCallObject.h"
#include "FGPlayerController.h"

//#pragma optimize("", off)
void FShowSplinePathModule::StartupModule()
{
#if !WITH_EDITOR
#endif
}


//#pragma optimize("", on)

IMPLEMENT_GAME_MODULE(FShowSplinePathModule, ShowSplinePath);
