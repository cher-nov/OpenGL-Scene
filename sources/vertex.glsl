#version 330

layout (location = 0) in vec3 in_Position;
layout (location = 1) in vec2 in_TexCoord;
layout (location = 2) in vec3 in_Normal;
layout (location = 3) in vec3 in_Tangent;

uniform mat4 gMW;
uniform mat4 gMWVP;
uniform mat4 gNormalMx; //special model-view matrix for normals

out vec4 Color;
out vec2 TexCoord;
out vec3 FragNormal;
out vec3 FragTangent;
out vec3 FragPos;

void main() {
  gl_Position = gMWVP * vec4(in_Position, 1.0);
  Color = vec4( clamp(in_Position, 0.2, 1.0), 1.0 );
  TexCoord = in_TexCoord;
  FragNormal = normalize( (gNormalMx * vec4(in_Normal, 0.0)).xyz );
  FragTangent = normalize( (gNormalMx * vec4(in_Tangent, 0.0)).xyz );
  FragPos = (gMW * vec4(in_Position, 1.0)).xyz;
}
