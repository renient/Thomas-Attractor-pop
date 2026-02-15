#include <iostream>
#include <vector>
#include <cmath>
#include <string>
#include <chrono>
#include <thread>
#include <algorithm>

#ifdef _WIN32
#include <windows.h>
#else
#include <sys/ioctl.h>
#include <unistd.h>
#endif

#ifndef ENABLE_VIRTUAL_TERMINAL_PROCESSING
#define ENABLE_VIRTUAL_TERMINAL_PROCESSING 0x0004
#endif

// --- Math & Physics Structures ---
struct Vec3 {
    double x, y, z;
};

struct Particle {
    Vec3 pos;
    int age;
};

class ChaosSystem {
public:
    enum Type { THOMAS, LORENZ, AIZAWA };
    
    ChaosSystem(Type type = THOMAS) : type(type) {
        reset();
    }

    void reset() {
        p.x = 0.1; p.y = 0.1; p.z = 0.1;
        trail.clear();
        trail.reserve(3000);
    }

    void update(double dt) {
        double dx, dy, dz;
        switch (type) {
            case THOMAS: {
                const double b = 0.19;
                dx = std::sin(p.y) - b * p.x;
                dy = std::sin(p.z) - b * p.y;
                dz = std::sin(p.x) - b * p.z;
                break;
            }
            case LORENZ: {
                const double s = 10.0, r = 28.0, b = 8.0/3.0;
                dx = s * (p.y - p.x);
                dy = p.x * (r - p.z) - p.y;
                dz = p.x * p.y - b * p.z;
                break;
            }
            case AIZAWA: {
                const double a = 0.95, b = 0.7, c = 0.6, d = 3.5, e = 0.25, f = 0.1;
                dx = (p.z - b) * p.x - d * p.y;
                dy = d * p.x + (p.z - b) * p.y;
                dz = c + a * p.z - std::pow(p.z, 3) / 3.0 - (std::pow(p.x, 2) + std::pow(p.y, 2)) * (1.0 + e * p.z) + f * p.z * std::pow(p.x, 3);
                break;
            }
        }
        p.x += dx * dt;
        p.y += dy * dt;
        p.z += dz * dt;

        Particle np = {p, 0};
        trail.insert(trail.begin(), np);
        if (trail.size() > 3000) trail.pop_back();

        for (size_t i = 0; i < trail.size(); i++) trail[i].age++;
    }

    const std::vector<Particle>& get_trail() const { return trail; }
    Type get_type() const { return type; }
    void set_type(Type t) { type = t; reset(); }

private:
    Type type;
    Vec3 p;
    std::vector<Particle> trail;
};

// --- Terminal Rendering Engine ---
class TerminalRenderer {
public:
    TerminalRenderer() {
        setup_console();
        update_dims();
    }

    void setup_console() {
#ifdef _WIN32
        HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
        DWORD dwMode = 0;
        GetConsoleMode(hOut, &dwMode);
        SetConsoleMode(hOut, dwMode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
#endif
        std::cout << "\033[?25l"; // Hide cursor
    }

    void update_dims() {
#ifdef _WIN32
        CONSOLE_SCREEN_BUFFER_INFO csbi;
        GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
        width = csbi.srWindow.Right - csbi.srWindow.Left + 1;
        height = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
#else
        struct winsize w;
        ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
        width = w.ws_col;
        height = w.ws_row;
#endif
    }

    void clear() {
        buffer = "\033[H"; // Reset cursor to top
    }

    void draw(const ChaosSystem& sys, double angle_x, double angle_y, double zoom_pop) {
        clear();
        
        // Background Grid / Decoration
        buffer += "\033[1;30m"; // Dark Gray
        std::string title = "[ THOMAS ATTRACTOR v2.0 - C++ CHAOS ]";
        buffer += "\033[1;1H" + title;

        const std::vector<Particle>& trail = sys.get_trail();
        for (size_t i = 0; i < trail.size(); i++) {
            const Particle& pt = trail[i];
            // 3D Projection
            double x = pt.pos.x, y = pt.pos.y, z = pt.pos.z;
            
            // Rotation
            double tx = x * std::cos(angle_y) + z * std::sin(angle_y);
            double tz = -x * std::sin(angle_y) + z * std::cos(angle_y);
            x = tx; z = tz;
            
            double ty = y * std::cos(angle_x) - z * std::sin(angle_x);
            tz = y * std::sin(angle_x) + z * std::cos(angle_x);
            y = ty; z = tz;

            // Perspective
            double dist = (sys.get_type() == ChaosSystem::THOMAS) ? 10.0 : 50.0;
            double scale = (height * 0.45 * zoom_pop) / (z + dist);
            int sx = static_cast<int>(width / 2 + x * scale * 2.1);
            int sy = static_cast<int>(height / 2 - y * scale);

            if (sx >= 1 && sx < width && sy >= 1 && sy < height) {
                // TrueColor Mapping (Glow Effect)
                int r, g, b;
                if (pt.age < 100) { r = 255; g = 255; b = 255; }
                else if (pt.age < 500) { r = 60; g = 220; b = 255; }
                else { r = 0; g = 50 + (int)(200 * (1.0 - (double)pt.age / 3000.0)); b = 150; }

                char c = (pt.age < 50) ? '@' : (pt.age < 200 ? '#' : (pt.age < 1000 ? '*' : '.'));
                
                buffer += "\033[" + std::to_string(sy) + ";" + std::to_string(sx) + "H";
                buffer += "\033[38;2;" + std::to_string(r) + ";" + std::to_string(g) + ";" + std::to_string(b) + "m";
                buffer += c;
            }
        }
        std::cout << buffer << std::flush;
    }

private:
    int width, height;
    std::string buffer;
};

int main() {
    TerminalRenderer renderer;
    ChaosSystem system(ChaosSystem::THOMAS);
    
    double angle_x = 0, angle_y = 0;
    double zoom_pop = 0.1;
    
    for (int frame = 0; ; frame++) {
        if (zoom_pop < 1.0) zoom_pop += 0.05;
        
        if (frame > 0 && frame % 800 == 0) {
            int next = (static_cast<int>(system.get_type()) + 1) % 3;
            system.set_type(static_cast<ChaosSystem::Type>(next));
            zoom_pop = 0.1;
        }

        system.update(system.get_type() == ChaosSystem::THOMAS ? 0.05 : 0.01);
        renderer.draw(system, angle_x, angle_y, zoom_pop);

        angle_x += 0.02;
        angle_y += 0.04;

#ifdef _WIN32
        Sleep(16);
#else
        usleep(16000);
#endif
    }
    return 0;
}

