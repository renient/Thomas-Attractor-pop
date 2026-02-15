#include <windows.h>
#include <GL/gl.h>
#include <cmath>
#include <vector>
#include <ctime>

#define MAX_PARTICLES 250000
#define THOMAS_B 0.19f
#define STEP_SIZE 0.012f
#define TRAIL_FADE 0.08f

struct Vec3 { float x, y, z; };
struct Particle {
    Vec3 pos;
    float speed;
};

std::vector<Particle> particles;
std::vector<float> vertexArray;
std::vector<float> colorArray;
float rotationY = 0.0f;
float rotationX = 0.0f;
int width = 1200, height = 800;

void update_physics() {
    if (vertexArray.size() != particles.size() * 3) {
        vertexArray.resize(particles.size() * 3);
        colorArray.resize(particles.size() * 3);
    }

    int vIndex = 0;
    int cIndex = 0;

    for (auto& p : particles) {
        float tx = sinf(p.pos.y) - THOMAS_B * p.pos.x;
        float ty = sinf(p.pos.z) - THOMAS_B * p.pos.y;
        float tz = sinf(p.pos.x) - THOMAS_B * p.pos.z;

        p.pos.x += tx * STEP_SIZE;
        p.pos.y += ty * STEP_SIZE;
        p.pos.z += tz * STEP_SIZE;
        
        float speedSq = tx*tx + ty*ty + tz*tz;
        p.speed = speedSq;

        vertexArray[vIndex++] = p.pos.x * 3.2f;
        vertexArray[vIndex++] = p.pos.y * 3.2f;
        vertexArray[vIndex++] = p.pos.z * 3.2f;

        float brightness = 0.03f + (p.speed * 0.08f);
        if (brightness > 0.2f) brightness = 0.2f;

        colorArray[cIndex++] = brightness * 0.9f;
        colorArray[cIndex++] = brightness * 0.95f;
        colorArray[cIndex++] = brightness * 1.0f;
    }
}

void setup_projection(int w, int h) {
    if (h == 0) h = 1;
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    float aspect = (float)w / (float)h;
    float f = 1.0f / tanf(45.0f * 0.5f * 3.14159f / 180.0f);
    float zNear = 0.1f, zFar = 100.0f;
    float m[16] = { 
        f/aspect, 0, 0, 0, 
        0, f, 0, 0, 
        0, 0, (zFar+zNear)/(zNear-zFar), -1, 
        0, 0, (2*zFar*zNear)/(zNear-zFar), 0 
    };
    glMultMatrixf(m);
    glMatrixMode(GL_MODELVIEW);
}

void display() {
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    glMatrixMode(GL_PROJECTION); glPushMatrix(); glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);  glPushMatrix(); glLoadIdentity();
    
    glColor4f(0.0f, 0.0f, 0.0f, TRAIL_FADE); 
    glBegin(GL_QUADS);
    glVertex2f(-1,-1); glVertex2f(1,-1); glVertex2f(1,1); glVertex2f(-1,1);
    glEnd();
    
    glPopMatrix(); glMatrixMode(GL_PROJECTION); glPopMatrix(); glMatrixMode(GL_MODELVIEW);

    glLoadIdentity();
    glTranslatef(0.0f, 0.0f, -17.0f);
    
    glRotatef(rotationX, 1, 0, 0);
    glRotatef(rotationY, 0, 1, 0);

    glBlendFunc(GL_ONE, GL_ONE); 
    
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);

    glVertexPointer(3, GL_FLOAT, 0, vertexArray.data());
    glColorPointer(3, GL_FLOAT, 0, colorArray.data());

    glDrawArrays(GL_POINTS, 0, particles.size());

    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_COLOR_ARRAY);

    rotationY += 0.15f;
    rotationX = 15.0f * sinf(rotationY * 0.01f);
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_CLOSE: 
            PostQuitMessage(0); 
            return 0;
            
        case WM_KEYDOWN:
            if (wParam == VK_ESCAPE) {
                PostQuitMessage(0);
                return 0;
            }
            break;

        case WM_RBUTTONUP:
            PostQuitMessage(0);
            return 0;

        case WM_NCHITTEST: {
            LRESULT hit = DefWindowProc(hwnd, uMsg, wParam, lParam);
            if (hit == HTCLIENT) return HTCAPTION;
            return hit;
        }

        case WM_SIZE: 
            width = LOWORD(lParam); 
            height = HIWORD(lParam);
            setup_projection(width, height);
            return 0;
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    WNDCLASS wc = { 0 };
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = "Void";
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    RegisterClass(&wc);

    int sWidth = GetSystemMetrics(SM_CXSCREEN);
    int sHeight = GetSystemMetrics(SM_CYSCREEN);
    int w = 1200;
    int h = 800;
    int x = (sWidth - w) / 2;
    int y = (sHeight - h) / 2;

    HWND hwnd = CreateWindowEx(0, wc.lpszClassName, "Void", 
        WS_POPUP | WS_VISIBLE, x, y, w, h, 0, 0, hInstance, 0);

    HDC hdc = GetDC(hwnd);
    PIXELFORMATDESCRIPTOR pfd = { sizeof(pfd), 1, PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER, PFD_TYPE_RGBA, 32 };
    SetPixelFormat(hdc, ChoosePixelFormat(hdc, &pfd), &pfd);
    wglMakeCurrent(hdc, wglCreateContext(hdc));

    srand(time(0));
    particles.reserve(MAX_PARTICLES);
    vertexArray.reserve(MAX_PARTICLES * 3);
    colorArray.reserve(MAX_PARTICLES * 3);

    for (int i = 0; i < MAX_PARTICLES; i++) {
        particles.push_back({
            {(float)rand()/RAND_MAX * 6 - 3, (float)rand()/RAND_MAX * 6 - 3, (float)rand()/RAND_MAX * 6 - 3},
            0.0f
        });
    }

    setup_projection(w, h);
    
    while (true) {
        MSG msg;
        while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
            if (msg.message == WM_QUIT) return 0;
            TranslateMessage(&msg); 
            DispatchMessage(&msg);
        }
        
        update_physics();
        display();
        SwapBuffers(hdc);
    }
    return 0;
}
