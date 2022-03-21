#pragma once

#include "Blueprint/UserWidget.h"
#include <Runtime/UMG/Public/Components/Widget.h>
#include "Modules/ModuleManager.h"
#include "CTB_Subsystem.h"

//DECLARE_DYNAMIC_DELEGATE_OneParam(FOnWidgetConstruct, UUserWidget*, Widget);

DECLARE_LOG_CATEGORY_EXTERN(CTB_Log, Display, All);

class FCloudToButtModule : public FDefaultGameModuleImpl {
public:
	virtual void StartupModule() override;

	virtual bool IsGameModule() const override { return true; }

	//static FOnWidgetConstruct OnWidgetConstruct;

};