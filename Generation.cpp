#include "Generation.hpp"

Generation::Generation() {}

Generation::Generation(int WIDTH, int HEIGHT, int borderLeft, int borderRight, int borderUp, int borderDown)
    :WIDTH(WIDTH), HEIGHT(HEIGHT), borderLeft(borderLeft), borderRight(borderRight),
    borderUp(borderUp), borderDown(borderDown) {
    map.reserve(HEIGHT);
    for (int j = 0; j < HEIGHT; j++) {
        map.push_back("");
        for (int i = 0; i < WIDTH; i++) {
            map[j].push_back(Tile_Empty);
        }
    }

    dungeonHeight = HEIGHT - (borderDown + borderUp);
    dungeonWidth = WIDTH - (borderLeft + borderRight);
}

int Generation::Start(int amountOfRooms, int minWidth, int maxWidth, int minHeight, int maxHeight) {
    this->amountOfRooms = amountOfRooms;

    if (CreateRooms(minWidth, maxWidth, minHeight, maxHeight) == -1)
        return -1;

    ShiftDungeon();

    FillOutSpace();

    SpawnDoors();

    for (int j = 0; j < HEIGHT; j++)
        std::cout << map[j] << std::endl;

    CleanDungeon();

    for (int j = 0; j < HEIGHT; j++)
        std::cout << map[j] << std::endl;

    map = get2DRenderMap();

    return 0;
}

int Generation::CreateRooms(int minWidth, int maxWidth, int minHeight, int maxHeight) {
    while (maxHeight * maxWidth * amountOfRooms > dungeonWidth * dungeonHeight)
        amountOfRooms--;

    if (amountOfRooms == 0) return -1;

    rooms.reserve(amountOfRooms);
    innerSectionRooms.reserve(amountOfRooms);

    //this is probably uneccesery
    POS spawnPoint = POS(dungeonHeight / 2, dungeonWidth / 2);
    SpawnRoom(minWidth, maxWidth, minHeight, maxHeight, spawnPoint);

    for (int i = 0; i < amountOfRooms; i++)
        SpawnRoom(minWidth, maxWidth, minHeight, maxHeight);
}

void Generation::SpawnRoom(const int& minWidth, const int& maxWidth, const int& minHeight, const int& maxHeight, const POS& spawnPos) {
    int width = 0;
    int height = 0;
    int y = 0;
    int x = 0;

    if (spawnPos.y != 0 && spawnPos.x != 0) {
        width = minWidth + (maxWidth - minWidth == 0 ? 0 : (rand() % (maxWidth - minWidth)));
        height = minHeight + (maxHeight - minHeight == 0 ? 0 : (rand() % (maxHeight - minHeight)));

        //spawn the first room exactly in the middle of the map.
        x = spawnPos.x - height / 2;
        y = spawnPos.y - width / 2;
    }
    else {
        int dirX = 0, dirY = 0;
        do {
            width = minWidth + (maxWidth - minWidth == 0 ? 0 : (rand() % (maxWidth - minWidth)));
            height = minHeight + (maxHeight - minHeight == 0 ? 0 : (rand() % (maxHeight - minHeight)));
            
            int i = rand() % walls.size();

            dirX = (rand() % 2 == 0 ? 0 : 1);
            dirY = (rand() % 2 == 0 ? 0 : 1);
            x = walls[i].x - dirX * (height - 1);
            y = walls[i].y - dirY * (width - 1);
        } while ((dirX == prevdirX && dirY == prevdirY) || !CanPlaceRoom(x, y, width, height));
        prevdirY = dirY;
        prevdirX = dirX;
    }


    rooms.push_back(std::vector<POS>());
    innerSectionRooms.push_back(std::vector<POS>());
    connectpoints.push_back(std::array<std::vector<POS>, 4>());

    for (int j = x; j < height + x; j++)
        for (int i = y; i < width + y; i++) {
            if (j == x || i == y || i == width + y - 1 || j == height + x - 1) {
                rooms.back().push_back(POS(j, i, Tile_Wall));
                if ((j == x && i == y) || (j == x + height - 1 && i == y + width - 1)
                    || (j == x && i == y + width - 1) || (j == x + height - 1 && i == y)) {  //corners
                    continue;
                }

                if (j == x) //upper side
                    connectpoints.back()[0].push_back(POS(j, i));
                if (j == height + x - 1) //bottom side
                    connectpoints.back()[1].push_back(POS(j, i));
                if (i == y + width - 1) //right side
                    connectpoints.back()[2].push_back(POS(j, i));
                if (i == y) //left side
                    connectpoints.back()[3].push_back(POS(j, i));

                if ((j == x && i == y + 1) || (j == x + height - 1 && i == y + width - 2)       //the rooms can't spawn on this places...
                    || (j == x + 1 && i == y + width - 1) || (j == x + height - 2 && i == y)  // the first wall after the corner
                    || (j == x + 1 && i == y) || (j == x + height - 2 && i == y + width - 1)
                    || (j == x && i == y + width - 2) || (j == x + height - 1 && i == y + 1)) {
                    continue;
                }

                walls.push_back(POS(j, i));
            }
            else {
                rooms.back().push_back(POS(j, i, Tile_Floor));
                if (j + spacing >= x + height || j - spacing <= x || i + spacing >= y + width || i - spacing <= y)
                    continue;
                innerSectionRooms.back().push_back(POS(j, i, Tile_Floor));
            }
        }
}

