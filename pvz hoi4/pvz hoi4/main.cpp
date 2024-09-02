#include <iostream>
#include <nlohmann/json.hpp>
#include <queue>
#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>
#include <shared_mutex>
#include <string>
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
#include "Account.h"
#include "Level.h"

//#define CENSORED

int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine,
	_In_ int nCmdShow) { //int main() {
#ifdef RUN_DEBUG
	AttachConsole();
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	signal(SIGABRT, &handle_aborts);
	//std::abort(); //RUN_DEBUG
#endif

	srand(static_cast<unsigned int>(time(nullptr)));

	initializeAudios(hInstance);

	std::vector<char> fontData = loadResourceData(nullHInstance, 4);
	defaultFont.loadFromMemory(fontData.data(), fontData.size());

	initializeScene1();

	//audios["soundtrack"]["battleofwuhan"]->setVolume(25); //RUN_NDEBUG

	sf::RectangleShape worldRect(sf::Vector2f(mapRatio * 5632.0f, mapRatio * 2048.0f)); //5632*2048
	//window.create(sf::VideoMode::getDesktopMode(), "Pvz Hoi4", sf::Style::Resize | sf::Style::Close);
	//world.setOrigin(93000.0f, 19500.0f);
	sf::Texture texture_world;
	//world_image.loadFromFile("world_images/world.png");
	texture_world.loadFromImage(world_image); //sf::IntRect(4555, 920, 200, 200)
	worldRect.setTexture(&texture_world);
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

	sf::Shader damaged_shader;
	std::vector<char> brightness_shader_data = loadResourceData(nullHInstance, 129);
	std::string brightness_shader_str(brightness_shader_data.begin(), brightness_shader_data.end());
	damaged_shader.loadFromMemory(brightness_shader_str, sf::Shader::Fragment);
	damaged_shader.setUniform("texture", sf::Shader::CurrentTexture);
	damaged_shader.setUniform("brightness", 1.75f);

	sf::Texture accountButtonTexture;
	sf::RectangleShape accountButton;
#ifdef CENSORED
	auto seedChooserBgImage = loadImageFromResource(nullHInstance, 133);
	auto awardSceneImage = loadImageFromResource(nullHInstance, 133);
#else
	auto seedChooserBgImage = loadImageFromResource(nullHInstance, 138);
	auto awardScreenImage = loadImageFromResource(nullHInstance, 139);
#endif
	accountButtonTexture.loadFromImage(seedChooserBgImage);
	accountButton.setTexture(&accountButtonTexture);

	sf::RectangleShape loginMenu;
	sf::RectangleShape usernameBox;
	usernameBox.setFillColor(sf::Color(192, 192, 192));
	usernameBox.setOutlineColor(sf::Color::Black);

	sf::Text usernameText;
	usernameText.setFont(defaultFont);
	usernameText.setFillColor(sf::Color(0, 0, 0, 64));
	usernameText.setString("username");
	//passwordText.setString(std::string(password.length(), '*'));

	bool enteringUsername = false;
	std::string username;

	sf::RectangleShape saveUsernameButton;
	saveUsernameButton.setFillColor(sf::Color::Green);

	sf::Text saveUsernameText;
	saveUsernameText.setFont(defaultFont);
	saveUsernameText.setString("SAVE");
	saveUsernameText.setFillColor(sf::Color::Black);

	sf::Text exportAccountText;
	exportAccountText.setFont(defaultFont);
	exportAccountText.setFillColor(sf::Color::Black);

	sf::Text levelId;
	levelId.setFont(defaultFont);
	levelId.setFillColor(sf::Color::Black);

	sf::RectangleShape selectCountryScreen;

	sf::Text selectCountryText;
	selectCountryText.setFont(defaultFont);
	selectCountryText.setString("Select Your Country");
	selectCountryText.setFillColor(sf::Color::Black);

	winLevelScreen.setFillColor(sf::Color(255, 255, 255, 0));

	flag_texture.create(383, 256);
	flag_texture.update(getFlagImage("Taiwan"));
	flag_rect.setTexture(&flag_texture);

	/*float pi = std::atan(1.0f) * 4.0f;
	float e = std::exp(1.0f);
	float phi = (1.0f + std::sqrt(5.0f)) / 2.0f;*/

	sf::Texture awardScreenTexture;
	awardScreenTexture.loadFromImage(awardScreenImage);
	awardScreen.setTexture(&awardScreenTexture);

	while (window.isOpen())
	{
		sf::Event evt;
		sf::Vector2i pixelPos = sf::Mouse::getPosition(window);
		sf::Vector2f mousePos = window.mapPixelToCoords(pixelPos);

		while (window.pollEvent(evt))
		{
			switch (scene) {
			default:
			case 0:
				if (evt.type == sf::Event::MouseWheelScrolled) {
					if (!loggingIn && plantExist(0)) {
						zoomViewAt({ evt.mouseWheelScroll.x, evt.mouseWheelScroll.y }, window,
							std::pow(1.1f, -evt.mouseWheelScroll.delta), view_world);
						view_world = window.getView();
						//std::cout << view.getCenter().x << " " << view.getCenter().y << std::endl;
					}
				}
				else if (evt.type == sf::Event::TextEntered && enteringUsername) {
					if (evt.text.unicode == '\b') {
						if (usernameText.getFillColor() != sf::Color(0, 0, 0, 64)) username.pop_back();
					}
					else if (evt.text.unicode > 31 && evt.text.unicode < 127 &&
						evt.text.unicode != 36 && evt.text.unicode != 94 && //$^|\ 
						evt.text.unicode != 124 && evt.text.unicode != 92) { //127: delete
						username += static_cast<char>(evt.text.unicode);
					}

					if (username.empty()) {
						usernameText.setFillColor(sf::Color(0, 0, 0, 64));
						usernameText.setString("username");
					}
					else {
						usernameText.setFillColor(sf::Color(0, 0, 0, 255));
						usernameText.setString(username);
					}
				}
				[[fallthrough]];
			case 1:
				if (evt.type == sf::Event::KeyPressed) {
					inputs.push(evt.key.code);
					if (inputs.size() > konamiCode.size()) {
						inputs.pop();
					}
					if (isKonamiCodeEntered(inputs)) {
						ShellExecute(0, 0, L"https://tinyurl.com/marcwu531underphaith706", 0, 0, SW_SHOW);
						while (!inputs.empty()) {
							inputs.pop();
						}
					}

					/*if (evt.key.code == 27) {
						int a = evt.key.code;
						int b = evt.key.code;
						//evt.key.code >= 27 && <= ?
					}*/

					if (scene == 1 && pvzScene == 3 && !pvzPacketOnSelected && evt.key.code >= 27
						&& evt.key.code <= 26 + std::min(maxPlantSelectAmount, getOwnedPlantsAmount()))
						selectSeedPacket(seedPacketsSelectedOrder[evt.key.code - 27]);

					if (evt.key.code == sf::Keyboard::Escape) {
						changeScene(-1);
						window.close();
					}
				}
				else if (evt.type == sf::Event::Closed) {
					changeScene(-1);
					window.close();
					/*case sf::Event::Resized:
						std::cout << "width:" << evt.size.width << " height:" << evt.size.height << std::endl;
						break;*/
						/*case sf::Event::TextEntered:
							printf("%c", evt.text.unicode);
							break;*/
				}
				else if (evt.type == sf::Event::MouseButtonReleased) {
					//std::cout << "A" << std::endl;
					if (evt.mouseButton.button == sf::Mouse::Left && leftClicking) leftClicking = false;
					if (evt.mouseButton.button == sf::Mouse::Right && rightClicking) rightClicking = false;
				}
				break;
			}
		}

		switch (scene) {
		case 0: {
			if (audios["soundtrack"]["battleofwuhan"]->getStatus() != sf::Music::Playing)
				audios["soundtrack"]["battleofwuhan"]->play();
			//view = window.getView();
			int dtx = 0, dty = 0;

			if (!loggingIn && plantExist(0)) {
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
			worldRect.move(sf::Vector2f(dx, dy));
			//world_blink.move(sf::Vector2f(dx, dy));

			if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Left) && !leftClicking) {
				leftClicking = true;

				if (accountButton.getGlobalBounds().contains(mousePos)) {
					loggingIn = !loggingIn;

					if (loggingIn && !account.username.empty()) {
						username = account.username;
						usernameText.setString(username);
					}
				}
				else if (loggingIn && !loginMenu.getGlobalBounds().contains(mousePos)) {
					loggingIn = false;
				}
				else {
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
					if (!loggingIn && plantExist(0)) {
						const sf::FloatRect rectBounds = levelStart.getGlobalBounds();

						if (clicking_state.empty() || !rectBounds.contains(mousePos)) {
							//std::cout << mouseInMapPosX << std::endl;
							//std::cout << mouseInMapPosY << std::endl;

							std::string levelIdStr =
								checkClickingState((mousePos.x - worldRect.getPosition().x) / mapRatio,
									(mousePos.y - worldRect.getPosition().y) / mapRatio);
							if (!levelIdStr.empty()) levelId.setString(levelIdStr);
						}
						else if (levelStartButton.getGlobalBounds().contains(mousePos)) {
							changeScene(1);
						}
					}
					else {
						if (loggingIn) {
							enteringUsername = false;

							if (saveUsernameButton.getGlobalBounds().contains(mousePos)) {
								if (!username.empty()) {
									account.username = username;
									std::shared_lock<std::shared_mutex> accountReadLock(accountMutex);
									exportAccountText.setString(encryptAccount(account));
								}
								else {
									exportAccountText.setString("Invalid Data");
								}
							}
							else if (usernameBox.getGlobalBounds().contains(mousePos)) {
								enteringUsername = true;
							}
						}
						else if (!plantExist(0)) {
							if (seedPackets[seedPacketIdToString(0)].getGlobalBounds().contains(mousePos)) {
								flag_rect.setOrigin(0, 0);
								unlockPlant(0);
							}
						}
					}
				}
			}
			//changeScene(1); //RUN_DEBUG
			break;
		}
		case 1:
			if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Left)) {
				if (!leftClicking) {
					leftClicking = true;

					//std::cout << "Mouse position (pixels): (" << pixelPos.x << ", " << pixelPos.y << ")" 
						//<< std::endl;
					//std::cout << "Mouse position (coords): (" << mousePos.x << ", " << mousePos.y << ")"
						//<< std::endl;

					if (pvzScene == 0 && !selectingSeedPacket) {
						for (int i = 0; i < maxPlantAmount; ++i) {
							if (seedPackets[seedPacketIdToString(i)].getGlobalBounds().contains(mousePos)) {
								selectingSeedPacket = true;
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
								//default:
									//break;
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
						if (!selectSun(mousePos)) {
							if (!pvzPacketOnSelected) {
								selectSeedPacket(mousePos);
							}
							else { //plant Plant
								createPlant(hoverPlant.getPosition(), seedPacketSelectedId);
							}
						}
					}
					else if (pvzScene == 4) {
						if (seedPackets[seedPacketIdToString(getPlantIdByLevel())]
							.getGlobalBounds().contains(mousePos)) {
							pvzScene = 5;
						}
					}
				}
				if (pvzScene == 3) {
					selectSun(mousePos);
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
			window.draw(worldRect); // render first: at bottom

			worldPos = worldRect.getPosition();
			const float viewWorldCenterX = view_world.getCenter().x;
			const float viewWorldCenterY = view_world.getCenter().y;
			const float viewWorldSizeX = view_world.getSize().x;
			const float viewWorldSizeY = view_world.getSize().y;

			if (blinkMap_readyToDraw.load()) {
				world_blink.setTexture(&texture_blink);
				blinkMap_readyToDraw.store(false);
			}

			if (!clicking_state.empty()) {
				{
					std::shared_lock<std::shared_mutex> mapReadLock(mapMutex);
					world_blink.setPosition(sf::Vector2f(mapSx * mapRatio, mapSy * mapRatio) + worldPos);
					window.draw(world_blink);
				}

				levelStart.setSize(sf::Vector2f(viewWorldSizeX / 2.0f, viewWorldSizeY));
				levelStart.setPosition(viewWorldCenterX - viewWorldSizeX / 2.0f,
					viewWorldCenterY - viewWorldSizeY / 2.0f);

				const sf::FloatRect rectBounds = levelStart.getGlobalBounds();

				const sf::FloatRect levelIdTextBounds = levelId.getLocalBounds();

				levelId.setCharacterSize(static_cast<unsigned int>(std::trunc(viewWorldSizeX / 20.0f)));
				levelId.setOrigin(levelIdTextBounds.left + levelIdTextBounds.width / 2.0f,
					levelIdTextBounds.top + levelIdTextBounds.height / 2.0f);
				levelId.setPosition(std::trunc(rectBounds.left + rectBounds.width / 2.0f),
					std::trunc(rectBounds.top + rectBounds.height / 3.0f));

				window.draw(levelStart);

				if (world == 1 && level == 1) {
					levelStartText.setCharacterSize(static_cast<unsigned int>(std::trunc(viewWorldSizeX / 38.4f)));

					const sf::FloatRect textBounds = levelStartText.getLocalBounds();

					levelStartText.setOrigin(textBounds.left + textBounds.width / 2.0f,
						textBounds.top + textBounds.height / 2.0f);
					levelStartText.setPosition(std::trunc(rectBounds.left + rectBounds.width / 2.0f),
						std::trunc(rectBounds.top + rectBounds.height / 2.0f));

					levelStartButton.setSize(sf::Vector2f(textBounds.width, textBounds.height));
					levelStartButton.setPosition(levelStartText.getPosition().x - textBounds.width / 2.0f,
						levelStartText.getPosition().y - textBounds.height / 2.0f);

					window.draw(levelStartButton);
					window.draw(levelStartText);
				}

				window.draw(levelId);
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

			accountButton.setSize(sf::Vector2f(viewWorldSizeX / 15.0f,
				viewWorldSizeX / 15.0f / (1.0f + std::sqrt(5.0f)) * 2.0f));
			accountButton.setOrigin(accountButton.getSize());
			accountButton.setPosition(viewWorldCenterX + viewWorldSizeX / 2.0f,
				viewWorldCenterY + viewWorldSizeY * 0.46f);
			//(e * (pi * pi - 1.0f)) / (10.0f * pi * phi)
			window.draw(accountButton);

			if (loggingIn) {
				loginMenu.setSize(sf::Vector2f(viewWorldSizeX / 3.0f, 2.0f * viewWorldSizeY / 3.0f));
				loginMenu.setPosition(viewWorldCenterX - loginMenu.getSize().x / 2.0f,
					viewWorldCenterY - 3.0f * viewWorldSizeY / 8.0f);

				usernameBox.setSize(sf::Vector2f(viewWorldSizeX / 5.0f, 2.0f * viewWorldSizeY / 50.0f));
				usernameBox.setPosition(viewWorldCenterX - usernameBox.getSize().x / 2.0f,
					viewWorldCenterY - viewWorldSizeY / 4.0f);
				usernameBox.setOutlineThickness(viewWorldSizeX / 1000.0f);

				usernameText.setCharacterSize(static_cast<unsigned int>(viewWorldSizeX / 50.0f));
				usernameText.setOrigin(usernameText.getGlobalBounds().width / 2.0f, 0.0f);
				usernameText.setPosition(viewWorldCenterX, viewWorldCenterY - viewWorldSizeY / 4.0f);

				saveUsernameButton.setSize(sf::Vector2f(viewWorldSizeX / 8.0f, 2.0f * viewWorldSizeY / 30.0f));
				saveUsernameButton.setPosition(viewWorldCenterX - saveUsernameButton.getSize().x / 2.0f,
					viewWorldCenterY - viewWorldSizeY / 32.0f);

				saveUsernameText.setCharacterSize(static_cast<unsigned int>(viewWorldSizeX / 30.0f));
				saveUsernameText.setOrigin(saveUsernameText.getGlobalBounds().width / 2.0f, 0.0f);
				saveUsernameText.setPosition(viewWorldCenterX, viewWorldCenterY - viewWorldSizeY / 32.0f);

				exportAccountText.setCharacterSize(static_cast<unsigned int>(viewWorldSizeX / 50.0f));
				exportAccountText.setOrigin(exportAccountText.getGlobalBounds().width / 2.0f, 0.0f);
				exportAccountText.setPosition(viewWorldCenterX, viewWorldCenterY + viewWorldSizeY / 16.0f);

				window.draw(loginMenu);
				window.draw(usernameBox);
				window.draw(usernameText);
				window.draw(saveUsernameButton);
				window.draw(saveUsernameText);
				window.draw(exportAccountText);
			}
			else {
				if (!plantExist(0)) {
					selectCountryScreen.setSize(sf::Vector2f(viewWorldSizeX, 4.0f * viewWorldSizeY / 5.0f));
					selectCountryScreen.setPosition(viewWorldCenterX - selectCountryScreen.getSize().x / 2.0f,
						viewWorldCenterY - selectCountryScreen.getSize().y / 2.0f);

					sf::RectangleShape& ps = seedPackets[seedPacketIdToString(0)];
					ps.setSize(sf::Vector2f(viewWorldSizeX / 10.0f, viewWorldSizeX / 10.0f * 7.0f / 5.0f));
					ps.setOrigin(ps.getSize().x / 2.0f, ps.getSize().y);
					ps.setPosition(viewWorldCenterX,
						selectCountryScreen.getPosition().y + selectCountryScreen.getSize().y * 0.9f);

					selectCountryText.setCharacterSize(static_cast<unsigned int>(viewWorldSizeX / 20.0f));
					selectCountryText.setOrigin(selectCountryText.getGlobalBounds().width / 2.0f, 0.0f);
					selectCountryText.setPosition(viewWorldCenterX, viewWorldCenterY - viewWorldSizeY / 2.5f);

					flag_rect.setSize(sf::Vector2f(viewWorldSizeX / 5.0f, viewWorldSizeX / 5.0f * 2.0f / 3.0f));
					flag_rect.setOrigin(flag_rect.getSize().x / 2.0f, 0);
					flag_rect.setPosition(viewWorldCenterX, selectCountryScreen.getPosition().y + 
						selectCountryText.getGlobalBounds().height * 1.5f);

					window.draw(selectCountryScreen);
					window.draw(seedPackets[seedPacketIdToString(0)]);
					window.draw(selectCountryText);
					window.draw(flag_rect);
				}
			}
			break;
		}

		case 1:
			window.setView(view_background);

			if (pvzScene < 6) {
				window.draw(background);

				/*const float viewWorldCenterX = view_background.getCenter().x;
				const float viewWorldCenterY = view_background.getCenter().y;
				const float viewWorldSizeX = view_background.getSize().x;
				const float viewWorldSizeY = view_background.getSize().y;*/

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
						std::shared_lock<std::shared_mutex> plantReadLock(plantsMutex);

						for (const auto& plant : plantsOnScene) {
							if (plant.damagedCd > 0) {
								window.draw(plant.anim.sprite, &damaged_shader);
							}
							else {
								window.draw(plant.anim.sprite);
							}
						}
					}

					std::shared_lock<std::shared_mutex> projReadLock(projsMutex);
					if (!projectilesOnScene.empty()) {
						for (const auto& projectile : projectilesOnScene) {
							window.draw(projectile.sprite);
						}
					}
				}

				if (!zombiesOnScene.empty()) {
					std::unique_lock<std::shared_mutex> zombieWriteLock(zombiesMutex);

					std::array<std::vector<zombieState>, 5> tempZombies; //5 rows rn

					for (auto& zombie : zombiesOnScene) {
						if (zombie.anim.row >= 0 && static_cast<size_t>(zombie.anim.row.value())
							< tempZombies.size())
							tempZombies[zombie.anim.row.value()].push_back(zombie);
					}

					zombiesOnScene.clear();
					for (auto& tmpZ : tempZombies) {
						zombiesOnScene.insert(zombiesOnScene.end(), tmpZ.begin(), tmpZ.end());
					}

					zombieWriteLock.unlock();
					std::shared_lock<std::shared_mutex> zombieReadLock(zombiesMutex);

					/*(std::sort(zombiesOnScene.begin(), zombiesOnScene.end(),
						[](const zombieState& a, const zombieState& b) {
						return a.anim.row < b.anim.row;
					});*/

					for (const auto& zombie : zombiesOnScene) {
						if (zombie.damagedCd > 0) {
							window.draw(zombie.anim.sprite, &damaged_shader);
						}
						else {
							window.draw(zombie.anim.sprite);
						}
					}
				}

				{
					std::unique_lock<std::shared_mutex> vanishProjWriteLock(vanishProjsMutex);
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

				{
					std::shared_lock<std::shared_mutex> sunReadLock(sunsMutex);
					for (auto& sun : sunsOnScene) {
						window.draw(sun.anim.sprite);
					}
				}

				if (pvzScene == 4 || pvzScene == 5) {
					if (pvzScene == 5) {
						window.draw(winLevelScreen);
					}

					window.draw(seedPackets[seedPacketIdToString(getPlantIdByLevel())]);
				}
			} else {
				window.draw(awardScreen);
				window.draw(winLevelScreen);
				window.draw(seedPackets[seedPacketIdToString(getPlantIdByLevel())]);
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

//Version 1.0.46