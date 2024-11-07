#include <iostream>
#include <vector>
#include <random>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <thread>
#include <atomic>
#include <filesystem>
#include <SFML/Graphics.hpp>
#include <sstream>
#include <fstream>
#include <sstream>
#include <string>


// Gobal // 
sf::VideoMode desktop = sf::VideoMode::getDesktopMode();

const int screenWidth = desktop.width;
const int screenHeight = desktop.height;

const int windowWidth = screenWidth / 1.5;
const int windowHeight = screenHeight / 1.5;

int CameraSpeed = 5 * 100;
int CameraX = 0;
int CameraY = 0;

float CellSize = 10;
int MapWidth, MapHeight;

int offset = 10;
int red = 21;
int green = 22;
int blue = 31;

bool LoadConfig(const std::string& filename) {
    std::ifstream file(filename);

    if (!file.is_open()) {
        std::cerr << "Error: Could not open file " << filename << std::endl;
        return false;
    }

    std::string line;
    while (std::getline(file, line)) {
        std::stringstream ss(line);
        std::string key;
        std::string value;

        // Read the key (before the colon) and the value (after the colon)
        if (std::getline(ss, key, ':') && std::getline(ss, value)) {
            // Remove any leading or trailing spaces from the value
            value = value.substr(value.find_first_not_of(" "), value.find_last_not_of(" ") + 1);

            // Convert the value based on the key
            if (key == "CellSize") {
                std::stringstream(value) >> CellSize;
            }
            else if (key == "MapWidth") {
                std::stringstream(value) >> MapWidth;
            }
            else if (key == "MapHeight") {
                std::stringstream(value) >> MapHeight;
            }
            else {
                std::cerr << "Warning: Unrecognized key in config file: " << key << std::endl;
            }
        }
    }

    file.close();
    return true;
}

enum CellType { DEBUGCELL = -1, EMPTYCELL, CONWAYCELL, SEEDCELL, PLANTCELL, DIRTCELL, WATERCELL };


class Cell {
public:
    CellType Type;
    CellType NextType;
    int Y;
    int X;
    std::vector<int> NeighboringCells = { -1,-1,-1,
                                         -1,-1,-1,
                                         -1,-1,-1 };

