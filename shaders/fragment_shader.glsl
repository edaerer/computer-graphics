#version 330 core
out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoord;

uniform vec3 objectColor;
uniform vec3 lightPos;
uniform vec3 viewPos;
uniform bool hasTexture;
uniform sampler2D textureSampler;

void main()
{
    // Ambient
    float ambientStrength = 0.3;
    vec3 ambient = ambientStrength * vec3(1.0);
    
    // Diffuse
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * vec3(1.0);
    
    // Specular
    float specularStrength = 0.5;
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    vec3 specular = specularStrength * spec * vec3(1.0);
    
    vec3 result;
    if (hasTexture) {
        vec3 textureColor = texture(textureSampler, TexCoord).rgb;
        result = (ambient + diffuse + specular) * textureColor;
    } else {
        result = (ambient + diffuse + specular) * objectColor;
    }
    
    FragColor = vec4(result, 1.0);
}