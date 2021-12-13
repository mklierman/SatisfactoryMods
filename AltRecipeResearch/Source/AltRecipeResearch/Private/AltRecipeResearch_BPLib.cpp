
#include "AltRecipeResearch_BPLib.h"
#include "FGResearchManager.h"

void UAltRecipeResearch_BPLib::PopulateResearchTrees(UWorld* World)
{
        AFGResearchManager::Get(World)->PopulateResearchTreeList();
}