bool Generation::CanPlaceRoom(int x, int y, int width, int height) {
    if (x < 0 || y < 0 || x + height >= dungeonHeight || y + width >= dungeonWidth)
        return false;

    auto tempRooms = rooms;
    auto tmpInnerSection = innerSectionRooms;
    auto tempWalls = walls;

    // iterating through the room
    for (int X = x; X < height + x; X++)
        for (int Y = y; Y < width + y; Y++) {

            // checking here if the walls are too close to the others room walls, if its true then the room can't be placed
            if (X == x) {
                for (int i = 1; i < spacing; i++) {
                    for (int j = 0; j < walls.size(); j++) {
                        if (walls[j].x == X - i && walls[j].y == Y) {
                            return false;
                        }
                    }
                }
            } else if (X == height + x - 1) {
                for (int i = 1; i < spacing; i++) {
                    for (int j = 0; j < walls.size(); j++) {
                        if (walls[j].x == X + i && walls[j].y == Y) {
                            return false;
                        }
                    }
                }
            } else if (Y == y) {
                for (int i = 1; i < spacing; i++) {
                    for (int j = 0; j < walls.size(); j++) {
                        if (walls[j].y == Y - i && walls[j].x == X) {
                            return false;
                        }
                    }
                }
            } else if (Y == width + y - 1) {
                for (int i = 1; i < spacing; i++) {
                    for (int j = 0; j < walls.size(); j++) {
                        if (walls[j].y == Y + i && walls[j].x == X) {
                            return false;
                        }
                    }
                }
            }

            //check if the room overlapses any other existing room, if it does the room can't be placed
            // iterating through all existing rooms
            for (int i = 0; i < tempRooms.size(); i++) {
                for (int j = 0; j < tempRooms[i].size(); j++) {

                    if (tempRooms[i][j].x == X && tempRooms[i][j].y == Y) {
                        for (int k = 0; k < tmpInnerSection[i].size(); k++) {
                            if (tmpInnerSection[i][k].x == X && tmpInnerSection[i][k].y == Y) {
                                tmpInnerSection[i].erase(tmpInnerSection[i].begin() + k);
                                break;
                            }
                        }

                        tempRooms[i].erase(tempRooms[i].begin() + j);
                        break;
                    }
                }

                if (tmpInnerSection[i].empty()) 
                    return false;
            }
        }

    innerSectionRooms = tmpInnerSection;
    rooms = tempRooms;
    walls = tempWalls;
    return true;
}

