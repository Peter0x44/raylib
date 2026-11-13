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

const float innerRadius = 45.0;  // Fully transparent inside this radius
const float outerRadius = 100.0; // Fully opaque outside this radius
const float ditherSize = 4.0;    // Size of every dither cell, in pixels

// Ordered dithering threshold values (4x4 Bayer matrix), in range [0.0..1.0)
float Bayer2(vec2 a) { a = floor(a); return fract(a.x/2.0 + a.y*a.y*0.75); }
float Bayer4(vec2 a) { return Bayer2(0.5*a)*0.25 + Bayer2(a); }

void main()
{
    vec4 texelColor = texture(texture0, fragTexCoord);

    // Work on dither cells instead of single pixels, so the pattern is clearly visible
    vec2 cell = floor(fragTexCoord*treeSize/ditherSize);
    float visibility = smoothstep(innerRadius, outerRadius, distance((cell + 0.5)*ditherSize, playerPosition));

    // Discard more pixels the closer they are to the player
    if (visibility <= Bayer4(cell)) discard;

    finalColor = texelColor*colDiffuse*fragColor;
}
