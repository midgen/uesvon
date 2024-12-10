#include <UESVON/Public/SVONOctreeData.h>



const SVONNode& FSVONOctreeData::GetNode(const SVONLink& aLink) const
{
	if (aLink.GetLayerIndex() < 14)
	{
		return GetLayer(aLink.GetLayerIndex())[aLink.GetNodeIndex()];
	}
	else
	{
		return GetLayer(NumLayers - 1)[0];
	}
}

const SVONLeafNode& FSVONOctreeData::GetLeafNode(nodeindex_t aIndex) const
{
	return LeafNodes[aIndex];
}

void FSVONOctreeData::GetLeafNeighbours(const SVONLink& aLink, TArray<SVONLink>& oNeighbours) const
{
	mortoncode_t leafIndex = aLink.GetSubnodeIndex();
	const SVONNode& node = GetNode(aLink);
	const SVONLeafNode& leaf = GetLeafNode(node.myFirstChild.GetNodeIndex());

	// Get our starting co-ordinates
	uint_fast32_t x = 0, y = 0, z = 0;
	morton3D_64_decode(leafIndex, x, y, z);

	for (int i = 0; i < 6; i++)
	{
		// Need to switch to signed ints
		int32 sX = x + SVONStatics::dirs[i].X;
		int32 sY = y + SVONStatics::dirs[i].Y;
		int32 sZ = z + SVONStatics::dirs[i].Z;

		// If the neighbour is in bounds of this leaf node
		if (sX >= 0 && sX < 4 && sY >= 0 && sY < 4 && sZ >= 0 && sZ < 4)
		{
			mortoncode_t thisIndex = morton3D_64_encode(sX, sY, sZ);
			// If this node is blocked, then no link in this direction, continue
			if (leaf.GetNode(thisIndex))
			{
				continue;
			}
			else // Otherwise, this is a valid link, add it
			{
				oNeighbours.Emplace(0, aLink.GetNodeIndex(), thisIndex);
				continue;
			}
		}
		else // the neighbours is out of bounds, we need to find our neighbour
		{
			const SVONLink& neighbourLink = node.myNeighbours[i];
			const SVONNode& neighbourNode = GetNode(neighbourLink);

			// If the neighbour layer 0 has no leaf nodes, just return it
			if (!neighbourNode.myFirstChild.IsValid())
			{
				oNeighbours.Add(neighbourLink);
				continue;
			}

			const SVONLeafNode& leafNode = GetLeafNode(neighbourNode.myFirstChild.GetNodeIndex());

			if (leafNode.IsCompletelyBlocked())
			{
				// The leaf node is completely blocked, we don't return it
				continue;
			}
			else // Otherwise, we need to find the correct subnode
			{
				if (sX < 0)
					sX = 3;
				else if (sX > 3)
					sX = 0;
				else if (sY < 0)
					sY = 3;
				else if (sY > 3)
					sY = 0;
				else if (sZ < 0)
					sZ = 3;
				else if (sZ > 3)
					sZ = 0;
				//
				mortoncode_t subNodeCode = morton3D_64_encode(sX, sY, sZ);

				// Only return the neighbour if it isn't blocked!
				if (!leafNode.GetNode(subNodeCode))
				{
					oNeighbours.Emplace(0, neighbourNode.myFirstChild.GetNodeIndex(), subNodeCode);
				}
			}
		}
	}
}

void FSVONOctreeData::GetNeighbours(const SVONLink& aLink, TArray<SVONLink>& oNeighbours) const
{
	const SVONNode& node = GetNode(aLink);

	for (int i = 0; i < 6; i++)
	{
		const SVONLink& neighbourLink = node.myNeighbours[i];

		if (!neighbourLink.IsValid())
			continue;

		const SVONNode& neighbour = GetNode(neighbourLink);

		// If the neighbour has no children, it's empty, we just use it
		if (!neighbour.HasChildren())
		{
			oNeighbours.Add(neighbourLink);
			continue;
		}

		// If the node has children, we need to look down the tree to see which children we want to add to the neighbour set

		// Start working set, and put the link into it
		TArray<SVONLink> workingSet;
		workingSet.Push(neighbourLink);

		while (workingSet.Num() > 0)
		{
			// Pop off the top of the working set
			SVONLink thisLink = workingSet.Pop();
			const SVONNode& thisNode = GetNode(thisLink);

			// If the node as no children, it's clear, so add to neighbours and continue
			if (!thisNode.HasChildren())
			{
				oNeighbours.Add(neighbourLink);
				continue;
			}

			// We know it has children

			if (thisLink.GetLayerIndex() > 0)
			{
				// If it's above layer 0, we will need to potentially add 4 children using our offsets
				for (const nodeindex_t& childIndex : SVONStatics::dirChildOffsets[i])
				{
					// Each of the childnodes
					SVONLink childLink = thisNode.myFirstChild;
					childLink.myNodeIndex += childIndex;
					const SVONNode& childNode = GetNode(childLink);

					if (childNode.HasChildren()) // If it has children, add them to the working set to keep going down
					{
						workingSet.Emplace(childLink);
					}
					else // Or just add to the outgoing links
					{
						oNeighbours.Emplace(childLink);
					}
				}
			}
			else
			{
				// If this is a leaf layer, then we need to add whichever of the 16 facing leaf nodes aren't blocked
				for (const nodeindex_t& leafIndex : SVONStatics::dirLeafChildOffsets[i])
				{
					// Each of the childnodes
					SVONLink link = neighbour.myFirstChild;
					const SVONLeafNode& leafNode = GetLeafNode(link.myNodeIndex);
					link.mySubnodeIndex = leafIndex;

					if (!leafNode.GetNode(leafIndex))
					{
						oNeighbours.Emplace(link);
					}
				}
			}
		}
	}
}