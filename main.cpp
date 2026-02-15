#include "raylib.h"
#include "raymath.h"
#include "rlgl.h"
#include <vector>
#include <cmath>
#include <string>

// --- Configuration & Constants ---
const int MAX_PARTICLESCount = 50000;
const int SCREEN_WIDTH = 1280;
const int SCREEN_HEIGHT = 720;

// --- Attractor Types ---
enum AttractorType { THOMAS, LORENZ, AIZAWA, DEQUAN };

struct AttractorParams {
    float a, b, c, d, e, f;
};

// --- Particle Structure ---
struct Particle {
    Vector3 position;
    Color color;
};

class AttractorSystem {
public:
    AttractorType type = THOMAS;
    AttractorParams params = { 0.19f, 10.0f, 28.0f, 8.0f/3.0f, 0.0f, 0.0f };
    std::vector<Particle> particles;
    float dt = 0.01f;
    float speed = 1.0f;
    
    AttractorSystem() {
        Reset();
    }

    void Reset() {
        particles.clear();
        Vector3 currentPos = { 0.1f, 0.0f, 0.0f };
        for (int i = 0; i < MAX_PARTICLESCount; i++) {
            UpdateMath(currentPos);
            float intensity = (float)i / MAX_PARTICLESCount;
            Color col = ColorFromHSV(190.0f + intensity * 50.0f, 0.8f, 1.0f);
            particles.push_back({ currentPos, col });
        }
    }

    void UpdateMath(Vector3& pos) {
        float dx, dy, dz;
        switch (type) {
            case THOMAS:
                dx = sinf(pos.y) - params.a * pos.x;
                dy = sinf(pos.z) - params.a * pos.y;
                dz = sinf(pos.x) - params.a * pos.z;
                break;
            case LORENZ:
                dx = params.b * (pos.y - pos.x);
                dy = pos.x * (params.c - pos.z) - pos.y;
                dz = pos.x * pos.y - params.d * pos.z;
                break;
            default:
                dx = dy = dz = 0;
        }
        pos.x += dx * dt;
        pos.y += dy * dt;
        pos.z += dz * dt;
    }

    void Update() {
        // Shift particles and add new one for "movement" effect
        for (size_t i = 0; i < particles.size() - 1; i++) {
            particles[i].position = particles[i+1].position;
        }
        Vector3 nextPos = particles.back().position;
        for(int s=0; s < (int)speed; s++) UpdateMath(nextPos);
        particles.back().position = nextPos;
    }

    void Draw() {
        rlBegin(RL_POINTS);
        for (const auto& p : particles) {
            rlColor4ub(p.color.r, p.color.g, p.color.b, 255);
            rlVertex3f(p.position.x * 2.0f, p.position.y * 2.0f, p.position.z * 2.0f);
        }
        rlEnd();
    }
};

int main() {
    // Initialization
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Thomas Attractor - Real-time Simulation");
    SetTargetFPS(60);

    Camera3D camera = { 0 };
    camera.position = (Vector3){ 15.0f, 15.0f, 15.0f };
    camera.target = (Vector3){ 0.0f, 0.0f, 0.0f };
    camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };
    camera.fovy = 45.0f;
    camera.projection = CAMERA_PERSPECTIVE;

    AttractorSystem system;
    bool autoRotate = true;
    float rotationAngle = 0.0f;

    // Shader for Bloom (Simplified strings for embedded use)
    const char* bloomFs = 
        "#version 330\n"
        "in vec2 fragTexCoord;"
        "in vec4 fragColor;"
        "out vec4 finalColor;"
        "uniform sampler2D texture0;"
        "void main() {"
        "    vec4 texel = texture(texture0, fragTexCoord);"
        "    vec3 bloom = texel.rgb * 1.5;" // Simple boost for glow
        "    finalColor = vec4(texel.rgb + bloom * 0.5, texel.a);"
        "}";

    // In a real app, we'd load this from a file. 
    // For this demo, we'll use standard rendering and explain the bloom.

    while (!WindowShouldClose()) {
        // Update
        if (autoRotate) {
            rotationAngle += 0.5f;
            camera.position.x = 20.0f * sinf(rotationAngle * DEG2RAD);
            camera.position.z = 20.0f * cosf(rotationAngle * DEG2RAD);
        }
        
        system.Update();

        // Draw
        BeginDrawing();
            ClearBackground(BLACK);

            BeginMode3D(camera);
                system.Draw();
                DrawGrid(20, 1.0f);
            EndMode3D();

            // UI Components
            DrawRectangle(10, 10, 300, 250, Fade(DARKGRAY, 0.8f));
            DrawText("ATTRACTOR CONTROLS", 20, 20, 20, CYAN);
            DrawText("Type: Thomas", 20, 50, 15, WHITE);
            DrawText(TextFormat("Parameter b: %.2f", system.params.a), 20, 80, 15, WHITE);
            DrawText(TextFormat("Speed: %.1f", system.speed), 20, 110, 15, WHITE);
            
            DrawRectangle(10, SCREEN_HEIGHT - 100, 400, 80, Fade(DARKGRAY, 0.8f));
            DrawText("EQUATIONS", 20, SCREEN_HEIGHT - 90, 18, CYAN);
            DrawText("dx/dt = sin(y) - b*x", 20, SCREEN_HEIGHT - 65, 15, RAYWHITE);
            DrawText("dy/dt = sin(z) - b*y", 250, SCREEN_HEIGHT - 65, 15, RAYWHITE);

            DrawFPS(SCREEN_WIDTH - 100, 10);
        EndDrawing();
    }

    CloseWindow();
    return 0;
}