void Generation::ShiftDungeon() {
    POS leftp = POS(0, dungeonWidth + 1),
        rightp = POS(0, -1),
        upp = POS(dungeonHeight + 1, 0),
        downp = POS(-1, 0);

    //find the leftpoint rightp upp downp of the whole dungeon
    for (int j = 0; j < rooms.size(); j++) {
        for (int i = 0; i < rooms[j].size(); i++) {
            if (leftp.y > rooms[j][i].y)
                leftp = rooms[j][i];
            if (rightp.y < rooms[j][i].y)
                rightp = rooms[j][i];


            if (downp.x < rooms[j][i].x)
                downp = rooms[j][i];
            if (upp.x > rooms[j][i].x)
                upp = rooms[j][i];
        }
    }

    int leftspan = leftp.y;
    int rightspan = dungeonWidth - rightp.y;
    int upspan = upp.x;
    int downspan = dungeonHeight - downp.x;

    //horizontal shift
    int diff;
   
    //then center the whole rooms on the dungeon (centering the whole dungeon)
    while (1) {
        diff = rightspan - leftspan;
        if (diff <= 1 && diff >= -1) {
            break;
        }
        else {
            if (diff < 0) { leftspan--; rightspan++; horizontalShift--; } //left shift
            else { leftspan++; rightspan--; horizontalShift++; } //right shift
        }
    }

    while (1) {
        diff = downspan - upspan;
        if (diff <= 1 && diff >= -1) {
            break;
        }
        else {
            if (diff < 0) { upspan--; downspan++; verticalShift--; }
            else { downspan--; upspan++; verticalShift++; }
        }
    }

    //write the dungeon to map
    for (int i = 0; i < rooms.size(); i++)
        for (int j = 0; j < rooms[i].size(); j++) {
            POS p = rooms[i][j];
            map[p.x + borderUp + verticalShift][p.y + borderRight + horizontalShift] = p.c;
        }
}

void Generation::FillOutSpace() {
    std::vector<POS> emptyspace;
    for (int k = 0; k < HEIGHT - 2; k++)
        for (int j = 0; j < WIDTH - 2; j++) {
            if (map[k][j] == Tile_Wall && map[k][j + 1] == Tile_Empty && map[k - 1][j + 1] == Tile_Wall) {
                for (int i = j + 1; i < WIDTH - borderRight; i++)
                    if (map[k][i] == Tile_Wall) {
                        emptyspace.push_back(POS(k, j + 1));
                        isValidSpace = true;
                        TryFillOutSpacing(POS(k, j + 1), emptyspace);
                        for (POS p : emptyspace)
                            map[p.x][p.y] = Tile_Floor;
                        emptyspace.clear();
                    }
            }
        }
}

void Generation::TryFillOutSpacing(POS pos, std::vector<POS>& spacing) {
    OpenSpace(POS(pos.x - 1, pos.y), spacing);
    OpenSpace(POS(pos.x + 1, pos.y), spacing);
    OpenSpace(POS(pos.x, pos.y - 1), spacing);
    OpenSpace(POS(pos.x, pos.y + 1), spacing);
}

void Generation::OpenSpace(POS pos, std::vector<POS>& spacing) {
    if (!isValidSpace) return;

    if (map[pos.x][pos.y] == Tile_Wall)
        return;

    for(int i = spacing.size()-1; i > -1; i--)
        if (spacing[i] == pos) return;

    if (pos.x >= dungeonHeight || pos.y >= dungeonWidth || pos.x <= borderUp || pos.y <= borderLeft) {
        isValidSpace = false;
        spacing.clear();
        return;
    }

    spacing.push_back(pos);
    TryFillOutSpacing(pos, spacing);
}

