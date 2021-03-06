//  An Irrlicht implementation of the Blackbox board game.
//
//  Copyright (C) 2018  Annemarie Mattmann
//
//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program.  If not, see <https://www.gnu.org/licenses/>.

#include <irrlicht.h>
#include <vector>
#include <algorithm>
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <sstream>
#include <string>
#include <random>

using namespace irr;

enum {
	GUI_ID_RESET_BUTTON = 0,
	GUI_ID_EVALUATE_BUTTON,
	GUI_ID_HELP_BUTTON,
	GUI_ID_MINUS_BUTTON,
	GUI_ID_PLUS_BUTTON
};

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
	struct SAppContext {
		IrrlichtDevice *device;
		bool reset;
		bool eval;
		bool help;
		bool decreaseAtoms;
		bool increaseAtoms;
		SAppContext(): reset(false), eval(false), help(false), decreaseAtoms(false), increaseAtoms(false) {}
	} context;

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
		// track gui clicks
		if (event.EventType == EET_GUI_EVENT) {
			s32 id = event.GUIEvent.Caller->getID();
			gui::IGUIEnvironment* guienv = context.device->getGUIEnvironment();

			switch(event.GUIEvent.EventType) {
				case gui::EGET_BUTTON_CLICKED:
					switch(id) {
						case GUI_ID_RESET_BUTTON:
							context.reset = true;
							break;
						case GUI_ID_EVALUATE_BUTTON:
							context.eval = true;
							break;
						case GUI_ID_HELP_BUTTON:
							context.help = !context.help;
							break;
						case GUI_ID_MINUS_BUTTON:
							context.decreaseAtoms = true;
							break;
						case GUI_ID_PLUS_BUTTON:
							context.increaseAtoms = true;
							break;
						default:
							break;
					}
				default:
					break;
			}
		}
		return false;
	}
};

scene::ISceneNode* createNode(video::IVideoDriver* driver, scene::ISceneManager* smgr, scene::IMesh* mesh, core::vector3df pos, video::SColor color, int id=-1) {
	scene::ISceneNode* node = smgr->addMeshSceneNode(mesh);
	if (node) {
		// add a texture to the cube and disable lighting as there is no light
		node->setMaterialFlag(video::EMF_LIGHTING, true);
		node->setMaterialFlag(video::EMF_BILINEAR_FILTER, false);
		//node->setMaterialTexture(0, driver->getTexture("../models/cube.png"));
		node->getMaterial(0).Shininess = 20.0f;
		node->getMaterial(0).AmbientColor = color;
		// correct Blender rotation for Irrlicht (not really necessary for a cube, just for reference)
		node->setRotation(core::vector3df(0,0,180));
		node->setPosition(pos);
		node->setID(id);
	}
	return node;
}

void buildGUI(gui::IGUIEnvironment* guienv, int screenX) {
	guienv->addButton(core::rect<s32>(10,10,200,50), 0, GUI_ID_EVALUATE_BUTTON, L"Evaluate", L"Show Results");
	guienv->addButton(core::rect<s32>(screenX-10-190,10,screenX-10,50), 0, GUI_ID_RESET_BUTTON, L"Reset", L"Reset Game");
	guienv->addButton(core::rect<s32>(220,10,260,50), 0, GUI_ID_HELP_BUTTON, L"?");
	guienv->addButton(core::rect<s32>(screenX-10-190-50,10,screenX-10-190-10,50), 0, GUI_ID_MINUS_BUTTON, L"-", L"Reduce the Number of Atoms (After Next Reset)");
	guienv->addButton(core::rect<s32>(screenX-10-190-50-40,10,screenX-10-190-50,50), 0, GUI_ID_PLUS_BUTTON, L"+", L"Increase the Number of Atoms (After Next Reset)");
}

