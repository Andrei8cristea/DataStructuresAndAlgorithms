#include <iostream>
#include <ctime>
#include <random>
#include <climits>
#include <memory>

#include <sstream>
#include <iomanip>
#include <cmath>

#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>

sf::Color DARK_BLUE(10, 10, 60);

constexpr int WINDOW_WIDTH = 800;
constexpr int WINDOW_HEIGHT = 600;

constexpr float LATERAL_MARGIN = 50.0f;
constexpr float SPACE_BETWEEN_BARS = 2.0f;
constexpr int CONST_MAX_LENGTH = 100;
constexpr int NUMBER_OF_COLUMNS = 100;
constexpr float DURATION = 1.0f;

// -----------------------------
// Steps / Buffer (shared types)
// -----------------------------
enum ActionKind {
    ACT_COMPARE = 0,
    ACT_SWAP = 1,
    ACT_OVERWRITE = 2,
    ACT_HIGHLIGHT = 3
};

struct SStep {
    ActionKind kind;
    int i;
    int j;
    int value; //eventually for overwrite
};

struct SStepBuffer {
    SStep* data;
    int size;
    int capacity;

    SStepBuffer(): data(nullptr), size(0), capacity(0) {}
    ~SStepBuffer() { delete[] data; }

    void reserve(int newCapacity) {
        if (newCapacity <= capacity) return;
        auto* nd = new SStep[newCapacity];
        for (int k = 0; k < size; ++k) nd[k] = data[k];
        delete[] data;
        data = nd;
        capacity = newCapacity;
    }

    void push_back(const SStep &s) {
        if (size == capacity) reserve(capacity == 0 ? 16 : capacity * 2);
        data[size++] = s;
    }

    void clear() { size = 0; }
};

// -----------------------------
// Sorter classes (algorithms)
// -----------------------------
class CSorter {
protected:
    int data[CONST_MAX_LENGTH]{};  //initializing array with 0
    int size;

public:
    CSorter(const int input[], int n) : size(n) {
        for (int i = 0; i < n; ++i) data[i] = input[i];
    }

    virtual ~CSorter() = default;

    //selection sort with optional recording
    void selectionSort(SStepBuffer* rec = nullptr) {
        for (int j = 0; j < size - 1; ++j) {
            int minVal = INT_MAX;
            int posMin = -1;
            for (int i = j; i < size; ++i) {
                int elem = data[i];
                if (rec) rec->push_back(SStep{ACT_COMPARE, posMin, i, 0});
                if (elem < minVal) {
                    minVal = elem;
                    posMin = i;
                }
            }
            if (posMin != j) {
                if (rec) rec->push_back(SStep{ACT_SWAP, j, posMin, 0});
                std::swap(data[j], data[posMin]);
            }
            if (rec) rec->push_back(SStep{ACT_HIGHLIGHT, j, -1, 0});
        }
    }

    virtual void heapSort() {}
    virtual void mergeSort() {}
    virtual void quickSort() {}

    void insertionSort(SStepBuffer* rec = nullptr) {
        for (int i = 0; i <= size - 1; ++i) {
            int j = i;
            while (j > 0 && data[j-1] > data[j]) {
                if (rec) {
                    rec->push_back(SStep{ACT_COMPARE,j-1,j,0});
                    rec->push_back(SStep{ACT_SWAP,j,j-1,0});
                }
                std::swap(data[j], data[j-1]);
                --j;
            }
            if (rec) rec->push_back(SStep{ACT_HIGHLIGHT, i, -1, 0});
        }
    }
};

class CHeapSorter : public CSorter {
public:
    CHeapSorter(const int input[], int n): CSorter(input, n) {}


    void heapSort() override {
        for (int i = size/2 - 1; i >= 0; --i) heapify(size, i);
        for (int i = size - 1; i > 0; --i) {
            std::swap(data[0], data[i]);
            heapify(i, 0);
        }
    }
private:
    void heapify(int n, int i) {
        int largest = i;
        int left = 2*i + 1;
        int right = 2*i + 2;
        if (left < n && data[left] > data[largest]) largest = left;
        if (right < n && data[right] > data[largest]) largest = right;
        if (largest != i) {
            std::swap(data[i], data[largest]);
            heapify(n, largest);
        }
    }
};

class CMergeSorter : public CSorter {
public:
    CMergeSorter(const int input[], int n): CSorter(input, n) {}