    void UpdateLogic(std::vector<Cell>& Cells) {
        if (Type == CellType::CONWAYCELL) {
            int NeighboringDirtCells = 0;
            int NeighboringPlantCells = 0;
            int NeighboringWaterCells = 0;
            int NeighboringEmptyCells = 0;
            int NeighboringConwayCells = 0;

            for (int i = 0; i < NeighboringCells.size(); i++) { 
                if (NeighboringCells.at(i) != -1 && Cells[NeighboringCells.at(i)].Type != CellType::EMPTYCELL) { 
                    if (Cells[NeighboringCells.at(i)].Type == CellType::DIRTCELL) { 
                        NeighboringDirtCells += 1; 
                    }
                    if (Cells[NeighboringCells.at(i)].Type == CellType::WATERCELL) { 
                        NeighboringWaterCells += 1; 
                    }
                    if (Cells[NeighboringCells.at(i)].Type == CellType::PLANTCELL) {  
                        NeighboringPlantCells += 1; 
                    }
                    if (Cells[NeighboringCells.at(i)].Type == CellType::CONWAYCELL) { 
                        NeighboringConwayCells += 1; 
                    }
                }
                else {
                    if (i != 5) {
                        NeighboringEmptyCells += 1;
                    }
                }
            }

            if (NeighboringConwayCells < 2) {
                NextType = CellType::EMPTYCELL;
            }
            else if (NeighboringConwayCells == 2 || NeighboringConwayCells == 3) {
                NextType = CellType::CONWAYCELL;
            }
            else if ((NeighboringWaterCells > 2 && NeighboringWaterCells < 3) || (NeighboringPlantCells > 2 && NeighboringPlantCells < 4)) {
                NextType = CellType::CONWAYCELL;
            }
            else {
                NextType = CellType::EMPTYCELL;
            }

        }
        if (Type == CellType::SEEDCELL) {
            int NeighboringDirtCells = 0;
            int NeighboringSeedCells = 0;
            int NeighboringWaterCells = 0;
            int NeighboringEmptyCells = 0;

            for (int i = 0; i < NeighboringCells.size(); i++) {
                if (NeighboringCells.at(i) != -1 && Cells[NeighboringCells.at(i)].Type != CellType::EMPTYCELL) {
                    if (Cells[NeighboringCells.at(i)].Type == CellType::DIRTCELL) {
                        NeighboringDirtCells += 1;
                    }
                    if (Cells[NeighboringCells.at(i)].Type == CellType::WATERCELL) {
                        NeighboringWaterCells += 1;
                    }
                    if (Cells[NeighboringCells.at(i)].Type == CellType::SEEDCELL) {
                        NeighboringSeedCells += 1;
                    }
                }
                else {
                    if (i != 5) {
                        NeighboringEmptyCells += 1;
                    }
                }
            }

            if (NeighboringSeedCells > 1) {
                NextType = CellType::DIRTCELL;
            } else if (NeighboringWaterCells > 5) {
                NextType = CellType::WATERCELL;
            }
            else {
                if (NeighboringDirtCells > 2) {
                    NextType = CellType::SEEDCELL;
                }
                else if (NeighboringDirtCells > 0 && NeighboringWaterCells > 0) {
                    NextType = CellType::PLANTCELL;
                }
                else {
                    NextType = CellType::SEEDCELL;
                }
            }
        }
        if (Type == CellType::WATERCELL) {
            int NeighboringDirtCells = 0;
            int NeighboringSeedCells = 0;
            int NeighboringPlantCells = 0;
            int NeighboringEmptyCells = 0;

            for (int i = 0; i < NeighboringCells.size(); i++) {
                if (NeighboringCells.at(i) != -1 && Cells[NeighboringCells.at(i)].Type != CellType::EMPTYCELL) {
                    if (Cells[NeighboringCells.at(i)].Type == CellType::PLANTCELL) {
                        NeighboringPlantCells += 1;
                    }
                    if (Cells[NeighboringCells.at(i)].Type == CellType::SEEDCELL) {
                        NeighboringSeedCells += 1;
                    }
                    if (Cells[NeighboringCells.at(i)].Type == CellType::DIRTCELL) {
                        NeighboringDirtCells += 1;
                    }
                }
                else {
                    if (i != 5) {
                        NeighboringEmptyCells += 1;
                    }
                }
            }

            if (NeighboringDirtCells > 2 || NeighboringPlantCells > 2 || NeighboringSeedCells > 2) {
                NextType = CellType::EMPTYCELL;
            }
            else {
                NextType = CellType::WATERCELL;
            }
        }
        if (Type == CellType::DIRTCELL) {
            

            int NeighboringPlantCells = 0;
            int NeighboringWaterCells = 0;
            int NeighboringEmptyCells = 0;

            for (int i = 0; i < NeighboringCells.size(); i++) {
                if (NeighboringCells.at(i) != -1 && Cells[NeighboringCells.at(i)].Type != CellType::EMPTYCELL) {
                    if (Cells[NeighboringCells.at(i)].Type == CellType::PLANTCELL) {
                        NeighboringPlantCells += 1;
                    }
                    if (Cells[NeighboringCells.at(i)].Type == CellType::WATERCELL) {
                        NeighboringWaterCells += 1;
                    }
                }
                else {
                    if (i != 5) {
                        NeighboringEmptyCells += 1;
                    }
                }
            }

            if (NeighboringWaterCells > 4) {
                NextType = CellType::EMPTYCELL;
            }
            else if (NeighboringPlantCells > 3) {
                NextType = CellType::EMPTYCELL;
            } 
            else {
                NextType = CellType::DIRTCELL;
            }

        }
        if (Type == CellType::PLANTCELL) {
            int NeighboringSeedCells = 0;
            int NeighboringPlantCells = 0;
            int NeighboringWaterCells = 0;
            int NeighboringEmptyCells = 0;
            int NeighboringDirtCells = 0;

            for (int i = 0; i < NeighboringCells.size(); i++) {
                if (NeighboringCells.at(i) != -1 && Cells[NeighboringCells.at(i)].Type != CellType::EMPTYCELL) {
                    if (Cells[NeighboringCells.at(i)].Type == CellType::SEEDCELL) {
                        NeighboringSeedCells += 1;
                    }
                    if (Cells[NeighboringCells.at(i)].Type == CellType::PLANTCELL) {
                        NeighboringPlantCells += 1;
                    }
                    if (Cells[NeighboringCells.at(i)].Type == CellType::WATERCELL) {
                        NeighboringWaterCells += 1;
                    }
                    if (Cells[NeighboringCells.at(i)].Type == CellType::DIRTCELL) {
                        NeighboringDirtCells += 1;
                    }
                }
                else {
                    if (i != 5) {
                        NeighboringEmptyCells += 1;
                    }
                }
            }

            if (NeighboringDirtCells > 2 || NeighboringSeedCells > 2 || NeighboringPlantCells < 1 || NeighboringPlantCells > 2) {
                if (NeighboringDirtCells > NeighboringSeedCells) {
                    NextType = CellType::DIRTCELL;
                }
                else {
                    NextType = CellType::WATERCELL;
                }
            }
            else if (NeighboringDirtCells > 0 && NeighboringWaterCells > 0 & NeighboringEmptyCells > 4) {
                NextType = CellType::SEEDCELL;
            } else {
                NextType = CellType::PLANTCELL;
            }
            
        }
        if (Type == CellType::EMPTYCELL) {
            int NeighboringConwayCells = 0;
            int NeighboringSeedCells   = 0;
            int NeighboringPlantCells  = 0;
            int NeighboringWaterCells  = 0;
            int NeighboringEmptyCells  = 0;
            int NeighboringDirtCells   = 0;

            for (int i = 0; i < NeighboringCells.size(); i++) {
                if (NeighboringCells.at(i) != -1 && Cells[NeighboringCells.at(i)].Type != CellType::EMPTYCELL) { 
                    if (Cells[NeighboringCells.at(i)].Type == CellType::CONWAYCELL) {
                        NeighboringConwayCells += 1;
                    }
                    if (Cells[NeighboringCells.at(i)].Type == CellType::SEEDCELL) {
                        NeighboringSeedCells += 1; 
                    }
                    if (Cells[NeighboringCells.at(i)].Type == CellType::PLANTCELL) {
                        NeighboringPlantCells += 1;
                    }
                    if (Cells[NeighboringCells.at(i)].Type == CellType::WATERCELL) {
                        NeighboringWaterCells += 1;
                    }
                    if (Cells[NeighboringCells.at(i)].Type == CellType::DIRTCELL) {
                        NeighboringDirtCells += 1;
                    }
                }
                else {
                    if (i != 5) {
                        NeighboringEmptyCells += 1;
                    }
                }
            }


            if (NeighboringPlantCells > 0 && NeighboringSeedCells == 0 && NeighboringEmptyCells > 2 && NeighboringPlantCells < 2  && NeighboringEmptyCells > 2 && NeighboringDirtCells < 3 && NeighboringWaterCells > 1) {
                NextType = CellType::PLANTCELL;
            }
            else if (NeighboringWaterCells > 3) {
                NextType = CellType::WATERCELL;
            } else if (NeighboringConwayCells == 3) {
                NextType = CellType::CONWAYCELL;
            }
            else {
                NextType = CellType::EMPTYCELL;
            }
        }

        Type = NextType;
    }

