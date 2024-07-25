#include <SFML/Graphics.hpp>
#include <iostream>
#include <algorithm>
#include <cstring>
#include "State.hpp"
#include <vector>
#include <array>
#include <future>
#include <thread>

void zoomViewAt(sf::Vector2i pixel, sf::RenderWindow& window, float zoom, sf::View view) {
    //std::cout << zoom << std::endl;
    const sf::Vector2f beforeCoord{ window.mapPixelToCoords(pixel) };
    //sf::View view = window.getView();
    view.zoom(zoom);
    window.setView(view);
    const sf::Vector2f afterCoord{ window.mapPixelToCoords(pixel) };
    const sf::Vector2f offsetCoords{ beforeCoord - afterCoord };
    view.move(offsetCoords);
    window.setView(view);
}

int getPixelColour(sf::Image& image, int imageX, int imageY, char colourType) {
    sf::Color color = image.getPixel(imageX, imageY);

    if (colourType == 'r') {
        return color.r;
    }
    if (colourType == 'g') {
        return color.g;
    }
    if (colourType == 'b') {
        return color.b;
    }
    if (colourType == 'a') {
        return color.a;
    }
    
    return 0;
}

std::string getRGBA(sf::Image& texture, int imageX, int imageY) {
    std::string string = std::to_string(getPixelColour(texture, imageX, imageY, 'r')) + ' '
        + std::to_string(getPixelColour(texture, imageX, imageY, 'g')) + ' '
        + std::to_string(getPixelColour(texture, imageX, imageY, 'b')) + ' '
        + std::to_string(getPixelColour(texture, imageX, imageY, 'a'));
    return string;
}

std::string clickingState(sf::Image image, float mouseInMapPosX, float mouseInMapPosY) {
    if (getRGBA(image, (int)std::floor(mouseInMapPosX), (int)std::floor(mouseInMapPosY)) == State::T::RGBA()) {
        if (mouseInMapPosX > State::T::sx && mouseInMapPosX < State::T::lx
            && mouseInMapPosY > State::T::sy && mouseInMapPosY < State::T::ly) {
            return "T";
        }
    }
    return "";
}

int blinkSpeed = 10;
int ra = 1;

sf::Image pixelsToBlink(std::vector<std::array<int, 2>> coords, sf::Image image) {
    for (int i = 1; i != coords.size(); i++) {
        int alpha = getPixelColour(image, std::get<0>(coords[i]), std::get<1>(coords[i]), 'a');
        if (ra > 0 && alpha <= 100) {
            ra = -1;
        }
        else if (ra < 0 && alpha >= 255) {
            ra = 1;
        }
        alpha -= blinkSpeed * ra;
        image.setPixel(std::get<0>(coords[i]), std::get<1>(coords[i]), sf::Color(89, 171, 196, alpha));
    }
    return image;
}

sf::Image cropImage(sf::Image image, std::array<int, 2> cropFromCoords, std::array<int, 2> resizedSize) {
    sf::Image cropped_image;
    cropped_image.create(cropFromCoords[0], cropFromCoords[1], sf::Color(50,50,50,255));

    for (int x = cropFromCoords[0]; x <= cropFromCoords[0] + resizedSize[0]; x++) {
        for (int y = cropFromCoords[1]; y <= cropFromCoords[1] + resizedSize[1]; y++) {
            //cropped_image.setPixel(x - cropFromCoords[0], y - cropFromCoords[1], sf::Color(10, 10, 10, 255)); //image.getPixel(x, y)
        }
    }

    return cropped_image;
}