    void mergeSort(SStepBuffer* rec = nullptr) {
        mergeSortHelper(data, size, rec, 0);
    }
private:
    void static mergeSortHelper(int array[], int length, SStepBuffer* rec, int start) {
        if (length <= 1) return;

        int middle = length / 2;
        int leftArray[CONST_MAX_LENGTH];
        int rightArray[CONST_MAX_LENGTH];
        int leftSize = middle, rightSize = length - middle;

        for (int i = 0; i < leftSize; ++i) {
            leftArray[i]  = array[i];
        }
        for (int i = 0; i < rightSize; ++i) {
            rightArray[i] = array[middle + i];
        }

        mergeSortHelper(leftArray, leftSize, rec,start);
        mergeSortHelper(rightArray, rightSize, rec, start + middle);
        merge(leftArray, leftSize, rightArray, rightSize, array, start, middle, rec);
    }

    static void merge(const int leftArray[], int leftSize, const int rightArray[],
        int rightSize, int array[], int start, int middle, SStepBuffer* rec) {

        int i = 0, l = 0, r = 0;

        while (l < leftSize && r < rightSize) {
            int leftGlobalIdx = start + l;
            int rightGlobalIdx = start +  middle + r;
            int writeGlobalIdx = start + i;

            if (rec) rec->push_back(SStep{ACT_COMPARE,leftGlobalIdx,rightGlobalIdx,0});



            if (leftArray[l] <= rightArray[r]) {
                array[i] = leftArray[l];
                if (rec) rec->push_back(SStep{ACT_OVERWRITE,writeGlobalIdx, -1, leftArray[l]});
                ++l;
            }
            else {
                array[i] = rightArray[r];
                if (rec) rec->push_back(SStep{ACT_OVERWRITE,writeGlobalIdx, -1, rightArray[r]});
                ++r;
            }
            ++i;
        }
        while (l < leftSize) {
            int writeGlobalIdx = start + i;
            array[i] = leftArray[l];
            if (rec) rec->push_back(SStep{ACT_OVERWRITE, writeGlobalIdx, -1, leftArray[l]});
            ++l; ++i;
        }
        while (r < rightSize) {
            int writeGlobalIdx = start + i;
            array[i] = rightArray[r];
            if (rec) rec->push_back(SStep{ACT_OVERWRITE, writeGlobalIdx, -1, rightArray[r]});
            ++r; ++i;
        }

        if (rec) {
            for (int k = 0; k < i; ++k) {
                int idx = start + k;
                rec->push_back(SStep{ACT_HIGHLIGHT, idx, -1, 0});
            }
        }
    }
};

class CQuickSorter : public CSorter {
public:
    CQuickSorter(int input[], int n): CSorter(input, n) {}

    void quickSort(SStepBuffer* rec = nullptr) {
        quickSortRecursive(data, 0, size - 1, rec);
    }
private:
    void static quickSortRecursive(int array[], int start, int end, SStepBuffer* rec) {
        if (start >= end) return;

        int pivot = array[end];

        int i = start - 1;
        for (int j = start; j < end; ++j) {

            if (rec) rec->push_back(SStep{ACT_COMPARE, j,end, 0});

            if (array[j] < pivot) {
                ++i;
                if (i != j) {
                    if (rec) rec->push_back(SStep{ACT_SWAP,i,j,0});
                }
                std::swap(array[j], array[i]);
            }
        }

        if (i+1 != end) {
            if (rec) rec ->push_back(SStep{ACT_SWAP, i+1, end, 0});
            std::swap(array[i+1],array[end]);
        }

        if (rec) rec-> push_back((SStep{ACT_HIGHLIGHT, i+1,-1,0}));


        quickSortRecursive(array, start, i, rec);
        quickSortRecursive(array, i+2, end, rec);
    }
};

// -----------------------------
// Visualizer classes
// -----------------------------
class CBar {
public:
    int value;
    sf::RectangleShape shape;
    CBar(): value(0) {}
    CBar(int v, float x, float y, float width): value(v) {
        shape.setSize(sf::Vector2f(width, static_cast<float>(v)));
        shape.setFillColor(sf::Color::Yellow);
        shape.setOutlineThickness(0.f);
        shape.setPosition(x, y - static_cast<float>(v));
    }
    void highlight(const sf::Color &c1, const sf::Color &c2, float thickness) {
        shape.setFillColor(c1);
        shape.setOutlineColor(c2);
        shape.setOutlineThickness(thickness);
    }
};