    inline void Init(std::vector<Cell>& Cells) {

        for (int i = 0;i < Cells.size();i++) {
            if (Cells.at(i).X == X - CellSize && Cells.at(i).Y == Y - CellSize) {
                NeighboringCells.at(0) = i;
            }
            if (Cells.at(i).X == X && Cells.at(i).Y == Y - CellSize) {
                NeighboringCells.at(1) = i;
            }
            if (Cells.at(i).X == X + CellSize && Cells.at(i).Y == Y - CellSize) {
                NeighboringCells.at(2) = i;
            }
            if (Cells.at(i).X == X - CellSize && Cells.at(i).Y == Y) {
                NeighboringCells.at(3) = i;
            }
            if (Cells.at(i).X == X + CellSize && Cells.at(i).Y == Y) {
                NeighboringCells.at(5) = i;
            }
            if (Cells.at(i).X == X - CellSize && Cells.at(i).Y == Y + CellSize) {
                NeighboringCells.at(6) = i;
            }
            if (Cells.at(i).X == X && Cells.at(i).Y == Y + CellSize) {
                NeighboringCells.at(7) = i;
            }
            if (Cells.at(i).X == X + CellSize && Cells.at(i).Y == Y + CellSize) {
                NeighboringCells.at(8) = i;
            }

        }
    }

private:

};

