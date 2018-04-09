

#include "SVONMediator.h"
#include "CoreMinimal.h"
#include "SVONVolume.h"
#include "SVONLink.h"

bool SVONMediator::GetLinkFromPosition(const FVector& aPosition, const ASVONVolume& aVolume, SVONLink& oLink)
{
	// Position is outside the volume, no can do
	if (!aVolume.EncompassesPoint(aPosition))
	{
		return false;
	}

	const FVector& origin = aVolume.GetOrigin();
	const FVector& extent = aVolume.GetExtent();
	// The z-order origin of the volume (where code == 0)
	FVector zOrigin = origin - extent;
	// The local position of the point in volume space
	FVector localPos = zOrigin - aPosition;

	int i = 0;
	while (i >= 0 && i < aVolume.GetMyNumLayers())
	{
		// Get the layer and voxel size
		float voxelSize = aVolume.GetVoxelSize(i);
		const TArray<SVONNode>& layer = aVolume.GetLayer(i);
		// Calculate the XYZ coordinates
		uint_fast32_t x, y, z;
		x = FMath::FloorToInt(localPos.X / voxelSize);
		y = FMath::FloorToInt(localPos.Y / voxelSize);
		z = FMath::FloorToInt(localPos.Z / voxelSize);
		// Get the morton code we want for this layer
		mortoncode code = morton3D_64_encode(x, y, z);

		for (nodeindex j = 0; j < layer.Num(); j++)
		{
			// This is the node we are in
			if (layer[j].myCode == code)
			{
				// There are no child nodes, so this is our nav position
				if (layer[j].myFirstChildIndex == -1)
				{
					oLink.myLayerIndex = i;
					oLink.myNodeIndex = j;
					return true;
				}
				else
				{
					//i = 
				}
				break; //out of the for loop
			}
		}
	}

	

	
	return false;


}
