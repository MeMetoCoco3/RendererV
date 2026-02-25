#version 330 core 

out vec4 FragColor;
in vec2 TexCoords; 

uniform sampler2D screen_texture;

// const float gradient[9] = float[9](0.0, 0.125, 0.25, 0.375, 0.5, 0.625, 0.75, 0.875, 1.0);
uniform vec3[5] colors;
// const vec3 colors[5] = vec3[5](vec3(0.95,0.95,0.85), vec3(0.859,0.617,0.507), vec3(0.753,0.43,0.43), vec3(0.08,0.12,0.23), vec3(0.01,0.0,0.152));
void main()
{
    FragColor = texture(screen_texture, TexCoords);
    
    // float grad = 0.2126 * FragColor.x + 0.7152 * FragColor.y + FragColor.z * 0.0722;

    float grad = (FragColor.x + FragColor.y + FragColor.z) /3.0;
    int grad_idx =int(grad * 5);
    FragColor = vec4(colors[grad_idx], 1.0);
}
