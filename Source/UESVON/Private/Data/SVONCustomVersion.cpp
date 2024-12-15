#pragma once

#include<UESVON/Public/Data/SVONCustomVersion.h>
#include <Serialization/CustomVersion.h>

const FGuid FSVONCustomVersion::GUID(0x2625B0F1, 0xBA46495F, 0xA0F4C769, 0xEC2F38C1);

FCustomVersionRegistration GUESVONAnimationCustomVersion(FSVONCustomVersion::GUID, FSVONCustomVersion::LatestVersion, TEXT("UESVONVer"));
