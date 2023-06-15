#include "LBModularLoadBalancer_ConnectorHologram.h"

// Build step

// Step 1 - placement
// Step 2 - Stretch along axis player is looking in 1m increments until collision with line trace
// Step 2.5 - If collided with snap point don't go past it