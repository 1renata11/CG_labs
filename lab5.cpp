#include </Library/Developer/CommandLineTools/SDKs/MacOSX.sdk/System/Library/Frameworks/GLUT.framework/Headers/glut.h>
#include <cmath>
#include <vector>
#include <algorithm>

using namespace std;

struct Vertex {
    float x, y, z;
};

struct Light {
    Vertex position;
    float intensityR, intensityG, intensityB;
};

struct Camera {
    Vertex position;
};

vector<Vertex> vertices = {
        {-1, -1, -1}, {1, -1, -1}, {1, 1, -1}, {-1, 1, -1},
        {-1, -1, 1},  {1, -1, 1},  {1, 1, 1},  {-1, 1, 1}
};

vector<vector<int>> faces = {
        {0, 1, 2, 3},
        {7, 6, 5, 4},
        {4, 5, 1, 0},
        {6, 7, 3, 2},
        {0, 3, 7, 4},
        {5, 6, 2, 1}
};

Light light = {{0, 0, 5}, 1.0, 1.0, 1.0};
Camera camera = {{0, 0, 5}};

float angleX = 30, angleY = 30, angleZ = 30;
float k_d = 0.8f;
float k_s = 0.5f;
float n = 16.0f;

Vertex normalize(const Vertex& v) {
    float len = sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
    return {v.x / len, v.y / len, v.z / len};
}

Vertex calculateNormal(const vector<int>& face) {
    Vertex v0 = vertices[face[0]];
    Vertex v1 = vertices[face[1]];
    Vertex v2 = vertices[face[2]];

    float x1 = v1.x - v0.x, y1 = v1.y - v0.y, z1 = v1.z - v0.z;
    float x2 = v2.x - v0.x, y2 = v2.y - v0.y, z2 = v2.z - v0.z;

    Vertex normal = {
            - y1 * z2 + z1 * y2,
            - z1 * x2 + x1 * z2,
            - x1 * y2 + y1 * x2
    };

    return normalize(normal);
}

float calculateIntensity(const Vertex& normal, const Vertex& vertex) {
    Vertex L = {light.position.x - vertex.x, light.position.y - vertex.y, light.position.z - vertex.z};
    L = normalize(L);

    float N_dot_L = max(0.0f, normal.x * L.x + normal.y * L.y + normal.z * L.z);

    Vertex V = {camera.position.x - vertex.x, camera.position.y - vertex.y, camera.position.z - vertex.z};
    V = normalize(V);

    Vertex R = {
            2 * N_dot_L * normal.x - L.x,
            2 * N_dot_L * normal.y - L.y,
            2 * N_dot_L * normal.z - L.z
    };
    R = normalize(R);

    float R_dot_V = max(0.0f, R.x * V.x + R.y * V.y + R.z * V.z);
    float specular = pow(R_dot_V, n);

    return k_d * light.intensityR * N_dot_L + k_s * light.intensityR * specular;
}

float interpolate(float i1, float i2, float t) {
    return i1 + (i2 - i1) * t;
}

void fillTriangle(const Vertex& v0, const Vertex& v1, const Vertex& v2, float i0, float i1, float i2) {
    int steps = 1000;
    for (int j = 0; j <= steps; j++) {
        float t1 = j / (float)steps;
        float x1 = interpolate(v0.x, v1.x, t1);
        float y1 = interpolate(v0.y, v1.y, t1);
        float i1_interp = interpolate(i0, i1, t1);

        float x2 = interpolate(v0.x, v2.x, t1);
        float y2 = interpolate(v0.y, v2.y, t1);
        float i2_interp = interpolate(i0, i2, t1);

        for (int k = 0; k <= steps; k++) {
            float t2 = k / (float)steps;
            float x = interpolate(x1, x2, t2);
            float y = interpolate(y1, y2, t2);
            float intensity = interpolate(i1_interp, i2_interp, t2);

            glColor3f(intensity, intensity, intensity);
            glVertex2f(x, y);
        }
    }
}

void fillFace(const vector<int>& face) {
    Vertex normal = calculateNormal(face);

    vector<float> intensities;
    for (int idx : face) {
        intensities.push_back(calculateIntensity(normal, vertices[idx]));
    }

    glBegin(GL_POINTS);
    fillTriangle(vertices[face[0]], vertices[face[1]], vertices[face[2]], intensities[0], intensities[1], intensities[2]);
    fillTriangle(vertices[face[0]], vertices[face[2]], vertices[face[3]], intensities[0], intensities[2], intensities[3]);
    glEnd();
}

void drawParallelProjection() {
    for (const auto& face : faces) {
        Vertex normal = calculateNormal(face);
        if (normal.z > 0) {
            fillFace(face);
        }
    }
}

void rotateVertices(float angle, float x, float y, float z) {
    float rad = angle * M_PI / 180.0;
    float c = cos(rad);
    float s = sin(rad);
    float len = sqrt(x * x + y * y + z * z);
    x /= len; y /= len; z /= len;

    float M[4][4] = {
            {c + (1 - c) * x * x, (1 - c) * x * y - s * z, (1 - c) * x * z + s * y, 0},
            {(1 - c) * y * x + s * z, c + (1 - c) * y * y, (1 - c) * y * z - s * x, 0},
            {(1 - c) * z * x - s * y, (1 - c) * z * y + s * x, c + (1 - c) * z * z, 0},
            {0, 0, 0, 1}
    };

    for (auto &vertex : vertices) {
        float x = vertex.x, y = vertex.y, z = vertex.z;
        vertex.x = x * M[0][0] + y * M[0][1] + z * M[0][2];
        vertex.y = x * M[1][0] + y * M[1][1] + z * M[1][2];
        vertex.z = x * M[2][0] + y * M[2][1] + z * M[2][2];
    }
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glPushMatrix();
    rotateVertices(angleX, 1, 0, 0);
    rotateVertices(angleY, 0, 1, 0);
    rotateVertices(angleZ, 0, 0, 1);

    drawParallelProjection();

    glPopMatrix();
    glutSwapBuffers();
}

void timer(int value) {
    angleX += 1.0;
    angleY += 1.0;
    angleZ += 1.0;
    if (angleX >= 360) angleX = 0;
    if (angleY >= 360) angleY = 0;
    if (angleZ >= 360) angleZ = 0;
    glutPostRedisplay();
    glutTimerFunc(100, timer, 0);
}

void initOpenGL() {
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(-2, 2, -2, 2);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glClearColor(0.0, 0.0, 0.0, 1.0);
}

int main(int argc, char **argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(800, 800);
    glutCreateWindow("Gouraud Shading - 3D Parallel Projection");

    initOpenGL();
    glutDisplayFunc(display);
    glutTimerFunc(100, timer, 0);
    glutMainLoop();
    return 0;
}
