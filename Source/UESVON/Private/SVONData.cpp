#include <UESVON/Public/SVONData.h>

#include <UESVON/Public/SVONCollisionQueryInterface.h>
#include <UESVON/Public/SVONDebugDrawInterface.h>
#include <UESVON/Public/SVONGenerationParameters.h>

void FSVONData::SetExtents(const FVector& Origin, const FVector& Extents)
{
	GenerationParameters.Origin = Origin;
	GenerationParameters.Extents = Extents;
}

void FSVONData::SetDebugPosition(const FVector& DebugPosition)
{
	GenerationParameters.DebugPosition = DebugPosition;
}

void FSVONData::ResetForGeneration()
{
	// Clear temp data
	OctreeData.BlockedIndices.Empty();
	// Clear existing Octree data
	OctreeData.Layers.Empty();
	OctreeData.LeafNodes.Empty();
}

void FSVONData::UpdateGenerationParameters(const FSVONGenerationParameters& Params)
{
	GenerationParameters = Params;
	OctreeData.NumLayers = Params.VoxelPower + 1;
}

const FSVONGenerationParameters& FSVONData::GetParams() const
{
	return GenerationParameters;
}

void FSVONData::Generate(UWorld& World, const ISVONCollisionQueryInterface& CollisionInterface, const ISVONDebugDrawInterface& DebugInterface)
{
	FirstPassRasterise(World);

		// Allocate the leaf node data
	OctreeData.LeafNodes.Empty();
	OctreeData.LeafNodes.AddDefaulted(OctreeData.BlockedIndices[0].Num() * 8 * 0.25f);

	// Add layers
	for (int i = 0; i < OctreeData.NumLayers; i++)
	{
		OctreeData.Layers.Emplace();
	}

	// Rasterize layer, bottom up, adding parent/child links
	for (int i = 0; i < OctreeData.NumLayers; i++)
	{
		RasteriseLayer(i, CollisionInterface, DebugInterface);
	}

	// Now traverse down, adding neighbour links
	for (int i = OctreeData.NumLayers - 2; i >= 0; i--)
	{
		BuildNeighbourLinks(i);
	}
}

int32 FSVONData::GetNumNodesInLayer(layerindex_t Layer) const
{
	return FMath::Pow(FMath::Pow(2.f, (GenerationParameters.VoxelPower - (Layer))), 3);
}

int32 FSVONData::GetNumNodesPerSide(layerindex_t Layer) const
{
	return FMath::Pow(2.f, (GenerationParameters.VoxelPower - (Layer)));
}

bool FSVONData::GetLinkPosition(const SVONLink& Link, FVector& Position) const
{
	const SVONNode& Node = OctreeData.GetLayer(Link.GetLayerIndex())[Link.GetNodeIndex()];

	GetNodePosition(Link.GetLayerIndex(), Node.myCode, Position);
	// If this is layer 0, and there are valid children
	if (Link.GetLayerIndex() == 0 && Node.myFirstChild.IsValid())
	{
		float VoxelSize = GetVoxelSize(0);
		uint_fast32_t X, Y, Z;
		morton3D_64_decode(Link.GetSubnodeIndex(), X, Y, Z);
		Position += FVector(X * VoxelSize * 0.25f, Y * VoxelSize * 0.25f, Z * VoxelSize * 0.25f) - FVector(VoxelSize * 0.375);
		const SVONLeafNode& LeafNode = OctreeData.GetLeafNode(Node.myFirstChild.myNodeIndex);
		bool bIsBlocked = LeafNode.GetNode(Link.GetSubnodeIndex());
		return !bIsBlocked;
	}
	return true;
}

bool FSVONData::GetNodePosition(layerindex_t aLayer, mortoncode_t aCode, FVector& oPosition) const
{
	float VoxelSize = GetVoxelSize(aLayer);
	uint_fast32_t X, Y, Z;
	morton3D_64_decode(aCode, X, Y, Z);
	oPosition = GenerationParameters.Origin - GenerationParameters.Extents + FVector(X * VoxelSize, Y * VoxelSize, Z * VoxelSize) + FVector(VoxelSize * 0.5f);
	return true;
}

