#include <iostream>
#include <ctime>
#include <random>
#include <climits>
#include <memory>

#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>

sf::Color DARK_BLUE(10, 10, 60);

constexpr int WINDOW_WIDTH = 800;
constexpr int WINDOW_HEIGHT = 600;

constexpr float LATERAL_MARGIN = 50.0f;
constexpr float SPACE_BETWEEN_BARS = 2.0f;
constexpr int CONST_MAX_LENGTH = 100;

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


    void mergeSort() override { mergeSortHelper(data, size); }
private:
    void static mergeSortHelper(int array[], int length) {
        if (length <= 1) return;
        int middle = length / 2;
        int leftArray[CONST_MAX_LENGTH];
        int rightArray[CONST_MAX_LENGTH];
        int leftSize = middle, rightSize = length - middle;
        int i = 0, j = 0;
        for (; i < length; ++i) {
            if (i < middle) leftArray[i] = array[i];
            else { rightArray[j] = array[i]; ++j; }
        }
        mergeSortHelper(leftArray, leftSize);
        mergeSortHelper(rightArray, rightSize);
        merge(leftArray, leftSize, rightArray, rightSize, array);
    }

    static void merge(const int leftArray[], int leftSize, const int rightArray[], int rightSize, int array[]) {
        int i = 0, l = 0, r = 0;
        while (l < leftSize && r < rightSize) {
            if (leftArray[l] < rightArray[r]) array[i++] = leftArray[l++];
            else array[i++] = rightArray[r++];
        }
        while (l < leftSize) array[i++] = leftArray[l++];
        while (r < rightSize) array[i++] = rightArray[r++];
    }
};

class CQuickSorter : public CSorter {
public:
    CQuickSorter(int input[], int n): CSorter(input, n) {}

    void quickSort() override { quickSortRecursive(data, 0, size - 1); }
private:
    void static quickSortRecursive(int array[], int start, int end) {
        if (start >= end) return;
        int i = start - 1;
        for (int j = start; j < end; ++j) {
            if (array[j] < array[end]) std::swap(array[j], array[++i]);
        }
        std::swap(array[end], array[i+1]);
        quickSortRecursive(array, start, i);
        quickSortRecursive(array, i+2, end);
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
    } swapAnim;

    struct HighlightEntry {
        int idx = -1;
        float duration = 0.0f;
        sf::Clock clock;
    };
    HighlightEntry highlights[128];
    int highlightsCount = 0;


    struct CompletionAnim {
        bool active = false;
        int currentIndex = 0;
        float highlightDuration = 0.08f;
        sf::Clock clock;
    } completionAnim;

