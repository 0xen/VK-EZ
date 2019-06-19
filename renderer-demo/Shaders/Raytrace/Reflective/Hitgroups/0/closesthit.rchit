#version 460
#extension GL_NV_ray_tracing : require
#extension GL_EXT_nonuniform_qualifier : enable

struct RayPayload {
	vec3 color;
	float distance;
	vec3 normal;
	float reflector;
};

layout(location = 0) rayPayloadInNV RayPayload rayPayload;

struct Light
{
  vec4 position;
  vec4 color;
};

layout(location = 2) rayPayloadNV bool isShadowed;

hitAttributeNV vec3 attribs;
layout(binding = 0, set = 0) uniform accelerationStructureNV topLevelAS;

layout(binding = 3, set = 0) buffer Vertices { vec4 v[]; }
vertices;
layout(binding = 4, set = 0) buffer Indices { uint i[]; }
indices;

layout(binding = 5, set = 0) buffer MatColorBufferObject { vec4[] m; }
materials;
layout(binding = 6, set = 0) uniform sampler2D[] textureSamplers;

layout(binding = 7, set = 0) buffer Lights { Light l[]; }
lights;

struct Vertex
{
  vec3 pos;
  vec3 nrm;
  vec3 color;
  vec2 texCoord;
  int matIndex;
};
// Number of vec4 values used to represent a vertex
uint vertexSize = 3;

Vertex unpackVertex(uint index)
{
  Vertex v;

  vec4 d0 = vertices.v[vertexSize * index + 0];
  vec4 d1 = vertices.v[vertexSize * index + 1];
  vec4 d2 = vertices.v[vertexSize * index + 2];

  v.pos = d0.xyz;
  v.nrm = vec3(d0.w, d1.x, d1.y);
  v.color = vec3(d1.z, d1.w, d2.x);
  v.texCoord = vec2(d2.y, d2.z);
  v.matIndex = floatBitsToInt(d2.w);
  return v;
}

struct WaveFrontMaterial
{
  vec3 ambient;
  vec3 diffuse;
  vec3 specular;
  vec3 transmittance;
  vec3 emission;
  float shininess;
  float ior;      // index of refraction
  float dissolve; // 1 == opaque; 0 == fully transparent
  int illum;      // illumination model (see http://www.fileformat.info/format/material/)
  int textureId;
};
// Number of vec4 values used to represent a material
const int sizeofMat = 5;

WaveFrontMaterial unpackMaterial(int matIndex)
{
  WaveFrontMaterial m;
  vec4 d0 = materials.m[sizeofMat * matIndex + 0];
  vec4 d1 = materials.m[sizeofMat * matIndex + 1];
  vec4 d2 = materials.m[sizeofMat * matIndex + 2];
  vec4 d3 = materials.m[sizeofMat * matIndex + 3];
  vec4 d4 = materials.m[sizeofMat * matIndex + 4];

  m.ambient = vec3(d0.x, d0.y, d0.z);
  m.diffuse = vec3(d0.w, d1.x, d1.y);
  m.specular = vec3(d1.z, d1.w, d2.x);
  m.transmittance = vec3(d2.y, d2.z, d2.w);
  m.emission = vec3(d3.x, d3.y, d3.z);
  m.shininess = d3.w;
  m.ior = d4.x;
  m.dissolve = d4.y;
  m.illum = int(d4.z);
  m.textureId = floatBitsToInt(d4.w);
  return m;
}

void main()
{

  ivec3 ind = ivec3(indices.i[3 * gl_PrimitiveID], indices.i[3 * gl_PrimitiveID + 1],
                    indices.i[3 * gl_PrimitiveID + 2]);

  Vertex v0 = unpackVertex(ind.x);
  Vertex v1 = unpackVertex(ind.y);
  Vertex v2 = unpackVertex(ind.z);

  const vec3 barycentrics = vec3(1.0 - attribs.x - attribs.y, attribs.x, attribs.y);

  vec3 normal =
    normalize(v0.nrm * barycentrics.x + v1.nrm * barycentrics.y + v2.nrm * barycentrics.z);

  WaveFrontMaterial mat = unpackMaterial(v1.matIndex);

  vec3 c = mat.diffuse;//dot_product * mat.diffuse; 
  if (mat.textureId >= 0)
  {
    vec2 texCoord = v0.texCoord * barycentrics.x + v1.texCoord * barycentrics.y +
                              v2.texCoord * barycentrics.z;
    c *= texture(textureSamplers[mat.textureId], texCoord).xyz;
  }




  float tmin = 0.001;
  float tmax = 100.0;
  vec3 origin = gl_WorldRayOriginNV + gl_WorldRayDirectionNV * gl_HitTNV;

  uint lightCount = 2;

  /*for(int i = 0 ; i < lightCount; i ++)
  {

    vec3 lightVector = normalize(lights.l[i].position.xyz);
    //float dot_product = max(dot(lightVector, normal), 0.2);

    isShadowed = true;

    uint sbtRecordOffset = 0;
    uint sbtRecordStride = 0;
    uint missIndex = 1;


    traceNV(topLevelAS, gl_RayFlagsTerminateOnFirstHitNV|gl_RayFlagsOpaqueNV|gl_RayFlagsSkipClosestHitShaderNV, 
            0xFF, sbtRecordOffset, sbtRecordStride,
            missIndex, origin, tmin, normal, tmax, 2);
    if (isShadowed)
      c *= 0.3;
    else
      c += lights.l[i].color.xyz; 
  }
  c /= lightCount;*/








  rayPayload.distance = gl_RayTmaxNV;
  rayPayload.normal = normal;

  rayPayload.reflector = mat.shininess;

  rayPayload.color = c;



}