float FSVONData::GetVoxelSize(layerindex_t Layer) const
{
	return (GenerationParameters.Extents.X / FMath::Pow(2.f, GenerationParameters.VoxelPower)) * (FMath::Pow(2.0f, Layer + 1));
}

bool FSVONData::IsInDebugRange(const FVector& aPosition) const
{
	return FVector::DistSquared(GenerationParameters.DebugPosition, aPosition) < GenerationParameters.DebugDistance * GenerationParameters.DebugDistance;
}

bool FSVONData::IsAnyMemberBlocked(layerindex_t aLayer, mortoncode_t aCode) const
{
	mortoncode_t parentCode = aCode >> 3;

	if (aLayer == OctreeData.BlockedIndices.Num())
	{
		return true;
	}
	// The parent of this code is blocked
	if (OctreeData.BlockedIndices[aLayer].Contains(parentCode))
	{
		return true;
	}

	return false;
}

bool FSVONData::GetIndexForCode(layerindex_t aLayer, mortoncode_t aCode, nodeindex_t& oIndex) const
{
	const TArray<SVONNode>& layer = OctreeData.GetLayer(aLayer);

	for (int i = 0; i < layer.Num(); i++)
	{
		if (layer[i].myCode == aCode)
		{
			oIndex = i;
			return true;
		}
	}

	return false;
}

void FSVONData::BuildNeighbourLinks(layerindex_t aLayer)
{
	TArray<SVONNode>& layer = OctreeData.GetLayer(aLayer);
	layerindex_t searchLayer = aLayer;

	// For each node
	for (nodeindex_t i = 0; i < layer.Num(); i++)
	{
		SVONNode& node = layer[i];
		// Get our world co-ordinate
		uint_fast32_t x, y, z;
		morton3D_64_decode(node.myCode, x, y, z);
		nodeindex_t backtrackIndex = -1;
		nodeindex_t index = i;
		FVector nodePos;
		GetNodePosition(aLayer, node.myCode, nodePos);

		// For each direction
		for (int d = 0; d < 6; d++)
		{
			SVONLink& linkToUpdate = node.myNeighbours[d];

			backtrackIndex = index;

			while (!FindLinkInDirection(searchLayer, index, d, linkToUpdate, nodePos) && aLayer < OctreeData.Layers.Num() - 2)
			{
				SVONLink& parent = OctreeData.GetLayer(searchLayer)[index].myParent;
				if (parent.IsValid())
				{
					index = parent.myNodeIndex;
					searchLayer = parent.myLayerIndex;
				}
				else
				{
					searchLayer++;
					GetIndexForCode(searchLayer, node.myCode >> 3, index);
				}
			}
			index = backtrackIndex;
			searchLayer = aLayer;
		}
	}
}

