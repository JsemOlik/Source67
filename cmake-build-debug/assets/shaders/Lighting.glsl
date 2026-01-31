#type vertex
#version 330 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec3 a_Normal;
layout(location = 2) in vec2 a_TexCoord;

uniform mat4 u_ViewProjection;
uniform mat4 u_Transform;

out vec3 v_Normal;
out vec3 v_FragPos;
out vec2 v_TexCoord;

void main() {
    v_Normal = mat3(transpose(inverse(u_Transform))) * a_Normal;
    v_FragPos = vec3(u_Transform * vec4(a_Position, 1.0));
    v_TexCoord = a_TexCoord;
    
    gl_Position = u_ViewProjection * vec4(v_FragPos, 1.0);
}

#type fragment
#version 330 core

layout(location = 0) out vec4 color;

struct DirLight {
    vec3 Direction;
    vec3 Color;
    float Intensity;
};

in vec3 v_Normal;
in vec3 v_FragPos;
in vec2 v_TexCoord;

uniform DirLight u_DirLight;
uniform sampler2D u_Texture;
uniform vec2 u_Tiling;

void main() {
    // Ambient
    float ambientStrength = 0.1;
    vec3 ambient = ambientStrength * u_DirLight.Color;
    
    // Diffuse
    vec3 norm = normalize(v_Normal);
    vec3 lightDir = normalize(-u_DirLight.Direction);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * u_DirLight.Color;
    
    vec4 texColor = texture(u_Texture, v_TexCoord * u_Tiling);
    color = vec4((ambient + diffuse) * texColor.rgb, 1.0) * u_DirLight.Intensity;
}
