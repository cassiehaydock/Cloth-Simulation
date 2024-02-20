#include <iostream>
#include <vector>
#include <SDL2/SDL.h>

const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;
const int NUM_POINTS_X = 10;
const int NUM_POINTS_Y = 10;
const int POINT_SPACING = SCREEN_WIDTH / (NUM_POINTS_X + 1);
const float GRAVITY = 0.1f;

struct Point
{
    float x, y;
    float oldX, oldY;
    float accelerationX, accelerationY;
};

std::vector<Point> points;
std::vector<std::pair<int, int>> sticks;

SDL_Window *window = nullptr;
SDL_Renderer *renderer = nullptr;

bool init()
{
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        std::cerr << "SDL could not initialize! SDL_Error: " << SDL_GetError() << std::endl;
        return false;
    }

    window = SDL_CreateWindow("Cloth Simulation", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                              SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    if (window == nullptr)
    {
        std::cerr << "Window could not be created! SDL_Error: " << SDL_GetError() << std::endl;
        return false;
    }

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (renderer == nullptr)
    {
        std::cerr << "Renderer could not be created! SDL_Error: " << SDL_GetError() << std::endl;
        return false;
    }

    return true;
}

void close()
{
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

void drawLine(float x1, float y1, float x2, float y2)
{
    SDL_RenderDrawLine(renderer, static_cast<int>(x1), static_cast<int>(y1),
                       static_cast<int>(x2), static_cast<int>(y2));
}

void initCloth()
{
    points.clear();
    sticks.clear();

    // Initialize points
    for (int y = 0; y < NUM_POINTS_Y; ++y)
    {
        for (int x = 0; x < NUM_POINTS_X; ++x)
        {
            Point point;
            point.x = x * POINT_SPACING + POINT_SPACING / 2;
            point.y = y * POINT_SPACING + POINT_SPACING / 2;
            point.oldX = point.x;
            point.oldY = point.y;
            point.accelerationX = 0;
            point.accelerationY = 0;

            // Fix the top row of points
            if (y == 0)
                point.y = POINT_SPACING / 2;

            points.push_back(point);
        }
    }

    // Create sticks
    for (int y = 0; y < NUM_POINTS_Y; ++y)
    {
        for (int x = 0; x < NUM_POINTS_X; ++x)
        {
            // Structural constraints (horizontal and vertical sticks)
            if (x < NUM_POINTS_X - 1)
                sticks.emplace_back(y * NUM_POINTS_X + x, y * NUM_POINTS_X + x + 1);
            if (y < NUM_POINTS_Y - 1)
                sticks.emplace_back(y * NUM_POINTS_X + x, (y + 1) * NUM_POINTS_X + x);

            // Shear constraints
            if (x < NUM_POINTS_X - 1 && y < NUM_POINTS_Y - 1)
            {
                sticks.emplace_back(y * NUM_POINTS_X + x, (y + 1) * NUM_POINTS_X + x + 1);
                sticks.emplace_back((y + 1) * NUM_POINTS_X + x, y * NUM_POINTS_X + x + 1);
            }

            // Bend constraints
            if (x < NUM_POINTS_X - 2)
                sticks.emplace_back(y * NUM_POINTS_X + x, y * NUM_POINTS_X + x + 2);
            if (y < NUM_POINTS_Y - 2)
                sticks.emplace_back(y * NUM_POINTS_X + x, (y + 2) * NUM_POINTS_X + x);
        }
    }
}

void updateCloth()
{
    // Verlet integration
    for (Point &point : points)
    {
        float tempX = point.x;
        float tempY = point.y;

        point.x += (point.x - point.oldX) + point.accelerationX;
        point.y += (point.y - point.oldY) + point.accelerationY;

        point.oldX = tempX;
        point.oldY = tempY;

        // Apply gravity
        if (point.y < (NUM_POINTS_Y - 1) * POINT_SPACING)
            point.accelerationY += GRAVITY;
        else
            point.accelerationY = 0; // Reset acceleration if point hits bottom
    }

    // Satisfy constraints
    for (int i = 0; i < 5; ++i)
    {
        for (const auto &stick : sticks)
        {
            Point &p1 = points[stick.first];
            Point &p2 = points[stick.second];

            float dx = p2.x - p1.x;
            float dy = p2.y - p1.y;
            float distance = sqrt(dx * dx + dy * dy);

            float diff = (distance - POINT_SPACING) / distance;

            p1.x += dx * 0.5f * diff;
            p1.y += dy * 0.5f * diff;

            p2.x -= dx * 0.5f * diff;
            p2.y -= dy * 0.5f * diff;
        }
    }
}

void renderCloth()
{
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    for (const auto &stick : sticks)
    {
        const Point &p1 = points[stick.first];
        const Point &p2 = points[stick.second];
        drawLine(p1.x, p1.y, p2.x, p2.y);
    }

    SDL_RenderPresent(renderer);
}

int main()
{
    if (!init())
    {
        std::cerr << "Failed to initialize!" << std::endl;
        return 1;
    }

    bool quit = false;
    SDL_Event e;

    initCloth();

    while (!quit)
    {
        while (SDL_PollEvent(&e) != 0)
        {
            if (e.type == SDL_QUIT)
                quit = true;
        }

        updateCloth();
        renderCloth();
    }

    close();
    return 0;
}
