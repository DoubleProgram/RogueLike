 struct POS {
        int y = 0, x = 0;
        char c;
        POS(int x, int y) :x(x), y(y) {}
        POS(int x, int y, char c) :x(x), y(y), c(c) {}
        POS() {}
        POS operator+(const POS& p) {
            return POS(this->x + p.x, this->y + p.y);
        }
        POS operator-(const POS& p) {
            return POS(this->x - p.x, this->y - p.y);
        }
        bool operator==(const POS& p) {
            return this->x == p.x && this->y == p.y;
        }
    };

    enum TileType {
        Tile_Empty = '*',
        Tile_Wall = '#',
        Tile_Floor = '.',
        Tile_Door = '%',
        Tile_Player = 'P',
        Tile_Enemy = 'E',
        WV = 'I', 
        WDL = '7',
        WDLU = 'A',
        WDR = 'F',
        WDRU = 'H',
        WH = '_',
        WLU = 'J',
        WRDL = 'T',
        WRU = 'L',
        WRUL = '-',
        WX = 'X',
        WR = 'R',
        WL = '/',
        WU = 'U',
        WD = 'D'
    };

    int WIDTH = 0, HEIGHT = 0;
  
    std::vector<std::string> map;
    std::vector<std::vector<POS>> rooms; // all rooms
    std::vector<POS> walls;  //all walls, on which the rooms can spawn, the corneres are not included here

    Generation();
    Generation(int WIDTH, int HEIGHT, int borderLeft, int borderRight, int borderUp, int borderDown);
    int Start(int amountOfRooms, int minWidth, int maxWidth, int minHeight, int maxHeight);
    
private:
    int dungeonWidth = 0, dungeonHeight = 0;
    int borderLeft = 0, borderRight = 0, borderUp = 0, borderDown = 0;
    int horizontalShift = 0, verticalShift = 0;
    int amountOfRooms;
    std::vector<std::array<std::vector<POS>, 4>> connectpoints; //[vector of rooms and the array of 4 vector walls] the doors can spawn here
    //up,down,right,left 
    std::vector<std::vector<POS>> innerSectionRooms; // all rooms, but they contain only the inner room section(if this section is overlapsed by a room, it has to respawn)
    int spacing = 2; // room spacing
    int prevdirY = -1, prevdirX = -1;
    bool isValidSpace = true;
    void TryFillOutSpacing(POS pos, std::vector<POS>& spacing);
    void OpenSpace(POS pos, std::vector<POS>& spacing);
    void SpawnDoors();
    void SpawnRoom(const int& minWidth, const int& maxWidth, const int& minHeight, const int& maxHeight, const POS& pos = POS());
    bool CanPlaceRoom(int x, int y, int width, int height);
    void ShiftDungeon();
    void FillOutSpace();
    void CleanDungeon();
    std::vector<std::string> get2DRenderMap();
    void SpawnEntities();
    int CreateRooms(int minWidth, int maxWidth, int minHeight, int maxHeight);
};