int main() {
    std::vector<std::array<int, 2>> targetCoords = { {NULL, NULL} };
    std::vector<std::array<int, 2>> nullVector = { {NULL, NULL} };

    sf::RenderWindow window(sf::VideoMode(1920, 1046), "Pvz Hoi4", sf::Style::Close | sf::Style::Resize);

    sf::View view(sf::Vector2f(0.0f, 0.0f), sf::Vector2f(1920.0f, 1046.0f));
    view.setCenter(sf::Vector2f(93000.0f, 19000.0f)); //94000.0f, 20000.0f

    float mapRatio = 20.0f;

    sf::RectangleShape world(sf::Vector2f(mapRatio * 5632.0f, mapRatio * 2048.0f)); //5632*2048
    //world.setOrigin(93000.0f, 19500.0f);
    sf::Texture texture_world;
    sf::Image image;
    image.loadFromFile("images/world.png");
    texture_world.loadFromImage(image); //, sf::IntRect(4555, 920, 200, 200)
    world.setTexture(&texture_world);

    float tx = 0.0f;
    float ty = 0.0f;
    bool leftClicking = false;
    window.setFramerateLimit(60);

    int blinkCd = 0;

    sf::RectangleShape world_blink(sf::Vector2f(15.0f * mapRatio, 15.0f * mapRatio)); //15s

    while (window.isOpen())
    {
        sf::Event e;
        while (window.pollEvent(e))
        {
            switch (e.type) {
                case sf::Event::KeyPressed:
                    if (e.key.code == sf::Keyboard::Escape) window.close();
                    break;
                case sf::Event::Closed:
                    window.close();
                    break;
                case sf::Event::Resized:
                    std::cout << "width:" << e.size.width << " height:" << e.size.height << std::endl;
                    break;
                /*case sf::Event::TextEntered:
                    printf("%c", e.text.unicode);
                    break;*/
                case sf::Event::MouseWheelScrolled:
                    zoomViewAt({ e.mouseWheelScroll.x, e.mouseWheelScroll.y }, window, std::pow(1.1f, -e.mouseWheelScroll.delta), view);
                    view = window.getView();
                    //std::cout << view.getCenter().x << " " << view.getCenter().y << std::endl;
                    break;
                case sf::Event::MouseButtonReleased:
                    //std::cout << "A" << std::endl;
                    if (e.mouseButton.button == sf::Mouse::Left && leftClicking) {
                        leftClicking = false;
                    }
                    break;
            }
        }

        //view = window.getView();
        int dtx = 0;
        int dty = 0;

        if (!(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A) &&
            sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D))) {
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A)) dtx = -1;
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D)) dtx = 1;
        }
        if (!(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W) &&
            sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S))) {
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W)) dty = -1;
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S)) dty = 1;
        }

        tx += dtx * std::max(std::min(tx, -80.0f), 80.0f);
        tx *= 0.9f;
        float dx = 0.02f * tx * view.getSize().x / 1920.0f;
        ty += dty * std::max(std::min(ty, -80.0f), 80.0f);
        ty *= 0.9f;
        float dy = 0.02f * ty * view.getSize().y / 1046.0f;

        view.move(sf::Vector2f(dx, dy));

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
            auto random = getRGBA(image, window.mapPixelToCoords(sf::Mouse::getPosition(window)).x / mapRatio, 
                window.mapPixelToCoords(sf::Mouse::getPosition(window)).y / mapRatio);*/
            float mouseInMapPosX = window.mapPixelToCoords(sf::Mouse::getPosition(window)).x / mapRatio;
            float mouseInMapPosY = window.mapPixelToCoords(sf::Mouse::getPosition(window)).y / mapRatio;
            //std::cout << mouseInMapPosX << std::endl;
            //std::cout << mouseInMapPosY << std::endl;

            std::string targetState = clickingState(image, mouseInMapPosX, mouseInMapPosY);

            //std::cout << targetState << std::endl;
            if (strcmp(targetState.c_str(), "T") == 0) {
                for (int x = State::T::sx; x <= State::T::lx; x++) {
                    for (int y = State::T::sy; y <= State::T::ly; y++) {
                        if (strcmp(getRGBA(image, x, y).c_str(), State::T::RGBA().c_str()) == 0) {
                            targetCoords.push_back({x, y});
                        }
                    }
                }
            }
        }

        window.clear();
        window.setView(view);
        window.draw(world);

        if (targetCoords != nullVector) {
            if (blinkCd < 10) {
                blinkCd++;
            } else {
                blinkCd = 0;
                sf::Image image_blink;
                sf::Texture texture_blink;
                
                image_blink = pixelsToBlink(targetCoords, image);
                image_blink = cropImage(image_blink, {4699, 986}, {15, 15});

                texture_blink.loadFromImage(image_blink); //4690
                world_blink.setTexture(&texture_blink);
                world_blink.setPosition(sf::Vector2f(view.getCenter().x - view.getSize().x / 2.0f, view.getCenter().y - view.getSize().y / 2.0f));
                /*sf::Image image_blink;
                sf::Texture texture_blink;
                sf::RectangleShape world_blink(sf::Vector2f(15.0f, 15.0f));
                image_blink = pixelsToBlink(targetCoords, image);

                texture_blink.loadFromImage(image_blink, sf::IntRect(4690, 980, 15, 15));
                world_blink.setTexture(&texture_blink);
                world_blink.setPosition(sf::Vector2f(4600.0f * mapRatio, 980.0f * mapRatio));
                window.draw(world_blink);
                //}
                //return;*/
                //updating = true;
            }
            window.draw(world_blink);
        }
        window.display();
    }
	return 0;
}

//Version 1.0.5