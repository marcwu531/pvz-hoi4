#include <SFML/Graphics.hpp>
#include <iostream>
#include <algorithm>
#include <cstring>

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

int main() {
    float tx = 0.0f;
    float ty = 0.0f;
    bool leftClicking = false;
    float mapRatio = 20.0f;

    sf::RenderWindow window(sf::VideoMode(1920, 1046), "Pvz Hoi4", sf::Style::Close | sf::Style::Resize);
    //window.setVerticalSyncEnabled(true);

    sf::View view(sf::Vector2f(0.0f, 0.0f), sf::Vector2f(1920.0f, 1046.0f));
    view.setCenter(sf::Vector2f(90000.0f, 20000.0f)); //94000.0f, 20000.0f
    //window.setFramerateLimit(100);

    sf::RectangleShape world(sf::Vector2f(mapRatio*5632.0f, mapRatio*2048.0f));
    //world.setOrigin(93000.0f, 19500.0f);
    sf::Texture texture_world;
    sf::Image image;
    image.loadFromFile("images/world.png");
    texture_world.loadFromImage(image);
    world.setTexture(&texture_world);

    sf::Texture Provinces[] = { texture_world };
    
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

        tx += dtx * std::max(std::min(tx, -10.0f), 10.0f);
        tx *= 0.99f;
        float dx = 0.002f * tx * view.getSize().x / 1920.0f;
        ty += dty * std::max(std::min(ty, -10.0f), 10.0f);
        ty *= 0.99f;
        float dy = 0.002f * ty * view.getSize().y / 1046.0f;

        view.move(sf::Vector2f(dx, dy));

        if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Left) && !leftClicking) {
            leftClicking = true;
            /*std::cout << "x: " << window.mapPixelToCoords(sf::Mouse::getPosition(window)).x
                << " y: " << window.mapPixelToCoords(sf::Mouse::getPosition(window)).y << std::endl;*/
            /*std::cout << getRGBA(texture_world,
                window.mapPixelToCoords(sf::Mouse::getPosition(window)).x / mapRatio,
                window.mapPixelToCoords(sf::Mouse::getPosition(window)).y / mapRatio)
                << std::endl;*/
            auto mx = sf::Mouse::getPosition(window);
            float x = window.mapPixelToCoords(mx).x;
            auto random = getRGBA(image, window.mapPixelToCoords(sf::Mouse::getPosition(window)).x / mapRatio, 
                window.mapPixelToCoords(sf::Mouse::getPosition(window)).y / mapRatio);
            if (getRGBA(image,
                window.mapPixelToCoords(sf::Mouse::getPosition(window)).x / mapRatio,
                window.mapPixelToCoords(sf::Mouse::getPosition(window)).y / mapRatio)
                == "89 171 196 255") {
                std::cout << "T" << std::endl;
            }
        }

        //Taipei: r: 89 g: 171 b: 196 a: 255

        window.clear();
        window.setView(view);
        window.draw(world);
        window.display();
    }
	return 0;
}

/*
* auto mouse_pos = sf::Mouse::getPosition(window); // Mouse position relative to the window
auto translated_pos = window.mapPixelToCoords(mouse_pos); // Mouse position translated into world coordinates
if(sprite.getGlobalBounds().contains(translated_pos)) // Rectangle-contains-point check
    // Mouse is inside the sprite.
*/

//Version 1.0.2.a