////////////////////////////////////////////////////////////
//
// TUGAS BESAR ALGORITMA & PEMROGRAMAN
// NIM : 2505551088 & 2505551163
//
////////////////////////////////////////////////////////////

#include <iostream>
#include <vector>
#include <queue>
#include <stack>
#include <algorithm>
#include <random>
#include <SFML/Graphics.hpp>

// 1. CONFIG
#define GRID_WIDTH 64
#define GRID_HEIGHT 32
// CELL_SIZE dihapus dari define karena akan dihitung dinamis agar pas di layar full

// 2. STRUKTUR DATA
class Class2D {
    public:
    enum class Directions { UP, DOWN, LEFT, RIGHT, COUNT };
    struct Coords { int x; int y; };

    static void shiftCoords(Coords* pos, Directions d, int units = 1){
        switch(d){
        case Directions::UP: pos->y -= units; break;
        case Directions::DOWN: pos->y += units; break;
        case Directions::LEFT: pos->x -= units; break;
        case Directions::RIGHT: pos->x += units; break;
        default: break;
        }
    }

    static Directions direction(int dir){ return static_cast<Directions>(dir); }
};

class Node {
    public:
    int dt;
    enum class Flags { UNVISITED, VISITING, VISITED, PATH };

    private:
    Node* parent;
    Node* neighbors[4]; 
    Flags status;

    public:
    Node() : neighbors{nullptr}, status(Flags::UNVISITED), dt(0), parent(nullptr) {}

    void reset() {
        status = Flags::UNVISITED;
        parent = nullptr;
        dt = 0;
        for(int i=0; i<4; i++) neighbors[i] = nullptr;
    }

    Flags getStatus(){ return status; }
    void setStatus(Flags flag){ status = flag; }
    Node* getParent(){ return parent; }
    void setParent(Node* p){ parent = p; } 
    
    Node* neighbor(Class2D::Directions d){
        int direction = static_cast<int>(d);
        return neighbors[direction];
    }

    int append(Class2D::Directions d, Node* tail){
        int direction = static_cast<int>(d);
        if (tail == nullptr || tail == this) return 0;
        if (neighbors[direction] != nullptr) return 0;

        switch(d){
        case Class2D::Directions::UP:
            neighbors[0] = tail; tail->neighbors[1] = this; break;
        case Class2D::Directions::DOWN:
            neighbors[1] = tail; tail->neighbors[0] = this; break;
        case Class2D::Directions::LEFT:
            neighbors[2] = tail; tail->neighbors[3] = this; break;
        case Class2D::Directions::RIGHT:
            neighbors[3] = tail; tail->neighbors[2] = this; break;
        default: return 0;
        }
        tail->parent = this;
        return 1;
    }
};

class Grid {
    public: 
    Node GridMap[GRID_HEIGHT][GRID_WIDTH];
    Class2D::Coords cursor;

    public:
    Grid() { cursor = {0, 0}; }
    
    void resetGraph() {
        for (int i = 0; i < GRID_WIDTH; i++){
            for(int j = 0; j < GRID_HEIGHT; j++){
                GridMap[j][i].reset();
            }
        }
    }

    void clearFlags(Node::Flags f = Node::Flags::UNVISITED){
        for (int i = 0; i < GRID_WIDTH; i++){
            for(int j = 0; j < GRID_HEIGHT; j++){
                if(GridMap[j][i].getStatus() != Node::Flags::UNVISITED)
                    GridMap[j][i].setStatus(f);
                GridMap[j][i].dt = 0; 
            }
        }
    }
    int isOutOfBounds(int x, int y){
        return (x < 0 || y < 0 || x >= GRID_WIDTH || y >= GRID_HEIGHT);
    }

    Class2D::Coords getRelativePosition(int x, int y, Class2D::Directions d){ 
        Class2D::Coords pos = {x, y};
        Class2D::shiftCoords(&pos, d, 1);
        if (isOutOfBounds(pos.x, pos.y)) return {x, y};
        return pos;
    }
    
    Node* extractByPos(int x, int y){ 
        if (isOutOfBounds(x, y)) return nullptr;
        return &GridMap[y][x];
    }
};

// 3. KELAS UTAMA (MENJALANKAN LABIRIN)

class MazeGeneratorSolver {

private:
    std::string currentTask = "Idle";
    int drawDelay = 30; 
    bool isPaused = false;
    
    sf::Font font;
    bool fontLoaded = false;

    float cellSize = 5.0f;
    float offsetX = 0.0f;
    float offsetY = 0.0f;

