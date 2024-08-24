#include <iostream>
#include <nlohmann/json.hpp>
#include <queue>
#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>

#include <thread>
#include <windows.h>

#ifdef RUN_DEBUG
#define _CRTDBG_MAP_ALLOC
#include <signal.h>
#endif

#include "Window.h"
#include "Colour.h"
#include "Resource.h"
#include "State.h"
#include "Display.h"
#include "Async.h"
#include "Scene1.h"
#include "Audio.h"
#include "Json.h"
#include "General.h"

#ifdef RUN_DEBUG
static void AttachConsole() {
	AllocConsole();
	FILE* consoleOutput;
	freopen_s(&consoleOutput, "CONOUT$", "w", stdout);
}

std::string WideStringToString(const std::wstring& wideStr) {
	int size_needed = WideCharToMultiByte(CP_UTF8, 0, wideStr.c_str(), (int)wideStr.length(),
		NULL, 0, NULL, NULL);
	std::string narrowStr(size_needed, 0);
	WideCharToMultiByte(CP_UTF8, 0, wideStr.c_str(), (int)wideStr.length(), &narrowStr[0],
		size_needed, NULL, NULL);
	return narrowStr;
}

void DisplayLastError() {
	DWORD error = GetLastError();
	if (error != 0) {
		LPWSTR msgBuffer = nullptr;
		DWORD formatResult = FormatMessage(
			FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL,
			error,
			0,
			(LPWSTR)&msgBuffer,
			0,
			NULL
		);
		if (formatResult > 0 && msgBuffer) {
			std::string errorMessage = WideStringToString(msgBuffer);
			std::cout << "Error: " << errorMessage << std::endl;
			LocalFree(msgBuffer);
		}
		else {
			std::cout << "Failed to format error message. Error code: " << error << std::endl;
		}
	}
	else {
		std::cout << "No error information available. Error code: " << error << std::endl;
	}
}

extern "C" void handle_aborts(int signal_number) {
	std::cout << "Caught signal " << signal_number << " (SIGABRT)." << std::endl;
	DisplayLastError();
	std::abort();
}
#endif

