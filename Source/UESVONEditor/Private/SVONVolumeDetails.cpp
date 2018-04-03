// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#include "SVONVolumeDetails.h"
#include "Layout/Visibility.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Text/STextBlock.h"
#include "EditorStyleSet.h"
#include "Engine/EngineBaseTypes.h"
#include "Components/ActorComponent.h"
#include "DetailLayoutBuilder.h"
#include "DetailWidgetRow.h"
#include "DetailCategoryBuilder.h"
#include "IDetailsView.h"
#include "ObjectEditorUtils.h"
#include "Widgets/SToolTip.h"
#include "IDocumentation.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SButton.h"
#include "SVONVolume.h"

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
	

	navigationCategory.AddProperty(showVoxelProperty);
	navigationCategory.AddProperty(showMortonCodesProperty);
	navigationCategory.AddProperty(showNeighbourLinksProperty);
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

	DetailBuilder.EditCategory("SVO Generation")
		.AddCustomRow(NSLOCTEXT("SVO Volume", "Update", "Update"))
		.NameContent()
		[
			SNew(STextBlock)
			.Font(IDetailLayoutBuilder::GetDetailFont())
		.Text(NSLOCTEXT("SVO Volume", "Update", "Update"))
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
		.Text(NSLOCTEXT("SVO Volume", "Update", "Update"))
		]
		];

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