bool FSVONData::FindLinkInDirection(layerindex_t aLayer, const nodeindex_t aNodeIndex, uint8 aDir, SVONLink& oLinkToUpdate, FVector& aStartPosForDebug)
{
	int32 maxCoord = GetNumNodesPerSide(aLayer);
	SVONNode& node = OctreeData.GetLayer(aLayer)[aNodeIndex];
	TArray<SVONNode>& layer = OctreeData.GetLayer(aLayer);

	// Get our world co-ordinate
	uint_fast32_t x = 0, y = 0, z = 0;
	morton3D_64_decode(node.myCode, x, y, z);
	int32 sX = x, sY = y, sZ = z;
	// Add the direction
	sX += SVONStatics::dirs[aDir].X;
	sY += SVONStatics::dirs[aDir].Y;
	sZ += SVONStatics::dirs[aDir].Z;

	// If the coords are out of bounds, the link is invalid.
	if (sX < 0 || sX >= maxCoord || sY < 0 || sY >= maxCoord || sZ < 0 || sZ >= maxCoord)
	{
		oLinkToUpdate.SetInvalid();
		if (GenerationParameters.ShowNeighbourLinks && IsInDebugRange(aStartPosForDebug))
		{
			FVector startPos, endPos;
			GetNodePosition(aLayer, node.myCode, startPos);
			endPos = startPos + (FVector(SVONStatics::dirs[aDir]) * 100.f);
			//DrawDebugLine(GetWorld(), aStartPosForDebug, endPos, FColor::Red, true, -1.f, 0, .0f);
		}
		return true;
	}
	x = sX;
	y = sY;
	z = sZ;
	// Get the morton code for the direction
	mortoncode_t thisCode = morton3D_64_encode(x, y, z);
	bool isHigher = thisCode > node.myCode;
	int32 nodeDelta = (isHigher ? 1 : -1);

	while ((aNodeIndex + nodeDelta) < layer.Num() && aNodeIndex + nodeDelta >= 0)
	{
		// This is the node we're looking for
		if (layer[aNodeIndex + nodeDelta].myCode == thisCode)
		{
			const SVONNode& thisNode = layer[aNodeIndex + nodeDelta];
			// This is a leaf node
			if (aLayer == 0 && thisNode.HasChildren())
			{
				// Set invalid link if the leaf node is completely blocked, no point linking to it
				if (OctreeData.GetLeafNode(thisNode.myFirstChild.GetNodeIndex()).IsCompletelyBlocked())
				{
					oLinkToUpdate.SetInvalid();
					return true;
				}
			}
			// Otherwise, use this link
			oLinkToUpdate.myLayerIndex = aLayer;
			check(aNodeIndex + nodeDelta < layer.Num());
			oLinkToUpdate.myNodeIndex = aNodeIndex + nodeDelta;
			if (GenerationParameters.ShowNeighbourLinks && IsInDebugRange(aStartPosForDebug))
			{
				FVector endPos;
				GetNodePosition(aLayer, thisCode, endPos);
				//DrawDebugLine(GetWorld(), aStartPosForDebug, endPos, SVONStatics::myLinkColors[aLayer], true, -1.f, 0, .0f);
			}
			return true;
		}
		// If we've passed the code we're looking for, it's not on this layer
		else if ((isHigher && layer[aNodeIndex + nodeDelta].myCode > thisCode) || (!isHigher && layer[aNodeIndex + nodeDelta].myCode < thisCode))
		{
			return false;
		}

		nodeDelta += (isHigher ? 1 : -1);
	}

	// I'm not entirely sure if it's valid to reach the end? Hmmm...
	return false;
}

void FSVONData::RasterizeLeafNode(FVector& aOrigin, nodeindex_t aLeafIndex, const ISVONCollisionQueryInterface& CollisionInterface, const ISVONDebugDrawInterface& DebugInterface)
{
	for (int i = 0; i < 64; i++)
	{

		uint_fast32_t x, y, z;
		morton3D_64_decode(i, x, y, z);
		float leafVoxelSize = GetVoxelSize(0) * 0.25f;
		FVector position = aOrigin + FVector(x * leafVoxelSize, y * leafVoxelSize, z * leafVoxelSize) + FVector(leafVoxelSize * 0.5f);

		if (aLeafIndex >= OctreeData.LeafNodes.Num() - 1)
			OctreeData.LeafNodes.AddDefaulted(1);

		if (CollisionInterface.IsBlocked(position, leafVoxelSize * 0.5f, GenerationParameters.CollisionChannel, GenerationParameters.Clearance))
		{
			OctreeData.LeafNodes[aLeafIndex].SetNode(i);

			if (GenerationParameters.ShowLeafVoxels && IsInDebugRange(position))
			{
				DebugInterface.SVONDrawDebugBox(position, leafVoxelSize * 0.5f, FColor::Red);
				//DrawDebugBox(GetWorld(), position, FVector(leafVoxelSize * 0.5f), FQuat::Identity, FColor::Red, true, -1.f, 0, .0f);
			}
			if (GenerationParameters.ShowMortonCodes && IsInDebugRange(position))
			{
				DebugInterface.SVONDrawDebugString(position, FString::FromInt(aLeafIndex) + ":" + FString::FromInt(i), FColor::Red);
				//DrawDebugString(GetWorld(), position, FString::FromInt(aLeafIndex) + ":" + FString::FromInt(i), nullptr, FColor::Red, -1, false);
			}
		}
	}
}

