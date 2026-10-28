#pragma once

enum class InteractionState
{
	Idle,		// Nothing clicked
	Holding,	// Mouse button down on an object, waiting for drag
	Dragging,	// actively moving selected object
	PanningCam	// panning camera
};