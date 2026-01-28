


#include "AutSigns_BPFL.h"

void UAutSigns_BPFL::UpdateSignElements(AFGBuildableWidgetSign* sign, FPrefabSignData& prefabSignData)
{
	sign->UpdateSignElements(prefabSignData);
}

void UAutSigns_BPFL::SetSignText(AFGBuildableWidgetSign* sign, FString key, FString value)
{
	FPrefabSignData signData;

	sign->GetSignPrefabData(signData);

	auto textData = signData.TextElementData;
	for (auto data : signData.TextElementData)
	{
		data.Value = value;
	}
	//if (signData.TextElementData.Contains(key))
	//	if (signData.TextElementData[key].Equals(value)) return;

	//signData.TextElementData[key] = value;
	signData.TextElementData = textData;
	sign->SetPrefabSignData(signData);
}