// based on https://en.wikipedia.org/wiki/Web_colors
std::vector<video::SColor> colors {
	video::SColor(255, 128, 0, 128),	// Purple
	video::SColor(255, 128, 0, 0),		// Maroon
	video::SColor(255, 128, 128, 0),	// Olive
	video::SColor(255, 0, 128, 128),	// Teal
	video::SColor(255, 250, 128, 114),	// Salmon
	video::SColor(255, 220, 20, 60),	// Crimson
	video::SColor(255, 139, 0, 0),		// DarkRed
	video::SColor(255, 255, 69, 0),		// OrangeRed
	video::SColor(255, 255, 165, 0),	// Orange
	video::SColor(255, 255, 215, 0),	// Gold
	video::SColor(255, 0, 0, 255),		// Blue
	video::SColor(255, 188, 143, 143),	// RosyBrown
	video::SColor(255, 189, 183, 107),	// DarkKhaki
	video::SColor(255, 0, 100, 0),		// DarkGreen
	video::SColor(255, 210, 105, 30),	// Chocolate
	video::SColor(255, 139, 69, 19),	// SaddleBrown
	video::SColor(255, 85, 107, 47),	// DarkOliveGreen
	video::SColor(255, 173, 255, 47),	// GreenYellow
	video::SColor(255, 50, 205, 50),	// LimeGreen
	video::SColor(255, 124, 252, 0),	// LawnGreen
	video::SColor(255, 0, 255, 127),	// SpringGreen
	video::SColor(255, 47, 79, 79),		// DarkSlateGray
	video::SColor(255, 102, 205, 170),	// MediumAquamarine
	video::SColor(255, 127, 255, 212),	// Aquamarine
	video::SColor(255, 0, 206, 209),	// DarkTurquoise
	video::SColor(255, 176, 224, 230),	// PowderBlue
	video::SColor(255, 0, 191, 255),	// DeepSkyBlue
	video::SColor(255, 100, 149, 237),	// CornflowerBlue
	video::SColor(255, 238, 130, 238),	// Violet
	video::SColor(255, 138, 43, 226),	// BlueViolet
	video::SColor(255, 75, 0, 130),		// Indigo
	video::SColor(255, 255, 192, 203),	// Pink
	video::SColor(255, 255, 20, 147),	// DeepPink
	video::SColor(255, 199, 21, 133)	// MediumVioletRed
};

