#version 330 core
out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;

// Structure pour les propriétés du matériau
struct Material {
    sampler2D texture_diffuse1;
    sampler2D texture_specular1;
    bool hasDiffuse;
    bool hasSpecular;
};

// Structure pour la lumière directionnelle
struct DirectionalLight {
    vec3 direction;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

// Uniforms
uniform Material material;
uniform DirectionalLight dirLight;
uniform vec3 viewPos;
uniform float environmentAmbientBoost;
uniform bool highlightActive;
uniform vec3 highlightColor;

void main()
{
    // Couleurs par défaut
    vec3 diffuseColor = vec3(0.8, 0.8, 0.8); // Couleur grise par défaut
    vec3 specularColor = vec3(0.2, 0.2, 0.2); // Couleur spéculaire par défaut plus foncée
    
    // Charger les textures si disponibles
    if (material.hasDiffuse) {
        diffuseColor = texture(material.texture_diffuse1, TexCoords).rgb;
    } else {
        // Si pas de texture diffuse, utiliser une couleur unie
        diffuseColor = vec3(0.8, 0.8, 0.8);
    }
    
    if (material.hasSpecular) {
        specularColor = texture(material.texture_specular1, TexCoords).rgb;
    } else {
        // Si pas de texture spéculaire, utiliser une valeur basse
        specularColor = vec3(0.2, 0.2, 0.2);
    }

    // Calcul de la lumière (Blinn-Phong)
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(-dirLight.direction);
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 halfDir = normalize(lightDir + viewDir);

    // Composantes
    float diff = max(dot(norm, lightDir), 0.0);
    float spec = pow(max(dot(norm, halfDir), 0.0), 64.0);

    // Calcul des composantes d'éclairage
    vec3 ambient = dirLight.ambient * diffuseColor * environmentAmbientBoost;
    vec3 diffuse = dirLight.diffuse * diff * diffuseColor;
    vec3 specular = dirLight.specular * spec * specularColor;

    // Combinaison finale
    vec3 result = ambient + diffuse + specular;

    if (highlightActive) {
        result = mix(result, highlightColor, 0.5);
    }
    
    // Correction gamma
    result = pow(result, vec3(1.0/2.2));
    
    FragColor = vec4(result, 1.0);
}
