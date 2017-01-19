/** Assignment: Harry Potter and the Final Quest
 *  Final Project for the Object Oriented Programming Lab.
 *
 *  DEVELOPER:                  John Bracciano
 *  TERMINAL CONTROL LIBRARY:   NCurses
 */

#ifndef KEY_RETURN
#define KEY_RETURN     0xD
#endif
#ifndef KEY_ESCAPE
#define KEY_ESCAPE     0x1B
#endif
#ifndef KEY_PAUSE
#define KEY_PAUSE      0x3E
#endif

#define PAUSE_MESSAGE "Paused"
#define NICKNAME_DEFAULT_LENGTH 11
#define DIAMONDS_DEFAULT_COUNT 10
#define MONSTER_MAP_THRESSHOLD 9
#define MENU_ITEMS_COUNT 3
#define SLC_QUIT 2

#define COLOR_PAIR_NORMAL           1
#define COLOR_PAIR_GREEN_BLACK      2
#define COLOR_PAIR_YELLOW_BLACK     3
#define COLOR_PAIR_BLACK_RED        4
#define COLOR_PAIR_BLACK_BLUE       5
#define COLOR_PAIR_BLACK_YELLOW     6

#define UP       0x01
#define RIGHT    0x02
#define DOWN     0x04
#define LEFT     0x08

#include <ncurses.h>
#include <string.h>
#include <vector>
#include <fstream>
#include <sstream>
#include <math.h>
#include <algorithm>
#include <time.h>

typedef unsigned char   UI8;
typedef unsigned short  UI16;
typedef unsigned int    UI32;
typedef char    I8;
typedef short   I16;
typedef int     I32;

/**
 *  COLL_T
 *  Defines an enumaration type with all the possible collision types.
 */
typedef enum
{
    NONE, WALL, DMND, MONSTER, PARCH
}   COLL_T;

/**
 *  STRUCT coords AS POS
 *  @brief      Defines a struct that holds a point (position) in a 2-Dimensional
 *              coordinate system.
 */
typedef struct coords
{
    UI8 x;
    UI8 y;

    coords() : x(0), y(0)
    {}

    coords(int coordx, int coordy) : x(coordx), y(coordy)
    {}

    bool operator == (const struct coords& arg)
    { return ((x == arg.x) && (y == arg.y)); }
    bool operator != (const struct coords& arg)
    { return (*this) == arg; }
} POS;

/**
 *  STRUCT score_struct AS SCOS
 *  @brief      Defines a struct to be used as a score object.
 *              The score object containes all the necessary information
 *              about a player's score.
 */
typedef struct score_struct
{
    UI32 player_score;
    I8 player_name[NICKNAME_DEFAULT_LENGTH];

    score_struct(void) : player_score(), player_name()
    {}

    score_struct(UI32& score, const I8 name[]) : player_score(score)
    { strcpy(player_name, name); }

    bool operator < (const struct score_struct& arg) const
    { return player_score < arg.player_score; }
    bool operator > (const struct score_struct& arg) const
    { return player_score > arg.player_score; }
}   SCOS;

/**
 *  STRUCT file_nfound_exc AS FILEEXP
 *  @brief      Defines a struct to be used as an exception when a file
 *              operation goes bad.
 */
typedef struct file_nfound_exc
{
    std::string filename;
    std::string open_purpose;

    file_nfound_exc(std::string arg1, std::string arg2):
        filename(arg1), open_purpose(arg2)
    {}
} FILEEXP;

/**
 *  STRUCT general_exc AS GENEXP
 *  @brief      Defines a struct to be used as a general purpose exception.
 */
typedef struct general_exc
{
    std::string message;
    general_exc(std::string arg): message(arg)
    {}
} GENEXP;

#ifndef LIVING_H_INCLUDED
#define LIVING_H_INCLUDED

/**
 *  CLASS: Living
 *  @brief      This is the main abstract class from which all the "living"
 *              objects in the game inherit (the player and the monsters).
 */
class Living
{
    public:
    UI8 CurX(void)      const   { return xypos.x; }
    UI8 CurY(void)      const   { return xypos.y; }
    POS CurPos(void)    const   { return xypos; }

    void SetX(UI8 x)       { xypos.x = x; }
    void SetY(UI8 y)       { xypos.y = y; }
    void SetPos(POS pos)   { xypos = pos; }

    void MoveDown(void)     { xypos.y++; }
    void MoveUp(void)       { xypos.y--; }
    void MoveRight(void)    { xypos.x++; }
    void MoveLeft(void)     { xypos.x--; }

    protected:
    Living() : xypos({0,0})
    {}
    // Having the constructor defined as "protected"
    // we manage to keep the class abstract

    POS xypos;
};

#endif // LIVING_H_INCLUDED

#ifndef POTTER_H_INCLUDED
#define POTTER_H_INCLUDED

/**
 *  CLASS: Potter
 *  @brief      Potter's instance is the main player of the game.
 *              It inherits from class Living the basic operations. Class Potter
 *              also contains two inner structs, win_exc and lose_exc. Their
 *              purpose is to create objects to be thrown as exceptions when the
 *              player either wins or loses.
 */
class Potter : public Living
{
    public:
    Potter(std::string _name) : Living(),
                                player_name(_name),
                                player_score(0),
                                collision_state(COLL_T::NONE)
    {}

    typedef struct win_exc  // win_exc struct to be used as an exception when the player wins a level
    {
        POS win_pos;
        win_exc(const POS& coords) : win_pos(coords)
        {}
    }   Win;

    typedef struct lose_exc // lose_exc struct to be used as an exception when the player loses
    {
        POS lose_pos;
        Living* monst;
        lose_exc(const POS& coords) : lose_pos(coords), monst(NULL)
        {}
    }   Lose;

    COLL_T CollisionState(void)     const   { return collision_state; }
    void CollisionState(COLL_T _state)      { collision_state = _state; }

    const std::string& Name(void)   const   { return player_name; }
    void Name(const std::string& _name)     { player_name = _name; }

    UI32 Score(void)    const   { return player_score; }
    void Score(UI32 _score)     { player_score = _score; }
    void AddToScore(UI32 _aval) { player_score += _aval; }

    private:
    std::string player_name;
    UI32 player_score;
    COLL_T collision_state;
};

#endif // POTTER_H_INCLUDED

#ifndef MONSTER_H_INCLUDED
#define MONSTER_H_INCLUDED

/**
 *  CLASS: Monster
 *  @brief      This is the class used to create the monsters of the game (gnome and traal).
 *              The monsters hold a movement map, helping them to make better decisions about
 *              where on the map to go next.
 */
class Monster : public Living
{
    friend class Engine;

    public:
    Monster() : Living(), moves(0), prev_pos({0,0})
    {}

    UI8 Moves(UI8 move)  const   { return moves & move; }
    void SetMoves(UI8 _moves)     { moves = _moves; }
    void ResetMoves(void)   { moves = 0; }

    POS PrevPos(void)   const   { return prev_pos; }
    UI8 PrevX(void)     const   { return prev_pos.x; }
    UI8 PrevY(void)     const   { return prev_pos.y; }
    void PrevPos(POS set)  { prev_pos = set; }

    private:
    UI8 moves;
    POS prev_pos;
    UI32** move_map;