int main() {
	// initialize random
	std::srand(std::time(nullptr));

	// start up the engine
	MyEventReceiver receiver;
	IrrlichtDevice *device = createDevice(video::EDT_OPENGL, core::dimension2d<u32>(1024,768), 16, false, false, false, &receiver);
	if (device == 0) {
		return 1;
	}
	// configure device
	device->setWindowCaption(L"Blackbox");
	//device->setResizable(true);

	// add video driver, scene manager and gui
	video::IVideoDriver* driver = device->getVideoDriver();
	scene::ISceneManager* smgr = device->getSceneManager();
	gui::IGUIEnvironment* guienv = device->getGUIEnvironment();

	// build and configure gui
	int screenX = driver->getScreenSize().Width;
	video::SColor textcolor(255,255,255,255);
	gui::IGUIFont* font = guienv->getFont("../fonts/bigfont.png");
	if (!font) {
		std::cout << "font not found" << std::endl;
		font = guienv->getBuiltInFont();
	}
	guienv->getSkin()->setFont(font);
	guienv->getSkin()->setColor(gui::EGUI_DEFAULT_COLOR::EGDC_BUTTON_TEXT, textcolor);
	guienv->getSkin()->setColor(gui::EGUI_DEFAULT_COLOR::EGDC_TOOLTIP, textcolor);
	buildGUI(guienv, screenX);
	receiver.context.device = device;

	// load example image
	video::ITexture* example = driver->getTexture("../images/exampleFullhelp.png");

	// add light
	smgr->setAmbientLight(video::SColorf(1,1,1));

	// load the cube
	scene::IMesh* cube = smgr->getMesh("../models/cube.obj");
	if (!cube) {
		device->drop();
		return 1;
	}

	// load the atom
	scene::IMesh* atom = smgr->getMesh("../models/atom.obj");
	if (!atom) {
		device->drop();
		return 1;
	}

	// init constant variables
	const int gameBoardSize = 8;
	const int gameBoardTopLeftOffset = -(3*gameBoardSize)/2;
	const video::SColor cubeColor = video::SColor(255,0,0,128);//255,0,16,156);
	const video::SColor raycubeColor = video::SColor(255,0,0,0);//video::SColor(255,0,128,0);//255,0,156,5);
	const video::SColor atomColor = video::SColor(255,255,255,0);
	const video::SColor reflectedCube = video::SColor(255,255,255,255);
	// init atom number
	int maxAtoms = 5;

	// add cubes to the scene to form the gameboard
	std::vector<std::vector<scene::ISceneNode*>> cubes;
	std::vector<std::vector<scene::ISceneNode*>> atoms;
	std::vector<scene::ISceneNode*> leftRaycubes;
	std::vector<scene::ISceneNode*> rightRaycubes;
	std::vector<scene::ISceneNode*> topRaycubes;
	std::vector<scene::ISceneNode*> bottomRaycubes;
	for (int y = 0; y < gameBoardSize; ++y) {
		cubes.push_back(std::vector<scene::ISceneNode*>());
		atoms.push_back(std::vector<scene::ISceneNode*>());
		for (int x = 0; x < gameBoardSize; ++x) {
			float cubeScale = cube->getBoundingBox().getExtent().X/2;
			core::vector3df cubePosition = core::vector3df(gameBoardTopLeftOffset + 3*x + cubeScale, 0, gameBoardTopLeftOffset + 3*y + cubeScale + 0.5);
			cubes[y].push_back(createNode(driver, smgr, cube, cubePosition, cubeColor, y*gameBoardSize+x));
			atoms[y].push_back(createNode(driver, smgr, atom, cubePosition + core::vector3df(0,-1,0), atomColor, y*gameBoardSize+x));
			atoms[y][x]->setVisible(false);

			if (x == 0) {
				core::vector3df raycubePosition = cubes[y][x]->getPosition() + core::vector3df(-5,0,0);
				bottomRaycubes.push_back(createNode(driver, smgr, cube, raycubePosition, raycubeColor));
			}
			if (y == 0) {
				core::vector3df raycubePosition = cubes[y][x]->getPosition() + core::vector3df(0,0,-5);
				leftRaycubes.push_back(createNode(driver, smgr, cube, raycubePosition, raycubeColor));
			}
			if (x == gameBoardSize-1) {
				core::vector3df raycubePosition = cubes[y][x]->getPosition() + core::vector3df(5,0,0);
				topRaycubes.push_back(createNode(driver, smgr, cube, raycubePosition, raycubeColor));
			}
			if (y == gameBoardSize-1) {
				core::vector3df raycubePosition = cubes[y][x]->getPosition() + core::vector3df(0,0,5);
				rightRaycubes.push_back(createNode(driver, smgr, cube, raycubePosition, raycubeColor));
			}
		}
	}
	std::vector<std::vector<scene::ISceneNode*>> raycubes = {leftRaycubes, rightRaycubes, bottomRaycubes, topRaycubes};

	// add a static camera that views the gameboard
	smgr->addCameraSceneNode(0, core::vector3df(0,-30,0), core::vector3df(0,0,0));
	//device->getCursorControl()->setVisible(true);

	// add collision manager
	scene::ISceneCollisionManager* collmgr = smgr->getSceneCollisionManager();

	// get random positions for atoms (defines their placement)
	std::vector<int> atomPositions;
	while (atomPositions.size() < maxAtoms) {
		int newPos = std::rand() % (gameBoardSize*gameBoardSize);
		if (std::find(atomPositions.begin(), atomPositions.end(), newPos) == atomPositions.end()) {
			atomPositions.push_back(newPos);
			//cubes[static_cast<int>(newPos/gameBoardSize)][newPos%gameBoardSize]->getMaterial(0).AmbientColor = video::SColor(255,255,255,255);
			//std::cout << "atom at: " << newPos << " x: " << static_cast<int>(newPos/gameBoardSize) << " , y: " << newPos%gameBoardSize << std::endl;
		}
	}

	// shuffle colors
	std::shuffle(colors.begin(), colors.end(), std::default_random_engine{});

	// init remaining required variables
	int atomsSet = 0;
	int penalty = 0;
	std::vector<video::SColor> raycolors(colors);
	bool feedback, atomsChanged = false;
	int nextMaxAtoms = maxAtoms;

	// run
	while(device->run() && driver) {
		if (device->isWindowActive()) {
			driver->beginScene(true, true, video::SColor(255,150,150,255));

			// check for resized window
			if (driver->getScreenSize().Width != screenX) {
				screenX = driver->getScreenSize().Width;
				guienv->clear();
				buildGUI(guienv, screenX);
			}

			// check for more or less atoms wanted
			if (receiver.context.decreaseAtoms) {
				if (nextMaxAtoms > 3) {
					--nextMaxAtoms;
					atomsChanged = true;
					receiver.context.decreaseAtoms = false;
				}
			}
			if (receiver.context.increaseAtoms) {
				if (nextMaxAtoms < gameBoardSize*2) {
					++nextMaxAtoms;
					atomsChanged = true;
					receiver.context.increaseAtoms = false;
				}
			}

			// check for reset
			if (receiver.context.reset) {
				atomsSet = 0;
				penalty = 0;
				raycolors = colors;
				maxAtoms = nextMaxAtoms;
				for (int y = 0; y < gameBoardSize; ++y) {
					for (int x = 0; x < gameBoardSize; ++x) {
						cubes[y][x]->getMaterial(0).AmbientColor = cubeColor;
						atoms[y][x]->setVisible(false);
					}
				}
				for (auto & vec : raycubes) {
					for (auto & cube : vec) {
						cube->getMaterial(0).AmbientColor = raycubeColor;
					}
				}
				atomPositions.clear();
				while (atomPositions.size() < maxAtoms) {
					int newPos = std::rand() % (gameBoardSize*gameBoardSize);
					if (std::find(atomPositions.begin(), atomPositions.end(), newPos) == atomPositions.end()) {
						atomPositions.push_back(newPos);
						//cubes[static_cast<int>(newPos/gameBoardSize)][newPos%gameBoardSize]->getMaterial(0).AmbientColor = video::SColor(255,255,255,255);
						//std::cout << "atom at: " << newPos << " x: " << static_cast<int>(newPos/gameBoardSize) << " , y: " << newPos%gameBoardSize << std::endl;
					}
				}
				atomsChanged = false;
				feedback = false;
				receiver.context.reset = false;
				continue;
			}

			// check eval
			if (receiver.context.eval) {
				for (auto & pos : atomPositions) {
					if (!atoms[static_cast<int>(pos/gameBoardSize)][pos%gameBoardSize]->isVisible()) {
						penalty += 5;
						cubes[static_cast<int>(pos/gameBoardSize)][pos%gameBoardSize]->getMaterial(0).AmbientColor = video::SColor(255,255,0,0);
					} else {
						cubes[static_cast<int>(pos/gameBoardSize)][pos%gameBoardSize]->getMaterial(0).AmbientColor = video::SColor(255,0,255,0);
					}
				}
				feedback = true;
				receiver.context.eval = false;
			}

			// check for a mouse click (not gui)
			core::position2d<s32> position;
			if (receiver.mouseState.leftButtonDown || receiver.mouseState.rightButtonDown) {
				position = receiver.mouseState.pos;

				// check collision of the mouse click with a scene node
				scene::ISceneNode * selectedSceneNode = collmgr->getSceneNodeFromScreenCoordinatesBB(position);

				// react on mouse clicks depending on the node type clicked
				if (selectedSceneNode) {
					// if a raycube is clicked, check at which position it is in the vector
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

					// if a raycube is selected, run game logic
					if (raycubeHit > -1 && selectedSceneNode->getMaterial(0).AmbientColor == raycubeColor) {
						// each raycube clicked costs a point
						++penalty;
						// init variables dependent on the raycube clicked
						if (receiver.mouseState.leftButtonDown) {
							//std::cout << "array containing clicked raycube: " << raycubeHit << " index: " << index << std::endl;
							bool horizontal;
							int incrementor;
							int x = 0;
							int y = 0;
							if (raycubeHit == 0) { // leftRaycubes
								horizontal = true;
								incrementor = 1;
								y = index;
							}
							if (raycubeHit == 1) { // rightRaycubes
								horizontal = true;
								incrementor = -1;
								x = gameBoardSize-1;
								y = index;
							}
							if (raycubeHit == 2) { // bottomRaycubes
								horizontal = false;
								incrementor = 1;
								x = index;
							}
							if (raycubeHit == 3) { // topRaycubes
								horizontal = false;
								incrementor = -1;
								y = gameBoardSize-1;
								x = index;
							}

							// if atom left or right of straight path: if step==0: lighten current (reflect)
							if ((horizontal && y < gameBoardSize-1
							&& std::find(atomPositions.begin(), atomPositions.end(), cubes[x][y+1]->getID()) != atomPositions.end())
							|| (horizontal && y > 0
							&& std::find(atomPositions.begin(), atomPositions.end(), cubes[x][y-1]->getID()) != atomPositions.end())
							|| (!horizontal && x < gameBoardSize-1
							&& std::find(atomPositions.begin(), atomPositions.end(), cubes[x+1][y]->getID()) != atomPositions.end())
							|| (!horizontal && x > 0
							&& std::find(atomPositions.begin(), atomPositions.end(), cubes[x-1][y]->getID()) != atomPositions.end())) {
								//std::cout << "atom next to ray step 0" << std::endl;
								raycubes[raycubeHit][index]->getMaterial(0).AmbientColor = reflectedCube;
							} else {
								// shoot ray
								while (true) {
									// if raycube reached: color raycube
									if (x >= gameBoardSize || x < 0 || y >= gameBoardSize || y < 0) {
										//std::cout << "border reached at x: " << x << " y: " << y << " index: " << index << " raycubeHit: " << raycubeHit << std::endl;
										raycubes[raycubeHit][index]->getMaterial(0).AmbientColor = raycolors.back();
										if (x < 0) {
											if (raycubeHit == 0 && index == y) { // if reflected back to the clicked raycube
												raycubes[raycubeHit][index]->getMaterial(0).AmbientColor = reflectedCube;
												break;
											}
											raycubes[0][y]->getMaterial(0).AmbientColor = raycolors.back();
										}
										if (x >= gameBoardSize) {
											if (raycubeHit == 1 && index == y) { // if reflected back to the clicked raycube
												raycubes[raycubeHit][index]->getMaterial(0).AmbientColor = reflectedCube;
												break;
											}
											raycubes[1][y]->getMaterial(0).AmbientColor = raycolors.back();
										}
										if (y < 0) {
											if (raycubeHit == 2 && index == x) { // if reflected back to the clicked raycube
												raycubes[raycubeHit][index]->getMaterial(0).AmbientColor = reflectedCube;
												break;
											}
											raycubes[2][x]->getMaterial(0).AmbientColor = raycolors.back();
										}
										if (y >= gameBoardSize) {
											if (raycubeHit == 3 && index == x) { // if reflected back to the clicked raycube
												raycubes[raycubeHit][index]->getMaterial(0).AmbientColor = reflectedCube;
												break;
											}
											raycubes[3][x]->getMaterial(0).AmbientColor = raycolors.back();
										}
										raycolors.pop_back();
										break;
									}
									// if atom in straight path: color current (hit)
									if (std::find(atomPositions.begin(), atomPositions.end(), cubes[x][y]->getID()) != atomPositions.end()) {
										//std::cout << "atom hit" << std::endl;
										raycubes[raycubeHit][index]->getMaterial(0).AmbientColor = raycolors.back();
										raycolors.pop_back();
										break;
									}
									// if atom left or right of straight path: if step!=0: change path away from atom (deflect)
									if (horizontal && y < gameBoardSize-1
									&& std::find(atomPositions.begin(), atomPositions.end(), cubes[x][y+1]->getID()) != atomPositions.end()) {
										if (horizontal && y > 0
										&& std::find(atomPositions.begin(), atomPositions.end(), cubes[x][y-1]->getID()) != atomPositions.end()) {
											//std::cout << "double deflection on horizontal path" << std::endl;
											x -= incrementor;
											incrementor *= -1;
											continue;
										}
										horizontal = !horizontal;
										x -= incrementor;
										incrementor = -1;
									}
									if (horizontal && y > 0
									&& std::find(atomPositions.begin(), atomPositions.end(), cubes[x][y-1]->getID()) != atomPositions.end()) {
										if (horizontal && y < gameBoardSize-1
										&& std::find(atomPositions.begin(), atomPositions.end(), cubes[x][y+1]->getID()) != atomPositions.end()) {
											//std::cout << "double deflection on horizontal path" << std::endl;
											x -= incrementor;
											incrementor *= -1;
											continue;
										}
										//std::cout << "deflected on horizontal path" << std::endl;
										horizontal = !horizontal;
										x -= incrementor;
										incrementor = 1;
									}
									if (!horizontal && x < gameBoardSize-1
									&& std::find(atomPositions.begin(), atomPositions.end(), cubes[x+1][y]->getID()) != atomPositions.end()) {
										if (!horizontal && x > 0
										&& std::find(atomPositions.begin(), atomPositions.end(), cubes[x-1][y]->getID()) != atomPositions.end()) {
											//std::cout << "double deflection on vertical path" << std::endl;
											y -= incrementor;
											incrementor *= -1;
											continue;
										}
										horizontal = !horizontal;
										y -= incrementor;
										incrementor = -1;
									}
									if (!horizontal && x > 0
									&& std::find(atomPositions.begin(), atomPositions.end(), cubes[x-1][y]->getID()) != atomPositions.end()) {
										if (!horizontal && x < gameBoardSize-1
										&& std::find(atomPositions.begin(), atomPositions.end(), cubes[x+1][y]->getID()) != atomPositions.end()) {
											//std::cout << "double deflection on vertical path" << std::endl;
											y -= incrementor;
											incrementor *= -1;
											continue;
										}
										//std::cout << "deflected on vertical path" << std::endl;
										horizontal = !horizontal;
										y -= incrementor;
										incrementor = 1;
									}
									// next step of ray
									if (horizontal) {
										x = x + incrementor;
									}
									if (!horizontal) {
										y = y + incrementor;
									}
								}
							}
						}
					} else {
						// if an inner gameboard cube (or an atom) is selected, set or remove the respective atom (if atoms are left)
						if (selectedSceneNode->getID() > -1) {
							scene::ISceneNode * selectedAtomCube = atoms[static_cast<int>(selectedSceneNode->getID()/gameBoardSize)][selectedSceneNode->getID()%gameBoardSize];
							if (!selectedAtomCube->isVisible() && receiver.mouseState.leftButtonDown && atomsSet < maxAtoms) {
								selectedAtomCube->setVisible(true);
								++atomsSet;
							} else if (selectedAtomCube->isVisible() && receiver.mouseState.rightButtonDown) {
								selectedAtomCube->setVisible(false);
								--atomsSet;
							}
						}
					}
					//std::cout << "cube id: " << selectedSceneNode->getID() << std::endl;
					//std::cout << "atoms set: " << atomsSet << std::endl;
				}
			}

			// show points
			if (font) {
				std::stringstream ss;
				ss << "Penalty: " << penalty;
				std::string s = ss.str();
				font->draw(s.c_str(), core::rect<s32>(screenX/2-90,10,screenX/2+90,50), textcolor);
				if (feedback) {
					std::string fb;
					if (penalty == gameBoardSize) {
						fb = "Perfect!";
					} else if (penalty == gameBoardSize) {
						fb = "Awesome!";
					} else if (penalty < gameBoardSize*2) {
						fb = "Well done!";
					} else if (penalty > gameBoardSize*4) {
						fb = "Better luck next time!";
					} else if (penalty > gameBoardSize*3) {
						fb = "Getting there!";
					} else {
						fb = "Nice!";
					}
					font->draw(fb.c_str(), core::rect<s32>(10,60,200,60), textcolor);
				}
				if (atomsChanged) {
					std::stringstream ss;
					ss << "Atoms: " << nextMaxAtoms;
					std::string s = ss.str();
					font->draw(s.c_str(), core::rect<s32>(screenX-200,60,screenX-10,60), textcolor);
				}
			}

			// draw scene (or help if called) and gui
			if (receiver.context.help) {
				driver->draw2DImage(example, core::position2d<s32>((screenX-790)/2,60));
			} else {
				smgr->drawAll();
			}
			guienv->drawAll();
			driver->endScene();
		}
	}

	// delete the device
	device->drop();
	return 0;
}
