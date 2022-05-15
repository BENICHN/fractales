#include <stdio.h>
#include <string.h>
#include <iostream>
#include <fstream>
#include <GL/glew.h>
#include <GL/freeglut.h>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/gtx/norm.hpp>
#include "imgui.h"
#include "imgui_impl_glut.h"
#include "imgui_impl_opengl3.h"

using namespace std;
using namespace glm;

int magneti = -1;
const float zf = 1.15;
const int r2 = 36;

const int Nmax = 3;
GLfloat magnets[2 * Nmax] = {
    1, 0,
    -0.5, 0.866,
    -0.5, -0.866};

GLuint program;
GLuint vbo;
GLuint magnetsLocation;
GLuint zoomLocation;
GLuint offsetLocation;
GLuint fbo;
GLuint tex;

GLuint d2Location;
GLfloat fs_d = 0.2;
GLuint CLocation;
GLfloat fs_C = 0.2;
GLuint RLocation;
GLfloat fs_R = 0.15;
GLuint hLocation;
GLfloat fs_T = 25.0;
GLuint NLocation;
GLint fs_N = Nmax;
GLuint nLocation;
GLint fs_n = 1000;

GLuint useRK4Location;
GLboolean fs_useRK4 = false;
GLuint useLinesLocation;
GLboolean fs_useLines = false;
GLuint usePointsLocation;
GLboolean fs_usePoints = true;

bool isFractalValid;
ImVec2 fractalSize;
ImVec2 screenPositionAbsolute;
bool isMouseCapturedByFractal;
bool isFractalHovered;
int mouseX;
int mouseY;

// util functions

bool readFile(const char *pFileName, string &outFile)
{
    ifstream f(pFileName);

    bool ret = false;

    if (f.is_open())
    {
        string line;
        while (getline(f, line))
        {
            outFile.append(line);
            outFile.append("\n");
        }

        f.close();

        ret = true;
    }
    else
    {
        cout << "error while reading file " << pFileName;
    }

    return ret;
}
void writeBMP(char *name, int w, int h, char *img)
{
    int x;
    int y;
    int c;
    int r;
    int g;
    int b;
    FILE *f;
    int filesize = 54 + 3 * w * h;

    unsigned char bmpfileheader[14] = {'B', 'M', 0, 0, 0, 0, 0, 0, 0, 0, 54, 0, 0, 0};
    unsigned char bmpinfoheader[40] = {40, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 24, 0};
    unsigned char bmppad[3] = {0, 0, 0};

    bmpfileheader[2] = (unsigned char)(filesize);
    bmpfileheader[3] = (unsigned char)(filesize >> 8);
    bmpfileheader[4] = (unsigned char)(filesize >> 16);
    bmpfileheader[5] = (unsigned char)(filesize >> 24);

    bmpinfoheader[4] = (unsigned char)(w);
    bmpinfoheader[5] = (unsigned char)(w >> 8);
    bmpinfoheader[6] = (unsigned char)(w >> 16);
    bmpinfoheader[7] = (unsigned char)(w >> 24);
    bmpinfoheader[8] = (unsigned char)(h);
    bmpinfoheader[9] = (unsigned char)(h >> 8);
    bmpinfoheader[10] = (unsigned char)(h >> 16);
    bmpinfoheader[11] = (unsigned char)(h >> 24);

    f = fopen(name, "wb");
    fwrite(bmpfileheader, 1, 14, f);
    fwrite(bmpinfoheader, 1, 40, f);
    for (int i = 0; i < h; i++)
    {
        fwrite(img + (w * (h - i - 1) * 3), 3, w, f);
        fwrite(bmppad, 1, (4 - (w * 3) % 4) % 4, f);
    }

    free(img);
    fclose(f);
}
float screenToMath(float v, float zoom, float offset) { return zoom * (v + offset); }
float mathToScreen(float v, float zoom, float offset)
{
    v /= zoom;
    return (v - offset);
}

// control functions