    /** PRIVATE MEMBER FUNCTION: CalculateShortest
     *  @brief      Calculates the best movement to be made byt the monster in order to
     *              get closer to the position given as a parametre.
     *  @param      coords: The cordinates of the referenced position.
     */
    void CalculateShortest(POS coords)
    {
        if (coords.y > xypos.y)     moves |= DOWN;
        else if (coords.y < xypos.y) moves |= UP;

        if (coords.x > xypos.x)     moves |= RIGHT;
        else if (coords.x < xypos.x) moves |= LEFT;
    }
};

#endif // MONSTER_H_INCLUDED

#ifndef SCOREPLAY_H_INCLUDED
#define SCOREPLAY_H_INCLUDED

/**
 *  CLASS: HighScore
 *  @brief      HighScore is used to retrieve and keep information about
 *              the game's high score table.
 */
class HighScore
{
    public:
    void InitTable(void);
    void SaveTable(void);
    void EmptyTable(void) { score_table.erase(score_table.begin(), score_table.end()); }
    void SortTable(void) { sort(score_table.rbegin(), score_table.rend()); }

    std::string PlayerName(void)    const       { return cur_player_name; }
    std::string& PlayerName(void)               { return cur_player_name; }
    void PlayerName(const std::string& name)    { cur_player_name = name; }

    const std::vector<SCOS>& ScoreTable(void)   const   { return score_table; }

    /**
     *  PUBLIC MEMBER OVERLOADED OPERATOR HighScore::operator <<
     *  @brief      Accepts a score integer as an argument and if the player name
     *              already exists on the high score table, it is assigned the
     *              score, else a new table record is created.
     */
    void friend operator << (HighScore& self, UI32 score)
    {
        std::vector<SCOS>::iterator existance;

        for (existance = self.score_table.begin(); existance != self.score_table.end(); existance++)
            if (strcmp((*existance).player_name,self.cur_player_name.c_str()) == 0)
                break;

        if (existance == self.score_table.end())
            self.score_table.push_back(SCOS(score, self.cur_player_name.c_str()));
        else if ((*existance).player_score < score)
                (*existance).player_score = score;
    }

    private:
    std::vector<SCOS> score_table;
    std::string cur_player_name;
    std::ifstream score_in;
    std::ofstream score_out;
};

#endif  // SCORE_H_INCLUDED

/**
 *  PUBLIC MEMBER FUNCTION HighScore::InitTable
 *  @brief      Initiates the high score table of the game retrieving the
 *              data from the file specified within the corresponding class
 *              of the function.
 */
void HighScore::InitTable(void)
{
    if (score_table.size() > 0) return;    // If The file is already open we return doing nothing

    score_in.open("scores", std::ios::in | std::ios::binary);
    if (!score_in) throw FILEEXP("scores", "input");

    SCOS tmp_score;
    UI8 buf_size = sizeof(SCOS);

    while (score_in.read((char*)&tmp_score, buf_size), score_in.good())
        score_table.push_back(tmp_score);

    score_in.close();
}

/**
 *  PUBLIC MEMBER FUNCTION HighScore::SaveTable
 *  @brief      Writes the high score table in the file specified within
 *              the corresponding class of the function.
 */
void HighScore::SaveTable(void)
{
    score_out.open("scores", std::ios::out | std::ios::binary);
    if (!score_out) throw FILEEXP("scores", "output");

    UI8 buf_size = sizeof(SCOS);
    SCOS tmp_score;

    for (std::vector<SCOS>::const_iterator it = score_table.begin(); it != score_table.end(); it++)
        score_out.write((char*)&(*it), buf_size);

    score_out.close();
}

#ifndef STAGE_H_INCLUDED
#define STAGE_H_INCLUDED

/**
 *  CLASS: Stage
 *  @brief      Stage is the main part of the game where all the living
 *              creatures exist and react. Stage holds the maze and the
 *              soulless objects.
 */
class Stage
{
    friend class Engine;

    public:
    Stage();

    UI8 MapHeight(void)     const       { return map_h; }
    UI8 MapWidth(void)      const       { return map_w; }
    POS ParchPos(void)      const       { return parch_pos; }
    UI8 DiamondsCount(void) const       { return diamonds_count; }
    void DiamondsCount(UI8 _count)      { diamonds_count = _count; }

    bool EraseDiamond(POS);

    void Load(std::ifstream&);
    void Unload(void);
    const std::vector<I8*>& Map() const { return map; }

    private:
    UI8 map_w, map_h, diamonds_count;
    POS parch_pos;
    std::vector<I8*> map;

    void PopDmnds(void);
    void PlaceParchment(void);
};

#endif // STAGE_H_INCLUDED

/* CLASS STAGE PRIVATE MEMBER DEFINITIONS */
/**
 *  PRIVATE MEMBER FUNCTION Stage::PopDmnds
 *  @brief  Populates the map with diamonds, represented as dots ('.')
 */
void Stage::PopDmnds(void)
{
    UI8 dx, dy;
    srand(time(NULL));

    for (UI8 cc = 0; cc < DIAMONDS_DEFAULT_COUNT; cc++)
    {
        do
        {
            dx = rand() % (map_w - 1);
            dy = rand() % (map_h - 1);
        } while (map[dy][dx] != ' ');

        map[dy][dx] = '.';
    }
}

/**
 *  PRIVATE MEMBER FUNCTION Stage::PlaceParchment
 *  @brief  Places the parchment on the map required for Harry to complete the level.
 *          The parchment is represented with the character 'P'.
 */
 void Stage::PlaceParchment(void)
 {
     UI8 dx, dy;

     do
     {
        dx = rand() % (map_w - 1);
        dy = rand() % (map_h - 1);
     } while (map[dy][dx] != ' ');

     parch_pos = {dx, dy};
 }

/* CLASS STAGE PUBLIC MEMBER DEFINITIONS */
/**
 *  PUBLIC DEFAULT CONTRUCTOR Stage
 *  @brief Initialises the private members needed for the object to be valid.
 */
Stage::Stage() : map_w(0), map_h(0),
                 diamonds_count(DIAMONDS_DEFAULT_COUNT)
{} //Stage::Stage()

/**
 *  PUBLIC MEMBER FUNCTION Stage::Load
 *  @brief  Loads the stage map (maze) from the given input file stream
 *  @param  mapdata: The input file stream
 *  @note   If a map was previously loaded it must be unloaded with Stage::Unload
 *          before a new call of this function occurs
 */
void Stage::Load(std::ifstream& mapdata)
{
    // ifstream must point to some file and
    // object's width and height must be set to 0
    if (mapdata == NULL)
        throw GENEXP("General error in Stage::Load:\nCould not load map file");

    if (map_w != 0 || map_h != 0)
        throw GENEXP("General error in Stage::Load:\nInvalid initial class values. You need to call Unload first.");

    I8 *buf, aux_ch = 0;
    std::stringstream ss;

    // File must contain only asterisks ('*') and new line characters to be valid
    // If not, return an error report state showing an invalid file
    // If valid, store the character and proceed to reading the next ones
    while (aux_ch != '\n')
    {
        mapdata.read(&aux_ch, 1);

        if (aux_ch != '*' &&
            aux_ch != '\n')
            throw GENEXP("General error in Stage::Load:\nInvalid data were found in map file");

        ss << aux_ch;
        map_w++;
    }

    buf = new I8[map_w + 1];
    ss.getline(buf, map_w);
    map.push_back(buf);
    map_h++;

    buf = new I8[map_w + 1];
    while (mapdata.read(buf, map_w), mapdata.good()) // Many thanks to GMeles for this good piece of code
    {
        // Each line read must have exactly the same width as the ones above it.
        // If not then an error report state is returned, showing invalid file.
        // If everything is OK and we haven't reached the end of the file
        // continue reading lines.
        if (buf[map_w - 1]  != '\n' &&
            buf[map_w - 2]  != '*' &&
            buf[0]          != '*')
            throw GENEXP("General error in Stage::Load:\nInvalid data were found in map file");

        map.push_back(buf);
        buf = new I8[map_w + 1];
        map_h++;
    }

    // Decreasing the map's width to represent the real width of the mase
    // (without the '\0' character)
    map_w--;

    //Populating the map with the diamonds and placing the parchment
    PopDmnds();
    PlaceParchment();
} // Stage::Load()

