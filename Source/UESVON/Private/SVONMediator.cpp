

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

	FBox box = aVolume.GetComponentsBoundingBox(true);

	FVector origin;
	FVector extent;

	box.GetCenterAndExtents(origin, extent);
	// The z-order origin of the volume (where code == 0)
	FVector zOrigin = origin - extent;
	// The local position of the point in volume space
	FVector localPos = aPosition - zOrigin;

	int layerIndex = aVolume.GetMyNumLayers() - 1;
	nodeindex_t nodeIndex = 0;
	while (layerIndex >= 0 && layerIndex < aVolume.GetMyNumLayers())
	{
		// Get the layer and voxel size
		//float voxelSize = aVolume.GetVoxelSize(layerIndex);
		const TArray<SVONNode>& layer = aVolume.GetLayer(layerIndex);
		// Calculate the XYZ coordinates

		FIntVector voxel;
		GetVolumeXYZ(localPos, aVolume, layerIndex, voxel);
		uint_fast32_t x, y, z;
		x = voxel.X;//FMath::FloorToInt((localPos.X / voxelSize));// +(voxelSize * 0.5f));
		y = voxel.Y;//FMath::FloorToInt((localPos.Y / voxelSize));// +(voxelSize * 0.5f));
		z = voxel.Z;//FMath::FloorToInt((localPos.Z / voxelSize));// +(voxelSize * 0.5f));

		//oPosition = myOrigin - myExtent + FVector(x * voxelSize, y * voxelSize, z * voxelSize) + FVector(voxelSize * 0.5f);
		// Get the morton code we want for this layer
		mortoncode_t code = morton3D_64_encode(x, y, z);

		for (nodeindex_t j = nodeIndex; j < layer.Num(); j++)
		{
			// This is the node we are in
			if (layer[j].myCode == code)
			{
				// There are no child nodes, so this is our nav position
				if (!layer[j].myFirstChild.IsValid())
				{
					oLink.myLayerIndex = layerIndex;
					oLink.myNodeIndex = j;
					return true;
				}
				if (layer[j].myFirstChild.IsLeafNode())
				{
					oLink.myLayerIndex = 15;
					oLink.myNodeIndex = j;
					return true;
				}
				
				// We need to go down a layer
				layerIndex = layer[j].myFirstChild.GetLayerIndex();
				nodeIndex = layer[j].myFirstChild.GetNodeIndex();

				
				break; //out of the for loop
			}
		}
	}

	

	
	return false;


}

void SVONMediator::GetVolumeXYZ(const FVector& aPosition, const ASVONVolume& aVolume, const int aLayer, FIntVector& oXYZ)
{
	FBox box = aVolume.GetComponentsBoundingBox(true);

	FVector origin;
	FVector extent;

	box.GetCenterAndExtents(origin, extent);
	// The z-order origin of the volume (where code == 0)
	FVector zOrigin = origin - extent;
	// The local position of the point in volume space
	FVector localPos = aPosition - zOrigin;

	int layerIndex = aLayer;

	// Get the layer and voxel size
	float voxelSize = aVolume.GetVoxelSize(layerIndex + 1);
	
	// Calculate the XYZ coordinates

	//uint_fast32_t x, y, z;
	oXYZ.X = FMath::FloorToInt((localPos.X / voxelSize));// +(voxelSize * 0.5f));
	oXYZ.Y = FMath::FloorToInt((localPos.Y / voxelSize));// +(voxelSize * 0.5f));
	oXYZ.Z = FMath::FloorToInt((localPos.Z / voxelSize));// +(voxelSize * 0.5f));

}
