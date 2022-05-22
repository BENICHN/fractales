#version 460 core

in vec4 gl_FragCoord;
out vec4 fragColor;

precision highp float;

// paramètres
const int Nmax = 1;
const int N = Nmax;
const int r2 = 36;
// const vec4 colors[] = vec4[](vec4(0.172,0.243,0.313,0),vec4(0.086,0.627,0.521,0),vec4(0.752,0.223,0.168,0),vec4(0.557,0.267,0.678,0));
const vec4 colors[] = vec4[](
    vec4(0.898,0.584,0,0),
    vec4(0.517,0,0.196,0),
    vec4(0,0.149,0.258,0),
    vec4(0.898,0.854,0.854,0),
    vec4(0.023,0.654,0.490,0),
    vec4(0.580,0.482,0.827,0),
    vec4(0.278,0.294,0.141,0),
    vec4(0.898,0.454,0.737,0),
    vec4(0.007,0.015,0.058,0)
);

uniform int n;
uniform int p;

uniform vec2 magnets[Nmax];
uniform float zoom;
uniform vec2 offset;

uniform bool useJulia;
uniform bool useLines;
uniform bool usePoints;

float sq(float x) { return x * x; }
float nsq(vec2 v) { return dot(v,v); }

float cabs(vec2 z)
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

vec2 cexp(vec2 z)
{
	return exp(z.x) * vec2(cos(z.y), sin(z.y));
}

vec2 clog(vec2 z)
{
	return vec2(log(cabs(z)), atan(z.y, z.x));
}

vec2 cpow(vec2 z, vec2 a)
{
	return cexp(cmul(a, clog(z)));
}


vec2 ccos(vec2 z)
{
	return(cexp(vec2(-z.y, z.x)) + cexp(vec2(z.y, -z.x))) / 2.0;
}

vec2 csin(vec2 z)
{
	vec2 t =(cexp(vec2(-z.y, z.x)) - cexp(vec2(z.y, -z.x))) / 2.0;
	return vec2(-t.y, -t.x);
}

vec2 ctan(vec2 z)
{
	return cdiv(csin(z), ccos(z));
}

vec2 ccosh(vec2 z)
{
	return(cexp(z) + cexp(-z)) / 2.0;
}

vec2 csinh(vec2 z)
{
	return(cexp(z) - cexp(-z)) / 2.0;
}

vec2 ctanh(vec2 z)
{
	return cdiv(csinh(z), ccosh(z));
}

vec2 cintpow(vec2 z, int power)
{
    bool isneg = power < 0;
    if (isneg) power = -power;
    vec2 res = vec2(1, 0);
    for (int i = 0; i < power; i++)
    {
        res = cmul (res, z);
    }
    return isneg ? cdiv(vec2(1, 0), res) : res;
}

int runSeq(vec2 z0, vec2 c)
{
    vec2 z = z0;

    for (int i = 0; i < n; i++)
    {
        if (nsq(z) > 4) return i;
        z = cintpow(z, p) + c;
    }

    return -1;
}

int julia(vec2 z0)
{
    vec2 c = magnets[0];
    return runSeq(z0, c);
}

int mandelbrot(vec2 c)
{
    return runSeq(c, c);
}

// pour passer des coordonnées écran aux coordonnées math
vec2 screenToMath(vec2 v) { return zoom * (v + offset); }
vec2 mathToScreen(vec2 v) { v /= zoom; return (v - offset); }

vec3 hsv2rgb(vec3 c)
{
    vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
    vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
    return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
}

void main()
{
    vec2 xy = vec2(gl_FragCoord.x, gl_FragCoord.y);
    vec2 p0 = screenToMath(xy);

    if (usePoints && useJulia) // coloration des aimants en violet
    {
        for (int i = 0; i < N; i++)
        {
            if (nsq(xy-mathToScreen(magnets[i])) <= r2)
            {
                fragColor = colors[Nmax+1];
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

    int r = useJulia ? julia(p0) : mandelbrot(p0); // simulation
    float hue = float(r)/float(n);
    float saturation = 1.0;
    float value;
    float t;
    if (r == -1) // choix de la couleur en conséquence
    {
        // fragColor = colors[0];
        value = 0.0;
    }
    else
    {
        value = 1.0;
        // t = 1.0 - log(float(r))/log(float(n));
        // fragColor = t * colors[1];
    }
    // fragColor = t * colors[0] + (1-t) * colors[1];
    fragColor = hsv2rgb(vec3(hue, saturation, value)).xyzz;
}