/**
 *  PUBLIC MEMBER FUNCTION Stage::Unload
 *  @brief  Unloads the stage map (maze) and sets the object appropriately to load new map.
 *  @note   This specific function must be called after every Stage::Load call in order to
 *          load a new map.
 */
void Stage::Unload(void)
{
    std::vector<I8*>::iterator iter = map.begin();

    for (int i = 0; i < map_h; i++)
    {
        delete[] map[0];
        map.erase(iter);
    }

    map_h = map_w = 0;
} // Stage::Unload

/**
 *  PUBLIC MEMBER FUNCTION Stage::EraseDiamond
 *  @brief  Erases the diamond at the position given as a parametre.
 *  @param  coords: The position of the diamond to erase.
 */
bool Stage::EraseDiamond(POS coords)
{
    if (map[coords.y][coords.x] == '.')
    {
        map[coords.y][coords.x] = ' ';
        diamonds_count--;
        return true;
    }

    return false;
} // EraseDiamonds

#ifndef ENGINE_H_INCLUDED
#define ENGINE_H_INCLUDED

/**
 *  CLASS: Engine
 *  @brief      Engine controls every basic aspect of the game;
 *              be that the monsters' and player's moves, collisions
 *              or the proper initialisation of levels. It also holds an
 *              inner struct for use as an exception when the user presses
 *              the escape key to end the current game.
 */
class Engine
{
    public:
    Engine() : player("Player 1"), thres(MONSTER_MAP_THRESSHOLD)
    {}

    typedef struct esc_struct
    {   // Used as exception for when the user presses the escape key
        std::string esc_reason;

        esc_struct(const std::string& _reason) : esc_reason(_reason)
        {}
    }   Escape;

    void InitLevel(std::ifstream&);
    void EndLevel(void);
    COLL_T CheckMapCollision(POS);

    void NewMove(Living*, I32);
    void NewSmartMove(Monster*);
    void NewDummyMove(Monster*);

    Stage stage;
    Potter player;
    Monster gnome;
    Monster traal;

    private:
    UI8 thres;
    void InitPos(void);
    void InitMoveMaps(void);
    void DestroyMoveMaps(void);
};

#endif // ENGINE_H_INCLUDED

/* CLASS ENGINE PRIVATE MEMBER DEFINITIONS */
/**
 *  PRIVATE MEMBER FUNCTION Engine::InitPos
 *  @brief  Initialises the positions of the living creatures on the
 *          map. Creatures are positioned randomly and monsters cannot
 *          be placed on player's initial position.
 */
void Engine::InitPos(void)
{
    UI8 dx = 0, dy = 0;
    srand(time(NULL));

    // Positioning Harry
    do
    {
        dx = rand() % (stage.MapWidth() - 1);
        dy = rand() % (stage.MapHeight() - 1);
    } while (stage.map[dy][dx] != ' ');

    player.SetX(dx);
    player.SetY(dy);

    // Positioning Gnome
    do
    {
        dx = rand() % (stage.MapWidth() - 1);
        dy = rand() % (stage.MapHeight() - 1);
    } while (stage.map[dy][dx] != ' ' || POS(dx, dy) == player.CurPos());

    gnome.SetX(dx);
    gnome.SetY(dy);

    // Positioning Traal
    do
    {
        dx = rand() % (stage.MapWidth() - 1);
        dy = rand() % (stage.MapHeight() - 1);
    } while (stage.map[dy][dx] != ' ' || POS(dx, dy) == player.CurPos());

    traal.SetX(dx);
    traal.SetY(dy);

    gnome.PrevPos(POS(0,0));
    traal.PrevPos(POS(0,0));
}

/**
 *  PRIVATE MEMBER FUNCTION Engine::InitMoveMaps
 *  @brief  Initialises movement map arrays of the monsters.
 */
void Engine::InitMoveMaps(void)
{
        gnome.move_map = new UI32*[stage.MapHeight()];
        traal.move_map = new UI32*[stage.MapHeight()];

        for (UI8 i = 0; i < stage.MapHeight(); i++)
        {
            gnome.move_map[i] = new UI32[stage.MapWidth()];
            traal.move_map[i] = new UI32[stage.MapWidth()];
        }

        for (UI8 i = 0; i < stage.MapHeight(); i++)
            for (UI8 j = 0; j < stage.MapWidth(); j++)
            {
                gnome.move_map[i][j] = 0;
                traal.move_map[i][j] = 0;
            }
}

/**
 *  PRIVATE MEMBER FUNCTION Engine::DestroyMoveMaps
 *  @brief  Dealocates the memory occupied by the monsters' movement maps.
 */
void Engine::DestroyMoveMaps(void)
{
    delete[] gnome.move_map;
    delete[] traal.move_map;
}

/* CLASS ENGINE PUBLIC MEMBER DEFINITIONS */
/**
 *  PUBLIC MEMBER FUNCTION Engine::InitLevel
 *  @brief  Performs all the necessary actions to set a game level,
 *          according to the map file passed as argument.
 *  @param  mapdata: The input filestream that points to the file
 *                   containing a valid map.
 */
void Engine::InitLevel(std::ifstream& mapdata)
{
    stage.Load(mapdata);
    InitMoveMaps();
    InitPos();
}

/**
 *  PUBLIC MEMBER FUNCTION Engine::EndLevel
 *  @brief  Performs all the necessary actions to end a game level,
 */
void Engine::EndLevel(void)
{
    DestroyMoveMaps();

    stage.Unload();
    stage.DiamondsCount(10);

    player.SetPos(POS());
    gnome.SetPos(POS());
    traal.SetPos(POS());
}

/**
 *  PUBLIC MEMBER FUNCTION Engine::CheckMapCollision
 *  @brief  Calculates whether a given Living creature collides with another
 *          object on the stage.
 *  @param  liv_pos: The position that needs to be checked for collisions.
 */
COLL_T Engine::CheckMapCollision(POS liv_pos)
{
    I8 block = stage.map[liv_pos.y][liv_pos.x];

    switch (block)
    {
        case '.':   return COLL_T::DMND;
        case '*':   return COLL_T::WALL;
    }

    if (stage.DiamondsCount() == 0)
        if (liv_pos == stage.parch_pos)
            return COLL_T::PARCH;

    return COLL_T::NONE;
}


/**
 *  PUBLIC MEMBER FUNCTION Engine::NewMove
 *  @brief  It moves a creature to a new position according to the key provided,
 *          only if  no wall collisions occur!
 *  @param  creature: The creature to be moved.
 *  @param  key: Value representing the direction of movement.
 */
