// ILikeBanas

#include "HologramLocation_BPLib.h"
#include <iostream>
#include <string>
#include <algorithm>
#include <regex>
#include "Misc/CoreDelegates.h"

FString UHologramLocation_BPLib::RegexReplace(FString inputString, FString regexCode, FString replacementText = "")
{
    std::string originalString = std::string(TCHAR_TO_UTF8(*inputString));
    std::string replacementString = std::string(TCHAR_TO_UTF8(*replacementText));
    std::regex regexp(std::string(TCHAR_TO_UTF8(*regexCode)));
    return std::regex_replace(originalString, regexp, replacementString).c_str();

}

FString UHologramLocation_BPLib::AddCommasToInt(int inputInt)
{
    int number = inputInt;
    auto src = std::to_string(number);
    auto dest = std::string();

    auto count = 3;
    for (auto i = src.crbegin(); i != src.crend(); ++i) {
        if (count == 0)
        {
            dest.push_back(',');
            count = 3;
        }
        if (count--) {
            dest.push_back(*i);
        }
    }
    std::reverse(dest.begin(), dest.end());

    return dest.c_str();
}

FVector UHologramLocation_BPLib::GetConnectionComponentLocation(UFGFactoryConnectionComponent* connection)
{
    return connection->GetConnectorLocation();
}

FVector UHologramLocation_BPLib::GetConnectionComponentRotation(UFGFactoryConnectionComponent* connection)
{
    return connection->GetConnectorNormal();
}

FTransform UHologramLocation_BPLib::GetLiftHologramTransform(AFGConveyorLiftHologram* clHologram)
{
    return clHologram->GetTopTransform();
}

float UHologramLocation_BPLib::GetLiftHeight(AFGConveyorLiftHologram* clHologram)
{
    return clHologram->GetHeight();
}

float UHologramLocation_BPLib::GetLiftBuildableHeight(AFGBuildableConveyorLift* cl)
{
    return cl->GetHeight();
}

FString UHologramLocation_BPLib::FormatIntAsNumberString(int32 number, bool usePeriod)
{
    int32 num = number;
    bool numIsNegative = false;
    if (num < 0)
    {
        numIsNegative = true;
        num = abs(num);
    }
    FString formattedString;
    formattedString = formattedString.FormatAsNumber(num);
    if (usePeriod)
    {
        formattedString = formattedString.Replace(TEXT(","), TEXT("."),ESearchCase::IgnoreCase);
    }
    if (numIsNegative)
    {
        formattedString.InsertAt(0, "-");
    }
    return formattedString;
}

FString UHologramLocation_BPLib::FormatFloatAsNumberString(float number, bool usePeriod)
{

    float num = number;
    bool numIsNegative = false;
    if (num < 0.0)
    {
        numIsNegative = true;
        num = abs(num);
    }
    FString formattedString;
    float fractional = modf(number, nullptr);
    formattedString = formattedString.FormatAsNumber(num);
    if (usePeriod)
    {
        formattedString = formattedString.Replace(TEXT(","), TEXT("."), ESearchCase::IgnoreCase);
    }
    if (numIsNegative)
    {
        formattedString.InsertAt(0, "-");
    }

    return formattedString;
}

float UHologramLocation_BPLib::GetBeamLength(AFGBuildableBeam* beam)
{
    return beam->GetLength();
}

float UHologramLocation_BPLib::GetElevatorHeight(AFGBuildableElevator* elevator)
{
    return elevator->GetElevatorHeight();
}