void FSVONData::RasteriseLayer(layerindex_t aLayer, const ISVONCollisionQueryInterface& CollisionInterface, const ISVONDebugDrawInterface& DebugInterface)
{
	nodeindex_t leafIndex = 0;
	// Layer 0 Leaf nodes are special
	if (aLayer == 0)
	{
		// Run through all our coordinates
		int32 numNodes = GetNumNodesInLayer(aLayer);
		for (int32 i = 0; i < numNodes; i++)
		{
			int index = (i);

			// If we know this node needs to be added, from the low res first pass
			if (OctreeData.BlockedIndices[0].Contains(i >> 3))
			{
				// Add a node
				index = OctreeData.GetLayer(aLayer).Emplace();
				SVONNode& node = OctreeData.GetLayer(aLayer)[index];

				// Set my code and position
				node.myCode = (i);

				FVector nodePos;
				GetNodePosition(aLayer, node.myCode, nodePos);

				// Debug stuff
				if (GenerationParameters.ShowMortonCodes && IsInDebugRange(nodePos))
				{
					DebugInterface.SVONDrawDebugString(nodePos, FString::FromInt(aLayer) + ":" + FString::FromInt(index), SVONStatics::myLayerColors[aLayer]);
					//DrawDebugString(GetWorld(), nodePos, FString::FromInt(aLayer) + ":" + FString::FromInt(index), nullptr, SVONStatics::myLayerColors[aLayer], -1, false);
				}
				if (GenerationParameters.ShowVoxels && IsInDebugRange(nodePos))
				{
					
					DebugInterface.SVONDrawDebugBox(nodePos, GetVoxelSize(aLayer) * 0.5f, SVONStatics::myLayerColors[aLayer]);
					//DrawDebugBox(GetWorld(), nodePos, FVector(GetVoxelSize(aLayer) * 0.5f), FQuat::Identity, SVONStatics::myLayerColors[aLayer], true, -1.f, 0, .0f);
				}

				// Now check if we have any blocking, and search leaf nodes
				FVector Position;
				GetNodePosition(0, i, Position);

				FCollisionQueryParams params;
				params.bFindInitialOverlaps = true;
				params.bTraceComplex = false;
				params.TraceTag = "SVONRasterize";
				if (CollisionInterface.IsBlocked(Position, GetVoxelSize(0) * 0.5f, GenerationParameters.CollisionChannel, GenerationParameters.Clearance))
				//if (IsBlocked(Position, GetVoxelSize(0) * 0.5f))
				{
					// Rasterize my leaf nodes
					FVector leafOrigin = nodePos - (FVector(GetVoxelSize(aLayer) * 0.5f));
					RasterizeLeafNode(leafOrigin, leafIndex, CollisionInterface, DebugInterface);
					node.myFirstChild.SetLayerIndex(0);
					node.myFirstChild.SetNodeIndex(leafIndex);
					node.myFirstChild.SetSubnodeIndex(0);
					leafIndex++;
				}
				else
				{
					OctreeData.LeafNodes.AddDefaulted(1);
					leafIndex++;
					node.myFirstChild.SetInvalid();
				}
			}
		}
	}
	// Deal with the other layers
	else if (OctreeData.GetLayer(aLayer - 1).Num() > 1)
	{
		int nodeCounter = 0;
		int32 numNodes = GetNumNodesInLayer(aLayer);
		for (int32 i = 0; i < numNodes; i++)
		{
			// Do we have any blocking children, or siblings?
			// Remember we must have 8 children per parent
			if (IsAnyMemberBlocked(aLayer, i))
			{
				// Add a node
				int32 index = OctreeData.GetLayer(aLayer).Emplace();
				nodeCounter++;
				SVONNode& node = OctreeData.GetLayer(aLayer)[index];
				// Set details
				node.myCode = i;
				nodeindex_t childIndex = 0;
				if (GetIndexForCode(aLayer - 1, node.myCode << 3, childIndex))
				{
					// Set parent->child links
					node.myFirstChild.SetLayerIndex(aLayer - 1);
					node.myFirstChild.SetNodeIndex(childIndex);
					// Set child->parent links, this can probably be done smarter, as we're duplicating work here
					for (int iter = 0; iter < 8; iter++)
					{
						OctreeData.GetLayer(node.myFirstChild.GetLayerIndex())[node.myFirstChild.GetNodeIndex() + iter].myParent.SetLayerIndex(aLayer);
						OctreeData.GetLayer(node.myFirstChild.GetLayerIndex())[node.myFirstChild.GetNodeIndex() + iter].myParent.SetNodeIndex(index);
					}

					if (GenerationParameters.ShowParentChildLinks) // Debug all the things
					{
						FVector startPos, endPos;
						GetNodePosition(aLayer, node.myCode, startPos);
						GetNodePosition(aLayer - 1, node.myCode << 3, endPos);
						if (IsInDebugRange(startPos))
						{
							// TODO: DEBUG
							// DrawDebugDirectionalArrow(GetWorld(), startPos, endPos, 0.f, SVONStatics::myLinkColors[aLayer], true);
						}
					}
				}
				else
				{
					node.myFirstChild.SetInvalid();
				}

				if (GenerationParameters.ShowMortonCodes || GenerationParameters.ShowVoxels)
				{
					FVector nodePos;
					GetNodePosition(aLayer, i, nodePos);

					// Debug stuff
					if (GenerationParameters.ShowVoxels && IsInDebugRange(nodePos))
					{
						DebugInterface.SVONDrawDebugBox(nodePos, GetVoxelSize(aLayer) * 0.5f, SVONStatics::myLayerColors[aLayer]);
						//DrawDebugBox(GetWorld(), nodePos, FVector(GetVoxelSize(aLayer) * 0.5f), FQuat::Identity, SVONStatics::myLayerColors[aLayer], true, -1.f, 0, .0f);
					}
					if (GenerationParameters.ShowMortonCodes && IsInDebugRange(nodePos))
					{
						DebugInterface.SVONDrawDebugString(nodePos, FString::FromInt(aLayer) + ":" + FString::FromInt(index), SVONStatics::myLayerColors[aLayer]);
						//DrawDebugString(GetWorld(), nodePos, FString::FromInt(aLayer) + ":" + FString::FromInt(index), nullptr, SVONStatics::myLayerColors[aLayer], -1, false);
					}
				}
			}
		}
	}
}