void Generation::SpawnDoors() {
    POS p;

    for (int j = 0; j < connectpoints.size(); j++) { //vector
        for (int i = 0; i < connectpoints[j].size(); i++) { //array
            POS p;
            int c = -1;
            bool remove = false;

            do {
                if (c != -1)
                    connectpoints[j][i].erase(connectpoints[j][i].begin() + c);

                if (connectpoints[j][i].size() == 0)
                    break;
                
                c = rand() % (connectpoints[j][i].size()); //Is here a bug?!
                p = connectpoints[j][i][c];
                p.x += borderUp + verticalShift;
                p.y += borderRight + horizontalShift;

                if (map[p.x - 1][p.y] != Tile_Empty && map[p.x + 1][p.y] != Tile_Empty
                    && map[p.x][p.y - 1] != Tile_Empty && map[p.x][p.y + 1] != Tile_Empty
                    && map[p.x - 1][p.y - 1] != Tile_Empty && map[p.x + 1][p.y + 1] != Tile_Empty
                    && map[p.x + 1][p.y - 1] != Tile_Empty && map[p.x - 1][p.y + 1] != Tile_Empty) {

                    if(((i == 0 || i == 1) && map[p.x - 1][p.y] != Tile_Wall && map[p.x + 1][p.y] != Tile_Wall)
                        || ((i == 2 || i == 3) && map[p.x][p.y - 1] != Tile_Wall && map[p.x][p.y + 1] != Tile_Wall))
                    remove = true;
                }
                
            } while (!remove);
            
            if (remove)
                map[p.x][p.y] = Tile_Floor;
        }
    }
}

void Generation::CleanDungeon() {
    //removing the small space between walls
    for (int j = 0; j < HEIGHT; j++) {
        for (int i = 0; i < WIDTH; i++) {
            if (map[j][i] == Tile_Wall) {
                if (i + 2 < WIDTH && i - 1 >= 0 && map[j][i - 1] == Tile_Floor && map[j][i + 1] == Tile_Floor && map[j][i + 2] == Tile_Wall)
                    map[j][i] = Tile_Floor;

                else if (i - 2 >= 0 && i + 1 < WIDTH && map[j][i + 1] == Tile_Floor && map[j][i - 1] == Tile_Floor && map[j][i - 2] == Tile_Wall)
                    map[j][i] = Tile_Floor;

                else if (j + 2 < HEIGHT && j - 1 >= 0 && map[j - 1][i] == Tile_Floor && map[j + 1][i] == Tile_Floor && map[j + 2][i] == Tile_Wall)
                    map[j][i] = Tile_Floor;

                else if (j - 2 >= 0 && j + 1 < HEIGHT && map[j + 1][i] == Tile_Floor && map[j - 1][i] == Tile_Floor && map[j - 2][i] == Tile_Wall)
                    map[j][i] = Tile_Floor;
            }
        }
    }

    auto isTile = [&](int x, int y, TileType tile = Tile_Wall) {
        if (x >= 0 && y >= 0 && x < HEIGHT && y < WIDTH)
            return map[x][y] == tile;
        return false;
    };

    //removing the double walls

    //side note: can't remove the wall if on the diagonal side is a empty tile.
    //give a priority to remove the wall where a lonely wall is on the side. 

    auto CountFloorTiles = [&](int x, int y) {
        int n = 0;
        if (isTile(x, y - 1, Tile_Floor)) n++;
        if (isTile(x, y + 1, Tile_Floor)) n++;
        if (isTile(x - 1, y, Tile_Floor)) n++;
        if (isTile(x + 1, y, Tile_Floor)) n++;
        if (isTile(x + 1, y - 1, Tile_Floor)) n++;
        if (isTile(x + 1, y + 1, Tile_Floor)) n++;
        if (isTile(x - 1, y - 1, Tile_Floor)) n++;
        if (isTile(x - 1, y + 1, Tile_Floor)) n++;
        return n;
    };

    struct WallFloorsAmount {
        int floors;
        int x, y;
        WallFloorsAmount(int x, int y, int floors): x(x), y(y), floors(floors) {}
    };

    for (int x = 0; x < HEIGHT; x++) {
        for (int y = 0; y < WIDTH; y++) {
            if (map[x][y] != Tile_Wall) continue;
            int n = 0;

            if (isTile(x - 1, y)) n++;
            if (isTile(x, y + 1)) n++;
            if (isTile(x - 1, y + 1)) n++;
            
            if (n == 3) {
                std::vector<WallFloorsAmount> walls = {
                    WallFloorsAmount(x,y,CountFloorTiles(x,y)),
                    WallFloorsAmount(x-1,y,CountFloorTiles(x-1,y)),
                    WallFloorsAmount(x,y+1,CountFloorTiles(x,y+1)),
                    WallFloorsAmount(x-1,y+1,CountFloorTiles(x-1,y+1))
                };

                do {
                    walls.erase(walls.begin() + (walls[0].floors >= walls[1].floors ? 1 : 0));
                } while (walls.size() != 1);

          
                if (map[walls[0].x - 1][walls[0].y + 1] == Tile_Empty)
                    map[walls[0].x + 1][walls[0].y] = Tile_Floor;
                
                else if (map[walls[0].x + 1][walls[0].y + 1] == Tile_Empty)
                    map[walls[0].x - 1][walls[0].y + 1] = Tile_Floor;
                
                else if (map[walls[0].x + 1][walls[0].y - 1] == Tile_Empty)
                    map[walls[0].x + 1][walls[0].y] = Tile_Floor;
                
                else if (map[walls[0].x - 1][walls[0].y - 1] == Tile_Empty)
                    map[walls[0].x + 1][walls[0].y - 1] = Tile_Floor;
                
                else 
                    map[walls[0].x][walls[0].y] = Tile_Floor;
            }
        }
    }

    //removing the lonely walls
    for (int x = 0; x < HEIGHT; x++) {
        for (int y = 0; y < WIDTH; y++) {
            if (map[x][y] != Tile_Wall) continue;

            if (!isTile(x - 1, y) && !isTile(x, y + 1)
                && !isTile(x + 1, y) && !isTile(x, y - 1))
                map[x][y] = Tile_Floor;
        }
    }
}

