#type vertex
#version 410 core

layout(location = 0) in vec2 a_Position;
layout(location = 1) in vec2 a_TexCoord;

out vec2 v_TexCoord;

uniform mat4 u_Projection;
uniform mat4 u_Transform;
uniform vec4 u_UVBounds; // x: minU, y: minV, z: maxU, w: maxV

void main()
{
    // Remap standard 0-1 UVs to the specific bounds in the atlas
    v_TexCoord = vec2(
        u_UVBounds.x + a_TexCoord.x * (u_UVBounds.z - u_UVBounds.x),
        u_UVBounds.y + a_TexCoord.y * (u_UVBounds.w - u_UVBounds.y)
    );
    gl_Position = u_Projection * u_Transform * vec4(a_Position, 0.0, 1.0);
}

#type fragment
#version 410 core

layout(location = 0) out vec4 color;

in vec2 v_TexCoord;

uniform vec4 u_Color;
uniform sampler2D u_Texture;
uniform bool u_UseTexture;

void main()
{
    if (u_UseTexture) {
        float alpha = texture(u_Texture, v_TexCoord).r;
        color = vec4(u_Color.rgb, u_Color.a * alpha);
    } else {
        color = u_Color;
    }
}
