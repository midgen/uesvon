#pragma once


struct UESVON_API SVONNode
{
	FVector myPosition;
	bool myIsBlocked;
	SVONNode() 
	{ 
		myPosition = FVector(0.0f); 
		myIsBlocked = false;
	}
};