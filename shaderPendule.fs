#version 460 core

in vec4 gl_FragCoord;
out vec4 fragColor;

precision highp float;

// paramètres
const int Nmax = 8;
const int r2 = 36;
//const vec4 colors[Nmax+1] = vec4[](vec4(0.172,0.243,0.313,0),vec4(0.086,0.627,0.521,0),vec4(0.752,0.223,0.168,0),vec4(0.557,0.267,0.678,0));
const vec4 colors[] = vec4[](
    vec4(0,0.149,0.258,0),
    vec4(0.517,0,0.196,0),
    vec4(0.898,0.584,0,0),
    vec4(0.898,0.854,0.854,0),
    vec4(0.023,0.654,0.490,0),
    vec4(0.580,0.482,0.827,0),
    vec4(0.278,0.294,0.141,0),
    vec4(0.898,0.454,0.737,0),
    vec4(0.007,0.015,0.058,0)
);

uniform int N;
uniform vec2 magnets[Nmax];
uniform float zoom;
uniform vec2 offset;

uniform float h;
uniform int n;
uniform float d2;
uniform float C;
uniform float R;

uniform bool useRK4;
uniform bool useLines;
uniform bool usePoints;

// pour trouver l'indice du plus petit élémant d'un tableau
int mini(int size, float a[Nmax])
{
    int r = -1;
    for (int i = 0; i < size; i++)
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
    vec2 v = vec2(0,0);
    vec2 a;

    for (int i = 0; i < n; i++)
    {
        a = getAccel(v, p);
        v += h * a;
        p += h * v;
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

    for (int i = 0; i < n; i++)
    {
        k1 = getAccel(v, p);
        k2 = getAccel(v + 0.5*h*k1, p + 0.5*h*v);
        k3 = getAccel(v + 0.5*h*k2, p + 0.5*h*v + 0.25*sq(h)*k1);
        k4 = getAccel(v + h*k3, p + h*v + 0.5*sq(h)*k2);
        
        p += h*v + sq(h)/6*(k1+k2+k3);
        v += h/6*(k1+2.*k2+2.*k3+k4);
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

    if (usePoints) // coloration des aimants en violet
    {
        for (int i = 0; i < N; i++)
        {
            if (nsq(xy-mathToScreen(magnets[i])) <= r2)
            {
                fragColor = colors[Nmax];
                return;
            }
        }
    }

    if (useLines) // coloration des lignes noires
    {
        vec2 zxy = mathToScreen(vec2(0,0));
        if (abs(xy.x - zxy.x) < 1 || abs(xy.y - zxy.y) < 1)
        {
            fragColor = vec4(0,0,0,0);
            return;
        }
    }

    vec2 pf = useRK4 ? rk4(p0) : predict(p0); // simulation
    float dists[Nmax];
    for (int i = 0; i < N; i++) // calcul de l'aimant le plus proche
    {
        dists[i] = nsq(pf - magnets[i]);
    }
    int im = mini(N, dists);
    fragColor = colors[im]; // choix de la couleur en conséquence
}