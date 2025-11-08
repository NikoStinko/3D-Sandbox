#version 330 core
out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;

struct Material {
    sampler2D texture_diffuse1;
    sampler2D texture_specular1;
};

uniform Material material;
uniform vec3 lightPos;
uniform vec3 viewPos;

void main()
{
    // Couleurs de base
    vec3 color = texture(material.texture_diffuse1, TexCoords).rgb;
    vec3 specColor = texture(material.texture_specular1, TexCoords).rgb;

    // Calcul de la lumière (Blinn-Phong)
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 halfDir = normalize(lightDir + viewDir);

    // Composantes
    float diff = max(dot(norm, lightDir), 0.0);
    float spec = pow(max(dot(norm, halfDir), 0.0), 64.0);

    vec3 ambient = 0.1 * color;
    vec3 diffuse = diff * color;
    vec3 specular = spec * specColor;

    FragColor = vec4(ambient + diffuse + specular, 1.0);
}