void Engine::NewMove(Living* creature, I32 key)
{
    POS mvpos = creature->CurPos();

    switch(key)
    {
        case KEY_UP:
        mvpos.y--;
        if ((CheckMapCollision(mvpos)) != COLL_T::WALL)
            creature->MoveUp();

        break;

        case KEY_DOWN:
        mvpos.y++;
        if ((CheckMapCollision(mvpos)) != COLL_T::WALL)
            creature->MoveDown();

        break;

        case KEY_LEFT:
        mvpos.x--;
        if ((CheckMapCollision(mvpos)) != COLL_T::WALL)
            creature->MoveLeft();

        break;

        case KEY_RIGHT:
        mvpos.x++;
        if ((CheckMapCollision(mvpos)) != COLL_T::WALL)
            creature->MoveRight();

        break;
    }
}

/**
 *  PUBLIC MEMBER FUNCTION Engine::NewSmartMove
 *  @brief  This may be the trickiest function of all! It decides the movement of a monster
 *          smartly enough to be able to track Harry's position. If you don't understand
 *          it's logic, be glad it works!!!
 *  @param  creature: The creature to be moved.
 */
void Engine::NewSmartMove(Monster* creature)
{

    COLL_T collision;
    bool monster_moved = false;

    POS toup = { (UI8)(creature->CurX()),
                 (UI8)(creature->CurY() - 1) };
    POS toright = { (UI8)(creature->CurX() + 1),
                    (UI8)(creature->CurY()) };
    POS todown = { (UI8)(creature->CurX()),
                   (UI8)(creature->CurY() + 1) };
    POS toleft = { (UI8)(creature->CurX() - 1),
                   (UI8)(creature->CurY()) };

    if (thres > 0)
    {   // Level 1
        creature->CalculateShortest(player.CurPos());

        if (((collision = CheckMapCollision(toup)) != COLL_T::WALL) && (creature->move_map[toup.y][toup.x] == 0))
        {
            if (creature->Moves(UP))
                {
                    creature->move_map[creature->PrevY()][creature->PrevX()]  = 0;
                    creature->PrevPos(creature->CurPos());
                    creature->move_map[creature->CurY()][creature->CurX()] = 1;
                    creature->MoveUp();
                    thres--;
                    monster_moved = true;
                }
        }

        if (((collision = CheckMapCollision(toright)) != COLL_T::WALL) && (creature->move_map[toright.y][toright.x] == 0))
        {
            if (creature->Moves(RIGHT) && !monster_moved)
                {
                    creature->move_map[creature->PrevY()][creature->PrevX()]  = 0;
                    creature->PrevPos(creature->CurPos());
                    creature->move_map[creature->CurY()][creature->CurX()] = 1;
                    creature->MoveRight();
                    thres--;
                    monster_moved = true;
                }
        }

        if (((collision = CheckMapCollision(toleft)) != COLL_T::WALL) && (creature->move_map[toleft.y][toleft.x] == 0))
        {
            if (creature->Moves(LEFT) && !monster_moved)
                {
                    creature->move_map[creature->PrevY()][creature->PrevX()]  = 0;
                    creature->PrevPos(creature->CurPos());
                    creature->move_map[creature->CurY()][creature->CurX()] = 1;
                    creature->MoveLeft();
                    thres--;
                    monster_moved = true;
                }
        }

        if (((collision = CheckMapCollision(todown)) != COLL_T::WALL) && (creature->move_map[todown.y][todown.x] == 0))
        {
            if (creature->Moves(DOWN) && !monster_moved)
                {
                    creature->move_map[creature->PrevY()][creature->PrevX()]  = 0;
                    creature->PrevPos(creature->CurPos());
                    creature->move_map[creature->CurY()][creature->CurX()] = 1;
                    creature->MoveDown();
                    thres--;
                    monster_moved = true;
                }
        }

        creature->ResetMoves();
    }   // Level 1

    if (!monster_moved && thres > 0)
    {   // Level 2
        if (((collision = CheckMapCollision(toup)) != COLL_T::WALL) && (creature->move_map[toup.y][toup.x] == 0))
        {
            creature->move_map[creature->PrevY()][creature->PrevX()]  = 0;
            creature->PrevPos(creature->CurPos());
            creature->move_map[creature->CurY()][creature->CurX()] = 1;
            creature->MoveUp();
            monster_moved = true;
        }

        if (((collision = CheckMapCollision(toright)) != COLL_T::WALL) && (creature->move_map[toright.y][toright.x] == 0))
        {
            if (!monster_moved)
            {
                creature->move_map[creature->PrevY()][creature->PrevX()]  = 0;
                creature->PrevPos(creature->CurPos());
                creature->move_map[creature->CurY()][creature->CurX()] = 1;
                creature->MoveRight();
                monster_moved = true;
            }
        }

        if (((collision = CheckMapCollision(todown)) != COLL_T::WALL) && (creature->move_map[todown.y][todown.x] == 0))
        {
            if (!monster_moved)
            {
                creature->move_map[creature->PrevY()][creature->PrevX()]  = 0;
                creature->PrevPos(creature->CurPos());
                creature->move_map[creature->CurY()][creature->CurX()] = 1;
                creature->MoveDown();
                monster_moved = true;
            }
        }

        if (((collision = CheckMapCollision(toleft)) != COLL_T::WALL) && (creature->move_map[toleft.y][toleft.x] == 0))
        {
            if (!monster_moved)
            {
                creature->move_map[creature->PrevY()][creature->PrevX()]  = 0;
                creature->PrevPos(creature->CurPos());
                creature->move_map[creature->CurY()][creature->CurX()] = 1;
                creature->MoveLeft();
                monster_moved = true;
            }
        }
    }   // Level 2

    if (!monster_moved)
    {   // Level 3
        if (((collision = CheckMapCollision(todown)) != COLL_T::WALL))
        {
            creature->move_map[creature->PrevY()][creature->PrevX()]  = 0;
            creature->PrevPos(creature->CurPos());
            creature->move_map[creature->CurY()][creature->CurX()] = 1;
            creature->MoveDown();
            monster_moved = true;
        }

        if (((collision = CheckMapCollision(toleft)) != COLL_T::WALL))
        {
            if (!monster_moved)
            {
                creature->move_map[creature->PrevY()][creature->PrevX()]  = 0;
                creature->PrevPos(creature->CurPos());
                creature->move_map[creature->CurY()][creature->CurX()] = 1;
                creature->MoveLeft();
                monster_moved = true;
            }
        }

        if (((collision = CheckMapCollision(toright)) != COLL_T::WALL))
        {
            if (!monster_moved)
            {
                creature->move_map[creature->PrevY()][creature->PrevX()]  = 0;
                creature->PrevPos(creature->CurPos());
                creature->move_map[creature->CurY()][creature->CurX()] = 1;
                creature->MoveRight();
                monster_moved = true;
            }
        }

        if (((collision = CheckMapCollision(toup)) != COLL_T::WALL))
        {
            if (!monster_moved)
            {
                creature->move_map[creature->PrevY()][creature->PrevX()]  = 0;
                creature->PrevPos(creature->CurPos());
                creature->move_map[creature->CurY()][creature->CurX()] = 1;
                creature->MoveUp();
                monster_moved = true;
            }
        }

        if (!thres) thres = MONSTER_MAP_THRESSHOLD;  // The higher the thresshold is set, the longest the gnome maps Harry!
    }   // Level 3
}   // Engine::NewSmartMove

/**
 *  PUBLIC MEMBER FUNCTION Engine::NewDummyMove
 *  @brief  This function is used to move a Monster randomly. The algorithm
 *          does not allow the creature to get stuck in one route.
 *  @param  creature: The creature to be moved.
 */
