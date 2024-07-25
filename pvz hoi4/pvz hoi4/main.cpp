#include <SFML/Graphics.hpp>
#include <SFML/Graphics/Image.hpp>
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

    switch (colourType) {
        case 'r': return color.r;
        case 'g': return color.g;
        case 'b': return color.b;
        case 'a': return color.a;
        default: return 0;
    }
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

int blinkSpeed = 5;
int alpha = 254;
int ra = 1;

sf::Image pixelsToBlink(std::vector<std::array<int, 2>> coords, sf::Image image) {
    if (ra > 0 && alpha <= 100) {
        ra = -1;
    }
    else if (ra < 0 && alpha >= 255) {
        ra = 1;
    }
    alpha -= blinkSpeed * ra;
    alpha = std::max(std::min(alpha, 254), 100);
    for (const auto& coord : coords) {
        //int alpha = getPixelColour(image, coord[0], coord[1], 'a');
        //std::cout << alpha << std::endl;
        sf::Color ogColor = image.getPixel(coord[0], coord[1]);
        image.setPixel(coord[0], coord[1], sf::Color(ogColor.r, ogColor.g, ogColor.b, alpha));
    }
    return image;
}

sf::Image cropImage(const sf::Image image, const sf::IntRect& cropArea) {
    sf::Image cropped_image;
    cropped_image.create(cropArea.width, cropArea.height, sf::Color::Transparent);

    for (int x = 0; x < cropArea.width; x++) {
        for (int y = 0; y < cropArea.height; y++) {
            cropped_image.setPixel(x, y, image.getPixel(cropArea.left + x, cropArea.top + y));
        }
    }

    return cropped_image;
}

sf::Image image;

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
    image.loadFromFile("images/world.png");
    texture_world.loadFromImage(image); //, sf::IntRect(4555, 920, 200, 200)
    world.setTexture(&texture_world);

    sf::RectangleShape world_blink(sf::Vector2f(2 * mapRatio * (State::T::lx - State::T::sx + 1), 
        mapRatio * (State::T::ly - State::T::sy + 1))); //15s
    sf::Texture texture_blink;

    float tx = 0.0f;
    float ty = 0.0f;
    bool leftClicking = false;
    window.setFramerateLimit(60);

    int blinkCd = 0;

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
            if (targetState == "T") {
                for (int x = State::T::sx; x <= State::T::lx; x++) {
                    for (int y = State::T::sy; y <= State::T::ly; y++) {
                        if (getRGBA(image, x, y) == State::T::RGBA()) {
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
                //image_blink.create(11, 12, sf::Color::Red);

                image_blink = pixelsToBlink(targetCoords, image);

                sf::IntRect cropArea(State::T::sx, State::T::sy, State::T::lx - State::T::sx + 1, State::T::ly - State::T::sy + 1);
                image_blink = cropImage(image_blink, cropArea);

                texture_blink.loadFromImage(image_blink);

                world_blink.setTexture(&texture_blink);
                world_blink.setPosition(sf::Vector2f(State::T::sx* mapRatio, State::T::sy* mapRatio));
                //State::T::sx * mapRatio, State::T::sy * mapRatio)
                //view.getCenter().x - view.getSize().x / 2.0f, view.getCenter().y - view.getSize().y / 2.0f)
                //window.draw(world_blink);
            }
            window.draw(world_blink);
        }
        window.display();
    }
	return 0;
}

//Version 1.0.6