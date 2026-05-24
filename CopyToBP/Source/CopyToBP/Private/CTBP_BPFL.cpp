


#include "CTBP_BPFL.h"
#include "Configuration/ConfigManager.h"
#include "Configuration/ConfigProperty.h"
#include "Configuration/Properties/ConfigPropertyBool.h"
#include "Logging/StructuredLog.h"

//DEFINE_LOG_CATEGORY(CTBP_Log);

#pragma optimize("", off)
void UCTBP_BPFL::GetAllSubclassesOf(TSubclassOf<UObject> ParentClass, TArray<UClass*>& OutClassess)
{
    if (!ParentClass) return;

    GetDerivedClasses(ParentClass, OutClassess, true);
}

void UCTBP_BPFL::SetShouldPasteInConfig(bool shouldPaste, UObject* WorldContextObject)
{
    UE_LOGFMT(CTBP_Log, Display, "shouldPaste = {0}", shouldPaste);
    UE_LOGFMT(CTBP_Log, Display, "SetShouldPasteInConfig");
    UConfigManager* ConfigManager = WorldContextObject->GetWorld()->GetGameInstance()->GetSubsystem<UConfigManager>();
    if (ConfigManager)
    {
        UE_LOGFMT(CTBP_Log, Display, "ConfigManager");
        auto root = ConfigManager->GetConfigurationRootSection({ "CopyToBP", "" });
        if (root)
        {
            UE_LOGFMT(CTBP_Log, Display, "root");
            auto sp = CastChecked<UConfigPropertyBool>(root->SectionProperties.FindChecked("ShouldPaste"));
            if (sp)
            {
                UE_LOGFMT(CTBP_Log, Display, "sp->Value = {0}", sp->Value);
                sp->Value = shouldPaste;
                UE_LOGFMT(CTBP_Log, Display, "Value set");
                //sp->MarkDirty();
                root->MarkDirty();
                UE_LOGFMT(CTBP_Log, Display, "MarkDirty");
                //ConfigManager->FlushPendingSaves();
            }
        }
    }
}

void UCTBP_BPFL::SetConfigValue(bool newValue, UConfigProperty* property)
{
    auto boolProp = Cast<UConfigPropertyBool>(property);
    boolProp->Value = newValue;
}

#pragma optimize("", on)