void passiveMove(int x, int y)
{
    ImGui_ImplGLUT_MotionFunc(x, y);

    mouseX = x;
    mouseY = y;
}
void move(int x, int y)
{
    ImGui_ImplGLUT_MotionFunc(x, y);

    if (isMouseCapturedByFractal)
    {
        int dx = x - mouseX;
        int dy = y - mouseY;

        if (magneti == -1)
        {
            GLfloat offset[2];
            glGetUniformfv(program, offsetLocation, offset);
            glUniform2f(offsetLocation, offset[0] - dx, offset[1] + dy);
        }
        else
        {
            GLfloat zoom;
            glGetUniformfv(program, zoomLocation, &zoom);
            magnets[2 * magneti] += zoom * dx;
            magnets[2 * magneti + 1] -= zoom * dy;
            glUniform2fv(magnetsLocation, Nmax, magnets);
        }

        isFractalValid = false;
    }

    mouseX = x;
    mouseY = y;
}
void mouse(int button, int state, int x, int y)
{
    ImGuiIO &io = ImGui::GetIO();
    io.AddMouseButtonEvent(button, state == GLUT_DOWN);
    // io.AddMouseWheelEvent(0.0, state == 3 ? -io.MouseWheel : state == 4 ? io.MouseWheel
    //                                                                     : 0.0);
    {
        x -= screenPositionAbsolute.x;
        y -= screenPositionAbsolute.y;
        y = fractalSize.y - y;

        GLfloat zoom;
        GLfloat offset[2];

        float newZoom;
        float newOffset[2];

        switch (button)
        {
        case 0:
            switch (state)
            {
            case GLUT_DOWN:
                for (int i = 0; i < fs_N; i++)
                {
                    glGetUniformfv(program, offsetLocation, offset);
                    glGetUniformfv(program, zoomLocation, &zoom);
                    vec2 m = {mathToScreen(magnets[2 * i], zoom, offset[0]), mathToScreen(magnets[2 * i + 1], zoom, offset[1])};
                    if (length2(m - vec2(x, y)) <= r2)
                    {
                        magneti = i;
                    }
                }
                break;
            case GLUT_UP:
                magneti = -1;
                break;
            default:
                break;
            }
            break;
        default:
            break;
        }
    }
    if (isFractalHovered)
    {
        GLfloat zoom;
        GLfloat offset[2];

        float newZoom;
        float newOffset[2];

        switch (button)
        {
        case 3:
            glGetUniformfv(program, offsetLocation, offset);
            glGetUniformfv(program, zoomLocation, &zoom);

            newZoom = zoom / zf;
            newOffset[0] = (zoom / newZoom) * (x + offset[0]) - x;
            newOffset[1] = (zoom / newZoom) * (y + offset[1]) - y;

            glUniform1f(zoomLocation, newZoom);
            glUniform2f(offsetLocation, newOffset[0], newOffset[1]);

            isFractalValid = false;
            break;
        case 4:
            glGetUniformfv(program, offsetLocation, offset);
            glGetUniformfv(program, zoomLocation, &zoom);

            newZoom = zoom * zf;
            newOffset[0] = (zoom / newZoom) * (x + offset[0]) - x;
            newOffset[1] = (zoom / newZoom) * (y + offset[1]) - y;

            glUniform1f(zoomLocation, newZoom);
            glUniform2f(offsetLocation, newOffset[0], newOffset[1]);

            isFractalValid = false;
            break;

        default:
            break;
        }
    }
}
void keyboard(unsigned char key, int x, int y)
{
    ImGui_ImplGLUT_KeyboardFunc(key, x, y);

    switch (key)
    {
    case 'z':
        break;
    case 'f':
        glutFullScreenToggle();
        break;
    default:
        break;
    }
}

void createFB(int w, int h)
{
    glDeleteFramebuffers(1, &fbo);
    glDeleteTextures(1, &tex);

    // creating and binding fb
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    // creating and binding tex
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    // specifying texture
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    // setting parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // unbinding texture
    glBindTexture(GL_TEXTURE_2D, 0);
    // attaching tex to fb
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex, 0);

    // checking errors
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
    // unbinding fb
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void updateVariables()
{
    glUniform1i(useRK4Location, fs_useRK4);
    glUniform1i(useLinesLocation, fs_useLines);
    glUniform1i(usePointsLocation, fs_usePoints);
    glUniform1i(NLocation, fs_N);
    glUniform1i(nLocation, fs_n);
    glUniform1f(hLocation, fs_T / fs_n);
    glUniform1f(RLocation, fs_R);
    glUniform1f(CLocation, fs_C);
    glUniform1f(d2Location, fs_d * fs_d);
}