void Engine::NewDummyMove(Monster* creature)
{

    COLL_T collision;
    bool monster_moved = false;

    POS toup = { (UI8)(creature->CurX()),
                 (UI8)(creature->CurY() - 1) };
    POS toright = { (UI8)(creature->CurX() + 1),
                    (UI8)(creature->CurY()) };
    POS todown = { (UI8)(creature->CurX()),
                   (UI8)(creature->CurY() + 1) };
    POS toleft = { (UI8)(creature->CurX() - 1),
                   (UI8)(creature->CurY()) };

    if (1)
    {
        if (((collision = CheckMapCollision(toup)) != COLL_T::WALL) && (creature->move_map[toup.y][toup.x] < creature->move_map[todown.y][todown.x]))
        {
            creature->PrevPos(creature->CurPos());
            creature->move_map[creature->CurY()][creature->CurX()]++;
            creature->MoveUp();
            monster_moved = true;
        }

        if (((collision = CheckMapCollision(todown)) != COLL_T::WALL) && (creature->move_map[todown.y][todown.x] < creature->move_map[toup.y][toup.x]))
        {
            if (!monster_moved)
            {
                creature->PrevPos(creature->CurPos());
                creature->move_map[creature->CurY()][creature->CurX()]++;
                creature->MoveDown();
                monster_moved = true;
            }
        }

        if (((collision = CheckMapCollision(toright)) != COLL_T::WALL) && (creature->move_map[toright.y][toright.x] < creature->move_map[toleft.y][toleft.x]))
        {
            if (!monster_moved)
            {
                creature->PrevPos(creature->CurPos());
                creature->move_map[creature->CurY()][creature->CurX()]++;
                creature->MoveRight();
                monster_moved = true;
            }
        }

        if (((collision = CheckMapCollision(toleft)) != COLL_T::WALL) && (creature->move_map[toleft.y][toleft.x] < creature->move_map[toright.y][toright.x]))
        {
            if (!monster_moved)
            {
                creature->PrevPos(creature->CurPos());
                creature->move_map[creature->CurY()][creature->CurX()]++;
                creature->MoveLeft();
                monster_moved = true;
            }
        }
    }   // Level 1


    if (!monster_moved)
    {
        if (((collision = CheckMapCollision(toup)) != COLL_T::WALL) && (creature->move_map[toup.y][toup.x] < creature->move_map[creature->PrevY()][creature->PrevX()]))
        {
            creature->PrevPos(creature->CurPos());
            creature->move_map[creature->CurY()][creature->CurX()]++;
            creature->MoveUp();
            monster_moved = true;
        }

        if (((collision = CheckMapCollision(toright)) != COLL_T::WALL) && (creature->move_map[toright.y][toright.x] < creature->move_map[creature->PrevY()][creature->PrevX()]))
        {
            if (!monster_moved)
            {
                creature->PrevPos(creature->CurPos());
                creature->move_map[creature->CurY()][creature->CurX()]++;
                creature->MoveRight();
                monster_moved = true;
            }
        }

        if (((collision = CheckMapCollision(todown)) != COLL_T::WALL) && (creature->move_map[todown.y][todown.x] < creature->move_map[creature->PrevY()][creature->PrevX()]))
        {
            if (!monster_moved)
            {
                creature->PrevPos(creature->CurPos());
                creature->move_map[creature->CurY()][creature->CurX()]++;
                creature->MoveDown();
                monster_moved = true;
            }
        }

        if (((collision = CheckMapCollision(toleft)) != COLL_T::WALL) && (creature->move_map[toleft.y][toleft.x] < creature->move_map[creature->PrevY()][creature->PrevX()]))
        {
            if (!monster_moved)
            {
                creature->PrevPos(creature->CurPos());
                creature->move_map[creature->CurY()][creature->CurX()]++;
                creature->MoveLeft();
                monster_moved = true;
            }
        }
    }   // Level 2

    if (!monster_moved)
    {
        if ((collision = CheckMapCollision(toup)) != COLL_T::WALL)
        {
            creature->PrevPos(creature->CurPos());
            creature->move_map[creature->CurY()][creature->CurX()]++;
            creature->MoveUp();
            monster_moved = true;
        }

        if ((collision = CheckMapCollision(toright)) != COLL_T::WALL)
        {
            if (!monster_moved)
            {
                creature->PrevPos(creature->CurPos());
                creature->move_map[creature->CurY()][creature->CurX()]++;
                creature->MoveRight();
                monster_moved = true;
            }
        }

        if ((collision = CheckMapCollision(todown)) != COLL_T::WALL)
        {
            if (!monster_moved)
            {
                creature->PrevPos(creature->CurPos());
                creature->move_map[creature->CurY()][creature->CurX()]++;
                creature->MoveDown();
                monster_moved = true;
            }
        }

        if ((collision = CheckMapCollision(toleft)) != COLL_T::WALL)
        {
            if (!monster_moved)
            {
                creature->PrevPos(creature->CurPos());
                creature->move_map[creature->CurY()][creature->CurX()]++;
                creature->MoveLeft();
                monster_moved = true;
            }
        }
    }   // Level 3
}   // Engine::NewDummyMove

#ifndef GAMEPLAY_H_INCLUDED
#define GAMEPLAY_H_INCLUDED

/**
 *  CLASS: Gameplay
 *  @brief      Gameplay is the class that represents the visual aspects
 *              of the game. Its instances hold every window and subwindow
 *              (ncurses' terminology) that is shown to the screen and control
 *              every interactive way of communication with the player.
 */
class Gameplay
{
    public:
    Gameplay() : stage_offset(COLS / 2), score_offset(1), debug_offset(LINES - 3)
    {}

    typedef struct win_exc
    {   // Used as an exceptiom to be thrown in case a window function goes bad
        std::string message;
        win_exc(std::string arg): message(arg) {}
    } WINEXP;

    void InitDebugWin(void);
    void InitInfoBar(const std::string&);
    void InitStageWin(void);
    void InitPlayerWin(void);
    void InitGnomeWin(void);
    void InitTraalWin(void);
    void InitLevel(const std::vector<I8*>);
    void EndLevel(void);
    void DrawMenu(UI8);
    void DrawParch(POS);
    void DrawScore(UI32);
    void DrawHighScores(const std::vector<SCOS>&);

    const WINDOW* Stage(void)   const   { return stage_win; }
    const WINDOW* Map(void)     const   { return map_win; }
    const WINDOW* Player(void)  const   { return player_win; }
    const WINDOW* Gnome(void)   const   { return gnome_win; }
    const WINDOW* Traal(void)   const   { return traal_win; }
    const WINDOW* Debug(void)   const   { return debug_win; }
    const WINDOW* InfoBar(void) const   { return info_win; }
    const WINDOW* Score(void)   const   { return score_win; }

    WINDOW* Stage(void)     { return stage_win; }
    WINDOW* Map(void)       { return map_win; }
    WINDOW* Player(void)    { return player_win; }
    WINDOW* Gnome(void)     { return gnome_win; }
    WINDOW* Traal(void)     { return traal_win; }
    WINDOW* Debug(void)     { return debug_win; }
    WINDOW* InfoBar(void)   { return info_win; }
    WINDOW* Score(void)     { return score_win; }

    void MoveWin(WINDOW*, POS);
    void ShowWin (WINDOW*) const;

    void ColorFlashWin(WINDOW*, UI32);
    void FlashToggleWin(WINDOW*, WINDOW*, UI32);

    I32 GetPlayerInput(void);
    void Pause(bool);
    bool Pause(void) const { return isPaused; }

