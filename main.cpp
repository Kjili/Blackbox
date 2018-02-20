#include <irrlicht.h>
#include <vector>
#include <algorithm>
#include <iostream>
#include <cstdlib>
#include <ctime>

using namespace irr;

// based on example 19 of irrlicht docs
class MyEventReceiver : public IEventReceiver {
public:
	// record info on the mouse state
	struct SMouseState {
		core::position2di pos;
		bool leftButtonDown;
		bool rightButtonDown;
		SMouseState(): leftButtonDown(false), rightButtonDown(false) {}
	} mouseState;

	// track mouse movements and clicks
	virtual bool OnEvent(const SEvent& event) {
		if (event.EventType == EET_MOUSE_INPUT_EVENT) {
			switch(event.MouseInput.Event) {
			case EMIE_LMOUSE_PRESSED_DOWN:
				mouseState.leftButtonDown = true;
				break;

			case EMIE_LMOUSE_LEFT_UP:
				mouseState.leftButtonDown = false;
				break;

			case EMIE_RMOUSE_PRESSED_DOWN:
				mouseState.rightButtonDown = true;
				break;

			case EMIE_RMOUSE_LEFT_UP:
				mouseState.rightButtonDown = false;
				break;

			case EMIE_MOUSE_MOVED:
				mouseState.pos.X = event.MouseInput.X;
				mouseState.pos.Y = event.MouseInput.Y;
				break;

			default:
				break;
			}
		}
		return false;
	}

	MyEventReceiver() {}
};

scene::ISceneNode* createCube(video::IVideoDriver* driver, scene::ISceneManager* smgr, scene::IMesh* cube, core::vector3df pos, video::SColor color, int id=-1) {
	scene::ISceneNode* node = smgr->addMeshSceneNode(cube);
	if (node) {
		// add a texture to the cube and disable lighting as there is no light
		node->setMaterialFlag(video::EMF_LIGHTING, true);
		node->setMaterialFlag(video::EMF_BILINEAR_FILTER, false);
		//node->setMaterialTexture(0, driver->getTexture("../models/cube.png"));
		node->getMaterial(0).AmbientColor = color;
		// correct Blender rotation for Irrlicht (not really necessary for a cube, just for reference)
		node->setRotation(core::vector3df(0,0,180));
		node->setPosition(pos);
		node->setID(id);
	}
	return node;
}

