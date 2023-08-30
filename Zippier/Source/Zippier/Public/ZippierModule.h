#pragma once

#include "Modules/ModuleManager.h"
#include "FGCharacterMovementComponent.h"

class FZippierModule : public FDefaultGameModuleImpl {
public:
	virtual void StartupModule() override;

	virtual bool IsGameModule() const override { return true; }

	void SetZiplineStuff(UFGCharacterMovementComponent* movementComp);
};