class CSortingVisualizer {
private:
    CBar* bars;
    int size;
    float barWidth;
    float baseY;

public:
    CSortingVisualizer(const int* values, int n, float windowWidth, float windowHeight)
        : bars(nullptr), size(n), barWidth(0.f), baseY(windowHeight - 50.0f) {
        if (n <= 0) {
            size = 0;
            bars = nullptr;
            return;
        }
        barWidth = (windowWidth - 2.0f * LATERAL_MARGIN) / static_cast<float>(n);
        bars = new CBar[n];
        for (int i = 0; i < n; ++i) {
            float x = LATERAL_MARGIN + static_cast<float>(i) * barWidth;
            bars[i] = CBar(values[i], x, baseY, barWidth - SPACE_BETWEEN_BARS);
        }
    }

    ~CSortingVisualizer() { delete[] bars; }

    //I made them non-copyable and non-movable to prevent issues
    CSortingVisualizer(const CSortingVisualizer&) = delete;
    CSortingVisualizer& operator=(const CSortingVisualizer&) = delete;
    CSortingVisualizer(CSortingVisualizer&&) = delete;
    CSortingVisualizer& operator=(CSortingVisualizer&&) = delete;

    void drawBars(sf::RenderWindow& window) const {
        if (!bars) return;
        for (int i = 0; i < size; ++i) window.draw(bars[i].shape);
    }

    void highlight(int index, const sf::Color &c1, const sf::Color &c2, float thickness) const  {
        if (!bars || index < 0 || index >= size) return;
        bars[index].highlight(c1, c2, thickness);
    }

    [[nodiscard]] float getBarX(int index) const {
        if (!bars || index < 0 || index >= size) return 0.0f;
        return bars[index].shape.getPosition().x;
    }

    void setBarX(int index, float x) const {
        if (!bars || index < 0 || index >= size) return;
        bars[index].shape.setPosition(x, baseY - static_cast<float>(bars[index].value));
    }

    void finalizeSwap(int i, int j) const {
        if (!bars || i < 0 || j < 0 || i >= size || j >= size) return;
        CBar tmp = bars[i];
        bars[i] = bars[j];
        bars[j] = tmp;
        float xi = LATERAL_MARGIN + static_cast<float>(i) * barWidth;
        float xj = LATERAL_MARGIN + static_cast<float>(j) * barWidth;
        bars[i].shape.setPosition(xi, baseY - static_cast<float>(bars[i].value));
        bars[j].shape.setPosition(xj, baseY - static_cast<float>(bars[j].value));
    }

    void overwriteValue(int index, int value) const{
        if (!bars || index < 0 || index >= size) return;
        bars[index].value = value;
        bars[index].shape.setSize(sf::Vector2f(barWidth - SPACE_BETWEEN_BARS, static_cast<float>(value)));
        float x = LATERAL_MARGIN + static_cast<float>(index) * barWidth;
        bars[index].shape.setPosition(x, baseY - static_cast<float>(value));
    }

    [[nodiscard]] int getSize() const { return size; }
};

// -----------------------------
// Application
// -----------------------------
enum appState { Welcome, Menu, Sorting };

class CApp {
private:
    sf::RenderWindow window;
    appState currentState;
    sf::Font font;

    // welcome
    sf::RectangleShape startButton;
    sf::Text startText;

    // menu
    static constexpr int NUM_METHODS = 5;
    std::string methods[NUM_METHODS] = {
        "Insertion Sort", "Selection Sort",
        "Quick Sort", "Merge Sort", "Heap Sort"
    };
    sf::RectangleShape buttons[NUM_METHODS];
    sf::Text texts[NUM_METHODS];

    int methodSelected = -1;

    SStepBuffer steps;
    int playIndex = 0;
    float stepInterval = 0.08f;
    sf::Clock stepClock;
    bool recorded = false;

    struct SwapAnim {
        bool active = false;
        int i = -1, j = -1;
        float startXi = 0.0f, startXj = 0.0f;
        float duration = 0.25f;
        sf::Clock clock;

        float accElapsed = 0.0f;
    } swapAnim;

    struct HighlightEntry {
        int idx = -1;
        float duration = 0.0f;
        sf::Clock clock;

        float accElapsed = 0.0f;
    };
    HighlightEntry highlights[128];
    int highlightsCount = 0;


    struct CompletionAnim {
        bool active = false;
        int currentIndex = 0;
        float highlightDuration = 0.08f;
        sf::Clock clock;

        float accElapsed = 0.0f;
    } completionAnim;