    void DiamondEaten(POS);

    private:
    WINDOW* stage_win;
    WINDOW* map_win;
    WINDOW* info_win;
    WINDOW* score_win;
    WINDOW* player_win;
    WINDOW* gnome_win;
    WINDOW* traal_win;
    WINDOW* debug_win;

    UI8 stage_offset;
    UI8 score_offset;
    UI8 debug_offset;
    bool isPaused;

    const static std::string menu_items[MENU_ITEMS_COUNT];

    void InitMapWin(UI8, UI8);
};

#endif // GAMEPLAY_H_INCLUDED

const std::string Gameplay::menu_items[MENU_ITEMS_COUNT] = // Main menu options
    {"Play game", "Show High Score Table", "Quit"};

/* CLASS GAMEPLAY PRIVATE MEMBER FUNCTIONS */
/**
 *  PUBLIC MEMBER FUNCTION Gameplay::InitMapWin
 *  @brief  Initialises the map window according to the width and height passed
 *          as arguments. Map window is stage's derivative window. Map window
 *          holds the visual part of the game's maze.
 *  @param  height: The height of the map window.
 *  @param  width: The width of the map window.
 */
void Gameplay::InitMapWin(UI8 height, UI8 width)
{
    stage_offset = (COLS - width) / 2;
    if ((map_win = derwin(stage_win, height, width, 0, stage_offset)) == NULL)
        throw WINEXP("Unable to acquire resources to construct window: map_win");
}

/* CLASS GAMEPLAY PUBLIC MEMBER FUNCTIONS */
/**
 *  PUBLIC MEMBER FUNCTION Gameplay::InitStageWin
 *  @brief  Initialises the stage window. The maze, the living creatures and some
 *          game messages are hosted in the stage window.
 */
void Gameplay::InitStageWin(void)
{
    if ((stage_win = newwin(LINES - 3, COLS - 1, score_offset, 0)) == NULL)
        throw WINEXP("Unable to acquire resources to construct window: stage_win");

    keypad(stage_win, TRUE);
    nodelay(stage_win, TRUE);
}

/**
 *  PUBLIC MEMBER FUNCTION Gameplay::InitPlayerWin
 *  @brief  Initialises the main player window. Player window is a 1X1 window
 *          that is assigned the letter H (stands for Harry). It visually re=
 *          presents Harry in the maze.
 */
void Gameplay::InitPlayerWin(void)
{
    if ((player_win = newwin(1, 1, 0, 0)) == NULL)
        throw WINEXP("Unable to acquire resources to construct window: player_win");

    waddch(player_win, 'H');
    wbkgd(player_win, COLOR_PAIR(COLOR_PAIR_YELLOW_BLACK));
}

/**
 *  PUBLIC MEMBER FUNCTION Gameplay::InitGnomeWin
 *  @brief  Initialises the gnome window. Gnome window is a 1X1 window
 *          that is assigned the letter G (stands for Gnome). It visually
 *          represents Gnome monster in the maze.
 */
void Gameplay::InitGnomeWin(void)
{
    if ((gnome_win = newwin(1, 1, 0, 0)) == NULL)
        throw WINEXP("Unable to acquire resources to construct window: gnome_win");

    waddch(gnome_win, 'G');
    wbkgd(gnome_win, COLOR_PAIR(COLOR_PAIR_BLACK_RED));
}

/**
 *  PUBLIC MEMBER FUNCTION Gameplay::InitTraalWin
 *  @brief  Initialises the traal window. Traal window is a 1X1 window
 *          that is assigned the letter T (stands for Traal). It visually
 *          represents Traal monster in the maze.
 */
void Gameplay::InitTraalWin(void)
{
    if ((traal_win = newwin(1, 1, 0, 0)) == NULL)
        throw WINEXP("Unable to acquire resources to construct window: traal_win");

    waddch(traal_win, 'T');
    wbkgd(traal_win, COLOR_PAIR(COLOR_PAIR_BLACK_RED));
}

/**
 *  PUBLIC MEMBER FUNCTION Gameplay::InitLevel
 *  @brief  Performs the necessary actions to set up the screen for a new level.
 *  @param  map: The data retrieved to draw the maze.
 */
void Gameplay::InitLevel(const std::vector<I8*> _map)
{
    UI8 map_height = _map.size();
    UI8 map_width  = ((std::string)_map[0]).length();

    InitMapWin(map_height, map_width);

    for (UI8 i = 0; i < map_height; i++)
    {
        wmove(map_win, i, 0);
        for (UI8 j = 0; j < map_width; j++)
        {
            if (_map[i][j] == '.')
                wattron(map_win, COLOR_PAIR(COLOR_PAIR_YELLOW_BLACK));
            waddch(map_win, _map[i][j]);

            wstandend(map_win);
        }
    }
}   // Gameplay::InitLevel

/**
 *  PUBLIC MEMBER FUNCTION Gameplay::EndLevel
 *  @brief  Clears the screen and ends the current level.
 */
void Gameplay::EndLevel(void)
{
    wclear(stage_win);
    wclear(map_win);
    delwin(map_win);
}

void Gameplay::InitDebugWin(void)
{
    debug_offset = LINES - 3;
    if ((debug_win = newwin(3, COLS, debug_offset, 0)) == NULL)
        throw WINEXP("Unable to acquire resources to construct window: debug_win");

    wbkgd(debug_win, COLOR_PAIR(COLOR_PAIR_BLACK_BLUE));
    scrollok(debug_win, TRUE);
}

/**
 *  PUBLIC MEMBER FUNCTION Gameplay::InitInfoBar
 *  @brief  Initialises the information bar window. The information bar shows
 *          information about the current player.
 *  @param  player_name: The player name to draw on the bar.
 */
void Gameplay::InitInfoBar(const std::string& player_name)
{
    if ((info_win = newwin(1,COLS, 0, 0)) == NULL)
        throw WINEXP("Unable to acquire resources to construct window: info_win");

    if ((score_win = derwin(info_win, 1, 5, 0, COLS - 5)) == NULL)
        throw WINEXP("Unable to acquire resources to construct window: score_win");

    wbkgd(info_win, COLOR_PAIR(COLOR_PAIR_BLACK_YELLOW));
    wbkgd(score_win, COLOR_PAIR(COLOR_PAIR_BLACK_YELLOW));
    wprintw(info_win, " %s", player_name.c_str());
}

/**
 *  PUBLIC MEMBER FUNCTION Gameplay::DrawMenu
 *  @brief  Draws the main menu on the screen.
 *  @param  slc: An integer showing which menu item to highlight.
 */
void Gameplay::DrawMenu(UI8 slc)
{
    for (UI8 i = 0; i < MENU_ITEMS_COUNT; i++)
    {
        if (slc == i) wattron(stage_win, COLOR_PAIR(COLOR_PAIR_YELLOW_BLACK));
        mvwaddstr(stage_win, i*2 + 3, (COLS - menu_items[i].length()) / 2, menu_items[i].c_str());
        wstandend(stage_win);
    }
}

/**
 *  PUBLIC MEMBER FUNCTION Gameplay::DrawParch
 *  @brief  Draws the parch in the maze (as a 'P' character). Must be called
 *          after all of the diamonds in the level are eaten.
 *  @param  parch_pos: The position to draw the parch.
 */
