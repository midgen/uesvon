# uesvon
Sparse Voxel Octree navigation plugin for UnrealEngine

Based on "3D Flight Navigation Using Sparse Voxel Octrees" by Daniel Brewer in Game AI Pro 3.

[Game AI Pro](https://www.gameaipro.com)

and his related GDC talk :

[GDC Vault - Getting Off The Navmesh](https://www.gdcvault.com/play/1022016/Getting-off-the-NavMesh-Navigating)

Status : Alpha. OCtree generation and pathfinding implemented as per the articles above. Not yet battle-tested, but a good basis for anyone to take further. Contributors welcome.

Engine Version : 4.21 (See branches for previous versions)

New - *Baking navigation data*

In the SVOVolume properties, there is a new Generation Strategy option.
* Generate OnBeginPlay - Rebuilds the navigation data when you start the game (not recommended)
* Use Baked - Generate navigation data in the editor, save, and load it when you start the game (recommended!)

Plugin is mostly working, just some edge cases where pathfinding can go wrong at the moment. Simple instructions :

* Install the plugin
* Add an SVONVolume to your scene
* Choose the GenerationStrategy (UseBaked is preferred).
* Adjust the SVONVolume properties, enable some debug viz and click 'Generate' to check it
* Save the volume to serialize the nav data

Then EITHER

* Create a new AIController from SVONAIController and use the regular MoveTo method or BT Task
OR
* Add an SVONavigationComponent to your standard AI Controller.
* Use the SVO Move To BT Task Node

[![UESVON Demo](http://img.youtube.com/vi/84AFdg0ykwY/0.jpg)](http://www.youtube.com/watch?v=84AFdg0ykwY "Video Title")