    // Improved colors and timing
    float compareDuration = 0.15f;
    float swapDuration = 0.05f;
    sf::Color compareColorA = sf::Color(255, 100, 100);
    sf::Color compareColorB = sf::Color(100, 150, 255);
    sf::Color defaultBarColor = sf::Color::Yellow;
    sf::Color sortedColor = sf::Color(80, 200, 80);
    sf::Color finalGreenColor = sf::Color(50, 150, 50);

    std::unique_ptr<CSortingVisualizer> visual;
    int currentArrayValues[CONST_MAX_LENGTH]{};
    int currentN = 0;

    //used for random number gen
    std::random_device rd;
    std::mt19937 gen;

    //for handle menu event ux
    int menuCursor = 0;

    //for handling speed of animations
    bool paused = false;

    float appSpeed = 1.0f;
    float stepAccElapsed = 0.0f;

    float baseStepInterval = 0.08f;
    float baseSwapDuration = 0.25f;
    float baseCompareDuration = 0.15f;
    float completionHighlightDuration = 0.08f;


public:
    CApp() : window(sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), "DSA"),
             currentState(Welcome),
             gen(rd()),menuCursor(0) {
        centerWindow();
        if (!font.loadFromFile("DejaVuSans.ttf")) {
            std::cerr << "Could not load font! (place DejaVuSans.ttf next to exe)\n";
            //continue anyway
        }

        startButton.setSize(sf::Vector2f(200, 60));
        startButton.setFillColor(sf::Color::Yellow);
        startButton.setOutlineThickness(6);
        startButton.setOutlineColor(sf::Color::Cyan);
        startButton.setPosition((WINDOW_WIDTH - 200) / 2.0f, (WINDOW_HEIGHT - 60) / 2.0f);

        startText.setFont(font);
        startText.setString("START");
        startText.setCharacterSize(24);
        startText.setFillColor(DARK_BLUE);
        centerText(startText, startButton);

        for (int i = 0; i < NUM_METHODS; ++i) {
            float buttonWidth = 300;
            float buttonHeight = 50;
            float spacing = 20;
            float startY = 160;

            buttons[i].setSize(sf::Vector2f(buttonWidth, buttonHeight));
            buttons[i].setFillColor(sf::Color::Yellow);
            buttons[i].setOutlineColor(sf::Color::Cyan);
            buttons[i].setOutlineThickness(2);
            buttons[i].setPosition(
                static_cast<float>(window.getSize().x) / 2.0f - buttonWidth / 2.0f,
                startY + static_cast<float>(i) * (buttonHeight + spacing)
            );

            texts[i].setFont(font);
            texts[i].setString(methods[i]);
            texts[i].setCharacterSize(24);
            texts[i].setFillColor(sf::Color::Black);
            centerText(texts[i], buttons[i]);
        }

        updateDurations();
    }

    void centerWindow() {
        sf::VideoMode desktop = sf::VideoMode::getDesktopMode();
        int posX = (static_cast<int>(desktop.width) - WINDOW_WIDTH) / 2;
        int posY = (static_cast<int>(desktop.height) - WINDOW_HEIGHT) / 2;
        window.setPosition(sf::Vector2i(posX, posY));
    }

    static void centerText(sf::Text &text, const sf::RectangleShape &rect) {
        sf::FloatRect textBounds = text.getLocalBounds();
        text.setOrigin(textBounds.left + textBounds.width / 2.0f,
                       textBounds.top + textBounds.height / 2.0f);
        sf::FloatRect rectBounds = rect.getGlobalBounds();
        text.setPosition(rectBounds.left + rectBounds.width / 2.0f,
                         rectBounds.top + rectBounds.height / 2.0f);
    }

    void run() {
        while (window.isOpen()) {
            sf::Event event{};
            while (window.pollEvent(event)) handleEvent(event);

            if (currentState == Sorting) updateSorting();

            render();
        }
    }

    void handleEvent(const sf::Event& event) {
        if (event.type == sf::Event::Closed ||
            (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Escape)) {
            window.close();
        }

        if (currentState == Welcome) {
            handleWelcomeEvent(event);
        } else if (currentState == Menu) {
            handleMenuEvent(event);
        } else if (currentState == Sorting) {
            handleSortingEvent(event);
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

private:
    void handleWelcomeEvent(const sf::Event& event) {
        if (event.type == sf::Event::KeyPressed &&
            event.key.code == sf::Keyboard::Enter) {
            currentState = Menu;
        }
        if (event.type == sf::Event::MouseButtonPressed &&
            event.mouseButton.button == sf::Mouse::Left) {
            sf::Vector2f mouseWorldPos = window.mapPixelToCoords(sf::Mouse::getPosition(window));
            if (startButton.getGlobalBounds().contains(mouseWorldPos)) {
                currentState = Menu;
            }
        }
    }
    void updateMenuColors() {
        for (int i = 0; i < NUM_METHODS; i++) {
            if (i == menuCursor) {
                buttons[i].setFillColor(sf::Color::Cyan);
                texts[i].setFillColor(sf::Color::Blue);
            } else {
                buttons[i].setFillColor(sf::Color::Yellow);
                texts[i].setFillColor(sf::Color::Black);
            }
        }
    }



    void handleMenuEvent(const sf::Event& event) {

        //adding interaction for up down keys and enter also
        if (event.type == sf::Event::KeyPressed) {
            if (event.key.code == sf::Keyboard::Down) {
                menuCursor = (menuCursor + 1) % NUM_METHODS;
                updateMenuColors();
            } else if (event.key.code == sf::Keyboard::Up) {
                menuCursor = (menuCursor - 1 + NUM_METHODS) % NUM_METHODS;

                updateMenuColors();
            } else if (event.key.code == sf::Keyboard::Enter) {
                methodSelected = menuCursor;
                prepareSorting(methodSelected);
                currentState = Sorting;
                return;
            } else if (event.key.code == sf::Keyboard::B) {
                currentState=Welcome;
                menuCursor = 0;
                return;
            }
        }


        //mouse interaction
        sf::Vector2f mouseWorldPos = window.mapPixelToCoords(sf::Mouse::getPosition(window));
        //bool anyHover = false;
        for (int i = 0; i < NUM_METHODS; ++i) {
            if (buttons[i].getGlobalBounds().contains(mouseWorldPos)) {

                //anyHover = true;
                menuCursor = i;


                buttons[i].setFillColor(sf::Color::Cyan);
                texts[i].setFillColor(sf::Color::Blue);
                if (event.type == sf::Event::MouseButtonPressed &&
                    event.mouseButton.button == sf::Mouse::Left) {
                    methodSelected = i;
                    prepareSorting(methodSelected);
                    currentState = Sorting;
                    }
            } if (i == menuCursor ){//&& !anyHover) {
                //highlight button
                buttons[i].setFillColor(sf::Color::Cyan);
                texts[i].setFillColor(sf::Color::Blue);
            } else {
                //unhighlight
                buttons[i].setFillColor(sf::Color::Yellow);
                texts[i].setFillColor(sf::Color::Black);
            }
            }
        }

    void handleSortingEvent(const sf::Event& event) {
        if (event.type == sf::Event::KeyPressed) {
            if (event.key.code == sf::Keyboard::B) {
                cleanupSorting();
                currentState = Menu;
                return;
            }


            if (event.key.code == sf::Keyboard::Right) {
                float oldSpeed = appSpeed;
                appSpeed += 0.1f;
                if (appSpeed > 5.0f) appSpeed += 0.9f;
                if (appSpeed > 10.0f) appSpeed += 1.0f;
                if (appSpeed > 30.0f) appSpeed = 30.0f;
                updateDurationsAndAdjustRunning(oldSpeed);
            } else if (event.key.code == sf::Keyboard::Left) {
                float oldSpeed = appSpeed;
                appSpeed -= 0.1f;
                if (appSpeed < 30.0f) appSpeed -=1.9f;
                if (appSpeed < 10.0f) appSpeed +=1.0f;
                if (appSpeed < 4.0f) appSpeed +=0.9f;
                if (appSpeed < 0.1f) appSpeed = 0.1f;
                updateDurationsAndAdjustRunning(oldSpeed);
            } else if (event.key.code == sf::Keyboard::Space
                || event.key.code == sf::Keyboard::P) {
                togglePause();
                return;
            }



        }





    }

    void renderWelcome() {
        sf::Text title("Sorting animation by Andrei Cristea", font, 30);
        title.setFillColor(sf::Color::Yellow);
        title.setPosition(100, 100);
        window.draw(title);

        renderStartButton();
    }

    void renderStartButton() {
        sf::Vector2f mouseWorldPos = window.mapPixelToCoords(sf::Mouse::getPosition(window));
        sf::RectangleShape button = startButton;
        sf::Text text = startText;

        if (button.getGlobalBounds().contains(mouseWorldPos)) {
            button.setFillColor(sf::Color::Cyan);
            button.setOutlineColor(sf::Color::Yellow);
            text.setFillColor(sf::Color::Blue);
            text.setCharacterSize(40);
            centerText(text, button);
        } else {
            button.setFillColor(sf::Color::Yellow);
            button.setOutlineColor(sf::Color::Cyan);
            text.setFillColor(sf::Color::Black);
            text.setCharacterSize(24);
            centerText(text, button);
        }
        window.draw(button);
        window.draw(text);
    }

    void renderMenu() {
        sf::Text title("Choose the sorting algorithm:", font, 30);
        title.setFillColor(sf::Color::Yellow);
        sf::FloatRect titleBounds = title.getLocalBounds();
        title.setOrigin(titleBounds.left + titleBounds.width / 2.0f,
                        titleBounds.top + titleBounds.height / 2.0f);
        title.setPosition(static_cast<float>(window.getSize().x) / 2.0f, 80.0f);

        sf::Text goBackText("Press \"B\" for going back" , font, 18);
        goBackText.setFillColor(sf::Color::White);
        goBackText.setPosition(287,510);

        window.draw(title);
        for (int i = 0; i < NUM_METHODS; ++i) {
            window.draw(buttons[i]);
            window.draw(texts[i]);
        }



        window.draw(goBackText);

    }

    void renderSorting() {
        sf::Text title(methods[methodSelected], font, 42);
        title.setFillColor(sf::Color::White);
        title.setPosition(20, 20);


        sf::Text subtitle("Sorting in progress...\nPress \"b\" for going back to menu" , font, 18 );
        subtitle.setFillColor(sf::Color::White);
        subtitle.setPosition(450,20);


        float speedDisplay = appSpeed;
        std::ostringstream ss;
        ss << "Speed x";

        if (std::fabs(speedDisplay - std::round(speedDisplay)) < 0.005f) {
            ss << static_cast<int>(std::lround(speedDisplay));
        } else {
            ss << std::fixed << std::setprecision(1) << speedDisplay;
        }

        if (paused) ss << " (paused)";

        std::string speedString =ss.str();



        sf::Text speedText(speedString , font, 16);
        speedText.setFillColor(sf::Color::White);
        speedText.setPosition(20,70);


        window.draw(title);
        window.draw(subtitle);
        if (visual) {
            visual->drawBars(window);
        }
        window.draw(speedText);
    }

    static float linearInterpolate(float a, float b, float t) {
        return a + (b - a) * t;
    }


    static float easeInOutCubic(float t) {
        if (t < 0.5f) return 4.0f * t * t * t;
        t = t - 1.0f;
        return 1.0f + 4.0f * t * t * t;
    }

    void startHighlight(int idx) {
        if (highlightsCount < 128) {
            highlights[highlightsCount] = HighlightEntry{idx, compareDuration, sf::Clock(), 0.0f};
            highlights[highlightsCount].clock.restart();
            highlights[highlightsCount].accElapsed = 0.0f;
            ++highlightsCount;
        } else {
            highlights[127] = HighlightEntry{idx, compareDuration, sf::Clock(), 0.0f};
            highlights[127].clock.restart();
            highlights[127].accElapsed = 0.0f;
        }
    }

    void updateHighlights() {
        for (int k = 0; k < highlightsCount; ++k) {
            float elapsed = highlights[k].accElapsed + highlights[k].clock.getElapsedTime().asSeconds();
            if (elapsed >= highlights[k].duration) {
                if (visual) {
                    visual->highlight(highlights[k].idx, defaultBarColor, sf::Color::Transparent, 0.0f);
                }
                highlights[k] = highlights[--highlightsCount];
                --k;
            }
        }
    }

    // Updated applyStepImmediate for better visual feedback
    void applyStepImmediate(const SStep &s) {
        if (!visual) return;

        switch (s.kind) {
            case ACT_COMPARE:
                for (int k = 0; k < visual->getSize(); ++k) {
                    if (k != s.i && k != s.j) {
                        visual->highlight(k, defaultBarColor, sf::Color::Transparent, 0.0f);
                    }
                }

                if (s.i >= 0) {
                    visual->highlight(s.i, compareColorA, sf::Color::White, 3.0f);
                    startHighlight(s.i);
                }
                if (s.j >= 0) {
                    visual->highlight(s.j, compareColorB, sf::Color::White, 3.0f);
                    startHighlight(s.j);
                }
                break;
            case ACT_HIGHLIGHT:
                if (s.i >= 0) {
                    visual->highlight(s.i, sortedColor, sf::Color::White, 2.0f);
                }
                break;
            case ACT_OVERWRITE:
                visual->overwriteValue(s.i, s.value);
                break;
            case ACT_SWAP:
                // swaps handled in animation block
                break;
        }
    }


    void updateSorting() {
        if (!visual) return;

        if (paused) return;

        updateHighlights();


        if (completionAnim.active) {
            updateCompletionAnimation();
            return;
        }


        if (swapAnim.active) {
            updateSwapAnimation();
            return;
        }
        float elapsedStep = stepAccElapsed + stepClock.getElapsedTime().asSeconds();
        if (playIndex >= steps.size) {
            if (recorded) {
                startCompletionAnimation();
            }
            return;
        }


        if (elapsedStep < stepInterval) return;

        SStep s = steps.data[playIndex];

        if (s.kind == ACT_SWAP) {
            startSwapAnimation(s);
        } else {
            applyStepImmediate(s);
        }

        ++playIndex;

        stepAccElapsed = 0.0f;
        stepClock.restart();
    }


    void startCompletionAnimation() {
        completionAnim.active = true;
        completionAnim.currentIndex = 0;
        completionAnim.accElapsed = 0.0f;
        completionAnim.clock.restart();

        // Clear all highlights first
        for (int i = 0; i < visual->getSize(); ++i) {
            visual->highlight(i, defaultBarColor, sf::Color::Transparent, 0.0f);
        }
    }

    void updateCompletionAnimation() {
        if (!visual) return;

        float elapsed = completionAnim.accElapsed + completionAnim.clock.getElapsedTime().asSeconds();
        if (elapsed >= completionAnim.highlightDuration) {

            if (completionAnim.currentIndex < visual->getSize()) {
                visual->highlight(completionAnim.currentIndex, finalGreenColor, sf::Color::White, 2.0f);
                if (completionAnim.currentIndex > 0) {
                    visual->highlight(completionAnim.currentIndex - 1, sortedColor, sf::Color::Transparent, 0.0f);
                }
                completionAnim.currentIndex++;

                completionAnim.accElapsed = 0.0f;
                completionAnim.clock.restart();
            } else {
                for (int i = 0; i < visual->getSize(); ++i) {
                    visual->highlight(i, sortedColor, sf::Color::Transparent, 0.0f);
                }
                completionAnim.active = false;
                completionAnim.accElapsed = 0.0f;
            }
        }
    }

    void updateSwapAnimation() {
        if (!visual) return;

        float elapsed = swapAnim.accElapsed + swapAnim.clock.getElapsedTime().asSeconds();
        float t = elapsed / swapAnim.duration;
        if (t >= 1.0f) {
            visual->finalizeSwap(swapAnim.i, swapAnim.j);
            swapAnim.active = false;
            swapAnim.accElapsed = 0.0f;
        } else {
            float e = easeInOutCubic(t);
            float xi = linearInterpolate(swapAnim.startXi, swapAnim.startXj, e);
            float xj = linearInterpolate(swapAnim.startXj, swapAnim.startXi, e);
            visual->setBarX(swapAnim.i, xi);
            visual->setBarX(swapAnim.j, xj);
        }
    }

    void startSwapAnimation(const SStep& s) {
        if (!visual) return;

        swapAnim.active = true;
        swapAnim.i = s.i;
        swapAnim.j = s.j;
        swapAnim.startXi = visual->getBarX(s.i);
        swapAnim.startXj = visual->getBarX(s.j);
        swapAnim.duration = swapDuration;
        swapAnim.accElapsed = 0.0f;
        swapAnim.clock.restart();
    }

    void prepareSorting(int method) {
        currentN = NUMBER_OF_COLUMNS;  // Increased for better visual effect
        std::uniform_int_distribution<> dis(20, 419);
        for (int k = 0; k < currentN; ++k) {
            currentArrayValues[k] = dis(gen);
        }

        steps.clear();

        CSorter s(currentArrayValues, currentN);

        switch (method) {
            case 0: {
                // Insertion Sort
                s.insertionSort(&steps);
                recorded = true;
                break;
            }
            case 1: {
                // Selection Sort
                s.selectionSort(&steps);
                recorded = true;
                break;
            }
            case 2: {
                // Quick Sort
                CQuickSorter qs(currentArrayValues, currentN);
                qs.quickSort(&steps);
                recorded = true;
                break;
            }
            case 3: {
                // Merge Sort
                CMergeSorter ms(currentArrayValues, currentN);
                ms.mergeSort(&steps);
                recorded = true;
                break;
            }
            case 4: {
                // Heap Sort
                s.heapSort();
                recorded = false;
                break;
            }
            default: {
                s.selectionSort(&steps);
                recorded = true;
                break;
            }
        }

        visual = std::make_unique<CSortingVisualizer>(
            currentArrayValues,
            currentN,
            static_cast<float>(WINDOW_WIDTH),
            static_cast<float>(WINDOW_HEIGHT)
        );

        playIndex = 0;
        stepClock.restart();
        swapAnim.active = false;
        completionAnim.active = false;
        highlightsCount = 0;
    }

    void cleanupSorting() {
        visual.reset();
        steps.clear();
        playIndex = 0;
        recorded = false;
        swapAnim.active = false;
        completionAnim.active = false;
        highlightsCount = 0;
    }

    void updateDurations() {
        stepInterval = baseStepInterval / appSpeed;
        swapDuration = baseSwapDuration / appSpeed;
        compareDuration = baseCompareDuration / appSpeed;

        completionAnim.highlightDuration = completionHighlightDuration / appSpeed;
    }

    void togglePause() {
        paused = !paused;
        if (paused) {
            if (swapAnim.active) swapAnim.accElapsed += swapAnim.clock.getElapsedTime().asSeconds();
            if (completionAnim.active) completionAnim.accElapsed += completionAnim.clock.getElapsedTime().asSeconds();
            for (int k = 0; k < highlightsCount; ++k) {
                highlights[k].accElapsed += highlights[k].clock.getElapsedTime().asSeconds();
            }
            stepAccElapsed += stepClock.getElapsedTime().asSeconds();
        } else {
            if (swapAnim.active) swapAnim.clock.restart();
            if (completionAnim.active) completionAnim.clock.restart();
            for (int k = 0; k < highlightsCount; ++k) {
                highlights[k].clock.restart();
            }
            stepClock.restart();
        }
    }

    void updateDurationsAndAdjustRunning(float oldSpeed) {

    float oldStepInterval = baseStepInterval / oldSpeed;
    float oldSwapDuration = baseSwapDuration / oldSpeed;
    float oldCompareDuration = baseCompareDuration / oldSpeed;
    float oldCompletionDuration = completionHighlightDuration / oldSpeed;


    stepInterval = baseStepInterval / appSpeed;
    float newSwapDuration = baseSwapDuration / appSpeed;
    float newCompareDuration = baseCompareDuration / appSpeed;
    float newCompletionDuration = completionHighlightDuration / appSpeed;

    if (swapAnim.active) {
        float elapsed = swapAnim.accElapsed + swapAnim.clock.getElapsedTime().asSeconds();
        float p = (oldSwapDuration > 0.0f) ? (elapsed / oldSwapDuration) : 0.0f;
        if (p < 0.0f) p = 0.0f; if (p > 1.0f) p = 1.0f;
        swapAnim.accElapsed = p * newSwapDuration;
        swapAnim.duration = newSwapDuration;
        swapAnim.clock.restart();
    }


    if (completionAnim.active) {
        float elapsed = completionAnim.accElapsed + completionAnim.clock.getElapsedTime().asSeconds();
        float p = (oldCompletionDuration > 0.0f) ? (elapsed / oldCompletionDuration) : 0.0f;
        if (p < 0.0f) p = 0.0f; if (p > 1.0f) p = 1.0f;
        completionAnim.accElapsed = p * newCompletionDuration;
        completionAnim.highlightDuration = newCompletionDuration;
        completionAnim.clock.restart();
    }

    for (int k = 0; k < highlightsCount; ++k) {
        float elapsed = highlights[k].accElapsed + highlights[k].clock.getElapsedTime().asSeconds();
        float p = (oldCompareDuration > 0.0f) ? (elapsed / oldCompareDuration) : 0.0f;
        if (p < 0.0f) p = 0.0f; if (p > 1.0f) p = 1.0f;
        highlights[k].accElapsed = p * newCompareDuration;
        highlights[k].duration = newCompareDuration;
        highlights[k].clock.restart();
    }

    {
        float elapsed = stepAccElapsed + stepClock.getElapsedTime().asSeconds();
        float p = (oldStepInterval > 0.0f) ? (elapsed / oldStepInterval) : 0.0f;
        if (p < 0.0f) p = 0.0f; if (p > 1.0f) p = 1.0f;
        stepAccElapsed = p * stepInterval;
        stepClock.restart();
    }

    swapDuration = newSwapDuration;
    compareDuration = newCompareDuration;
    completionAnim.highlightDuration = newCompletionDuration;
}

};

// -----------------------------
// main
// -----------------------------
int main() {
    CApp app;
    app.run();
    return 0;
}