void Generation::SpawnEntities() {
    //spawn player
    int playerRoom = rand() % rooms.size();
    POS p;
    do {
        int j = rand() % rooms[playerRoom].size();
        p = rooms[playerRoom][j];
    } while (map[p.x + borderUp + verticalShift][p.y + borderRight + horizontalShift] == Tile_Wall);
    map[p.x + borderUp + verticalShift][p.y + borderRight + horizontalShift] = Tile_Player;
    rooms.erase(rooms.begin() + playerRoom);


    //spawning enemies
    const int enemyDensity = 10 / 1;
    for (int j = 0; j < rooms.size(); j++) {
        int floors = 0;
        for (int i = 0; i < rooms[j].size(); i++) {
            POS p = POS(rooms[j][i].x + borderUp + verticalShift, rooms[j][i].y + borderRight + horizontalShift);
            if (map[p.x][p.y] != Tile_Wall)
                floors++;
        }

        int enemies = floors / enemyDensity;
        int eamount = 0;
        do {
            int i = rand() % rooms[j].size();
            POS p = POS(rooms[j][i].x + borderUp + verticalShift, rooms[j][i].y + borderRight + horizontalShift);
            if (map[p.x][p.y] == Tile_Floor) {
                eamount++;
                map[p.x][p.y] = Tile_Enemy;
            }
        } while (eamount < enemies);
    }
}