std::vector<Cell> Cells;

inline void AddCell(CellType Type, int X, int Y) {
    Cell CellInstance = { Type, Type, CellSize * X, CellSize * Y };
    Cells.push_back(CellInstance);
}

void CreateCells(sf::RenderWindow& window, int Width, int Height) {
    Cells.reserve(Width * Height);

    Cells = {};

    red = 21 + (rand() % (offset * 2 + 1) - offset);
    green = 22 + (rand() % (offset * 2 + 1) - offset);
    blue = 31 + (rand() % (offset * 2 + 1) - offset);

    sf::Font font;
    font.loadFromFile("./Assets/dogicapixelbold.ttf");

    sf::Text Text;
    Text.setFont(font);
    Text.setString("");
    Text.setCharacterSize(20);
    Text.setFillColor(sf::Color(255,255,255, 255));
    Text.setPosition(0,0);

    int Y = 0;
    int X = 0;
    while (Cells.size() < Width * Height) {
        AddCell(CellType::EMPTYCELL, X, Y);

        X += 1;
        if (X == Width) {
            X = 0;
            Y += 1;

            window.clear(sf::Color(red, green, blue));
            Text.setString(std::to_string(Y) + "/" + std::to_string(Height) + " Rows of cells created");
            window.draw(Text);
            window.display();
        }
    }

    window.clear(sf::Color(red, green, blue));
    Text.setString("Initializing cells :Might take some time if your map is large:");
    window.draw(Text);
    window.display();

    std::vector<std::thread> threads;
    size_t batchSize = Cells.size() / std::thread::hardware_concurrency(); 

    for (size_t t = 0; t < std::thread::hardware_concurrency(); ++t) {
        threads.push_back(std::thread([&, t, batchSize]() { 
            size_t startIdx = t * batchSize; 
            size_t endIdx = (t + 1 == std::thread::hardware_concurrency()) ? Cells.size() : (t + 1) * batchSize; 
            for (size_t i = startIdx; i < endIdx; ++i) { 
                Cells[i].Init(Cells); 
            }
            }));
    }

    for (auto& t : threads) {
        t.join();
    }

}

inline void AppendVertices(sf::VertexArray& vertices, sf::Color Color, int x, int y) {
    vertices.append(sf::Vertex(sf::Vector2f(x, y), Color));
    vertices.append(sf::Vertex(sf::Vector2f(x + std::max(CellSize - 1.0,1.0), y), Color));
    vertices.append(sf::Vertex(sf::Vector2f(x + std::max(CellSize - 1.0, 1.0), y + std::max(CellSize - 1.0, 1.0)), Color));
    vertices.append(sf::Vertex(sf::Vector2f(x, y + std::max(CellSize - 1.0, 1.0)), Color));
}

