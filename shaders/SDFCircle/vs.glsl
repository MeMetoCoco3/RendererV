#version 330 core
layout(location = 0) in vec3 aPos;

out vec2 in_NDCCoord;
out vec2 in_UV;
uniform mat4 model_mat;
uniform mat4 view_mat;
uniform mat4 proj_mat;

void main()
{
    gl_Position = proj_mat * view_mat * model_mat * vec4(aPos, 1.0);

    in_NDCCoord = aPos.xy;
    in_UV.x = (aPos.x * 0.5) + 0.5;
    in_UV.y = -(aPos.y * 0.5) + 0.5;
}

