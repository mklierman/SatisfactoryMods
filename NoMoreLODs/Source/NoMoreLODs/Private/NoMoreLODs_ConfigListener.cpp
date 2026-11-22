#include "NoMoreLODs_ConfigListener.h"

#include "Configuration/ConfigProperty.h"
#include "NoMoreLODs.h"

void UNoMoreLODsConfigListener::Bind(FNoMoreLODsModule* newModule, UConfigProperty* newProperty)
{
	if (module == newModule && property == newProperty)
	{
		return;
	}

	Unbind();
	module = newModule;
	property = newProperty;
	if (property != nullptr)
	{
		property->OnPropertyValueChanged.AddDynamic(
			this, &UNoMoreLODsConfigListener::HandlePropertyValueChanged);
	}
}

void UNoMoreLODsConfigListener::Unbind()
{
	if (property != nullptr)
	{
		property->OnPropertyValueChanged.RemoveDynamic(
			this, &UNoMoreLODsConfigListener::HandlePropertyValueChanged);
	}
	property = nullptr;
	module = nullptr;
}

void UNoMoreLODsConfigListener::HandlePropertyValueChanged()
{
	if (module != nullptr)
	{
		module->HandleSMLConfigurationChanged(this);
	}
}