    void addQuad(sf::VertexArray& va, float x, float y, float size, sf::Color color) {
        va.append(sf::Vertex(sf::Vector2f(x, y), color));
        va.append(sf::Vertex(sf::Vector2f(x + size, y), color));
        va.append(sf::Vertex(sf::Vector2f(x + size, y + size), color));
        va.append(sf::Vertex(sf::Vector2f(x, y + size), color));
    }

    void updateLayout(sf::RenderWindow* window) {
        sf::Vector2u winSize = window->getSize();
        
        float availableW = (float)winSize.x;
        float availableH = (float)winSize.y - 64.0f;

        float scaleX = availableW / (2.0f * GRID_WIDTH);
        float scaleY = availableH / (2.0f * GRID_HEIGHT);
        
        cellSize = std::min(scaleX, scaleY);
        
        float mazePixelWidth = 2.0f * GRID_WIDTH * cellSize;
        float mazePixelHeight = 2.0f * GRID_HEIGHT * cellSize;

        offsetX = (availableW - mazePixelWidth) / 2.0f;
        offsetY = (availableH - mazePixelHeight) / 2.0f;
    }

    void drawGUI(sf::RenderWindow* window) {
        window->setView(window->getDefaultView());

        float panelHeight = 64.0f;
        float panelY = window->getSize().y - panelHeight;
        float centerY = panelY + (panelHeight / 2.0f);

        sf::RectangleShape panel(sf::Vector2f((float)window->getSize().x, panelHeight));
        panel.setPosition(0, panelY);
        panel.setFillColor(sf::Color(30, 30, 30));
        window->draw(panel);

        struct LegendItem { std::string label; sf::Color color; };
        std::vector<LegendItem> items = {
            { "Unvisited", sf::Color(70, 70, 70) },
            { "Visiting",  sf::Color(255, 0, 0) },
            { "Visited",   sf::Color(200, 200, 200) },
            { "Path",      sf::Color(0, 255, 0) },
            { "Wall",      sf::Color(0, 0, 0) }
        };

        float startX = 20.0f; // Padding kiri
        
        for (const auto& item : items) {
            // Gambar Kotak
            sf::RectangleShape box(sf::Vector2f(20.0f, 20.0f));
            // Letakkan box agak di atas sedikit dari tengah panel
            box.setPosition(startX, centerY - 10.0f); 
            box.setFillColor(item.color);
            box.setOutlineColor(sf::Color::White);
            box.setOutlineThickness(1);
            window->draw(box);

            // Gambar Label
            if (fontLoaded) {
                sf::Text text;
                text.setFont(font);
                text.setString(item.label);
                text.setCharacterSize(14);
                text.setFillColor(sf::Color::White);
                // Teks di sebelah kotak
                text.setPosition(startX + 30.0f, centerY - 10.0f); 
                window->draw(text);
            }

            startX += 110.0f; // Geser ke kanan
        }

        if (fontLoaded) {
            sf::Text taskText;
            taskText.setFont(font);
            taskText.setString(currentTask);
            taskText.setCharacterSize(24);
            taskText.setStyle(sf::Text::Bold);
            taskText.setFillColor(sf::Color::White); // Warna mencolok

            sf::FloatRect textRect = taskText.getLocalBounds();
            taskText.setOrigin(textRect.left + textRect.width/2.0f, textRect.top + textRect.height/2.0f);
            taskText.setPosition(sf::Vector2f(window->getSize().x / 2.0f, centerY));
            
            window->draw(taskText);
        }

        if (fontLoaded) {
            sf::Text statusText;
            statusText.setFont(font);
            
            std::string stateStr = isPaused ? "PAUSED (Q)" : "RUNNING";
            std::string info = stateStr + " | Delay: " + std::to_string(drawDelay) + "ms";
            
            statusText.setString(info);
            statusText.setCharacterSize(16);
            statusText.setFillColor(sf::Color::Yellow);
            
            sf::FloatRect bounds = statusText.getLocalBounds();
            statusText.setOrigin(bounds.left + bounds.width, bounds.top + bounds.height / 2.0f);
            statusText.setPosition(window->getSize().x - 20.0f, centerY);
            
            window->draw(statusText);
        }
    }

