#include "HPPR_Subsystem.h"

void AHPPR_Subsystem::Mk1Updated()
{
	auto config = FHPPR_ConfigStruct::GetActiveConfig(GetWorld());
	mMk1Range = config.Mk1;
}

void AHPPR_Subsystem::Mk2Updated()
{
	auto config = FHPPR_ConfigStruct::GetActiveConfig(GetWorld());
	mMk2Range = config.Mk2;
}

void AHPPR_Subsystem::Mk3Updated()
{
	auto config = FHPPR_ConfigStruct::GetActiveConfig(GetWorld());
	mMk3Range = config.Mk3;
}

void AHPPR_Subsystem::RailsUpdated()
{
	auto config = FHPPR_ConfigStruct::GetActiveConfig(GetWorld());
	mRailRange = config.Rails;
}

void AHPPR_Subsystem::EverythingElseUpdated()
{
	auto config = FHPPR_ConfigStruct::GetActiveConfig(GetWorld());
	mElseRange = config.EverythingElse;
}

void AHPPR_Subsystem::SetConfigValues()
{
	auto config = FHPPR_ConfigStruct::GetActiveConfig(GetWorld());
	mMk1Range = config.Mk1;
	mMk2Range = config.Mk2;
	mMk3Range = config.Mk3;
	mRailRange = config.Rails;
	mElseRange = config.EverythingElse;
}
