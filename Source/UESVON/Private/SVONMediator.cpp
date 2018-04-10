

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

	int layerIndex = aVolume.GetMyNumLayers() - 1;
	nodeindex_t nodeIndex = 0;
	while (layerIndex >= 0 && layerIndex < aVolume.GetMyNumLayers())
	{
		// Get the layer and voxel size
		float voxelSize = aVolume.GetVoxelSize(layerIndex);
		const TArray<SVONNode>& layer = aVolume.GetLayer(layerIndex);
		// Calculate the XYZ coordinates
		uint_fast32_t x, y, z;
		x = FMath::FloorToInt(localPos.X / voxelSize);
		y = FMath::FloorToInt(localPos.Y / voxelSize);
		z = FMath::FloorToInt(localPos.Z / voxelSize);
		// Get the morton code we want for this layer
		mortoncode_t code = morton3D_64_encode(x, y, z);

		for (nodeindex_t j = nodeIndex; j < layer.Num(); j++)
		{
			// This is the node we are in
			if (layer[j].myCode == code)
			{
				// There are no child nodes, so this is our nav position
				if (!layer[j].myFirstChildIndex.IsValid())
				{
					oLink.myLayerIndex = layerIndex;
					oLink.myNodeIndex = j;
					return true;
				}
				else
				{
					// We need to go down a layer
					layerIndex = layer[j].myFirstChildIndex.GetLayerIndex();
					nodeIndex = layer[j].myFirstChildIndex.GetNodeIndex();

				}
				break; //out of the for loop
			}
		}
	}

	

	
	return false;


}