    void draw(Grid* Maze, sf::RenderWindow* window) {
        
        do {
            sf::Event event;
            bool inputDetected = false;

            while (window->pollEvent(event)) {
                if (event.type == sf::Event::Closed) {
                    window->close();
                    exit(0);
                }
                
                if (event.type == sf::Event::KeyPressed) {
                    //esc untuk keluar Fullscreen
                    if (event.key.code == sf::Keyboard::Escape) {
                        window->close();
                        exit(0);
                    }
                    if (event.key.code == sf::Keyboard::Up) {
                        drawDelay -= 5;
                        if (drawDelay < 0) drawDelay = 0;
                        inputDetected = true;
                    }
                    else if (event.key.code == sf::Keyboard::Down) {
                        drawDelay += 5;
                        inputDetected = true;
                    }
                    else if (event.key.code == sf::Keyboard::Q) {
                        isPaused = !isPaused;
                        inputDetected = true;
                    }
                }
            }

            if (inputDetected || isPaused) {
                std::string status = isPaused ? "[PAUSED] " : "[RUNNING] ";
                std::string title = "Labirin " + status + " - Speed Delay: " + std::to_string(drawDelay) + " ms";
                window->setTitle(title);
            }

            updateLayout(window);

            window->clear(sf::Color::Black);
            sf::VertexArray vertices(sf::Quads);

            const sf::Color cUnvisited(70, 70, 70);
            const sf::Color cVisiting(255, 0, 0);
            const sf::Color cVisited(200, 200, 200);
            const sf::Color cPath(0, 255, 0);

            for (int i = 0; i < GRID_WIDTH; i++) {
                for (int j = 0; j < GRID_HEIGHT; j++) {
                    
                    Node* currentNode = &Maze->GridMap[j][i];
                    Node::Flags currentStatus = currentNode->getStatus();

                    sf::Color currentColor;
                    switch (currentStatus) {
                        case Node::Flags::VISITING: currentColor = cVisiting; break;
                        case Node::Flags::VISITED:  currentColor = cVisited; break;
                        case Node::Flags::PATH:     currentColor = cPath; break;
                        default:                    currentColor = cUnvisited; break;
                    }

                    float px = offsetX + (2.0f * i * cellSize);
                    float py = offsetY + (2.0f * j * cellSize);
                    
                    addQuad(vertices, px, py, cellSize, currentColor);

                    for (int k = 0; k < 4; k++) {
                        Node* neighbor = currentNode->neighbor(Class2D::direction(k));
                        if (neighbor != nullptr) {
                            if (currentStatus == Node::Flags::PATH && neighbor->getStatus() != Node::Flags::PATH) continue;

                            Class2D::Coords wallPos = { (int)px, (int)py };
                            
                            if (k == 0) wallPos.y -= (int)cellSize; // UP
                            if (k == 1) wallPos.y += (int)cellSize; // DOWN
                            if (k == 2) wallPos.x -= (int)cellSize; // LEFT
                            if (k == 3) wallPos.x += (int)cellSize; // RIGHT
                            
                            addQuad(vertices, (float)wallPos.x, (float)wallPos.y, cellSize, currentColor);
                        }
                    }
                }
            }
            
            // 1. Gambar Maze
            window->draw(vertices);
            
            // 2. Gambar GUI
            drawGUI(window);
            
            window->display();

            if (isPaused) {
                sf::sleep(sf::milliseconds(50));
            } else {
                sf::sleep(sf::milliseconds(drawDelay));
                break; 
            }

        } while (window->isOpen());
    }

    void waitForInput(Grid* Maze, sf::RenderWindow* window) {
        isPaused = true;
        draw(Maze, window);
    }

    void carve(Grid* Maze, int startX, int startY, sf::RenderWindow* window) {
        std::stack<Class2D::Coords> stack;
        Node* startNode = Maze->extractByPos(startX, startY);
        if (!startNode) return;

        startNode->setStatus(Node::Flags::VISITING);
        stack.push({startX, startY});

        static std::random_device rd;
        static std::mt19937 g(rd());

        while (!stack.empty()) {
            draw(Maze, window); 

            Class2D::Coords currCoords = stack.top();
            Node* current = Maze->extractByPos(currCoords.x, currCoords.y);

            std::vector<int> neighbors;
            for (int i = 0; i < 4; i++) {
                Class2D::Directions dir = Class2D::direction(i);
                Class2D::Coords nextPos = Maze->getRelativePosition(currCoords.x, currCoords.y, dir);
                
                if (nextPos.x == currCoords.x && nextPos.y == currCoords.y) continue;
                
                Node* nextNode = Maze->extractByPos(nextPos.x, nextPos.y);
                if (nextNode && nextNode->getStatus() == Node::Flags::UNVISITED) {
                    neighbors.push_back(i);
                }
            }

            if (!neighbors.empty()) {
                std::uniform_int_distribution<> distr(0, neighbors.size() - 1);
                int randIndex = distr(g);
                Class2D::Directions dir = Class2D::direction(neighbors[randIndex]);
                Class2D::Coords nextPos = Maze->getRelativePosition(currCoords.x, currCoords.y, dir);
                Node* nextNode = Maze->extractByPos(nextPos.x, nextPos.y);

                current->append(dir, nextNode);
                nextNode->setStatus(Node::Flags::VISITING);
                stack.push(nextPos);
            } else {
                current->setStatus(Node::Flags::VISITED);
                stack.pop();
            }
        }
        draw(Maze, window);
    }

