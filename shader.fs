#version 400 core

in vec4 gl_FragCoord;
out vec4 fragColor;

precision highp float;

// paramètres
const int N = 3;
const int r2 = 36;
const vec4 colors[] = vec4[](vec4(0.172,0.243,0.313,0),vec4(0.086,0.627,0.521,0),vec4(0.752,0.223,0.168,0),vec4(0.557,0.267,0.678,0));
const float T = 25;
const int n = 1000;
const int mul = 1000;
const int st = n/mul;
const float tol = 0.5;
const float h = T/n;
const float R = 0.15;
const float C = 0.2;
const float d2 = 0.04;

uniform vec2 magnets[N];
uniform float zoom;
uniform vec2 offset;

// pour trouver l'indice du plus petit élémant d'un tableau
int mini(float a[N])
{
    int r = -1;
    for (int i = 0; i < N; i++)
    {
        if (r == -1)
        {
            r = 0;
        }
        else if (a[i] < a[r])
        {
            r = i;
        }
    }
    return r;
}

float sq(float x) { return x * x; }
float nsq(vec2 v) { return dot(v,v); }

// fonction accélération(vitesse, position)
vec2 getAccel(vec2 v, vec2 p)
{
    vec2 m;
    vec2 s = vec2(0,0);
    for (int i = 0; i < N; i++)
    {
        m = magnets[i];
        s += (m - p) / pow(sq(m.x - p.x) + sq(m.y - p.y) + d2, 1.5);
    }

    return s - R * v - C * p;
}

// algorithme de Euler explicite
vec2 predict(vec2 p0)
{
    vec2 p = p0;
    vec2 pt;
    vec2 v = vec2(0,0);
    vec2 a;

    for (int i = 0; i < st; i++)
    {
        pt = p;
        for (int i = 0; i < mul; i++)
        {
            a = getAccel(v, p);
            v += h * a;
            p += h * v;
        }
        if (nsq(p - pt) < tol)
        {
            return p;
        }
    }

    return p;
}

// algorithme de Runge-Kutta d'ordre 4
vec2 rk4(vec2 p0)
{
    vec2 p = p0;
    vec2 pt;
    vec2 v = vec2(0,0);
    
    vec2 k1;
    vec2 k2;
    vec2 k3;
    vec2 k4;

    for (int i = 0; i < st; i++)
    {
        pt = p;
        for (int i = 0; i < mul; i++)
        {
            k1 = getAccel(v, p);
            k2 = getAccel(v + 0.5*h*k1, p + 0.5*h*v);
            k3 = getAccel(v + 0.5*h*k2, p + 0.5*h*v + 0.25*sq(h)*k1);
            k4 = getAccel(v + h*k3, p + h*v + 0.5*sq(h)*k2);

            p += h*v + sq(h)/6*(k1+k2+k3);
            v += h/6*(k1+2.*k2+2.*k3+k4);
        }
        if (nsq(p - pt) < tol)
        {
            return p;
        }
    }

    return p;
}

// pour passer des coordonnées écran aux coordonnées math
vec2 screenToMath(vec2 v) { return zoom * (v + offset); }
vec2 mathToScreen(vec2 v) { v /= zoom; return (v - offset); }

void main()
{
    vec2 xy = vec2(gl_FragCoord.x, gl_FragCoord.y);
    vec2 p0 = screenToMath(xy);

    // coloration des aimants en violet
    for (int i = 0; i < N; i++)
    {
        if (nsq(xy-mathToScreen(magnets[i])) <= r2)
        {
            fragColor = colors[N];
            return;
        }
    }

    // coloration des lignes noires
    vec2 zxy = mathToScreen(vec2(0,0));
    if (abs(xy.x - zxy.x) < 1 || abs(xy.y - zxy.y) < 1)
    {
        fragColor = vec4(0,0,0,0);
        return;
    }

    vec2 pf = predict(p0); // simulation
    float dists[N];
    for (int i = 0; i < N; i++) // calcul de l'aimant le plus proche
    {
        dists[i] = nsq(pf - magnets[i]);
    }
    int im = mini(dists);
    fragColor = colors[im]; // choix de la couleur en conséquence
}