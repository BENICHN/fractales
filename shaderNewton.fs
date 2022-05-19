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

uniform int n;

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

float cabs (vec2 z)
{
	return length(z);
}

vec2 cconj(vec2 a)
{
    return vec2(a.x, -a.y);
}

vec2 cmul(vec2 a, vec2 b)
{
    return vec2(a.x * b.x - a.y * b.y, a.x * b.y + a.y * b.x);
}

vec2 cdiv(vec2 a, vec2 b)
{
    return cmul(a, cconj(b)) / nsq(b);
}

vec2 cexp (vec2 z)
{
	return exp(z.x) * vec2(cos(z.y), sin(z.y));
}

vec2 clog (vec2 z)
{
	return vec2(log(cabs(z)), atan(z.y, z.x));
}

vec2 cpow (vec2 z, vec2 a)
{
	return cexp(cmul(a, clog(z)));
}


vec2 ccos (vec2 z)
{
	return (cexp(vec2(-z.y, z.x)) + cexp(vec2(z.y, -z.x))) / 2.0;
}

vec2 csin (vec2 z)
{
	vec2 t = (cexp(vec2(-z.y, z.x)) - cexp(vec2(z.y, -z.x))) / 2.0;
	return vec2(-t.y, -t.x);
}

vec2 ctan (vec2 z)
{
	return cdiv(csin(z), ccos(z));
}

vec2 ccosh (vec2 z)
{
	return (cexp(z) + cexp(-z)) / 2.0;
}

vec2 csinh (vec2 z)
{
	return (cexp(z) - cexp(-z)) / 2.0;
}

vec2 ctanh (vec2 z)
{
	return cdiv(csinh(z), ccosh(z));
}

vec2 poly (vec2 x)
{
    vec2 p = vec2(1, 0);
    for (int i = 0; i < N; i++)
    {
        p = cmul(p, x - magnets[i]);
    }

    return p;
}

vec2 polyprime (vec2 x)
{
    vec2 s = vec2(0,0);
    for (int i = 0; i < N; i++)
    {
        vec2 p = vec2(1, 0);
        for (int j = 0; j < N; j++)
        {
            if (j != i)
            {
                p = cmul(p, x - magnets[j]);
            }
        }
        s += p;
    }

    return s;
}

vec2 newton(vec2 p0)
{
    vec2 p = p0;

    for (int i = 0; i < n; i++)
    {
        vec2 diff = cdiv(poly(p), polyprime(p));
        p -= diff;
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

    vec2 pf = newton(p0); // simulation
    float dists[Nmax];
    for (int i = 0; i < N; i++) // calcul de l'aimant le plus proche
    {
        dists[i] = nsq(pf - magnets[i]);
    }
    int im = mini(N, dists);
    fragColor = colors[im]; // choix de la couleur en conséquence
}