    void addLoops(Grid* Maze, sf::RenderWindow* window, int attempts) {
        static std::random_device rd;
        static std::mt19937 g(rd());
        std::uniform_int_distribution<> wDist(0, GRID_WIDTH-1);
        std::uniform_int_distribution<> hDist(0, GRID_HEIGHT-1);
        std::uniform_int_distribution<> dDist(0, 3);

        for(int i=0; i<attempts; i++){
            int x = wDist(g);
            int y = hDist(g);
            Class2D::Directions dir = Class2D::direction(dDist(g));

            Node* current = Maze->extractByPos(x, y);
            Class2D::Coords nPos = Maze->getRelativePosition(x, y, dir);
            
            if (nPos.x != x || nPos.y != y) {
                 Node* neighbor = Maze->extractByPos(nPos.x, nPos.y);
                 if(current->append(dir, neighbor)){
                     draw(Maze, window); 
                 }
            }
        }
    }

    void solve(Grid* Maze, int x, int y, int dx, int dy, sf::RenderWindow* window) {
        if (Maze->isOutOfBounds(dx, dy) || Maze->isOutOfBounds(x, y)) return;

        Node* start = Maze->extractByPos(x, y);
        Node* end = Maze->extractByPos(dx, dy);
        if (!start || !end) return;

        std::queue<Node*> q;
        q.push(start);
        start->setStatus(Node::Flags::VISITED);
        start->setParent(nullptr);

        while (!q.empty()) {
            draw(Maze, window); 
            
            Node* temp = q.front();
            q.pop();

            if (temp == end) {
                std::vector<Node*> path;
                Node* curr = end;
                while (curr != nullptr) {
                    path.push_back(curr);
                    curr = curr->getParent();
                }
                std::reverse(path.begin(), path.end());
                Maze->clearFlags(); 
                
                for (Node* n : path) {
                    n->setStatus(Node::Flags::PATH);
                    draw(Maze, window);
                }
                return;
            }

            for (int i = 0; i < 4; i++) {
                Node* adj = temp->neighbor(Class2D::direction(i));
                if (adj == nullptr || adj->getStatus() != Node::Flags::UNVISITED) continue;
                
                adj->setParent(temp); 
                adj->setStatus(Node::Flags::VISITED);
                q.push(adj);
            }
        }
    }

public:
    MazeGeneratorSolver() {
        if (!font.loadFromFile("ARIAL.TTF")) {
            if (!font.loadFromFile("arial/ARIAL.TTF")) {
                std::cerr << "ERROR: Failed to load font (ARIAL.TTF). Legend text will not be visible." << std::endl;
                fontLoaded = false;
            } else {
                fontLoaded = true;
            }
        } else {
            fontLoaded = true;
        }
    }

    void run(Grid* Maze, sf::RenderWindow* window) {
        window->setFramerateLimit(0);

        waitForInput(Maze, window);

        while (window->isOpen()) {
            sf::Event event;
            while (window->pollEvent(event)) {
                if (event.type == sf::Event::Closed) window->close();
            }
            isPaused = false; 

            // 1. Generate Labirin
            currentTask = "Generating Maze";
            carve(Maze, 0, 0, window);

            // 2. Tambahkan cycles
            addLoops(Maze, window, (GRID_WIDTH * GRID_HEIGHT) / 6); 
            Maze->clearFlags(Node::Flags::UNVISITED);
            draw(Maze, window);

            currentTask = "Idle";
            
            waitForInput(Maze, window);


            currentTask = "Traversing BFS";
            Maze->clearFlags(); 

            // 3. Penelsuran (BFS)
            solve(Maze, 0, 0, GRID_WIDTH - 1, GRID_HEIGHT - 1, window);
            
            currentTask = "Idle";

            waitForInput(Maze, window); 
            
            Maze->resetGraph();
        }
    }
};

int main(){

    Grid Maze;
    sf::VideoMode desktop = sf::VideoMode::getDesktopMode();
    sf::RenderWindow window(desktop, "Labirin - Tugas Akhir", sf::Style::Fullscreen);
    
    window.setFramerateLimit(60);

    MazeGeneratorSolver Master;
    Master.run(&Maze, &window);

    return 0;
}