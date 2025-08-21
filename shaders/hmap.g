#version 330 core
layout(triangles) in;
layout(triangle_strip, max_vertices=3) out;

in vec3 vN[];
in vec3 vW[];
in vec2 vUV[];

out vec3 gsN;
out vec2 gsUV;
flat out vec3 triColor;

uniform vec3 uLightDir = normalize(vec3(0.3,1.0,0.2));

void main()
{
    // Average normal for lighting
    vec3 avgNormal = normalize(vN[0] + vN[1] + vN[2]);
    vec2 avgUV = (vUV[0] + vUV[1] + vUV[2]) / 3.0;

    // Face normal for slope detection
    vec3 edge1 = vW[1] - vW[0];
    vec3 edge2 = vW[2] - vW[0];
    vec3 faceNormal = normalize(cross(edge1, edge2));

    // Slope angle relative to world up
    float slopeAngle = degrees(acos(dot(faceNormal, vec3(0,1,0))));

    // Determine triangle color
    if (slopeAngle > 50.0)
    {
        triColor = vec3(1.0, 0.0, 0.0); // steep -> red
    }
    else
    {
        float ndl = max(dot(avgNormal, normalize(uLightDir)), 0.0);
        vec3 base = mix(vec3(0.15,0.35,0.15), vec3(0.5,0.4,0.3), avgUV.y);
        triColor = base * (0.2 + 0.8 * ndl);
    }

    // Emit vertices
    for(int i = 0; i < 3; i++)
    {
        gsN = vN[i];
        gsUV = vUV[i];
        gl_Position = gl_in[i].gl_Position;
        EmitVertex();
    }
    EndPrimitive();
}