int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine,
	_In_ int nCmdShow) { //int main() {   
#ifdef RUN_DEBUG
	AttachConsole();
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	signal(SIGABRT, &handle_aborts);
	//std::abort(); //RUN_DEBUG
#endif

	srand(static_cast<unsigned>(time(0)));

	initializeAudios(hInstance);

	std::vector<char> fontData = loadResourceData(nullHInstance, 4);
	defaultFont.loadFromMemory(fontData.data(), fontData.size());

	initializeScene1();

	audios["soundtrack"]["battleofwuhan"]->setVolume(25);

	sf::RectangleShape world(sf::Vector2f(mapRatio * 5632.0f, mapRatio * 2048.0f)); //5632*2048
	//window.create(sf::VideoMode::getDesktopMode(), "Pvz Hoi4", sf::Style::Resize | sf::Style::Close);
	//world.setOrigin(93000.0f, 19500.0f);
	sf::Texture texture_world;
	//world_image.loadFromFile("world_images/world.png");
	texture_world.loadFromImage(world_image); //sf::IntRect(4555, 920, 200, 200)
	world.setTexture(&texture_world);
	view_world.setCenter(sf::Vector2f(93000.0f, 19000.0f)); //94000.0f, 20000.0f
	window.setFramerateLimit(fps);

	float tx = 0.0f, ty = 0.0f;

	thread_asyncBlinkMap = std::thread(asyncBlinkMap);
	thread_asyncLoadFlag = std::thread(asyncLoadFlag);
	thread_asyncLoadLevelStart = std::thread(asyncLoadLevelStart);

	sf::RectangleShape levelStart(sf::Vector2f(view_world.getSize().x / 2.0f, view_world.getSize().y));
	levelStart.setFillColor(sf::Color::White);

	sf::Text levelStartText("START", defaultFont, 50);
	levelStartText.setFillColor(sf::Color::Black);

	sf::RectangleShape levelStartButton(sf::Vector2f(view_world.getSize().x / 20.0f,
		view_world.getSize().y / 10.0f));
	levelStartButton.setFillColor(sf::Color::Green);

	bool leftClicking = false, rightClicking = false;

	std::queue<sf::Keyboard::Key> inputs;

	//changeScene(1); //DEBUG

	/*sf::Shader brightness_shader;
	std::vector<char> brightness_shader_data = loadResourceData(nullHInstance, 129);
	std::string brightness_shader_str(brightness_shader_data.begin(), brightness_shader_data.end());
	brightness_shader.loadFromMemory(brightness_shader_str, sf::Shader::Fragment);
	brightness_shader.setUniform("texture", sf::Shader::CurrentTexture);
	brightness_shader.setUniform("brightness", 1.75f);*/

	while (window.isOpen())
	{
		sf::Event e;
		sf::Vector2i pixelPos = sf::Mouse::getPosition(window);
		sf::Vector2f mousePos = window.mapPixelToCoords(pixelPos);

		while (window.pollEvent(e))
		{
			switch (scene) {
			default:
			case 0:
				if (e.type == sf::Event::MouseWheelScrolled) {
					zoomViewAt({ e.mouseWheelScroll.x, e.mouseWheelScroll.y }, window,
						std::pow(1.1f, -e.mouseWheelScroll.delta), view_world);
					view_world = window.getView();
					//std::cout << view.getCenter().x << " " << view.getCenter().y << std::endl;
				}
				[[fallthrough]];
			case 1:
				if (e.type == sf::Event::KeyPressed) {
					inputs.push(e.key.code);
					if (inputs.size() > konamiCode.size()) {
						inputs.pop();
					}
					if (isKonamiCodeEntered(inputs)) {
						ShellExecute(0, 0, L"https://tinyurl.com/marcwu531underphaith706", 0, 0, SW_SHOW);
						while (!inputs.empty()) {
							inputs.pop();
						}
					}

					/*if (e.key.code == 27) {
						int a = e.key.code;
						int b = e.key.code;
						//e.key.code >= 27 && <= ?
					}*/

					if (scene == 1 && pvzScene == 3 && !pvzPacketOnSelected && e.key.code >= 27
						&& e.key.code <= 26 + std::min(maxPlantSelectAmount, getOwnedPlantsAmount()))
						selectSeedPacket(seedPacketsSelectedOrder[e.key.code - 27]);

					if (e.key.code == sf::Keyboard::Escape) {
						changeScene(-1);
						window.close();
					}
				}
				else if (e.type == sf::Event::Closed) {
					changeScene(-1);
					window.close();
					/*case sf::Event::Resized:
						std::cout << "width:" << e.size.width << " height:" << e.size.height << std::endl;
						break;*/
						/*case sf::Event::TextEntered:
							printf("%c", e.text.unicode);
							break;*/
				}
				else if (e.type == sf::Event::MouseButtonReleased) {
					//std::cout << "A" << std::endl;
					if (e.mouseButton.button == sf::Mouse::Left && leftClicking) leftClicking = false;
					if (e.mouseButton.button == sf::Mouse::Right && rightClicking) rightClicking = false;
				}
				break;
			}
		}

		switch (scene) {
		default:
		case 0: {
			if (audios["soundtrack"]["battleofwuhan"]->getStatus() != sf::Music::Playing)
				audios["soundtrack"]["battleofwuhan"]->play();
			//view = window.getView();
			int dtx = 0, dty = 0;

			if (!(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A) &&
				sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D))) {
				if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A)) dtx = 1;
				if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D)) dtx = -1;
			}
			if (!(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W) &&
				sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S))) {
				if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W)) dty = 1;
				if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S)) dty = -1;
			}

			const float maxVelocity = 80.0f;
			tx += dtx * std::max(std::min(tx, -maxVelocity), maxVelocity);
			tx *= 0.9f;
			float dx = 0.02f * tx * view_world.getSize().x / 1920.0f;
			ty += dty * std::max(std::min(ty, -maxVelocity), maxVelocity);
			ty *= 0.9f;
			float dy = 0.02f * ty * view_world.getSize().y / 1046.0f;

			/*std::cout << "dtx: " << dtx << ", dty: " << dty << std::endl;
			std::cout << "tx: " << tx << ", ty: " << ty << std::endl;
			std::cout << "dx: " << dx << ", dy: " << dy << std::endl;*/

			//view.move(sf::Vector2f(dx, dy));
			world.move(sf::Vector2f(dx, dy));
			//world_blink.move(sf::Vector2f(dx, dy));

			if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Left) && !leftClicking) {
				leftClicking = true;
				/*std::cout << "x: " << window.mapPixelToCoords(sf::Mouse::getPosition(window)).x
					<< " y: " << window.mapPixelToCoords(sf::Mouse::getPosition(window)).y << std::endl;*/
					/*std::cout << getRGBA(texture_world,
						window.mapPixelToCoords(sf::Mouse::getPosition(window)).x / mapRatio,
						window.mapPixelToCoords(sf::Mouse::getPosition(window)).y / mapRatio)
						<< std::endl;*/
						/*auto mx = sf::Mouse::getPosition(window);
						float x = window.mapPixelToCoords(mx).x;
						auto random = getRGBA(world_image,
							window.mapPixelToCoords(sf::Mouse::getPosition(window)).x / mapRatio,
							window.mapPixelToCoords(sf::Mouse::getPosition(window)).y / mapRatio);*/
				const sf::FloatRect rectBounds = levelStart.getGlobalBounds();

				if (clicking_state.empty() || !rectBounds.contains(mousePos)) {
					const float mouseInMapPosX = (mousePos.x - world.getPosition().x) / mapRatio;
					const float mouseInMapPosY = (mousePos.y - world.getPosition().y) / mapRatio;
					//std::cout << mouseInMapPosX << std::endl;
					//std::cout << mouseInMapPosY << std::endl;

					checkClickingState(mouseInMapPosX, mouseInMapPosY);
				}
				else if (levelStartButton.getGlobalBounds().contains(mousePos)) {
					changeScene(1);
				}
			}
			//changeScene(1); //RUN_DEBUG
			break;
		}
		case 1:
			if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Left) && !leftClicking) {
				leftClicking = true;

				//std::cout << "Mouse position (pixels): (" << pixelPos.x << ", " << pixelPos.y << ")" 
					//<< std::endl;
				//std::cout << "Mouse position (coords): (" << mousePos.x << ", " << mousePos.y << ")" 
					//<< std::endl;

				if (pvzScene == 0) {
					for (int i = 0; i < maxPlantAmount; ++i) {
						if (seedPackets[seedPacketIdToString(i)].getGlobalBounds().contains(mousePos)) {
							switch (static_cast<int>(seedPacketState[i][0])) {
							case 0: //select place
								if (seedPacketSelected < std::min(maxPlantSelectAmount,
									getOwnedPlantsAmount())) {
									seedPacketsSelectedOrder[seedPacketsSelectedOrder.empty() ? 0 :
										seedPacketsSelectedOrder.size()] = i;
									seedPacketState[i][0] = 1;
								}
								//stdcoutMap(&seedPacketsSelectedOrder);
								break;
							case 2: //selected
							{
								for (auto& pair : seedPacketsSelectedOrder) {
									if (pair.second == i) {
										pair.second = -1;
										mapShift(seedPacketsSelectedOrder);
										break;
									}
								}

								//stdcoutMap(&seedPacketsSelectedOrder);
								seedPacketState[i][0] = 3;
								break;
							}
							default:
								break;
								//case 1: //moving
									//seedPacketState[i][1]++; //put in async loop thread
							}
						}
					}

					if (seedPacketSelected >= std::min(maxPlantSelectAmount, getOwnedPlantsAmount())) {
						if (seedChooserButton.getGlobalBounds().contains(mousePos)) pvzScene = 1;
					}
				}
				else if (pvzScene == 3) {
					if (!pvzPacketOnSelected) {
						selectSeedPacket(mousePos);
					}
					else { //plant Plant
						createPlant(hoverPlant.getPosition(), seedPacketSelectedId);
					}
				}
			}
			else if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Right) && !rightClicking) {
				createPlant(std::nullopt, -1);
			}
			break;
		}

		window.clear();

		switch (scene) {
		default:
		case 0:
		{
			window.setView(view_world);
			window.draw(world); // render first: at bottom

			const auto worldPos = world.getPosition();
			const float viewWorldCenterX = view_world.getCenter().x;
			const float viewWorldCenterY = view_world.getCenter().y;
			const float viewWorldSizeX = view_world.getSize().x;
			const float viewWorldSizeY = view_world.getSize().y;

			if (blinkMap_readyToDraw.load()) {
				world_blink.setTexture(&texture_blink);
				blinkMap_readyToDraw.store(false);
			}

			if (!clicking_state.empty()) {
				world_blink.setPosition(
					worldPos.x + blinkCoords.x * mapRatio,
					worldPos.y + blinkCoords.y * mapRatio
				);
				window.draw(world_blink);

				levelStart.setSize(sf::Vector2f(viewWorldSizeX / 2.0f, viewWorldSizeY));
				levelStart.setPosition(viewWorldCenterX - viewWorldSizeX / 2.0f,
					viewWorldCenterY - viewWorldSizeY / 2.0f);

				int size = static_cast<unsigned int>(viewWorldSizeX / 38.4f);
				levelStartText.setCharacterSize(size);

				const sf::FloatRect rectBounds = levelStart.getGlobalBounds();
				const sf::FloatRect textBounds = levelStartText.getLocalBounds();
				const float textOriginX = textBounds.left + textBounds.width / 2.0f;
				const float textOriginY = textBounds.top + textBounds.height / 2.0f;

				levelStartText.setOrigin(textOriginX, textOriginY);
				levelStartText.setPosition(rectBounds.left + rectBounds.width / 2.0f,
					rectBounds.top + rectBounds.height / 2.0f);

				levelStartButton.setSize(sf::Vector2f(textBounds.width, textBounds.height));
				levelStartButton.setPosition(levelStartText.getPosition().x - textBounds.width / 2.0f,
					levelStartText.getPosition().y - textBounds.height / 2.0f);

				window.draw(levelStart);
				window.draw(levelStartButton);
				window.draw(levelStartText);
			}

			if (loadFlag_readyToDraw.load()) {
				flag_rect.setTexture(&flag_texture);
				loadFlag_readyToDraw.store(false);
			}

			if (!flag.empty()) {
				flag_rect.setSize(sf::Vector2f(15 * mapRatio * viewWorldSizeX / window.getSize().x,
					10 * mapRatio * viewWorldSizeY / window.getSize().y)); // 3:2
				flag_rect.setPosition(viewWorldCenterX - viewWorldSizeX / 2.0f,
					viewWorldCenterY - viewWorldSizeY / 2.0f);
				window.draw(flag_rect);
			}
			break;
		}

		case 1:
			window.setView(view_background);
			window.draw(background);

			if (!zombiesOnScene.empty()) {
				std::sort(zombiesOnScene.begin(), zombiesOnScene.end(), [](const auto& a, const auto& b) {
					return a.anim.row < b.anim.row;
					});

				for (const auto& zombie : zombiesOnScene) {
					if (zombie.damagedCd > 0) {
						window.draw(zombie.anim.sprite/*, &brightness_shader*/);
					}
					else {
						window.draw(zombie.anim.sprite);
					}
				}
			}

			if (pvzScene == 0 || pvzScene == 1) {
				window.draw(seedChooser_background);
				window.draw(seedChooserButton);
			}

			if (pvzScene == 2) {
				window.draw(pvzStartText);
			}

			pvzSunText.setString(std::to_string(pvzSun));
			const sf::FloatRect pvzSunTextRect = pvzSunText.getLocalBounds();
			pvzSunText.setOrigin(pvzSunTextRect.left + pvzSunTextRect.width / 2.0f,
				pvzSunTextRect.top + pvzSunTextRect.height / 2.0f);
			window.draw(seedBank);
			window.draw(pvzSunText);

			if (pvzScene == 0 || pvzScene == 1) {
				for (int i = 0; i < maxPlantAmount; ++i) {
					if (plantExist(i)) {
						window.draw(seedPackets.find(seedPacketIdToString(i))->second);
					}
				}
			}
			else {
				for (const auto& sp : seedPacketsSelectedOrder) {
					if (plantExist(sp.second)) {
						window.draw(seedPackets.find(seedPacketIdToString(sp.second))->second);
					}
				}
			}

			if (pvzScene == 3) {
				if (pvzPacketOnSelected) {
					window.draw(overlayShade);
					if (canPlant(hoverPlant.getPosition())) {
						window.draw(hoverPlant);
						window.draw(hoverShade);
					}
					window.draw(idlePlants[idlePlantToString[seedPacketSelectedId]]);
				}

				if (!plantsOnScene.empty()) {
					for (const auto& plant : plantsOnScene) {
						if (plant.damagedCd > 0) {
							window.draw(plant.anim.sprite/*, &brightness_shader*/);
						}
						else {
							window.draw(plant.anim.sprite);
						}
					}
				}

				if (!projectilesOnScene.empty()) {
					for (const auto& projectile : projectilesOnScene) {
						if (projectile.sprite.getTexture() != nullptr) {
							window.draw(projectile.sprite);
						}
					}
				}

				for (auto it = vanishProjectilesOnScene.begin(); it != vanishProjectilesOnScene.end();) {
					if (++it->frame >= 13) {
						it = vanishProjectilesOnScene.erase(it);
					}
					else {
						it->proj.sprite.setTextureRect(peaSplatsFrames[std::min(it->frame, 3)].frameRect);
						window.draw(it->proj.sprite);
						++it;
					}
				}
			}
			break;
		}

		window.display();
	}

	changeScene(-1);

#ifdef RUN_DEBUG
	FreeConsole();
	_CrtDumpMemoryLeaks();
#endif

	return 0;
}

//Version 1.0.34