int main() {
	//initialize random
	std::srand(std::time(nullptr));

	// start up the engine
	MyEventReceiver receiver;
	IrrlichtDevice *device = createDevice(video::EDT_OPENGL, core::dimension2d<u32>(640,480), 16, false, false, false, &receiver);
	if (device == 0) {
		return 1;
	}
	// set title
	device->setWindowCaption(L"Blackbox");
	//device->setResizable(true);

	// add video driver and scene manager
	video::IVideoDriver* driver = device->getVideoDriver();
	scene::ISceneManager* smgr = device->getSceneManager();

	// add light
	smgr->setAmbientLight(video::SColorf(0.5,0.5,0.5,1));
	scene::ILightSceneNode* light1 = smgr->addLightSceneNode(0, core::vector3df(0,-100,0), video::SColorf(0.3,0.3,0.3), 1.0f, 1);

	// load the cube
	scene::IMesh* cube = smgr->getMesh("../models/cube.obj");
	if (!cube) {
		device->drop();
		return 1;
	}

	// add cubes to the scene to form the gameboard
	const int gameBoardSize = 8;
	const int gameBoardTopLeftOffset = -(3*gameBoardSize)/2;
	video::SColor cubeColor = video::SColor(0,16,156,255);
	video::SColor raycubeColor = video::SColor(0,156,5,255);

	std::vector<std::vector<scene::ISceneNode*>> cubes;
	std::vector<scene::ISceneNode*> leftRaycubes;
	std::vector<scene::ISceneNode*> rightRaycubes;
	std::vector<scene::ISceneNode*> topRaycubes;
	std::vector<scene::ISceneNode*> bottomRaycubes;
	for (int y = 0; y < gameBoardSize; ++y) {
		cubes.push_back(std::vector<scene::ISceneNode*>());
		for (int x = 0; x < gameBoardSize; ++x) {

			core::vector3df cubePosition = core::vector3df(gameBoardTopLeftOffset + 3*x, 0, gameBoardTopLeftOffset + 3*y);
			cubes[y].push_back(createCube(driver, smgr, cube, cubePosition, cubeColor, y*gameBoardSize+x));

			if (x == 0) {
				core::vector3df raycubePosition = cubes[y][x]->getPosition() + core::vector3df(-5,0,0);
				leftRaycubes.push_back(createCube(driver, smgr, cube, raycubePosition, raycubeColor));
			}
			if (y == 0) {
				core::vector3df raycubePosition = cubes[y][x]->getPosition() + core::vector3df(0,0,-5);
				topRaycubes.push_back(createCube(driver, smgr, cube, raycubePosition, raycubeColor));
			}
			if (x == gameBoardSize-1) {
				core::vector3df raycubePosition = cubes[y][x]->getPosition() + core::vector3df(5,0,0);
				rightRaycubes.push_back(createCube(driver, smgr, cube, raycubePosition, raycubeColor));
			}
			if (y == gameBoardSize-1) {
				core::vector3df raycubePosition = cubes[y][x]->getPosition() + core::vector3df(0,0,5);
				bottomRaycubes.push_back(createCube(driver, smgr, cube, raycubePosition, raycubeColor));
			}
		}
	}
	std::vector<std::vector<scene::ISceneNode*>> raycubes = {leftRaycubes, rightRaycubes, topRaycubes, bottomRaycubes};

	// add a static camera that views the gameboard
	smgr->addCameraSceneNode(0, core::vector3df(0,-30,0), core::vector3df(0,0,0));
	//device->getCursorControl()->setVisible(true);

	// add collision manager
	scene::ISceneCollisionManager* collmgr = smgr->getSceneCollisionManager();

	// set atoms
	const int maxAtoms = 5;
	std::vector<int> atoms;
	while (atoms.size() < maxAtoms) {
		int newPos = std::rand() % (gameBoardSize*gameBoardSize);
		if (std::find(atoms.begin(), atoms.end(), newPos) == atoms.end()) {
			atoms.push_back(newPos);
			std::cout << "atom at: " << newPos << " x: " << static_cast<int>(newPos/gameBoardSize) << " , y: " << newPos%gameBoardSize << std::endl;
		}
	}

	// draw the scene
	int atomsSet = 0;
	while(device->run() && driver) {
		if (device->isWindowActive()) {
			driver->beginScene(true, true, video::SColor(255,150,150,255));

			// check for a mouse click
			core::position2d<s32> position;
			if (receiver.mouseState.leftButtonDown || receiver.mouseState.rightButtonDown) {
				position = receiver.mouseState.pos;

				// check collision
				scene::ISceneNode * selectedSceneNode = collmgr->getSceneNodeFromScreenCoordinatesBB(position);

				// if there is a node below the position color the node
				if (selectedSceneNode) {
					// if a raycube is clicked, check at which position it is in the array
					int raycubeHit = -1;
					std::vector<scene::ISceneNode*>::iterator it;
					int index;
					for (int i = 0; i < raycubes.size(); ++i) {
						it = std::find(raycubes[i].begin(), raycubes[i].end(), selectedSceneNode);
						if (it != raycubes[i].end()) {
							raycubeHit = i;
							index = distance(raycubes[i].begin(), it);
							break;
						}
					}

					// react on mouse clicks depending on the cube type hit
					if (raycubeHit > -1) {
						if (receiver.mouseState.leftButtonDown) {
							selectedSceneNode->getMaterial(0).AmbientColor.set(0,255,0,255);
						}
					} else {
						if (selectedSceneNode->getID() > -1) {
							if (selectedSceneNode->getMaterial(0).AmbientColor != video::SColor(255,255,0,255) && receiver.mouseState.leftButtonDown && atomsSet < maxAtoms) {
								selectedSceneNode->getMaterial(0).AmbientColor.set(255,255,0,255);
								++atomsSet;
							} else if (selectedSceneNode->getMaterial(0).AmbientColor != cubeColor && receiver.mouseState.rightButtonDown) {
								selectedSceneNode->getMaterial(0).AmbientColor = cubeColor;
								--atomsSet;
							}
						}
					}
					//std::cout << "cube id: " << selectedSceneNode->getID() << std::endl;
					std::cout << "atoms set: " << atomsSet << std::endl;
				}
			}

			smgr->drawAll();
			driver->endScene();
		}
	}

	// delete the device
	device->drop();
	return 0;
}
