#include <iostream>
#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>

sf::Color DARK_BLUE(10, 10, 60);


int main() {

    sf::RenderWindow window1(sf::VideoMode(800,600), "DSA");

    window1.clear(DARK_BLUE);
    window1.display();








    while (window1.isOpen()) {

        sf::Event event;

        while (window1.pollEvent(event)) {
            switch (event.type)
            {

                case sf::Event::Closed:
                    window1.close();
                    break;

                case sf::Event::KeyPressed:
                    if (event.key.code == sf::Keyboard::Escape) {
                        window1.close(); // Close window when Escape is pressed
                    }
                    if (event.key.code == sf::Keyboard::Num1) {
                        window1.clear(sf::Color::Red);
                        window1.display();
                    }
                    if (event.key.code == sf::Keyboard::Num2) {
                        window1.clear(sf::Color::Black);

                        int array[10] = {1,2,3,4,5,6,7,8,9,10} ;

                        for (int i = 0; i < 10; i++) {
                            int length = array[i];
                            sf::RectangleShape rectangle(sf::Vector2f(50.0, length*10.0f));
                            rectangle.setPosition(i * 60.0f, 500.0f - length * 10.0f); // aliniere jos
                            window1.draw(rectangle);
                        }
                        // // change the size to 100x100
                        // rectangle.setSize(sf::Vector2f(100.f, 100.f));


                        window1.display();
                    }
                    break;

                default:
                    break;
            }

        }
    }
}