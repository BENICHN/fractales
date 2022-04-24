#include <stdio.h>
#include <string.h>
#include <iostream>
#include <fstream>
#include <GL/glew.h>
#include <GL/freeglut.h>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/gtx/norm.hpp>

using namespace std;
using namespace glm;

const int N = 3;
const float zf = 1.15;
const int r2 = 36;

int magneti = -1;
GLfloat magnets[] = {1, 0, -0.5, 0.866, -0.5, -0.866};

GLuint program;
GLuint VBO;
GLuint magnetsLocation;
GLuint zoomLocation;
GLuint offsetLocation;

int mouseX;
int mouseY;

void passiveMove(int x, int y)
{
    mouseX = x;
    mouseY = y;
}

float screenToMath(float v, float zoom, float offset) { return zoom * (v + offset); }
float mathToScreen(float v, float zoom, float offset)
{
    v /= zoom;
    return (v - offset);
}

void move(int x, int y)
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
        glUniform2fv(magnetsLocation, N, magnets);
    }

    mouseX = x;
    mouseY = y;

    glutPostRedisplay();
}

void mouse(int button, int state, int x, int y)
{
    GLfloat zoom;
    GLfloat offset[2];

    float newZoom;
    float newOffset[2];

    y = glutGet(GLUT_WINDOW_HEIGHT) - y;

    switch (button)
    {
    case 0:
        switch (state)
        {
        case GLUT_DOWN:
            glGetUniformfv(program, offsetLocation, offset);
            glGetUniformfv(program, zoomLocation, &zoom);
            for (int i = 0; i < N; i++)
            {
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
    case 3:
        glGetUniformfv(program, offsetLocation, offset);
        glGetUniformfv(program, zoomLocation, &zoom);

        newZoom = zoom / zf;
        newOffset[0] = (zoom / newZoom) * (x + offset[0]) - x;
        newOffset[1] = (zoom / newZoom) * (y + offset[1]) - y;

        glUniform1f(zoomLocation, newZoom);
        glUniform2f(offsetLocation, newOffset[0], newOffset[1]);

        glutPostRedisplay();
        break;
    case 4:
        glGetUniformfv(program, offsetLocation, offset);
        glGetUniformfv(program, zoomLocation, &zoom);

        newZoom = zoom * zf;
        newOffset[0] = (zoom / newZoom) * (x + offset[0]) - x;
        newOffset[1] = (zoom / newZoom) * (y + offset[1]) - y;

        glUniform1f(zoomLocation, newZoom);
        glUniform2f(offsetLocation, newOffset[0], newOffset[1]);

        glutPostRedisplay();
        break;

    default:
        break;
    }
}

void keyboard(unsigned char key, int x, int y)
{
    switch (key)
    {
    case 'z':
        break;

    default:
        break;
    }
}

static void RenderSceneCB()
{
    glClear(GL_COLOR_BUFFER_BIT);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    glEnableVertexAttribArray(0);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

    glDrawArrays(GL_QUADS, 0, 4);

    glDisableVertexAttribArray(0);

    glutSwapBuffers();
}

static void CreateVertexBuffer()
{
    vec3 Vertices[4];

    Vertices[0] = {-1.0f, -1.0f, 0.0f};
    Vertices[1] = {1.0f, -1.0f, 0.0f};
    Vertices[2] = {1.0f, 1.0f, 0.0f};
    Vertices[3] = {-1.0f, 1.0f, 0.0f};

    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vertices), Vertices, GL_STATIC_DRAW);
}

static void AddShader(GLuint program, const char *pShaderText, GLenum ShaderType)
{
    GLuint ShaderObj = glCreateShader(ShaderType);

    if (ShaderObj == 0)
    {
        fprintf(stderr, "Error creating shader type %d\n", ShaderType);
        exit(0);
    }

    const GLchar *p[1];
    p[0] = pShaderText;

    GLint Lengths[1];
    Lengths[0] = (GLint)strlen(pShaderText);

    glShaderSource(ShaderObj, 1, p, Lengths);

    glCompileShader(ShaderObj);

    GLint success;
    glGetShaderiv(ShaderObj, GL_COMPILE_STATUS, &success);

    if (!success)
    {
        GLchar InfoLog[1024];
        glGetShaderInfoLog(ShaderObj, 1024, NULL, InfoLog);
        fprintf(stderr, "Error compiling shader type %d: '%s'\n", ShaderType, InfoLog);
        exit(1);
    }

    glAttachShader(program, ShaderObj);
}

const char *pVSFileName = "shader.vs";
const char *pFSFileName = "shader.fs";

bool ReadFile(const char *pFileName, string &outFile)
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

static void CompileShaders()
{
    program = glCreateProgram();

    if (program == 0)
    {
        fprintf(stderr, "Error creating shader program\n");
        exit(1);
    }

    std::string vs, fs;

    if (!ReadFile(pVSFileName, vs))
    {
        exit(1);
    };

    AddShader(program, vs.c_str(), GL_VERTEX_SHADER);

    if (!ReadFile(pFSFileName, fs))
    {
        exit(1);
    };

    AddShader(program, fs.c_str(), GL_FRAGMENT_SHADER);

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

    magnetsLocation = glGetUniformLocation(program, "magnets");
    if (magnetsLocation == -1)
    {
        printf("Error getting uniform location of 'magnets'\n");
        exit(1);
    }
    glUniform2fv(magnetsLocation, N, magnets);

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
    glUniform2f(offsetLocation, -glutGet(GLUT_WINDOW_WIDTH) /2, -glutGet(GLUT_WINDOW_HEIGHT) / 2);
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

    GLclampf Red = 0.0f, Green = 0.0f, Blue = 0.0f, Alpha = 0.0f;
    glClearColor(Red, Green, Blue, Alpha);

    CreateVertexBuffer();

    CompileShaders();

    glutDisplayFunc(RenderSceneCB);
    glutPassiveMotionFunc(passiveMove);
    glutMotionFunc(move);
    glutMouseFunc(mouse);
    glutKeyboardFunc(keyboard);

    glutMainLoop();

    return 0;
}
