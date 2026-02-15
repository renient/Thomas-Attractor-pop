#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#include <sys/ioctl.h>
#endif

// --- Configuration ---
#define MAX_POINTS 4000
#define THOMAS_B 0.19
#define LORENZ_S 10.0
#define LORENZ_R 28.0
#define LORENZ_B 2.666
#define DT 0.05

typedef struct { double x, y, z; } Vec3;

// Global State
Vec3 trail[MAX_POINTS];
int head = 0;
int width = 100, height = 40;
double angle_x = 0, angle_y = 0;
int current_system = 0; // 0 = Thomas, 1 = Lorenz

void setup_terminal() {
#ifdef _WIN32
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD dwMode = 0;
    GetConsoleMode(hOut, &dwMode);
    dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    SetConsoleMode(hOut, dwMode);
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(hOut, &csbi);
    width = csbi.srWindow.Right - csbi.srWindow.Left + 1;
    height = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
#else
    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    width = w.ws_col;
    height = w.ws_row;
    printf("\033[?25l"); // Hide cursor
#endif
}

void project(Vec3 p, int *sx, int *sy, double *depth) {
    // Rotation
    double x = p.x, y = p.y, z = p.z;
    double tx, ty, tz;
    
    // Y-axis rotation
    tx = x * cos(angle_y) + z * sin(angle_y);
    tz = -x * sin(angle_y) + z * cos(angle_y);
    x = tx; z = tz;

    // X-axis rotation
    ty = y * cos(angle_x) - z * sin(angle_x);
    tz = y * sin(angle_x) + z * cos(angle_x);
    y = ty; z = tz;

    // Perspective
    double dist = (current_system == 0) ? 12.0 : 60.0;
    double scale = (height * 0.8) / (z + dist);
    
    // Zoom factor based on system
    if (current_system == 1) scale *= 0.8;

    *sx = (int)(width / 2 + x * scale * 2.2);
    *sy = (int)(height / 2 - y * scale);
    *depth = z;
}

void get_glow_color(int age, int *r, int *g, int *b) {
    // Gradient from White -> Cyan -> Deep Blue -> Dark
    if (age < 100) { *r = 255; *g = 255; *b = 255; }
    else if (age < 500) { *r = 100; *g = 220; *b = 255; }
    else if (age < 1500) { *r = 0;   *g = 120; *b = 220; }
    else if (age < 3000) { *r = 0;   *g = 60;  *b = 130; }
    else { *r = 40;  *g = 40;  *b = 60; }
}

char get_density_char(int age) {
    const char *chars = "@#*+:. ";
    if (age < 100) return chars[0];
    if (age < 400) return chars[1];
    if (age < 1000) return chars[2];
    if (age < 2000) return chars[3];
    if (age < 3000) return chars[4];
    return chars[5];
}

void intro_animation() {
    printf("\033[2J\033[H");
    const char* lines[] = {
        "  [ SYSTEM INITIALIZING ]",
        "  > LOADING CHAOS MODULE...",
        "  > CALCULATING THOMAS ATTRACTOR TENSOR...",
        "  > ENABLING TRUECOLOR BITMAP RENDERING...",
        "  > DONE. STARTING POP ANIMATION."
    };
    for (int i = 0; i < 5; i++) {
        printf("\r\033[36m%s\033[0m", lines[i]);
        fflush(stdout);
#ifdef _WIN32
        Sleep(400);
#else
        usleep(400000);
#endif
        printf("\n");
    }
}

int main() {
    setup_terminal();
    intro_animation();

    Vec3 p = {0.1, 0, 0};
    char *frame_buf = malloc(width * height * 64);
    
    // Main loop
    for (int frame = 0; ; frame++) {
        // Switch systems every 1000 frames
        if (frame % 1000 == 0) {
            current_system = (current_system + 1) % 2;
            p = (Vec3){0.1, 0.1, 0.1};
            memset(trail, 0, sizeof(trail));
        }

        // 1. Update Math (Physics)
        double dx, dy, dz;
        if (current_system == 0) { // THOMAS
            dx = sin(p.y) - THOMAS_B * p.x;
            dy = sin(p.z) - THOMAS_B * p.y;
            dz = sin(p.x) - THOMAS_B * p.z;
        } else { // LORENZ
            dx = LORENZ_S * (p.y - p.x);
            dy = p.x * (LORENZ_R - p.z) - p.y;
            dz = p.x * p.y - LORENZ_B * p.z;
        }
        
        p.x += dx * (current_system == 1 ? 0.01 : DT);
        p.y += dy * (current_system == 1 ? 0.01 : DT);
        p.z += dz * (current_system == 1 ? 0.01 : DT);
        
        trail[head] = p;
        head = (head + 1) % MAX_POINTS;

        // 2. Clear buffers
        char out_chars[height][width];
        int out_ages[height][width];
        memset(out_chars, ' ', sizeof(out_chars));
        for(int y=0; y<height; y++) for(int x=0; x<width; x++) out_ages[y][x] = 99999;

        // 3. Render Trajectory
        for (int i = 0; i < MAX_POINTS; i++) {
            int idx = (head - 1 - i + MAX_POINTS) % MAX_POINTS;
            if (trail[idx].x == 0) continue;
            
            int sx, sy;
            double depth;
            project(trail[idx], &sx, &sy, &depth);
            
            if (sx >= 0 && sx < width && sy >= 0 && sy < height) {
                if (i < out_ages[sy][sx]) {
                    out_ages[sy][sx] = i;
                    out_chars[sy][sx] = get_density_char(i);
                }
            }
        }

        // 4. Build Frame String
        char *ptr = frame_buf;
        ptr += sprintf(ptr, "\033[H"); // Cursor to home
        
        // Header
        ptr += sprintf(ptr, " \033[1;36m%s STRANGE ATTRACTOR \033[0m| Mode: %s | Pts: %d\r\n", 
               current_system == 0 ? "THOMAS" : "LORENZ", 
               current_system == 0 ? "sin(y)-bx" : "standard", frame);

        for (int y = 1; y < height - 1; y++) {
            for (int x = 0; x < width; x++) {
                if (out_chars[y][x] != ' ') {
                    int r, g, b;
                    get_glow_color(out_ages[y][x], &r, &g, &b);
                    ptr += sprintf(ptr, "\033[38;2;%d;%d;%dm%c", r, g, b, out_chars[y][x]);
                } else {
                    *ptr++ = ' ';
                }
            }
            *ptr++ = '\r';
            *ptr++ = '\n';
        }
        *ptr = '\0';

        // 5. Draw
        printf("%s", frame_buf);

        angle_x += 0.03;
        angle_y += 0.05;

#ifdef _WIN32
        Sleep(16);
#else
        usleep(16000);
#endif
    }
    return 0;
}
