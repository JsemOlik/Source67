#type vertex
#version 410 core

layout(location = 0) in vec3 a_Position;

uniform mat4 u_ViewProjection;
uniform mat4 u_Transform;

out vec3 v_Position;

void main()
{
    v_Position = a_Position;
    // We want the skybox to be centered around the camera, so we remove the translation from the view matrix
    // However, our u_ViewProjection might already have it. 
    // We'll handle centering in the Application side by setting the skybox position to the camera position.
    gl_Position = u_ViewProjection * u_Transform * vec4(a_Position, 1.0);
    // Force depth to be max
    gl_Position = gl_Position.xyww; 
}

#type fragment
#version 410 core

layout(location = 0) out vec4 color;

in vec3 v_Position;

uniform sampler2D u_SkyboxTexture;

const vec2 invAtan = vec2(0.1591, 0.3183);
vec2 SampleEquirectangularMap(vec3 v)
{
    vec2 uv = vec2(atan(v.z, v.x), asin(v.y));
    uv *= invAtan;
    uv += 0.5;
    return uv;
}

void main()
{
    vec2 uv = SampleEquirectangularMap(normalize(v_Position));
    color = texture(u_SkyboxTexture, uv);
}
