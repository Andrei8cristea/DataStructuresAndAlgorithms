#include <iostream>
#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>

sf::Color DARK_BLUE(10, 10, 60);

int WINDOW_WIDTH = 800;
int WINDOW_HEIGHT = 600;

enum appState { Welcome, Menu, Sorting };

class CApp {
private:
    sf::RenderWindow window;
    appState currentState;
    sf::Font font;

    //welcome screen elements declared as data
    sf::RectangleShape startButton;
    sf::Text startText;

    //menu screen
    static constexpr int NUM_METHODS = 5;
    std::string methods[NUM_METHODS] = {
        "Insertion Sort", "Selection Sort",
        "Quick Sort", "Merge Sort", "Heap Sort"
    };
    sf::RectangleShape buttons[NUM_METHODS];
    sf::Text texts[NUM_METHODS];

public:
    CApp() : window(sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), "DSA"),
             currentState(Welcome) {
        centerWindow();

        if (!font.loadFromFile("DejaVuSans.ttf")) {
            std::cerr << "Could not load font!\n";
            return;
        }

        //welcome button
        startButton.setSize(sf::Vector2f(200, 60));
        startButton.setFillColor(sf::Color::Yellow);
        startButton.setOutlineThickness(6);
        startButton.setOutlineColor(sf::Color::Cyan);
        startButton.setPosition((WINDOW_WIDTH - 200) / 2, (WINDOW_HEIGHT - 60) / 2);

        startText.setFont(font);
        startText.setString("START");
        startText.setCharacterSize(24);
        startText.setFillColor(DARK_BLUE);
        centerText(startText, startButton);

        //menu buttons
        float buttonWidth = 300;
        float buttonHeight = 50;
        float spacing = 20;
        float startY = 160;

        for (int i = 0; i < NUM_METHODS; ++i) {
            buttons[i].setSize(sf::Vector2f(buttonWidth, buttonHeight));
            buttons[i].setFillColor(sf::Color::Yellow);
            buttons[i].setOutlineColor(sf::Color::Cyan);
            buttons[i].setOutlineThickness(2);
            buttons[i].setPosition(
                window.getSize().x / 2.0f - buttonWidth / 2.0f,
                startY + i * (buttonHeight + spacing)
            );

            texts[i].setFont(font);
            texts[i].setString(methods[i]);
            texts[i].setCharacterSize(24);
            texts[i].setFillColor(sf::Color::Black);
            centerText(texts[i], buttons[i]);
        }
    }

    void centerWindow() {
        sf::VideoMode desktop = sf::VideoMode::getDesktopMode();
        int posX = (desktop.width - WINDOW_WIDTH) / 2;
        int posY = (desktop.height - WINDOW_HEIGHT) / 2;
        window.setPosition(sf::Vector2i(posX, posY));
    }

    void centerText(sf::Text &text, const sf::RectangleShape &rect) {
        sf::FloatRect textBounds = text.getLocalBounds();
        text.setOrigin(textBounds.left + textBounds.width / 2.0f,
                       textBounds.top + textBounds.height / 2.0f);
        sf::FloatRect rectBounds = rect.getGlobalBounds();
        text.setPosition(rectBounds.left + rectBounds.width / 2.0f,
                         rectBounds.top + rectBounds.height / 2.0f);
    }

    void run() {
        while (window.isOpen()) {
            sf::Event event;
            while (window.pollEvent(event)) {
                handleEvent(event);
            }
            render();
        }
    }

    void handleEvent(const sf::Event& event) {
        if (event.type == sf::Event::Closed ||
            (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Escape)) {
            window.close();
        }

        if (currentState == Welcome) {
            if (event.type == sf::Event::KeyPressed &&
                event.key.code == sf::Keyboard::Enter) {
                currentState = Menu;
            }
            if (event.type == sf::Event::MouseButtonPressed &&
                event.mouseButton.button == sf::Mouse::Left) {
                sf::Vector2f mouseWorldPos = window.mapPixelToCoords(
                    sf::Mouse::getPosition(window));
                if (startButton.getGlobalBounds().contains(mouseWorldPos)) {
                    currentState = Menu;
                }
            }
        }
        else if (currentState == Menu) {
            sf::Vector2f mouseWorldPos = window.mapPixelToCoords(
                sf::Mouse::getPosition(window));

            for (int i = 0; i < NUM_METHODS; ++i) {
                if (buttons[i].getGlobalBounds().contains(mouseWorldPos)) {
                    buttons[i].setFillColor(sf::Color::Cyan);
                    texts[i].setFillColor(sf::Color::Blue);

                    if (event.type == sf::Event::MouseButtonPressed &&
                        event.mouseButton.button == sf::Mouse::Left) {
                        //i have to transmit the method to sorting eventhandler
                        //std::cout << "Selected: " << methods[i] << "\n";
                        currentState = Sorting;
                    }
                } else {
                    buttons[i].setFillColor(sf::Color::Yellow);
                    texts[i].setFillColor(sf::Color::Black);
                }
            }
        }
        else if (currentState == Sorting) {
            //TODO - i have to take the method selected and implement it s animation here
            //TODO - first i need to analyze what is common for most animations and build from there

        }
    }

    void render() {
        window.clear(DARK_BLUE);
        switch (currentState) {
            case Welcome: renderWelcome(); break;
            case Menu: renderMenu(); break;
            case Sorting: renderSorting(); break;
        }
        window.display();
    }

    void renderWelcome() {
        sf::Text title("Sorting animation by Andrei Cristea", font, 30);
        title.setFillColor(sf::Color::Yellow);
        title.setPosition(100, 100);
        window.draw(title);

        // Hover effect
        sf::Vector2f mouseWorldPos = window.mapPixelToCoords(sf::Mouse::getPosition(window));
        if (startButton.getGlobalBounds().contains(mouseWorldPos)) {
            startButton.setFillColor(sf::Color::Cyan);
            startButton.setOutlineColor(sf::Color::Yellow);
            startText.setFillColor(sf::Color::Blue);
            startText.setCharacterSize(40);
            centerText(startText, startButton);
        } else {
            startButton.setFillColor(sf::Color::Yellow);
            startButton.setOutlineColor(sf::Color::Cyan);
            startText.setFillColor(sf::Color::Black);
            startText.setCharacterSize(24);
            centerText(startText, startButton);
        }

        window.draw(startButton);
        window.draw(startText);
    }

    void renderMenu() {
        sf::Text title("Choose the sorting algorithm:", font, 30);
        title.setFillColor(sf::Color::Yellow);
        sf::FloatRect titleBounds = title.getLocalBounds();
        title.setOrigin(titleBounds.left + titleBounds.width / 2.0f,
                        titleBounds.top + titleBounds.height / 2.0f);
        title.setPosition(window.getSize().x / 2.0f, 80);

        window.draw(title);
        for (int i = 0; i < NUM_METHODS; ++i) {
            window.draw(buttons[i]);
            window.draw(texts[i]);
        }
    }

    void renderSorting() {
        sf::Text title("Sorting in progress...", font, 30);
        //TODO
        title.setFillColor(sf::Color::White);
        title.setPosition(200, 200);
        window.draw(title);
    }
};

int main() {
    CApp app;
    app.run();
    return 0;
}
