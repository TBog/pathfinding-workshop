#include "App.h"
#include <cstring>

// RGBA colors for each cell type
static const float s_cellColors[CELL_COUNT][4] =
{
    { 0.90f, 0.90f, 0.90f, 1.0f }, // CELL_EMPTY   – light grey
    { 0.20f, 0.20f, 0.20f, 1.0f }, // CELL_WALL    – dark grey
    { 0.00f, 0.80f, 0.00f, 1.0f }, // CELL_START   – green
    { 0.80f, 0.00f, 0.00f, 1.0f }, // CELL_END     – red
    { 0.50f, 0.50f, 1.00f, 1.0f }, // CELL_VISITED – light blue
    { 1.00f, 0.80f, 0.00f, 1.0f }, // CELL_PATH    – yellow
};

App::App()
    : m_renderer(NULL)
    , m_screenWidth(0)
    , m_screenHeight(0)
    , m_startX(-1), m_startY(-1)
    , m_endX(-1),   m_endY(-1)
    , m_editMode(MODE_WALL)
{
    ClearGrid();
}

App::~App()
{
}

bool App::Initialize(HWND hwnd, int width, int height)
{
    m_screenWidth  = width;
    m_screenHeight = height;

    m_renderer = new Renderer();
    if (!m_renderer->Initialize(hwnd, width, height))
    {
        delete m_renderer;
        m_renderer = NULL;
        return false;
    }
    return true;
}

void App::Shutdown()
{
    if (m_renderer)
    {
        m_renderer->Shutdown();
        delete m_renderer;
        m_renderer = NULL;
    }
}

void App::Update()
{
    // Reserved for future pathfinding step logic
}

void App::Render()
{
    if (!m_renderer)
        return;

    m_renderer->BeginScene(0.30f, 0.30f, 0.30f, 1.0f);

    for (int y = 0; y < GRID_HEIGHT; y++)
    {
        for (int x = 0; x < GRID_WIDTH; x++)
        {
            CellType type        = m_grid[GetCellIndex(x, y)];
            const float* color   = s_cellColors[type];
            m_renderer->DrawCell(x, y, CELL_SIZE, color[0], color[1], color[2]);
        }
    }

    m_renderer->EndScene();
}

void App::OnKeyDown(int key)
{
    switch (key)
    {
    case '1': m_editMode = MODE_WALL;  break; // 1 – draw walls
    case '2': m_editMode = MODE_START; break; // 2 – place start
    case '3': m_editMode = MODE_END;   break; // 3 – place end
    case '4': m_editMode = MODE_ERASE; break; // 4 – erase
    case 'C': ClearGrid();             break; // C – clear the entire grid
    }
}

void App::OnMouseDown(int x, int y, int button)
{
    int gx, gy;
    ScreenToGrid(x, y, gx, gy);
    if (gx < 0 || gx >= GRID_WIDTH || gy < 0 || gy >= GRID_HEIGHT)
        return;
    SetCell(gx, gy, button);
}

void App::OnMouseMove(int x, int y, int buttons)
{
    if (buttons & MK_LBUTTON)
        OnMouseDown(x, y, 0);
    else if (buttons & MK_RBUTTON)
        OnMouseDown(x, y, 1);
}

void App::ClearGrid()
{
    memset(m_grid, CELL_EMPTY, sizeof(m_grid));
    m_startX = -1; m_startY = -1;
    m_endX   = -1; m_endY   = -1;
}

int App::GetCellIndex(int x, int y) const
{
    return y * GRID_WIDTH + x;
}

void App::ScreenToGrid(int sx, int sy, int& gx, int& gy) const
{
    gx = sx / CELL_SIZE;
    gy = sy / CELL_SIZE;
}

void App::SetCell(int gx, int gy, int button)
{
    int idx = GetCellIndex(gx, gy);

    if (button == 1) // Right-click always erases
    {
        if (gx == m_startX && gy == m_startY) { m_startX = -1; m_startY = -1; }
        if (gx == m_endX   && gy == m_endY)   { m_endX   = -1; m_endY   = -1; }
        m_grid[idx] = CELL_EMPTY;
        return;
    }

    // Left-click applies the current edit mode
    switch (m_editMode)
    {
    case MODE_WALL:
        if (m_grid[idx] != CELL_START && m_grid[idx] != CELL_END)
            m_grid[idx] = CELL_WALL;
        break;

    case MODE_START:
        if (m_startX >= 0)
            m_grid[GetCellIndex(m_startX, m_startY)] = CELL_EMPTY;
        m_startX = gx;
        m_startY = gy;
        m_grid[idx] = CELL_START;
        break;

    case MODE_END:
        if (m_endX >= 0)
            m_grid[GetCellIndex(m_endX, m_endY)] = CELL_EMPTY;
        m_endX = gx;
        m_endY = gy;
        m_grid[idx] = CELL_END;
        break;

    case MODE_ERASE:
        if (gx == m_startX && gy == m_startY) { m_startX = -1; m_startY = -1; }
        if (gx == m_endX   && gy == m_endY)   { m_endX   = -1; m_endY   = -1; }
        m_grid[idx] = CELL_EMPTY;
        break;
    }
}
