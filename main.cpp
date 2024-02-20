#include <SFML/Graphics.hpp>
#include <vector>
#include <cmath>
#include <iostream>
#include <cstdlib>
#include <ctime>

// Verlet Integration
//  In Verlet integration, instead of explicitly calculating velocities, you calculate the next position based on the current position and the previous position.

// SFML Library
// sf::Vector2f --> class for representing 2-dimensional vector with floating-point components (namspace::className)

const int WINDOW_WIDTH = 800;  // Width of the window
const int WINDOW_HEIGHT = 600; // Height of the window
const int CLOTH_SIZE = 50;     // Cloth size (10x10)
const float GRAVITY = 0.5f;    // Gravity acceleration
const float TIME_STEP = 0.1f;  // Time step for Verlet integration

// Point struct representing a point in the cloth
struct Point
{
    sf::Vector2f position;    // current position of point (vector of x and y)
    sf::Vector2f oldPosition; // previous position of the point (vector of x and y)
    bool pinned;

    // custom contructor for the point struct
    // As the simulation progresses, the oldPosition will be updated to the previous position before position is
    // updated with the new calculated position in each iteration of the simulation loop.
    Point(sf::Vector2f pos, bool isPinned = false) : position(pos), oldPosition(pos), pinned(isPinned) {}
};

// Stick struct representing a constraint between two points
struct Stick
{
    Point *p1;
    Point *p2;
    float length; // distance between point 1 (p1) and point 2 (p2)

    // custom contructor for the Stick struct
    // replaces the getDistance() function
    Stick(Point *point1, Point *point2) : p1(point1), p2(point2)
    {
        length = std::sqrt(std::pow(p1->position.x - p2->position.x, 2) + std::pow(p1->position.y - p2->position.y, 2));
    }
};

class ClothSimulation
{
private:
    sf::RenderWindow window;        // SFML window object
    std::vector<Point> clothPoints; // Vector to store points in the cloth
    std::vector<Stick> sticks;      // Vector to store sticks (constraints)
    int iterations = 0;             // to "wait" until system is settled

public:
    // Constructor for ClothSimulation clas
    // sf:VideoMode  used to represent the video mode (size and depth) of the window or the screen.
    // "Cloth Simulation" sets the title of the window to "Cloth Simulation".
    ClothSimulation() : window(sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), "Cloth Simulation")
    {
        // Set the position of the window to (100, 100)
        window.setPosition(sf::Vector2i(500, 100));
    }

    // Method to initialize the cloth simulation
    void initialize()
    {
        // initislaize points for cloth
        for (int i = 0; i < CLOTH_SIZE; i++)
        {
            for (int j = 0; j < CLOTH_SIZE; j++)
            {
                // setting the initial positions for the points
                // cast index i to float * fixed distance between adjacent points on x-axis + 150 units right
                float x = static_cast<float>(i) * 10.0f + 150.0f;
                // cast index j to float * fixed distance between adjacent points on y-axis
                float y = 0.0f;
                // create a point and add it to the clothPoints vector
                // j == 0 is TRUE for top row (pins top row)
                clothPoints.push_back(Point(sf::Vector2f(x, y), j == 0));
            }
        }

        // Create sticks (constraints) between points
        for (int i = 0; i < CLOTH_SIZE; i++)
        {
            for (int j = 0; j < CLOTH_SIZE; j++)
            {
                // the points are represented in column-major order
                //  0 10 20
                //  1 11 21
                //  2 12 22
                //.. .. ..
                //  check if current point is not in the last row of the cloth grid.
                if (i < CLOTH_SIZE - 1)
                    sticks.push_back(Stick(&clothPoints[i * CLOTH_SIZE + j], &clothPoints[(i + 1) * CLOTH_SIZE + j])); // Horizontal stick
                // check if current point is not in the last column of the cloth grid.
                if (j < CLOTH_SIZE - 1)
                    sticks.push_back(Stick(&clothPoints[i * CLOTH_SIZE + j], &clothPoints[i * CLOTH_SIZE + (j + 1)])); // Vertical stick
            }
        }
    }

    sf::Vector2f getDifference(Point *point1, Point *point2)
    {
        sf::Vector2f diff(point1->position.x - point2->position.x, point1->position.y - point2->position.y);
        return diff;
    }

    float getLength(sf::Vector2f v)
    {
        return std::sqrt(v.x * v.x + v.y * v.y);
    }

    void update()
    {
        // Update positions of points
        for (auto &point : clothPoints)
        {
            // apply gravity to all points exepct the ones pinned(top row)
            if (point.pinned)
            {
                continue;
            }

            sf::Vector2f acceleration(0.0f, GRAVITY);

            sf::Vector2f temp = point.position;
            point.position = 2.0f * point.position - point.oldPosition + acceleration * TIME_STEP * TIME_STEP; // Verlet integration with gravity
            point.oldPosition = temp;

            iterations++;

            // check if the system has settled
            //(aka give it 5 000 000 iterations to settle then move a random point)
            if (iterations >= 5000000)
            {
                // Randomly move a point
                int randomIndex = rand() % clothPoints.size(); // Generate a random index
                Point &randomPoint = clothPoints[randomIndex]; // Get a reference to the random point
                std::cout << "I am moving random point " << randomIndex << "which is " << randomPoint.position.x << ", " << randomPoint.position.y << std::endl;

                // make sure the random point is not pinned
                if (!randomPoint.pinned)
                {
                    // Generate a random angle in radians
                    float angle = static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * 2 * 3.14159f;

                    // Calculate movement in x and y directions
                    float moveX = cos(angle) * 60.0f;
                    float moveY = sin(angle) * 60.0f;

                    // Update the position of the random point
                    randomPoint.position.x += moveX;
                    randomPoint.position.y += moveY;
                }

                iterations = 0;
            }

            // Collision with bottom of window
            if (point.position.y >= WINDOW_HEIGHT)
            {
                point.position.y = WINDOW_HEIGHT; // Stop at the bottom
            }
        }

        // Apply constraints (sticks)
        for (auto &stick : sticks)
        {
            // increase stick length as the points fall (to get the unravel effect)
            // returns minimum between a and b
            // aka increase stick.length by 1.0f each time but if stick length > 10.0f
            // set stick.length = 10.0f;
            stick.length = std::min(10.0f, stick.length + 1.0f);

            sf::Vector2f diff = getDifference(stick.p1, stick.p2);
            float diffFactor = (stick.length - getLength(diff)) / getLength(diff) * 0.5f;
            sf::Vector2f offset(diff.x * diffFactor, diff.y * diffFactor);

            // Only move the non-pinned points
            if (!stick.p1->pinned)
            {
                stick.p1->position.x += offset.x;
                stick.p1->position.y += offset.y;
            }
            if (!stick.p2->pinned)
            {
                stick.p2->position.x -= offset.x;
                stick.p2->position.y -= offset.y;
            }
        }
    }

    void draw()
    {
        window.clear();

        // Draw sticks
        for (auto &stick : sticks)
        {
            sf::Vertex line[] = {
                sf::Vertex(stick.p1->position),
                sf::Vertex(stick.p2->position)};
            window.draw(line, 2, sf::Lines);
        }

        window.display();
    }

    void run()
    {
        initialize();

        while (window.isOpen())
        {
            sf::Event event;
            while (window.pollEvent(event))
            {
                if (event.type == sf::Event::Closed)
                    window.close();
            }

            update();
            draw();
        }
    }
};

int main()
{
    // Seed the random number generator
    srand(static_cast<unsigned int>(time(nullptr)));

    ClothSimulation simulation;
    simulation.run();

    return 0;
}