    // Improved colors and timing
    static constexpr float compareDuration = 0.15f;
    float swapDuration = 0.25f;
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

public:
    CApp() : window(sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), "DSA"),
             currentState(Welcome),
             gen(rd()) {
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

    void handleMenuEvent(const sf::Event& event) {
        sf::Vector2f mouseWorldPos = window.mapPixelToCoords(sf::Mouse::getPosition(window));
        for (int i = 0; i < NUM_METHODS; ++i) {
            if (buttons[i].getGlobalBounds().contains(mouseWorldPos)) {
                buttons[i].setFillColor(sf::Color::Cyan);
                texts[i].setFillColor(sf::Color::Blue);
                if (event.type == sf::Event::MouseButtonPressed &&
                    event.mouseButton.button == sf::Mouse::Left) {
                    methodSelected = i;
                    prepareSorting(methodSelected);
                    currentState = Sorting;
                }
            } else {
                buttons[i].setFillColor(sf::Color::Yellow);
                texts[i].setFillColor(sf::Color::Black);
            }
        }
    }

    void handleSortingEvent(const sf::Event& event) {
        if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::B) {
            cleanupSorting();
            currentState = Menu;
        }
        //TODO implement pause and speed of sorting adjustments
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
        window.draw(title);
        for (int i = 0; i < NUM_METHODS; ++i) {
            window.draw(buttons[i]);
            window.draw(texts[i]);
        }
    }

    void renderSorting() {
        sf::Text title(methods[methodSelected], font, 42);
        title.setFillColor(sf::Color::White);
        title.setPosition(20, 20);


        sf::Text subtitle("Sorting in progress...\nPress \"b\" for going back to menu" , font, 18 );
        subtitle.setFillColor(sf::Color::White);
        subtitle.setPosition(450,20);


        window.draw(title);
        window.draw(subtitle);
        if (visual) {
            visual->drawBars(window);
        }
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
            highlights[highlightsCount++] = HighlightEntry{idx, compareDuration, sf::Clock()};
        } else {
            highlights[127] = HighlightEntry{idx, compareDuration, sf::Clock()};
        }
    }

    void updateHighlights() {
        for (int k = 0; k < highlightsCount; ++k) {
            if (highlights[k].clock.getElapsedTime().asSeconds() >= highlights[k].duration) {
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

        updateHighlights();


        if (completionAnim.active) {
            updateCompletionAnimation();
            return;
        }


        if (swapAnim.active) {
            updateSwapAnimation();
            return;
        }

        if (playIndex >= steps.size) {
            if (!completionAnim.active && recorded) {
                startCompletionAnimation();

            }
            return;
        }

        if (stepClock.getElapsedTime().asSeconds() < stepInterval) return;

        SStep s = steps.data[playIndex];

        if (s.kind == ACT_SWAP) {
            startSwapAnimation(s);
        } else {
            applyStepImmediate(s);
        }

        ++playIndex;
        stepClock.restart();
    }


    void startCompletionAnimation() {
        completionAnim.active = true;
        completionAnim.currentIndex = 0;
        completionAnim.clock.restart();

        // Clear all highlights first
        for (int i = 0; i < visual->getSize(); ++i) {
            visual->highlight(i, defaultBarColor, sf::Color::Transparent, 0.0f);
        }
    }

    void updateCompletionAnimation() {
        if (!visual) return;

        if (completionAnim.clock.getElapsedTime().asSeconds() >= completionAnim.highlightDuration) {
            if (completionAnim.currentIndex < visual->getSize()) {

                visual->highlight(completionAnim.currentIndex, finalGreenColor, sf::Color::White, 2.0f);


                if (completionAnim.currentIndex > 0) {
                    visual->highlight(completionAnim.currentIndex - 1, sortedColor, sf::Color::Transparent, 0.0f);
                }

                completionAnim.currentIndex++;
                completionAnim.clock.restart();
            } else {

                for (int i = 0; i < visual->getSize(); ++i) {
                    visual->highlight(i, sortedColor, sf::Color::Transparent, 0.0f);
                }
                completionAnim.active = false;
            }
        }
    }

    // Updated updateSwapAnimation with better easing
    void updateSwapAnimation() {
        if (!visual) return;

        float t = swapAnim.clock.getElapsedTime().asSeconds() / swapAnim.duration;
        if (t >= 1.0f) {
            visual->finalizeSwap(swapAnim.i, swapAnim.j);
            swapAnim.active = false;
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
        swapAnim.clock.restart();
    }

    void prepareSorting(int method) {
        currentN = 10;  // Increased for better visual effect
        std::uniform_int_distribution<> dis(20, 419);
        for (int k = 0; k < currentN; ++k) {
            currentArrayValues[k] = dis(gen);
        }

        steps.clear();

        CSorter s(currentArrayValues, currentN);

        switch (method) {
            case 0: // Insertion Sort
                s.insertionSort(&steps);
                recorded = true;
                break;
            case 1: // Selection Sort
                s.selectionSort(&steps);
                recorded = true;
                break;
            case 2: // Quick Sort
                s.quickSort();
                recorded = false;
                break;
            case 3: // Merge Sort
                s.mergeSort();
                recorded = false;
                break;
            case 4: // Heap Sort
                s.heapSort();
                recorded = false;
                break;
            default:
                s.selectionSort(&steps);
                recorded = true;
                break;
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
};

// -----------------------------
// main
// -----------------------------
int main() {
    CApp app;
    app.run();
    return 0;
}