void renderFractal()
{
    if (isFractalValid)
        return;
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glClear(GL_COLOR_BUFFER_BIT);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glDrawArrays(GL_QUADS, 0, 4);
    glDisableVertexAttribArray(0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    isFractalValid = true;
}

static ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
void my_display_code()
{
    ImGui::Begin("Fractale");
    {
        ImGui::BeginChild("Fractale Render");
        ImVec2 size = ImGui::GetWindowSize();
        if (fractalSize.x != size.x || fractalSize.y != size.y)
        {
            isFractalValid = false;
            fractalSize = size;
        }
        if (!isFractalValid)
            createFB(fractalSize.x, fractalSize.y);

        screenPositionAbsolute = ImGui::GetItemRectMin();

        isMouseCapturedByFractal = ImGui::IsItemActive();
        isFractalHovered = ImGui::IsWindowHovered(ImGuiHoveredFlags_AllowWhenBlockedByActiveItem);

        updateVariables();
        renderFractal();

        ImGui::Image((ImTextureID)tex, fractalSize, ImVec2(0, 1), ImVec2(1, 0));
        ImGui::EndChild();
    }
    ImGui::End();

    ImGui::Begin("Paramètres");
    {
        if (ImGui::InputInt("Nombre d'aimants", &fs_N, 1, Nmax))
        {
            GLint N;
            glGetUniformiv(program, NLocation, &N);

            fs_N = clamp(fs_N, 1, Nmax);
            if (N != fs_N)
                isFractalValid = false;
        }
        if (ImGui::DragInt("n", &fs_n, 10.0f, 0, 100000, "%.3f", ImGuiSliderFlags_Logarithmic))
            isFractalValid = false;
        if (ImGui::DragFloat("T", &fs_T, 0.1f, 0.0f, 300.0f, "%.3f", ImGuiSliderFlags_Logarithmic))
            isFractalValid = false;
        if (ImGui::DragFloat("R", &fs_R, 0.01f, 0.0f, 20.0f, "%.3f", ImGuiSliderFlags_Logarithmic))
            isFractalValid = false;
        if (ImGui::DragFloat("C", &fs_C, 0.01f, 0.0f, 20.0f, "%.3f", ImGuiSliderFlags_Logarithmic))
            isFractalValid = false;
        if (ImGui::DragFloat("d", &fs_d, 0.01f, 0.0f, 20.0f, "%.3f", ImGuiSliderFlags_Logarithmic))
            isFractalValid = false;
        if (ImGui::Checkbox("RK4", (bool *)&fs_useRK4))
            isFractalValid = false;
        if (ImGui::Checkbox("Linges", (bool *)&fs_useLines))
            isFractalValid = false;
        if (ImGui::Checkbox("Points", (bool *)&fs_usePoints))
            isFractalValid = false;
    }
    ImGui::End();
}

void display()
{
    // Start the Dear ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGLUT_NewFrame();

    my_display_code();

    // Rendering
    ImGui::Render();
    ImGuiIO &io = ImGui::GetIO();
    glViewport(0, 0, (GLsizei)io.DisplaySize.x, (GLsizei)io.DisplaySize.y);
    glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
    glClear(GL_COLOR_BUFFER_BIT);
    // glUseProgram(0); // You may want this if using this code in an OpenGL 3+ context where shaders may be bound, but prefer using the GL3+ code.
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    glutSwapBuffers();
    glutPostRedisplay();
}
void createVertexBuffer()
{
    vec3 vertices[4] = {
        {-1.0f, -1.0f, 0.0f},
        {1.0f, -1.0f, 0.0f},
        {1.0f, 1.0f, 0.0f},
        {-1.0f, 1.0f, 0.0f}};

    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
}
void addShader(GLuint program, const char *pShaderText, GLenum shaderType)
{
    GLuint sho = glCreateShader(shaderType);
    assert(sho != 0);

    const GLchar *p[1];
    p[0] = pShaderText;

    GLint lengths[1];
    lengths[0] = (GLint)strlen(pShaderText);

    glShaderSource(sho, 1, p, lengths);

    glCompileShader(sho);

    GLint success;
    glGetShaderiv(sho, GL_COMPILE_STATUS, &success);

    if (!success)
    {
        GLchar InfoLog[1024];
        glGetShaderInfoLog(sho, 1024, NULL, InfoLog);
        fprintf(stderr, "Error compiling shader type %d: '%s'\n", shaderType, InfoLog);
        exit(1);
    }

    glAttachShader(program, sho);
}

const char *pVSFileName = "shader.vs";
const char *pFSFileName = "shader.fs";
void compileShaders()
{
    program = glCreateProgram();
    assert(program != 0);

    string vs, fs;

    if (!readFile(pVSFileName, vs))
    {
        exit(1);
    };
    addShader(program, vs.c_str(), GL_VERTEX_SHADER);

    if (!readFile(pFSFileName, fs))
    {
        exit(1);
    };
    addShader(program, fs.c_str(), GL_FRAGMENT_SHADER);

    GLint Success = 0;
    GLchar ErrorLog[1024] = {0};

    glLinkProgram(program);

    glGetProgramiv(program, GL_LINK_STATUS, &Success);
    if (Success == 0)
    {
        glGetProgramInfoLog(program, sizeof(ErrorLog), NULL, ErrorLog);
        fprintf(stderr, "Error linking shader program: '%s'\n", ErrorLog);
        exit(1);
    }

    glValidateProgram(program);
    glGetProgramiv(program, GL_VALIDATE_STATUS, &Success);
    if (!Success)
    {
        glGetProgramInfoLog(program, sizeof(ErrorLog), NULL, ErrorLog);
        fprintf(stderr, "Invalid shader program: '%s'\n", ErrorLog);
        exit(1);
    }

    glUseProgram(program);

    // linking to uniform variables

    magnetsLocation = glGetUniformLocation(program, "magnets");
    if (magnetsLocation == -1)
    {
        printf("Error getting uniform location of 'magnets'\n");
        exit(1);
    }
    glUniform2fv(magnetsLocation, Nmax, magnets);

    zoomLocation = glGetUniformLocation(program, "zoom");
    if (zoomLocation == -1)
    {
        printf("Error getting uniform location of 'zoom'\n");
        exit(1);
    }
    glUniform1f(zoomLocation, 0.005);

    offsetLocation = glGetUniformLocation(program, "offset");
    if (offsetLocation == -1)
    {
        printf("Error getting uniform location of 'offset'\n");
        exit(1);
    }
    glUniform2f(offsetLocation, -glutGet(GLUT_WINDOW_WIDTH) / 2, -glutGet(GLUT_WINDOW_HEIGHT) / 2);

    nLocation = glGetUniformLocation(program, "n");
    if (nLocation == -1)
    {
        printf("Error getting uniform location of 'n'\n");
        exit(1);
    }

    NLocation = glGetUniformLocation(program, "N");
    if (NLocation == -1)
    {
        printf("Error getting uniform location of 'N'\n");
        exit(1);
    }

    hLocation = glGetUniformLocation(program, "h");
    if (hLocation == -1)
    {
        printf("Error getting uniform location of 'h'\n");
        exit(1);
    }

    RLocation = glGetUniformLocation(program, "R");
    if (RLocation == -1)
    {
        printf("Error getting uniform location of 'R'\n");
        exit(1);
    }

    CLocation = glGetUniformLocation(program, "C");
    if (CLocation == -1)
    {
        printf("Error getting uniform location of 'C'\n");
        exit(1);
    }

    d2Location = glGetUniformLocation(program, "d2");
    if (d2Location == -1)
    {
        printf("Error getting uniform location of 'd2'\n");
        exit(1);
    }

    useRK4Location = glGetUniformLocation(program, "useRK4");
    if (useRK4Location == -1)
    {
        printf("Error getting uniform location of 'useRK4'\n");
        exit(1);
    }

    useLinesLocation = glGetUniformLocation(program, "useLines");
    if (useLinesLocation == -1)
    {
        printf("Error getting uniform location of 'useLines'\n");
        exit(1);
    }

    usePointsLocation = glGetUniformLocation(program, "usePoints");
    if (usePointsLocation == -1)
    {
        printf("Error getting uniform location of 'usePoints'\n");
        exit(1);
    }

    updateVariables();
}

int main(int argc, char **argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
    int width = 1920;
    int height = 1080;
    glutInitWindowSize(width, height);
    int x = 200;
    int y = 100;
    glutInitWindowPosition(x, y);
    int win = glutCreateWindow("Le Pendule");
    printf("window id: %d\n", win);

    // Must be done after glut is initialized!
    GLenum res = glewInit();
    if (res != GLEW_OK)
    {
        fprintf(stderr, "Error: '%s'\n", glewGetErrorString(res));
        return 1;
    }

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    io.ConfigWindowsMoveFromTitleBarOnly = true;
    // io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    // ImGui::StyleColorsClassic();

    // Setup Platform/Renderer backends
    // FIXME: Consider reworking this example to install our own GLUT funcs + forward calls ImGui_ImplGLUT_XXX ones, instead of using ImGui_ImplGLUT_InstallFuncs().
    ImGui_ImplGLUT_Init();
    ImGui_ImplGLUT_InstallFuncs();
    ImGui_ImplOpenGL3_Init();

    GLclampf Red = 0.0f, Green = 0.0f, Blue = 0.0f, Alpha = 0.0f;
    glClearColor(Red, Green, Blue, Alpha);

    createVertexBuffer();

    compileShaders();

    glutDisplayFunc(display);
    glutPassiveMotionFunc(passiveMove);
    glutMotionFunc(move);
    glutMouseFunc(mouse);
    glutKeyboardFunc(keyboard);

    glutMainLoop();

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGLUT_Shutdown();
    ImGui::DestroyContext();

    return 0;
}