void Gameplay::DrawParch(POS parch_pos)
{
    wattron(map_win, COLOR_PAIR(COLOR_PAIR_YELLOW_BLACK) | A_BOLD);
    mvwdelch(map_win, parch_pos.y, parch_pos.x);
    mvwinsch(map_win, parch_pos.y, parch_pos.x, 'P');
    wstandend(map_win);
}

/**
 *  PUBLIC MEMBER FUNCTION Gameplay::DrawScore
 *  @brief  Draws the score passed as an argument on the info bar.
 *  @param  score: The value to draw.
 */
void Gameplay::DrawScore(UI32 score)
{
    wclear(score_win);
    wprintw(score_win, "%d", score);
    wrefresh(score_win);
}

/**
 *  PUBLIC MEMBER FUNCTION Gameplay::DrawHighScores
 *  @brief  Draws the high score table with a nice delay effect.
 *  @param  table: The score table to retrieve data from.
 */
void Gameplay::DrawHighScores(const std::vector<SCOS>& sc_table)
{
    UI8 counter = 1;
    std::string sc_label = "HIGHSCORE TABLE";

    wattron(stage_win, COLOR_PAIR(COLOR_PAIR_GREEN_BLACK) | A_BOLD);
    mvwprintw(stage_win, 0, (COLS - sc_label.length()) / 2, "%s", sc_label.c_str());
    wstandend(stage_win);

    if (sc_table.begin() == sc_table.end())
    {
        mvwprintw(stage_win, counter*2, (COLS - std::string("(empty)").length()) / 2,
                  "(empty)");
        wrefresh(stage_win);
    }
    else
        for(std::vector<SCOS>::const_iterator it = sc_table.begin(); it != sc_table.end(); it++)
        {
            mvwprintw(stage_win, counter*2, (COLS - 20) / 2,
                      "%d \t %s", (*it).player_score, (*it).player_name);
            counter++;
            if (counter*2 == LINES) break;

            napms(50);
            wrefresh(stage_win);
        }
}

/**
 *  PUBLIC MEMBER FUNCTION Gameplay::GetPlayerInput
 *  @brief  Handles the input from the keyboard.
 *  @return The keycode.
 */
I32 Gameplay::GetPlayerInput(void)
{
    I32 key = wgetch(stage_win);

    if (key == ' ') ShowWin(debug_win);

    return key;
}

/**
 *  PUBLIC MEMBER FUNCTION Gameplay::MoveWin
 *  @brief  Moves window win in the coordinates specified by coords.
 *  @param  win: The window to be moved.
 *  @param  coords: The new coordinates of the window.
 */
void Gameplay::MoveWin(WINDOW* _win, POS coords)
{
    mvwin(_win, coords.y + score_offset, stage_offset + coords.x);
}

/**
 *  PUBLIC MEMBER FUNCTION Gameplay::ShowWin
 *  @brief  Brings the window passed as argument to the front and
 *          refreshes the view in order to draw it.
 *  @param  win: The window to be drawn.
 */
void Gameplay::ShowWin (WINDOW* _win) const
{
    touchwin(_win);
    wrefresh(_win);
}


/**
 *  PUBLIC MEMBER FUNCTION Gameplay::GetPlayerInput
 *  @brief  Pauses or unpauses the gameplay according to the
 *          boolean parametre passed.
 *  @param  state: True for paused or false for unpaused.
 */
void Gameplay::Pause(bool state)
{
    int stage_x, stage_y;
    getmaxyx(stage_win, stage_y, stage_x);

    if (!isPaused)
        mvwprintw(stage_win, stage_y / 2, (stage_x - getmaxx(map_win)) / 2 - strlen(PAUSE_MESSAGE) - 2, PAUSE_MESSAGE);
    else
    {
        for (int i = 0; i < strlen(PAUSE_MESSAGE); i++)
            mvwdelch(stage_win, stage_y / 2, (stage_x - getmaxx(map_win)) / 2 - strlen(PAUSE_MESSAGE) - 2);
        for (int i = 0; i < strlen(PAUSE_MESSAGE); i++)
            mvwinsch(stage_win, stage_y / 2, (stage_x - getmaxx(map_win)) / 2 - strlen(PAUSE_MESSAGE) - 2, ' ');
    }

    nodelay(stage_win, !state);
    isPaused = state;
    ShowWin(stage_win);
    ShowWin(player_win);
    ShowWin(gnome_win);
    ShowWin(traal_win);
}

/**
 *  PUBLIC MEMBER FUNCTION Gameplay::FlashToggleWin
 *  @brief  Given two window pointers, this function repeatedly toggles
 *          their visible state for as many times as specified by the
 *          third argument.
 *  @param  w1: The first window.
 *  @param  w2: The second window.
 *  @param  times: Maximun times to flash the windows.
 */
void Gameplay::FlashToggleWin(WINDOW* w1, WINDOW* w2, UI32 times)
{
    for (UI32 i = 0; i < times; i++)
    {
        ShowWin(w1);
        napms(200);
        ShowWin(w2);
        napms(200);
    }
}


/**
 *  PUBLIC MEMBER FUNCTION Gameplay::DiamondEaten
 *  @brief  Deletes a diamond (dot) from the screen at the specified coordinates.
 *  @param  coords: The coordinates of the diamond to be erased.
 */
void Gameplay::DiamondEaten(POS coords)
{
    mvwdelch(map_win, coords.y, coords.x);
    mvwinsch(map_win, coords.y, coords.x, ' ');
}

#ifndef GAMEBASE_H_INCLUDED
#define GAMEBASE_H_INCLUDED

Gameplay gpl;   // Gameplay
Engine glen;    // Global Engine
HighScore hsc;  // High Scores Controller

void init_curses(void)
{
    initscr();
    curs_set(0);

    if (has_colors())
    {
        start_color();
        init_pair(COLOR_PAIR_NORMAL, COLOR_WHITE, COLOR_BLACK);
        init_pair(COLOR_PAIR_GREEN_BLACK, COLOR_GREEN, COLOR_BLACK);
        init_pair(COLOR_PAIR_YELLOW_BLACK, COLOR_YELLOW, COLOR_BLACK);
        init_pair(COLOR_PAIR_BLACK_RED, COLOR_BLACK, COLOR_RED);
        init_pair(COLOR_PAIR_BLACK_BLUE, COLOR_BLACK, COLOR_BLUE);
        init_pair(COLOR_PAIR_BLACK_YELLOW, COLOR_BLACK, COLOR_YELLOW);
    }

    raw();
    noecho();
    nonl();
}

void init_gameplay(void)
{
    gpl.InitStageWin();
    gpl.InitPlayerWin();
    gpl.InitGnomeWin();
    gpl.InitTraalWin();
    gpl.InitDebugWin();
}

void kill_gameplay()
{
    wclear(gpl.InfoBar());
    wclear(gpl.Debug());
    wclear(gpl.Player());
    wclear(gpl.Gnome());
    wclear(gpl.Traal());

    delwin(gpl.Score());
    delwin(gpl.InfoBar());
    delwin(gpl.Debug());
    delwin(gpl.Map());
    delwin(gpl.Stage());
    delwin(gpl.Player());
    delwin(gpl.Gnome());
    delwin(gpl.Traal());
}

void load_next_level(std::ifstream& _mapdata)
{
    glen.InitLevel(_mapdata);
    gpl.InitLevel(glen.stage.Map());

    gpl.MoveWin(gpl.Player(), glen.player.CurPos());
    gpl.MoveWin(gpl.Gnome(), glen.gnome.CurPos());
    gpl.MoveWin(gpl.Traal(), glen.traal.CurPos());

    gpl.ShowWin(gpl.InfoBar());
    gpl.ShowWin(gpl.Stage());
    gpl.ShowWin(gpl.Player());
    gpl.ShowWin(gpl.Gnome());
    gpl.ShowWin(gpl.Traal());
}

