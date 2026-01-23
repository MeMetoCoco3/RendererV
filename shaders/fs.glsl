#version 330 core
in vec2 in_NDCCoord;
in vec2 in_UV;
out vec4 FragColor;

uniform sampler2D texture0;

const float RADIUS = 0.2;

float sdCircle( vec2 p, float r )
{
    return length(p) - r;
}

void main()
{
    float dist = sdCircle(in_NDCCoord, RADIUS);
    //float aa = fwidth(dist);

    float alpha = 1.0 - smoothstep(0.0, 0.005, dist);
    
    //FragColor = vec4(vec3(1.0), alpha);
    vec4 Tex = texture(texture0, in_UV);
    FragColor = vec4(Tex.xyz, alpha);
}

