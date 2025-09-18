#include <GL/glut.h>
#include <vector>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <algorithm>

#define PI 3.14159265358979323846

// Animation state
float fish_t = 0.0f;
const float animation_speed = 0.0015f;
float fish_x, fish_y;
float fish_angle = 0.0f;

// Window size
const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 600;

// Color palette
const float COLOR_SEAWEED[] = {0.2f, 0.7f, 0.3f};
const float COLOR_FISH_BODY[] = {0.9f, 0.6f, 0.1f};
const float COLOR_FISH_FINS[] = {0.8f, 0.5f, 0.0f};
const float COLOR_BUBBLE[] = {0.8f, 0.9f, 1.0f};

// Scanline Fill Algorithm (modified for floats)
void scanlineFillPolygon(const std::vector<std::pair<float, float>>& polygon, const float color[3]) {
    if (polygon.empty()) return;
    float minY = polygon[0].second, maxY = polygon[0].second;
    for (const auto& p : polygon) {
        if (p.second < minY) minY = p.second;
        if (p.second > maxY) maxY = p.second;
    }

    glColor3fv(color);
    for (int y = static_cast<int>(minY); y <= static_cast<int>(maxY); ++y) {
        std::vector<int> intersections;
        for (size_t i = 0; i < polygon.size(); ++i) {
            std::pair<float, float> p1 = polygon[i];
            std::pair<float, float> p2 = polygon[(i + 1) % polygon.size()];

            if (p1.second == p2.second) continue;
            if ((y >= std::min(p1.second, p2.second)) && (y < std::max(p1.second, p2.second))) {
                float x = p1.first + (static_cast<float>(y) - p1.second) * (p2.first - p1.first) / (p2.second - p1.second);
                intersections.push_back(static_cast<int>(x));
            }
        }
        std::sort(intersections.begin(), intersections.end());
        for (size_t i = 0; i + 1 < intersections.size(); i += 2) {
            glBegin(GL_POINTS);
            for (int x = intersections[i]; x <= intersections[i + 1]; ++x) {
                glVertex2i(x, y);
            }
            glEnd();
        }
    }
}

// Draw a single bubble
void drawBubble(float x, float y, float r) {
    glColor4f(COLOR_BUBBLE[0], COLOR_BUBBLE[1], COLOR_BUBBLE[2], 0.5f); // Semi-transparent
    glBegin(GL_POINTS);
    for (int i = 0; i <= 360; i++) {
        float angle_rad = i * PI / 180.0f;
        float px = x + r * cos(angle_rad);
        float py = y + r * sin(angle_rad);
        glVertex2f(px, py);
    }
    glEnd();
}

// Draw a filled circle using GLUT (faster)
void drawFilledCircle(float x, float y, float r, const float color[3]) {
    glColor3fv(color);
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(x, y);
    for (int i = 0; i <= 360; i++) {
        float angle = i * PI / 180.0f;
        glVertex2f(x + r * cos(angle), y + r * sin(angle));
    }
    glEnd();
}

// Draw swaying seaweed
void drawSeaweed(float x, float y, float height, float sway_factor) {
    std::vector<std::pair<float, float>> poly;
    poly.push_back({x, y});
    poly.push_back({x - 5, y + 20});
    poly.push_back({x + 5, y + 40});
    poly.push_back({x, y + height});
    poly.push_back({x + 5, y + 40});
    poly.push_back({x + 10, y + 20});

    // Apply sway
    for (size_t i = 0; i < poly.size(); ++i) {
        float sway_x = sin(glutGet(GLUT_ELAPSED_TIME) * 0.001 + x) * sway_factor;
        poly[i].first += sway_x;
    }

    scanlineFillPolygon(poly, COLOR_SEAWEED);
}

// Draw a fish
void drawFish(float x, float y, float angle) {
    // Apply transformations
    glPushMatrix();
    glTranslatef(x, y, 0);
    glRotatef(angle, 0, 0, 1);

    // Body
    drawFilledCircle(0, 0, 20, COLOR_FISH_BODY);

    // Tail fin
    std::vector<std::pair<float, float>> tail;
    tail.push_back({-20, 0});
    tail.push_back({-35, 10});
    tail.push_back({-35, -10});
    scanlineFillPolygon(tail, COLOR_FISH_FINS);

    // Eye
    glColor3f(0.0f, 0.0f, 0.0f);
    drawFilledCircle(10, 5, 3, COLOR_FISH_BODY);
    
    // Dorsal Fin
    std::vector<std::pair<float, float>> dorsal;
    dorsal.push_back({-10, 20});
    dorsal.push_back({10, 20});
    dorsal.push_back({5, 30});
    scanlineFillPolygon(dorsal, COLOR_FISH_FINS);


    glPopMatrix();
}

// Render the scene
void display() {
    glClear(GL_COLOR_BUFFER_BIT);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0, WINDOW_WIDTH, 0, WINDOW_HEIGHT);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    // Draw the scene elements
    drawSeaweed(100, 0, 100, 10);
    drawSeaweed(250, 0, 120, 15);
    drawSeaweed(400, 0, 150, 20);

    // Draw bubbles
    for (int i = 0; i < 50; ++i) {
        float bubble_x = (rand() % WINDOW_WIDTH);
        float bubble_y = (rand() % WINDOW_HEIGHT) * 1.5 - glutGet(GLUT_ELAPSED_TIME) * 0.05;
        drawBubble(bubble_x, fmod(bubble_y, WINDOW_HEIGHT), 5);
    }

    drawFish(fish_x, fish_y, fish_angle);

    glutSwapBuffers();
}

// Update animation state
void update(int value) {
    // Bézier curve for fish movement
    const std::vector<std::pair<float, float>> curve = {
        {100, 300}, // Start
        {400, 550}, // Control Point 1
        {700, 400}  // End
    };

    fish_t += animation_speed;
    if (fish_t > 1.0f) {
        fish_t = 0.0f; // Loop the animation
    }

    float u = 1.0f - fish_t;
    fish_x = u * u * curve[0].first + 2 * u * fish_t * curve[1].first + fish_t * fish_t * curve[2].first;
    fish_y = u * u * curve[0].second + 2 * u * fish_t * curve[1].second + fish_t * fish_t * curve[2].second;

    // Calculate angle for facing direction (simple approximation)
    float prev_x = (1.0f - (fish_t-0.01f))*(1.0f - (fish_t-0.01f))*curve[0].first + 2*(1.0f - (fish_t-0.01f))*(fish_t-0.01f)*curve[1].first + (fish_t-0.01f)*(fish_t-0.01f)*curve[2].first;
    float prev_y = (1.0f - (fish_t-0.01f))*(1.0f - (fish_t-0.01f))*curve[0].second + 2*(1.0f - (fish_t-0.01f))*(fish_t-0.01f)*curve[1].second + (fish_t-0.01f)*(fish_t-0.01f)*curve[2].second;

    fish_angle = atan2(fish_y - prev_y, fish_x - prev_x) * 180.0f / PI;


    glutPostRedisplay();
    glutTimerFunc(16, update, 0); // 60 FPS
}


// Main function
int main(int argc, char** argv) {
    srand(time(0));
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
    glutCreateWindow("Underwater Scene");
    glutDisplayFunc(display);
    glutTimerFunc(16, update, 0);
    glutMainLoop();
    return 0;
}