void kill_cur_level(void)
{
    glen.EndLevel();
    gpl.EndLevel();

    wrefresh(gpl.Stage());
}

void new_turn(void)
{
    UI32 inp = gpl.GetPlayerInput();

    if (inp == KEY_ESCAPE) throw Engine::Escape("User pressed escape key\n");
    if (inp == KEY_PAUSE || inp == 'P' || inp == 'p')
        gpl.Pause()?gpl.Pause(false):gpl.Pause(true);

    if (!gpl.Pause()) {
        glen.NewMove(&glen.player, inp);

        if (glen.CheckMapCollision(glen.player.CurPos()) == COLL_T::NONE)
            glen.player.CollisionState(COLL_T::NONE);
        if (glen.CheckMapCollision(glen.player.CurPos()) == COLL_T::DMND)
            glen.player.CollisionState(COLL_T::DMND);
        if (glen.CheckMapCollision(glen.player.CurPos()) == COLL_T::PARCH)
            glen.player.CollisionState(COLL_T::PARCH);
        if (glen.player.CurPos() == glen.gnome.CurPos() ||
            glen.player.CurPos() == glen.traal.CurPos())
            glen.player.CollisionState(COLL_T::MONSTER);

        if (glen.player.CollisionState() != COLL_T::MONSTER)
        {
            glen.NewSmartMove(&glen.gnome);
            glen.NewDummyMove(&glen.traal);

            if (glen.player.CurPos() == glen.gnome.CurPos() ||
                glen.player.CurPos() == glen.traal.CurPos())
                glen.player.CollisionState(COLL_T::MONSTER);
        }

        gpl.MoveWin(gpl.Player(), glen.player.CurPos());
        gpl.MoveWin(gpl.Gnome(), glen.gnome.CurPos());
        gpl.MoveWin(gpl.Traal(), glen.traal.CurPos());

        gpl.ShowWin(gpl.Map());
        gpl.ShowWin(gpl.Player());
        gpl.ShowWin(gpl.Gnome());
        gpl.ShowWin(gpl.Traal());
    }
}

void handle_score(void)
{
    if (glen.player.CollisionState() == COLL_T::DMND)
        glen.player.AddToScore(10);
    if (glen.player.CollisionState() == COLL_T::PARCH)
        glen.player.AddToScore(100);
    gpl.DrawScore(glen.player.Score());
}

void play(void)
{
    while(1)
    {
        if (glen.stage.DiamondsCount() == 0)
            gpl.DrawParch(glen.stage.ParchPos());

        new_turn();

        handle_score();

        flushinp();

        napms(250);

        if (glen.player.CollisionState() == COLL_T::DMND)
        {
            glen.stage.EraseDiamond(glen.player.CurPos());
            gpl.DiamondEaten(glen.player.CurPos());
        }
        if (glen.player.CollisionState() == COLL_T::MONSTER)
            throw Potter::Lose(glen.player.CurPos());
        if (glen.player.CollisionState() == COLL_T::PARCH)
            throw Potter::Win(glen.player.CurPos());
    }
}

void get_player_name(I8 name[])
{
    std::string name_prompt
        = "Please enter a nick up to ten characters: ";

    echo();
    nodelay(gpl.Stage(), FALSE);

    mvwprintw(gpl.Stage(), (LINES - 4) / 2, (COLS - name_prompt.length() - (NICKNAME_DEFAULT_LENGTH - 1)) / 2, "%s", name_prompt.c_str());
    wrefresh(gpl.Stage());
    wgetnstr(gpl.Stage(), name, NICKNAME_DEFAULT_LENGTH - 1);

    noecho();
    nodelay(gpl.Stage(), TRUE);

    wclear(gpl.Stage());
}

void handle_player_score(void)
{
    I8 player_name[NICKNAME_DEFAULT_LENGTH];

    get_player_name(player_name);
    if (strcmp(player_name, "") != 0)
        glen.player.Name(player_name);

    hsc.InitTable();
    hsc.PlayerName(glen.player.Name());
    hsc << glen.player.Score();
}

#endif // GAMEBASE_H_INCLUDED


int main(int argc, char* argv[])
{
    I32 kstroke;
    UI8 selection = 0;
    std::ifstream mapdata;

    try
    {
        init_curses();
        init_gameplay();

        while(selection != SLC_QUIT)
        {
            gpl.ShowWin(stdscr);
            wclear(gpl.Stage());
            selection = 0;

            do
            {
                gpl.DrawMenu(selection);

                kstroke = gpl.GetPlayerInput();
                if (kstroke == KEY_UP) selection > 0?selection--:selection = MENU_ITEMS_COUNT - 1;
                if (kstroke == KEY_DOWN) selection < MENU_ITEMS_COUNT - 1?selection++:selection = 0;
            } while (kstroke != KEY_ENTER && kstroke != KEY_RETURN);

            wclear(gpl.Stage());
            wrefresh(gpl.Stage());
            flushinp();

            switch(selection)
            {
                case 0:
                try
                {
                    gpl.InitInfoBar(glen.player.Name());
                    glen.player.Score(0);

                    for (UI8 i = 1; i < argc; i++)
                    {
                        try
                        {
                            mapdata.open(argv[i], std::ios::in);
                            load_next_level(mapdata);
                            mapdata.close();

                            wgetch(gpl.Player());

                            try{ play(); }
                            catch (Potter::Win& exp)
                            {
                                gpl.FlashToggleWin(gpl.Map(), gpl.Player(), 5);
                            }

                            kill_cur_level();
                            flushinp();
                        }
                        catch(GENEXP& exp)
                        {
                            printw("%s", exp.message.c_str());
                            gpl.ShowWin(stdscr);
                            glen.stage.Unload();
                            mapdata.close();
                            getch();
                            wclear(stdscr);
                        }
                    }
                    flushinp();

                    if (glen.player.Score() > 0)
                        handle_player_score();
                }
                catch(Potter::Lose& exp)
                {
                    if (glen.player.CurPos() == glen.gnome.CurPos())
                        gpl.FlashToggleWin(gpl.Player(), gpl.Gnome(), 5);
                    else gpl.FlashToggleWin(gpl.Player(), gpl.Traal(), 5);

                    kill_cur_level();

                    flushinp();

                    if (glen.player.Score() > 0)
                        handle_player_score();
                }
                catch(Engine::Escape& exp)
                {
                    kill_cur_level();
                }
                catch(FILEEXP& exp)
                {
                    printw("Could not load file '%s' for '%s'\n", exp.filename.c_str(), exp.open_purpose.c_str());
                    gpl.ShowWin(stdscr);
                    getch();
                    wclear(stdscr);
                }

                case 1:
                wclear(gpl.Stage());

                hsc.InitTable();
                hsc.SortTable();

                gpl.DrawHighScores(hsc.ScoreTable());
                wrefresh(gpl.Stage());

                hsc.SaveTable();
                hsc.EmptyTable();

                flushinp();
                wgetch(gpl.Player());
                break;

                case 2:     break;
            }
        }
    }
    catch(Gameplay::WINEXP& exp)
    {
        printw("An error occured while loading: %s", exp.message.c_str());
        printw("Press any key to terminate every process\n");
        getch();
    }

    kill_gameplay();
    endwin();

    return 0;
}
