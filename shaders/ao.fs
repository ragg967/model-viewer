#version 330

in vec3 fragPosition;
in vec3 fragNormal;
in vec2 fragTexCoord;

uniform vec4 colDiffuse;
uniform vec3 viewPos;

out vec4 finalColor;

// Fake AO based on surface curvature and normal orientation
float calculateAO(vec3 normal, vec3 viewDir)
{
    // Sky occlusion - surfaces facing up get more light
    float skyAO = max(0.0, normal.y) * 0.5 + 0.5;
    
    // Fresnel-like effect - edges get darker (cavity occlusion)
    float fresnel = pow(1.0 - max(0.0, dot(normal, viewDir)), 2.0);
    float cavityAO = 1.0 - fresnel * 0.4;
    
    // Height-based darkening - lower parts are darker
    float heightAO = smoothstep(-2.0, 2.0, fragPosition.y) * 0.3 + 0.7;
    
    // Combine AO factors
    return skyAO * cavityAO * heightAO;
}

void main()
{
    vec3 normal = normalize(fragNormal);
    vec3 viewDir = normalize(viewPos - fragPosition);
    
    // Fake directional light from above-front
    vec3 lightDir = normalize(vec3(0.3, 0.8, 0.5));
    float diffuse = max(dot(normal, lightDir), 0.0);
    
    // Softer back lighting
    vec3 backLightDir = normalize(vec3(-0.2, 0.2, -0.5));
    float backLight = max(dot(normal, backLightDir), 0.0) * 0.3;
    
    // Calculate ambient occlusion
    float ao = calculateAO(normal, viewDir);
    
    // Ambient term with AO
    vec3 ambient = vec3(0.4, 0.45, 0.5) * ao;
    
    // Combine lighting
    vec3 lighting = ambient + vec3(diffuse) * vec3(1.0, 0.95, 0.9) + vec3(backLight) * vec3(0.5, 0.6, 0.8);
    
    // Apply to material color
    vec3 color = colDiffuse.rgb * lighting;
    
    // Slight color grading for better look
    color = pow(color, vec3(0.95)); // Slight gamma adjustment
    
    finalColor = vec4(color, colDiffuse.a);
}