void FSVONData::FirstPassRasterise(UWorld& World)
{
	// Add the first layer of blocking
	OctreeData.BlockedIndices.Emplace();

	int32 NumNodes = GetNumNodesInLayer(1);
	for (int32 i = 0; i < NumNodes; i++)
	{
		FVector Position;
		GetNodePosition(1, i, Position);
		FCollisionQueryParams Params;
		Params.bFindInitialOverlaps = true;
		Params.bTraceComplex = false;
		Params.TraceTag = "SVONFirstPassRasterize";
		if (World.OverlapBlockingTestByChannel(Position, FQuat::Identity, GenerationParameters.CollisionChannel, FCollisionShape::MakeBox(FVector(GetVoxelSize(1) * 0.5f)), Params))
		{
			OctreeData.BlockedIndices[0].Add(i);
		}
	}

	int LayerIndex = 0;

	while (OctreeData.BlockedIndices[LayerIndex].Num() > 1)
	{
		// Add a new layer to structure
		OctreeData.BlockedIndices.Emplace();
		// Add any parent morton codes to the new layer
		for (mortoncode_t& code : OctreeData.BlockedIndices[LayerIndex])
		{
			OctreeData.BlockedIndices[LayerIndex + 1].Add(code >> 3);
		}
		LayerIndex++;
	}
}
