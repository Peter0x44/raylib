#version 330

// Input vertex attributes (from vertex shader)
in vec2 fragTexCoord;
in vec4 fragColor;

// Input uniform values
uniform sampler2D texture0;     // Tree texture
uniform sampler2D texture1;     // Player spritesheet texture
uniform vec4 colDiffuse;

uniform vec2 treeSize;          // Tree texture size, in pixels
uniform vec2 playerSize;        // Player frame size, in pixels
uniform vec2 playerPosition;    // Player center relative to tree texture, in pixels
uniform vec4 playerFrame;       // Player current frame in spritesheet, normalized [x, y, width, height]

// Output fragment color
out vec4 finalColor;

const vec3 silhouetteColor = vec3(0.25, 0.45, 0.9);

void main()
{
    vec4 texelColor = texture(texture0, fragTexCoord)*colDiffuse*fragColor;

    // Map current tree pixel into player frame coordinates, [0..1] when inside the player
    vec2 frameCoord = (fragTexCoord*treeSize - playerPosition)/playerSize + 0.5;

    float playerAlpha = 0.0;
    if (all(greaterThanEqual(frameCoord, vec2(0.0))) && all(lessThanEqual(frameCoord, vec2(1.0))))
    {
        playerAlpha = texture(texture1, playerFrame.xy + frameCoord*playerFrame.zw).a;
    }

    // Where the player is behind the tree, tint the tree with the silhouette color
    texelColor.rgb = mix(texelColor.rgb, silhouetteColor, 0.8*playerAlpha);

    finalColor = texelColor;
}
