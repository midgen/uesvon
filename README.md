# uesvon
Sparse Voxel Octree navigation plugin for UnrealEngine

Based on "3D Flight Navigation Using Sparse Voxel Octrees" by Daniel Brewer in Game AI Pro 3.

Status : Alpha

Plugin is mostly working, just some edge cases where pathfinding can go wrong at the moment. Simple instructions :

* Install the plugin
* Create a new AIController from SVONAIController
* Add an SVONVolume to your scene
* Adjust the SVONVolume properties, enable some debug viz and click 'Generate' to check it
* On play, the SVONVolume will generate the octree (so you will get a pause with a large number of layers)
* Use the SVONAIController MoveTo (through BT if you want) to pathfind and follow the 3D path

[![UESVON Demo](http://img.youtube.com/vi/84AFdg0ykwY/0.jpg)](http://www.youtube.com/watch?v=84AFdg0ykwY "Video Title")


