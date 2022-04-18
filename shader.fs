#version 330 core

in vec4 gl_FragCoord;
out vec4 gl_FragColor;

precision highp float;

const int N = 3;
const int r2 = 36;
const vec4 colors[] = vec4[](vec4(0.172,0.243,0.313,0),vec4(0.086,0.627,0.521,0),vec4(0.752,0.223,0.168,0),vec4(0.557,0.267,0.678,0));
const float h = 0.1;
const float R = 0.15;
const float C = 0.2;
const float d2 = 0.04;

uniform vec2 magnets[N];
uniform float zoom;
uniform vec2 offset;

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

vec2 predict(vec2 p0, int st, int mul, float tol)
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

vec2 screenToMath(vec2 v) { return zoom * (v + offset); }
vec2 mathToScreen(vec2 v) { v /= zoom; return (v - offset); }

void main()
{
    vec2 xy = vec2(gl_FragCoord.x, gl_FragCoord.y);
    vec2 p0 = screenToMath(xy);

    for (int i = 0; i < N; i++)
    {
        if (nsq(xy-mathToScreen(magnets[i])) <= r2)
        {
            gl_FragColor = colors[N];
            return;
        }
    }

    vec2 zxy = mathToScreen(vec2(0,0));
    if (abs(xy.x - zxy.x) < 1 || abs(xy.y - zxy.y) < 1)
    {
        gl_FragColor = vec4(0,0,0,0);
        return;
    }

    vec2 pf = predict(p0, 10, 100, 1.);
    float dists[N];
    for (int i = 0; i < N; i++)
    {
        dists[i] = nsq(pf - magnets[i]);
    }
    int im = mini(dists);
    gl_FragColor = colors[im];
}