#version 330

// Input vertex attributes (from vertex shader)
in vec2 fragTexCoord;
in vec4 fragColor;

// Input uniform values
uniform sampler2D texture0;
uniform vec4 colDiffuse;

uniform vec2 treeSize;          // Tree texture size, in pixels
uniform vec2 playerPosition;    // Player center relative to tree texture, in pixels

// Output fragment color
out vec4 finalColor;

const float radius = 75.0;      // Cutout circle radius, in pixels

void main()
{
    vec4 texelColor = texture(texture0, fragTexCoord)*colDiffuse*fragColor;

    // Remove pixels inside the circle, smoothing its edge by one pixel
    float dist = distance(fragTexCoord*treeSize, playerPosition);
    texelColor.a *= smoothstep(radius - 1.0, radius + 1.0, dist);

    finalColor = texelColor;
}
