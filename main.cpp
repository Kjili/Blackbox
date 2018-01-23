#include <irrlicht.h>
using namespace irr;

int main() {
	// start up the engine
	IrrlichtDevice *device = createDevice(video::EDT_OPENGL, core::dimension2d<u32>(640,480));
	if (device == 0) {
		return 1;
	}
	device->setWindowCaption(L"Blackbox");

	video::IVideoDriver* driver = device->getVideoDriver();
	scene::ISceneManager* smgr = device->getSceneManager();

	// load the cube and add it to the scene
	scene::IMesh* cube = smgr->getMesh("../models/cube.obj");
	if (!cube) {
		device->drop();
		return 1;
	}
	scene::ISceneNode* node = smgr->addMeshSceneNode(cube);

	if (node) {
		// add a texture to the cube and disable lighting as there is no light
		node->setMaterialFlag(video::EMF_LIGHTING, false);
		node->setMaterialFlag(video::EMF_BILINEAR_FILTER, false);
		node->setMaterialTexture(0, driver->getTexture("../models/cube.png"));
		// correct Blender rotation for Irrlicht (not really necessary for a cube, just for reference)
		node->setRotation(core::vector3df(0,0,180));
	}

	// add a static camera that views the cube
	smgr->addCameraSceneNode(0, core::vector3df(0,10,0), core::vector3df(0,0,0));

	// draw the scene
	while(device->run() && driver) {
		driver->beginScene(true, true, video::SColor(255,150,150,255));
		smgr->drawAll();
		driver->endScene();
	}

	// delete the device
	device->drop();
	return 0;
}