inline void RenderCells(sf::RenderWindow& window) {
    sf::VertexArray vertices(sf::Quads);

    for (int i = 0; i < Cells.size(); i++) {
        int x = Cells.at(i).X + CameraX;
        int y = Cells.at(i).Y + CameraY;

        switch (Cells.at(i).Type) {

            case CellType::CONWAYCELL:
                AppendVertices(vertices, sf::Color::White, x, y);
                break;
            case CellType::SEEDCELL:
                AppendVertices(vertices, sf::Color(242, 245, 66), x, y);
                break;
            case CellType::WATERCELL:
                AppendVertices(vertices, sf::Color(66, 135, 245), x, y);
                break;
            case CellType::DIRTCELL:
                AppendVertices(vertices, sf::Color(139, 69, 19), x, y);
                break;
            case CellType::PLANTCELL:
                AppendVertices(vertices, sf::Color(66, 245, 84), x, y);
                break;

            case CellType::EMPTYCELL:
                AppendVertices(vertices, sf::Color::Black, x, y);
                break;
            case CellType::DEBUGCELL:
                AppendVertices(vertices, sf::Color::Magenta, x, y);
                break;


        }
    }

    window.draw(vertices);
}


bool DebugMenu = false;
bool GamePaused = true;
CellType CurrentPlacementCellType = CellType::CONWAYCELL;
sf::RectangleShape DisplayCell;
int PlacementIndex = 1;



