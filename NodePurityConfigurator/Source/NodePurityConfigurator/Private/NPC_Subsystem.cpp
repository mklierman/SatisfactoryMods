


#include "NPC_Subsystem.h"


#pragma optimize("", off)

void ANPC_Subsystem::BeginPlay()
{
	FillPurityMaps();
	Super::BeginPlay();
}

bool ANPC_Subsystem::ShouldSave_Implementation() const
{
	return true;
}

void ANPC_Subsystem::FillPurityMaps()
{
	if (IronPurities.Num() == 0)
	{
		IronPurities.Add(EResourcePurity::RP_Inpure, 39);
		IronPurities.Add(EResourcePurity::RP_Normal, 42);
		IronPurities.Add(EResourcePurity::RP_Pure, 46);
	}
	if (CopperPurities.Num() == 0)
	{
		CopperPurities.Add(EResourcePurity::RP_Inpure, 13);
		CopperPurities.Add(EResourcePurity::RP_Normal, 29);
		CopperPurities.Add(EResourcePurity::RP_Pure, 13);
	}
	if (CoalPurities.Num() == 0)
	{
		CoalPurities.Add(EResourcePurity::RP_Inpure, 15);
		CoalPurities.Add(EResourcePurity::RP_Normal, 31);
		CoalPurities.Add(EResourcePurity::RP_Pure, 16);
	}
	if (LimestonePurities.Num() == 0)
	{
		LimestonePurities.Add(EResourcePurity::RP_Inpure, 15);
		LimestonePurities.Add(EResourcePurity::RP_Normal, 50);
		LimestonePurities.Add(EResourcePurity::RP_Pure, 29);
	}
	if (CateriumPurities.Num() == 0)
	{
		CateriumPurities.Add(EResourcePurity::RP_Inpure, 0);
		CateriumPurities.Add(EResourcePurity::RP_Normal, 9);
		CateriumPurities.Add(EResourcePurity::RP_Pure, 8);
	}
	if (BauxitePurities.Num() == 0)
	{
		BauxitePurities.Add(EResourcePurity::RP_Inpure, 5);
		BauxitePurities.Add(EResourcePurity::RP_Normal, 6);
		BauxitePurities.Add(EResourcePurity::RP_Pure, 6);
	}
	if (OilPurities.Num() == 0)
	{
		OilPurities.Add(EResourcePurity::RP_Inpure, 10);
		OilPurities.Add(EResourcePurity::RP_Normal, 12);
		OilPurities.Add(EResourcePurity::RP_Pure, 8);
	}
	if (OilWellPurities.Num() == 0)
	{
		OilWellPurities.Add(EResourcePurity::RP_Inpure, 8);
		OilWellPurities.Add(EResourcePurity::RP_Normal, 6);
		OilWellPurities.Add(EResourcePurity::RP_Pure, 4);
	}
	if (SAMPurities.Num() == 0)
	{
		SAMPurities.Add(EResourcePurity::RP_Inpure, 10);
		SAMPurities.Add(EResourcePurity::RP_Normal, 6);
		SAMPurities.Add(EResourcePurity::RP_Pure, 3);
	}
	if (UraniumPurities.Num() == 0)
	{
		UraniumPurities.Add(EResourcePurity::RP_Inpure, 3);
		UraniumPurities.Add(EResourcePurity::RP_Normal, 2);
		UraniumPurities.Add(EResourcePurity::RP_Pure, 0);
	}
	if (QuartzPurities.Num() == 0)
	{
		QuartzPurities.Add(EResourcePurity::RP_Inpure, 3);
		QuartzPurities.Add(EResourcePurity::RP_Normal, 7);
		QuartzPurities.Add(EResourcePurity::RP_Pure, 7);
	}
	if (SulfurPurities.Num() == 0)
	{
		SulfurPurities.Add(EResourcePurity::RP_Inpure, 6);
		SulfurPurities.Add(EResourcePurity::RP_Normal, 5);
		SulfurPurities.Add(EResourcePurity::RP_Pure, 5);
	}
	if (NitrogenWellPurities.Num() == 0)
	{
		NitrogenWellPurities.Add(EResourcePurity::RP_Inpure, 2);
		NitrogenWellPurities.Add(EResourcePurity::RP_Normal, 7);
		NitrogenWellPurities.Add(EResourcePurity::RP_Pure, 36);
	}
	if (WaterWellPurities.Num() == 0)
	{
		WaterWellPurities.Add(EResourcePurity::RP_Inpure, 7);
		WaterWellPurities.Add(EResourcePurity::RP_Normal, 12);
		WaterWellPurities.Add(EResourcePurity::RP_Pure, 36);
	}
}

#pragma optimize("", on)