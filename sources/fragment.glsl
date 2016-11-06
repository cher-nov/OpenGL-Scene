#version 330

// NOTE: names of the input variables here MUST be the same
// as names of output variables in the vertex shader!

in vec4 Color;
in vec2 TexCoord;
in vec3 FragNormal;
in vec3 FragTangent;
in vec3 FragPos;

struct DirLight {
  vec3 Color;
  float AmbientIntensity;
  vec3 Direction;
  float DiffuseIntensity;
};

uniform bool NormalMapping;
uniform DirLight gDirLight;
uniform sampler2D TexGFX;
uniform sampler2D TexMap;

out vec4 FragColor;

vec3 CalcBumpedNormal()
{
  vec3 Tangent = normalize( FragTangent - dot(FragTangent, FragNormal) * FragNormal );
  vec3 Bitangent = cross( Tangent, FragNormal );

  vec3 BumpMapNormal = texture( TexMap, TexCoord ).rgb;
  BumpMapNormal = BumpMapNormal * 2.0 - 1.0; //transforms [0;1] range to [-1;1]

  mat3 TBN = mat3( Tangent, Bitangent, FragNormal );

  return normalize( TBN * BumpMapNormal );
}

void main() {
  vec3 Normal;
  if (NormalMapping)
  {
    Normal = CalcBumpedNormal();
  } else {
    Normal = FragNormal;
  }

  //vec3 LightDirection = normalize( gDirLight.Direction - FragPos );
  vec3 LightDirection = -gDirLight.Direction;
  float DiffuseFactor = max( dot(Normal, LightDirection), 0.0 );

  vec3 AmbientColor = gDirLight.Color * gDirLight.AmbientIntensity;
  vec3 DiffuseColor = gDirLight.Color * gDirLight.DiffuseIntensity * DiffuseFactor;

  FragColor = texture( TexGFX, TexCoord ) * //Color *
    vec4( AmbientColor + DiffuseColor, 1.0 );
}
