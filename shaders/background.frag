#version 460 core

in vec2 TexCoord;
in vec3 FragPos;
in vec3 Normal;

out vec4 FragColor;

uniform sampler2D tex0;
uniform vec3 lightPos;
uniform vec3 viewPos;

void main() {
    float ambientStrength = 0.2f;
    vec3 ambient = ambientStrength * vec3(1.0f, 1.0f, 1.0f);

    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0f);
    vec3 diffuse = diff * vec3(1.0f, 1.0f, 1.0f);

    float specularStrength = 1.5f;
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0f), 32);
    vec3 specular = specularStrength * spec * vec3(1.0f, 1.0f, 1.0f);

    vec3 result = (ambient + diffuse + specular);
    FragColor = texture(tex0, TexCoord) * vec4(result, 1.0f);

    // For no lighting
    // FragColor = texture(tex0, TexCoord);
}