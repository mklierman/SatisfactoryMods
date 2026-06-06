#pragma once

#include "Hologram/FGResourceExtractorHologram.h"
#include "Hologram/FGWaterPumpHologram.h"
#include "Hologram/FGGeoThermalGeneratorHologram.h"
#include "Hologram/FGRailroadSignalHologram.h"
#include "NudgeableHologram.generated.h"

UCLASS()
class INFINITENUDGE_API ANudgeableResourceExtractorHologram : public AFGResourceExtractorHologram {
	GENERATED_BODY()
public:
	virtual bool CanNudgeHologram() const override { return true; }
};


UCLASS()
class INFINITENUDGE_API ANudgeableWaterPumpHologram : public AFGWaterPumpHologram {
	GENERATED_BODY()
public:
	virtual bool CanNudgeHologram() const override { return true; }
};


UCLASS()
class INFINITENUDGE_API ANudgeableGeoThermalGeneratorHologram : public AFGGeoThermalGeneratorHologram {
	GENERATED_BODY()
public:
	virtual bool CanNudgeHologram() const override { return true; }
};

UCLASS()
class INFINITENUDGE_API ANudgeableRailroadSignalHologram : public AFGRailroadSignalHologram {
	GENERATED_BODY()
public:
	virtual bool CanNudgeHologram() const override { return true; }
};