std::vector<std::string> Generation::get2DRenderMap() {
    //adding the right walls for 2D renderer
    std::vector<std::string> newMap = map;

    auto OutOfBounds = [&](POS pos) {
        return pos.y < 0 || pos.y > WIDTH || pos.x < 0 || pos.x > HEIGHT;
    };

    for (int x = 0; x < HEIGHT; x++) {
        for (int y = 0; y < WIDTH; y++) {
            if (map[x][y] == Tile_Wall) {
                if (!OutOfBounds(POS(x - 1, y)) && map[x - 1][y] == Tile_Wall &&
                    !OutOfBounds(POS(x + 1, y)) && map[x + 1][y] == Tile_Wall &&
                    !OutOfBounds(POS(x, y - 1)) && map[x][y - 1] == Tile_Wall &&
                    !OutOfBounds(POS(x, y + 1)) && map[x][y + 1] == Tile_Wall) {
                    newMap[x][y] = WX;
                    continue;
                }

                else if (!OutOfBounds(POS(x + 1, y)) && map[x + 1][y] == Tile_Wall &&
                    !OutOfBounds(POS(x - 1, y)) && map[x - 1][y] == Tile_Wall &&
                    !OutOfBounds(POS(x, y + 1)) && map[x][y + 1] == Tile_Wall) {
                    newMap[x][y] = WDRU;
                    continue;
                }

                else if (!OutOfBounds(POS(x + 1, y)) && map[x + 1][y] == Tile_Wall &&
                    !OutOfBounds(POS(x - 1, y)) && map[x - 1][y] == Tile_Wall &&
                    !OutOfBounds(POS(x, y - 1)) && map[x][y - 1] == Tile_Wall) {
                    newMap[x][y] = WDLU;
                    continue;
                }

                else if (!OutOfBounds(POS(x + 1, y)) && map[x + 1][y] == Tile_Wall &&
                    !OutOfBounds(POS(x, y + 1)) && map[x][y + 1] == Tile_Wall &&
                    !OutOfBounds(POS(x, y - 1)) && map[x][y - 1] == Tile_Wall) {
                    newMap[x][y] = WRDL;
                    continue;
                }

                else if (!OutOfBounds(POS(x - 1, y)) && map[x - 1][y] == Tile_Wall &&
                    !OutOfBounds(POS(x, y + 1)) && map[x][y + 1] == Tile_Wall &&
                    !OutOfBounds(POS(x, y - 1)) && map[x][y - 1] == Tile_Wall) {
                    newMap[x][y] = WRUL;
                    continue;
                }

                else if (!OutOfBounds(POS(x - 1, y)) && map[x - 1][y] == Tile_Wall &&
                    !OutOfBounds(POS(x + 1, y)) && map[x + 1][y] == Tile_Wall) {
                    newMap[x][y] = WV;
                }

                else if (!OutOfBounds(POS(x, y - 1)) && map[x][y - 1] == Tile_Wall &&
                    !OutOfBounds(POS(x, y + 1)) && map[x][y + 1] == Tile_Wall) {
                    newMap[x][y] = WH;
                }

                else if (
                    !OutOfBounds(POS(x + 1, y)) && map[x + 1][y] == Tile_Wall &&
                    !OutOfBounds(POS(x, y - 1)) && map[x][y - 1] == Tile_Wall) {
                    newMap[x][y] = WDL;
                }

                else if (!OutOfBounds(POS(x + 1, y)) && map[x + 1][y] == Tile_Wall &&
                    !OutOfBounds(POS(x, y + 1)) && map[x][y + 1] == Tile_Wall) {
                    newMap[x][y] = WDR;
                }

                else if (!OutOfBounds(POS(x - 1, y)) && map[x - 1][y] == Tile_Wall &&
                    !OutOfBounds(POS(x, y - 1)) && map[x][y - 1] == Tile_Wall) {
                    newMap[x][y] = WLU;
                }

                else if (!OutOfBounds(POS(x - 1, y)) && map[x - 1][y] == Tile_Wall &&
                    !OutOfBounds(POS(x, y + 1)) && map[x][y + 1] == Tile_Wall) {
                    newMap[x][y] = WRU;
                }

                else if (!OutOfBounds(POS(x - 1, y)) && map[x - 1][y] == Tile_Wall) {
                    newMap[x][y] = WU;
                }

                else if (!OutOfBounds(POS(x + 1, y)) && map[x + 1][y] == Tile_Wall) {
                    newMap[x][y] = WD;
                }

                else if (!OutOfBounds(POS(x, y - 1)) && map[x][y - 1] == Tile_Wall) {
                    newMap[x][y] = WL;
                }

                else if (!OutOfBounds(POS(x, y + 1)) && map[x][y + 1] == Tile_Wall) {
                    newMap[x][y] = WR;
                }
            }
        }
    }
    return newMap;
}