int main() {
    LoadConfig("Config.txt");
    srand(time(0));

    sf::Font font;
    font.loadFromFile("./Assets/dogicapixelbold.ttf");

    sf::RenderWindow window(sf::VideoMode(windowWidth, windowHeight), "Colors of Life", sf::Style::Titlebar | sf::Style::Close);
    window.setPosition(sf::Vector2i((screenWidth - windowWidth) / 2, (screenHeight - windowHeight) / 2));
    window.setFramerateLimit(60);

    sf::Event Event;
    sf::Clock DetlaClock;
    DisplayCell.setSize(sf::Vector2f(CellSize,CellSize));
    DisplayCell.setPosition(sf::Vector2f(CellSize, CellSize));

    CreateCells(window,MapWidth, MapHeight); // 100-100

    sf::RectangleShape PausedBackground;
    PausedBackground.setSize(sf::Vector2f(windowWidth, windowHeight));
    PausedBackground.setFillColor(sf::Color(0,0,0,0));

    sf::Text PausedText;
    PausedText.setFont(font);               
    PausedText.setString("GAME PAUSED"); 
    PausedText.setCharacterSize(60);         
    PausedText.setFillColor(sf::Color(235, 64, 52,0));
    sf::FloatRect textBounds = PausedText.getLocalBounds(); 
    PausedText.setPosition(windowWidth / 2.f - textBounds.width / 2.f,windowHeight / 10.f - textBounds.height / 10.f);

    sf::Text DisplayText;
    DisplayText.setFont(font);
    DisplayText.setString("Conway Cell");
    DisplayText.setCharacterSize(10);
    DisplayText.setFillColor(sf::Color(255,255,255));
    sf::FloatRect DisplayTextBounds = DisplayText.getLocalBounds();
    DisplayText.setPosition(windowWidth / 25.f - textBounds.width / 25.f, windowHeight / 65.f - textBounds.height / 65.f);


    while (window.isOpen()) {
        float DeltaTime = DetlaClock.restart().asSeconds();
        const float MaxDeltaTime = 0.1f;

        DeltaTime = std::min(DeltaTime, MaxDeltaTime);

        if (GamePaused) {
            PausedBackground.setFillColor(sf::Color(0, 0, 0,66));
            PausedText.setFillColor(sf::Color(235, 64, 52, 66));
        }
        else {
            PausedBackground.setFillColor(sf::Color(0, 0, 0, 0));
            PausedText.setFillColor(sf::Color(235, 64, 52, 0));
        }

        while (window.pollEvent(Event)) {
            if (Event.type == sf::Event::Closed) {
                window.close();
            }

            if (Event.type == sf::Event::MouseButtonPressed) {
                if (Event.mouseButton.button == sf::Mouse::Left) {
                    sf::Vector2i mousePos = sf::Mouse::getPosition(window);

                    int mouseX = mousePos.x - CameraX;
                    int mouseY = mousePos.y - CameraY;

                    for (int i = 0; i < Cells.size(); i++) {

                        if (mouseX >= Cells.at(i).X && mouseX < Cells.at(i).X + CellSize && mouseY >= Cells.at(i).Y && mouseY < Cells.at(i).Y + CellSize) {
                            if (Cells.at(i).Type == CurrentPlacementCellType) {
                                Cells.at(i).Type = EMPTYCELL;
                            }
                            else {
                                Cells.at(i).Type = CurrentPlacementCellType;
                            }
                            break;
                        }
                    }
                }
                if (Event.mouseButton.button == sf::Mouse::Right) {
                    PlacementIndex += 1;

                    if (PlacementIndex > 5) {
                        PlacementIndex = 1;
                    }

                    switch (PlacementIndex) {
                        case 1:
                            CurrentPlacementCellType = CellType::CONWAYCELL;
                            DisplayCell.setFillColor(sf::Color::White);
                            DisplayText.setString("Conway Cell");
                            DisplayText.setFillColor(sf::Color(255, 255, 255));
                            break;
                        case 2:
                            CurrentPlacementCellType = CellType::SEEDCELL;
                            DisplayCell.setFillColor(sf::Color(242, 245, 66));
                            DisplayText.setString("Seed Cell");
                            DisplayText.setFillColor(sf::Color(242, 245, 66));
                            break;
                        case 3:
                            CurrentPlacementCellType = CellType::PLANTCELL; 
                            DisplayCell.setFillColor(sf::Color(66, 245, 84));
                            DisplayText.setString("Plant Cell");
                            DisplayText.setFillColor(sf::Color(66, 245, 84));
                            break;
                        case 4:
                            CurrentPlacementCellType = CellType::DIRTCELL; 
                            DisplayCell.setFillColor(sf::Color(139, 69, 19));
                            DisplayText.setString("Dirt Cell");
                            DisplayText.setFillColor(sf::Color(139, 69, 19));
                            break;
                        case 5:
                            CurrentPlacementCellType = CellType::WATERCELL; 
                            DisplayCell.setFillColor(sf::Color(66, 135, 245));
                            DisplayText.setString("Water Cell");
                            DisplayText.setFillColor(sf::Color(66, 135, 245));
                            break;

                    }
                }

            }

            if (Event.type == sf::Event::KeyPressed && Event.key.code == sf::Keyboard::Space) {
                GamePaused = !GamePaused;
            }
        }

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up) || sf::Keyboard::isKeyPressed(sf::Keyboard::W)) {
            CameraY += std::round(CameraSpeed * DeltaTime);
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down) || sf::Keyboard::isKeyPressed(sf::Keyboard::S)) {
            CameraY -= std::round(CameraSpeed * DeltaTime);
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left) || sf::Keyboard::isKeyPressed(sf::Keyboard::A)) {
            CameraX += std::round(CameraSpeed * DeltaTime);
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right) || sf::Keyboard::isKeyPressed(sf::Keyboard::D)) {
            CameraX -= std::round(CameraSpeed * DeltaTime);
        }

        if (GamePaused == false) {
            for (int i = 0; i < Cells.size(); i++) {
                Cells.at(i).UpdateLogic(Cells);
            }
        }

        window.clear(sf::Color(red, green, blue));
        RenderCells(window);

        window.draw(PausedBackground);
        window.draw(DisplayCell);
        window.draw(PausedText);
        window.draw(DisplayText);

        window.display();
    }

    return 0;
}

