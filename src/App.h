#pragma once

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "Renderer.h"

// Cell types used by the pathfinding grid
enum CellType
{
    CELL_EMPTY   = 0,
    CELL_WALL,
    CELL_START,
    CELL_END,
    CELL_VISITED,
    CELL_PATH,
    CELL_COUNT
};

// Edit modes selectable with keyboard keys 1-4
enum EditMode
{
    MODE_WALL  = 0,
    MODE_START,
    MODE_END,
    MODE_ERASE
};

class App
{
public:
    static const int GRID_WIDTH  = 20;
    static const int GRID_HEIGHT = 15;
    static const int CELL_SIZE   = 40;

    App();
    ~App();

    bool Initialize(HWND hwnd, int width, int height);
    void Shutdown();

    void Update();
    void Render();

    void OnKeyDown(int key);
    void OnMouseDown(int x, int y, int button);
    void OnMouseMove(int x, int y, int buttons);

private:
    void        ClearGrid();
    int         GetCellIndex(int x, int y) const;
    void        ScreenToGrid(int sx, int sy, int& gx, int& gy) const;
    void        SetCell(int gx, int gy, int button);

    Renderer*   m_renderer;
    int         m_screenWidth;
    int         m_screenHeight;

    CellType    m_grid[GRID_WIDTH * GRID_HEIGHT];
    int         m_startX,  m_startY;
    int         m_endX,    m_endY;

    EditMode    m_editMode;
};
