// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#include "SVONVolumeDetails.h"
#include "SVONVolume.h"
#include "DetailLayoutBuilder.h"
#include "DetailWidgetRow.h"
#include "DetailCategoryBuilder.h"
#include "Components/BrushComponent.h"

#define LOCTEXT_NAMESPACE "SVONVolumeDetails"

TSharedRef<IDetailCustomization> FSVONVolumeDetails::MakeInstance()
{
	return MakeShareable( new FSVONVolumeDetails);
}

void FSVONVolumeDetails::CustomizeDetails( IDetailLayoutBuilder& DetailBuilder )
{
	TSharedPtr<IPropertyHandle> PrimaryTickProperty = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UActorComponent, PrimaryComponentTick));

	// Defaults only show tick properties
	if (PrimaryTickProperty->IsValidHandle() && DetailBuilder.HasClassDefaultObject())
	{
		IDetailCategoryBuilder& TickCategory = DetailBuilder.EditCategory("ComponentTick");

		TickCategory.AddProperty(PrimaryTickProperty->GetChildHandle(GET_MEMBER_NAME_CHECKED(FTickFunction, bStartWithTickEnabled)));
		TickCategory.AddProperty(PrimaryTickProperty->GetChildHandle(GET_MEMBER_NAME_CHECKED(FTickFunction, TickInterval)));
		TickCategory.AddProperty(PrimaryTickProperty->GetChildHandle(GET_MEMBER_NAME_CHECKED(FTickFunction, bTickEvenWhenPaused)), EPropertyLocation::Advanced);
		TickCategory.AddProperty(PrimaryTickProperty->GetChildHandle(GET_MEMBER_NAME_CHECKED(FTickFunction, bAllowTickOnDedicatedServer)), EPropertyLocation::Advanced);
		TickCategory.AddProperty(PrimaryTickProperty->GetChildHandle(GET_MEMBER_NAME_CHECKED(FTickFunction, TickGroup)), EPropertyLocation::Advanced);
	}

	PrimaryTickProperty->MarkHiddenByCustomization();

	IDetailCategoryBuilder& navigationCategory = DetailBuilder.EditCategory("SVO Navigation");

	TSharedPtr<IPropertyHandle> showVoxelProperty = DetailBuilder.GetProperty("myShowVoxels");
	TSharedPtr<IPropertyHandle> showMortonCodesProperty = DetailBuilder.GetProperty("myShowMortonCodes");
	TSharedPtr<IPropertyHandle> showNeighbourLinksProperty = DetailBuilder.GetProperty("myShowNeighbourLinks");
	TSharedPtr<IPropertyHandle> voxelPowerProperty = DetailBuilder.GetProperty("myVoxelPower");
	TSharedPtr<IPropertyHandle> collisionChannelProperty = DetailBuilder.GetProperty("myCollisionChannel");
	
	showVoxelProperty->SetPropertyDisplayName(NSLOCTEXT("SVO Volume", "Debug Voxels", "Debug Voxels"));
	showMortonCodesProperty->SetPropertyDisplayName(NSLOCTEXT("SVO Volume", "Debug Morton Codes", "Debug Morton Codes"));
	showNeighbourLinksProperty->SetPropertyDisplayName(NSLOCTEXT("SVO Volume", "Debug Links", "Debug Links"));
	voxelPowerProperty->SetPropertyDisplayName(NSLOCTEXT("SVO Volume", "Density Power", "Density Power"));
	voxelPowerProperty->SetInstanceMetaData("UIMin", TEXT("1"));
	voxelPowerProperty->SetInstanceMetaData("UIMax", TEXT("12"));
	collisionChannelProperty->SetPropertyDisplayName(NSLOCTEXT("SVO Volume", "Collision Channel", "Collision Channel"));

	navigationCategory.AddProperty(voxelPowerProperty);
	navigationCategory.AddProperty(collisionChannelProperty);

	const TArray< TWeakObjectPtr<UObject> >& SelectedObjects = DetailBuilder.GetSelectedObjects();

	for (int32 ObjectIndex = 0; ObjectIndex < SelectedObjects.Num(); ++ObjectIndex)
	{
		const TWeakObjectPtr<UObject>& CurrentObject = SelectedObjects[ObjectIndex];
		if (CurrentObject.IsValid())
		{
			ASVONVolume* currentVolume = Cast<ASVONVolume>(CurrentObject.Get());
			if (currentVolume != NULL)
			{
				myVolume = currentVolume;
				break;
			}
		}
	}

	DetailBuilder.EditCategory("SVO Navigation")
		.AddCustomRow(NSLOCTEXT("SVO Volume", "Generate", "Generate"))
		.NameContent()
		[
			SNew(STextBlock)
			.Font(IDetailLayoutBuilder::GetDetailFont())
		.Text(NSLOCTEXT("SVO Volume", "Generate", "Generate"))
		]
	.ValueContent()
		.MaxDesiredWidth(125.f)
		.MinDesiredWidth(125.f)
		[
			SNew(SButton)
			.ContentPadding(2)
		.VAlign(VAlign_Center)
		.HAlign(HAlign_Center)
		.OnClicked(this, &FSVONVolumeDetails::OnUpdateVolume)
		[
			SNew(STextBlock)
			.Font(IDetailLayoutBuilder::GetDetailFont())
		.Text(NSLOCTEXT("SVO Volume", "Generate", "Generate"))
		]
		];

	navigationCategory.AddProperty(showVoxelProperty);
	navigationCategory.AddProperty(showMortonCodesProperty);
	navigationCategory.AddProperty(showNeighbourLinksProperty);
}

FReply FSVONVolumeDetails::OnUpdateVolume()
{
	if (myVolume.IsValid())
	{		
		myVolume->Generate();
	}

	return FReply::Handled();
}

#undef LOCTEXT_NAMESPACE
