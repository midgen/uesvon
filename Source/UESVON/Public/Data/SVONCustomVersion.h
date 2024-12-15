#pragma once

#include <Misc/Guid.h>

// Custom serialization version for assets/classes in the UESVON module
struct UESVON_API FSVONCustomVersion
{
	enum Type
	{
		// Before any version changes were made in the plugin
		BeforeCustomVersionWasAdded = 0,

		// Major refactor of plugin
		V2Refactor = 1,

		// -----<new versions can be added above this line>-------------------------------------------------
		VersionPlusOne,
		LatestVersion = VersionPlusOne - 1
	};

	// The GUID for this custom version number
	const static FGuid GUID;

private:
	FSVONCustomVersion() {}
};
